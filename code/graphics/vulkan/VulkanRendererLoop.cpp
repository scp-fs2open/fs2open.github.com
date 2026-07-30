
#include "VulkanRenderer.h"
#include "VulkanBarrier.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"


#include "backends/imgui_impl_vulkan.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"

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

void VulkanRenderer::waitForFrameSlot(uint32_t slot)
{
	// Every target has its own sync objects, but the command pool and the descriptor manager's
	// pools are shared and keyed on this slot alone. So recycling the slot is only safe once the
	// work *every* target put in it has completed -- waiting on the target that happens to be
	// current is not enough. Getting this wrong let a viewport switch reset a descriptor pool and
	// free command buffers that the other target's still-pending submit was using
	// (VUID-vkResetDescriptorPool-descriptorPool-00313,
	// VUID-vkFreeCommandBuffers-pCommandBuffers-00047), which wedged the queue.
	for (auto& entry : m_targets) {
		auto& frame = entry.second->frames[slot];
		if (!frame) {
			continue;
		}

		SCP_string what;
		sprintf(what, "frame slot %u of the %s target (slot wait)", slot,
			entry.second.get() == m_mainTarget ? "main" : "secondary");
		waitOrReport(*frame, what.c_str());
	}
}

void VulkanRenderer::waitOrReport(VulkanRenderFrame& frame, const char* what)
{
	constexpr uint64_t PROBE_NS = 200000000ULL;    // 0.2s -- far longer than any real frame
	constexpr uint64_t GIVE_UP_NS = 2000000000ULL; // 2s more before calling it wedged

	if (frame.waitForFinish(PROBE_NS)) {
		return;
	}

	mprintf(("Vulkan: still waiting on %s after 0.2s; the queue looks wedged.\n", what));

	if (!frame.waitForFinish(GIVE_UP_NS)) {
		Error(LOCATION, "Vulkan: %s never completed. This is a renderer synchronisation bug, not bad data.",
			what);
	}
}

