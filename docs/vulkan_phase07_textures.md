# Phase 7: Textures

**Status**: IN PROGRESS (texture manager implemented and hooked into `gr_screen`, but descriptor/sampler binding and partial updates still pending)

## VulkanTexture + VulkanSamplerCache + VulkanTextureManager

### VulkanTexture
- Inherits from `gr_bitmap_info` so it can live inside `bitmap_slot::gr_info`.
- `create()` builds the `vk::Image`, allocates device-local memory, and creates an `ImageView` for sampling.
- `transitionLayout()` implements the canonical barriers between `Undefined`, `TransferDstOptimal`, `ShaderReadOnlyOptimal`, and `ColorAttachmentOptimal`.
- `uploadData()` and `generateMipmaps()` cover full mip chains and layout transitions, so textures are left in `ShaderReadOnlyOptimal`.

### VulkanSamplerCache
- Caches `vk::Sampler` objects keyed by min/mag filter, addressing mode, anisotropy, and mipmap usage.
- `getSampler()` lazily creates the sampler with the requested options and respects `maxSamplerAnisotropy` from the physical device.
- Reuses `vk::UniqueSampler` handles so repeated descriptor updates don’t re-create samplers.

### VulkanTextureManager
- Ring-buffer staging area: 16 MB buffer partitioned by `FRAMES_IN_FLIGHT=2`. Each frame can upload without waiting because partitions are advanced in `beginFrame()`.
- Per-frame upload command buffers (`m_uploadCmds[]`) record non-blocking copies; `ensureUploadRecording()`/`submitUploads()` manage begin/end/submit.
- Format selection (`selectFormat`) handles uncompressed (RGB→RGBA conversion, 8‑bit palette expansion) and compressed formats (DXT1/3/5, BC7).
- `uploadTextureData()` stages palette conversion or compressed mip chains, transitions the texture to `vk::ImageLayout::eTransferDstOptimal`, copies from staging, and either generates mipmaps or transitions to `ShaderReadOnlyOptimal`.
- `allocateStagingMemory()` enforces partition bounds and alignment so CPU writes never clash with GPU reads.
- `readbackTexture()` can copy GPU-backed textures back to CPU memory via temporary staging buffers when needed.

## Render targets & readback
- `createRenderTarget()` builds `VulkanRenderTarget` objects that own HDR color attachments and optional depth. It registers per-handle framebuffers (including cubemap faces) that can be bound like FBOs.
- `setRenderTarget()` switches the active render target, records the active handle, and handles cubemap face validation.
- `destroyRenderTarget()` tears down the framebuffer while leaving the texture owned by bmpman.
- `readbackTexture()` uses `VulkanTexture`’s `notifyLayoutChanged()` to keep layout tracking accurate after staging copies.

## BMP integration
- `vulkan_stubs.cpp` wires `gr_screen.gf_bm_*` to the Vulkan implementations (`gr_vulkan_bm_create/init/data/free`, `gr_vulkan_bm_make_render_target`, `gr_vulkan_bm_set_render_target`).
- `gr_vulkan_bm_data()` forwards uploads to `VulkanTextureManager::uploadTextureData()`. The real texture cache is therefore hot-swappable with bmpman slots.
- `VulkanRenderer::initialize()` instantiates `VulkanTextureManager` and exposes it through `g_vulkanTextureManager`.
- `gr_vulkan_update_texture()` and `gr_vulkan_set_texture_addressing()` currently log TODOs; they will be wired into descriptor updates once Phase 11 does full descriptor binding.

## Outstanding work
- Partial texture updates (`gr_vulkan_update_texture`) need fast buffer copies; the staging/upload path exists but must be split per-region.
- Texture addressing and sampler state must flow into descriptor set updates when binding materials (Phase 11).
- Render targets can be created/activated, but the draw path still needs to respect `VulkanTextureManager::getActiveRenderTarget()`.
- GPU readback exists but is only used when explicit `gr_get_bitmap_from_texture()` calls occur; automatic capture helpers would be a future step.

## Key files
- `code/graphics/vulkan/VulkanTexture.{h,cpp}` – texture/sampler manager implementation
- `code/graphics/vulkan/VulkanRenderer.cpp` – texture manager initialization and `beginFrame()` hooks
- `code/graphics/vulkan/vulkan_stubs.cpp` – `gr_screen` bitmap function wiring
