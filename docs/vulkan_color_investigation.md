# Vulkan Color Pipeline Investigation

## Problem
Colors are displayed incorrectly in the Vulkan renderer - R and B channels appear swapped, resulting in red/olive tones instead of blue/gray.

## Key Findings

### 1. Texture Data Format (bmpman)
**Source:** `code/graphics/opengl/gropengltexture.cpp` lines 388-414

OpenGL uses: `GL_BGRA + GL_UNSIGNED_INT_8_8_8_8_REV`

On little-endian x86, this means bmpman stores pixels as:
- **byte[0] = B, byte[1] = G, byte[2] = R, byte[3] = A** (BGRA in memory)

### 2. Vertex Color Format
**Source:** `code/graphics/opengl/gropengltnl.cpp` line 79

```cpp
{ vertex_format_data::COLOR4, 4, GL_UNSIGNED_BYTE, GL_TRUE, opengl_vert_attrib::COLOR }
```

This is plain `GL_UNSIGNED_BYTE` with 4 components - NO special format like GL_BGRA.
Vertex colors are stored as **RGBA in memory**.

### 3. Color Struct
**Source:** `code/graphics/2d.h` lines 421-431

```cpp
typedef struct color {
    int is_alphacolor;
    int alphacolor;
    int magic;
    ubyte red;    // offset after ints
    ubyte green;
    ubyte blue;
    ubyte alpha;
    ...
} color;
```

When copying to vertex buffer: `v[0].r = clr.red; v[0].g = clr.green; v[0].b = clr.blue;`
This confirms **RGBA order** for vertex colors.

### 4. Current Vulkan Configuration

| Component | Format | Memory Layout | Status |
|-----------|--------|---------------|--------|
| Textures | eB8G8R8A8Unorm | BGRA | CORRECT |
| Vertex colors | eR8G8B8A8Unorm | RGBA | CORRECT |
| Scene framebuffer | eR16G16B16A16Sfloat | RGBA float | OK |
| Swapchain | eB8G8R8A8Unorm (SDR) | BGRA | OK |
| Blit shader | Pass-through (no swap) | - | SHOULD BE CORRECT |

### 5. Current Test Results
- **3D ship model**: GREEN (CORRECT) - uses vertex colors
- **UI text**: RED (WRONG) - should be green/white

### 6. Theories for Remaining Issue

**Theory A: Font textures are different**
Font glyphs might be stored differently than regular textures. Need to check font texture loading path.

**Theory B: UI uses different color path**
The UI/SCPUI system might pass colors through a different mechanism (uniforms vs vertex colors).

**Theory C: Compressed textures (DXT/BC)**
Compressed textures use different formats and might have different byte ordering.

## Files Modified So Far

1. **VulkanTexture.cpp** - Texture format set to BGRA (eB8G8R8A8Unorm)
2. **VulkanRenderer.cpp** - HDR forced OFF, swapchain prefers non-sRGB BGRA
3. **vulkan_blit.frag** - Removed .bgra swap, now pass-through

## Files to Investigate

1. `code/graphics/font.cpp` - Font texture loading
2. `code/scpui/` - SCPUI rendering system
3. `code/graphics/uniforms.cpp` - Color uniforms
4. `code/graphics/material.cpp` - Material colors

## Recent Findings

- **Palettized uploads now explicitly reorder palette colors to BGRA when writing into the Vulkan staging buffer**, and 24-bit bmpman data is interpreted as BGR so the conversion to the `vk::Format::eB8G8R8A8Unorm` texture avoids accidental R/B swaps. These changes still leave UI palettes rendering red/olive, so the issue is likely elsewhere.
- **Menus/loading screens follow the `material_set_interface` path**, ultimately using the `default-material` shader and its `genericData` uniform block (binding 8) that the Vulkan backend populates in `gr_vulkan.cpp`. Because that block simply forwards `material::get_color()` (typically white) and `mat->get_color_scale()` (default `1.0f`), the uniform data itself shouldn’t be producing a tint.
- **Runtime log (`fs2_open.log`) shows Vulkan shader/descriptor issues**—notably the NanoVG shader fails to compile because it lacks explicit `layout(binding=...)` and `layout(location=...)` declarations, and descriptor sets (`UniformBuffers`) are reported as leaked at shutdown. These errors may hide pipeline faults but the red UI persist even when material/textures appear correct.

## Key Code Locations

- Texture format selection: `VulkanTexture.cpp:937-953`
- Vertex color format: `VulkanPipelineManager.cpp:1136-1138`
- Swapchain format: `VulkanRenderer.cpp:245-263`
- Blit shader: `code/graphics/shaders/vulkan_blit.frag`

## What Works
- 3D model rendering with vertex colors (green ship is correct)

## What Doesn't Work
- UI text colors (showing red instead of green/white)
- Menu background textures (wrong colors)
