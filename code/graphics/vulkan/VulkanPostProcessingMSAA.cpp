#include "VulkanPostProcessing.h"

#include <array>

#include "cmdline/cmdline.h"
#include "gr_vulkan.h"
#include "VulkanRenderer.h"
#include "VulkanTexture.h"
#include "VulkanDescriptorManager.h"
#include "lighting/lighting_profiles.h"
#include "lighting/lighting.h"
#include "nebula/neb.h"
#include "mission/missionparse.h"

extern float Sun_spot;
extern int Game_subspace_effect;
extern SCP_vector<light> Lights;
extern int Num_lights;


namespace graphics::vulkan {


// ===== MSAA G-Buffer =====

bool VulkanDeferredGBuffer::initMsaa()
{
	if (m_msaaInitialized) {
		return true;
	}

	auto* renderer = getRendererInstance();
	vk::SampleCountFlagBits msaaSamples = renderer->getMsaaSampleCount();
	if (msaaSamples == vk::SampleCountFlagBits::e1) {
		return false;
	}

	const uint32_t w = m_ctx->sceneExtent.width;
	const uint32_t h = m_ctx->sceneExtent.height;
	const vk::ImageUsageFlags msaaUsage =
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

	// Create MSAA color images (5 total: color, position, normal, specular, emissive)
	struct MsaaTarget {
		RenderTarget* target;
		vk::Format format;
		const char* name;
	};

	std::array<MsaaTarget, 5> targets = {{
		{&m_msaaColor,    GBUF_FORMAT_COLOR,    "msaa-color"},
		{&m_msaaPosition, GBUF_FORMAT_POSITION, "msaa-position"},
		{&m_msaaNormal,   GBUF_FORMAT_NORMAL,   "msaa-normal"},
		{&m_msaaSpecular, GBUF_FORMAT_SPECULAR, "msaa-specular"},
		{&m_msaaEmissive, GBUF_FORMAT_EMISSIVE, "msaa-emissive"},
	}};

	for (auto& t : targets) {
		if (!m_ctx->createImage(w, h, t.format, msaaUsage, vk::ImageAspectFlagBits::eColor,
		                 t.target->image, t.target->view, t.target->allocation, msaaSamples)) {
			mprintf(("VulkanPostProcessor: Failed to create %s image!\n", t.name));
			shutdownMsaa();
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
		imageInfo.format = m_ctx->depthFormat;
		imageInfo.extent = vk::Extent3D(w, h, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = msaaSamples;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_msaaDepthImage = m_ctx->device.createImage(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA depth image: %s\n", e.what()));
			shutdownMsaa();
			return false;
		}

		if (!m_ctx->memoryManager->allocateImageMemory(m_msaaDepthImage, MemoryUsage::GpuOnly, m_msaaDepthAlloc)) {
			mprintf(("VulkanPostProcessor: Failed to allocate MSAA depth memory!\n"));
			m_ctx->device.destroyImage(m_msaaDepthImage);
			m_msaaDepthImage = nullptr;
			shutdownMsaa();
			return false;
		}

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_msaaDepthImage;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = m_ctx->depthFormat;
		viewInfo.subresourceRange.aspectMask = imageAspectFromFormat(m_ctx->depthFormat);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		try {
			m_msaaDepthView = m_ctx->device.createImageView(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA depth view: %s\n", e.what()));
			shutdownMsaa();
			return false;
		}
	}

	// MSAA G-buffer render pass (eClear) — 5 color + depth
	try {
		m_msaaGbufRenderPass = createGbufRenderPass({
			false, msaaSamples,
			vk::AttachmentLoadOp::eClear, vk::AttachmentLoadOp::eClear,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
			false, // useResolveDependency
			{{GBUF_ATT_EMISSIVE, vk::AttachmentLoadOp::eLoad}},
		});
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create MSAA G-buffer render pass: %s\n", e.what()));
		shutdownMsaa();
		return false;
	}

	// MSAA G-buffer render pass (eLoad) — emissive preserving variant
	try {
		m_msaaGbufRenderPassLoad = createGbufRenderPass({
			false, msaaSamples,
			vk::AttachmentLoadOp::eLoad, vk::AttachmentLoadOp::eLoad,
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::eDepthStencilAttachmentOptimal,
		});
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create MSAA G-buffer load render pass: %s\n", e.what()));
		shutdownMsaa();
		return false;
	}

	// MSAA G-buffer framebuffer (5 color + depth)
	try {
		m_msaaGbufFramebuffer = createGbufFramebuffer(m_msaaGbufRenderPass, false, true);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create MSAA G-buffer framebuffer: %s\n", e.what()));
		shutdownMsaa();
		return false;
	}

	// Emissive copy render pass — 1 MS color attachment for upsampling non-MSAA → MSAA
	{
		vk::AttachmentDescription att;
		att.format = HDR_COLOR_FORMAT;
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
			m_msaaEmissiveCopyRenderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA emissive copy render pass: %s\n", e.what()));
			shutdownMsaa();
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
			m_msaaEmissiveCopyFramebuffer = m_ctx->device.createFramebuffer(fbInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA emissive copy framebuffer: %s\n", e.what()));
			shutdownMsaa();
			return false;
		}
	}

	// MSAA Resolve render pass — 5 non-MSAA color + depth (via gl_FragDepth)
	// Writes to the non-MSAA G-buffer images. loadOp=eDontCare (fully overwritten).
	try {
		m_msaaResolveRenderPass = createGbufRenderPass({
			false, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eUndefined,
			true, // useResolveDependency
		});
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create MSAA resolve render pass: %s\n", e.what()));
		shutdownMsaa();
		return false;
	}

	// MSAA Resolve framebuffer — references non-MSAA G-buffer images
	try {
		m_msaaResolveFramebuffer = createGbufFramebuffer(m_msaaResolveRenderPass, false, false);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPostProcessor: Failed to create MSAA resolve framebuffer: %s\n", e.what()));
		shutdownMsaa();
		return false;
	}

	// Create per-frame MSAA resolve UBO (persistently mapped)
	// Two 256-byte slots (one per frame in flight) hold {int samples; float fov;} data.
	{
		vk::BufferCreateInfo bufInfo;
		bufInfo.size = MAX_FRAMES_IN_FLIGHT * 256;
		bufInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			m_msaaResolveUBO = m_ctx->device.createBuffer(bufInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanPostProcessor: Failed to create MSAA resolve UBO: %s\n", e.what()));
			shutdownMsaa();
			return false;
		}

		if (!m_ctx->memoryManager->allocateBufferMemory(m_msaaResolveUBO, MemoryUsage::CpuToGpu, m_msaaResolveUBOAlloc)) {
			mprintf(("VulkanPostProcessor: Failed to allocate MSAA resolve UBO memory!\n"));
			m_ctx->device.destroyBuffer(m_msaaResolveUBO);
			m_msaaResolveUBO = nullptr;
			shutdownMsaa();
			return false;
		}

		m_msaaResolveUBOMapped = m_ctx->memoryManager->mapMemory(m_msaaResolveUBOAlloc);
		if (!m_msaaResolveUBOMapped) {
			mprintf(("VulkanPostProcessor: Failed to map MSAA resolve UBO!\n"));
			shutdownMsaa();
			return false;
		}
	}

	// Transition MSAA images to the render pass's initial layout at creation time.
	// The validation layer tracks framebuffer attachment layouts from creation,
	// so we must match the eClear render pass's initialLayout exactly.
	{
		auto* texMgr = getTextureManager();

		std::array<RenderTarget*, 5> colorTargets = {
			&m_msaaColor, &m_msaaPosition, &m_msaaNormal,
			&m_msaaSpecular, &m_msaaEmissive,
		};
		for (auto* t : colorTargets) {
			texMgr->transitionImageLayout(t->image, t->format,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
		}

		texMgr->transitionImageLayout(m_msaaDepthImage, m_ctx->depthFormat,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	}

	m_msaaInitialized = true;
	mprintf(("VulkanPostProcessor: MSAA initialized (%ux%u, %dx samples, 5 color + depth)\n",
		w, h, Cmdline_msaa_enabled));
	return true;
}

void VulkanDeferredGBuffer::shutdownMsaa()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}

	// Destroy MSAA resolve UBO
	if (m_msaaResolveUBOMapped) {
		m_ctx->memoryManager->unmapMemory(m_msaaResolveUBOAlloc);
		m_msaaResolveUBOMapped = nullptr;
	}
	if (m_msaaResolveUBO) {
		m_ctx->device.destroyBuffer(m_msaaResolveUBO);
		m_msaaResolveUBO = nullptr;
	}
	if (m_msaaResolveUBOAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_msaaResolveUBOAlloc);
	}

	if (m_msaaResolveFramebuffer) {
		m_ctx->device.destroyFramebuffer(m_msaaResolveFramebuffer);
		m_msaaResolveFramebuffer = nullptr;
	}
	if (m_msaaResolveRenderPass) {
		m_ctx->device.destroyRenderPass(m_msaaResolveRenderPass);
		m_msaaResolveRenderPass = nullptr;
	}
	if (m_msaaEmissiveCopyFramebuffer) {
		m_ctx->device.destroyFramebuffer(m_msaaEmissiveCopyFramebuffer);
		m_msaaEmissiveCopyFramebuffer = nullptr;
	}
	if (m_msaaEmissiveCopyRenderPass) {
		m_ctx->device.destroyRenderPass(m_msaaEmissiveCopyRenderPass);
		m_msaaEmissiveCopyRenderPass = nullptr;
	}
	if (m_msaaGbufFramebuffer) {
		m_ctx->device.destroyFramebuffer(m_msaaGbufFramebuffer);
		m_msaaGbufFramebuffer = nullptr;
	}
	if (m_msaaGbufRenderPassLoad) {
		m_ctx->device.destroyRenderPass(m_msaaGbufRenderPassLoad);
		m_msaaGbufRenderPassLoad = nullptr;
	}
	if (m_msaaGbufRenderPass) {
		m_ctx->device.destroyRenderPass(m_msaaGbufRenderPass);
		m_msaaGbufRenderPass = nullptr;
	}

	// Destroy MSAA depth
	if (m_msaaDepthView) {
		m_ctx->device.destroyImageView(m_msaaDepthView);
		m_msaaDepthView = nullptr;
	}
	if (m_msaaDepthImage) {
		m_ctx->device.destroyImage(m_msaaDepthImage);
		m_msaaDepthImage = nullptr;
	}
	if (m_msaaDepthAlloc.isValid()) {
		m_ctx->memoryManager->freeAllocation(m_msaaDepthAlloc);
	}

	// Destroy MSAA color targets
	std::array<RenderTarget*, 5> msaaTargets = {
		&m_msaaColor, &m_msaaPosition, &m_msaaNormal,
		&m_msaaSpecular, &m_msaaEmissive,
	};
	for (auto* rt : msaaTargets) {
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

	m_msaaInitialized = false;
}

void VulkanDeferredGBuffer::transitionMsaaForResume(vk::CommandBuffer /*cmd*/)
{
	// No-op: MSAA render passes use finalLayout == subpass layout (no implicit
	// transition at endRenderPass), so color attachments remain in
	// eColorAttachmentOptimal — exactly what the eLoad pass expects.
}

void VulkanDeferredGBuffer::transitionMsaaForBegin(vk::CommandBuffer /*cmd*/)
{
	// No-op: MSAA images are always in eColorAttachmentOptimal /
	// eDepthStencilAttachmentOptimal between frames. Init-time transitions
	// set this layout, and the post-resolve barriers in
	// vulkan_deferred_lighting_msaa restore it after each frame's resolve pass.
}

} // namespace graphics::vulkan
