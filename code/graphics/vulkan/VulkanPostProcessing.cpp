#include "VulkanPostProcessing.h"

#include <array>

#include "cmdline/cmdline.h"
#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanState.h"
#include "VulkanDescriptorManager.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/post_processing.h"
#include "graphics/grinternal.h"
#include "graphics/shadows.h"
#include "graphics/2d.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "math/vecmat.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {

// Global post-processor pointer
static VulkanPostProcessor* g_postProcessor = nullptr;

VulkanPostProcessor* getPostProcessor()
{
	return g_postProcessor;
}

void setPostProcessor(VulkanPostProcessor* pp)
{
	g_postProcessor = pp;
}

bool VulkanPostProcessor::init(vk::Device device, vk::PhysicalDevice physDevice,
                               VulkanMemoryManager* memMgr, vk::Extent2D extent,
                               vk::Format depthFormat, bool hdrActive)
{
	if (m_initialized) {
		return true;
	}

	m_ctx.device = device;
	m_ctx.memoryManager = memMgr;
	m_ctx.sceneExtent = extent;
	m_ctx.depthFormat = depthFormat;
	m_ctx.hdrActive = hdrActive;
	// In HDR the tonemapped scene must be able to exceed paper white, so use fp16.
	m_ctx.ldrFormat = hdrActive ? HDR_COLOR_FORMAT : LDR_COLOR_FORMAT;

	// Verify RGBA16F support for color attachment + sampling
	{
		vk::FormatProperties props = physDevice.getFormatProperties(HDR_COLOR_FORMAT);
		if (!(props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment) ||
		    !(props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage)) {
			nprintf(("vulkan", "VulkanPostProcessor: RGBA16F not supported for color attachment + sampling!\n"));
			return false;
		}
	}

	// Create HDR scene color target (RGBA16F)
	// eTransferSrc needed for copy_effect_texture (mid-scene snapshot)
	// eTransferDst needed for deferred_lighting_finish (emissive→color copy)
	if (!createImage(extent.width, extent.height, HDR_COLOR_FORMAT,
	                 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
	                 | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneColor.image, m_sceneColor.view, m_sceneColor.allocation)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create scene color image!\n"));
		return false;
	}
	m_sceneColor.format = HDR_COLOR_FORMAT;
	m_sceneColor.width = extent.width;
	m_sceneColor.height = extent.height;

	// Create scene depth target
	if (!createImage(extent.width, extent.height, depthFormat,
	                 vk::ImageUsageFlagBits::eDepthStencilAttachment
	                 | vk::ImageUsageFlagBits::eSampled
	                 | vk::ImageUsageFlagBits::eTransferSrc,
	                 vk::ImageAspectFlagBits::eDepth,  // View uses depth-only aspect
	                 m_sceneDepth.image, m_sceneDepth.view, m_sceneDepth.allocation)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create scene depth image!\n"));
		shutdown();
		return false;
	}
	m_sceneDepth.format = depthFormat;
	m_sceneDepth.width = extent.width;
	m_sceneDepth.height = extent.height;

	// Create effect/composite texture (RGBA16F, snapshot of scene color for distortion/soft particles)
	if (!createImage(extent.width, extent.height, HDR_COLOR_FORMAT,
	                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneEffect.image, m_sceneEffect.view, m_sceneEffect.allocation)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create scene effect image!\n"));
		shutdown();
		return false;
	}
	m_sceneEffect.format = HDR_COLOR_FORMAT;
	m_sceneEffect.width = extent.width;
	m_sceneEffect.height = extent.height;

	// Create scene depth copy (samplable copy for soft particles)
	// Same depth format, usage: eTransferDst (copy target) + eSampled (fragment shader reads)
	if (!createImage(extent.width, extent.height, depthFormat,
	                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eDepth,
	                 m_sceneDepthCopy.image, m_sceneDepthCopy.view, m_sceneDepthCopy.allocation)) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create scene depth copy image!\n"));
		shutdown();
		return false;
	}
	m_sceneDepthCopy.format = depthFormat;
	m_sceneDepthCopy.width = extent.width;
	m_sceneDepthCopy.height = extent.height;

	// Create HDR scene render pass
	// Attachment 0: Color (RGBA16F)
	//   loadOp=eClear: clear to black each frame
	//   finalLayout=eShaderReadOnlyOptimal: ready for post-processing sampling
	// Attachment 1: Depth
	//   loadOp=eClear: clear to far plane
	//   finalLayout=eDepthStencilAttachmentOptimal
	{
		std::array<vk::AttachmentDescription, 2> attachments;

		// Color
		attachments[0].format = HDR_COLOR_FORMAT;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		// Depth — storeOp=eStore required for:
		// 1. copy_effect_texture mid-scene interruption (depth must survive render pass end/resume)
		// 2. lightshafts pass (samples scene depth after render pass ends)
		attachments[1].format = depthFormat;
		attachments[1].samples = vk::SampleCountFlagBits::e1;
		attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eClear;
		attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eStore;
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

		// Dependency: external → subpass 0
		// Includes eTransfer in srcStageMask so this render pass is compatible with
		// m_sceneRenderPassLoad (which follows copy_effect_texture transfer ops).
		// Vulkan requires render passes sharing a framebuffer to have identical dependencies.
		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests
		                        | vk::PipelineStageFlagBits::eTransfer;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentRead;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_sceneRenderPass = m_ctx.device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create scene render pass: %s\n", e.what()));
			shutdown();
			return false;
		}
	}

	// Create scene render pass with loadOp=eLoad (for resuming after copy_effect_texture)
	// Compatible with m_sceneRenderPass (same formats/samples) so shares the same framebuffer
	{
		std::array<vk::AttachmentDescription, 2> attachments;

		// Color — load existing content, keep final layout for post-processing
		attachments[0].format = HDR_COLOR_FORMAT;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
		attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		// Depth — load existing content
		attachments[1].format = depthFormat;
		attachments[1].samples = vk::SampleCountFlagBits::e1;
		attachments[1].loadOp = vk::AttachmentLoadOp::eLoad;
		attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eLoad;
		attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eStore;
		attachments[1].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
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

		// Must match m_sceneRenderPass dependency exactly for render pass compatibility
		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests
		                        | vk::PipelineStageFlagBits::eTransfer;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentRead;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_sceneRenderPassLoad = m_ctx.device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create scene load render pass: %s\n", e.what()));
			shutdown();
			return false;
		}
	}

	// Create scene framebuffer
	{
		std::array<vk::ImageView, 2> fbAttachments = {m_sceneColor.view, m_sceneDepth.view};

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_sceneRenderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		fbInfo.pAttachments = fbAttachments.data();
		fbInfo.width = extent.width;
		fbInfo.height = extent.height;
		fbInfo.layers = 1;

		try {
			m_sceneFramebuffer = m_ctx.device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create scene framebuffer: %s\n", e.what()));
			shutdown();
			return false;
		}
	}

	// Create linear sampler for post-processing texture reads
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;

		try {
			m_ctx.linearSampler = m_ctx.device.createSampler(samplerInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create sampler: %s\n", e.what()));
			shutdown();
			return false;
		}
	}

	// Create mipmap sampler for bloom textures (supports textureLod)
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(VulkanBloom::MAX_MIP_BLUR_LEVELS);
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;

		try {
			m_ctx.mipmapSampler = m_ctx.device.createSampler(samplerInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create mipmap sampler: %s\n", e.what()));
			shutdown();
			return false;
		}
	}

	// Create persistent UBO for tonemapping parameters
	{
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = sizeof(graphics::generic_data::tonemapping_data);
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_tonemapUBO = m_ctx.device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to create tonemap UBO: %s\n", e.what()));
			shutdown();
			return false;
		}

		if (!m_ctx.memoryManager->allocateBufferMemory(m_tonemapUBO, MemoryUsage::CpuToGpu, m_tonemapUBOAlloc)) {
			nprintf(("vulkan", "VulkanPostProcessor: Failed to allocate tonemap UBO memory!\n"));
			m_ctx.device.destroyBuffer(m_tonemapUBO);
			m_tonemapUBO = nullptr;
			shutdown();
			return false;
		}

		// Write default passthrough tonemapping data (linear, exposure=1.0)
		auto* mapped = static_cast<graphics::generic_data::tonemapping_data*>(m_ctx.memoryManager->mapMemory(m_tonemapUBOAlloc));
		if (mapped) {
			memset(mapped, 0, sizeof(graphics::generic_data::tonemapping_data));
			mapped->exposure = 1.0f;
			mapped->tonemapper = 0;  // Linear
			m_ctx.memoryManager->unmapMemory(m_tonemapUBOAlloc);
		}
	}

	// Create persistent UBO for the final output-encode pass (HDR10 leg only;
	// the SDR leg is a blit or the UBO-less SDR_TYPE_COPY fallback)
	{
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = sizeof(graphics::generic_data::hdr10_encode_data);
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_outputEncodeUBO = m_ctx.device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create output-encode UBO: %s\n", e.what()));
			shutdown();
			return false;
		}

		if (!m_ctx.memoryManager->allocateBufferMemory(m_outputEncodeUBO, MemoryUsage::CpuToGpu, m_outputEncodeUBOAlloc)) {
			mprintf(("VulkanPostProcessor: Failed to allocate output-encode UBO memory!\n"));
			m_ctx.device.destroyBuffer(m_outputEncodeUBO);
			m_outputEncodeUBO = nullptr;
			shutdown();
			return false;
		}
	}

	// Create the shared scratch UBO (used by every subsystem's drawFullscreenTriangle)
	if (!m_ctx.initScratchUBO()) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to create scratch UBO!\n"));
		shutdown();
		return false;
	}

	// Initialize bloom resources (non-fatal if it fails)
	if (!initBloom()) {
		nprintf(("vulkan", "VulkanPostProcessor: Bloom initialization failed (non-fatal)\n"));
	}

	// Initialize LDR targets for tonemapping + FXAA (non-fatal if it fails)
	if (!initLDRTargets()) {
		nprintf(("vulkan", "VulkanPostProcessor: LDR target initialization failed (non-fatal)\n"));
	}

	// Initialize SMAA resources (non-fatal if it fails; requires LDR targets)
	if (m_ldrInitialized && !initSMAA()) {
		nprintf(("vulkan", "VulkanPostProcessor: SMAA initialization failed (non-fatal)\n"));
	}

	// Initialize distortion ping-pong textures (non-fatal if it fails)
	m_distortion.init(m_ctx);

	// Initialize G-buffer for deferred lighting (non-fatal)
	if (!initGBuffer()) {
		nprintf(("vulkan", "VulkanPostProcessor: G-buffer initialization failed (non-fatal)\n"));
	}

	// Wire the deferred light accumulation + fog subsystems (resources are lazy).
	m_lighting.init(m_ctx, m_sceneColor, m_deferred, m_shadow);
	m_fog.init(m_ctx, m_sceneColor, m_sceneDepth, m_sceneDepthCopy, m_deferred);

	// Initialize MSAA resources if MSAA is enabled and G-buffer is ready
	if (m_deferred.isInitialized() && Cmdline_msaa_enabled > 0) {
		if (!initMSAA()) {
			nprintf(("vulkan", "VulkanPostProcessor: MSAA initialization failed (non-fatal, disabling MSAA)\n"));
			Cmdline_msaa_enabled = 0;
		}
	}

	m_initialized = true;
	nprintf(("vulkan", "VulkanPostProcessor: Initialized (%ux%u, RGBA16F scene color)\n",
		extent.width, extent.height));
	return true;
}

