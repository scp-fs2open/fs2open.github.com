#include "VulkanPostProcessing.h"

#include <array>

#include "cmdline/cmdline.h"
#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanDeletionQueue.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include "VulkanState.h"
#include "VulkanDraw.h"
#include "VulkanDescriptorManager.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/util/primitives.h"
#include "graphics/post_processing.h"
#include "graphics/grinternal.h"
#include "graphics/light.h"
#include "graphics/matrix.h"
#include "graphics/shadows.h"
#include "graphics/2d.h"
#include "bmpman/bmpman.h"
#include "io/timer.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "math/floating.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "tracing/tracing.h"
#include "utils/Random.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;

namespace graphics {
namespace vulkan {

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
                               vk::Format depthFormat)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;
	m_memoryManager = memMgr;
	m_extent = extent;
	m_depthFormat = depthFormat;

	// Verify RGBA16F support for color attachment + sampling
	{
		vk::FormatProperties props = physDevice.getFormatProperties(vk::Format::eR16G16B16A16Sfloat);
		if (!(props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment) ||
		    !(props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage)) {
			mprintf(("VulkanPostProcessor: RGBA16F not supported for color attachment + sampling!\n"));
			return false;
		}
	}

	// Create HDR scene color target (RGBA16F)
	// eTransferSrc needed for copy_effect_texture (mid-scene snapshot)
	// eTransferDst needed for deferred_lighting_finish (emissive→color copy)
	if (!createImage(extent.width, extent.height, vk::Format::eR16G16B16A16Sfloat,
	                 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
	                 | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneColor.image, m_sceneColor.view, m_sceneColor.allocation)) {
		mprintf(("VulkanPostProcessor: Failed to create scene color image!\n"));
		return false;
	}
	m_sceneColor.format = vk::Format::eR16G16B16A16Sfloat;
	m_sceneColor.width = extent.width;
	m_sceneColor.height = extent.height;

	// Create scene depth target
	vk::ImageAspectFlags depthAspect = vk::ImageAspectFlagBits::eDepth;
	if (depthFormat == vk::Format::eD24UnormS8Uint || depthFormat == vk::Format::eD32SfloatS8Uint) {
		depthAspect |= vk::ImageAspectFlagBits::eStencil;
	}

	if (!createImage(extent.width, extent.height, depthFormat,
	                 vk::ImageUsageFlagBits::eDepthStencilAttachment
	                 | vk::ImageUsageFlagBits::eSampled
	                 | vk::ImageUsageFlagBits::eTransferSrc,
	                 vk::ImageAspectFlagBits::eDepth,  // View uses depth-only aspect
	                 m_sceneDepth.image, m_sceneDepth.view, m_sceneDepth.allocation)) {
		mprintf(("VulkanPostProcessor: Failed to create scene depth image!\n"));
		shutdown();
		return false;
	}
	m_sceneDepth.format = depthFormat;
	m_sceneDepth.width = extent.width;
	m_sceneDepth.height = extent.height;

	// Create effect/composite texture (RGBA16F, snapshot of scene color for distortion/soft particles)
	if (!createImage(extent.width, extent.height, vk::Format::eR16G16B16A16Sfloat,
	                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneEffect.image, m_sceneEffect.view, m_sceneEffect.allocation)) {
		mprintf(("VulkanPostProcessor: Failed to create scene effect image!\n"));
		shutdown();
		return false;
	}
	m_sceneEffect.format = vk::Format::eR16G16B16A16Sfloat;
	m_sceneEffect.width = extent.width;
	m_sceneEffect.height = extent.height;

	// Create scene depth copy (samplable copy for soft particles)
	// Same depth format, usage: eTransferDst (copy target) + eSampled (fragment shader reads)
	if (!createImage(extent.width, extent.height, depthFormat,
	                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eDepth,
	                 m_sceneDepthCopy.image, m_sceneDepthCopy.view, m_sceneDepthCopy.allocation)) {
		mprintf(("VulkanPostProcessor: Failed to create scene depth copy image!\n"));
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
		attachments[0].format = vk::Format::eR16G16B16A16Sfloat;
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
			m_sceneRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create scene render pass: %s\n", e.what()));
			shutdown();
			return false;
		}
	}

	// Create scene render pass with loadOp=eLoad (for resuming after copy_effect_texture)
	// Compatible with m_sceneRenderPass (same formats/samples) so shares the same framebuffer
	{
		std::array<vk::AttachmentDescription, 2> attachments;

		// Color — load existing content, keep final layout for post-processing
		attachments[0].format = vk::Format::eR16G16B16A16Sfloat;
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
			m_sceneRenderPassLoad = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create scene load render pass: %s\n", e.what()));
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
			m_sceneFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create scene framebuffer: %s\n", e.what()));
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
			m_linearSampler = m_device.createSampler(samplerInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create sampler: %s\n", e.what()));
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
		samplerInfo.maxLod = static_cast<float>(MAX_MIP_BLUR_LEVELS);
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;

		try {
			m_mipmapSampler = m_device.createSampler(samplerInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create mipmap sampler: %s\n", e.what()));
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
			m_tonemapUBO = m_device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create tonemap UBO: %s\n", e.what()));
			shutdown();
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_tonemapUBO, MemoryUsage::CpuToGpu, m_tonemapUBOAlloc)) {
			mprintf(("VulkanPostProcessor: Failed to allocate tonemap UBO memory!\n"));
			m_device.destroyBuffer(m_tonemapUBO);
			m_tonemapUBO = nullptr;
			shutdown();
			return false;
		}

		// Write default passthrough tonemapping data (linear, exposure=1.0)
		auto* mapped = static_cast<graphics::generic_data::tonemapping_data*>(m_memoryManager->mapMemory(m_tonemapUBOAlloc));
		if (mapped) {
			memset(mapped, 0, sizeof(graphics::generic_data::tonemapping_data));
			mapped->exposure = 1.0f;
			mapped->tonemapper = 0;  // Linear
			m_memoryManager->unmapMemory(m_tonemapUBOAlloc);
		}
	}

	// Initialize bloom resources (non-fatal if it fails)
	if (!initBloom()) {
		mprintf(("VulkanPostProcessor: Bloom initialization failed (non-fatal)\n"));
	}

	// Initialize LDR targets for tonemapping + FXAA (non-fatal if it fails)
	if (!initLDRTargets()) {
		mprintf(("VulkanPostProcessor: LDR target initialization failed (non-fatal)\n"));
	}

	// Initialize distortion ping-pong textures (non-fatal if it fails)
	{
		bool distOk = true;
		for (int i = 0; i < 2; i++) {
			if (!createImage(32, 32, vk::Format::eR8G8B8A8Unorm,
			                 vk::ImageUsageFlagBits::eTransferSrc
			                 | vk::ImageUsageFlagBits::eTransferDst
			                 | vk::ImageUsageFlagBits::eSampled,
			                 vk::ImageAspectFlagBits::eColor,
			                 m_distortionTex[i].image, m_distortionTex[i].view,
			                 m_distortionTex[i].allocation)) {
				mprintf(("VulkanPostProcessor: Failed to create distortion texture %d\n", i));
				distOk = false;
				break;
			}
			m_distortionTex[i].format = vk::Format::eR8G8B8A8Unorm;
			m_distortionTex[i].width = 32;
			m_distortionTex[i].height = 32;
		}

		if (distOk) {
			// Create LINEAR/REPEAT sampler for distortion textures
			vk::SamplerCreateInfo samplerInfo;
			samplerInfo.magFilter = vk::Filter::eLinear;
			samplerInfo.minFilter = vk::Filter::eLinear;
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
			samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
			samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
			samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;
			samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;

			try {
				m_distortionSampler = m_device.createSampler(samplerInfo);
				m_distortionInitialized = true;
				mprintf(("VulkanPostProcessor: Distortion textures initialized\n"));
			} catch (const vk::SystemError& e) {
				mprintf(("VulkanPostProcessor: Failed to create distortion sampler: %s\n", e.what()));
			}
		}
	}

	// Initialize G-buffer for deferred lighting (non-fatal)
	if (!initGBuffer()) {
		mprintf(("VulkanPostProcessor: G-buffer initialization failed (non-fatal)\n"));
	}

	// Initialize MSAA resources if MSAA is enabled and G-buffer is ready
	if (m_gbufInitialized && Cmdline_msaa_enabled > 0) {
		if (!initMSAA()) {
			mprintf(("VulkanPostProcessor: MSAA initialization failed (non-fatal, disabling MSAA)\n"));
			Cmdline_msaa_enabled = 0;
		}
	}

	m_initialized = true;
	mprintf(("VulkanPostProcessor: Initialized (%ux%u, RGBA16F scene color)\n",
		extent.width, extent.height));
	return true;
}

void VulkanPostProcessor::shutdown()
{
	if (m_device) {
		m_device.waitIdle();

		shutdownFogPass();
		shutdownShadowPass();
		shutdownMSAA();
		shutdownLightVolumes();
		shutdownGBuffer();
		shutdownLDRTargets();
		shutdownBloom();

		if (m_mipmapSampler) {
			m_device.destroySampler(m_mipmapSampler);
			m_mipmapSampler = nullptr;
		}

		if (m_tonemapUBO) {
			m_device.destroyBuffer(m_tonemapUBO);
			m_tonemapUBO = nullptr;
		}
		if (m_tonemapUBOAlloc.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(m_tonemapUBOAlloc);
		}

		if (m_linearSampler) {
			m_device.destroySampler(m_linearSampler);
			m_linearSampler = nullptr;
		}
		if (m_sceneFramebuffer) {
			m_device.destroyFramebuffer(m_sceneFramebuffer);
			m_sceneFramebuffer = nullptr;
		}
		if (m_sceneRenderPassLoad) {
			m_device.destroyRenderPass(m_sceneRenderPassLoad);
			m_sceneRenderPassLoad = nullptr;
		}
		if (m_sceneRenderPass) {
			m_device.destroyRenderPass(m_sceneRenderPass);
			m_sceneRenderPass = nullptr;
		}

		// Destroy scene effect/composite target
		if (m_sceneEffect.view) {
			m_device.destroyImageView(m_sceneEffect.view);
			m_sceneEffect.view = nullptr;
		}
		if (m_sceneEffect.image) {
			m_device.destroyImage(m_sceneEffect.image);
			m_sceneEffect.image = nullptr;
		}
		if (m_sceneEffect.allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(m_sceneEffect.allocation);
		}

		// Destroy scene color target
		if (m_sceneColor.view) {
			m_device.destroyImageView(m_sceneColor.view);
			m_sceneColor.view = nullptr;
		}
		if (m_sceneColor.image) {
			m_device.destroyImage(m_sceneColor.image);
			m_sceneColor.image = nullptr;
		}
		if (m_sceneColor.allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(m_sceneColor.allocation);
		}

		// Destroy scene depth target
		if (m_sceneDepth.view) {
			m_device.destroyImageView(m_sceneDepth.view);
			m_sceneDepth.view = nullptr;
		}
		if (m_sceneDepth.image) {
			m_device.destroyImage(m_sceneDepth.image);
			m_sceneDepth.image = nullptr;
		}
		if (m_sceneDepth.allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(m_sceneDepth.allocation);
		}

		// Destroy scene depth copy target
		if (m_sceneDepthCopy.view) {
			m_device.destroyImageView(m_sceneDepthCopy.view);
			m_sceneDepthCopy.view = nullptr;
		}
		if (m_sceneDepthCopy.image) {
			m_device.destroyImage(m_sceneDepthCopy.image);
			m_sceneDepthCopy.image = nullptr;
		}
		if (m_sceneDepthCopy.allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(m_sceneDepthCopy.allocation);
		}

		// Destroy distortion textures
		if (m_distortionSampler) {
			m_device.destroySampler(m_distortionSampler);
			m_distortionSampler = nullptr;
		}
		for (int i = 0; i < 2; i++) {
			if (m_distortionTex[i].view) {
				m_device.destroyImageView(m_distortionTex[i].view);
				m_distortionTex[i].view = nullptr;
			}
			if (m_distortionTex[i].image) {
				m_device.destroyImage(m_distortionTex[i].image);
				m_distortionTex[i].image = nullptr;
			}
			if (m_distortionTex[i].allocation.memory != VK_NULL_HANDLE) {
				m_memoryManager->freeAllocation(m_distortionTex[i].allocation);
			}
		}
		m_distortionInitialized = false;
	}

	m_initialized = false;
}

void VulkanPostProcessor::updateTonemappingUBO()
{
	if (!m_tonemapUBO || !m_memoryManager) {
		return;
	}

	namespace ltp = lighting_profiles;

	auto* mapped = static_cast<graphics::generic_data::tonemapping_data*>(
		m_memoryManager->mapMemory(m_tonemapUBOAlloc));
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
		mapped->linearOut = 0;  // Apply sRGB conversion (HDR → swap chain)
		m_memoryManager->unmapMemory(m_tonemapUBOAlloc);
	}
}

// ===== G-Buffer (Deferred Lighting) Implementation =====

