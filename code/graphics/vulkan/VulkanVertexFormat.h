#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/shader_types.h"

#include <vulkan/vulkan.hpp>


namespace graphics::vulkan {

/**
 * @brief Mapping from FSO vertex_format to Vulkan format and location
 */
struct VertexFormatMapping {
	vertex_format_data::vertex_format format;
	vk::Format vkFormat;
	VertexAttributeLocation location;
	uint32_t componentCount;
	uint32_t sizeInBytes;
};

/**
 * @brief Get the Vulkan format mapping for a given vertex format
 * @param format The FSO vertex format type
 * @return Pointer to mapping info, or nullptr if not found
 */
const VertexFormatMapping* getVertexFormatMapping(vertex_format_data::vertex_format format);

// Reserved binding indices for fallback buffers when vertex data is missing attributes
static constexpr uint32_t FALLBACK_COLOR_BINDING = 15;
static constexpr uint32_t FALLBACK_TEXCOORD_BINDING = 14;

/**
 * @brief Cached vertex input configuration
 */
struct VertexInputConfig {
	SCP_vector<vk::VertexInputBindingDescription> bindings;
	SCP_vector<vk::VertexInputAttributeDescription> attributes;
	vk::PipelineVertexInputStateCreateInfo createInfo;

	// Bitmask of vertex input locations natively provided by the layout (bit N = location N).
	// Does NOT include fallback attributes. Compare with shader's vertexInputMask to
	// determine which fallbacks are actually needed: shaderMask & ~providedInputMask.
	uint32_t providedInputMask = 0;

	// Update createInfo pointers after vector modifications
	void updatePointers();
};

/**
 * @brief Manages vertex format to Vulkan vertex input state conversion
 *
 * Converts FSO vertex_layout objects to Vulkan VkPipelineVertexInputStateCreateInfo.
 * Caches configurations to avoid repeated conversions.
 */
class VulkanVertexFormatCache {
public:
	VulkanVertexFormatCache() = default;
	~VulkanVertexFormatCache() = default;

	// Non-copyable
	VulkanVertexFormatCache(const VulkanVertexFormatCache&) = delete;
	VulkanVertexFormatCache& operator=(const VulkanVertexFormatCache&) = delete;

	/**
	 * @brief Get Vulkan vertex input state for a given layout
	 * @param layout The FSO vertex layout
	 * @return Reference to cached vertex input configuration
	 */
	const VertexInputConfig& getVertexInputConfig(const vertex_layout& layout);

	/**
	 * @brief Clear all cached configurations
	 */
	void clear();

	/**
	 * @brief Get number of cached configurations
	 */
	size_t getCacheSize() const { return m_cache.size(); }

private:
	/**
	 * @brief Create a new vertex input configuration for a layout
	 */
	static VertexInputConfig createVertexInputConfig(const vertex_layout& layout);

	// Cache: layout hash -> vertex input config
	SCP_unordered_map<size_t, VertexInputConfig> m_cache;
};

// Global vertex format mapping table
extern const VertexFormatMapping VERTEX_FORMAT_MAPPINGS[];
extern const size_t VERTEX_FORMAT_MAPPINGS_COUNT;

} // namespace graphics::vulkan