void VulkanPostProcessor::shutdown()
{
	if (m_ctx.device) {
		m_ctx.device.waitIdle();

		m_fog.shutdown();
		shutdownShadowPass();
		shadow_cascade_params_shutdown();
		shutdownMSAA();
		m_lighting.shutdown();
		shutdownGBuffer();
		shutdownSMAA();
		shutdownLDRTargets();
		shutdownBloom();

		m_ctx.shutdownScratchUBO();

		if (m_ctx.mipmapSampler) {
			m_ctx.device.destroySampler(m_ctx.mipmapSampler);
			m_ctx.mipmapSampler = nullptr;
		}

		if (m_tonemapUBO) {
			m_ctx.device.destroyBuffer(m_tonemapUBO);
			m_tonemapUBO = nullptr;
		}
		if (m_tonemapUBOAlloc.isValid()) {
			m_ctx.memoryManager->freeAllocation(m_tonemapUBOAlloc);
		}

		if (m_outputEncodeUBO) {
			m_ctx.device.destroyBuffer(m_outputEncodeUBO);
			m_outputEncodeUBO = nullptr;
		}
		if (m_outputEncodeUBOAlloc.isValid()) {
			m_ctx.memoryManager->freeAllocation(m_outputEncodeUBOAlloc);
		}

		if (m_ctx.linearSampler) {
			m_ctx.device.destroySampler(m_ctx.linearSampler);
			m_ctx.linearSampler = nullptr;
		}
		if (m_sceneFramebuffer) {
			m_ctx.device.destroyFramebuffer(m_sceneFramebuffer);
			m_sceneFramebuffer = nullptr;
		}
		if (m_sceneRenderPassLoad) {
			m_ctx.device.destroyRenderPass(m_sceneRenderPassLoad);
			m_sceneRenderPassLoad = nullptr;
		}
		if (m_sceneRenderPass) {
			m_ctx.device.destroyRenderPass(m_sceneRenderPass);
			m_sceneRenderPass = nullptr;
		}

		// Destroy scene effect/composite target
		if (m_sceneEffect.view) {
			m_ctx.device.destroyImageView(m_sceneEffect.view);
			m_sceneEffect.view = nullptr;
		}
		if (m_sceneEffect.image) {
			m_ctx.device.destroyImage(m_sceneEffect.image);
			m_sceneEffect.image = nullptr;
		}
		if (m_sceneEffect.allocation.isValid()) {
			m_ctx.memoryManager->freeAllocation(m_sceneEffect.allocation);
		}

		// Destroy scene color target
		if (m_sceneColor.view) {
			m_ctx.device.destroyImageView(m_sceneColor.view);
			m_sceneColor.view = nullptr;
		}
		if (m_sceneColor.image) {
			m_ctx.device.destroyImage(m_sceneColor.image);
			m_sceneColor.image = nullptr;
		}
		if (m_sceneColor.allocation.isValid()) {
			m_ctx.memoryManager->freeAllocation(m_sceneColor.allocation);
		}

		// Destroy scene depth target
		if (m_sceneDepth.view) {
			m_ctx.device.destroyImageView(m_sceneDepth.view);
			m_sceneDepth.view = nullptr;
		}
		if (m_sceneDepth.image) {
			m_ctx.device.destroyImage(m_sceneDepth.image);
			m_sceneDepth.image = nullptr;
		}
		if (m_sceneDepth.allocation.isValid()) {
			m_ctx.memoryManager->freeAllocation(m_sceneDepth.allocation);
		}

		// Destroy scene depth copy target
		if (m_sceneDepthCopy.view) {
			m_ctx.device.destroyImageView(m_sceneDepthCopy.view);
			m_sceneDepthCopy.view = nullptr;
		}
		if (m_sceneDepthCopy.image) {
			m_ctx.device.destroyImage(m_sceneDepthCopy.image);
			m_sceneDepthCopy.image = nullptr;
		}
		if (m_sceneDepthCopy.allocation.isValid()) {
			m_ctx.memoryManager->freeAllocation(m_sceneDepthCopy.allocation);
		}

		// Destroy distortion textures
		m_distortion.shutdown();
	}

	m_initialized = false;
}

