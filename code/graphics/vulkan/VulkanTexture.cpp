#include "VulkanTexture.h"

#ifdef WITH_VULKAN

#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"
#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace graphics {
namespace vulkan {

// Global texture manager instance
VulkanTextureManager* g_vulkanTextureManager = nullptr;

// ============================================================================
// VulkanTexture Implementation
// ============================================================================

VulkanTexture::~VulkanTexture()
{
	destroy();
}

bool VulkanTexture::create(vk::Device device, vk::PhysicalDevice physicalDevice,
                           uint32_t width, uint32_t height, vk::Format format,
                           uint32_t mipLevels, uint32_t arrayLayers,
                           vk::ImageUsageFlags usage)
{
	m_device = device;
	m_extent = vk::Extent3D{width, height, 1};
	m_format = format;
	m_mipLevels = mipLevels;
	m_arrayLayers = arrayLayers;

	// Create image
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.extent = m_extent;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	try {
		m_image = device.createImageUnique(imageInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create texture image: %s\n", e.what()));
		return false;
	}

	// Allocate memory
	vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(m_image.get());
	uint32_t memTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits,
	                                        vk::MemoryPropertyFlagBits::eDeviceLocal);
	if (memTypeIndex == std::numeric_limits<uint32_t>::max()) {
		mprintf(("Vulkan: Failed to find suitable memory type for texture image\n"));
		m_image.reset();
		return false;
	}

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = memTypeIndex;

	try {
		m_memory = device.allocateMemoryUnique(allocInfo);
		device.bindImageMemory(m_image.get(), m_memory.get(), 0);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to allocate texture memory: %s\n", e.what()));
		m_image.reset();
		return false;
	}

	// Create image view
	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.image = m_image.get();
	viewInfo.viewType = (arrayLayers > 1) ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = arrayLayers;

	try {
		m_imageView = device.createImageViewUnique(viewInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create texture image view: %s\n", e.what()));
		m_memory.reset();
		m_image.reset();
		return false;
	}

	m_currentLayout = vk::ImageLayout::eUndefined;
	return true;
}

void VulkanTexture::destroy()
{
	m_imageView.reset();
	m_memory.reset();
	m_image.reset();
	m_extent = vk::Extent3D{0, 0, 1};
	m_format = vk::Format::eUndefined;
	m_mipLevels = 1;
	m_arrayLayers = 1;
	m_currentLayout = vk::ImageLayout::eUndefined;
}

void VulkanTexture::transitionLayout(vk::CommandBuffer cmd,
                                      vk::ImageLayout oldLayout,
                                      vk::ImageLayout newLayout)
{
	if (oldLayout == newLayout) {
		return;
	}

	vk::ImageMemoryBarrier barrier{};
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_image.get();
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = m_mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = m_arrayLayers;

	vk::PipelineStageFlags srcStage;
	vk::PipelineStageFlags dstStage;

	// Determine access masks and stages based on layouts
	if (oldLayout == vk::ImageLayout::eUndefined &&
	    newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
	         newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
	         newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		srcStage = vk::PipelineStageFlagBits::eFragmentShader;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined &&
	         newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
	         newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else {
		// Generic fallback - may not be optimal
		barrier.srcAccessMask = vk::AccessFlagBits::eMemoryWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
		srcStage = vk::PipelineStageFlagBits::eAllCommands;
		dstStage = vk::PipelineStageFlagBits::eAllCommands;
	}

	cmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);
	m_currentLayout = newLayout;
}

void VulkanTexture::uploadData(vk::CommandBuffer cmd, vk::Buffer stagingBuffer,
                                vk::DeviceSize stagingOffset, vk::DeviceSize dataSize,
                                uint32_t mipLevel, uint32_t arrayLayer)
{
	// Calculate mip dimensions
	uint32_t mipWidth = std::max(m_extent.width >> mipLevel, 1u);
	uint32_t mipHeight = std::max(m_extent.height >> mipLevel, 1u);

	vk::BufferImageCopy region{};
	region.bufferOffset = stagingOffset;
	region.bufferRowLength = 0;   // Tightly packed
	region.bufferImageHeight = 0; // Tightly packed
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = mipLevel;
	region.imageSubresource.baseArrayLayer = arrayLayer;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D{0, 0, 0};
	region.imageExtent = vk::Extent3D{mipWidth, mipHeight, 1};

	cmd.copyBufferToImage(stagingBuffer, m_image.get(),
	                      vk::ImageLayout::eTransferDstOptimal, region);
}

void VulkanTexture::generateMipmaps(vk::CommandBuffer cmd)
{
	if (m_mipLevels <= 1) {
		return;
	}

	vk::ImageMemoryBarrier barrier{};
	barrier.image = m_image.get();
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = m_arrayLayers;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = static_cast<int32_t>(m_extent.width);
	int32_t mipHeight = static_cast<int32_t>(m_extent.height);

	for (uint32_t i = 1; i < m_mipLevels; i++) {
		// Transition mip i-1: TRANSFER_DST -> TRANSFER_SRC
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
		                    vk::PipelineStageFlagBits::eTransfer,
		                    {}, {}, {}, barrier);

		// Blit from mip i-1 to mip i
		vk::ImageBlit blit{};
		blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = m_arrayLayers;
		blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
		blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};

		int32_t nextWidth = mipWidth > 1 ? mipWidth / 2 : 1;
		int32_t nextHeight = mipHeight > 1 ? mipHeight / 2 : 1;

		blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = m_arrayLayers;
		blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
		blit.dstOffsets[1] = vk::Offset3D{nextWidth, nextHeight, 1};

		cmd.blitImage(m_image.get(), vk::ImageLayout::eTransferSrcOptimal,
		              m_image.get(), vk::ImageLayout::eTransferDstOptimal,
		              blit, vk::Filter::eLinear);

		// Transition mip i-1: TRANSFER_SRC -> SHADER_READ
		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
		                    vk::PipelineStageFlagBits::eFragmentShader,
		                    {}, {}, {}, barrier);

		mipWidth = nextWidth;
		mipHeight = nextHeight;
	}

	// Transition final mip level: TRANSFER_DST -> SHADER_READ
	barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
	                    vk::PipelineStageFlagBits::eFragmentShader,
	                    {}, {}, {}, barrier);

	m_currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

