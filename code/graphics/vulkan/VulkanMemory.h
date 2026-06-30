#pragma once

#include "globalincs/pstypes.h"

#include <vulkan/vulkan.hpp>

// VMA requires the Vulkan function pointers. We use the dynamic dispatcher,
// so we supply them via VMA_DYNAMIC_VULKAN_FUNCTIONS.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1001000  // Vulkan 1.1

#include <vk_mem_alloc.h>


namespace graphics::vulkan {

// Forward declarations
class VulkanRenderer;

/**
 * @brief Memory allocation info returned when allocating GPU memory.
 *
 * Wraps a VmaAllocation handle. Callers should use isValid() instead of
 * checking internal fields directly.
 */
struct VulkanAllocation {
	VmaAllocation vmaAlloc = VK_NULL_HANDLE;
	vk::DeviceSize size = 0;
	void* mappedPtr = nullptr;  // Non-null if memory is persistently mapped

	bool isValid() const { return vmaAlloc != VK_NULL_HANDLE; }
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
 * @brief Memory manager for Vulkan GPU memory allocations, backed by VMA.
 *
 * VMA sub-allocates from large VkDeviceMemory blocks, avoiding the
 * per-object allocation limit (maxMemoryAllocationCount) that the
 * previous simple allocator could hit with large mods.
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
	bool init(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);

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
	void flushMemory(const VulkanAllocation& allocation, vk::DeviceSize offset, vk::DeviceSize size);

	/**
	 * @brief Invalidate mapped memory to make GPU writes visible to CPU
	 * @param allocation The allocation containing the range to invalidate
	 * @param offset Offset within the allocation
	 * @param size Size of the range to invalidate (VK_WHOLE_SIZE for entire allocation)
	 */
	void invalidateMemory(const VulkanAllocation& allocation, vk::DeviceSize offset, vk::DeviceSize size);

	/**
	 * @brief Get memory statistics
	 */
	size_t getAllocationCount() const { return m_allocationCount; }
	size_t getTotalAllocatedBytes() const { return m_totalAllocatedBytes; }

private:
	static VmaMemoryUsage toVmaUsage(MemoryUsage usage);

	VmaAllocator m_allocator = VK_NULL_HANDLE;

	size_t m_allocationCount = 0;
	size_t m_totalAllocatedBytes = 0;

	bool m_initialized = false;
};

// Global memory manager instance (set during renderer init)
VulkanMemoryManager* getMemoryManager();
void setMemoryManager(VulkanMemoryManager* manager);

} // namespace graphics::vulkan
