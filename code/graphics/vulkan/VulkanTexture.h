#pragma once

#include "globalincs/pstypes.h"

#ifdef WITH_VULKAN

#include "bmpman/bm_internal.h"
#include "VulkanFramebuffer.h"

#include <array>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace graphics {
namespace vulkan {

// Forward declarations
class VulkanRenderer;
class VulkanFramebuffer;

/**
 * @brief Render target data associated with a bitmap handle
 *
 * Contains the Vulkan resources needed to render to a texture:
 * - The texture itself (VulkanTexture with color attachment usage)
 * - A dedicated render pass for this RT
 * - A framebuffer binding the texture to the render pass
 * - Optional depth attachment for 3D rendering to RT
 */
struct VulkanRenderTarget {
	vk::UniqueRenderPass renderPass;        // Render pass for this RT
	std::unique_ptr<VulkanFramebuffer> framebuffer;  // Framebuffer with color + optional depth
	int workingHandle = -1;                 // Bitmap handle being rendered to
	bool isStatic = false;                  // Static RT (save on unbind)
	bool isCubemap = false;                 // Is this a cubemap RT
	int activeFace = 0;                     // Current cubemap face (0-5)
	
	// For cubemaps, we need per-face framebuffers and image views
	std::array<vk::UniqueImageView, 6> cubeFaceViews;
	std::array<std::unique_ptr<VulkanFramebuffer>, 6> cubeFaceFramebuffers;
};

/**
 * @brief Vulkan texture resource - inherits from gr_bitmap_info for bmpman integration
 *
 * Manages:
 * - vk::Image for GPU storage
 * - vk::DeviceMemory allocation
 * - vk::ImageView for shader access
 * - Layout state tracking
 */
class VulkanTexture : public gr_bitmap_info {
public:
	VulkanTexture() = default;
	~VulkanTexture() override;

	// Non-copyable
	VulkanTexture(const VulkanTexture&) = delete;
	VulkanTexture& operator=(const VulkanTexture&) = delete;

	/**
	 * @brief Create texture image with specified parameters
	 * @param device Vulkan logical device
	 * @param physicalDevice Physical device for memory queries
	 * @param width Texture width in pixels
	 * @param height Texture height in pixels
	 * @param format Vulkan format
	 * @param mipLevels Number of mipmap levels
	 * @param arrayLayers Number of array layers (for texture arrays)
	 * @param usage Image usage flags
	 * @return true on success
	 */
	bool create(vk::Device device, vk::PhysicalDevice physicalDevice,
	            uint32_t width, uint32_t height, vk::Format format,
	            uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
	            vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled |
	                                        vk::ImageUsageFlagBits::eTransferDst |
	                                        vk::ImageUsageFlagBits::eTransferSrc);

	/**
	 * @brief Upload pixel data from staging buffer to texture
	 * @param cmd Command buffer to record upload commands
	 * @param stagingBuffer Source staging buffer
	 * @param stagingOffset Offset into staging buffer
	 * @param dataSize Size of data to copy
	 * @param mipLevel Target mip level
	 * @param arrayLayer Target array layer
	 */
	void uploadData(vk::CommandBuffer cmd, vk::Buffer stagingBuffer,
	                vk::DeviceSize stagingOffset, vk::DeviceSize dataSize,
	                uint32_t mipLevel = 0, uint32_t arrayLayer = 0);

	/**
	 * @brief Destroy texture resources
	 */
	void destroy();

	/**
	 * @brief Generate mipmaps using vkCmdBlitImage chain
	 * @param cmd Command buffer to record blit commands
	 * @note Only call for formats that support linear filtering blit
	 */
	void generateMipmaps(vk::CommandBuffer cmd);

	/**
	 * @brief Transition image layout with appropriate barriers
	 * @param cmd Command buffer to record barrier
	 * @param oldLayout Current layout (use eUndefined if unknown)
	 * @param newLayout Desired layout
	 */
	void transitionLayout(vk::CommandBuffer cmd,
	                      vk::ImageLayout oldLayout,
	                      vk::ImageLayout newLayout);

