# Vulkan Renderer Architecture

## Overview

The Vulkan renderer is an alternative graphics backend to OpenGL, located in `code/graphics/vulkan/`. It integrates with FSO's existing graphics abstraction layer via `gr_screen` function pointers and targets the Vulkan 1.4 API level (buffer device address, descriptor indexing, dynamic rendering).

## File Structure

```
code/graphics/vulkan/
├── VulkanRenderer.cpp/h      - Main renderer, initialization, frame management
├── VulkanBuffer.cpp/h        - Buffer management (vertex, index, uniform)
├── VulkanShader.cpp/h        - Shader compilation (GLSL->SPIR-V via shaderc)
├── VulkanTexture.cpp/h       - Texture management, bmpman integration
├── VulkanFramebuffer.cpp/h   - Framebuffer and render pass management
├── VulkanDescriptorManager.cpp/h - Descriptor set/pool management
├── VulkanPipelineManager.cpp/h   - Graphics pipeline caching
├── RenderFrame.cpp/h         - Per-frame resources and synchronization
└── vulkan_stubs.cpp          - Stub implementations for unimplemented gr_screen functions
```

## Key Classes

### VulkanRenderer
Main entry point. Manages:
- Vulkan instance, surface, device creation
- Swapchain management
- Command pools (with `eTransient | eResetCommandBuffer` flags)
- Frame synchronization (MAX_FRAMES_IN_FLIGHT = 2)

Key members:
- `m_graphicsCommandPool` - Command pool for graphics operations
- `m_bufferManager` - Global buffer management
- `m_shaderManager` - Runtime shader compilation
- `m_descriptorManager` - Descriptor set allocation
- `m_pipelineManager` - Pipeline state caching

### VulkanBufferManager
Manages GPU buffers with a ring buffer strategy:
- `STAGING_BUFFER_SIZE` = 64MB ring buffer
- Per-frame partitioning to avoid stalls
- Automatic alignment handling

Global instance: `g_vulkanBufferManager`

### VulkanTextureManager
Handles texture creation and uploads:
- 16MB staging ring buffer for texture uploads
- Format conversion (24-bit RGB -> 32-bit RGBA on CPU)
- Compressed texture support (BC1/BC2/BC3/BC7)
- Multi-mip upload for precomputed mipmaps
- Mipmap generation via blit chain for uncompressed formats

Global instance: `g_vulkanTextureManager`

### VulkanTexture
Individual texture resource, inherits from `gr_bitmap_info` for bmpman integration:
- `vk::UniqueImage`, `vk::UniqueDeviceMemory`, `vk::UniqueImageView`
- Layout state tracking with `m_currentLayout`
- `notifyLayoutChanged()` for external layout transitions (render passes)

### VulkanSamplerCache
Caches samplers by configuration to avoid duplicates:
- Key: filter modes, address mode, anisotropy, mipmap enable
- FSO uses ~5 distinct sampler configurations

### VulkanShaderManager
Runtime GLSL to SPIR-V compilation:
- Uses shaderc library (requires `FSO_HAVE_SHADERC`)
- Caches compiled shaders by source hash + flags
- Handles #include resolution and macro injection

### VulkanDescriptorManager
Manages descriptor sets and layouts:
- Set 0: Global uniforms (view/projection matrices)
- Set 1: Texture bindings (4 combined image samplers)
- Pool-based allocation with automatic growth

### VulkanPipelineManager
Caches graphics pipelines by state hash:
- Blend state, depth state, vertex layout
- Shader combination
- Render pass compatibility

## Integration Points

### gr_screen Function Pointers
Vulkan implementations are wired via `gr_screen` in `code/graphics/2d.h`:

```cpp
// Texture management
gr_screen.gf_bm_create      = gr_vulkan_bm_create;
gr_screen.gf_bm_init        = gr_vulkan_bm_init;
gr_screen.gf_bm_data        = gr_vulkan_bm_data;
gr_screen.gf_bm_free_data   = gr_vulkan_bm_free_data;
```

### bmpman Integration
`VulkanTexture` inherits from `gr_bitmap_info`:
- Stored in `bitmap_slot::gr_info`
- Lifecycle managed by bmpman, destroyed via `gr_vulkan_bm_free_data`

## Synchronization Strategy

### Frame Resources
Each frame-in-flight has:
- Command buffer
- Semaphores (image available, render finished)
- Fence (CPU-GPU sync)

### Texture Uploads
- Dedicated upload command buffer with fence
- Reset command buffer before each upload batch
- Wait on fence before reusing staging memory

### Staging Buffer Management
Ring buffer with per-frame partitioning:
```cpp
// Partition: STAGING_BUFFER_SIZE / MAX_FRAMES_IN_FLIGHT per frame
// Each frame waits only on its own upload fence before reuse
```

## Build Configuration

CMake option: `FSO_BUILD_WITH_VULKAN=ON` (default ON except macOS)

Required:
- Vulkan SDK with vulkan.hpp headers
- shaderc library (for runtime shader compilation)
- SPIRV-Cross (for shader reflection)

## Known Limitations / TODOs

### Implemented
- Basic texture upload (uncompressed + compressed)
- Multi-mip upload for DDS/BC textures
- Mipmap generation for uncompressed formats
- Sampler caching
- Command buffer reset handling

### Not Yet Implemented
- `gr_vulkan_update_texture` - Partial texture updates
- `gr_vulkan_bm_make_render_target` - Render target creation
- `gr_vulkan_bm_set_render_target` - Render target binding
- `gr_vulkan_get_bitmap_from_texture` - GPU readback
- Per-frame staging buffer partitioning (optimization)
- Descriptor set binding for textures

## Common Patterns

### Layout Transitions
```cpp
texture->transitionLayout(cmd,
    vk::ImageLayout::eUndefined,
    vk::ImageLayout::eTransferDstOptimal);
// ... upload ...
texture->transitionLayout(cmd,
    vk::ImageLayout::eTransferDstOptimal,
    vk::ImageLayout::eShaderReadOnlyOptimal);
```

### Staging Memory Allocation
```cpp
vk::DeviceSize offset;
void* ptr = allocateStagingMemory(size, offset);
// offset is captured BEFORE alignment padding
std::memcpy(ptr, data, size);
// Use offset for buffer copy commands
```

### Compressed Format Detection
```cpp
bool isCompressed = isCompressedFormat(format);
// BC1/BC2/BC3/BC7 -> true
// Block size: BC1 = 8 bytes, BC2/BC3/BC7 = 16 bytes
```
