
#include "VulkanPresentTarget.h"

#include "VulkanRenderer.h"

#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "mod_table/mod_table.h"

namespace graphics::vulkan {

namespace {

vk::SurfaceFormatKHR chooseSurfaceFormat(const PhysicalDeviceValues& values)
{
	// When HDR output is requested, prefer a 10-bit HDR10 (PQ / ST.2084) surface
	// using BT.2020 primaries. The final output-encode pass writes PQ-encoded
	// BT.2020 values into this surface.
	//
	// Never in the editor: its surface is a window embedded in a desktop-composited
	// application, so what an HDR10 swap chain would actually look like there is not
	// something we can verify. It would also drag in the format-change limitation
	// recreateSwapChain() documents.
	if (Gr_enable_hdr && !Fred_running) {
		for (const auto& availableFormat : values.surfaceFormats) {
			if ((availableFormat.format == vk::Format::eA2B10G10R10UnormPack32 ||
			     availableFormat.format == vk::Format::eA2R10G10B10UnormPack32) &&
			    availableFormat.colorSpace == vk::ColorSpaceKHR::eHdr10St2084EXT) {
				nprintf(("vulkan", "Vulkan: Selected HDR10 surface (10-bit, ST.2084/BT.2020)\n"));
				return availableFormat;
			}
		}
		nprintf(("vulkan", "Vulkan: HDR requested but no HDR10 surface format available; falling back to SDR\n"));
	}

	// Use a non-sRGB (UNORM) format to match OpenGL's default framebuffer behavior.
	// The FSO shaders handle gamma correction manually in the fragment shader and
	// post-processing pipeline, so hardware sRGB conversion would double-correct.
	for (const auto& availableFormat : values.surfaceFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
			availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	// Fallback: no preferred format matched. Pick the first concrete format,
	// defensively skipping any eUndefined entry (the legacy "any format allowed"
	// sentinel), and log the actual choice so it's visible in the log.
	for (const auto& availableFormat : values.surfaceFormats) {
		if (availableFormat.format != vk::Format::eUndefined) {
			nprintf(("vulkan", "Vulkan: no preferred surface format available; falling back to format=%d colorSpace=%d\n",
				static_cast<int>(availableFormat.format), static_cast<int>(availableFormat.colorSpace)));
			return availableFormat;
		}
	}

	// Degenerate list (all eUndefined) — return the front entry and warn.
	nprintf(("vulkan", "Vulkan: surface format list has no concrete entry; using front (format=%d)\n",
		static_cast<int>(values.surfaceFormats.front().format)));
	return values.surfaceFormats.front();
}

vk::PresentModeKHR choosePresentMode(const PhysicalDeviceValues& values)
{
	// With vsync requested, use FIFO: it is the only spec-guaranteed mode and
	// the only one that actually caps the frame rate to the display. Mailbox is
	// tear-free but uncapped ("fast vsync") and must not be silently substituted
	// for requested vsync. Without vsync prefer Immediate (true uncapped), then
	// Mailbox (uncapped but tear-free), then the guaranteed FIFO fallback.
	vk::PresentModeKHR chosen = vk::PresentModeKHR::eFifo;

	if (!Gr_enable_vsync) {
		for (const auto& availablePresentMode : values.presentModes) {
			if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
				chosen = availablePresentMode;
				break;
			}
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				chosen = availablePresentMode;
			}
		}
	}

	const char* name = "Unknown";
	switch (chosen) {
		case vk::PresentModeKHR::eImmediate:    name = "Immediate"; break;
		case vk::PresentModeKHR::eMailbox:       name = "Mailbox"; break;
		case vk::PresentModeKHR::eFifo:          name = "FIFO (vsync)"; break;
		case vk::PresentModeKHR::eFifoRelaxed:   name = "FIFO Relaxed"; break;
		default: break;
	}
	mprintf(("Vulkan: Present mode: %s (Gr_enable_vsync=%d)\n", name, Gr_enable_vsync ? 1 : 0));

	return chosen;
}

vk::Extent2D chooseSwapChainExtent(const PhysicalDeviceValues& values, uint32_t width, uint32_t height)
{
	if (values.surfaceCapabilities.currentExtent.width != UINT32_MAX) {
		return values.surfaceCapabilities.currentExtent;
	} else {
		VkExtent2D actualExtent = {width, height};

		actualExtent.width = std::max(values.surfaceCapabilities.minImageExtent.width,
			std::min(values.surfaceCapabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(values.surfaceCapabilities.minImageExtent.height,
			std::min(values.surfaceCapabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

} // namespace

bool checkSwapChainSupport(PhysicalDeviceValues& values, vk::SurfaceKHR surface)
{
	values.surfaceCapabilities = values.device.getSurfaceCapabilitiesKHR(surface);
	auto fmts = values.device.getSurfaceFormatsKHR(surface);
	values.surfaceFormats.assign(fmts.begin(), fmts.end());
	auto modes = values.device.getSurfacePresentModesKHR(surface);
	values.presentModes.assign(modes.begin(), modes.end());

	return !values.surfaceFormats.empty() && !values.presentModes.empty();
}

VulkanSurfaceHandle::VulkanSurfaceHandle(os::VulkanSurfaceProvider* provider,
	vk::Instance instance,
	vk::SurfaceKHR surface)
	: m_provider(provider), m_instance(instance), m_surface(surface)
{
}
VulkanSurfaceHandle::~VulkanSurfaceHandle()
{
	reset();
}
VulkanSurfaceHandle::VulkanSurfaceHandle(VulkanSurfaceHandle&& other) noexcept
	: m_provider(other.m_provider), m_instance(other.m_instance), m_surface(other.m_surface)
{
	other.m_provider = nullptr;
	other.m_instance = vk::Instance();
	other.m_surface = vk::SurfaceKHR();
}
VulkanSurfaceHandle& VulkanSurfaceHandle::operator=(VulkanSurfaceHandle&& other) noexcept
{
	if (this != &other) {
		reset();

		m_provider = other.m_provider;
		m_instance = other.m_instance;
		m_surface = other.m_surface;

		other.m_provider = nullptr;
		other.m_instance = vk::Instance();
		other.m_surface = vk::SurfaceKHR();
	}
	return *this;
}
void VulkanSurfaceHandle::reset()
{
	if (m_provider != nullptr && m_surface) {
		m_provider->destroyVulkanSurface(static_cast<VkInstance>(m_instance),
			os::vulkan_handle_value(static_cast<VkSurfaceKHR>(m_surface)));
	}

	m_provider = nullptr;
	m_instance = vk::Instance();
	m_surface = vk::SurfaceKHR();
}

bool VulkanRenderer::createTargetSurface(VulkanPresentTarget& target)
{
	auto* vulkanSupport = m_graphicsOps->getVulkanSupport();
	Assertion(vulkanSupport != nullptr, "initializeInstance() should have rejected this already!");

	const auto surface =
		vulkanSupport->createVulkanSurface(target.viewport, static_cast<VkInstance>(*m_vkInstance));
	if (surface == 0) {
		nprintf(("vulkan", "Vulkan: failed to create a surface for this viewport.\n"));
		return false;
	}

	target.surface = VulkanSurfaceHandle(vulkanSupport,
		*m_vkInstance,
		vk::SurfaceKHR(os::vulkan_handle_cast<VkSurfaceKHR>(surface)));
	return true;
}

// ========== Extent-sized resources ==========
//
// Every one of these is rebuilt whenever the swap chain is, which is why they live next to
// it rather than with the renderer-wide setup: their size comes from the target's surface.

void VulkanRenderer::createCompositionResources(VulkanPresentTarget& target)
{
	// Free any previous composition resources (swap chain recreation path)
	target.compositionImageViews.clear();
	target.compositionImages.clear();
	for (auto& alloc : target.compositionAllocations) {
		if (alloc.isValid()) {
			m_memoryManager->freeAllocation(alloc);
		}
	}
	target.compositionAllocations.clear();

	const size_t count = target.imageViews.size();
	target.compositionImages.reserve(count);
	target.compositionImageViews.reserve(count);
	target.compositionAllocations.reserve(count);

	for (size_t i = 0; i < count; ++i) {
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = HDR_COLOR_FORMAT;
		imageInfo.extent = vk::Extent3D(target.extent.width, target.extent.height, 1);
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled |
		                   vk::ImageUsageFlagBits::eTransferSrc;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		auto image = m_device->createImageUnique(imageInfo);

		VulkanAllocation alloc{};
		m_memoryManager->allocateImageMemory(image.get(), MemoryUsage::GpuOnly, alloc, MemoryPurpose::RenderTarget);

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = image.get();
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = HDR_COLOR_FORMAT;
		viewInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
		auto view = m_device->createImageViewUnique(viewInfo);

		target.compositionImages.push_back(std::move(image));
		target.compositionAllocations.push_back(alloc);
		target.compositionImageViews.push_back(std::move(view));
	}

	// Sampler used by the output-encode pass to read the composition image.
	if (!m_compositionSampler) {
		vk::SamplerCreateInfo sampInfo;
		sampInfo.magFilter = vk::Filter::eNearest;
		sampInfo.minFilter = vk::Filter::eNearest;
		sampInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		sampInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		sampInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		sampInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		m_compositionSampler = m_device->createSamplerUnique(sampInfo);
	}
}


void VulkanRenderer::createFrameBuffers(VulkanPresentTarget& target)
{
	target.framebuffers.clear();
	target.encodeFramebuffers.clear();

	// Composition framebuffers: color = fp16 composition image, depth shared.
	// Indexed by swap chain image so each in-flight frame uses its own image.
	target.framebuffers.reserve(target.compositionImageViews.size());
	for (const auto& compView : target.compositionImageViews) {
		const vk::ImageView attachments[] = {
			compView.get(),
			target.depthImageView.get(),
		};

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = m_renderPass.get();
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = target.extent.width;
		framebufferInfo.height = target.extent.height;
		framebufferInfo.layers = 1;

		target.framebuffers.push_back(m_device->createFramebufferUnique(framebufferInfo));
	}

	// Encode framebuffers: color = actual swap chain image.
	target.encodeFramebuffers.reserve(target.imageViews.size());
	for (const auto& scView : target.imageViews) {
		const vk::ImageView attachments[] = { scView.get() };

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = m_encodeRenderPass.get();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = target.extent.width;
		framebufferInfo.height = target.extent.height;
		framebufferInfo.layers = 1;

		target.encodeFramebuffers.push_back(m_device->createFramebufferUnique(framebufferInfo));
	}
}


vk::Format VulkanRenderer::findDepthFormat()
{
	// Prefer D32_SFLOAT for best precision, fall back to D32_SFLOAT_S8 or D24_UNORM_S8
	const vk::Format candidates[] = {
		vk::Format::eD32Sfloat,
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eD24UnormS8Uint,
	};

	for (auto format : candidates) {
		auto props = m_physicalDevice.getFormatProperties(format);
		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
			return format;
		}
	}

	// Should never happen on any real GPU
	Error(LOCATION, "Failed to find supported depth format!");
	return vk::Format::eD32Sfloat;
}
void VulkanRenderer::createDepthResources(VulkanPresentTarget& target)
{
	const vk::Format depthFormat = findDepthFormat();
	// The render passes (m_renderPass, scene/G-buffer passes, ...) bake in the
	// depth format, and they are deliberately kept alive across swap chain
	// recreation. A driver changing its supported depth formats mid-session
	// would make them all incompatible with the new attachment.
	if (m_depthFormat != vk::Format::eUndefined && depthFormat != m_depthFormat) {
		Error(LOCATION, "Vulkan: depth format changed across swap chain recreation (%d -> %d)!",
			static_cast<int>(m_depthFormat), static_cast<int>(depthFormat));
	}
	m_depthFormat = depthFormat;

	// Create depth image
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = m_depthFormat;
	imageInfo.extent.width = target.extent.width;
	imageInfo.extent.height = target.extent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	target.depthImage = m_device->createImageUnique(imageInfo);

	// Allocate GPU memory for the depth image
	m_memoryManager->allocateImageMemory(
		target.depthImage.get(), MemoryUsage::GpuOnly, target.depthImageMemory, MemoryPurpose::RenderTarget);

	// Create depth image view
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = target.depthImage.get();
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = m_depthFormat;
	viewInfo.subresourceRange.aspectMask = imageAspectFromFormat(m_depthFormat);
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	target.depthImageView = m_device->createImageViewUnique(viewInfo);

	nprintf(("vulkan", "Vulkan: Created depth buffer (%dx%d, format %d)\n",
		target.extent.width, target.extent.height, static_cast<int>(m_depthFormat)));
}



void VulkanRenderer::createPresentSyncObjects(VulkanPresentTarget& target)
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		target.frames[i] = std::make_unique<VulkanRenderFrame>(m_device.get(), target.swapChain.get(), m_graphicsQueue, m_presentQueue);
	}

	target.imageRenderFrame.resize(target.images.size(), nullptr);

	// One more than the frames in flight: at any moment the in-flight frames can each be holding
	// one, and a viewport switch can have retained one on top of that.
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;
	target.acquireSemaphores.clear();
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT + 1; ++i) {
		VulkanPresentTarget::AcquireSemaphore entry;
		entry.semaphore = m_device->createSemaphoreUnique(semaphoreCreateInfo);
		target.acquireSemaphores.push_back(std::move(entry));
	}
	target.nextAcquire = 0;
	target.currentAcquire = 0;
	target.hasRetainedAcquire = false;

	createRenderFinishedSemaphores(target);
}
void VulkanRenderer::createRenderFinishedSemaphores(VulkanPresentTarget& target)
{
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;

	target.renderFinishedSemaphores.clear();
	target.renderFinishedSemaphores.reserve(target.images.size());
	for (size_t i = 0; i < target.images.size(); ++i) {
		target.renderFinishedSemaphores.push_back(m_device->createSemaphoreUnique(semaphoreCreateInfo));
	}
}


bool VulkanRenderer::createTargetResources(VulkanPresentTarget& target)
{
	if (!createTargetSurface(target)) {
		return false;
	}

	// The device was already chosen against the main surface, so only the parts that are per-surface
	// get re-queried here. The present queue is checked rather than assumed: a device is allowed to
	// support presentation to one surface and not another.
	PhysicalDeviceValues values;
	values.device = m_physicalDevice;
	values.graphicsQueueIndex = {true, m_graphicsQueueFamilyIndex};
	values.presentQueueIndex = {true, m_presentQueueFamilyIndex};

	if (!m_physicalDevice.getSurfaceSupportKHR(m_presentQueueFamilyIndex, target.surface.get())) {
		mprintf(("Vulkan: the present queue cannot present to this viewport's surface.\n"));
		return false;
	}

	if (!checkSwapChainSupport(values, target.surface.get())) {
		mprintf(("Vulkan: this viewport's surface reports no usable formats or present modes.\n"));
		return false;
	}

	if (!createSwapChain(target, values)) {
		mprintf(("Vulkan: failed to create a swap chain for this viewport.\n"));
		return false;
	}

	// m_encodeRenderPass is shared and bakes in the swap chain format it was built for, so a target
	// that negotiates a different one cannot present through it. In practice every surface here is
	// the same device, driver and window system and they agree -- which is exactly why this is
	// checked rather than assumed, since a mismatch would otherwise be silent and only appear on
	// somebody else's hardware. Fail the target instead; the caller falls back.
	if (target.imageFormat != m_mainTarget->imageFormat) {
		mprintf(("Vulkan: this viewport's surface negotiated format %d but the output-encode pass was "
		         "built for %d; cannot present to it.\n",
			static_cast<int>(target.imageFormat), static_cast<int>(m_mainTarget->imageFormat)));
		return false;
	}

	createDepthResources(target);
	createCompositionResources(target);
	createFrameBuffers(target);
	createPresentSyncObjects(target);

	nprintf(("vulkan", "Vulkan: created a present target for a second viewport (%ux%u, %zu images)\n",
		target.extent.width, target.extent.height, target.images.size()));

	return true;
}

void VulkanRenderer::destroyDepthResources(VulkanPresentTarget& target)
{
	target.depthImageView.reset();
	target.depthImage.reset();
	if (m_memoryManager && target.depthImageMemory.isValid()) {
		m_memoryManager->freeAllocation(target.depthImageMemory);
		target.depthImageMemory = {};
	}
}

void VulkanRenderer::destroyTargetSwapChain(VulkanPresentTarget& target)
{
	// Framebuffers and views reference the swap chain images, and the frames hold the swap chain
	// handle for their acquires and presents, so all of them go first.
	target.framebuffers.clear();
	target.encodeFramebuffers.clear();
	target.imageViews.clear();
	target.images.clear();
	target.imageRenderFrame.clear();

	for (auto& frame : target.frames) {
		frame.reset();
	}
	target.acquireSemaphores.clear();
	target.renderFinishedSemaphores.clear();
	target.hasRetainedAcquire = false;

	target.swapChain.reset();
}

void VulkanRenderer::releaseTargetMemory(VulkanPresentTarget& target)
{
	destroyDepthResources(target);

	target.compositionImageViews.clear();
	target.compositionImages.clear();
	if (m_memoryManager) {
		for (auto& alloc : target.compositionAllocations) {
			if (alloc.isValid()) {
				m_memoryManager->freeAllocation(alloc);
			}
		}
	}
	target.compositionAllocations.clear();
}

bool VulkanRenderer::createSwapChain(VulkanPresentTarget& target,
	const PhysicalDeviceValues& deviceValues,
	vk::SwapchainKHR oldSwapchain)
{
	// Choose one more than the minimum to avoid driver synchronization if it is not done with a thread yet
	uint32_t imageCount = deviceValues.surfaceCapabilities.minImageCount + 1;
	if (deviceValues.surfaceCapabilities.maxImageCount > 0 &&
		imageCount > deviceValues.surfaceCapabilities.maxImageCount) {
		imageCount = deviceValues.surfaceCapabilities.maxImageCount;
	}

	const auto surfaceFormat = chooseSurfaceFormat(deviceValues);

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = target.surface.get();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = chooseSwapChainExtent(deviceValues, gr_screen.max_w, gr_screen.max_h);
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment
	                      | vk::ImageUsageFlagBits::eTransferSrc
	                      | vk::ImageUsageFlagBits::eTransferDst;

	const uint32_t queueFamilyIndices[] = {deviceValues.graphicsQueueIndex.index, deviceValues.presentQueueIndex.index};
	if (deviceValues.graphicsQueueIndex.index != deviceValues.presentQueueIndex.index) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	createInfo.preTransform = deviceValues.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = choosePresentMode(deviceValues);
	createInfo.clipped = true;
	createInfo.oldSwapchain = oldSwapchain;

	auto newSwapChain = m_device->createSwapchainKHRUnique(createInfo);

	// Clear old resources before replacing the swap chain
	target.framebuffers.clear();
	target.imageViews.clear();

	target.swapChain = std::move(newSwapChain);

	auto swapChainImages = m_device->getSwapchainImagesKHR(target.swapChain.get());
	target.images.assign(swapChainImages.begin(), swapChainImages.end());
	target.imageFormat = surfaceFormat.format;
	target.colorSpace = surfaceFormat.colorSpace;
	target.hdrActive = (surfaceFormat.colorSpace == vk::ColorSpaceKHR::eHdr10St2084EXT);
	Gr_hdr_output_active = target.hdrActive;
	target.extent = createInfo.imageExtent;
	mprintf(("Vulkan: Swap chain output mode: %s\n", target.hdrActive ? "HDR10 (PQ/BT.2020)" : "SDR (sRGB)"));

	target.imageViews.reserve(target.images.size());
	for (const auto& image : target.images) {
		vk::ImageViewCreateInfo viewCreateInfo;
		viewCreateInfo.image = image;
		viewCreateInfo.viewType = vk::ImageViewType::e2D;
		viewCreateInfo.format = target.imageFormat;

		viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;

		viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		target.imageViews.push_back(m_device->createImageViewUnique(viewCreateInfo));
	}

	// No layout transition needed for the new images: the only pass that writes
	// them (m_encodeRenderPass) uses initialLayout=eUndefined with
	// loadOp=eDontCare, so their first use never reads prior contents.

	// Advertise HDR10 mastering/content metadata to the compositor when active.
	if (target.hdrActive && m_hdrMetadataSupported) {
		vk::HdrMetadataEXT metadata;
		// BT.2020 display primaries and D65 white point
		metadata.displayPrimaryRed   = vk::XYColorEXT{0.708f, 0.292f};
		metadata.displayPrimaryGreen = vk::XYColorEXT{0.170f, 0.797f};
		metadata.displayPrimaryBlue  = vk::XYColorEXT{0.131f, 0.046f};
		metadata.whitePoint          = vk::XYColorEXT{0.3127f, 0.3290f};
		metadata.maxLuminance        = Gr_hdr_peak_nits;
		metadata.minLuminance        = 0.0f;
		metadata.maxContentLightLevel        = Gr_hdr_peak_nits;
		metadata.maxFrameAverageLightLevel   = Gr_hdr_paperwhite_nits;
		m_device->setHdrMetadataEXT(target.swapChain.get(), metadata);
		mprintf(("Vulkan: HDR10 metadata set (peak %.0f nits, paper white %.0f nits)\n",
			Gr_hdr_peak_nits, Gr_hdr_paperwhite_nits));
	}

	return true;
}

bool VulkanRenderer::recreateSwapChain(VulkanPresentTarget& target)
{
	nprintf(("vulkan", "Vulkan: Recreating swap chain...\n"));

	// Wait for all frames to finish so no resources are in use
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		target.frames[i]->waitForFinish();
	}
	m_device->waitIdle();

	// Re-query surface state (may have changed due to resize/compositor)
	PhysicalDeviceValues freshValues;
	freshValues.device = m_physicalDevice;
	freshValues.graphicsQueueIndex = {true, m_graphicsQueueFamilyIndex};
	freshValues.presentQueueIndex = {true, m_presentQueueFamilyIndex};

	if (!checkSwapChainSupport(freshValues, target.surface.get())) {
		nprintf(("vulkan", "Vulkan: surface no longer reports usable formats or present modes, "
		                   "deferring swap chain recreation\n"));
		return false;
	}

	// Check for 0x0 extent (minimized window) — caller should retry later
	auto extent = chooseSwapChainExtent(freshValues, gr_screen.max_w, gr_screen.max_h);
	if (extent.width == 0 || extent.height == 0) {
		nprintf(("vulkan", "Vulkan: Surface extent is 0x0 (minimized), deferring swap chain recreation\n"));
		return false;
	}

	// Recreate all size-dependent resources. The render passes (including
	// m_encodeRenderPass) are intentionally NOT recreated so cached pipelines
	// remain valid; only images, views, and framebuffers are rebuilt.
	const vk::Format oldSwapChainFormat = target.imageFormat;
	createSwapChain(target, freshValues, target.swapChain.get());

	// Known limitation: if the surface format changes across recreation (e.g.
	// the window moves to a display that flips HDR10 availability),
	// m_encodeRenderPass and the post-processor's LDR format would need a full
	// rebuild, which we don't support yet. Log it loudly.
	if (target.imageFormat != oldSwapChainFormat) {
		mprintf(("Vulkan: WARNING - swap chain surface format changed across recreation (%d -> %d); "
		         "rendering may be broken until restart\n",
			static_cast<int>(oldSwapChainFormat), static_cast<int>(target.imageFormat)));
	}

	// The depth buffer is extent-sized; recreate it before the framebuffers
	// that attach its view. createDepthResources() verifies the format is stable
	// (the kept render passes bake it in).
	destroyDepthResources(target);
	createDepthResources(target);

	createCompositionResources(target);
	createFrameBuffers(target);

	// Recreate the post-processor's extent-sized targets (scene color/depth,
	// G-buffer, bloom chains, LDR/SMAA targets, ...). Its render passes and
	// samplers are extent-independent and stay alive, keeping pipelines valid.
	if (m_postProcessor && !m_postProcessor->resize(target.extent)) {
		mprintf(("Vulkan: post-processor resize failed, disabling post-processing!\n"));
		setPostProcessor(nullptr);
		m_postProcessor->shutdown();
		m_postProcessor.reset();
	}

	// Drop renderer-side cached state that may reference destroyed views
	if (m_drawManager) {
		m_drawManager->onResize();
	}
	m_sceneDepthCopiedThisFrame = false;

	// Update VulkanRenderFrame handles to point to the new swap chain.
	for (auto& frame : target.frames) {
		frame->updateSwapChain(target.swapChain.get());
	}

	// The render-finished semaphores are keyed on swap chain image, and the image count can change
	// across recreation, so they go with the images they belonged to. Any of them still held by the
	// presentation engine belonged to the old swap chain, which is retired here. All frames are
	// idle (waited above), so nothing is still signalling one.
	createRenderFinishedSemaphores(target);

	// The acquire semaphores were signalled against the swap chain that just went away, and any
	// image a viewport switch was holding on to belonged to it too. Start both over.
	constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo;
	for (auto& acquire : target.acquireSemaphores) {
		acquire.semaphore = m_device->createSemaphoreUnique(semaphoreCreateInfo);
		acquire.consumer = nullptr;
	}
	target.nextAcquire = 0;
	target.currentAcquire = 0;
	target.hasRetainedAcquire = false;

	// Reset swap chain image tracking
	target.imageRenderFrame.clear();
	target.imageRenderFrame.resize(target.images.size(), nullptr);
	target.previousImage = UINT32_MAX;

	target.needsRecreation = false;

	nprintf(("vulkan", "Vulkan: Swap chain recreated successfully (%ux%u, %zu images)\n",
		target.extent.width, target.extent.height, target.images.size()));

	return true;
}

} // namespace graphics::vulkan
