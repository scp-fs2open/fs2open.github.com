#define VMA_IMPLEMENTATION
#include "VulkanMemory.h"

#include "globalincs/pstypes.h"


namespace graphics::vulkan {

namespace {
VulkanMemoryManager* g_memoryManager = nullptr;
}

VulkanMemoryManager* getMemoryManager()
{
	return g_memoryManager;
}

void setMemoryManager(VulkanMemoryManager* manager)
{
	g_memoryManager = manager;
}

VulkanMemoryManager::VulkanMemoryManager() = default;

VulkanMemoryManager::~VulkanMemoryManager()
{
	if (m_initialized) {
		shutdown();
	}
}

bool VulkanMemoryManager::init(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device)
{
	if (m_initialized) {
		mprintf(("VulkanMemoryManager::init called when already initialized!\n"));
		return false;
	}

	// Log memory properties for diagnostics
	auto memoryProperties = physicalDevice.getMemoryProperties();
	mprintf(("Vulkan Memory Manager initializing (VMA)\n"));
	mprintf(("  Memory heaps: %u\n", memoryProperties.memoryHeapCount));
	for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
		const auto& heap = memoryProperties.memoryHeaps[i];
		mprintf(("    Heap %u: %zu MB%s\n",
			i,
			static_cast<size_t>(heap.size / (1024 * 1024)),
			(heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) ? " (device local)" : ""));
	}

	// Resolve Vulkan function pointers for VMA from the dynamic dispatcher
	VmaVulkanFunctions vulkanFunctions = {};
	auto const& d = VULKAN_HPP_DEFAULT_DISPATCHER;
	vulkanFunctions.vkGetInstanceProcAddr                   = d.vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr                     = d.vkGetDeviceProcAddr;
	vulkanFunctions.vkGetPhysicalDeviceProperties           = d.vkGetPhysicalDeviceProperties;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties     = d.vkGetPhysicalDeviceMemoryProperties;
	vulkanFunctions.vkAllocateMemory                        = d.vkAllocateMemory;
	vulkanFunctions.vkFreeMemory                            = d.vkFreeMemory;
	vulkanFunctions.vkMapMemory                             = d.vkMapMemory;
	vulkanFunctions.vkUnmapMemory                           = d.vkUnmapMemory;
	vulkanFunctions.vkFlushMappedMemoryRanges               = d.vkFlushMappedMemoryRanges;
	vulkanFunctions.vkInvalidateMappedMemoryRanges          = d.vkInvalidateMappedMemoryRanges;
	vulkanFunctions.vkBindBufferMemory                      = d.vkBindBufferMemory;
	vulkanFunctions.vkBindImageMemory                       = d.vkBindImageMemory;
	vulkanFunctions.vkGetBufferMemoryRequirements           = d.vkGetBufferMemoryRequirements;
	vulkanFunctions.vkGetImageMemoryRequirements            = d.vkGetImageMemoryRequirements;
	vulkanFunctions.vkCreateBuffer                          = d.vkCreateBuffer;
	vulkanFunctions.vkDestroyBuffer                         = d.vkDestroyBuffer;
	vulkanFunctions.vkCreateImage                           = d.vkCreateImage;
	vulkanFunctions.vkDestroyImage                          = d.vkDestroyImage;
	vulkanFunctions.vkCmdCopyBuffer                         = d.vkCmdCopyBuffer;
	vulkanFunctions.vkGetBufferMemoryRequirements2KHR       = d.vkGetBufferMemoryRequirements2;
	vulkanFunctions.vkGetImageMemoryRequirements2KHR        = d.vkGetImageMemoryRequirements2;
	vulkanFunctions.vkBindBufferMemory2KHR                  = d.vkBindBufferMemory2;
	vulkanFunctions.vkBindImageMemory2KHR                   = d.vkBindImageMemory2;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = d.vkGetPhysicalDeviceMemoryProperties2;

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_1;
	allocatorInfo.physicalDevice = static_cast<VkPhysicalDevice>(physicalDevice);
	allocatorInfo.device = static_cast<VkDevice>(device);
	allocatorInfo.instance = static_cast<VkInstance>(instance);
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;

	VkResult result = vmaCreateAllocator(&allocatorInfo, &m_allocator);
	if (result != VK_SUCCESS) {
		mprintf(("Failed to create VMA allocator! VkResult: %d\n", static_cast<int>(result)));
		return false;
	}

	mprintf(("Vulkan Memory Manager initialized (VMA)\n"));
	m_initialized = true;
	return true;
}

void VulkanMemoryManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	if (m_allocationCount > 0) {
		mprintf(("WARNING: VulkanMemoryManager shutdown with %zu allocations still active!\n", m_allocationCount));
	}

	if (m_allocator != VK_NULL_HANDLE) {
		vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;
	}

	m_allocationCount = 0;
	m_totalAllocatedBytes = 0;
	m_initialized = false;
}

VmaMemoryUsage VulkanMemoryManager::toVmaUsage(MemoryUsage usage)
{
	switch (usage) {
	case MemoryUsage::GpuOnly:
		return VMA_MEMORY_USAGE_GPU_ONLY;
	case MemoryUsage::CpuToGpu:
		return VMA_MEMORY_USAGE_CPU_TO_GPU;
	case MemoryUsage::GpuToCpu:
		return VMA_MEMORY_USAGE_GPU_TO_CPU;
	case MemoryUsage::CpuOnly:
		return VMA_MEMORY_USAGE_CPU_ONLY;
	default:
		return VMA_MEMORY_USAGE_CPU_TO_GPU;
	}
}

