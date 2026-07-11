#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanRenderer.h"
#include "VulkanDescriptorManager.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/post_processing.h"
#include "graphics/grinternal.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "math/floating.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {


// ===== LDR Targets + FXAA Pipeline Implementation =====

bool VulkanLDR::init(PostProcessContext& ctx, const RenderTarget& sceneColor, const RenderTarget& sceneDepth,
                      const VulkanBloom& bloom)
{
	m_ctx = &ctx;
	m_sceneColor = &sceneColor;
	m_sceneDepth = &sceneDepth;
	m_bloom = &bloom;

	const vk::Format ldrFormat = m_ctx->ldrFormat;

	// Create LDR render pass (color-only, loadOp=eDontCare, finalLayout=eShaderReadOnlyOptimal)
	{
		vk::AttachmentDescription att;
		att.format = ldrFormat;
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
			m_ldrRenderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanLDR: Failed to create LDR render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create LDR load render pass (loadOp=eLoad for additive blending onto existing content)
	{
		vk::AttachmentDescription att;
		att.format = ldrFormat;
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
			m_ldrLoadRenderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanLDR: Failed to create LDR load render pass: %s\n", e.what()));
			return false;
		}
	}

	if (!createTargets()) {
		return false;
	}

	m_initialized = true;
	nprintf(("vulkan", "VulkanLDR: LDR targets initialized (%ux%u, RGBA8)\n",
		m_ctx->sceneExtent.width, m_ctx->sceneExtent.height));
	return true;
}

bool VulkanLDR::createTargets()
{
	const vk::Format ldrFormat = m_ctx->ldrFormat;

	// Create Scene_ldr (full resolution) — tonemapped display-referred output.
	// fp16 in HDR (carries values above paper white), 8-bit UNORM in SDR.
	if (!m_ctx->createImage(m_ctx->sceneExtent.width, m_ctx->sceneExtent.height, ldrFormat,
	                 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneLdr.image, m_sceneLdr.view, m_sceneLdr.allocation)) {
		nprintf(("vulkan", "VulkanLDR: Failed to create Scene_ldr image!\n"));
		return false;
	}
	m_sceneLdr.format = ldrFormat;
	m_sceneLdr.width = m_ctx->sceneExtent.width;
	m_sceneLdr.height = m_ctx->sceneExtent.height;

	// Create Scene_luminance (full resolution) — LDR with luma in alpha for FXAA
	if (!m_ctx->createImage(m_ctx->sceneExtent.width, m_ctx->sceneExtent.height, ldrFormat,
	                 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneLuminance.image, m_sceneLuminance.view, m_sceneLuminance.allocation)) {
		nprintf(("vulkan", "VulkanLDR: Failed to create Scene_luminance image!\n"));
		return false;
	}
	m_sceneLuminance.format = ldrFormat;
	m_sceneLuminance.width = m_ctx->sceneExtent.width;
	m_sceneLuminance.height = m_ctx->sceneExtent.height;

	// Create Scene_ldr_compressed (full resolution) — tonemapped to [0,1] using the
	// real tone curve, purely for FXAA/SMAA edge detection while HDR output is
	// active (see executeTonemap/getAADetectionView).
	if (!m_ctx->createImage(m_ctx->sceneExtent.width, m_ctx->sceneExtent.height, ldrFormat,
	                 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneLdrCompressed.image, m_sceneLdrCompressed.view, m_sceneLdrCompressed.allocation)) {
		nprintf(("vulkan", "VulkanLDR: Failed to create Scene_ldr_compressed image!\n"));
		return false;
	}
	m_sceneLdrCompressed.format = ldrFormat;
	m_sceneLdrCompressed.width = m_ctx->sceneExtent.width;
	m_sceneLdrCompressed.height = m_ctx->sceneExtent.height;

	// Create framebuffers
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_ldrRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &m_sceneLdr.view;
		fbInfo.width = m_ctx->sceneExtent.width;
		fbInfo.height = m_ctx->sceneExtent.height;
		fbInfo.layers = 1;

		try {
			m_sceneLdrFB = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanLDR: Failed to create Scene_ldr framebuffer: %s\n", e.what()));
			return false;
		}

		fbInfo.pAttachments = &m_sceneLuminance.view;
		try {
			m_sceneLuminanceFB = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanLDR: Failed to create Scene_luminance framebuffer: %s\n", e.what()));
			return false;
		}

		fbInfo.pAttachments = &m_sceneLdrCompressed.view;
		try {
			m_sceneLdrCompressedFB = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanLDR: Failed to create Scene_ldr_compressed framebuffer: %s\n", e.what()));
			return false;
		}
	}

	return true;
}

