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

bool VulkanPostProcessor::initSMAA()
{
	if (m_smaaInitialized) {
		return true;
	}

	if (!m_ldrInitialized) {
		return false;
	}

	auto* texMgr = getTextureManager();
	if (!texMgr) {
		return false;
	}

	if (!texMgr->createStaticTexture2D(AREATEX_WIDTH, AREATEX_HEIGHT, vk::Format::eR8G8Unorm,
	                                   areaTexBytes, sizeof(areaTexBytes), "SMAA Area Texture",
	                                   m_smaaAreaTexImage, m_smaaAreaTexView, m_smaaAreaTexAlloc)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to upload SMAA area texture!\n"));
		return false;
	}

	if (!texMgr->createStaticTexture2D(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, vk::Format::eR8Unorm,
	                                   searchTexBytes, sizeof(searchTexBytes), "SMAA Search Texture",
	                                   m_smaaSearchTexImage, m_smaaSearchTexView, m_smaaSearchTexAlloc)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to upload SMAA search texture!\n"));
		shutdownSMAA();
		return false;
	}

	const vk::Format ldrFormat = m_ctx.ldrFormat;
	const uint32_t w = m_ctx.sceneExtent.width;
	const uint32_t h = m_ctx.sceneExtent.height;

	if (!m_ctx.createImage(w, h, ldrFormat,
	                       vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                       vk::ImageAspectFlagBits::eColor,
	                       m_smaaEdges.image, m_smaaEdges.view, m_smaaEdges.allocation)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create SMAA edges image!\n"));
		shutdownSMAA();
		return false;
	}
	m_smaaEdges.format = ldrFormat;
	m_smaaEdges.width = w;
	m_smaaEdges.height = h;

	if (!m_ctx.createImage(w, h, ldrFormat,
	                       vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                       vk::ImageAspectFlagBits::eColor,
	                       m_smaaBlend.image, m_smaaBlend.view, m_smaaBlend.allocation)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create SMAA blend image!\n"));
		shutdownSMAA();
		return false;
	}
	m_smaaBlend.format = ldrFormat;
	m_smaaBlend.width = w;
	m_smaaBlend.height = h;

	// Framebuffers reuse the existing LDR render pass: same format, sample count,
	// loadOp=eDontCare and finalLayout=eShaderReadOnlyOptimal, so it's compatible.
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_ldrRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.width = w;
		fbInfo.height = h;
		fbInfo.layers = 1;

		fbInfo.pAttachments = &m_smaaEdges.view;
		try {
			m_smaaEdgesFB = m_ctx.device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create SMAA edges framebuffer: %s\n", e.what()));
			shutdownSMAA();
			return false;
		}

		fbInfo.pAttachments = &m_smaaBlend.view;
		try {
			m_smaaBlendFB = m_ctx.device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create SMAA blend framebuffer: %s\n", e.what()));
			shutdownSMAA();
			return false;
		}
	}

	m_smaaInitialized = true;
	nprintf(("vulkan", "VulkanPostProcessor: SMAA initialized (%ux%u)\n", w, h));
	return true;
}

