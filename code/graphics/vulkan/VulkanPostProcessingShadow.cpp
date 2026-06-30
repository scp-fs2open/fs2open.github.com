#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanRenderer.h"
#include "VulkanDescriptorManager.h"
#include "graphics/shadows.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "nebula/neb.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {


// ===== Shadow Map Implementation =====

bool VulkanShadowMap::init(PostProcessContext& ctx)
{
	m_ctx = &ctx;

	if (m_initialized) {
		return true;
	}

	if (Shadow_quality == ShadowQuality::Disabled) {
		return false;
	}

	int size;
	switch (Shadow_quality) {
	case ShadowQuality::Low:    size = 512; break;
	case ShadowQuality::Medium: size = 1024; break;
	case ShadowQuality::High:   size = 2048; break;
	case ShadowQuality::Ultra:  size = 4096; break;
	default:                    size = 512; break;
	}

	mprintf(("VulkanPostProcessor: Creating %dx%d shadow map (%d cascades)\n", size, size, MAX_SHADOW_CASCADES));

	const uint32_t layers = MAX_SHADOW_CASCADES;

	// Create shadow color image (RGBA16F, 2D array, MAX_SHADOW_CASCADES layers)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = HDR_COLOR_FORMAT;
		imageInfo.extent = vk::Extent3D(static_cast<uint32_t>(size), static_cast<uint32_t>(size), 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = layers;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_color.image = m_ctx->device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow color image: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateImageMemory(m_color.image, MemoryUsage::GpuOnly, m_color.allocation)) {
			m_ctx->device.destroyImage(m_color.image);
			m_color.image = nullptr;
			return false;
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_color.image;
		viewInfo.viewType = vk::ImageViewType::e2DArray;
		viewInfo.format = HDR_COLOR_FORMAT;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layers;

		try {
			m_color.view = m_ctx->device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow color view: %s\n", e.what()));
			return false;
		}

		m_color.format = HDR_COLOR_FORMAT;
		m_color.width = static_cast<uint32_t>(size);
		m_color.height = static_cast<uint32_t>(size);
	}

	// Create shadow depth image (D32F, 2D array, MAX_SHADOW_CASCADES layers)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = SHADOW_DEPTH_FORMAT;
		imageInfo.extent = vk::Extent3D(static_cast<uint32_t>(size), static_cast<uint32_t>(size), 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = layers;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_depth.image = m_ctx->device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow depth image: %s\n", e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateImageMemory(m_depth.image, MemoryUsage::GpuOnly, m_depth.allocation)) {
			m_ctx->device.destroyImage(m_depth.image);
			m_depth.image = nullptr;
			return false;
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_depth.image;
		viewInfo.viewType = vk::ImageViewType::e2DArray;
		viewInfo.format = SHADOW_DEPTH_FORMAT;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layers;

		try {
			m_depth.view = m_ctx->device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow depth view: %s\n", e.what()));
			return false;
		}

		m_depth.format = SHADOW_DEPTH_FORMAT;
		m_depth.width = static_cast<uint32_t>(size);
		m_depth.height = static_cast<uint32_t>(size);
	}

	// Create shadow render pass: 1 color (RGBA16F) + 1 depth (D32F), both eClear
	{
		std::array<vk::AttachmentDescription, 2> attachments;

		// Color attachment (RGBA16F) — stores VSM depth variance
		attachments[0].format = HDR_COLOR_FORMAT;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		// Depth attachment (D32F)
		attachments[1].format = SHADOW_DEPTH_FORMAT;
		attachments[1].samples = vk::SampleCountFlagBits::e1;
		attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
		attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[1].initialLayout = vk::ImageLayout::eUndefined;
		attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference colorRef;
		colorRef.attachment = 0;
		colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentReference depthRef;
		depthRef.attachment = 1;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.pDepthStencilAttachment = &depthRef;

		vk::SubpassDependency dep;
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dep.srcAccessMask = {};
		dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dep;

		try {
			m_renderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create layered framebuffer (all MAX_SHADOW_CASCADES layers at once)
	{
		std::array<vk::ImageView, 2> fbAttachments = {
			m_color.view,
			m_depth.view,
		};

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_renderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		fbInfo.pAttachments = fbAttachments.data();
		fbInfo.width = static_cast<uint32_t>(size);
		fbInfo.height = static_cast<uint32_t>(size);
		fbInfo.layers = layers;

		try {
			m_framebuffer = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow framebuffer: %s\n", e.what()));
			return false;
		}
	}

	m_textureSize = size;
	m_initialized = true;
	mprintf(("VulkanPostProcessor: Shadow map initialized (%dx%d, %d cascades)\n", size, size, MAX_SHADOW_CASCADES));
	return true;
}

void VulkanShadowMap::shutdown()
{
	if (!m_initialized) {
		return;
	}

	if (m_framebuffer) {
		m_ctx->device.destroyFramebuffer(m_framebuffer);
		m_framebuffer = nullptr;
	}
	if (m_renderPass) {
		m_ctx->device.destroyRenderPass(m_renderPass);
		m_renderPass = nullptr;
	}

	if (m_color.view) {
		m_ctx->device.destroyImageView(m_color.view);
		m_color.view = nullptr;
	}
	if (m_color.image) {
		m_ctx->device.destroyImage(m_color.image);
		m_color.image = nullptr;
	}
	if (m_color.allocation.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_color.allocation);
	}

	if (m_depth.view) {
		m_ctx->device.destroyImageView(m_depth.view);
		m_depth.view = nullptr;
	}
	if (m_depth.image) {
		m_ctx->device.destroyImage(m_depth.image);
		m_depth.image = nullptr;
	}
	if (m_depth.allocation.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_depth.allocation);
	}

	m_textureSize = 0;
	m_initialized = false;
}

} // namespace graphics::vulkan