void VulkanRenderer::acquireNextSwapChainImage()
{
	waitForFrameSlot(m_currentFrame);

	// Acquire an image, recreating the swap chain as often as needed (bounded).
	// The frame must never proceed without an acquired image: its image-available
	// semaphore would never be signaled and the submit would deadlock/corrupt.
	// A viewport switch retains this target's acquired image rather than abandoning it, since only a
	// present hands one back. Reuse it instead of acquiring a second one.
	if (m_current->hasRetainedAcquire && !m_current->needsRecreation) {
		m_current->currentAcquire = m_current->retainedAcquire;
		m_current->currentImage = m_current->retainedImage;
		m_current->hasRetainedAcquire = false;

		if (m_current->imageRenderFrame[m_current->currentImage]) {
			waitOrReport(*m_current->imageRenderFrame[m_current->currentImage],
				"the frame still rendering into a retained image");
		}
		m_current->imageRenderFrame[m_current->currentImage] = m_current->frames[m_currentFrame].get();
		return;
	}

	constexpr int MAX_ACQUIRE_ATTEMPTS = 5;
	uint32_t imageIndex = 0;
	SwapChainStatus status = SwapChainStatus::eOutOfDate;

	for (int attempt = 0; attempt < MAX_ACQUIRE_ATTEMPTS; ++attempt) {
		// Recreate if flagged (from a previous frame, a failed acquire below, or
		// a suboptimal present). Waits out a minimized window (0x0 extent).
		if (m_current->needsRecreation) {
			while (!recreateSwapChain(*m_current)) {
				os_sleep(100);
				SDL_PumpEvents();
			}
		}

		// Take the next acquire semaphore round-robin, making sure whatever last presented with it
		// has finished before it is signalled again.
		auto& acquire = m_current->acquireSemaphores[m_current->nextAcquire];
		if (acquire.consumer != nullptr) {
			waitOrReport(*acquire.consumer, "the frame that last presented with this acquire semaphore");
			acquire.consumer = nullptr;
		}
		m_current->currentAcquire = m_current->nextAcquire;
		m_current->nextAcquire = (m_current->nextAcquire + 1) %
		                         static_cast<uint32_t>(m_current->acquireSemaphores.size());

		status = m_current->frames[m_currentFrame]->acquireSwapchainImage(imageIndex,
			acquire.semaphore.get());
		if (status != SwapChainStatus::eOutOfDate) {
			break;
		}
		m_current->needsRecreation = true;
	}

	if (status == SwapChainStatus::eOutOfDate) {
		Error(LOCATION, "Vulkan: failed to acquire a swap chain image after %d recreation attempts!",
			MAX_ACQUIRE_ATTEMPTS);
	}

	if (status == SwapChainStatus::eSuboptimal) {
		m_current->needsRecreation = true;
	}

	m_current->currentImage = imageIndex;

	// Ensure that this image is no longer in use
	if (m_current->imageRenderFrame[m_current->currentImage]) {
		waitOrReport(*m_current->imageRenderFrame[m_current->currentImage],
			"the frame still rendering into the newly acquired image");
	}
	// Reserve the image as in use
	m_current->imageRenderFrame[m_current->currentImage] = m_current->frames[m_currentFrame].get();
}
void VulkanRenderer::setupFrame()
{
	if (m_frameInProgress) {
		Warning(LOCATION, "VulkanRenderer::setupFrame called while frame already in progress!");
		return;
	}

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

	// Reset the post-processing scratch UBO ring cursor for this frame-in-flight
	// slot (safe: acquireNextSwapChainImage already waited on this slot's fence)
	if (m_postProcessor) {
		m_postProcessor->beginFrame(m_currentFrame);
	}

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
	pass.framebuffer = m_current->framebuffers[m_current->currentImage].get();
	pass.extent = m_current->extent;
	pass.clearValues = clearValues;
	beginTrackedRenderPass(pass);

	m_frameInProgress = true;
}

void VulkanRenderer::discardFrame()
{
	if (!m_frameInProgress) {
		return;
	}

	// Whatever pass is open (composition, or a post-processing/shadow pass if this is ever called
	// from somewhere less tidy) has to be closed before the command buffer can be ended.
	if (m_stateTracker->getCurrentRenderPass()) {
		m_currentCommandBuffer.endRenderPass();
		m_stateTracker->setRenderPass(vk::RenderPass());
	}

	m_currentCommandBuffer.end();
	m_device->freeCommandBuffers(m_graphicsCommandPool.get(), m_currentCommandBuffers);

	m_currentCommandBuffer = nullptr;
	m_currentCommandBuffers.clear();
	m_frameInProgress = false;

	// This frame never submitted, so it does not own the image any more.
	if (m_current->currentImage < m_current->imageRenderFrame.size()) {
		m_current->imageRenderFrame[m_current->currentImage] = nullptr;
	}

	// Keep the acquire rather than dropping it. vkAcquireNextImageKHR hands out an image that only
	// vkQueuePresentKHR gives back, so abandoning one here would leak an image on this target every
	// time a viewport switch passed through -- and with a handful of images per swap chain, that
	// wedges the next acquire within a few frames. The semaphore it signalled is owned by the
	// target, so it survives the frame slot moving on.
	m_current->hasRetainedAcquire = true;
	m_current->retainedAcquire = m_current->currentAcquire;
	m_current->retainedImage = m_current->currentImage;
}