void VulkanLDR::destroyTargets()
{
	if (m_sceneLdrCompressedFB) {
		m_ctx->device.destroyFramebuffer(m_sceneLdrCompressedFB);
		m_sceneLdrCompressedFB = nullptr;
	}
	if (m_sceneLuminanceFB) {
		m_ctx->device.destroyFramebuffer(m_sceneLuminanceFB);
		m_sceneLuminanceFB = nullptr;
	}
	if (m_sceneLdrFB) {
		m_ctx->device.destroyFramebuffer(m_sceneLdrFB);
		m_sceneLdrFB = nullptr;
	}

	m_ctx->destroyTarget(m_sceneLdrCompressed);
	m_ctx->destroyTarget(m_sceneLuminance);
	m_ctx->destroyTarget(m_sceneLdr);
}

bool VulkanLDR::resize()
{
	if (!m_initialized) {
		return true;
	}
	destroyTargets();
	return createTargets();
}

void VulkanLDR::shutdown()
{
	if (!m_initialized) {
		return;
	}

	destroyTargets();

	if (m_ldrLoadRenderPass) {
		m_ctx->device.destroyRenderPass(m_ldrLoadRenderPass);
		m_ldrLoadRenderPass = nullptr;
	}
	if (m_ldrRenderPass) {
		m_ctx->device.destroyRenderPass(m_ldrRenderPass);
		m_ldrRenderPass = nullptr;
	}

	m_initialized = false;
}

void VulkanLDR::executeTonemap(vk::CommandBuffer cmd)
{
	if (!m_initialized) {
		return;
	}

	GR_DEBUG_SCOPE("Tonemapping");

	namespace ltp = lighting_profiles;

	// Build tonemapping data directly from lighting profiles
	graphics::generic_data::tonemapping_data tmData;
	memset(&tmData, 0, sizeof(tmData));
	auto ppc = ltp::current_piecewise_intermediates();
	tmData.exposure = ltp::current_exposure();
	tmData.tonemapper = static_cast<int>(ltp::current_tonemapper());
	tmData.x0 = ppc.x0;
	tmData.y0 = ppc.y0;
	tmData.x1 = ppc.x1;
	tmData.toe_B = ppc.toe_B;
	tmData.toe_lnA = ppc.toe_lnA;
	tmData.sh_B = ppc.sh_B;
	tmData.sh_lnA = ppc.sh_lnA;
	tmData.sh_offsetX = ppc.sh_offsetX;
	tmData.sh_offsetY = ppc.sh_offsetY;

	// current_tonemapper() (used above) forces TonemapperAlgorithm::HdrScene
	// whenever HDR10 output is active, so tmData.tonemapper already selects
	// the right shader case; just supply the nits it needs.
	if (m_ctx->hdrActive) {
		tmData.hdr_paperwhite_nits = Gr_hdr_paperwhite_nits;
		tmData.hdr_peak_nits = Gr_hdr_peak_nits;
	}

	// HDR scene → Scene_ldr via tonemapping shader
	m_ctx->drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLdrFB, m_ctx->sceneExtent,
		SDR_TYPE_POST_PROCESS_TONEMAPPING,
		m_sceneColor->view, m_ctx->linearSampler,
		&tmData, sizeof(tmData),
		ALPHA_BLEND_NONE);

	// While HDR is active, Scene_ldr carries extended-range values (the
	// HdrScene tonemapper only clamps to headroom, not [0,1]), which breaks
	// the fixed [0,1] luma thresholds FXAA/SMAA edge detection rely on. So
	// also derive a cheap [0,1]-bounded proxy for those passes to detect
	// edges from — they still blend the real extended-range Scene_ldr for
	// the actual output. This just clamps the already-tonemapped Scene_ldr,
	// rather than re-running exposure+tonemap from Scene_color a second time.
	if (m_ctx->hdrActive) {
		m_ctx->drawFullscreenTriangle(cmd, m_ldrRenderPass,
			m_sceneLdrCompressedFB, m_ctx->sceneExtent,
			SDR_TYPE_COPY,
			m_sceneLdr.view, m_ctx->linearSampler,
			nullptr, 0,
			ALPHA_BLEND_NONE,
			SDR_FLAG_COPY_CLAMP01);
	}
}

