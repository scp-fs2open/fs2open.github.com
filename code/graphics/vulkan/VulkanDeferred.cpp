
#include "VulkanDeferred.h"

#include <array>

#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanDescriptorManager.h"
#include "VulkanPipeline.h"
#include "VulkanState.h"
#include "VulkanDraw.h"
#include "VulkanPostProcessing.h"
#include "gr_vulkan.h"

#include "cmdline/cmdline.h"
#include "graphics/2d.h"
#include "graphics/matrix.h"
#include "graphics/material.h"
#include "graphics/grinternal.h"
#include "graphics/shadows.h"
#include "lighting/lighting.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "nebula/volumetrics.h"
#include "render/3d.h"

namespace graphics {
namespace vulkan {

namespace {

static bool s_vulkanOverrideFog = false;

} // anonymous namespace

// ========== Deferred Lighting ==========

void vulkan_deferred_lighting_begin(bool clearNonColorBufs)
{
	if (!light_deferred_enabled()) {
		return;
	}

	auto* pp = getPostProcessor();
	if (!pp || !pp->isGbufInitialized()) {
		return;
	}

	auto* renderer = getRendererInstance();
	if (!renderer->isSceneRendering()) {
		return;
	}

	auto* stateTracker = getStateTracker();
	vk::CommandBuffer cmd = stateTracker->getCommandBuffer();

	const bool msaaActive = (Cmdline_msaa_enabled > 0 && pp->isMsaaInitialized());

	// End the current G-buffer render pass to perform the color→emissive copy.
	// All 6 color attachments transition to eShaderReadOnlyOptimal (finalLayout).
	cmd.endRenderPass();

	// Copy scene color → non-MSAA emissive (pre-deferred content becomes emissive).
	// Skip both post-barriers — conditional MSAA/non-MSAA code below handles transitions.
	copyImageToImage(cmd,
		pp->getSceneColorImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal,
		pp->getGbufEmissiveImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal,
		pp->getSceneExtent());

	if (msaaActive) {
		// --- MSAA path ---
		// Transition scene color: eTransferSrcOptimal → eShaderReadOnlyOptimal
		// (will be sampled inside MSAA pass to fill emissive)
		// Transition non-MSAA emissive: eTransferDstOptimal → eShaderReadOnlyOptimal (preserved for later)
		{
			std::array<vk::ImageMemoryBarrier, 2> barriers;

			barriers[0].srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barriers[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;
			barriers[0].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barriers[0].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].image = pp->getSceneColorImage();
			barriers[0].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

			barriers[1].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barriers[1].dstAccessMask = {};
			barriers[1].oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barriers[1].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].image = pp->getGbufEmissiveImage();
			barriers[1].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

			cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				{}, nullptr, nullptr, barriers);
		}

		// Transition MSAA images to expected initial layouts
		pp->transitionMsaaGbufForBegin(cmd);

		// Begin MSAA G-buffer render pass (eClear — clears all attachments)
		{
			auto extent = pp->getSceneExtent();
			vk::RenderPassBeginInfo rpBegin;
			rpBegin.renderPass = pp->getMsaaGbufRenderPass();
			rpBegin.framebuffer = pp->getMsaaGbufFramebuffer();
			rpBegin.renderArea.offset = vk::Offset2D(0, 0);
			rpBegin.renderArea.extent = extent;
			std::array<vk::ClearValue, 6> clearValues{};
			clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
			clearValues[1].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
			clearValues[2].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
			clearValues[3].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
			clearValues[4].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
			clearValues[5].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
			rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
			rpBegin.pClearValues = clearValues.data();
			cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
			stateTracker->setRenderPass(pp->getMsaaGbufRenderPass(), 0);
			stateTracker->setColorAttachmentCount(VulkanPostProcessor::MSAA_COLOR_ATTACHMENT_COUNT);
			stateTracker->setCurrentSampleCount(renderer->getMsaaSampleCount());
		}

		// Fill MSAA emissive with pre-deferred scene content (starfield, backgrounds).
		// Draw a fullscreen tri sampling non-MSAA scene color, writing to all attachments.
		// Only emissive (attachment 4) matters — the other attachments will be overwritten
		// by model rendering. Use per-attachment color write mask to write only att 4.
		{
			auto* pipelineMgr = getPipelineManager();

			PipelineConfig config;
			config.shaderType = SDR_TYPE_COPY;
			config.primitiveType = PRIM_TYPE_TRIS;
			config.depthMode = ZBUFFER_TYPE_NONE;
			config.blendMode = ALPHA_BLEND_NONE;
			config.cullEnabled = false;
			config.depthWriteEnabled = false;
			config.renderPass = pp->getMsaaGbufRenderPass();
			config.sampleCount = renderer->getMsaaSampleCount();
			config.colorAttachmentCount = VulkanPostProcessor::MSAA_COLOR_ATTACHMENT_COUNT;

			// Per-attachment blend: only write to attachment 4 (emissive)
			config.perAttachmentBlendEnabled = true;
			for (uint32_t i = 0; i < config.colorAttachmentCount; ++i) {
				config.attachmentBlends[i].blendMode = ALPHA_BLEND_NONE;
				config.attachmentBlends[i].writeMask = {false, false, false, false};
			}
			config.attachmentBlends[4].writeMask = {true, true, true, true};

			vertex_layout emptyLayout;
			vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
			if (pipeline) {
				// Use drawFullscreenTriangle pattern but inline since we're already in a render pass
				auto* descriptorMgr = getDescriptorManager();
				auto* bufferMgr = getBufferManager();
				auto* texMgr = getTextureManager();

				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

				auto extent = pp->getSceneExtent();
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

				// Bind descriptors with scene color as source
				auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
				auto fallbackBufSize = static_cast<vk::DeviceSize>(bufferMgr->getFallbackUniformBufferSize());
				auto fallbackView = texMgr->getFallbackTextureView2D();
				auto fallbackSampler = texMgr->getDefaultSampler();

				DescriptorWriter writer;
				writer.reset(descriptorMgr->getDevice());

				vk::DescriptorSet globalSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Global);
				Verify(globalSet);
				writer.writeUniformBuffer(globalSet, 0, fallbackBuf, 0, fallbackBufSize);
				writer.writeUniformBuffer(globalSet, 1, fallbackBuf, 0, fallbackBufSize);
				writer.writeTexture(globalSet, 2, fallbackView, fallbackSampler);
				auto fallbackCubeView = texMgr->getFallbackCubeView();
				writer.writeTexture(globalSet, 3, fallbackCubeView, fallbackSampler);
				writer.writeTexture(globalSet, 4, fallbackCubeView, fallbackSampler);
				writer.flush();
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
					pipelineMgr->getPipelineLayout(),
					static_cast<uint32_t>(DescriptorSetIndex::Global), globalSet, {});

				vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
				Verify(materialSet);
				writer.writeUniformBuffer(materialSet, 0, fallbackBuf, 0, fallbackBufSize);
				writer.writeUniformBuffer(materialSet, 2, fallbackBuf, 0, fallbackBufSize);
				writer.writeStorageBuffer(materialSet, 3, fallbackBuf, 0, fallbackBufSize);

				// Build texture array with scene color at slot 0, fallback at slots 1-15
				std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texImages;
				texImages[0].sampler = pp->getSceneColorSampler();
				texImages[0].imageView = pp->getSceneColorView();
				texImages[0].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				for (uint32_t slot = 1; slot < VulkanDescriptorManager::MAX_TEXTURE_BINDINGS; ++slot) {
					texImages[slot].sampler = fallbackSampler;
					texImages[slot].imageView = fallbackView;
					texImages[slot].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				}
				writer.writeTextureArray(materialSet, 1, texImages.data(), static_cast<uint32_t>(texImages.size()));
				writer.writeTexture(materialSet, 4, fallbackView, fallbackSampler);
				writer.writeTexture(materialSet, 5, fallbackView, fallbackSampler);
				writer.writeTexture(materialSet, 6, fallbackView, fallbackSampler);
				writer.flush();
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
					pipelineMgr->getPipelineLayout(),
					static_cast<uint32_t>(DescriptorSetIndex::Material), materialSet, {});

				vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
				Verify(perDrawSet);
				for (uint32_t b = 0; b < 5; ++b) {
					writer.writeUniformBuffer(perDrawSet, b, fallbackBuf, 0, fallbackBufSize);
				}
				writer.flush();
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
					pipelineMgr->getPipelineLayout(),
					static_cast<uint32_t>(DescriptorSetIndex::PerDraw), perDrawSet, {});

				cmd.draw(3, 1, 0, 0);
			}
		}
	} else {
		// --- Non-MSAA path (original) ---
		// Transition scene color back to eColorAttachmentOptimal.
		// Transition emissive to eShaderReadOnlyOptimal (where transitionGbufForResume expects it).
		{
			std::array<vk::ImageMemoryBarrier, 2> barriers;

			barriers[0].srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barriers[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barriers[0].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barriers[0].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].image = pp->getSceneColorImage();
			barriers[0].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

			barriers[1].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barriers[1].dstAccessMask = {};
			barriers[1].oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barriers[1].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[1].image = pp->getGbufEmissiveImage();
			barriers[1].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

			cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				{}, nullptr, nullptr, barriers);
		}

		// Transition G-buffer attachments 1-5 from eShaderReadOnlyOptimal → eColorAttachmentOptimal
		pp->transitionGbufForResume(cmd);

		// Resume G-buffer render pass with eLoad
		{
			auto extent = pp->getSceneExtent();
			vk::RenderPassBeginInfo rpBegin;
			rpBegin.renderPass = pp->getGbufRenderPassLoad();
			rpBegin.framebuffer = pp->getGbufFramebuffer();
			rpBegin.renderArea.offset = vk::Offset2D(0, 0);
			rpBegin.renderArea.extent = extent;
			std::array<vk::ClearValue, 7> clearValues{};
			clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
			rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
			rpBegin.pClearValues = clearValues.data();
			cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
			stateTracker->setRenderPass(pp->getGbufRenderPassLoad(), 0);
		}

		// Optionally clear non-color G-buffer attachments
		if (clearNonColorBufs) {
			vk::ClearAttachment clearAtt;
			clearAtt.aspectMask = vk::ImageAspectFlagBits::eColor;
			clearAtt.clearValue.color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});

			auto extent = pp->getSceneExtent();
			vk::ClearRect clearRect;
			clearRect.rect.offset = vk::Offset2D(0, 0);
			clearRect.rect.extent = extent;
			clearRect.baseArrayLayer = 0;
			clearRect.layerCount = 1;

			for (uint32_t att : {1u, 2u, 3u, 5u}) {
				clearAtt.colorAttachment = att;
				cmd.clearAttachments(clearAtt, clearRect);
			}
		}
	}

	Deferred_lighting = true;
}

