#pragma once

#include "osapi/osapi.h"

#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
#include "VulkanDescriptorManager.h"
#include "VulkanPipeline.h"
#include "VulkanState.h"
#include "VulkanDraw.h"
#include "VulkanDeletionQueue.h"
#include "VulkanPostProcessing.h"
#include "VulkanQuery.h"
#include "VulkanRenderFrame.h"

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

	SCP_vector<vk::ExtensionProperties> extensions;

	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	SCP_vector<vk::SurfaceFormatKHR> surfaceFormats;
	SCP_vector<vk::PresentModeKHR> presentModes;

	SCP_vector<vk::QueueFamilyProperties> queueProperties;
	QueueIndex graphicsQueueIndex;
	QueueIndex transferQueueIndex;
	QueueIndex presentQueueIndex;
};

class VulkanRenderer {
  public:
	explicit VulkanRenderer(std::unique_ptr<os::GraphicsOperations> graphicsOps);

	bool initialize();

	/**
	 * @brief Setup for a new frame - begins command buffer and render pass
	 * Called at the START of each frame before any draw calls
	 */
	void setupFrame();

	/**
	 * @brief End frame - ends render pass, submits, and presents
	 * Called at the END of each frame after all draw calls
	 */
	void flip();

	void shutdown();

	/**
	 * @brief Read back the previous frame's framebuffer to CPU memory
	 *
	 * Copies the previously presented swap chain image to a vm_malloc'd RGBA
	 * pixel buffer. Handles the BGRA→RGBA swizzle since the swap chain uses
	 * B8G8R8A8 format. Caller must vm_free the returned buffer.
	 *
	 * @param[out] outPixels Receives the vm_malloc'd RGBA pixel buffer
	 * @param[out] outWidth  Receives the image width
	 * @param[out] outHeight Receives the image height
	 * @return true on success, false on failure
	 */
	bool readbackFramebuffer(ubyte** outPixels, uint32_t* outWidth, uint32_t* outHeight);

	/**
	 * @brief Get the minimum uniform buffer offset alignment requirement
	 * @return The alignment in bytes (typically 64 or 256)
	 */
	uint32_t getMinUniformBufferOffsetAlignment() const;

	/**
	 * @brief Get the current frame number (total frames rendered)
	 */
	uint64_t getCurrentFrameNumber() const { return m_frameNumber; }

	/**
	 * @brief Wait for a specific frame's GPU work to complete
	 *
	 * Waits on that frame's fence rather than stalling the entire device.
	 * No-op if the frame has already completed.
	 */
	void waitForFrame(uint64_t frameNumber);

	/**
	 * @brief Wait for all GPU work to complete
	 */
	void waitIdle();

	/**
	 * @brief Get the current command buffer as a raw Vulkan handle (for ImGui)
	 */
	VkCommandBuffer getVkCurrentCommandBuffer() const;

	/**
	 * @brief Check if VK_EXT_debug_utils is enabled
	 */
	bool isDebugUtilsEnabled() const { return m_debugUtilsEnabled; }

	/**
	 * @brief Get the maximum uniform buffer range
	 */
	uint32_t getMaxUniformBufferSize() const;

	/**
	 * @brief Get the maximum sampler anisotropy
	 */
	float getMaxAnisotropy() const;

	/**
	 * @brief Check if BC texture compression is supported
	 */
	bool isTextureCompressionBCSupported() const;

	/**
	 * @brief Check if vertex shader layer output is supported (for shadow cascades)
	 */
	bool supportsShaderViewportLayerOutput() const { return m_supportsShaderViewportLayerOutput; }

	/**
	 * @brief Switch from swap chain pass to HDR scene pass
	 *
	 * Called by vulkan_scene_texture_begin(). Ends the current swap chain
	 * render pass and begins the HDR scene render pass.
	 */
	void beginSceneRendering();

	/**
	 * @brief Switch from HDR scene pass back to swap chain
	 *
	 * Called by vulkan_scene_texture_end(). Ends the HDR scene render pass,
	 * runs post-processing, and begins the resumed swap chain render pass.
	 */
	void endSceneRendering();

	/**
	 * @brief Copy scene color to effect texture mid-scene
	 *
	 * Called by vulkan_copy_effect_texture(). Ends the current scene render
	 * pass, copies scene color → effect texture, then resumes the scene
	 * render pass with loadOp=eLoad to preserve existing content.
	 */
	void copyEffectTexture();

	/**
	 * @brief Copy scene depth mid-scene for soft particle sampling
	 *
	 * Called lazily from the first particle draw per frame. Ends the current
	 * scene render pass, copies depth → samplable copy, then resumes the
	 * scene render pass with loadOp=eLoad. No-op if already copied this frame.
	 */
	void copySceneDepthForParticles();

	/**
	 * @brief Check if scene depth copy is available for sampling this frame
	 */
	bool isSceneDepthCopied() const { return m_sceneDepthCopiedThisFrame; }

	/**
	 * @brief Check if we're currently rendering to the HDR scene target
	 */
	bool isSceneRendering() const { return m_sceneRendering; }