// Returns the buffer FXAA/SMAA edge detection should read for luma computation:
// the real Scene_ldr when it's already bounded to [0,1], or the compressed
// tonemap proxy when HDR output has left it in extended range.
vk::ImageView VulkanLDR::getAADetectionView() const
{
	return m_ctx->hdrActive ? m_sceneLdrCompressed.view : m_sceneLdr.view;
}

void VulkanLDR::executeFXAA(vk::CommandBuffer cmd)
{
	if (!m_initialized || !gr_is_fxaa_mode(Gr_aa_mode)) {
		return;
	}

	GR_DEBUG_SCOPE("FXAA");

	// FXAA prepass: Scene_ldr → Scene_luminance (real color passthrough + luma in
	// alpha computed from the detection proxy, see getAADetectionView)
	{
		std::array<vk::ImageView, 2> views = {m_sceneLdr.view, getAADetectionView()};
		m_ctx->drawFullscreenTriangleMulti(cmd, m_ldrRenderPass, m_sceneLuminanceFB, m_ctx->sceneExtent,
			SDR_TYPE_POST_PROCESS_FXAA_PREPASS, views.data(), static_cast<uint32_t>(views.size()),
			nullptr, 0);
	}

	// FXAA main pass: Scene_luminance → Scene_ldr
	graphics::generic_data::fxaa_data fxaaData;
	fxaaData.rt_w = static_cast<float>(m_ctx->sceneExtent.width);
	fxaaData.rt_h = static_cast<float>(m_ctx->sceneExtent.height);
	fxaaData.pad[0] = 0.0f;
	fxaaData.pad[1] = 0.0f;

	m_ctx->drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLdrFB, m_ctx->sceneExtent,
		SDR_TYPE_POST_PROCESS_FXAA,
		m_sceneLuminance.view, m_ctx->linearSampler,
		&fxaaData, sizeof(fxaaData),
		ALPHA_BLEND_NONE);
}

bool VulkanLDR::executePostEffects(vk::CommandBuffer cmd)
{
	m_postEffectsApplied = false;

	if (!m_initialized || !graphics::Post_processing_manager) {
		return false;
	}

	const auto& postEffects = graphics::Post_processing_manager->getPostEffects();
	if (postEffects.empty()) {
		return false;
	}

	// Compute effect flags from current state
	int effectFlags = 0;
	for (size_t idx = 0; idx < postEffects.size(); idx++) {
		if (postEffects[idx].always_on || (postEffects[idx].intensity != postEffects[idx].default_intensity)) {
			effectFlags |= (1 << idx);
		}
	}

	if (effectFlags == 0) {
		return false;
	}

	GR_DEBUG_SCOPE("Draw post effects");

	// Build the extended post_data UBO with effectFlags appended
	struct PostEffectsUBOData {
		graphics::generic_data::post_data base;
		int effectFlags;
		int pad[3];
	};

	PostEffectsUBOData uboData;
	memset(&uboData, 0, sizeof(uboData));
	uboData.base.timer = static_cast<float>((timer_get_milliseconds() % 100) + 1);
	uboData.effectFlags = effectFlags;

	// Fill effect parameters
	for (size_t idx = 0; idx < postEffects.size(); idx++) {
		if (!(effectFlags & (1 << idx))) {
			continue;
		}
		float value = postEffects[idx].intensity;
		switch (postEffects[idx].uniform_type) {
		case graphics::PostEffectUniformType::NoiseAmount:
			uboData.base.noise_amount = value;
			break;
		case graphics::PostEffectUniformType::Saturation:
			uboData.base.saturation = value;
			break;
		case graphics::PostEffectUniformType::Brightness:
			uboData.base.brightness = value;
			break;
		case graphics::PostEffectUniformType::Contrast:
			uboData.base.contrast = value;
			break;
		case graphics::PostEffectUniformType::FilmGrain:
			uboData.base.film_grain = value;
			break;
		case graphics::PostEffectUniformType::TvStripes:
			uboData.base.tv_stripes = value;
			break;
		case graphics::PostEffectUniformType::Cutoff:
			uboData.base.cutoff = value;
			break;
		case graphics::PostEffectUniformType::Dither:
			uboData.base.dither = value;
			break;
		case graphics::PostEffectUniformType::Tint:
			uboData.base.tint = postEffects[idx].rgb;
			break;
		case graphics::PostEffectUniformType::CustomEffectVEC3A:
			uboData.base.custom_effect_vec3_a = postEffects[idx].rgb;
			break;
		case graphics::PostEffectUniformType::CustomEffectFloatA:
			uboData.base.custom_effect_float_a = value;
			break;
		case graphics::PostEffectUniformType::CustomEffectVEC3B:
			uboData.base.custom_effect_vec3_b = postEffects[idx].rgb;
			break;
		case graphics::PostEffectUniformType::CustomEffectFloatB:
			uboData.base.custom_effect_float_b = value;
			break;
		default:
			break;
		}
	}

	// Post-effects: Scene_ldr → Scene_luminance (reusing luminance target as temp)
	m_ctx->drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLuminanceFB, m_ctx->sceneExtent,
		SDR_TYPE_POST_PROCESS_MAIN,
		m_sceneLdr.view, m_ctx->linearSampler,
		&uboData, sizeof(uboData),
		ALPHA_BLEND_NONE,
		static_cast<unsigned int>(effectFlags));

	m_postEffectsApplied = true;
	return true;
}

