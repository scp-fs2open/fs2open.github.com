#pragma once

#include "VulkanConstants.h"
#include "VulkanMemory.h"

#include <array>
#include <vulkan/vulkan.hpp>

namespace graphics::vulkan {

/**
 * @brief Per-frame-in-flight slotted uniform buffer ring
 *
 * One persistently-mapped VkBuffer holding MAX_FRAMES_IN_FLIGHT independent
 * regions of `slotsPerFrame` fixed-size slots. Writers call alloc() with the
 * current frame-in-flight index; the returned offset is bound as the UBO
 * descriptor offset. Because each in-flight frame writes only its own region,
 * the CPU can never overwrite a slot the GPU is still reading from the
 * previous frame (the same idea as the buffer manager's frame bump allocators
 * and the raytracing manager's FrameTlasResources).
 *
 * alloc() flushes the written range itself, so callers cannot forget the
 * flush on non-coherent memory.
 */
class PerFrameUboRing {
  public:
	PerFrameUboRing() = default;
	~PerFrameUboRing() = default;

	// Non-copyable (owns a VkBuffer)
	PerFrameUboRing(const PerFrameUboRing&) = delete;
	PerFrameUboRing& operator=(const PerFrameUboRing&) = delete;

	/**
	 * @brief Create the backing buffer (frames x slotsPerFrame x slotSize) and map it
	 * @param slotSize Must be >= minUniformBufferOffsetAlignment (256 is always safe)
	 * @return false on failure; the ring is unusable (isValid() == false)
	 */
	bool init(vk::Device device, VulkanMemoryManager* memoryManager, uint32_t slotsPerFrame,
		vk::DeviceSize slotSize);

	void shutdown();

	bool isValid() const { return m_buffer && m_mapped != nullptr; }

	/**
	 * @brief Reset the given frame's slot cursor (call once per frame at frame start)
	 *
	 * Only safe once the GPU work of the previous frame using this index has
	 * completed (i.e. after the frame fence wait) - the same contract as
	 * VulkanBufferManager::setCurrentFrame().
	 */
	void resetCursor(uint32_t frameIndex);

	/**
	 * @brief Copy `size` bytes into the next free slot of `frameIndex`'s region and flush it
	 * @return Byte offset within buffer() for descriptor binding
	 *
	 * Asserts on slot overflow and on size > slotSize.
	 */
	vk::DeviceSize alloc(uint32_t frameIndex, const void* data, size_t size);

	vk::Buffer buffer() const { return m_buffer; }
	vk::DeviceSize slotSize() const { return m_slotSize; }
	uint32_t slotsPerFrame() const { return m_slotsPerFrame; }

	/**
	 * @brief Slots consumed so far this frame (diagnostics)
	 */
	uint32_t cursor(uint32_t frameIndex) const { return m_cursors[frameIndex % MAX_FRAMES_IN_FLIGHT]; }

  private:
	vk::Device m_device;
	VulkanMemoryManager* m_memoryManager = nullptr;

	vk::Buffer m_buffer;
	VulkanAllocation m_allocation;
	void* m_mapped = nullptr;

	uint32_t m_slotsPerFrame = 0;
	vk::DeviceSize m_slotSize = 0;
	std::array<uint32_t, MAX_FRAMES_IN_FLIGHT> m_cursors = {};
};

} // namespace graphics::vulkan