uint32_t VulkanTexture::findMemoryType(vk::PhysicalDevice physicalDevice,
                                        uint32_t typeFilter,
                                        vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
		    (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	// Fallback to any matching type
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i)) {
			return i;
		}
	}

	return std::numeric_limits<uint32_t>::max();
}

// ============================================================================
// VulkanSamplerCache Implementation
// ============================================================================

bool VulkanSamplerCache::initialize(vk::Device device, float maxAnisotropy)
{
	m_device = device;
	m_maxAnisotropy = maxAnisotropy;
	m_initialized = true;
	return true;
}

void VulkanSamplerCache::shutdown()
{
	m_samplers.clear();
	m_initialized = false;
}

vk::Sampler VulkanSamplerCache::getSampler(vk::Filter minFilter, vk::Filter magFilter,
                                            vk::SamplerAddressMode addressMode,
                                            float anisotropy, bool enableMipmaps)
{
	if (!m_initialized) {
		return nullptr;
	}

	SamplerKey key{minFilter, magFilter, addressMode, anisotropy, enableMipmaps};

	auto it = m_samplers.find(key);
	if (it != m_samplers.end()) {
		return it->second.get();
	}

	// Create new sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.magFilter = magFilter;
	samplerInfo.minFilter = minFilter;
	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;
	samplerInfo.anisotropyEnable = (anisotropy > 1.0f) ? VK_TRUE : VK_FALSE;
	samplerInfo.maxAnisotropy = std::min(anisotropy, m_maxAnisotropy);
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = vk::CompareOp::eAlways;

	if (enableMipmaps) {
		samplerInfo.mipmapMode = (minFilter == vk::Filter::eLinear)
		                             ? vk::SamplerMipmapMode::eLinear
		                             : vk::SamplerMipmapMode::eNearest;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerInfo.mipLodBias = 0.0f;
	} else {
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.mipLodBias = 0.0f;
	}

	try {
		auto sampler = m_device.createSamplerUnique(samplerInfo);
		vk::Sampler result = sampler.get();
		m_samplers[key] = std::move(sampler);
		return result;
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create sampler: %s\n", e.what()));
		return nullptr;
	}
}

// ============================================================================
// VulkanTextureManager Implementation
// ============================================================================

bool VulkanTextureManager::initialize(vk::Device device, vk::PhysicalDevice physicalDevice,
                                       vk::CommandPool commandPool, vk::Queue transferQueue)
{
	m_device = device;
	m_physicalDevice = physicalDevice;
	m_commandPool = commandPool;
	m_transferQueue = transferQueue;

	mprintf(("VulkanTextureManager: init with commandPool=%p\n",
		reinterpret_cast<void*>(static_cast<VkCommandPool>(commandPool))));

	// Create per-frame upload command buffers
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = FRAMES_IN_FLIGHT;

	try {
		auto cmdBuffers = device.allocateCommandBuffers(allocInfo);
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
		m_uploadCmds[i] = cmdBuffers[i];
		m_uploadCmdRecording[i] = false;
		m_uploadFenceSubmitted[i] = false;  // No pending work to wait for initially
		// Create per-frame upload fences (unsignaled - we track submission state)
		vk::FenceCreateInfo fenceInfo;
		m_uploadFences[i] = m_device.createFence(fenceInfo);
		mprintf(("VulkanTextureManager: created upload fence[%u]=%p\n",
			i, reinterpret_cast<void*>(static_cast<VkFence>(m_uploadFences[i]))));
	}
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to allocate upload command buffers: %s\n", e.what()));
		return false;
	}

	// Create staging buffer
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = STAGING_BUFFER_SIZE;
	bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		m_stagingBuffer = device.createBufferUnique(bufferInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create staging buffer: %s\n", e.what()));
		return false;
	}

	// Allocate staging memory (host visible + coherent)
	vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(m_stagingBuffer.get());
	vk::PhysicalDeviceMemoryProperties memProps = physicalDevice.getMemoryProperties();

	uint32_t memTypeIndex = 0;
	vk::MemoryPropertyFlags desiredProps = vk::MemoryPropertyFlagBits::eHostVisible |
	                                        vk::MemoryPropertyFlagBits::eHostCoherent;

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((memReqs.memoryTypeBits & (1 << i)) &&
		    (memProps.memoryTypes[i].propertyFlags & desiredProps) == desiredProps) {
			memTypeIndex = i;
			break;
		}
	}

	vk::MemoryAllocateInfo allocMemInfo{};
	allocMemInfo.allocationSize = memReqs.size;
	allocMemInfo.memoryTypeIndex = memTypeIndex;

	try {
		m_stagingMemory = device.allocateMemoryUnique(allocMemInfo);
		device.bindBufferMemory(m_stagingBuffer.get(), m_stagingMemory.get(), 0);
		m_stagingMapped = device.mapMemory(m_stagingMemory.get(), 0, STAGING_BUFFER_SIZE);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to allocate staging memory: %s\n", e.what()));
		return false;
	}

	// Initialize sampler cache
	vk::PhysicalDeviceProperties deviceProps = physicalDevice.getProperties();
	if (!m_samplerCache.initialize(device, deviceProps.limits.maxSamplerAnisotropy)) {
		mprintf(("Vulkan: Failed to initialize sampler cache\n"));
		return false;
	}

	m_initialized = true;
	mprintf(("Vulkan: Texture manager initialized (staging buffer: %zu MB, %u partitions)\n",
	         STAGING_BUFFER_SIZE / (1024 * 1024), FRAMES_IN_FLIGHT));
	return true;
}

void VulkanTextureManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Wait for device idle to ensure all uploads complete
	m_device.waitIdle();

	// Textures are owned by bitmap_slot::gr_info and destroyed via gr_vulkan_bm_free_data
	// Just clear our tracking map
	m_textures.clear();

	// Clean up any pending texture deletions that haven't been processed yet
	for (auto& queue : m_pendingTextureDeletions) {
		for (auto* tex : queue) {
			tex->destroy();
			delete tex;
		}
		queue.clear();
	}

	// Destroy upload fences
	for (auto& fence : m_uploadFences) {
		if (fence) {
			m_device.destroyFence(fence);
			fence = nullptr;
		}
	}

	m_samplerCache.shutdown();

	if (m_stagingMapped) {
		m_device.unmapMemory(m_stagingMemory.get());
		m_stagingMapped = nullptr;
	}

	m_stagingMemory.reset();
	m_stagingBuffer.reset();

	// Free command buffers
	if (m_commandPool) {
		SCP_vector<vk::CommandBuffer> cmdsToFree;
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			if (m_uploadCmds[i]) {
				cmdsToFree.push_back(m_uploadCmds[i]);
				m_uploadCmds[i] = nullptr;
			}
			m_uploadCmdRecording[i] = false;
		}
		if (!cmdsToFree.empty()) {
			m_device.freeCommandBuffers(m_commandPool, cmdsToFree);
		}
	}

	m_initialized = false;
}

