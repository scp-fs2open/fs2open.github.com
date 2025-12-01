# Vulkan Shader System

## Overview

The Vulkan shader system handles GLSL to SPIR-V compilation, caching, and shader variant management. Located in `code/graphics/vulkan/VulkanShader.cpp/h`.

## Architecture

### VulkanShaderManager
Main class managing shader compilation and caching:
```cpp
class VulkanShaderManager {
    vk::Device m_device;
    shaderc::Compiler m_compiler;
    SCP_unordered_map<size_t, CompiledShader> m_cache;
};
```

### CompiledShader
Cached shader data:
```cpp
struct CompiledShader {
    vk::UniqueShaderModule module;
    std::vector<uint32_t> spirv;  // For reflection
    ShaderType type;
};
```

## Shader Compilation Pipeline

### 1. Source Loading
Shaders loaded from `data/effects/` via cfile:
```cpp
// Shader naming convention
main.sdr           // Main shader (contains multiple stages)
tonemapping-f.sdr  // Fragment-only shader
lighting-v.sdr     // Vertex-only shader
```

### 2. Preprocessing
Custom preprocessor (`ShaderPreprocessor.cpp/h`):
- `#include` resolution via cfile
- Macro injection based on flags
- Line number tracking for errors

### 3. Compilation
Using shaderc library:
```cpp
shaderc::CompileOptions options;
options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
options.SetSourceLanguage(shaderc_source_language_glsl);

auto result = compiler.CompileGlslToSpv(source, kind, filename, options);
```

### 4. Caching
Cache key generation:
```cpp
size_t cacheKey = hashCombine(
    sourceHash,
    flagsHash,
    shaderType
);
```

## Shader Flags

Defined in `code/graphics/util/uniform_structs.h`:

| Flag | Purpose |
|------|---------|
| `SDR_FLAG_DIFFUSE_MAP` | Has diffuse texture |
| `SDR_FLAG_GLOW_MAP` | Has glow/emissive texture |
| `SDR_FLAG_SPEC_MAP` | Has specular texture |
| `SDR_FLAG_NORMAL_MAP` | Has normal map |
| `SDR_FLAG_HEIGHT_MAP` | Has height/parallax map |
| `SDR_FLAG_ENV_MAP` | Has environment map |
| `SDR_FLAG_ANIMATED` | Animated texture |
| `SDR_FLAG_SOFT_QUAD` | Soft particle rendering |
| `SDR_FLAG_DISTORTION` | Distortion effect |

## Uniform Blocks

### Global Uniforms (Set 0, Binding 0)
```glsl
layout(std140, set = 0, binding = 0) uniform GlobalData {
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 viewProjMatrix;
    vec4 cameraPosition;
    vec4 nearFar;  // x=near, y=far, z=1/near, w=1/far
};
```

### Model Uniforms (Set 0, Binding 1)
```glsl
layout(std140, set = 0, binding = 1) uniform ModelData {
    mat4 modelMatrix;
    mat4 normalMatrix;
    vec4 color;
};
```

### Material Uniforms (Set 0, Binding 2)
```glsl
layout(std140, set = 0, binding = 2) uniform MaterialData {
    vec4 diffuseColor;
    vec4 specularColor;
    float shininess;
    float emissive;
};
```

## Texture Bindings (Set 1)

```glsl
layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D glowTex;
layout(set = 1, binding = 2) uniform sampler2D specTex;
layout(set = 1, binding = 3) uniform sampler2D normalTex;
```

## Shader Variants

The shader system generates variants based on feature flags:
```cpp
// Example: 8 flags = up to 256 variants per shader
// Lazy compilation - only compile when first needed
ShaderVariant getVariant(ShaderFlags flags) {
    auto key = computeKey(baseShader, flags);
    if (!cache.contains(key)) {
        cache[key] = compileWithFlags(baseShader, flags);
    }
    return cache[key];
}
```

## Geometry Shader Requirements

Some effects require geometry shaders:
```cpp
bool requiresGeometryShader(ShaderFlags flags) {
    return flags & (SDR_FLAG_SOFT_QUAD | SDR_FLAG_DISTORTION);
}
```

## Error Handling

Compilation errors include source location:
```cpp
if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    mprintf(("Shader compilation failed: %s\n", result.GetErrorMessage().c_str()));
    // Error message includes line numbers from preprocessor
}
```

## SPIR-V Reflection

Used for automatic descriptor set layout creation:
```cpp
// Using SPIRV-Cross
spirv_cross::Compiler compiler(spirvCode);
auto resources = compiler.get_shader_resources();

// Extract uniform buffers
for (auto& ubo : resources.uniform_buffers) {
    uint32_t set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
    uint32_t binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
}

// Extract samplers
for (auto& sampler : resources.sampled_images) {
    // ...
}
```

## Build Requirements

### Required Libraries
- `shaderc` - GLSL to SPIR-V compiler
- `spirv-cross` - SPIR-V reflection

### CMake
```cmake
# Found automatically from Vulkan SDK
find_package(shaderc REQUIRED)
find_package(spirv-cross REQUIRED)
```

### Preprocessor Define
```cpp
#ifdef FSO_HAVE_SHADERC
// Runtime compilation available
#else
// Must use precompiled SPIR-V
#endif
```

## Performance Considerations

1. **Compile on first use** - Avoid startup stall
2. **Cache aggressively** - Same flags = same shader
3. **Precompile common variants** - Optional warmup phase
4. **SPIR-V caching** - Save compiled shaders to disk

## Migration from OpenGL

Key differences:
- GLSL version 450 (not 150/330)
- Explicit descriptor sets and bindings
- No built-in uniforms (gl_ModelViewMatrix, etc.)
- Push constants for frequently changing data
- Separate sampler and texture objects (combined for simplicity)