void VulkanPostProcessor::updateTonemappingUBO()
{
	if (!m_tonemapUBO || !m_ctx.memoryManager) {
		return;
	}

	namespace ltp = lighting_profiles;

	auto* mapped = static_cast<graphics::generic_data::tonemapping_data*>(
		m_ctx.memoryManager->mapMemory(m_tonemapUBOAlloc));
	if (mapped) {
		auto ppc = ltp::current_piecewise_intermediates();
		mapped->exposure = ltp::current_exposure();
		mapped->tonemapper = static_cast<int>(ltp::current_tonemapper());
		mapped->x0 = ppc.x0;
		mapped->y0 = ppc.y0;
		mapped->x1 = ppc.x1;
		mapped->toe_B = ppc.toe_B;
		mapped->toe_lnA = ppc.toe_lnA;
		mapped->sh_B = ppc.sh_B;
		mapped->sh_lnA = ppc.sh_lnA;
		mapped->sh_offsetX = ppc.sh_offsetX;
		mapped->sh_offsetY = ppc.sh_offsetY;
		mapped->hdr_paperwhite_nits = 0.0f;
		mapped->hdr_peak_nits = 0.0f;
		m_ctx.memoryManager->unmapMemory(m_tonemapUBOAlloc);
	}
}

void VulkanPostProcessor::copyEffectTexture(vk::CommandBuffer cmd) const
{
	// Called mid-scene, outside a render pass.
	// Scene color is in eShaderReadOnlyOptimal (from the ended scene render pass).
	// Copies scene color → effect texture so distortion/soft particle shaders can sample it.
	copyImageToImage(cmd,
		m_sceneColor.image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
		m_sceneEffect.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_ctx.sceneExtent);
}

