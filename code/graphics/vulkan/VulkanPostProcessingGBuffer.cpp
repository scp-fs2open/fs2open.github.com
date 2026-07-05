#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanRenderer.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {


// ===== G-Buffer (Deferred Lighting) Implementation =====

vk::RenderPass VulkanDeferredGBuffer::createGbufRenderPass(const GbufRenderPassConfig& config)
{
	// All G-buffer variants share formats; without composite only the first 5 are used
	static constexpr std::array<vk::Format, GBUF_COLOR_ATTACHMENT_COUNT> COLOR_FORMATS = {{
		GBUF_FORMAT_COLOR,
		GBUF_FORMAT_POSITION,
		GBUF_FORMAT_NORMAL,
		GBUF_FORMAT_SPECULAR,
		GBUF_FORMAT_EMISSIVE,
		GBUF_FORMAT_COMPOSITE,
	}};

	const uint32_t colorCount = config.includeComposite
		? GBUF_COLOR_ATTACHMENT_COUNT : MSAA_COLOR_ATTACHMENT_COUNT;
	const uint32_t depthIndex = colorCount;
	const uint32_t totalAttachments = colorCount + 1;

	// Max 6 color + 1 depth = 7 attachments
	std::array<vk::AttachmentDescription, 7> attachments;
		for (uint32_t i = 0; i < colorCount; ++i) {
		attachments[i].format = COLOR_FORMATS[i];
		attachments[i].samples = config.samples;
		auto opIt = config.attachmentLoadOpOverrides.find(i);
		attachments[i].loadOp = (opIt != config.attachmentLoadOpOverrides.end()) ? opIt->second : config.colorLoadOp;
		attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[i].initialLayout = config.colorInitialLayout;
		attachments[i].finalLayout = config.colorFinalLayout;
	}

	// Depth — stencil ops mirror the depth loadOp
	attachments[depthIndex].format = m_ctx->depthFormat;
	attachments[depthIndex].samples = config.samples;
	auto depthOpIt = config.attachmentLoadOpOverrides.find(depthIndex);
	attachments[depthIndex].loadOp = (depthOpIt != config.attachmentLoadOpOverrides.end()) ? depthOpIt->second : config.depthLoadOp;
	attachments[depthIndex].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[depthIndex].stencilLoadOp = config.depthLoadOp;
	attachments[depthIndex].stencilStoreOp =
		(config.depthLoadOp == vk::AttachmentLoadOp::eDontCare)
			? vk::AttachmentStoreOp::eDontCare
			: vk::AttachmentStoreOp::eStore;
	attachments[depthIndex].initialLayout = config.depthInitialLayout;
	attachments[depthIndex].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::array<vk::AttachmentReference, 6> colorRefs;
	for (uint32_t i = 0; i < colorCount; ++i) {
		colorRefs[i].attachment = i;
		colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
	}

	vk::AttachmentReference depthRef;
	depthRef.attachment = depthIndex;
	depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = colorCount;
	subpass.pColorAttachments = colorRefs.data();
	subpass.pDepthStencilAttachment = &depthRef;

	vk::SubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	if (config.useResolveDependency) {
		// MSAA resolve: previous pass read these textures as shader inputs
		dependency.srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	} else {
		// Standard G-buffer: previous pass may have done transfers (copies)
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests
		                        | vk::PipelineStageFlagBits::eTransfer;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eTransferRead
		                         | vk::AccessFlagBits::eTransferWrite;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentRead;
	}

	vk::RenderPassCreateInfo rpInfo;
	rpInfo.attachmentCount = totalAttachments;
	rpInfo.pAttachments = attachments.data();
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = 1;
	rpInfo.pDependencies = &dependency;

	return m_ctx->device.createRenderPass(rpInfo);
}