void vulkan_deferred_lighting_msaa()
{
	if (Cmdline_msaa_enabled <= 0) {
		return;
	}

	auto* pp = getPostProcessor();
	if (!pp || !pp->isMsaaInitialized()) {
		return;
	}

	auto* stateTracker = getStateTracker();
	vk::CommandBuffer cmd = stateTracker->getCommandBuffer();

	// End MSAA G-buffer render pass.
	// With finalLayout == subpass layout, all attachments stay in their subpass layouts:
	// colors remain eColorAttachmentOptimal, depth remains eDepthStencilAttachmentOptimal.
	cmd.endRenderPass();

	// Reset sample count to 1x (resolve and subsequent passes are non-MSAA)
	stateTracker->setCurrentSampleCount(vk::SampleCountFlagBits::e1);

	// Explicit barriers: transition all 6 MSAA images to eShaderReadOnlyOptimal
	// for sampling by the resolve shader. We use explicit barriers instead of
	// render pass finalLayout transitions to ensure the validation layer tracks
	// the layout changes correctly.
	{
		std::array<vk::ImageMemoryBarrier, 6> barriers;

		// 5 color images: eColorAttachmentOptimal → eShaderReadOnlyOptimal
		vk::Image msaaImages[5] = {
			pp->getMsaaColorImage(),
			pp->getMsaaPositionImage(),
			pp->getMsaaNormalImage(),
			pp->getMsaaSpecularImage(),
			pp->getMsaaEmissiveImage(),
		};
		for (int i = 0; i < 5; ++i) {
			barriers[i].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barriers[i].dstAccessMask = vk::AccessFlagBits::eShaderRead;
			barriers[i].oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
			barriers[i].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[i].image = msaaImages[i];
			barriers[i].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
		}

		// Depth: eDepthStencilAttachmentOptimal → eShaderReadOnlyOptimal
		barriers[5].srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		barriers[5].dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barriers[5].oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		barriers[5].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[5].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[5].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[5].image = pp->getMsaaDepthImage();
		barriers[5].subresourceRange = {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::PipelineStageFlagBits::eFragmentShader,
			{}, nullptr, nullptr, barriers);
	}

	// Begin resolve render pass (non-MSAA, writes to standard G-buffer images)
	{
		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getMsaaResolveRenderPass();
		rpBegin.framebuffer = pp->getMsaaResolveFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;
		// 6 attachments: 5 color + depth. loadOp=eDontCare for all (fully overwritten).
		std::array<vk::ClearValue, 6> clearValues{};
		clearValues[5].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);

		auto* pipelineMgr = getPipelineManager();
		auto* descriptorMgr = getDescriptorManager();
		auto* bufferMgr = getBufferManager();
		auto* texMgr = getTextureManager();

		PipelineConfig config;
		config.shaderType = SDR_TYPE_MSAA_RESOLVE;
		config.primitiveType = PRIM_TYPE_TRIS;
		config.depthMode = ZBUFFER_TYPE_FULL;
		config.blendMode = ALPHA_BLEND_NONE;
		config.cullEnabled = false;
		config.depthWriteEnabled = true;
		config.renderPass = pp->getMsaaResolveRenderPass();
		config.colorAttachmentCount = 5;

		vertex_layout emptyLayout;
		vk::Pipeline pipeline = pipelineMgr->getPipeline(config, emptyLayout);
		if (pipeline) {
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

			auto fallbackBuf = bufferMgr->getFallbackUniformBuffer();
			auto fallbackBufSize = static_cast<vk::DeviceSize>(bufferMgr->getFallbackUniformBufferSize());
			auto fallbackView = texMgr->getFallbackTextureView2D();
			auto fallbackSampler = texMgr->getDefaultSampler();

			DescriptorWriter writer;
			writer.reset(descriptorMgr->getDevice());

			// Global set (fallback — resolve shader doesn't use global bindings)
			vk::DescriptorSet globalSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Global);
			Verify(globalSet);
			writer.writeUniformBuffer(globalSet, 0, fallbackBuf, 0, fallbackBufSize);
			writer.writeUniformBuffer(globalSet, 1, fallbackBuf, 0, fallbackBufSize);
			writer.writeTexture(globalSet, 2, fallbackView, fallbackSampler);
			auto fallbackCubeView = texMgr->getFallbackCubeView();
			writer.writeTexture(globalSet, 3, fallbackCubeView, fallbackSampler);
			writer.writeTexture(globalSet, 4, fallbackCubeView, fallbackSampler);
			writer.flush();
			writer.reset(descriptorMgr->getDevice());
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				pipelineMgr->getPipelineLayout(),
				static_cast<uint32_t>(DescriptorSetIndex::Global), globalSet, {});

			// Material set: All 6 MSAA textures in binding 1 array (elements 0-5)
			// [0]=color, [1]=position, [2]=normal, [3]=specular, [4]=emissive, [5]=depth
			vk::DescriptorSet materialSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::Material);
			Verify(materialSet);
			writer.writeUniformBuffer(materialSet, 0, fallbackBuf, 0, fallbackBufSize);
			writer.writeUniformBuffer(materialSet, 2, fallbackBuf, 0, fallbackBufSize);
			writer.writeStorageBuffer(materialSet, 3, fallbackBuf, 0, fallbackBufSize);

			// Build texture array: elements 0-5 are MSAA textures, 6-15 are fallback
			vk::Sampler nearestSampler = texMgr->getSampler(
				vk::Filter::eNearest, vk::Filter::eNearest,
				vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false);

			std::array<vk::DescriptorImageInfo, VulkanDescriptorManager::MAX_TEXTURE_BINDINGS> texImages;
			// MSAA textures at slots 0-5
			texImages[0] = {nearestSampler, pp->getMsaaColorView(), vk::ImageLayout::eShaderReadOnlyOptimal};
			texImages[1] = {nearestSampler, pp->getMsaaPositionView(), vk::ImageLayout::eShaderReadOnlyOptimal};
			texImages[2] = {nearestSampler, pp->getMsaaNormalView(), vk::ImageLayout::eShaderReadOnlyOptimal};
			texImages[3] = {nearestSampler, pp->getMsaaSpecularView(), vk::ImageLayout::eShaderReadOnlyOptimal};
			texImages[4] = {nearestSampler, pp->getMsaaEmissiveView(), vk::ImageLayout::eShaderReadOnlyOptimal};
			texImages[5] = {nearestSampler, pp->getMsaaDepthView(), vk::ImageLayout::eShaderReadOnlyOptimal};
			// Remaining slots must also be multisampled (validation checks ALL
			// elements even though the shader only accesses 0-5). Reuse the
			// MSAA color view — content doesn't matter, only sample count.
			for (uint32_t slot = 6; slot < VulkanDescriptorManager::MAX_TEXTURE_BINDINGS; ++slot) {
				texImages[slot] = {nearestSampler, pp->getMsaaColorView(), vk::ImageLayout::eShaderReadOnlyOptimal};
			}
			writer.writeTextureArray(materialSet, 1, texImages.data(), static_cast<uint32_t>(texImages.size()));

			// Fallback for single-sampler bindings 4-6
			writer.writeTexture(materialSet, 4, fallbackView, fallbackSampler);
			writer.writeTexture(materialSet, 5, fallbackView, fallbackSampler);
			writer.writeTexture(materialSet, 6, fallbackView, fallbackSampler);
			writer.flush();
			writer.reset(descriptorMgr->getDevice());
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				pipelineMgr->getPipelineLayout(),
				static_cast<uint32_t>(DescriptorSetIndex::Material), materialSet, {});

			// PerDraw set: GenericData UBO with {samples, fov} at binding 0
			vk::DescriptorSet perDrawSet = descriptorMgr->allocateFrameSet(DescriptorSetIndex::PerDraw);
			Verify(perDrawSet);
			// Write resolve data to per-frame UBO slot
			struct MsaaResolveData {
				int samples;
				float fov;
			} resolveData;
			resolveData.samples = Cmdline_msaa_enabled;
			resolveData.fov = g3_get_hfov(Proj_fov);

			uint32_t frame = bufferMgr->getCurrentFrame();
			uint32_t slotOffset = frame * 256;
			memcpy(static_cast<uint8_t*>(pp->getMsaaResolveUBOMapped()) + slotOffset,
				&resolveData, sizeof(resolveData));

			writer.writeUniformBuffer(perDrawSet, 0,
				pp->getMsaaResolveUBO(), slotOffset, 256);

			// Fallback for remaining PerDraw UBO bindings (1-4)
			for (uint32_t b = 1; b <= 4; ++b) {
				writer.writeUniformBuffer(perDrawSet, b, fallbackBuf, 0, fallbackBufSize);
			}
			writer.flush();
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				pipelineMgr->getPipelineLayout(),
				static_cast<uint32_t>(DescriptorSetIndex::PerDraw), perDrawSet, {});

			cmd.draw(3, 1, 0, 0);
		}

		cmd.endRenderPass();
	}

	// Transition MSAA images back to their resting layout (eColorAttachmentOptimal /
	// eDepthStencilAttachmentOptimal) so they match the validation layer's global
	// tracking state for the next frame. The post-G-buffer barriers moved them to
	// eShaderReadOnlyOptimal for the resolve pass; now we restore them.
	{
		std::array<vk::ImageMemoryBarrier, 6> restoreBarriers;

		vk::Image msaaImages[5] = {
			pp->getMsaaColorImage(),
			pp->getMsaaPositionImage(),
			pp->getMsaaNormalImage(),
			pp->getMsaaSpecularImage(),
			pp->getMsaaEmissiveImage(),
		};
		for (int i = 0; i < 5; ++i) {
			restoreBarriers[i].srcAccessMask = vk::AccessFlagBits::eShaderRead;
			restoreBarriers[i].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			restoreBarriers[i].oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			restoreBarriers[i].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
			restoreBarriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			restoreBarriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			restoreBarriers[i].image = msaaImages[i];
			restoreBarriers[i].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
		}

		restoreBarriers[5].srcAccessMask = vk::AccessFlagBits::eShaderRead;
		restoreBarriers[5].dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		restoreBarriers[5].oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		restoreBarriers[5].newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		restoreBarriers[5].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		restoreBarriers[5].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		restoreBarriers[5].image = pp->getMsaaDepthImage();
		restoreBarriers[5].subresourceRange = {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			{}, nullptr, nullptr, restoreBarriers);
	}

	// After resolve, the non-MSAA G-buffer has properly resolved data.
	// Color attachments 0-4 are in eShaderReadOnlyOptimal (from resolve pass finalLayout).
	// Depth is in eDepthStencilAttachmentOptimal.
	// Subsequent deferred_lighting_end/finish operate on the non-MSAA G-buffer unchanged.

	// Transition scene color from eShaderReadOnlyOptimal → eColorAttachmentOptimal
	// (deferred_lighting_end resumes the non-MSAA gbuf pass and needs scene color writable)
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = pp->getSceneColorImage();
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, nullptr, nullptr, barrier);
	}

	// Composite is not part of the resolve framebuffer, so its layout is
	// indeterminate (UNDEFINED on first frame, eTransferSrcOptimal from
	// previous frame's composite→scene copy, etc.). Use oldLayout=eUndefined
	// to transition it regardless of current state — content will be fully
	// overwritten by emissive→composite copy in deferred_lighting_finish().
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = {};
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = pp->getGbufCompositeImage();
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, nullptr, nullptr, barrier);
	}

	// Transition G-buffer attachments 1-5 for resume
	// (all now in eShaderReadOnlyOptimal: 1-4 from resolve finalLayout, 5 from above)
	pp->transitionGbufForResume(cmd);

	// Resume the non-MSAA G-buffer render pass with eLoad
	{
		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getGbufRenderPassLoad();
		rpBegin.framebuffer = pp->getGbufFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;
		std::array<vk::ClearValue, 7> clearValues{};
		clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getGbufRenderPassLoad(), 0);
		stateTracker->setColorAttachmentCount(VulkanPostProcessor::GBUF_COLOR_ATTACHMENT_COUNT);
	}
}

