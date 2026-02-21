#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/material.h"

#include "VulkanShader.h"
#include "VulkanVertexFormat.h"
#include "VulkanDescriptorManager.h"

#include <vulkan/vulkan.hpp>

namespace graphics {
namespace vulkan {

/**
 * @brief Pipeline configuration key
 *
 * All state that affects pipeline creation. Two configurations with the
 * same values will produce identical pipelines.
 */
struct PipelineConfig {
	// Shader identification
	shader_type shaderType = SDR_TYPE_NONE;

	// Vertex format
	size_t vertexLayoutHash = 0;

	// Render state
	primitive_type primitiveType = PRIM_TYPE_TRIS;
	gr_zbuffer_type depthMode = ZBUFFER_TYPE_NONE;
	gr_alpha_blend blendMode = ALPHA_BLEND_NONE;
	bool cullEnabled = true;
	bool frontFaceCW = false;  // Match OpenGL default (CCW); models override to CW
	bool depthWriteEnabled = true;

	// Stencil state
	bool stencilEnabled = false;
	ComparisionFunction stencilFunc = ComparisionFunction::Always;
	uint32_t stencilMask = 0xFF;
	material::StencilOp frontStencilOp;
	material::StencilOp backStencilOp;

	// Fill mode (0 = solid, 1 = wireframe)
	int fillMode = 0;

	// Depth bias
	bool depthBiasEnabled = false;

	// Color write mask
	bvec4 colorWriteMask = {true, true, true, true};

	// Render pass compatibility
	vk::RenderPass renderPass;
	uint32_t subpass = 0;

	// Color attachment count (for multiple render targets)
	uint32_t colorAttachmentCount = 1;

	// MSAA sample count (default e1 = no multisampling)
	vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

	// Per-attachment blend (used by decal rendering to write-mask unused G-buffer attachments)
	bool perAttachmentBlendEnabled = false;
	struct AttachmentBlend {
		gr_alpha_blend blendMode = ALPHA_BLEND_NONE;
		bvec4 writeMask = {true, true, true, true};
	};
	static constexpr uint32_t MAX_COLOR_ATTACHMENTS = 8;
	AttachmentBlend attachmentBlends[MAX_COLOR_ATTACHMENTS];

	bool operator==(const PipelineConfig& other) const;
	size_t hash() const;
};

struct PipelineConfigHasher {
	size_t operator()(const PipelineConfig& config) const {
		return config.hash();
	}
};

/**
 * @brief Manages Vulkan graphics pipelines
 *
 * Creates and caches pipelines based on configuration. Uses VkPipelineCache
 * for driver-level caching and an application-level cache for fast lookups.
 */
class VulkanPipelineManager {
public:
	VulkanPipelineManager() = default;
	~VulkanPipelineManager() = default;

	// Non-copyable
	VulkanPipelineManager(const VulkanPipelineManager&) = delete;
	VulkanPipelineManager& operator=(const VulkanPipelineManager&) = delete;

	/**
	 * @brief Initialize the pipeline manager
	 * @param device Vulkan logical device
	 * @param shaderManager Shader manager for loading shader modules
	 * @param descriptorManager Descriptor manager for set layouts
	 * @return true on success
	 */
	bool init(vk::Device device, VulkanShaderManager* shaderManager,
	          VulkanDescriptorManager* descriptorManager);

	/**
	 * @brief Shutdown and release resources
	 */
	void shutdown();

	/**
	 * @brief Get or create a pipeline for the given configuration
	 * @param config Pipeline configuration
	 * @param vertexLayout Vertex layout for the pipeline
	 * @return Pipeline handle, or null handle on failure
	 */
	vk::Pipeline getPipeline(const PipelineConfig& config, const vertex_layout& vertexLayout);

	/**
	 * @brief Get the common pipeline layout
	 *
	 * All pipelines share the same pipeline layout (descriptor set layouts
	 * and push constant ranges).
	 */
	vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout.get(); }

	/**
	 * @brief Load pipeline cache from file
	 * @param filename Cache file path
	 * @return true if cache was loaded
	 */
	bool loadPipelineCache(const SCP_string& filename);

	/**
	 * @brief Save pipeline cache to file
	 * @param filename Cache file path
	 * @return true if cache was saved
	 */
	bool savePipelineCache(const SCP_string& filename);

	/**
	 * @brief Get number of cached pipelines
	 */
	size_t getPipelineCount() const { return m_pipelines.size(); }

	/**
	 * @brief Check if a draw needs a fallback buffer for a given vertex attribute
	 * @param vertexLayout The vertex layout to check
	 * @param shaderType The shader being used (checked against vertexInputMask)
	 * @param location The vertex attribute location to check
	 * @return true if the layout doesn't provide this attribute AND the shader consumes it
	 */
	bool needsFallbackAttribute(const vertex_layout& vertexLayout, shader_type shaderType,
	                             VertexAttributeLocation location);

private:
	/**
	 * @brief Create the common pipeline layout
	 */
	void createPipelineLayout();

	/**
	 * @brief Create a new pipeline
	 */
	vk::UniquePipeline createPipeline(const PipelineConfig& config, const vertex_layout& vertexLayout);

	vk::Device m_device;
	VulkanShaderManager* m_shaderManager = nullptr;
	VulkanDescriptorManager* m_descriptorManager = nullptr;

	// Common pipeline layout (shared by all pipelines)
	vk::UniquePipelineLayout m_pipelineLayout;

	// Driver-level pipeline cache
	vk::UniquePipelineCache m_pipelineCache;

	// Application-level pipeline cache: config -> pipeline
	SCP_unordered_map<PipelineConfig, vk::UniquePipeline, PipelineConfigHasher> m_pipelines;

	// Vertex format cache
	VulkanVertexFormatCache m_vertexFormatCache;

	bool m_initialized = false;
};

// Global pipeline manager access
VulkanPipelineManager* getPipelineManager();
void setPipelineManager(VulkanPipelineManager* manager);

} // namespace vulkan
} // namespace graphics
