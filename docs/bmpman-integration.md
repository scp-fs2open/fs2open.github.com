# Bitmap Manager (bmpman) Integration

## Overview

The bitmap manager (`code/bmpman/`) is FSO's central texture management system. Graphics backends (OpenGL, Vulkan) integrate via function pointers and the `gr_bitmap_info` base class.

## Key Data Structures

### bitmap (code/bmpman/bmpman.h)
```cpp
struct bitmap {
    int w, h;           // Dimensions
    int bpp;            // Bits per pixel (8, 16, 24, 32)
    int flags;          // BMP_FLAG_* flags
    ptr_u data;         // Pixel data (note: ptr_u is size_t, cast to void*)
    // ...
};
```

### bitmap_entry (code/bmpman/bm_internal.h)
```cpp
struct bitmap_entry {
    bitmap bm;              // The bitmap data
    int num_mipmaps;        // Mipmap count (DDS files)
    size_t mem_taken;       // Total memory size (all mips)
    // ...
};
```

### bitmap_slot (code/bmpman/bm_internal.h)
```cpp
struct bitmap_slot {
    bitmap_entry entry;
    gr_bitmap_info* gr_info;  // Graphics backend data (VulkanTexture*)
    // ...
};
```

### gr_bitmap_info (code/bmpman/bm_internal.h)
Base class for graphics backend texture data:
```cpp
class gr_bitmap_info {
public:
    virtual ~gr_bitmap_info() = default;
};
```

## gr_screen Function Pointers

Located in `code/graphics/2d.h`, set during graphics init:

| Function | Purpose | Vulkan Implementation |
|----------|---------|----------------------|
| `gf_bm_create` | Allocate backend resources | `gr_vulkan_bm_create` |
| `gf_bm_init` | Initialize texture | `gr_vulkan_bm_init` |
| `gf_bm_data` | Upload pixel data | `gr_vulkan_bm_data` |
| `gf_bm_free_data` | Release backend resources | `gr_vulkan_bm_free_data` |
| `gf_update_texture` | Partial texture update | `gr_vulkan_update_texture` (stub) |
| `gf_bm_make_render_target` | Create render target | `gr_vulkan_bm_make_render_target` (stub) |
| `gf_bm_set_render_target` | Bind render target | `gr_vulkan_bm_set_render_target` (stub) |

## Texture Lifecycle

### 1. Creation
```
bm_load() / bm_create()
  -> allocates bitmap_slot
  -> calls gr_screen.gf_bm_create(handle)
     -> VulkanTextureManager::createTexture()
        -> new VulkanTexture()
        -> slot->gr_info = texture
```

### 2. Data Upload
```
bm_lock() - locks bitmap, loads pixel data into bm->data
  -> calls gr_screen.gf_bm_data(handle, &bm)
     -> VulkanTextureManager::uploadTextureData()
        -> allocate staging memory
        -> copy/convert data
        -> record upload commands
        -> transition layout
bm_unlock() - releases lock
```

### 3. Destruction
```
bm_release() / bm_unload()
  -> calls gr_screen.gf_bm_free_data(handle, release)
     -> VulkanTextureManager::destroyTexture()
        -> remove from tracking map
     -> delete slot->gr_info (VulkanTexture destructor)
```

## Format Mapping

### Uncompressed Formats
| bm->bpp | bm->flags | Vulkan Format |
|---------|-----------|---------------|
| 32 | - | eR8G8B8A8Unorm |
| 24 | - | eR8G8B8A8Unorm (CPU convert) |
| 16 | - | eR5G6B5UnormPack16 |
| 8 | BMP_AABITMAP | eR8Unorm |
| 8 | - | eR8G8B8A8Unorm |

### Compressed Formats (DDS)
| bitmap_entry flags | Vulkan Format |
|-------------------|---------------|
| DDS_DXT1 | eBc1RgbaUnormBlock |
| DDS_DXT3 | eBc2UnormBlock |
| DDS_DXT5 | eBc3UnormBlock |
| DDS_BC7 | eBc7UnormBlock |

## Mipmap Handling

### Precomputed Mipmaps (DDS)
- `entry->num_mipmaps` contains count
- All mip data stored sequentially in `bm->data`
- Size per mip: `((h+3)/4) * ((w+3)/4) * blockSize`
- Upload all levels, do NOT generate

### Generated Mipmaps (Uncompressed)
- Calculate levels: `floor(log2(max(w,h))) + 1`
- Upload base level only
- Generate via `vkCmdBlitImage` chain
- Only for formats supporting linear filter blit

## Helper Functions

```cpp
// Get bitmap entry
bitmap_entry* bm_get_entry(int handle);

// Get bitmap slot
bitmap_slot* bm_get_slot(int handle);

// Get mipmap count
int bm_get_num_mipmaps(int handle);

// Check if compressed
bool bm_is_compressed(int handle);

// Get graphics backend data
template<typename T>
T* bm_get_gr_info(int handle, bool create = false);
```

## Important Notes

### ptr_u Type
`bm->data` is `ptr_u` (typedef for `size_t`), not a pointer:
```cpp
// WRONG
memcpy(dst, bm->data, size);

// CORRECT
memcpy(dst, reinterpret_cast<const void*>(bm->data), size);
```

### Ownership
- `VulkanTexture*` stored in `slot->gr_info`
- Owned by bitmap_slot, deleted when slot is freed
- `VulkanTextureManager::m_textures` only tracks, does not own

### Thread Safety
- bmpman operations are NOT thread-safe
- Texture uploads should happen on main thread
- Lock bitmap before accessing `bm->data`
