
#include "VulkanBuffer.h"

#include "VulkanDescriptorManager.h"
#include "globalincs/pstypes.h"
#include <cstdint>
#include <limits>
#include <vector>

namespace graphics {
namespace vulkan {

// Direct file logging for debugging crashes
static void buf_debug(const char* msg) {
	FILE* f = fopen("vulkan_debug.log", "a");
	if (f) {
		fprintf(f, "VulkanBuffer: %s\n", msg);
		fflush(f);
		fclose(f);
	}
}

// Global buffer manager instance
VulkanBufferManager* g_vulkanBufferManager = nullptr;

// ============================================================================
// VulkanBufferManager Implementation
// ============================================================================

VulkanBufferManager::VulkanBufferManager(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue graphicsQueue, uint32_t graphicsQueueFamily)
    : m_device(device), m_physicalDevice(physicalDevice), m_graphicsQueue(graphicsQueue), m_graphicsQueueFamily(graphicsQueueFamily)
{
	initialize(device, physicalDevice, graphicsQueue, graphicsQueueFamily);
}

VulkanBufferManager::~VulkanBufferManager()
{
	shutdown();
}

void VulkanBufferManager::initialize(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Queue graphicsQueue, uint32_t graphicsQueueFamily)
{
	if (m_initialized) {
		return;
	}

	m_device = device;
	m_physicalDevice = physicalDevice;
	m_graphicsQueue = graphicsQueue;
	m_graphicsQueueFamily = graphicsQueueFamily;
	m_memoryProperties = physicalDevice.getMemoryProperties();

	// Query device limits
	auto properties = physicalDevice.getProperties();
	m_minUboAlignment = static_cast<size_t>(properties.limits.minUniformBufferOffsetAlignment);

	mprintf(("Vulkan Buffer Manager initialized\n"));
	mprintf(("  Min UBO alignment: %zu bytes\n", m_minUboAlignment));

	// Create command pool for transfer operations (on graphics queue for sync)
	vk::CommandPoolCreateInfo poolInfo;
	poolInfo.queueFamilyIndex = m_graphicsQueueFamily;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	m_commandPool = m_device.createCommandPool(poolInfo);

	// Allocate per-frame transfer command buffers
	vk::CommandBufferAllocateInfo cmdAlloc;
	cmdAlloc.commandPool = m_commandPool;
	cmdAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdAlloc.commandBufferCount = FRAMES_IN_FLIGHT;

	auto cmdBuffers = m_device.allocateCommandBuffers(cmdAlloc);
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
		m_transferCmds[i] = cmdBuffers[i];
		m_transferCmdRecording[i] = false;
		m_transferFenceSubmitted[i] = false;  // No pending work to wait for
		// Create unsignaled fence - we track whether to wait via m_transferFenceSubmitted
		vk::FenceCreateInfo fenceInfo;
		m_transferFences[i] = m_device.createFence(fenceInfo);
	}

	// Create placeholder buffer for uninitialized uniform bindings
	// This ensures all descriptor bindings are valid even before real buffers are bound
	vk::BufferCreateInfo placeholderInfo;
	placeholderInfo.size = PLACEHOLDER_BUFFER_SIZE;
	placeholderInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	placeholderInfo.sharingMode = vk::SharingMode::eExclusive;
	m_placeholderUniformBuffer = m_device.createBuffer(placeholderInfo);

	auto memReqs = m_device.getBufferMemoryRequirements(m_placeholderUniformBuffer);
	vk::MemoryAllocateInfo memAlloc;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits,
	    vk::MemoryPropertyFlagBits::eDeviceLocal);
	m_placeholderUniformMemory = m_device.allocateMemory(memAlloc);
	m_device.bindBufferMemory(m_placeholderUniformBuffer, m_placeholderUniformMemory, 0);
	mprintf(("VulkanBuffer: created placeholder uniform buffer (%zu bytes)\n", PLACEHOLDER_BUFFER_SIZE));

	m_initialized = true;
}

void VulkanBufferManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Delete all active buffers
	for (auto& buffer : m_buffers) {
		if (buffer.buffer) {
			if (buffer.mappedPtr) {
				m_device.unmapMemory(buffer.memory);
				buffer.mappedPtr = nullptr;
			}
			m_device.destroyBuffer(buffer.buffer);
			m_device.freeMemory(buffer.memory);
			buffer.buffer = nullptr;
			buffer.memory = nullptr;
		}
	}
	m_buffers.clear();
	m_freeSlots.clear();

	// Clean up any pending deferred deletions from all frame queues
	for (auto& frameQueue : m_pendingDeletions) {
		for (auto& pending : frameQueue) {
			m_device.destroyBuffer(pending.buffer);
			m_device.freeMemory(pending.memory);
		}
		frameQueue.clear();
	}

	// Clean up any pending staging buffer deletions
	for (auto& frameQueue : m_pendingStagingDeletions) {
		for (auto& pending : frameQueue) {
			m_device.destroyBuffer(pending.buffer);
			m_device.freeMemory(pending.memory);
		}
		frameQueue.clear();
	}

	// Free per-frame command buffers before destroying the pool
	if (m_commandPool && m_transferCmds[0]) {
		m_device.freeCommandBuffers(m_commandPool, m_transferCmds);
		for (auto& cmd : m_transferCmds) {
			cmd = nullptr;
		}
	}

	// Destroy transfer fences
	for (auto& fence : m_transferFences) {
		if (fence) {
			m_device.destroyFence(fence);
			fence = nullptr;
		}
	}

	if (m_commandPool) {
		m_device.destroyCommandPool(m_commandPool);
		m_commandPool = nullptr;
	}

	// Destroy placeholder uniform buffer
	if (m_placeholderUniformBuffer) {
		m_device.destroyBuffer(m_placeholderUniformBuffer);
		m_placeholderUniformBuffer = nullptr;
	}
	if (m_placeholderUniformMemory) {
		m_device.freeMemory(m_placeholderUniformMemory);
		m_placeholderUniformMemory = nullptr;
	}

	m_initialized = false;
	mprintf(("Vulkan Buffer Manager shut down\n"));
}

