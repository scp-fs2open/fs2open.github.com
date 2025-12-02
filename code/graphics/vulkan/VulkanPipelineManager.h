#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/material.h"

#ifdef WITH_VULKAN

#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include <map>

namespace graphics {
namespace vulkan {

// Forward declarations
class VulkanShaderManager;

/**
 * @brief Key for pipeline cache lookup - all state that requires pipeline recreation
 *
 * Note: Viewport and scissor are dynamic state, so they are NOT included.
 * This key captures all immutable pipeline state.
 */
struct PipelineKey {
	// Shader identification
	shader_type shaderType = SDR_TYPE_NONE;
	uint32_t shaderFlags = 0;

	// Vertex input state (hashed from vertex_layout)
	size_t vertexLayoutHash = 0;

	// Input assembly
	primitive_type primitiveType = PRIM_TYPE_TRIS;

	// Rasterization state
	bool cullEnabled = true;
	vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

	// Depth/stencil state
	gr_zbuffer_type depthMode = ZBUFFER_TYPE_FULL;

	// Stencil state (only considered when stencilEnabled)
	bool stencilEnabled = false;
	ComparisionFunction stencilFunc = ComparisionFunction::Always;
	StencilOperation stencilFailOp = StencilOperation::Keep;
	StencilOperation stencilDepthFailOp = StencilOperation::Keep;
	StencilOperation stencilPassOp = StencilOperation::Keep;

	// Color blend state
	gr_alpha_blend blendMode = ALPHA_BLEND_NONE;
	bool hasPerBufferBlend = false;
	std::array<gr_alpha_blend, material::NUM_BUFFER_BLENDS> bufferBlendModes = {};

	// Color write mask
	bvec4 colorMask = {true, true, true, true};

	// Dynamic rendering formats (Vulkan 1.3+)
	// These replace renderPass for pipeline compatibility
	vk::Format colorFormat = vk::Format::eR8G8B8A8Srgb;
	vk::Format depthFormat = vk::Format::eUndefined;  // eUndefined = no depth

	// Multisampling
	vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

	/**
	 * @brief Equality comparison for cache lookup
	 */
	bool operator==(const PipelineKey& other) const;

	/**
	 * @brief Inequality comparison for testing
	 */
	bool operator!=(const PipelineKey& other) const { return !(*this == other); }

	/**
	 * @brief Compute hash for unordered_map
	 */
	size_t hash() const;
};

} // namespace vulkan
} // namespace graphics

// Custom hash function for PipelineKey
namespace std {
template<>
struct hash<graphics::vulkan::PipelineKey> {
	size_t operator()(const graphics::vulkan::PipelineKey& key) const {
		return key.hash();
	}
};
} // namespace std

namespace graphics {
namespace vulkan {

/**
 * @brief Manages Vulkan graphics pipeline creation, caching, and lifecycle
 *
 * Design principles:
 * - Hash-based pipeline cache to avoid redundant creation
 * - Integration with VulkanShaderManager for shader modules
 * - Support for all FSO vertex formats via vertex_layout
 * - Dynamic state for viewport/scissor to avoid resize recreations
 * - VkPipelineCache for driver-level binary caching with disk persistence
 */
class VulkanPipelineManager {
public:
	VulkanPipelineManager() = default;
	~VulkanPipelineManager() = default;

	// Non-copyable, non-movable
	VulkanPipelineManager(const VulkanPipelineManager&) = delete;
	VulkanPipelineManager& operator=(const VulkanPipelineManager&) = delete;

	/**
	 * @brief Initialize the pipeline manager
	 * @param device Vulkan logical device
	 * @param physicalDevice Vulkan physical device (for querying limits)
	 * @param shaderManager Reference to shader manager for shader modules
	 * @return true on success
	 */
	bool initialize(vk::Device device, vk::PhysicalDevice physicalDevice,
	                VulkanShaderManager* shaderManager);

	/**
	 * @brief Shutdown and cleanup all pipelines
	 */
	void shutdown();

	// =========================================================================
	// Pipeline Retrieval
	// =========================================================================