	/**
	 * @brief Notify that external code changed layout (e.g., render pass)
	 * @param newLayout The layout the image is now in
	 */
	void notifyLayoutChanged(vk::ImageLayout newLayout) { m_currentLayout = newLayout; }

	// Accessors
	vk::Image getImage() const { return m_image.get(); }
	vk::ImageView getImageView() const { return m_imageView.get(); }
	/**
	 * @brief Get 2D array view for use with sampler2DArray shaders
	 * For single-layer textures, returns a 2DArray view of the same image.
	 * For multi-layer textures, returns the same as getImageView().
	 */
	vk::ImageView getImageViewArray() const {
		return m_imageViewArray ? m_imageViewArray.get() : m_imageView.get();
	}
	vk::Format getFormat() const { return m_format; }
	uint32_t getWidth() const { return m_extent.width; }
	uint32_t getHeight() const { return m_extent.height; }
	uint32_t getMipLevels() const { return m_mipLevels; }
	uint32_t getArrayLayers() const { return m_arrayLayers; }
	vk::ImageLayout getCurrentLayout() const { return m_currentLayout; }
	bool isValid() const { return m_image.get() != nullptr; }

private:
	vk::Device m_device;
	vk::UniqueImage m_image;
	vk::UniqueDeviceMemory m_memory;
	vk::UniqueImageView m_imageView;
	vk::UniqueImageView m_imageViewArray;  // 2DArray view for single-layer textures (for sampler2DArray)

	vk::Extent3D m_extent = {0, 0, 1};
	vk::Format m_format = vk::Format::eUndefined;
	uint32_t m_mipLevels = 1;
	uint32_t m_arrayLayers = 1;
	vk::ImageLayout m_currentLayout = vk::ImageLayout::eUndefined;

	/**
	 * @brief Find suitable memory type for image allocation
	 */
	uint32_t findMemoryType(vk::PhysicalDevice physicalDevice,
	                        uint32_t typeFilter,
	                        vk::MemoryPropertyFlags properties);
};

/**
 * @brief Cache for Vulkan samplers - FSO uses ~5 distinct sampler configs
 *
 * Samplers are immutable and can be shared across textures with same settings.
 * This avoids creating thousands of duplicate samplers.
 */
class VulkanSamplerCache {
public:
	VulkanSamplerCache() = default;
	~VulkanSamplerCache() = default;

	// Non-copyable
	VulkanSamplerCache(const VulkanSamplerCache&) = delete;
	VulkanSamplerCache& operator=(const VulkanSamplerCache&) = delete;

	/**
	 * @brief Initialize the sampler cache
	 * @param device Vulkan device
	 * @param maxAnisotropy Max anisotropy supported by device
	 */
	bool initialize(vk::Device device, float maxAnisotropy);

	/**
	 * @brief Shutdown and destroy all cached samplers
	 */
	void shutdown();

	/**
	 * @brief Get or create sampler with specified parameters
	 * @param minFilter Minification filter
	 * @param magFilter Magnification filter
	 * @param addressMode Wrap/clamp mode
	 * @param anisotropy Anisotropy level (1.0 = disabled)
	 * @param enableMipmaps Enable mipmap filtering
	 * @return Sampler handle (valid until shutdown)
	 */
	vk::Sampler getSampler(vk::Filter minFilter, vk::Filter magFilter,
	                       vk::SamplerAddressMode addressMode,
	                       float anisotropy = 1.0f,
	                       bool enableMipmaps = true);

private:
	struct SamplerKey {
		vk::Filter minFilter;
		vk::Filter magFilter;
		vk::SamplerAddressMode addressMode;
		float anisotropy;
		bool enableMipmaps;

		bool operator==(const SamplerKey& other) const {
			return minFilter == other.minFilter &&
			       magFilter == other.magFilter &&
			       addressMode == other.addressMode &&
			       anisotropy == other.anisotropy &&
			       enableMipmaps == other.enableMipmaps;
		}
	};

	struct SamplerKeyHash {
		size_t operator()(const SamplerKey& key) const {
			size_t h = 0;
			h ^= std::hash<int>()(static_cast<int>(key.minFilter)) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>()(static_cast<int>(key.magFilter)) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>()(static_cast<int>(key.addressMode)) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<float>()(key.anisotropy) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<bool>()(key.enableMipmaps) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};

	vk::Device m_device;
	float m_maxAnisotropy = 1.0f;
	std::unordered_map<SamplerKey, vk::UniqueSampler, SamplerKeyHash> m_samplers;
	bool m_initialized = false;
};

/**
 * @brief Manages texture creation, uploads, and bmpman integration
 *
 * Handles:
 * - Staging buffer for texture uploads (16MB ring buffer)
 * - Format conversion (24-bit -> 32-bit on CPU)
 * - Compressed texture support (DXT/BC direct upload)
 * - Render target management
 * - Non-blocking batched uploads (per-frame command buffer batching)
 */
class VulkanTextureManager {
public:
	VulkanTextureManager() = default;
	~VulkanTextureManager() = default;

	// Non-copyable
	VulkanTextureManager(const VulkanTextureManager&) = delete;
	VulkanTextureManager& operator=(const VulkanTextureManager&) = delete;

	/**
	 * @brief Number of frames in flight - must match VulkanRenderer
	 */
	static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

	/**
	 * @brief Initialize texture manager
	 * @param device Vulkan device
	 * @param physicalDevice Physical device for memory queries
	 * @param commandPool Command pool for upload commands
	 * @param transferQueue Queue for transfer operations
	 * @return true on success
	 */
	bool initialize(vk::Device device, vk::PhysicalDevice physicalDevice,
	                vk::CommandPool commandPool, vk::Queue transferQueue);

	/**
	 * @brief Shutdown and release all resources
	 */
	void shutdown();

	/**
	 * @brief Create texture for a bitmap handle
	 * @param bitmapHandle FSO bitmap handle
	 * @param bm Bitmap data
	 * @return true on success
	 */
	bool createTexture(int bitmapHandle, const bitmap* bm);

	/**
	 * @brief Upload texture data
	 * @param bitmapHandle FSO bitmap handle
	 * @param bm Bitmap data to upload
	 * @return true on success
	 */
	bool uploadTextureData(int bitmapHandle, const bitmap* bm);

	/**
	 * @brief Destroy texture for a bitmap handle
	 * @param bitmapHandle FSO bitmap handle
	 */
	void destroyTexture(int bitmapHandle);

	/**
	 * @brief Get texture for rendering
	 * @param bitmapHandle FSO bitmap handle
	 * @return VulkanTexture pointer, or nullptr if not found
	 */
	VulkanTexture* getTexture(int bitmapHandle);

	/**
	 * @brief Get currently active render target
	 */
	VulkanRenderTarget* getActiveRenderTarget() const { return m_activeRenderTarget; }

	/**
	 * @brief Get sampler cache
	 */
	VulkanSamplerCache& getSamplerCache() { return m_samplerCache; }

	/**
	 * @brief Get placeholder texture (1x1 white pixel) for unbound material slots
	 */
	VulkanTexture* getPlaceholderTexture() { return m_placeholderTexture.get(); }

	/**
	 * @brief Queue a texture for deferred deletion
	 *
	 * Called when bmpman frees texture data. Instead of immediately destroying
	 * the texture (which may still be referenced by in-flight command buffers),
	 * we queue it for deletion after the GPU finishes using it.
	 *
	 * @param texture The texture to delete (ownership transferred to manager)
	 */
	void queueTextureForDeletion(VulkanTexture* texture);

	/**
	 * @brief Called at frame start AFTER the frame's fence has been waited on
	 *
	 * Processes deferred staging offset reset for the current frame.
	 * Since we've waited on the fence, we know the GPU is done with uploads
	 * from when this frame index was last active.
	 *
	 * @param frameIndex Current frame index (0 to FRAMES_IN_FLIGHT-1)
	 */
	void beginFrame(uint32_t frameIndex);

