# Phase 4: Descriptor Management

**Status**: COMPLETE

## Overview

Phase 4 provides descriptor pool allocation and descriptor set management for the Vulkan renderer. VulkanDescriptorManager handles allocation, updating, and binding of descriptor sets for shaders, using a single large pool with dynamic uniform buffer offsets for efficient per-frame uniform updates.

## Component: VulkanDescriptorManager

Manages Vulkan descriptor pools and sets, handling allocation, updating, and binding of descriptor sets for shaders.

**Key Features:**
- Single large descriptor pool for simplicity
- Dynamic uniform buffer offsets for efficient per-frame updates
- Helper methods for common update operations (UBOs, samplers)
- Descriptor set allocation and freeing
- Per-frame descriptor set cycling (eliminates sync issues)

**Implementation:**
- Header: `code/graphics/vulkan/VulkanDescriptorManager.h` (150 lines)
- Source: `code/graphics/vulkan/VulkanDescriptorManager.cpp` (264 lines)

## Architecture

### Pool Strategy

Uses a single large pool pre-allocated with enough descriptors for typical FSO rendering:
- **Uniform Buffers**: 1000 descriptors
- **Combined Image Samplers**: 2000 descriptors
- **Storage Buffers**: 100 descriptors
- **Storage Images**: 100 descriptors

Avoids fragmentation and pool allocation overhead during rendering.

### Dynamic Uniform Buffers

Uses `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC` for frequently updated uniforms:
- Allows binding different offsets into the same buffer
- Enables ring buffer pattern for per-frame uniform data
- Eliminates need for separate descriptor sets per draw call

## Key Methods

### Initialization
```cpp
bool initialize(vk::Device device, vk::PhysicalDevice physicalDevice);
void shutdown();
```

### Descriptor Set Management
```cpp
vk::DescriptorSet allocateSet(vk::DescriptorSetLayout layout,
                               const SCP_string& debugName = "");
void freeSet(vk::DescriptorSet set);
```

### Descriptor Updates
```cpp
void updateUniformBuffer(vk::DescriptorSet set, uint32_t binding,
                         vk::Buffer buffer, vk::DeviceSize offset,
                         vk::DeviceSize range, bool dynamic = true);

void updateCombinedImageSampler(vk::DescriptorSet set, uint32_t binding,
                                vk::ImageView imageView, vk::Sampler sampler,
                                vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

void updateStorageBuffer(vk::DescriptorSet set, uint32_t binding,
                         vk::Buffer buffer, vk::DeviceSize offset,
                         vk::DeviceSize range);

void updateStorageImage(vk::DescriptorSet set, uint32_t binding,
                        vk::ImageView imageView,
                        vk::ImageLayout layout = vk::ImageLayout::eGeneral);
```

### Frame Management
```cpp
void beginFrame(uint32_t frameIndex);
void resetPool();
```

## Integration with Shader Reflection

VulkanDescriptorManager works with layouts created by VulkanDescriptorSetLayoutBuilder:

1. **Shader Load**: Reflection creates descriptor set layouts
2. **Set Allocation**: Allocate descriptor sets from pool using those layouts
3. **Update Bindings**: Populate descriptor sets with actual buffers/images
4. **Bind for Draw**: Bind descriptor sets before draw calls

## Usage Pattern

```cpp
// Initialize (once at startup)
descriptorManager.initialize(device, physicalDevice);

// Per-frame
descriptorManager.beginFrame(currentFrameIndex);

// Per-draw
vk::DescriptorSet set = descriptorManager.allocateSet(layout, "ModelDraw");
descriptorManager.updateUniformBuffer(set, 0, uniformBuffer, offset, size);
descriptorManager.updateCombinedImageSampler(set, 1, diffuseView, sampler);
commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                 pipelineLayout, 0, {set}, {dynamicOffset});
```

## Performance Considerations

- **Pool Size**: Single large pool avoids allocation overhead but wastes memory if undersized
- **Dynamic Offsets**: Efficient for per-frame/per-draw uniform data without descriptor set churn
- **Per-Frame Reset**: Pool is reset each frame, requiring descriptor set re-allocation (tradeoff for simplicity)

## Limitations

- Single pool may run out of descriptors during complex scenes (requires fallback logic)
- No support for descriptor set caching/reuse across frames
- No push descriptor extension support (Vulkan 1.2 feature)
- No descriptor indexing/bindless support yet

## Key Implementation Files

| File | Lines | Description |
|------|-------|-------------|
| `code/graphics/vulkan/VulkanDescriptorManager.h` | 150 | Class declaration and API |
| `code/graphics/vulkan/VulkanDescriptorManager.cpp` | 264 | Complete implementation |

## Dependencies

- **Vulkan SDK**: Descriptor pool and set types
- **Phase 3**: Uses descriptor set layouts from VulkanDescriptorSetLayoutBuilder
- **Phase 1**: Works with VulkanBuffer for uniform/storage buffers

## Next Phase

**Phase 5: Pipeline State** - Pipeline creation, state caching, and hash-based pipeline lookup via VulkanPipelineManager.
