#include "VulkanTexture.h"

#include <algorithm>
#include "VulkanBarrier.h"
#include "VulkanBuffer.h"
#include "VulkanDeletionQueue.h"
#include "VulkanRenderer.h"
#include "gr_vulkan.h"

#include "graphics/util/pixel_swizzle.h"
#include "bmpman/bmpman.h"
#include "ddsutils/ddsutils.h"
#include "ddsutils/bcdec.h"
#include "globalincs/systemvars.h"
#include "ktxutils/ktxutils.h"

namespace graphics::vulkan {

namespace {
VulkanTextureManager* g_textureManager = nullptr;

// Geometry/format description shared by every texture-upload path (single 2D,
// animation array, cubemap). Captures exactly what the staging-size, copy-region
// and per-mip math need, so those calculations live in one place instead of
// being copy-pasted into each upload function.
struct TextureUploadLayout {
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t mipLevels = 1;
	bool isCompressed = false;
	size_t blockSize = 0;          // compressed block size (DXT1=8, others=16)
	size_t dstBytesPerPixel = 0;   // uncompressed destination bpp (24bpp stored as 4)
};

// Byte size of a single mip level for one layer.
size_t mipLevelSize(const TextureUploadLayout& l, uint32_t mipW, uint32_t mipH)
{
	if (l.isCompressed) {
		// Works for DDS and KTX
		return dds_compressed_mip_size(static_cast<int>(mipW), static_cast<int>(mipH),
			static_cast<int>(l.blockSize));
	}
	return static_cast<size_t>(mipW) * mipH * l.dstBytesPerPixel;
}

// Get compressed block size for DDS and KTX textures
size_t get_compressed_block_size(int compType)
{
	switch (compType) {
	// DDS
	case DDS_DXT1:
	case DDS_CUBEMAP_DXT1:
	case DDS_DXT3:
	case DDS_CUBEMAP_DXT3:
	case DDS_DXT5:
	case DDS_CUBEMAP_DXT5:
	case DDS_BC7:
		return dds_block_size(compType);
	// KTX
	case KTX_ETC2_RGB:
	case KTX_ETC2_SRGB:
	case KTX_ETC2_RGB_A1:
	case KTX_ETC2_SRGB_A1:
	case KTX_ETC2_RGBA_EAC:
	case KTX_ETC2_SRGBA_EAC:
		return ktx_etc_block_size(ktx_map_ktx_format_to_gl_internal(compType));

	default:
		return 0;
	}
}

// Total bytes occupied by one layer (all mip levels). Matches the staging
// layout produced by appendLayerCopyRegions().
size_t layerByteSize(const TextureUploadLayout& l)
{
	size_t total = 0;
	uint32_t mipW = l.width;
	uint32_t mipH = l.height;
	for (uint32_t m = 0; m < l.mipLevels; ++m) {
		total += mipLevelSize(l, mipW, mipH);
		mipW = std::max(1u, mipW / 2);
		mipH = std::max(1u, mipH / 2);
	}
	return total;
}

// Append one vk::BufferImageCopy per mip level for a single array layer/face,
// starting at layerBufferOffset. Returns the number of bytes the layer occupies
// (so callers can advance their staging offset). The regions depend only on the
// layout, not on the pixel data, so this is safe to call even when a frame's
// data could not be locked.
size_t appendLayerCopyRegions(SCP_vector<vk::BufferImageCopy>& regions,
	const TextureUploadLayout& l, uint32_t layerIndex, size_t layerBufferOffset)
{
	uint32_t mipW = l.width;
	uint32_t mipH = l.height;
	size_t offset = layerBufferOffset;
	for (uint32_t m = 0; m < l.mipLevels; ++m) {
		vk::BufferImageCopy region;
		region.bufferOffset = static_cast<vk::DeviceSize>(offset);
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = m;
		region.imageSubresource.baseArrayLayer = layerIndex;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(mipW, mipH, 1);
		regions.push_back(region);

		offset += mipLevelSize(l, mipW, mipH);
		mipW = std::max(1u, mipW / 2);
		mipH = std::max(1u, mipH / 2);
	}
	return offset - layerBufferOffset;
}

} // namespace

VulkanTextureManager* getTextureManager()
{
	Assertion(g_textureManager != nullptr, "Vulkan TextureManager not initialized!");
	return g_textureManager;
}

void setTextureManager(VulkanTextureManager* manager)
{
	g_textureManager = manager;
}

// tcache_slot_vulkan implementation

void tcache_slot_vulkan::reset()
{
	image = nullptr;
	imageView = nullptr;
	allocation = VulkanAllocation();
	format = vk::Format::eUndefined;
	currentLayout = vk::ImageLayout::eUndefined;
	width = 0;
	height = 0;
	mipLevels = 1;
	arrayLayers = 1;
	bpp = 0;
	bitmapHandle = -1;
	arrayIndex = 0;
	used = false;
	framebuffer = nullptr;
	framebufferView = nullptr;
	renderPass = nullptr;
	renderPassLoad = nullptr;
	isRenderTarget = false;
	depthImage = nullptr;
	depthImageView = nullptr;
	depthAllocation = VulkanAllocation();
	is3D = false;
	depth = 1;
	isCubemap = false;
	for (auto& v : cubeFaceViews) v = nullptr;
	for (auto& fb : cubeFaceFramebuffers) fb = nullptr;
	uScale = 1.0f;
	vScale = 1.0f;
}

// VulkanTextureManager implementation

VulkanTextureManager::VulkanTextureManager() = default;

VulkanTextureManager::~VulkanTextureManager()
{
	if (m_initialized) {
		shutdown();
	}
}

bool VulkanTextureManager::init(vk::Device device, vk::PhysicalDevice physicalDevice,
                                VulkanMemoryManager* memoryManager,
                                vk::CommandPool commandPool, vk::Queue graphicsQueue)
{
	if (m_initialized) {
		nprintf(("vulkan", "VulkanTextureManager::init called when already initialized!\n"));
		return false;
	}

	m_device = device;
	m_physicalDevice = physicalDevice;
	m_memoryManager = memoryManager;
	m_commandPool = commandPool;
	m_graphicsQueue = graphicsQueue;

	// Query device limits
	auto properties = physicalDevice.getProperties();
	m_maxTextureSize = properties.limits.maxImageDimension2D;
	m_maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	nprintf(("vulkan", "Vulkan Texture Manager initialized\n"));
	nprintf(("vulkan", "  Max texture size: %u\n", m_maxTextureSize));
	nprintf(("vulkan", "  Max anisotropy: %.1f\n", m_maxAnisotropy));

	// Create default sampler
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	// Use ClampToEdge by default to match OpenGL's behavior for UI/interface textures.
	// OpenGL creates all textures with GL_CLAMP_TO_EDGE and only switches to GL_REPEAT
	// for 3D model textures at bind time (excluding AABITMAP, INTERFACE, CUBEMAP types).
	// Using eRepeat here causes visible 1-pixel seams on UI bitmaps where edge texels
	// blend with the opposite edge via linear filtering.
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.anisotropyEnable = (m_maxAnisotropy > 1.0f);
	samplerInfo.maxAnisotropy = m_maxAnisotropy;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

	try {
		m_defaultSampler = m_device.createSampler(samplerInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "Failed to create default sampler: %s\n", e.what()));
		return false;
	}

	// Create 1x1 white fallback textures for unbound descriptor slots
	if (!createFallbackTexture(m_fallback2DArrayTexture, m_fallback2DArrayAllocation,
	                           m_fallback2DArrayView, ImageViewType::Array2D)) {
		return false;
	}
	if (!createFallbackTexture(m_fallbackTexture2D, m_fallbackTexture2DAllocation,
	                           m_fallbackTextureView2D, ImageViewType::Plain2D)) {
		return false;
	}
	if (!createFallbackTexture(m_fallbackCubeTexture, m_fallbackCubeAllocation,
	                           m_fallbackCubeView, ImageViewType::Cube, 6, true)) {
		return false;
	}
	if (!createFallbackTexture(m_fallback3DTexture, m_fallback3DAllocation,
	                           m_fallback3DView, ImageViewType::Volume3D, 1, false, vk::ImageType::e3D)) {
		return false;
	}

	// Check BCx and ETC2 Support
	auto features = m_physicalDevice.getFeatures();
	bool supportsETC2 = features.textureCompressionETC2;
	bool supportsBC = features.textureCompressionBC;

	if (!supportsETC2) {
		std::array<vk::Format, 6> etcFormats = {vk::Format::eEtc2R8G8B8UnormBlock,
			vk::Format::eEtc2R8G8B8SrgbBlock,
			vk::Format::eEtc2R8G8B8A1UnormBlock,
			vk::Format::eEtc2R8G8B8A1SrgbBlock,
			vk::Format::eEtc2R8G8B8A8UnormBlock,
			vk::Format::eEtc2R8G8B8A8SrgbBlock};

		for (auto fmt : etcFormats) {
			auto props = m_physicalDevice.getFormatProperties(fmt);
			if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) {
				supportsETC2 = true;
				break;
			}
		}
	}

	if (!supportsBC) {
		std::array<vk::Format, 4> bcFormats = {
			vk::Format::eBc1RgbaUnormBlock, // DXT1
			vk::Format::eBc2UnormBlock,     // DXT3
			vk::Format::eBc3UnormBlock,     // DXT5
			vk::Format::eBc7UnormBlock      // BC7
		};

		for (auto fmt : bcFormats) {
			auto props = m_physicalDevice.getFormatProperties(fmt);
			if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) {
				supportsBC = true;
				break;
			}
		}
	}

	mprintf(("VulkanTextureManager: ETC2 Texture Support = %s\n", supportsETC2 ? "YES" : "NO"));
	mprintf(("VulkanTextureManager: BCn Texture Support = %s\n", supportsBC ? "YES" : "NO"));

	m_initialized = true;
	return true;
}

void VulkanTextureManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Destroy fallback 3D texture
	if (m_fallback3DView) {
		m_device.destroyImageView(m_fallback3DView);
		m_fallback3DView = nullptr;
	}
	if (m_fallback3DTexture) {
		m_device.destroyImage(m_fallback3DTexture);
		m_fallback3DTexture = nullptr;
	}
	if (m_fallback3DAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_fallback3DAllocation);
	}

	// Destroy fallback cubemap
	if (m_fallbackCubeView) {
		m_device.destroyImageView(m_fallbackCubeView);
		m_fallbackCubeView = nullptr;
	}
	if (m_fallbackCubeTexture) {
		m_device.destroyImage(m_fallbackCubeTexture);
		m_fallbackCubeTexture = nullptr;
	}
	if (m_fallbackCubeAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_fallbackCubeAllocation);
	}

	// Destroy fallback textures
	if (m_fallbackTextureView2D) {
		m_device.destroyImageView(m_fallbackTextureView2D);
		m_fallbackTextureView2D = nullptr;
	}
	if (m_fallbackTexture2D) {
		m_device.destroyImage(m_fallbackTexture2D);
		m_fallbackTexture2D = nullptr;
	}
	if (m_fallbackTexture2DAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_fallbackTexture2DAllocation);
	}
	if (m_fallback2DArrayView) {
		m_device.destroyImageView(m_fallback2DArrayView);
		m_fallback2DArrayView = nullptr;
	}
	if (m_fallback2DArrayTexture) {
		m_device.destroyImage(m_fallback2DArrayTexture);
		m_fallback2DArrayTexture = nullptr;
	}
	if (m_fallback2DArrayAllocation.isValid()) {
		m_memoryManager->freeAllocation(m_fallback2DArrayAllocation);
	}

	// Destroy samplers
	if (m_defaultSampler) {
		m_device.destroySampler(m_defaultSampler);
		m_defaultSampler = nullptr;
	}

	for (auto& pair : m_samplerCache) {
		m_device.destroySampler(pair.second);
	}
	m_samplerCache.clear();

	m_initialized = false;
	nprintf(("vulkan", "Vulkan Texture Manager shutdown\n"));
}

