#include "VulkanPostProcessing.h"

#include <array>

#include "VulkanRenderer.h"
#include "VulkanTexture.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/2d.h"
#include "graphics/opengl/SmaaAreaTex.h"
#include "graphics/opengl/SmaaSearchTex.h"

namespace graphics::vulkan {


// ===== SMAA Pipeline Implementation =====
//
// Mirrors gropenglpostprocessing.cpp's SMAA implementation: edge detection ->
// blending weight calculation -> neighborhood blending -> resolve. The shared
// SMAA.sdr algorithm body is untouched; only the thin wrapper shaders differ
// per-backend (see smaa-edge/blend/neighbour-*.sdr).
//
// Unlike OpenGL, Vulkan can't read and write the same attachment within one
// render pass, so neighborhood blending writes to Scene_luminance (safe to
// reuse as scratch: FXAA and SMAA are mutually exclusive via Gr_aa_mode) and
// a final resolve pass copies that back into Scene_ldr.

bool VulkanSMAA::init(PostProcessContext& ctx, const VulkanLDR& ldr)
{
	if (m_initialized) {
		return true;
	}

	if (!ldr.isInitialized()) {
		return false;
	}

	m_ctx = &ctx;
	m_ldr = &ldr;

	auto* texMgr = getTextureManager();
	if (!texMgr) {
		return false;
	}

	if (!texMgr->createStaticTexture2D(AREATEX_WIDTH, AREATEX_HEIGHT, vk::Format::eR8G8Unorm,
	                                   areaTexBytes, sizeof(areaTexBytes), "SMAA Area Texture",
	                                   m_smaaAreaTexImage, m_smaaAreaTexView, m_smaaAreaTexAlloc)) {
		nprintf(("vulkan", "VulkanSMAA: Failed to upload SMAA area texture!\n"));
		return false;
	}

	if (!texMgr->createStaticTexture2D(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, vk::Format::eR8Unorm,
	                                   searchTexBytes, sizeof(searchTexBytes), "SMAA Search Texture",
	                                   m_smaaSearchTexImage, m_smaaSearchTexView, m_smaaSearchTexAlloc)) {
		nprintf(("vulkan", "VulkanSMAA: Failed to upload SMAA search texture!\n"));
		shutdown();
		return false;
	}

	if (!createTargets()) {
		shutdown();
		return false;
	}

	m_initialized = true;
	nprintf(("vulkan", "VulkanSMAA: SMAA initialized (%ux%u)\n",
		m_ctx->sceneExtent.width, m_ctx->sceneExtent.height));
	return true;
}

bool VulkanSMAA::createTargets()
{
	const vk::Format ldrFormat = m_ctx->ldrFormat;
	const uint32_t w = m_ctx->sceneExtent.width;
	const uint32_t h = m_ctx->sceneExtent.height;

	if (!m_ctx->createImage(w, h, ldrFormat,
	                       vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                       vk::ImageAspectFlagBits::eColor,
	                       m_smaaEdges.image, m_smaaEdges.view, m_smaaEdges.allocation)) {
		nprintf(("vulkan", "VulkanSMAA: Failed to create SMAA edges image!\n"));
		return false;
	}
	m_smaaEdges.format = ldrFormat;
	m_smaaEdges.width = w;
	m_smaaEdges.height = h;

	if (!m_ctx->createImage(w, h, ldrFormat,
	                       vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                       vk::ImageAspectFlagBits::eColor,
	                       m_smaaBlend.image, m_smaaBlend.view, m_smaaBlend.allocation)) {
		nprintf(("vulkan", "VulkanSMAA: Failed to create SMAA blend image!\n"));
		return false;
	}
	m_smaaBlend.format = ldrFormat;
	m_smaaBlend.width = w;
	m_smaaBlend.height = h;

	// Framebuffers reuse the existing LDR render pass: same format, sample count,
	// loadOp=eDontCare and finalLayout=eShaderReadOnlyOptimal, so it's compatible.
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_ldr->renderPass();
		fbInfo.attachmentCount = 1;
		fbInfo.width = w;
		fbInfo.height = h;
		fbInfo.layers = 1;

		fbInfo.pAttachments = &m_smaaEdges.view;
		try {
			m_smaaEdgesFB = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanSMAA: Failed to create SMAA edges framebuffer: %s\n", e.what()));
			return false;
		}

		fbInfo.pAttachments = &m_smaaBlend.view;
		try {
			m_smaaBlendFB = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanSMAA: Failed to create SMAA blend framebuffer: %s\n", e.what()));
			return false;
		}
	}

	return true;
}

void VulkanSMAA::destroyTargets()
{
	if (m_smaaBlendFB) {
		m_ctx->device.destroyFramebuffer(m_smaaBlendFB);
		m_smaaBlendFB = nullptr;
	}
	if (m_smaaEdgesFB) {
		m_ctx->device.destroyFramebuffer(m_smaaEdgesFB);
		m_smaaEdgesFB = nullptr;
	}

	m_ctx->destroyTarget(m_smaaBlend);
	m_ctx->destroyTarget(m_smaaEdges);
}

