#pragma once

#include "osapi/osapi.h"

#include "RenderFrame.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorManager.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipelineManager.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"

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
	// Draw state tracking - avoids redundant bindings
	struct DrawState {
		vk::Pipeline boundPipeline;
		vk::PipelineLayout boundLayout;
		gr_buffer_handle boundVertexBuffer;
		gr_buffer_handle boundIndexBuffer;
		vk::DeviceSize boundVertexOffset = 0;
		vk::DeviceSize boundIndexOffset = 0;
		bool viewportSet = false;
		bool scissorSet = false;
		
		void reset() {
			boundPipeline = nullptr;
			boundLayout = nullptr;
			boundVertexBuffer = gr_buffer_handle();
			boundIndexBuffer = gr_buffer_handle();
			boundVertexOffset = 0;
			boundIndexOffset = 0;
			viewportSet = false;
			scissorSet = false;
		}
	};

	explicit VulkanRenderer(std::unique_ptr<os::GraphicsOperations> graphicsOps);

	bool initialize();

	void flip();

	void shutdown();

	// Scene pass control - called from gr_screen hooks
	void beginScenePass();
	void endScenePass();

	/**
	 * @brief Ensure a render pass is active for drawing
	 * 
	 * If scene pass is active, does nothing.
	 * Otherwise, starts a direct-to-swapchain render pass for menu/UI rendering.
	 * This is called automatically by draw functions.
	 */
	void ensureRenderPassActive();

	// Command buffer access for draw recording
	/**
	 * @brief Get current command buffer for recording draw commands
	 * @return Command buffer if in active render pass, null handle otherwise
	 */
	vk::CommandBuffer getCurrentCommandBuffer() const;

	/**
	 * @brief Get current render pass for pipeline creation
	 * @return Current render pass if active, null handle otherwise
	 */
	vk::RenderPass getCurrentRenderPass() const;

	/**
	 * @brief Get scene extent for viewport/scissor setup
	 */
	vk::Extent2D getSceneExtent() const { return m_sceneExtent; }

	/**
	 * @brief Get the device for resource creation
	 */
	vk::Device getDevice() const { return m_device.get(); }

	/**
	 * @brief Get the physical device for resource creation
	 */
	vk::PhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

	/**
	 * @brief Get the scene render pass (for render target framebuffer creation)
	 */
	vk::RenderPass getSceneRenderPass() const;

	/**
	 * @brief Get mutable draw state for render functions
	 */
	DrawState& getDrawState() { return m_drawState; }

	/**
	 * @brief Get descriptor manager for binding resources
	 */
	VulkanDescriptorManager* getDescriptorManager() const { return m_descriptorManager.get(); }

	/**
	 * @brief Queue descriptor set for freeing after the current frame
	 */
	void queueDescriptorSetFree(vk::DescriptorSet set);

	/**
	 * @brief Reset draw state (call at start of scene pass)
	 */
	void resetDrawState() { m_drawState.reset(); }

	/**
	 * @brief Set active render target
	 * @param framebuffer Framebuffer to use (nullptr for scene framebuffer)
	 * @param extent Extent of render target
	 * @param bitmapHandle Bitmap handle (-1 for scene)
	 */
	void setActiveRenderTarget(VulkanFramebuffer* framebuffer, vk::Extent2D extent, int bitmapHandle = -1);
	
	/**
	 * @brief Get active render target framebuffer
	 */
	VulkanFramebuffer* getActiveRenderTarget() const {
		return m_activeRenderTarget.isActive ? m_activeRenderTarget.framebuffer : nullptr;
	}

	/**
	 * @brief Store framebuffer for a render target bitmap handle
	 */
	void storeRenderTargetFramebuffer(int bitmapHandle, std::unique_ptr<VulkanFramebuffer> framebuffer);

	/**
	 * @brief Get render target framebuffers map (for access by gr_vulkan functions)
	 */
	const std::map<int, std::unique_ptr<VulkanFramebuffer>>& getRenderTargetFramebuffers() const {
		return m_renderTargetFramebuffers;
	}

	/**
	 * @brief Set clear color for scene rendering
	 */
	void setClearColor(float r, float g, float b, float a) {
		m_clearColor[0] = r;
		m_clearColor[1] = g;
		m_clearColor[2] = b;
		m_clearColor[3] = a;
	}

	/**
	 * @brief Get clear color
	 */
	const std::array<float, 4>& getClearColor() const { return m_clearColor; }

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
	void createTransferCommandPool(const PhysicalDeviceValues& values);

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

	// Per-swapchain-image semaphores (indexed by acquired image index, not frame index)
	// This fixes the semaphore reuse validation error - presentation holds onto semaphores
	// until the specific image is re-acquired, so we need one set per swapchain image
	SCP_vector<vk::UniqueSemaphore> m_imageAvailableSemaphores;
	SCP_vector<vk::UniqueSemaphore> m_renderingFinishedSemaphores;

	vk::UniqueCommandPool m_graphicsCommandPool;
	vk::UniqueCommandPool m_transferCommandPool;

	// Buffer management
	std::unique_ptr<VulkanBufferManager> m_bufferManager;
	vk::PhysicalDevice m_physicalDevice;  // Cached for buffer manager

	// Texture management
	std::unique_ptr<VulkanTextureManager> m_textureManager;

	// Shader management
	std::unique_ptr<VulkanShaderManager> m_shaderManager;

	// Descriptor management
	std::unique_ptr<VulkanDescriptorManager> m_descriptorManager;

	// Pipeline management
	std::unique_ptr<VulkanPipelineManager> m_pipelineManager;

	// Active render target tracking
	struct RenderTargetInfo {
		VulkanFramebuffer* framebuffer = nullptr;
		vk::Extent2D extent;
		int bitmapHandle = -1;
		bool isActive = false;
	};
	RenderTargetInfo m_activeRenderTarget;
	
	// Map of bitmap handles to render target framebuffers
	std::map<int, std::unique_ptr<VulkanFramebuffer>> m_renderTargetFramebuffers;

	// Scene pass state
	bool m_scenePassActive = false;
	bool m_directPassActive = false;  // Direct-to-swapchain pass (for menus)
	vk::CommandBuffer m_sceneCommandBuffer;  // Allocated per-frame for scene rendering
	DrawState m_drawState;
	vk::Extent2D m_sceneExtent = {0, 0};
	std::array<float, 4> m_clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

	// Blit pipeline for scene-to-swapchain copy
	vk::UniquePipelineLayout m_blitPipelineLayout;
	vk::UniquePipeline m_blitPipeline;
	vk::UniqueDescriptorSetLayout m_blitDescriptorSetLayout;
	vk::UniqueSampler m_blitSampler;

	bool createBlitPipeline();
	void recordBlitToSwapchain(vk::CommandBuffer cmdBuffer);

	// Swapchain recreation
	bool m_swapchainNeedsRecreate = false;
	PhysicalDeviceValues m_cachedDeviceValues;  // Cached for swapchain recreation

	void waitForAllFrames();
	bool recreateSwapChain();
	void cleanupSwapChain();

#if SDL_SUPPORTS_VULKAN
	bool m_debugReportEnabled = false;
#endif
};

} // namespace vulkan
} // namespace graphics