bool VulkanTextureManager::releaseAnimationSlotRef(tcache_slot_vulkan* ts)
{
	// Non-array textures own their image outright — nothing to ref-count.
	if (ts->arrayLayers <= 1 || ts->bitmapHandle < 0) {
		return true;
	}

	// For shared animation texture arrays: mark this frame as unused, then check
	// whether any other frame still references the shared image. We compute the
	// base frame from slot data (bitmapHandle - arrayIndex) rather than calling
	// bm_get_base_frame(), because during shutdown/mission-unload the bitmap
	// entries may already be cleaned up, causing bm_get_base_frame() to return -1.
	// That would skip ref-counting and every frame slot would independently queue
	// the same shared resources for destruction (double-free).
	ts->used = false;

	int baseFrame = ts->bitmapHandle - static_cast<int>(ts->arrayIndex);
	int numFrames = static_cast<int>(ts->arrayLayers);
	vk::Image sharedImage = ts->image;

	bool anyInUse = false;
	for (int f = baseFrame; f < baseFrame + numFrames; f++) {
		if (f == ts->bitmapHandle) {
			continue;  // skip self (already marked unused)
		}
		auto* fSlot = bm_get_slot(f, true);
		if (fSlot && fSlot->gr_info) {
			auto* fTs = static_cast<tcache_slot_vulkan*>(fSlot->gr_info);
			if (fTs->used && fTs->image == sharedImage) {
				anyInUse = true;
				break;
			}
		}
	}

	if (anyInUse) {
		// Other frames still reference the shared image — just detach this slot.
		ts->image = nullptr;
		ts->imageView = nullptr;
		ts->allocation = VulkanAllocation{};
		ts->reset();
		return false;
	}

	// Last live reference — caller destroys the shared image.
	return true;
}

void VulkanTextureManager::flushTextures() const
{
	if (!m_initialized) {
		return;
	}

	int flushed = 0;
	auto* deletionQueue = getDeletionQueue();

	for (auto& block : bm_blocks) {
		for (auto& slot : block) {
			if (!slot.gr_info) {
				continue;
			}

			auto* ts = static_cast<tcache_slot_vulkan*>(slot.gr_info);

			if (!ts->image) {
				continue;
			}

			// Skip render targets — they are managed by the post-processor
			// and scene texture system, not by the bitmap paging system
			if (ts->isRenderTarget) {
				continue;
			}

			// For shared animation texture arrays: only destroy the actual image
			// when no other frame references it (detaches this slot in place and
			// returns false otherwise).
			if (!releaseAnimationSlotRef(ts)) {
				continue;
			}

			// Queue deferred destruction of GPU resources
			if (ts->imageView) {
				deletionQueue->queueImageView(ts->imageView);
			}
			if (ts->image) {
				deletionQueue->queueImage(ts->image, ts->allocation);
			}

			ts->reset();
			flushed++;
		}
	}

	nprintf(("vulkan", "VulkanTextureManager: Flushed %d textures for level transition\n", flushed));
}

void VulkanTextureManager::bm_init(bitmap_slot* slot) const
{
	if (!m_initialized || !slot) {
		return;
	}

	// Allocate Vulkan-specific data
	if (slot->gr_info == nullptr) {
		slot->gr_info = new tcache_slot_vulkan();
	} else {
		static_cast<tcache_slot_vulkan*>(slot->gr_info)->reset();
	}
}

void VulkanTextureManager::bm_create(bitmap_slot* slot) const
{
	if (!m_initialized || !slot) {
		return;
	}

	// Ensure gr_info is allocated
	if (slot->gr_info == nullptr) {
		slot->gr_info = new tcache_slot_vulkan();
	}
}

void VulkanTextureManager::bm_free_data(bitmap_slot* slot, bool release) const
{
	if (!m_initialized || !slot || !slot->gr_info) {
		return;
	}

	auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);
	auto* deletionQueue = getDeletionQueue();

	// For shared animation texture arrays: if another frame still references the
	// shared image, detach this slot only (releaseAnimationSlotRef returns false);
	// otherwise fall through to destroy the shared image.
	if (!releaseAnimationSlotRef(ts)) {
		if (release) {
			delete ts;
			slot->gr_info = nullptr;
		}
		return;
	}

	// Queue resources for deferred destruction to avoid destroying
	// resources that may still be referenced by in-flight command buffers

	// Cubemap per-face framebuffers and views (must be before ts->framebuffer
	// since framebuffer may alias cubeFaceFramebuffers[0])
	for (auto& fb : ts->cubeFaceFramebuffers) {
		if (fb) {
			deletionQueue->queueFramebuffer(fb);
			fb = nullptr;
		}
	}
	for (auto& v : ts->cubeFaceViews) {
		if (v) {
			deletionQueue->queueImageView(v);
			v = nullptr;
		}
	}
	// If framebuffer was aliased to cubeFaceFramebuffers[0], it's already cleaned up
	if (ts->isCubemap) {
		ts->framebuffer = nullptr;
	}

	if (ts->framebuffer) {
		deletionQueue->queueFramebuffer(ts->framebuffer);
		ts->framebuffer = nullptr;
	}

	if (ts->renderPass) {
		deletionQueue->queueRenderPass(ts->renderPass);
		ts->renderPass = nullptr;
	}

	if (ts->renderPassLoad) {
		deletionQueue->queueRenderPass(ts->renderPassLoad);
		ts->renderPassLoad = nullptr;
	}

	if (ts->imageView) {
		deletionQueue->queueImageView(ts->imageView);
		ts->imageView = nullptr;
	}

	if (ts->framebufferView) {
		deletionQueue->queueImageView(ts->framebufferView);
		ts->framebufferView = nullptr;
	}

	if (ts->depthImageView) {
		deletionQueue->queueImageView(ts->depthImageView);
		ts->depthImageView = nullptr;
	}

	if (ts->depthImage) {
		deletionQueue->queueImage(ts->depthImage, ts->depthAllocation);
		ts->depthImage = nullptr;
		ts->depthAllocation = VulkanAllocation{};  // Clear to prevent double-free
	}

	if (ts->image) {
		deletionQueue->queueImage(ts->image, ts->allocation);
		ts->image = nullptr;
		ts->allocation = VulkanAllocation{};  // Clear to prevent double-free
	}

	ts->reset();

	if (release) {
		delete ts;
		slot->gr_info = nullptr;
	}
}

bool VulkanTextureManager::uploadAnimationFrames(int handle, bitmap* bm, int compType,
                                                   int baseFrame, int numFrames)
{
	nprintf(("vulkan", "VulkanTexture: Uploading animation array: base=%d numFrames=%d triggered by handle=%d\n",
		baseFrame, numFrames, handle));

	// Get dimensions and format from the triggering frame's bitmap
	auto width = static_cast<uint32_t>(bm->w);
	auto height = static_cast<uint32_t>(bm->h);
	auto arrayLayerCount = static_cast<uint32_t>(numFrames);

	bool isCompressed = (compType == DDS_DXT1 || compType == DDS_DXT3 || compType == DDS_DXT5 || compType == DDS_BC7 ||
						 compType == KTX_ETC2_RGB || compType == KTX_ETC2_SRGB || compType == KTX_ETC2_RGBA_EAC ||
						 compType == KTX_ETC2_SRGBA_EAC || compType == KTX_ETC2_RGB_A1 || compType == KTX_ETC2_SRGB_A1);

	// Determine format
	vk::Format format;
	if (isCompressed) {
		format = bppToVkFormat(bm->bpp, true, compType);
	} else {
		format = bppToVkFormat(bm->bpp);
	}
	if (format == vk::Format::eUndefined) {
		nprintf(("vulkan", "VulkanTexture: uploadAnimationFrames: unsupported format bpp=%d compType=%d\n",
			bm->bpp, compType));
		return false;
	}

	// Each animation frame is one array layer; they all share this layout.
	uint32_t mipLevels = 1;
	if (isCompressed) {
		mipLevels = static_cast<uint32_t>(bm_get_num_mipmaps(handle));
		mipLevels = std::max<uint32_t>(mipLevels, 1);
	}

	TextureUploadLayout layout;
	layout.width = width;
	layout.height = height;
	layout.mipLevels = mipLevels;
	layout.isCompressed = isCompressed;
	layout.blockSize = isCompressed ? get_compressed_block_size(compType) : 0;
	layout.dstBytesPerPixel = (bm->bpp == 24) ? 4 : (bm->bpp / 8);

	size_t layerDataSize = layerByteSize(layout);
	size_t totalDataSize = layerDataSize * arrayLayerCount;

	// Create multi-layer image
	// eTransferSrc: animation frames can also be hit by the
	// get_bitmap_from_texture() readback (it copies a single array layer out
	// via eTransferSrcOptimal).
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
	                            vk::ImageUsageFlagBits::eTransferSrc;
	vk::Image image;
	VulkanAllocation allocation;

	if (!createImage(width, height, mipLevels, format, vk::ImageTiling::eOptimal,
	                 usage, MemoryUsage::GpuOnly, image, allocation, arrayLayerCount)) {
		nprintf(("vulkan", "VulkanTexture: uploadAnimationFrames: failed to create %ux%u x%d array image\n",
			width, height, numFrames));
		return false;
	}

	// Create multi-layer image view
	vk::ImageView imageView = createImageView(image, format,
		vk::ImageAspectFlagBits::eColor, mipLevels, ImageViewType::Array2D, arrayLayerCount);
	if (!imageView) {
		nprintf(("vulkan", "VulkanTexture: uploadAnimationFrames: failed to create image view\n"));
		m_device.destroyImage(image);
		m_memoryManager->freeAllocation(allocation);
		return false;
	}

	// Create staging buffer for all layers
	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAllocation;
	if (!createStagingBuffer(totalDataSize, stagingBuffer, stagingAllocation)) {
		m_device.destroyImageView(imageView);
		m_device.destroyImage(image);
		m_memoryManager->freeAllocation(allocation);
		return false;
	}

	void* mapped = m_memoryManager->mapMemory(stagingAllocation);
	if (!mapped) {
		m_memoryManager->freeAllocation(stagingAllocation);
		m_device.destroyBuffer(stagingBuffer);
		m_device.destroyImageView(imageView);
		m_device.destroyImage(image);
		m_memoryManager->freeAllocation(allocation);
		return false;
	}

	// Build per-layer copy regions and upload each frame's data
	SCP_vector<vk::BufferImageCopy> copyRegions;

	// Use the same lock parameters that were used for the triggering frame.
	// bm->flags contains the lock flags (BMP_AABITMAP, BMP_TEX_OTHER, BMP_TEX_DXT*, etc.)
	// bm->bpp contains the requested bpp. Using these ensures all frames are locked
	// consistently (e.g., 8bpp for aabitmaps, 32bpp for RGBA textures).
	auto lockBpp = bm->bpp;
	auto lockFlags = bm->flags;

	// Set guard flag to make recursive bm_data calls no-ops
	m_uploadingAnimation = true;

	for (int frame = baseFrame; frame < baseFrame + numFrames; frame++) {
		auto layerIndex = static_cast<uint32_t>(frame - baseFrame);
		size_t layerOffset = layerIndex * layerDataSize;
		uint8_t* dst = static_cast<uint8_t*>(mapped) + layerOffset;

		// Copy regions depend only on the layout, not the pixel data, so build
		// them up front — even a frame that fails to lock still occupies its layer.
		appendLayerCopyRegions(copyRegions, layout, layerIndex, layerOffset);

		bitmap* frameBm;
		bool needUnlock = false;

		if (frame == handle) {
			// This is the frame that triggered us — use the passed bitmap directly
			frameBm = bm;
		} else {
			// Lock this frame to get its data
			frameBm = bm_lock(frame, lockBpp, lockFlags);
			if (!frameBm) {
				nprintf(("vulkan", "VulkanTexture: uploadAnimationFrames: failed to lock frame %d\n", frame));
				// Fill with zeros to avoid undefined data
				memset(dst, 0, layerDataSize);
				continue;
			}
			needUnlock = true;
		}

		// Copy frame data to staging buffer
		if (!isCompressed && frameBm->bpp == 24) {
			graphics::util::expand_BGR_to_BGRA(reinterpret_cast<const uint8_t*>(frameBm->data), dst,
				static_cast<size_t>(width) * height);
		} else {
			memcpy(dst, reinterpret_cast<const void*>(frameBm->data), layerDataSize);
		}

		if (needUnlock) {
			bm_unlock(frame);
		}
	}

	m_uploadingAnimation = false;

	// Flush staging buffer
	m_memoryManager->flushMemory(stagingAllocation, 0, totalDataSize);
	m_memoryManager->unmapMemory(stagingAllocation);

	// Record transitions + copy and submit async
	vk::CommandBuffer cmd = beginSingleTimeCommands();
	recordUploadCommands(cmd, image, stagingBuffer, format, width, height,
	                     mipLevels, vk::ImageLayout::eUndefined, false, copyRegions,
	                     arrayLayerCount);
	submitUploadAsync(cmd, stagingBuffer, stagingAllocation);

	// Store shared image in ALL frame slots
	for (int frame = baseFrame; frame < baseFrame + numFrames; frame++) {
		int layerIndex = frame - baseFrame;
		auto* frameSlot = bm_get_slot(frame, true);
		if (!frameSlot) {
			continue;
		}
		if (!frameSlot->gr_info) {
			bm_init(frameSlot);
		}
		auto* ts = static_cast<tcache_slot_vulkan*>(frameSlot->gr_info);

		// Defer destruction of any existing image in this slot
		if (ts->image && ts->arrayLayers <= 1) {
			auto* deletionQueue = getDeletionQueue();
			if (ts->imageView) {
				deletionQueue->queueImageView(ts->imageView);
			}
			deletionQueue->queueImage(ts->image, ts->allocation);
		}

		ts->image = image;
		ts->imageView = imageView;
		ts->allocation = allocation;
		ts->width = width;
		ts->height = height;
		ts->format = format;
		ts->mipLevels = mipLevels;
		ts->bpp = bm->bpp;
		ts->arrayLayers = arrayLayerCount;
		ts->arrayIndex = static_cast<uint32_t>(layerIndex);
		ts->bitmapHandle = frame;
		ts->currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		ts->used = true;
		ts->uScale = 1.0f;
		ts->vScale = 1.0f;
	}

	nprintf(("vulkan", "VulkanTexture: Animation array uploaded: %ux%u x%d layers, %zu bytes total\n",
		width, height, numFrames, totalDataSize));
	return true;
}