uint32_t VulkanBufferManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
	for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
		    (m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	Error(LOCATION, "Vulkan: Failed to find suitable memory type!");
	return 0;
}

void VulkanBufferManager::createVkBuffer(VulkanBufferData& bufferData, size_t size, vk::BufferUsageFlags usage,
                                         vk::MemoryPropertyFlags properties)
{
	// Destroy existing buffer if any
	if (bufferData.buffer) {
		if (bufferData.mappedPtr) {
			m_device.unmapMemory(bufferData.memory);
			bufferData.mappedPtr = nullptr;
		}

		// Always defer deletion - the old buffer may be in-flight on the GPU
		queueDeferredDeletion(bufferData.buffer, bufferData.memory);
	}

	// Check if this is a uniform buffer - enable BDA for uniform buffers
	bool enableBDA = (usage & vk::BufferUsageFlagBits::eUniformBuffer) == vk::BufferUsageFlagBits::eUniformBuffer;
	if (enableBDA) {
		usage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
	}

	// Create buffer
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	bufferData.buffer = m_device.createBuffer(bufferInfo);

	// Get memory requirements
	auto memRequirements = m_device.getBufferMemoryRequirements(bufferData.buffer);

	// Allocate memory with device address support for BDA-enabled buffers
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	// For BDA, add memory allocate flags info
	vk::MemoryAllocateFlagsInfo allocFlagsInfo;
	if (enableBDA) {
		allocFlagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
		allocInfo.pNext = &allocFlagsInfo;
	}

	bufferData.memory = m_device.allocateMemory(allocInfo);
	m_device.bindBufferMemory(bufferData.buffer, bufferData.memory, 0);

	bufferData.size = size;
	bufferData.hostVisible = (properties & vk::MemoryPropertyFlagBits::eHostVisible) ==
	                         vk::MemoryPropertyFlagBits::eHostVisible;

	// Get device address for BDA-enabled buffers
	if (enableBDA) {
		vk::BufferDeviceAddressInfo addressInfo;
		addressInfo.buffer = bufferData.buffer;
		bufferData.deviceAddress = m_device.getBufferAddress(addressInfo);
	} else {
		bufferData.deviceAddress = 0;
	}
}

gr_buffer_handle VulkanBufferManager::createBuffer(BufferType type, BufferUsageHint usage)
{
	Assertion(m_initialized, "VulkanBufferManager not initialized!");

	VulkanBufferData bufferData;
	bufferData.type = type;
	bufferData.usage = usage;
	bufferData.lastUsedFrame = m_currentFrameIndex;

	// Determine handle slot
	int slot;
	if (!m_freeSlots.empty()) {
		slot = m_freeSlots.back();
		m_freeSlots.pop_back();
		m_buffers[slot] = bufferData;
	} else {
		slot = static_cast<int>(m_buffers.size());
		m_buffers.push_back(bufferData);
	}

	mprintf(("Vulkan: Created buffer handle %d (type=%d, usage=%d)\n", slot, static_cast<int>(type),
	         static_cast<int>(usage)));

	return gr_buffer_handle(slot);
}

void VulkanBufferManager::deleteBuffer(gr_buffer_handle handle)
{
	if (!handle.isValid() || static_cast<size_t>(handle.value()) >= m_buffers.size()) {
		return;
	}

	VulkanBufferData& buffer = m_buffers[handle.value()];

	if (buffer.buffer) {
		// Unmap if mapped
		if (buffer.mappedPtr) {
			m_device.unmapMemory(buffer.memory);
			buffer.mappedPtr = nullptr;
		}

		// Always defer deletion - the buffer may be in-flight on the GPU
		// It will be destroyed when beginFrame() processes this frame's queue
		// after the corresponding fence has been waited on
		queueDeferredDeletion(buffer.buffer, buffer.memory);

		buffer.buffer = nullptr;
		buffer.memory = nullptr;
	}

	buffer.size = 0;
	m_freeSlots.push_back(handle.value());
}

