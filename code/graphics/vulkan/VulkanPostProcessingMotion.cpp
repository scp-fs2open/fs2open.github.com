#include "VulkanPostProcessing.h"

#include "gr_vulkan.h"
#include "VulkanBarrier.h"
#include "VulkanRenderer.h"
#include "VulkanTexture.h"
#include "graphics/matrix.h"
#include "graphics/openxr.h"

namespace graphics::vulkan {

namespace {

struct MotionVectorData {
	matrix4 reprojection;
	float viewportRect[4];
	float sceneNear;
	float cockpitNear;
	float farPlane;
	float pad;
};

} // namespace

bool VulkanMotionVectors::wanted()
{
	// The attachment needs blend state of its own, and VR isn't supported yet.
	if (!getRendererInstance()->supportsIndependentBlend() || openxr_requested()) {
		return false;
	}
	return Gr_fsr_mode != FsrMode::Off && gr_is_capable(gr_capability::CAPABILITY_FSR);
}

void VulkanMotionVectors::init(PostProcessContext& ctx)
{
	m_ctx = &ctx;
}

bool VulkanMotionVectors::createResources()
{
	if (m_framebuffer) {
		return true;
	}

	if (!m_renderPass) {
		std::array<vk::AttachmentDescription, 2> attachments;
		std::array<vk::AttachmentReference, 2> colorRefs;
		for (uint32_t i = 0; i < attachments.size(); ++i) {
			attachments[i].format = i == 0 ? FORMAT : DEPTH_FORMAT;
			attachments[i].loadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].storeOp = vk::AttachmentStoreOp::eStore;
			attachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachments[i].initialLayout = vk::ImageLayout::eUndefined;
			attachments[i].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			colorRefs[i].attachment = i;
			colorRefs[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
		}

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
		subpass.pColorAttachments = colorRefs.data();

		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
		                        | vk::PipelineStageFlagBits::eFragmentShader
		                        | vk::PipelineStageFlagBits::eComputeShader;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eShaderRead;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo rpInfo;
		rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		rpInfo.pAttachments = attachments.data();
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 1;
		rpInfo.pDependencies = &dependency;

		try {
			m_renderPass = m_ctx->device.createRenderPass(rpInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "VulkanMotionVectors: Failed to create render pass: %s\n", e.what()));
			return false;
		}
	}

	const uint32_t w = m_ctx->sceneExtent.width;
	const uint32_t h = m_ctx->sceneExtent.height;

	const auto createTarget = [this, w, h](RenderTarget& target, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) {
		target.format = format;
		target.width = w;
		target.height = h;
		return m_ctx->createImage(w, h, format, usage, aspect, target.image, target.view, target.allocation);
	};

	const auto renderable = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
	if (!createTarget(m_target, FORMAT, renderable, vk::ImageAspectFlagBits::eColor)
		|| !createTarget(m_drawnMotion, DRAWN_FORMAT, renderable, vk::ImageAspectFlagBits::eColor)
		|| !createTarget(m_depth, DEPTH_FORMAT, renderable, vk::ImageAspectFlagBits::eColor)) {
		nprintf(("vulkan", "VulkanMotionVectors: Failed to create image!\n"));
		destroyTargets();
		return false;
	}

	const std::array<vk::ImageView, 2> views = {m_target.view, m_depth.view};

	vk::FramebufferCreateInfo fbInfo;
	fbInfo.renderPass = m_renderPass;
	fbInfo.attachmentCount = static_cast<uint32_t>(views.size());
	fbInfo.pAttachments = views.data();
	fbInfo.width = w;
	fbInfo.height = h;
	fbInfo.layers = 1;

	try {
		m_framebuffer = m_ctx->device.createFramebuffer(fbInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanMotionVectors: Failed to create framebuffer: %s\n", e.what()));
		destroyTargets();
		return false;
	}

	return true;
}

void VulkanMotionVectors::destroyTargets()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}

	if (m_framebuffer) {
		m_ctx->device.destroyFramebuffer(m_framebuffer);
		m_framebuffer = nullptr;
	}
	m_ctx->destroyTarget(m_target);
	m_ctx->destroyTarget(m_drawnMotion);
	m_ctx->destroyTarget(m_depth);
}

void VulkanMotionVectors::shutdown()
{
	if (!m_ctx || !m_ctx->device) {
		return;
	}

	destroyTargets();

	if (m_renderPass) {
		m_ctx->device.destroyRenderPass(m_renderPass);
		m_renderPass = nullptr;
	}
}

void VulkanMotionVectors::execute(vk::CommandBuffer cmd, vk::ImageView sceneDepthCopy)
{
	GR_DEBUG_SCOPE("Motion vectors");

	ImageBarrier2 barrier;
	barrier.image = m_drawnMotion.image;
	barrier.levelCount = 1;
	barrier.layerCount = 1;
	barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	barrier.srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
	barrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
	barrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;
	cmdImageBarrier(cmd, barrier);

	MotionVectorData data;
	matrix4 viewProj, previousViewProj, inverseViewProj;
	gr_get_velocity_camera(&viewProj, &previousViewProj, data.viewportRect);

	// Not invertible until a scene has captured a camera
	if (vm_inverse_matrix4(&inverseViewProj, &viewProj)) {
		vm_matrix4_x_matrix4(&data.reprojection, &previousViewProj, &inverseViewProj);
	} else {
		vm_matrix4_set_identity(&data.reprojection);
	}

	vk::Sampler nearestSampler = getTextureManager()->getSampler(
		vk::Filter::eNearest, vk::Filter::eNearest,
		vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false);

	data.sceneNear = Min_draw_distance;
	data.cockpitNear = Min_draw_distance_cockpit;
	data.farPlane = Max_draw_distance;
	data.pad = 0.0f;

	const std::array<vk::ImageView, 2> inputs = {sceneDepthCopy, m_drawnMotion.view};
	m_ctx->drawFullscreenTriangleMulti(cmd, m_renderPass, m_framebuffer, m_ctx->sceneExtent,
		SDR_TYPE_MOTION_VECTORS, inputs.data(), static_cast<uint32_t>(inputs.size()), &data, sizeof(data), nearestSampler, 2);
}

} // namespace graphics::vulkan