bool VulkanTextureManager::uploadCubemap(int handle, bitmap* bm, int compType)
{
	nprintf(("vulkan", "VulkanTexture: Uploading cubemap: handle=%d w=%d h=%d compType=%d\n",
		handle, bm->w, bm->h, compType));

	auto* slot = bm_get_slot(handle, true);
	if (!slot) {
		return false;
	}
	if (!slot->gr_info) {
		bm_init(slot);
	}
	auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);

	auto faceW = static_cast<uint32_t>(bm->w);
	auto faceH = static_cast<uint32_t>(bm->h);

	// Map cubemap DDS compression types to base types
	int baseCompType = compType;
	if (compType == DDS_CUBEMAP_DXT1) baseCompType = DDS_DXT1;
	else if (compType == DDS_CUBEMAP_DXT3) baseCompType = DDS_DXT3;
	else if (compType == DDS_CUBEMAP_DXT5) baseCompType = DDS_DXT5;

	bool isCompressed = (baseCompType == DDS_DXT1 || baseCompType == DDS_DXT3 ||
	                     baseCompType == DDS_DXT5 || baseCompType == DDS_BC7 ||
	                     baseCompType == KTX_ETC2_RGB || baseCompType == KTX_ETC2_SRGB ||
	                     baseCompType == KTX_ETC2_RGBA_EAC || baseCompType == KTX_ETC2_SRGBA_EAC ||
	                     baseCompType == KTX_ETC2_RGB_A1 || baseCompType == KTX_ETC2_SRGB_A1);

	vk::Format format;
	if (isCompressed) {
		format = bppToVkFormat(bm->bpp, true, baseCompType);
	} else {
		format = bppToVkFormat(bm->bpp);
	}
	if (format == vk::Format::eUndefined) {
		nprintf(("vulkan", "VulkanTexture: uploadCubemap: unsupported format\n"));
		return false;
	}

	uint32_t mipLevels = 1;
	size_t blockSize = 0;

	if (isCompressed) {
		blockSize = get_compressed_block_size(baseCompType);
		mipLevels = static_cast<uint32_t>(bm_get_num_mipmaps(handle));
		mipLevels = std::max<uint32_t>(mipLevels, 1);
	}

	// A cubemap is six array layers; each face has the same layout.
	TextureUploadLayout layout;
	layout.width = faceW;
	layout.height = faceH;
	layout.mipLevels = mipLevels;
	layout.isCompressed = isCompressed;
	layout.blockSize = blockSize;
	layout.dstBytesPerPixel = (bm->bpp == 24) ? 4 : (bm->bpp / 8);

	size_t perFaceSize = layerByteSize(layout);
	size_t totalDataSize = perFaceSize * 6;

	// Defer destruction of existing resources
	if (ts->image) {
		auto* deletionQueue = getDeletionQueue();
		if (ts->imageView) {
			deletionQueue->queueImageView(ts->imageView);
			ts->imageView = nullptr;
		}
		deletionQueue->queueImage(ts->image, ts->allocation);
		ts->image = nullptr;
		ts->allocation = VulkanAllocation{};
	}

	// Create cubemap image (6 layers, eCubeCompatible)
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	if (!createImage(faceW, faceH, mipLevels, format, vk::ImageTiling::eOptimal,
	                 usage, MemoryUsage::GpuOnly, ts->image, ts->allocation, 6, true)) {
		nprintf(("vulkan", "VulkanTexture: uploadCubemap: failed to create cubemap image\n"));
		return false;
	}

	// Create cubemap image view (samplerCube)
	ts->imageView = createImageView(ts->image, format, vk::ImageAspectFlagBits::eColor,
	                                mipLevels, ImageViewType::Cube, 6);
	if (!ts->imageView) {
		nprintf(("vulkan", "VulkanTexture: uploadCubemap: failed to create cube image view\n"));
		m_device.destroyImage(ts->image);
		ts->image = nullptr;
		m_memoryManager->freeAllocation(ts->allocation);
		return false;
	}

	// Create staging buffer
	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAllocation;
	if (!createStagingBuffer(totalDataSize, stagingBuffer, stagingAllocation)) {
		m_device.destroyImageView(ts->imageView);
		ts->imageView = nullptr;
		m_device.destroyImage(ts->image);
		ts->image = nullptr;
		m_memoryManager->freeAllocation(ts->allocation);
		return false;
	}

	void* mapped = m_memoryManager->mapMemory(stagingAllocation);
	if (!mapped) {
		m_memoryManager->freeAllocation(stagingAllocation);
		m_device.destroyBuffer(stagingBuffer);
		return false;
	}

	// Copy data to staging buffer
	// DDS cubemap data layout: face0[mip0..mipN], face1[mip0..mipN], ..., face5[mip0..mipN]
	if (!isCompressed && bm->bpp == 24) {
		// Convert BGR to BGRA for all 6 faces
		graphics::util::expand_BGR_to_BGRA(reinterpret_cast<const uint8_t*>(bm->data),
			static_cast<uint8_t*>(mapped), static_cast<size_t>(faceW) * faceH * 6);
	} else {
		memcpy(mapped, reinterpret_cast<const void*>(bm->data), totalDataSize);
	}

	// Build per-face, per-mip copy regions (each face is one array layer)
	SCP_vector<vk::BufferImageCopy> copyRegions;
	size_t bufferOffset = 0;
	for (uint32_t face = 0; face < 6; face++) {
		bufferOffset += appendLayerCopyRegions(copyRegions, layout, face, bufferOffset);
	}

	m_memoryManager->flushMemory(stagingAllocation, 0, totalDataSize);
	m_memoryManager->unmapMemory(stagingAllocation);

	// Record transitions + copy and submit async
	vk::CommandBuffer cmd = beginSingleTimeCommands();
	recordUploadCommands(cmd, ts->image, stagingBuffer, format, faceW, faceH,
	                     mipLevels, vk::ImageLayout::eUndefined, false, copyRegions, 6);
	submitUploadAsync(cmd, stagingBuffer, stagingAllocation);

	// Update slot info
	ts->width = faceW;
	ts->height = faceH;
	ts->format = format;
	ts->mipLevels = mipLevels;
	ts->bpp = bm->bpp;
	ts->arrayLayers = 6;
	ts->bitmapHandle = handle;
	ts->currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	ts->used = true;
	ts->isCubemap = true;
	ts->uScale = 1.0f;
	ts->vScale = 1.0f;

	nprintf(("vulkan", "VulkanTexture: Cubemap uploaded: %ux%u, %u mips, format=%d\n",
		faceW, faceH, mipLevels, static_cast<int>(format)));
	return true;
}

bool VulkanTextureManager::upload3DTexture(int handle, bitmap* bm, int texDepth)
{
	auto* slot = bm_get_slot(handle, true);
	if (!slot) {
		return false;
	}

	if (!slot->gr_info) {
		bm_init(slot);
	}

	auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);

	auto width = static_cast<uint32_t>(bm->w);
	auto height = static_cast<uint32_t>(bm->h);
	auto depth3D = static_cast<uint32_t>(texDepth);

	// 3D textures are always 32bpp RGBA uncompressed, single mip
	vk::Format format = vk::Format::eR8G8B8A8Unorm;
	size_t dataSize = width * height * depth3D * 4;

	// Defer destruction of existing resources
	if (ts->image) {
		auto* deletionQueue = getDeletionQueue();
		if (ts->imageView) {
			deletionQueue->queueImageView(ts->imageView);
			ts->imageView = nullptr;
		}
		deletionQueue->queueImage(ts->image, ts->allocation);
		ts->image = nullptr;
		ts->allocation = VulkanAllocation{};
	}

	// Create 3D image
	if (!createImage(width, height, 1, format, vk::ImageTiling::eOptimal,
	                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	                 MemoryUsage::GpuOnly, ts->image, ts->allocation,
	                 1, false, depth3D, vk::ImageType::e3D)) {
		nprintf(("vulkan", "Failed to create 3D texture image!\n"));
		return false;
	}

	// Create 3D image view
	ts->imageView = createImageView(ts->image, format, vk::ImageAspectFlagBits::eColor, 1, ImageViewType::Volume3D);
	if (!ts->imageView) {
		nprintf(("vulkan", "Failed to create 3D texture image view!\n"));
		m_device.destroyImage(ts->image);
		ts->image = nullptr;
		m_memoryManager->freeAllocation(ts->allocation);
		return false;
	}

	// Create staging buffer
	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAllocation;
	if (!createStagingBuffer(dataSize, stagingBuffer, stagingAllocation)) {
		nprintf(("vulkan", "Failed to create staging buffer for 3D texture!\n"));
		return false;
	}

	// Copy data to staging buffer
	void* mapped = m_memoryManager->mapMemory(stagingAllocation);
	Assert(mapped);
	memcpy(mapped, reinterpret_cast<const void*>(bm->data), dataSize);
	m_memoryManager->flushMemory(stagingAllocation, 0, dataSize);
	m_memoryManager->unmapMemory(stagingAllocation);

	// Record transitions + copy and submit. Route through the shared
	// recordUploadCommands path with a single copy region carrying the full 3D
	// extent (depth = depth3D); the barrier sequence is identical to the 2D path.
	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D(0, 0, 0);
	region.imageExtent = vk::Extent3D(width, height, depth3D);

	vk::CommandBuffer cmd = beginSingleTimeCommands();
	recordUploadCommands(cmd, ts->image, stagingBuffer, format, width, height,
	                     1, vk::ImageLayout::eUndefined, false, {region});
	submitUploadAsync(cmd, stagingBuffer, stagingAllocation);

	// Update slot info
	ts->width = width;
	ts->height = height;
	ts->depth = depth3D;
	ts->is3D = true;
	ts->format = format;
	ts->mipLevels = 1;
	ts->bpp = 32;
	ts->bitmapHandle = handle;
	ts->currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	ts->used = true;
	ts->uScale = 1.0f;
	ts->vScale = 1.0f;

	nprintf(("vulkan", "VulkanTexture: 3D texture uploaded: %ux%ux%u, format=%d\n",
		width, height, depth3D, static_cast<int>(format)));
	return true;
}