void VulkanPostProcessor::copySceneDepth(vk::CommandBuffer cmd) const
{
	// Called mid-scene, outside a render pass.
	// Copies scene depth → depth copy texture so soft particle shaders can sample it.
	// Scene depth is in eDepthStencilAttachmentOptimal (from the ended scene render pass).
	copyImageToImage(cmd,
		m_sceneDepth.image, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
		m_sceneDepthCopy.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_ctx.sceneExtent,
		imageAspectFromFormat(m_ctx.depthFormat));
}

void VulkanPostProcessor::blitToSwapChain(vk::CommandBuffer cmd)
{
	// If LDR targets exist, executeTonemap()+executeFXAA() already ran.
	// Blit from the latest post-processing result with passthrough settings.
	// Otherwise, fall back to direct HDR→swap chain tonemapping.
	bool useLdr = m_ldrInitialized;

	if (!useLdr) {
		// Update tonemapping parameters from engine lighting profile
		updateTonemappingUBO();
	}

	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	auto* stateTracker = getStateTracker();
	auto* bufferMgr = getBufferManager();

	if (!pipelineMgr || !descriptorMgr || !stateTracker || !bufferMgr) {
		return;
	}

	GR_DEBUG_SCOPE("Draw scene texture");

	// Build pipeline config for tonemapping (fullscreen, no depth, no blending)
	PipelineConfig config;
	config.shaderType = SDR_TYPE_POST_PROCESS_TONEMAPPING;
	config.shaderFlags = useLdr ? SDR_FLAG_TONEMAPPING_LINEAR_OUT : 0;
	config.vertexLayoutHash = 0;  // Empty vertex layout
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = stateTracker->getCurrentRenderPass();

	// Get or create the pipeline
	vertex_layout emptyLayout;  // No vertex components
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		nprintf(("vulkan", "VulkanPostProcessor: Failed to get tonemapping pipeline!\n"));
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();
	stateTracker->bindPipeline(pipeline, pipelineLayout);

	// Set viewport (non-flipped for post-processing — textures are already
	// in the correct Vulkan orientation, no Y-flip needed)
	stateTracker->setViewport(0.0f, 0.0f,
		static_cast<float>(m_ctx.sceneExtent.width),
		static_cast<float>(m_ctx.sceneExtent.height));

	stateTracker->applyDynamicState();

	DescriptorWriter writer;
	writer.reset(m_ctx.device, descriptorMgr->getFallbacks());

	// Set 1: Material — source texture at array slot 0
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		texArrayInfos[0].sampler = m_ctx.linearSampler;
		texArrayInfos[0].imageView = m_postEffectsApplied ? m_sceneLuminance.view
		                            : useLdr ? m_sceneLdr.view
		                            : m_sceneColor.view;
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}

	// Set 2: PerDraw — tonemapping UBO
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));

	// When blitting LDR, use passthrough tonemapping (exposure=1, linear)
	if (useLdr) {
		auto* mapped = static_cast<graphics::generic_data::tonemapping_data*>(
			m_ctx.memoryManager->mapMemory(m_tonemapUBOAlloc));
		Verify(mapped);
		memset(mapped, 0, sizeof(graphics::generic_data::tonemapping_data));
		mapped->exposure = 1.0f;
		mapped->tonemapper = 0;  // Linear passthrough
		m_ctx.memoryManager->unmapMemory(m_tonemapUBOAlloc);
	}
	writer.setBuffer(PerDrawBinding::GenericData, {m_tonemapUBO, 0,
		sizeof(graphics::generic_data::tonemapping_data)});
	writer.flush();
	stateTracker->bindDescriptorSet(DescriptorSetIndex::Material, materialSet);
	stateTracker->bindDescriptorSet(DescriptorSetIndex::PerDraw, perDrawSet);

	// Draw fullscreen triangle (3 vertices from gl_VertexIndex, no vertex buffer)
	cmd.draw(3, 1, 0, 0);
}