void vulkan_deferred_lighting_end()
{
	if (!Deferred_lighting) {
		return;
	}

	Deferred_lighting = false;

	// After this, rendering goes back to writing only attachment 0.
	// The pipeline still has 6 blend states (matching the G-buffer render pass)
	// but the shader only outputs to location 0. Attachments 1-5 are untouched.
}

void vulkan_deferred_lighting_finish()
{
	if (!light_deferred_enabled()) {
		return;
	}

	auto* pp = getPostProcessor();
	if (!pp || !pp->isGbufInitialized()) {
		return;
	}

	auto* renderer = getRendererInstance();
	if (!renderer->isSceneRendering()) {
		return;
	}

	auto* stateTracker = getStateTracker();
	vk::CommandBuffer cmd = stateTracker->getCommandBuffer();

	// 1. End G-buffer render pass
	// All 6 color attachments → eShaderReadOnlyOptimal
	// Depth → eDepthStencilAttachmentOptimal
	cmd.endRenderPass();

	// 2. Copy emissive → composite (the emissive data becomes the base for light accumulation)
	// Emissive → eShaderReadOnlyOptimal (done), composite → eColorAttachmentOptimal (for light accum)
	copyImageToImage(cmd,
		pp->getGbufEmissiveImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		pp->getGbufCompositeImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
		pp->getSceneExtent());

	// 3. Render deferred lights (begins + ends light accum render pass internally)
	// After this, composite is in eShaderReadOnlyOptimal
	pp->renderDeferredLights(cmd);

	// 4. Fog rendering (between light accumulation and forward rendering)
	// Matches OpenGL flow in opengl_deferred_lighting_finish()
	bool bDrawFullNeb = The_mission.flags[Mission::Mission_Flags::Fullneb]
		&& Neb2_render_mode != NEB2_RENDER_NONE && !s_vulkanOverrideFog;
	bool bDrawNebVolumetrics = The_mission.volumetrics
		&& The_mission.volumetrics->get_enabled() && !s_vulkanOverrideFog;

	bool fogRendered = false;
	if (bDrawFullNeb) {
		// Scene fog reads composite + depth → writes scene color
		pp->renderSceneFog(cmd);
		fogRendered = true;

		if (bDrawNebVolumetrics) {
			// Copy scene color → composite so volumetric reads the fogged result
			copyImageToImage(cmd,
				pp->getSceneColorImage(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal,
				pp->getGbufCompositeImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
				pp->getSceneExtent());
		}
	}
	if (bDrawNebVolumetrics) {
		// Volumetric fog reads composite + emissive + depth + 3D volumes → writes scene color
		pp->renderVolumetricFog(cmd);
		fogRendered = true;
	}

	if (!fogRendered) {
		// No fog — copy composite → scene color (existing behavior)
		// Skip src post-barrier (composite not used again in this path)
		copyImageToImage(cmd,
			pp->getGbufCompositeImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal,
			pp->getSceneColorImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
			pp->getSceneExtent());
	}

	// 5. Switch to scene render pass for forward transparent objects
	// After light accumulation, use the 2-attachment scene render pass instead
	// of the 6-attachment G-buffer pass. Forward-rendered transparent objects
	// only write to fragOut0 — using the G-buffer pass would leave undefined
	// values at attachment locations 1-5.
	renderer->setUseGbufRenderPass(false);
	stateTracker->setColorAttachmentCount(1);

	// Resume scene render pass (loadOp=eLoad) with depth preserved
	{
		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getSceneRenderPassLoad();
		rpBegin.framebuffer = pp->getSceneFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getSceneRenderPassLoad(), 0);
	}
}

