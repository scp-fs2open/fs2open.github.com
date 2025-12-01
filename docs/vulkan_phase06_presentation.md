# Phase 6: Presentation

**Status**: COMPLETE

## Overview

Phase 6 wires up Vulkan presentation: swapchain creation, HDR surface selection, the present render pass, and the per-frame synchronization that submits `RenderFrame`s to the display.

## Swapchain creation pipeline

- `VulkanRenderer::createSwapChain()` enumerates the surface formats and present modes, then stores the `vk::SwapchainKHR`, `m_swapChainExtent`, `m_swapChainImageFormat`, and display-friendly color space for the rest of the renderer.
- `chooseSurfaceFormat()` prefers the HDR combo (`vk::Format::eA2B10G10R10UnormPack32` + `vk::ColorSpaceKHR::eHdr10St2084EXT`) when `gr_hdr_output_enabled()` and the hardware exposes it. If HDR is unavailable, it falls back to `vk::Format::eB8G8R8A8Srgb`.
- `choosePresentMode()` returns `vk::PresentModeKHR::eMailbox` when vsync is on, `vk::PresentModeKHR::eImmediate` when vsync is disabled, and otherwise uses the guaranteed `vk::PresentModeKHR::eFifo`.
- `chooseSwapChainExtent()` clamps the requested size to the surface’s min/max extents so `m_swapChainExtent` always matches what the surface can accept.

## Framebuffers & render pass

- `VulkanRenderPassManager::createPresentRenderPass()` defines the present pass with a single color attachment whose final layout is `vk::ImageLayout::ePresentSrcKHR`.
- `VulkanRenderer::createSwapchainFramebuffers()` wraps each swapchain image view in a `VulkanFramebuffer` so the present pass can render into it without owning any image memory.
- The HDR-ready scene pass (Phase 8) renders into an offscreen `VulkanFramebuffer` using `R16G16B16A16_SFLOAT` so the presentation pipeline can sample the linear HDR result before tonemapping.

## Presentation loop

- `VulkanRenderer::createPresentSyncObjects()` allocates `RenderFrame` objects (one per `MAX_FRAMES_IN_FLIGHT=2`) with image-available and rendering-finished semaphores plus an in-flight fence.
- `acquireNextSwapChainImage()` waits on the previous frame’s fence, resets per-frame state, notifies the buffer/texture managers via their `beginFrame()` hooks, and calls `RenderFrame::acquireSwapchainImage()` to get the next image index.
- `VulkanRenderer::flip()` submits the command buffers via `RenderFrame::submitAndPresent()` and handles `PresentResult::Suboptimal`/`OutOfDate` by recreating the swapchain when necessary. `RenderFrame` ensures the GPU work is fenced and the resources are recycled safely.

## Key files

- `code/graphics/vulkan/VulkanRenderer.cpp` (`createSwapChain`, `chooseSurfaceFormat`, `flip`)
- `code/graphics/vulkan/RenderFrame.{cpp,h}` (semaphore/fence lifecycle, `submitAndPresent`)
