# Phase 12: Render Targets

**Status**: IN PROGRESS (scene framebuffer is hosted in VulkanRenderer and the VulkanTextureManager exposes render target creation; the draw path still needs to honor transitions between scene and custom render targets)

## Scene framebuffer (FBO equivalent)

- `VulkanRenderer::createSceneFramebuffer()` builds a framebuffer whose color attachment is `vk::Format::eR16G16B16A16Sfloat` and whose depth attachment uses the best available depth format (preferring `D32Sfloat`). The attachments use `DeviceLocal` memory and play back to the scene render pass created by `VulkanRenderPassManager`.
- The scene render pass stores the color attachment in `ShaderReadOnlyOptimal` so the post-pass blit pipeline can sample it via `recordBlitToSwapchain()`.
- Culminates Phase 12’s goal of rendering to an intermediate texture before presenting or dispatching post-processing.

## VulkanTextureManager render target API

- `createRenderTarget()` allocates a new `VulkanTexture`, stores it in `bitmap_slot::gr_info`, and builds a `VulkanRenderTarget` that owns a render pass, optional depth attachment, and framebuffers (including cubemap faces when `BMP_FLAG_CUBEMAP` is set).
- `setRenderTarget()` switches the active render target handle (or restores the default scene framebuffer when `handle < 0`). It validates cubemap face indices and records the working handle so draw routines know what to render into.
- `destroyRenderTarget()` releases the framebuffer while leaving the texture pointer alive so bmpman can continue to manage the `gr_bitmap`.
- `readbackTexture()` provides GPU→CPU copies for screenshots or tools, honoring layout transitions and cleanup.

## Bitmap hooks

- `vulkan_stubs.cpp` assigns `gr_screen.gf_bm_make_render_target` and `gr_screen.gf_bm_set_render_target` to the Vulkan implementations, so the code paths that create render targets for cubemaps, movie textures, and RTTs already invoke this API.
- `gr_vulkan_bm_make_render_target()` and `gr_vulkan_bm_set_render_target()` forward directly to the manager, ensuring handles are tracked consistently.

## Outstanding work

- Draw call helpers now query `VulkanTextureManager::isRenderingToTexture()` and use the active `VulkanRenderTarget` framebuffer/render pass if one is bound, but per-material bindings beyond the base texture (e.g., impulse, movie textures) still require descriptor layout extensions.
- Post-processing chains (Phase 13) still need to sample from whichever render target finished last and may add render-target-specific synchronizations prior to tonemapping/bloom passes.

## Key files

- `code/graphics/vulkan/VulkanRenderer.cpp` – scene framebuffer creation (`createSceneFramebuffer`), render pass transitions, blit pipeline.
- `code/graphics/vulkan/VulkanTexture.cpp` – render-target allocation, `createRenderTarget`, `setRenderTarget`, `readbackTexture`.
- `code/graphics/vulkan/vulkan_stubs.cpp` – `gr_screen` hook wiring for render target APIs.