void VulkanPostProcessor::encodeOutput(vk::CommandBuffer cmd, vk::RenderPass renderPass,
	vk::Framebuffer framebuffer, vk::Extent2D extent, vk::ImageView sourceView, vk::Sampler sampler,
	float paperwhiteNits, float peakNits)
{
	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	if (!pipelineMgr || !descriptorMgr || !m_outputEncodeUBO) {
		return;
	}

	GR_DEBUG_SCOPE("Draw scene texture");

	// HDR10/PQ/BT.2020 encode only -- the SDR leg is handled by a blit
	// (or encodeOutputPassthrough() as a fallback) instead of a shader draw.
	PipelineConfig config;
	config.shaderType = SDR_TYPE_POST_PROCESS_HDR10_ENCODE;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = renderPass;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		mprintf(("VulkanPostProcessor: Failed to get output-encode pipeline!\n"));
		return;
	}
	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Update encode parameters
	auto* mapped = static_cast<graphics::generic_data::hdr10_encode_data*>(
		m_ctx.memoryManager->mapMemory(m_outputEncodeUBOAlloc));
	if (mapped) {
		memset(mapped, 0, sizeof(graphics::generic_data::hdr10_encode_data));
		mapped->hdr_paperwhite_nits = paperwhiteNits;
		mapped->hdr_peak_nits = peakNits;
		m_ctx.memoryManager->unmapMemory(m_outputEncodeUBOAlloc);
	}

	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = renderPass;
	rpBegin.framebuffer = framebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = extent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = extent;
	cmd.setScissor(0, scissor);

	DescriptorWriter writer;
	writer.reset(m_ctx.device, descriptorMgr->getFallbacks());

	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		texArrayInfos[0].sampler = sampler;
		texArrayInfos[0].imageView = sourceView;
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}

	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
	writer.setBuffer(PerDrawBinding::GenericData, {m_outputEncodeUBO, 0,
		sizeof(graphics::generic_data::hdr10_encode_data)});
	writer.flush();

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();
}