	/**
	 * @brief Get or create a pipeline for the given state
	 * @param key Pipeline state key
	 * @return Pipeline handle (may be null if creation failed)
	 */
	vk::Pipeline getOrCreatePipeline(const PipelineKey& key);

	/**
	 * @brief Get or create a pipeline from material and vertex layout
	 *
	 * This is the primary interface for the rendering code - extracts
	 * pipeline state from the material class.
	 *
	 * @param mat Material containing render state
	 * @param layout Vertex layout for this draw call
	 * @param primType Primitive topology
	 * @param colorFormat Color attachment format for dynamic rendering
	 * @param depthFormat Depth attachment format (eUndefined if no depth)
	 * @return Pipeline handle
	 */
	vk::Pipeline getOrCreatePipeline(const material& mat,
	                                  const vertex_layout& layout,
	                                  primitive_type primType,
	                                  vk::Format colorFormat,
	                                  vk::Format depthFormat = vk::Format::eUndefined);

	/**
	 * @brief Get pipeline layout for a shader type
	 * @param type Shader type
	 * @param flags Shader variant flags
	 * @return Pipeline layout (creates if needed)
	 */
	vk::PipelineLayout getPipelineLayout(shader_type type, uint32_t flags);

	/**
	 * @brief Descriptor set layout used by material shaders
	 */
	vk::DescriptorSetLayout getMaterialDescriptorSetLayout() const;

	/**
	 * @brief Get descriptor set layout for uniform buffers
	 */
	vk::DescriptorSetLayout getUniformDescriptorSetLayout() const;

	// =========================================================================
	// Cache Management
	// =========================================================================

	/**
	 * @brief Clear all cached pipelines (e.g., on render pass recreation)
	 */
	void clearCache();

	/**
	 * @brief Invalidate pipelines for a specific render pass
	 * @param renderPass Render pass being destroyed/recreated
	 */
	void invalidatePipelinesForRenderPass(vk::RenderPass renderPass);

	/**
	 * @brief Get cache statistics
	 * @param[out] numPipelines Number of cached pipelines
	 * @param[out] numLayouts Number of cached pipeline layouts
	 */
	void getCacheStats(size_t& numPipelines, size_t& numLayouts) const;

	// =========================================================================
	// Disk Persistence
	// =========================================================================

	/**
	 * @brief Load VkPipelineCache data from disk
	 * @param path File path for cache data
	 * @return true if loaded successfully
	 */
	bool loadPipelineCacheFromDisk(const SCP_string& path);

	/**
	 * @brief Save VkPipelineCache data to disk
	 * @param path File path for cache data
	 * @return true if saved successfully
	 */
	bool savePipelineCacheToDisk(const SCP_string& path);

	// =========================================================================
	// MSAA Configuration
	// =========================================================================

	/**
	 * @brief Set current MSAA sample count for new pipelines
	 * @param samples Sample count (e1, e4, e8, e16)
	 */
	void setMsaaSamples(vk::SampleCountFlagBits samples);

	/**
	 * @brief Get current MSAA sample count
	 */
	vk::SampleCountFlagBits getMsaaSamples() const;

private:
	vk::Device m_device;
	vk::PhysicalDevice m_physicalDevice;
	VulkanShaderManager* m_shaderManager = nullptr;

	// Current MSAA setting
	vk::SampleCountFlagBits m_currentMsaaSamples = vk::SampleCountFlagBits::e1;

	// Pipeline cache - key is PipelineKey
	std::unordered_map<PipelineKey, vk::UniquePipeline> m_pipelineCache;

	// Vertex layout cache - key is layout hash, value is the layout
	// Needed because PipelineKey only stores the hash
	std::unordered_map<size_t, vertex_layout> m_vertexLayoutCache;

	// Pipeline layout cache - key is "shaderType_flags"
	std::map<SCP_string, vk::UniquePipelineLayout> m_layoutCache;

	// Descriptor set layout for material textures
	vk::UniqueDescriptorSetLayout m_materialDescriptorSetLayout;

	// Descriptor set layout for uniform buffers (Set 0)
	vk::UniqueDescriptorSetLayout m_uniformDescriptorSetLayout;

