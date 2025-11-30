#pragma once

#include "graphics/2d.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

// Forward declarations
class VulkanRenderer;

/**
 * @brief Buffer data tracking synchronization, memory, and mapping state
 */
struct VulkanBufferData {
	vk::Buffer buffer;
	vk::DeviceMemory memory;
	BufferType type = BufferType::Vertex;
	BufferUsageHint usage = BufferUsageHint::Static;
	size_t size = 0;
	void* mappedPtr = nullptr;           // For persistent mapping
	uint32_t lastUsedFrame = 0;          // Frame sync tracking
	bool hostVisible = false;            // Memory property
};

/**
 * @brief Manages Vulkan buffer allocation, updates, and lifecycle
 *
 * Design notes:
 * - Uses slot allocator (free list + dense array) for fast handle management
 * - Tracks frame usage for synchronization (can't update in-flight buffers)
 * - Supports persistent mapping for streaming/dynamic buffers
 * - Staging buffer pool for device-local buffer updates
 */
class VulkanBufferManager {
public:
	explicit VulkanBufferManager(vk::Device device, vk::PhysicalDevice physicalDevice);
	~VulkanBufferManager();

	// Disable copy
	VulkanBufferManager(const VulkanBufferManager&) = delete;
	VulkanBufferManager& operator=(const VulkanBufferManager&) = delete;

	/**
	 * @brief Initialize the buffer manager
	 * @param device Logical Vulkan device
	 * @param physicalDevice Physical device for memory properties
	 */
	void initialize(vk::Device device, vk::PhysicalDevice physicalDevice);

	/**
	 * @brief Shutdown and release all buffers
	 */
	void shutdown();

	/**
	 * @brief Create a new buffer
	 * @param type Buffer type (Vertex, Index, Uniform)
	 * @param usage Usage hint for memory allocation strategy
	 * @return Handle to the created buffer
	 */
	gr_buffer_handle createBuffer(BufferType type, BufferUsageHint usage);

	/**
	 * @brief Delete a buffer
	 * @param handle Buffer handle to delete
	 */
	void deleteBuffer(gr_buffer_handle handle);

	/**
	 * @brief Update buffer data (full replacement)
	 * @param handle Buffer handle
	 * @param size Data size in bytes
	 * @param data Pointer to data (can be nullptr for allocation only)
	 */
	void updateBufferData(gr_buffer_handle handle, size_t size, const void* data);

	/**
	 * @brief Update buffer data at offset (partial update)
	 * @param handle Buffer handle
	 * @param offset Byte offset into buffer
	 * @param size Data size in bytes
	 * @param data Pointer to data
	 */
	void updateBufferDataOffset(gr_buffer_handle handle, size_t offset, size_t size, const void* data);

	/**
	 * @brief Map buffer for persistent write access
	 * @param handle Buffer handle (must be PersistentMapping usage)
	 * @return Mapped pointer, or nullptr on failure
	 */
	void* mapBuffer(gr_buffer_handle handle);

	/**
	 * @brief Flush a range of a mapped buffer to make writes visible to GPU
	 * @param handle Buffer handle
	 * @param offset Byte offset of range to flush
	 * @param size Size of range to flush
	 */
	void flushMappedBuffer(gr_buffer_handle handle, size_t offset, size_t size);

	/**
	 * @brief Bind a uniform buffer to a binding point
	 * @param bindPoint Uniform block binding point
	 * @param offset Byte offset (must respect minUniformBufferOffsetAlignment)
	 * @param size Size of the range
	 * @param handle Buffer handle
	 */
	void bindUniformBuffer(uniform_block_type bindPoint, size_t offset, size_t size, gr_buffer_handle handle);

	/**
	 * @brief Get the Vulkan buffer handle for a gr_buffer_handle
	 * @param handle Buffer handle
	 * @return Vulkan buffer, or null handle if invalid
	 */
	vk::Buffer getBuffer(gr_buffer_handle handle) const;

	/**
	 * @brief Get buffer data (for internal use)
	 * @param handle Buffer handle
	 * @return Pointer to buffer data, or nullptr if invalid
	 */
	const VulkanBufferData* getBufferData(gr_buffer_handle handle) const;

	/**
	 * @brief Called at end of frame to advance frame counter
	 */
	void endFrame();

	/**
	 * @brief Get minimum UBO offset alignment
	 */
	size_t getMinUniformBufferOffsetAlignment() const { return m_minUboAlignment; }

private:
	/**
	 * @brief Find suitable memory type for buffer allocation
	 * @param typeFilter Bitmask of acceptable memory types
	 * @param properties Required memory properties
	 * @return Memory type index
	 */
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

	/**
	 * @brief Create buffer with given usage and properties
	 */
	void createVkBuffer(VulkanBufferData& bufferData, size_t size, vk::BufferUsageFlags usage,
	                    vk::MemoryPropertyFlags properties);

	/**
	 * @brief Copy data via staging buffer (for device-local buffers)
	 */
	void copyViaStaging(VulkanBufferData& dst, size_t offset, size_t size, const void* data);

	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	vk::PhysicalDeviceMemoryProperties m_memoryProperties;

	// Buffer storage with free list for slot reuse
	SCP_vector<VulkanBufferData> m_buffers;
	SCP_vector<int> m_freeSlots;

	// Device limits
	size_t m_minUboAlignment = 256;

	// Frame tracking
	uint32_t m_currentFrame = 0;

	// Staging resources (for device-local buffer updates)
	// TODO: Implement staging buffer pool
	vk::CommandPool m_transferCommandPool;
	vk::Queue m_transferQueue;

	bool m_initialized = false;
};

// Global buffer manager instance (set by VulkanRenderer)
extern VulkanBufferManager* g_vulkanBufferManager;

// Function pointer implementations for gr_screen
gr_buffer_handle gr_vulkan_create_buffer(BufferType type, BufferUsageHint usage);
void gr_vulkan_delete_buffer(gr_buffer_handle handle);
void gr_vulkan_update_buffer_data(gr_buffer_handle handle, size_t size, const void* data);
void gr_vulkan_update_buffer_data_offset(gr_buffer_handle handle, size_t offset, size_t size, const void* data);
void* gr_vulkan_map_buffer(gr_buffer_handle handle);
void gr_vulkan_flush_mapped_buffer(gr_buffer_handle handle, size_t offset, size_t size);
void gr_vulkan_bind_uniform_buffer(uniform_block_type bindPoint, size_t offset, size_t size, gr_buffer_handle handle);

} // namespace vulkan
} // namespace graphics
