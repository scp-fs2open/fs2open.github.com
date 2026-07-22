#pragma once

#include "globalincs/pstypes.h"
#include "VulkanMemory.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

#include <array>
#include <vulkan/vulkan.hpp>


namespace graphics::vulkan {

/**
 * @brief Vulkan-specific texture data stored in bitmap slots
 *
 * Extends gr_bitmap_info to store Vulkan image handles and metadata.
 * This is the Vulkan equivalent of tcache_slot_opengl.
 */
class tcache_slot_vulkan : public gr_bitmap_info {
public:
	vk::Image image;
	vk::ImageView imageView;
	VulkanAllocation allocation;
	vk::Format format = vk::Format::eUndefined;
	vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t mipLevels = 1;
	uint32_t arrayLayers = 1;
	int bpp = 0;

	int bitmapHandle = -1;
	uint32_t arrayIndex = 0;
	bool used = false;

	// For render targets
	vk::Framebuffer framebuffer;
	vk::ImageView framebufferView;  // Single-mip view for framebuffer (when mipLevels > 1)
	vk::RenderPass renderPass;  // Render pass compatible with this target
	bool isRenderTarget = false;

	// Optional depth/stencil attachment (BMP_FLAG_RENDER_TARGET_DEPTH_ATTACHMENT).
	// Without this, models drawn into the target get no z-testing: the Vulkan spec
	// ignores the pipeline's depth/stencil state when the subpass has no depth
	// attachment, so depth-tested geometry renders in submission order.
	vk::Image depthImage;
	vk::ImageView depthImageView;
	VulkanAllocation depthAllocation;

	// 3D texture support
	bool is3D = false;
	uint32_t depth = 1;

	// Cubemap support
	bool isCubemap = false;
	std::array<vk::ImageView, 6> cubeFaceViews = {};  // Per-face 2D views for render-to-cubemap
	std::array<vk::Framebuffer, 6> cubeFaceFramebuffers = {};  // Per-face framebuffers for render-to-cubemap
	// Note: the cube-sampling view (viewType=eCube, layerCount=6) lives in imageView
	// for both the render-target and static-file cubemap paths — there is no separate
	// cube view member.

	// Texture scaling (for non-power-of-two handling)
	float uScale = 1.0f;
	float vScale = 1.0f;

	tcache_slot_vulkan() { reset(); }
	~tcache_slot_vulkan() override = default;

	void reset();
};

/**
 * @brief Manages Vulkan textures, samplers, and render targets
 */
class VulkanTextureManager {
public:
	VulkanTextureManager();
	~VulkanTextureManager();

	// Non-copyable
	VulkanTextureManager(const VulkanTextureManager&) = delete;
	VulkanTextureManager& operator=(const VulkanTextureManager&) = delete;

	/**
	 * @brief Initialize the texture manager
	 */
	bool init(vk::Device device, vk::PhysicalDevice physicalDevice,
	          VulkanMemoryManager* memoryManager,
	          vk::CommandPool commandPool, vk::Queue graphicsQueue);

	/**
	 * @brief Shutdown and free all textures
	 */
	void shutdown();

	/**
	 * @brief Flush all GPU texture resources from bitmap slots
	 *
	 * Called between missions (from vulkan_bm_page_in_start) to release
	 * VkImage/VMA allocations for textures that won't be needed.
	 * Textures are re-uploaded on demand when accessed again.
	 */
	void flushTextures() const;

	// Bitmap management functions (implement gr_screen function pointers)

	/**
	 * @brief Initialize a bitmap slot for Vulkan
	 */
	void bm_init(bitmap_slot* slot) const;

	/**
	 * @brief Create Vulkan resources for a bitmap slot
	 */
	void bm_create(bitmap_slot* slot) const;

	/**
	 * @brief Free Vulkan resources for a bitmap slot
	 */
	void bm_free_data(bitmap_slot* slot, bool release) const;

	/**
	 * @brief Upload bitmap data to GPU
	 * @param compType Compression type (DDS_DXT1/3/5, DDS_BC7) or 0 for uncompressed
	 */
	bool bm_data(int handle, bitmap* bm, int compType);

