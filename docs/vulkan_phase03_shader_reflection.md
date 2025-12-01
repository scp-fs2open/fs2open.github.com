# Phase 3: Shader Reflection

**Status**: COMPLETE

## Overview

Phase 3 provides automatic shader resource binding extraction from compiled SPIR-V using SPIRV-Cross. Reflection data (uniform buffers, samplers, push constants) is used to automatically create descriptor set layouts and pipeline layouts, eliminating manual binding specification and ensuring shader/pipeline compatibility.

## Components

### 1. VulkanShaderReflection
Introspects compiled SPIR-V to extract resource bindings using SPIRV-Cross.

**Extracted Resources:**
- Uniform buffers (UBOs) with sizes
- Storage buffers (SSBOs)
- Combined image samplers (textures)
- Separate images and samplers
- Storage images (imageLoad/imageStore)
- Push constant ranges with sizes

**Cross-Platform:**
- C API path for Windows (DLL compatibility)
- C++ API path for Linux/macOS (static linking)

**Stage Merging:**
- Combines reflection data from vertex, fragment, and geometry shaders
- Merges shader stage flags for resources used across multiple stages

**Implementation:** `code/graphics/vulkan/VulkanShader.cpp:736-1017`

### 2. VulkanDescriptorSetLayoutBuilder
Creates Vulkan descriptor set layouts and pipeline layouts from reflection data.

**Features:**
- Groups descriptor bindings by set number
- Handles sparse descriptor set layouts (maintains empty sets for indexing)
- Sorts bindings within each set for consistency
- Creates pipeline layouts with push constant ranges
- Full error handling with detailed logging

**Implementation:** `code/graphics/vulkan/VulkanShader.cpp:247-348`

## Reflection Pipeline

The complete flow for shader reflection:

1. **Compile**: SPIR-V binary produced by VulkanShaderManager
2. **Parse**: SPIRV-Cross parses SPIR-V bytecode
3. **Extract Resources**: Enumerate all resource types (UBOs, samplers, push constants)
4. **Capture Metadata**: Record set numbers, binding slots, sizes, names
5. **Merge Stages**: Combine reflection data from vertex/fragment/geometry shaders
6. **Build Layouts**: Create `vk::DescriptorSetLayout` for each set number
7. **Create Pipeline Layout**: Create `vk::PipelineLayout` with layouts and push constants

## Data Structures

### DescriptorBindingInfo
```cpp
struct DescriptorBindingInfo {
    uint32_t set;                    // Descriptor set number
    uint32_t binding;                // Binding slot
    vk::DescriptorType type;         // UBO, sampler, etc.
    uint32_t count;                  // Array size (usually 1)
    vk::ShaderStageFlags stageFlags; // Which stages use this
    SCP_string name;                 // Variable name
    size_t size;                     // Buffer size (for UBOs)
};
```

### PushConstantInfo
```cpp
struct PushConstantInfo {
    vk::ShaderStageFlags stageFlags; // Which stages use this
    uint32_t offset;                 // Byte offset
    uint32_t size;                   // Size in bytes
};
```

### ShaderReflectionData
```cpp
struct ShaderReflectionData {
    SCP_vector<DescriptorBindingInfo> descriptors;
    SCP_vector<PushConstantInfo> pushConstants;
    uint32_t maxDescriptorSet;       // Highest set number used
};
```

## Key Implementation Files

| File | Lines | Description |
|------|-------|-------------|
| `code/graphics/vulkan/VulkanShader.h` | 329 | Class declarations (partial - reflection classes) |
| `code/graphics/vulkan/VulkanShader.cpp` | 1024 | Complete implementation (partial - lines 247-348, 736-1017) |

## Dependencies

- **SPIRV-Cross**: Shader reflection (conditional: `FSO_HAVE_SPIRV_CROSS`)
  - C API on Windows (`FSO_SPIRV_CROSS_C_API`)
  - C++ API on Linux/macOS
- **Vulkan SDK**: Descriptor and pipeline layout types
- **FSO Libraries**: SCP containers

## Benefits

- **Automatic Binding**: No manual descriptor set layout specification needed
- **Type Safety**: Reflection ensures shader resources match descriptor sets
- **Maintainability**: Shader changes automatically propagate to descriptor layouts
- **Debug Information**: Resource names available for debugging

## Limitations

- Reflection happens at shader load time (adds ~1-2ms per shader first load)
- Requires SPIRV-Cross library (adds ~500KB to binary)
- No support for bindless descriptors or descriptor indexing extensions yet

## Next Phase

**Phase 4: Descriptor Management** - Pool allocation, descriptor set management, and runtime binding updates via VulkanDescriptorManager.
