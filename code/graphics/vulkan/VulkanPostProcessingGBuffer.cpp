#include "VulkanPostProcessing.h"

#include <array>

#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanBarrier.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"
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
	const uint32_t velocityIndex = colorCount;
	const uint32_t depthIndex = colorCount + (config.includeVelocity ? 1 : 0);
	const uint32_t totalAttachments = depthIndex + 1;

	// Max 6 color + 1 velocity + 1 depth = 8 attachments
	std::array<vk::AttachmentDescription, 8> attachments;
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

	// Motion vectors stay an attachment through the whole scene, which spares every mid-scene copy from having
	// to transition them around each pass end and resume.
	if (config.includeVelocity) {
		auto& velocity = attachments[velocityIndex];
		velocity.format = VulkanMotionVectors::FORMAT;
		velocity.samples = config.samples;
		velocity.loadOp = config.velocityLoadOp;
		velocity.storeOp = vk::AttachmentStoreOp::eStore;
		velocity.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		velocity.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		velocity.initialLayout = config.velocityLoadOp == vk::AttachmentLoadOp::eLoad
			? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eUndefined;
		velocity.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
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

	std::array<vk::AttachmentReference, VELOCITY_COLOR_LOCATION_COUNT> colorRefs;
	for (auto& ref : colorRefs) {
		ref.attachment = VK_ATTACHMENT_UNUSED;
		ref.layout = vk::ImageLayout::eUndefined;
	}
	for (uint32_t i = 0; i < colorCount; ++i) {
		colorRefs[i].attachment = i;
		colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
	}
	if (config.includeVelocity) {
		colorRefs[PipelineConfig::VELOCITY_ATTACHMENT].attachment = velocityIndex;
		colorRefs[PipelineConfig::VELOCITY_ATTACHMENT].layout = vk::ImageLayout::eColorAttachmentOptimal;
	}

	vk::AttachmentReference depthRef;
	depthRef.attachment = depthIndex;
	depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = config.includeVelocity ? VELOCITY_COLOR_LOCATION_COUNT : colorCount;
	subpass.pColorAttachments = colorRefs.data();
	subpass.pDepthStencilAttachment = &depthRef;

	vk::SubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	// In all cases srcAccessMask must make the PREVIOUS frame's writes to these
	// (single-instance, shared-across-frames) G-buffer attachments available
	// before this frame writes them again -- eColorAttachmentWrite /
	// eDepthStencilAttachmentWrite. Without it, frame N+1's beginRenderPass
	// races frame N's color storeOp (cross-frame WRITE_AFTER_WRITE, flagged by
	// -gr_sync_validation; visible as MSAA surface acne once timing shifts).
	if (config.useResolveDependency) {
		// MSAA resolve: previous pass read these textures as shader inputs, and
		// the prior frame's resolve wrote these same non-MSAA targets.
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests
		                        | vk::PipelineStageFlagBits::eLateFragmentTests
		                        | vk::PipelineStageFlagBits::eFragmentShader;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite
		                         | vk::AccessFlagBits::eShaderRead;
		// The read covers the motion vectors, which the resolve loads and blends
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eColorAttachmentRead
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	} else {
		// Standard G-buffer: previous pass may have done transfers (copies), and
		// the prior frame wrote these attachments as color/depth.
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests
		                        | vk::PipelineStageFlagBits::eLateFragmentTests
		                        | vk::PipelineStageFlagBits::eFragmentShader
		                        | vk::PipelineStageFlagBits::eTransfer;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite
		                         | vk::AccessFlagBits::eShaderRead
		                         | vk::AccessFlagBits::eTransferRead
		                         | vk::AccessFlagBits::eTransferWrite;
		// The read covers attachments the load variants keep, such as the motion vectors
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eColorAttachmentRead
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
	vk::RenderPass renderPass, bool includeComposite, bool useMsaaImages, const RenderTarget* velocity)
{
	// Attachment order: color, position, normal, specular, emissive, [composite], [velocity], depth
	std::array<vk::ImageView, 8> views;
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

	if (velocity) {
		views[count++] = velocity->view;
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

	// Create G-buffer render pass (eClear) — 6 color + depth
	try {
		m_gbufRenderPass = createGbufRenderPass({
			true, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear, vk::AttachmentLoadOp::eClear,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eUndefined,
		});
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create G-buffer render pass: %s\n", e.what()));
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
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create G-buffer load render pass: %s\n", e.what()));
		shutdown();
		return false;
	}

	if (!createTargets()) {
		shutdown();
		return false;
	}

	m_gbufInitialized = true;
	nprintf(("vulkan", "VulkanPostProcessor: G-buffer initialized (%ux%u, 6 color + depth)\n",
		m_ctx->sceneExtent.width, m_ctx->sceneExtent.height));
	return true;
}

bool VulkanDeferredGBuffer::createTargets()
{
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
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create G-buffer %s image!\n", t.name));
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
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create G-buffer normal copy!\n"));
			return false;
		}
		m_gbufNormalCopy.format = GBUF_FORMAT_NORMAL;
		m_gbufNormalCopy.width = w;
		m_gbufNormalCopy.height = h;
	}

	// Create G-buffer framebuffer (6 color + depth)
	try {
		m_gbufFramebuffer = createGbufFramebuffer(m_gbufRenderPass, true, false);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create G-buffer framebuffer: %s\n", e.what()));
		return false;
	}

	return true;
}

