
#include "VulkanRenderer.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

#include "graphics/grinternal.h"
#include "graphics/post_processing.h"

#include "backends/imgui_impl_vulkan.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "mod_table/mod_table.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

extern float flFrametime;

namespace graphics::vulkan {


VulkanRenderer::VulkanRenderer(std::unique_ptr<os::GraphicsOperations> graphicsOps)
	: m_graphicsOps(std::move(graphicsOps))
{
}
void VulkanRenderer::createFrameBuffers()
{
	m_swapChainFramebuffers.reserve(m_swapChainImageViews.size());
	for (const auto& imageView : m_swapChainImageViews) {
		// Attachment 0: color, Attachment 1: depth (shared across all framebuffers)
		const vk::ImageView attachments[] = {
			imageView.get(),
			m_depthImageView.get(),
		};

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.renderPass = m_renderPass.get();
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		m_swapChainFramebuffers.push_back(m_device->createFramebufferUnique(framebufferInfo));
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
void VulkanRenderer::createDepthResources()
{
	m_depthFormat = findDepthFormat();

	// Create depth image
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = m_depthFormat;
	imageInfo.extent.width = m_swapChainExtent.width;
	imageInfo.extent.height = m_swapChainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	m_depthImage = m_device->createImageUnique(imageInfo);

	// Allocate GPU memory for the depth image
	m_memoryManager->allocateImageMemory(m_depthImage.get(), MemoryUsage::GpuOnly, m_depthImageMemory);

	// Create depth image view
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = m_depthImage.get();
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = m_depthFormat;
	viewInfo.subresourceRange.aspectMask = imageAspectFromFormat(m_depthFormat);
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	m_depthImageView = m_device->createImageViewUnique(viewInfo);

	mprintf(("Vulkan: Created depth buffer (%dx%d, format %d)\n",
		m_swapChainExtent.width, m_swapChainExtent.height, static_cast<int>(m_depthFormat)));
}
void VulkanRenderer::createRenderPass()
{
	// Attachment 0: Color - clear each frame
	// UI screens draw their own full-screen backgrounds; 3D clears via scene_texture_begin.
	// Popups that need previous frame content use gr_save_screen/gr_restore_screen.
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

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

	vk::SubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput
	                        | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
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

	// Create a second render pass with loadOp=eLoad for resuming the swap chain
	// after post-processing. Same formats/samples = render-pass-compatible with m_renderPass.
	colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	colorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;

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
void VulkanRenderer::createPresentSyncObjects()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_frames[i].reset(new VulkanRenderFrame(m_device.get(), m_swapChain.get(), m_graphicsQueue, m_presentQueue));
	}

	m_swapChainImageRenderImage.resize(m_swapChainImages.size(), nullptr);
}

bool VulkanRenderer::readbackFramebuffer(ubyte** outPixels, uint32_t* outWidth, uint32_t* outHeight)
{
	*outPixels = nullptr;
	*outWidth = 0;
	*outHeight = 0;

	if (m_previousSwapChainImage == UINT32_MAX) {
		mprintf(("VulkanRenderer::readbackFramebuffer - no previous frame available\n"));
		return false;
	}

	if (!m_frameInProgress) {
		mprintf(("VulkanRenderer::readbackFramebuffer - no frame in progress\n"));
		return false;
	}

	auto prevImage = m_swapChainImages[m_previousSwapChainImage];
	uint32_t w = m_swapChainExtent.width;
	uint32_t h = m_swapChainExtent.height;
	vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(w) * h * 4;

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

	// Transition previous swap chain image for transfer read
	vk::ImageMemoryBarrier preBarrier;
	preBarrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
	preBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	preBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	preBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	preBarrier.image = prevImage;
	preBarrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
	preBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	preBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eTransfer,
		{}, nullptr, nullptr, preBarrier);

	// Create staging buffer for readback
	vk::BufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

	auto stagingBuffer = m_device->createBuffer(bufferCreateInfo);

	VulkanAllocation stagingAlloc{};
	if (!m_memoryManager->allocateBufferMemory(stagingBuffer, MemoryUsage::GpuToCpu, stagingAlloc)) {
		mprintf(("VulkanRenderer::readbackFramebuffer - failed to allocate staging buffer\n"));
		m_device->destroyBuffer(stagingBuffer);
		cmd.end();
		m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);

		// Re-begin render pass so the frame can continue
		vk::RenderPassBeginInfo renderPassBegin;
		renderPassBegin.renderPass = m_renderPass.get();
		renderPassBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
		renderPassBegin.renderArea.offset.x = 0;
		renderPassBegin.renderArea.offset.y = 0;
		renderPassBegin.renderArea.extent = m_swapChainExtent;
		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
		clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
		renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBegin.pClearValues = clearValues.data();
		m_currentCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
		m_stateTracker->setRenderPass(m_renderPass.get(), 0);
		m_stateTracker->setViewport(0.0f,
			static_cast<float>(m_swapChainExtent.height),
			static_cast<float>(m_swapChainExtent.width),
			-static_cast<float>(m_swapChainExtent.height));
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

	cmd.copyImageToBuffer(prevImage, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, region);

	// Transition previous swap chain image back
	vk::ImageMemoryBarrier postBarrier;
	postBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
	postBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
	postBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postBarrier.image = prevImage;
	postBarrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
	postBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	postBarrier.dstAccessMask = {};

	cmd.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eBottomOfPipe,
		{}, nullptr, nullptr, postBarrier);