void VulkanBufferManager::updateBufferData(gr_buffer_handle handle, size_t size, const void* data)
{
	Assertion(size > 0, "Buffer update must include data!");
	Assertion(handle.isValid(), "Invalid buffer handle!");
	Assertion(static_cast<size_t>(handle.value()) < m_buffers.size(), "Buffer handle out of range!");

	VulkanBufferData& buffer = m_buffers[handle.value()];

	// Determine buffer usage flags based on type
	vk::BufferUsageFlags usageFlags;
	switch (buffer.type) {
	case BufferType::Vertex:
		usageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
		break;
	case BufferType::Index:
		usageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
		break;
	case BufferType::Uniform:
		usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		break;
	default:
		UNREACHABLE("Unknown buffer type!");
		return;
	}

	// Add transfer destination flag for staging buffer uploads
	usageFlags |= vk::BufferUsageFlagBits::eTransferDst;

	// Determine memory properties based on usage hint
	vk::MemoryPropertyFlags memProperties;
	switch (buffer.usage) {
	case BufferUsageHint::Static:
		// Prefer device-local; will use staging copy if graphics queue available, otherwise fall back to host visible
		if (m_graphicsQueue) {
			memProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		} else {
			memProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			buffer.hostVisible = true;
		}
		break;
	case BufferUsageHint::Dynamic:
	case BufferUsageHint::Streaming:
		// Host-visible for frequent CPU updates
		memProperties =
		    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		break;
	case BufferUsageHint::PersistentMapping:
		// Host-visible, will be persistently mapped
		memProperties =
		    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		break;
	default:
		UNREACHABLE("Unknown buffer usage hint!");
		return;
	}

	// Create or resize buffer
	if (!buffer.buffer || buffer.size != size) {
		createVkBuffer(buffer, size, usageFlags, memProperties);
	}

	// Update data
	if (data != nullptr) {
		if (buffer.hostVisible) {
			// Direct host-visible upload
			void* mapped = m_device.mapMemory(buffer.memory, 0, size);
			auto* dst = reinterpret_cast<std::uint8_t*>(mapped);
			auto* src = reinterpret_cast<const std::uint8_t*>(data);
			memcpy(dst, src, size);
			m_device.unmapMemory(buffer.memory);
		} else {
			// Device-local: need staging buffer
			copyViaStaging(buffer, 0, size, data);
		}
	}

	// Set up persistent mapping if requested
	if (buffer.usage == BufferUsageHint::PersistentMapping && !buffer.mappedPtr) {
		buffer.mappedPtr = m_device.mapMemory(buffer.memory, 0, buffer.size);
	}

	buffer.lastUsedFrame = m_currentFrameIndex;
}

void VulkanBufferManager::updateBufferDataOffset(gr_buffer_handle handle, size_t offset, size_t size, const void* data)
{
	Assertion(handle.isValid(), "Invalid buffer handle!");
	Assertion(static_cast<size_t>(handle.value()) < m_buffers.size(), "Buffer handle out of range!");
	Assertion(data != nullptr, "Data cannot be null for offset update!");

	VulkanBufferData& buffer = m_buffers[handle.value()];
	Assertion(buffer.buffer, "Buffer not allocated!");
	Assertion(offset + size <= buffer.size, "Update would overflow buffer!");
	Assertion(buffer.usage != BufferUsageHint::PersistentMapping,
	          "Use map/flush for persistently mapped buffers!");

	if (buffer.hostVisible) {
		void* mapped = m_device.mapMemory(buffer.memory, offset, size);
		auto* dst = reinterpret_cast<std::uint8_t*>(mapped);
		auto* src = reinterpret_cast<const std::uint8_t*>(data);
		memcpy(dst, src, size);
		m_device.unmapMemory(buffer.memory);
	} else {
		copyViaStaging(buffer, offset, size, data);
	}

	buffer.lastUsedFrame = m_currentFrameIndex;
}

void* VulkanBufferManager::mapBuffer(gr_buffer_handle handle)
{
	Assertion(handle.isValid(), "Invalid buffer handle!");
	Assertion(static_cast<size_t>(handle.value()) < m_buffers.size(), "Buffer handle out of range!");

	VulkanBufferData& buffer = m_buffers[handle.value()];
	Assertion(buffer.usage == BufferUsageHint::PersistentMapping,
	          "Only persistently mapped buffers can be mapped!");
	Assertion(buffer.buffer, "Buffer not allocated!");

	if (!buffer.mappedPtr) {
		buffer.mappedPtr = m_device.mapMemory(buffer.memory, 0, buffer.size);
	}

	return buffer.mappedPtr;
}

void VulkanBufferManager::flushMappedBuffer(gr_buffer_handle handle, size_t offset, size_t size)
{
	Assertion(handle.isValid(), "Invalid buffer handle!");
	Assertion(static_cast<size_t>(handle.value()) < m_buffers.size(), "Buffer handle out of range!");

	VulkanBufferData& buffer = m_buffers[handle.value()];
	Assertion(buffer.usage == BufferUsageHint::PersistentMapping, "Buffer is not persistently mapped!");
	Assertion(buffer.mappedPtr, "Buffer is not mapped!");

	// For coherent memory, no explicit flush needed
	// If using non-coherent memory, would need vkFlushMappedMemoryRanges here

	buffer.lastUsedFrame = m_currentFrameIndex;
}

