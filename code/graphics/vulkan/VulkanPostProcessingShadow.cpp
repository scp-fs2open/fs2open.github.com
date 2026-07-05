#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanRenderer.h"
#include "VulkanDescriptorManager.h"
#include "graphics/shadows.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "mod_table/mod_table.h"
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

	const auto layers = static_cast<uint32_t>(Num_shadow_cascades + Num_cockpit_shadow_cascades);

	nprintf(("vulkan", "VulkanPostProcessor: Creating %dx%d shadow map (%d cascades)\n", size, size, layers));

	// Create shadow depth image (D32F, 2D array, dynamic cascade count layers).
	// Depth-only (no VSM color target): sampled directly with a depth-compare
	// sampler (sampler2DArrayShadow), matching the hardware PCF approach used
	// by the OpenGL backend.
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = SHADOW_DEPTH_FORMAT;
		imageInfo.extent = vk::Extent3D(static_cast<uint32_t>(size), static_cast<uint32_t>(size), 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = layers;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_depth.image = m_ctx->device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create shadow depth image: %s\n", e.what()));
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
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create shadow depth view: %s\n", e.what()));
			return false;
		}

		m_depth.format = SHADOW_DEPTH_FORMAT;
		m_depth.width = static_cast<uint32_t>(size);
		m_depth.height = static_cast<uint32_t>(size);
	}

	// Create shadow render pass: depth-only (D32F), eClear
	{
		std::array<vk::AttachmentDescription, 1> attachments;

		// Depth attachment (D32F) — sampled afterwards via a depth-compare sampler
		attachments[0].format = SHADOW_DEPTH_FORMAT;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::AttachmentReference depthRef;
		depthRef.attachment = 0;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 0;
		subpass.pColorAttachments = nullptr;
		subpass.pDepthStencilAttachment = &depthRef;

		vk::SubpassDependency dep;
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dep.dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dep.srcAccessMask = {};
		dep.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

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
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create shadow render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create layered framebuffer (all cascade layers at once, depth-only)
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_renderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &m_depth.view;
		fbInfo.width = static_cast<uint32_t>(size);
		fbInfo.height = static_cast<uint32_t>(size);
		fbInfo.layers = layers;

		try {
			m_framebuffer = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create shadow framebuffer: %s\n", e.what()));
			return false;
		}
	}

	// Depth-compare sampler: lets shaders declare this binding as
	// sampler2DArrayShadow and get hardware PCF (matches GL_TEXTURE_COMPARE_MODE
	// = GL_COMPARE_REF_TO_TEXTURE on the OpenGL side).
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.compareEnable = VK_TRUE;
		samplerInfo.compareOp = vk::CompareOp::eLessOrEqual;
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;

		try {
			m_compareSampler = m_ctx->device.createSampler(samplerInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create shadow compare sampler: %s\n", e.what()));
			return false;
		}
	}

	m_textureSize = size;
	m_initialized = true;
	nprintf(("vulkan", "VulkanPostProcessor: Shadow map initialized (%dx%d, %d cascades)\n", size, size, layers));
	return true;
}

void VulkanShadowMap::shutdown()
{
	if (!m_initialized) {
		return;
	}

	if (m_compareSampler) {
		m_ctx->device.destroySampler(m_compareSampler);
		m_compareSampler = nullptr;
	}
	if (m_framebuffer) {
		m_ctx->device.destroyFramebuffer(m_framebuffer);
		m_framebuffer = nullptr;
	}
	if (m_renderPass) {
		m_ctx->device.destroyRenderPass(m_renderPass);
		m_renderPass = nullptr;
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
