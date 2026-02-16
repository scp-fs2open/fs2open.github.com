#pragma once

#include "globalincs/pstypes.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

// Forward declarations
class VulkanRenderer;

/**
 * @brief Memory allocation info returned when allocating GPU memory
 */
struct VulkanAllocation {
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize offset = 0;
	VkDeviceSize size = 0;
	void* mappedPtr = nullptr;  // Non-null if memory is mapped
	uint32_t memoryTypeIndex = 0;
	bool dedicated = false;  // True if this is a dedicated allocation
};

/**
 * @brief Flags for memory allocation requirements
 */
enum class MemoryUsage {
	GpuOnly,          // Device local, not host visible (fastest for GPU)
	CpuToGpu,         // Host visible, preferably device local (for uploads)
	GpuToCpu,         // Host visible, preferably cached (for readbacks)
	CpuOnly           // Host visible and coherent (for staging)
};

/**
 * @brief Memory manager for Vulkan GPU memory allocations
 *
 * This is a simple allocator that creates one VkDeviceMemory per allocation.
 * It's designed to be easily replaceable with VMA (Vulkan Memory Allocator)
 * in the future for better memory efficiency.
 */
class VulkanMemoryManager {
public:
	VulkanMemoryManager();
	~VulkanMemoryManager();

	// Non-copyable
	VulkanMemoryManager(const VulkanMemoryManager&) = delete;
	VulkanMemoryManager& operator=(const VulkanMemoryManager&) = delete;

	/**
	 * @brief Initialize the memory manager
	 * @param physicalDevice The physical device to query memory properties from
	 * @param device The logical device for allocations
	 * @return true on success
	 */
	bool init(vk::PhysicalDevice physicalDevice, vk::Device device);

	/**
	 * @brief Shutdown and free all allocations
	 */
	void shutdown();

	/**
	 * @brief Allocate memory for a buffer
	 * @param buffer The buffer to allocate memory for
	 * @param usage The intended memory usage pattern
	 * @param[out] allocation Output allocation info
	 * @return true on success
	 */
	bool allocateBufferMemory(vk::Buffer buffer, MemoryUsage usage, VulkanAllocation& allocation);

	/**
	 * @brief Allocate memory for an image
	 * @param image The image to allocate memory for
	 * @param usage The intended memory usage pattern
	 * @param[out] allocation Output allocation info
	 * @return true on success
	 */
	bool allocateImageMemory(vk::Image image, MemoryUsage usage, VulkanAllocation& allocation);

	/**
	 * @brief Free a previous allocation
	 * @param allocation The allocation to free
	 */
	void freeAllocation(VulkanAllocation& allocation);

	/**
	 * @brief Map memory for CPU access
	 * @param allocation The allocation to map
	 * @return Pointer to mapped memory, or nullptr on failure
	 */
	void* mapMemory(VulkanAllocation& allocation);

	/**
	 * @brief Unmap previously mapped memory
	 * @param allocation The allocation to unmap
	 */
	void unmapMemory(VulkanAllocation& allocation);

	/**
	 * @brief Flush mapped memory to make writes visible to GPU
	 * @param allocation The allocation containing the range to flush
	 * @param offset Offset within the allocation
	 * @param size Size of the range to flush (VK_WHOLE_SIZE for entire allocation)
	 */
	void flushMemory(const VulkanAllocation& allocation, VkDeviceSize offset, VkDeviceSize size);

	/**
	 * @brief Invalidate mapped memory to make GPU writes visible to CPU
	 * @param allocation The allocation containing the range to invalidate
	 * @param offset Offset within the allocation
	 * @param size Size of the range to invalidate (VK_WHOLE_SIZE for entire allocation)
	 */
	void invalidateMemory(const VulkanAllocation& allocation, VkDeviceSize offset, VkDeviceSize size);

	/**
	 * @brief Get memory statistics
	 */
	size_t getAllocationCount() const { return m_allocationCount; }
	size_t getTotalAllocatedBytes() const { return m_totalAllocatedBytes; }

private:
	/**
	 * @brief Find a suitable memory type index
	 * @param memoryTypeBits Bitmask of acceptable memory types
	 * @param requiredFlags Required memory property flags
	 * @param preferredFlags Preferred memory property flags (optional)
	 * @param[out] memoryTypeIndex Output memory type index
	 * @return true if a suitable memory type was found
	 */
	bool findMemoryType(uint32_t memoryTypeBits,
	                    vk::MemoryPropertyFlags requiredFlags,
	                    vk::MemoryPropertyFlags preferredFlags,
	                    uint32_t& memoryTypeIndex);

	/**
	 * @brief Convert usage enum to Vulkan memory property flags
	 */
	void getMemoryFlags(MemoryUsage usage,
	                    vk::MemoryPropertyFlags& requiredFlags,
	                    vk::MemoryPropertyFlags& preferredFlags);

	vk::PhysicalDevice m_physicalDevice;
	vk::Device m_device;
	vk::PhysicalDeviceMemoryProperties m_memoryProperties;

	size_t m_allocationCount = 0;
	size_t m_totalAllocatedBytes = 0;

	bool m_initialized = false;
};

// Global memory manager instance (set during renderer init)
VulkanMemoryManager* getMemoryManager();
void setMemoryManager(VulkanMemoryManager* manager);

} // namespace vulkan
} // namespace graphics
