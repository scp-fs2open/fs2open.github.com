#include "VulkanFramebuffer.h"

#ifdef WITH_VULKAN

#include "globalincs/pstypes.h"

namespace graphics {
namespace vulkan {

// ============================================================================
// FramebufferAttachment
// ============================================================================

bool FramebufferAttachment::isDepthFormat() const
{
	switch (format) {
	case vk::Format::eD16Unorm:
	case vk::Format::eD32Sfloat:
	case vk::Format::eD16UnormS8Uint:
	case vk::Format::eD24UnormS8Uint:
	case vk::Format::eD32SfloatS8Uint:
		return true;
	default:
		return false;
	}
}

bool FramebufferAttachment::isStencilFormat() const
{
	switch (format) {
	case vk::Format::eS8Uint:
	case vk::Format::eD16UnormS8Uint:
	case vk::Format::eD24UnormS8Uint:
	case vk::Format::eD32SfloatS8Uint:
		return true;
	default:
		return false;
	}
}

// ============================================================================
// VulkanFramebuffer
// ============================================================================

bool VulkanFramebuffer::create(vk::Device device,
                               vk::PhysicalDevice physicalDevice,
                               uint32_t width,
                               uint32_t height,
                               const SCP_vector<vk::Format>& colorFormats,
                               vk::Format depthFormat)
{
	m_device = device;
	m_extent = vk::Extent2D{width, height};
	m_ownsAttachments = true;

	// Create color attachments
	m_colorAttachments.resize(colorFormats.size());
	for (size_t i = 0; i < colorFormats.size(); ++i) {
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment |
		                            vk::ImageUsageFlagBits::eSampled |
		                            vk::ImageUsageFlagBits::eTransferSrc; // For screenshots

		if (!createAttachment(physicalDevice, m_colorAttachments[i], colorFormats[i], usage,
		        vk::ImageAspectFlagBits::eColor)) {
			mprintf(("Vulkan: Failed to create color attachment %zu\n", i));
			destroy();
			return false;
		}
	}

	// Create depth attachment if requested
	if (depthFormat != vk::Format::eUndefined) {
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eDepth;

		// Add stencil aspect if format includes stencil
		FramebufferAttachment tempAttachment;
		tempAttachment.format = depthFormat;
		if (tempAttachment.isStencilFormat()) {
			aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}

		if (!createAttachment(physicalDevice, m_depthAttachment, depthFormat, usage, aspectMask)) {
			mprintf(("Vulkan: Failed to create depth attachment\n"));
			destroy();
			return false;
		}
	}

	// Note: With dynamic rendering (Vulkan 1.3+), VkFramebuffer objects are not needed.
	// The image views are used directly in VkRenderingAttachmentInfo.

	mprintf(("Vulkan: Created framebuffer attachments %ux%u with %zu color attachment(s)%s\n", width, height,
	    m_colorAttachments.size(), m_depthAttachment.view ? " + depth" : ""));

	return true;
}

bool VulkanFramebuffer::createFromImageViews(vk::Device device,
                                             uint32_t width,
                                             uint32_t height,
                                             const SCP_vector<vk::ImageView>& colorViews,
                                             vk::Format colorFormat,
                                             vk::ImageView depthView,
                                             vk::Format depthFormat)
{
	m_device = device;
	m_extent = vk::Extent2D{width, height};
	m_ownsAttachments = false;

	// Store external views and their formats
	m_externalColorViews = colorViews;
	m_externalColorFormat = colorFormat;
	m_externalDepthView = depthView;
	m_externalDepthFormat = depthFormat;

	// Note: With dynamic rendering (Vulkan 1.3+), VkFramebuffer objects are not needed.
	// The image views are used directly in VkRenderingAttachmentInfo.

	return true;
}

void VulkanFramebuffer::destroy()
{
	if (m_ownsAttachments) {
		// Clean up owned attachments (UniqueHandle types automatically clean up when reset)
		for (auto& attachment : m_colorAttachments) {
			attachment.view.reset();
			attachment.image.reset();
			attachment.memory.reset();
		}
		m_colorAttachments.clear();

		m_depthAttachment.view.reset();
		m_depthAttachment.image.reset();
		m_depthAttachment.memory.reset();
	}

	m_externalColorViews.clear();
	m_externalColorFormat = vk::Format::eUndefined;
	m_externalDepthView = nullptr;
	m_externalDepthFormat = vk::Format::eUndefined;
	m_extent = vk::Extent2D{0, 0};
}

vk::Framebuffer VulkanFramebuffer::getFramebuffer() const
{
	// VkFramebuffer is not used with dynamic rendering (Vulkan 1.3+)
	return nullptr;
}

vk::Extent2D VulkanFramebuffer::getExtent() const { return m_extent; }

size_t VulkanFramebuffer::getColorAttachmentCount() const
{
	if (m_ownsAttachments) {
		return m_colorAttachments.size();
	}
	return m_externalColorViews.size();
}

vk::ImageView VulkanFramebuffer::getColorImageView(size_t index) const
{
	if (m_ownsAttachments) {
		if (index < m_colorAttachments.size()) {
			return m_colorAttachments[index].view.get();
		}
	} else {
		if (index < m_externalColorViews.size()) {
			return m_externalColorViews[index];
		}
	}
	return nullptr;
}

vk::Image VulkanFramebuffer::getColorImage(size_t index) const
{
	if (m_ownsAttachments && index < m_colorAttachments.size()) {
		return m_colorAttachments[index].image.get();
	}
	// External views don't have direct image access
	return nullptr;
}

bool VulkanFramebuffer::hasDepthAttachment() const
{
	if (m_ownsAttachments) {
		return static_cast<bool>(m_depthAttachment.view);
	}
	return m_externalDepthView != nullptr;
}

vk::ImageView VulkanFramebuffer::getDepthImageView() const
{
	if (m_ownsAttachments) {
		return m_depthAttachment.view.get();
	}
	return m_externalDepthView;
}

vk::Image VulkanFramebuffer::getDepthImage() const
{
	if (m_ownsAttachments) {
		return m_depthAttachment.image.get();
	}
	return nullptr;
}

vk::Format VulkanFramebuffer::getColorFormat(size_t index) const
{
	if (m_ownsAttachments) {
		if (index < m_colorAttachments.size()) {
			return m_colorAttachments[index].format;
		}
		return vk::Format::eUndefined;
	}
	// For external views, return the stored format (same for all views)
	if (index < m_externalColorViews.size()) {
		return m_externalColorFormat;
	}
	return vk::Format::eUndefined;
}

vk::Format VulkanFramebuffer::getDepthFormat() const
{
	if (m_ownsAttachments) {
		return m_depthAttachment.format;
	}
	return m_externalDepthFormat;
}

bool VulkanFramebuffer::createAttachment(vk::PhysicalDevice physicalDevice,
                                         FramebufferAttachment& attachment,
                                         vk::Format format,
                                         vk::ImageUsageFlags usage,
                                         vk::ImageAspectFlags aspectMask)
{
	attachment.format = format;
	attachment.extent = m_extent;
	attachment.usage = usage;
	attachment.aspectMask = aspectMask;

	// Create image
	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.extent = vk::Extent3D{m_extent.width, m_extent.height, 1};
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	try {
		attachment.image = m_device.createImageUnique(imageInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create attachment image: %s\n", e.what()));
		return false;
	}

	// Get memory requirements and allocate device-local memory
	vk::MemoryRequirements memRequirements = m_device.getImageMemoryRequirements(attachment.image.get());

	uint32_t memTypeIndex =
	    findMemoryType(physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
	if (memTypeIndex == UINT32_MAX) {
		mprintf(("Vulkan: Failed to find suitable memory type for attachment\n"));
		return false;
	}

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memTypeIndex;

	try {
		attachment.memory = m_device.allocateMemoryUnique(allocInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to allocate attachment memory: %s\n", e.what()));
		return false;
	}

	// Bind memory to image
	try {
		m_device.bindImageMemory(attachment.image.get(), attachment.memory.get(), 0);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to bind attachment memory: %s\n", e.what()));
		return false;
	}

	// Create image view
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.image = attachment.image.get();
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectMask;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	try {
		attachment.view = m_device.createImageViewUnique(viewInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create attachment image view: %s\n", e.what()));
		return false;
	}

	return true;
}

uint32_t VulkanFramebuffer::findMemoryType(vk::PhysicalDevice physicalDevice,
                                           uint32_t typeFilter,
                                           vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return UINT32_MAX;
}

// ============================================================================
// VulkanRenderPassManager
// ============================================================================

bool VulkanRenderPassManager::initialize(vk::Device device)
{
	m_device = device;
	return true;
}

void VulkanRenderPassManager::shutdown()
{
	m_sceneRenderPass.reset();
	m_presentRenderPass.reset();
}

bool VulkanRenderPassManager::createSceneRenderPass(vk::Format colorFormat, vk::Format depthFormat)
{
	std::array<vk::AttachmentDescription, 2> attachments;

	// Attachment 0: Color
	attachments[0].format = colorFormat;
	attachments[0].samples = vk::SampleCountFlagBits::e1;
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	// Attachment 1: Depth
	attachments[1].format = depthFormat;
	attachments[1].samples = vk::SampleCountFlagBits::e1;
	attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
	attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachments[1].initialLayout = vk::ImageLayout::eUndefined;
	attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	// Subpass 0: Main rendering
	vk::AttachmentReference colorRef;
	colorRef.attachment = 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthRef;
	depthRef.attachment = 1;
	depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = &depthRef;

	// Subpass dependencies for proper synchronization
	std::array<vk::SubpassDependency, 2> dependencies;

	// Dependency from external to subpass 0
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask =
	    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependencies[0].srcAccessMask = vk::AccessFlagBits::eNone;
	dependencies[0].dstStageMask =
	    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependencies[0].dstAccessMask =
	    vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	// Dependency from subpass 0 to external (for shader read)
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;

	// Create render pass
	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	try {
		m_sceneRenderPass = m_device.createRenderPassUnique(renderPassInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create scene render pass: %s\n", e.what()));
		return false;
	}

	mprintf(("Vulkan: Created scene render pass (color: %s, depth: %s)\n", vk::to_string(colorFormat).c_str(),
	    vk::to_string(depthFormat).c_str()));

	return true;
}

bool VulkanRenderPassManager::createPresentRenderPass(vk::Format colorFormat)
{
	// Single color attachment for swapchain presentation
	vk::AttachmentDescription colorAttachment;
	colorAttachment.format = colorFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	// Use eClear so direct pass (menu rendering) can clear to background color.
	// For blit pass, the fullscreen quad overwrites everything anyway.
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// Subpass
	vk::AttachmentReference colorRef;
	colorRef.attachment = 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = nullptr; // No depth for present pass

	// Dependency for synchronization
	vk::SubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask =
	    vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eFragmentShader;
	dependency.srcAccessMask = vk::AccessFlagBits::eShaderRead;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	// Create render pass
	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	try {
		m_presentRenderPass = m_device.createRenderPassUnique(renderPassInfo);
	} catch (const vk::SystemError& e) {
		mprintf(("Vulkan: Failed to create present render pass: %s\n", e.what()));
		return false;
	}

	mprintf(("Vulkan: Created present render pass (format: %s)\n", vk::to_string(colorFormat).c_str()));

	return true;
}

vk::RenderPass VulkanRenderPassManager::getSceneRenderPass() const { return m_sceneRenderPass.get(); }

vk::RenderPass VulkanRenderPassManager::getPresentRenderPass() const { return m_presentRenderPass.get(); }

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
