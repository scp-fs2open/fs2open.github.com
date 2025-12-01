#pragma once

#include "globalincs/pstypes.h"

#ifdef WITH_VULKAN

#include "bmpman/bm_internal.h"

#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace graphics {
namespace vulkan {

// Forward declarations
class VulkanRenderer;

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
 */
class VulkanTextureManager {
public:
	VulkanTextureManager() = default;
	~VulkanTextureManager() = default;

	// Non-copyable
	VulkanTextureManager(const VulkanTextureManager&) = delete;
	VulkanTextureManager& operator=(const VulkanTextureManager&) = delete;

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
	 * @brief Get sampler cache
	 */
	VulkanSamplerCache& getSamplerCache() { return m_samplerCache; }

	/**
	 * @brief Called at frame start to manage staging buffer
	 * @param frameIndex Current frame index (for ring buffer partitioning)
	 */
	void beginFrame(uint32_t frameIndex);

	/**
	 * @brief Check if format supports GPU mipmap generation
	 */
	bool canGenerateMipmaps(vk::Format format) const;

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
	 * @brief Allocate region from staging buffer
	 * @param size Size needed
	 * @param outOffset Returns the offset into the staging buffer
	 * @return Pointer to staging memory, or nullptr if full
	 */
	void* allocateStagingMemory(vk::DeviceSize size, vk::DeviceSize& outOffset);

	/**
	 * @brief Begin recording upload commands
	 */
	void beginUpload();

	/**
	 * @brief Submit upload commands and signal fence
	 */
	void endUpload();

	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	vk::CommandPool m_commandPool;
	vk::Queue m_transferQueue;

	// Upload command buffer and synchronization
	vk::UniqueCommandBuffer m_uploadCommandBuffer;
	vk::UniqueFence m_uploadFence;

	// Staging buffer (16MB ring buffer, partitioned by frame)
	static constexpr vk::DeviceSize STAGING_BUFFER_SIZE = 16 * 1024 * 1024;
	vk::UniqueBuffer m_stagingBuffer;
	vk::UniqueDeviceMemory m_stagingMemory;
	void* m_stagingMapped = nullptr;
	vk::DeviceSize m_stagingOffset = 0;
	uint32_t m_currentFrameIndex = 0;

	// Sampler cache
	VulkanSamplerCache m_samplerCache;

	// Texture storage - keyed by bitmap handle
	// Note: VulkanTexture is stored as pointer since it inherits from gr_bitmap_info
	// and is owned by bitmap_slot::gr_info
	SCP_unordered_map<int, VulkanTexture*> m_textures;

	bool m_initialized = false;
	bool m_uploadInProgress = false;
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