void VulkanPostProcessor::encodeOutputPassthrough(vk::CommandBuffer cmd, vk::RenderPass renderPass,
	vk::Framebuffer framebuffer, vk::Extent2D extent, vk::ImageView sourceView, vk::Sampler sampler)
{
	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	if (!pipelineMgr || !descriptorMgr) {
		return;
	}

	GR_DEBUG_SCOPE("Draw scene texture");

	// SDR fallback for devices that can't blit HDR_COLOR_FORMAT -> the swap
	// chain format directly (see VulkanRenderer::encodeToSwapChain()). Plain
	// passthrough copy, no UBO -- the composition image already carries the
	// final display-ready sRGB-encoded frame.
	PipelineConfig config;
	config.shaderType = SDR_TYPE_COPY;
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = ALPHA_BLEND_NONE;
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = renderPass;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		mprintf(("VulkanPostProcessor: Failed to get output-encode passthrough pipeline!\n"));
		return;
	}
	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = renderPass;
	rpBegin.framebuffer = framebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = extent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = extent;
	cmd.setScissor(0, scissor);

	DescriptorWriter writer;
	writer.reset(m_ctx.device, descriptorMgr->getFallbacks());

	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeSet(materialSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::Material));
	{
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos.fill(descriptorMgr->getFallbacks().texture2D);
		texArrayInfos[0].sampler = sampler;
		texArrayInfos[0].imageView = sourceView;
		writer.setImageArray(MaterialBinding::TextureArray, texArrayInfos);
	}

	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	writer.writeSet(perDrawSet, VulkanDescriptorManager::getSetTemplate(DescriptorSetIndex::PerDraw));
	writer.flush();

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();
}

