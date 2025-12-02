#pragma once

#include "globalincs/pstypes.h"

#ifdef WITH_VULKAN

#include "VulkanFramebuffer.h"

#include <array>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

class VulkanRenderer;

/**
 * @brief Post-processing pipeline for Vulkan renderer
 *
 * Manages the multi-pass post-processing pipeline:
 * - Bloom (bright pass, blur iterations, composite)
 * - Tonemapping (HDR to SDR/HDR10 output)
 * - Future: SSAO, TAA, lightshafts
 *
 * Uses ping-pong textures for multi-pass effects.
 */
class VulkanPostProcessing {
public:
	VulkanPostProcessing() = default;
	~VulkanPostProcessing() { shutdown(); }

	// Non-copyable
	VulkanPostProcessing(const VulkanPostProcessing&) = delete;
	VulkanPostProcessing& operator=(const VulkanPostProcessing&) = delete;

	/**
	 * @brief Initialize post-processing resources
	 * @param device Vulkan device
	 * @param physicalDevice Physical device for memory queries
	 * @param sceneWidth Scene texture width
	 * @param sceneHeight Scene texture height
	 * @param sceneColorFormat Scene color format (typically R16G16B16A16_SFLOAT for HDR)
	 * @return true on success
	 */
	bool initialize(vk::Device device,
	                vk::PhysicalDevice physicalDevice,
	                vk::CommandPool commandPool,
	                uint32_t sceneWidth,
	                uint32_t sceneHeight,
	                vk::Format sceneColorFormat);

	/**
	 * @brief Shutdown and release all resources
	 */
	void shutdown();

	/**
	 * @brief Resize post-processing resources (e.g., window resize)
	 */
	bool resize(uint32_t newWidth, uint32_t newHeight);

	/**
	 * @brief Check if post-processing is initialized
	 */
	bool isInitialized() const { return m_initialized; }

	/**
	 * @brief Begin post-processing frame
	 *
	 * Called at the start of scene rendering. Prepares resources.
	 */
	void beginFrame();

	/**
	 * @brief End post-processing frame and execute all passes
	 *
	 * Called after scene rendering is complete. Executes:
	 * 1. Bloom (if enabled)
	 * 2. Tonemapping
	 *
	 * @param cmdBuffer Command buffer to record into
	 * @param sceneColorView Scene color texture to process
	 * @param outputView Output texture (typically LDR scene or swapchain)
	 */
	void endFrame(vk::CommandBuffer cmdBuffer,
	              vk::ImageView sceneColorView,
	              vk::Image sceneColorImage,
	              vk::ImageView outputView,
	              vk::Image outputImage);

	/**
	 * @brief Set post-processing effect parameter
	 * @param name Effect name
	 * @param value Effect value
	 * @param rgb Optional RGB value for tint effects
	 */
	void setEffect(const char* name, int value, const vec3d* rgb);

	/**
	 * @brief Reset all effects to defaults
	 */
	void setDefaults();

	// Bloom control
	void setBloomEnabled(bool enabled) { m_bloomEnabled = enabled; }
	void setBloomIntensity(float intensity) { m_bloomIntensity = intensity; }
	bool isBloomEnabled() const { return m_bloomEnabled; }
	float getBloomIntensity() const { return m_bloomIntensity; }

	// HDR output control
	void setHDROutputEnabled(bool enabled) { m_hdrOutputEnabled = enabled; }
	void setHDRMaxNits(float nits) { m_hdrMaxNits = nits; }
	void setHDRPaperWhiteNits(float nits) { m_hdrPaperWhiteNits = nits; }
	bool isHDROutputEnabled() const { return m_hdrOutputEnabled; }
	float getHDRMaxNits() const { return m_hdrMaxNits; }
	float getHDRPaperWhiteNits() const { return m_hdrPaperWhiteNits; }

	// Tonemapping control
	float getExposure() const { return m_exposure; }
	int getTonemapper() const { return m_tonemapper; }

private:
	// Maximum mip levels for bloom blur
	static constexpr int MAX_BLOOM_MIP_LEVELS = 4;

	// Number of bloom textures for ping-pong
	static constexpr int BLOOM_TEXTURE_COUNT = 2;

	/**
	 * @brief Create post-processing render pass (color only, no depth)
	 */
	vk::UniqueRenderPass createPostProcessRenderPass(vk::Format colorFormat);

	/**
	 * @brief Create bloom resources (textures, framebuffers)
	 */
	bool createBloomResources();

	/**
	 * @brief Create tonemapping pipeline
	 */
	bool createTonemapPipeline();