	/**
	 * @brief Submit all pending upload commands for the current frame
	 *
	 * Called by VulkanRenderer before graphics work submission. If no uploads
	 * were recorded this frame, this is a no-op.
	 *
	 * @note Upload commands use the graphics queue and synchronize with the frame fence,
	 * so staging memory is safe to reuse when beginFrame() is called for this frame again.
	 */
	void submitUploads();

	/**
	 * @brief Check if format supports GPU mipmap generation
	 */
	bool canGenerateMipmaps(vk::Format format) const;

	// =========================================================================
	// Static Utility Functions (for unit testing)
	// =========================================================================

	/**
	 * @brief Calculate number of mip levels for dimensions (static version)
	 */
	static uint32_t calculateMipLevelsStatic(uint32_t width, uint32_t height);

	/**
	 * @brief Calculate data size for a mip level (static version)
	 */
	static size_t calculateMipSizeStatic(uint32_t width, uint32_t height, vk::Format format);

	/**
	 * @brief Check if format is block-compressed (static version)
	 */
	static bool isCompressedFormatStatic(vk::Format format);

private:
	/**
	 * @brief Select Vulkan format for bitmap
	 */
	vk::Format selectFormat(const bitmap* bm) const;

	/**
	 * @brief Calculate number of mip levels for dimensions
	 */
	uint32_t calculateMipLevels(uint32_t width, uint32_t height) const;

	/**
	 * @brief Calculate data size for a mip level (handles compressed formats)
	 */
	size_t calculateMipSize(uint32_t width, uint32_t height, vk::Format format) const;

	/**
	 * @brief Check if format is block-compressed
	 */
	bool isCompressedFormat(vk::Format format) const;

	/**
	 * @brief Allocate region from staging buffer for current frame
	 * @param size Size needed
	 * @param outOffset Returns the offset into the staging buffer
	 * @return Pointer to staging memory, or nullptr if partition full
	 *
	 * @note Each frame uses its own partition of the staging buffer to avoid
	 * GPU/CPU synchronization issues. If the partition is full, returns nullptr
	 * and the caller should submit pending uploads and try again next frame.
	 */
	void* allocateStagingMemory(vk::DeviceSize size, vk::DeviceSize& outOffset);

	/**
	 * @brief Ensure upload command buffer is recording for current frame
	 */
	void ensureUploadRecording();

	/**
	 * @brief Get the command buffer for recording upload commands
	 */
	vk::CommandBuffer getUploadCommandBuffer();

	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	vk::CommandPool m_commandPool;
	vk::Queue m_transferQueue;

	// Per-frame upload command buffers for batched non-blocking uploads
	std::array<vk::CommandBuffer, FRAMES_IN_FLIGHT> m_uploadCmds{};
	std::array<bool, FRAMES_IN_FLIGHT> m_uploadCmdRecording{};
	std::array<vk::Fence, FRAMES_IN_FLIGHT> m_uploadFences{};
	std::array<bool, FRAMES_IN_FLIGHT> m_uploadFenceSubmitted{};  // True if fence was submitted and needs wait

	// Staging buffer (16MB ring buffer)
	// Each frame gets half the buffer to avoid overlap
	static constexpr vk::DeviceSize STAGING_BUFFER_SIZE = 16 * 1024 * 1024;
	static constexpr vk::DeviceSize STAGING_PARTITION_SIZE = STAGING_BUFFER_SIZE / FRAMES_IN_FLIGHT;
	vk::UniqueBuffer m_stagingBuffer;
	vk::UniqueDeviceMemory m_stagingMemory;
	void* m_stagingMapped = nullptr;
	vk::DeviceSize m_stagingOffset = 0;
	uint32_t m_currentFrameIndex = 0;

	// Sampler cache
	VulkanSamplerCache m_samplerCache;