	/**
	 * @brief Create a render target
	 */
	int bm_make_render_target(int handle, int* width, int* height, int* bpp, int* mm_lvl, int flags);

	/**
	 * @brief Set active render target
	 */
	int bm_set_render_target(int handle, int face);

	/**
	 * @brief Update texture data
	 */
	void update_texture(int bitmap_handle, int bpp, const ubyte* data, int width, int height);

	/**
	 * @brief Read texture data back to CPU
	 *
	 * Mirrors gr_opengl_get_bitmap_from_texture(): returns a newly
	 * vm_malloc()'d buffer of tightly packed RGB/RGBA8 pixels (caller frees
	 * with vm_free), sized from the texture's actual GPU dimensions written
	 * to width_out/height_out. Returns nullptr on failure.
	 */
	ubyte* get_bitmap_from_texture(int bitmap_num, int* width_out, int* height_out);

	// Sampler management

	/**
	 * @brief Get or create a sampler with specified parameters
	 */
	vk::Sampler getSampler(vk::Filter magFilter, vk::Filter minFilter,
	                       vk::SamplerAddressMode addressMode,
	                       bool enableAnisotropy, float maxAnisotropy,
	                       bool enableMipmaps);

	/**
	 * @brief Get a ready-to-use DescriptorImageInfo for the 2D fallback texture
	 */
	vk::DescriptorImageInfo getFallbackTextureInfo2D();

	/**
	 * @brief Get a ready-to-use DescriptorImageInfo for the cubemap fallback texture
	 */
	vk::DescriptorImageInfo getFallbackTextureInfoCube();

	/**
	 * @brief Get a ready-to-use DescriptorImageInfo for the 2D array fallback texture
	 */
	vk::DescriptorImageInfo getFallbackTextureInfo2DArray();

	/**
	 * @brief Get a ready-to-use DescriptorImageInfo for the 3D fallback texture
	 */
	vk::DescriptorImageInfo getFallbackTextureInfo3D();

	// Texture access

	/**
	 * @brief Get texture slot data
	 */
	tcache_slot_vulkan* getTextureSlot(int handle);

	/**
	 * @brief Create a static, single-mip, single-layer 2D texture from CPU pixel data
	 *
	 * For small fixed lookup tables (e.g. SMAA area/search textures) that aren't
	 * part of the regular bitmap/tcache system. Uploads synchronously.
	 */
	bool createStaticTexture2D(uint32_t width, uint32_t height, vk::Format format,
	                           const void* pixelData, size_t dataSize, const char* debugName,
	                           vk::Image& outImage, vk::ImageView& outView, VulkanAllocation& outAlloc);

	// Utility functions

	/**
	 * @brief Convert FSO bitmap format to Vulkan format
	 */
	static vk::Format bppToVkFormat(int bpp, bool compressed = false, int compressionType = 0);

	/**
	 * @brief Transition image layout
	 */
	void transitionImageLayout(vk::Image image, vk::Format format,
	                           vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	                           uint32_t mipLevels = 1, uint32_t arrayLayers = 1);

	/**
	 * @brief Called at start of frame
	 */
	void frameStart();

private:
	/**
	 * @brief Create a Vulkan image
	 * @param cubemap If true, sets eCubeCompatible flag (requires arrayLayers=6)
	 * @param imageType Vulkan image type (e2D, e3D, etc.)
	 */
	bool createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
	                 vk::Format format, vk::ImageTiling tiling,
	                 vk::ImageUsageFlags usage, MemoryUsage memUsage,
	                 vk::Image& image, VulkanAllocation& allocation,
	                 uint32_t arrayLayers = 1, bool cubemap = false,
	                 uint32_t imageDepth = 1,
	                 vk::ImageType imageType = vk::ImageType::e2D);

	enum class ImageViewType { Array2D, Plain2D, Cube, Volume3D };

	/**
	 * @brief Create an image view
	 * @param viewType Controls view type: Array2D=sampler2DArray, Plain2D=sampler2D, Cube=samplerCube
	 */
	vk::ImageView createImageView(vk::Image image, vk::Format format,
	                               vk::ImageAspectFlags aspectFlags,
	                               uint32_t mipLevels,
	                               ImageViewType viewType = ImageViewType::Array2D,
	                               uint32_t layerCount = 1,
	                               uint32_t baseArrayLayer = 0);

