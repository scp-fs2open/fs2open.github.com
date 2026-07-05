
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"


#include "backends/imgui_impl_vulkan.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "mod_table/mod_table.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)
#endif


extern float flFrametime;

namespace graphics::vulkan {

void VulkanRenderer::acquireNextSwapChainImage()
{
	m_frames[m_currentFrame]->waitForFinish();

	// Recreate swap chain if flagged from a previous frame
	if (m_swapChainNeedsRecreation) {
		// Wait for minimized window (0x0 extent) before recreating
		while (true) {
			if (recreateSwapChain()) {
				break;
			}
			// Window is minimized — wait and pump events until surface is valid again
			os_sleep(100);
			SDL_PumpEvents();
		}
	}

	uint32_t imageIndex = 0;
	auto status = m_frames[m_currentFrame]->acquireSwapchainImage(imageIndex);

	if (status == SwapChainStatus::eOutOfDate) {
		// Must recreate immediately and retry
		while (true) {
			if (recreateSwapChain()) {
				break;
			}
			os_sleep(100);
			SDL_PumpEvents();
		}
		status = m_frames[m_currentFrame]->acquireSwapchainImage(imageIndex);
		if (status == SwapChainStatus::eOutOfDate) {
			// If still failing after recreation, flag for next frame
			m_swapChainNeedsRecreation = true;
		}
	}

	if (status == SwapChainStatus::eSuboptimal) {
		m_swapChainNeedsRecreation = true;
	}

	m_currentSwapChainImage = imageIndex;

	// Ensure that this image is no longer in use
	if (m_swapChainImageRenderImage[m_currentSwapChainImage]) {
		m_swapChainImageRenderImage[m_currentSwapChainImage]->waitForFinish();
	}
	// Reserve the image as in use
	m_swapChainImageRenderImage[m_currentSwapChainImage] = m_frames[m_currentFrame].get();
}
void VulkanRenderer::setupFrame()
{
	if (m_frameInProgress) {
		Warning(LOCATION, "VulkanRenderer::setupFrame called while frame already in progress!");
		return;
	}

	// Free completed texture upload command buffers
	Assertion(m_textureManager, "Vulkan TextureManager not initialized in setupFrame!");
	m_textureManager->frameStart();

	// Allocate command buffer for this frame
	vk::CommandBufferAllocateInfo cmdBufferAlloc;
	cmdBufferAlloc.commandPool = m_graphicsCommandPool.get();
	cmdBufferAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdBufferAlloc.commandBufferCount = 1;

	auto cmdBufs = m_device->allocateCommandBuffers(cmdBufferAlloc);
	m_currentCommandBuffers.assign(cmdBufs.begin(), cmdBufs.end());
	m_currentCommandBuffer = m_currentCommandBuffers.front();

	// Begin command buffer
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	m_currentCommandBuffer.begin(beginInfo);

	Assertion(m_descriptorManager, "Vulkan DescriptorManager not initialized in setupFrame!");
	m_descriptorManager->beginFrame();

	Assertion(m_stateTracker, "Vulkan StateTracker not initialized in setupFrame!");
	m_stateTracker->beginFrame(m_currentCommandBuffer);

	// Reset timestamp queries that were written last frame (must be outside render pass)
	if (m_queryManager) {
		m_queryManager->beginFrame(m_currentCommandBuffer);
	}

	// Reset per-frame flags
	m_sceneDepthCopiedThisFrame = false;

	// Reset per-frame draw statistics
	Assertion(m_drawManager, "Vulkan DrawManager not initialized in setupFrame!");
	m_drawManager->resetFrameStats();

	// Begin render pass
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = m_renderPass.get();
	renderPassBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent = m_swapChainExtent;

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});  // Clear to black each frame
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);  // Clear depth to far plane

	renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBegin.pClearValues = clearValues.data();

	m_currentCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	// Set up state tracker for FSO draws
	m_stateTracker->setRenderPass(m_renderPass.get(), 0);
	// Negative viewport height for OpenGL-compatible Y-up NDC (VK_KHR_maintenance1)
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(m_swapChainExtent.height),
		static_cast<float>(m_swapChainExtent.width),
		-static_cast<float>(m_swapChainExtent.height));

	m_frameInProgress = true;
}