bool VulkanMemoryManager::allocateBufferMemory(vk::Buffer buffer, MemoryUsage usage, VulkanAllocation& allocation)
{
	if (!m_initialized) {
		mprintf(("VulkanMemoryManager::allocateBufferMemory called before initialization!\n"));
		return false;
	}

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = toVmaUsage(usage);

	VmaAllocationInfo allocInfo;
	VkResult result = vmaAllocateMemoryForBuffer(
		m_allocator,
		static_cast<VkBuffer>(buffer),
		&allocCreateInfo,
		&allocation.vmaAlloc,
		&allocInfo);

	if (result != VK_SUCCESS) {
		mprintf(("Failed to allocate buffer memory via VMA! VkResult: %d (allocations: %zu, total: %zu bytes)\n",
			static_cast<int>(result), m_allocationCount, m_totalAllocatedBytes));
		allocation.vmaAlloc = VK_NULL_HANDLE;
		return false;
	}

	// Bind the memory to the buffer
	result = vmaBindBufferMemory(m_allocator, allocation.vmaAlloc, static_cast<VkBuffer>(buffer));
	if (result != VK_SUCCESS) {
		mprintf(("Failed to bind buffer memory via VMA! VkResult: %d\n", static_cast<int>(result)));
		vmaFreeMemory(m_allocator, allocation.vmaAlloc);
		allocation.vmaAlloc = VK_NULL_HANDLE;
		return false;
	}

	allocation.size = allocInfo.size;
	allocation.mappedPtr = allocInfo.pMappedData;

	++m_allocationCount;
	m_totalAllocatedBytes += allocation.size;

	return true;
}

bool VulkanMemoryManager::allocateImageMemory(vk::Image image, MemoryUsage usage, VulkanAllocation& allocation)
{
	if (!m_initialized) {
		mprintf(("VulkanMemoryManager::allocateImageMemory called before initialization!\n"));
		return false;
	}

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = toVmaUsage(usage);

	VmaAllocationInfo allocInfo;
	VkResult result = vmaAllocateMemoryForImage(
		m_allocator,
		static_cast<VkImage>(image),
		&allocCreateInfo,
		&allocation.vmaAlloc,
		&allocInfo);

	if (result != VK_SUCCESS) {
		mprintf(("Failed to allocate image memory via VMA! VkResult: %d (allocations: %zu, total: %zu bytes)\n",
			static_cast<int>(result), m_allocationCount, m_totalAllocatedBytes));
		allocation.vmaAlloc = VK_NULL_HANDLE;
		return false;
	}

	// Bind the memory to the image
	result = vmaBindImageMemory(m_allocator, allocation.vmaAlloc, static_cast<VkImage>(image));
	if (result != VK_SUCCESS) {
		mprintf(("Failed to bind image memory via VMA! VkResult: %d\n", static_cast<int>(result)));
		vmaFreeMemory(m_allocator, allocation.vmaAlloc);
		allocation.vmaAlloc = VK_NULL_HANDLE;
		return false;
	}

	allocation.size = allocInfo.size;
	allocation.mappedPtr = allocInfo.pMappedData;

	++m_allocationCount;
	m_totalAllocatedBytes += allocation.size;

	return true;
}

void VulkanMemoryManager::freeAllocation(VulkanAllocation& allocation)
{
	if (!m_initialized || !allocation.isValid()) {
		return;
	}

	// Unmap if still mapped — VMA asserts map count == 0 on free
	if (allocation.mappedPtr != nullptr) {
		vmaUnmapMemory(m_allocator, allocation.vmaAlloc);
		allocation.mappedPtr = nullptr;
	}

	vmaFreeMemory(m_allocator, allocation.vmaAlloc);

	--m_allocationCount;
	m_totalAllocatedBytes -= allocation.size;

	allocation.vmaAlloc = VK_NULL_HANDLE;
	allocation.size = 0;
	allocation.mappedPtr = nullptr;
}

void* VulkanMemoryManager::mapMemory(VulkanAllocation& allocation)
{
	if (!m_initialized || !allocation.isValid()) {
		return nullptr;
	}

	if (allocation.mappedPtr != nullptr) {
		// Already mapped (VMA supports nested map calls via refcount)
		return allocation.mappedPtr;
	}

	VkResult result = vmaMapMemory(m_allocator, allocation.vmaAlloc, &allocation.mappedPtr);
	if (result != VK_SUCCESS) {
		mprintf(("Failed to map memory via VMA! VkResult: %d\n", static_cast<int>(result)));
		allocation.mappedPtr = nullptr;
		return nullptr;
	}

	return allocation.mappedPtr;
}

void VulkanMemoryManager::unmapMemory(VulkanAllocation& allocation)
{
	if (!m_initialized || !allocation.isValid() || allocation.mappedPtr == nullptr) {
		return;
	}

	vmaUnmapMemory(m_allocator, allocation.vmaAlloc);
	allocation.mappedPtr = nullptr;
}

void VulkanMemoryManager::flushMemory(const VulkanAllocation& allocation, vk::DeviceSize offset, vk::DeviceSize size)
{
	if (!m_initialized || !allocation.isValid()) {
		return;
	}

	vmaFlushAllocation(m_allocator, allocation.vmaAlloc,
		static_cast<VkDeviceSize>(offset),
		(size == VK_WHOLE_SIZE) ? allocation.size : static_cast<VkDeviceSize>(size));
}

void VulkanMemoryManager::invalidateMemory(const VulkanAllocation& allocation, vk::DeviceSize offset, vk::DeviceSize size)
{
	if (!m_initialized || !allocation.isValid()) {
		return;
	}

	vmaInvalidateAllocation(m_allocator, allocation.vmaAlloc,
		static_cast<VkDeviceSize>(offset),
		(size == VK_WHOLE_SIZE) ? allocation.size : static_cast<VkDeviceSize>(size));
}

} // namespace graphics::vulkan
