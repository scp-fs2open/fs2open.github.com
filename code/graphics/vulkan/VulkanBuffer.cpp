#include "VulkanBuffer.h"
#include "VulkanDeletionQueue.h"
#include "VulkanDraw.h"

#include "globalincs/pstypes.h"

namespace graphics {
namespace vulkan {

namespace {
VulkanBufferManager* g_bufferManager = nullptr;
}

VulkanBufferManager* getBufferManager()
{
	Assertion(g_bufferManager != nullptr, "Vulkan BufferManager not initialized!");
	return g_bufferManager;
}

void setBufferManager(VulkanBufferManager* manager)
{
	g_bufferManager = manager;
}

VulkanBufferManager::VulkanBufferManager() = default;

VulkanBufferManager::~VulkanBufferManager()
{
	if (m_initialized) {
		shutdown();
	}
}

bool VulkanBufferManager::createOneShotBuffer(vk::Flags<vk::BufferUsageFlagBits> usage, const void* data, size_t size, vk::Buffer& buf, VulkanAllocation& alloc) const
{
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		buf = m_device.createBuffer(bufferInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Failed to create buffer: %s\n", e.what()));
		return false;
	}

	if (!m_memoryManager->allocateBufferMemory(buf, MemoryUsage::CpuToGpu, alloc)) {
		m_device.destroyBuffer(buf);
		buf = nullptr;
		mprintf(("Failed to allocate buffer memory!\n"));
		return false;
	}

	void* mapped = m_memoryManager->mapMemory(alloc);
	if (mapped) {
		memcpy(mapped, data, size);
		m_memoryManager->flushMemory(alloc, 0, size);
		m_memoryManager->unmapMemory(alloc);
	} else {
		m_memoryManager->freeAllocation(alloc);
		m_device.destroyBuffer(buf);
		buf = nullptr;

		mprintf(("Failed to map buffer memory!\n"));
		return false;
	}
	return true;
}

// ========== Frame bump allocator ==========

bool VulkanBufferManager::createFrameAllocBuffer(FrameBumpAllocator& alloc, size_t size)
{
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer
	                 | vk::BufferUsageFlagBits::eIndexBuffer
	                 | vk::BufferUsageFlagBits::eUniformBuffer
	                 | vk::BufferUsageFlagBits::eStorageBuffer
	                 | vk::BufferUsageFlagBits::eTransferDst;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		alloc.buffer = m_device.createBuffer(bufferInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Failed to create frame allocator buffer: %s\n", e.what()));
		return false;
	}

	if (!m_memoryManager->allocateBufferMemory(alloc.buffer, MemoryUsage::CpuToGpu, alloc.allocation)) {
		m_device.destroyBuffer(alloc.buffer);
		alloc.buffer = nullptr;
		mprintf(("Failed to allocate frame allocator buffer memory!\n"));
		return false;
	}

	alloc.mappedPtr = m_memoryManager->mapMemory(alloc.allocation);
	if (!alloc.mappedPtr) {
		m_memoryManager->freeAllocation(alloc.allocation);
		m_device.destroyBuffer(alloc.buffer);
		alloc.buffer = nullptr;
		alloc.allocation = {};
		mprintf(("Failed to map frame allocator buffer!\n"));
		return false;
	}

	alloc.capacity = size;
	alloc.cursor = 0;
	return true;
}

void VulkanBufferManager::initFrameAllocators()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		Verify(createFrameAllocBuffer(m_frameAllocs[i], FRAME_ALLOC_INITIAL_SIZE));
	}
	mprintf(("Frame bump allocators initialized: %u x %zuKB\n",
		MAX_FRAMES_IN_FLIGHT, FRAME_ALLOC_INITIAL_SIZE / 1024));
}

void VulkanBufferManager::shutdownFrameAllocators()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		auto& alloc = m_frameAllocs[i];
		if (alloc.mappedPtr) {
			m_memoryManager->unmapMemory(alloc.allocation);
			alloc.mappedPtr = nullptr;
		}
		if (alloc.buffer) {
			m_device.destroyBuffer(alloc.buffer);
			alloc.buffer = nullptr;
		}
		if (alloc.allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(alloc.allocation);
			alloc.allocation = {};
		}
		alloc.capacity = 0;
		alloc.cursor = 0;
	}
}

