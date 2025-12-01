# Phase 9: Frame Execution

**Status**: IN PROGRESS (per-frame buffering, transfers, and the scene-pass/post-pass plumbing are wired; descriptor/state binding still needs to feed `gr_vulkan_render_*`)

## Achievements

- **Non-blocking buffer transfers**: `VulkanBufferManager::copyViaStaging()` records copies into per-frame transfer command buffers instead of waiting on fences. `submitTransfers()` flushes the recording before graphics work, and `beginFrame()` cleans up deferred staging buffers after each frame’s fence fires.
- **Non-blocking texture uploads**: `VulkanTextureManager` allocates per-frame upload command buffers, partitions the 16 MB staging buffer by `FRAMES_IN_FLIGHT`, and records copies via `ensureUploadRecording()` so uploads can be batched without blocking the CPU. `submitUploads()` is called right before rendering work to submit the batched commands.
- **Frame lifecycle management**: `VulkanRenderer::acquireNextSwapChainImage()` waits on `RenderFrame` fences, calls `beginFrame()` on every manager, acquires the next image, and tracks per-swapchain bookkeeping. The render loop now always calls `getCurrentCommandBuffer()` only during an active scene pass.
- **Scene pass & post-pass control**: `VulkanRenderer::beginScenePass()`/`endScenePass()` manage the scene framebuffer’s render pass, and `recordBlitToSwapchain()` renders a fullscreen triangle that samples the scene texture before `flip()` submits the commands via `RenderFrame::submitAndPresent()`.
- **`gr_screen` hook alignment**: `gr_vulkan_setup_frame()`, `gr_vulkan_scene_texture_begin()`, and `gr_vulkan_scene_texture_end()` now forward to the renderer, exposing the scene-pass lifecycle to the higher-level rendering path.

## Current render loop (Phase 9 flow)

1. `RenderFrame::waitForFinish()` and fence reset ensure the GPU has finished frame N‑2.
2. `VulkanRenderer::acquireNextSwapChainImage()` calls `beginFrame(currentFrame)` on every manager so frame-indexed staging allocations can be recycled and deferred deletions processed.
3. `acquireNextSwapChainImage()` grabs the next swapchain image via `RenderFrame::acquireSwapchainImage()` and tracks it in `m_swapChainImageRenderImage`.
4. `gr_vulkan_setup_frame()` resets any CPU-side draw state.
5. `gr_vulkan_scene_texture_begin()` submits pending buffer/texture transfers, allocates a new command buffer, begins the scene render pass (`m_sceneFramebuffer`), and clears the attachments.
6. Game code records draw commands via the `gr_vulkan_render_*` helpers that call `g_vulkanPipelineManager->getOrCreatePipeline()` and emit draws into the active scene command buffer.
7. `gr_vulkan_scene_texture_end()` closes the scene render pass so the color image transitions to `vk::ImageLayout::eShaderReadOnlyOptimal`.
8. `VulkanRenderer::flip()` calls `recordBlitToSwapchain()` to sample the scene texture via the blit pipeline, ends the command buffer, and submits it through `RenderFrame::submitAndPresent()` while handling `PresentResult`s.
9. The frame index wraps via `m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT`, and the loop repeats.

## Outstanding work

- Descriptor updates now allocate descriptor sets via `VulkanDescriptorManager`, but the next step is to expand the binding helpers to cover more material texture slots and uniform buffers (e.g., movie textures, decal buffers).
- Multi-pass post-processing (bloom/SSAO/TAA/tonemapping) still needs work to sample from the scene texture/render targets managed in Phase 12.

## Key files

- `code/graphics/vulkan/VulkanBuffer.{h,cpp}` – staging allocation, per-frame transfers, `submitTransfers()`.
- `code/graphics/vulkan/VulkanTexture.{h,cpp}` – per-frame upload, staging partitioning, `submitUploads()`.
- `code/graphics/vulkan/VulkanRenderer.cpp` – `acquireNextSwapChainImage()`, scene pass, blit pipeline, `flip()`.
- `code/graphics/vulkan/gr_vulkan.cpp` – `gr_vulkan_*` hooks and raw draw helpers.