vk::Framebuffer VulkanDeferredGBuffer::createGbufFramebuffer(
	vk::RenderPass renderPass, bool includeComposite, bool useMsaaImages)
{
	// Attachment order: color, position, normal, specular, emissive, [composite], depth
	std::array<vk::ImageView, 7> views;
	uint32_t count = 0;

	if (useMsaaImages) {
		views[count++] = m_msaaColor.view;
		views[count++] = m_msaaPosition.view;
		views[count++] = m_msaaNormal.view;
		views[count++] = m_msaaSpecular.view;
		views[count++] = m_msaaEmissive.view;
	} else {
		views[count++] = m_sceneColor->view;
		views[count++] = m_gbufPosition.view;
		views[count++] = m_gbufNormal.view;
		views[count++] = m_gbufSpecular.view;
		views[count++] = m_gbufEmissive.view;
	}

	if (includeComposite) {
		views[count++] = m_gbufComposite.view;
	}

	views[count++] = useMsaaImages ? m_msaaDepthView : m_sceneDepth->view;

	vk::FramebufferCreateInfo fbInfo;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = count;
	fbInfo.pAttachments = views.data();
	fbInfo.width = m_ctx->sceneExtent.width;
	fbInfo.height = m_ctx->sceneExtent.height;
	fbInfo.layers = 1;

	return m_ctx->device.createFramebuffer(fbInfo);
}

bool VulkanDeferredGBuffer::init(PostProcessContext& ctx, const RenderTarget& sceneColor,
                                 const RenderTarget& sceneDepth)
{
	m_ctx = &ctx;
	m_sceneColor = &sceneColor;
	m_sceneDepth = &sceneDepth;

	if (m_gbufInitialized) {
		return true;
	}

	const uint32_t w = m_ctx->sceneExtent.width;
	const uint32_t h = m_ctx->sceneExtent.height;
	const vk::ImageUsageFlags gbufUsage =
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
		| vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

	// Create G-buffer images (position, normal, specular, emissive, composite)
	struct GbufTarget {
		RenderTarget* target;
		vk::Format format;
		const char* name;
	};

	std::array<GbufTarget, 5> targets = {{
		{&m_gbufPosition,  GBUF_FORMAT_POSITION,  "position"},
		{&m_gbufNormal,    GBUF_FORMAT_NORMAL,    "normal"},
		{&m_gbufSpecular,  GBUF_FORMAT_SPECULAR,  "specular"},
		{&m_gbufEmissive,  GBUF_FORMAT_EMISSIVE,  "emissive"},
		{&m_gbufComposite, GBUF_FORMAT_COMPOSITE, "composite"},
	}};

	for (auto& t : targets) {
		if (!m_ctx->createImage(w, h, t.format, gbufUsage, vk::ImageAspectFlagBits::eColor,
		                 t.target->image, t.target->view, t.target->allocation)) {
			mprintf(("VulkanPostProcessor: Failed to create G-buffer %s image!\n", t.name));
			shutdown();
			return false;
		}
		t.target->format = t.format;
		t.target->width = w;
		t.target->height = h;
	}

	// Create samplable copy of G-buffer normal (for decal angle rejection)
	{
		vk::ImageUsageFlags copyUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		if (!m_ctx->createImage(w, h, GBUF_FORMAT_NORMAL, copyUsage,
		                 vk::ImageAspectFlagBits::eColor,
		                 m_gbufNormalCopy.image, m_gbufNormalCopy.view, m_gbufNormalCopy.allocation)) {
			mprintf(("VulkanPostProcessor: Failed to create G-buffer normal copy!\n"));
			shutdown();
			return false;
		}
		m_gbufNormalCopy.format = GBUF_FORMAT_NORMAL;
		m_gbufNormalCopy.width = w;
		m_gbufNormalCopy.height = h;
	}

	// Create G-buffer render pass (eClear) — 6 color + depth
	try {
		m_gbufRenderPass = createGbufRenderPass({
			true, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear, vk::AttachmentLoadOp::eClear,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eUndefined,
		});
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create G-buffer render pass: %s\n", e.what()));
		shutdown();
		return false;
	}

	// Create G-buffer render pass (eLoad) — for resuming after mid-pass copies
	try {
		m_gbufRenderPassLoad = createGbufRenderPass({
			true, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eLoad, vk::AttachmentLoadOp::eLoad,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
		});
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create G-buffer load render pass: %s\n", e.what()));
		shutdown();
		return false;
	}

	// Create G-buffer framebuffer (6 color + depth)
	try {
		m_gbufFramebuffer = createGbufFramebuffer(m_gbufRenderPass, true, false);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create G-buffer framebuffer: %s\n", e.what()));
		shutdown();
		return false;
	}

	m_gbufInitialized = true;
	mprintf(("VulkanPostProcessor: G-buffer initialized (%ux%u, 6 color + depth)\n", w, h));
	return true;
}

