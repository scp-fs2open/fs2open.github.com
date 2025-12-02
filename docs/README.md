# FSO Developer Documentation

This folder contains technical documentation for the FreeSpace Open codebase, focusing on implementation details that are frequently referenced during development.

## Vulkan Renderer

The Vulkan renderer is an alternative graphics backend under active development. It targets Vulkan 1.4, so development/testing should use an SDK/driver that exposes that API level and its promoted features (BDA, descriptor indexing, dynamic rendering).

### Reference Documentation
| Document | Description |
|----------|-------------|
| [vulkan-renderer.md](vulkan-renderer.md) | Architecture overview, file structure, key classes |
| [vulkan-shaders.md](vulkan-shaders.md) | Shader compilation, uniform blocks, texture bindings |
| [vulkan-buffers.md](vulkan-buffers.md) | Buffer management, staging strategy, synchronization |
| [bmpman-integration.md](bmpman-integration.md) | Texture lifecycle, gr_screen hooks, format mapping |

### Implementation Phases
| Phase | Document | Scope | Status |
|-------|----------|-------|--------|
| 1 | [vulkan_phase01_core_buffer_memory.md](vulkan_phase01_core_buffer_memory.md) | Core Buffer & Memory (VulkanBuffer) | Complete |
| 2 | [vulkan_phase02_shader_compilation.md](vulkan_phase02_shader_compilation.md) | Shader pipeline (GLSL → SPIR-V, shaderc, preprocessing, disk cache) | Complete |
| 3 | [vulkan_phase03_shader_reflection.md](vulkan_phase03_shader_reflection.md) | VulkanShaderReflection (SPIRV-Cross) | Complete |
| 4 | [vulkan_phase04_descriptor_management.md](vulkan_phase04_descriptor_management.md) | VulkanDescriptorManager | Complete |
| 5 | [vulkan_phase05_pipeline_state.md](vulkan_phase05_pipeline_state.md) | VulkanPipelineManager (hash cache, blend/depth modes, vertex formats, disk persistence) | Complete |
| 6 | [vulkan_phase06_presentation.md](vulkan_phase06_presentation.md) | HDR10 swapchain (A2B10G10R10 + ST2084 PQ) | Complete |
| 7 | [vulkan_phase07_textures.md](vulkan_phase07_textures.md) | VulkanTexture + VulkanSamplerCache + VulkanTextureManager | In Progress (per-frame staging/upload batching wired; sampler descriptors & partial updates pending) |
| 8 | [vulkan_phase08_framebuffers.md](vulkan_phase08_framebuffers.md) | VulkanFramebuffer + VulkanRenderPassManager | Complete |
| 9 | [vulkan_phase09_frame_execution.md](vulkan_phase09_frame_execution.md) | Command buffer strategy (per-frame pools, recording patterns) | In Progress (per-frame transfers and descriptor binding for base textures implemented, multi-texture/post-process chaining still pending) |
| 10 | [vulkan_phase10_synchronization.md](vulkan_phase10_synchronization.md) | Frame sync (semaphores, fences, MAX_FRAMES_IN_FLIGHT) | Complete |
| 11 | [vulkan_phase11_draw_calls.md](vulkan_phase11_draw_calls.md) | Wire up gr_screen, actual geometry rendering | In Progress (base texture descriptor binding in place, additional bindings/uniform updates remain) |
| 12 | [vulkan_phase12_render_targets.md](vulkan_phase12_render_targets.md) | FBO equivalents, render-to-texture | In Progress (scene framebuffer + VulkanTextureManager RT API ready; draw path still needs active target management) |
| 13 | [vulkan_phase13_post_processing.md](vulkan_phase13_post_processing.md) | Multi-pass pipeline (bloom, SSAO, TAA, tonemapping) | Planned |

See [vulkan_phases.md](vulkan_phases.md) for the overall roadmap.

## Quick Reference

### Key File Locations
```
code/graphics/vulkan/     - Vulkan renderer implementation
code/graphics/opengl/     - OpenGL renderer (reference)
code/graphics/2d.h        - gr_screen function pointer definitions
code/bmpman/              - Bitmap/texture manager
code/def_files/data/effects/ - Shader source files
```

### Global Instances
```cpp
g_vulkanBufferManager   // VulkanBufferManager*
g_vulkanTextureManager  // VulkanTextureManager*
```

### Common Patterns

**Staging memory allocation:**
```cpp
vk::DeviceSize offset;
void* ptr = allocateStagingMemory(size, offset);
// offset captured before alignment
```

**Layout transition:**
```cpp
texture->transitionLayout(cmd, oldLayout, newLayout);
// or for external transitions (render pass):
texture->notifyLayoutChanged(newLayout);
```

**Compressed format check:**
```cpp
bool compressed = isCompressedFormat(format);
// BC1/BC2/BC3/BC7 -> true
```

### Build Commands
```bash
# Configure with Vulkan
cmake -DFSO_BUILD_WITH_VULKAN=ON ..

# Build code library
cmake --build . --config Release --target code --parallel

# Run tests
./bin/Release/unittests --gtest_shuffle
```

## Documentation Guidelines

When adding new documentation:
1. Focus on implementation details, not user-facing features
2. Include code snippets for common patterns
3. Document integration points and dependencies
4. Note known limitations and TODOs
5. Keep format consistent with existing docs

### Shader Pipeline (OpenGL/Vulkan Reuse)

Shaders are written once in GLSL and cross-compiled for both backends:

```
Source GLSL                    glslc                      shadertool
(code/graphics/shaders/)  -->  SPIR-V (.spv)  -->  GLSL for OpenGL (.spv.glsl)
       |                           |                      + C++ struct headers
       |                           |                        (_structs.*.h)
       v                           v
   Vulkan uses              code/graphics/shaders/compiled/
   runtime compile              (checked into git)
```

**Source files:** `code/graphics/shaders/` (e.g., `vulkan.frag`, `vulkan_blit.vert`)

**Compiled outputs** in `code/graphics/shaders/compiled/`:
- `.spv` - SPIR-V bytecode for Vulkan
- `.spv.glsl` - Cross-compiled GLSL for OpenGL
- `_structs.*.h` - C++ headers with uniform struct definitions

**Build-time tools:**
- `glslc` (from Vulkan SDK) - compiles GLSL to SPIR-V
- `shadertool` (auto-downloaded from `scp-fs2open/fso-shadertool`) - cross-compiles SPIR-V back to GLSL and generates struct headers

**Default behavior:** `SHADERS_ENABLE_COMPILATION=OFF` - pre-compiled shaders from git are used. No shader tools required for normal builds.

**To enable shader compilation:**
```bash
cmake -DSHADERS_ENABLE_COMPILATION=ON -DFSO_BUILD_WITH_VULKAN=ON ..
```
Requires `glslc` in PATH. `shadertool` is auto-downloaded from GitHub releases.