	/**
	 * @brief Set whether the G-buffer render pass is active
	 *
	 * Called by deferred_lighting_finish() to switch from G-buffer to
	 * scene render pass mid-frame for forward transparent rendering.
	 */
	void setUseGbufRenderPass(bool use) { m_useGbufRenderPass = use; }
	bool isUsingGbufRenderPass() const { return m_useGbufRenderPass; }

	/**
	 * @brief Get the validated MSAA sample count for deferred lighting
	 */
	vk::SampleCountFlagBits getMsaaSampleCount() const { return m_msaaSampleCount; }

  private:
	bool initDisplayDevice() const;

	bool initializeInstance();

	bool initializeSurface();

	bool pickPhysicalDevice(PhysicalDeviceValues& deviceValues);

	bool createLogicalDevice(const PhysicalDeviceValues& deviceValues);

	bool createSwapChain(const PhysicalDeviceValues& deviceValues, vk::SwapchainKHR oldSwapchain = nullptr);

	void createRenderPass();

	void createFrameBuffers();

	void createDepthResources();

	vk::Format findDepthFormat();

	void createCommandPool(const PhysicalDeviceValues& values);

	void createPresentSyncObjects();

	void acquireNextSwapChainImage();

	bool recreateSwapChain();

	void createImGuiDescriptorPool();
	void initImGui();
	void shutdownImGui();

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
	vk::Extent2D m_swapChainExtent;
	SCP_vector<vk::Image> m_swapChainImages;
	SCP_vector<vk::UniqueImageView> m_swapChainImageViews;
	SCP_vector<vk::UniqueFramebuffer> m_swapChainFramebuffers;
	SCP_vector<VulkanRenderFrame*> m_swapChainImageRenderImage;

	uint32_t m_currentSwapChainImage = 0;
	uint32_t m_previousSwapChainImage = UINT32_MAX;  // For saveScreen() readback of previous frame

	// Depth buffer
	vk::UniqueImage m_depthImage;
	vk::UniqueImageView m_depthImageView;
	VulkanAllocation m_depthImageMemory;
	vk::Format m_depthFormat = vk::Format::eUndefined;

	vk::UniqueRenderPass m_renderPass;        // Swap chain pass with loadOp=eClear
	vk::UniqueRenderPass m_renderPassLoad;    // Swap chain pass with loadOp=eLoad (resumed after post-processing)
	vk::UniqueDescriptorPool m_imguiDescriptorPool;

	uint32_t m_currentFrame = 0;
	uint64_t m_frameNumber = 0;  // Total frames rendered (for sync tracking)
	std::array<std::unique_ptr<VulkanRenderFrame>, MAX_FRAMES_IN_FLIGHT> m_frames;

	vk::UniqueCommandPool m_graphicsCommandPool;

	// Current frame command buffer (valid between setupFrame and flip)
	vk::CommandBuffer m_currentCommandBuffer;
	SCP_vector<vk::CommandBuffer> m_currentCommandBuffers;  // For cleanup
	bool m_frameInProgress = false;

	// Swap chain recreation
	bool m_swapChainNeedsRecreation = false;

	// Physical device info (needed for memory manager)
	vk::PhysicalDevice m_physicalDevice;
	uint32_t m_graphicsQueueFamilyIndex = 0;
	uint32_t m_transferQueueFamilyIndex = 0;
	uint32_t m_presentQueueFamilyIndex = 0;

	// Memory, buffer, and texture management
	std::unique_ptr<VulkanMemoryManager> m_memoryManager;
	std::unique_ptr<VulkanBufferManager> m_bufferManager;
	std::unique_ptr<VulkanTextureManager> m_textureManager;
	std::unique_ptr<VulkanDeletionQueue> m_deletionQueue;

	// Shader, descriptor, and pipeline management
	std::unique_ptr<VulkanShaderManager> m_shaderManager;
	std::unique_ptr<VulkanDescriptorManager> m_descriptorManager;
	std::unique_ptr<VulkanPipelineManager> m_pipelineManager;

	// State tracking and draw management
	std::unique_ptr<VulkanStateTracker> m_stateTracker;
	std::unique_ptr<VulkanDrawManager> m_drawManager;

	// Query management (GPU timestamp profiling)
	std::unique_ptr<VulkanQueryManager> m_queryManager;

	// Post-processing
	std::unique_ptr<VulkanPostProcessor> m_postProcessor;
	bool m_sceneRendering = false;
	bool m_sceneDepthCopiedThisFrame = false;
	bool m_useGbufRenderPass = false;  // True when scene uses G-buffer (deferred lighting)

	bool m_supportsShaderViewportLayerOutput = false;  // VK_EXT_shader_viewport_index_layer
	vk::SampleCountFlagBits m_msaaSampleCount = vk::SampleCountFlagBits::e1;  // Validated MSAA sample count

#if SDL_SUPPORTS_VULKAN
	bool m_debugReportEnabled = false;
	bool m_debugUtilsEnabled = false;
#endif

};

} // namespace vulkan
} // namespace graphics