bool VulkanDeferredGBuffer::createVelocityRenderPasses()
{
	if (m_velocityRenderPass) {
		return true;
	}

	try {
		m_velocityRenderPass = createGbufRenderPass({
			true, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear, vk::AttachmentLoadOp::eClear,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eUndefined,
			false, {}, true, vk::AttachmentLoadOp::eClear,
		});
		m_velocityRenderPassLoad = createGbufRenderPass({
			true, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eLoad, vk::AttachmentLoadOp::eLoad,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			false, {}, true, vk::AttachmentLoadOp::eLoad,
		});

		if (m_msaaInitialized) {
			const auto samples = getRendererInstance()->getMsaaSampleCount();
			m_velocityMsaaRenderPass = createGbufRenderPass({
				false, samples,
				vk::AttachmentLoadOp::eClear, vk::AttachmentLoadOp::eClear,
				vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				false, {{GBUF_ATT_EMISSIVE, vk::AttachmentLoadOp::eLoad}}, true, vk::AttachmentLoadOp::eClear,
			});
			m_velocityMsaaRenderPassLoad = createGbufRenderPass({
				false, samples,
				vk::AttachmentLoadOp::eLoad, vk::AttachmentLoadOp::eLoad,
				vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::eDepthStencilAttachmentOptimal,
				false, {}, true, vk::AttachmentLoadOp::eLoad,
			});
			// Loads rather than discards velocity, because the cockpit resolves on top of the main scene's.
			m_velocityResolveRenderPass = createGbufRenderPass({
				false, vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
				vk::ImageLayout::eUndefined,
				true, {}, true, vk::AttachmentLoadOp::eLoad,
			});
		}
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanDeferredGBuffer: Failed to create motion vector render passes: %s\n", e.what()));
		destroyVelocityRenderPasses();
		return false;
	}

	return true;
}

void VulkanDeferredGBuffer::destroyVelocityRenderPasses()
{
	for (auto* pass : {&m_velocityRenderPass, &m_velocityRenderPassLoad, &m_velocityMsaaRenderPass,
	                   &m_velocityMsaaRenderPassLoad, &m_velocityResolveRenderPass}) {
		if (*pass) {
			m_ctx->device.destroyRenderPass(*pass);
			*pass = nullptr;
		}
	}
}