bool VulkanTextureManager::bm_data(int handle, bitmap* bm, int compType)
{
	if (m_bmDataLogCount < 20) {
		nprintf(("vulkan", "VulkanTextureManager::bm_data #%d: handle=%d bm=%p bm->data=%p compType=%d\n",
			m_bmDataLogCount++, handle, bm, bm ? reinterpret_cast<void*>(bm->data) : nullptr, compType));
	}

	if (!m_initialized || !bm || !bm->data) {
		return false;
	}

	// Guard: nested bm_lock→bm_data calls during animation upload are no-ops
	if (m_uploadingAnimation) {
		return true;
	}

	// Detect animated texture arrays
	int numFrames = 0;
	int baseFrame = bm_get_base_frame(handle, &numFrames);
	if (baseFrame < 0) {
		return false;
	}

	if (numFrames > 1) {
		// Check if the shared image already exists (earlier frame created it)
		auto* baseSlot = bm_get_slot(baseFrame, true);
		if (baseSlot) {
			if (!baseSlot->gr_info) {
				bm_init(baseSlot);
			}
			auto* baseTs = static_cast<tcache_slot_vulkan*>(baseSlot->gr_info);
			if (baseTs->image && baseTs->arrayLayers == static_cast<uint32_t>(numFrames)) {
				// Share existing image with this frame's slot
				auto* slot = bm_get_slot(handle, true);
				if (!slot->gr_info) {
					bm_init(slot);
				}
				auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);
				ts->image = baseTs->image;
				ts->imageView = baseTs->imageView;
				ts->allocation = baseTs->allocation;
				ts->width = baseTs->width;
				ts->height = baseTs->height;
				ts->format = baseTs->format;
				ts->mipLevels = baseTs->mipLevels;
				ts->bpp = baseTs->bpp;
				ts->arrayLayers = baseTs->arrayLayers;
				ts->arrayIndex = static_cast<uint32_t>(handle - baseFrame);
				ts->bitmapHandle = handle;
				ts->currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				ts->used = true;
				return true;
			}
		}
		// First frame requested — create array and upload all frames
		return uploadAnimationFrames(handle, bm, compType, baseFrame, numFrames);
	}

	// Detect cubemap textures
	bool isCubemapUpload = (bm->flags & BMP_TEX_CUBEMAP) != 0;
	if (!isCubemapUpload) {
		// Also check compression type for cubemap DDS variants
		isCubemapUpload = (compType == DDS_CUBEMAP_DXT1 || compType == DDS_CUBEMAP_DXT3 ||
		                   compType == DDS_CUBEMAP_DXT5);
	}

	if (isCubemapUpload) {
		return uploadCubemap(handle, bm, compType);
	}

	// Detect 3D textures (volumetric data)
	if (bm->d > 1) {
		return upload3DTexture(handle, bm, bm->d);
	}

	return uploadTexture2D(handle, bm, compType);
}

bool VulkanTextureManager::uploadTexture2D(int handle, bitmap* bm, int compType)
{
	auto* slot = bm_get_slot(handle, true);
	if (!slot) {
		return false;
	}

	// Ensure slot is initialized
	if (!slot->gr_info) {
		bm_init(slot);
	}

	auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);

	auto width = static_cast<uint32_t>(bm->w);
	auto height = static_cast<uint32_t>(bm->h);
	uint32_t mipLevels = 1;
	bool autoGenerateMips = false;
	bool isCompressed = (compType == DDS_DXT1 || compType == DDS_DXT3 || compType == DDS_DXT5 || compType == DDS_BC7 ||
	                     compType == KTX_ETC2_RGB || compType == KTX_ETC2_SRGB || compType == KTX_ETC2_RGBA_EAC ||
	                     compType == KTX_ETC2_SRGBA_EAC || compType == KTX_ETC2_RGB_A1 || compType == KTX_ETC2_SRGB_A1);

	if (m_uploadFmtLogCount < 30) {
		nprintf(("vulkan", "VulkanTextureManager::bm_data: handle=%d w=%d h=%d bpp=%d true_bpp=%d flags=0x%x compType=%d\n",
			handle, bm->w, bm->h, bm->bpp, bm->true_bpp, bm->flags, compType));
		m_uploadFmtLogCount++;
	}

	// Determine format and data size
	vk::Format format;
	size_t dataSize;
	size_t blockSize = 0;
	SCP_vector<vk::BufferImageCopy> copyRegions;

	if (isCompressed) {
		format = bppToVkFormat(bm->bpp, true, compType);
		if (format == vk::Format::eUndefined) {
			nprintf(("vulkan", "VulkanTextureManager::bm_data: Unsupported compression type %d\n", compType));
			return false;
		}

		blockSize = get_compressed_block_size(compType);

		// Get pre-baked mipmap count from DDS file
		mipLevels = static_cast<uint32_t>(bm_get_num_mipmaps(handle));
		mipLevels = std::max<uint32_t>(mipLevels, 1);

		// Calculate total data size for all mip levels and build copy regions
		TextureUploadLayout layout;
		layout.width = width;
		layout.height = height;
		layout.mipLevels = mipLevels;
		layout.isCompressed = true;
		layout.blockSize = blockSize;
		dataSize = appendLayerCopyRegions(copyRegions, layout, 0, 0);
	} else {
		format = bppToVkFormat(bm->bpp);
		if (format == vk::Format::eUndefined) {
			nprintf(("vulkan", "VulkanTextureManager::bm_data: Unsupported bpp %d\n", bm->bpp));
			return false;
		}

		// 24bpp textures uploaded as 32bpp (Vulkan doesn't support 24bpp optimal tiling)
		size_t dstBytesPerPixel = (bm->bpp == 24) ? 4 : (bm->bpp / 8);
		dataSize = width * height * dstBytesPerPixel;

		// Auto-generate mipmaps for textures whose files originally had them.
		// This only triggers for uncompressed textures that were originally DDS
		// with mipmaps but got decompressed by a non-DDS lock path.
		if (width > 4 && height > 4) {
			int numMipmaps = bm_get_num_mipmaps(handle);
			if (numMipmaps > 1) {
				vk::FormatProperties fmtProps = m_physicalDevice.getFormatProperties(format);
				if ((fmtProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear) &&
				    (fmtProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) &&
				    (fmtProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst)) {
					mipLevels = calculateMipLevels(width, height);
					autoGenerateMips = true;
				}
			}
		}
	}

	// If the texture already exists with identical extent+format we could update
	// it in place (transition from ts->currentLayout, copy, transition back — the
	// exact pattern update_texture already uses) instead of orphaning the old
	// image and creating a fresh one. That in-place path is a behavior change with
	// a cross-frame write into a still-referenced image, so it's gated on evidence
	// that this condition actually fires often enough to matter. Log the first few
	// hits so a test run reveals the real trigger frequency; behavior is unchanged
	// (fall through to recreate).
	if (ts->image && ts->width == width && ts->height == height && ts->format == format &&
	    ts->arrayLayers <= 1) {
		if (m_reuploadLogCount < 20) {
			nprintf(("vulkan", "VulkanTextureManager::uploadTexture2D: re-upload of resident texture "
				"handle=%d (%ux%u) — recreating (in-place update not yet implemented)\n",
				handle, width, height));
			m_reuploadLogCount++;
		}
	}

	// Defer destruction of existing resources — they may still be referenced
	// by in-flight render or upload command buffers
	if (ts->image) {
		if (ts->arrayLayers > 1) {
			// Shared animation image — just clear references, don't destroy
			// (the image is shared with other frame slots)
			ts->imageView = nullptr;
			ts->image = nullptr;
			ts->allocation = VulkanAllocation{};
		} else {
			auto* deletionQueue = getDeletionQueue();
			if (ts->imageView) {
				deletionQueue->queueImageView(ts->imageView);
				ts->imageView = nullptr;
			}
			deletionQueue->queueImage(ts->image, ts->allocation);
			ts->image = nullptr;
			ts->allocation = VulkanAllocation{};
		}
	}

	// Create image
	// eTransferSrc is always set: needed for vkCmdBlitImage mipmap generation
	// (autoGenerateMips) and for the get_bitmap_from_texture() readback path
	// (bitmap_lookup alpha queries during model load), which copies the image
	// back to a buffer via eTransferSrcOptimal.
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
	                            vk::ImageUsageFlagBits::eTransferSrc;

	if (!createImage(width, height, mipLevels, format, vk::ImageTiling::eOptimal,
	                 usage, MemoryUsage::GpuOnly, ts->image, ts->allocation)) {
		nprintf(("vulkan", "Failed to create texture image!\n"));
		return false;
	}

	// Create image view (sampler2DArray for regular textures)
	ts->imageView = createImageView(ts->image, format, vk::ImageAspectFlagBits::eColor, mipLevels, ImageViewType::Array2D);
	if (!ts->imageView) {
		nprintf(("vulkan", "Failed to create texture image view!\n"));
		m_device.destroyImage(ts->image);
		ts->image = nullptr;
		m_memoryManager->freeAllocation(ts->allocation);
		return false;
	}

	// Create staging buffer
	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAllocation;
	if (!createStagingBuffer(dataSize, stagingBuffer, stagingAllocation)) {
		m_device.destroyImageView(ts->imageView);
		ts->imageView = nullptr;
		m_device.destroyImage(ts->image);
		ts->image = nullptr;
		m_memoryManager->freeAllocation(ts->allocation);
		return false;
	}

	// Copy data to staging buffer
	void* mapped = m_memoryManager->mapMemory(stagingAllocation);
	Assert(mapped);
	if (isCompressed) {
		// Compressed data: copy raw block data directly (includes all mip levels)
		memcpy(mapped, reinterpret_cast<const void*>(bm->data), dataSize);
	} else if (bm->bpp == 24) {
		graphics::util::expand_BGR_to_BGRA(reinterpret_cast<const uint8_t*>(bm->data),
			static_cast<uint8_t*>(mapped), static_cast<size_t>(width) * height);
	} else {
		memcpy(mapped, reinterpret_cast<const void*>(bm->data), dataSize);
	}
	m_memoryManager->flushMemory(stagingAllocation, 0, dataSize);
	m_memoryManager->unmapMemory(stagingAllocation);

	// Record transitions + copy (+ optional mipmap generation) and submit async
	vk::CommandBuffer cmd = beginSingleTimeCommands();
	recordUploadCommands(cmd, ts->image, stagingBuffer, format, width, height,
	                     mipLevels, vk::ImageLayout::eUndefined, autoGenerateMips, copyRegions);
	submitUploadAsync(cmd, stagingBuffer, stagingAllocation);

	// Update slot info
	ts->width = width;
	ts->height = height;
	ts->format = format;
	ts->mipLevels = mipLevels;
	ts->bpp = bm->bpp;
	ts->bitmapHandle = handle;
	ts->currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	ts->used = true;
	ts->uScale = 1.0f;
	ts->vScale = 1.0f;

	return true;
}

