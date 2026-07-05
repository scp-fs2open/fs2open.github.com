#include "VulkanPostProcessing.h"

#include <array>

#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorManager.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/grinternal.h"
#include "graphics/matrix.h"
#include "graphics/2d.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "tracing/tracing.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {


// ========== Fog / Volumetric Nebula ==========

void VulkanFog::init(PostProcessContext& ctx, const RenderTarget& sceneColor,
                     const RenderTarget& sceneDepth, const RenderTarget& sceneDepthCopy,
                     const VulkanDeferredGBuffer& gbuffer)
{
	m_ctx = &ctx;
	m_sceneColor = &sceneColor;
	m_sceneDepth = &sceneDepth;
	m_sceneDepthCopy = &sceneDepthCopy;
	m_gbuffer = &gbuffer;
}

void VulkanFog::copySceneDepth(vk::CommandBuffer cmd)
{
	// Copies scene depth → depth copy texture so the fog shaders can sample it.
	// Scene depth is in eDepthStencilAttachmentOptimal (from the ended scene render pass).
	copyImageToImage(cmd,
		m_sceneDepth->image, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
		m_sceneDepthCopy->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_ctx->sceneExtent,
		imageAspectFromFormat(m_ctx->depthFormat));
}

bool VulkanFog::initFogPass()
{
	if (m_fogInitialized) {
		return true;
	}

	// Create fog render pass: 1 RGBA16F color attachment, loadOp=eDontCare (writing every pixel),
	// initialLayout/finalLayout = eColorAttachmentOptimal (scene color stays as render target)
	{
		vk::AttachmentDescription att;
		att.format = HDR_COLOR_FORMAT;
		att.samples = vk::SampleCountFlagBits::e1;
		att.loadOp = vk::AttachmentLoadOp::eDontCare;
		att.storeOp = vk::AttachmentStoreOp::eStore;
		att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		att.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
		att.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

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
			m_fogRenderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create fog render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create fog framebuffer (scene color as attachment)
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_fogRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &m_sceneColor->view;
		fbInfo.width = m_ctx->sceneExtent.width;
		fbInfo.height = m_ctx->sceneExtent.height;
		fbInfo.layers = 1;

		try {
			m_fogFramebuffer = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create fog framebuffer: %s\n", e.what()));
			return false;
		}
	}

	m_fogInitialized = true;
	mprintf(("VulkanPostProcessor: Fog pass initialized\n"));
	return true;
}

void VulkanFog::shutdown()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}
	if (m_emissiveMipmappedFullView) {
		m_ctx->device.destroyImageView(m_emissiveMipmappedFullView);
		m_emissiveMipmappedFullView = nullptr;
	}
	if (m_emissiveMipmapped.view) {
		m_ctx->device.destroyImageView(m_emissiveMipmapped.view);
		m_emissiveMipmapped.view = nullptr;
	}
	if (m_emissiveMipmapped.image) {
		m_ctx->device.destroyImage(m_emissiveMipmapped.image);
		m_emissiveMipmapped.image = nullptr;
	}
	if (m_emissiveMipmapped.allocation.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_emissiveMipmapped.allocation);
	}
	m_emissiveMipmappedInitialized = false;

	if (m_fogFramebuffer) {
		m_ctx->device.destroyFramebuffer(m_fogFramebuffer);
		m_fogFramebuffer = nullptr;
	}
	if (m_fogRenderPass) {
		m_ctx->device.destroyRenderPass(m_fogRenderPass);
		m_fogRenderPass = nullptr;
	}
	m_fogInitialized = false;
}