void vulkan_override_fog(bool set_override) {
	s_vulkanOverrideFog = set_override;
}

// ========== Shadow Map Rendering ==========

} // namespace vulkan
} // namespace graphics

extern bool Glowpoint_override;
extern bool gr_htl_projection_matrix_set;

namespace graphics {
namespace vulkan {

namespace {
static bool Glowpoint_override_save = false;
} // anonymous namespace

void vulkan_shadow_map_start(matrix4* shadow_view_matrix, const matrix* light_matrix, vec3d* eye_pos)
{
	if (Shadow_quality == ShadowQuality::Disabled || !getRendererInstance()->supportsShaderViewportLayerOutput()) {
		return;
	}

	// Shadows require the G-buffer render pass (deferred lighting).
	// In contexts without deferred lighting (e.g. tech room), the active
	// render pass is the swap chain or 2-attachment scene pass — ending it
	// and resuming the G-buffer pass would break rendering.
	if (!getRendererInstance()->isUsingGbufRenderPass()) {
		return;
	}

	auto* pp = getPostProcessor();
	if (!pp) {
		return;
	}

	// Lazy-init shadow resources
	if (!pp->isShadowInitialized()) {
		if (!pp->initShadowPass()) {
			return;
		}
	}

	auto* stateTracker = getStateTracker();
	vk::CommandBuffer cmd = stateTracker->getCommandBuffer();

	// End the current G-buffer render pass
	cmd.endRenderPass();

	// Shadow render pass is always non-MSAA (1x sample count)
	stateTracker->setCurrentSampleCount(vk::SampleCountFlagBits::e1);

	// Begin shadow render pass (eClear for both color and depth)
	{
		int shadowSize = pp->getShadowTextureSize();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getShadowRenderPass();
		rpBegin.framebuffer = pp->getShadowFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = vk::Extent2D(static_cast<uint32_t>(shadowSize), static_cast<uint32_t>(shadowSize));

		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getShadowRenderPass(), 0);
		stateTracker->setColorAttachmentCount(1);
	}

