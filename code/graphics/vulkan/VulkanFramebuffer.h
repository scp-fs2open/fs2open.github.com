#pragma once

#include "globalincs/pstypes.h"

#ifdef WITH_VULKAN

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

/**
 * @brief Manages a Vulkan image suitable for use as a framebuffer attachment
 *
 * Handles image creation, memory allocation, and image view creation.
 */
struct FramebufferAttachment {
	vk::UniqueImage image;
	vk::UniqueDeviceMemory memory;
	vk::UniqueImageView view;
	vk::Format format = vk::Format::eUndefined;
	vk::Extent2D extent = {0, 0};
	vk::ImageUsageFlags usage;
	vk::ImageAspectFlags aspectMask;

	bool isDepthFormat() const;
	bool isStencilFormat() const;
};

/**
 * @brief Manages a complete framebuffer with its attachments
 *
 * Can hold multiple color attachments plus optional depth/stencil.
 * Used for scene rendering, post-processing passes, etc.
 *
 * Supports two modes:
 * - Owned attachments: Creates and manages images/memory internally
 * - External views: Wraps existing image views (e.g., swapchain images)
 */
class VulkanFramebuffer {
  public:
	VulkanFramebuffer() = default;
	~VulkanFramebuffer() = default;

	// Non-copyable
	VulkanFramebuffer(const VulkanFramebuffer&) = delete;
	VulkanFramebuffer& operator=(const VulkanFramebuffer&) = delete;

	// Movable
	VulkanFramebuffer(VulkanFramebuffer&&) = default;
	VulkanFramebuffer& operator=(VulkanFramebuffer&&) = default;

	/**
	 * @brief Create framebuffer with owned attachments
	 * @param device Vulkan device
	 * @param physicalDevice For memory type queries
	 * @param width Framebuffer width
	 * @param height Framebuffer height
	 * @param colorFormats Color attachment formats (can be empty for depth-only)
	 * @param depthFormat Depth format (eUndefined for no depth)
	 * @return true on success
	 *
	 * @note With dynamic rendering (Vulkan 1.3+), VkFramebuffer objects are not created.
	 *       This class now only manages the images and their views.
	 */
	bool create(vk::Device device,
	            vk::PhysicalDevice physicalDevice,
	            uint32_t width,
	            uint32_t height,
	            const SCP_vector<vk::Format>& colorFormats,
	            vk::Format depthFormat = vk::Format::eUndefined);

	/**
	 * @brief Create framebuffer wrapping external image views (e.g., swapchain)
	 * @param device Vulkan device
	 * @param width Framebuffer width
	 * @param height Framebuffer height
	 * @param colorViews External color image views to wrap
	 * @param colorFormat Format of the external color views (for getColorFormat())
	 * @param depthView Optional external depth view (nullptr for no depth)
	 * @param depthFormat Format of depth view (for getDepthFormat())
	 * @return true on success
	 *
	 * @note With dynamic rendering (Vulkan 1.3+), VkFramebuffer objects are not created.
	 *       This class now only manages the image views.
	 */
	bool createFromImageViews(vk::Device device,
	                          uint32_t width,
	                          uint32_t height,
	                          const SCP_vector<vk::ImageView>& colorViews,
	                          vk::Format colorFormat,
	                          vk::ImageView depthView = nullptr,
	                          vk::Format depthFormat = vk::Format::eUndefined);

	void destroy();

	vk::Framebuffer getFramebuffer() const;
	vk::Extent2D getExtent() const;

	size_t getColorAttachmentCount() const;
	vk::ImageView getColorImageView(size_t index) const;
	vk::Image getColorImage(size_t index) const;

	bool hasDepthAttachment() const;
	vk::ImageView getDepthImageView() const;
	vk::Image getDepthImage() const;

	/**
	 * @brief Get color attachment format (for dynamic rendering)
	 * @param index Color attachment index
	 * @return Format of the attachment, or eUndefined if invalid index
	 */
	vk::Format getColorFormat(size_t index = 0) const;

	/**
	 * @brief Get depth attachment format (for dynamic rendering)
	 * @return Format of depth attachment, or eUndefined if no depth
	 */
	vk::Format getDepthFormat() const;

  private:
	vk::Device m_device;
	vk::Extent2D m_extent = {0, 0};

	SCP_vector<FramebufferAttachment> m_colorAttachments;
	FramebufferAttachment m_depthAttachment;
	bool m_ownsAttachments = true; // false when wrapping external views

	// For external views (swapchain) - stores views and their formats
	SCP_vector<vk::ImageView> m_externalColorViews;
	vk::Format m_externalColorFormat = vk::Format::eUndefined;
	vk::ImageView m_externalDepthView;
	vk::Format m_externalDepthFormat = vk::Format::eUndefined;

	bool createAttachment(vk::PhysicalDevice physicalDevice,
	                      FramebufferAttachment& attachment,
	                      vk::Format format,
	                      vk::ImageUsageFlags usage,
	                      vk::ImageAspectFlags aspectMask);

	uint32_t findMemoryType(vk::PhysicalDevice physicalDevice,
	                        uint32_t typeFilter,
	                        vk::MemoryPropertyFlags properties);
};

/**
 * @brief Manages render passes for the Vulkan renderer
 *
 * Currently provides:
 * - Scene render pass: Color + depth for 3D geometry rendering
 * - Present render pass: Color only for fullscreen quad blit to swapchain
 */
class VulkanRenderPassManager {
  public:
	bool initialize(vk::Device device);
	void shutdown();

	/**
	 * @brief Create the main scene render pass (color + depth)
	 *
	 * Color attachment:
	 * - loadOp: Clear (clear to background color)
	 * - storeOp: Store (preserve for post-processing)
	 * - initialLayout: Undefined (content discarded)
	 * - finalLayout: ShaderReadOnlyOptimal (for sampling in post-processing)
	 *
	 * Depth attachment:
	 * - loadOp: Clear (clear to 1.0 for reverse-Z or 0.0 for standard)
	 * - storeOp: DontCare (not needed after rendering)
	 * - initialLayout: Undefined
	 * - finalLayout: DepthStencilAttachmentOptimal
	 */
	bool createSceneRenderPass(vk::Format colorFormat, vk::Format depthFormat);

	/**
	 * @brief Create a simple present render pass (color only, no depth)
	 *
	 * For fullscreen quad blit to swapchain - depth testing not needed.
	 *
	 * Color attachment:
	 * - loadOp: DontCare (fullscreen quad overwrites everything)
	 * - storeOp: Store (for presentation)
	 * - initialLayout: Undefined
	 * - finalLayout: PresentSrcKHR
	 */
	bool createPresentRenderPass(vk::Format colorFormat);

	vk::RenderPass getSceneRenderPass() const;
	vk::RenderPass getPresentRenderPass() const;

  private:
	vk::Device m_device;
	vk::UniqueRenderPass m_sceneRenderPass;
	vk::UniqueRenderPass m_presentRenderPass;
};

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