	/**
	 * @brief Create a 1x1 white fallback texture (image + view + upload)
	 */
	bool createFallbackTexture(vk::Image& outImage, VulkanAllocation& outAlloc,
	                           vk::ImageView& outView, ImageViewType viewType,
	                           uint32_t arrayLayers = 1, bool cubemap = false,
	                           vk::ImageType imageType = vk::ImageType::e2D);

	/**
	 * @brief Begin single-time command buffer
	 */
	vk::CommandBuffer beginSingleTimeCommands();

	/**
	 * @brief End and submit single-time command buffer (synchronous, blocks on waitIdle)
	 */
	void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

	/**
	 * @brief Create a host-visible staging buffer (eTransferSrc) sized for an upload
	 *
	 * Creates the buffer and allocates CpuOnly memory for it. On failure the
	 * buffer is destroyed and false is returned (outBuffer left null). Callers
	 * remain responsible for any image/view they allocated beforehand.
	 */
	bool createStagingBuffer(size_t size, vk::Buffer& outBuffer, VulkanAllocation& outAllocation);

	/**
	 * @brief Record layout transitions and buffer-to-image copy into a command buffer
	 */
	static void recordUploadCommands(vk::CommandBuffer cmd, vk::Image image, vk::Buffer stagingBuffer,
	                          vk::Format format, uint32_t width, uint32_t height,
	                          uint32_t mipLevels, vk::ImageLayout oldLayout,
	                          bool generateMips = false,
	                          const SCP_vector<vk::BufferImageCopy>& regions = {},
	                          uint32_t arrayLayers = 1);

	/**
	 * @brief Submit an upload command buffer asynchronously and defer resource cleanup
	 *
	 * Submits without waitIdle. Queues staging buffer and command buffer for
	 * deferred destruction/free after enough frames have elapsed.
	 */
	void submitUploadAsync(vk::CommandBuffer cmd, vk::Buffer stagingBuffer,
	                       VulkanAllocation stagingAllocation);

	/**
	 * @brief Free command buffers whose GPU work has completed
	 */
	void processPendingCommandBuffers();

	/**
	 * @brief Calculate number of mipmap levels
	 */
	static uint32_t calculateMipLevels(uint32_t width, uint32_t height);

	/**
	 * @brief Upload all frames of an animation as layers of a single texture array
	 */
	bool uploadAnimationFrames(int handle, bitmap* bm, int compType,
	                           int baseFrame, int numFrames);

	/**
	 * @brief Upload a cubemap DDS texture (6 faces) as a single cubemap image
	 */
	bool uploadCubemap(int handle, bitmap* bm, int compType);

	/**
	 * @brief Upload a 3D texture (volumetric data) as a single 3D image
	 */
	bool upload3DTexture(int handle, bitmap* bm, int texDepth);

	/**
	 * @brief Upload a plain 2D texture (the common case once animation/cubemap/3D are ruled out)
	 */
	bool uploadTexture2D(int handle, bitmap* bm, int compType);

	/**
	 * @brief Ref-count release for a shared animation texture-array slot
	 *
	 * Shared animation frames all point at one array VkImage; the slot for the
	 * triggering frame must not destroy that image while other frames still
	 * reference it. Marks this slot unused and walks the frame range:
	 *  - non-array slot (arrayLayers <= 1)                -> returns true (caller destroys normally)
	 *  - shared array, another frame still in use         -> detaches this slot in place
	 *                                                        (image/view/allocation cleared + reset),
	 *                                                        returns false
	 *  - shared array, this was the last live reference   -> returns true (caller destroys the shared image)
	 *
	 * The caller keeps ownership of the actual destruction sequence (it differs
	 * between flushTextures and bm_free_data), this only makes the shared-vs-last
	 * decision.
	 */
	static bool releaseAnimationSlotRef(tcache_slot_vulkan* ts);

	/**
	 * @brief Decode a get_bitmap_from_texture() readback buffer into data_out
	 *
	 * Handles both the BC-compressed (block-decode via bcdec) and uncompressed
	 * (per-format channel expand/convert) cases; data_out is tightly packed
	 * w*h pixels at outChannels (3 or 4) bytes each.
	 */
	static void decodeReadbackBuffer(const void* mapped, vk::Format format, bool isCompressed,
	                          uint32_t w, uint32_t h, uint32_t blockW, uint32_t blockH,
	                          int blockSize, int outChannels, void* data_out);

	// Guard flag to prevent recursion when bm_lock calls bm_data during animation upload
	bool m_uploadingAnimation = false;

	// First-N debug-log counters: members rather than function-local statics
	// so they reset when the manager is recreated on a renderer restart, and aren't
	// shared across a hypothetical second instance. Purely gate nprintf spam.
	int m_bmDataLogCount = 0;       // bm_data entry trace
	int m_uploadFmtLogCount = 0;    // uploadTexture2D format trace
	int m_reuploadLogCount = 0;     // uploadTexture2D resident-texture re-upload notice

	// Deferred command buffer free list
	struct PendingCommandBuffer {
		vk::CommandBuffer cb;
		uint32_t framesRemaining;
	};
	SCP_vector<PendingCommandBuffer> m_pendingCommandBuffers;

	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	VulkanMemoryManager* m_memoryManager = nullptr;
	vk::CommandPool m_commandPool;
	vk::Queue m_graphicsQueue;

	// Cached samplers (key: packed sampler state)
	SCP_unordered_map<uint64_t, vk::Sampler> m_samplerCache;
	vk::Sampler m_defaultSampler;

	// Fallback 1x1 white textures for unbound texture slots
	vk::Image m_fallback2DArrayTexture;
	vk::ImageView m_fallback2DArrayView;      // 2D_ARRAY view (for material texture arrays)
	VulkanAllocation m_fallback2DArrayAllocation;

	vk::Image m_fallbackTexture2D;
	vk::ImageView m_fallbackTextureView2D;    // 2D view (for post-processing sampler2D)
	VulkanAllocation m_fallbackTexture2DAllocation;

	// Fallback 1x1x6 white cubemap for unbound samplerCube slots
	vk::Image m_fallbackCubeTexture;
	vk::ImageView m_fallbackCubeView;         // Cube view (for samplerCube)
	VulkanAllocation m_fallbackCubeAllocation;

	// Fallback 1x1x1 white 3D texture for unbound sampler3D slots
	vk::Image m_fallback3DTexture;
	vk::ImageView m_fallback3DView;           // 3D view (for sampler3D)
	VulkanAllocation m_fallback3DAllocation;

	// Device limits
	uint32_t m_maxTextureSize = 4096;
	float m_maxAnisotropy = 1.0f;

	// Current render target state
	int m_currentRenderTarget = -1;

	bool m_initialized = false;
};

// Global texture manager instance
VulkanTextureManager* getTextureManager();
void setTextureManager(VulkanTextureManager* manager);

/**
 * @brief Generate mip levels 1..mipLevels-1 via blit chain from the previous level.
 *
 * Prerequisite: mip 0 must already be in eTransferSrcOptimal.
 * Result: ALL mip levels transitioned to eShaderReadOnlyOptimal.
 */
void vulkan_generate_mipmap_chain(vk::CommandBuffer cmd, vk::Image image,
                                  uint32_t width, uint32_t height,
                                  uint32_t mipLevels, uint32_t arrayLayers = 1);

// ========== gr_screen function pointer implementations ==========

int vulkan_preload(int bitmap_num, int is_aabitmap);
void vulkan_bm_create(bitmap_slot* slot);
void vulkan_bm_free_data(bitmap_slot* slot, bool release);
void vulkan_bm_init(bitmap_slot* slot);
bool vulkan_bm_data(int handle, bitmap* bm);
void vulkan_bm_page_in_start();
int vulkan_bm_make_render_target(int handle, int* width, int* height, int* bpp, int* mm_lvl, int flags);
int vulkan_bm_set_render_target(int handle, int face);
void vulkan_update_texture(int bitmap_handle, int bpp, const ubyte* data, int width, int height);
ubyte* vulkan_get_bitmap_from_texture(int bitmap_num, int* width_out, int* height_out);

} // namespace graphics::vulkan