	// Set viewport and scissor to shadow texture size
	{
		int shadowSize = pp->getShadowTextureSize();
		vk::Viewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(shadowSize);
		viewport.height = static_cast<float>(shadowSize);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, viewport);

		vk::Rect2D scissor;
		scissor.offset = vk::Offset2D(0, 0);
		scissor.extent = vk::Extent2D(static_cast<uint32_t>(shadowSize), static_cast<uint32_t>(shadowSize));
		cmd.setScissor(0, scissor);
	}

	Rendering_to_shadow_map = true;
	Glowpoint_override_save = Glowpoint_override;
	Glowpoint_override = true;

	gr_htl_projection_matrix_set = true;

	gr_set_view_matrix(eye_pos, light_matrix);

	*shadow_view_matrix = gr_view_matrix;
}

void vulkan_shadow_map_end()
{
	if (!Rendering_to_shadow_map) {
		return;
	}

	auto* pp = getPostProcessor();
	auto* stateTracker = getStateTracker();
	vk::CommandBuffer cmd = stateTracker->getCommandBuffer();

	gr_end_view_matrix();
	Rendering_to_shadow_map = false;

	gr_zbuffer_set(ZBUFFER_TYPE_FULL);

	Glowpoint_override = Glowpoint_override_save;
	gr_htl_projection_matrix_set = false;

	// End shadow render pass (color transitions to eShaderReadOnlyOptimal via finalLayout)
	cmd.endRenderPass();

	const bool msaaActive = (Cmdline_msaa_enabled > 0 && pp->isMsaaInitialized());

	if (msaaActive) {
		// Resume MSAA G-buffer render pass
		pp->transitionMsaaGbufForResume(cmd);

		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getMsaaGbufRenderPassLoad();
		rpBegin.framebuffer = pp->getMsaaGbufFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;
		std::array<vk::ClearValue, 6> clearValues{};
		clearValues[5].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getMsaaGbufRenderPassLoad(), 0);
		stateTracker->setColorAttachmentCount(VulkanPostProcessor::MSAA_COLOR_ATTACHMENT_COUNT);
		stateTracker->setCurrentSampleCount(getRendererInstance()->getMsaaSampleCount());
	} else {
		// Transition scene color: eShaderReadOnlyOptimal → eColorAttachmentOptimal
		// (Scene color was in eShaderReadOnlyOptimal from ending G-buffer pass before shadow start)
		{
			vk::ImageMemoryBarrier barrier;
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = pp->getSceneColorImage();
			barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

			cmd.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				{}, nullptr, nullptr, barrier);
		}

		// Transition G-buffer attachments 1-5 for resume
		pp->transitionGbufForResume(cmd);

		// Resume G-buffer render pass with eLoad
		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getGbufRenderPassLoad();
		rpBegin.framebuffer = pp->getGbufFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;

		std::array<vk::ClearValue, 7> clearValues{};
		clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getGbufRenderPassLoad(), 0);
		stateTracker->setColorAttachmentCount(VulkanPostProcessor::GBUF_COLOR_ATTACHMENT_COUNT);
	}

	// Restore viewport and scissor to scene size
	{
		vk::Viewport viewport;
		viewport.x = static_cast<float>(gr_screen.offset_x);
		viewport.y = static_cast<float>(gr_screen.offset_y);
		viewport.width = static_cast<float>(gr_screen.clip_width);
		viewport.height = static_cast<float>(gr_screen.clip_height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, viewport);

		vk::Rect2D scissor;
		scissor.offset = vk::Offset2D(gr_screen.offset_x, gr_screen.offset_y);
		scissor.extent = vk::Extent2D(static_cast<uint32_t>(gr_screen.clip_width), static_cast<uint32_t>(gr_screen.clip_height));
		cmd.setScissor(0, scissor);
	}
}

// ========== Decal Pass ==========

