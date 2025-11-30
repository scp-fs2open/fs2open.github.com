#pragma once

#include "osapi/osapi.h"

#include "RenderFrame.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorManager.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipelineManager.h"
#include "VulkanShader.h"

#include <vulkan/vulkan.hpp>

#if SDL_VERSION_ATLEAST(2, 0, 6)
#define SDL_SUPPORTS_VULKAN 1
#else
#define SDL_SUPPORTS_VULKAN 0
#endif

namespace graphics {
namespace vulkan {

struct QueueIndex {
	// Poor mans std::optional
	bool initialized = false;
	uint32_t index = 0;
};

struct PhysicalDeviceValues {
	vk::PhysicalDevice device;
	vk::PhysicalDeviceProperties properties;
	vk::PhysicalDeviceFeatures features;

	std::vector<vk::ExtensionProperties> extensions;

	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<vk::SurfaceFormatKHR> surfaceFormats;
	std::vector<vk::PresentModeKHR> presentModes;

	std::vector<vk::QueueFamilyProperties> queueProperties;
	QueueIndex graphicsQueueIndex;
	QueueIndex transferQueueIndex;
	QueueIndex presentQueueIndex;

	// HDR capability detection
	bool supportsHDR10 = false;
	vk::ColorSpaceKHR preferredHDRColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
};

class VulkanRenderer {
  public:
	explicit VulkanRenderer(std::unique_ptr<os::GraphicsOperations> graphicsOps);

	bool initialize();

	void flip();

	void shutdown();

  private:
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	bool initDisplayDevice() const;

	bool initializeInstance();

	bool initializeSurface();

	bool pickPhysicalDevice(PhysicalDeviceValues& deviceValues);

	bool createLogicalDevice(const PhysicalDeviceValues& deviceValues);

	bool createSwapChain(const PhysicalDeviceValues& deviceValues);

	vk::UniqueShaderModule loadShader(const SCP_string& name);

	void createGraphicsPipeline();

	bool createRenderPasses();

	bool createSceneFramebuffer();

	void createSwapchainFramebuffers();

	vk::Format findDepthFormat();

	void createCommandPool(const PhysicalDeviceValues& values);

	void createPresentSyncObjects();

	void drawScene(vk::Framebuffer destinationFb, vk::CommandBuffer cmdBuffer);

	void acquireNextSwapChainImage();

	std::unique_ptr<os::GraphicsOperations> m_graphicsOps;

	vk::UniqueInstance m_vkInstance;
	vk::UniqueDebugReportCallbackEXT m_debugReport;

	vk::UniqueSurfaceKHR m_vkSurface;

	vk::UniqueDevice m_device;

	vk::Queue m_graphicsQueue;
	vk::Queue m_transferQueue;
	vk::Queue m_presentQueue;

	vk::UniqueSwapchainKHR m_swapChain;
	vk::Format m_swapChainImageFormat;
	vk::ColorSpaceKHR m_swapChainColorSpace;
	vk::Extent2D m_swapChainExtent;
	SCP_vector<vk::Image> m_swapChainImages;
	SCP_vector<vk::UniqueImageView> m_swapChainImageViews;
	SCP_vector<RenderFrame*> m_swapChainImageRenderImage;

	uint32_t m_currentSwapChainImage = 0;

	// Render pass management
	std::unique_ptr<VulkanRenderPassManager> m_renderPassManager;
	vk::Format m_depthFormat = vk::Format::eUndefined;

	// Framebuffer management
	std::unique_ptr<VulkanFramebuffer> m_sceneFramebuffer;
	SCP_vector<std::unique_ptr<VulkanFramebuffer>> m_swapchainFramebuffers;

	vk::UniquePipelineLayout m_pipelineLayout;
	vk::UniquePipeline m_graphicsPipeline;

	uint32_t m_currentFrame = 0;
	std::array<std::unique_ptr<RenderFrame>, MAX_FRAMES_IN_FLIGHT> m_frames;

	vk::UniqueCommandPool m_graphicsCommandPool;

	// Buffer management
	std::unique_ptr<VulkanBufferManager> m_bufferManager;
	vk::PhysicalDevice m_physicalDevice;  // Cached for buffer manager

	// Shader management
	std::unique_ptr<VulkanShaderManager> m_shaderManager;

	// Descriptor management
	std::unique_ptr<VulkanDescriptorManager> m_descriptorManager;

	// Pipeline management
	std::unique_ptr<VulkanPipelineManager> m_pipelineManager;

#if SDL_SUPPORTS_VULKAN
	bool m_debugReportEnabled = false;
#endif
};

} // namespace vulkan
} // namespace graphics
