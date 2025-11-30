
#include "VulkanBuffer.h"

#include "globalincs/pstypes.h"
#include <cstdint>

namespace graphics {
namespace vulkan {

// Global buffer manager instance
VulkanBufferManager* g_vulkanBufferManager = nullptr;

// ============================================================================
// VulkanBufferManager Implementation
// ============================================================================

VulkanBufferManager::VulkanBufferManager(vk::Device device, vk::PhysicalDevice physicalDevice)
    : m_device(device), m_physicalDevice(physicalDevice)
{
	initialize(device, physicalDevice);
}

VulkanBufferManager::~VulkanBufferManager()
{
	shutdown();
}

void VulkanBufferManager::initialize(vk::Device device, vk::PhysicalDevice physicalDevice)
{
	if (m_initialized) {
		return;
	}

	m_device = device;
	m_physicalDevice = physicalDevice;
	m_memoryProperties = physicalDevice.getMemoryProperties();

	// Query device limits
	auto properties = physicalDevice.getProperties();
	m_minUboAlignment = static_cast<size_t>(properties.limits.minUniformBufferOffsetAlignment);

	mprintf(("Vulkan Buffer Manager initialized\n"));
	mprintf(("  Min UBO alignment: %zu bytes\n", m_minUboAlignment));

	m_initialized = true;
}

void VulkanBufferManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Delete all buffers
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
		m_device.destroyBuffer(bufferData.buffer);
		m_device.freeMemory(bufferData.memory);
	}

	// Create buffer
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	bufferData.buffer = m_device.createBuffer(bufferInfo);

	// Get memory requirements
	auto memRequirements = m_device.getBufferMemoryRequirements(bufferData.buffer);

	// Allocate memory
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	bufferData.memory = m_device.allocateMemory(allocInfo);
	m_device.bindBufferMemory(bufferData.buffer, bufferData.memory, 0);

	bufferData.size = size;
	bufferData.hostVisible = (properties & vk::MemoryPropertyFlagBits::eHostVisible) ==
	                         vk::MemoryPropertyFlagBits::eHostVisible;
}

gr_buffer_handle VulkanBufferManager::createBuffer(BufferType type, BufferUsageHint usage)
{
	Assertion(m_initialized, "VulkanBufferManager not initialized!");

	VulkanBufferData bufferData;
	bufferData.type = type;
	bufferData.usage = usage;
	bufferData.lastUsedFrame = m_currentFrame;

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
		// TODO: Check if buffer is in-flight and defer deletion
		if (buffer.mappedPtr) {
			m_device.unmapMemory(buffer.memory);
			buffer.mappedPtr = nullptr;
		}
		m_device.destroyBuffer(buffer.buffer);
		m_device.freeMemory(buffer.memory);
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
		// Device-local for best GPU performance, will need staging buffer
		memProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
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

	buffer.lastUsedFrame = m_currentFrame;
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

	buffer.lastUsedFrame = m_currentFrame;
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

	buffer.lastUsedFrame = m_currentFrame;
}

void VulkanBufferManager::bindUniformBuffer(uniform_block_type bindPoint, size_t offset, size_t size,
                                            gr_buffer_handle handle)
{
	// Validate alignment
	Assertion(offset % m_minUboAlignment == 0,
	          "UBO offset %zu must be aligned to %zu!", offset, m_minUboAlignment);

	if (!handle.isValid()) {
		// Unbind - nothing to do in Vulkan (handled by descriptor set)
		return;
	}

	Assertion(static_cast<size_t>(handle.value()) < m_buffers.size(), "Buffer handle out of range!");

	VulkanBufferData& buffer = m_buffers[handle.value()];
	Assertion(buffer.type == BufferType::Uniform, "Only uniform buffers can be bound to UBO points!");

	// In Vulkan, binding is done through descriptor sets, not direct binding
	// This will be implemented when descriptor management is added
	// For now, just track the binding request

	buffer.lastUsedFrame = m_currentFrame;
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

void VulkanBufferManager::endFrame()
{
	m_currentFrame++;
}

void VulkanBufferManager::copyViaStaging(VulkanBufferData& dst, size_t offset, size_t size, const void* data)
{
	// TODO: Implement proper staging buffer pool
	// For now, create a temporary staging buffer per copy (inefficient but functional)

	// Create staging buffer
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

	// Copy data to staging buffer
	void* mapped = m_device.mapMemory(stagingMemory, 0, size);
	auto* dstPtr = reinterpret_cast<std::uint8_t*>(mapped);
	auto* src = reinterpret_cast<const std::uint8_t*>(data);
	memcpy(dstPtr, src, size);
	m_device.unmapMemory(stagingMemory);

	// TODO: Record and submit transfer command
	// For now, this is a placeholder - we need command buffer infrastructure
	mprintf(("Vulkan: WARNING - copyViaStaging not fully implemented, transfer commands needed\n"));

	// Cleanup staging buffer
	// In a real implementation, this would be deferred until the transfer completes
	m_device.destroyBuffer(stagingBuffer);
	m_device.freeMemory(stagingMemory);
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
