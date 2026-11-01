#include "VulkanPerFrameUbo.h"

#include "globalincs/pstypes.h"

namespace graphics::vulkan {

bool PerFrameUboRing::init(vk::Device device, VulkanMemoryManager* memoryManager, uint32_t slotsPerFrame,
	vk::DeviceSize slotSize)
{
	Assertion(!m_buffer, "PerFrameUboRing::init called twice!");
	Assertion(slotsPerFrame > 0 && slotSize > 0, "PerFrameUboRing::init called with empty geometry!");

	m_device = device;
	m_memoryManager = memoryManager;
	m_slotsPerFrame = slotsPerFrame;
	m_slotSize = slotSize;

	vk::BufferCreateInfo bufInfo;
	bufInfo.size = static_cast<vk::DeviceSize>(MAX_FRAMES_IN_FLIGHT) * slotsPerFrame * slotSize;
	bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	bufInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		m_buffer = m_device.createBuffer(bufInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "PerFrameUboRing: failed to create buffer: %s\n", e.what()));
		return false;
	}

	if (!m_memoryManager->allocateBufferMemory(m_buffer, MemoryUsage::CpuToGpu, m_allocation)) {
		nprintf(("vulkan", "PerFrameUboRing: failed to allocate memory\n"));
		m_device.destroyBuffer(m_buffer);
		m_buffer = nullptr;
		return false;
	}

	m_mapped = m_memoryManager->mapMemory(m_allocation);
	if (m_mapped == nullptr) {
		nprintf(("vulkan", "PerFrameUboRing: failed to map memory\n"));
		m_memoryManager->freeAllocation(m_allocation);
		m_device.destroyBuffer(m_buffer);
		m_buffer = nullptr;
		return false;
	}

	m_cursors = {};
	return true;
}

void PerFrameUboRing::shutdown()
{
	if (m_mapped != nullptr) {
		m_memoryManager->unmapMemory(m_allocation);
		m_mapped = nullptr;
	}
	if (m_buffer) {
		m_device.destroyBuffer(m_buffer);
		m_buffer = nullptr;
	}
	if (m_allocation.isValid()) {
		m_memoryManager->freeAllocation(m_allocation);
	}
	m_cursors = {};
}

void PerFrameUboRing::resetCursor(uint32_t frameIndex)
{
	m_cursors[frameIndex % MAX_FRAMES_IN_FLIGHT] = 0;
}

vk::DeviceSize PerFrameUboRing::alloc(uint32_t frameIndex, const void* data, size_t size)
{
	Assertion(isValid(), "PerFrameUboRing::alloc called on an uninitialized ring!");
	Assertion(static_cast<vk::DeviceSize>(size) <= m_slotSize,
		"PerFrameUboRing::alloc: %zu bytes exceeds the %zu-byte slot size!", size,
		static_cast<size_t>(m_slotSize));

	const uint32_t frame = frameIndex % MAX_FRAMES_IN_FLIGHT;
	uint32_t& cursor = m_cursors[frame];
	Assertion(cursor < m_slotsPerFrame, "PerFrameUboRing::alloc: slot overflow (%u slots per frame)!",
		m_slotsPerFrame);

	const vk::DeviceSize frameBase = static_cast<vk::DeviceSize>(frame) * m_slotsPerFrame * m_slotSize;
	const vk::DeviceSize offset = frameBase + static_cast<vk::DeviceSize>(cursor) * m_slotSize;
	++cursor;

	memcpy(static_cast<uint8_t*>(m_mapped) + offset, data, size);
	// CpuToGpu memory is not guaranteed HOST_COHERENT; flushing here means no
	// call site can forget it.
	m_memoryManager->flushMemory(m_allocation, offset, static_cast<vk::DeviceSize>(size));

	return offset;
}

} // namespace graphics::vulkan