size_t VulkanBufferManager::bumpAllocate(size_t size)
{
	auto& alloc = m_frameAllocs[m_currentFrame];

	// Align cursor up to UBO alignment (satisfies UBO/SSBO/vertex alignment)
	size_t alignedOffset = (alloc.cursor + m_uboAlignment - 1) & ~(static_cast<size_t>(m_uboAlignment) - 1);

	if (alignedOffset + size > alloc.capacity) {
		growFrameAllocator();
		// After growth, cursor is 0 so alignedOffset is 0
		alignedOffset = 0;
		Assertion(size <= alloc.capacity, "Frame allocator growth failed to provide enough capacity");
	}

	alloc.cursor = alignedOffset + size;
	return alignedOffset;
}

void VulkanBufferManager::growFrameAllocator()
{
	auto& alloc = m_frameAllocs[m_currentFrame];

	// Double capacity until sufficient
	size_t newCapacity = alloc.capacity > 0 ? alloc.capacity * 2 : FRAME_ALLOC_INITIAL_SIZE;
	// Ensure at least the current cursor position can fit (handles pathological single-alloc case)
	while (newCapacity < alloc.cursor) {
		newCapacity *= 2;
	}

	mprintf(("Growing frame allocator %u: %zuKB -> %zuKB\n",
		m_currentFrame, alloc.capacity / 1024, newCapacity / 1024));

	// Queue old buffer for deferred destruction - the deletion queue's FRAMES_TO_WAIT=2
	// ensures the old buffer survives through current frame's GPU execution.
	// Existing handles with frameAllocBuffer pointing to the old buffer remain valid.
	auto* deletionQueue = getDeletionQueue();
	if (alloc.mappedPtr) {
		m_memoryManager->unmapMemory(alloc.allocation);
	}
	deletionQueue->queueBuffer(alloc.buffer, alloc.allocation);

	// Create new buffer
	alloc = {};
	Verify(createFrameAllocBuffer(alloc, newCapacity));
}

// ========== Init / Shutdown ==========

bool VulkanBufferManager::init(vk::Device device,
                               VulkanMemoryManager* memoryManager,
                               uint32_t graphicsQueueFamily,
                               uint32_t transferQueueFamily,
                               uint32_t minUboAlignment)
{
	if (m_initialized) {
		mprintf(("VulkanBufferManager::init called when already initialized!\n"));
		return false;
	}

	if (!device || !memoryManager) {
		mprintf(("VulkanBufferManager::init called with null device or memory manager!\n"));
		return false;
	}

	m_device = device;
	m_memoryManager = memoryManager;
	m_graphicsQueueFamily = graphicsQueueFamily;
	m_transferQueueFamily = transferQueueFamily;
	m_currentFrame = 0;
	m_uboAlignment = minUboAlignment > 0 ? minUboAlignment : 256;

	// Create fallback color buffer with white (1,1,1,1) for shaders expecting vertColor
	float whiteColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (!createOneShotBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, whiteColor, sizeof(whiteColor), m_fallbackColorBuffer, m_fallbackColorAllocation)) {
		mprintf(("VulkanBufferManager::init could not create fallback color buffer\n"));
		return false;
	}

	float zeroTexCoord[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (!createOneShotBuffer(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, zeroTexCoord, sizeof(zeroTexCoord), m_fallbackTexCoordBuffer, m_fallbackTexCoordAllocation)) {
		mprintf(("VulkanBufferManager::init could not create fallback texcoord buffer\n"));
		return false;
	}

	// Create fallback uniform buffer (zeros) for uninitialized descriptor set bindings
	// Without this, descriptor set UBO bindings left unwritten after pool reset
	// contain undefined data, causing intermittent rendering failures
	float dummy_ubo[FALLBACK_UNIFORM_BUFFER_SIZE] = {};
	if (!createOneShotBuffer(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer, dummy_ubo, sizeof(dummy_ubo), m_fallbackUniformBuffer, m_fallbackUniformAllocation)) {
		mprintf(("VulkanBufferManager::init could not create fallback uniform buffer\n"));
		return false;
	}

	initFrameAllocators();

	m_initialized = true;
	mprintf(("Vulkan Buffer Manager initialized (frame bump allocator, UBO alignment=%u, %u frames)\n",
		m_uboAlignment, MAX_FRAMES_IN_FLIGHT));
	return true;
}

void VulkanBufferManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Destroy fallback color buffer
	if (m_fallbackColorBuffer) {
		m_device.destroyBuffer(m_fallbackColorBuffer);
		m_fallbackColorBuffer = nullptr;
	}
	if (m_fallbackColorAllocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_fallbackColorAllocation);
		m_fallbackColorAllocation = {};
	}

	// Destroy fallback texcoord buffer
	if (m_fallbackTexCoordBuffer) {
		m_device.destroyBuffer(m_fallbackTexCoordBuffer);
		m_fallbackTexCoordBuffer = nullptr;
	}
	if (m_fallbackTexCoordAllocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_fallbackTexCoordAllocation);
		m_fallbackTexCoordAllocation = {};
	}

	// Destroy fallback uniform buffer
	if (m_fallbackUniformBuffer) {
		m_device.destroyBuffer(m_fallbackUniformBuffer);
		m_fallbackUniformBuffer = nullptr;
	}
	if (m_fallbackUniformAllocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_fallbackUniformAllocation);
		m_fallbackUniformAllocation = {};
	}

	// Free all remaining static buffers
	for (auto& bufferObj : m_buffers) {
		if (bufferObj.valid) {
			if (!bufferObj.isStreaming() && bufferObj.buffer) {
				m_device.destroyBuffer(bufferObj.buffer);
			}
			if (!bufferObj.isStreaming() && bufferObj.allocation.memory != VK_NULL_HANDLE) {
				m_memoryManager->freeAllocation(bufferObj.allocation);
			}
			bufferObj.valid = false;
		}
	}

	shutdownFrameAllocators();

	m_buffers.clear();
	m_freeIndices.clear();
	m_activeBufferCount = 0;
	m_totalBufferMemory = 0;
	m_initialized = false;

	mprintf(("Vulkan Buffer Manager shutdown\n"));
}

void VulkanBufferManager::setCurrentFrame(uint32_t frameIndex)
{
	m_currentFrame = frameIndex % MAX_FRAMES_IN_FLIGHT;
	// Reset bump cursor — safe because the GPU fence for this frame-in-flight
	// was already waited on before setCurrentFrame is called.
	m_frameAllocs[m_currentFrame].cursor = 0;
}

// ========== Buffer usage / memory helpers ==========

vk::BufferUsageFlags VulkanBufferManager::getVkUsageFlags(BufferType type) const
{
	vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eTransferDst;

	switch (type) {
	case BufferType::Vertex:
		flags |= vk::BufferUsageFlagBits::eVertexBuffer;
		break;
	case BufferType::Index:
		flags |= vk::BufferUsageFlagBits::eIndexBuffer;
		break;
	case BufferType::Uniform:
		flags |= vk::BufferUsageFlagBits::eUniformBuffer;
		break;
	}

	return flags;
}

MemoryUsage VulkanBufferManager::getMemoryUsage(BufferUsageHint hint) const
{
	switch (hint) {
	case BufferUsageHint::Static:
		// Static data goes to device-local memory for best GPU performance
		// For simplicity, we use CpuToGpu which allows host writes
		// A more optimized path would use staging buffers for truly static data
		return MemoryUsage::CpuToGpu;

	case BufferUsageHint::Dynamic:
	case BufferUsageHint::Streaming:
		// Frequently updated data needs to be host visible
		return MemoryUsage::CpuToGpu;

	case BufferUsageHint::PersistentMapping:
		// Persistent mapping requires host visible memory
		return MemoryUsage::CpuOnly;

	default:
		return MemoryUsage::CpuToGpu;
	}
}

// ========== Buffer create / delete ==========

gr_buffer_handle VulkanBufferManager::createBuffer(BufferType type, BufferUsageHint usage)
{
	Verify(m_initialized);

	VulkanBufferObject bufferObj;
	bufferObj.type = type;
	bufferObj.usage = usage;
	bufferObj.valid = true;
	// Note: actual buffer creation is deferred until data is uploaded

	int index;
	if (!m_freeIndices.empty()) {
		// Reuse a freed slot
		index = m_freeIndices.back();
		m_freeIndices.pop_back();
		m_buffers[index] = bufferObj;
	} else {
		// Add new slot
		index = static_cast<int>(m_buffers.size());
		m_buffers.push_back(bufferObj);
	}

	++m_activeBufferCount;
	return gr_buffer_handle(index);
}

