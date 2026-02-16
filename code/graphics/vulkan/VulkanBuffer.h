#pragma once

#include "graphics/2d.h"
#include "VulkanConstants.h"
#include "VulkanMemory.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

/**
 * @brief Per-frame bump allocator for streaming/dynamic buffers
 *
 * Two of these exist (one per frame-in-flight). At frame start the cursor
 * resets to 0. Each streaming upload bumps the cursor forward.
 * The buffer is persistently mapped for the lifetime of the allocator.
 */
struct FrameBumpAllocator {
	vk::Buffer buffer;
	VulkanAllocation allocation = {};
	void* mappedPtr = nullptr;
	size_t capacity = 0;
	size_t cursor = 0;
};

/**
 * @brief Internal representation of a Vulkan buffer
 *
 * Static buffers own their own VkBuffer. Streaming/Dynamic buffers are
 * sub-allocated from a shared FrameBumpAllocator each frame.
 */
struct VulkanBufferObject {
	BufferType type = BufferType::Vertex;
	BufferUsageHint usage = BufferUsageHint::Static;
	bool valid = false;
	size_t dataSize = 0; // Usable data size. Static: total VkBuffer allocation. Streaming: current frame allocation.

	// Static buffer fields (unused for streaming)
	vk::Buffer buffer = nullptr;
	VulkanAllocation allocation = {};

	// Frame bump allocator sub-allocation (streaming/dynamic only)
	vk::Buffer frameAllocBuffer;       // VkBuffer at upload time (may be old allocator buffer after growth)
	size_t frameAllocOffset = 0;       // Byte offset within the frame allocator buffer
	uint32_t frameAllocFrame = UINT32_MAX; // Frame index when last allocated

	bool isStreaming() const {
		return usage == BufferUsageHint::Streaming || usage == BufferUsageHint::Dynamic;
	}
};

/**
 * @brief Manages GPU buffer creation, updates, and destruction
 *
 * Streaming/Dynamic buffers are sub-allocated from a global per-frame bump
 * allocator (two large VkBuffers, one per frame-in-flight). Static buffers
 * keep their own VkBuffer. PersistentMapping buffers are handled separately.
 */
class VulkanBufferManager {
public:
	VulkanBufferManager();
	~VulkanBufferManager();

	// Non-copyable
	VulkanBufferManager(const VulkanBufferManager&) = delete;
	VulkanBufferManager& operator=(const VulkanBufferManager&) = delete;

	/**
	 * @brief Initialize the buffer manager
	 * @param device The Vulkan logical device
	 * @param memoryManager The memory manager for allocations
	 * @param graphicsQueueFamily Graphics queue family index
	 * @param transferQueueFamily Transfer queue family index
	 * @param minUboAlignment Minimum uniform buffer offset alignment from device limits
	 * @return true on success
	 */
	bool init(vk::Device device,
	          VulkanMemoryManager* memoryManager,
	          uint32_t graphicsQueueFamily,
	          uint32_t transferQueueFamily,
	          uint32_t minUboAlignment);

	/**
	 * @brief Shutdown and free all buffers
	 */
	void shutdown();

	/**
	 * @brief Set the current frame index and reset the bump allocator cursor
	 * Must be called at the start of each frame before any buffer updates
	 * @param frameIndex The current frame index (0 to MAX_FRAMES_IN_FLIGHT-1)
	 */
	void setCurrentFrame(uint32_t frameIndex);

	/**
	 * @brief Get the current frame index
	 */
	uint32_t getCurrentFrame() const { return m_currentFrame; }

	/**
	 * @brief Get the Vulkan logical device
	 */
	vk::Device getDevice() const { return m_device; }

	/**
	 * @brief Create a new buffer
	 * @param type The buffer type (Vertex, Index, Uniform)
	 * @param usage Usage hint for optimization
	 * @return Handle to the created buffer, or invalid handle on failure
	 */
	gr_buffer_handle createBuffer(BufferType type, BufferUsageHint usage);