int VulkanTextureManager::bm_make_render_target(int handle, int* width, int* height,
                                                 int* bpp, int* mm_lvl, int flags)
{
	if (!m_initialized || !width || !height) {
		return 0;
	}

	// Clamp to max size
	if (static_cast<uint32_t>(*width) > m_maxTextureSize) {
		*width = static_cast<int>(m_maxTextureSize);
	}
	if (static_cast<uint32_t>(*height) > m_maxTextureSize) {
		*height = static_cast<int>(m_maxTextureSize);
	}

	auto* slot = bm_get_slot(handle, true);
	if (!slot) {
		return 0;
	}

	if (!slot->gr_info) {
		bm_init(slot);
	}

	auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);

	// Free any existing resources
	bm_free_data(slot, false);

	auto w = static_cast<uint32_t>(*width);
	auto h = static_cast<uint32_t>(*height);
	uint32_t mipLevels = 1;

	if (flags & BMP_FLAG_RENDER_TARGET_MIPMAP) {
		mipLevels = calculateMipLevels(w, h);
	}

	bool isCubemapRT = (flags & BMP_FLAG_CUBEMAP) != 0;
	uint32_t arrayLayers = isCubemapRT ? 6 : 1;
	vk::Format format = LDR_COLOR_FORMAT;

	// Create image for render target
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment |
	                            vk::ImageUsageFlagBits::eSampled |
	                            vk::ImageUsageFlagBits::eTransferSrc;

	if (flags & BMP_FLAG_RENDER_TARGET_MIPMAP) {
		usage |= vk::ImageUsageFlagBits::eTransferDst;  // For mipmap generation
	}

	if (!createImage(w, h, mipLevels, format, vk::ImageTiling::eOptimal,
	                 usage, MemoryUsage::GpuOnly, ts->image, ts->allocation, arrayLayers, isCubemapRT)) {
		nprintf(("vulkan", "Failed to create render target image!\n"));
		return 0;
	}

	if (isCubemapRT) {
		// Cubemap render target: create cube view for sampling + per-face 2D views for framebuffer
		ts->imageView = createImageView(ts->image, format, vk::ImageAspectFlagBits::eColor,
		                                mipLevels, ImageViewType::Cube, 6);
		if (!ts->imageView) {
			m_device.destroyImage(ts->image);
			ts->image = nullptr;
			m_memoryManager->freeAllocation(ts->allocation);
			return 0;
		}

		// Create per-face 2D views for framebuffer attachments
		for (size_t face = 0; face < ts->cubeFaceViews.size(); face++) {
			ts->cubeFaceViews[face] = createImageView(ts->image, format, vk::ImageAspectFlagBits::eColor,
			                                           1, ImageViewType::Plain2D, 1, static_cast<uint32_t>(face));
			if (!ts->cubeFaceViews[face]) {
				nprintf(("vulkan", "Failed to create cubemap face %zu view!\n", face));
				// Clean up previously created views
				for (size_t j = 0; j < face; j++) {
					m_device.destroyImageView(ts->cubeFaceViews[j]);
					ts->cubeFaceViews[j] = nullptr;
				}
				m_device.destroyImageView(ts->imageView);
				m_device.destroyImage(ts->image);
				ts->image = nullptr;
				ts->imageView = nullptr;
				m_memoryManager->freeAllocation(ts->allocation);
				return 0;
			}
		}
	} else {
		// Regular render target: array view for shader compatibility
		ts->imageView = createImageView(ts->image, format, vk::ImageAspectFlagBits::eColor, mipLevels, ImageViewType::Array2D);
		if (!ts->imageView) {
			m_device.destroyImage(ts->image);
			ts->image = nullptr;
			m_memoryManager->freeAllocation(ts->allocation);
			return 0;
		}

		// For mipmapped render targets, create a single-mip view for framebuffer use
		// (framebuffer attachments must have levelCount == 1)
		if (mipLevels > 1) {
			ts->framebufferView = createImageView(ts->image, format, vk::ImageAspectFlagBits::eColor, 1, ImageViewType::Array2D);
			if (!ts->framebufferView) {
				m_device.destroyImageView(ts->imageView);
				m_device.destroyImage(ts->image);
				ts->image = nullptr;
				ts->imageView = nullptr;
				m_memoryManager->freeAllocation(ts->allocation);
				return 0;
			}
		}
	}

	// Optional depth/stencil attachment. Without it, models rendered into the target
	// get no z-testing at all: the Vulkan spec ignores the pipeline's depth/stencil
	// state when the subpass has no depth attachment, so depth-tested geometry draws
	// in submission order. This mirrors the OpenGL FBO's depth renderbuffer. Cubemap
	// render targets (env/irradiance maps) never request this.
	const bool wantDepth = ((flags & BMP_FLAG_RENDER_TARGET_DEPTH_ATTACHMENT) != 0) && !isCubemapRT;
	vk::Format depthFormat = vk::Format::eUndefined;
	if (wantDepth) {
		depthFormat = getRendererInstance()->getDepthFormat();
		if (depthFormat == vk::Format::eUndefined) {
			Warning(LOCATION, "Vulkan render target requested a depth attachment before the depth format "
			                  "was initialized; z-buffering will be unavailable for this target.");
		} else if (!createImage(w, h, 1, depthFormat, vk::ImageTiling::eOptimal,
		                        vk::ImageUsageFlagBits::eDepthStencilAttachment, MemoryUsage::GpuOnly,
		                        ts->depthImage, ts->depthAllocation, 1, false)) {
			nprintf(("vulkan", "Failed to create render target depth image!\n"));
			depthFormat = vk::Format::eUndefined;
		} else {
			// Array2D view with a single layer matches the color framebuffer attachment.
			ts->depthImageView = createImageView(ts->depthImage, depthFormat,
			                                     imageAspectFromFormat(depthFormat), 1, ImageViewType::Array2D);
			if (!ts->depthImageView) {
				m_device.destroyImage(ts->depthImage);
				ts->depthImage = nullptr;
				m_memoryManager->freeAllocation(ts->depthAllocation);
				ts->depthAllocation = VulkanAllocation{};
				depthFormat = vk::Format::eUndefined;
			}
		}
	}
	const bool hasDepth = (depthFormat != vk::Format::eUndefined) && static_cast<bool>(ts->depthImage);

	// Create render pass for this target
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = format;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	// Depth attachment: cleared at every pass begin (loadOp=eClear) so the target
	// starts with a clean depth buffer regardless of what the caller does; contents
	// are never sampled afterward (storeOp=eDontCare). Layout stays in
	// eDepthStencilAttachmentOptimal across frames to avoid per-pass transitions.
	vk::AttachmentDescription depthAttachment;
	depthAttachment.format = depthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef;
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	if (hasDepth) {
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = hasDepth ? 2u : 1u;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	try {
		ts->renderPass = m_device.createRenderPass(renderPassInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "Failed to create render pass: %s\n", e.what()));
		if (hasDepth) {
			m_device.destroyImageView(ts->depthImageView);
			m_device.destroyImage(ts->depthImage);
			ts->depthImageView = nullptr;
			ts->depthImage = nullptr;
			m_memoryManager->freeAllocation(ts->depthAllocation);
			ts->depthAllocation = VulkanAllocation{};
		}
		m_device.destroyImageView(ts->imageView);
		m_device.destroyImage(ts->image);
		ts->image = nullptr;
		ts->imageView = nullptr;
		m_memoryManager->freeAllocation(ts->allocation);
		return 0;
	}

	// Load-variant of the render pass (color loadOp=eLoad) so a mid-frame readback can
	// resume rendering into this target without clearing what was already drawn. Only
	// flat targets are read back via gr.screenToBlob, so skip it for cubemaps.
	if (!isCubemapRT) {
		vk::AttachmentDescription loadColorAttachment = colorAttachment;
		loadColorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
		std::array<vk::AttachmentDescription, 2> loadAttachments = {loadColorAttachment, depthAttachment};

		vk::RenderPassCreateInfo loadPassInfo;
		loadPassInfo.attachmentCount = hasDepth ? 2u : 1u;
		loadPassInfo.pAttachments = loadAttachments.data();
		loadPassInfo.subpassCount = 1;
		loadPassInfo.pSubpasses = &subpass;

		try {
			ts->renderPassLoad = m_device.createRenderPass(loadPassInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "Failed to create load-variant render pass: %s\n", e.what()));
			m_device.destroyRenderPass(ts->renderPass);
			ts->renderPass = nullptr;
			if (hasDepth) {
				m_device.destroyImageView(ts->depthImageView);
				m_device.destroyImage(ts->depthImage);
				ts->depthImageView = nullptr;
				ts->depthImage = nullptr;
				m_memoryManager->freeAllocation(ts->depthAllocation);
				ts->depthAllocation = VulkanAllocation{};
			}
			m_device.destroyImageView(ts->imageView);
			m_device.destroyImage(ts->image);
			ts->image = nullptr;
			ts->imageView = nullptr;
			m_memoryManager->freeAllocation(ts->allocation);
			return 0;
		}
	}

	if (isCubemapRT) {
		// Create per-face framebuffers
		for (size_t face = 0; face < ts->cubeFaceFramebuffers.size(); face++) {
			vk::FramebufferCreateInfo framebufferInfo;
			framebufferInfo.renderPass = ts->renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &ts->cubeFaceViews[face];
			framebufferInfo.width = w;
			framebufferInfo.height = h;
			framebufferInfo.layers = 1;

			try {
				ts->cubeFaceFramebuffers[face] = m_device.createFramebuffer(framebufferInfo);
			} catch (const vk::SystemError& e) {
				nprintf(("vulkan", "Failed to create cubemap face %zu framebuffer: %s\n", face, e.what()));
				return 0;
			}
		}
		// Default framebuffer points to face 0
		ts->framebuffer = ts->cubeFaceFramebuffers[0];
	} else {
		// Create framebuffer
		// Use framebufferView (single-mip) if available, otherwise imageView.
		// The depth view (when present) is attachment 1, matching the render pass.
		std::array<vk::ImageView, 2> fbAttachments = {
			ts->framebufferView ? ts->framebufferView : ts->imageView,
			ts->depthImageView};
		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = ts->renderPass;
		framebufferInfo.attachmentCount = hasDepth ? 2u : 1u;
		framebufferInfo.pAttachments = fbAttachments.data();
		framebufferInfo.width = w;
		framebufferInfo.height = h;
		framebufferInfo.layers = 1;

		try {
			ts->framebuffer = m_device.createFramebuffer(framebufferInfo);
		} catch (const vk::SystemError& e) {
			nprintf(("vulkan", "Failed to create framebuffer: %s\n", e.what()));
			if (hasDepth) {
				m_device.destroyImageView(ts->depthImageView);
				m_device.destroyImage(ts->depthImage);
				ts->depthImageView = nullptr;
				ts->depthImage = nullptr;
				m_memoryManager->freeAllocation(ts->depthAllocation);
				ts->depthAllocation = VulkanAllocation{};
			}
			m_device.destroyRenderPass(ts->renderPass);
			if (ts->renderPassLoad) {
				m_device.destroyRenderPass(ts->renderPassLoad);
				ts->renderPassLoad = nullptr;
			}
			m_device.destroyImageView(ts->imageView);
			m_device.destroyImage(ts->image);
			ts->image = nullptr;
			ts->imageView = nullptr;
			ts->renderPass = nullptr;
			m_memoryManager->freeAllocation(ts->allocation);
			return 0;
		}
	}

	// Transition image to eShaderReadOnlyOptimal so it's in a valid layout
	// if sampled before being rendered into (render pass expects this initial layout)
	transitionImageLayout(ts->image, format, vk::ImageLayout::eUndefined,
	                      vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels, arrayLayers);

	// Put the depth image in the layout the render pass expects as its initial layout.
	// It stays there across frames (finalLayout matches), so no per-pass transition.
	if (hasDepth) {
		transitionImageLayout(ts->depthImage, depthFormat, vk::ImageLayout::eUndefined,
		                      vk::ImageLayout::eDepthStencilAttachmentOptimal, 1, 1);
	}

	// Update slot info
	ts->width = w;
	ts->height = h;
	ts->format = format;
	ts->mipLevels = mipLevels;
	ts->bpp = 32;
	ts->arrayLayers = arrayLayers;
	ts->bitmapHandle = handle;
	ts->isRenderTarget = true;
	ts->isCubemap = isCubemapRT;
	ts->used = true;
	ts->uScale = 1.0f;
	ts->vScale = 1.0f;
	ts->currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	if (bpp) {
		*bpp = 32;
	}
	if (mm_lvl) {
		*mm_lvl = static_cast<int>(mipLevels);
	}

	nprintf(("vulkan", "Created Vulkan render target: %ux%u\n", w, h));
	return 1;
}

int VulkanTextureManager::bm_set_render_target(int handle, int face)
{
	if (!m_initialized) {
		return 0;
	}

	auto* renderer = getRendererInstance();

	// handle < 0 means reset to default framebuffer
	if (handle < 0) {
		if (renderer->isRenderTargetActive()) {
			tcache_slot_vulkan* prevTs = nullptr;
			if (m_currentRenderTarget >= 0) {
				auto* prevSlot = bm_get_slot(m_currentRenderTarget, true);
				if (prevSlot && prevSlot->gr_info) {
					prevTs = static_cast<tcache_slot_vulkan*>(prevSlot->gr_info);
				}
			}
			renderer->endRenderTarget(prevTs);
		}
		m_currentRenderTarget = -1;
		return 1;
	}

	auto* slot = bm_get_slot(handle, true);
	if (!slot || !slot->gr_info) {
		return 0;
	}

	auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);
	if (!ts->isRenderTarget || !ts->framebuffer) {
		return 0;
	}

	renderer->beginRenderTarget(ts, face);
	m_currentRenderTarget = handle;

	return 1;
}