bool VulkanRenderer::useViewport(os::Viewport* viewport)
{
	if (viewport == nullptr) {
		return false;
	}

	if (m_current != nullptr && m_current->viewport == viewport) {
		return true;
	}

	// Resolve the target -- building it on first use -- before disturbing anything. Creating one
	// touches only its own resources, so a failure here leaves the renderer exactly as it was: the
	// frame in progress is still the outgoing target's, and the caller keeps drawing where it was.
	auto entry = m_targets.find(viewport);
	if (entry == m_targets.end()) {
		auto created = std::make_unique<VulkanPresentTarget>();
		created->viewport = viewport;

		if (!createTargetResources(*created)) {
			mprintf(("Vulkan: could not present to this viewport; staying on the previous one.\n"));
			releaseTargetMemory(*created);
			return false;
		}

		entry = m_targets.emplace(viewport, std::move(created)).first;
	}

	// A frame is always open here: gr_flip() ends with gr_setup_frame(), so the outgoing target has
	// both an open command buffer and an already-acquired image. Neither survives the switch.
	const bool restartFrame = m_frameInProgress;
	discardFrame();

	m_current = entry->second.get();

	if (restartFrame) {
		acquireNextSwapChainImage();
		setupFrame();
	}

	return true;
}

void VulkanRenderer::releaseViewport(os::Viewport* viewport)
{
	auto entry = m_targets.find(viewport);
	if (entry == m_targets.end()) {
		return;
	}

	if (entry->second.get() == m_mainTarget) {
		// The main target lives as long as the renderer does; letting it go here would leave
		// nothing to fall back to.
		return;
	}

	// Anything still reading this target's images has to be done before they are destroyed. The
	// frames are per-target, so this waits only on that target's work -- but the surface is about to
	// go with the window, so be blunt about it and drain the device too.
	for (auto& frame : entry->second->frames) {
		if (frame) {
			frame->waitForFinish();
		}
	}
	m_device->waitIdle();

	if (m_current == entry->second.get()) {
		// Whatever was set up against this target cannot be presented now.
		discardFrame();
		m_current = m_mainTarget;
		acquireNextSwapChainImage();
		setupFrame();
	}

	// Order matters and is not left to member destruction: the swap chain has to go before the
	// surface, and the surface goes as soon as the handle releases it back to the window system.
	releaseTargetMemory(*entry->second);
	destroyTargetSwapChain(*entry->second);
	entry->second->surface.reset();

	m_targets.erase(entry);
}

bool VulkanRenderer::syncToSurfaceExtent()
{
	if (!m_current->surface || !m_current->swapChain) {
		return false;
	}

	const auto capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_current->surface.get());

	// 0xFFFFFFFF means "the surface takes its size from the swap chain", so there is nothing to
	// follow. A 0x0 extent is a minimized window; leave the swap chain alone and let the regular
	// acquire path wait it out rather than recreating into an unusable size here.
	if (capabilities.currentExtent.width == UINT32_MAX ||
		capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0) {
		return false;
	}

	if (capabilities.currentExtent == m_current->extent) {
		return false;
	}

	// Rebuilding means discarding the frame in progress, which is only free while nothing has been
	// drawn into it -- the top of a frame, where every gr_screen_resize() caller sits today.
	//
	// This catches only the narrowest case of getting that wrong: a resize from inside an open
	// scene-texture scope, which would throw away a composed scene. It is deliberately not the
	// equivalent of OpenGL's Scene_framebuffer_in_frame check, which is broader --
	// beginSceneRendering() never runs at all when post-processing is off, and off is qtFRED's
	// default, so this is false for the whole frame in the common case. Widening it would mean
	// tracking "has anything been recorded into this command buffer", which nothing needs yet.
	if (m_sceneRendering) {
		Assertion(false, "Tried to resize the Vulkan swap chain to %ux%u while a scene was being "
			"rendered into it! The resize has been deferred to the next frame.",
			capabilities.currentExtent.width, capabilities.currentExtent.height);
		return false;
	}

	nprintf(("vulkan", "Vulkan: window is %ux%u but the swap chain is %ux%u, rebuilding\n",
		capabilities.currentExtent.width, capabilities.currentExtent.height,
		m_current->extent.width, m_current->extent.height));

	const bool restartFrame = m_frameInProgress;
	discardFrame();

	m_current->needsRecreation = true;
	acquireNextSwapChainImage();

	if (restartFrame) {
		setupFrame();
	}

	return true;
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
	// MoltenVK: the encode render pass's VK_SUBPASS_EXTERNAL dependency (see
	// createEncodeRenderPass(): srcStage=eColorAttachmentOutput|eFragmentShader,
	// srcAccess=eColorAttachmentWrite, dst covers the encode's eShaderRead) is
	// spec-legal and sufficient on desktop Vulkan drivers, and -gr_sync_validation
	// confirms the composition-image ordering is clean there without this
	// barrier. It is kept only because MoltenVK's translation of the implicit
	// cross-render-pass sync into Metal fences/barriers has historically been
	// unreliable (tearing/garbage in the composition image once the encode pass
	// samples it). RE-TEST on macOS hardware; if MoltenVK now honors the subpass
	// dependency, this explicit barrier can be removed.
	if (m_current->currentImage < m_current->compositionImages.size()) {
		ImageBarrier2 compositionBarrier;
		compositionBarrier.image = m_current->compositionImages[m_current->currentImage].get();
		compositionBarrier.levelCount = 1;
		compositionBarrier.layerCount = 1;
		compositionBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		compositionBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		compositionBarrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		compositionBarrier.srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
		compositionBarrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
		compositionBarrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;

		cmdImageBarrier(m_currentCommandBuffer, compositionBarrier);
	}
