#include "VulkanBarrier.h"


namespace graphics::vulkan {

void cmdImageBarriers(vk::CommandBuffer cmd, ArrayView<const ImageBarrier2> barriers)
{
	if (barriers.size == 0) {
		return;
	}

	SCP_vector<vk::ImageMemoryBarrier2> vkBarriers;
	vkBarriers.reserve(barriers.size);

	for (const auto& b : barriers) {
		vk::ImageMemoryBarrier2 barrier;
		barrier.srcStageMask = b.srcStage;
		barrier.srcAccessMask = b.srcAccess;
		barrier.dstStageMask = b.dstStage;
		barrier.dstAccessMask = b.dstAccess;
		barrier.oldLayout = b.oldLayout;
		barrier.newLayout = b.newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = b.image;
		barrier.subresourceRange.aspectMask = b.aspectMask;
		barrier.subresourceRange.baseMipLevel = b.baseMipLevel;
		barrier.subresourceRange.levelCount = b.levelCount;
		barrier.subresourceRange.baseArrayLayer = b.baseArrayLayer;
		barrier.subresourceRange.layerCount = b.layerCount;
		vkBarriers.push_back(barrier);
	}

	vk::DependencyInfo depInfo;
	depInfo.imageMemoryBarrierCount = static_cast<uint32_t>(vkBarriers.size());
	depInfo.pImageMemoryBarriers = vkBarriers.data();

	cmd.pipelineBarrier2KHR(depInfo);
}

void cmdImageBarrier(vk::CommandBuffer cmd, const ImageBarrier2& barrier)
{
	cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(&barrier, 1));
}

void cmdMemoryBarrier(vk::CommandBuffer cmd,
	vk::PipelineStageFlags2 srcStage, vk::AccessFlags2 srcAccess,
	vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess)
{
	vk::MemoryBarrier2 barrier;
	barrier.srcStageMask = srcStage;
	barrier.srcAccessMask = srcAccess;
	barrier.dstStageMask = dstStage;
	barrier.dstAccessMask = dstAccess;

	vk::DependencyInfo depInfo;
	depInfo.memoryBarrierCount = 1;
	depInfo.pMemoryBarriers = &barrier;

	cmd.pipelineBarrier2KHR(depInfo);
}

} // namespace graphics::vulkan