bool VulkanTextureManager::createTexture(int bitmapHandle, const bitmap* bm)
{
	if (!m_initialized || !bm) {
		return false;
	}

	// Check if already exists
	if (m_textures.find(bitmapHandle) != m_textures.end()) {
		return true; // Already created
	}

	vk::Format format = selectFormat(bm);
	uint32_t mipLevels = 1;

	// Only calculate mip levels for non-compressed textures that don't have precomputed mipmaps
	bitmap_entry* entry = bm_get_entry(bitmapHandle);
	if (entry && entry->num_mipmaps > 1) {
		mipLevels = static_cast<uint32_t>(entry->num_mipmaps);
	} else if (!isCompressedFormat(format) && canGenerateMipmaps(format)) {
		mipLevels = calculateMipLevels(bm->w, bm->h);
	}

	auto* texture = new VulkanTexture();
	if (!texture->create(m_device, m_physicalDevice, bm->w, bm->h, format, mipLevels)) {
		delete texture;
		return false;
	}

	m_textures[bitmapHandle] = texture;

	// Store in bitmap slot for bmpman integration
	bitmap_slot* slot = bm_get_slot(bitmapHandle);
	if (slot) {
		slot->gr_info = texture;
	}

	return true;
}

bool VulkanTextureManager::uploadTextureData(int bitmapHandle, const bitmap* bm)
{
	if (!m_initialized || !bm || !bm->data) {
		return false;
	}

	auto it = m_textures.find(bitmapHandle);
	if (it == m_textures.end()) {
		// Create texture first if not exists
		if (!createTexture(bitmapHandle, bm)) {
			return false;
		}
		it = m_textures.find(bitmapHandle);
	}

	VulkanTexture* texture = it->second;
	vk::Format format = texture->getFormat();
	uint32_t textureMipLevels = texture->getMipLevels();
	bool isCompressed = isCompressedFormat(format);

	// Handle 24-bit -> 32-bit conversion (only for uncompressed)
	bool needsConversion = (bm->bpp == 24) && !isCompressed;
	bool needsPaletteExpansion = (bm->bpp == 8) && !(bm->flags & BMP_AABITMAP) && !isCompressed;

	// For compressed textures with precomputed mips, calculate total size of all mip levels
	// For uncompressed, we only upload base level and optionally generate mipmaps
	size_t totalDataSize = 0;
	uint32_t mipWidth = static_cast<uint32_t>(bm->w);
	uint32_t mipHeight = static_cast<uint32_t>(bm->h);

	if (isCompressed && textureMipLevels > 1) {
		// Calculate total size of all precomputed mip levels
		for (uint32_t mip = 0; mip < textureMipLevels; mip++) {
			totalDataSize += calculateMipSize(mipWidth, mipHeight, format);
			mipWidth = std::max(1u, mipWidth >> 1);
			mipHeight = std::max(1u, mipHeight >> 1);
		}
	} else if (needsConversion || needsPaletteExpansion) {
		totalDataSize = static_cast<size_t>(bm->w) * bm->h * 4;
	} else {
		totalDataSize = calculateMipSize(bm->w, bm->h, format);
	}

	// Allocate staging memory
	vk::DeviceSize stagingOffset = 0;
	void* stagingPtr = allocateStagingMemory(totalDataSize, stagingOffset);
	if (!stagingPtr) {
		mprintf(("Vulkan: Failed to allocate staging memory for texture upload\n"));
		return false;
	}

	// Copy/convert data to staging buffer
	if (needsPaletteExpansion) {
		// Palettized 8bpp texture: expand to RGBA using bitmap palette
		const uint8_t* src = reinterpret_cast<const uint8_t*>(bm->data);
		uint8_t* dst = static_cast<uint8_t*>(stagingPtr);
		const uint8_t* pal = bm->palette; // Expected 3 bytes per entry (PCX stores palette as RGB)

		// Graceful fallback: if palette is missing, treat index as grayscale to avoid crashes
		static bool warnedMissingPalette = false;
		if (!pal) {
			if (!warnedMissingPalette) {
				mprintf(("Vulkan: 8bpp texture without palette, falling back to grayscale expansion (handle=%d, %dx%d)\n",
					bitmapHandle, bm->w, bm->h));
				warnedMissingPalette = true;
			}
		}

		const size_t pixelCount = static_cast<size_t>(bm->w) * bm->h;
		for (size_t i = 0; i < pixelCount; i++) {
			const uint8_t idx = src[i];
			if (pal) {
				const size_t palOffset = static_cast<size_t>(idx) * 3;
				// Convert RGB palette entry to BGRA texel expected by vk::Format::eB8G8R8A8Unorm
				dst[i * 4 + 0] = pal[palOffset + 2]; // B
				dst[i * 4 + 1] = pal[palOffset + 1]; // G
				dst[i * 4 + 2] = pal[palOffset + 0]; // R
			} else {
				dst[i * 4 + 0] = idx;
				dst[i * 4 + 1] = idx;
				dst[i * 4 + 2] = idx;
			}
			dst[i * 4 + 3] = 255;
		}
	} else if (needsConversion) {
		// BGR -> BGRA conversion (bmpman 24-bit data is BGR in memory)
		const uint8_t* src = reinterpret_cast<const uint8_t*>(bm->data);
		uint8_t* dst = static_cast<uint8_t*>(stagingPtr);
		int pixelCount = bm->w * bm->h;
		for (int i = 0; i < pixelCount; i++) {
			dst[i * 4 + 0] = src[i * 3 + 0]; // B
			dst[i * 4 + 1] = src[i * 3 + 1]; // G
			dst[i * 4 + 2] = src[i * 3 + 2]; // R
			dst[i * 4 + 3] = 255;             // A
		}
	} else {
		auto* dstPtr = static_cast<uint8_t*>(stagingPtr);
		auto* srcPtr = reinterpret_cast<const uint8_t*>(bm->data);
		memcpy(dstPtr, srcPtr, totalDataSize);
	}

	// Get command buffer for this frame's uploads (starts recording if needed)
	vk::CommandBuffer cmd = getUploadCommandBuffer();

	// Transition entire image to transfer dst
	texture->transitionLayout(cmd,
	                          vk::ImageLayout::eUndefined,
	                          vk::ImageLayout::eTransferDstOptimal);

	if (isCompressed && textureMipLevels > 1) {
		// Upload all precomputed mip levels for compressed textures
		mipWidth = static_cast<uint32_t>(bm->w);
		mipHeight = static_cast<uint32_t>(bm->h);
		vk::DeviceSize mipOffset = stagingOffset;

		for (uint32_t mip = 0; mip < textureMipLevels; mip++) {
			size_t mipSize = calculateMipSize(mipWidth, mipHeight, format);

			texture->uploadData(cmd, m_stagingBuffer.get(),
			                    mipOffset, mipSize, mip, 0);

			mipOffset += mipSize;
			mipWidth = std::max(1u, mipWidth >> 1);
			mipHeight = std::max(1u, mipHeight >> 1);
		}

		// All mips uploaded, transition to shader read
		texture->transitionLayout(cmd,
		                          vk::ImageLayout::eTransferDstOptimal,
		                          vk::ImageLayout::eShaderReadOnlyOptimal);
	} else {
		// Upload base mip level only
		size_t baseMipSize = needsConversion
		                         ? static_cast<size_t>(bm->w) * bm->h * 4
		                         : needsPaletteExpansion ? static_cast<size_t>(bm->w) * bm->h * 4
		                         : calculateMipSize(bm->w, bm->h, format);

		texture->uploadData(cmd, m_stagingBuffer.get(),
		                    stagingOffset, baseMipSize, 0, 0);

		// Generate mipmaps or transition to shader read
		if (textureMipLevels > 1 && canGenerateMipmaps(format)) {
			texture->generateMipmaps(cmd);
		} else {
			texture->transitionLayout(cmd,
			                          vk::ImageLayout::eTransferDstOptimal,
			                          vk::ImageLayout::eShaderReadOnlyOptimal);
		}
	}

	// Upload commands are batched and submitted later via submitUploads()
	return true;
}