	/**
	 * @brief Delete a buffer
	 * @param handle The buffer to delete
	 */
	void deleteBuffer(gr_buffer_handle handle);

	/**
	 * @brief Update buffer data (full replacement)
	 * @param handle The buffer to update
	 * @param size Size of data in bytes
	 * @param data Pointer to data
	 */
	void updateBufferData(gr_buffer_handle handle, size_t size, const void* data);

	/**
	 * @brief Update buffer data at an offset
	 * @param handle The buffer to update
	 * @param offset Offset in bytes
	 * @param size Size of data in bytes
	 * @param data Pointer to data
	 */
	void updateBufferDataOffset(gr_buffer_handle handle, size_t offset, size_t size, const void* data);

	/**
	 * @brief Map buffer for CPU access
	 * @param handle The buffer to map
	 * @return Pointer to mapped memory, or nullptr on failure
	 */
	void* mapBuffer(gr_buffer_handle handle);

	/**
	 * @brief Flush a range of a mapped buffer
	 * @param handle The buffer to flush
	 * @param offset Offset in bytes
	 * @param size Size of range in bytes
	 */
	void flushMappedBuffer(gr_buffer_handle handle, size_t offset, size_t size);

	/**
	 * @brief Bind uniform buffer to a binding slot
	 * @param blockType The uniform block type
	 * @param offset Offset within the buffer
	 * @param size Size of the bound range
	 * @param buffer The buffer to bind
	 */
	void bindUniformBuffer(uniform_block_type blockType, size_t offset, size_t size, gr_buffer_handle buffer);

	/**
	 * @brief Get the Vulkan buffer handle for the current frame
	 * @param handle The buffer handle
	 * @return The VkBuffer, or VK_NULL_HANDLE if invalid
	 */
	vk::Buffer getVkBuffer(gr_buffer_handle handle) const;

	/**
	 * @brief Get buffer size
	 * For streaming buffers, returns the current frame allocation size.
	 * For static buffers, returns the total buffer size.
	 * @param handle The buffer handle
	 * @return Size in bytes, or 0 if invalid
	 */
	size_t getBufferSize(gr_buffer_handle handle) const;

	/**
	 * @brief Get the base offset for the current frame's allocation
	 * For streaming buffers, returns the bump allocator offset.
	 * For static buffers, returns 0.
	 * @param handle The buffer handle
	 * @return Byte offset for current frame's allocation
	 */
	size_t getFrameBaseOffset(gr_buffer_handle handle) const;

	/**
	 * @brief Check if a handle is valid
	 */
	bool isValidHandle(gr_buffer_handle handle) const;

	/**
	 * @brief Get statistics
	 */
	size_t getBufferCount() const { return m_activeBufferCount; }
	size_t getTotalBufferMemory() const { return m_totalBufferMemory; }

	/**
	 * @brief Get the constant white color buffer for fallback vertex colors
	 * This buffer contains vec4(1,1,1,1) for shaders expecting vertColor
	 */
	vk::Buffer getFallbackColorBuffer() const { return m_fallbackColorBuffer; }

	/**
	 * @brief Get the constant zero texcoord buffer for fallback vertex texcoords
	 * This buffer contains vec4(0,0,0,0) for shaders expecting vertTexCoord
	 */
	vk::Buffer getFallbackTexCoordBuffer() const { return m_fallbackTexCoordBuffer; }

	/**
	 * @brief Get the fallback uniform buffer for uninitialized descriptor bindings
	 * This buffer contains zeros and is used to pre-fill all UBO descriptor bindings
	 * to avoid undefined behavior from uninitialized descriptors after pool reset
	 */
	vk::Buffer getFallbackUniformBuffer() const { return m_fallbackUniformBuffer; }