bool VulkanPostProcessor::initGBuffer()
{
	if (m_gbufInitialized) {
		return true;
	}

	const uint32_t w = m_extent.width;
	const uint32_t h = m_extent.height;
	const vk::ImageUsageFlags gbufUsage =
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
		| vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

	// Create G-buffer images (position, normal, specular, emissive, composite)
	struct GbufTarget {
		RenderTarget* target;
		vk::Format format;
		const char* name;
	};

	GbufTarget targets[] = {
		{&m_gbufPosition,  vk::Format::eR16G16B16A16Sfloat, "position"},
		{&m_gbufNormal,    vk::Format::eR16G16B16A16Sfloat, "normal"},
		{&m_gbufSpecular,  vk::Format::eR8G8B8A8Unorm,      "specular"},
		{&m_gbufEmissive,  vk::Format::eR16G16B16A16Sfloat, "emissive"},
		{&m_gbufComposite, vk::Format::eR16G16B16A16Sfloat, "composite"},
	};

	for (auto& t : targets) {
		if (!createImage(w, h, t.format, gbufUsage, vk::ImageAspectFlagBits::eColor,
		                 t.target->image, t.target->view, t.target->allocation)) {
			mprintf(("VulkanPostProcessor: Failed to create G-buffer %s image!\n", t.name));
			shutdownGBuffer();
			return false;
		}
		t.target->format = t.format;
		t.target->width = w;
		t.target->height = h;
	}

	// Create samplable copy of G-buffer normal (for decal angle rejection)
	{
		vk::ImageUsageFlags copyUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		if (!createImage(w, h, vk::Format::eR16G16B16A16Sfloat, copyUsage,
		                 vk::ImageAspectFlagBits::eColor,
		                 m_gbufNormalCopy.image, m_gbufNormalCopy.view, m_gbufNormalCopy.allocation)) {
			mprintf(("VulkanPostProcessor: Failed to create G-buffer normal copy!\n"));
			shutdownGBuffer();
			return false;
		}
		m_gbufNormalCopy.format = vk::Format::eR16G16B16A16Sfloat;
		m_gbufNormalCopy.width = w;
		m_gbufNormalCopy.height = h;
	}

	// Create G-buffer render pass (eClear) — 6 color + depth
	// Attachment order: [0]=color, [1]=position, [2]=normal, [3]=specular, [4]=emissive, [5]=composite, [6]=depth
	{
		std::array<vk::AttachmentDescription, 7> attachments;

		// Formats for the 6 color attachments
		vk::Format colorFormats[6] = {
			vk::Format::eR16G16B16A16Sfloat, // 0: color (scene color)
			vk::Format::eR16G16B16A16Sfloat, // 1: position
			vk::Format::eR16G16B16A16Sfloat, // 2: normal
			vk::Format::eR8G8B8A8Unorm,      // 3: specular
			vk::Format::eR16G16B16A16Sfloat, // 4: emissive
			vk::Format::eR16G16B16A16Sfloat, // 5: composite
		};

		for (uint32_t i = 0; i < 6; ++i) {
			attachments[i].format = colorFormats[i];
			attachments[i].samples = vk::SampleCountFlagBits::e1;
			attachments[i].loadOp = vk::AttachmentLoadOp::eClear;
			attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachments[i].initialLayout = vk::ImageLayout::eUndefined;
			attachments[i].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		// Depth
		attachments[6].format = m_depthFormat;
		attachments[6].samples = vk::SampleCountFlagBits::e1;
		attachments[6].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[6].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[6].stencilLoadOp = vk::AttachmentLoadOp::eClear;
		attachments[6].stencilStoreOp = vk::AttachmentStoreOp::eStore;
		attachments[6].initialLayout = vk::ImageLayout::eUndefined;
		attachments[6].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		std::array<vk::AttachmentReference, 6> colorRefs;
		for (uint32_t i = 0; i < 6; ++i) {
			colorRefs[i].attachment = i;
			colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		vk::AttachmentReference depthRef;
		depthRef.attachment = 6;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 6;
		subpass.pColorAttachments = colorRefs.data();
		subpass.pDepthStencilAttachment = &depthRef;

		// Dependency matching the scene render pass (for render pass compatibility)
		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
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

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_gbufRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create G-buffer render pass: %s\n", e.what()));
			shutdownGBuffer();
			return false;
		}
	}

	// Create G-buffer render pass (eLoad) — for resuming after mid-pass copies
	{
		std::array<vk::AttachmentDescription, 7> attachments;

		vk::Format colorFormats[6] = {
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR16G16B16A16Sfloat,
		};

		for (uint32_t i = 0; i < 6; ++i) {
			attachments[i].format = colorFormats[i];
			attachments[i].samples = vk::SampleCountFlagBits::e1;
			attachments[i].loadOp = vk::AttachmentLoadOp::eLoad;
			attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachments[i].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
			attachments[i].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		// Depth
		attachments[6].format = m_depthFormat;
		attachments[6].samples = vk::SampleCountFlagBits::e1;
		attachments[6].loadOp = vk::AttachmentLoadOp::eLoad;
		attachments[6].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[6].stencilLoadOp = vk::AttachmentLoadOp::eLoad;
		attachments[6].stencilStoreOp = vk::AttachmentStoreOp::eStore;
		attachments[6].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		attachments[6].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		std::array<vk::AttachmentReference, 6> colorRefs;
		for (uint32_t i = 0; i < 6; ++i) {
			colorRefs[i].attachment = i;
			colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		vk::AttachmentReference depthRef;
		depthRef.attachment = 6;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 6;
		subpass.pColorAttachments = colorRefs.data();
		subpass.pDepthStencilAttachment = &depthRef;

		// Must match eClear pass dependency for compatibility
		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
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

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_gbufRenderPassLoad = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create G-buffer load render pass: %s\n", e.what()));
			shutdownGBuffer();
			return false;
		}
	}

	// Create G-buffer framebuffer (6 color + depth)
	{
		std::array<vk::ImageView, 7> fbAttachments = {
			m_sceneColor.view,      // 0: color (shared with scene framebuffer)
			m_gbufPosition.view,    // 1: position
			m_gbufNormal.view,      // 2: normal
			m_gbufSpecular.view,    // 3: specular
			m_gbufEmissive.view,    // 4: emissive
			m_gbufComposite.view,   // 5: composite
			m_sceneDepth.view,      // 6: depth (shared with scene framebuffer)
		};

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_gbufRenderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		fbInfo.pAttachments = fbAttachments.data();
		fbInfo.width = w;
		fbInfo.height = h;
		fbInfo.layers = 1;

		try {
			m_gbufFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create G-buffer framebuffer: %s\n", e.what()));
			shutdownGBuffer();
			return false;
		}
	}

	m_gbufInitialized = true;
	mprintf(("VulkanPostProcessor: G-buffer initialized (%ux%u, 6 color + depth)\n", w, h));
	return true;
}

void VulkanPostProcessor::shutdownGBuffer()
{
	if (!m_device) {
		return;
	}

	if (m_gbufFramebuffer) {
		m_device.destroyFramebuffer(m_gbufFramebuffer);
		m_gbufFramebuffer = nullptr;
	}
	if (m_gbufRenderPassLoad) {
		m_device.destroyRenderPass(m_gbufRenderPassLoad);
		m_gbufRenderPassLoad = nullptr;
	}
	if (m_gbufRenderPass) {
		m_device.destroyRenderPass(m_gbufRenderPass);
		m_gbufRenderPass = nullptr;
	}

	RenderTarget* gbufTargets[] = {
		&m_gbufPosition, &m_gbufNormal, &m_gbufSpecular,
		&m_gbufEmissive, &m_gbufComposite, &m_gbufNormalCopy,
	};
	for (auto* rt : gbufTargets) {
		if (rt->view) {
			m_device.destroyImageView(rt->view);
			rt->view = nullptr;
		}
		if (rt->image) {
			m_device.destroyImage(rt->image);
			rt->image = nullptr;
		}
		if (rt->allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(rt->allocation);
		}
	}

	m_gbufInitialized = false;
}

void VulkanPostProcessor::transitionGbufForResume(vk::CommandBuffer cmd)
{
	if (!m_gbufInitialized) {
		return;
	}

	// After ending the G-buffer render pass, color attachments 1-5 are in
	// eShaderReadOnlyOptimal (from finalLayout). The eLoad pass expects
	// eColorAttachmentOptimal. Transition them in a single barrier batch.
	vk::Image gbufImages[5] = {
		m_gbufPosition.image,
		m_gbufNormal.image,
		m_gbufSpecular.image,
		m_gbufEmissive.image,
		m_gbufComposite.image,
	};

	std::array<vk::ImageMemoryBarrier, 5> barriers;
	for (int i = 0; i < 5; ++i) {
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

// ===== MSAA G-Buffer =====

bool VulkanPostProcessor::initMSAA()
{
	if (m_msaaInitialized) {
		return true;
	}

	auto* renderer = getRendererInstance();
	vk::SampleCountFlagBits msaaSamples = renderer->getMsaaSampleCount();
	if (msaaSamples == vk::SampleCountFlagBits::e1) {
		return false;
	}

	const uint32_t w = m_extent.width;
	const uint32_t h = m_extent.height;
	const vk::ImageUsageFlags msaaUsage =
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

	// Create MSAA color images (5 total: color, position, normal, specular, emissive)
	struct MsaaTarget {
		RenderTarget* target;
		vk::Format format;
		const char* name;
	};

	MsaaTarget targets[] = {
		{&m_msaaColor,    vk::Format::eR16G16B16A16Sfloat, "msaa-color"},
		{&m_msaaPosition, vk::Format::eR16G16B16A16Sfloat, "msaa-position"},
		{&m_msaaNormal,   vk::Format::eR16G16B16A16Sfloat, "msaa-normal"},
		{&m_msaaSpecular, vk::Format::eR8G8B8A8Unorm,      "msaa-specular"},
		{&m_msaaEmissive, vk::Format::eR16G16B16A16Sfloat, "msaa-emissive"},
	};

	for (auto& t : targets) {
		if (!createImage(w, h, t.format, msaaUsage, vk::ImageAspectFlagBits::eColor,
		                 t.target->image, t.target->view, t.target->allocation, msaaSamples)) {
			mprintf(("VulkanPostProcessor: Failed to create %s image!\n", t.name));
			shutdownMSAA();
			return false;
		}
		t.target->format = t.format;
		t.target->width = w;
		t.target->height = h;
	}

	// Create MSAA depth image
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = m_depthFormat;
		imageInfo.extent = vk::Extent3D(w, h, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = msaaSamples;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_msaaDepthImage = m_device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA depth image: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}

		if (!m_memoryManager->allocateImageMemory(m_msaaDepthImage, MemoryUsage::GpuOnly, m_msaaDepthAlloc)) {
			mprintf(("VulkanPostProcessor: Failed to allocate MSAA depth memory!\n"));
			m_device.destroyImage(m_msaaDepthImage);
			m_msaaDepthImage = nullptr;
			shutdownMSAA();
			return false;
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_msaaDepthImage;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = m_depthFormat;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		try {
			m_msaaDepthView = m_device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA depth view: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// MSAA G-buffer render pass (eClear) — 5 color + depth
	// Attachment order: [0]=color(MS), [1]=pos(MS), [2]=norm(MS), [3]=spec(MS), [4]=emissive(MS), [5]=depth(MS)
	{
		std::array<vk::AttachmentDescription, 6> attachments;

		vk::Format colorFormats[5] = {
			vk::Format::eR16G16B16A16Sfloat, // 0: color
			vk::Format::eR16G16B16A16Sfloat, // 1: position
			vk::Format::eR16G16B16A16Sfloat, // 2: normal
			vk::Format::eR8G8B8A8Unorm,      // 3: specular
			vk::Format::eR16G16B16A16Sfloat, // 4: emissive
		};

		for (uint32_t i = 0; i < 5; ++i) {
			attachments[i].format = colorFormats[i];
			attachments[i].samples = msaaSamples;
			attachments[i].loadOp = vk::AttachmentLoadOp::eClear;
			attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachments[i].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
			attachments[i].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		// Depth (MS)
		attachments[5].format = m_depthFormat;
		attachments[5].samples = msaaSamples;
		attachments[5].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[5].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[5].stencilLoadOp = vk::AttachmentLoadOp::eClear;
		attachments[5].stencilStoreOp = vk::AttachmentStoreOp::eStore;
		attachments[5].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		attachments[5].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		std::array<vk::AttachmentReference, 5> colorRefs;
		for (uint32_t i = 0; i < 5; ++i) {
			colorRefs[i].attachment = i;
			colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		vk::AttachmentReference depthRef;
		depthRef.attachment = 5;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 5;
		subpass.pColorAttachments = colorRefs.data();
		subpass.pDepthStencilAttachment = &depthRef;

		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
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

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_msaaGbufRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA G-buffer render pass: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// MSAA G-buffer render pass (eLoad) — emissive preserving variant
	// All attachments eLoad except we accept eColorAttachmentOptimal as initial layout
	{
		std::array<vk::AttachmentDescription, 6> attachments;

		vk::Format colorFormats[5] = {
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR16G16B16A16Sfloat,
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR16G16B16A16Sfloat,
		};

		for (uint32_t i = 0; i < 5; ++i) {
			attachments[i].format = colorFormats[i];
			attachments[i].samples = msaaSamples;
			attachments[i].loadOp = vk::AttachmentLoadOp::eLoad;
			attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachments[i].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
			attachments[i].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		attachments[5].format = m_depthFormat;
		attachments[5].samples = msaaSamples;
		attachments[5].loadOp = vk::AttachmentLoadOp::eLoad;
		attachments[5].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[5].stencilLoadOp = vk::AttachmentLoadOp::eLoad;
		attachments[5].stencilStoreOp = vk::AttachmentStoreOp::eStore;
		attachments[5].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		attachments[5].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		std::array<vk::AttachmentReference, 5> colorRefs;
		for (uint32_t i = 0; i < 5; ++i) {
			colorRefs[i].attachment = i;
			colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		vk::AttachmentReference depthRef;
		depthRef.attachment = 5;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 5;
		subpass.pColorAttachments = colorRefs.data();
		subpass.pDepthStencilAttachment = &depthRef;

		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
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

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_msaaGbufRenderPassLoad = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA G-buffer load render pass: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// MSAA G-buffer framebuffer (5 color + depth)
	{
		std::array<vk::ImageView, 6> fbAttachments = {
			m_msaaColor.view,
			m_msaaPosition.view,
			m_msaaNormal.view,
			m_msaaSpecular.view,
			m_msaaEmissive.view,
			m_msaaDepthView,
		};

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_msaaGbufRenderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		fbInfo.pAttachments = fbAttachments.data();
		fbInfo.width = w;
		fbInfo.height = h;
		fbInfo.layers = 1;

		try {
			m_msaaGbufFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA G-buffer framebuffer: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// Emissive copy render pass — 1 MS color attachment for upsampling non-MSAA → MSAA
	{
		vk::AttachmentDescription att;
		att.format = vk::Format::eR16G16B16A16Sfloat;
		att.samples = msaaSamples;
		att.loadOp = vk::AttachmentLoadOp::eDontCare;
		att.storeOp = vk::AttachmentStoreOp::eStore;
		att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		att.initialLayout = vk::ImageLayout::eUndefined;
		att.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentReference colorRef;
		colorRef.attachment = 0;
		colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = 1;
		rpInfo.pAttachments = &att;
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_msaaEmissiveCopyRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA emissive copy render pass: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// Emissive copy framebuffer (MSAA emissive as sole attachment)
	{
		vk::ImageView att = m_msaaEmissive.view;
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_msaaEmissiveCopyRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &att;
		fbInfo.width = w;
		fbInfo.height = h;
		fbInfo.layers = 1;

		try {
			m_msaaEmissiveCopyFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA emissive copy framebuffer: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// MSAA Resolve render pass — 5 non-MSAA color + depth (via gl_FragDepth)
	// Writes to the non-MSAA G-buffer images. loadOp=eDontCare (fully overwritten).
	{
		std::array<vk::AttachmentDescription, 6> attachments;

		vk::Format colorFormats[5] = {
			vk::Format::eR16G16B16A16Sfloat, // 0: color
			vk::Format::eR16G16B16A16Sfloat, // 1: position
			vk::Format::eR16G16B16A16Sfloat, // 2: normal
			vk::Format::eR8G8B8A8Unorm,      // 3: specular
			vk::Format::eR16G16B16A16Sfloat, // 4: emissive
		};

		for (uint32_t i = 0; i < 5; ++i) {
			attachments[i].format = colorFormats[i];
			attachments[i].samples = vk::SampleCountFlagBits::e1;
			attachments[i].loadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachments[i].initialLayout = vk::ImageLayout::eUndefined;
			attachments[i].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		// Depth (non-MSAA, written via gl_FragDepth)
		attachments[5].format = m_depthFormat;
		attachments[5].samples = vk::SampleCountFlagBits::e1;
		attachments[5].loadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[5].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[5].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[5].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[5].initialLayout = vk::ImageLayout::eUndefined;
		attachments[5].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		std::array<vk::AttachmentReference, 5> colorRefs;
		for (uint32_t i = 0; i < 5; ++i) {
			colorRefs[i].attachment = i;
			colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		vk::AttachmentReference depthRef;
		depthRef.attachment = 5;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 5;
		subpass.pColorAttachments = colorRefs.data();
		subpass.pDepthStencilAttachment = &depthRef;

		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_msaaResolveRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA resolve render pass: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// MSAA Resolve framebuffer — references non-MSAA G-buffer images
	// Attachment order: [0]=scene color, [1]=position, [2]=normal, [3]=specular, [4]=emissive, [5]=depth
	{
		std::array<vk::ImageView, 6> fbAttachments = {
			m_sceneColor.view,
			m_gbufPosition.view,
			m_gbufNormal.view,
			m_gbufSpecular.view,
			m_gbufEmissive.view,
			m_sceneDepth.view,
		};

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_msaaResolveRenderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		fbInfo.pAttachments = fbAttachments.data();
		fbInfo.width = w;
		fbInfo.height = h;
		fbInfo.layers = 1;

		try {
			m_msaaResolveFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA resolve framebuffer: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}
	}

	// Create per-frame MSAA resolve UBO (persistently mapped)
	// Two 256-byte slots (one per frame in flight) hold {int samples; float fov;} data.
	{
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = MAX_FRAMES_IN_FLIGHT * 256;
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_msaaResolveUBO = m_device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA resolve UBO: %s\n", e.what()));
			shutdownMSAA();
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_msaaResolveUBO, MemoryUsage::CpuToGpu, m_msaaResolveUBOAlloc)) {
			mprintf(("VulkanPostProcessor: Failed to allocate MSAA resolve UBO memory!\n"));
			m_device.destroyBuffer(m_msaaResolveUBO);
			m_msaaResolveUBO = nullptr;
			shutdownMSAA();
			return false;
		}

		m_msaaResolveUBOMapped = m_memoryManager->mapMemory(m_msaaResolveUBOAlloc);
		if (!m_msaaResolveUBOMapped) {
			mprintf(("VulkanPostProcessor: Failed to map MSAA resolve UBO!\n"));
			shutdownMSAA();
			return false;
		}
	}

	// Transition MSAA images to the render pass's initial layout at creation time.
	// The validation layer tracks framebuffer attachment layouts from creation,
	// so we must match the eClear render pass's initialLayout exactly.
	{
		auto* texMgr = getTextureManager();

		RenderTarget* colorTargets[] = {
			&m_msaaColor, &m_msaaPosition, &m_msaaNormal,
			&m_msaaSpecular, &m_msaaEmissive,
		};
		for (auto* t : colorTargets) {
			texMgr->transitionImageLayout(t->image, t->format,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
		}

		texMgr->transitionImageLayout(m_msaaDepthImage, m_depthFormat,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	}

	m_msaaInitialized = true;
	mprintf(("VulkanPostProcessor: MSAA initialized (%ux%u, %dx samples, 5 color + depth)\n",
		w, h, Cmdline_msaa_enabled));
	return true;
}

void VulkanPostProcessor::shutdownMSAA()
{
	if (!m_device) {
		return;
	}

	// Destroy MSAA resolve UBO
	if (m_msaaResolveUBOMapped) {
		m_memoryManager->unmapMemory(m_msaaResolveUBOAlloc);
		m_msaaResolveUBOMapped = nullptr;
	}
	if (m_msaaResolveUBO) {
		m_device.destroyBuffer(m_msaaResolveUBO);
		m_msaaResolveUBO = nullptr;
	}
	if (m_msaaResolveUBOAlloc.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_msaaResolveUBOAlloc);
	}

	if (m_msaaResolveFramebuffer) {
		m_device.destroyFramebuffer(m_msaaResolveFramebuffer);
		m_msaaResolveFramebuffer = nullptr;
	}
	if (m_msaaResolveRenderPass) {
		m_device.destroyRenderPass(m_msaaResolveRenderPass);
		m_msaaResolveRenderPass = nullptr;
	}
	if (m_msaaEmissiveCopyFramebuffer) {
		m_device.destroyFramebuffer(m_msaaEmissiveCopyFramebuffer);
		m_msaaEmissiveCopyFramebuffer = nullptr;
	}
	if (m_msaaEmissiveCopyRenderPass) {
		m_device.destroyRenderPass(m_msaaEmissiveCopyRenderPass);
		m_msaaEmissiveCopyRenderPass = nullptr;
	}
	if (m_msaaGbufFramebuffer) {
		m_device.destroyFramebuffer(m_msaaGbufFramebuffer);
		m_msaaGbufFramebuffer = nullptr;
	}
	if (m_msaaGbufRenderPassLoad) {
		m_device.destroyRenderPass(m_msaaGbufRenderPassLoad);
		m_msaaGbufRenderPassLoad = nullptr;
	}
	if (m_msaaGbufRenderPass) {
		m_device.destroyRenderPass(m_msaaGbufRenderPass);
		m_msaaGbufRenderPass = nullptr;
	}

	// Destroy MSAA depth
	if (m_msaaDepthView) {
		m_device.destroyImageView(m_msaaDepthView);
		m_msaaDepthView = nullptr;
	}
	if (m_msaaDepthImage) {
		m_device.destroyImage(m_msaaDepthImage);
		m_msaaDepthImage = nullptr;
	}
	if (m_msaaDepthAlloc.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_msaaDepthAlloc);
	}

	// Destroy MSAA color targets
	RenderTarget* msaaTargets[] = {
		&m_msaaColor, &m_msaaPosition, &m_msaaNormal,
		&m_msaaSpecular, &m_msaaEmissive,
	};
	for (auto* rt : msaaTargets) {
		if (rt->view) {
			m_device.destroyImageView(rt->view);
			rt->view = nullptr;
		}
		if (rt->image) {
			m_device.destroyImage(rt->image);
			rt->image = nullptr;
		}
		if (rt->allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(rt->allocation);
		}
	}

	m_msaaInitialized = false;
}

void VulkanPostProcessor::transitionMsaaGbufForResume(vk::CommandBuffer /*cmd*/)
{
	// No-op: MSAA render passes use finalLayout == subpass layout (no implicit
	// transition at endRenderPass), so color attachments remain in
	// eColorAttachmentOptimal — exactly what the eLoad pass expects.
}

void VulkanPostProcessor::transitionMsaaGbufForBegin(vk::CommandBuffer /*cmd*/)
{
	// No-op: MSAA images are always in eColorAttachmentOptimal /
	// eDepthStencilAttachmentOptimal between frames. Init-time transitions
	// set this layout, and the post-resolve barriers in
	// vulkan_deferred_lighting_msaa restore it after each frame's resolve pass.
}

// ===== Light Accumulation (Deferred Lighting) =====

bool VulkanPostProcessor::initLightVolumes()
{
	if (m_lightVolumesInitialized) {
		return true;
	}

	// Generate sphere mesh (16 rings x 16 segments)
	{
		auto mesh = graphics::util::generate_sphere_mesh(16, 16);
		m_sphereMesh.vertexCount = mesh.vertex_count;
		m_sphereMesh.indexCount = mesh.index_count;

		// Create VBO
		vk::BufferCreateInfo vboInfo;
		vboInfo.size = mesh.vertices.size() * sizeof(float);
		vboInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		vboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_sphereMesh.vbo = m_device.createBuffer(vboInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create sphere VBO: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_sphereMesh.vbo, MemoryUsage::CpuToGpu, m_sphereMesh.vboAlloc)) {
			m_device.destroyBuffer(m_sphereMesh.vbo);
			m_sphereMesh.vbo = nullptr;
			return false;
		}

		auto* mapped = m_memoryManager->mapMemory(m_sphereMesh.vboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.vertices.data(), mesh.vertices.size() * sizeof(float));
			m_memoryManager->unmapMemory(m_sphereMesh.vboAlloc);
		}

		// Create IBO
		vk::BufferCreateInfo iboInfo;
		iboInfo.size = mesh.indices.size() * sizeof(ushort);
		iboInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		iboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_sphereMesh.ibo = m_device.createBuffer(iboInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create sphere IBO: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_sphereMesh.ibo, MemoryUsage::CpuToGpu, m_sphereMesh.iboAlloc)) {
			m_device.destroyBuffer(m_sphereMesh.ibo);
			m_sphereMesh.ibo = nullptr;
			return false;
		}

		mapped = m_memoryManager->mapMemory(m_sphereMesh.iboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.indices.data(), mesh.indices.size() * sizeof(ushort));
			m_memoryManager->unmapMemory(m_sphereMesh.iboAlloc);
		}
	}

	// Generate cylinder mesh (16 segments)
	{
		auto mesh = graphics::util::generate_cylinder_mesh(16);
		m_cylinderMesh.vertexCount = mesh.vertex_count;
		m_cylinderMesh.indexCount = mesh.index_count;

		vk::BufferCreateInfo vboInfo;
		vboInfo.size = mesh.vertices.size() * sizeof(float);
		vboInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		vboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_cylinderMesh.vbo = m_device.createBuffer(vboInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create cylinder VBO: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_cylinderMesh.vbo, MemoryUsage::CpuToGpu, m_cylinderMesh.vboAlloc)) {
			m_device.destroyBuffer(m_cylinderMesh.vbo);
			m_cylinderMesh.vbo = nullptr;
			return false;
		}

		auto* mapped = m_memoryManager->mapMemory(m_cylinderMesh.vboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.vertices.data(), mesh.vertices.size() * sizeof(float));
			m_memoryManager->unmapMemory(m_cylinderMesh.vboAlloc);
		}

		vk::BufferCreateInfo iboInfo;
		iboInfo.size = mesh.indices.size() * sizeof(ushort);
		iboInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		iboInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_cylinderMesh.ibo = m_device.createBuffer(iboInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create cylinder IBO: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_cylinderMesh.ibo, MemoryUsage::CpuToGpu, m_cylinderMesh.iboAlloc)) {
			m_device.destroyBuffer(m_cylinderMesh.ibo);
			m_cylinderMesh.ibo = nullptr;
			return false;
		}

		mapped = m_memoryManager->mapMemory(m_cylinderMesh.iboAlloc);
		if (mapped) {
			memcpy(mapped, mesh.indices.data(), mesh.indices.size() * sizeof(ushort));
			m_memoryManager->unmapMemory(m_cylinderMesh.iboAlloc);
		}
	}

	// Create deferred UBO for light data (per-frame, host-visible)
	{
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = DEFERRED_UBO_SIZE;
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_deferredUBO = m_device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create deferred UBO: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_deferredUBO, MemoryUsage::CpuToGpu, m_deferredUBOAlloc)) {
			m_device.destroyBuffer(m_deferredUBO);
			m_deferredUBO = nullptr;
			return false;
		}
	}

	m_lightVolumesInitialized = true;
	mprintf(("VulkanPostProcessor: Light volumes initialized (sphere: %u verts/%u idx, cylinder: %u verts/%u idx)\n",
		m_sphereMesh.vertexCount, m_sphereMesh.indexCount,
		m_cylinderMesh.vertexCount, m_cylinderMesh.indexCount));
	return true;
}

void VulkanPostProcessor::shutdownLightVolumes()
{
	if (!m_device) {
		return;
	}

	auto destroyMesh = [&](LightVolumeMesh& mesh) {
		if (mesh.vbo) { m_device.destroyBuffer(mesh.vbo); mesh.vbo = nullptr; }
		if (mesh.vboAlloc.memory != VK_NULL_HANDLE) { m_memoryManager->freeAllocation(mesh.vboAlloc); }
		if (mesh.ibo) { m_device.destroyBuffer(mesh.ibo); mesh.ibo = nullptr; }
		if (mesh.iboAlloc.memory != VK_NULL_HANDLE) { m_memoryManager->freeAllocation(mesh.iboAlloc); }
		mesh.vertexCount = 0;
		mesh.indexCount = 0;
	};

	destroyMesh(m_sphereMesh);
	destroyMesh(m_cylinderMesh);

	if (m_deferredUBO) {
		m_device.destroyBuffer(m_deferredUBO);
		m_deferredUBO = nullptr;
	}
	if (m_deferredUBOAlloc.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_deferredUBOAlloc);
	}

	if (m_lightAccumFramebuffer) {
		m_device.destroyFramebuffer(m_lightAccumFramebuffer);
		m_lightAccumFramebuffer = nullptr;
	}
	if (m_lightAccumRenderPass) {
		m_device.destroyRenderPass(m_lightAccumRenderPass);
		m_lightAccumRenderPass = nullptr;
	}

	m_lightVolumesInitialized = false;
}

bool VulkanPostProcessor::initLightAccumPass()
{
	// Light accumulation render pass: single RGBA16F color attachment
	// loadOp=eLoad (preserves emissive copy), storeOp=eStore
	// initialLayout=eColorAttachmentOptimal, finalLayout=eShaderReadOnlyOptimal
	{
		vk::AttachmentDescription att;
		att.format = vk::Format::eR16G16B16A16Sfloat;
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
		dep.srcStageMask = vk::PipelineStageFlagBits::eTransfer
		                  | vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                  | vk::PipelineStageFlagBits::eFragmentShader;
		dep.srcAccessMask = vk::AccessFlagBits::eTransferWrite
		                  | vk::AccessFlagBits::eColorAttachmentWrite;
		dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead
		                  | vk::AccessFlagBits::eColorAttachmentWrite
		                  | vk::AccessFlagBits::eShaderRead;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = 1;
		rpInfo.pAttachments = &att;
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dep;

		try {
			m_lightAccumRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create light accum render pass: %s\n", e.what()));
			return false;
		}
	}

	// Framebuffer using composite image as sole color attachment
	{
		vk::ImageView attachments[] = { m_gbufComposite.view };

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_lightAccumRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = attachments;
		fbInfo.width = m_extent.width;
		fbInfo.height = m_extent.height;
		fbInfo.layers = 1;

		try {
			m_lightAccumFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create light accum framebuffer: %s\n", e.what()));
			return false;
		}
	}

	return true;
}

namespace ltp = lighting_profiles;

static graphics::deferred_light_data* prepare_light_uniforms(light& l, uint8_t* dest, const ltp::profile* lp)
{
	auto* light_data = reinterpret_cast<graphics::deferred_light_data*>(dest);
	memset(light_data, 0, sizeof(graphics::deferred_light_data));

	light_data->lightType = static_cast<int>(l.type);

	float intensity =
		(Lighting_mode == lighting_mode::COCKPIT) ? lp->cockpit_light_intensity_modifier.handle(l.intensity) : l.intensity;

	vec3d diffuse;
	diffuse.xyz.x = l.r * intensity;
	diffuse.xyz.y = l.g * intensity;
	diffuse.xyz.z = l.b * intensity;

	light_data->diffuseLightColor = diffuse;
	light_data->enable_shadows = 0;
	light_data->sourceRadius = l.source_radius;
	return light_data;
}

void VulkanPostProcessor::renderDeferredLights(vk::CommandBuffer cmd)
{
	TRACE_SCOPE(tracing::ApplyLights);

	if (!m_gbufInitialized) {
		return;
	}

	// Lazy-init light volumes and accumulation pass on first use
	if (!m_lightVolumesInitialized) {
		if (!initLightVolumes() || !initLightAccumPass()) {
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

	// Sort lights by type (same stable sort as OpenGL)
	std::stable_sort(Lights.begin(), Lights.end(), light_compare_by_type);

	// Categorize lights
	SCP_vector<light> full_frame_lights;
	SCP_vector<light> sphere_lights;
	SCP_vector<light> cylinder_lights;
	for (auto& l : Lights) {
		switch (l.type) {
		case Light_Type::Directional:
			full_frame_lights.push_back(l);
			break;
		case Light_Type::Cone:
		case Light_Type::Point:
			sphere_lights.push_back(l);
			break;
		case Light_Type::Tube:
			cylinder_lights.push_back(l);
			break;
		case Light_Type::Ambient:
			break;
		}
	}

	// Add ambient light
	{
		light& l = full_frame_lights.emplace_back();
		memset(&l, 0, sizeof(light));
		vec3d ambient;
		gr_get_ambient_light(&ambient);
		l.r = ambient.xyz.x;
		l.g = ambient.xyz.y;
		l.b = ambient.xyz.z;
		l.type = Light_Type::Ambient;
		l.intensity = 1.f;
		l.source_radius = 0.f;
	}

	size_t total_lights = full_frame_lights.size() + sphere_lights.size() + cylinder_lights.size();
	if (total_lights == 0) {
		return;
	}

	// Map UBO and pack data
	auto* uboMapped = static_cast<uint8_t*>(m_memoryManager->mapMemory(m_deferredUBOAlloc));
	if (!uboMapped) {
		return;
	}

	// Determine alignment requirement
	uint32_t uboAlign = getRendererInstance()->getMinUniformBufferOffsetAlignment();
	auto alignUp = [uboAlign](uint32_t v) -> uint32_t {
		return (v + uboAlign - 1) & ~(uboAlign - 1);
	};

	// Layout in UBO:
	// [0]: deferred_global_data (header)
	// [aligned offset 1..N]: deferred_light_data per light
	// [aligned offset N+1..2N]: matrix_uniforms per light
	uint32_t globalDataSize = alignUp(static_cast<uint32_t>(sizeof(graphics::deferred_global_data)));
	uint32_t lightDataSize = alignUp(static_cast<uint32_t>(sizeof(graphics::deferred_light_data)));
	uint32_t matrixDataSize = alignUp(static_cast<uint32_t>(sizeof(graphics::matrix_uniforms)));

	uint32_t lightDataOffset = globalDataSize;
	uint32_t matrixDataOffset = lightDataOffset + static_cast<uint32_t>(total_lights) * lightDataSize;
	uint32_t totalUBOSize = matrixDataOffset + static_cast<uint32_t>(total_lights) * matrixDataSize;

	if (totalUBOSize > DEFERRED_UBO_SIZE) {
		mprintf(("VulkanPostProcessor: Deferred UBO overflow (%u > %u), skipping lights\n", totalUBOSize, DEFERRED_UBO_SIZE));
		m_memoryManager->unmapMemory(m_deferredUBOAlloc);
		return;
	}

	// Pack global header
	auto lp = ltp::current();
	// Determine if environment maps are available
	bool envMapAvailable = (ENVMAP > 0);
	tcache_slot_vulkan* envMapSlot = nullptr;
	tcache_slot_vulkan* irrMapSlot = nullptr;
	if (envMapAvailable) {
		envMapSlot = texMgr->getTextureSlot(ENVMAP);
		if (!envMapSlot || !envMapSlot->imageView || !envMapSlot->isCubemap) {
			envMapAvailable = false;
		}
	}
	if (envMapAvailable && IRRMAP > 0) {
		irrMapSlot = texMgr->getTextureSlot(IRRMAP);
		if (!irrMapSlot || !irrMapSlot->imageView || !irrMapSlot->isCubemap) {
			irrMapSlot = nullptr;  // Fall back to fallback cube for irrmap
		}
	}

	{
		auto* header = reinterpret_cast<graphics::deferred_global_data*>(uboMapped);
		memset(header, 0, sizeof(graphics::deferred_global_data));
		header->invScreenWidth = 1.0f / gr_screen.max_w;
		header->invScreenHeight = 1.0f / gr_screen.max_h;
		header->nearPlane = gr_near_plane;
		header->use_env_map = envMapAvailable ? 1 : 0;

		if (m_shadowInitialized && Shadow_quality != ShadowQuality::Disabled) {
			header->shadow_mv_matrix = Shadow_view_matrix_light;
			for (size_t i = 0; i < MAX_SHADOW_CASCADES; ++i) {
				header->shadow_proj_matrix[i] = Shadow_proj_matrix[i];
			}
			header->veryneardist = Shadow_cascade_distances[0];
			header->neardist = Shadow_cascade_distances[1];
			header->middist = Shadow_cascade_distances[2];
			header->fardist = Shadow_cascade_distances[3];
			vm_inverse_matrix4(&header->inv_view_matrix, &Shadow_view_matrix_render);
		}
	}

	// Pack per-light data
	size_t lightIdx = 0;
	bool first_directional = true;

	for (auto& l : full_frame_lights) {
		auto* ld = prepare_light_uniforms(l, uboMapped + lightDataOffset + lightIdx * lightDataSize, lp);

		if (l.type == Light_Type::Directional) {
			if (m_shadowInitialized && Shadow_quality != ShadowQuality::Disabled) {
				ld->enable_shadows = first_directional ? 1 : 0;
			}

			if (first_directional) {
				first_directional = false;
			}

			vec4 light_dir;
			light_dir.xyzw.x = -l.vec.xyz.x;
			light_dir.xyzw.y = -l.vec.xyz.y;
			light_dir.xyzw.z = -l.vec.xyz.z;
			light_dir.xyzw.w = 0.0f;
			vec4 view_dir;
			vm_vec_transform(&view_dir, &light_dir, &gr_view_matrix);
			ld->lightDir.xyz.x = view_dir.xyzw.x;
			ld->lightDir.xyz.y = view_dir.xyzw.y;
			ld->lightDir.xyz.z = view_dir.xyzw.z;
		}

		// Matrix: env texture matrix for full-frame lights
		auto* md = reinterpret_cast<graphics::matrix_uniforms*>(uboMapped + matrixDataOffset + lightIdx * matrixDataSize);
		memset(md, 0, sizeof(graphics::matrix_uniforms));
		md->modelViewMatrix = gr_env_texture_matrix;
		++lightIdx;
	}

	for (auto& l : sphere_lights) {
		auto* ld = prepare_light_uniforms(l, uboMapped + lightDataOffset + lightIdx * lightDataSize, lp);

		if (l.type == Light_Type::Cone) {
			ld->dualCone = (l.flags & LF_DUAL_CONE) ? 1.0f : 0.0f;
			ld->coneAngle = l.cone_angle;
			ld->coneInnerAngle = l.cone_inner_angle;
			ld->coneDir = l.vec2;
		}
		float rad = (Lighting_mode == lighting_mode::COCKPIT)
						? lp->cockpit_light_radius_modifier.handle(MAX(l.rada, l.radb))
						: MAX(l.rada, l.radb);
		ld->lightRadius = rad;
		ld->scale.xyz.x = rad * 1.05f;
		ld->scale.xyz.y = rad * 1.05f;
		ld->scale.xyz.z = rad * 1.05f;

		// Matrix: model-view + projection for light volume
		auto* md = reinterpret_cast<graphics::matrix_uniforms*>(uboMapped + matrixDataOffset + lightIdx * matrixDataSize);
		g3_start_instance_matrix(&l.vec, &vmd_identity_matrix, true);
		md->modelViewMatrix = gr_model_view_matrix;
		md->projMatrix = gr_projection_matrix;
		g3_done_instance(true);
		++lightIdx;
	}

	for (auto& l : cylinder_lights) {
		auto* ld = prepare_light_uniforms(l, uboMapped + lightDataOffset + lightIdx * lightDataSize, lp);
		float rad =
			(Lighting_mode == lighting_mode::COCKPIT) ? lp->cockpit_light_radius_modifier.handle(l.radb) : l.radb;
		ld->lightRadius = rad;
		ld->lightType = LT_TUBE;

		vec3d a;
		vm_vec_sub(&a, &l.vec, &l.vec2);
		auto length = vm_vec_mag(&a);
		length += ld->lightRadius * 2.0f;

		ld->scale.xyz.x = rad * 1.05f;
		ld->scale.xyz.y = rad * 1.05f;
		ld->scale.xyz.z = length;

		// Matrix: oriented instance matrix for cylinder
		auto* md = reinterpret_cast<graphics::matrix_uniforms*>(uboMapped + matrixDataOffset + lightIdx * matrixDataSize);
		vec3d dir, newPos;
		matrix orient;
		vm_vec_normalized_dir(&dir, &l.vec, &l.vec2);
		vm_vector_2_matrix_norm(&orient, &dir, nullptr, nullptr);
		vm_vec_scale_sub(&newPos, &l.vec2, &dir, l.radb);

		g3_start_instance_matrix(&newPos, &orient, true);
		md->modelViewMatrix = gr_model_view_matrix;
		md->projMatrix = gr_projection_matrix;
		g3_done_instance(true);
		++lightIdx;
	}

	m_memoryManager->unmapMemory(m_deferredUBOAlloc);

	// Both fullscreen and volume lights use the same vertex layout (POSITION3).
	// For fullscreen lights the shader ignores vertex data and generates positions
	// from gl_VertexIndex, but Vulkan requires all declared vertex inputs to have
	// matching pipeline attributes and bound buffers.
	vertex_layout volLayout;
	volLayout.add_vertex_component(vertex_format_data::POSITION3, sizeof(float) * 3, 0);

	PipelineConfig lightConfig;
	lightConfig.shaderType = SDR_TYPE_DEFERRED_LIGHTING;
	lightConfig.vertexLayoutHash = volLayout.hash();
	lightConfig.primitiveType = PRIM_TYPE_TRIS;
	lightConfig.depthMode = ZBUFFER_TYPE_NONE;
	lightConfig.blendMode = ALPHA_BLEND_ADDITIVE;
	lightConfig.cullEnabled = false;
	lightConfig.depthWriteEnabled = false;
	lightConfig.renderPass = m_lightAccumRenderPass;

	vk::Pipeline lightPipeline = pipelineMgr->getPipeline(lightConfig, volLayout);
	if (!lightPipeline) {
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Prepare G-buffer texture infos for material descriptor set
	vk::DescriptorImageInfo gbufTexInfos[4];
	gbufTexInfos[0].sampler = m_linearSampler;
	gbufTexInfos[0].imageView = m_sceneColor.view;  // ColorBuffer
	gbufTexInfos[0].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	gbufTexInfos[1].sampler = m_linearSampler;
	gbufTexInfos[1].imageView = m_gbufNormal.view;  // NormalBuffer
	gbufTexInfos[1].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	gbufTexInfos[2].sampler = m_linearSampler;
	gbufTexInfos[2].imageView = m_gbufPosition.view;  // PositionBuffer
	gbufTexInfos[2].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	gbufTexInfos[3].sampler = m_linearSampler;
	gbufTexInfos[3].imageView = m_gbufSpecular.view;  // SpecBuffer
	gbufTexInfos[3].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	// Fallback buffer and textures for unused descriptor bindings
	auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
	vk::DescriptorBufferInfo fallbackBufInfo;
	fallbackBufInfo.buffer = fallbackBuf;
	fallbackBufInfo.offset = 0;
	fallbackBufInfo.range = 4096;

	vk::ImageView fallbackView = texMgr->getFallbackTextureView2D();
	vk::Sampler defaultSampler = texMgr->getDefaultSampler();

	// Begin light accumulation render pass
	{
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = m_lightAccumRenderPass;
		rpBegin.framebuffer = m_lightAccumFramebuffer;
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = m_extent;

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	}

	// Set viewport and scissor
	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_extent.width);
	viewport.height = static_cast<float>(m_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_extent;
	cmd.setScissor(0, scissor);

	// Helper lambda to allocate + write descriptor sets for a single light draw
	auto bindLightDescriptors = [&](size_t li) {
		// Global set (Set 0): light UBO at binding 0, globals UBO at binding 1
		vk::DescriptorSet globalSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Global);
		if (!globalSet) return false;

		vk::DescriptorBufferInfo lightBufInfo;
		lightBufInfo.buffer = m_deferredUBO;
		lightBufInfo.offset = lightDataOffset + li * lightDataSize;
		lightBufInfo.range = sizeof(graphics::deferred_light_data);

		vk::DescriptorBufferInfo globalBufInfo;
		globalBufInfo.buffer = m_deferredUBO;
		globalBufInfo.offset = 0;
		globalBufInfo.range = sizeof(graphics::deferred_global_data);

		// Shadow map at binding 2
		vk::DescriptorImageInfo shadowTexInfo;
		if (m_shadowInitialized && m_shadowColor.view) {
			shadowTexInfo.sampler = m_linearSampler;
			shadowTexInfo.imageView = m_shadowColor.view;
			shadowTexInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		} else {
			shadowTexInfo.sampler = defaultSampler;
			shadowTexInfo.imageView = fallbackView;
			shadowTexInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		// Env map at binding 3
		vk::ImageView fallbackCubeView = texMgr->getFallbackCubeView();
		vk::DescriptorImageInfo envTexInfo;
		if (envMapAvailable && envMapSlot) {
			envTexInfo.sampler = defaultSampler;
			envTexInfo.imageView = envMapSlot->imageView;
			envTexInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		} else {
			envTexInfo.sampler = defaultSampler;
			envTexInfo.imageView = fallbackCubeView;
			envTexInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		// Irradiance map at binding 4
		vk::DescriptorImageInfo irrTexInfo;
		if (envMapAvailable && irrMapSlot) {
			irrTexInfo.sampler = defaultSampler;
			irrTexInfo.imageView = irrMapSlot->imageView;
			irrTexInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		} else {
			irrTexInfo.sampler = defaultSampler;
			irrTexInfo.imageView = fallbackCubeView;
			irrTexInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		std::array<vk::WriteDescriptorSet, 5> globalWrites;
		globalWrites[0].dstSet = globalSet;
		globalWrites[0].dstBinding = 0;
		globalWrites[0].dstArrayElement = 0;
		globalWrites[0].descriptorCount = 1;
		globalWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		globalWrites[0].pBufferInfo = &lightBufInfo;

		globalWrites[1].dstSet = globalSet;
		globalWrites[1].dstBinding = 1;
		globalWrites[1].dstArrayElement = 0;
		globalWrites[1].descriptorCount = 1;
		globalWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
		globalWrites[1].pBufferInfo = &globalBufInfo;

		globalWrites[2].dstSet = globalSet;
		globalWrites[2].dstBinding = 2;
		globalWrites[2].dstArrayElement = 0;
		globalWrites[2].descriptorCount = 1;
		globalWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		globalWrites[2].pImageInfo = &shadowTexInfo;

		globalWrites[3].dstSet = globalSet;
		globalWrites[3].dstBinding = 3;
		globalWrites[3].dstArrayElement = 0;
		globalWrites[3].descriptorCount = 1;
		globalWrites[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		globalWrites[3].pImageInfo = &envTexInfo;

		globalWrites[4].dstSet = globalSet;
		globalWrites[4].dstBinding = 4;
		globalWrites[4].dstArrayElement = 0;
		globalWrites[4].descriptorCount = 1;
		globalWrites[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		globalWrites[4].pImageInfo = &irrTexInfo;

		m_device.updateDescriptorSets(globalWrites, {});

		// Material set (Set 1): G-buffer textures at binding 1[0..3]
		vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
		if (!materialSet) return false;

		// ModelData UBO at binding 0 (fallback)
		vk::WriteDescriptorSet modelWrite;
		modelWrite.dstSet = materialSet;
		modelWrite.dstBinding = 0;
		modelWrite.dstArrayElement = 0;
		modelWrite.descriptorCount = 1;
		modelWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		modelWrite.pBufferInfo = &fallbackBufInfo;

		// G-buffer textures at binding 1 elements 0-3
		vk::WriteDescriptorSet gbufTexWrite;
		gbufTexWrite.dstSet = materialSet;
		gbufTexWrite.dstBinding = 1;
		gbufTexWrite.dstArrayElement = 0;
		gbufTexWrite.descriptorCount = 4;
		gbufTexWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		gbufTexWrite.pImageInfo = gbufTexInfos;

		// Fill remaining texture array elements with fallback
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS - 4> fallbackImages;
		for (auto& fi : fallbackImages) {
			fi.sampler = defaultSampler;
			fi.imageView = fallbackView;
			fi.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		vk::WriteDescriptorSet fallbackTexWrite;
		fallbackTexWrite.dstSet = materialSet;
		fallbackTexWrite.dstBinding = 1;
		fallbackTexWrite.dstArrayElement = 4;
		fallbackTexWrite.descriptorCount = static_cast<uint32_t>(fallbackImages.size());
		fallbackTexWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		fallbackTexWrite.pImageInfo = fallbackImages.data();

		// DecalGlobals at binding 2 (fallback)
		vk::WriteDescriptorSet decalWrite;
		decalWrite.dstSet = materialSet;
		decalWrite.dstBinding = 2;
		decalWrite.dstArrayElement = 0;
		decalWrite.descriptorCount = 1;
		decalWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		decalWrite.pBufferInfo = &fallbackBufInfo;

		// Transform SSBO at binding 3 (fallback)
		vk::WriteDescriptorSet ssboWrite;
		ssboWrite.dstSet = materialSet;
		ssboWrite.dstBinding = 3;
		ssboWrite.dstArrayElement = 0;
		ssboWrite.descriptorCount = 1;
		ssboWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
		ssboWrite.pBufferInfo = &fallbackBufInfo;

		// Bindings 4-6: depth, scene color, distortion fallbacks
		vk::DescriptorImageInfo fallbackImg;
		fallbackImg.sampler = defaultSampler;
		fallbackImg.imageView = fallbackView;
		fallbackImg.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		std::array<vk::WriteDescriptorSet, 3> texFallbackWrites;
		for (uint32_t b = 4; b <= 6; ++b) {
			auto& w = texFallbackWrites[b - 4];
			w.dstSet = materialSet;
			w.dstBinding = b;
			w.dstArrayElement = 0;
			w.descriptorCount = 1;
			w.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			w.pImageInfo = &fallbackImg;
		}

		SCP_vector<vk::WriteDescriptorSet> matWrites = {
			modelWrite, gbufTexWrite, fallbackTexWrite, decalWrite, ssboWrite,
			texFallbackWrites[0], texFallbackWrites[1], texFallbackWrites[2]
		};
		m_device.updateDescriptorSets(matWrites, {});

		// PerDraw set (Set 2): matrices UBO at binding 1
		vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
		if (!perDrawSet) return false;

		vk::DescriptorBufferInfo matrixBufInfo;
		matrixBufInfo.buffer = m_deferredUBO;
		matrixBufInfo.offset = matrixDataOffset + li * matrixDataSize;
		matrixBufInfo.range = sizeof(graphics::matrix_uniforms);

		// GenericData at binding 0 (fallback)
		vk::WriteDescriptorSet genWrite;
		genWrite.dstSet = perDrawSet;
		genWrite.dstBinding = 0;
		genWrite.dstArrayElement = 0;
		genWrite.descriptorCount = 1;
		genWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		genWrite.pBufferInfo = &fallbackBufInfo;

		vk::WriteDescriptorSet matWrite;
		matWrite.dstSet = perDrawSet;
		matWrite.dstBinding = 1;
		matWrite.dstArrayElement = 0;
		matWrite.descriptorCount = 1;
		matWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		matWrite.pBufferInfo = &matrixBufInfo;

		// Bindings 2-4: NanoVG, Decal, Movie (fallback)
		std::array<vk::WriteDescriptorSet, 3> pdFallbacks;
		for (uint32_t b = 2; b <= 4; ++b) {
			auto& w = pdFallbacks[b - 2];
			w.dstSet = perDrawSet;
			w.dstBinding = b;
			w.dstArrayElement = 0;
			w.descriptorCount = 1;
			w.descriptorType = vk::DescriptorType::eUniformBuffer;
			w.pBufferInfo = &fallbackBufInfo;
		}

		SCP_vector<vk::WriteDescriptorSet> pdWrites = {genWrite, matWrite, pdFallbacks[0], pdFallbacks[1], pdFallbacks[2]};
		m_device.updateDescriptorSets(pdWrites, {});

		// Bind all 3 descriptor sets
		std::array<vk::DescriptorSet, 3> sets = { globalSet, materialSet, perDrawSet };
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, sets, {});

		return true;
	};

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, lightPipeline);

	// Draw full-frame lights (directional + ambient)
	// Bind sphere VBO as dummy — shader ignores vertex data for these light types.
	lightIdx = 0;
	if (!full_frame_lights.empty()) {
		cmd.bindVertexBuffers(0, m_sphereMesh.vbo, vk::DeviceSize(0));
		for (size_t i = 0; i < full_frame_lights.size(); ++i) {
			if (bindLightDescriptors(lightIdx)) {
				cmd.draw(3, 1, 0, 0);
			}
			++lightIdx;
		}
	}

	// Draw sphere lights (point + cone)
	if (!sphere_lights.empty()) {
		cmd.bindVertexBuffers(0, m_sphereMesh.vbo, vk::DeviceSize(0));
		cmd.bindIndexBuffer(m_sphereMesh.ibo, 0, vk::IndexType::eUint16);
		for (size_t i = 0; i < sphere_lights.size(); ++i) {
			if (bindLightDescriptors(lightIdx)) {
				cmd.drawIndexed(m_sphereMesh.indexCount, 1, 0, 0, 0);
			}
			++lightIdx;
		}
	}

	// Draw cylinder lights (tube)
	if (!cylinder_lights.empty()) {
		cmd.bindVertexBuffers(0, m_cylinderMesh.vbo, vk::DeviceSize(0));
		cmd.bindIndexBuffer(m_cylinderMesh.ibo, 0, vk::IndexType::eUint16);
		for (size_t i = 0; i < cylinder_lights.size(); ++i) {
			if (bindLightDescriptors(lightIdx)) {
				cmd.drawIndexed(m_cylinderMesh.indexCount, 1, 0, 0, 0);
			}
			++lightIdx;
		}
	}

	// End render pass (composite → eShaderReadOnlyOptimal)
	cmd.endRenderPass();
}

// ===== Bloom Pipeline Implementation =====

// Local UBO struct for blur shader (extends blur_data with runtime direction parameter)
struct BlurUBOData {
	float texSize;
	int level;
	int direction; // 0 = horizontal, 1 = vertical
	int pad;
};

bool VulkanPostProcessor::initBloom()
{
	m_bloomWidth = m_extent.width / 2;
	m_bloomHeight = m_extent.height / 2;

	const uint32_t mipLevels = MAX_MIP_BLUR_LEVELS;

	// Create 2 bloom textures (RGBA16F, half-res, 4 mip levels each)
	for (int i = 0; i < 2; i++) {
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = vk::Format::eR16G16B16A16Sfloat;
		imageInfo.extent.width = m_bloomWidth;
		imageInfo.extent.height = m_bloomHeight;
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
			m_bloomTex[i].image = m_device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create bloom image %d: %s\n", i, e.what()));
			return false;
		}

		if (!m_memoryManager->allocateImageMemory(m_bloomTex[i].image, MemoryUsage::GpuOnly, m_bloomTex[i].allocation)) {
			mprintf(("VulkanPostProcessor: Failed to allocate bloom image %d memory!\n", i));
			return false;
		}

		// Full image view (all mip levels, for textureLod sampling)
		vk::ImageViewCreateInfo fullViewInfo;
		fullViewInfo.image = m_bloomTex[i].image;
		fullViewInfo.viewType = vk::ImageViewType::e2D;
		fullViewInfo.format = vk::Format::eR16G16B16A16Sfloat;
		fullViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		fullViewInfo.subresourceRange.baseMipLevel = 0;
		fullViewInfo.subresourceRange.levelCount = mipLevels;
		fullViewInfo.subresourceRange.baseArrayLayer = 0;
		fullViewInfo.subresourceRange.layerCount = 1;

		try {
			m_bloomTex[i].fullView = m_device.createImageView(fullViewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create bloom %d full view: %s\n", i, e.what()));
			return false;
		}

		// Per-mip image views (for framebuffer attachment)
		for (uint32_t mip = 0; mip < mipLevels; mip++) {
			vk::ImageViewCreateInfo mipViewInfo = fullViewInfo;
			mipViewInfo.subresourceRange.baseMipLevel = mip;
			mipViewInfo.subresourceRange.levelCount = 1;

			try {
				m_bloomTex[i].mipViews[mip] = m_device.createImageView(mipViewInfo);
			} catch (const vk::SystemError& e) {
				mprintf(("VulkanPostProcessor: Failed to create bloom %d mip %u view: %s\n", i, mip, e.what()));
				return false;
			}
		}
	}

	// Create bloom render pass (color-only RGBA16F, loadOp=eDontCare for overwriting)
	{
		vk::AttachmentDescription att;
		att.format = vk::Format::eR16G16B16A16Sfloat;
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
			m_bloomRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create bloom render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create bloom composite render pass (loadOp=eLoad for additive compositing onto scene color)
	{
		vk::AttachmentDescription att;
		att.format = vk::Format::eR16G16B16A16Sfloat;
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
			m_bloomCompositeRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create bloom composite render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create per-mip framebuffers for bloom textures
	for (int i = 0; i < 2; i++) {
		for (uint32_t mip = 0; mip < mipLevels; mip++) {
			uint32_t mipW = std::max(1u, m_bloomWidth >> mip);
			uint32_t mipH = std::max(1u, m_bloomHeight >> mip);

			vk::FramebufferCreateInfo fbInfo;
			fbInfo.renderPass = m_bloomRenderPass;
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = &m_bloomTex[i].mipViews[mip];
			fbInfo.width = mipW;
			fbInfo.height = mipH;
			fbInfo.layers = 1;

			try {
				m_bloomTex[i].mipFramebuffers[mip] = m_device.createFramebuffer(fbInfo);
			} catch (const vk::SystemError& e) {
				mprintf(("VulkanPostProcessor: Failed to create bloom %d mip %u framebuffer: %s\n", i, mip, e.what()));
				return false;
			}
		}
	}

	// Create scene color framebuffer for bloom composite (wraps m_sceneColor as attachment)
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_bloomCompositeRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &m_sceneColor.view;
		fbInfo.width = m_extent.width;
		fbInfo.height = m_extent.height;
		fbInfo.layers = 1;

		try {
			m_sceneColorBloomFB = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create scene color bloom framebuffer: %s\n", e.what()));
			return false;
		}
	}

	// Create bloom UBO buffer (slot-based allocation for per-draw data)
	{
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = BLOOM_UBO_MAX_SLOTS * BLOOM_UBO_SLOT_SIZE;
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_bloomUBO = m_device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create bloom UBO: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateBufferMemory(m_bloomUBO, MemoryUsage::CpuToGpu, m_bloomUBOAlloc)) {
			mprintf(("VulkanPostProcessor: Failed to allocate bloom UBO memory!\n"));
			m_device.destroyBuffer(m_bloomUBO);
			m_bloomUBO = nullptr;
			return false;
		}

	}

	m_bloomInitialized = true;
	mprintf(("VulkanPostProcessor: Bloom initialized (%ux%u, %d mip levels)\n",
		m_bloomWidth, m_bloomHeight, MAX_MIP_BLUR_LEVELS));
	return true;
}

void VulkanPostProcessor::shutdownBloom()
{
	if (!m_bloomInitialized) {
		return;
	}

	if (m_bloomUBO) {
		m_device.destroyBuffer(m_bloomUBO);
		m_bloomUBO = nullptr;
	}
	if (m_bloomUBOAlloc.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_bloomUBOAlloc);
	}

	if (m_sceneColorBloomFB) {
		m_device.destroyFramebuffer(m_sceneColorBloomFB);
		m_sceneColorBloomFB = nullptr;
	}

	for (int i = 0; i < 2; i++) {
		for (uint32_t mip = 0; mip < MAX_MIP_BLUR_LEVELS; mip++) {
			if (m_bloomTex[i].mipFramebuffers[mip]) {
				m_device.destroyFramebuffer(m_bloomTex[i].mipFramebuffers[mip]);
				m_bloomTex[i].mipFramebuffers[mip] = nullptr;
			}
			if (m_bloomTex[i].mipViews[mip]) {
				m_device.destroyImageView(m_bloomTex[i].mipViews[mip]);
				m_bloomTex[i].mipViews[mip] = nullptr;
			}
		}
		if (m_bloomTex[i].fullView) {
			m_device.destroyImageView(m_bloomTex[i].fullView);
			m_bloomTex[i].fullView = nullptr;
		}
		if (m_bloomTex[i].image) {
			m_device.destroyImage(m_bloomTex[i].image);
			m_bloomTex[i].image = nullptr;
		}
		if (m_bloomTex[i].allocation.memory != VK_NULL_HANDLE) {
			m_memoryManager->freeAllocation(m_bloomTex[i].allocation);
		}
	}

	if (m_bloomCompositeRenderPass) {
		m_device.destroyRenderPass(m_bloomCompositeRenderPass);
		m_bloomCompositeRenderPass = nullptr;
	}
	if (m_bloomRenderPass) {
		m_device.destroyRenderPass(m_bloomRenderPass);
		m_bloomRenderPass = nullptr;
	}

	m_bloomInitialized = false;
}

void VulkanPostProcessor::generateMipmaps(vk::CommandBuffer cmd, vk::Image image,
                                           uint32_t width, uint32_t height, uint32_t mipLevels)
{
	// Transition mip 0 from eShaderReadOnlyOptimal (after brightpass) to eTransferSrcOptimal
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eTransfer,
			{}, {}, {}, barrier);
	}

	vulkan_generate_mipmap_chain(cmd, image, width, height, mipLevels);
}

void VulkanPostProcessor::drawFullscreenTriangle(vk::CommandBuffer cmd, vk::RenderPass renderPass,
                                                  vk::Framebuffer framebuffer, vk::Extent2D extent,
                                                  int shaderType,
                                                  vk::ImageView textureView, vk::Sampler sampler,
                                                  const void* uboData, size_t uboSize,
                                                  int blendMode)
{
	auto* pipelineMgr = getPipelineManager();
	auto* descriptorMgr = getDescriptorManager();
	auto* bufferMgr = getBufferManager();
	auto* texMgr = getTextureManager();

	if (!pipelineMgr || !descriptorMgr || !bufferMgr || !texMgr) {
		return;
	}

	// Get/create pipeline for this shader + render pass combination
	PipelineConfig config;
	config.shaderType = static_cast<shader_type>(shaderType);
	config.vertexLayoutHash = 0;
	config.primitiveType = PRIM_TYPE_TRIS;
	config.depthMode = ZBUFFER_TYPE_NONE;
	config.blendMode = static_cast<gr_alpha_blend>(blendMode);
	config.cullEnabled = false;
	config.depthWriteEnabled = false;
	config.renderPass = renderPass;

	vertex_layout emptyLayout;
	vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
	if (!pipeline) {
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Begin render pass
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = renderPass;
	rpBegin.framebuffer = framebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = extent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	// Set viewport and scissor
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

	// Allocate Material descriptor set (Set 1)
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);

	{
		// Source texture at binding 1 element 0
		vk::DescriptorImageInfo imageInfo;
		imageInfo.sampler = sampler;
		imageInfo.imageView = textureView;
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet texWrite;
		texWrite.dstSet = materialSet;
		texWrite.dstBinding = 1;
		texWrite.dstArrayElement = 0;
		texWrite.descriptorCount = 1;
		texWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		texWrite.pImageInfo = &imageInfo;

		// Fallback UBO for binding 0 (ModelData) and binding 2 (DecalGlobals)
		auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo fallbackBufInfo;
		fallbackBufInfo.buffer = fallbackBuf;
		fallbackBufInfo.offset = 0;
		fallbackBufInfo.range = 4096;

		vk::WriteDescriptorSet modelWrite;
		modelWrite.dstSet = materialSet;
		modelWrite.dstBinding = 0;
		modelWrite.dstArrayElement = 0;
		modelWrite.descriptorCount = 1;
		modelWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		modelWrite.pBufferInfo = &fallbackBufInfo;

		vk::WriteDescriptorSet decalWrite;
		decalWrite.dstSet = materialSet;
		decalWrite.dstBinding = 2;
		decalWrite.dstArrayElement = 0;
		decalWrite.descriptorCount = 1;
		decalWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		decalWrite.pBufferInfo = &fallbackBufInfo;

		// Fill remaining texture array elements with fallback (use 2D view since
		// post-processing shaders declare sampler2D, not sampler2DArray)
		vk::ImageView fallbackView = texMgr->getFallbackTextureView2D();
		vk::Sampler defaultSampler = texMgr->getDefaultSampler();

		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS - 1> fallbackImages;
		for (auto& fi : fallbackImages) {
			fi.sampler = defaultSampler;
			fi.imageView = fallbackView;
			fi.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		vk::WriteDescriptorSet fallbackTexWrite;
		fallbackTexWrite.dstSet = materialSet;
		fallbackTexWrite.dstBinding = 1;
		fallbackTexWrite.dstArrayElement = 1;
		fallbackTexWrite.descriptorCount = static_cast<uint32_t>(fallbackImages.size());
		fallbackTexWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		fallbackTexWrite.pImageInfo = fallbackImages.data();

		// Binding 3: Transform SSBO (fallback to zero UBO)
		vk::WriteDescriptorSet ssboWrite;
		ssboWrite.dstSet = materialSet;
		ssboWrite.dstBinding = 3;
		ssboWrite.dstArrayElement = 0;
		ssboWrite.descriptorCount = 1;
		ssboWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
		ssboWrite.pBufferInfo = &fallbackBufInfo;

		// Binding 4: Depth map (fallback to 2D white texture)
		vk::DescriptorImageInfo depthMapFallback;
		depthMapFallback.sampler = defaultSampler;
		depthMapFallback.imageView = fallbackView;
		depthMapFallback.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet depthMapWrite;
		depthMapWrite.dstSet = materialSet;
		depthMapWrite.dstBinding = 4;
		depthMapWrite.dstArrayElement = 0;
		depthMapWrite.descriptorCount = 1;
		depthMapWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		depthMapWrite.pImageInfo = &depthMapFallback;

		// Binding 5: Scene color / frameBuffer (fallback to 2D white texture)
		vk::DescriptorImageInfo sceneColorFallback;
		sceneColorFallback.sampler = defaultSampler;
		sceneColorFallback.imageView = fallbackView;
		sceneColorFallback.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet sceneColorWrite;
		sceneColorWrite.dstSet = materialSet;
		sceneColorWrite.dstBinding = 5;
		sceneColorWrite.dstArrayElement = 0;
		sceneColorWrite.descriptorCount = 1;
		sceneColorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		sceneColorWrite.pImageInfo = &sceneColorFallback;

		// Binding 6: Distortion map (fallback to 2D white texture)
		vk::DescriptorImageInfo distMapFallback;
		distMapFallback.sampler = defaultSampler;
		distMapFallback.imageView = fallbackView;
		distMapFallback.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet distMapWrite;
		distMapWrite.dstSet = materialSet;
		distMapWrite.dstBinding = 6;
		distMapWrite.dstArrayElement = 0;
		distMapWrite.descriptorCount = 1;
		distMapWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		distMapWrite.pImageInfo = &distMapFallback;

		std::array<vk::WriteDescriptorSet, 8> writes = {texWrite, modelWrite, decalWrite, fallbackTexWrite, ssboWrite, depthMapWrite, sceneColorWrite, distMapWrite};
		m_device.updateDescriptorSets(writes, {});
	}

	// Allocate PerDraw descriptor set (Set 2)
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);

	{
		vk::DescriptorBufferInfo uboInfo;

		if (uboData && uboSize > 0 && m_bloomUBOMapped) {
			// Write UBO data to current bloom UBO slot
			Assertion(m_bloomUBOCursor < BLOOM_UBO_MAX_SLOTS, "Bloom UBO slot overflow!");
			uint32_t slotOffset = m_bloomUBOCursor * static_cast<uint32_t>(BLOOM_UBO_SLOT_SIZE);
			memcpy(static_cast<uint8_t*>(m_bloomUBOMapped) + slotOffset, uboData, uboSize);
			m_bloomUBOCursor++;

			uboInfo.buffer = m_bloomUBO;
			uboInfo.offset = slotOffset;
			uboInfo.range = BLOOM_UBO_SLOT_SIZE;
		} else {
			// No UBO data — use fallback zero buffer
			uboInfo.buffer = bufferMgr->getFallbackUniformBuffer();
			uboInfo.offset = 0;
			uboInfo.range = 4096;
		}

		vk::WriteDescriptorSet write;
		write.dstSet = perDrawSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eUniformBuffer;
		write.pBufferInfo = &uboInfo;

		// Fallback for remaining per-draw bindings (1-4: Matrices, NanoVGData, DecalInfo, MovieData)
		auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo fallbackInfo;
		fallbackInfo.buffer = fallbackBuf;
		fallbackInfo.offset = 0;
		fallbackInfo.range = 4096;

		SCP_vector<vk::WriteDescriptorSet> writes;
		writes.push_back(write);
		for (uint32_t b = 1; b <= 4; ++b) {
			vk::WriteDescriptorSet fw;
			fw.dstSet = perDrawSet;
			fw.dstBinding = b;
			fw.dstArrayElement = 0;
			fw.descriptorCount = 1;
			fw.descriptorType = vk::DescriptorType::eUniformBuffer;
			fw.pBufferInfo = &fallbackInfo;
			writes.push_back(fw);
		}

		m_device.updateDescriptorSets(writes, {});
	}

	// Bind descriptor sets (Set 0 already bound from frame setup)
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();
}

void VulkanPostProcessor::executeBloom(vk::CommandBuffer cmd)
{
	if (!m_bloomInitialized || gr_bloom_intensity() <= 0) {
		return;
	}

	// Map bloom UBO for writing per-draw data
	m_bloomUBOMapped = m_memoryManager->mapMemory(m_bloomUBOAlloc);
	if (!m_bloomUBOMapped) {
		return;
	}
	m_bloomUBOCursor = 0;

	// 1. Bright pass: extract pixels brighter than 1.0 from scene color → bloom_tex[0] mip 0
	drawFullscreenTriangle(cmd, m_bloomRenderPass,
		m_bloomTex[0].mipFramebuffers[0],
		vk::Extent2D(m_bloomWidth, m_bloomHeight),
		SDR_TYPE_POST_PROCESS_BRIGHTPASS,
		m_sceneColor.view, m_linearSampler,
		nullptr, 0,  // Brightpass has no UBO
		ALPHA_BLEND_NONE);

	// 2. Generate mipmaps for bloom_tex[0] (fill mips 1-3 from mip 0)
	generateMipmaps(cmd, m_bloomTex[0].image, m_bloomWidth, m_bloomHeight, MAX_MIP_BLUR_LEVELS);

	// 3. Blur iterations (2 iterations of vertical + horizontal ping-pong)
	for (int iteration = 0; iteration < 2; iteration++) {
		for (int pass = 0; pass < 2; pass++) {
			// pass 0 = vertical (tex[0] → tex[1]), pass 1 = horizontal (tex[1] → tex[0])
			int srcIdx = pass;
			int dstIdx = 1 - pass;
			int direction = (pass == 0) ? 1 : 0;  // 1=vertical, 0=horizontal

			for (int mip = 0; mip < MAX_MIP_BLUR_LEVELS; mip++) {
				uint32_t mipW = std::max(1u, m_bloomWidth >> mip);
				uint32_t mipH = std::max(1u, m_bloomHeight >> mip);

				BlurUBOData blurData;
				blurData.texSize = (direction == 0) ? 1.0f / static_cast<float>(mipW)
				                                    : 1.0f / static_cast<float>(mipH);
				blurData.level = mip;
				blurData.direction = direction;
				blurData.pad = 0;

				drawFullscreenTriangle(cmd, m_bloomRenderPass,
					m_bloomTex[dstIdx].mipFramebuffers[mip],
					vk::Extent2D(mipW, mipH),
					SDR_TYPE_POST_PROCESS_BLUR,
					m_bloomTex[srcIdx].fullView, m_mipmapSampler,
					&blurData, sizeof(blurData),
					ALPHA_BLEND_NONE);
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
		barrier.image = m_sceneColor.image;
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

	drawFullscreenTriangle(cmd, m_bloomCompositeRenderPass,
		m_sceneColorBloomFB,
		m_extent,
		SDR_TYPE_POST_PROCESS_BLOOM_COMP,
		m_bloomTex[0].fullView, m_mipmapSampler,
		&compData, sizeof(compData),
		ALPHA_BLEND_ADDITIVE);

	// Scene_color is now in eShaderReadOnlyOptimal (from bloom composite render pass finalLayout)

	// Unmap bloom UBO
	m_memoryManager->unmapMemory(m_bloomUBOAlloc);
	m_bloomUBOMapped = nullptr;
}

// ===== LDR Targets + FXAA Pipeline Implementation =====

bool VulkanPostProcessor::initLDRTargets()
{
	// Create Scene_ldr (RGBA8, full resolution) — tonemapped LDR output
	if (!createImage(m_extent.width, m_extent.height, vk::Format::eR8G8B8A8Unorm,
	                 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneLdr.image, m_sceneLdr.view, m_sceneLdr.allocation)) {
		mprintf(("VulkanPostProcessor: Failed to create Scene_ldr image!\n"));
		return false;
	}
	m_sceneLdr.format = vk::Format::eR8G8B8A8Unorm;
	m_sceneLdr.width = m_extent.width;
	m_sceneLdr.height = m_extent.height;

	// Create Scene_luminance (RGBA8, full resolution) — LDR with luma in alpha for FXAA
	if (!createImage(m_extent.width, m_extent.height, vk::Format::eR8G8B8A8Unorm,
	                 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
	                 vk::ImageAspectFlagBits::eColor,
	                 m_sceneLuminance.image, m_sceneLuminance.view, m_sceneLuminance.allocation)) {
		mprintf(("VulkanPostProcessor: Failed to create Scene_luminance image!\n"));
		return false;
	}
	m_sceneLuminance.format = vk::Format::eR8G8B8A8Unorm;
	m_sceneLuminance.width = m_extent.width;
	m_sceneLuminance.height = m_extent.height;

	// Create LDR render pass (color-only RGBA8, loadOp=eDontCare, finalLayout=eShaderReadOnlyOptimal)
	{
		vk::AttachmentDescription att;
		att.format = vk::Format::eR8G8B8A8Unorm;
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
			m_ldrRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create LDR render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create framebuffers
	{
		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_ldrRenderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &m_sceneLdr.view;
		fbInfo.width = m_extent.width;
		fbInfo.height = m_extent.height;
		fbInfo.layers = 1;

		try {
			m_sceneLdrFB = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create Scene_ldr framebuffer: %s\n", e.what()));
			return false;
		}

		fbInfo.pAttachments = &m_sceneLuminance.view;
		try {
			m_sceneLuminanceFB = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create Scene_luminance framebuffer: %s\n", e.what()));
			return false;
		}
	}

	// Create LDR load render pass (loadOp=eLoad for additive blending onto existing content)
	{
		vk::AttachmentDescription att;
		att.format = vk::Format::eR8G8B8A8Unorm;
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
			m_ldrLoadRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create LDR load render pass: %s\n", e.what()));
			return false;
		}
	}

	m_ldrInitialized = true;
	mprintf(("VulkanPostProcessor: LDR targets initialized (%ux%u, RGBA8)\n",
		m_extent.width, m_extent.height));
	return true;
}

void VulkanPostProcessor::shutdownLDRTargets()
{
	if (!m_ldrInitialized) {
		return;
	}

	if (m_sceneLuminanceFB) {
		m_device.destroyFramebuffer(m_sceneLuminanceFB);
		m_sceneLuminanceFB = nullptr;
	}
	if (m_sceneLdrFB) {
		m_device.destroyFramebuffer(m_sceneLdrFB);
		m_sceneLdrFB = nullptr;
	}
	if (m_ldrLoadRenderPass) {
		m_device.destroyRenderPass(m_ldrLoadRenderPass);
		m_ldrLoadRenderPass = nullptr;
	}
	if (m_ldrRenderPass) {
		m_device.destroyRenderPass(m_ldrRenderPass);
		m_ldrRenderPass = nullptr;
	}

	// Scene_luminance
	if (m_sceneLuminance.view) {
		m_device.destroyImageView(m_sceneLuminance.view);
		m_sceneLuminance.view = nullptr;
	}
	if (m_sceneLuminance.image) {
		m_device.destroyImage(m_sceneLuminance.image);
		m_sceneLuminance.image = nullptr;
	}
	if (m_sceneLuminance.allocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_sceneLuminance.allocation);
	}

	// Scene_ldr
	if (m_sceneLdr.view) {
		m_device.destroyImageView(m_sceneLdr.view);
		m_sceneLdr.view = nullptr;
	}
	if (m_sceneLdr.image) {
		m_device.destroyImage(m_sceneLdr.image);
		m_sceneLdr.image = nullptr;
	}
	if (m_sceneLdr.allocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_sceneLdr.allocation);
	}

	m_ldrInitialized = false;
}

void VulkanPostProcessor::executeTonemap(vk::CommandBuffer cmd)
{
	if (!m_ldrInitialized) {
		return;
	}

	namespace ltp = lighting_profiles;

	// Map bloom UBO for the tonemapping draw's UBO slot
	m_bloomUBOMapped = m_memoryManager->mapMemory(m_bloomUBOAlloc);
	if (!m_bloomUBOMapped) {
		return;
	}

	// Reset cursor if bloom didn't run this frame (bloom resets to 0 when it runs)
	if (gr_bloom_intensity() <= 0 || !m_bloomInitialized) {
		m_bloomUBOCursor = 0;
	}

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

	// HDR scene → Scene_ldr via tonemapping shader
	drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLdrFB, m_extent,
		SDR_TYPE_POST_PROCESS_TONEMAPPING,
		m_sceneColor.view, m_linearSampler,
		&tmData, sizeof(tmData),
		ALPHA_BLEND_NONE);

	m_memoryManager->unmapMemory(m_bloomUBOAlloc);
	m_bloomUBOMapped = nullptr;
}

void VulkanPostProcessor::executeFXAA(vk::CommandBuffer cmd)
{
	if (!m_ldrInitialized || !gr_is_fxaa_mode(Gr_aa_mode)) {
		return;
	}

	m_bloomUBOMapped = m_memoryManager->mapMemory(m_bloomUBOAlloc);
	if (!m_bloomUBOMapped) {
		return;
	}

	// FXAA prepass: Scene_ldr → Scene_luminance (compute luma in alpha)
	drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLuminanceFB, m_extent,
		SDR_TYPE_POST_PROCESS_FXAA_PREPASS,
		m_sceneLdr.view, m_linearSampler,
		nullptr, 0,
		ALPHA_BLEND_NONE);

	// FXAA main pass: Scene_luminance → Scene_ldr
	graphics::generic_data::fxaa_data fxaaData;
	fxaaData.rt_w = static_cast<float>(m_extent.width);
	fxaaData.rt_h = static_cast<float>(m_extent.height);
	fxaaData.pad[0] = 0.0f;
	fxaaData.pad[1] = 0.0f;

	drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLdrFB, m_extent,
		SDR_TYPE_POST_PROCESS_FXAA,
		m_sceneLuminance.view, m_linearSampler,
		&fxaaData, sizeof(fxaaData),
		ALPHA_BLEND_NONE);

	m_memoryManager->unmapMemory(m_bloomUBOAlloc);
	m_bloomUBOMapped = nullptr;
}

bool VulkanPostProcessor::executePostEffects(vk::CommandBuffer cmd)
{
	m_postEffectsApplied = false;

	if (!m_ldrInitialized || !graphics::Post_processing_manager) {
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

	m_bloomUBOMapped = m_memoryManager->mapMemory(m_bloomUBOAlloc);
	if (!m_bloomUBOMapped) {
		return false;
	}

	// Build the extended post_data UBO with effectFlags appended
	struct PostEffectsUBOData {
		graphics::generic_data::post_data base;
		int effectFlags;
		int pad[3];
	};

	PostEffectsUBOData uboData;
	memset(&uboData, 0, sizeof(uboData));
	uboData.base.timer = static_cast<float>(timer_get_milliseconds() % 100 + 1);
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
	drawFullscreenTriangle(cmd, m_ldrRenderPass,
		m_sceneLuminanceFB, m_extent,
		SDR_TYPE_POST_PROCESS_MAIN,
		m_sceneLdr.view, m_linearSampler,
		&uboData, sizeof(uboData),
		ALPHA_BLEND_NONE);

	m_memoryManager->unmapMemory(m_bloomUBOAlloc);
	m_bloomUBOMapped = nullptr;

	m_postEffectsApplied = true;
	return true;
}

void VulkanPostProcessor::executeLightshafts(vk::CommandBuffer cmd)
{
	if (!m_ldrInitialized || !graphics::Post_processing_manager) {
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

	// Transition scene depth from eDepthStencilAttachmentOptimal to eShaderReadOnlyOptimal for sampling
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_sceneDepth.image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		if (m_depthFormat == vk::Format::eD24UnormS8Uint || m_depthFormat == vk::Format::eD32SfloatS8Uint) {
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
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

	m_bloomUBOMapped = m_memoryManager->mapMemory(m_bloomUBOAlloc);
	if (!m_bloomUBOMapped) {
		return;
	}

	// Additive blend lightshafts onto Scene_ldr
	drawFullscreenTriangle(cmd, m_ldrLoadRenderPass,
		m_sceneLdrFB, m_extent,
		SDR_TYPE_POST_PROCESS_LIGHTSHAFTS,
		m_sceneDepth.view, m_linearSampler,
		&lsData, sizeof(lsData),
		ALPHA_BLEND_ADDITIVE);

	m_memoryManager->unmapMemory(m_bloomUBOAlloc);
	m_bloomUBOMapped = nullptr;
}

void VulkanPostProcessor::copyEffectTexture(vk::CommandBuffer cmd)
{
	// Called mid-scene, outside a render pass.
	// Scene color is in eShaderReadOnlyOptimal (from the ended scene render pass).
	// Copies scene color → effect texture so distortion/soft particle shaders can sample it.
	copyImageToImage(cmd,
		m_sceneColor.image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
		m_sceneEffect.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_extent);
}

void VulkanPostProcessor::copySceneDepth(vk::CommandBuffer cmd)
{
	// Called mid-scene, outside a render pass.
	// Copies scene depth → depth copy texture so soft particle shaders can sample it.
	// Scene depth is in eDepthStencilAttachmentOptimal (from the ended scene render pass).
	copyImageToImage(cmd,
		m_sceneDepth.image, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
		m_sceneDepthCopy.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_extent,
		vk::ImageAspectFlagBits::eDepth);
}

void VulkanPostProcessor::copyGbufNormal(vk::CommandBuffer cmd)
{
	// Called mid-scene, outside a render pass.
	// Copies G-buffer normal → normal copy so decal shader can sample it for angle rejection.
	// G-buffer normal is in eShaderReadOnlyOptimal (from the ended G-buffer render pass).
	// Normal goes back to eShaderReadOnlyOptimal (transitionGbufForResume handles the rest).
	copyImageToImage(cmd,
		m_gbufNormal.image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_gbufNormalCopy.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_extent);
}

void VulkanPostProcessor::updateDistortion(vk::CommandBuffer cmd, float frametime)
{
	if (!m_distortionInitialized) {
		return;
	}

	m_distortionTimer += frametime;
	if (m_distortionTimer < 0.03f) {
		return;
	}
	m_distortionTimer = 0.0f;

	int dst = !m_distortionSwitch;  // Write target
	int src = m_distortionSwitch;   // Read source

	// On first update, images are still in eUndefined layout
	vk::ImageLayout srcOldLayout = m_distortionFirstUpdate
		? vk::ImageLayout::eUndefined : vk::ImageLayout::eShaderReadOnlyOptimal;
	vk::AccessFlags srcOldAccess = m_distortionFirstUpdate
		? vk::AccessFlags{} : vk::AccessFlagBits::eShaderRead;

	// Transition both distortion textures for transfer operations
	{
		std::array<vk::ImageMemoryBarrier, 2> barriers;

		// dst: eShaderReadOnlyOptimal (or eUndefined on first use) → eTransferDstOptimal
		barriers[0].srcAccessMask = srcOldAccess;
		barriers[0].dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		barriers[0].oldLayout = srcOldLayout;
		barriers[0].newLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].image = m_distortionTex[dst].image;
		barriers[0].subresourceRange = vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		// src: eShaderReadOnlyOptimal (or eUndefined on first use) → eTransferSrcOptimal
		barriers[1].srcAccessMask = srcOldAccess;
		barriers[1].dstAccessMask = vk::AccessFlagBits::eTransferRead;
		barriers[1].oldLayout = srcOldLayout;
		barriers[1].newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].image = m_distortionTex[src].image;
		barriers[1].subresourceRange = vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eTransfer,
			{}, {}, {}, barriers);
	}

	// Clear dest to mid-gray (0.5, 0.5, 0.0, 1.0) = no distortion
	{
		vk::ClearColorValue clearColor;
		clearColor.setFloat32({0.5f, 0.5f, 0.0f, 1.0f});
		vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
		cmd.clearColorImage(m_distortionTex[dst].image,
			vk::ImageLayout::eTransferDstOptimal, clearColor, range);
	}

	// Blit: scroll old data right by 1 pixel
	// src columns 0-30 → dst columns 1-31 (with LINEAR filtering)
	{
		vk::ImageBlit blit;
		blit.srcSubresource = vk::ImageSubresourceLayers(
			vk::ImageAspectFlagBits::eColor, 0, 0, 1);
		blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.srcOffsets[1] = vk::Offset3D(31, 32, 1);
		blit.dstSubresource = vk::ImageSubresourceLayers(
			vk::ImageAspectFlagBits::eColor, 0, 0, 1);
		blit.dstOffsets[0] = vk::Offset3D(1, 0, 0);
		blit.dstOffsets[1] = vk::Offset3D(32, 32, 1);

		cmd.blitImage(
			m_distortionTex[src].image, vk::ImageLayout::eTransferSrcOptimal,
			m_distortionTex[dst].image, vk::ImageLayout::eTransferDstOptimal,
			blit, vk::Filter::eLinear);
	}

	// Generate random noise and copy to column 0 of dst
	// OpenGL draws 33 GL_POINTS at x=0 with random R,G values — we write 32 pixels
	{
		// Create a small host-visible staging buffer for 32 RGBA8 pixels (128 bytes)
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = 32 * 4;
		bufInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		vk::Buffer stagingBuf;
		VulkanAllocation stagingAlloc;
		try {
			stagingBuf = m_device.createBuffer(bufInfo);
		} catch (const vk::SystemError&) {
			// Non-fatal: skip noise injection this frame
			goto skip_noise;
		}

		Verify(m_memoryManager->allocateBufferMemory(stagingBuf, MemoryUsage::CpuOnly, stagingAlloc));

		{
			auto* pixels = static_cast<uint8_t*>(m_memoryManager->mapMemory(stagingAlloc));
			Verify(pixels);
			for (int i = 0; i < 32; i++) {
				pixels[i * 4 + 0] = static_cast<uint8_t>(::util::Random::next(256));  // R
				pixels[i * 4 + 1] = static_cast<uint8_t>(::util::Random::next(256));  // G
				pixels[i * 4 + 2] = 255;  // B
				pixels[i * 4 + 3] = 255;  // A
			}
			m_memoryManager->unmapMemory(stagingAlloc);

			// Copy staging buffer → column 0 of dst (1 pixel wide, 32 pixels tall)
			vk::BufferImageCopy region;
			region.bufferOffset = 0;
			region.bufferRowLength = 0;    // Tightly packed
			region.bufferImageHeight = 0;
			region.imageSubresource = vk::ImageSubresourceLayers(
				vk::ImageAspectFlagBits::eColor, 0, 0, 1);
			region.imageOffset = vk::Offset3D(0, 0, 0);
			region.imageExtent = vk::Extent3D(1, 32, 1);

			cmd.copyBufferToImage(stagingBuf, m_distortionTex[dst].image,
				vk::ImageLayout::eTransferDstOptimal, region);
		}

		// Schedule staging buffer for deferred destruction (GPU may still be reading)
		auto* delQueue = getDeletionQueue();
		if (delQueue) {
			delQueue->queueBuffer(stagingBuf, stagingAlloc);
		} else {
			m_device.destroyBuffer(stagingBuf);
			m_memoryManager->freeAllocation(stagingAlloc);
		}
	}

skip_noise:
	// Transition both textures back to eShaderReadOnlyOptimal
	{
		std::array<vk::ImageMemoryBarrier, 2> barriers;

		// dst: eTransferDstOptimal → eShaderReadOnlyOptimal
		barriers[0].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barriers[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barriers[0].oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[0].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].image = m_distortionTex[dst].image;
		barriers[0].subresourceRange = vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		// src: eTransferSrcOptimal → eShaderReadOnlyOptimal
		barriers[1].srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barriers[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barriers[1].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[1].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].image = m_distortionTex[src].image;
		barriers[1].subresourceRange = vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			{}, {}, {}, barriers);
	}

	m_distortionSwitch = !m_distortionSwitch;
	m_distortionFirstUpdate = false;
}

vk::ImageView VulkanPostProcessor::getDistortionTextureView() const
{
	if (!m_distortionInitialized) {
		return nullptr;
	}
	// Return the most recently written texture (matching OpenGL's
	// Distortion_texture[!Distortion_switch] binding for thrusters).
	// After updateDistortion toggles the switch, m_distortionSwitch points
	// to the old read source. The write target was !old_switch = new switch.
	// So the most recently written texture is m_distortionTex[m_distortionSwitch].
	return m_distortionTex[m_distortionSwitch].view;
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

	// Build pipeline config for tonemapping (fullscreen, no depth, no blending)
	// sRGB conversion is controlled by the linearOut UBO field, not shader variants
	PipelineConfig config;
	config.shaderType = SDR_TYPE_POST_PROCESS_TONEMAPPING;
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
		mprintf(("VulkanPostProcessor: Failed to get tonemapping pipeline!\n"));
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();
	stateTracker->bindPipeline(pipeline, pipelineLayout);

	// Set viewport (non-flipped for post-processing — textures are already
	// in the correct Vulkan orientation, no Y-flip needed)
	stateTracker->setViewport(0.0f, 0.0f,
		static_cast<float>(m_extent.width),
		static_cast<float>(m_extent.height));

	stateTracker->applyDynamicState();

	// Allocate and write Material descriptor set (Set 1) with source texture
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);

	{
		// Bind source texture based on post-processing chain state:
		// - Post-effects ran: read Scene_luminance (post-effects output)
		// - LDR only (tonemap/FXAA): read Scene_ldr
		// - No LDR: read Scene_color (raw HDR, tonemapping applied by this shader)
		vk::DescriptorImageInfo imageInfo;
		imageInfo.sampler = m_linearSampler;
		imageInfo.imageView = m_postEffectsApplied ? m_sceneLuminance.view
		                    : useLdr ? m_sceneLdr.view
		                    : m_sceneColor.view;
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet write;
		write.dstSet = materialSet;
		write.dstBinding = 1;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		write.pImageInfo = &imageInfo;

		// Pre-initialize binding 0 (ModelData UBO) with fallback zero buffer
		auto fallbackBuffer = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = fallbackBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = 4096;

		vk::WriteDescriptorSet uboWrite;
		uboWrite.dstSet = materialSet;
		uboWrite.dstBinding = 0;
		uboWrite.dstArrayElement = 0;
		uboWrite.descriptorCount = 1;
		uboWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		uboWrite.pBufferInfo = &bufferInfo;

		// Pre-initialize binding 2 (DecalGlobals UBO) with fallback
		vk::WriteDescriptorSet decalWrite;
		decalWrite.dstSet = materialSet;
		decalWrite.dstBinding = 2;
		decalWrite.dstArrayElement = 0;
		decalWrite.descriptorCount = 1;
		decalWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		decalWrite.pBufferInfo = &bufferInfo;

		// Fill remaining texture array elements with fallback (use 2D view since
		// post-processing shaders declare sampler2D, not sampler2DArray)
		auto* texMgr = getTextureManager();
		vk::ImageView fallbackView = texMgr->getFallbackTextureView2D();
		vk::Sampler defaultSampler = texMgr->getDefaultSampler();

		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS - 1> fallbackImages;
		for (auto& fi : fallbackImages) {
			fi.sampler = defaultSampler;
			fi.imageView = fallbackView;
			fi.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		vk::WriteDescriptorSet fallbackTexWrite;
		fallbackTexWrite.dstSet = materialSet;
		fallbackTexWrite.dstBinding = 1;
		fallbackTexWrite.dstArrayElement = 1;
		fallbackTexWrite.descriptorCount = static_cast<uint32_t>(fallbackImages.size());
		fallbackTexWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		fallbackTexWrite.pImageInfo = fallbackImages.data();

		// Binding 3: Transform SSBO (fallback to zero UBO)
		vk::WriteDescriptorSet ssboWrite;
		ssboWrite.dstSet = materialSet;
		ssboWrite.dstBinding = 3;
		ssboWrite.dstArrayElement = 0;
		ssboWrite.descriptorCount = 1;
		ssboWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
		ssboWrite.pBufferInfo = &bufferInfo;

		// Binding 4: Depth map (fallback to 2D white texture)
		vk::DescriptorImageInfo depthMapFallback;
		depthMapFallback.sampler = defaultSampler;
		depthMapFallback.imageView = fallbackView;
		depthMapFallback.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet depthMapWrite;
		depthMapWrite.dstSet = materialSet;
		depthMapWrite.dstBinding = 4;
		depthMapWrite.dstArrayElement = 0;
		depthMapWrite.descriptorCount = 1;
		depthMapWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		depthMapWrite.pImageInfo = &depthMapFallback;

		// Binding 5: Scene color / frameBuffer (fallback to 2D white texture)
		vk::DescriptorImageInfo sceneColorFallback;
		sceneColorFallback.sampler = defaultSampler;
		sceneColorFallback.imageView = fallbackView;
		sceneColorFallback.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet sceneColorWrite;
		sceneColorWrite.dstSet = materialSet;
		sceneColorWrite.dstBinding = 5;
		sceneColorWrite.dstArrayElement = 0;
		sceneColorWrite.descriptorCount = 1;
		sceneColorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		sceneColorWrite.pImageInfo = &sceneColorFallback;

		// Binding 6: Distortion map (fallback to 2D white texture)
		vk::DescriptorImageInfo distMapFallback;
		distMapFallback.sampler = defaultSampler;
		distMapFallback.imageView = fallbackView;
		distMapFallback.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet distMapWrite;
		distMapWrite.dstSet = materialSet;
		distMapWrite.dstBinding = 6;
		distMapWrite.dstArrayElement = 0;
		distMapWrite.descriptorCount = 1;
		distMapWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		distMapWrite.pImageInfo = &distMapFallback;

		std::array<vk::WriteDescriptorSet, 8> writes = {write, uboWrite, decalWrite, fallbackTexWrite, ssboWrite, depthMapWrite, sceneColorWrite, distMapWrite};
		m_device.updateDescriptorSets(writes, {});
	}

	stateTracker->bindDescriptorSet(DescriptorSetIndex::Material, materialSet);

	// Allocate and write PerDraw descriptor set (Set 2) with tonemapping UBO
	// For now, use fallback (zero) UBO — exposure=0 would give black,
	// so we need a valid graphics::generic_data::tonemapping_data with exposure=1.0 and tonemapper=0 (linear).
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);

	{
		// When blitting LDR, use passthrough tonemapping (exposure=1, linear)
		// Otherwise, use the real tonemapping UBO (already updated above)
		if (useLdr) {
			auto* mapped = static_cast<graphics::generic_data::tonemapping_data*>(
				m_memoryManager->mapMemory(m_tonemapUBOAlloc));
			Verify(mapped);
			memset(mapped, 0, sizeof(graphics::generic_data::tonemapping_data));
			mapped->exposure = 1.0f;
			mapped->tonemapper = 0;  // Linear passthrough
			mapped->linearOut = 1;   // Skip sRGB — LDR input already has sRGB applied
			m_memoryManager->unmapMemory(m_tonemapUBOAlloc);
		}

		vk::DescriptorBufferInfo uboInfo;
		uboInfo.buffer = m_tonemapUBO;
		uboInfo.offset = 0;
		uboInfo.range = sizeof(graphics::generic_data::tonemapping_data);

		vk::WriteDescriptorSet write;
		write.dstSet = perDrawSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eUniformBuffer;
		write.pBufferInfo = &uboInfo;

		// Pre-initialize other bindings with fallback
		auto fallbackBuffer = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo fallbackInfo;
		fallbackInfo.buffer = fallbackBuffer;
		fallbackInfo.offset = 0;
		fallbackInfo.range = 4096;

		SCP_vector<vk::WriteDescriptorSet> writes;
		writes.push_back(write);

		// Bindings 1-4: Matrices, NanoVGData, DecalInfo, MovieData
		for (uint32_t b = 1; b <= 4; ++b) {
			vk::WriteDescriptorSet fw;
			fw.dstSet = perDrawSet;
			fw.dstBinding = b;
			fw.dstArrayElement = 0;
			fw.descriptorCount = 1;
			fw.descriptorType = vk::DescriptorType::eUniformBuffer;
			fw.pBufferInfo = &fallbackInfo;
			writes.push_back(fw);
		}

		m_device.updateDescriptorSets(writes, {});
	}

	stateTracker->bindDescriptorSet(DescriptorSetIndex::PerDraw, perDrawSet);

	// Draw fullscreen triangle (3 vertices from gl_VertexIndex, no vertex buffer)
	cmd.draw(3, 1, 0, 0);
}

// ===== Shadow Map Implementation =====

bool VulkanPostProcessor::initShadowPass()
{
	if (m_shadowInitialized) {
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

	mprintf(("VulkanPostProcessor: Creating %dx%d shadow map (4 cascades)\n", size, size));

	const uint32_t layers = 4;

	// Create shadow color image (RGBA16F, 2D array, 4 layers)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = vk::Format::eR16G16B16A16Sfloat;
		imageInfo.extent = vk::Extent3D(static_cast<uint32_t>(size), static_cast<uint32_t>(size), 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = layers;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_shadowColor.image = m_device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow color image: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateImageMemory(m_shadowColor.image, MemoryUsage::GpuOnly, m_shadowColor.allocation)) {
			m_device.destroyImage(m_shadowColor.image);
			m_shadowColor.image = nullptr;
			return false;
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_shadowColor.image;
		viewInfo.viewType = vk::ImageViewType::e2DArray;
		viewInfo.format = vk::Format::eR16G16B16A16Sfloat;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layers;

		try {
			m_shadowColor.view = m_device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow color view: %s\n", e.what()));
			return false;
		}

		m_shadowColor.format = vk::Format::eR16G16B16A16Sfloat;
		m_shadowColor.width = static_cast<uint32_t>(size);
		m_shadowColor.height = static_cast<uint32_t>(size);
	}

	// Create shadow depth image (D32F, 2D array, 4 layers)
	{
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = vk::Format::eD32Sfloat;
		imageInfo.extent = vk::Extent3D(static_cast<uint32_t>(size), static_cast<uint32_t>(size), 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = layers;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_shadowDepth.image = m_device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow depth image: %s\n", e.what()));
			return false;
		}

		if (!m_memoryManager->allocateImageMemory(m_shadowDepth.image, MemoryUsage::GpuOnly, m_shadowDepth.allocation)) {
			m_device.destroyImage(m_shadowDepth.image);
			m_shadowDepth.image = nullptr;
			return false;
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_shadowDepth.image;
		viewInfo.viewType = vk::ImageViewType::e2DArray;
		viewInfo.format = vk::Format::eD32Sfloat;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layers;

		try {
			m_shadowDepth.view = m_device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow depth view: %s\n", e.what()));
			return false;
		}

		m_shadowDepth.format = vk::Format::eD32Sfloat;
		m_shadowDepth.width = static_cast<uint32_t>(size);
		m_shadowDepth.height = static_cast<uint32_t>(size);
	}

	// Create shadow render pass: 1 color (RGBA16F) + 1 depth (D32F), both eClear
	{
		std::array<vk::AttachmentDescription, 2> attachments;

		// Color attachment (RGBA16F) — stores VSM depth variance
		attachments[0].format = vk::Format::eR16G16B16A16Sfloat;
		attachments[0].samples = vk::SampleCountFlagBits::e1;
		attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
		attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachments[0].initialLayout = vk::ImageLayout::eUndefined;
		attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		// Depth attachment (D32F)
		attachments[1].format = vk::Format::eD32Sfloat;
		attachments[1].samples = vk::SampleCountFlagBits::e1;
		attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
		attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
		attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
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

		vk::SubpassDependency dep;
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dep.srcAccessMask = {};
		dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dep;

		try {
			m_shadowRenderPass = m_device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow render pass: %s\n", e.what()));
			return false;
		}
	}

	// Create layered framebuffer (all 4 layers at once)
	{
		std::array<vk::ImageView, 2> fbAttachments = {
			m_shadowColor.view,
			m_shadowDepth.view,
		};

		vk::FramebufferCreateInfo fbInfo;
		fbInfo.renderPass = m_shadowRenderPass;
		fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
		fbInfo.pAttachments = fbAttachments.data();
		fbInfo.width = static_cast<uint32_t>(size);
		fbInfo.height = static_cast<uint32_t>(size);
		fbInfo.layers = layers;

		try {
			m_shadowFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create shadow framebuffer: %s\n", e.what()));
			return false;
		}
	}

	m_shadowTextureSize = size;
	m_shadowInitialized = true;
	mprintf(("VulkanPostProcessor: Shadow map initialized (%dx%d, 4 cascades)\n", size, size));
	return true;
}

void VulkanPostProcessor::shutdownShadowPass()
{
	if (!m_shadowInitialized) {
		return;
	}

	if (m_shadowFramebuffer) {
		m_device.destroyFramebuffer(m_shadowFramebuffer);
		m_shadowFramebuffer = nullptr;
	}
	if (m_shadowRenderPass) {
		m_device.destroyRenderPass(m_shadowRenderPass);
		m_shadowRenderPass = nullptr;
	}

	if (m_shadowColor.view) {
		m_device.destroyImageView(m_shadowColor.view);
		m_shadowColor.view = nullptr;
	}
	if (m_shadowColor.image) {
		m_device.destroyImage(m_shadowColor.image);
		m_shadowColor.image = nullptr;
	}
	if (m_shadowColor.allocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_shadowColor.allocation);
	}

	if (m_shadowDepth.view) {
		m_device.destroyImageView(m_shadowDepth.view);
		m_shadowDepth.view = nullptr;
	}
	if (m_shadowDepth.image) {
		m_device.destroyImage(m_shadowDepth.image);
		m_shadowDepth.image = nullptr;
	}
	if (m_shadowDepth.allocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_shadowDepth.allocation);
	}

	m_shadowTextureSize = 0;
	m_shadowInitialized = false;
}

bool VulkanPostProcessor::createImage(uint32_t width, uint32_t height, vk::Format format,
                                      vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect,
                                      vk::Image& outImage, vk::ImageView& outView,
                                      VulkanAllocation& outAllocation,
                                      vk::SampleCountFlagBits sampleCount)
{
	// Create image
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = sampleCount;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	try {
		outImage = m_device.createImage(imageInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create image: %s\n", e.what()));
		return false;
	}

	// Allocate memory
	if (!m_memoryManager->allocateImageMemory(outImage, MemoryUsage::GpuOnly, outAllocation)) {
		mprintf(("VulkanPostProcessor: Failed to allocate image memory!\n"));
		m_device.destroyImage(outImage);
		outImage = nullptr;
		return false;
	}

	// Create image view (plain 2D, not array)
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = outImage;
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	try {
		outView = m_device.createImageView(viewInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create image view: %s\n", e.what()));
		m_device.destroyImage(outImage);
		m_memoryManager->freeAllocation(outAllocation);
		outImage = nullptr;
		return false;
	}

	return true;
}

// ========== Fog / Volumetric Nebula ==========

bool VulkanPostProcessor::initFogPass()
{
	if (m_fogInitialized) {
		return true;
	}

	// Create fog render pass: 1 RGBA16F color attachment, loadOp=eDontCare (writing every pixel),
	// initialLayout/finalLayout = eColorAttachmentOptimal (scene color stays as render target)
	{
		vk::AttachmentDescription att;
		att.format = vk::Format::eR16G16B16A16Sfloat;
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
			m_fogRenderPass = m_device.createRenderPass(rpInfo);
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
		fbInfo.pAttachments = &m_sceneColor.view;
		fbInfo.width = m_extent.width;
		fbInfo.height = m_extent.height;
		fbInfo.layers = 1;

		try {
			m_fogFramebuffer = m_device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create fog framebuffer: %s\n", e.what()));
			return false;
		}
	}

	m_fogInitialized = true;
	mprintf(("VulkanPostProcessor: Fog pass initialized\n"));
	return true;
}

void VulkanPostProcessor::shutdownFogPass()
{
	if (m_emissiveMipmappedFullView) {
		m_device.destroyImageView(m_emissiveMipmappedFullView);
		m_emissiveMipmappedFullView = nullptr;
	}
	if (m_emissiveMipmapped.view) {
		m_device.destroyImageView(m_emissiveMipmapped.view);
		m_emissiveMipmapped.view = nullptr;
	}
	if (m_emissiveMipmapped.image) {
		m_device.destroyImage(m_emissiveMipmapped.image);
		m_emissiveMipmapped.image = nullptr;
	}
	if (m_emissiveMipmapped.allocation.memory != VK_NULL_HANDLE) {
		m_memoryManager->freeAllocation(m_emissiveMipmapped.allocation);
	}
	m_emissiveMipmappedInitialized = false;

	if (m_fogFramebuffer) {
		m_device.destroyFramebuffer(m_fogFramebuffer);
		m_fogFramebuffer = nullptr;
	}
	if (m_fogRenderPass) {
		m_device.destroyRenderPass(m_fogRenderPass);
		m_fogRenderPass = nullptr;
	}
	m_fogInitialized = false;
}

void VulkanPostProcessor::renderSceneFog(vk::CommandBuffer cmd)
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
		barrier.image = m_sceneColor.image;
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, nullptr, nullptr, barrier);
	}

	// Map bloom UBO for fog UBO data
	m_bloomUBOMapped = m_memoryManager->mapMemory(m_bloomUBOAlloc);
	Verify(m_bloomUBOMapped);

	// Fill fog UBO
	graphics::generic_data::fog_data fogData;
	{
		float fog_near, fog_far, fog_density;
		neb2_get_adjusted_fog_values(&fog_near, &fog_far, &fog_density);
		unsigned char r, g, b;
		neb2_get_fog_color(&r, &g, &b);

		fogData.fog_start = fog_near;
		fogData.fog_density = fog_density;
		fogData.fog_color.xyz.x = r / 255.f;
		fogData.fog_color.xyz.y = g / 255.f;
		fogData.fog_color.xyz.z = b / 255.f;
		fogData.zNear = Min_draw_distance;
		fogData.zFar = Max_draw_distance;
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
		m_memoryManager->unmapMemory(m_bloomUBOAlloc);
		m_bloomUBOMapped = nullptr;
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Begin render pass
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = m_fogRenderPass;
	rpBegin.framebuffer = m_fogFramebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_extent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_extent.width);
	viewport.height = static_cast<float>(m_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_extent;
	cmd.setScissor(0, scissor);

	// Allocate Material descriptor set (Set 1)
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);

	{
		auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo fallbackBufInfo;
		fallbackBufInfo.buffer = fallbackBuf;
		fallbackBufInfo.offset = 0;
		fallbackBufInfo.range = 4096;

		vk::Sampler defaultSampler = texMgr->getDefaultSampler();
		vk::ImageView fallbackView = texMgr->getFallbackTextureView2D();

		// Binding 0: ModelData UBO (fallback)
		vk::WriteDescriptorSet modelWrite;
		modelWrite.dstSet = materialSet;
		modelWrite.dstBinding = 0;
		modelWrite.dstArrayElement = 0;
		modelWrite.descriptorCount = 1;
		modelWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		modelWrite.pBufferInfo = &fallbackBufInfo;

		// Binding 1: composite (lit result) at element [0]
		vk::DescriptorImageInfo compositeInfo;
		compositeInfo.sampler = m_linearSampler;
		compositeInfo.imageView = m_gbufComposite.view;
		compositeInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet texWrite;
		texWrite.dstSet = materialSet;
		texWrite.dstBinding = 1;
		texWrite.dstArrayElement = 0;
		texWrite.descriptorCount = 1;
		texWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		texWrite.pImageInfo = &compositeInfo;

		// Fill remaining texture array elements with fallback
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS - 1> fallbackImages;
		for (auto& fi : fallbackImages) {
			fi.sampler = defaultSampler;
			fi.imageView = fallbackView;
			fi.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		vk::WriteDescriptorSet fallbackTexWrite;
		fallbackTexWrite.dstSet = materialSet;
		fallbackTexWrite.dstBinding = 1;
		fallbackTexWrite.dstArrayElement = 1;
		fallbackTexWrite.descriptorCount = static_cast<uint32_t>(fallbackImages.size());
		fallbackTexWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		fallbackTexWrite.pImageInfo = fallbackImages.data();

		// Binding 2: DecalGlobals UBO (fallback)
		vk::WriteDescriptorSet decalWrite;
		decalWrite.dstSet = materialSet;
		decalWrite.dstBinding = 2;
		decalWrite.dstArrayElement = 0;
		decalWrite.descriptorCount = 1;
		decalWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		decalWrite.pBufferInfo = &fallbackBufInfo;

		// Binding 3: Transform SSBO (fallback)
		vk::WriteDescriptorSet ssboWrite;
		ssboWrite.dstSet = materialSet;
		ssboWrite.dstBinding = 3;
		ssboWrite.dstArrayElement = 0;
		ssboWrite.descriptorCount = 1;
		ssboWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
		ssboWrite.pBufferInfo = &fallbackBufInfo;

		// Binding 4: Depth copy (actual depth, not fallback)
		vk::DescriptorImageInfo depthInfo;
		depthInfo.sampler = m_linearSampler;
		depthInfo.imageView = m_sceneDepthCopy.view;
		depthInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet depthWrite;
		depthWrite.dstSet = materialSet;
		depthWrite.dstBinding = 4;
		depthWrite.dstArrayElement = 0;
		depthWrite.descriptorCount = 1;
		depthWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		depthWrite.pImageInfo = &depthInfo;

		// Bindings 5, 6: Fallback texture
		vk::DescriptorImageInfo sceneColorFallback;
		sceneColorFallback.sampler = defaultSampler;
		sceneColorFallback.imageView = fallbackView;
		sceneColorFallback.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet bind5Write;
		bind5Write.dstSet = materialSet;
		bind5Write.dstBinding = 5;
		bind5Write.dstArrayElement = 0;
		bind5Write.descriptorCount = 1;
		bind5Write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		bind5Write.pImageInfo = &sceneColorFallback;

		vk::WriteDescriptorSet bind6Write;
		bind6Write.dstSet = materialSet;
		bind6Write.dstBinding = 6;
		bind6Write.dstArrayElement = 0;
		bind6Write.descriptorCount = 1;
		bind6Write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		bind6Write.pImageInfo = &sceneColorFallback;

		std::array<vk::WriteDescriptorSet, 8> writes = {
			texWrite, modelWrite, decalWrite, fallbackTexWrite,
			ssboWrite, depthWrite, bind5Write, bind6Write
		};
		m_device.updateDescriptorSets(writes, {});
	}

	// Allocate PerDraw descriptor set (Set 2) with fog UBO
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);

	{
		Assertion(m_bloomUBOCursor < BLOOM_UBO_MAX_SLOTS, "Fog UBO slot overflow!");
		uint32_t slotOffset = m_bloomUBOCursor * static_cast<uint32_t>(BLOOM_UBO_SLOT_SIZE);
		memcpy(static_cast<uint8_t*>(m_bloomUBOMapped) + slotOffset, &fogData, sizeof(fogData));
		m_bloomUBOCursor++;

		vk::DescriptorBufferInfo uboInfo;
		uboInfo.buffer = m_bloomUBO;
		uboInfo.offset = slotOffset;
		uboInfo.range = BLOOM_UBO_SLOT_SIZE;

		vk::WriteDescriptorSet write;
		write.dstSet = perDrawSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eUniformBuffer;
		write.pBufferInfo = &uboInfo;

		auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo fallbackInfo;
		fallbackInfo.buffer = fallbackBuf;
		fallbackInfo.offset = 0;
		fallbackInfo.range = 4096;

		SCP_vector<vk::WriteDescriptorSet> writes;
		writes.push_back(write);
		for (uint32_t b = 1; b <= 4; ++b) {
			vk::WriteDescriptorSet fw;
			fw.dstSet = perDrawSet;
			fw.dstBinding = b;
			fw.dstArrayElement = 0;
			fw.descriptorCount = 1;
			fw.descriptorType = vk::DescriptorType::eUniformBuffer;
			fw.pBufferInfo = &fallbackInfo;
			writes.push_back(fw);
		}

		m_device.updateDescriptorSets(writes, {});
	}

	// Bind descriptor sets and draw
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();

	// Scene color is now in eColorAttachmentOptimal (fog render pass finalLayout)

	m_memoryManager->unmapMemory(m_bloomUBOAlloc);
	m_bloomUBOMapped = nullptr;
}

void VulkanPostProcessor::renderVolumetricFog(vk::CommandBuffer cmd)
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
		mprintf(("VulkanPostProcessor::renderVolumetricFog: Volume texture not available\n"));
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
		uint32_t dim = std::max(m_extent.width, m_extent.height);
		while (dim > 1) {
			dim >>= 1;
			m_emissiveMipLevels++;
		}

		vk::ImageCreateInfo imgInfo;
		imgInfo.imageType = vk::ImageType::e2D;
		imgInfo.format = vk::Format::eR16G16B16A16Sfloat;
		imgInfo.extent = vk::Extent3D(m_extent.width, m_extent.height, 1);
		imgInfo.mipLevels = m_emissiveMipLevels;
		imgInfo.arrayLayers = 1;
		imgInfo.samples = vk::SampleCountFlagBits::e1;
		imgInfo.tiling = vk::ImageTiling::eOptimal;
		imgInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst
		              | vk::ImageUsageFlagBits::eSampled;
		imgInfo.sharingMode = vk::SharingMode::eExclusive;
		imgInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_emissiveMipmapped.image = m_device.createImage(imgInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create mipmapped emissive: %s\n", e.what()));
			return;
		}

		Verify(m_memoryManager->allocateImageMemory(m_emissiveMipmapped.image, MemoryUsage::GpuOnly, m_emissiveMipmapped.allocation));

		// Create full-mip-chain view for LOD sampling
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_emissiveMipmapped.image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = vk::Format::eR16G16B16A16Sfloat;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = m_emissiveMipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		try {
			m_emissiveMipmappedFullView = m_device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create mipmapped emissive view: %s\n", e.what()));
			return;
		}

		m_emissiveMipmapped.format = vk::Format::eR16G16B16A16Sfloat;
		m_emissiveMipmapped.width = m_extent.width;
		m_emissiveMipmapped.height = m_extent.height;
		m_emissiveMipmappedInitialized = true;
	}

	// Copy G-buffer emissive (mip 0) to mipmapped emissive, then generate mips.
	// dstMipLevels transitions ALL mip levels to eTransferDstOptimal in the pre-barrier.
	// Skip dst post-barrier (stays in eTransferDstOptimal for generateMipmaps).
	copyImageToImage(cmd,
		m_gbufEmissive.image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		m_emissiveMipmapped.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
		m_extent,
		vk::ImageAspectFlagBits::eColor,
		m_emissiveMipLevels);

	// Generate mipmaps via blit chain (expects dst in eTransferDstOptimal).
	// After return, all mips are in eShaderReadOnlyOptimal.
	generateMipmaps(cmd, m_emissiveMipmapped.image, m_extent.width, m_extent.height, m_emissiveMipLevels);

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
		barrier.image = m_sceneColor.image;
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, nullptr, nullptr, barrier);
	}

	// Map bloom UBO for volumetric fog UBO data
	m_bloomUBOMapped = m_memoryManager->mapMemory(m_bloomUBOAlloc);
	Verify(m_bloomUBOMapped);

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
		volData.doEdgeSmoothing = neb.getEdgeSmoothing() ? 1 : 0;
		volData.useNoise = noiseActive ? 1 : 0;
	}

	// We need to use a custom descriptor write because the volumetric shader uses sampler3D
	// at bindings 5 and 6, which differs from the default drawFullscreenTriangle fallbacks (sampler2D).
	// So we replicate the drawFullscreenTriangle pattern but customize the material set.

	PipelineConfig config;
	config.shaderType = SDR_TYPE_VOLUMETRIC_FOG;
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
		m_memoryManager->unmapMemory(m_bloomUBOAlloc);
		m_bloomUBOMapped = nullptr;
		return;
	}

	vk::PipelineLayout pipelineLayout = pipelineMgr->getPipelineLayout();

	// Begin render pass
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = m_fogRenderPass;
	rpBegin.framebuffer = m_fogFramebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_extent;

	cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	vk::Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_extent.width);
	viewport.height = static_cast<float>(m_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmd.setViewport(0, viewport);

	vk::Rect2D scissor;
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_extent;
	cmd.setScissor(0, scissor);

	// Allocate Material descriptor set (Set 1)
	vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);

	{
		auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo fallbackBufInfo;
		fallbackBufInfo.buffer = fallbackBuf;
		fallbackBufInfo.offset = 0;
		fallbackBufInfo.range = 4096;

		vk::Sampler defaultSampler = texMgr->getDefaultSampler();
		vk::ImageView fallbackView = texMgr->getFallbackTextureView2D();
		vk::ImageView fallback3DView = texMgr->getFallback3DView();

		// Binding 0: ModelData UBO (fallback)
		vk::WriteDescriptorSet modelWrite;
		modelWrite.dstSet = materialSet;
		modelWrite.dstBinding = 0;
		modelWrite.dstArrayElement = 0;
		modelWrite.descriptorCount = 1;
		modelWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		modelWrite.pBufferInfo = &fallbackBufInfo;

		// Binding 1: Texture array — [0]=composite, [1]=emissive, rest=fallback
		std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texArrayInfos;
		texArrayInfos[0].sampler = m_linearSampler;
		texArrayInfos[0].imageView = m_gbufComposite.view;
		texArrayInfos[0].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		texArrayInfos[1].sampler = m_mipmapSampler;
		texArrayInfos[1].imageView = m_emissiveMipmappedFullView;
		texArrayInfos[1].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		for (size_t i = 2; i < texArrayInfos.size(); i++) {
			texArrayInfos[i].sampler = defaultSampler;
			texArrayInfos[i].imageView = fallbackView;
			texArrayInfos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		vk::WriteDescriptorSet texWrite;
		texWrite.dstSet = materialSet;
		texWrite.dstBinding = 1;
		texWrite.dstArrayElement = 0;
		texWrite.descriptorCount = static_cast<uint32_t>(texArrayInfos.size());
		texWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		texWrite.pImageInfo = texArrayInfos.data();

		// Binding 2: DecalGlobals UBO (fallback)
		vk::WriteDescriptorSet decalWrite;
		decalWrite.dstSet = materialSet;
		decalWrite.dstBinding = 2;
		decalWrite.dstArrayElement = 0;
		decalWrite.descriptorCount = 1;
		decalWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		decalWrite.pBufferInfo = &fallbackBufInfo;

		// Binding 3: Transform SSBO (fallback)
		vk::WriteDescriptorSet ssboWrite;
		ssboWrite.dstSet = materialSet;
		ssboWrite.dstBinding = 3;
		ssboWrite.dstArrayElement = 0;
		ssboWrite.descriptorCount = 1;
		ssboWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
		ssboWrite.pBufferInfo = &fallbackBufInfo;

		// Binding 4: Depth copy
		vk::DescriptorImageInfo depthInfo;
		depthInfo.sampler = m_linearSampler;
		depthInfo.imageView = m_sceneDepthCopy.view;
		depthInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet depthWrite;
		depthWrite.dstSet = materialSet;
		depthWrite.dstBinding = 4;
		depthWrite.dstArrayElement = 0;
		depthWrite.descriptorCount = 1;
		depthWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		depthWrite.pImageInfo = &depthInfo;

		// Binding 5: 3D volume texture
		vk::DescriptorImageInfo volumeInfo;
		volumeInfo.sampler = m_linearSampler;
		volumeInfo.imageView = volSlot->imageView;
		volumeInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet volumeWrite;
		volumeWrite.dstSet = materialSet;
		volumeWrite.dstBinding = 5;
		volumeWrite.dstArrayElement = 0;
		volumeWrite.descriptorCount = 1;
		volumeWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		volumeWrite.pImageInfo = &volumeInfo;

		// Binding 6: 3D noise texture (or fallback 3D if noise inactive)
		vk::DescriptorImageInfo noiseInfo;
		noiseInfo.sampler = m_linearSampler;
		if (noiseSlot && noiseSlot->imageView) {
			noiseInfo.imageView = noiseSlot->imageView;
		} else {
			noiseInfo.imageView = fallback3DView;
		}
		noiseInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::WriteDescriptorSet noiseWrite;
		noiseWrite.dstSet = materialSet;
		noiseWrite.dstBinding = 6;
		noiseWrite.dstArrayElement = 0;
		noiseWrite.descriptorCount = 1;
		noiseWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		noiseWrite.pImageInfo = &noiseInfo;

		std::array<vk::WriteDescriptorSet, 7> writes = {
			modelWrite, texWrite, decalWrite, ssboWrite,
			depthWrite, volumeWrite, noiseWrite
		};
		m_device.updateDescriptorSets(writes, {});
	}

	// Allocate PerDraw descriptor set (Set 2) with volumetric fog UBO
	vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);

	{
		Assertion(m_bloomUBOCursor < BLOOM_UBO_MAX_SLOTS, "Fog UBO slot overflow!");
		uint32_t slotOffset = m_bloomUBOCursor * static_cast<uint32_t>(BLOOM_UBO_SLOT_SIZE);
		memcpy(static_cast<uint8_t*>(m_bloomUBOMapped) + slotOffset, &volData, sizeof(volData));
		m_bloomUBOCursor++;

		vk::DescriptorBufferInfo uboInfo;
		uboInfo.buffer = m_bloomUBO;
		uboInfo.offset = slotOffset;
		uboInfo.range = BLOOM_UBO_SLOT_SIZE;

		vk::WriteDescriptorSet write;
		write.dstSet = perDrawSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eUniformBuffer;
		write.pBufferInfo = &uboInfo;

		auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
		vk::DescriptorBufferInfo fallbackInfo;
		fallbackInfo.buffer = fallbackBuf;
		fallbackInfo.offset = 0;
		fallbackInfo.range = 4096;

		SCP_vector<vk::WriteDescriptorSet> writes;
		writes.push_back(write);
		for (uint32_t b = 1; b <= 4; ++b) {
			vk::WriteDescriptorSet fw;
			fw.dstSet = perDrawSet;
			fw.dstBinding = b;
			fw.dstArrayElement = 0;
			fw.descriptorCount = 1;
			fw.descriptorType = vk::DescriptorType::eUniformBuffer;
			fw.pBufferInfo = &fallbackInfo;
			writes.push_back(fw);
		}

		m_device.updateDescriptorSets(writes, {});
	}

	// Bind descriptor sets and draw
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
		static_cast<uint32_t>(DescriptorSetIndex::Material),
		{materialSet, perDrawSet}, {});

	cmd.draw(3, 1, 0, 0);
	cmd.endRenderPass();

	// Scene color is now in eColorAttachmentOptimal (fog render pass finalLayout)

	m_memoryManager->unmapMemory(m_bloomUBOAlloc);
	m_bloomUBOMapped = nullptr;
}