void VulkanBufferManager::bindUniformBuffer(uniform_block_type bindPoint, size_t offset, size_t size,
                                            gr_buffer_handle handle)
{
	// Validate alignment
	Assertion(offset % m_minUboAlignment == 0,
	          "UBO offset %zu must be aligned to %zu!", offset, m_minUboAlignment);

	if (!handle.isValid()) {
		// Unbind - clear the binding
		auto it = m_boundUniformBuffers.find(bindPoint);
		if (it != m_boundUniformBuffers.end()) {
			it->second = BoundUniformBuffer(); // Clear but keep entry
		}
		// NOTE: Don't update descriptor set here - unbinding is handled by
		// not including this binding in dynamic offsets, or by binding a null buffer
		// at frame start if needed.
		return;
	}

	Assertion(static_cast<size_t>(handle.value()) < m_buffers.size(),
	          "Buffer handle out of range!");

	VulkanBufferData& buffer = m_buffers[handle.value()];
	Assertion(buffer.type == BufferType::Uniform,
	          "Only uniform buffers can be bound to UBO points!");

	vk::Buffer vkBuffer = buffer.buffer;
	if (!vkBuffer) {
		mprintf(("VulkanBufferManager: Invalid buffer handle for uniform binding\n"));
		return;
	}

	// Check if this is a new buffer (different from what was previously bound)
	BoundUniformBuffer& bound = m_boundUniformBuffers[bindPoint];
	bool bufferChanged = (bound.vkBuffer != vkBuffer);

	// Store binding info - offset and size are used as dynamic offsets
	bound.handle = handle;
	bound.offset = offset;
	bound.size = size;
	bound.vkBuffer = vkBuffer;
	bound.deviceAddress = buffer.deviceAddress;  // Store BDA address

	buffer.lastUsedFrame = m_currentFrameIndex;

	// Handle buffer changes - the underlying VkBuffer may have been reallocated
	// (e.g., FSO's UniformBufferManager::changeSegmentSize() grows the ring buffer)
	if (bufferChanged && m_uniformDescriptorSet && m_descriptorManager) {
		if (m_uniformSetEverBound) {
			// The descriptor set has been bound to a command buffer.
			// We CANNOT update the existing set - it's in-flight on the GPU.
			// We MUST recreate the descriptor set to point to the new buffer.
			mprintf(("VulkanBufferManager: Buffer changed after bind for binding %d! Recreating descriptor set.\n",
			         static_cast<int>(bindPoint)));
			if (!recreateUniformDescriptorSet()) {
				mprintf(("VulkanBufferManager: CRITICAL - Failed to recreate descriptor set!\n"));
				return;
			}
			// After recreation, m_uniformSetEverBound is false and all bindings
			// have been re-initialized. Now update this specific binding with the new buffer.
		}

		// Update the descriptor with the new buffer
		// For dynamic uniform buffers, the "range" in the descriptor is the size
		// of data accessed per-draw (not the full buffer). The dynamic offset
		// selects which portion of the buffer to access.
		m_descriptorManager->updateUniformBuffer(m_uniformDescriptorSet,
		                                         static_cast<uint32_t>(bindPoint),
		                                         vkBuffer, 0, size, true);
	}
}

VulkanBufferManager::BoundUniformBuffer VulkanBufferManager::getBoundUniformBuffer(uniform_block_type bindPoint) const
{
	auto it = m_boundUniformBuffers.find(bindPoint);
	if (it != m_boundUniformBuffers.end()) {
		return it->second;
	}
	return BoundUniformBuffer(); // Invalid
}

vk::DeviceAddress VulkanBufferManager::getBufferDeviceAddress(gr_buffer_handle handle) const
{
	if (!handle.isValid() || static_cast<size_t>(handle.value()) >= m_buffers.size()) {
		return 0;
	}
	return m_buffers[handle.value()].deviceAddress;
}

VulkanBufferManager::UniformAddressPushConstants VulkanBufferManager::getUniformAddresses() const
{
	UniformAddressPushConstants result = {};

	for (int i = 0; i < static_cast<int>(uniform_block_type::NUM_BLOCK_TYPES); ++i) {
		auto bindPoint = static_cast<uniform_block_type>(i);
		auto it = m_boundUniformBuffers.find(bindPoint);
		if (it != m_boundUniformBuffers.end() && it->second.isValid()) {
			// Use effective address (base + offset) for direct shader access
			result.addresses[i] = it->second.getEffectiveAddress();
		} else {
			result.addresses[i] = 0;  // Invalid/unbound
		}
	}

	return result;
}

bool VulkanBufferManager::initializeUniformDescriptorSet(vk::DescriptorSetLayout layout)
{
	if (!m_descriptorManager || !layout) {
		return false;
	}

	m_uniformDescriptorSetLayout = layout;
	m_uniformDescriptorSet = m_descriptorManager->allocateSet(layout, "UniformBuffers");

	if (!m_uniformDescriptorSet) {
		mprintf(("VulkanBufferManager: Failed to allocate uniform buffer descriptor set\n"));
		return false;
	}

	// Initialize all bindings to placeholder buffer to ensure descriptors are valid
	// Real buffers will update these as they are bound (before first command buffer bind)
	for (int i = 0; i < static_cast<int>(uniform_block_type::NUM_BLOCK_TYPES); ++i) {
		auto blockType = static_cast<uniform_block_type>(i);
		m_boundUniformBuffers[blockType] = BoundUniformBuffer();

		// Initialize descriptor to point to placeholder buffer
		// This ensures all bindings are valid even if never explicitly bound
		if (m_placeholderUniformBuffer) {
			m_descriptorManager->updateUniformBuffer(m_uniformDescriptorSet,
			    static_cast<uint32_t>(i), m_placeholderUniformBuffer, 0, PLACEHOLDER_BUFFER_SIZE, true);
		}
	}
	mprintf(("VulkanBufferManager: Initialized %d uniform buffer bindings with placeholder\n",
	         static_cast<int>(uniform_block_type::NUM_BLOCK_TYPES)));

	return true;
}

