#pragma once

#include "globalincs/pstypes.h"
#include "VulkanMemory.h"

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

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

	// 3D texture support
	bool is3D = false;
	uint32_t depth = 1;

	// Cubemap support
	bool isCubemap = false;
	vk::ImageView cubeFaceViews[6] = {};  // Per-face 2D views for render-to-cubemap
	vk::Framebuffer cubeFaceFramebuffers[6] = {};  // Per-face framebuffers for render-to-cubemap
	vk::ImageView cubeImageView;  // Cube view for sampling (viewType=eCube, layerCount=6)

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

	// Bitmap management functions (implement gr_screen function pointers)

	/**
	 * @brief Initialize a bitmap slot for Vulkan
	 */
	void bm_init(bitmap_slot* slot);

	/**
	 * @brief Create Vulkan resources for a bitmap slot
	 */
	void bm_create(bitmap_slot* slot);

	/**
	 * @brief Free Vulkan resources for a bitmap slot
	 */
	void bm_free_data(bitmap_slot* slot, bool release);

	/**
	 * @brief Upload bitmap data to GPU
	 * @param compType Compression type (DDS_DXT1/3/5, DDS_BC7) or 0 for uncompressed
	 */
	bool bm_data(int handle, bitmap* bm, int compType = 0);

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
	 */
	void get_bitmap_from_texture(void* data_out, int bitmap_num);

	// Sampler management

	/**
	 * @brief Get or create a sampler with specified parameters
	 */
	vk::Sampler getSampler(vk::Filter magFilter, vk::Filter minFilter,
	                       vk::SamplerAddressMode addressMode,
	                       bool enableAnisotropy, float maxAnisotropy,
	                       bool enableMipmaps);

	/**
	 * @brief Get default sampler for standard textures
	 */
	vk::Sampler getDefaultSampler();

	/**
	 * @brief Get fallback white texture image view (2D_ARRAY) for unbound material texture slots
	 */
	vk::ImageView getFallback2DArrayView();

	/**
	 * @brief Get fallback white texture image view (2D) for post-processing sampler2D slots
	 */
	vk::ImageView getFallbackTextureView2D();

	/**
	 * @brief Get fallback white cubemap image view (Cube) for unbound samplerCube slots
	 */
	vk::ImageView getFallbackCubeView();

	/**
	 * @brief Get fallback white 3D texture image view for unbound sampler3D slots
	 */
	vk::ImageView getFallback3DView();

	// Texture access

	/**
	 * @brief Get texture slot data
	 */
	tcache_slot_vulkan* getTextureSlot(int handle);

	/**
	 * @brief Check if texture is valid and ready for use
	 */
	bool isTextureValid(int handle);

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
	 * @brief Record layout transitions and buffer-to-image copy into a command buffer
	 */
	void recordUploadCommands(vk::CommandBuffer cmd, vk::Image image, vk::Buffer stagingBuffer,
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

	// Guard flag to prevent recursion when bm_lock calls bm_data during animation upload
	bool m_uploadingAnimation = false;

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
void vulkan_get_bitmap_from_texture(void* data_out, int bitmap_num);

} // namespace vulkan
} // namespace graphics
