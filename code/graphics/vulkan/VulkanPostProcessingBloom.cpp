#include "VulkanPostProcessing.h"

#include <array>

#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanDescriptorManager.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/post_processing.h"
#include "graphics/grinternal.h"
#include "graphics/2d.h"


namespace graphics::vulkan {


// ===== Bloom Pipeline Implementation =====

// Local UBO struct for blur shader (matches OpenGL's blur_data layout).
struct BlurUBOData {
	float texSize;
	int level;
	int pad[2];
};

bool VulkanBloom::init(PostProcessContext& ctx, const RenderTarget& sceneColor)
{
	m_ctx = &ctx;
	m_sceneColor = &sceneColor;

	m_width = m_ctx->sceneExtent.width / 2;
	m_height = m_ctx->sceneExtent.height / 2;

	const uint32_t mipLevels = MAX_MIP_BLUR_LEVELS;

	// Create 2 bloom textures (RGBA16F, half-res, 4 mip levels each)
	for (size_t i = 0; i < m_tex.size(); i++) {
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = HDR_COLOR_FORMAT;
		imageInfo.extent.width = m_width;
		imageInfo.extent.height = m_height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment
		                | vk::ImageUsageFlagBits::eSampled
		                | vk::ImageUsageFlagBits::eTransferSrc
		                | vk::ImageUsageFlagBits::eTransferDst;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_tex[i].image = m_ctx->device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanBloom: Failed to create bloom image %zu: %s\n", i, e.what()));
			return false;
		}

		if (!m_ctx->memoryManager->allocateImageMemory(m_tex[i].image, MemoryUsage::GpuOnly, m_tex[i].allocation)) {
			mprintf(("VulkanBloom: Failed to allocate bloom image %zu memory!\n", i));
			return false;
		}

		// Full image view (all mip levels, for textureLod sampling)
		vk::ImageViewCreateInfo fullViewInfo;
		fullViewInfo.image = m_tex[i].image;
		fullViewInfo.viewType = vk::ImageViewType::e2D;
		fullViewInfo.format = HDR_COLOR_FORMAT;
		fullViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		fullViewInfo.subresourceRange.baseMipLevel = 0;
		fullViewInfo.subresourceRange.levelCount = mipLevels;
		fullViewInfo.subresourceRange.baseArrayLayer = 0;
		fullViewInfo.subresourceRange.layerCount = 1;

		try {
			m_tex[i].fullView = m_ctx->device.createImageView(fullViewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanBloom: Failed to create bloom %zu full view: %s\n", i, e.what()));
			return false;
		}

		// Per-mip image views (for framebuffer attachment)
		for (uint32_t mip = 0; mip < mipLevels; mip++) {
			vk::ImageViewCreateInfo mipViewInfo = fullViewInfo;
			mipViewInfo.subresourceRange.baseMipLevel = mip;
			mipViewInfo.subresourceRange.levelCount = 1;

			try {
				m_tex[i].mipViews[mip] = m_ctx->device.createImageView(mipViewInfo);
			} catch (const vk::SystemError& e) {
				mprintf(("VulkanBloom: Failed to create bloom %zu mip %u view: %s\n", i, mip, e.what()));
				return false;
			}
		}
	}

	// Create bloom render pass (color-only RGBA16F, loadOp=eDontCare for overwriting)
	{
		vk::AttachmentDescription att;
		att.format = HDR_COLOR_FORMAT;
		att.samples = vk::SampleCountFlagBits::e1;
		att.loadOp = vk::AttachmentLoadOp::eDontCare;
		att.storeOp = vk::AttachmentStoreOp::eStore;
		att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		att.initialLayout = vk::ImageLayout::eUndefined;
		att.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::AttachmentReference colorRef;
		colorRef.attachment = 0;
		colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		vk::SubpassDependency dep;
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = vk::PipelineStageFlagBits::eFragmentShader
		                  | vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dep.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader
		                  | vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dep.srcAccessMask = vk::AccessFlagBits::eShaderRead
		                  | vk::AccessFlagBits::eColorAttachmentWrite;
		dep.dstAccessMask = vk::AccessFlagBits::eShaderRead
		                  | vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = 1;
		rpInfo.pAttachments = &att;
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dep;

		try {
			m_renderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanBloom: Failed to create bloom render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create bloom composite render pass (loadOp=eLoad for additive compositing onto scene color)
	{
		vk::AttachmentDescription att;
		att.format = HDR_COLOR_FORMAT;
		att.samples = vk::SampleCountFlagBits::e1;
		att.loadOp = vk::AttachmentLoadOp::eLoad;
		att.storeOp = vk::AttachmentStoreOp::eStore;
		att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		att.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
		att.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::AttachmentReference colorRef;
		colorRef.attachment = 0;
		colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		vk::SubpassDependency dep;
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = vk::PipelineStageFlagBits::eFragmentShader
		                  | vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dep.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader
		                  | vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dep.srcAccessMask = vk::AccessFlagBits::eShaderRead
		                  | vk::AccessFlagBits::eColorAttachmentWrite;
		dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
		                  | vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = 1;
		rpInfo.pAttachments = &att;
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dep;

		try {
			m_compositeRenderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanBloom: Failed to create bloom composite render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create per-mip framebuffers for bloom textures
	for (size_t i = 0; i < m_tex.size(); i++) {
		for (uint32_t mip = 0; mip < mipLevels; mip++) {
			uint32_t mipW = std::max(1u, m_width >> mip);
			uint32_t mipH = std::max(1u, m_height >> mip);

			vk::FramebufferCreateInfo fbInfo;
			fbInfo.renderPass = m_renderPass;
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = &m_tex[i].mipViews[mip];
			fbInfo.width = mipW;
			fbInfo.height = mipH;
			fbInfo.layers = 1;

			try {
				m_tex[i].mipFramebuffers[mip] = m_ctx->device.createFramebuffer(fbInfo);
			} catch (const vk::SystemError& e) {
				mprintf(("VulkanBloom: Failed to create bloom %zu mip %u framebuffer: %s\n", i, mip, e.what()));
				return false;
			}
		}
	}

	// Create scene color framebuffer for bloom composite (wraps the scene color as attachment)
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_compositeRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &m_sceneColor->view;
		fbInfo.width = m_ctx->sceneExtent.width;
		fbInfo.height = m_ctx->sceneExtent.height;
		fbInfo.layers = 1;

		try {
			m_sceneColorFB = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanBloom: Failed to create scene color bloom framebuffer: %s\n", e.what()));
			return false;
		}
	}

	m_initialized = true;
	mprintf(("VulkanBloom: Bloom initialized (%ux%u, %d mip levels)\n",
		m_width, m_height, MAX_MIP_BLUR_LEVELS));
	return true;
}

void VulkanBloom::shutdown()
{
	if (!m_initialized) {
		return;
	}

	if (m_sceneColorFB) {
		m_ctx->device.destroyFramebuffer(m_sceneColorFB);
		m_sceneColorFB = nullptr;
	}

	for (auto& bt : m_tex) {
		for (size_t mip = 0; mip < bt.mipFramebuffers.size(); mip++) {
			if (bt.mipFramebuffers[mip]) {
				m_ctx->device.destroyFramebuffer(bt.mipFramebuffers[mip]);
				bt.mipFramebuffers[mip] = nullptr;
			}
			if (bt.mipViews[mip]) {
				m_ctx->device.destroyImageView(bt.mipViews[mip]);
				bt.mipViews[mip] = nullptr;
			}
		}
		if (bt.fullView) {
			m_ctx->device.destroyImageView(bt.fullView);
			bt.fullView = nullptr;
		}
		if (bt.image) {
			m_ctx->device.destroyImage(bt.image);
			bt.image = nullptr;
		}
		if (bt.allocation.isValid()) {
			m_ctx->memoryManager->freeAllocation(bt.allocation);
		}
	}

	if (m_compositeRenderPass) {
		m_ctx->device.destroyRenderPass(m_compositeRenderPass);
		m_compositeRenderPass = nullptr;
	}
	if (m_renderPass) {
		m_ctx->device.destroyRenderPass(m_renderPass);
		m_renderPass = nullptr;
	}

	m_initialized = false;
}

void VulkanBloom::execute(vk::CommandBuffer cmd)
{
	if (!m_initialized || gr_bloom_intensity() <= 0) {
		return;
	}

	// Map shared scratch UBO for writing per-draw data
	m_ctx->scratchUBOMapped = m_ctx->memoryManager->mapMemory(m_ctx->scratchUBOAlloc);
	if (!m_ctx->scratchUBOMapped) {
		return;
	}
	m_ctx->scratchUBOCursor = 0;

	// 1. Bright pass: extract pixels brighter than 1.0 from scene color → bloom_tex[0] mip 0
	m_ctx->drawFullscreenTriangle(cmd, m_renderPass,
		m_tex[0].mipFramebuffers[0],
		vk::Extent2D(m_width, m_height),
		SDR_TYPE_POST_PROCESS_BRIGHTPASS,
		m_sceneColor->view, m_ctx->linearSampler,
		nullptr, 0,  // Brightpass has no UBO
		ALPHA_BLEND_NONE);

	// 2. Generate mipmaps for bloom_tex[0] (fill mips 1-3 from mip 0)
	PostProcessContext::generateMipmaps(cmd, m_tex[0].image, m_width, m_height, MAX_MIP_BLUR_LEVELS);

	// 3. Blur iterations (2 iterations of vertical + horizontal ping-pong)
	for (int iteration = 0; iteration < 2; iteration++) {
		for (int pass = 0; pass < 2; pass++) {
			// pass 0 = vertical (tex[0] → tex[1]), pass 1 = horizontal (tex[1] → tex[0])
			int srcIdx = pass;
			int dstIdx = 1 - pass;
			bool isVertical = (pass == 0);
			unsigned int blurFlags = isVertical ? SDR_FLAG_BLUR_VERTICAL : SDR_FLAG_BLUR_HORIZONTAL;

			for (int mip = 0; mip < MAX_MIP_BLUR_LEVELS; mip++) {
				uint32_t mipW = std::max(1u, m_width >> mip);
				uint32_t mipH = std::max(1u, m_height >> mip);

				BlurUBOData blurData;
				blurData.texSize = isVertical ? 1.0f / static_cast<float>(mipH)
				                              : 1.0f / static_cast<float>(mipW);
				blurData.level = mip;
				blurData.pad[0] = 0;
				blurData.pad[1] = 0;

				m_ctx->drawFullscreenTriangle(cmd, m_renderPass,
					m_tex[dstIdx].mipFramebuffers[mip],
					vk::Extent2D(mipW, mipH),
					SDR_TYPE_POST_PROCESS_BLUR,
					m_tex[srcIdx].fullView, m_ctx->mipmapSampler,
					&blurData, sizeof(blurData),
					ALPHA_BLEND_NONE,
					blurFlags);
			}
		}
	}

	// 4. Transition scene color for bloom composite (eShaderReadOnlyOptimal → eColorAttachmentOptimal)
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
		                      | vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_sceneColor->image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, {}, {}, barrier);
	}

	// 5. Bloom composite: additively blend blurred bloom onto scene color
	graphics::generic_data::bloom_composition_data compData;
	compData.bloom_intensity = gr_bloom_intensity() / 100.0f;
	compData.levels = MAX_MIP_BLUR_LEVELS;
	compData.pad[0] = 0.0f;
	compData.pad[1] = 0.0f;

	m_ctx->drawFullscreenTriangle(cmd, m_compositeRenderPass,
		m_sceneColorFB,
		m_ctx->sceneExtent,
		SDR_TYPE_POST_PROCESS_BLOOM_COMP,
		m_tex[0].fullView, m_ctx->mipmapSampler,
		&compData, sizeof(compData),
		ALPHA_BLEND_ADDITIVE);

	// Scene color is now in eShaderReadOnlyOptimal (from bloom composite render pass finalLayout)

	// Unmap shared scratch UBO
	m_ctx->memoryManager->unmapMemory(m_ctx->scratchUBOAlloc);
	m_ctx->scratchUBOMapped = nullptr;
}

} // namespace graphics::vulkan