void vulkan_start_decal_pass()
{
	auto* renderer = getRendererInstance();
	auto* pp = getPostProcessor();
	auto* stateTracker = getStateTracker();

	if (!renderer->isSceneRendering() || !pp || !pp->isGbufInitialized()) {
		return;
	}

	vk::CommandBuffer cmd = stateTracker->getCommandBuffer();

	// End the G-buffer render pass (transitions all color attachments to eShaderReadOnlyOptimal)
	cmd.endRenderPass();

	// Copy scene depth → samplable depth copy (for fragment depth reconstruction)
	pp->copySceneDepth(cmd);

	// Copy G-buffer normal → samplable normal copy (for angle rejection)
	pp->copyGbufNormal(cmd);

	// Transition scene color: eShaderReadOnlyOptimal → eColorAttachmentOptimal
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = pp->getSceneColorImage();
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, nullptr, nullptr, barrier);
	}

	// Transition G-buffer attachments 1-5 for render pass resume
	pp->transitionGbufForResume(cmd);

	// Resume G-buffer render pass with eLoad
	{
		auto extent = pp->getSceneExtent();
		vk::RenderPassBeginInfo rpBegin;
		rpBegin.renderPass = pp->getGbufRenderPassLoad();
		rpBegin.framebuffer = pp->getGbufFramebuffer();
		rpBegin.renderArea.offset = vk::Offset2D(0, 0);
		rpBegin.renderArea.extent = extent;

		std::array<vk::ClearValue, 7> clearValues{};
		clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		cmd.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		stateTracker->setRenderPass(pp->getGbufRenderPassLoad(), 0);
		stateTracker->setColorAttachmentCount(VulkanPostProcessor::GBUF_COLOR_ATTACHMENT_COUNT);
	}

	// Restore viewport (Y-flipped for Vulkan scene rendering)
	auto extent = pp->getSceneExtent();
	stateTracker->setViewport(0.0f,
		static_cast<float>(extent.height),
		static_cast<float>(extent.width),
		-static_cast<float>(extent.height));
}

void vulkan_stop_decal_pass()
{
	// No-op — decals draw within the resumed G-buffer render pass
}

