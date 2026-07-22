#pragma once

#include "VulkanConstants.h"

#include <vulkan/vulkan.hpp>


namespace graphics::vulkan {

/**
 * @brief Description of a single image layout transition / access barrier
 *
 * Always expressed in VK_KHR_synchronization2 terms (64-bit stage/access
 * flags), which this renderer requires (see RequiredDeviceExtensions in
 * VulkanRendererSetup.cpp) -- there is no legacy vkCmdPipelineBarrier path.
 */
struct ImageBarrier2 {
	vk::Image image;
	vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
	uint32_t baseMipLevel = 0;
	uint32_t levelCount = 1;
	uint32_t baseArrayLayer = 0;
	uint32_t layerCount = 1;
	vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
	vk::ImageLayout newLayout = vk::ImageLayout::eUndefined;
	vk::PipelineStageFlags2 srcStage;
	vk::AccessFlags2 srcAccess;
	vk::PipelineStageFlags2 dstStage;
	vk::AccessFlags2 dstAccess;
};

/**
 * @brief Record one or more image barriers via vkCmdPipelineBarrier2KHR
 *
 * Note: ArrayView has no implicit conversion from SCP_vector<T> -- callers
 * building a dynamically-sized barrier list must construct
 * ArrayView(vec.data(), vec.size()) explicitly.
 */
void cmdImageBarriers(vk::CommandBuffer cmd, ArrayView<const ImageBarrier2> barriers);

/**
 * @brief Convenience wrapper for a single image barrier
 */
void cmdImageBarrier(vk::CommandBuffer cmd, const ImageBarrier2& barrier);

/**
 * @brief Record a global memory barrier (no image/buffer involved) via vkCmdPipelineBarrier2KHR
 */
void cmdMemoryBarrier(vk::CommandBuffer cmd,
	vk::PipelineStageFlags2 srcStage, vk::AccessFlags2 srcAccess,
	vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess);

} // namespace graphics::vulkan
