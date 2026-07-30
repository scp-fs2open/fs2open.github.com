
#include "VulkanRenderer.h"
#include "VulkanBarrier.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanPostProcessing.h"

#include "cmdline/cmdline.h"
#include "graphics/grinternal.h"
#include "graphics/post_processing.h"
#include "graphics/util/pixel_swizzle.h"

#include "backends/imgui_impl_vulkan.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "mod_table/mod_table.h"

#include <SDL3/SDL_vulkan.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

extern float flFrametime;

namespace graphics::vulkan {

// VulkanSurfaceHandle's implementation lives in VulkanRendererSetup.cpp, next to
// createTargetSurface()/createTargetResources() -- the rest of a target's surface/swap-chain
// lifecycle.

VulkanRenderer::VulkanRenderer(std::unique_ptr<os::GraphicsOperations> graphicsOps)
	: m_graphicsOps(std::move(graphicsOps))
{
}
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

void VulkanRenderer::createEncodeRenderPass(vk::Format swapChainFormat)
{
	// Color-only pass that writes the actual swap chain image. The fullscreen
	// encode draw overwrites the whole image, so the prior contents are discarded.
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = swapChainFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorRef;
	colorRef.attachment = 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;

	// Make the composition image (written by the main pass) available for
	// sampling, and order against the swap chain image's prior presentation.
	vk::SubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eFragmentShader;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eFragmentShader;
	dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
	                         | vk::AccessFlagBits::eShaderRead;

	vk::RenderPassCreateInfo rpInfo;
	rpInfo.attachmentCount = 1;
	rpInfo.pAttachments = &colorAttachment;
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = 1;
	rpInfo.pDependencies = &dependency;

	m_encodeRenderPass = m_device->createRenderPassUnique(rpInfo);
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

void VulkanRenderer::encodeToSwapChain()
{
	if (!m_postProcessor || m_current->currentImage >= m_current->images.size()) {
		return;
	}
	if (m_current->currentImage >= m_current->encodeFramebuffers.size()) {
		return;
	}

	const float gamma = Cmdline_no_set_gamma ? 1.0f : Gr_gamma;

	// HDR10: PQ/BT.2020 encode plus the user gamma slider, must run as a
	// shader (encodeOutput()).
	if (m_current->hdrActive) {
		m_postProcessor->encodeOutput(
			m_currentCommandBuffer,
			m_encodeRenderPass.get(),
			m_current->encodeFramebuffers[m_current->currentImage].get(),
			m_current->extent,
			m_current->compositionImageViews[m_current->currentImage].get(),
			m_compositionSampler.get(),
			Gr_hdr_paperwhite_nits,
			Gr_hdr_peak_nits,
			gamma);
		return;
	}

	// SDR: the composition image already carries the final display-ready
	// sRGB-encoded frame (3D scene + menus/HUD alike). Always run the
	// gamma-encode shader here -- rather than a raw blit -- so the user
	// gamma/brightness slider has a uniform, whole-frame effect regardless
	// of device blit support.
	m_postProcessor->encodeOutputSdr(
		m_currentCommandBuffer,
		m_encodeRenderPass.get(),
		m_current->encodeFramebuffers[m_current->currentImage].get(),
		m_current->extent,
		m_current->compositionImageViews[m_current->currentImage].get(),
		m_compositionSampler.get(),
		gamma);
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


void VulkanRenderer::createRenderPass()
{
	// Attachment 0: Color - clear each frame
	// The whole frame is rendered into the fp16 composition image (not directly
	// into the swap chain). A separate output-encode pass (m_encodeRenderPass)
	// later samples this image, so the final layout is eShaderReadOnlyOptimal.
	// UI screens draw their own full-screen backgrounds; 3D clears via scene_texture_begin.
	// Popups that need previous frame content use gr_save_screen/gr_restore_screen.
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = HDR_COLOR_FORMAT;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	// Attachment 1: Depth
	vk::AttachmentDescription depthAttachment;
	depthAttachment.format = m_depthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference colorAttachRef;
	colorAttachRef.attachment = 0;
	colorAttachRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachRef;
	depthAttachRef.attachment = 1;
	depthAttachRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;
	subpass.pDepthStencilAttachment = &depthAttachRef;

	// External dependency must make the PREVIOUS frame's accesses to these
	// attachments available/ordered before this frame writes them again. The
	// depth buffer is a single shared image (one m_current->depthImage for every
	// swap-chain framebuffer), so frame N+1's loadOp=eClear collides with frame
	// N's depth writes (WRITE_AFTER_WRITE) unless srcAccessMask lists the prior
	// depth write; likewise the composition color's store when a swap image
	// repeats. eFragmentShader in srcStageMask additionally orders this write
	// after the prior frame's output-encode pass sampling the composition color
	// (a write-after-read). (All detected/verified via -gr_sync_validation.)
	vk::SubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eEarlyFragmentTests
	                        | vk::PipelineStageFlagBits::eLateFragmentTests
	                        | vk::PipelineStageFlagBits::eFragmentShader;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
	                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	// eColorAttachmentRead covers the loadOp=eLoad read in the m_renderPassLoad
	// variant (which shares this dependency): its color initialLayout
	// (eShaderReadOnlyOptimal) differs from the subpass layout, so begin performs
	// an automatic layout transition (a write) that the load read must be ordered
	// after -- otherwise a READ_AFTER_WRITE hazard vs the transition (flagged by
	// -gr_sync_validation). Harmless for the eClear m_renderPass variant.
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
	                         | vk::AccessFlagBits::eColorAttachmentRead
	                         | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	m_renderPass = m_device->createRenderPassUnique(renderPassInfo);

	// Create a second render pass with loadOp=eLoad for resuming the composition
	// pass after post-processing. Render-pass compatibility (which is what lets a
	// pipeline built against m_renderPass bind under m_renderPassLoad, and lets
	// both share m_current->framebuffers) is defined ONLY by matching attachment
	// formats/sample counts and subpass structure -- it deliberately ignores
	// load/store ops, layouts, AND subpass dependencies. So this variant reuses
	// the same renderPassInfo/dependency but only overrides the ops/layouts below.
	colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	colorAttachment.initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	// Depth is deliberately re-cleared (loadOp=eClear) on resume: the resumed
	// composition pass draws only HUD / 2D / UI on top of the already-composited
	// post-processed color, none of which depends on the 3D scene's depth (that
	// lives in the separate post-processor scene-depth target). initialLayout is
	// eDepthStencilAttachmentOptimal (not eUndefined) so no cross-frame layout
	// transition write occurs here -- the depth WAW is handled by the shared
	// dependency's eDepthStencilAttachmentWrite srcAccessMask above.
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	attachments = {colorAttachment, depthAttachment};

	m_renderPassLoad = m_device->createRenderPassUnique(renderPassInfo);
}
void VulkanRenderer::createCommandPool(const PhysicalDeviceValues& values)
{
	vk::CommandPoolCreateInfo poolCreate;
	poolCreate.queueFamilyIndex = values.graphicsQueueIndex.index;
	poolCreate.flags |= vk::CommandPoolCreateFlagBits::eTransient;

	m_graphicsCommandPool = m_device->createCommandPoolUnique(poolCreate);
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

bool VulkanRenderer::readbackFramebuffer(ubyte** outPixels, uint32_t* outWidth, uint32_t* outHeight)
{
	*outPixels = nullptr;
	*outWidth = 0;
	*outHeight = 0;

	if (m_current->previousImage == UINT32_MAX) {
		nprintf(("vulkan", "VulkanRenderer::readbackFramebuffer - no previous frame available\n"));
		return false;
	}

	if (!m_frameInProgress) {
		nprintf(("vulkan", "VulkanRenderer::readbackFramebuffer - no frame in progress\n"));
		return false;
	}

	// Read the previous frame's fp16 composition image in both SDR and HDR
	// modes -- never the presented swap chain image. The swap chain image is
	// owned by the presentation engine after present (reading it back violates
	// its layout/ownership contract), and in HDR it's 10-bit PQ-packed anyway.
	// The composition image is app-owned, stays in eShaderReadOnlyOptimal after
	// the encode pass sampled it, and carries the final sRGB-encoded frame
	// (1.0 == paper white) before the user gamma slider is applied -- the same
	// fidelity class as OpenGL's back-buffer read. Converted to BGRA8 on the
	// CPU below.
	const vk::Image srcImage = m_current->compositionImages[m_current->previousImage].get();
	const vk::ImageLayout srcInitialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	const vk::PipelineStageFlags2 srcStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
	const vk::AccessFlags2 srcAccessMask = vk::AccessFlagBits2::eShaderSampledRead;
	const uint32_t bytesPerSrcPixel = 8; // fp16 RGBA = 4 x 2 bytes
	uint32_t w = m_current->extent.width;
	uint32_t h = m_current->extent.height;
	vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(w) * h * bytesPerSrcPixel;

	// End the current render pass so we can record transfer commands
	m_currentCommandBuffer.endRenderPass();

	// --- One-shot command buffer to copy previous frame to staging buffer ---

	vk::CommandBufferAllocateInfo cmdAlloc;
	cmdAlloc.commandPool = m_graphicsCommandPool.get();
	cmdAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdAlloc.commandBufferCount = 1;

	auto cmdBuffers = m_device->allocateCommandBuffers(cmdAlloc);
	auto cmd = cmdBuffers.front();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	cmd.begin(beginInfo);

	// Transition source image to eTransferSrcOptimal for readback (dest stage is
	// eCopy: this barrier exists solely to feed the cmd.copyImageToBuffer() below)
	ImageBarrier2 preBarrier;
	preBarrier.image = srcImage;
	preBarrier.levelCount = 1;
	preBarrier.layerCount = 1;
	preBarrier.oldLayout = srcInitialLayout;
	preBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	preBarrier.srcStage = srcStageMask;
	preBarrier.srcAccess = srcAccessMask;
	preBarrier.dstStage = vk::PipelineStageFlagBits2::eCopy;
	preBarrier.dstAccess = vk::AccessFlagBits2::eTransferRead;

	cmdImageBarrier(cmd, preBarrier);

	// Create staging buffer for readback
	vk::BufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

	auto stagingBuffer = m_device->createBuffer(bufferCreateInfo);

	VulkanAllocation stagingAlloc{};
	if (!m_memoryManager->allocateBufferMemory(stagingBuffer, MemoryUsage::GpuToCpu, stagingAlloc)) {
		nprintf(("vulkan", "VulkanRenderer::readbackFramebuffer - failed to allocate staging buffer\n"));
		m_device->destroyBuffer(stagingBuffer);
		cmd.end();
		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);

		// Re-begin render pass so the frame can continue
		resumeSwapChainPass();
		return false;
	}

	// Copy image to staging buffer
	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;   // tightly packed
	region.bufferImageHeight = 0; // tightly packed
	region.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
	region.imageOffset = vk::Offset3D(0, 0, 0);
	region.imageExtent = vk::Extent3D(w, h, 1);

	cmd.copyImageToBuffer(srcImage, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, region);

	// Transition source image back to its original layout
	ImageBarrier2 postBarrier;
	postBarrier.image = srcImage;
	postBarrier.levelCount = 1;
	postBarrier.layerCount = 1;
	postBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	postBarrier.newLayout = srcInitialLayout;
	postBarrier.srcStage = vk::PipelineStageFlagBits2::eCopy;
	postBarrier.srcAccess = vk::AccessFlagBits2::eTransferRead;
	// The dst scope must cover every later consumer of this image: the encode
	// pass samples it (eShaderSampledRead @ eFragmentShader), and the next
	// composition pass targeting it performs a layout transition plus a
	// loadOp=eLoad read at begin (read+write @ eColorAttachmentOutput). An
	// empty dst scope here left this transition's write unordered against
	// those loads (READ_AFTER_WRITE flagged by -gr_sync_validation); the fence
	// wait below only synchronizes the host, not later GPU submissions.
	postBarrier.dstStage = vk::PipelineStageFlagBits2::eFragmentShader
	                     | vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	postBarrier.dstAccess = vk::AccessFlagBits2::eShaderSampledRead
	                      | vk::AccessFlagBits2::eColorAttachmentRead
	                      | vk::AccessFlagBits2::eColorAttachmentWrite;

	cmdImageBarrier(cmd, postBarrier);

	cmd.end();

	// Submit one-shot command buffer and wait. The mid-frame fence stall is
	// acceptable: readback is rare and user-triggered (save-screen/screenshot).
	// A stall-free alternative (copy now, map after the next frame's fence)
	// is possible but not worth the complexity today.
	auto fence = m_device->createFence({});

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	m_graphicsQueue.submit(submitInfo, fence);

	auto waitResult = m_device->waitForFences(fence, VK_TRUE, UINT64_MAX);
	if (waitResult != vk::Result::eSuccess) {
		nprintf(("vulkan", "VulkanRenderer::readbackFramebuffer - fence wait failed\n"));
	}

	m_device->destroyFence(fence);
	m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);

	// Read back and convert pixels from staging buffer: fp16 RGBA -> BGRA8.
	// The composition image holds sRGB-encoded values (1.0 == paper white) so we
	// just clamp to [0,1] and scale; no gamma re-encoding needed. Swap R<->B for
	// BGRA output and force alpha = 255 so the bitmap restores as fully opaque.
	const uint32_t outStride = w * 4u; // always 4 bytes/pixel BGRA8 output
	bool success = false;
	auto* mappedPtr = static_cast<ubyte*>(m_memoryManager->mapMemory(stagingAlloc));

	if (mappedPtr) {
		auto* pixels = static_cast<ubyte*>(vm_malloc(static_cast<int>(outStride) * static_cast<int>(h)));
		if (pixels) {
			// TODO: support true HDR10 screenshots. The scene is rendered in fp16, but
			// screenshots are currently tonemapped/clamped down to 8-bit LDR PNG here.
			// Clamp fp16 value to [0,1]; NaN-safe (all NaN comparisons yield false → 0).
			auto hf = [](uint16_t v) -> float {
				float f = graphics::util::half_to_float(v);
				return (f >= 0.0f && f <= 1.0f) ? f : (f >= 1.0f ? 1.0f : 0.0f);
			};
			const auto* src = reinterpret_cast<const uint16_t*>(mappedPtr);
			for (uint32_t i = 0; i < w * h; ++i) {
				pixels[i * 4 + 0] = static_cast<ubyte>(hf(src[i * 4 + 2]) * 255.0f); // B ← src B
				pixels[i * 4 + 1] = static_cast<ubyte>(hf(src[i * 4 + 1]) * 255.0f); // G ← src G
				pixels[i * 4 + 2] = static_cast<ubyte>(hf(src[i * 4 + 0]) * 255.0f); // R ← src R
				pixels[i * 4 + 3] = 255u;                                              // A = opaque
			}
			*outPixels = pixels;
			*outWidth = w;
			*outHeight = h;
			success = true;
		}
		m_memoryManager->unmapMemory(stagingAlloc);
	}

	// Free staging buffer
	m_device->destroyBuffer(stagingBuffer);
	m_memoryManager->freeAllocation(stagingAlloc);

	// Resume the composition pass with loadOp=eLoad, preserving everything drawn
	// into the composition image so far this frame. (The interrupted pass's
	// finalLayout left it in eShaderReadOnlyOptimal, matching the load pass's
	// initialLayout.)
	resumeSwapChainPass();

	return success;
}

bool VulkanRenderer::readbackRenderTarget(tcache_slot_vulkan* ts, ubyte** outPixels, uint32_t* outWidth,
	uint32_t* outHeight)
{
	*outPixels = nullptr;
	*outWidth = 0;
	*outHeight = 0;

	if (!ts || !ts->image || !ts->renderPassLoad || !ts->framebuffer) {
		nprintf(("vulkan", "readbackRenderTarget - render target has no readable/resumable resources\n"));
		return false;
	}

	if (!m_frameInProgress || !m_currentCommandBuffer) {
		nprintf(("vulkan", "readbackRenderTarget - no frame in progress\n"));
		return false;
	}

	const uint32_t w = ts->width;
	const uint32_t h = ts->height;
	if (w == 0 || h == 0) {
		return false;
	}

	// The target's draws are in the current (not-yet-submitted) frame command buffer, so
	// we can't read them via a side command buffer like readbackFramebuffer does. Instead
	// we flush the frame command buffer mid-frame: end the active render-target pass,
	// record the color image -> staging copy into the SAME buffer, submit it (segment 1)
	// with a temporary fence, and host-wait so the pixels are ready. Rendering then
	// resumes on a fresh command buffer (segment 2) that flip() ultimately submits.

	const vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(w) * h * 4u; // R8G8B8A8

	// Allocate the staging buffer BEFORE touching the render pass or image layout. If it
	// fails we return with the render-target pass still active and the image untouched, so
	// the frame carries on exactly as if the capture never happened.
	vk::BufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
	auto stagingBuffer = m_device->createBuffer(bufferCreateInfo);

	VulkanAllocation stagingAlloc{};
	if (!m_memoryManager->allocateBufferMemory(stagingBuffer, MemoryUsage::GpuToCpu, stagingAlloc)) {
		nprintf(("vulkan", "readbackRenderTarget - failed to allocate staging buffer\n"));
		m_device->destroyBuffer(stagingBuffer);
		return false;
	}

	// End the active render-target pass; its finalLayout leaves the color image in
	// eShaderReadOnlyOptimal.
	m_currentCommandBuffer.endRenderPass();

	// Transition color image eShaderReadOnlyOptimal -> eTransferSrcOptimal for the copy.
	ImageBarrier2 preBarrier;
	preBarrier.image = ts->image;
	preBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	preBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	preBarrier.srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	preBarrier.srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
	preBarrier.dstStage = vk::PipelineStageFlagBits2::eCopy;
	preBarrier.dstAccess = vk::AccessFlagBits2::eTransferRead;
	cmdImageBarrier(m_currentCommandBuffer, preBarrier);

	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;   // tightly packed
	region.bufferImageHeight = 0; // tightly packed
	region.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
	region.imageOffset = vk::Offset3D(0, 0, 0);
	region.imageExtent = vk::Extent3D(w, h, 1);
	m_currentCommandBuffer.copyImageToBuffer(ts->image, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, region);

	// Transition color image back to eShaderReadOnlyOptimal (the load-variant resume pass
	// expects this initial layout). The dst scope covers the loadOp=eLoad color read/write
	// when rendering resumes.
	ImageBarrier2 postBarrier;
	postBarrier.image = ts->image;
	postBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	postBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	postBarrier.srcStage = vk::PipelineStageFlagBits2::eCopy;
	postBarrier.srcAccess = vk::AccessFlagBits2::eTransferRead;
	postBarrier.dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput | vk::PipelineStageFlagBits2::eFragmentShader;
	postBarrier.dstAccess = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite |
	                        vk::AccessFlagBits2::eShaderSampledRead;
	cmdImageBarrier(m_currentCommandBuffer, postBarrier);

	// --- Submit segment 1 (RT draws + copy) and wait. No swap-chain image is touched here,
	// so this submission carries no acquire/present semaphores -- just a throwaway fence. ---
	m_currentCommandBuffer.end();

	auto fence = m_device->createFence({});
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = static_cast<uint32_t>(m_currentCommandBuffers.size());
	submitInfo.pCommandBuffers = m_currentCommandBuffers.data();
	m_graphicsQueue.submit(submitInfo, fence);

	auto waitResult = m_device->waitForFences(fence, VK_TRUE, UINT64_MAX);
	if (waitResult != vk::Result::eSuccess) {
		nprintf(("vulkan", "readbackRenderTarget - fence wait failed\n"));
	}
	m_device->destroyFence(fence);

	// Segment 1's command buffers are done executing (fence signaled), so free them now.
	m_device->freeCommandBuffers(m_graphicsCommandPool.get(), m_currentCommandBuffers);
	m_currentCommandBuffers.clear();
	m_currentCommandBuffer = nullptr;

	// Copy pixels out: R8G8B8A8_UNORM is already tightly-packed RGBA with real alpha, so no
	// swizzle and no forced opacity (the icons rely on the transparent background).
	bool success = false;
	auto* mappedPtr = static_cast<ubyte*>(m_memoryManager->mapMemory(stagingAlloc));
	if (mappedPtr) {
		auto* pixels = static_cast<ubyte*>(vm_malloc(static_cast<int>(bufferSize)));
		if (pixels) {
			memcpy(pixels, mappedPtr, static_cast<size_t>(bufferSize));
			*outPixels = pixels;
			*outWidth = w;
			*outHeight = h;
			success = true;
		}
		m_memoryManager->unmapMemory(stagingAlloc);
	}
	m_device->destroyBuffer(stagingBuffer);
	m_memoryManager->freeAllocation(stagingAlloc);

	// --- Begin segment 2: a fresh frame command buffer that flip() will submit+present. ---
	vk::CommandBufferAllocateInfo cmdAlloc;
	cmdAlloc.commandPool = m_graphicsCommandPool.get();
	cmdAlloc.level = vk::CommandBufferLevel::ePrimary;
	cmdAlloc.commandBufferCount = 1;
	auto cmdBufs = m_device->allocateCommandBuffers(cmdAlloc);
	m_currentCommandBuffers.assign(cmdBufs.begin(), cmdBufs.end());
	m_currentCommandBuffer = m_currentCommandBuffers.front();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	m_currentCommandBuffer.begin(beginInfo);

	// A fresh command buffer starts with no Vulkan state, so re-point the state tracker at
	// it and invalidate every cached binding/dynamic-state value. beginFrame() does exactly
	// this (and touches nothing else), so the next tracked draw re-binds pipeline,
	// descriptors, viewport, scissor, etc.
	// NOTE: the fence wait above leaves the device provably idle here, which makes this the only
	// point in an off-screen capture where frame-scoped pools could be recycled. That is
	// deliberately NOT done unconditionally -- this path is shared with gr.screenToBlob(), which
	// mods call mid-frame with plenty of live allocations still to come. Callers that have
	// genuinely finished a frame's worth of work opt in via gr_end_offscreen_frame() instead.
	m_stateTracker->beginFrame(m_currentCommandBuffer);

	// Resume drawing into the target (loadOp=eLoad) so content survives the flush.
	resumeRenderTargetPass(ts);

	return success;
}

uint32_t VulkanRenderer::getMinUniformBufferOffsetAlignment() const
{
	if (!m_physicalDevice) {
		// Fallback to common value if device not initialized
		return 256;
	}

	return static_cast<uint32_t>(m_deviceProperties.limits.minUniformBufferOffsetAlignment);
}

uint32_t VulkanRenderer::getMaxUniformBufferSize() const
{
	if (!m_physicalDevice) {
		return 65536;
	}

	return m_deviceProperties.limits.maxUniformBufferRange;
}

float VulkanRenderer::getMaxAnisotropy() const
{
	if (!m_physicalDevice) {
		return 1.0f;
	}

	return m_deviceProperties.limits.maxSamplerAnisotropy;
}

bool VulkanRenderer::isTextureCompressionBCSupported() const
{
	if (!m_physicalDevice) {
		return false;
	}

	return m_deviceFeatures.textureCompressionBC == VK_TRUE;
}

bool VulkanRenderer::isDepthClampSupported() const
{
	if (!m_physicalDevice) {
		return false;
	}

	return m_deviceFeatures.depthClamp == VK_TRUE;
}

void VulkanRenderer::waitIdle()
{
	if (m_device) {
		m_device->waitIdle();
	}
}

void VulkanRenderer::endOffscreenFrame()
{
	++m_frameNumber;

	// Same call flip() makes, with the frame-in-flight index deliberately unchanged -- it rewinds
	// the bump cursor and bumps its generation so sub-allocations from the frame just finished are
	// invalidated rather than silently overlapped.
	if (m_bufferManager) {
		m_bufferManager->setCurrentFrame(m_currentFrame, m_frameNumber);
	}
}

FrameSyncPoint VulkanRenderer::captureSyncPoint() const
{
	FrameSyncPoint point;
	point.viewport = m_current != nullptr ? m_current->viewport : nullptr;
	point.slot = m_currentFrame;
	point.frameNumber = m_frameNumber;
	return point;
}

bool VulkanRenderer::waitForSyncPoint(const FrameSyncPoint& point, uint64_t timeoutNs)
{
	// Taken during the frame still being recorded: both flip() and endOffscreenFrame() advance
	// m_frameNumber only once the frame is closed out, so this means nothing has been submitted for
	// it yet. Its work cannot be complete, and blocking here would deadlock -- submission happens on
	// this thread.
	if (point.frameNumber >= m_frameNumber) {
		return false;
	}

	// The target went away with its viewport. releaseViewport() drains every frame it owned before
	// letting it go, so all of its work has retired.
	const auto entry = m_targets.find(point.viewport);
	if (entry == m_targets.end()) {
		return true;
	}

	const auto& frame = entry->second->frames[point.slot];

	// The frame has been closed out (checked above) but this slot's fence is not the one guarding
	// it. Three ways to get here, all of them meaning the work has retired:
	//  - the slot was recycled by a later frame, which waitForFrameSlot() only allows once this
	//    fence has been waited on;
	//  - the frame never went through a submit at all, which is the off-screen path -- and
	//    gr_end_offscreen_frame()'s precondition is that its GPU work had already completed;
	//  - the swap chain was rebuilt, which waits for every frame before replacing them.
	// Waiting on the fence now would be waiting for whatever holds the slot today, not for this.
	if (!frame || frame->getSubmittedFrameNumber() != point.frameNumber) {
		return true;
	}

	return frame->waitForFinish(timeoutNs);
}

VkCommandBuffer VulkanRenderer::getVkCurrentCommandBuffer() const
{
	return static_cast<VkCommandBuffer>(m_currentCommandBuffer);
}

void VulkanRenderer::shutdown()
{
	// Wait for all frames to complete to ensure no drawing is in progress when we destroy the
	// device. Every target has its own sync objects, and a target that has not been presented to
	// for a while can still have work in flight, so all of them have to be drained -- not just the
	// one that happens to be current.
	for (auto& entry : m_targets) {
		for (auto& frame : entry.second->frames) {
			if (frame) {
				frame->waitForFinish();
			}
		}
	}
	// For good measure, also wait until the device is idle
	m_device->waitIdle();

	// Shutdown ImGui Vulkan backend before destroying any Vulkan objects
	shutdownImGui();

	// Shutdown managers in reverse order of initialization
	if (m_raytracingManager) {
		setRaytracingManager(nullptr);
		m_raytracingManager->shutdown();
		m_raytracingManager.reset();
	}

	if (m_queryManager) {
		setQueryManager(nullptr);
		m_queryManager->shutdown();
		m_queryManager.reset();
	}

	if (m_postProcessor) {
		setPostProcessor(nullptr);
		m_postProcessor->shutdown();
		m_postProcessor.reset();
	}

	// Clean up shared post-processing manager
	if (graphics::Post_processing_manager) {
		graphics::Post_processing_manager->clear();
		graphics::Post_processing_manager = nullptr;
	}

	if (m_drawManager) {
		setDrawManager(nullptr);
		m_drawManager->shutdown();
		m_drawManager.reset();
	}

	if (m_stateTracker) {
		setStateTracker(nullptr);
		m_stateTracker->shutdown();
		m_stateTracker.reset();
	}

	if (m_pipelineManager) {
		m_pipelineManager->savePipelineCache("vulkan_pipeline.cache");
		setPipelineManager(nullptr);
		m_pipelineManager->shutdown();
		m_pipelineManager.reset();
	}

	if (m_descriptorManager) {
		setDescriptorManager(nullptr);
		m_descriptorManager->shutdown();
		m_descriptorManager.reset();
	}

	if (m_shaderManager) {
		setShaderManager(nullptr);
		m_shaderManager->shutdown();
		m_shaderManager.reset();
	}

	if (m_textureManager) {
		setTextureManager(nullptr);
		m_textureManager->shutdown();
		m_textureManager.reset();
	}

	if (m_bufferManager) {
		setBufferManager(nullptr);
		m_bufferManager->shutdown();
		m_bufferManager.reset();
	}

	// Depth and composition images are backed by the memory manager, so every target has to give
	// them up before it shuts down. The rest of a target (swap chain, views, framebuffers) is
	// vk::Unique* and goes when m_targets does.
	for (auto& entry : m_targets) {
		releaseTargetMemory(*entry.second);
		destroyTargetSwapChain(*entry.second);
		entry.second->surface.reset();
	}

	// Deletion queue must be flushed before memory manager shutdown
	if (m_deletionQueue) {
		setDeletionQueue(nullptr);
		m_deletionQueue->shutdown();
		m_deletionQueue.reset();
	}

	if (m_memoryManager) {
		setMemoryManager(nullptr);
		m_memoryManager->shutdown();
		m_memoryManager.reset();
	}
}

} // namespace graphics::vulkan
