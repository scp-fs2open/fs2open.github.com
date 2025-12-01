# Phase 2: Shader Compilation

**Status**: COMPLETE

## Overview

Phase 2 provides runtime GLSL to SPIR-V shader compilation with disk caching for the Vulkan renderer. All shaders are compiled on-demand using shaderc, with preprocessing support for FSO's shader system (includes, defines, variants). Compiled SPIR-V binaries are cached to disk to avoid recompilation on subsequent runs.

## Components

### 1. VulkanShaderCache
Handles disk caching of compiled SPIR-V shaders to avoid recompilation.

**Features:**
- Cache keys include: source MD5 hash + shader type + variant flags + shaderc version
- Cache location: `vulkan_shaders` directory (via cfile system)
- Automatic cache invalidation when shaderc version changes
- Respects `-noshadercache` command line flag

**Implementation:** `code/graphics/vulkan/VulkanShader.cpp:93-192`

### 2. VulkanShaderCompiler
Compiles GLSL source code to SPIR-V bytecode using the shaderc library.

**Configuration:**
- Target: Vulkan 1.1
- Optimization level: Performance
- Supports vertex, fragment, and geometry shaders
- Full error reporting and compilation warnings

**Implementation:** `code/graphics/vulkan/VulkanShader.cpp:198-240`

### 3. VulkanShaderManager
High-level orchestration of shader loading, preprocessing, compilation, and caching.

**Features:**
- Shader preprocessing: version headers, defines, includes
- Integration with FSO's `ShaderPreprocessor` for `#include` and `#predefine` directives
- Automatic geometry shader detection based on variant flags
- In-memory cache of loaded `VkShaderModule` objects
- Support for 36 shader types: model, effects, post-processing, deferred lighting, etc.
- Shader variant system via compile-time defines:
  - Model shaders: lighting, shadows, HDR, diffuse/spec/normal maps, fog, team colors, etc.
  - Effect shaders: particle point generation (geometry shader)
  - Post-process: horizontal/vertical blur passes
- MD5 source hashing for cache keying
- Modding support: checks for external shader files when `Enable_external_shaders` is true

**Implementation:** `code/graphics/vulkan/VulkanShader.cpp:354-662`

## Shader Compilation Pipeline

The complete flow for loading a shader:

1. **Request**: `VulkanShaderManager::getShader(type, flags, stage)`
2. **Memory Cache Check**: Return cached `VkShaderModule` if already loaded
3. **Preprocessing**: Load source file, generate header with version/defines, process includes and predefines
4. **Cache Key**: Compute MD5 hash of preprocessed source
5. **Disk Cache Check**: Attempt to load cached SPIR-V binary
6. **Compilation**: If cache miss, compile GLSL→SPIR-V via shaderc
7. **Disk Cache Save**: Store compiled SPIR-V for future runs
8. **Module Creation**: Create `VkShaderModule` from SPIR-V binary
9. **Memory Cache**: Store module for this session
10. **Return**: `VkShaderModule` handle ready for pipeline creation

## Integration with FSO Systems

- **Virtual File System**: Uses cfile for shader loading and cache storage, supporting VP archives
- **ShaderPreprocessor**: Reuses existing `#include` and `#predefine` handling from OpenGL renderer
- **Shader Types**: Mirrors OpenGL shader type enumeration (36 shader types)
- **Variant System**: Uses same flag definitions as OpenGL (model_shader_flags.h)
- **Command Line**: Respects existing flags like `-noshadercache` and `Enable_external_shaders`
- **Logging**: Uses nprintf/mprintf for debug and error output

## Key Implementation Files

| File | Lines | Description |
|------|-------|-------------|
| `code/graphics/vulkan/VulkanShader.h` | 329 | Class declarations (partial - compilation classes) |
| `code/graphics/vulkan/VulkanShader.cpp` | 1024 | Complete implementation (partial - lines 93-662) |

## Dependencies

- **shaderc**: Runtime GLSL→SPIR-V compilation (conditional: `FSO_HAVE_SHADERC`)
- **Vulkan SDK**: Core Vulkan types and `VkShaderModule`
- **FSO Libraries**: cfile, MD5, ShaderPreprocessor

## Limitations

- Shader compilation happens on first use (not at startup), causing frame hitches on first shader load
- Geometry shaders only supported for specific variants (shadow maps, particle point sprites)
- No compute shader support yet (future work)
- Tessellation shaders not supported (OpenGL renderer doesn't use them either)

## Next Phase

**Phase 3: Shader Reflection** - Extract resource bindings from compiled SPIR-V using SPIRV-Cross to drive descriptor set layout creation.