void copyImageToImage(
    vk::CommandBuffer cmd,
    vk::Image src, vk::ImageLayout srcOldLayout, vk::ImageLayout srcNewLayout,
    vk::Image dst, vk::ImageLayout dstOldLayout, vk::ImageLayout dstNewLayout,
    vk::Extent2D extent,
    vk::ImageAspectFlags aspect,
    uint32_t dstMipLevels)
{
	// Derive access mask and pipeline stage from a layout.
	// 'leaving' = true for srcAccessMask (flushing writes before transition),
	// false for dstAccessMask (making data available after transition).
	auto layoutInfo = [](vk::ImageLayout layout, bool leaving)
	    -> std::pair<vk::AccessFlags, vk::PipelineStageFlags> {
		switch (layout) {
		case vk::ImageLayout::eUndefined:
			return {{}, vk::PipelineStageFlagBits::eTopOfPipe};
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			return {leaving ? vk::AccessFlags{} : vk::AccessFlagBits::eShaderRead,
			        vk::PipelineStageFlagBits::eFragmentShader};
		case vk::ImageLayout::eColorAttachmentOptimal:
			return {leaving ? vk::AccessFlagBits::eColorAttachmentWrite
			               : (vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite),
			        vk::PipelineStageFlagBits::eColorAttachmentOutput};
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			return {leaving ? vk::AccessFlagBits::eDepthStencilAttachmentWrite
			               : (vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite),
			        leaving ? vk::PipelineStageFlagBits::eLateFragmentTests
			                : vk::PipelineStageFlagBits::eEarlyFragmentTests};
		case vk::ImageLayout::eTransferSrcOptimal:
			return {vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer};
		case vk::ImageLayout::eTransferDstOptimal:
			return {vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer};
		default:
			Assertion(false, "copyImageToImage: unsupported layout %d", static_cast<int>(layout));
			return {{}, vk::PipelineStageFlagBits::eAllCommands};
		}
	};

	// 1. Pre-barriers: transition src → eTransferSrcOptimal, dst → eTransferDstOptimal
	{
		auto [srcAccess, srcStage] = layoutInfo(srcOldLayout, true);
		auto [dstAccess, dstStage] = layoutInfo(dstOldLayout, true);

		std::array<vk::ImageMemoryBarrier, 2> barriers;

		barriers[0].srcAccessMask = srcAccess;
		barriers[0].dstAccessMask = vk::AccessFlagBits::eTransferRead;
		barriers[0].oldLayout = srcOldLayout;
		barriers[0].newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].image = src;
		barriers[0].subresourceRange = {aspect, 0, 1, 0, 1};

		barriers[1].srcAccessMask = dstAccess;
		barriers[1].dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		barriers[1].oldLayout = dstOldLayout;
		barriers[1].newLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].image = dst;
		barriers[1].subresourceRange = {aspect, 0, dstMipLevels, 0, 1};

		cmd.pipelineBarrier(
			srcStage | dstStage,
			vk::PipelineStageFlagBits::eTransfer,
			{}, nullptr, nullptr, barriers);
	}

	// 2. Copy (always mip 0, layer 0)
	{
		vk::ImageCopy region;
		region.srcSubresource = {aspect, 0, 0, 1};
		region.dstSubresource = {aspect, 0, 0, 1};
		region.extent = vk::Extent3D(extent.width, extent.height, 1);

		cmd.copyImage(
			src, vk::ImageLayout::eTransferSrcOptimal,
			dst, vk::ImageLayout::eTransferDstOptimal,
			region);
	}

	// 3. Post-barriers: transition src → srcNewLayout, dst → dstNewLayout
	// Skip rule: if newLayout matches the transfer layout, skip that barrier
	{
		bool skipSrc = (srcNewLayout == vk::ImageLayout::eTransferSrcOptimal);
		bool skipDst = (dstNewLayout == vk::ImageLayout::eTransferDstOptimal);

		if (skipSrc && skipDst) {
			return;
		}

		std::array<vk::ImageMemoryBarrier, 2> barriers;
		uint32_t count = 0;
		vk::PipelineStageFlags postDstStage = {};

		if (!skipSrc) {
			auto [access, stage] = layoutInfo(srcNewLayout, false);
			barriers[count].srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barriers[count].dstAccessMask = access;
			barriers[count].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barriers[count].newLayout = srcNewLayout;
			barriers[count].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].image = src;
			barriers[count].subresourceRange = {aspect, 0, 1, 0, 1};
			count++;
			postDstStage |= stage;
		}

		if (!skipDst) {
			auto [access, stage] = layoutInfo(dstNewLayout, false);
			barriers[count].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barriers[count].dstAccessMask = access;
			barriers[count].oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barriers[count].newLayout = dstNewLayout;
			barriers[count].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[count].image = dst;
			barriers[count].subresourceRange = {aspect, 0, dstMipLevels, 0, 1};
			count++;
			postDstStage |= stage;
		}

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			postDstStage,
			{}, nullptr, nullptr,
			vk::ArrayProxy<const vk::ImageMemoryBarrier>(count, barriers.data()));
	}
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
	for (size_t idx = 0; idx < postEffects.size(); idx++) {
		if (!stricmp(postEffects[idx].name.c_str(), name)) {
			postEffects[idx].intensity = (value / postEffects[idx].div) + postEffects[idx].add;
			if ((rgb != nullptr) && !(vmd_zero_vector == *rgb)) {
				postEffects[idx].rgb = *rgb;
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

} // namespace vulkan
} // namespace graphics
