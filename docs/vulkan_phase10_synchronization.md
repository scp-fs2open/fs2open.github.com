# Phase 10: Synchronization

**Status**: COMPLETE

## Overview

Phase 10 introduces the per-frame synchronization primitives that keep GPU work ordered without excessive stalls: each `RenderFrame` owns the semaphores and fence needed for acquire, submit, and present, and the renderer cycles through `MAX_FRAMES_IN_FLIGHT=2` frame resources.

## RenderFrame primitives

- `RenderFrame` owns three synchronization handles:
  - `imageAvailableSemaphore` is signaled by `vkAcquireNextImageKHR` when a swapchain image is ready.
  - `renderingFinishedSemaphore` is signaled when the graphics work is complete.
  - `frameInFlightFence` lets the CPU wait for that frame’s GPU work before reusing the command buffers/staging memory.
- `createPresentSyncObjects()` in `VulkanRenderer` allocates `MAX_FRAMES_IN_FLIGHT` `RenderFrame` objects so the renderer can overlap one frame on the GPU while the next one is recorded.

## Execution flow

1. Each frame starts by calling `RenderFrame::waitForFinish()` to block on `frameInFlightFence`, ensuring the GPU has finished the previous use of that slot.
2. `RenderFrame::acquireSwapchainImage()` calls `vkAcquireNextImageKHR`, waits on the fence, and signals `imageAvailableSemaphore` when the image is ready for rendering.
3. `VulkanRenderer::flip()` submits the command buffer(s) with `renderingFinishedSemaphore` in the signal list and waits on `imageAvailableSemaphore`, so the GPU doesn’t start rendering until the swapchain image is ready.
4. `RenderFrame::submitAndPresent()` then calls `vkQueuePresentKHR`, waits on `renderingFinishedSemaphore`, and in turn signals presentation completion.
5. `PresentResult` is propagated back, and `VulkanRenderer` recreates the swapchain when the result is `Suboptimal` or `OutOfDate`.

## Resource lifetime rules

- Because the renderer waits on each frame’s fence before reusing staging buffers or `RenderFrame` command buffers, the managers’ `beginFrame()` functions can safely delete deferred resources that belonged to that frame index.
- The `m_swapChainImageRenderImage` array ensures that each swapchain image is not re-acquired until the previous GPU work referencing it is finished.

## Key files

- `code/graphics/vulkan/RenderFrame.{h,cpp}` – semaphore/fence lifecycle, `submitAndPresent()`, `acquireSwapchainImage()`.
- `code/graphics/vulkan/VulkanRenderer.cpp` – `createPresentSyncObjects()`, `flip()`, `acquireNextSwapChainImage()`, and swapchain recreation logic that relies on the synchronization results.