	/**
	 * @brief Create bloom pipelines (bright pass, blur, composite)
	 */
	bool createBloomPipelines();

	/**
	 * @brief Record bloom pass commands
	 */
	void recordBloomPass(vk::CommandBuffer cmdBuffer, vk::ImageView sceneColorView, vk::Image sceneColorImage);

	/**
	 * @brief Record tonemapping pass commands
	 */
	void recordTonemapPass(vk::CommandBuffer cmdBuffer,
	                       vk::ImageView inputView,
	                       vk::Image inputImage,
	                       vk::ImageView outputView,
	                       vk::Image outputImage);

	/**
	 * @brief Draw a fullscreen triangle (no vertex buffer needed)
	 */
	void drawFullscreenTriangle(vk::CommandBuffer cmdBuffer);

	/**
	 * @brief Transition image layout with barrier
	 */
	void transitionImageLayout(vk::CommandBuffer cmdBuffer,
	                           vk::Image image,
	                           vk::ImageLayout oldLayout,
	                           vk::ImageLayout newLayout,
	                           vk::PipelineStageFlags srcStage,
	                           vk::PipelineStageFlags dstStage,
	                           vk::AccessFlags srcAccess,
	                           vk::AccessFlags dstAccess);

	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	vk::CommandPool m_commandPool;

	uint32_t m_width = 0;
	uint32_t m_height = 0;
	vk::Format m_sceneFormat = vk::Format::eUndefined;

	bool m_initialized = false;
	bool m_inFrame = false;

	// Render passes
	vk::UniqueRenderPass m_postProcessRenderPass;  // For intermediate passes
	vk::UniqueRenderPass m_outputRenderPass;       // For final output

	// Bloom resources (half resolution)
	uint32_t m_bloomWidth = 0;
	uint32_t m_bloomHeight = 0;
	std::array<vk::UniqueImage, BLOOM_TEXTURE_COUNT> m_bloomImages;
	std::array<vk::UniqueDeviceMemory, BLOOM_TEXTURE_COUNT> m_bloomMemory;
	std::array<vk::UniqueImageView, BLOOM_TEXTURE_COUNT> m_bloomViews;
	std::array<std::unique_ptr<VulkanFramebuffer>, BLOOM_TEXTURE_COUNT> m_bloomFramebuffers;

	// Tonemapping resources
	vk::UniquePipelineLayout m_tonemapPipelineLayout;
	vk::UniquePipeline m_tonemapPipeline;
	vk::UniqueDescriptorSetLayout m_tonemapDescriptorSetLayout;

	// Bloom pipelines
	vk::UniquePipelineLayout m_bloomBrightPipelineLayout;
	vk::UniquePipeline m_bloomBrightPipeline;
	vk::UniqueDescriptorSetLayout m_bloomBrightDescriptorSetLayout;

	vk::UniquePipelineLayout m_bloomBlurPipelineLayout;
	vk::UniquePipeline m_bloomBlurHPipeline;  // Horizontal blur
	vk::UniquePipeline m_bloomBlurVPipeline;  // Vertical blur
	vk::UniqueDescriptorSetLayout m_bloomBlurDescriptorSetLayout;

	vk::UniquePipelineLayout m_bloomCompositePipelineLayout;
	vk::UniquePipeline m_bloomCompositePipeline;
	vk::UniqueDescriptorSetLayout m_bloomCompositeDescriptorSetLayout;

	// Sampler for post-processing textures
	vk::UniqueSampler m_linearSampler;

	// Descriptor pool for post-processing
	vk::UniqueDescriptorPool m_descriptorPool;

	// Effect parameters
	bool m_bloomEnabled = true;
	float m_bloomIntensity = 0.25f;  // 25% default

	bool m_hdrOutputEnabled = false;
	float m_hdrMaxNits = 1000.0f;
	float m_hdrPaperWhiteNits = 200.0f;

	// Tonemapping parameters
	float m_exposure = 1.0f;
	int m_tonemapper = 0;  // 0 = Reinhard, 1 = ACES, etc.

	// Helper for memory type selection
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};

// Global post-processing manager (set by VulkanRenderer)
extern VulkanPostProcessing* g_vulkanPostProcessing;

// gr_screen function implementations
void gr_vulkan_post_process_set_effect(const char* name, int value, const vec3d* rgb);
void gr_vulkan_post_process_set_defaults();
void gr_vulkan_post_process_begin();
void gr_vulkan_post_process_end();
void gr_vulkan_post_process_save_zbuffer();
void gr_vulkan_post_process_restore_zbuffer();

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN

