# Phase 11: Draw Calls

**Status**: IN PROGRESS (`gr_screen` hooks and pipeline lookups are wired; descriptor binding and full material/state translation still need to land)

## Overview

Phase 11 is responsible for wiring `gr_screen` draw calls into Vulkan. Each draw needs a pipeline that matches the material, vertex layout, and render pass, along with descriptor sets for textures, uniforms, and samplers.

## Current implementation

- `gr_vulkan_render_primitives()` (and its particle/batched/distortion variants) obtain the current command buffer from `VulkanRenderer::getCurrentCommandBuffer()` and fetch the active render pass.
- Pipelines are requested through `g_vulkanPipelineManager->getOrCreatePipeline(material, layout, prim_type, renderPass)`, which covers blend/depth/stencil state and vertex formats (Phase 5). The draw state cache avoids redundant bindings via `VulkanRenderer::DrawState`.
- Vertex/index buffers are bound via `bindVertexBuffer()`/`bindIndexBuffer()`. Viewport/scissor are set once per scene pass using `setViewportAndScissor()`, so dynamic state is handled in `VulkanRenderer::DrawState`.
- `gr_vulkan_render_model()` reuses the pipeline lookup and supports indexed draws.

## Descriptor binding status

- `bindMaterialDescriptors()` now allocates descriptor sets from `VulkanDescriptorManager`, updates the base texture sampler via `VulkanTextureManager`, binds the descriptor set to each pipeline layout (which includes the shared material descriptor layout), and schedules the set for freeing when the frame completes.
- Texture addressing is respected through the sampler cache, so per-material clamp/repeat/mirror modes are reflected in bound samplers for the base texture. Additional texture slots (specular, glow, normal) are the next TODO and will reuse the same descriptor layout as extra bindings are exposed.
 

## Integration with `gr_screen`

- `vulkan_stubs.cpp` wires `gr_screen.gf_render_primitives*`, `gr_screen.gf_render_model()`, and other draw helpers to the `gr_vulkan_*` implementations.
- `gr_screen.gf_scene_texture_begin`/`gf_scene_texture_end` now call `VulkanRenderer::beginScenePass()`/`endScenePass()`, so draw calls always happen inside an active render pass.
- `gr_screen.gf_set_texture_addressing` and `gr_screen.gf_update_texture` still log TODOs; future descriptor updates will consume these flags.

## Outstanding work

- Implement descriptor allocation/updating/binding for every material type so textures and uniforms are visible to the shaders.
- Honor per-material sampler options (addressing mode, anisotropy, mipmap usage) when binding descriptors.
- Connect particle/model/movie draw paths to their respective shader variants and descriptor layouts.
- Once descriptors are wired, revisit `gr_vulkan_update_texture()` to handle partial uploads and `gr_vulkan_render_primitives_batched()` to issue instanced draws with proper state.

## Key files

- `code/graphics/vulkan/gr_vulkan.cpp` – draw helpers, bind/set state, TODO placeholders.
- `code/graphics/vulkan/VulkanRenderer.cpp` – draw state tracking, viewport/scissor helpers, command buffer access.
- `code/graphics/vulkan/VulkanPipelineManager.{h,cpp}` – pipeline lookup logic used by all draw helpers.
