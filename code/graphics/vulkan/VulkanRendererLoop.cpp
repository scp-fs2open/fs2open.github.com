
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

void VulkanRenderer::beginTrackedRenderPass(const PassBeginDesc& desc)
{
	vk::RenderPassBeginInfo rpBegin;
	rpBegin.renderPass = desc.renderPass;
	rpBegin.framebuffer = desc.framebuffer;
	rpBegin.renderArea.offset = vk::Offset2D(0, 0);
	rpBegin.renderArea.extent = desc.extent;
	rpBegin.clearValueCount = static_cast<uint32_t>(desc.clearValues.size);
	rpBegin.pClearValues = desc.clearValues.data;

	m_currentCommandBuffer.beginRenderPass(rpBegin, vk::SubpassContents::eInline);

	m_stateTracker->setRenderPass(desc.renderPass, 0);
	m_stateTracker->setColorAttachmentCount(desc.colorAttachmentCount);
	m_stateTracker->setCurrentSampleCount(desc.sampleCount);
	m_stateTracker->setRenderArea(desc.extent);

	switch (desc.viewport) {
	case PassViewport::FlipY:
		// Negative viewport height for OpenGL-compatible Y-up NDC (VK_KHR_maintenance1)
		m_stateTracker->setViewport(0.0f,
			static_cast<float>(desc.extent.height),
			static_cast<float>(desc.extent.width),
			-static_cast<float>(desc.extent.height));
		break;
	case PassViewport::NoFlip:
		m_stateTracker->setViewport(0.0f, 0.0f,
			static_cast<float>(desc.extent.width),
			static_cast<float>(desc.extent.height));
		break;
	case PassViewport::Keep:
		break;
	}
}

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

	// Begin the composition render pass (clears color to black, depth to far plane)
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	PassBeginDesc pass;
	pass.renderPass = m_renderPass.get();
	pass.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	pass.extent = m_swapChainExtent;
	pass.clearValues = clearValues;
	beginTrackedRenderPass(pass);

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

#ifdef __APPLE__
	// MoltenVK: the render pass's automatic finalLayout transition +
	// VK_SUBPASS_EXTERNAL subpass dependency (in createEncodeRenderPass()) is
	// spec-legal and sufficient on desktop Vulkan drivers, but MoltenVK's
	// translation of an implicit cross-render-pass layout transition into
	// Metal fences/barriers has been unreliable in practice, showing up as
	// tearing/garbage in the composition image once it's immediately sampled
	// by the output-encode pass below. Insert an explicit, standalone barrier
	// (same access/stage transition the automatic one already promises) so
	// MoltenVK gets an unambiguous synchronization point instead of inferring
	// one from the render pass boundary.
	if (m_currentSwapChainImage < m_compositionImages.size()) {
		vk::ImageMemoryBarrier compositionBarrier;
		compositionBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		compositionBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		compositionBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		compositionBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		compositionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		compositionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		compositionBarrier.image = m_compositionImages[m_currentSwapChainImage].get();
		compositionBarrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		m_currentCommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eFragmentShader,
			{}, nullptr, nullptr, compositionBarrier);
	}
#endif

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
	PassBeginDesc pass;
	pass.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		// Clear values: 6 color + depth
		std::array<vk::ClearValue, VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT + 1> clearValues{};
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_COLOR].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_POSITION].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_NORMAL].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_SPECULAR].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_EMISSIVE].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_ATT_COMPOSITE].color.setFloat32({0.0f, 0.0f, 0.0f, 0.0f});
		clearValues[VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		pass.renderPass = m_postProcessor->deferred().renderPass();
		pass.framebuffer = m_postProcessor->deferred().framebuffer();
		pass.clearValues = clearValues;
		pass.colorAttachmentCount = VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT;
		beginTrackedRenderPass(pass);
	} else {
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		pass.renderPass = m_postProcessor->getSceneRenderPass();
		pass.framebuffer = m_postProcessor->getSceneFramebuffer();
		pass.clearValues = clearValues;
		beginTrackedRenderPass(pass);
	}

	m_sceneRendering = true;
}

void VulkanRenderer::resumeSceneRendering()
{
	PassBeginDesc pass;
	pass.renderPass = m_postProcessor->getSceneRenderPassLoad();
	pass.framebuffer = m_postProcessor->getSceneFramebuffer();
	pass.extent = m_postProcessor->getSceneExtent();
	beginTrackedRenderPass(pass);
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

	// Begin the resumed swap chain render pass (loadOp=eLoad to preserve pre-scene content).
	// Non-flipped viewport for the post-processing blit (HDR texture is already in
	// Vulkan orientation); restored to flipped for HUD rendering below.
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	PassBeginDesc pass;
	pass.renderPass = m_renderPassLoad.get();
	pass.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	pass.extent = m_swapChainExtent;
	pass.clearValues = clearValues;
	pass.viewport = PassViewport::NoFlip;
	beginTrackedRenderPass(pass);

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
	resumeScenePassAfterCopy();
}

// Shared tail of copyEffectTexture()/copySceneDepthForParticles(): resume the
// scene (or G-buffer) render pass with loadOp=eLoad after a mid-scene copy.
void VulkanRenderer::resumeScenePassAfterCopy()
{
	PassBeginDesc pass;
	pass.extent = m_postProcessor->getSceneExtent();

	if (m_useGbufRenderPass) {
		// Clear values ignored for eLoad but array must cover all attachments
		std::array<vk::ClearValue, VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT + 1> clearValues{};
		clearValues[VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		pass.renderPass = m_postProcessor->deferred().renderPassLoad();
		pass.framebuffer = m_postProcessor->deferred().framebuffer();
		pass.clearValues = clearValues;
		pass.colorAttachmentCount = VulkanDeferredGBuffer::GBUF_COLOR_ATTACHMENT_COUNT;
		beginTrackedRenderPass(pass);
	} else {
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		pass.renderPass = m_postProcessor->getSceneRenderPassLoad();
		pass.framebuffer = m_postProcessor->getSceneFramebuffer();
		pass.clearValues = clearValues;
		beginTrackedRenderPass(pass);
	}
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
	resumeScenePassAfterCopy();

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

	PassBeginDesc pass;
	pass.renderPass = ts->renderPass;
	pass.framebuffer = fb;
	pass.extent = vk::Extent2D(ts->width, ts->height);
	pass.clearValues = ArrayView<vk::ClearValue>(&clearValue, 1);
	// The engine sets the RTT viewport itself via gr_set_viewport (positive
	// height; the RTT projection matrix handles the Y-flip), so keep it.
	pass.viewport = PassViewport::Keep;
	beginTrackedRenderPass(pass);

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

	PassBeginDesc pass;
	pass.renderPass = m_renderPassLoad.get();
	pass.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	pass.extent = m_swapChainExtent;
	pass.clearValues = clearValues;
	beginTrackedRenderPass(pass);
}

} // namespace graphics::vulkan