void VulkanDeferredGBuffer::shutdown()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}

	if (m_gbufFramebuffer) {
		m_ctx->device.destroyFramebuffer(m_gbufFramebuffer);
		m_gbufFramebuffer = nullptr;
	}
	if (m_gbufRenderPassLoad) {
		m_ctx->device.destroyRenderPass(m_gbufRenderPassLoad);
		m_gbufRenderPassLoad = nullptr;
	}
	if (m_gbufRenderPass) {
		m_ctx->device.destroyRenderPass(m_gbufRenderPass);
		m_gbufRenderPass = nullptr;
	}

	std::array<RenderTarget*, 6> gbufTargets = {
		&m_gbufPosition, &m_gbufNormal, &m_gbufSpecular,
		&m_gbufEmissive, &m_gbufComposite, &m_gbufNormalCopy,
	};
	for (auto* rt : gbufTargets) {
		if (rt->view) {
			m_ctx->device.destroyImageView(rt->view);
			rt->view = nullptr;
		}
		if (rt->image) {
			m_ctx->device.destroyImage(rt->image);
			rt->image = nullptr;
		}
		if (rt->allocation.isValid()) {
			m_ctx->memoryManager->freeAllocation(rt->allocation);
		}
	}

	m_gbufInitialized = false;
}

void VulkanDeferredGBuffer::transitionForResume(vk::CommandBuffer cmd)
{
	if (!m_gbufInitialized) {
		return;
	}

	// After ending the G-buffer render pass, color attachments 1-5 are in
	// eShaderReadOnlyOptimal (from finalLayout). The eLoad pass expects
	// eColorAttachmentOptimal. Transition them in a single barrier batch.
	std::array<vk::Image, 5> gbufImages = {
		m_gbufPosition.image,
		m_gbufNormal.image,
		m_gbufSpecular.image,
		m_gbufEmissive.image,
		m_gbufComposite.image,
	};

	std::array<vk::ImageMemoryBarrier, 5> barriers;
	for (size_t i = 0; i < gbufImages.size(); ++i) {
		barriers[i].srcAccessMask = {};
		barriers[i].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barriers[i].oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[i].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[i].image = gbufImages[i];
		barriers[i].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barriers[i].subresourceRange.baseMipLevel = 0;
		barriers[i].subresourceRange.levelCount = 1;
		barriers[i].subresourceRange.baseArrayLayer = 0;
		barriers[i].subresourceRange.layerCount = 1;
	}

	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		{}, nullptr, nullptr, barriers);
}

void VulkanDeferredGBuffer::copyNormal(vk::CommandBuffer cmd)
{
	// Called mid-scene, outside a render pass.
	// Copies G-buffer normal → normal copy so decal shader can sample it for angle rejection.
	// G-buffer normal is in eShaderReadOnlyOptimal (from the ended G-buffer render pass).
	// Normal goes back to eShaderReadOnlyOptimal (transitionGbufForResume handles the rest).
	copyImageToImage(cmd,
		m_gbufNormal.image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_gbufNormalCopy.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_ctx->sceneExtent);
}

} // namespace graphics::vulkan