bool VulkanBufferManager::recreateUniformDescriptorSet()
{
	if (!m_descriptorManager || !m_uniformDescriptorSetLayout) {
		mprintf(("VulkanBufferManager: Cannot recreate uniform descriptor set - missing manager or layout\n"));
		return false;
	}

	// Queue old set for deferred deletion (GPU may still be using it)
	if (m_uniformDescriptorSet) {
		mprintf(("VulkanBufferManager: Queueing old uniform descriptor set %p for deletion\n",
		         (void*)static_cast<VkDescriptorSet>(m_uniformDescriptorSet)));
		queueDescriptorSetDeletion(m_uniformDescriptorSet);
	}

	// Allocate fresh descriptor set
	m_uniformDescriptorSet = m_descriptorManager->allocateSet(
	    m_uniformDescriptorSetLayout, "UniformBuffers-Recreated");

	if (!m_uniformDescriptorSet) {
		mprintf(("VulkanBufferManager: CRITICAL - Failed to recreate uniform descriptor set\n"));
		return false;
	}

	// Reset the bound flag so we can update descriptors
	m_uniformSetEverBound = false;

	// Re-initialize all bindings with current state
	for (int i = 0; i < static_cast<int>(uniform_block_type::NUM_BLOCK_TYPES); ++i) {
		auto blockType = static_cast<uniform_block_type>(i);
		auto& bound = m_boundUniformBuffers[blockType];

		vk::Buffer bufferToBind = m_placeholderUniformBuffer;
		size_t sizeToBind = PLACEHOLDER_BUFFER_SIZE;

		// Use real buffer if one is bound and valid
		if (bound.isValid() && bound.vkBuffer) {
			bufferToBind = bound.vkBuffer;
			sizeToBind = bound.size;
		}

		m_descriptorManager->updateUniformBuffer(m_uniformDescriptorSet,
		    static_cast<uint32_t>(i), bufferToBind, 0, sizeToBind, true);
	}

	mprintf(("VulkanBufferManager: Recreated uniform descriptor set %p due to buffer reallocation\n",
	         (void*)static_cast<VkDescriptorSet>(m_uniformDescriptorSet)));
	return true;
}

vk::Buffer VulkanBufferManager::getBuffer(gr_buffer_handle handle) const
{
	if (!handle.isValid() || static_cast<size_t>(handle.value()) >= m_buffers.size()) {
		return vk::Buffer();
	}
	return m_buffers[handle.value()].buffer;
}

const VulkanBufferData* VulkanBufferManager::getBufferData(gr_buffer_handle handle) const
{
	if (!handle.isValid() || static_cast<size_t>(handle.value()) >= m_buffers.size()) {
		return nullptr;
	}
	return &m_buffers[handle.value()];
}

void VulkanBufferManager::beginFrame(uint32_t frameIndex)
{
	char buf[128];
	sprintf(buf, "beginFrame: frameIndex=%u, old_currentFrameIndex=%u", frameIndex, m_currentFrameIndex);
	buf_debug(buf);

	Assertion(frameIndex < FRAMES_IN_FLIGHT, "Frame index %u out of range!", frameIndex);

	m_currentFrameIndex = frameIndex;

	// Reset uniform set bound flag at frame start - allows descriptor updates before first bind
	m_uniformSetEverBound = false;

	// Also reset tracking state for the current descriptor set so it can be updated
	if (m_uniformDescriptorSet && m_descriptorManager) {
		m_descriptorManager->resetTrackingForReuse(m_uniformDescriptorSet, "UniformBuffers-FrameReset");
	}

	// Wait for this frame's transfer fence to ensure the per-frame transfer
	// command buffer and staging allocations are no longer in-flight.
	// Only wait if transfers were actually submitted for this frame.
	if (m_transferFenceSubmitted[frameIndex]) {
		auto tfence = m_transferFences[frameIndex];
		if (tfence) {
			mprintf(("VulkanBuffer: beginFrame idx=%u waiting transfer fence=%p\n",
				frameIndex, reinterpret_cast<void*>(static_cast<VkFence>(tfence))));
			char fbuf[128];
			sprintf(fbuf, "beginFrame: waiting transfer fence[%u]=%p",
				frameIndex, (void*)static_cast<VkFence>(tfence));
			buf_debug(fbuf);
			auto waitResult = m_device.waitForFences(tfence, true, std::numeric_limits<uint64_t>::max());
			if (waitResult != vk::Result::eSuccess) {
				mprintf(("VulkanBuffer: WARNING - waitForFences returned %d\n", static_cast<int>(waitResult)));
			}
			m_device.resetFences(tfence);
			mprintf(("VulkanBuffer: beginFrame idx=%u transfer fence reset\n",
				frameIndex));
			sprintf(fbuf, "beginFrame: reset transfer fence[%u]=%p",
				frameIndex, (void*)static_cast<VkFence>(tfence));
			buf_debug(fbuf);
		}
		m_transferFenceSubmitted[frameIndex] = false;
	} else {
		mprintf(("VulkanBuffer: beginFrame idx=%u no transfer fence wait needed\n", frameIndex));
	}

	// Process deferred deletions for this frame index
	// Since VulkanRenderer calls this AFTER waiting on this frame's fence,
	// and we just waited on the transfer fence above, it is now safe to free.
	//
	// IMPORTANT: Descriptor sets must be freed FIRST because they reference buffers.
	// Freeing buffers while descriptor sets still reference them causes validation errors.

	// 1. Process deferred descriptor set deletions FIRST
	auto& descriptorSetQueue = m_pendingDescriptorSetDeletions[frameIndex];
	sprintf(buf, "beginFrame: processing %zu pending descriptor set deletions", descriptorSetQueue.size());
	buf_debug(buf);
	for (auto& set : descriptorSetQueue) {
		if (set && m_descriptorManager) {
			m_descriptorManager->freeSet(set);
		}
	}
	descriptorSetQueue.clear();

	// 2. Process deferred buffer deletions (now safe - no descriptors reference them)
	auto& deletionQueue = m_pendingDeletions[frameIndex];
	sprintf(buf, "beginFrame: processing %zu pending deletions", deletionQueue.size());
	buf_debug(buf);
	for (auto& pending : deletionQueue) {
		m_device.destroyBuffer(pending.buffer);
		m_device.freeMemory(pending.memory);
	}
	deletionQueue.clear();

	// 3. Process deferred staging buffer deletions
	auto& stagingQueue = m_pendingStagingDeletions[frameIndex];
	sprintf(buf, "beginFrame: processing %zu pending staging deletions", stagingQueue.size());
	buf_debug(buf);
	for (auto& pending : stagingQueue) {
		m_device.destroyBuffer(pending.buffer);
		m_device.freeMemory(pending.memory);
	}
	stagingQueue.clear();

	// Reset transfer command buffer recording state for this frame
	m_transferCmdRecording[frameIndex] = false;
	mprintf(("VulkanBuffer: beginFrame idx=%u cmd=%p recording=%d\n",
		frameIndex,
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(m_transferCmds[frameIndex])),
		0));
	buf_debug("beginFrame: done");
}