	/**
	 * @brief Get the size of the fallback uniform buffer
	 */
	size_t getFallbackUniformBufferSize() const { return FALLBACK_UNIFORM_BUFFER_SIZE; }

private:
	/**
	 * @brief Create a one-shot buffer (used in initialization only).
	 */
	bool createOneShotBuffer(vk::Flags<vk::BufferUsageFlagBits> usage, const void* data, size_t size, vk::Buffer& buf, VulkanAllocation& alloc) const;

	/**
	 * @brief Convert BufferType to Vulkan usage flags
	 */
	vk::BufferUsageFlags getVkUsageFlags(BufferType type) const;

	/**
	 * @brief Convert BufferUsageHint to memory usage
	 */
	MemoryUsage getMemoryUsage(BufferUsageHint hint) const;

	/**
	 * @brief Create or resize a static buffer
	 * Streaming buffers must NOT call this — they use the frame bump allocator.
	 */
	bool createOrResizeBuffer(VulkanBufferObject& bufferObj, size_t size);

	/**
	 * @brief Get buffer object from handle
	 */
	VulkanBufferObject* getBufferObject(gr_buffer_handle handle);
	const VulkanBufferObject* getBufferObject(gr_buffer_handle handle) const;

	// Frame bump allocator
	static constexpr size_t FRAME_ALLOC_INITIAL_SIZE = 4 * 1024 * 1024;

	bool createFrameAllocBuffer(FrameBumpAllocator& alloc, size_t size);
	void initFrameAllocators();
	void shutdownFrameAllocators();
	size_t bumpAllocate(size_t size);
	void growFrameAllocator();

	FrameBumpAllocator m_frameAllocs[MAX_FRAMES_IN_FLIGHT];
	uint32_t m_uboAlignment = 256;

	vk::Device m_device;
	VulkanMemoryManager* m_memoryManager = nullptr;

	uint32_t m_graphicsQueueFamily = 0;
	uint32_t m_transferQueueFamily = 0;
	uint32_t m_currentFrame = 0;

	SCP_vector<VulkanBufferObject> m_buffers;
	SCP_vector<int> m_freeIndices;  // Recycled buffer indices

	// Fallback color buffer containing white (1,1,1,1) for vertex data without colors
	vk::Buffer m_fallbackColorBuffer;
	VulkanAllocation m_fallbackColorAllocation;

	// Fallback texcoord buffer containing (0,0,0,0) for vertex data without texcoords
	vk::Buffer m_fallbackTexCoordBuffer;
	VulkanAllocation m_fallbackTexCoordAllocation;

	// Fallback uniform buffer (zeros) for uninitialized descriptor set UBO bindings
	static constexpr size_t FALLBACK_UNIFORM_BUFFER_SIZE = 4096;
	vk::Buffer m_fallbackUniformBuffer;
	VulkanAllocation m_fallbackUniformAllocation;

	size_t m_activeBufferCount = 0;
	size_t m_totalBufferMemory = 0;

	bool m_initialized = false;
};

// Global buffer manager instance (set during renderer init)
VulkanBufferManager* getBufferManager();
void setBufferManager(VulkanBufferManager* manager);

// ========== gr_screen function pointer implementations ==========

gr_buffer_handle vulkan_create_buffer(BufferType type, BufferUsageHint usage);
void vulkan_delete_buffer(gr_buffer_handle handle);
void vulkan_update_buffer_data(gr_buffer_handle handle, size_t size, const void* data);
void vulkan_update_buffer_data_offset(gr_buffer_handle handle, size_t offset, size_t size, const void* data);
void* vulkan_map_buffer(gr_buffer_handle handle);
void vulkan_flush_mapped_buffer(gr_buffer_handle handle, size_t offset, size_t size);
void vulkan_bind_uniform_buffer(uniform_block_type blockType, size_t offset, size_t size, gr_buffer_handle buffer);

} // namespace vulkan
} // namespace graphics
