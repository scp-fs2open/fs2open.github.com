#include "VulkanPostProcessing.h"

#include <array>

#include "gr_vulkan.h"
#include "VulkanBarrier.h"
#include "VulkanRenderer.h"
#include "VulkanDescriptorManager.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/post_processing.h"
#include "graphics/grinternal.h"
#include "graphics/2d.h"


namespace graphics::vulkan {


// ===== Bloom Pipeline Implementation =====

bool VulkanBloom::init(PostProcessContext& ctx, const RenderTarget& sceneColor)
{
	m_ctx = &ctx;
	m_sceneColor = &sceneColor;

	// Create the two mip render passes. They share the per-mip framebuffers,
	// thus they must stay compatible: the attachment format and the subpass
	// dependency must be identical, and only the load operation and the layouts
	// may differ.
	{
		auto createMipRenderPass = [this](vk::AttachmentLoadOp loadOp, vk::ImageLayout initialLayout,
		                                  vk::RenderPass& outPass, const char* name) {
			vk::AttachmentDescription att;
			att.format = HDR_COLOR_FORMAT;
			att.samples = vk::SampleCountFlagBits::e1;
			att.loadOp = loadOp;
			att.storeOp = vk::AttachmentStoreOp::eStore;
			att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			att.initialLayout = initialLayout;
			att.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			vk::AttachmentReference colorRef;
			colorRef.attachment = 0;
			colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

			vk::SubpassDescription subpass;
			subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorRef;

			// A pass writes one mip of the bloom image while it samples another
			// mip of the same image. The dependency must thus cover both the
			// write-then-sample and the sample-then-write order. The eLoad pass
			// also reads its own attachment, thus eColorAttachmentRead is in the
			// destination scope of both passes to keep them compatible.
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
			                  | vk::AccessFlagBits::eColorAttachmentRead
			                  | vk::AccessFlagBits::eColorAttachmentWrite;

			vk::RenderPassCreateInfo rpInfo;
			rpInfo.attachmentCount = 1;
			rpInfo.pAttachments = &att;
			rpInfo.subpassCount = 1;
			rpInfo.pSubpasses = &subpass;
			rpInfo.dependencyCount = 1;
			rpInfo.pDependencies = &dep;

			try {
				outPass = m_ctx->device.createRenderPass(rpInfo);
			} catch (const vk::SystemError& e) {
				nprintf(("vulkan", "VulkanBloom: Failed to create bloom %s render pass: %s\n", name, e.what()));
				return false;
			}
			return true;
		};

		// The downsample overwrites the whole mip, thus it does not load it.
		if (!createMipRenderPass(vk::AttachmentLoadOp::eDontCare, vk::ImageLayout::eUndefined,
		                         m_renderPass, "downsample")) {
			return false;
		}

		// The upsample adds to the downsampled content of the mip, thus it must
		// load it. The downsample pass left the mip in eShaderReadOnlyOptimal.
		if (!createMipRenderPass(vk::AttachmentLoadOp::eLoad, vk::ImageLayout::eShaderReadOnlyOptimal,
		                         m_upsampleRenderPass, "upsample")) {
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
			nprintf(("vulkan", "VulkanBloom: Failed to create bloom composite render pass: %s\n", e.what()));
			return false;
		}
	}

	if (!createTargets()) {
		return false;
	}

	m_initialized = true;
	nprintf(("vulkan", "VulkanBloom: Bloom initialized (%ux%u, %d mip levels)\n",
		m_width, m_height, m_mipCount));
	return true;
}

bool VulkanBloom::createTargets()
{
	m_width = std::max(1u, m_ctx->sceneExtent.width / 2);
	m_height = std::max(1u, m_ctx->sceneExtent.height / 2);
	m_mipCount = gr_bloom_mip_levels(static_cast<int>(m_width), static_cast<int>(m_height));

	const uint32_t mipLevels = static_cast<uint32_t>(m_mipCount);

	// Create the bloom image (RGBA16F, half-res, one mip per bloom layer)
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
		m_image = m_ctx->device.createImage(imageInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanBloom: Failed to create bloom image: %s\n", e.what()));
		return false;
	}

	if (!m_ctx->memoryManager->allocateImageMemory(m_image, MemoryUsage::GpuOnly, m_allocation)) {
		nprintf(("vulkan", "VulkanBloom: Failed to allocate bloom image memory!\n"));
		return false;
	}

	// One view per mip. A pass writes mip N while it samples mip N-1 or N+1, so
	// the mips are in different layouts and a view of the whole chain is not
	// usable as a descriptor.
	for (uint32_t mip = 0; mip < mipLevels; mip++) {
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = HDR_COLOR_FORMAT;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = mip;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		try {
			m_mipViews[mip] = m_ctx->device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanBloom: Failed to create bloom mip %u view: %s\n", mip, e.what()));
			return false;
		}
	}

	// One framebuffer per mip. The downsample, upsample, and composite render
	// passes all use one RGBA16F color attachment, thus they are compatible and
	// share these framebuffers.
	for (uint32_t mip = 0; mip < mipLevels; mip++) {
		uint32_t mipW = std::max(1u, m_width >> mip);
		uint32_t mipH = std::max(1u, m_height >> mip);

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_renderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &m_mipViews[mip];
		fbInfo.width = mipW;
		fbInfo.height = mipH;
		fbInfo.layers = 1;

		try {
			m_mipFramebuffers[mip] = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanBloom: Failed to create bloom mip %u framebuffer: %s\n", mip, e.what()));
			return false;
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
			nprintf(("vulkan", "VulkanBloom: Failed to create scene color bloom framebuffer: %s\n", e.what()));
			return false;
		}
	}

	return true;
}

void VulkanBloom::destroyTargets()
{
	if (m_sceneColorFB) {
		m_ctx->device.destroyFramebuffer(m_sceneColorFB);
		m_sceneColorFB = nullptr;
	}

	for (size_t mip = 0; mip < m_mipFramebuffers.size(); mip++) {
		if (m_mipFramebuffers[mip]) {
			m_ctx->device.destroyFramebuffer(m_mipFramebuffers[mip]);
			m_mipFramebuffers[mip] = nullptr;
		}
		if (m_mipViews[mip]) {
			m_ctx->device.destroyImageView(m_mipViews[mip]);
			m_mipViews[mip] = nullptr;
		}
	}

	if (m_image) {
		m_ctx->device.destroyImage(m_image);
		m_image = nullptr;
	}
	if (m_allocation.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_allocation);
		m_allocation = {};
	}