void VulkanBufferManager::queueDeferredDeletion(vk::Buffer buffer, vk::DeviceMemory memory)
{
	if (!buffer) {
		return;
	}

	PendingBufferDeletion pending;
	pending.buffer = buffer;
	pending.memory = memory;

	// Queue for deletion when this frame index is next processed
	// That happens after VulkanRenderer waits on this frame's fence
	m_pendingDeletions[m_currentFrameIndex].push_back(pending);
}

void VulkanBufferManager::queueStagingDeletion(vk::Buffer buffer, vk::DeviceMemory memory)
{
	if (!buffer) {
		return;
	}

	PendingBufferDeletion pending;
	pending.buffer = buffer;
	pending.memory = memory;

	// Queue staging buffer for deletion when this frame's fence is waited on
	m_pendingStagingDeletions[m_currentFrameIndex].push_back(pending);

	char debugbuf[256];
	sprintf(debugbuf, "queueStagingDeletion: staging=%p queued to frame %u (queue now has %zu entries)",
		(void*)static_cast<VkBuffer>(buffer),
		m_currentFrameIndex,
		m_pendingStagingDeletions[m_currentFrameIndex].size());
	buf_debug(debugbuf);
}

void VulkanBufferManager::queueDescriptorSetDeletion(vk::DescriptorSet set)
{
	if (!set) {
		return;
	}

	m_pendingDescriptorSetDeletions[m_currentFrameIndex].push_back(set);

	char debugbuf[256];
	sprintf(debugbuf, "queueDescriptorSetDeletion: set=%p queued to frame %u (queue now has %zu entries)",
		(void*)static_cast<VkDescriptorSet>(set),
		m_currentFrameIndex,
		m_pendingDescriptorSetDeletions[m_currentFrameIndex].size());
	buf_debug(debugbuf);
}