void VulkanLDR::executeLightshafts(vk::CommandBuffer cmd)
{
	if (!m_initialized || !graphics::Post_processing_manager) {
		return;
	}

	if (Game_subspace_effect || !gr_sunglare_enabled() || !gr_lightshafts_enabled()) {
		return;
	}

	// Find a global light with glare facing the camera
	int n_lights = light_get_global_count();
	float sun_x = 0.0f, sun_y = 0.0f;
	bool found = false;

	for (int idx = 0; idx < n_lights; idx++) {
		vec3d light_dir;
		light_get_global_dir(&light_dir, idx);

		if (!light_has_glare(idx)) {
			continue;
		}

		float dot = vm_vec_dot(&light_dir, &Eye_matrix.vec.fvec);
		if (dot > 0.7f) {
			sun_x = asinf_safe(vm_vec_dot(&light_dir, &Eye_matrix.vec.rvec)) / PI * 1.5f + 0.5f;
			sun_y = asinf_safe(vm_vec_dot(&light_dir, &Eye_matrix.vec.uvec)) / PI * 1.5f * gr_screen.clip_aspect + 0.5f;
			found = true;
			break;
		}
	}

	if (!found) {
		return;
	}

	GR_DEBUG_SCOPE("Lightshafts");

	// Transition scene depth from eDepthStencilAttachmentOptimal to eShaderReadOnlyOptimal for sampling
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_sceneDepth->image;
		barrier.subresourceRange.aspectMask = imageAspectFromFormat(m_ctx->depthFormat);
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::PipelineStageFlagBits::eFragmentShader,
			{}, {}, {}, barrier);
	}

	// Transition Scene_ldr to eColorAttachmentOptimal for loadOp=eLoad render pass
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_sceneLdr.image;
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

	// Build lightshaft UBO data
	auto& ls_params = graphics::Post_processing_manager->getLightshaftParams();

	graphics::generic_data::lightshaft_data lsData;
	lsData.sun_pos.x = sun_x;
	lsData.sun_pos.y = sun_y;
	lsData.density = ls_params.density;
	lsData.weight = ls_params.weight;
	lsData.falloff = ls_params.falloff;
	lsData.intensity = Sun_spot * ls_params.intensity;
	lsData.cp_intensity = Sun_spot * ls_params.cpintensity;
	lsData.pad[0] = 0.0f;

	// Additive blend lightshafts onto Scene_ldr
	m_ctx->drawFullscreenTriangle(cmd, m_ldrLoadRenderPass,
		m_sceneLdrFB, m_ctx->sceneExtent,
		SDR_TYPE_POST_PROCESS_LIGHTSHAFTS,
		m_sceneDepth->view, m_ctx->linearSampler,
		&lsData, sizeof(lsData),
		ALPHA_BLEND_ADDITIVE);
}

} // namespace graphics::vulkan
