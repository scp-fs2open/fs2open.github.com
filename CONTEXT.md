# Vulkan Vulkan Backend Context (FSO)

> This file exists **only** to feed context to LLM-based tooling.

---

## Architecture

- **API version**: Vulkan 1.4 (`VK_API_VERSION_1_4` in `VulkanRenderer.cpp`)
- **Dynamic rendering**:
  - Uses `vkCmdBeginRendering` + `VkRenderingAttachmentInfo`
  - `VkFramebuffer` objects are *not* created (`VulkanFramebuffer::getFramebuffer()` returns `nullptr`)
  - Render passes are still created (`createRenderTargetRenderPass`) but graphics pipelines use `renderPass = nullptr` and specify formats via `pNext`
- **Key managers**:
  - `VulkanRenderer` – main renderer, swapchain, dynamic rendering, owns other managers
  - `VulkanTextureManager` – textures, render targets (2D + cubemap), sampler cache
  - `VulkanDescriptorManager` – single descriptor pool, allocation/free, tracking
  - `VulkanPipelineManager` – pipeline creation/caching, descriptor set layouts
  - `VulkanBufferManager` – buffer allocation (vertex/index/uniform/staging)

---

## Crash fix: `gf_calculate_irrmap` (null std::function)

- **Problem**: `std::bad_function_call` in `stars_setup_environment_mapping` when calling `gr_screen.gf_calculate_irrmap()`.
- **Root cause**:
  - `gf_calculate_irrmap` is declared in `code/graphics/2d.h` and used in `code/starfield/starfield.cpp`.
  - OpenGL assigns it in `code/graphics/opengl/gropengldraw.cpp` (`gr_opengl_calculate_irrmap`).
  - Vulkan never assigned it; `std::function` remained empty → null-call crash.
- **Fix**:
  - Implemented `gr_vulkan_calculate_irrmap()` in `code/graphics/vulkan/gr_vulkan.cpp`.
  - Declared in `code/graphics/vulkan/gr_vulkan.h`.
  - Wired in `code/graphics/vulkan/vulkan_stubs.cpp`:
    - `gr_screen.gf_calculate_irrmap = gr_vulkan_calculate_irrmap;`
  - Implementation:
    - Uses `VulkanTextureManager` to fetch envmap (`ENVMAP`) and irrmap render target.
    - For each cubemap face:
      - `bm_set_render_target(gr_screen.irrmap_render_target, face)`
      - `VulkanRenderer::beginAuxiliaryRenderPass(...)` with that face’s `VulkanFramebuffer`
      - Bind `SDR_TYPE_IRRADIANCE_MAP_GEN` pipeline via `VulkanPipelineManager`
      - Bind envmap texture + sampler in a descriptor set
      - Draw fullscreen triangle (no vertex buffer, `gl_VertexIndex`-style)
      - End auxiliary render pass
- **Status**: Null `gf_calculate_irrmap` crash is **fixed**. Missions can still crash from other causes (not assumed solved here).

---

## Mistakes to avoid (DO NOT REPEAT)

### Sampler type mismatch: `sampler2DArray` in default material

- **Problem**: Changing `default-material.frag` to use `sampler2DArray` made all menus/UI invisible (black).
- **Why**:
  - Menu textures are 2D with `VK_IMAGE_VIEW_TYPE_2D` views.
  - `sampler2DArray` expects `VK_IMAGE_VIEW_TYPE_2D_ARRAY`.
  - Binding 2D views to a `sampler2DArray` uniform silently fails (no validation error, nothing rendered).
- **DO NOT**:
  - Change `code/def_files/data/effects/default-material.frag` to `sampler2DArray`.
  - Add array-view accessors (e.g. `getImageViewArray()`) to `VulkanTexture` for default materials.
  - Bind 2D-array views to descriptor slots intended for `sampler2D`.
- **DO**:
  - Keep default material shaders using `sampler2D`:
    - `default-material.frag`: `layout(set = 1, binding = 0) uniform sampler2D baseMap;`
    - `default-material.vert`: `fragColor = color;` (uniform only, no `vertColor` input).
  - In `gr_vulkan.cpp` material binding:
    - Use `texture->getImageView()` (2D views) for default materials.
  - Treat NanoVG separately:
    - NanoVG shaders (`nanovg-f.sdr`, `nanovg-v.sdr`) legitimately use `sampler2DArray` and need array textures.