void VulkanFog::renderScene(vk::CommandBuffer cmd)
{
	GR_DEBUG_SCOPE("Scene Fog");

	if (!m_fogInitialized) {
		if (!initFogPass()) {
			return;
		}
	}

	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	auto* bufferMgr = getBufferManager();
	auto* texMgr = getTextureManager();

	if (!pipelineMgr || !descriptorMgr || !bufferMgr || !texMgr) {
		return;
	}

	// Copy scene depth for fog sampling
	copySceneDepth(cmd);

	// Transition scene color: eShaderReadOnlyOptimal -> eColorAttachmentOptimal
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_sceneColor->image;
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, nullptr, nullptr, barrier);
	}

	// Map bloom UBO for fog UBO data
	m_ctx->scratchUBOMapped = m_ctx->memoryManager->mapMemory(m_ctx->scratchUBOAlloc);
	Verify(m_ctx->scratchUBOMapped);

	// Fill fog UBO
	graphics::generic_data::fog_data fogData;
	{
		float fog_near, fog_density;
		neb2_get_adjusted_fog_values(&fog_near, &fog_density);
		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);

		fogData.fog_start = fog_near;
		fogData.fog_density = fog_density;
		fogData.fog_color.xyz.x = r / 255.f;
		fogData.fog_color.xyz.y = g / 255.f;
		fogData.fog_color.xyz.z = b / 255.f;
		fogData.zNear = Min_draw_distance;
		fogData.zFar = Max_draw_distance;
		fogData.clip_dist = Neb2_fog_clip_distance;
		fogData.clip_inf_dist = Neb2_fog_skybox_clip_distance;
	}

	// Custom descriptor writes to bind depth copy at binding 4
	PipelineConfig config;
	config.shaderType = SDR_TYPE_SCENE_FOG;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = m_fogRenderPass;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		m_ctx->memoryManager->unmapMemory(m_ctx->scratchUBOAlloc);
		m_ctx->scratchUBOMapped = nullptr;
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Begin render pass
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = m_fogRenderPass;
	rpBegin.framebuffer = m_fogFramebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_ctx->sceneExtent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_ctx->sceneExtent.width);
	viewport.height = static_cast<float>(m_ctx->sceneExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_ctx->sceneExtent;
	cmd.setScissor(0, scissor);

	DescriptorWriter writer;
	writer.reset(m_ctx->device, descriptorMgr->getFallbacks());

	// Set 1: Material
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		texArrayInfos[0] = {m_ctx->linearSampler, m_gbuffer->compositeView(), vk::ImageLayout::eShaderReadOnlyOptimal};
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}
	writer.setImage(MaterialBinding::DepthMap, {m_ctx->linearSampler, m_sceneDepthCopy->view, vk::ImageLayout::eShaderReadOnlyOptimal});

	// Set 2: PerDraw — fog UBO
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
	{
		Assertion(m_ctx->scratchUBOCursor < PostProcessContext::SCRATCH_UBO_MAX_SLOTS, "Fog UBO slot overflow!");
		uint32_t slotOffset = m_ctx->scratchUBOCursor * static_cast<uint32_t>(PostProcessContext::SCRATCH_UBO_SLOT_SIZE);
		memcpy(static_cast<uint8_t*>(m_ctx->scratchUBOMapped) + slotOffset, &fogData, sizeof(fogData));
		m_ctx->scratchUBOCursor++;
		writer.setBuffer(PerDrawBinding::GenericData, {m_ctx->scratchUBO, slotOffset, PostProcessContext::SCRATCH_UBO_SLOT_SIZE});
	}
	writer.flush();

	// Bind descriptor sets and draw
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();

	// Scene color is now in eColorAttachmentOptimal (fog render pass finalLayout)

	m_ctx->memoryManager->unmapMemory(m_ctx->scratchUBOAlloc);
	m_ctx->scratchUBOMapped = nullptr;
}