void VulkanTextureManager::destroyTexture(int bitmapHandle)
{
	auto it = m_textures.find(bitmapHandle);
	if (it != m_textures.end()) {
		// Texture is owned by bitmap_slot::gr_info, will be deleted by caller
		m_textures.erase(it);
	}
}

VulkanTexture* VulkanTextureManager::getTexture(int bitmapHandle)
{
	auto it = m_textures.find(bitmapHandle);
	return (it != m_textures.end()) ? it->second : nullptr;
}

void VulkanTextureManager::beginFrame(uint32_t frameIndex)
{
	// Crash-safe logging
	FILE* f = fopen("vulkan_debug.log", "a");
	if (f) {
		fprintf(f, "VulkanTextureManager::beginFrame: frameIndex=%u, pendingDeletions=%zu\n",
			frameIndex, m_pendingTextureDeletions[frameIndex].size());
		fflush(f);
		fclose(f);
	}

	m_currentFrameIndex = frameIndex;

	// Only wait on upload fence if we actually submitted work last time
	if (m_uploadFenceSubmitted[frameIndex]) {
		FILE* flog = fopen("vulkan_debug.log", "a");
		if (flog) {
			fprintf(flog, "VulkanTextureManager::beginFrame: waiting on upload fence[%u]\n", frameIndex);
			fflush(flog);
			fclose(flog);
		}
		auto waitResult = m_device.waitForFences(m_uploadFences[frameIndex], true, std::numeric_limits<uint64_t>::max());
		if (waitResult != vk::Result::eSuccess) {
			mprintf(("VulkanTextureManager: WARNING - waitForFences returned %d\n", static_cast<int>(waitResult)));
		}
		m_device.resetFences(m_uploadFences[frameIndex]);
		m_uploadFenceSubmitted[frameIndex] = false;
	} else {
		FILE* flog = fopen("vulkan_debug.log", "a");
		if (flog) {
			fprintf(flog, "VulkanTextureManager::beginFrame: no upload fence wait needed for frame %u\n", frameIndex);
			fflush(flog);
			fclose(flog);
		}
	}

	// Process deferred texture deletions for this frame
	// Safe because we've waited on this frame's fence, so GPU is done using these textures
	for (auto* tex : m_pendingTextureDeletions[frameIndex]) {
		f = fopen("vulkan_debug.log", "a");
		if (f) {
			fprintf(f, "VulkanTextureManager::beginFrame: deleting texture=%p\n", reinterpret_cast<void*>(tex));
			fflush(f);
			fclose(f);
		}
		tex->destroy();
		delete tex;
	}
	m_pendingTextureDeletions[frameIndex].clear();

	// Reset staging offset to this frame's partition start
	// This is safe because we've waited on this frame's fence, so the GPU
	// is done with any uploads that used this partition
	m_stagingOffset = frameIndex * STAGING_PARTITION_SIZE;

	// Reset recording state for this frame's command buffer
	m_uploadCmdRecording[frameIndex] = false;
}

void VulkanTextureManager::queueTextureForDeletion(VulkanTexture* texture)
{
	if (texture) {
		// Remove from m_textures map to prevent dangling pointer lookups
		// The texture pointer is still valid until actually deleted in beginFrame()
		for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
			if (it->second == texture) {
				// Crash-safe logging
				FILE* f = fopen("vulkan_debug.log", "a");
				if (f) {
					fprintf(f, "queueTextureForDeletion: removing handle %d from m_textures\n", it->first);
					fflush(f);
					fclose(f);
				}
				m_textures.erase(it);
				break;
			}
		}

		// Crash-safe logging
		FILE* f = fopen("vulkan_debug.log", "a");
		if (f) {
			fprintf(f, "queueTextureForDeletion: tex=%p queued to frame %u (queue now has %zu entries)\n",
				reinterpret_cast<void*>(texture), m_currentFrameIndex,
				m_pendingTextureDeletions[m_currentFrameIndex].size() + 1);
			fflush(f);
			fclose(f);
		}
		m_pendingTextureDeletions[m_currentFrameIndex].push_back(texture);
	}
}