// No-op: In OpenGL, begin/end push/pop an FBO and run the post-processing
// pipeline. In Vulkan, this is handled by vulkan_scene_texture_begin/end
// which manage the HDR render pass and post-processing passes. These
// functions are not actively called by the engine.
void vulkan_post_process_begin() {}
void vulkan_post_process_end() {}

// No-op: In OpenGL, save/restore swap the depth attachment between
// Scene_depth_texture and Cockpit_depth_texture to isolate cockpit
// depth from the main scene. In Vulkan, the render pass loadOp=eClear
// clears depth at the start of each scene pass, and separate cockpit
// depth isolation is not yet implemented. Called from ship.cpp during
// cockpit rendering but degrades gracefully as a no-op (cockpit just
// shares the scene depth buffer).
void vulkan_post_process_save_zbuffer() {}
void vulkan_post_process_restore_zbuffer() {}

void vulkan_post_process_set_effect(const char* name, int value, const vec3d* rgb)
{
	if (!Gr_post_processing_enabled || !graphics::Post_processing_manager) {
		return;
	}
	if (name == nullptr) {
		return;
	}

	auto& ls_params = graphics::Post_processing_manager->getLightshaftParams();
	if (!stricmp("lightshafts", name)) {
		ls_params.intensity = value / 100.0f;
		ls_params.on = !!value;
		return;
	}

	auto& postEffects = graphics::Post_processing_manager->getPostEffects();
	for (auto & postEffect : postEffects) {
		if (!stricmp(postEffect.name.c_str(), name)) {
			postEffect.intensity = (value / postEffect.div) + postEffect.add;
			if ((rgb != nullptr) && !(vmd_zero_vector == *rgb)) {
				postEffect.rgb = *rgb;
			}
			break;
		}
	}
}

void vulkan_post_process_set_defaults()
{
	if (!graphics::Post_processing_manager) {
		return;
	}

	auto& postEffects = graphics::Post_processing_manager->getPostEffects();
	for (auto& effect : postEffects) {
		effect.intensity = effect.default_intensity;
	}
}

} // namespace graphics::vulkan