void VulkanFog::renderVolumetric(vk::CommandBuffer cmd)
{
	GR_DEBUG_SCOPE("Volumetric Nebulae");
	TRACE_SCOPE(tracing::Volumetrics);

	if (!m_fogInitialized) {
		if (!initFogPass()) {
			return;
		}
	}

	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	auto* bufferMgr = getBufferManager();
	auto* texMgr = getTextureManager();

	if (!pipelineMgr || !descriptorMgr || !bufferMgr || !texMgr) {
		return;
	}

	const volumetric_nebula& neb = *The_mission.volumetrics;
	Assertion(neb.isVolumeBitmapValid(), "Volumetric nebula was not properly initialized!");

	// Get 3D texture handles
	int volHandle = neb.getVolumeBitmapHandle();
	auto* volSlot = texMgr->getTextureSlot(volHandle);
	if (!volSlot || !volSlot->imageView) {
		mprintf(("VulkanFog::renderVolumetric: Volume texture not available\n"));
		return;
	}

	bool noiseActive = neb.getNoiseActive();
	tcache_slot_vulkan* noiseSlot = nullptr;
	if (noiseActive) {
		int noiseHandle = neb.getNoiseVolumeBitmapHandle();
		noiseSlot = texMgr->getTextureSlot(noiseHandle);
	}

	// Prepare mipmapped emissive copy for LOD sampling
	if (!m_emissiveMipmappedInitialized) {
		m_emissiveMipLevels = 1;
		uint32_t dim = std::max(m_ctx->sceneExtent.width, m_ctx->sceneExtent.height);
		while (dim > 1) {
			dim >>= 1;
			m_emissiveMipLevels++;
		}

		vk::ImageCreateInfo imgInfo;
		imgInfo.imageType = vk::ImageType::e2D;
		imgInfo.format = HDR_COLOR_FORMAT;
		imgInfo.extent = vk::Extent3D(m_ctx->sceneExtent.width, m_ctx->sceneExtent.height, 1);
		imgInfo.mipLevels = m_emissiveMipLevels;
		imgInfo.arrayLayers = 1;
		imgInfo.samples = vk::SampleCountFlagBits::e1;
		imgInfo.tiling = vk::ImageTiling::eOptimal;
		imgInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
		              | vk::ImageUsageFlagBits::eSampled;
		imgInfo.sharingMode = vk::SharingMode::eExclusive;
		imgInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_emissiveMipmapped.image = m_ctx->device.createImage(imgInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create mipmapped emissive: %s\n", e.what()));
			return;
		}

		Verify(m_ctx->memoryManager->allocateImageMemory(m_emissiveMipmapped.image, MemoryUsage::GpuOnly, m_emissiveMipmapped.allocation));

		// Create full-mip-chain view for LOD sampling
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_emissiveMipmapped.image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = HDR_COLOR_FORMAT;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = m_emissiveMipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		try {
			m_emissiveMipmappedFullView = m_ctx->device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create mipmapped emissive view: %s\n", e.what()));
			return;
		}

		m_emissiveMipmapped.format = HDR_COLOR_FORMAT;
		m_emissiveMipmapped.width = m_ctx->sceneExtent.width;
		m_emissiveMipmapped.height = m_ctx->sceneExtent.height;
		m_emissiveMipmappedInitialized = true;
	}

	// Copy G-buffer emissive (mip 0) to mipmapped emissive, then generate mips.
	// dstMipLevels transitions ALL mip levels to eTransferDstOptimal in the pre-barrier.
	// Skip dst post-barrier (stays in eTransferDstOptimal for generateMipmaps).
	copyImageToImage(cmd,
		m_gbuffer->emissiveImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_emissiveMipmapped.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
		m_ctx->sceneExtent,
		vk::ImageAspectFlagBits::eColor,
		m_emissiveMipLevels);

	// Generate mipmaps via blit chain (expects dst in eTransferDstOptimal).
	// After return, all mips are in eShaderReadOnlyOptimal.
	PostProcessContext::generateMipmaps(cmd, m_emissiveMipmapped.image, m_ctx->sceneExtent.width, m_ctx->sceneExtent.height, m_emissiveMipLevels);

	// Copy scene depth (if not already done by renderSceneFog)
	// copySceneDepth is safe to call multiple times — but it re-transitions the depth buffer.
	// The fog pass already called it if scene fog ran. For standalone volumetric, we need it.
	copySceneDepth(cmd);

	// Transition scene color → eColorAttachmentOptimal for the fog render pass.
	// oldLayout=eUndefined is safe: render pass has loadOp=eDontCare (overwrites every pixel).
	// Scene color may be in eShaderReadOnlyOptimal (volumetric-only) or
	// eColorAttachmentOptimal (after scene fog + copySceneColorToComposite).
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_sceneColor->image;
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, nullptr, nullptr, barrier);
	}

	// Map bloom UBO for volumetric fog UBO data
	m_ctx->scratchUBOMapped = m_ctx->memoryManager->mapMemory(m_ctx->scratchUBOAlloc);
	Verify(m_ctx->scratchUBOMapped);

	// Fill volumetric fog UBO
	graphics::generic_data::volumetric_fog_data volData;
	{
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
		vm_inverse_matrix4(&volData.p_inv, &gr_projection_matrix);
		vm_inverse_matrix4(&volData.v_inv, &gr_view_matrix);
		gr_end_view_matrix();
		gr_end_proj_matrix();

		volData.zNear = Min_draw_distance;
		volData.zFar = Max_draw_distance;
		volData.cameraPos = Eye_position;

		// Find first directional light for global light direction/color
		vec3d global_light_dir = ZERO_VECTOR;
		vec3d global_light_diffuse = ZERO_VECTOR;
		for (const auto& l : Lights) {
			if (l.type == Light_Type::Directional) {
				global_light_dir = l.vec;
				global_light_diffuse.xyz.x = l.r * l.intensity;
				global_light_diffuse.xyz.y = l.g * l.intensity;
				global_light_diffuse.xyz.z = l.b * l.intensity;
				break;
			}
		}

		volData.globalLightDirection = global_light_dir;
		volData.globalLightDiffuse = global_light_diffuse;
		volData.nebPos = neb.getPos();
		volData.nebSize = neb.getSize();
		volData.stepsize = neb.getStepsize();
		volData.opacitydistance = neb.getOpacityDistance();
		volData.alphalimit = neb.getAlphaLim();
		auto nebColor = neb.getNebulaColor();
		volData.nebColor[0] = std::get<0>(nebColor);
		volData.nebColor[1] = std::get<1>(nebColor);
		volData.nebColor[2] = std::get<2>(nebColor);
		volData.udfScale = neb.getUDFScale();
		volData.emissiveSpreadFactor = neb.getEmissiveSpread();
		volData.emissiveIntensity = neb.getEmissiveIntensity();
		volData.emissiveFalloff = neb.getEmissiveFalloff();
		volData.henyeyGreensteinCoeff = neb.getHenyeyGreensteinCoeff();
		volData.directionalLightSampleSteps = neb.getGlobalLightSteps();
		volData.directionalLightStepSize = neb.getGlobalLightStepsize();
		auto noiseColor = neb.getNoiseColor();
		volData.noiseColor[0] = std::get<0>(noiseColor);
		volData.noiseColor[1] = std::get<1>(noiseColor);
		volData.noiseColor[2] = std::get<2>(noiseColor);
		auto noiseScale = neb.getNoiseColorScale();
		volData.noiseColorScale1 = std::get<0>(noiseScale);
		volData.noiseColorScale2 = std::get<1>(noiseScale);
		volData.noiseColorIntensity = neb.getNoiseColorIntensity();
		volData.aspect = gr_screen.clip_aspect;
		volData.fov = g3_get_hfov(Proj_fov);
	}

	// Compute shader flags for volumetric fog variants
	unsigned int volFogFlags = 0;
	if (neb.getEdgeSmoothing()) {
		volFogFlags |= SDR_FLAG_VOLUMETRICS_DO_EDGE_SMOOTHING;
	}
	if (noiseActive) {
		volFogFlags |= SDR_FLAG_VOLUMETRICS_NOISE;
	}

	// We need to use a custom descriptor write because the volumetric shader uses sampler3D
	// at bindings 5 and 6, which differs from the default drawFullscreenTriangle fallbacks (sampler2D).
	// So we replicate the drawFullscreenTriangle pattern but customize the material set.

	PipelineConfig config;
	config.shaderType = SDR_TYPE_VOLUMETRIC_FOG;
	config.shaderFlags = volFogFlags;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = m_fogRenderPass;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		m_ctx->memoryManager->unmapMemory(m_ctx->scratchUBOAlloc);
		m_ctx->scratchUBOMapped = nullptr;
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Begin render pass
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = m_fogRenderPass;
	rpBegin.framebuffer = m_fogFramebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_ctx->sceneExtent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_ctx->sceneExtent.width);
	viewport.height = static_cast<float>(m_ctx->sceneExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_ctx->sceneExtent;
	cmd.setScissor(0, scissor);

	DescriptorWriter writer;
	writer.reset(m_ctx->device, descriptorMgr->getFallbacks());

	// Set 1: Material
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		texArrayInfos[0] = {m_ctx->linearSampler, m_gbuffer->compositeView(), vk::ImageLayout::eShaderReadOnlyOptimal};
		texArrayInfos[1] = {m_ctx->mipmapSampler, m_emissiveMipmappedFullView, vk::ImageLayout::eShaderReadOnlyOptimal};
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}
	writer.setImage(MaterialBinding::DepthMap, {m_ctx->linearSampler, m_sceneDepthCopy->view, vk::ImageLayout::eShaderReadOnlyOptimal});
	// Binding 5: 3D volume texture (reuses SceneColor slot)
	writer.setImage(MaterialBinding::SceneColor, {m_ctx->linearSampler, volSlot->imageView, vk::ImageLayout::eShaderReadOnlyOptimal});
	// Binding 6: 3D noise texture (or fallback 3D)
	{
		auto noiseInfo = descriptorMgr->getFallbacks().texture3D;
		noiseInfo.sampler = m_ctx->linearSampler;
		if (noiseSlot && noiseSlot->imageView) {
			noiseInfo.imageView = noiseSlot->imageView;
		}
		writer.setImage(MaterialBinding::DistortionMap, noiseInfo);
	}

	// Set 2: PerDraw — volumetric fog UBO
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
	{
		Assertion(m_ctx->scratchUBOCursor < PostProcessContext::SCRATCH_UBO_MAX_SLOTS, "Fog UBO slot overflow!");
		uint32_t slotOffset = m_ctx->scratchUBOCursor * static_cast<uint32_t>(PostProcessContext::SCRATCH_UBO_SLOT_SIZE);
		memcpy(static_cast<uint8_t*>(m_ctx->scratchUBOMapped) + slotOffset, &volData, sizeof(volData));
		m_ctx->scratchUBOCursor++;
		writer.setBuffer(PerDrawBinding::GenericData, {m_ctx->scratchUBO, slotOffset, PostProcessContext::SCRATCH_UBO_SLOT_SIZE});
	}
	writer.flush();

	// Bind descriptor sets and draw
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();

	// Scene color is now in eColorAttachmentOptimal (fog render pass finalLayout)

	m_ctx->memoryManager->unmapMemory(m_ctx->scratchUBOAlloc);
	m_ctx->scratchUBOMapped = nullptr;
}

} // namespace graphics::vulkan