void VulkanTextureManager::update_texture(int bitmap_handle, int bpp, const ubyte* data,
                                          int width, int height)
{
	if (!m_initialized || !data) {
		return;
	}

	auto* slot = bm_get_slot(bitmap_handle, true);
	if (!slot || !slot->gr_info) {
		return;
	}

	auto* ts = static_cast<tcache_slot_vulkan*>(slot->gr_info);
	if (!ts->image) {
		return;
	}

	auto w = static_cast<uint32_t>(width);
	auto h = static_cast<uint32_t>(height);

	// Verify dimensions match existing texture
	if (ts->width != w || ts->height != h) {
		nprintf(("vulkan", "VulkanTextureManager::update_texture: Size mismatch (%ux%u vs %ux%u)\n",
			w, h, ts->width, ts->height));
		return;
	}

	// Use bppToVkFormat to determine format, matching how bm_data creates textures
	vk::Format format = bppToVkFormat(bpp);
	if (format == vk::Format::eUndefined) {
		nprintf(("vulkan", "VulkanTextureManager::update_texture: Unsupported bpp %d\n", bpp));
		return;
	}

	// The update path copies straight into the existing image with its baked-in
	// format; a bpp implying a different format than the texture was created with
	// would silently corrupt the copy. Reject the mismatch instead.
	if (format != ts->format) {
		nprintf(("vulkan", "VulkanTextureManager::update_texture: format mismatch for bpp %d "
			"(bitmap implies %d, texture is %d) — skipping update\n",
			bpp, static_cast<int>(format), static_cast<int>(ts->format)));
		return;
	}

	// Calculate staging buffer size (24bpp is uploaded as 32bpp BGRA)
	size_t srcBytesPerPixel = bpp / 8;
	size_t dstBytesPerPixel = (bpp == 24) ? 4 : srcBytesPerPixel;
	size_t dataSize = w * h * dstBytesPerPixel;

	// Create staging buffer
	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAllocation;
	if (!createStagingBuffer(dataSize, stagingBuffer, stagingAllocation)) {
		nprintf(("vulkan", "VulkanTextureManager::update_texture: Failed to create staging buffer\n"));
		return;
	}

	// Copy data to staging buffer
	void* mapped = m_memoryManager->mapMemory(stagingAllocation);
	Assert(mapped);
	if (bpp == 24) {
		graphics::util::expand_BGR_to_BGRA(data, static_cast<uint8_t*>(mapped), w * h);
	} else {
		memcpy(mapped, data, dataSize);
	}
	m_memoryManager->flushMemory(stagingAllocation, 0, dataSize);
	m_memoryManager->unmapMemory(stagingAllocation);

	// Record transitions + copy into a single command buffer and submit async
	vk::CommandBuffer cmd = beginSingleTimeCommands();
	recordUploadCommands(cmd, ts->image, stagingBuffer, format, w, h,
	                     ts->mipLevels, ts->currentLayout);
	submitUploadAsync(cmd, stagingBuffer, stagingAllocation);

	// Update layout tracking
	ts->currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

ubyte* VulkanTextureManager::get_bitmap_from_texture(int bitmap_num, int* width_out, int* height_out)
{
	if (!m_initialized || !width_out || !height_out) {
		return nullptr;
	}

	*width_out = 0;
	*height_out = 0;

	auto* ts = bm_get_gr_info<tcache_slot_vulkan>(bitmap_num, true);
	if (!ts) {
		return nullptr;
	}

	// Ensure the texture is uploaded to the GPU.  At model load time the
	// Vulkan slot exists (vulkan_bm_create) but bm_data hasn't been called
	// yet, so ts->image is null.  This mirrors gr_opengl_tcache_set which
	// the OpenGL readback calls to guarantee the texture is resident.
	if (!ts->image) {
		vulkan_preload(bitmap_num, 0);

		// Re-fetch slot after preload (bm_data may have replaced the image)
		ts = bm_get_gr_info<tcache_slot_vulkan>(bitmap_num, true);
		if (!ts || !ts->image) {
			return nullptr;
		}

		// bm_data submits uploads asynchronously via submitUploadAsync.
		// Wait for completion so the GPU layout matches ts->currentLayout
		// before we record the readback barriers.
		m_graphicsQueue.waitIdle();
	}

	// Validate static-texture assumption: format must be one known
	// from the bm_data upload path, and layout must be initialised.
	Assertion(ts->format != vk::Format::eUndefined,
		"get_bitmap_from_texture: undefined format on bitmap %d", bitmap_num);
	Assertion(ts->currentLayout != vk::ImageLayout::eUndefined,
		"get_bitmap_from_texture: undefined layout on bitmap %d", bitmap_num);
	Assertion(
		ts->format == vk::Format::eB8G8R8A8Unorm ||
		ts->format == vk::Format::eR8Unorm ||
		ts->format == vk::Format::eA1R5G5B5UnormPack16 ||
		ts->format == vk::Format::eBc1RgbaUnormBlock ||
		ts->format == vk::Format::eBc2UnormBlock ||
		ts->format == vk::Format::eBc3UnormBlock ||
		ts->format == vk::Format::eBc7UnormBlock,
		"get_bitmap_from_texture: unsupported format %d on bitmap %d"
		" — may be a render target or dynamic texture",
		static_cast<int>(ts->format), bitmap_num);

	const uint32_t w = ts->width;
	const uint32_t h = ts->height;
	const uint32_t pixelCount = w * h;
	const bool isCompressed = (ts->format == vk::Format::eBc1RgbaUnormBlock ||
	                           ts->format == vk::Format::eBc2UnormBlock ||
	                           ts->format == vk::Format::eBc3UnormBlock ||
	                           ts->format == vk::Format::eBc7UnormBlock);
	const bool hasAlpha = bm_has_alpha_channel(bitmap_num);
	const int outChannels = hasAlpha ? 4 : 3;

	// ---- calculate source buffer size & per-pixel layout ----
	size_t srcBytesPerPixel = 0;
	size_t srcBufferSize = 0;
	int blockSize = 0;
	uint32_t blockW = 0;
	uint32_t blockH = 0;

	if (isCompressed) {
		switch (ts->format) {
		case vk::Format::eBc1RgbaUnormBlock: blockSize = BCDEC_BC1_BLOCK_SIZE; break;
		case vk::Format::eBc2UnormBlock: blockSize = BCDEC_BC2_BLOCK_SIZE; break;
		case vk::Format::eBc3UnormBlock: blockSize = BCDEC_BC3_BLOCK_SIZE; break;
		case vk::Format::eBc7UnormBlock: blockSize = BCDEC_BC7_BLOCK_SIZE; break;
		default: return nullptr;
		}
		blockW = (w + 3) / 4;
		blockH = (h + 3) / 4;
		srcBufferSize = static_cast<size_t>(blockW) * static_cast<size_t>(blockH) * static_cast<size_t>(blockSize);
	} else {
		switch (ts->format) {
		case vk::Format::eB8G8R8A8Unorm: srcBytesPerPixel = 4; break;
		case vk::Format::eR8Unorm:        srcBytesPerPixel = 1; break;
		case vk::Format::eA1R5G5B5UnormPack16: srcBytesPerPixel = 2; break;
		default: return nullptr;
		}
		srcBufferSize = static_cast<size_t>(pixelCount) * srcBytesPerPixel;
	}

	// Output buffer: tightly packed w*h pixels at outChannels (3 or 4) bytes each,
	// matching the layout gr_opengl_get_bitmap_from_texture() hands back. Caller
	// owns this and frees it with vm_free (see bmpman.cpp's bm_lookup_cache).
	auto* data_out = static_cast<ubyte*>(vm_malloc(static_cast<size_t>(pixelCount) * outChannels));

	// ---- create readback staging buffer ----
	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAlloc;
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.size = srcBufferSize;
		bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		try {
			stagingBuffer = m_device.createBuffer(bufferInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("get_bitmap_from_texture: failed to create readback buffer: %s\n", e.what()));
			vm_free(data_out);
			return nullptr;
		}

		if (!m_memoryManager->allocateBufferMemory(stagingBuffer, MemoryUsage::GpuToCpu, stagingAlloc)) {
			m_device.destroyBuffer(stagingBuffer);
			vm_free(data_out);
			return nullptr;
		}
	}

	// ---- record layout-transition + copy command ----
	const vk::ImageLayout oldLayout = ts->currentLayout;
	vk::CommandBuffer cmd = beginSingleTimeCommands();

	// Transition to eTransferSrcOptimal (single mip-0, single layer)
	{
		ImageBarrier2 barrier;
		barrier.image = ts->image;
		barrier.baseMipLevel = 0;
		barrier.levelCount = 1;
		barrier.baseArrayLayer = ts->arrayIndex;
		barrier.layerCount = 1;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barrier.srcAccess = vk::AccessFlagBits2::eShaderSampledRead;
		barrier.dstStage = vk::PipelineStageFlagBits2::eCopy;
		barrier.dstAccess = vk::AccessFlagBits2::eTransferRead;

		cmdImageBarrier(cmd, barrier);
	}

	// Copy image -> buffer
	{
		vk::BufferImageCopy region;
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = ts->arrayIndex;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(w, h, 1);

		cmd.copyImageToBuffer(ts->image, vk::ImageLayout::eTransferSrcOptimal,
		                      stagingBuffer, region);
	}

	// Transition back to original layout
	{
		ImageBarrier2 barrier;
		barrier.image = ts->image;
		barrier.baseMipLevel = 0;
		barrier.levelCount = 1;
		barrier.baseArrayLayer = ts->arrayIndex;
		barrier.layerCount = 1;
		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = oldLayout;
		barrier.srcStage = vk::PipelineStageFlagBits2::eCopy;
		barrier.srcAccess = vk::AccessFlagBits2::eTransferRead;
		barrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;

		cmdImageBarrier(cmd, barrier);
	}

	endSingleTimeCommands(cmd); // synchronous submit + waitIdle

	// ---- read back & convert ----
	void* mapped = m_memoryManager->mapMemory(stagingAlloc);
	if (mapped) {
		m_memoryManager->invalidateMemory(stagingAlloc, 0, srcBufferSize);
		decodeReadbackBuffer(mapped, ts->format, isCompressed, w, h, blockW, blockH, blockSize, outChannels, data_out);
		m_memoryManager->unmapMemory(stagingAlloc);
	}

	m_device.destroyBuffer(stagingBuffer);
	m_memoryManager->freeAllocation(stagingAlloc);

	if (!mapped) {
		vm_free(data_out);
		return nullptr;
	}

	*width_out = ts->width;
	*height_out = ts->height;
	return data_out;
}

void VulkanTextureManager::decodeReadbackBuffer(const void* mapped, vk::Format format, bool isCompressed,
                                                 uint32_t w, uint32_t h, uint32_t blockW, uint32_t blockH,
                                                 int blockSize, int outChannels, void* data_out)
{
	if (isCompressed) {
		const uint32_t copyW = blockW * 4;
		const uint32_t copyH = blockH * 4;
		const size_t decompressedSize = static_cast<size_t>(copyW) * static_cast<size_t>(copyH) * 4;
		SCP_vector<uint8_t> temp(decompressedSize);

		const auto* srcBlock = static_cast<const uint8_t*>(mapped);
		uint8_t* blockRowBase = temp.data();

		for (uint32_t by = 0; by < blockH; ++by) {
			uint8_t* blockColBase = blockRowBase;
			for (uint32_t bx = 0; bx < blockW; ++bx) {
				switch (format) {
				case vk::Format::eBc1RgbaUnormBlock:
					bcdec_bc1(srcBlock, blockColBase, static_cast<int>(copyW * 4));
					break;
				case vk::Format::eBc2UnormBlock:
					bcdec_bc2(srcBlock, blockColBase, static_cast<int>(copyW * 4));
					break;
				case vk::Format::eBc3UnormBlock:
					bcdec_bc3(srcBlock, blockColBase, static_cast<int>(copyW * 4));
					break;
				case vk::Format::eBc7UnormBlock:
					bcdec_bc7(srcBlock, blockColBase, static_cast<int>(copyW * 4));
					break;
				default: break;
				}
				srcBlock += blockSize;
				blockColBase += 4 * 4; // 4 pixels wide × 4 bytes/pixel
			}
			blockRowBase += static_cast<size_t>(4) * copyW * 4; // 4 rows × full stride
		}

		const auto* src = temp.data();
		auto* dst = static_cast<uint8_t*>(data_out);
		if (outChannels == 4) {
			for (uint32_t y = 0; y < h; ++y) {
				memcpy(dst + y * w * 4, src + y * copyW * 4, w * 4);
			}
		} else {
			for (uint32_t y = 0; y < h; ++y) {
				graphics::util::convert_RGBA8888_to_RGB888(
					src + y * copyW * 4, dst + y * w * 3, w);
			}
		}
	} else {
		const uint32_t pixelCount = w * h;
		const auto* src = static_cast<const uint8_t*>(mapped);
		auto* dst = static_cast<uint8_t*>(data_out);

		switch (format) {
		case vk::Format::eB8G8R8A8Unorm:
			if (outChannels == 4) {
				graphics::util::convert_BGRA8888_to_RGBA8888(src, dst, pixelCount);
			} else {
				graphics::util::convert_BGRA8888_to_RGB888(src, dst, pixelCount);
			}
			break;

		case vk::Format::eR8Unorm:
			if (outChannels == 4) {
				graphics::util::expand_R8_to_RGBA(src, dst, pixelCount);
			} else {
				graphics::util::expand_R8_to_RGB(src, dst, pixelCount);
			}
			break;

		case vk::Format::eA1R5G5B5UnormPack16:
			if (outChannels == 4) {
				graphics::util::convert_BGRA1555_REV_to_RGBA8888(reinterpret_cast<const uint16_t*>(src), dst, pixelCount);
			} else {
				graphics::util::convert_BGRA1555_REV_to_RGB888(reinterpret_cast<const uint16_t*>(src), dst, pixelCount);
			}
			break;

		default: break;
		}
	}
}

