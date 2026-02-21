#include "VulkanMemory.h"

#include "globalincs/pstypes.h"

namespace graphics {
namespace vulkan {

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

bool VulkanMemoryManager::init(vk::PhysicalDevice physicalDevice, vk::Device device)
{
	if (m_initialized) {
		mprintf(("VulkanMemoryManager::init called when already initialized!\n"));
		return false;
	}

	m_physicalDevice = physicalDevice;
	m_device = device;
	m_memoryProperties = physicalDevice.getMemoryProperties();

	mprintf(("Vulkan Memory Manager initialized\n"));
	mprintf(("  Memory heaps: %u\n", m_memoryProperties.memoryHeapCount));
	for (uint32_t i = 0; i < m_memoryProperties.memoryHeapCount; ++i) {
		const auto& heap = m_memoryProperties.memoryHeaps[i];
		mprintf(("    Heap %u: %zu MB%s\n",
			i,
			static_cast<size_t>(heap.size / (1024 * 1024)),
			(heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) ? " (device local)" : ""));
	}

	mprintf(("  Memory types: %u\n", m_memoryProperties.memoryTypeCount));
	for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; ++i) {
		const auto& type = m_memoryProperties.memoryTypes[i];
		SCP_string flags;
		if (type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
			flags += "DeviceLocal ";
		if (type.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible)
			flags += "HostVisible ";
		if (type.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent)
			flags += "HostCoherent ";
		if (type.propertyFlags & vk::MemoryPropertyFlagBits::eHostCached)
			flags += "HostCached ";
		mprintf(("    Type %u: heap %u, flags: %s\n", i, type.heapIndex, flags.c_str()));
	}

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

	m_physicalDevice = nullptr;
	m_device = nullptr;
	m_allocationCount = 0;
	m_totalAllocatedBytes = 0;
	m_initialized = false;
}

bool VulkanMemoryManager::findMemoryType(uint32_t memoryTypeBits,
                                         vk::MemoryPropertyFlags requiredFlags,
                                         vk::MemoryPropertyFlags preferredFlags,
                                         uint32_t& memoryTypeIndex)
{
	// First try to find a memory type with both required and preferred flags
	for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; ++i) {
		if ((memoryTypeBits & (1u << i)) &&
		    (m_memoryProperties.memoryTypes[i].propertyFlags & (requiredFlags | preferredFlags)) ==
		        (requiredFlags | preferredFlags)) {
			memoryTypeIndex = i;
			return true;
		}
	}

	// Fall back to just required flags
	for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; ++i) {
		if ((memoryTypeBits & (1u << i)) &&
		    (m_memoryProperties.memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags) {
			memoryTypeIndex = i;
			return true;
		}
	}

	return false;
}

void VulkanMemoryManager::getMemoryFlags(MemoryUsage usage,
                                         vk::MemoryPropertyFlags& requiredFlags,
                                         vk::MemoryPropertyFlags& preferredFlags)
{
	switch (usage) {
	case MemoryUsage::GpuOnly:
		requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		preferredFlags = {};
		break;

	case MemoryUsage::CpuToGpu:
		requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible;
		preferredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		break;

	case MemoryUsage::GpuToCpu:
		requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible;
		preferredFlags = vk::MemoryPropertyFlagBits::eHostCached;
		break;

	case MemoryUsage::CpuOnly:
		requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		preferredFlags = {};
		break;
	}
}

bool VulkanMemoryManager::allocateBufferMemory(vk::Buffer buffer, MemoryUsage usage, VulkanAllocation& allocation)
{
	if (!m_initialized) {
		mprintf(("VulkanMemoryManager::allocateBufferMemory called before initialization!\n"));
		return false;
	}

	vk::MemoryRequirements memReqs = m_device.getBufferMemoryRequirements(buffer);

	vk::MemoryPropertyFlags requiredFlags, preferredFlags;
	getMemoryFlags(usage, requiredFlags, preferredFlags);

	uint32_t memoryTypeIndex;
	if (!findMemoryType(memReqs.memoryTypeBits, requiredFlags, preferredFlags, memoryTypeIndex)) {
		mprintf(("Failed to find suitable memory type for buffer!\n"));
		return false;
	}

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;

	try {
		allocation.memory = m_device.allocateMemory(allocInfo);
		allocation.offset = 0;
		allocation.size = memReqs.size;
		allocation.memoryTypeIndex = memoryTypeIndex;
		allocation.mappedPtr = nullptr;
		allocation.dedicated = true;  // Simple allocator always does dedicated allocations

		m_device.bindBufferMemory(buffer, allocation.memory, 0);

		++m_allocationCount;
		m_totalAllocatedBytes += allocation.size;

		return true;
	} catch (const vk::SystemError& e) {
		mprintf(("Failed to allocate buffer memory: %s\n", e.what()));
		return false;
	}
}

bool VulkanMemoryManager::allocateImageMemory(vk::Image image, MemoryUsage usage, VulkanAllocation& allocation)
{
	if (!m_initialized) {
		mprintf(("VulkanMemoryManager::allocateImageMemory called before initialization!\n"));
		return false;
	}

	vk::MemoryRequirements memReqs = m_device.getImageMemoryRequirements(image);

	vk::MemoryPropertyFlags requiredFlags, preferredFlags;
	getMemoryFlags(usage, requiredFlags, preferredFlags);

	uint32_t memoryTypeIndex;
	if (!findMemoryType(memReqs.memoryTypeBits, requiredFlags, preferredFlags, memoryTypeIndex)) {
		mprintf(("Failed to find suitable memory type for image!\n"));
		return false;
	}

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;

	try {
		allocation.memory = m_device.allocateMemory(allocInfo);
		allocation.offset = 0;
		allocation.size = memReqs.size;
		allocation.memoryTypeIndex = memoryTypeIndex;
		allocation.mappedPtr = nullptr;
		allocation.dedicated = true;

		m_device.bindImageMemory(image, allocation.memory, 0);

		++m_allocationCount;
		m_totalAllocatedBytes += allocation.size;

		return true;
	} catch (const vk::SystemError& e) {
		mprintf(("Failed to allocate image memory: %s\n", e.what()));
		return false;
	}
}

void VulkanMemoryManager::freeAllocation(VulkanAllocation& allocation)
{
	if (!m_initialized || allocation.memory == VK_NULL_HANDLE) {
		return;
	}

	// Unmap if mapped
	if (allocation.mappedPtr != nullptr) {
		unmapMemory(allocation);
	}

	m_device.freeMemory(allocation.memory);

	--m_allocationCount;
	m_totalAllocatedBytes -= allocation.size;

	allocation.memory = VK_NULL_HANDLE;
	allocation.offset = 0;
	allocation.size = 0;
	allocation.mappedPtr = nullptr;
}

void* VulkanMemoryManager::mapMemory(VulkanAllocation& allocation)
{
	if (!m_initialized || allocation.memory == VK_NULL_HANDLE) {
		return nullptr;
	}

	if (allocation.mappedPtr != nullptr) {
		// Already mapped
		return allocation.mappedPtr;
	}

	// Check if memory is host visible
	const auto& memType = m_memoryProperties.memoryTypes[allocation.memoryTypeIndex];
	if (!(memType.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible)) {
		mprintf(("Attempted to map non-host-visible memory!\n"));
		return nullptr;
	}

	try {
		allocation.mappedPtr = m_device.mapMemory(allocation.memory, allocation.offset, allocation.size);
		return allocation.mappedPtr;
	} catch (const vk::SystemError& e) {
		mprintf(("Failed to map memory: %s\n", e.what()));
		return nullptr;
	}
}

void VulkanMemoryManager::unmapMemory(VulkanAllocation& allocation)
{
	if (!m_initialized || allocation.memory == VK_NULL_HANDLE || allocation.mappedPtr == nullptr) {
		return;
	}

	m_device.unmapMemory(allocation.memory);
	allocation.mappedPtr = nullptr;
}

void VulkanMemoryManager::flushMemory(const VulkanAllocation& allocation, VkDeviceSize offset, VkDeviceSize size)
{
	if (!m_initialized || allocation.memory == VK_NULL_HANDLE) {
		return;
	}

	// Check if memory is host coherent - if so, no flush needed
	const auto& memType = m_memoryProperties.memoryTypes[allocation.memoryTypeIndex];
	if (memType.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) {
		return;  // Coherent memory doesn't need explicit flushing
	}

	vk::MappedMemoryRange range;
	range.memory = allocation.memory;
	range.offset = allocation.offset + offset;
	range.size = (size == VK_WHOLE_SIZE) ? allocation.size : size;

	m_device.flushMappedMemoryRanges(range);
}

void VulkanMemoryManager::invalidateMemory(const VulkanAllocation& allocation, VkDeviceSize offset, VkDeviceSize size)
{
	if (!m_initialized || allocation.memory == VK_NULL_HANDLE) {
		return;
	}

	// Check if memory is host coherent - if so, no invalidate needed
	const auto& memType = m_memoryProperties.memoryTypes[allocation.memoryTypeIndex];
	if (memType.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) {
		return;  // Coherent memory doesn't need explicit invalidation
	}

	vk::MappedMemoryRange range;
	range.memory = allocation.memory;
	range.offset = allocation.offset + offset;
	range.size = (size == VK_WHOLE_SIZE) ? allocation.size : size;

	m_device.invalidateMappedMemoryRanges(range);
}

} // namespace vulkan
} // namespace graphics