#endif

	encodeToSwapChain();
	m_stateTracker->endFrame();

	// End command buffer
	m_currentCommandBuffer.end();

	// Set up cleanup callback for command buffers
	auto buffersToFree = m_currentCommandBuffers;
	m_current->frames[m_currentFrame]->onFrameFinished([this, buffersToFree]() mutable {
		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), buffersToFree);
	});

	// Submit and present
	auto& acquire = m_current->acquireSemaphores[m_current->currentAcquire];
	acquire.consumer = m_current->frames[m_currentFrame].get();
	auto presentStatus = m_current->frames[m_currentFrame]->submitAndPresent(m_currentCommandBuffers,
		acquire.semaphore.get(),
		m_current->renderFinishedSemaphores[m_current->currentImage].get(),
		m_current->currentImage,
		m_frameNumber);

	if (presentStatus == SwapChainStatus::eSuboptimal || presentStatus == SwapChainStatus::eOutOfDate) {
		m_current->needsRecreation = true;
	}

	// Notify query manager that this frame's command buffer was submitted
	if (m_queryManager) {
		m_queryManager->notifySubmission();
	}

	// Track which swap chain image was just presented so saveScreen() can read it
	m_current->previousImage = m_current->currentImage;

	// Clear current command buffer reference
	m_currentCommandBuffer = nullptr;
	m_currentCommandBuffers.clear();
	m_frameInProgress = false;

	// Advance the single frame-in-flight index and push it to every consumer that
	// tracks it, so they all agree by construction. Done immediately after
	// incrementing so any buffer/descriptor operations before setupFrame() use the
	// correct frame.
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	++m_frameNumber;

	m_bufferManager->setCurrentFrame(m_currentFrame, m_frameNumber);
	m_descriptorManager->setCurrentFrame(m_currentFrame);

	acquireNextSwapChainImage();

	// Process deferred resource deletions AFTER the fence wait in
	// acquireNextSwapChainImage, so we know the previous frame's commands
	// (including async upload CBs) have completed before destroying resources.
	m_deletionQueue->processDestructions();

	// Retire finished texture upload command buffers. This belongs here rather than in
	// setupFrame(): the texture manager frees them on a countdown of FRAMES_TO_WAIT calls, which
	// only means "the GPU has moved on" if each call corresponds to a frame that actually
	// completed. setupFrame() stopped being that the moment viewports could be switched --
	// useViewport() sets a frame up on the incoming target without ever flipping it, so with
	// qtFRED's briefing map running there are about three setupFrame() calls per completed frame.
	// The countdown then ran out while uploads were still executing and freed command buffers in
	// the pending state (VUID-vkFreeCommandBuffers-pCommandBuffers-00047). Ticking it here, next
	// to the deletion queue and after the fence wait above, ties it back to real frame completion.
	Assertion(m_textureManager, "Vulkan TextureManager not initialized in flip!");
	m_textureManager->frameStart();
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
	pass.framebuffer = m_current->framebuffers[m_current->currentImage].get();
	pass.extent = m_current->extent;
	pass.clearValues = clearValues;
	pass.viewport = PassViewport::NoFlip;
	beginTrackedRenderPass(pass);

	// Blit the HDR scene to swap chain through post-processing
	m_postProcessor->blitToSwapChain(m_currentCommandBuffer);

	// Restore Y-flipped viewport for HUD rendering
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(m_current->extent.height),
		static_cast<float>(m_current->extent.width),
		-static_cast<float>(m_current->extent.height));

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
		ImageBarrier2 barrier;
		barrier.image = m_postProcessor->getSceneColorImage();
		barrier.levelCount = 1;
		barrier.layerCount = 1;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barrier.srcAccess = {};
		barrier.dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		// eColorAttachmentRead as well as Write: resumeScenePassAfterCopy() resumes
		// the scene pass with loadOp=eLoad, which reads this attachment; the read
		// must be ordered after the transition (else READ_AFTER_WRITE).
		barrier.dstAccess = vk::AccessFlagBits2::eColorAttachmentWrite
		                  | vk::AccessFlagBits2::eColorAttachmentRead;

		cmdImageBarrier(m_currentCommandBuffer, barrier);
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

	// Clear values must match the render pass attachments: color at index 0, and
	// depth at index 1 when this target has a depth attachment.
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color = m_stateTracker->getClearColor();
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
	const bool hasDepth = static_cast<bool>(ts->depthImage);

	PassBeginDesc pass;
	pass.renderPass = ts->renderPass;
	pass.framebuffer = fb;
	pass.extent = vk::Extent2D(ts->width, ts->height);
	pass.clearValues = ArrayView<vk::ClearValue>(clearValues.data(), hasDepth ? 2 : 1);
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

		ImageBarrier2 barrier;
		barrier.image = ts->image;
		barrier.levelCount = 1;
		barrier.layerCount = layers;
		barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barrier.srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
		barrier.dstStage = vk::PipelineStageFlagBits2::eBlit;
		barrier.dstAccess = vk::AccessFlagBits2::eTransferRead;

		cmdImageBarrier(m_currentCommandBuffer, barrier);

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
	pass.framebuffer = m_current->framebuffers[m_current->currentImage].get();
	pass.extent = m_current->extent;
	pass.clearValues = clearValues;
	beginTrackedRenderPass(pass);
}

void VulkanRenderer::resumeRenderTargetPass(tcache_slot_vulkan* ts)
{
	// loadOp=eLoad on color keeps whatever was already drawn into the target; depth (if
	// any) still clears. Only the depth clear value is consumed, but supply both so the
	// index layout matches the pass attachments.
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color = m_stateTracker->getClearColor();
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
	const bool hasDepth = static_cast<bool>(ts->depthImage);

	PassBeginDesc pass;
	pass.renderPass = ts->renderPassLoad;
	pass.framebuffer = ts->framebuffer;
	pass.extent = vk::Extent2D(ts->width, ts->height);
	pass.clearValues = ArrayView<vk::ClearValue>(clearValues.data(), hasDepth ? 2 : 1);
	// Match beginRenderTarget: the engine drives the RTT viewport via gr_set_viewport.
	pass.viewport = PassViewport::Keep;
	beginTrackedRenderPass(pass);

	m_renderTargetActive = true;
}

} // namespace graphics::vulkan
