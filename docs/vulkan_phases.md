# Vulkan Implementation Phases

Keeping track of the Vulkan bring-up so work stays organized and scoped.

## Phase 1: Core Buffer & Memory [COMPLETE]
- VulkanBuffer manager (allocation, staging uploads, mapping, lifetime)

## Phase 2: Shader Compilation [COMPLETE]
- Shader pipeline: GLSL → SPIR-V (shaderc, preprocessing, disk cache)
- VulkanShaderCache, VulkanShaderCompiler, VulkanShaderManager

## Phase 3: Shader Reflection [COMPLETE]
- VulkanShaderReflection via SPIRV-Cross
- VulkanDescriptorSetLayoutBuilder (descriptor set layout creation)

## Phase 4: Descriptor Management [COMPLETE]
- VulkanDescriptorManager (pools, layouts, descriptor updates)

## Phase 5: Pipeline State [COMPLETE]
- VulkanPipelineManager (hash cache, blend/depth modes, vertex formats, disk persistence)

## Phase 6: Presentation [COMPLETE]
- HDR10 swapchain (A2B10G10R10 + ST2084 PQ)

## Phase 7: Textures [IN PROGRESS]
- VulkanTexture + VulkanSamplerCache + VulkanTextureManager (per-frame staging, palette expansion, render-target API). `gr_screen.gf_bm_*` now routes to the Vulkan upload path, but sampler addressing/partial updates will be wired up once descriptor binding is finished.

## Phase 8: Framebuffers [COMPLETE]
- VulkanFramebuffer + VulkanRenderPassManager

## Phase 9: Frame Execution [IN PROGRESS]
- Non-blocking buffer and texture transfers (`copyViaStaging`, `submitTransfers`, `submitUploads`)
- Scene-pass control (`beginScenePass`, `endScenePass`, `recordBlitToSwapchain`) plus the `gr_vulkan_scene_texture_*` hooks
 
## Phase 10: Synchronization [COMPLETE]
- Frame sync (semaphores, fences, MAX_FRAMES_IN_FLIGHT)

## Phase 11: Draw Calls [IN PROGRESS]
- `gr_screen.gf_render_*` now call `gr_vulkan_render_*`, pipelines are selected via `VulkanPipelineManager`, but descriptors/uniform updates remain TODO.

## Phase 12: Render Targets [IN PROGRESS]
- Scene framebuffer + swapchain-target blit are functional, and `VulkanTextureManager` exposes `createRenderTarget`/`setRenderTarget`/`readback`, yet the draw path still needs to honor the active render target.

## Phase 13: Post-Processing [PLANNED]
- Multi-pass pipeline (bloom, SSAO, TAA, tonemapping)
