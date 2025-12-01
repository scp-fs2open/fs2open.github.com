FreeSpace2 *S*ource *C*ode *P*roject
==
[![Coverity](https://img.shields.io/coverity/scan/870.svg)](https://scan.coverity.com/projects/870)

Building
--
Before you do anything, make sure you have updated your git submodules, either by running `git submodule update --init --recursive` or by cloning the repository with the `--recursive` flag.<br/>

The main instructions for building can be found at our github wiki, on the [Building](https://github.com/scp-fs2open/fs2open.github.com/wiki/Building) page.

Shader Pipeline (OpenGL/Vulkan Reuse)
--

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