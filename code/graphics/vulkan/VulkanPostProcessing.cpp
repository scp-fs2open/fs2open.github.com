#include "VulkanPostProcessing.h"

#ifdef WITH_VULKAN

#include "globalincs/systemvars.h"

#include <cstring>

namespace graphics {
namespace vulkan {

// Global post-processing manager instance
VulkanPostProcessing* g_vulkanPostProcessing = nullptr;

// ============================================================================
// VulkanPostProcessing Implementation
// ============================================================================

bool VulkanPostProcessing::initialize(vk::Device device,
                                      vk::PhysicalDevice physicalDevice,
                                      vk::CommandPool commandPool,
                                      uint32_t sceneWidth,
                                      uint32_t sceneHeight,
                                      vk::Format sceneColorFormat)
{
	m_device = device;
	m_physicalDevice = physicalDevice;
	m_commandPool = commandPool;
	m_width = sceneWidth;
	m_height = sceneHeight;
	m_sceneFormat = sceneColorFormat;

	// Bloom operates at half resolution
	m_bloomWidth = sceneWidth / 2;
	m_bloomHeight = sceneHeight / 2;

	// Create linear sampler for texture sampling
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	samplerInfo.maxLod = static_cast<float>(MAX_BLOOM_MIP_LEVELS);

	try {
		m_linearSampler = m_device.createSamplerUnique(samplerInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan PostProcess: Failed to create sampler: %s\n", e.what()));
		return false;
	}

	// Create render passes
	m_postProcessRenderPass = createPostProcessRenderPass(sceneColorFormat);
	if (!m_postProcessRenderPass) {
		mprintf(("Vulkan PostProcess: Failed to create post-process render pass\n"));
		return false;
	}

	// Create LDR output render pass (for final tonemap output)
	m_outputRenderPass = createPostProcessRenderPass(vk::Format::eR8G8B8A8Unorm);
	if (!m_outputRenderPass) {
		mprintf(("Vulkan PostProcess: Failed to create output render pass\n"));
		return false;
	}

	// Create descriptor pool
	std::array<vk::DescriptorPoolSize, 2> poolSizes = {{
		{vk::DescriptorType::eCombinedImageSampler, 16},
		{vk::DescriptorType::eUniformBuffer, 8}
	}};

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.maxSets = 16;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

	try {
		m_descriptorPool = m_device.createDescriptorPoolUnique(poolInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan PostProcess: Failed to create descriptor pool: %s\n", e.what()));
		return false;
	}

	// Create bloom resources
	if (!createBloomResources()) {
		mprintf(("Vulkan PostProcess: Failed to create bloom resources\n"));
		return false;
	}

	// Create tonemapping pipeline
	if (!createTonemapPipeline()) {
		mprintf(("Vulkan PostProcess: Failed to create tonemap pipeline\n"));
		return false;
	}

	// Create bloom pipelines
	if (!createBloomPipelines()) {
		mprintf(("Vulkan PostProcess: Failed to create bloom pipelines\n"));
		return false;
	}

	m_initialized = true;
	mprintf(("Vulkan PostProcess: Initialized (%dx%d, bloom %dx%d)\n",
		m_width, m_height, m_bloomWidth, m_bloomHeight));

	return true;
}

void VulkanPostProcessing::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Wait for device idle before cleanup
	m_device.waitIdle();

	// Reset all resources
	m_bloomBrightPipeline.reset();
	m_bloomBrightPipelineLayout.reset();
	m_bloomBrightDescriptorSetLayout.reset();

	m_bloomBlurHPipeline.reset();
	m_bloomBlurVPipeline.reset();
	m_bloomBlurPipelineLayout.reset();
	m_bloomBlurDescriptorSetLayout.reset();

	m_bloomCompositePipeline.reset();
	m_bloomCompositePipelineLayout.reset();
	m_bloomCompositeDescriptorSetLayout.reset();

	m_tonemapPipeline.reset();
	m_tonemapPipelineLayout.reset();
	m_tonemapDescriptorSetLayout.reset();

	for (int i = 0; i < BLOOM_TEXTURE_COUNT; ++i) {
		m_bloomFramebuffers[i].reset();
		m_bloomViews[i].reset();
		m_bloomImages[i].reset();
		m_bloomMemory[i].reset();
	}

	m_descriptorPool.reset();
	m_linearSampler.reset();
	m_outputRenderPass.reset();
	m_postProcessRenderPass.reset();

	m_initialized = false;
	mprintf(("Vulkan PostProcess: Shutdown complete\n"));
}

bool VulkanPostProcessing::resize(uint32_t newWidth, uint32_t newHeight)
{
	if (!m_initialized) {
		return false;
	}

	// Only resize if dimensions changed
	if (newWidth == m_width && newHeight == m_height) {
		return true;
	}

	m_width = newWidth;
	m_height = newHeight;
	m_bloomWidth = newWidth / 2;
	m_bloomHeight = newHeight / 2;

	// Recreate bloom resources at new size
	// TODO: Implement resize (destroy and recreate bloom textures)

	return true;
}

vk::UniqueRenderPass VulkanPostProcessing::createPostProcessRenderPass(vk::Format colorFormat)
{
	// Single color attachment, no depth
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = colorFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;  // Fullscreen pass overwrites
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentReference colorRef;
	colorRef.attachment = 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = nullptr;

	// Dependencies
	std::array<vk::SubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
	dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	try {
		return m_device.createRenderPassUnique(renderPassInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan PostProcess: Failed to create render pass: %s\n", e.what()));
		return {};
	}
}

bool VulkanPostProcessing::createBloomResources()
{
	// Create bloom textures (half resolution, with mipmaps for blur)
	vk::Format bloomFormat = vk::Format::eR16G16B16A16Sfloat;  // HDR format

	for (int i = 0; i < BLOOM_TEXTURE_COUNT; ++i) {
		// Create image
		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.format = bloomFormat;
		imageInfo.extent = vk::Extent3D{m_bloomWidth, m_bloomHeight, 1};
		imageInfo.mipLevels = MAX_BLOOM_MIP_LEVELS;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment |
		                  vk::ImageUsageFlagBits::eSampled |
		                  vk::ImageUsageFlagBits::eTransferDst |
		                  vk::ImageUsageFlagBits::eTransferSrc;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;

		try {
			m_bloomImages[i] = m_device.createImageUnique(imageInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("Vulkan PostProcess: Failed to create bloom image %d: %s\n", i, e.what()));
			return false;
		}

		// Allocate memory
		vk::MemoryRequirements memReqs = m_device.getImageMemoryRequirements(m_bloomImages[i].get());
		uint32_t memTypeIndex = findMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		if (memTypeIndex == UINT32_MAX) {
			mprintf(("Vulkan PostProcess: Failed to find memory type for bloom image\n"));
			return false;
		}

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = memTypeIndex;

		try {
			m_bloomMemory[i] = m_device.allocateMemoryUnique(allocInfo);
			m_device.bindImageMemory(m_bloomImages[i].get(), m_bloomMemory[i].get(), 0);
		} catch (const vk::SystemError& e) {
			mprintf(("Vulkan PostProcess: Failed to allocate bloom memory %d: %s\n", i, e.what()));
			return false;
		}

		// Create image view
		vk::ImageViewCreateInfo viewInfo;
		viewInfo.image = m_bloomImages[i].get();
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = bloomFormat;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = MAX_BLOOM_MIP_LEVELS;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		try {
			m_bloomViews[i] = m_device.createImageViewUnique(viewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("Vulkan PostProcess: Failed to create bloom view %d: %s\n", i, e.what()));
			return false;
		}

		// Create framebuffer for mip level 0
		m_bloomFramebuffers[i] = std::make_unique<VulkanFramebuffer>();
		
		// Create a view for just mip level 0 for framebuffer
		vk::ImageViewCreateInfo mip0ViewInfo = viewInfo;
		mip0ViewInfo.subresourceRange.levelCount = 1;

		vk::UniqueImageView mip0View;
		try {
			mip0View = m_device.createImageViewUnique(mip0ViewInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("Vulkan PostProcess: Failed to create bloom mip0 view %d: %s\n", i, e.what()));
			return false;
		}

		SCP_vector<vk::ImageView> fbViews = {mip0View.get()};
		if (!m_bloomFramebuffers[i]->createFromImageViews(
			m_device, m_bloomWidth, m_bloomHeight, fbViews, bloomFormat, nullptr, vk::Format::eUndefined)) {
			mprintf(("Vulkan PostProcess: Failed to create bloom framebuffer %d\n", i));
			return false;
		}
	}

	return true;
}

bool VulkanPostProcessing::createTonemapPipeline()
{
	// Create descriptor set layout for tonemapping
	vk::DescriptorSetLayoutBinding samplerBinding;
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerBinding;

	try {
		m_tonemapDescriptorSetLayout = m_device.createDescriptorSetLayoutUnique(layoutInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan PostProcess: Failed to create tonemap descriptor layout: %s\n", e.what()));
		return false;
	}

	// Create pipeline layout with push constants for tonemap parameters
	vk::PushConstantRange pushConstant;
	pushConstant.stageFlags = vk::ShaderStageFlagBits::eFragment;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(float) * 4;  // exposure, tonemapper, hdrMaxNits, hdrPaperWhite

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_tonemapDescriptorSetLayout.get();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

	try {
		m_tonemapPipelineLayout = m_device.createPipelineLayoutUnique(pipelineLayoutInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan PostProcess: Failed to create tonemap pipeline layout: %s\n", e.what()));
		return false;
	}

	// Pipeline creation would require shaders - stub for now
	// The actual pipeline will use the blit shaders modified for tonemapping
	// TODO: Create tonemap shaders and pipeline

	mprintf(("Vulkan PostProcess: Tonemap pipeline layout created (pipeline stub)\n"));
	return true;
}

bool VulkanPostProcessing::createBloomPipelines()
{
	// Create descriptor set layouts for bloom passes
	vk::DescriptorSetLayoutBinding samplerBinding;
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerBinding;

	try {
		m_bloomBrightDescriptorSetLayout = m_device.createDescriptorSetLayoutUnique(layoutInfo);
		m_bloomBlurDescriptorSetLayout = m_device.createDescriptorSetLayoutUnique(layoutInfo);
		m_bloomCompositeDescriptorSetLayout = m_device.createDescriptorSetLayoutUnique(layoutInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan PostProcess: Failed to create bloom descriptor layouts: %s\n", e.what()));
		return false;
	}

	// Create pipeline layouts
	vk::PushConstantRange pushConstant;
	pushConstant.stageFlags = vk::ShaderStageFlagBits::eFragment;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(float) * 4;  // Various per-pass parameters

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

	try {
		pipelineLayoutInfo.pSetLayouts = &m_bloomBrightDescriptorSetLayout.get();
		m_bloomBrightPipelineLayout = m_device.createPipelineLayoutUnique(pipelineLayoutInfo);

		pipelineLayoutInfo.pSetLayouts = &m_bloomBlurDescriptorSetLayout.get();
		m_bloomBlurPipelineLayout = m_device.createPipelineLayoutUnique(pipelineLayoutInfo);

		pipelineLayoutInfo.pSetLayouts = &m_bloomCompositeDescriptorSetLayout.get();
		m_bloomCompositePipelineLayout = m_device.createPipelineLayoutUnique(pipelineLayoutInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan PostProcess: Failed to create bloom pipeline layouts: %s\n", e.what()));
		return false;
	}

	// Pipeline creation would require shaders - stub for now
	// TODO: Create bloom shaders and pipelines

	mprintf(("Vulkan PostProcess: Bloom pipeline layouts created (pipelines stub)\n"));
	return true;
}

void VulkanPostProcessing::beginFrame()
{
	if (!m_initialized) {
		return;
	}
	m_inFrame = true;
}

void VulkanPostProcessing::endFrame(vk::CommandBuffer cmdBuffer,
                                    vk::ImageView sceneColorView,
                                    vk::Image sceneColorImage,
                                    vk::ImageView outputView,
                                    vk::Image outputImage)
{
	if (!m_initialized || !m_inFrame) {
		return;
	}

	// Execute post-processing passes
	// For now, this is a stub - actual implementation would:
	// 1. Run bloom pass (bright -> blur -> composite)
	// 2. Run tonemapping pass

	if (m_bloomEnabled) {
		recordBloomPass(cmdBuffer, sceneColorView, sceneColorImage);
	}

	recordTonemapPass(cmdBuffer, sceneColorView, sceneColorImage, outputView, outputImage);

	m_inFrame = false;
}

void VulkanPostProcessing::recordBloomPass(vk::CommandBuffer cmdBuffer, 
                                           vk::ImageView sceneColorView, 
                                           vk::Image sceneColorImage)
{
	(void)cmdBuffer;
	(void)sceneColorView;
	(void)sceneColorImage;
	
	// TODO: Implement bloom pass
	// 1. Bright pass: Extract bright pixels to bloom texture 0
	// 2. Blur passes: Ping-pong between bloom textures
	// 3. Result is in one of the bloom textures for composite
}

void VulkanPostProcessing::recordTonemapPass(vk::CommandBuffer cmdBuffer,
                                             vk::ImageView inputView,
                                             vk::Image inputImage,
                                             vk::ImageView outputView,
                                             vk::Image outputImage)
{
	(void)cmdBuffer;
	(void)inputView;
	(void)inputImage;
	(void)outputView;
	(void)outputImage;

	// TODO: Implement tonemapping pass
	// 1. Transition input to shader read
	// 2. Transition output to color attachment
	// 3. Begin render pass
	// 4. Bind pipeline and descriptor set
	// 5. Set push constants (exposure, tonemapper type, HDR params)
	// 6. Draw fullscreen triangle
	// 7. End render pass
}

void VulkanPostProcessing::drawFullscreenTriangle(vk::CommandBuffer cmdBuffer)
{
	// Draw a single triangle that covers the screen
	// Vertex positions are generated in the vertex shader using gl_VertexIndex
	cmdBuffer.draw(3, 1, 0, 0);
}

void VulkanPostProcessing::transitionImageLayout(vk::CommandBuffer cmdBuffer,
                                                  vk::Image image,
                                                  vk::ImageLayout oldLayout,
                                                  vk::ImageLayout newLayout,
                                                  vk::PipelineStageFlags srcStage,
                                                  vk::PipelineStageFlags dstStage,
                                                  vk::AccessFlags srcAccess,
                                                  vk::AccessFlags dstAccess)
{
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;

	cmdBuffer.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);
}

void VulkanPostProcessing::setEffect(const char* name, int value, const vec3d* rgb)
{
	(void)name;
	(void)value;
	(void)rgb;
	// TODO: Map effect names to internal parameters
}

void VulkanPostProcessing::setDefaults()
{
	m_bloomEnabled = true;
	m_bloomIntensity = 0.25f;
	m_exposure = 1.0f;
	m_tonemapper = 0;
}

uint32_t VulkanPostProcessing::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProps = m_physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) &&
			(memProps.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return UINT32_MAX;
}

// ============================================================================
// gr_screen Function Implementations
// ============================================================================

void gr_vulkan_post_process_set_effect(const char* name, int value, const vec3d* rgb)
{
	if (g_vulkanPostProcessing) {
		g_vulkanPostProcessing->setEffect(name, value, rgb);
	}
}

void gr_vulkan_post_process_set_defaults()
{
	if (g_vulkanPostProcessing) {
		g_vulkanPostProcessing->setDefaults();
	}
}

void gr_vulkan_post_process_begin()
{
	if (g_vulkanPostProcessing) {
		g_vulkanPostProcessing->beginFrame();
	}
}

void gr_vulkan_post_process_end()
{
	// Note: The actual post-processing is triggered from VulkanRenderer::flip()
	// This function just marks the end of the frame for the engine
	// The endFrame() with actual command buffer recording happens in flip()
}

void gr_vulkan_post_process_save_zbuffer()
{
	// TODO: Implement z-buffer save for post-processing effects that need depth
}

void gr_vulkan_post_process_restore_zbuffer()
{
	// TODO: Implement z-buffer restore
}

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN

