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

	// Create upload command buffer
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = 1;

	try {
		auto cmdBuffers = device.allocateCommandBuffersUnique(allocInfo);
		m_uploadCommandBuffer = std::move(cmdBuffers[0]);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to allocate upload command buffer: %s\n", e.what()));
		return false;
	}

	// Create upload fence
	vk::FenceCreateInfo fenceInfo{};
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled; // Start signaled

	try {
		m_uploadFence = device.createFenceUnique(fenceInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create upload fence: %s\n", e.what()));
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
	mprintf(("Vulkan: Texture manager initialized (staging buffer: %zu MB)\n",
	         STAGING_BUFFER_SIZE / (1024 * 1024)));
	return true;
}

void VulkanTextureManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Wait for any pending uploads
	if (m_uploadFence) {
		(void)m_device.waitForFences(m_uploadFence.get(), VK_TRUE, UINT64_MAX);
	}

	// Textures are owned by bitmap_slot::gr_info and destroyed via gr_vulkan_bm_free_data
	// Just clear our tracking map
	m_textures.clear();

	m_samplerCache.shutdown();

	if (m_stagingMapped) {
		m_device.unmapMemory(m_stagingMemory.get());
		m_stagingMapped = nullptr;
	}

	m_stagingMemory.reset();
	m_stagingBuffer.reset();
	m_uploadFence.reset();
	m_uploadCommandBuffer.reset();

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
		if (!bm->palette) {
			mprintf(("Vulkan: 8bpp texture without palette is unsupported\n"));
			return false;
		}

		const uint8_t* src = reinterpret_cast<const uint8_t*>(bm->data);
		uint8_t* dst = static_cast<uint8_t*>(stagingPtr);
		const uint8_t* pal = bm->palette; // Expected 3 bytes per entry

		const size_t pixelCount = static_cast<size_t>(bm->w) * bm->h;
		for (size_t i = 0; i < pixelCount; i++) {
			const uint8_t idx = src[i];
			const size_t palOffset = static_cast<size_t>(idx) * 3;
			dst[i * 4 + 0] = pal[palOffset + 0];
			dst[i * 4 + 1] = pal[palOffset + 1];
			dst[i * 4 + 2] = pal[palOffset + 2];
			dst[i * 4 + 3] = 255;
		}
	} else if (needsConversion) {
		// RGB -> RGBA conversion
		const uint8_t* src = reinterpret_cast<const uint8_t*>(bm->data);
		uint8_t* dst = static_cast<uint8_t*>(stagingPtr);
		int pixelCount = bm->w * bm->h;
		for (int i = 0; i < pixelCount; i++) {
			dst[i * 4 + 0] = src[i * 3 + 0]; // R
			dst[i * 4 + 1] = src[i * 3 + 1]; // G
			dst[i * 4 + 2] = src[i * 3 + 2]; // B
			dst[i * 4 + 3] = 255;             // A
		}
	} else {
		std::memcpy(stagingPtr, reinterpret_cast<const void*>(bm->data), totalDataSize);
	}

	// Begin upload command buffer
	beginUpload();

	// Transition entire image to transfer dst
	texture->transitionLayout(m_uploadCommandBuffer.get(),
	                          vk::ImageLayout::eUndefined,
	                          vk::ImageLayout::eTransferDstOptimal);

	if (isCompressed && textureMipLevels > 1) {
		// Upload all precomputed mip levels for compressed textures
		mipWidth = static_cast<uint32_t>(bm->w);
		mipHeight = static_cast<uint32_t>(bm->h);
		vk::DeviceSize mipOffset = stagingOffset;

		for (uint32_t mip = 0; mip < textureMipLevels; mip++) {
			size_t mipSize = calculateMipSize(mipWidth, mipHeight, format);

			texture->uploadData(m_uploadCommandBuffer.get(), m_stagingBuffer.get(),
			                    mipOffset, mipSize, mip, 0);

			mipOffset += mipSize;
			mipWidth = std::max(1u, mipWidth >> 1);
			mipHeight = std::max(1u, mipHeight >> 1);
		}

		// All mips uploaded, transition to shader read
		texture->transitionLayout(m_uploadCommandBuffer.get(),
		                          vk::ImageLayout::eTransferDstOptimal,
		                          vk::ImageLayout::eShaderReadOnlyOptimal);
	} else {
		// Upload base mip level only
		size_t baseMipSize = needsConversion
		                         ? static_cast<size_t>(bm->w) * bm->h * 4
		                         : needsPaletteExpansion ? static_cast<size_t>(bm->w) * bm->h * 4
		                         : calculateMipSize(bm->w, bm->h, format);

		texture->uploadData(m_uploadCommandBuffer.get(), m_stagingBuffer.get(),
		                    stagingOffset, baseMipSize, 0, 0);

		// Generate mipmaps or transition to shader read
		if (textureMipLevels > 1 && canGenerateMipmaps(format)) {
			texture->generateMipmaps(m_uploadCommandBuffer.get());
		} else {
			texture->transitionLayout(m_uploadCommandBuffer.get(),
			                          vk::ImageLayout::eTransferDstOptimal,
			                          vk::ImageLayout::eShaderReadOnlyOptimal);
		}
	}

	// Submit upload
	endUpload();

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
	m_currentFrameIndex = frameIndex;
	// Could partition staging buffer by frame here for deadlock avoidance
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
	switch (bm->bpp) {
		case 32:
			return vk::Format::eR8G8B8A8Unorm;
		case 24:
			return vk::Format::eR8G8B8A8Unorm; // CPU converts RGB->RGBA before upload
		case 16:
			return vk::Format::eR5G6B5UnormPack16;
		case 8:
			return (bm->flags & BMP_AABITMAP) ? vk::Format::eR8Unorm : vk::Format::eR8G8B8A8Unorm;
		default:
			return vk::Format::eR8G8B8A8Unorm;
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
	const vk::DeviceSize alignedNext = (m_stagingOffset + size + 3) & ~vk::DeviceSize(3);
	if (alignedNext > STAGING_BUFFER_SIZE) {
		// Need to wrap - wait for previous uploads to complete
		(void)m_device.waitForFences(m_uploadFence.get(), VK_TRUE, UINT64_MAX);
		m_stagingOffset = 0;
	}

	// Capture offset before advancing
	outOffset = m_stagingOffset;
	void* ptr = static_cast<uint8_t*>(m_stagingMapped) + m_stagingOffset;
	m_stagingOffset += size;

	// Align to 4 bytes for next allocation
	m_stagingOffset = (m_stagingOffset + 3) & ~3;

	return ptr;
}

void VulkanTextureManager::beginUpload()
{
	if (m_uploadInProgress) {
		return;
	}

	// Wait for previous upload to complete
	(void)m_device.waitForFences(m_uploadFence.get(), VK_TRUE, UINT64_MAX);
	(void)m_device.resetFences(m_uploadFence.get());

	// Reset command buffer before re-recording
	m_uploadCommandBuffer->reset(vk::CommandBufferResetFlags{});

	m_uploadCommandBuffer->begin(vk::CommandBufferBeginInfo{
	    vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
	m_uploadInProgress = true;
}

void VulkanTextureManager::endUpload()
{
	if (!m_uploadInProgress) {
		return;
	}

	m_uploadCommandBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	vk::CommandBuffer cmdBuf = m_uploadCommandBuffer.get();
	submitInfo.pCommandBuffers = &cmdBuf;

	m_transferQueue.submit(submitInfo, m_uploadFence.get());
	m_uploadInProgress = false;
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
	// Initialize texture slot
	(void)slot;
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

	// The VulkanTexture is stored in gr_info - just delete it directly
	// (similar to OpenGL's approach with tcache_slot_opengl)
	if (slot->gr_info) {
		auto* texture = static_cast<VulkanTexture*>(slot->gr_info);
		texture->destroy();
		delete texture;
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
	// Render target creation - TODO implement in Phase 10
	(void)handle; (void)width; (void)height; (void)bpp; (void)mm_lvl; (void)flags;
	mprintf(("Vulkan: gr_vulkan_bm_make_render_target not yet implemented\n"));
	return 0;
}

int gr_vulkan_bm_set_render_target(int handle, int face)
{
	// Render target binding - TODO implement in Phase 10
	(void)handle; (void)face;
	mprintf(("Vulkan: gr_vulkan_bm_set_render_target not yet implemented\n"));
	return 0;
}

void gr_vulkan_get_bitmap_from_texture(void* data_out, int handle)
{
	// Readback - TODO implement
	(void)data_out; (void)handle;
	mprintf(("Vulkan: gr_vulkan_get_bitmap_from_texture not yet implemented\n"));
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