void vulkan_render_decals(decal_material* material_info,
                          primitive_type prim_type,
                          vertex_layout* layout,
                          int num_elements,
                          const indexed_vertex_source& buffers,
                          const gr_buffer_handle& instance_buffer,
                          int num_instances)
{
	if (!material_info || !layout || num_instances <= 0) {
		return;
	}

	auto* stateTracker = getStateTracker();
	auto* pipelineManager = getPipelineManager();
	auto* descManager = getDescriptorManager();
	auto* bufferManager = getBufferManager();
	auto* drawManager = getDrawManager();
	auto* texManager = getTextureManager();
	auto* pp = getPostProcessor();

	// Set up matrices
	gr_matrix_set_uniforms();

	// Build pipeline config for decal rendering
	PipelineConfig config;
	config.shaderType = SDR_TYPE_DECAL;
	config.primitiveType = prim_type;
	config.depthMode = material_info->get_depth_mode();
	config.depthWriteEnabled = false;
	config.cullEnabled = false;
	config.frontFaceCW = false;
	config.blendMode = material_info->get_blend_mode();
	config.renderPass = stateTracker->getCurrentRenderPass();
	config.colorAttachmentCount = stateTracker->getColorAttachmentCount();

	// Per-attachment blend: active attachments (0=color, 2=normal, 4=emissive) get
	// the material's blend mode with RGB-only write mask. Inactive attachments get
	// write mask = 0 to avoid corrupting G-buffer data.
	config.perAttachmentBlendEnabled = true;
	for (uint32_t i = 0; i < config.colorAttachmentCount; ++i) {
		config.attachmentBlends[i].blendMode = ALPHA_BLEND_NONE;
		config.attachmentBlends[i].writeMask = {false, false, false, false};
	}
	// Attachment 0: color/diffuse — use material blend mode 0
	config.attachmentBlends[0].blendMode = material_info->get_blend_mode(0);
	config.attachmentBlends[0].writeMask = {true, true, true, false};
	// Attachment 2: normal — always additive
	config.attachmentBlends[2].blendMode = ALPHA_BLEND_ADDITIVE;
	config.attachmentBlends[2].writeMask = {true, true, true, false};
	// Attachment 4: emissive — use material blend mode 2
	config.attachmentBlends[4].blendMode = material_info->get_blend_mode(2);
	config.attachmentBlends[4].writeMask = {true, true, true, false};

	// Get or create pipeline
	vk::Pipeline pipeline = pipelineManager->getPipeline(config, *layout);
	if (!pipeline) {
		mprintf(("vulkan_render_decals: Failed to get pipeline!\n"));
		return;
	}

	stateTracker->bindPipeline(pipeline, pipelineManager->getPipelineLayout());

	// Get fallback resources
	vk::Buffer fallbackUBO = bufferManager->getFallbackUniformBuffer();
	vk::DeviceSize fallbackUBOSize = static_cast<vk::DeviceSize>(bufferManager->getFallbackUniformBufferSize());
	vk::Sampler fallbackSampler = texManager->getDefaultSampler();
	vk::ImageView fallbackView = texManager->getFallback2DArrayView();
	vk::ImageView fallbackView2D = texManager->getFallbackTextureView2D();

	// Helper: write real pending UBO or fallback
	auto writeUBOOrFallback = [&](DescriptorWriter& w, vk::DescriptorSet set,
	                              uint32_t binding, size_t blockIdx) {
		const auto& pending = drawManager->getPendingUniformBinding(blockIdx);
		if (pending.valid) {
			vk::Buffer buf = bufferManager->getVkBuffer(pending.bufferHandle);
			if (buf) {
				w.writeUniformBuffer(set, binding, buf, pending.offset, pending.size);
				return;
			}
		}
		w.writeUniformBuffer(set, binding, fallbackUBO, 0, fallbackUBOSize);
	};

	DescriptorWriter writer;
	writer.reset(descManager->getDevice());

	// Set 0: Global
	vk::DescriptorSet globalSet = descManager->allocateFrameSet(DescriptorSetIndex::Global);
	Verify(globalSet);
	writer.writeUniformBuffer(globalSet, 0, fallbackUBO, 0, fallbackUBOSize);
	writer.writeUniformBuffer(globalSet, 1, fallbackUBO, 0, fallbackUBOSize);
	writer.writeTexture(globalSet, 2, fallbackView, fallbackSampler);
	vk::ImageView fallbackCubeView = texManager->getFallbackCubeView();
	writer.writeTexture(globalSet, 3, fallbackCubeView, fallbackSampler);
	writer.writeTexture(globalSet, 4, fallbackCubeView, fallbackSampler);
	writer.flush();
	stateTracker->bindDescriptorSet(DescriptorSetIndex::Global, globalSet);

	// Set 1: Material
	vk::DescriptorSet materialSet = descManager->allocateFrameSet(DescriptorSetIndex::Material);
	Verify(materialSet);
	writer.writeUniformBuffer(materialSet, 0, fallbackUBO, 0, fallbackUBOSize);
	writer.writeStorageBuffer(materialSet, 3, fallbackUBO, 0, fallbackUBOSize);

	// Binding 1: decal textures (diffuse, glow, normal as texture array)
	drawManager->bindMaterialTextures(material_info, materialSet, &writer);

	// Binding 2: DecalGlobals UBO
	writeUBOOrFallback(writer, materialSet, 2,
		static_cast<size_t>(uniform_block_type::DecalGlobals));

	// Binding 4: scene depth copy (for fragment depth reconstruction)
	{
		vk::Sampler nearestSampler = texManager->getSampler(
			vk::Filter::eNearest, vk::Filter::eNearest,
			vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false);
		vk::ImageView depthView = pp->getSceneDepthCopyView();
		if (depthView && nearestSampler) {
			writer.writeTexture(materialSet, 4, depthView, nearestSampler);
		} else {
			writer.writeTexture(materialSet, 4, fallbackView2D, fallbackSampler);
		}
	}

	// Binding 5: scene color (fallback — not used by decals)
	writer.writeTexture(materialSet, 5, fallbackView2D, fallbackSampler);

	// Binding 6: G-buffer normal copy (for angle rejection)
	{
		vk::Sampler nearestSampler = texManager->getSampler(
			vk::Filter::eNearest, vk::Filter::eNearest,
			vk::SamplerAddressMode::eClampToEdge, false, 0.0f, false);
		vk::ImageView normalView = pp->getGbufNormalCopyView();
		if (normalView && nearestSampler) {
			writer.writeTexture(materialSet, 6, normalView, nearestSampler);
		} else {
			writer.writeTexture(materialSet, 6, fallbackView2D, fallbackSampler);
		}
	}

	writer.flush();
	stateTracker->bindDescriptorSet(DescriptorSetIndex::Material, materialSet);

	// Set 2: PerDraw
	vk::DescriptorSet perDrawSet = descManager->allocateFrameSet(DescriptorSetIndex::PerDraw);
	Verify(perDrawSet);
	// Pre-initialize all bindings with fallback, then overwrite real ones
	for (uint32_t b = 0; b < 5; ++b) {
		writer.writeUniformBuffer(perDrawSet, b, fallbackUBO, 0, fallbackUBOSize);
	}

	// Binding 1: Matrices UBO (overwrite fallback if valid)
	{
		size_t idx = static_cast<size_t>(uniform_block_type::Matrices);
		const auto& binding = drawManager->getPendingUniformBinding(idx);
		if (binding.valid) {
			vk::Buffer buf = bufferManager->getVkBuffer(binding.bufferHandle);
			if (buf) {
				writer.writeUniformBuffer(perDrawSet, 1, buf, binding.offset, binding.size);
			}
		}
	}

	// Binding 3: DecalInfo UBO (overwrite fallback if valid)
	{
		size_t idx = static_cast<size_t>(uniform_block_type::DecalInfo);
		const auto& binding = drawManager->getPendingUniformBinding(idx);
		if (binding.valid) {
			vk::Buffer buf = bufferManager->getVkBuffer(binding.bufferHandle);
			if (buf) {
				writer.writeUniformBuffer(perDrawSet, 3, buf, binding.offset, binding.size);
			}
		}
	}

	writer.flush();
	stateTracker->bindDescriptorSet(DescriptorSetIndex::PerDraw, perDrawSet);

	// Bind vertex buffers: binding 0 = box VBO, binding 1 = instance buffer
	vk::Buffer boxVBO = bufferManager->getVkBuffer(buffers.Vbuffer_handle);
	vk::Buffer boxIBO = bufferManager->getVkBuffer(buffers.Ibuffer_handle);
	vk::Buffer instBuf = bufferManager->getVkBuffer(instance_buffer);

	if (!boxVBO || !boxIBO || !instBuf) {
		mprintf(("vulkan_render_decals: Missing buffer(s)!\n"));
		return;
	}

	stateTracker->bindVertexBuffer(0, boxVBO, 0);

	// Instance buffer needs frame base offset for streaming buffers
	size_t instFrameOffset = bufferManager->getFrameBaseOffset(instance_buffer);
	stateTracker->bindVertexBuffer(1, instBuf, static_cast<vk::DeviceSize>(instFrameOffset));

	stateTracker->bindIndexBuffer(boxIBO, 0, vk::IndexType::eUint32);

	// Flush dynamic state and draw
	stateTracker->applyDynamicState();

	auto cmdBuffer = stateTracker->getCommandBuffer();
	cmdBuffer.drawIndexed(
		static_cast<uint32_t>(num_elements),  // index count
		static_cast<uint32_t>(num_instances), // instance count
		0,                                     // first index
		0,                                     // vertex offset
		0                                      // first instance
	);
}

} // namespace vulkan
} // namespace graphics