	m_mipCount = 0;
}

bool VulkanBloom::resize()
{
	if (!m_initialized) {
		return true;
	}
	destroyTargets();
	return createTargets();
}

void VulkanBloom::shutdown()
{
	if (!m_initialized) {
		return;
	}

	destroyTargets();

	if (m_compositeRenderPass) {
		m_ctx->device.destroyRenderPass(m_compositeRenderPass);
		m_compositeRenderPass = nullptr;
	}
	if (m_upsampleRenderPass) {
		m_ctx->device.destroyRenderPass(m_upsampleRenderPass);
		m_upsampleRenderPass = nullptr;
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

	GR_DEBUG_SCOPE("Bloom");

	// Per-draw UBO slots come from the persistently-mapped scratch ring
	// (cursor reset once per frame in VulkanPostProcessor::beginFrame()).

	// The downsample filter keeps the total brightness of the image, it only
	// makes the image blurry. Thus the composite pass must divide by the sum of
	// all the layer contributions to keep the total brightness of the bloom.
	float totalBloomContrib = 0.0f;

	// 1. Downsample: the scene color into mip 0, then each mip into the next.
	//    Each pass leaves its mip in eShaderReadOnlyOptimal, which is the layout
	//    that the next pass needs to sample it.
	{
		GR_DEBUG_SCOPE("Bloom Downsampling");

		for (int mip = 0; mip < m_mipCount; mip++) {
			uint32_t mipW = std::max(1u, m_width >> mip);
			uint32_t mipH = std::max(1u, m_height >> mip);

			// The first bloom layer downsamples the scene texture. Every other
			// layer downsamples the previous mip of the bloom image.
			vk::ImageView srcView = (mip == 0) ? m_sceneColor->view : m_mipViews[mip - 1];

			graphics::generic_data::bloom_sample_data sampleData;
			// 0.5f, because the source image is twice the target size.
			sampleData.xSize = 0.5f / static_cast<float>(mipW);
			sampleData.ySize = 0.5f / static_cast<float>(mipH);
			// Every source is a single-mip view, thus the shader reads level 0.
			sampleData.mip = 0.0f;
			sampleData.intensity = 1.0f;

			m_ctx->drawFullscreenTriangle(cmd, m_renderPass,
				m_mipFramebuffers[mip],
				vk::Extent2D(mipW, mipH),
				SDR_TYPE_POST_PROCESS_BLOOM_DOWNSAMPLE,
				srcView, m_ctx->linearSampler,
				&sampleData, sizeof(sampleData),
				ALPHA_BLEND_NONE);
		}
	}

	// 2. Upsample: blur each mip and add it to the mip above it. The bloom
	//    approximates a "spiky" distribution with a set of gaussians of
	//    different widths.
	{
		GR_DEBUG_SCOPE("Bloom Upsampling");

		for (int mip = m_mipCount - 2; mip >= 0; mip--) {
			uint32_t mipW = std::max(1u, m_width >> mip);
			uint32_t mipH = std::max(1u, m_height >> mip);

			const float layerContrib = gr_bloom_layer_contribution(mip);
			totalBloomContrib += layerContrib;

			graphics::generic_data::bloom_sample_data sampleData;
			sampleData.xSize = 1.0f / static_cast<float>(mipW);
			sampleData.ySize = 1.0f / static_cast<float>(mipH);
			// The source is a single-mip view of the mip below, thus level 0.
			sampleData.mip = 0.0f;
			sampleData.intensity = layerContrib;

			m_ctx->drawFullscreenTriangle(cmd, m_upsampleRenderPass,
				m_mipFramebuffers[mip],
				vk::Extent2D(mipW, mipH),
				SDR_TYPE_POST_PROCESS_BLOOM_UPSAMPLE,
				m_mipViews[mip + 1], m_ctx->linearSampler,
				&sampleData, sizeof(sampleData),
				ALPHA_BLEND_ADDITIVE);
		}
	}

	// 3. Transition scene color for bloom composite (eShaderReadOnlyOptimal → eColorAttachmentOptimal)
	{
		ImageBarrier2 barrier;
		barrier.image = m_sceneColor->image;
		barrier.levelCount = 1;
		barrier.layerCount = 1;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barrier.srcAccess = vk::AccessFlagBits2::eShaderSampledRead;
		barrier.dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barrier.dstAccess = vk::AccessFlagBits2::eColorAttachmentRead
		                  | vk::AccessFlagBits2::eColorAttachmentWrite;

		cmdImageBarrier(cmd, barrier);
	}

	// 4. Composite: blend the finished bloom (mip 0) onto the scene color.
	{
		GR_DEBUG_SCOPE("Bloom composite step");

		graphics::generic_data::bloom_composition_data compData;
		compData.xSize = 1.0f / static_cast<float>(m_width);
		compData.ySize = 1.0f / static_cast<float>(m_height);
		compData.normalization = (totalBloomContrib > 0.0f) ? 1.0f / totalBloomContrib : 0.0f;
		compData.bloom_intensity = gr_bloom_intensity() / 100.0f;

		// A blend between the bloom and the rendered image is more physically
		// accurate than an add. Light that optical flaws or dirt on the lens
		// scatter does not stay in the unscattered image.
		m_ctx->drawFullscreenTriangle(cmd, m_compositeRenderPass,
			m_sceneColorFB,
			m_ctx->sceneExtent,
			SDR_TYPE_POST_PROCESS_BLOOM_COMP,
			m_mipViews[0], m_ctx->linearSampler,
			&compData, sizeof(compData),
			ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	}

	// Scene color is now in eShaderReadOnlyOptimal (from bloom composite render pass finalLayout)
}

} // namespace graphics::vulkan
