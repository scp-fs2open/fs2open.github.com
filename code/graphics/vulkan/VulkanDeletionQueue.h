
#pragma once

#include "globalincs/pstypes.h"
#include "VulkanConstants.h"
#include "VulkanMemory.h"

#include <vulkan/vulkan.hpp>
#include <functional>
#include <variant>

namespace graphics {
namespace vulkan {

/**
 * @brief Unified deferred resource deletion queue for Vulkan
 *
 * Resources that may still be referenced by in-flight command buffers are
 * queued here instead of being destroyed immediately. After waiting the
 * configured number of frames, they are safely destroyed.
 *
 * This prevents validation errors like "can't be called on VkImageView that
 * is currently in use by VkDescriptorSet".
 */
class VulkanDeletionQueue {
public:
	static constexpr uint32_t FRAMES_TO_WAIT = MAX_FRAMES_IN_FLIGHT;

	VulkanDeletionQueue() = default;
	~VulkanDeletionQueue();

	void init(vk::Device device, VulkanMemoryManager* memoryManager);
	void shutdown();

	/**
	 * @brief Queue a buffer for deferred destruction
	 */
	void queueBuffer(vk::Buffer buffer, VulkanAllocation allocation);

	/**
	 * @brief Queue an image for deferred destruction
	 */
	void queueImage(vk::Image image, VulkanAllocation allocation);

	/**
	 * @brief Queue an image view for deferred destruction
	 */
	void queueImageView(vk::ImageView imageView);

	/**
	 * @brief Queue a framebuffer for deferred destruction
	 */
	void queueFramebuffer(vk::Framebuffer framebuffer);

	/**
	 * @brief Queue a render pass for deferred destruction
	 */
	void queueRenderPass(vk::RenderPass renderPass);

	/**
	 * @brief Queue a sampler for deferred destruction
	 */
	void queueSampler(vk::Sampler sampler);

	/**
	 * @brief Process pending destructions - call once per frame
	 *
	 * Decrements frame counters and destroys resources that have waited
	 * enough frames.
	 */
	void processDestructions();

	/**
	 * @brief Flush all pending destructions immediately
	 *
	 * Used during shutdown when we know the device is idle.
	 */
	void flushAll();

private:
	struct PendingBuffer {
		vk::Buffer buffer;
		VulkanAllocation allocation;
	};

	struct PendingImage {
		vk::Image image;
		VulkanAllocation allocation;
	};

	using PendingResource = std::variant<
		PendingBuffer,
		PendingImage,
		vk::ImageView,
		vk::Framebuffer,
		vk::RenderPass,
		vk::Sampler
	>;

	struct PendingDestruction {
		PendingResource resource;
		uint32_t framesRemaining;
	};

	void destroyResource(const PendingResource& resource);

	vk::Device m_device;
	VulkanMemoryManager* m_memoryManager = nullptr;
	SCP_vector<PendingDestruction> m_pendingDestructions;
	bool m_initialized = false;
};

// Global deletion queue instance
VulkanDeletionQueue* getDeletionQueue();
void setDeletionQueue(VulkanDeletionQueue* queue);

} // namespace vulkan
} // namespace graphics
