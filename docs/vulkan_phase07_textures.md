# Phase 7: Textures

## Status: IN PROGRESS (Code implemented but not integrated)

Texture classes and manager are implemented, but the Vulkan renderer still uses stub bitmap function pointers (`gr_screen.gf_bm_*`) so the texture path is not wired into rendering yet.

## Files Created

| File | Purpose |
|------|---------|
| `VulkanTexture.h` | Class declarations for VulkanTexture, VulkanSamplerCache, VulkanTextureManager |
| `VulkanTexture.cpp` | Full implementations |

## Components Implemented

### VulkanTexture
Inherits from `gr_bitmap_info` for bmpman integration.

```cpp
class VulkanTexture : public gr_bitmap_info {
    vk::UniqueImage m_image;
    vk::UniqueDeviceMemory m_memory;
    vk::UniqueImageView m_imageView;
    vk::ImageLayout m_currentLayout;
    // ...
};
```

**Key methods:**
- `create()` - Image + memory + view allocation with configurable usage flags
- `transitionLayout()` - Pipeline barrier helper with layout-specific access masks
- `uploadData()` - Copy from staging buffer to specific mip level
- `generateMipmaps()` - Blit chain for uncompressed formats
- `notifyLayoutChanged()` - External layout sync (for render pass transitions)

### VulkanSamplerCache
Caches samplers by configuration to avoid duplicates.

```cpp
vk::Sampler getSampler(vk::Filter minFilter, vk::Filter magFilter,
                       vk::SamplerAddressMode addressMode,
                       float anisotropy, bool enableMipmaps);
```

Cache key: filter modes + address mode + anisotropy + mipmap enable.

### VulkanTextureManager
Handles staging buffer and bmpman integration.

**Features:**
- 16MB staging ring buffer
- Format conversion (24-bit RGB -> 32-bit RGBA on CPU)
- Palette expansion for 8bpp palettized textures (RGB palette -> RGBA)
- Compressed format support (BC1/BC2/BC3/BC7)
- Multi-mip upload for precomputed mipmaps (DDS)
- Mipmap generation via blit chain for uncompressed

**Global instance:** `g_vulkanTextureManager`

## gr_screen Function Implementations

| Function | Status | Notes |
|----------|--------|-------|
| `gr_vulkan_bm_create` | Implemented | Allocates VulkanTexture |
| `gr_vulkan_bm_init` | Implemented | Initializes texture slot |
| `gr_vulkan_bm_data` | Implemented | Uploads pixel data |
| `gr_vulkan_bm_free_data` | Implemented | Destroys texture |
| `gr_vulkan_update_texture` | Stub | Partial updates not yet needed |
| `gr_vulkan_bm_make_render_target` | Stub | Phase 10 |
| `gr_vulkan_bm_set_render_target` | Stub | Phase 10 |
| `gr_vulkan_get_bitmap_from_texture` | Stub | GPU readback |
| `gr_vulkan_set_texture_addressing` | Stub | Mode ignored (needs bind-time integration) |
| `gr_vulkan_bm_generate_mip_maps` | Stub | On-demand generation |

## Format Mapping

### Uncompressed
| bpp | Flags | Vulkan Format | Notes |
|-----|-------|---------------|-------|
| 32 | - | eR8G8B8A8Unorm | Direct upload |
| 24 | - | eR8G8B8A8Unorm | CPU converts RGB->RGBA |
| 16 | - | eR5G6B5UnormPack16 | Direct upload |
| 8 | BMP_AABITMAP | eR8Unorm | Single-channel alpha, direct upload |
| 8 | - | eR8G8B8A8Unorm | Palette expanded to RGBA on upload |

### Compressed (DDS)
| Flag | Vulkan Format |
|------|---------------|
| DDS_DXT1 | eBc1RgbaUnormBlock |
| DDS_DXT3 | eBc2UnormBlock |
| DDS_DXT5 | eBc3UnormBlock |
| DDS_BC7 | eBc7UnormBlock |

## Critical Implementation Details

### Staging Memory Allocation
Offset captured BEFORE alignment to ensure correct buffer copy; wrap check includes alignment padding:
```cpp
void* allocateStagingMemory(vk::DeviceSize size, vk::DeviceSize& outOffset) {
    const vk::DeviceSize alignedNext = (m_stagingOffset + size + 3) & ~vk::DeviceSize(3);
    if (alignedNext > STAGING_BUFFER_SIZE) {
        device.waitForFences(uploadFence, true, UINT64_MAX);
        m_stagingOffset = 0;
    }
    outOffset = m_stagingOffset;
    void* ptr = mappedMemory + m_stagingOffset;
    m_stagingOffset += size;
    m_stagingOffset = (m_stagingOffset + 3) & ~vk::DeviceSize(3);
    return ptr;
}
```

### Multi-Mip Upload (Compressed)
For DDS with precomputed mips, all levels uploaded sequentially:
```cpp
if (isCompressed && textureMipLevels > 1) {
    for (uint32_t mip = 0; mip < textureMipLevels; mip++) {
        size_t mipSize = calculateMipSize(mipWidth, mipHeight, format);
        texture->uploadData(cmd, stagingBuffer, mipOffset, mipSize, mip, 0);
        mipOffset += mipSize;
        mipWidth = std::max(1u, mipWidth >> 1);
        mipHeight = std::max(1u, mipHeight >> 1);
    }
}
```

### Command Buffer Reset
Pool created with `eResetCommandBuffer` flag, buffer reset before each recording:
```cpp
m_uploadCommandBuffer->reset(vk::CommandBufferResetFlags{});
m_uploadCommandBuffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
```

### ptr_u Type Handling
`bm->data` is `ptr_u` (size_t), not a pointer:
```cpp
std::memcpy(stagingPtr, reinterpret_cast<const void*>(bm->data), dataSize);
```

## Bug Fixes Applied

1. Missing `<cmath>` include - Added for `std::log2`/`std::floor`
2. stagingOffset calculation - Capture offset before alignment padding
3. Multi-mip upload - Iterate all precomputed mip levels for compressed formats
4. Command buffer reset - Reset before re-recording, pool flag added
5. Staging buffer wrap check accounts for alignment padding (prevents overflow)
6. Palette expansion for 8bpp palettized textures (convert to RGBA)
7. Texture memory type selection now fails cleanly if no suitable type exists

## Known Gaps

- Texture addressing mode handling is stubbed (`gr_vulkan_set_texture_addressing` does nothing). Needs descriptor/sampler integration during Phase 9.
- Texture path is not wired into `gr_screen`; Vulkan still uses stub bitmap hooks.

## Not Yet Implemented

- Per-frame staging buffer partitioning (optimization)
- Descriptor set binding for textures (Phase 9 integration)
- Render targets (Phase 10)
- GPU readback

## Dependencies

- **Requires:** VulkanRenderer (command pool), VulkanBuffer (staging pattern reference)
- **Required by:** Phase 9 (draw calls need texture binding)

## Testing

Build verification: Compiles without errors
Unit tests: All 237 tests pass

## Key Files Reference

- `code/graphics/vulkan/VulkanTexture.h` - Class declarations
- `code/graphics/vulkan/VulkanTexture.cpp` - Implementation
- `code/graphics/vulkan/VulkanRenderer.cpp:949-956` - Command pool flags
- `code/bmpman/bm_internal.h` - gr_bitmap_info base class
- `code/graphics/2d.h` - gr_screen function pointer declarations