---

## File map (for symbol lookup)

- **Core Vulkan renderer**:
  - `code/graphics/vulkan/VulkanRenderer.cpp/.h`
  - `code/graphics/vulkan/RenderFrame.cpp/.h`
  - `code/graphics/vulkan/gr_vulkan.cpp/.h`
  - `code/graphics/vulkan/vulkan_stubs.cpp`
- **Resources & pipelines**:
  - `code/graphics/vulkan/VulkanTexture.cpp/.h`
  - `code/graphics/vulkan/VulkanDescriptorManager.cpp/.h`
  - `code/graphics/vulkan/VulkanPipelineManager.cpp/.h`
  - `code/graphics/vulkan/VulkanBuffer.cpp/.h`
  - `code/graphics/vulkan/VulkanFramebuffer.cpp/.h`
  - `code/graphics/vulkan/VulkanShader.cpp/.h`
  - `code/graphics/vulkan/VulkanPostProcessing.cpp/.h`
- **Irradiance/envmap path**:
  - `code/starfield/starfield.cpp` – `irradiance_map_gen()`, `stars_setup_environment_mapping()`
  - `code/graphics/2d.h` – `gf_calculate_irrmap` declaration
  - `code/graphics/opengl/gropengldraw.cpp` – OpenGL reference (`gr_opengl_calculate_irrmap`)
  - `code/graphics/vulkan/gr_vulkan.cpp/.h` – `gr_vulkan_calculate_irrmap` implementation
  - `code/graphics/vulkan/VulkanShader.cpp` – `SDR_TYPE_IRRADIANCE_MAP_GEN` mapping
- **Default material / UI shaders**:
  - `code/def_files/data/effects/default-material.frag`
  - `code/def_files/data/effects/default-material.vert`
  - `code/def_files/data/effects/nanovg-f.sdr`
  - `code/def_files/data/effects/nanovg-v.sdr`

---

## Testing & debugging

- **Run Vulkan build (Debug, windowed)**:

```cmd
build\bin\Debug\fs2_26_0_0.exe -vulkan -window
```

- **Enable validation layers**:

```cmd
set VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
set VK_LOADER_DEBUG=all
build\bin\Debug\fs2_26_0_0.exe -vulkan -window
```

- **Crash dump analysis (CDB)**:

```cmd
cdbx64.exe -z "<dump>.mdmp" -c ".symfix; .sympath+ build\\bin\\Debug; .reload; !analyze -v; .ecxr; kv; q"
```

> Tip: `cdbx64.exe` ships with the Windows SDK under `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\`. Add that directory to `PATH` or invoke the tool via its full path when running the command above.

- **Unit tests**:

```cmd
cd build
ctest -C Debug --output-on-failure -VV
```

- **Logs**:
  - Main log: `%APPDATA%\HardLightProductions\FreeSpaceOpen\data\fs2_open.log`
  - Validation log: `vulkan_debug.log` (game directory, written by `debugReportCallback`)
  - HDR surface debug: `vulkan_hdr_debug.txt` (game directory)
  - Helper scripts: `scripts/vulkan_debug_session.sh` (bash) and `scripts/vulkan_debug_session.ps1` (PowerShell) run the Vulkan exe and snapshot these logs into `out/vulkan_debug/<timestamp>/` for easier sharing with LLM tooling.

---

## Historical validation errors (for pattern matching)

- **Descriptor pool exhaustion** (historical):
  - Error: `vk::Device::allocateDescriptorSets: ErrorOutOfPoolMemory`
  - Likely site: `bindMaterialDescriptors` in `gr_vulkan.cpp` (label `"MaterialTextures"`)
  - Current pool is large (`POOL_SIZE_COMBINED_IMAGE_SAMPLER = 65536`, `POOL_MAX_SETS = 4096`), so this may not repro now.
  - If it does: consider caching/reusing per-material descriptor sets instead of allocating per draw.

- **Framebuffer attachment count mismatch** (pre–dynamic rendering):
  - Error: `vkCreateFramebuffer(): pCreateInfo->attachmentCount 1 does not match attachmentCount of 2`
  - Came from old code that actually created `VkFramebuffer` objects for render targets.
  - With dynamic rendering, check `vkCmdBeginRendering` attachments instead (color/depth views + formats) if a similar error reappears.