void VulkanBufferManager::deleteBuffer(gr_buffer_handle handle)
{
	Verify(m_initialized && isValidHandle(handle));

	VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	Verify(bufferObj.valid);

	if (!bufferObj.isStreaming()) {
		// Queue static buffer for deferred destruction
		auto* deletionQueue = getDeletionQueue();
		if (bufferObj.buffer) {
			deletionQueue->queueBuffer(bufferObj.buffer, bufferObj.allocation);
			m_totalBufferMemory -= bufferObj.dataSize;
		}
		bufferObj.buffer = nullptr;
		bufferObj.allocation = {};
		bufferObj.dataSize = 0;
	} else {
		// Streaming buffers have no per-buffer resources — just mark invalid
	}

	--m_activeBufferCount;
	bufferObj.valid = false;

	// Add to free list for reuse
	m_freeIndices.push_back(handle.value());
}

// ========== createOrResizeBuffer (static only) ==========

bool VulkanBufferManager::createOrResizeBuffer(VulkanBufferObject& bufferObj, size_t size)
{
	Assertion(!bufferObj.isStreaming(), "createOrResizeBuffer called on streaming buffer!");

	// If buffer exists and is large enough, no-op
	if (bufferObj.buffer && bufferObj.dataSize >= size) {
		return true;
	}

	// Save old buffer info for data copy
	vk::Buffer oldBuffer = bufferObj.buffer;
	VulkanAllocation oldAllocation = bufferObj.allocation;
	size_t oldDataSize = bufferObj.dataSize;

	// Create new buffer
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = getVkUsageFlags(bufferObj.type);

	// Handle queue family sharing
	uint32_t queueFamilies[] = {m_graphicsQueueFamily, m_transferQueueFamily};
	if (m_graphicsQueueFamily != m_transferQueueFamily) {
		bufferInfo.sharingMode = vk::SharingMode::eConcurrent;
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queueFamilies;
	} else {
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;
	}

	try {
		bufferObj.buffer = m_device.createBuffer(bufferInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Failed to create Vulkan buffer: %s\n", e.what()));
		bufferObj.buffer = oldBuffer;
		return false;
	}

	// Allocate memory
	MemoryUsage memUsage = getMemoryUsage(bufferObj.usage);
	if (!m_memoryManager->allocateBufferMemory(bufferObj.buffer, memUsage, bufferObj.allocation)) {
		m_device.destroyBuffer(bufferObj.buffer);
		bufferObj.buffer = oldBuffer;
		bufferObj.allocation = oldAllocation;
		return false;
	}

	// Copy existing data from old buffer
	if (oldBuffer && oldDataSize > 0) {
		void* oldMapped = m_memoryManager->mapMemory(oldAllocation);
		void* newMapped = m_memoryManager->mapMemory(bufferObj.allocation);
		Verify(oldMapped);
		Verify(newMapped);

		size_t copySize = std::min(oldDataSize, size);
		memcpy(newMapped, oldMapped, copySize);
		m_memoryManager->flushMemory(bufferObj.allocation, 0, copySize);

		m_memoryManager->unmapMemory(oldAllocation);
		m_memoryManager->unmapMemory(bufferObj.allocation);
	}

	// Queue old buffer for deferred destruction
	if (oldBuffer) {
		auto* deletionQueue = getDeletionQueue();
		deletionQueue->queueBuffer(oldBuffer, oldAllocation);
		m_totalBufferMemory -= oldDataSize;
	}

	bufferObj.dataSize = size;
	m_totalBufferMemory += size;

	return true;
}

// ========== Buffer data updates ==========

void VulkanBufferManager::updateBufferData(gr_buffer_handle handle, size_t size, const void* data)
{
	Verify(m_initialized && isValidHandle(handle));

	if (size == 0) {
		mprintf(("WARNING: updateBufferData called with size 0\n"));
		return;
	}

	VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	Verify(bufferObj.valid);

	if (bufferObj.isStreaming()) {
		auto& alloc = m_frameAllocs[m_currentFrame];

		if (data) {
			// Pattern A: full replacement — allocate and copy
			size_t offset = bumpAllocate(size);
			memcpy(static_cast<uint8_t*>(alloc.mappedPtr) + offset, data, size);
			m_memoryManager->flushMemory(alloc.allocation, offset, size);

			bufferObj.frameAllocBuffer = alloc.buffer;
			bufferObj.frameAllocOffset = offset;
			bufferObj.dataSize = size;
			bufferObj.frameAllocFrame = m_currentFrame;
		} else {
			// Pattern B: pre-alloc for offset writes (null data)
			if (bufferObj.frameAllocFrame != m_currentFrame || size > bufferObj.dataSize) {
				// First allocation this frame, or need more space
				size_t offset = bumpAllocate(size);
				bufferObj.frameAllocBuffer = alloc.buffer;
				bufferObj.frameAllocOffset = offset;
				bufferObj.dataSize = size;
				bufferObj.frameAllocFrame = m_currentFrame;
			}
			// Otherwise: same frame and size fits — keep current allocation
		}
	} else {
		// Static / PersistentMapping path
		Verify(createOrResizeBuffer(bufferObj, size));

		// A null data pointer just allocates/resizes the buffer without writing
		if (data) {
			void* mapped = m_memoryManager->mapMemory(bufferObj.allocation);
			Verify(mapped);
			memcpy(mapped, data, size);
			m_memoryManager->flushMemory(bufferObj.allocation, 0, size);
			m_memoryManager->unmapMemory(bufferObj.allocation);
		}
	}
}

void VulkanBufferManager::updateBufferDataOffset(gr_buffer_handle handle, size_t offset, size_t size, const void* data)
{
	Verify(m_initialized && isValidHandle(handle));

	VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	Verify(bufferObj.valid);

	if (bufferObj.isStreaming()) {
		// Auto-allocate if not yet allocated this frame.  This happens when
		// the caller skips updateBufferData (e.g. gr_add_to_immediate_buffer
		// when the data fits the existing buffer size).
		if (bufferObj.frameAllocFrame != m_currentFrame) {
			size_t allocSize = std::max(bufferObj.dataSize, offset + size);
			Verify(allocSize > 0);
			auto& fa = m_frameAllocs[m_currentFrame];
			size_t allocOffset = bumpAllocate(allocSize);
			bufferObj.frameAllocBuffer = fa.buffer;
			bufferObj.frameAllocOffset = allocOffset;
			bufferObj.dataSize = allocSize;
			bufferObj.frameAllocFrame = m_currentFrame;
		}

		Verify(offset + size <= bufferObj.dataSize);

		auto& alloc = m_frameAllocs[m_currentFrame];
		size_t totalOffset = bufferObj.frameAllocOffset + offset;
		memcpy(static_cast<uint8_t*>(alloc.mappedPtr) + totalOffset, data, size);
		m_memoryManager->flushMemory(alloc.allocation, totalOffset, size);
	} else {
		// Static path
		Verify(bufferObj.buffer);
		Verify(offset + size <= bufferObj.dataSize);

		// Map, update region, and unmap
		void* mapped = m_memoryManager->mapMemory(bufferObj.allocation);
		Verify(mapped);
		memcpy(static_cast<uint8_t*>(mapped) + offset, data, size);
		m_memoryManager->flushMemory(bufferObj.allocation, offset, size);
		m_memoryManager->unmapMemory(bufferObj.allocation);
	}
}

// ========== Map / Flush ==========

void* VulkanBufferManager::mapBuffer(gr_buffer_handle handle)
{
	if (!m_initialized || !isValidHandle(handle)) {
		return nullptr;
	}

	VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	if (!bufferObj.valid) {
		return nullptr;
	}

	if (bufferObj.isStreaming()) {
		Verify(bufferObj.frameAllocFrame == m_currentFrame);
		auto& alloc = m_frameAllocs[m_currentFrame];
		return static_cast<uint8_t*>(alloc.mappedPtr) + bufferObj.frameAllocOffset;
	}

	// Static / PersistentMapping
	if (!bufferObj.buffer) {
		return nullptr;
	}

	// Only persistent mapping buffers should stay mapped
	if (bufferObj.usage != BufferUsageHint::PersistentMapping) {
		mprintf(("WARNING: mapBuffer called on non-persistent buffer\n"));
	}

	// Map the entire buffer
	void* mapped = m_memoryManager->mapMemory(bufferObj.allocation);
	return mapped;
}

void VulkanBufferManager::flushMappedBuffer(gr_buffer_handle handle, size_t offset, size_t size)
{
	Verify(m_initialized && isValidHandle(handle));

	VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	Verify(bufferObj.valid);

	if (bufferObj.isStreaming()) {
		// Adjust offset for current frame's allocation
		Verify(bufferObj.frameAllocFrame == m_currentFrame);
		auto& alloc = m_frameAllocs[m_currentFrame];
		m_memoryManager->flushMemory(alloc.allocation, bufferObj.frameAllocOffset + offset, size);
	} else {
		m_memoryManager->flushMemory(bufferObj.allocation, offset, size);
	}
}

// ========== Uniform buffer binding ==========

void VulkanBufferManager::bindUniformBuffer(uniform_block_type blockType, size_t offset, size_t size, gr_buffer_handle buffer)
{
	// Resolve the full offset NOW (frame base + caller offset) so the binding
	// captures the correct allocation. The vk::Buffer is still looked up at
	// draw time (via handle) to survive buffer recreation.
	size_t resolvedOffset = getFrameBaseOffset(buffer) + offset;

	auto* drawManager = getDrawManager();
	drawManager->setPendingUniformBinding(blockType, buffer,
	                                       static_cast<vk::DeviceSize>(resolvedOffset),
	                                       static_cast<vk::DeviceSize>(size));
}

// ========== Buffer queries ==========

vk::Buffer VulkanBufferManager::getVkBuffer(gr_buffer_handle handle) const
{
	if (!isValidHandle(handle)) {
		return nullptr;
	}

	const VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	if (!bufferObj.valid) {
		return nullptr;
	}

	if (bufferObj.isStreaming()) {
		// Streaming buffers return the frame allocator buffer they were uploaded to
		Verify(bufferObj.frameAllocFrame == m_currentFrame);
		return bufferObj.frameAllocBuffer;
	} else {
		return bufferObj.buffer;
	}
}

size_t VulkanBufferManager::getBufferSize(gr_buffer_handle handle) const
{
	if (!isValidHandle(handle)) {
		return 0;
	}

	const VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	if (!bufferObj.valid) {
		return 0;
	}

	return bufferObj.dataSize;
}

size_t VulkanBufferManager::getFrameBaseOffset(gr_buffer_handle handle) const
{
	if (!isValidHandle(handle)) {
		return 0;
	}

	const VulkanBufferObject& bufferObj = m_buffers[handle.value()];
	if (!bufferObj.valid) {
		return 0;
	}

	if (bufferObj.isStreaming()) {
		// Return the bump allocator offset for the most recent upload this frame.
		// Stale handle detection: if frameAllocFrame != m_currentFrame, this buffer
		// was not uploaded this frame and the offset would be meaningless (the bump
		// allocator has been reset). This indicates a buffer marked Streaming/Dynamic
		// is being bound for rendering without being uploaded first.
		Verify(bufferObj.frameAllocFrame == m_currentFrame);
		return bufferObj.frameAllocOffset;
	} else {
		return 0;
	}
}

bool VulkanBufferManager::isValidHandle(gr_buffer_handle handle) const
{
	if (!handle.isValid()) {
		return false;
	}
	if (static_cast<size_t>(handle.value()) >= m_buffers.size()) {
		return false;
	}
	return m_buffers[handle.value()].valid;
}

VulkanBufferObject* VulkanBufferManager::getBufferObject(gr_buffer_handle handle)
{
	if (!isValidHandle(handle)) {
		return nullptr;
	}
	return &m_buffers[handle.value()];
}

const VulkanBufferObject* VulkanBufferManager::getBufferObject(gr_buffer_handle handle) const
{
	if (!isValidHandle(handle)) {
		return nullptr;
	}
	return &m_buffers[handle.value()];
}

// ========== gr_screen function pointer implementations ==========

gr_buffer_handle vulkan_create_buffer(BufferType type, BufferUsageHint usage)
{
	auto* bufferManager = getBufferManager();
	return bufferManager->createBuffer(type, usage);
}

void vulkan_delete_buffer(gr_buffer_handle handle)
{
	auto* bufferManager = getBufferManager();
	bufferManager->deleteBuffer(handle);
}

void vulkan_update_buffer_data(gr_buffer_handle handle, size_t size, const void* data)
{
	auto* bufferManager = getBufferManager();
	bufferManager->updateBufferData(handle, size, data);
}

void vulkan_update_buffer_data_offset(gr_buffer_handle handle, size_t offset, size_t size, const void* data)
{
	auto* bufferManager = getBufferManager();
	bufferManager->updateBufferDataOffset(handle, offset, size, data);
}

void* vulkan_map_buffer(gr_buffer_handle handle)
{
	auto* bufferManager = getBufferManager();
	void* result = bufferManager->mapBuffer(handle);
	Verify(result);
	return result;
}

void vulkan_flush_mapped_buffer(gr_buffer_handle handle, size_t offset, size_t size)
{
	auto* bufferManager = getBufferManager();
	bufferManager->flushMappedBuffer(handle, offset, size);
}

void vulkan_bind_uniform_buffer(uniform_block_type blockType, size_t offset, size_t size, gr_buffer_handle buffer)
{
	auto* bufferManager = getBufferManager();
	bufferManager->bindUniformBuffer(blockType, offset, size, buffer);
}

} // namespace vulkan
} // namespace graphics