void VulkanBufferManager::copyViaStaging(VulkanBufferData& dst, size_t offset, size_t size, const void* data)
{
	char debugbuf[256];
	sprintf(debugbuf, "copyViaStaging: frameIndex=%u, size=%zu, offset=%zu, wasRecording=%d",
		m_currentFrameIndex, size, offset, m_transferCmdRecording[m_currentFrameIndex] ? 1 : 0);
	buf_debug(debugbuf);

	// Log destination buffer state - this is a critical suspect for DEVICE_LOST
	sprintf(debugbuf, "copyViaStaging: dst.buffer=%p, dst.size=%zu, dst.type=%d, dst.hostVisible=%d",
		(void*)static_cast<VkBuffer>(dst.buffer), dst.size, static_cast<int>(dst.type), dst.hostVisible ? 1 : 0);
	buf_debug(debugbuf);

	if (!dst.buffer) {
		buf_debug("copyViaStaging: ERROR - dst.buffer is NULL! Aborting copy.");
		return;
	}

	if (offset + size > dst.size) {
		sprintf(debugbuf, "copyViaStaging: ERROR - copy would overflow! offset=%zu + size=%zu > dst.size=%zu",
			offset, size, dst.size);
		buf_debug(debugbuf);
		return;
	}

	if (!m_commandPool || !m_graphicsQueue) {
		buf_debug("copyViaStaging: no command pool or queue, skipping");
		return;
	}

	// Create a staging buffer for this copy
	vk::BufferCreateInfo stagingBufferInfo;
	stagingBufferInfo.size = size;
	stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	stagingBufferInfo.sharingMode = vk::SharingMode::eExclusive;

	auto stagingBuffer = m_device.createBuffer(stagingBufferInfo);
	auto memRequirements = m_device.getBufferMemoryRequirements(stagingBuffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
	    memRequirements.memoryTypeBits,
	    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	auto stagingMemory = m_device.allocateMemory(allocInfo);
	m_device.bindBufferMemory(stagingBuffer, stagingMemory, 0);

	// Log staging buffer info
	sprintf(debugbuf, "copyViaStaging: staging=%p, stagingMem=%p, allocSize=%zu",
		(void*)static_cast<VkBuffer>(stagingBuffer),
		(void*)static_cast<VkDeviceMemory>(stagingMemory),
		static_cast<size_t>(allocInfo.allocationSize));
	buf_debug(debugbuf);

	// Copy data to staging buffer
	void* mapped = m_device.mapMemory(stagingMemory, 0, size);
	auto* dstPtr = static_cast<uint8_t*>(mapped);
	auto* srcPtr = static_cast<const uint8_t*>(data);
	memcpy(dstPtr, srcPtr, size);
	m_device.unmapMemory(stagingMemory);

	// Begin recording to per-frame command buffer if not already recording
	vk::CommandBuffer cmdBuffer = m_transferCmds[m_currentFrameIndex];
	char cmdbuf[256];
	sprintf(cmdbuf, "copyViaStaging: cmdBuffer[%u]=%p, wasRecording=%d",
		m_currentFrameIndex, (void*)static_cast<VkCommandBuffer>(cmdBuffer),
		m_transferCmdRecording[m_currentFrameIndex] ? 1 : 0);
	buf_debug(cmdbuf);
	// Log destination buffer slot/index for debugging
	int dstIndex = -1;
	for (size_t i = 0; i < m_buffers.size(); ++i) {
		if (&m_buffers[i] == &dst) {
			dstIndex = static_cast<int>(i);
			break;
		}
	}
	mprintf(("VulkanBuffer: copyViaStaging frame=%u cmd=%p recording=%d size=%zu dstBufIdx=%d dstVkBuf=%p dstOffset=%zu stagingOffset=%d\n",
		m_currentFrameIndex,
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer)),
		m_transferCmdRecording[m_currentFrameIndex] ? 1 : 0,
		size,
		dstIndex,
		reinterpret_cast<void*>(static_cast<VkBuffer>(dst.buffer)),
		offset,
		0));

	if (!m_transferCmdRecording[m_currentFrameIndex]) {
		buf_debug("copyViaStaging: calling vkResetCommandBuffer");
		cmdBuffer.reset();
		buf_debug("copyViaStaging: calling vkBeginCommandBuffer");
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		cmdBuffer.begin(beginInfo);
		m_transferCmdRecording[m_currentFrameIndex] = true;
		buf_debug("copyViaStaging: command buffer now recording");
		mprintf(("VulkanBuffer: copyViaStaging reset/begin frame=%u cmd=%p\n",
			m_currentFrameIndex,
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer))));
	} else {
		buf_debug("copyViaStaging: SKIPPING reset/begin - already recording");
	}

	// Record copy command
	vk::BufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = offset;
	copyRegion.size = size;

	sprintf(debugbuf, "copyViaStaging: COPY src=%p[0] -> dst=%p[%zu], size=%zu",
		(void*)static_cast<VkBuffer>(stagingBuffer),
		(void*)static_cast<VkBuffer>(dst.buffer),
		offset, size);
	buf_debug(debugbuf);

	cmdBuffer.copyBuffer(stagingBuffer, dst.buffer, copyRegion);

	// Pipeline barrier to ensure transfer writes are visible to subsequent operations
	vk::BufferMemoryBarrier bufferBarrier;
	bufferBarrier.buffer = dst.buffer;
	bufferBarrier.offset = offset;
	bufferBarrier.size = size;
	bufferBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	// Set destination access and stage based on buffer type
	vk::PipelineStageFlags dstStage;
	switch (dst.type) {
	case BufferType::Vertex:
		bufferBarrier.dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead;
		dstStage = vk::PipelineStageFlagBits::eVertexInput;
		break;
	case BufferType::Index:
		bufferBarrier.dstAccessMask = vk::AccessFlagBits::eIndexRead;
		dstStage = vk::PipelineStageFlagBits::eVertexInput;
		break;
	case BufferType::Uniform:
		bufferBarrier.dstAccessMask = vk::AccessFlagBits::eUniformRead;
		dstStage = vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
		break;
	default:
		// Generic case for any shader read
		bufferBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		dstStage = vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
		break;
	}

	cmdBuffer.pipelineBarrier(
	    vk::PipelineStageFlagBits::eTransfer,
	    dstStage,
	    {},
	    nullptr,
	    bufferBarrier,
	    nullptr);

	// Queue staging buffer for deferred deletion (will be destroyed when frame fence is waited on)
	queueStagingDeletion(stagingBuffer, stagingMemory);

	// NOTE: Do NOT submit or wait here. Transfers are batched and submitted
	// by submitTransfers() before graphics work, synchronized with the frame fence.
}