void VulkanTextureManager::submitUploads()
{
	// Crash-safe logging
	FILE* f = fopen("vulkan_debug.log", "a");
	if (f) {
		fprintf(f, "VulkanTextureManager::submitUploads: frame=%u, recording=%d\n",
			m_currentFrameIndex, m_uploadCmdRecording[m_currentFrameIndex] ? 1 : 0);
		fflush(f);
		fclose(f);
	}

	if (!m_uploadCmdRecording[m_currentFrameIndex]) {
		return; // No uploads recorded this frame
	}

	try {
		// End recording
		m_uploadCmds[m_currentFrameIndex].end();

		// Submit to graphics queue (no fence - uses frame fence for synchronization)
		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_uploadCmds[m_currentFrameIndex];

		// Submit with per-frame upload fence (fence is unsignaled since we track submission state)
		m_transferQueue.submit(submitInfo, m_uploadFences[m_currentFrameIndex]);
		m_uploadFenceSubmitted[m_currentFrameIndex] = true;  // Mark fence as needing wait

		f = fopen("vulkan_debug.log", "a");
		if (f) {
			fprintf(f, "VulkanTextureManager::submitUploads: submit succeeded, fence submitted\n");
			fflush(f);
			fclose(f);
		}
	} catch (const vk::SystemError& e) {
		f = fopen("vulkan_debug.log", "a");
		if (f) {
			fprintf(f, "VulkanTextureManager::submitUploads: EXCEPTION: %s\n", e.what());
			fflush(f);
			fclose(f);
		}
		throw;
	}

	m_uploadCmdRecording[m_currentFrameIndex] = false;
}

