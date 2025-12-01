# Fix Vulkan Texture Implementation Stubs

## Issues Found

1. `gr_vulkan_update_texture()` - completely stubbed (partial texture updates)
2. `gr_vulkan_set_texture_addressing()` - empty implementation (addressing mode ignored)
3. Render target mipmap generation - TODO when unbinding RTs
4. Cubemap creation - unused imageInfo and missing flags support
5. `VulkanTexture::create()` - missing cubemap flag and view type support

## Detailed Code Path Analysis

### Issue 1: gr_vulkan_update_texture() - Partial Texture Updates

**Call Path:**
- Called from `generic.cpp:380,468,512` during animated graphics updates
- Called from `NanoVGRenderer.cpp:514` for nanovg texture updates
- Function signature: `void gr_vulkan_update_texture(int handle, int bpp, const ubyte* data, int width, int height)`

**OpenGL Reference (gropengltexture.cpp:1489-1556):**
- Gets texture slot via `bm_get_gr_info<tcache_slot_opengl>(bitmap_handle)`
- Uses `t->array_index` for array layer (Vulkan doesn't track this yet - assume layer 0)
- Handles format conversion: BGRA for 4bpp, BGR for 3bpp, converts 1bpp to grayscale
- Uses `glTexSubImage3D(target, mip=0, x=0, y=0, z=array_index, width, height, depth=1, format, type, data)`
- Updates mip level 0 only, at offset (0,0) in the texture

**Vulkan Implementation Logic:**
1. Get texture: `VulkanTexture* texture = g_vulkanTextureManager->getTexture(handle)`
2. Validate: Check texture exists and is valid
3. Get texture format: `vk::Format format = texture->getFormat()`
4. Calculate data size: Use `calculateMipSize(width, height, format)` - but note: width/height are update region size, not texture size
5. Format conversion (CPU-side, before staging):
   - If texture is RGBA8 and input is 24bpp: convert RGB->RGBA (add alpha=255)
   - If texture is RGBA8 and input is 8bpp: convert to grayscale RGBA (like OpenGL luminance calc)
   - Otherwise: direct copy if formats match
6. Allocate staging memory: `allocateStagingMemory(dataSize, stagingOffset)` - may fail if partition full
7. Copy converted data to staging buffer
8. Get upload command buffer: `vk::CommandBuffer cmd = getUploadCommandBuffer()`
9. Transition layout:
   - Current layout: `texture->getCurrentLayout()` (likely eShaderReadOnlyOptimal)
   - Transition to: eTransferDstOptimal for copy
10. Record copy command:
    - `vk::BufferImageCopy region` with:
      - `bufferOffset = stagingOffset`
      - `imageSubresource.mipLevel = 0`
      - `imageSubresource.baseArrayLayer = 0` (Vulkan doesn't use arrays yet)
      - `imageSubresource.layerCount = 1`
      - `imageOffset = {0, 0, 0}` (always update from top-left)
      - `imageExtent = {width, height, 1}`
    - `cmd.copyBufferToImage(stagingBuffer, texture->getImage(), eTransferDstOptimal, region)`
11. Transition back: eTransferDstOptimal -> eShaderReadOnlyOptimal
12. Submit: Commands batched, submitted later via `submitUploads()` (called by renderer before frame submit)

**Edge Cases:**
- Staging partition full: Return early, log warning (caller should retry next frame)
- Texture not found: Return early (silent fail like OpenGL)
- Format mismatch: Convert on CPU (24bpp->32bpp, 8bpp->grayscale)
- Compressed textures: Not supported for partial updates (return early)

### Issue 2: gr_vulkan_set_texture_addressing() - Store Addressing Mode

**Call Path:**
- Called from `gropengldeferred.cpp:785,789` for deferred lighting
- Called from `modelrender.cpp:2594,2596` for model rendering
- Function signature: `void gr_vulkan_set_texture_addressing(int mode)`
- Constants: `TMAP_ADDRESS_WRAP=1, TMAP_ADDRESS_MIRROR=2, TMAP_ADDRESS_CLAMP=3` (from tmapper.h:95-97)

**Current Usage:**
- Material-level addressing used in `gr_vulkan.cpp:267` via `mat->get_texture_addressing()`
- Global mode may be used as fallback or for non-material textures
- Sampler selection happens in `gr_vulkan.cpp:274` via `getSamplerCache().getSampler()`

**Implementation Logic:**
1. Add static member to VulkanTextureManager:
   ```cpp
   static int s_globalTextureAddressing = TMAP_ADDRESS_WRAP; // Default wrap
   ```

2. Store mode in `gr_vulkan_set_texture_addressing()`:
   ```cpp
   void gr_vulkan_set_texture_addressing(int mode) {
       if (g_vulkanTextureManager) {
           VulkanTextureManager::s_globalTextureAddressing = mode;
       }
   }
   ```

3. Add getter to VulkanTextureManager:
   ```cpp
   static int getGlobalTextureAddressing() { return s_globalTextureAddressing; }
   ```

4. Future use: When binding textures without material, use global mode to select sampler

**Note:** This is a minimal implementation - full integration requires updating sampler selection logic to use global mode when material doesn't provide addressing.

### Issue 3: Render Target Mipmap Generation

**Call Path:**
- `setRenderTarget()` called from `gr_vulkan_bm_set_render_target()`
- When `handle < 0`: unbinding RT, restoring scene framebuffer
- Current code at `VulkanTexture.cpp:1348-1360` has TODO at line 1353

**Timing Analysis:**
- `setRenderTarget(-1)` called after rendering to RT completes
- At this point, RT texture is in eColorAttachmentOptimal layout (from render pass)
- Need to generate mipmaps before texture can be sampled
- Command buffer availability: `getUploadCommandBuffer()` is always available (creates if needed)

**Implementation Logic:**

When unbinding RT (handle < 0):
```cpp
if (m_activeRenderTarget != nullptr) {
    VulkanTexture* tex = getTexture(m_activeRenderTargetHandle);
    if (tex && tex->getMipLevels() > 1) {
        vk::CommandBuffer cmd = getUploadCommandBuffer();

        // Transition from color attachment to transfer dst
        tex->transitionLayout(cmd,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::eTransferDstOptimal);

        // Generate mipmaps (handles all transitions internally)
        tex->generateMipmaps(cmd);

        // Note: generateMipmaps() transitions final mip to eShaderReadOnlyOptimal
        // No need to submit here - batched with other uploads
    }
}
```

- Submit happens later: `submitUploads()` called by renderer before frame submit
- Synchronization: Upload fence ensures GPU completes before staging buffer reuse

**Layout Transitions:**
- `generateMipmaps()` expects base mip in eTransferDstOptimal
- It transitions mips: TRANSFER_DST -> TRANSFER_SRC -> SHADER_READ for each level
- Final mip stays in eShaderReadOnlyOptimal (correct for sampling)

### Issue 4 & 5: Cubemap Creation and VulkanTexture::create() Enhancement

**Call Path:**
- `createRenderTarget()` called from `gr_vulkan_bm_make_render_target()`
- Cubemap detected via `flags & BMP_FLAG_CUBEMAP`
- Current code creates unused imageInfo at lines 1241-1252, then calls `texture->create()` which doesn't accept flags

**Vulkan Cubemap Requirements:**
- Image must be created with `VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT` flag
- Image must have `arrayLayers = 6`
- Image view type: `VK_IMAGE_VIEW_TYPE_CUBE` for full cubemap, `VK_IMAGE_VIEW_TYPE_2D` for per-face views
- Per-face views already created correctly at lines 1287-1316 (for framebuffers)
- Main texture view (in VulkanTexture) should be `eCube` type for sampling

**Implementation Logic:**

**Step 1:** Update `VulkanTexture::create()` signature:
```cpp
// In VulkanTexture.h
bool create(vk::Device device, vk::PhysicalDevice physicalDevice,
            uint32_t width, uint32_t height, vk::Format format,
            uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
            vk::ImageUsageFlags usage = ...,
            vk::ImageCreateFlags flags = {}); // NEW PARAMETER
```

**Step 2:** Use flags in image creation:
```cpp
// In VulkanTexture.cpp:42-52
vk::ImageCreateInfo imageInfo{};
// ... existing fields ...
imageInfo.flags = flags; // NEW LINE at line 52
```

**Step 3:** Fix image view type selection:
```cpp
// In VulkanTexture.cpp:87
if (arrayLayers == 6 && (flags & vk::ImageCreateFlagBits::eCubeCompatible)) {
    viewInfo.viewType = vk::ImageViewType::eCube;
} else if (arrayLayers > 1) {
    viewInfo.viewType = vk::ImageViewType::e2DArray;
} else {
    viewInfo.viewType = vk::ImageViewType::e2D;
}
```

**Step 4:** Fix `createRenderTarget()`:
- Remove unused imageInfo creation (lines 1241-1252)
- Create flags: `vk::ImageCreateFlags imageFlags = isCubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags{};`
- Pass flags to `texture->create()`: `texture->create(..., imageFlags)`

**Step 5:** Update all `create()` call sites:
- `VulkanTexture.cpp:563`: `createTexture()` - pass `{}` (default, no flags)
- `VulkanTexture.cpp:1254`: `createRenderTarget()` - pass `imageFlags` (cubemap or empty)

## Files to Modify

**code/graphics/vulkan/VulkanTexture.h:**
- Add `vk::ImageCreateFlags flags = {}` parameter to `VulkanTexture::create()`
- Add `static int s_globalTextureAddressing` to VulkanTextureManager
- Add `static int getGlobalTextureAddressing()` getter

**code/graphics/vulkan/VulkanTexture.cpp:**
- Implement `gr_vulkan_update_texture()` (full implementation with format conversion)
- Implement `gr_vulkan_set_texture_addressing()` (store global mode)
- Fix `setRenderTarget()` mipmap generation (use `getUploadCommandBuffer()`)
- Fix `createRenderTarget()` (remove unused code, pass flags)
- Update `VulkanTexture::create()` (accept flags, set in imageInfo, fix view type)
- Update `createTexture()` call site (pass `{}` for flags)

## Testing Considerations

1. **Partial texture updates:** Test animated graphics (generic.cpp calls), verify frame updates correctly
2. **Addressing modes:** Test wrap/clamp/mirror (may require sampler selection updates)
3. **RT mipmaps:** Render to RT with mipmaps, unbind, verify mipmaps generated (check via readback)
4. **Cubemaps:** Create cubemap RT, render to faces, sample cubemap texture, verify correct faces