void VulkanRenderer::flip()
{
	if (!m_frameInProgress) {
		nprintf(("Vulkan", "VulkanRenderer::flip called without frame in progress, skipping\n"));
		return;
	}

	// Print per-frame diagnostic summary before ending
	Assertion(m_drawManager, "Vulkan DrawManager not initialized in flip!");
	m_drawManager->printFrameStats();

	// End the composition render pass (composition image is now in
	// eShaderReadOnlyOptimal) and run the final output-encode pass that writes
	// the actual swap chain image (SDR passthrough or HDR10 PQ/BT.2020).
	m_currentCommandBuffer.endRenderPass();
	encodeToSwapChain();
	m_stateTracker->endFrame();
	m_descriptorManager->endFrame();

	// End command buffer
	m_currentCommandBuffer.end();

	// Set up cleanup callback for command buffers
	auto buffersToFree = m_currentCommandBuffers;
	m_frames[m_currentFrame]->onFrameFinished([this, buffersToFree]() mutable {
		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), buffersToFree);
	});

	// Submit and present
	auto presentStatus = m_frames[m_currentFrame]->submitAndPresent(m_currentCommandBuffers);

	if (presentStatus == SwapChainStatus::eSuboptimal || presentStatus == SwapChainStatus::eOutOfDate) {
		m_swapChainNeedsRecreation = true;
	}

	// Notify query manager that this frame's command buffer was submitted
	if (m_queryManager) {
		m_queryManager->notifySubmission();
	}

	// Track which swap chain image was just presented so saveScreen() can read it
	m_previousSwapChainImage = m_currentSwapChainImage;

	// Clear current command buffer reference
	m_currentCommandBuffer = nullptr;
	m_currentCommandBuffers.clear();
	m_frameInProgress = false;

	// Advance counters to prepare for the next frame
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	++m_frameNumber;

	// Set the frame index for the buffer manager immediately after incrementing
	// This ensures any buffer operations that happen before setupFrame() use the correct frame
	m_bufferManager->setCurrentFrame(m_currentFrame);

	acquireNextSwapChainImage();

	// Process deferred resource deletions AFTER the fence wait in
	// acquireNextSwapChainImage, so we know the previous frame's commands
	// (including async upload CBs) have completed before destroying resources.
	m_deletionQueue->processDestructions();
}

void VulkanRenderer::beginSceneRendering()
{
	if (!m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}
	if (m_sceneRendering) {
		return;
	}

	// End the current swap chain render pass
	m_currentCommandBuffer.endRenderPass();

	// Use G-buffer render pass when deferred lighting is enabled and G-buffer is ready
	m_useGbufRenderPass = m_postProcessor->deferred().isInitialized() && light_deferred_enabled();

	// Begin the HDR scene render pass (or G-buffer render pass for deferred)
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		rpBegin.renderPass = m_postProcessor->deferred().renderPass();
		rpBegin.framebuffer = m_postProcessor->deferred().framebuffer();

		// Clear values: 6 color + depth
		std::array<vk::ClearValue, VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT + 1> clearValues{};
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_COLOR].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_POSITION].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_NORMAL].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_SPECULAR].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_EMISSIVE].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_COMPOSITE].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->deferred().renderPass(), 0);
		m_stateTracker->setColorAttachmentCount(VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT);
	} else {
		rpBegin.renderPass = m_postProcessor->getSceneRenderPass();
		rpBegin.framebuffer = m_postProcessor->getSceneFramebuffer();

		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();

		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getSceneRenderPass(), 0);
		m_stateTracker->setColorAttachmentCount(1);
	}

	// Negative viewport height for Y-flip (same as swap chain pass)
	auto extent = m_postProcessor->getSceneExtent();
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(extent.height),
		static_cast<float>(extent.width),
		-static_cast<float>(extent.height));

	m_sceneRendering = true;
}

void VulkanRenderer::resumeSceneRendering()
{
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_postProcessor->getSceneExtent();

 	rpBegin.renderPass = m_postProcessor->getSceneRenderPassLoad();
 	rpBegin.framebuffer = m_postProcessor->getSceneFramebuffer();

	m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
 	m_stateTracker->setRenderPass(m_postProcessor->getSceneRenderPassLoad(), 0);
 	m_stateTracker->setColorAttachmentCount(1);
}