bool VulkanSMAA::resize()
{
	if (!m_initialized) {
		return true;
	}
	destroyTargets();
	return createTargets();
}

void VulkanSMAA::shutdown()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}

	destroyTargets();

	if (m_smaaSearchTexView) {
		m_ctx->device.destroyImageView(m_smaaSearchTexView);
		m_smaaSearchTexView = nullptr;
	}
	if (m_smaaSearchTexImage) {
		m_ctx->device.destroyImage(m_smaaSearchTexImage);
		m_smaaSearchTexImage = nullptr;
	}
	if (m_smaaSearchTexAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_smaaSearchTexAlloc);
	}

	if (m_smaaAreaTexView) {
		m_ctx->device.destroyImageView(m_smaaAreaTexView);
		m_smaaAreaTexView = nullptr;
	}
	if (m_smaaAreaTexImage) {
		m_ctx->device.destroyImage(m_smaaAreaTexImage);
		m_smaaAreaTexImage = nullptr;
	}
	if (m_smaaAreaTexAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_smaaAreaTexAlloc);
	}

	m_initialized = false;
}

void VulkanSMAA::execute(vk::CommandBuffer cmd)
{
	if (!m_initialized || !m_ldr->isInitialized() || !gr_is_smaa_mode(Gr_aa_mode)) {
		return;
	}

	GR_DEBUG_SCOPE("SMAA");

	graphics::generic_data::smaa_data smaaData;
	smaaData.smaa_rt_metrics.x = static_cast<float>(m_ctx->sceneExtent.width);
	smaaData.smaa_rt_metrics.y = static_cast<float>(m_ctx->sceneExtent.height);
	smaaData.pad[0] = 0.0f;
	smaaData.pad[1] = 0.0f;

	// Pass 1: edge detection. Detection proxy (Scene_ldr, or the tonemap-compressed
	// version while HDR is active) -> Smaa_edges
	{
		GR_DEBUG_SCOPE("SMAA Detect Edges");

		// The edge detection shader discards non-edge pixels, so Smaa_edges must be
		// cleared every frame -- otherwise discarded pixels keep last frame's edges
		// forever. Explicit clearColor (vkCmdClearAttachments) rather than relying on
		// the render pass's own loadOp=eDontCare, which leaves those pixels undefined.
		static const std::array<float, 4> kClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		m_ctx->drawFullscreenTriangle(cmd, m_ldr->renderPass(),
			m_smaaEdgesFB, m_ctx->sceneExtent,
			SDR_TYPE_POST_PROCESS_SMAA_EDGE,
			m_ldr->getAADetectionView(), m_ctx->linearSampler,
			&smaaData, sizeof(smaaData),
			ALPHA_BLEND_NONE,
			0, vk::SampleCountFlagBits::e1, false,
			&kClearColor);
	}

	// Pass 2: blending weight calculation. Smaa_edges + area + search -> Smaa_blend
	{
		GR_DEBUG_SCOPE("SMAA Blending Weights calculation");

		std::array<vk::ImageView, 3> views = {m_smaaEdges.view, m_smaaAreaTexView, m_smaaSearchTexView};
		m_ctx->drawFullscreenTriangleMulti(cmd, m_ldr->renderPass(), m_smaaBlendFB, m_ctx->sceneExtent,
			SDR_TYPE_POST_PROCESS_SMAA_BLENDING_WEIGHT, views.data(), static_cast<uint32_t>(views.size()),
			&smaaData, sizeof(smaaData));
	}

	// Pass 3: neighborhood blending. Scene_ldr (real, extended-range) + Smaa_blend ->
	// Scene_luminance (scratch; safe to reuse since FXAA and SMAA are mutually exclusive).
	{
		GR_DEBUG_SCOPE("SMAA Neighborhood Blending");

		std::array<vk::ImageView, 2> views = {m_ldr->ldrView(), m_smaaBlend.view};
		m_ctx->drawFullscreenTriangleMulti(cmd, m_ldr->renderPass(), m_ldr->luminanceFramebuffer(), m_ctx->sceneExtent,
			SDR_TYPE_POST_PROCESS_SMAA_NEIGHBORHOOD_BLENDING, views.data(), static_cast<uint32_t>(views.size()),
			&smaaData, sizeof(smaaData));
	}

	// Pass 4: resolve. Scene_luminance -> Scene_ldr
	{
		GR_DEBUG_SCOPE("SMAA Resolve");

		m_ctx->drawFullscreenTriangle(cmd, m_ldr->renderPass(),
			m_ldr->ldrFramebuffer(), m_ctx->sceneExtent,
			SDR_TYPE_POST_PROCESS_SMAA_RESOLVE,
			m_ldr->luminanceView(), m_ctx->linearSampler,
			nullptr, 0,
			ALPHA_BLEND_NONE);
	}
}

} // namespace graphics::vulkan