void VulkanBufferManager::submitTransfers()
{
	char buf[128];
	sprintf(buf, "submitTransfers: frameIndex=%u, recording=%d",
		m_currentFrameIndex, m_transferCmdRecording[m_currentFrameIndex] ? 1 : 0);
	buf_debug(buf);

	// If no transfers were recorded this frame, nothing to do
	if (!m_transferCmdRecording[m_currentFrameIndex]) {
		buf_debug("submitTransfers: no transfers, returning early");
		return;
	}

	// End recording
	vk::CommandBuffer cmdBuffer = m_transferCmds[m_currentFrameIndex];
	char cmdbuf[256];
	sprintf(cmdbuf, "submitTransfers: ending cmdBuffer[%u]=%p",
		m_currentFrameIndex, (void*)static_cast<VkCommandBuffer>(cmdBuffer));
	buf_debug(cmdbuf);
	mprintf(("VulkanBuffer: submitTransfers end frame=%u cmd=%p\n",
		m_currentFrameIndex,
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer))));
	cmdBuffer.end();

	sprintf(cmdbuf, "submitTransfers: submitting cmdBuffer[%u]=%p to queue",
		m_currentFrameIndex, (void*)static_cast<VkCommandBuffer>(cmdBuffer));
	buf_debug(cmdbuf);
	mprintf(("VulkanBuffer: submitTransfers submit frame=%u cmd=%p\n",
		m_currentFrameIndex,
		reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer))));
	// Submit to graphics queue (ensures ordering with subsequent graphics work)
	// NOTE: We don't use a fence here because synchronization is handled by
	// the frame fence in VulkanRenderer. When beginFrame() is called for this
	// frame index again, the fence will have been waited on, guaranteeing all
	// transfer work is complete and staging buffers can be safely deleted.
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	try {
		// Ensure the transfer fence is unsignaled before submitting
		if (m_transferFences[m_currentFrameIndex]) {
			m_device.resetFences(m_transferFences[m_currentFrameIndex]);
		}
		mprintf(("VulkanBuffer: submitTransfers submitting frame=%u fence=%p cmd=%p\n",
			m_currentFrameIndex,
			reinterpret_cast<void*>(static_cast<VkFence>(m_transferFences[m_currentFrameIndex])),
			reinterpret_cast<void*>(static_cast<VkCommandBuffer>(cmdBuffer))));
		char fbuf2[128];
		sprintf(fbuf2, "submitTransfers: frame=%u fence=%p cmd=%p",
			m_currentFrameIndex,
			(void*)static_cast<VkFence>(m_transferFences[m_currentFrameIndex]),
			(void*)static_cast<VkCommandBuffer>(cmdBuffer));
		buf_debug(fbuf2);
		m_graphicsQueue.submit(submitInfo, m_transferFences[m_currentFrameIndex]);
		m_transferFenceSubmitted[m_currentFrameIndex] = true;  // Mark fence as needing wait
		buf_debug("submitTransfers: submit succeeded");
		// TEMP: block to catch issues early
		m_graphicsQueue.waitIdle();
		buf_debug("submitTransfers: queue idle after submit");
	} catch (const vk::SystemError& e) {
		sprintf(cmdbuf, "submitTransfers: VULKAN ERROR: %s", e.what());
		buf_debug(cmdbuf);
		throw;
	} catch (const std::exception& e) {
		sprintf(cmdbuf, "submitTransfers: EXCEPTION: %s", e.what());
		buf_debug(cmdbuf);
		throw;
	}

	buf_debug("submitTransfers: done");
	// Mark as no longer recording (will be reset in beginFrame)
	m_transferCmdRecording[m_currentFrameIndex] = false;
}

// ============================================================================
// gr_screen function pointer implementations
// ============================================================================

gr_buffer_handle gr_vulkan_create_buffer(BufferType type, BufferUsageHint usage)
{
	if (!g_vulkanBufferManager) {
		return gr_buffer_handle::invalid();
	}
	return g_vulkanBufferManager->createBuffer(type, usage);
}

void gr_vulkan_delete_buffer(gr_buffer_handle handle)
{
	if (g_vulkanBufferManager) {
		g_vulkanBufferManager->deleteBuffer(handle);
	}
}

void gr_vulkan_update_buffer_data(gr_buffer_handle handle, size_t size, const void* data)
{
	if (g_vulkanBufferManager) {
		g_vulkanBufferManager->updateBufferData(handle, size, data);
	}
}

void gr_vulkan_update_buffer_data_offset(gr_buffer_handle handle, size_t offset, size_t size, const void* data)
{
	if (g_vulkanBufferManager) {
		g_vulkanBufferManager->updateBufferDataOffset(handle, offset, size, data);
	}
}

void* gr_vulkan_map_buffer(gr_buffer_handle handle)
{
	if (!g_vulkanBufferManager) {
		return nullptr;
	}
	return g_vulkanBufferManager->mapBuffer(handle);
}

void gr_vulkan_flush_mapped_buffer(gr_buffer_handle handle, size_t offset, size_t size)
{
	if (g_vulkanBufferManager) {
		g_vulkanBufferManager->flushMappedBuffer(handle, offset, size);
	}
}

void gr_vulkan_bind_uniform_buffer(uniform_block_type bindPoint, size_t offset, size_t size, gr_buffer_handle handle)
{
	if (g_vulkanBufferManager) {
		g_vulkanBufferManager->bindUniformBuffer(bindPoint, offset, size, handle);
	}
}

} // namespace vulkan
} // namespace graphics