bool VulkanDeferredGBuffer::createVelocityTargets(const RenderTarget& drawnMotion)
{
	if (m_velocityFramebuffer) {
		return true;
	}

	try {
		m_velocityFramebuffer = createGbufFramebuffer(m_velocityRenderPass, true, false, &drawnMotion);

		if (m_msaaInitialized) {
			if (!m_ctx->createImage(m_ctx->sceneExtent.width, m_ctx->sceneExtent.height, VulkanMotionVectors::FORMAT,
			                        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			                        vk::ImageAspectFlagBits::eColor,
			                        m_msaaVelocity.image, m_msaaVelocity.view, m_msaaVelocity.allocation,
			                        getRendererInstance()->getMsaaSampleCount())) {
				nprintf(("vulkan", "VulkanDeferredGBuffer: Failed to create MSAA motion vector image!\n"));
				destroyVelocityTargets();
				return false;
			}
			m_msaaVelocity.format = VulkanMotionVectors::FORMAT;
			m_msaaVelocity.width = m_ctx->sceneExtent.width;
			m_msaaVelocity.height = m_ctx->sceneExtent.height;

			m_velocityMsaaFramebuffer = createGbufFramebuffer(m_velocityMsaaRenderPass, false, true, &m_msaaVelocity);
			m_velocityResolveFramebuffer = createGbufFramebuffer(m_velocityResolveRenderPass, false, false, &drawnMotion);

			getTextureManager()->transitionImageLayout(m_msaaVelocity.image, m_msaaVelocity.format,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
		}
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanDeferredGBuffer: Failed to create motion vector framebuffers: %s\n", e.what()));
		destroyVelocityTargets();
		return false;
	}

	return true;
}

void VulkanDeferredGBuffer::destroyVelocityTargets()
{
	for (auto* framebuffer : {&m_velocityFramebuffer, &m_velocityMsaaFramebuffer, &m_velocityResolveFramebuffer}) {
		if (*framebuffer) {
			m_ctx->device.destroyFramebuffer(*framebuffer);
			*framebuffer = nullptr;
		}
	}
	m_ctx->destroyTarget(m_msaaVelocity);
}

bool VulkanDeferredGBuffer::setVelocityTarget(const RenderTarget* drawnMotion)
{
	m_velocityActive = false;

	if (!drawnMotion || !m_gbufInitialized) {
		return drawnMotion == nullptr;
	}

	if (!createVelocityRenderPasses() || !createVelocityTargets(*drawnMotion)) {
		return false;
	}

	m_velocityActive = true;
	return true;
}

void VulkanDeferredGBuffer::destroyTargets()
{
	// Rebuilt at the new size when next selected
	destroyVelocityTargets();

	if (m_gbufFramebuffer) {
		m_ctx->device.destroyFramebuffer(m_gbufFramebuffer);
		m_gbufFramebuffer = nullptr;
	}

	std::array<RenderTarget*, 6> gbufTargets = {
		&m_gbufPosition, &m_gbufNormal, &m_gbufSpecular,
		&m_gbufEmissive, &m_gbufComposite, &m_gbufNormalCopy,
	};
	for (auto* rt : gbufTargets) {
		m_ctx->destroyTarget(*rt);
	}
}

bool VulkanDeferredGBuffer::resize()
{
	if (!m_gbufInitialized) {
		return true;
	}

	// The MSAA resolve framebuffer references the single-sample G-buffer images,
	// so the MSAA resources bracket the G-buffer recreation.
	const bool msaaActive = m_msaaInitialized;
	if (msaaActive) {
		destroyMsaaTargets();
	}

	destroyTargets();
	if (!createTargets()) {
		return false;
	}

	if (msaaActive && !createMsaaTargets()) {
		return false;
	}
	return true;
}

void VulkanDeferredGBuffer::shutdown()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}

	destroyTargets();
	destroyVelocityRenderPasses();

	if (m_gbufRenderPassLoad) {
		m_ctx->device.destroyRenderPass(m_gbufRenderPassLoad);
		m_gbufRenderPassLoad = nullptr;
	}
	if (m_gbufRenderPass) {
		m_ctx->device.destroyRenderPass(m_gbufRenderPass);
		m_gbufRenderPass = nullptr;
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

	std::array<ImageBarrier2, 5> barriers;
	for (size_t i = 0; i < gbufImages.size(); ++i) {
		barriers[i].image = gbufImages[i];
		barriers[i].levelCount = 1;
		barriers[i].layerCount = 1;
		barriers[i].oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[i].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barriers[i].srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barriers[i].srcAccess = {};
		barriers[i].dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		// eColorAttachmentRead as well as Write: the resumed pass loads these
		// attachments (loadOp=eLoad), a read that must be ordered after this
		// transition -- otherwise a READ_AFTER_WRITE hazard between the loadOp
		// and the layout transition (flagged by -gr_sync_validation).
		barriers[i].dstAccess = vk::AccessFlagBits2::eColorAttachmentWrite
		                      | vk::AccessFlagBits2::eColorAttachmentRead;
	}

	cmdImageBarriers(cmd, ArrayView<const ImageBarrier2>(barriers.data(), barriers.size()));
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