void VulkanRenderer::endSceneRendering()
{
	if (!m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}
	if (!m_sceneRendering) {
		return;
	}

	// End HDR scene render pass (transitions scene color to eShaderReadOnlyOptimal)
	m_currentCommandBuffer.endRenderPass();

	// Update distortion ping-pong textures (every ~30ms, matching OpenGL)
	if (Gr_framebuffer_effects.any_set()) {
		m_postProcessor->updateDistortion(m_currentCommandBuffer, flFrametime);
	}

	// Execute post-processing passes (all between HDR scene pass and swap chain pass)
	m_postProcessor->executeBloom(m_currentCommandBuffer);
	m_postProcessor->executeTonemap(m_currentCommandBuffer);
	m_postProcessor->executeFXAA(m_currentCommandBuffer);
	m_postProcessor->executeSMAA(m_currentCommandBuffer);
	m_postProcessor->executeLightshafts(m_currentCommandBuffer);
	m_postProcessor->executePostEffects(m_currentCommandBuffer);

	// Begin the resumed swap chain render pass (loadOp=eLoad to preserve pre-scene content)
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = m_renderPassLoad.get();
	rpBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_swapChainExtent;

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
	rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	rpBegin.pClearValues = clearValues.data();

	m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);

	// Update state tracker for the resumed swap chain pass
	m_stateTracker->setRenderPass(m_renderPassLoad.get(), 0);
	m_stateTracker->setColorAttachmentCount(1);
	// Non-flipped viewport for post-processing blit (HDR texture is already correct orientation)
	m_stateTracker->setViewport(0.0f, 0.0f,
		static_cast<float>(m_swapChainExtent.width),
		static_cast<float>(m_swapChainExtent.height));

	// Blit the HDR scene to swap chain through post-processing
	m_postProcessor->blitToSwapChain(m_currentCommandBuffer);

	// Restore Y-flipped viewport for HUD rendering
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(m_swapChainExtent.height),
		static_cast<float>(m_swapChainExtent.width),
		-static_cast<float>(m_swapChainExtent.height));

	m_sceneRendering = false;
	m_useGbufRenderPass = false;
}

void VulkanRenderer::copyEffectTexture()
{
	if (!m_sceneRendering || !m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}

	// End the current scene render pass
	// This transitions scene color to eShaderReadOnlyOptimal (the render pass's finalLayout)
	// For G-buffer: all 6 color attachments transition to eShaderReadOnlyOptimal
	m_currentCommandBuffer.endRenderPass();

	// Copy scene color → effect texture (handles scene color transitions)
	m_postProcessor->copyEffectTexture(m_currentCommandBuffer);

	// If G-buffer is active, transition attachments 1-5 for render pass resume
	if (m_useGbufRenderPass) {
		m_postProcessor->deferred().transitionForResume(m_currentCommandBuffer);
	}

	// Resume the scene render pass with loadOp=eLoad to preserve existing content
	// Scene color is now in eColorAttachmentOptimal (copyEffectTexture transitions it back)
	// Depth is still in eDepthStencilAttachmentOptimal (untouched by the copy)
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		rpBegin.renderPass = m_postProcessor->deferred().renderPassLoad();
		rpBegin.framebuffer = m_postProcessor->deferred().framebuffer();
		// Clear values ignored for eLoad but array must cover all attachments
		std::array<vk::ClearValue, VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT + 1> clearValues{};
		clearValues[VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->deferred().renderPassLoad(), 0);
	} else {
		rpBegin.renderPass = m_postProcessor->getSceneRenderPassLoad();
		rpBegin.framebuffer = m_postProcessor->getSceneFramebuffer();
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getSceneRenderPassLoad(), 0);
	}

	// Restore Y-flipped viewport for scene rendering
	auto extent = m_postProcessor->getSceneExtent();
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(extent.height),
		static_cast<float>(extent.width),
		-static_cast<float>(extent.height));
}

void VulkanRenderer::copySceneDepthForParticles()
{
	if (m_sceneDepthCopiedThisFrame || !m_sceneRendering || !m_postProcessor || !m_postProcessor->isInitialized()) {
		return;
	}

	// End the current scene render pass
	// This transitions: color → eShaderReadOnlyOptimal, depth → eDepthStencilAttachmentOptimal
	// For G-buffer: all 6 color attachments → eShaderReadOnlyOptimal
	m_currentCommandBuffer.endRenderPass();

	// Copy scene depth → samplable depth copy (handles all depth image transitions)
	m_postProcessor->copySceneDepth(m_currentCommandBuffer);

	// Transition scene color: eShaderReadOnlyOptimal → eColorAttachmentOptimal
	// (needed for the resumed render pass with loadOp=eLoad, which expects
	// initialLayout=eColorAttachmentOptimal; copySceneDepth only touches depth)
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_postProcessor->getSceneColorImage();
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		m_currentCommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, {}, {}, barrier);
	}

	// If G-buffer is active, transition attachments 1-5 for render pass resume
	if (m_useGbufRenderPass) {
		m_postProcessor->deferred().transitionForResume(m_currentCommandBuffer);
	}

	// Resume the scene render pass with loadOp=eLoad
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		rpBegin.renderPass = m_postProcessor->deferred().renderPassLoad();
		rpBegin.framebuffer = m_postProcessor->deferred().framebuffer();
		std::array<vk::ClearValue, VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT + 1> clearValues{};
		clearValues[VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->deferred().renderPassLoad(), 0);
	} else {
		rpBegin.renderPass = m_postProcessor->getSceneRenderPassLoad();
		rpBegin.framebuffer = m_postProcessor->getSceneFramebuffer();
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		rpBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_postProcessor->getSceneRenderPassLoad(), 0);
	}

	// Restore Y-flipped viewport for scene rendering
	auto extent = m_postProcessor->getSceneExtent();
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(extent.height),
		static_cast<float>(extent.width),
		-static_cast<float>(extent.height));

	m_sceneDepthCopiedThisFrame = true;
}

void VulkanRenderer::beginRenderTarget(tcache_slot_vulkan* ts, int face)
{
	// End current render pass (swap chain or previous RT face)
	m_currentCommandBuffer.endRenderPass();

	// Select the correct framebuffer for cubemap face or flat RT
	vk::Framebuffer fb = (ts->isCubemap && face >= 0 && face < 6)
	                    ? ts->cubeFaceFramebuffers[face] : ts->framebuffer;

	vk::ClearValue clearValue;
	clearValue.color = m_stateTracker->getClearColor();

	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = ts->renderPass;
	rpBegin.framebuffer = fb;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = vk::Extent2D(ts->width, ts->height);
	rpBegin.clearValueCount = 1;
	rpBegin.pClearValues = &clearValue;

	m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);

	m_stateTracker->setRenderPass(ts->renderPass, 0);
	m_stateTracker->setColorAttachmentCount(1);
	m_stateTracker->setCurrentSampleCount(vk::SampleCountFlagBits::e1);
	m_renderTargetActive = true;
}

void VulkanRenderer::endRenderTarget(tcache_slot_vulkan* ts)
{
	if (!m_renderTargetActive) {
		return;
	}

	// End the RT render pass (finalLayout transitions mip 0 to eShaderReadOnlyOptimal)
	m_currentCommandBuffer.endRenderPass();
	m_renderTargetActive = false;

	// Render passes only ever draw into mip 0 (each face's framebuffer is a
	// single-mip view), so higher mips are otherwise left uninitialized.
	// Regenerate the chain now via blits so effects that sample it with an
	// explicit LOD (e.g. the env map's roughness-based reflection blur) see
	// real data instead of garbage/undefined content.
	if (ts && ts->mipLevels > 1) {
		uint32_t layers = ts->isCubemap ? 6u : ts->arrayLayers;

		vk::ImageMemoryBarrier barrier;
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = ts->image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layers;

		m_currentCommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eTransfer,
			{}, {}, {}, barrier);

		vulkan_generate_mipmap_chain(m_currentCommandBuffer, ts->image, ts->width, ts->height, ts->mipLevels, layers);
	}

	resumeSwapChainPass();
}

void VulkanRenderer::resumeSwapChainPass()
{
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = m_renderPassLoad.get();
	rpBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = m_swapChainExtent;
	rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	rpBegin.pClearValues = clearValues.data();

	m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);

	m_stateTracker->setRenderPass(m_renderPassLoad.get(), 0);
	m_stateTracker->setColorAttachmentCount(1);
	m_stateTracker->setCurrentSampleCount(vk::SampleCountFlagBits::e1);
}

} // namespace graphics::vulkan
