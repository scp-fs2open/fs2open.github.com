# Context for Vulkan mission crash investigation (FSO Vulkan backend)

## Key crash and log findings
- Crash occurs when starting a mission in Vulkan: `std::bad_function_call` thrown inside `stars_setup_environment_mapping` (stack from WinDbg).
- Last log lines show massive `MaterialTextures` descriptor allocations, followed by `ErrorOutOfPoolMemory` for descriptor sets. Then repeated framebuffer validation errors (attachmentCount mismatch) and crash dump generation.
- Shutdown validation reports leaked VkImages and a descriptor set not freed.
- Crash dump analyzed: `build/bin/Debug/fs2_26_0_0_20251201_201603.mdmp` (user mini dump).
- Log path: `%APPDATA%\HardLightProductions\FreeSpaceOpen\data\fs2_open.log`. Key symptoms from the log:
  - Repeated “VulkanDescriptorManager: Allocated descriptor set 'MaterialTextures'” then `vk::Device::allocateDescriptorSets: ErrorOutOfPoolMemory`.
  - Validation spam: `vkCreateFramebuffer(): pCreateInfo->attachmentCount 1 does not match attachmentCount of 2 of VkRenderPass ...` (VUID 00876).
  - Shutdown: multiple `vkDestroyDevice` warnings about VkImages not destroyed.

## Root cause identified (crash)
- `stars_setup_environment_mapping` calls `gr_screen.gf_calculate_irrmap()` (starfield.cpp ~3182).
- `gf_calculate_irrmap` is declared in `code/graphics/2d.h` but Vulkan never assigned it; OpenGL assigns it in `gropengl.cpp` (~990). The empty std::function caused `std::bad_function_call`.
- Fix applied: added `gr_vulkan_calculate_irrmap` (safe/logging stub) and wired it in `vulkan_stubs.cpp` so the callback is non-null. File changes:
  - `code/graphics/vulkan/gr_vulkan.h`: declared `gr_vulkan_calculate_irrmap`.
  - `code/graphics/vulkan/gr_vulkan.cpp`: added stub `gr_vulkan_calculate_irrmap` that logs once and returns.
  - `code/graphics/vulkan/vulkan_stubs.cpp`: set `gr_screen.gf_calculate_irrmap = gr_vulkan_calculate_irrmap`.
  - Note: This does NOT generate irradiance; it only prevents the null-call crash.

## Remaining major issues to fix
- Descriptor pool exhaustion: `MaterialTextures` descriptor sets are allocated repeatedly (likely per envmap face/material) without reuse/reset; leads to `ErrorOutOfPoolMemory`.
  - Allocation site: `code/graphics/vulkan/gr_vulkan.cpp` in `bindMaterialDescriptors` (allocates with label "MaterialTextures").
  - Pool logic: `code/graphics/vulkan/VulkanDescriptorManager.cpp` (per-frame pools, allocateSet, reset).
- Framebuffer validation errors: `vkCreateFramebuffer(): attachmentCount 1 does not match attachmentCount of 2` for render pass 0x3936… immediately before crash; likely envmap/irrmap render target using a render pass expecting color+depth but providing one attachment.
  - Render target creation for envmap/irrmap: `code/starfield/starfield.cpp` uses `bm_make_render_target`; Vulkan path via `gr_vulkan_bm_make_render_target` -> `VulkanTextureManager::createRenderTarget` (adds depth).
  - Render pass creation for RTs: `VulkanTextureManager::createRenderTargetRenderPass` (color + optional depth).
  - Framebuffer creation for cubemap faces: `VulkanTextureManager::createRenderTarget` uses `VulkanFramebuffer::createFromImageViews` with views vector; need to ensure attachment count matches render pass (likely needs depth view included).
- Resource leaks: validation on shutdown shows undestroyed VkImages and one descriptor set not freed; likely related to envmap/irrmap render targets and descriptor allocations.

## Relevant file locations
- Crash site call: `code/starfield/starfield.cpp` (`stars_setup_environment_mapping`).
- `gf_calculate_irrmap` declaration: `code/graphics/2d.h` (~line 776).
- OpenGL irrmap implementation: `code/graphics/opengl/gropengldraw.cpp` (`gr_opengl_calculate_irrmap` uses shader `SDR_TYPE_IRRADIANCE_MAP_GEN`, draws full-screen quad per cubemap face).
  - OpenGL sequence to mirror: save previous RT; `gr_opengl_maybe_create_shader(SDR_TYPE_IRRADIANCE_MAP_GEN)`; bind envmap cubemap; for each face { `bm_set_render_target(gr_screen.irrmap_render_target, face)`; `gr_clear()`; set uniform `irrmap_data.face = i`; draw full-screen quad/tri }; restore previous RT.
- `irradiance_map_gen` (starfield.cpp ~920) creates irrmap RT: size 16x16, flags `BMP_FLAG_RENDER_TARGET_STATIC | BMP_FLAG_CUBEMAP | BMP_FLAG_RENDER_TARGET_MIPMAP` (switches to dynamic for subspace/dynamic env).
- Vulkan shader mapping includes `SDR_TYPE_IRRADIANCE_MAP_GEN`: `code/graphics/vulkan/VulkanShader.cpp`.
- Vulkan function pointer setup: `code/graphics/vulkan/vulkan_stubs.cpp` (now assigns `gf_calculate_irrmap`).
- Vulkan render target creation: `code/graphics/vulkan/VulkanTexture.cpp` (`createRenderTarget`, `setRenderTarget`, per-face framebuffers).
- Vulkan render pass for RTs: `VulkanTextureManager::createRenderTargetRenderPass` (color + optional depth).
- Descriptor allocations: `code/graphics/vulkan/gr_vulkan.cpp` `bindMaterialDescriptors`.
- Descriptor manager: `code/graphics/vulkan/VulkanDescriptorManager.cpp`.

## Repro/testing commands
- Run mission in Vulkan Debug: `build\bin\Debug\fs2_26_0_0.exe -vulkan -window` (adjust exe name).
- Enable validation for repro: set `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, `VK_LOADER_DEBUG=all`.
- Analyze crash dump: `cdbX64.exe -z "<dump>.mdmp" -c ".symfix; .sympath+ build\\bin\\Debug; .reload; !analyze -v; .ecxr; kv; q"`.
- Tests (if available): `ctest -C Debug --output-on-failure -VV`.

## Next fixes to implement
1) Implement Vulkan irradiance map generation (parity with OpenGL):
   - Use envmap cubemap as input, render to `gr_screen.irrmap_render_target` (cubemap) per face.
   - Pipeline/shader: `SDR_TYPE_IRRADIANCE_MAP_GEN` (present in `VulkanShader.cpp`).
   - Fullscreen triangle draw per face with uniform `irrmap_data { int face; }`.
   - Bind envmap texture/sampler and output to irrmap framebuffer for each face; then restore render target.
2) Descriptor exhaustion:
   - Ensure descriptor pools are reset per frame; cache material descriptor sets per material if possible; avoid allocating per face without reuse.
3) Framebuffer attachment mismatch:
   - When creating cubemap face framebuffers, include both color (and depth if render pass expects it). Match `attachmentCount` to render pass from `createRenderTargetRenderPass`.
4) Cleanup leaks:
   - Ensure envmap/irrmap images and framebuffers are destroyed on mission end/shutdown; descriptor sets freed/reset per frame.