void VulkanPostProcessor::shutdownSMAA()
{
	if (!m_ctx.device) {
		return;
	}

	if (m_smaaBlendFB) {
		m_ctx.device.destroyFramebuffer(m_smaaBlendFB);
		m_smaaBlendFB = nullptr;
	}
	if (m_smaaEdgesFB) {
		m_ctx.device.destroyFramebuffer(m_smaaEdgesFB);
		m_smaaEdgesFB = nullptr;
	}

	if (m_smaaBlend.view) {
		m_ctx.device.destroyImageView(m_smaaBlend.view);
		m_smaaBlend.view = nullptr;
	}
	if (m_smaaBlend.image) {
		m_ctx.device.destroyImage(m_smaaBlend.image);
		m_smaaBlend.image = nullptr;
	}
	if (m_smaaBlend.allocation.isValid()) {
		m_ctx.memoryManager->freeAllocation(m_smaaBlend.allocation);
	}

	if (m_smaaEdges.view) {
		m_ctx.device.destroyImageView(m_smaaEdges.view);
		m_smaaEdges.view = nullptr;
	}
	if (m_smaaEdges.image) {
		m_ctx.device.destroyImage(m_smaaEdges.image);
		m_smaaEdges.image = nullptr;
	}
	if (m_smaaEdges.allocation.isValid()) {
		m_ctx.memoryManager->freeAllocation(m_smaaEdges.allocation);
	}

	if (m_smaaSearchTexView) {
		m_ctx.device.destroyImageView(m_smaaSearchTexView);
		m_smaaSearchTexView = nullptr;
	}
	if (m_smaaSearchTexImage) {
		m_ctx.device.destroyImage(m_smaaSearchTexImage);
		m_smaaSearchTexImage = nullptr;
	}
	if (m_smaaSearchTexAlloc.isValid()) {
		m_ctx.memoryManager->freeAllocation(m_smaaSearchTexAlloc);
	}

	if (m_smaaAreaTexView) {
		m_ctx.device.destroyImageView(m_smaaAreaTexView);
		m_smaaAreaTexView = nullptr;
	}
	if (m_smaaAreaTexImage) {
		m_ctx.device.destroyImage(m_smaaAreaTexImage);
		m_smaaAreaTexImage = nullptr;
	}
	if (m_smaaAreaTexAlloc.isValid()) {
		m_ctx.memoryManager->freeAllocation(m_smaaAreaTexAlloc);
	}

	m_smaaInitialized = false;
}

void VulkanPostProcessor::executeSMAA(vk::CommandBuffer cmd)
{
	if (!m_smaaInitialized || !m_ldrInitialized || !gr_is_smaa_mode(Gr_aa_mode)) {
		return;
	}

	m_ctx.scratchUBOMapped = m_ctx.memoryManager->mapMemory(m_ctx.scratchUBOAlloc);
	if (!m_ctx.scratchUBOMapped) {
		return;
	}

	graphics::generic_data::smaa_data smaaData;
	smaaData.smaa_rt_metrics.x = static_cast<float>(m_ctx.sceneExtent.width);
	smaaData.smaa_rt_metrics.y = static_cast<float>(m_ctx.sceneExtent.height);
	smaaData.pad[0] = 0.0f;
	smaaData.pad[1] = 0.0f;

	// Pass 1: edge detection. Detection proxy (Scene_ldr, or the tonemap-compressed
	// version while HDR is active) -> Smaa_edges
	drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_smaaEdgesFB, m_ctx.sceneExtent,
		SDR_TYPE_POST_PROCESS_SMAA_EDGE,
		getAADetectionView(), m_ctx.linearSampler,
		&smaaData, sizeof(smaaData),
		ALPHA_BLEND_NONE);

	// Pass 2: blending weight calculation. Smaa_edges + area + search -> Smaa_blend
	{
		std::array<vk::ImageView, 3> views = {m_smaaEdges.view, m_smaaAreaTexView, m_smaaSearchTexView};
		drawFullscreenTriangleMulti(cmd, m_ldrRenderPass, m_smaaBlendFB, m_ctx.sceneExtent,
			SDR_TYPE_POST_PROCESS_SMAA_BLENDING_WEIGHT, views.data(), static_cast<uint32_t>(views.size()),
			&smaaData, sizeof(smaaData));
	}

	// Pass 3: neighborhood blending. Scene_ldr (real, extended-range) + Smaa_blend ->
	// Scene_luminance (scratch; safe to reuse since FXAA and SMAA are mutually exclusive).
	{
		std::array<vk::ImageView, 2> views = {m_sceneLdr.view, m_smaaBlend.view};
		drawFullscreenTriangleMulti(cmd, m_ldrRenderPass, m_sceneLuminanceFB, m_ctx.sceneExtent,
			SDR_TYPE_POST_PROCESS_SMAA_NEIGHBORHOOD_BLENDING, views.data(), static_cast<uint32_t>(views.size()),
			&smaaData, sizeof(smaaData));
	}

	// Pass 4: resolve. Scene_luminance -> Scene_ldr
	drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLdrFB, m_ctx.sceneExtent,
		SDR_TYPE_POST_PROCESS_SMAA_RESOLVE,
		m_sceneLuminance.view, m_ctx.linearSampler,
		nullptr, 0,
		ALPHA_BLEND_NONE);

	m_ctx.memoryManager->unmapMemory(m_ctx.scratchUBOAlloc);
	m_ctx.scratchUBOMapped = nullptr;
}

} // namespace graphics::vulkan
