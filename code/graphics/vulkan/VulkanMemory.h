#pragma once

#include "globalincs/pstypes.h"

#include <vulkan/vulkan.hpp>

// VMA requires the Vulkan function pointers. We use the dynamic dispatcher,
// so we supply them via VMA_DYNAMIC_VULKAN_FUNCTIONS.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
// Must stay in lockstep with VulkanApiVersion below: VMA asserts at allocator
// creation if VmaAllocatorCreateInfo::vulkanApiVersion exceeds what this
// compile-time macro enabled.
#define VMA_VULKAN_VERSION 1002000  // Vulkan 1.2

#include <vk_mem_alloc.h>


namespace graphics::vulkan {

// Forward declarations
class VulkanRenderer;

// Vulkan API version requested at instance, device, and VMA allocator
// creation. This is a declared target passed to vkCreateInstance's
// VkApplicationInfo, not a hard requirement -- the actual minimum supported
// version is enforced separately (see MinVulkanVersion in
// VulkanRendererSetup.cpp, which stays at 1.1 so 1.1-only drivers can still
// start the renderer). Bumping this value requires updating VMA_VULKAN_VERSION
// above in lockstep, or VMA's internal assert will fire.
constexpr uint32_t VulkanApiVersion = VK_API_VERSION_1_2;

/**
 * @brief What an allocation is used for, for the profiler overlay's per-purpose GPU memory
 * breakdown. Unknown covers anything not worth categorizing for that panel (transient staging
 * buffers, uniform buffers -- the latter are already tracked separately via
 * UniformBufferManager/gr_debug_stats).
 */
enum class MemoryPurpose {
	Unknown,
	Texture,
	Geometry,
	RenderTarget
};

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
	MemoryPurpose purpose = MemoryPurpose::Unknown;

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
	 * @param enableBufferDeviceAddress If true, creates the VMA allocator with
	 *        VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, required for any
	 *        buffer created with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
	 *        (raytraced shadow geometry buffers). Only valid if the device was
	 *        created with the VK_KHR_buffer_device_address feature enabled.
	 * @return true on success
	 */
	bool init(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device,
		bool enableBufferDeviceAddress = false);

	/**
	 * @brief Shutdown and free all allocations
	 */
	void shutdown();

	/**
	 * @brief Allocate memory for a buffer
	 * @param buffer The buffer to allocate memory for
	 * @param usage The intended memory usage pattern
	 * @param[out] allocation Output allocation info
	 * @param purpose What the buffer is used for, for the per-purpose memory breakdown. Defaults
	 *        to Unknown for callers that don't care about that breakdown (staging buffers, etc).
	 * @return true on success
	 */
	bool allocateBufferMemory(vk::Buffer buffer, MemoryUsage usage, VulkanAllocation& allocation,
		MemoryPurpose purpose = MemoryPurpose::Unknown);

	/**
	 * @brief Allocate memory for an image
	 * @param image The image to allocate memory for
	 * @param usage The intended memory usage pattern
	 * @param[out] allocation Output allocation info
	 * @param purpose What the image is used for, for the per-purpose memory breakdown. Defaults
	 *        to Unknown for callers that don't care about that breakdown.
	 * @return true on success
	 */
	bool allocateImageMemory(vk::Image image, MemoryUsage usage, VulkanAllocation& allocation,
		MemoryPurpose purpose = MemoryPurpose::Unknown);

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

	size_t getTextureBytes() const { return m_bytesByPurpose[static_cast<size_t>(MemoryPurpose::Texture)]; }
	size_t getGeometryBytes() const { return m_bytesByPurpose[static_cast<size_t>(MemoryPurpose::Geometry)]; }
	size_t getRenderTargetBytes() const { return m_bytesByPurpose[static_cast<size_t>(MemoryPurpose::RenderTarget)]; }

private:
	static VmaMemoryUsage toVmaUsage(MemoryUsage usage);

	VmaAllocator m_allocator = VK_NULL_HANDLE;

	size_t m_allocationCount = 0;
	size_t m_totalAllocatedBytes = 0;
	size_t m_bytesByPurpose[4] = {}; // indexed by MemoryPurpose

	bool m_initialized = false;
};

// Global memory manager instance (set during renderer init)
VulkanMemoryManager* getMemoryManager();
void setMemoryManager(VulkanMemoryManager* manager);

} // namespace graphics::vulkan