vk::Sampler VulkanTextureManager::getSampler(vk::Filter magFilter, vk::Filter minFilter,
                                              vk::SamplerAddressMode addressMode,
                                              bool enableAnisotropy, float maxAnisotropy,
                                              bool enableMipmaps)
{
	// Create a key from sampler state
	uint64_t key = 0;
	key |= static_cast<uint64_t>(magFilter) << 0;
	key |= static_cast<uint64_t>(minFilter) << 4;
	key |= static_cast<uint64_t>(addressMode) << 8;
	key |= static_cast<uint64_t>(enableAnisotropy) << 16;
	key |= static_cast<uint64_t>(enableMipmaps) << 17;
	key |= static_cast<uint64_t>(maxAnisotropy * 10) << 24;

	auto it = m_samplerCache.find(key);
	if (it != m_samplerCache.end()) {
		return it->second;
	}

	// Create new sampler
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = magFilter;
	samplerInfo.minFilter = minFilter;
	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;
	samplerInfo.anisotropyEnable = enableAnisotropy && (m_maxAnisotropy > 1.0f);
	samplerInfo.maxAnisotropy = std::max(1.0f, std::min(maxAnisotropy > 0.0f ? maxAnisotropy : m_maxAnisotropy, m_maxAnisotropy));
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = enableMipmaps ? vk::SamplerMipmapMode::eLinear : vk::SamplerMipmapMode::eNearest;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = enableMipmaps ? VK_LOD_CLAMP_NONE : 0.0f;

	try {
		vk::Sampler sampler = m_device.createSampler(samplerInfo);
		m_samplerCache[key] = sampler;
		return sampler;
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "Failed to create sampler: %s\n", e.what()));
		return m_defaultSampler;
	}
}

vk::DescriptorImageInfo VulkanTextureManager::getFallbackTextureInfo2D()
{
	return {m_defaultSampler, m_fallbackTextureView2D,
	                               vk::ImageLayout::eShaderReadOnlyOptimal};
}

vk::DescriptorImageInfo VulkanTextureManager::getFallbackTextureInfoCube()
{
	return {m_defaultSampler, m_fallbackCubeView,
	                               vk::ImageLayout::eShaderReadOnlyOptimal};
}

vk::DescriptorImageInfo VulkanTextureManager::getFallbackTextureInfo2DArray()
{
	return {m_defaultSampler, m_fallback2DArrayView,
	                               vk::ImageLayout::eShaderReadOnlyOptimal};
}

vk::DescriptorImageInfo VulkanTextureManager::getFallbackTextureInfo3D()
{
	return {m_defaultSampler, m_fallback3DView,
	                               vk::ImageLayout::eShaderReadOnlyOptimal};
}

tcache_slot_vulkan* VulkanTextureManager::getTextureSlot(int handle)
{
	(void)this;
	auto* slot = bm_get_slot(handle, true);
	if (!slot || !slot->gr_info) {
		return nullptr;
	}
	return static_cast<tcache_slot_vulkan*>(slot->gr_info);
}

vk::Format VulkanTextureManager::bppToVkFormat(int bpp, bool compressed, int compressionType)
{
	if (compressed) {
		// DDS compression types
		switch (compressionType) {
		case DDS_DXT1:
			return vk::Format::eBc1RgbaUnormBlock;
		case DDS_DXT3:
			return vk::Format::eBc2UnormBlock;
		case DDS_DXT5:
			return vk::Format::eBc3UnormBlock;
		case DDS_BC7:
			return vk::Format::eBc7UnormBlock;
		// KTX compression types
		case KTX_ETC2_RGB:
			return vk::Format::eEtc2R8G8B8UnormBlock;
		case KTX_ETC2_SRGB:
			return vk::Format::eEtc2R8G8B8SrgbBlock;
		case KTX_ETC2_RGB_A1:
			return vk::Format::eEtc2R8G8B8A1UnormBlock;
		case KTX_ETC2_SRGB_A1:
			return vk::Format::eEtc2R8G8B8A1SrgbBlock;
		case KTX_ETC2_RGBA_EAC:
			return vk::Format::eEtc2R8G8B8A8UnormBlock;
		case KTX_ETC2_SRGBA_EAC:
			return vk::Format::eEtc2R8G8B8A8SrgbBlock;

		default:
			return vk::Format::eUndefined;
		}
	}

	switch (bpp) {
	case 8:
		return vk::Format::eR8Unorm;
	case 16:
		// OpenGL uses GL_UNSIGNED_SHORT_1_5_5_5_REV with GL_BGRA (A1R5G5B5)
		return vk::Format::eA1R5G5B5UnormPack16;
	case 24:
		// 24bpp (BGR) is almost never supported for optimal tiling in Vulkan.
		// We convert to 32bpp BGRA at upload time, so return the 32bpp format.
		return vk::Format::eB8G8R8A8Unorm;
	case 32:
		// FSO uses BGRA format (BMP_AARRGGBB = BGRA in memory)
		return vk::Format::eB8G8R8A8Unorm;
	default:
		return vk::Format::eUndefined;
	}
}

void VulkanTextureManager::transitionImageLayout(vk::Image image, vk::Format format,
                                                  vk::ImageLayout oldLayout,
                                                  vk::ImageLayout newLayout,
                                                  uint32_t mipLevels,
                                                  uint32_t arrayLayers)
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	ImageBarrier2 barrier;
	barrier.image = image;
	barrier.aspectMask = imageAspectFromFormat(format);
	barrier.baseMipLevel = 0;
	barrier.levelCount = mipLevels;
	barrier.baseArrayLayer = 0;
	barrier.layerCount = arrayLayers;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	// This is a generic, reusable transition helper -- the transfer/copy or
	// blit operation (if any) that follows a given call happens in the caller,
	// not here, so the transfer branches below stay at the generic eTransfer
	// stage rather than being tightened to eCopy/eBlit.
	if (oldLayout == vk::ImageLayout::eUndefined &&
	    newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccess = {};
		barrier.dstAccess = vk::AccessFlagBits2::eTransferWrite;
		barrier.srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
		barrier.dstStage = vk::PipelineStageFlagBits2::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
	           newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccess = vk::AccessFlagBits2::eTransferWrite;
		barrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;
		barrier.srcStage = vk::PipelineStageFlagBits2::eTransfer;
		barrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
	} else if (oldLayout == vk::ImageLayout::eUndefined &&
	           newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccess = {};
		barrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;
		barrier.srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
		barrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
	} else if (oldLayout == vk::ImageLayout::eUndefined &&
	           newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		barrier.srcAccess = {};
		barrier.dstAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
		barrier.srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
		barrier.dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	} else if (oldLayout == vk::ImageLayout::eUndefined &&
	           newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.srcAccess = {};
		barrier.dstAccess = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
		barrier.srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
		barrier.dstStage = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
	} else {
		// Generic transition: the caller's prior/next GPU state isn't known
		// here, so this deliberately stays maximally conservative rather than
		// being narrowed -- an incorrect guess here would be a real under-sync
		// hazard, which is a worse outcome than validation being coarse on
		// this specific (currently unreached) fallback path.
		barrier.srcAccess = vk::AccessFlagBits2::eMemoryWrite;
		barrier.dstAccess = vk::AccessFlagBits2::eMemoryRead;
		barrier.srcStage = vk::PipelineStageFlagBits2::eAllCommands;
		barrier.dstStage = vk::PipelineStageFlagBits2::eAllCommands;
		// This path is currently unreached; if it fires, a specific, narrower
		// transition entry should be added for this layout pair above.
		nprintf(("vulkan", "VulkanTexture: generic (conservative) layout transition %d -> %d; "
		                   "consider adding a dedicated barrier for this pair\n",
			static_cast<int>(oldLayout), static_cast<int>(newLayout)));
	}

	cmdImageBarrier(commandBuffer, barrier);

	endSingleTimeCommands(commandBuffer);
}

void vulkan_generate_mipmap_chain(vk::CommandBuffer cmd, vk::Image image,
                                  uint32_t width, uint32_t height,
                                  uint32_t mipLevels, uint32_t arrayLayers)
{
	if (mipLevels <= 1) {
		return;
	}

	// Generate each mip level via blit from the previous level
	for (uint32_t i = 1; i < mipLevels; i++) {
		uint32_t srcW = std::max(1u, width >> (i - 1));
		uint32_t srcH = std::max(1u, height >> (i - 1));
		uint32_t dstW = std::max(1u, width >> i);
		uint32_t dstH = std::max(1u, height >> i);

		// Transition mip i from eUndefined to eTransferDstOptimal (blit destination)
		{
			ImageBarrier2 barrier;
			barrier.image = image;
			barrier.baseMipLevel = i;
			barrier.levelCount = 1;
			barrier.baseArrayLayer = 0;
			barrier.layerCount = arrayLayers;
			barrier.oldLayout = vk::ImageLayout::eUndefined;
			barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.srcStage = vk::PipelineStageFlagBits2::eBlit;
			barrier.srcAccess = {};
			barrier.dstStage = vk::PipelineStageFlagBits2::eBlit;
			barrier.dstAccess = vk::AccessFlagBits2::eTransferWrite;

			cmdImageBarrier(cmd, barrier);
		}

		// Blit from mip i-1 to mip i
		vk::ImageBlit blit;
		blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = arrayLayers;
		blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.srcOffsets[1] = vk::Offset3D(static_cast<int32_t>(srcW), static_cast<int32_t>(srcH), 1);

		blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = arrayLayers;
		blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.dstOffsets[1] = vk::Offset3D(static_cast<int32_t>(dstW), static_cast<int32_t>(dstH), 1);

		cmd.blitImage(image, vk::ImageLayout::eTransferSrcOptimal,
		              image, vk::ImageLayout::eTransferDstOptimal,
		              blit, vk::Filter::eLinear);

		// Transition mip i to eTransferSrcOptimal (source for next blit)
		{
			ImageBarrier2 barrier;
			barrier.image = image;
			barrier.baseMipLevel = i;
			barrier.levelCount = 1;
			barrier.baseArrayLayer = 0;
			barrier.layerCount = arrayLayers;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.srcStage = vk::PipelineStageFlagBits2::eBlit;
			barrier.srcAccess = vk::AccessFlagBits2::eTransferWrite;
			barrier.dstStage = vk::PipelineStageFlagBits2::eBlit;
			barrier.dstAccess = vk::AccessFlagBits2::eTransferRead;

			cmdImageBarrier(cmd, barrier);
		}
	}

	// Final transition: all mips to eShaderReadOnlyOptimal
	{
		ImageBarrier2 barrier;
		barrier.image = image;
		barrier.baseMipLevel = 0;
		barrier.levelCount = mipLevels;
		barrier.baseArrayLayer = 0;
		barrier.layerCount = arrayLayers;
		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcStage = vk::PipelineStageFlagBits2::eBlit;
		barrier.srcAccess = vk::AccessFlagBits2::eTransferRead;
		barrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
		barrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;

		cmdImageBarrier(cmd, barrier);
	}
}

void VulkanTextureManager::frameStart()
{
	processPendingCommandBuffers();
}

bool VulkanTextureManager::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                                        vk::Format format, vk::ImageTiling tiling,
                                        vk::ImageUsageFlags usage, MemoryUsage memUsage,
                                        vk::Image& image, VulkanAllocation& allocation,
                                        uint32_t arrayLayers, bool cubemap,
                                        uint32_t imageDepth, vk::ImageType imageType)
{
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = imageType;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = imageDepth;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.samples = vk::SampleCountFlagBits::e1;

	if (cubemap) {
		imageInfo.flags |= vk::ImageCreateFlagBits::eCubeCompatible;
		Assertion(arrayLayers == 6, "Cubemap images must have exactly 6 array layers!");
	}

	try {
		image = m_device.createImage(imageInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "Failed to create image: %s\n", e.what()));
		return false;
	}

	if (!m_memoryManager->allocateImageMemory(image, memUsage, allocation)) {
		m_device.destroyImage(image);
		image = nullptr;
		return false;
	}

	return true;
}

vk::ImageView VulkanTextureManager::createImageView(vk::Image image, vk::Format format,
                                                     vk::ImageAspectFlags aspectFlags,
                                                     uint32_t mipLevels,
                                                     ImageViewType viewType,
                                                     uint32_t layerCount,
                                                     uint32_t baseArrayLayer)
{
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = image;
	switch (viewType) {
	case ImageViewType::Cube:
		viewInfo.viewType = vk::ImageViewType::eCube;
		break;
	case ImageViewType::Array2D:
		viewInfo.viewType = vk::ImageViewType::e2DArray;
		break;
	case ImageViewType::Volume3D:
		viewInfo.viewType = vk::ImageViewType::e3D;
		break;
	case ImageViewType::Plain2D:
	default:
		viewInfo.viewType = vk::ImageViewType::e2D;
		break;
	}
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
	viewInfo.subresourceRange.layerCount = layerCount;

	try {
		return m_device.createImageView(viewInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "Failed to create image view: %s\n", e.what()));
		return nullptr;
	}
}

bool VulkanTextureManager::createFallbackTexture(vk::Image& outImage, VulkanAllocation& outAlloc,
                                                  vk::ImageView& outView, ImageViewType viewType,
                                                  uint32_t arrayLayers, bool cubemap,
                                                  vk::ImageType imageType)
{
	if (!createImage(1, 1, 1, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
	                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	                 MemoryUsage::GpuOnly, outImage, outAlloc, arrayLayers, cubemap, 1, imageType)) {
		nprintf(("vulkan", "Failed to create fallback texture image!\n"));
		return false;
	}

	outView = createImageView(outImage, vk::Format::eR8G8B8A8Unorm,
	                          vk::ImageAspectFlagBits::eColor, 1, viewType, arrayLayers);
	if (!outView) {
		nprintf(("vulkan", "Failed to create fallback texture view!\n"));
		m_device.destroyImage(outImage);
		m_memoryManager->freeAllocation(outAlloc);
		return false;
	}

	// Upload white pixels via staging buffer
	SCP_vector<uint32_t> whitePixels(arrayLayers, 0xFFFFFFFF);
	vk::DeviceSize bufferSize = arrayLayers * sizeof(uint32_t);

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAlloc;
	try {
		stagingBuffer = m_device.createBuffer(bufferInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "Failed to create fallback staging buffer: %s\n", e.what()));
		m_device.destroyImageView(outView);
		m_device.destroyImage(outImage);
		m_memoryManager->freeAllocation(outAlloc);
		return false;
	}

	if (!m_memoryManager->allocateBufferMemory(stagingBuffer, MemoryUsage::CpuToGpu, stagingAlloc)) {
		m_device.destroyBuffer(stagingBuffer);
		m_device.destroyImageView(outView);
		m_device.destroyImage(outImage);
		m_memoryManager->freeAllocation(outAlloc);
		return false;
	}

	void* mapped = m_memoryManager->mapMemory(stagingAlloc);
	memcpy(mapped, whitePixels.data(), static_cast<size_t>(bufferSize));
	m_memoryManager->unmapMemory(stagingAlloc);

	SCP_vector<vk::BufferImageCopy> regions;
	for (uint32_t i = 0; i < arrayLayers; i++) {
		vk::BufferImageCopy region;
		region.bufferOffset = i * sizeof(uint32_t);
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = i;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(1, 1, 1);
		regions.push_back(region);
	}

	vk::CommandBuffer cmd = beginSingleTimeCommands();
	recordUploadCommands(cmd, outImage, stagingBuffer, vk::Format::eR8G8B8A8Unorm,
	                     1, 1, 1, vk::ImageLayout::eUndefined, false, regions, arrayLayers);
	endSingleTimeCommands(cmd);

	m_device.destroyBuffer(stagingBuffer);
	m_memoryManager->freeAllocation(stagingAlloc);

	return true;
}

bool VulkanTextureManager::createStaticTexture2D(uint32_t width, uint32_t height, vk::Format format,
                                                  const void* pixelData, size_t dataSize, const char* debugName,
                                                  vk::Image& outImage, vk::ImageView& outView,
                                                  VulkanAllocation& outAlloc)
{
	if (!createImage(width, height, 1, format, vk::ImageTiling::eOptimal,
	                 vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
	                 MemoryUsage::GpuOnly, outImage, outAlloc)) {
		nprintf(("vulkan", "Failed to create static texture image '%s'!\n", debugName));
		return false;
	}

	outView = createImageView(outImage, format, vk::ImageAspectFlagBits::eColor, 1, ImageViewType::Plain2D);
	if (!outView) {
		nprintf(("vulkan", "Failed to create static texture view '%s'!\n", debugName));
		m_device.destroyImage(outImage);
		m_memoryManager->freeAllocation(outAlloc);
		return false;
	}

	vk::Buffer stagingBuffer;
	VulkanAllocation stagingAlloc;
	if (!createStagingBuffer(dataSize, stagingBuffer, stagingAlloc)) {
		nprintf(("vulkan", "Failed to create staging buffer for static texture '%s'!\n", debugName));
		m_device.destroyImageView(outView);
		m_device.destroyImage(outImage);
		m_memoryManager->freeAllocation(outAlloc);
		return false;
	}

	void* mapped = m_memoryManager->mapMemory(stagingAlloc);
	memcpy(mapped, pixelData, dataSize);
	m_memoryManager->unmapMemory(stagingAlloc);

	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D(0, 0, 0);
	region.imageExtent = vk::Extent3D(width, height, 1);

	vk::CommandBuffer cmd = beginSingleTimeCommands();
	recordUploadCommands(cmd, outImage, stagingBuffer, format, width, height,
	                     1, vk::ImageLayout::eUndefined, false, {region}, 1);
	endSingleTimeCommands(cmd);

	m_device.destroyBuffer(stagingBuffer);
	m_memoryManager->freeAllocation(stagingAlloc);

	return true;
}

bool VulkanTextureManager::createStagingBuffer(size_t size, vk::Buffer& outBuffer,
                                               VulkanAllocation& outAllocation)
{
	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	try {
		outBuffer = m_device.createBuffer(bufferInfo);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanTexture: failed to create staging buffer: %s\n", e.what()));
		outBuffer = nullptr;
		return false;
	}

	if (!m_memoryManager->allocateBufferMemory(outBuffer, MemoryUsage::CpuOnly, outAllocation)) {
		m_device.destroyBuffer(outBuffer);
		outBuffer = nullptr;
		return false;
	}
	return true;
}

vk::CommandBuffer VulkanTextureManager::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer = m_device.allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

void VulkanTextureManager::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	m_graphicsQueue.submit(submitInfo, nullptr);
	m_graphicsQueue.waitIdle();

	m_device.freeCommandBuffers(m_commandPool, commandBuffer);
}

void VulkanTextureManager::recordUploadCommands(vk::CommandBuffer cmd, vk::Image image,
                                                 vk::Buffer stagingBuffer, vk::Format format,
                                                 uint32_t width, uint32_t height,
                                                 uint32_t mipLevels, vk::ImageLayout oldLayout,
                                                 bool generateMips,
                                                 const SCP_vector<vk::BufferImageCopy>& regions,
                                                 uint32_t arrayLayers)
{
	(void)format;  // May be needed for depth/stencil transitions in the future

	// Barrier 1: oldLayout -> eTransferDstOptimal (all mip levels, all layers).
	// Destination stage is always eCopy: both branches below are unconditionally
	// followed by a cmd.copyBufferToImage() a few lines down in this function.
	{
		ImageBarrier2 barrier;
		barrier.image = image;
		barrier.baseMipLevel = 0;
		barrier.levelCount = mipLevels;
		barrier.baseArrayLayer = 0;
		barrier.layerCount = arrayLayers;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.dstStage = vk::PipelineStageFlagBits2::eCopy;
		barrier.dstAccess = vk::AccessFlagBits2::eTransferWrite;

		if (oldLayout == vk::ImageLayout::eUndefined) {
			barrier.srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
			barrier.srcAccess = {};
		} else {
			// The image's prior producer isn't known at this generic reuse
			// site (this path re-uploads an image that could have last been
			// used in any number of ways) -- stay maximally conservative
			// rather than guess.
			barrier.srcStage = vk::PipelineStageFlagBits2::eAllCommands;
			barrier.srcAccess = vk::AccessFlagBits2::eMemoryWrite;
		}

		cmdImageBarrier(cmd, barrier);
	}

	if (!regions.empty()) {
		// Pre-baked mip levels: copy all regions (one per mip level) from the staging buffer
		cmd.copyBufferToImage(stagingBuffer, image, vk::ImageLayout::eTransferDstOptimal,
		                      static_cast<uint32_t>(regions.size()), regions.data());
	} else {
		// Single mip-0 copy
		vk::BufferImageCopy region;
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(width, height, 1);

		cmd.copyBufferToImage(stagingBuffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	}

	if (generateMips && mipLevels > 1 && regions.empty()) {
		// Generate mipmaps via blit chain: upload mip 0, then downsample each level

		// Transition mip 0 from eTransferDstOptimal to eTransferSrcOptimal
		// (source for the blit chain below)
		{
			ImageBarrier2 barrier;
			barrier.image = image;
			barrier.baseMipLevel = 0;
			barrier.levelCount = 1;
			barrier.baseArrayLayer = 0;
			barrier.layerCount = arrayLayers;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.srcStage = vk::PipelineStageFlagBits2::eCopy;
			barrier.srcAccess = vk::AccessFlagBits2::eTransferWrite;
			barrier.dstStage = vk::PipelineStageFlagBits2::eBlit;
			barrier.dstAccess = vk::AccessFlagBits2::eTransferRead;

			cmdImageBarrier(cmd, barrier);
		}

		vulkan_generate_mipmap_chain(cmd, image, width, height, mipLevels, arrayLayers);
	} else {
		// Simple transition: all mips from eTransferDstOptimal to eShaderReadOnlyOptimal
		{
			ImageBarrier2 barrier;
			barrier.image = image;
			barrier.baseMipLevel = 0;
			barrier.levelCount = mipLevels;
			barrier.baseArrayLayer = 0;
			barrier.layerCount = arrayLayers;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcStage = vk::PipelineStageFlagBits2::eCopy;
			barrier.srcAccess = vk::AccessFlagBits2::eTransferWrite;
			barrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
			barrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead;

			cmdImageBarrier(cmd, barrier);
		}
	}
}

void VulkanTextureManager::submitUploadAsync(vk::CommandBuffer cmd, vk::Buffer stagingBuffer,
                                             VulkanAllocation stagingAllocation)
{
	cmd.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	m_graphicsQueue.submit(submitInfo, nullptr);

	// Defer staging buffer destruction (2 frames matches MAX_FRAMES_IN_FLIGHT)
	auto* deletionQueue = getDeletionQueue();
	deletionQueue->queueBuffer(stagingBuffer, stagingAllocation);

	// Defer command buffer free
	m_pendingCommandBuffers.push_back({cmd, VulkanDeletionQueue::FRAMES_TO_WAIT});
}

void VulkanTextureManager::processPendingCommandBuffers()
{
	auto it = m_pendingCommandBuffers.begin();
	while (it != m_pendingCommandBuffers.end()) {
		if (it->framesRemaining == 0) {
			m_device.freeCommandBuffers(m_commandPool, it->cb);
			it = m_pendingCommandBuffers.erase(it);
		} else {
			it->framesRemaining--;
			++it;
		}
	}
}

uint32_t VulkanTextureManager::calculateMipLevels(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}


} // namespace graphics::vulkan
