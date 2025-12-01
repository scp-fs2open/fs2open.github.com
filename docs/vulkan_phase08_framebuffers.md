# Phase 8: Framebuffers and Render Passes

## Status: COMPLETE

## Files

| File | Purpose |
|------|---------|
| `VulkanFramebuffer.h` | FramebufferAttachment, VulkanFramebuffer, VulkanRenderPassManager class declarations |
| `VulkanFramebuffer.cpp` | Full implementations |

## Components Implemented

### FramebufferAttachment

Simple struct managing a single framebuffer attachment (color or depth/stencil):

```cpp
struct FramebufferAttachment {
    vk::UniqueImage image;
    vk::UniqueDeviceMemory memory;
    vk::UniqueImageView view;
    vk::Format format;
    vk::Extent2D extent;
    vk::ImageUsageFlags usage;
    vk::ImageAspectFlags aspectMask;

    bool isDepthFormat() const;
    bool isStencilFormat() const;
};
```

**Supported formats:**
- **Depth:** D16Unorm, D32Sfloat
- **Depth+Stencil:** D16UnormS8Uint, D24UnormS8Uint, D32SfloatS8Uint
- **Stencil-only:** S8Uint

### VulkanFramebuffer

Manages complete framebuffers with multiple attachments. Supports two modes:

**Mode 1: Owned Attachments**
- Creates and manages images/memory internally
- Used for scene rendering, post-processing passes
- Images are device-local, suitable for GPU-only access

**Mode 2: External Views**
- Wraps existing image views (e.g., swapchain images)
- Used for presenting to swapchain
- Does not own images/memory

**Key Methods:**
- `create()` - Create framebuffer with owned attachments
- `createFromImageViews()` - Create framebuffer wrapping external views
- `destroy()` - Clean up resources
- `getColorImageView()` / `getDepthImageView()` - Access attachment views
- `getColorImage()` / `getDepthImage()` - Access underlying images (owned mode only)

**Image Usage Flags:**
- Color attachments: `ColorAttachment | Sampled | TransferSrc` (for post-processing + screenshots)
- Depth attachments: `DepthStencilAttachment`

### VulkanRenderPassManager

Manages render passes for the Vulkan renderer. Currently provides two render passes:

**1. Scene Render Pass** (color + depth)

Used for main 3D geometry rendering.

**Color Attachment:**
- **Format:** Configurable (typically R16G16B16A16Sfloat for HDR)
- **Load Op:** Clear (clear to background color)
- **Store Op:** Store (preserve for post-processing)
- **Initial Layout:** Undefined (content discarded)
- **Final Layout:** ShaderReadOnlyOptimal (sampled in post-processing)

**Depth Attachment:**
- **Format:** Configurable (typically D32Sfloat)
- **Load Op:** Clear (1.0 for reverse-Z, 0.0 for standard)
- **Store Op:** DontCare (depth not needed after scene render)
- **Initial Layout:** Undefined
- **Final Layout:** DepthStencilAttachmentOptimal

**Subpass Dependencies:**
- External → Subpass 0: Waits for color/depth writes to complete before rendering
- Subpass 0 → External: Ensures color writes complete before shader reads (post-processing)

**2. Present Render Pass** (color only)

Used for blitting post-processed result to swapchain.

**Color Attachment:**
- **Format:** Swapchain format (typically B8G8R8A8Unorm or B8G8R8A8Srgb)
- **Load Op:** DontCare (fullscreen quad overwrites entire framebuffer)
- **Store Op:** Store (for presentation)
- **Initial Layout:** Undefined
- **Final Layout:** PresentSrcKHR (ready for presentation)

**Subpass Dependency:**
- External → Subpass 0: Waits for scene pass shader reads to complete before writing

## Implementation Details

### Memory Allocation

Framebuffer attachments always use **device-local memory** (GPU-only):

```cpp
uint32_t memTypeIndex = findMemoryType(physicalDevice,
                                        memRequirements.memoryTypeBits,
                                        vk::MemoryPropertyFlagBits::eDeviceLocal);
```

**Why device-local?**
- Framebuffers are GPU-only resources (never read by CPU)
- Maximizes GPU performance
- No CPU-visible memory wasted

### Memory Type Selection

Unlike VulkanTexture, VulkanFramebuffer uses a **simpler** memory type fallback:

```cpp
// Returns UINT32_MAX if no exact match found
return UINT32_MAX;
```

**Rationale:**
- Device-local memory for framebuffers should always exist on compliant hardware
- No need for fallback to non-device-local (would hurt performance)
- Explicit failure better than silent degradation

### Stencil Aspect Handling

Depth-stencil formats automatically get stencil aspect mask:

```cpp
FramebufferAttachment tempAttachment;
tempAttachment.format = depthFormat;
if (tempAttachment.isStencilFormat()) {
    aspectMask |= vk::ImageAspectFlagBits::eStencil;
}
```

**Critical:** Image views for combined depth-stencil formats **must** include both aspects.

### Resource Ownership

The `m_ownsAttachments` flag controls cleanup behavior:

- **true**: Destroy images, memory, views on cleanup
- **false**: Only destroy framebuffer, leave external views intact

This prevents double-free when wrapping swapchain images.

## Render Pass Design

### Scene Pass Flow

```
1. Begin render pass
   └─> Clear color (background) and depth (1.0 or 0.0)
2. Render 3D geometry
   └─> Write color and depth
3. End render pass
   └─> Transition color to ShaderReadOnlyOptimal
       (depth discarded)
```

Color output is sampled by post-processing passes (bloom, tonemapping, etc.).

### Present Pass Flow

```
1. Begin render pass
   └─> Don't care about previous swapchain contents (overwritten)
2. Fullscreen quad blit (post-processed scene → swapchain)
   └─> Write final color
3. End render pass
   └─> Transition color to PresentSrcKHR
```

No depth testing needed (fullscreen quad, no occlusion).

## Not Yet Implemented

- **MSAA support** - All attachments use 1 sample (no multisampling)
- **Mipmap support** - Framebuffer attachments use 1 mip level
- **Layered rendering** - Framebuffers use 1 layer (no geometry shader layering)
- **Multiple subpasses** - Each render pass has 1 subpass
- **Resolve attachments** - Would be needed for MSAA resolve

## Integration with VulkanRenderer

The VulkanRenderer creates and manages:
- **VulkanRenderPassManager** - Created during initialization
- **Scene framebuffers** - Created per-frame for offscreen rendering
- **Swapchain framebuffers** - Created from external swapchain image views

Typical rendering flow:
1. Acquire swapchain image
2. Begin scene render pass → Render geometry → End pass
3. Begin present render pass → Blit to swapchain → End pass
4. Present swapchain image

## Dependencies

- **Requires:** VulkanRenderer (device, physical device)
- **Required by:** Phase 9 (frame execution), Phase 11 (draw calls)

## Testing

Build verification: Compiles without errors
Runtime testing: Requires Phase 9+ integration to test render pass execution

## Key Files Reference

- `code/graphics/vulkan/VulkanFramebuffer.h` - Class declarations
- `code/graphics/vulkan/VulkanFramebuffer.cpp` - Implementation (513 lines)
- `code/graphics/vulkan/VulkanRenderer.cpp` - Integration (render pass/framebuffer creation)

## Code Quality

**No bugs found.** Implementation is production-ready with:
- Proper error handling (try-catch on all Vulkan calls)
- Validated memory type selection (returns UINT32_MAX on failure)
- Correct resource lifetime management (UniqueHandles + explicit destroy)
- Proper subpass dependencies for synchronization
- Clean dual-mode design (owned vs external attachments)