	// Placeholder texture (1x1 white pixel) for unbound material descriptor slots
	std::unique_ptr<VulkanTexture> m_placeholderTexture;

	// Texture storage - keyed by bitmap handle
	// Note: VulkanTexture is stored as pointer since it inherits from gr_bitmap_info
	// and is owned by bitmap_slot::gr_info
	SCP_unordered_map<int, VulkanTexture*> m_textures;

	// Render target registry - keyed by bitmap handle
	SCP_unordered_map<int, std::unique_ptr<VulkanRenderTarget>> m_renderTargets;

	// Currently active render target (nullptr = scene framebuffer)
	VulkanRenderTarget* m_activeRenderTarget = nullptr;
	int m_activeRenderTargetHandle = -1;

	// Per-frame deferred texture deletion queues
	// Textures are queued here when bmpman frees them, then actually deleted
	// in beginFrame() after the frame fence ensures GPU is done using them
	std::array<SCP_vector<VulkanTexture*>, FRAMES_IN_FLIGHT> m_pendingTextureDeletions;

	bool m_initialized = false;

public:
	// Render target API
	/**
	 * @brief Create a render target for a bitmap handle
	 * @param handle Bitmap handle
	 * @param width Width in pixels
	 * @param height Height in pixels
	 * @param bpp Bits per pixel (output)
	 * @param mm_lvl Mipmap levels (output)
	 * @param flags BMP_FLAG_RENDER_TARGET_* flags
	 * @return 1 on success, 0 on failure
	 */
	int createRenderTarget(int handle, int* width, int* height, int* bpp, int* mm_lvl, int flags);

	/**
	 * @brief Set the active render target
	 * @param handle Bitmap handle (-1 to restore scene framebuffer)
	 * @param face Cubemap face (0-5, or -1 for 2D)
	 * @return 1 on success, 0 on failure
	 */
	int setRenderTarget(int handle, int face);

	/**
	 * @brief Get render target by handle
	 * @param handle Bitmap handle
	 * @return Render target or nullptr
	 */
	VulkanRenderTarget* getRenderTarget(int handle);

	/**
	 * @brief Get currently active render target handle
	 */
	int getActiveRenderTargetHandle() const { return m_activeRenderTargetHandle; }

	/**
	 * @brief Check if currently rendering to a render target
	 */
	bool isRenderingToTexture() const { return m_activeRenderTarget != nullptr; }

	/**
	 * @brief Read back texture data to CPU memory
	 * @param data_out Output buffer (must be pre-allocated)
	 * @param handle Bitmap handle
	 */
	void readbackTexture(void* data_out, int handle);

	/**
	 * @brief Destroy a render target
	 * @param handle Bitmap handle
	 */
	void destroyRenderTarget(int handle);

private:
	/**
	 * @brief Create a render pass for a render target
	 * @param colorFormat Color attachment format
	 * @param withDepth Whether to include depth attachment
	 * @param isCubemap Whether this is for a cubemap
	 * @return Unique render pass handle
	 */
	vk::UniqueRenderPass createRenderTargetRenderPass(vk::Format colorFormat, bool withDepth, bool isCubemap);
};

// Global texture manager instance (set by VulkanRenderer)
extern VulkanTextureManager* g_vulkanTextureManager;

// gr_screen function pointer implementations (must match signatures in 2d.h)
void gr_vulkan_bm_create(bitmap_slot* entry);
void gr_vulkan_bm_init(bitmap_slot* slot);
bool gr_vulkan_bm_data(int handle, bitmap* bm);
void gr_vulkan_bm_free_data(bitmap_slot* slot, bool release);
void gr_vulkan_update_texture(int handle, int bpp, const ubyte* data, int width, int height);
int gr_vulkan_bm_make_render_target(int handle, int* width, int* height, int* bpp, int* mm_lvl, int flags);
int gr_vulkan_bm_set_render_target(int handle, int face);
void gr_vulkan_get_bitmap_from_texture(void* /*dip_in*/, int handle);
void gr_vulkan_set_texture_addressing(int mode);

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