void VulkanTextureManager::ensureUploadRecording()
{
	if (m_uploadCmdRecording[m_currentFrameIndex]) {
		return; // Already recording
	}

	// Reset and begin recording
	m_uploadCmds[m_currentFrameIndex].reset(vk::CommandBufferResetFlags{});
	m_uploadCmds[m_currentFrameIndex].begin(vk::CommandBufferBeginInfo{
	    vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

	m_uploadCmdRecording[m_currentFrameIndex] = true;
}

vk::CommandBuffer VulkanTextureManager::getUploadCommandBuffer()
{
	ensureUploadRecording();
	return m_uploadCmds[m_currentFrameIndex];
}

bool VulkanTextureManager::canGenerateMipmaps(vk::Format format) const
{
	if (isCompressedFormat(format)) {
		return false;
	}

	vk::FormatProperties props = m_physicalDevice.getFormatProperties(format);
	return (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear) &&
	       (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) &&
	       (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst);
}

vk::Format VulkanTextureManager::selectFormat(const bitmap* bm) const
{
	// Compressed formats (direct upload, no conversion)
	if (bm->flags & BMP_TEX_DXT1) {
		return vk::Format::eBc1RgbaUnormBlock;
	}
	if (bm->flags & BMP_TEX_DXT3) {
		return vk::Format::eBc2UnormBlock;
	}
	if (bm->flags & BMP_TEX_DXT5) {
		return vk::Format::eBc3UnormBlock;
	}
	// BMP_TEX_BC7 check - may not be defined in older FSO
#ifdef BMP_TEX_BC7
	if (bm->flags & BMP_TEX_BC7) {
		return vk::Format::eBc7UnormBlock;
	}
#endif

	// Uncompressed based on bpp
	// bmpman stores pixels as BGRA in memory (matches OpenGL's GL_BGRA + GL_UNSIGNED_INT_8_8_8_8_REV)
	// Use BGRA format so Vulkan interprets the bytes correctly.
	// The blit shader then swaps R/B for the swapchain, making everything consistent.
	// DO NOT CHANGE THIS - the blit shader R/B swap depends on this being BGRA.
	switch (bm->bpp) {
		case 32:
			return vk::Format::eB8G8R8A8Unorm;
		case 24:
			return vk::Format::eB8G8R8A8Unorm;
		case 16:
			return vk::Format::eR5G6B5UnormPack16;
		case 8:
			return (bm->flags & BMP_AABITMAP) ? vk::Format::eR8Unorm : vk::Format::eB8G8R8A8Unorm;
		default:
			return vk::Format::eB8G8R8A8Unorm;
	}
}

uint32_t VulkanTextureManager::calculateMipLevels(uint32_t width, uint32_t height) const
{
	return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

size_t VulkanTextureManager::calculateMipSize(uint32_t width, uint32_t height, vk::Format format) const
{
	if (isCompressedFormat(format)) {
		// Block-compressed formats use 4x4 blocks
		uint32_t blockWidth = (width + 3) / 4;
		uint32_t blockHeight = (height + 3) / 4;
		size_t blockSize = 0;

		switch (format) {
			case vk::Format::eBc1RgbaUnormBlock:
			case vk::Format::eBc1RgbUnormBlock:
				blockSize = 8; // 64 bits per block
				break;
			case vk::Format::eBc2UnormBlock:
			case vk::Format::eBc3UnormBlock:
			case vk::Format::eBc7UnormBlock:
				blockSize = 16; // 128 bits per block
				break;
			default:
				blockSize = 16;
				break;
		}

		return static_cast<size_t>(blockWidth) * blockHeight * blockSize;
	}

	// Uncompressed
	size_t bytesPerPixel = 4; // Default RGBA8
	switch (format) {
		case vk::Format::eR8Unorm:
			bytesPerPixel = 1;
			break;
		case vk::Format::eR5G6B5UnormPack16:
			bytesPerPixel = 2;
			break;
		case vk::Format::eR8G8B8A8Unorm:
		case vk::Format::eB8G8R8A8Unorm:
			bytesPerPixel = 4;
			break;
		default:
			bytesPerPixel = 4;
			break;
	}

	return static_cast<size_t>(width) * height * bytesPerPixel;
}

bool VulkanTextureManager::isCompressedFormat(vk::Format format) const
{
	switch (format) {
		case vk::Format::eBc1RgbUnormBlock:
		case vk::Format::eBc1RgbaUnormBlock:
		case vk::Format::eBc2UnormBlock:
		case vk::Format::eBc3UnormBlock:
		case vk::Format::eBc7UnormBlock:
			return true;
		default:
			return false;
	}
}

void* VulkanTextureManager::allocateStagingMemory(vk::DeviceSize size, vk::DeviceSize& outOffset)
{
	// Calculate partition bounds for current frame
	vk::DeviceSize partitionStart = m_currentFrameIndex * STAGING_PARTITION_SIZE;
	vk::DeviceSize partitionEnd = partitionStart + STAGING_PARTITION_SIZE;

	// Calculate aligned next offset
	vk::DeviceSize alignedNext = (m_stagingOffset + size + 3) & ~vk::DeviceSize(3);

	// Check if allocation fits in current frame's partition
	if (alignedNext > partitionEnd) {
		// Partition full - cannot allocate without blocking
		// Caller should submit pending uploads and wait for next frame
		mprintf(("Vulkan: Staging buffer partition full (need %llu bytes, %llu available)\n",
		         static_cast<unsigned long long>(size),
		         static_cast<unsigned long long>(partitionEnd - m_stagingOffset)));
		return nullptr;
	}

	// Capture offset before advancing
	outOffset = m_stagingOffset;
	void* ptr = static_cast<uint8_t*>(m_stagingMapped) + m_stagingOffset;
	m_stagingOffset += size;

	// Align to 4 bytes for next allocation
	m_stagingOffset = (m_stagingOffset + 3) & ~vk::DeviceSize(3);

	return ptr;
}

// ============================================================================
// Static Utility Functions (for unit testing)
// ============================================================================

uint32_t VulkanTextureManager::calculateMipLevelsStatic(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

size_t VulkanTextureManager::calculateMipSizeStatic(uint32_t width, uint32_t height, vk::Format format)
{
	if (isCompressedFormatStatic(format)) {
		// Block-compressed formats use 4x4 blocks
		uint32_t blockWidth = (width + 3) / 4;
		uint32_t blockHeight = (height + 3) / 4;
		size_t blockSize = 0;

		switch (format) {
			case vk::Format::eBc1RgbaUnormBlock:
			case vk::Format::eBc1RgbUnormBlock:
				blockSize = 8; // 64 bits per block
				break;
			case vk::Format::eBc2UnormBlock:
			case vk::Format::eBc3UnormBlock:
			case vk::Format::eBc7UnormBlock:
				blockSize = 16; // 128 bits per block
				break;
			default:
				blockSize = 16;
				break;
		}

		return static_cast<size_t>(blockWidth) * blockHeight * blockSize;
	}

	// Uncompressed
	size_t bytesPerPixel = 4; // Default RGBA8
	switch (format) {
		case vk::Format::eR8Unorm:
			bytesPerPixel = 1;
			break;
		case vk::Format::eR5G6B5UnormPack16:
			bytesPerPixel = 2;
			break;
		case vk::Format::eR8G8B8A8Unorm:
		case vk::Format::eB8G8R8A8Unorm:
			bytesPerPixel = 4;
			break;
		case vk::Format::eR16G16B16A16Sfloat:
			bytesPerPixel = 8;
			break;
		default:
			bytesPerPixel = 4;
			break;
	}

	return static_cast<size_t>(width) * height * bytesPerPixel;
}

bool VulkanTextureManager::isCompressedFormatStatic(vk::Format format)
{
	switch (format) {
		case vk::Format::eBc1RgbUnormBlock:
		case vk::Format::eBc1RgbaUnormBlock:
		case vk::Format::eBc2UnormBlock:
		case vk::Format::eBc3UnormBlock:
		case vk::Format::eBc7UnormBlock:
			return true;
		default:
			return false;
	}
}

// ============================================================================
// Render Target Implementation
// ============================================================================

vk::UniqueRenderPass VulkanTextureManager::createRenderTargetRenderPass(vk::Format colorFormat, bool withDepth, bool isCubemap)
{
	(void)isCubemap; // Cubemap uses same render pass structure, just different framebuffer

	SCP_vector<vk::AttachmentDescription> attachments;

	// Color attachment
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = colorFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	attachments.push_back(colorAttachment);

	// Depth attachment (optional)
	vk::AttachmentDescription depthAttachment;
	if (withDepth) {
		depthAttachment.format = vk::Format::eD24UnormS8Uint;
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		attachments.push_back(depthAttachment);
	}

	// Subpass
	vk::AttachmentReference colorRef;
	colorRef.attachment = 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthRef;
	if (withDepth) {
		depthRef.attachment = 1;
		depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = withDepth ? &depthRef : nullptr;

	// Dependencies
	std::array<vk::SubpassDependency, 2> dependencies;
	
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
	dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	if (withDepth) {
		dependencies[0].dstStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependencies[0].dstAccessMask |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	try {
		return m_device.createRenderPassUnique(renderPassInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create render target render pass: %s\n", e.what()));
		return {};
	}
}

int VulkanTextureManager::createRenderTarget(int handle, int* width, int* height, int* bpp, int* mm_lvl, int flags)
{
	if (!m_initialized) {
		mprintf(("Vulkan: Texture manager not initialized\n"));
		return 0;
	}

	// Validate parameters
	if (!width || !height) {
		return 0;
	}

	bool isCubemap = (flags & BMP_FLAG_CUBEMAP) != 0;
	bool withMipmaps = (flags & BMP_FLAG_RENDER_TARGET_MIPMAP) != 0;
	bool isStatic = (flags & BMP_FLAG_RENDER_TARGET_STATIC) != 0;

	// Clamp dimensions to device limits
	vk::PhysicalDeviceProperties props = m_physicalDevice.getProperties();
	uint32_t maxDim = props.limits.maxImageDimension2D;
	if (static_cast<uint32_t>(*width) > maxDim) *width = static_cast<int>(maxDim);
	if (static_cast<uint32_t>(*height) > maxDim) *height = static_cast<int>(maxDim);

	// Select format - render targets use RGBA8
	vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;
	if (bpp) *bpp = 32;

	// Calculate mip levels
	uint32_t mipLevels = 1;
	if (withMipmaps) {
		mipLevels = calculateMipLevels(*width, *height);
	}
	if (mm_lvl) *mm_lvl = static_cast<int>(mipLevels);

	// Create the texture with render target usage
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment |
	                            vk::ImageUsageFlagBits::eSampled |
	                            vk::ImageUsageFlagBits::eTransferSrc |
	                            vk::ImageUsageFlagBits::eTransferDst;

	uint32_t arrayLayers = isCubemap ? 6 : 1;
	vk::ImageCreateFlags imageFlags = isCubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags{};

	// Create texture
	auto texture = new VulkanTexture();
	
	// Need special create for cubemap
	vk::ImageCreateInfo imageInfo{};
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = colorFormat;
	imageInfo.extent = vk::Extent3D{static_cast<uint32_t>(*width), static_cast<uint32_t>(*height), 1};
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.flags = imageFlags;

	if (!texture->create(m_device, m_physicalDevice, *width, *height, colorFormat, mipLevels, arrayLayers, usage)) {
		delete texture;
		mprintf(("Vulkan: Failed to create render target texture\n"));
		return 0;
	}

	// Store texture in bmpman
	auto* slot = bm_get_slot(handle, true);
	if (slot) {
		if (slot->gr_info) {
			delete static_cast<VulkanTexture*>(slot->gr_info);
		}
		slot->gr_info = texture;
	}
	m_textures[handle] = texture;

	// Create render target structure
	auto rt = std::make_unique<VulkanRenderTarget>();
	rt->isCubemap = isCubemap;
	rt->isStatic = isStatic;
	rt->workingHandle = handle;

	// Create render pass for this RT (with depth for 3D rendering)
	rt->renderPass = createRenderTargetRenderPass(colorFormat, true, isCubemap);
	if (!rt->renderPass) {
		m_textures.erase(handle);
		delete texture;
		return 0;
	}

	if (isCubemap) {
		// Create per-face image views and framebuffers
		for (int face = 0; face < 6; ++face) {
			vk::ImageViewCreateInfo viewInfo;
			viewInfo.image = texture->getImage();
			viewInfo.viewType = vk::ImageViewType::e2D;
			viewInfo.format = colorFormat;
			viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = face;
			viewInfo.subresourceRange.layerCount = 1;

			try {
				rt->cubeFaceViews[face] = m_device.createImageViewUnique(viewInfo);
			} catch (const vk::SystemError& e) {
				mprintf(("Vulkan: Failed to create cubemap face %d view: %s\n", face, e.what()));
				m_textures.erase(handle);
				delete texture;
				return 0;
			}

			// Create framebuffer for this face
			rt->cubeFaceFramebuffers[face] = std::make_unique<VulkanFramebuffer>();
			SCP_vector<vk::ImageView> views = {rt->cubeFaceViews[face].get()};
			
			if (!rt->cubeFaceFramebuffers[face]->createFromImageViews(
				m_device, rt->renderPass.get(), *width, *height, views, nullptr)) {
				mprintf(("Vulkan: Failed to create cubemap face %d framebuffer\n", face));
				m_textures.erase(handle);
				delete texture;
				return 0;
			}
		}
	} else {
		// Create single framebuffer for 2D render target
		rt->framebuffer = std::make_unique<VulkanFramebuffer>();
		SCP_vector<vk::Format> colorFormats = {colorFormat};
		
		if (!rt->framebuffer->create(m_device, m_physicalDevice, rt->renderPass.get(),
			*width, *height, colorFormats, vk::Format::eD24UnormS8Uint)) {
			mprintf(("Vulkan: Failed to create render target framebuffer\n"));
			m_textures.erase(handle);
			delete texture;
			return 0;
		}
	}

	mprintf(("Vulkan: Created render target %d (%dx%d, %s%s)\n",
		handle, *width, *height,
		isCubemap ? "cubemap" : "2D",
		withMipmaps ? ", mipmapped" : ""));

	m_renderTargets[handle] = std::move(rt);
	return 1;
}

int VulkanTextureManager::setRenderTarget(int handle, int face)
{
	if (!m_initialized) {
		return 0;
	}

	// Handle -1 = restore default (scene framebuffer)
	if (handle < 0) {
		if (m_activeRenderTarget != nullptr) {
			// Generate mipmaps if needed
			VulkanTexture* tex = getTexture(m_activeRenderTargetHandle);
			if (tex && tex->getMipLevels() > 1) {
				// TODO: Generate mipmaps - requires command buffer
				// For now, skip this
			}
		}

		m_activeRenderTarget = nullptr;
		m_activeRenderTargetHandle = -1;
		return 1;
	}

	// Find render target
	auto it = m_renderTargets.find(handle);
	if (it == m_renderTargets.end()) {
		mprintf(("Vulkan: Render target %d not found\n", handle));
		return 0;
	}

	VulkanRenderTarget* rt = it->second.get();
	
	// For cubemaps, validate and store face
	if (rt->isCubemap) {
		if (face < 0 || face > 5) {
			mprintf(("Vulkan: Invalid cubemap face %d\n", face));
			return 0;
		}
		rt->activeFace = face;
	}

	rt->workingHandle = handle;
	m_activeRenderTarget = rt;
	m_activeRenderTargetHandle = handle;

	return 1;
}

VulkanRenderTarget* VulkanTextureManager::getRenderTarget(int handle)
{
	auto it = m_renderTargets.find(handle);
	if (it != m_renderTargets.end()) {
		return it->second.get();
	}
	return nullptr;
}

void VulkanTextureManager::destroyRenderTarget(int handle)
{
	auto it = m_renderTargets.find(handle);
	if (it != m_renderTargets.end()) {
		// If this is the active RT, deactivate it
		if (m_activeRenderTargetHandle == handle) {
			m_activeRenderTarget = nullptr;
			m_activeRenderTargetHandle = -1;
		}
		m_renderTargets.erase(it);
	}

	// Also remove from texture map (but don't delete - bmpman owns it)
	m_textures.erase(handle);
}

void VulkanTextureManager::readbackTexture(void* data_out, int handle)
{
	if (!data_out || !m_initialized) {
		return;
	}

	VulkanTexture* tex = getTexture(handle);
	if (!tex || !tex->isValid()) {
		mprintf(("Vulkan: Cannot read back texture %d - not found or invalid\n", handle));
		return;
	}

	uint32_t width = tex->getWidth();
	uint32_t height = tex->getHeight();
	vk::Format format = tex->getFormat();

	// Calculate data size
	size_t dataSize = calculateMipSize(width, height, format);
	if (dataSize == 0) {
		return;
	}

	// Create staging buffer for readback
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = dataSize;
	bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	vk::UniqueBuffer stagingBuffer;
	vk::UniqueDeviceMemory stagingMemory;

	try {
		stagingBuffer = m_device.createBufferUnique(bufferInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create readback staging buffer: %s\n", e.what()));
		return;
	}

	// Allocate memory
	vk::MemoryRequirements memReqs = m_device.getBufferMemoryRequirements(stagingBuffer.get());
	
	vk::PhysicalDeviceMemoryProperties memProps = m_physicalDevice.getMemoryProperties();
	uint32_t memTypeIndex = UINT32_MAX;
	vk::MemoryPropertyFlags desiredProps = vk::MemoryPropertyFlagBits::eHostVisible |
	                                       vk::MemoryPropertyFlagBits::eHostCoherent;
	
	for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
		if ((memReqs.memoryTypeBits & (1 << i)) &&
			(memProps.memoryTypes[i].propertyFlags & desiredProps) == desiredProps) {
			memTypeIndex = i;
			break;
		}
	}

	if (memTypeIndex == UINT32_MAX) {
		mprintf(("Vulkan: Failed to find suitable memory for readback\n"));
		return;
	}

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = memTypeIndex;

	try {
		stagingMemory = m_device.allocateMemoryUnique(allocInfo);
		m_device.bindBufferMemory(stagingBuffer.get(), stagingMemory.get(), 0);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to allocate readback memory: %s\n", e.what()));
		return;
	}

	// Create command buffer for copy
	vk::CommandBufferAllocateInfo cmdAllocInfo;
	cmdAllocInfo.commandPool = m_commandPool;
	cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;
	cmdAllocInfo.commandBufferCount = 1;

	std::vector<vk::CommandBuffer> cmdBuffers;
	try {
		cmdBuffers = m_device.allocateCommandBuffers(cmdAllocInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to allocate readback command buffer: %s\n", e.what()));
		return;
	}

	vk::CommandBuffer cmd = cmdBuffers[0];

	// Record copy command
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	cmd.begin(beginInfo);

	// Transition image to transfer src
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = tex->getCurrentLayout();
	barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = tex->getImage();
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
	barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eTransfer,
		{}, {}, {}, barrier);

	// Copy image to buffer
	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D{0, 0, 0};
	region.imageExtent = vk::Extent3D{width, height, 1};

	cmd.copyImageToBuffer(tex->getImage(), vk::ImageLayout::eTransferSrcOptimal,
		stagingBuffer.get(), region);

	// Transition image back to shader read
	barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader,
		{}, {}, {}, barrier);

	cmd.end();

	// Submit and wait
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	try {
		m_transferQueue.submit(submitInfo, nullptr);
		m_transferQueue.waitIdle();
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to submit readback commands: %s\n", e.what()));
		m_device.freeCommandBuffers(m_commandPool, cmdBuffers);
		return;
	}

	// Map and copy data
	void* mapped = nullptr;
	try {
		mapped = m_device.mapMemory(stagingMemory.get(), 0, dataSize);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to map readback memory: %s\n", e.what()));
		m_device.freeCommandBuffers(m_commandPool, cmdBuffers);
		return;
	}

	memcpy(data_out, mapped, dataSize);
	m_device.unmapMemory(stagingMemory.get());

	// Cleanup
	m_device.freeCommandBuffers(m_commandPool, cmdBuffers);

	// Update texture layout state
	tex->notifyLayoutChanged(vk::ImageLayout::eShaderReadOnlyOptimal);
}

// ============================================================================
// gr_screen Function Implementations
// ============================================================================

void gr_vulkan_bm_create(bitmap_slot* entry)
{
	// Called when a bitmap slot is first being used
	// Actual texture creation happens in bm_data when we have the pixel data
	(void)entry;
}

void gr_vulkan_bm_init(bitmap_slot* slot)
{
	if (!slot) {
		return;
	}

	// Initialize the gr_info pointer to null
	// The actual VulkanTexture object is created on-demand in gr_vulkan_bm_data
	// when texture data is uploaded for the first time
	slot->gr_info = nullptr;
}

bool gr_vulkan_bm_data(int handle, bitmap* bm)
{
	if (g_vulkanTextureManager) {
		return g_vulkanTextureManager->uploadTextureData(handle, bm);
	}
	return false;
}

void gr_vulkan_bm_free_data(bitmap_slot* slot, bool release)
{
	if (!slot) {
		return;
	}

	// The VulkanTexture is stored in gr_info
	// Use deferred deletion because the texture memory may still be referenced
	// by in-flight command buffers (e.g., upload commands not yet submitted/completed)
	if (slot->gr_info) {
		auto* texture = static_cast<VulkanTexture*>(slot->gr_info);
		if (g_vulkanTextureManager) {
			// Crash-safe logging for debugging
			FILE* f = fopen("vulkan_debug.log", "a");
			if (f) {
				fprintf(f, "gr_vulkan_bm_free_data: DEFERRED deletion for texture=%p\n",
					reinterpret_cast<void*>(texture));
				fflush(f);
				fclose(f);
			}
			// Queue for deferred deletion - will be deleted after GPU is done using it
			g_vulkanTextureManager->queueTextureForDeletion(texture);
		} else {
			// Crash-safe logging for debugging
			FILE* f = fopen("vulkan_debug.log", "a");
			if (f) {
				fprintf(f, "gr_vulkan_bm_free_data: IMMEDIATE deletion (no manager) for texture=%p\n",
					reinterpret_cast<void*>(texture));
				fflush(f);
				fclose(f);
			}
			// No manager available (shutdown), delete immediately
			texture->destroy();
			delete texture;
		}
		slot->gr_info = nullptr;
	}

	(void)release;
}

void gr_vulkan_update_texture(int handle, int bpp, const ubyte* data, int width, int height)
{
	// Partial texture update - TODO implement
	(void)handle; (void)bpp; (void)data; (void)width; (void)height;
	mprintf(("Vulkan: gr_vulkan_update_texture not yet implemented\n"));
}

int gr_vulkan_bm_make_render_target(int handle, int* width, int* height, int* bpp, int* mm_lvl, int flags)
{
	if (g_vulkanTextureManager) {
		return g_vulkanTextureManager->createRenderTarget(handle, width, height, bpp, mm_lvl, flags);
	}
	return 0;
}

int gr_vulkan_bm_set_render_target(int handle, int face)
{
	if (g_vulkanTextureManager) {
		return g_vulkanTextureManager->setRenderTarget(handle, face);
	}
	return 0;
}

void gr_vulkan_get_bitmap_from_texture(void* data_out, int handle)
{
	if (g_vulkanTextureManager) {
		g_vulkanTextureManager->readbackTexture(data_out, handle);
	}
}

void gr_vulkan_set_texture_addressing(int mode)
{
	// Texture addressing mode - affects sampler selection at bind time
	// This sets a global mode that affects subsequent texture binds
	(void)mode;
}

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