	// Vulkan pipeline cache object (for driver-level caching)
	vk::UniquePipelineCache m_vkPipelineCache;

	bool m_initialized = false;

	// =========================================================================
	// Pipeline Creation Helpers
	// =========================================================================

	/**
	 * @brief Create a new pipeline for the given key
	 * @param key Pipeline state
	 * @return Created pipeline (may be null on failure)
	 */
	vk::UniquePipeline createPipeline(const PipelineKey& key);

	/**
	 * @brief Build pipeline key from material
	 */
	PipelineKey buildKeyFromMaterial(const material& mat,
	                                  const vertex_layout& layout,
	                                  primitive_type primType,
	                                  vk::Format colorFormat,
	                                  vk::Format depthFormat);

	/**
	 * @brief Create vertex input state from vertex_layout
	 */
	void createVertexInputState(const vertex_layout& layout,
	                             SCP_vector<vk::VertexInputBindingDescription>& bindings,
	                             SCP_vector<vk::VertexInputAttributeDescription>& attributes);

	/**
	 * @brief Convert FSO primitive type to Vulkan topology
	 */
	vk::PrimitiveTopology convertPrimitiveType(primitive_type primType);

	/**
	 * @brief Create rasterization state from key
	 */
	vk::PipelineRasterizationStateCreateInfo createRasterizationState(const PipelineKey& key);

	/**
	 * @brief Create depth/stencil state from key
	 */
	vk::PipelineDepthStencilStateCreateInfo createDepthStencilState(const PipelineKey& key);

	/**
	 * @brief Create color blend state from key
	 * @param[in] key Pipeline key with blend settings
	 * @param[out] attachments Color blend attachment states (must remain valid)
	 * @param[out] blendInfo Color blend state create info
	 */
	void createColorBlendState(const PipelineKey& key,
                                    SCP_vector<vk::PipelineColorBlendAttachmentState>& attachments,
                                    vk::PipelineColorBlendStateCreateInfo& blendInfo);

	/**
	 * @brief Create the per-material descriptor set layout
	 */
	bool createMaterialDescriptorLayout();

	/**
	 * @brief Create the uniform buffer descriptor set layout
	 */
	bool createUniformDescriptorLayout();

	/**
	 * @brief Create or get pipeline layout for shader type
	 */
	vk::PipelineLayout getOrCreateLayout(shader_type type, uint32_t flags);

	/**
	 * @brief Generate layout cache key
	 */
	SCP_string makeLayoutKey(shader_type type, uint32_t flags);

	// =========================================================================
	// State Conversion Helpers
	// =========================================================================

	/**
	 * @brief Convert FSO blend mode to Vulkan blend factors
	 */
	void convertBlendMode(gr_alpha_blend mode,
	                       vk::BlendFactor& srcColor, vk::BlendFactor& dstColor,
	                       vk::BlendFactor& srcAlpha, vk::BlendFactor& dstAlpha);

	/**
	 * @brief Convert FSO depth mode to Vulkan depth state
	 */
	void convertDepthMode(gr_zbuffer_type mode,
	                       vk::CompareOp& compareOp, bool& depthWrite, bool& depthTest);

	/**
	 * @brief Convert FSO comparison function to Vulkan
	 */
	vk::CompareOp convertCompareFunc(ComparisionFunction func);

	/**
	 * @brief Convert FSO stencil operation to Vulkan
	 */
	vk::StencilOp convertStencilOp(StencilOperation op);

	/**
	 * @brief Convert FSO vertex format to Vulkan format
	 */
	vk::Format convertVertexFormat(vertex_format_data::vertex_format format);

public:
	// =========================================================================
	// Static Conversion Functions (for unit testing)
	// =========================================================================

	/**
	 * @brief Convert FSO vertex format to Vulkan format (static version)
	 */
	static vk::Format convertVertexFormatStatic(vertex_format_data::vertex_format format);

	/**
	 * @brief Convert FSO primitive type to Vulkan topology (static version)
	 */
	static vk::PrimitiveTopology convertPrimitiveTypeStatic(primitive_type primType);
};

// Global instance (set by VulkanRenderer)
extern VulkanPipelineManager* g_vulkanPipelineManager;

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