	cmd.end();

	// Submit one-shot command buffer and wait
	auto fence = m_device->createFence({});

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;
	m_graphicsQueue.submit(submitInfo, fence);

	auto waitResult = m_device->waitForFences(fence, VK_TRUE, UINT64_MAX);
	if (waitResult != vk::Result::eSuccess) {
		mprintf(("VulkanRenderer::readbackFramebuffer - fence wait failed\n"));
	}

	m_device->destroyFence(fence);
	m_device->freeCommandBuffers(m_graphicsCommandPool.get(), cmdBuffers);

	// Read back pixels from staging buffer (raw BGRA matching swap chain format)
	bool success = false;
	auto* mappedPtr = static_cast<ubyte*>(m_memoryManager->mapMemory(stagingAlloc));

	if (mappedPtr) {
		auto* pixels = static_cast<ubyte*>(vm_malloc(static_cast<int>(bufferSize)));
		if (pixels) {
			memcpy(pixels, mappedPtr, bufferSize);
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

	// Re-begin render pass on main command buffer
	vk::RenderPassBeginInfo renderPassBegin;
	renderPassBegin.renderPass = m_renderPass.get();
	renderPassBegin.framebuffer = m_swapChainFramebuffers[m_currentSwapChainImage].get();
	renderPassBegin.renderArea.offset.x = 0;
	renderPassBegin.renderArea.offset.y = 0;
	renderPassBegin.renderArea.extent = m_swapChainExtent;

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	renderPassBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBegin.pClearValues = clearValues.data();

	m_currentCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

	m_stateTracker->setRenderPass(m_renderPass.get(), 0);
	m_stateTracker->setViewport(0.0f,
		static_cast<float>(m_swapChainExtent.height),
		static_cast<float>(m_swapChainExtent.width),
		-static_cast<float>(m_swapChainExtent.height));

	return success;
}

uint32_t VulkanRenderer::getMinUniformBufferOffsetAlignment() const
{
	if (!m_physicalDevice) {
		// Fallback to common value if device not initialized
		return 256;
	}

	auto properties = m_physicalDevice.getProperties();
	return static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
}

uint32_t VulkanRenderer::getMaxUniformBufferSize() const
{
	if (!m_physicalDevice) {
		return 65536;
	}

	auto properties = m_physicalDevice.getProperties();
	return properties.limits.maxUniformBufferRange;
}

float VulkanRenderer::getMaxAnisotropy() const
{
	if (!m_physicalDevice) {
		return 1.0f;
	}

	auto properties = m_physicalDevice.getProperties();
	return properties.limits.maxSamplerAnisotropy;
}

bool VulkanRenderer::isTextureCompressionBCSupported() const
{
	if (!m_physicalDevice) {
		return false;
	}

	auto features = m_physicalDevice.getFeatures();
	return features.textureCompressionBC == VK_TRUE;
}

void VulkanRenderer::waitIdle()
{
	if (m_device) {
		m_device->waitIdle();
	}
}

void VulkanRenderer::waitForFrame(uint64_t frameNumber)
{
	// Fast path: if enough frames have elapsed, the work is definitely done
	if (m_frameNumber >= frameNumber + MAX_FRAMES_IN_FLIGHT) {
		return;
	}

	// Wait on the specific frame's fence
	auto frameIndex = static_cast<uint32_t>(frameNumber % MAX_FRAMES_IN_FLIGHT);
	m_frames[frameIndex]->waitForFinish();
}

VkCommandBuffer VulkanRenderer::getVkCurrentCommandBuffer() const
{
	return static_cast<VkCommandBuffer>(m_currentCommandBuffer);
}

void VulkanRenderer::shutdown()
{
	// Wait for all frames to complete to ensure no drawing is in progress when we destroy the device
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		m_frames[i]->waitForFinish();
	}
	// For good measure, also wait until the device is idle
	m_device->waitIdle();

	// Shutdown ImGui Vulkan backend before destroying any Vulkan objects
	shutdownImGui();

	// Shutdown managers in reverse order of initialization
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

	// Destroy depth resources before memory manager
	m_depthImageView.reset();
	m_depthImage.reset();
	if (m_memoryManager && m_depthImageMemory.isValid()) {
		m_memoryManager->freeAllocation(m_depthImageMemory);
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
