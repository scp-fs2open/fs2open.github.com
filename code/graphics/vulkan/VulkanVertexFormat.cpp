#include "VulkanVertexFormat.h"

namespace graphics {
namespace vulkan {

// Vertex format mapping table
// Maps FSO vertex_format_data::vertex_format to Vulkan formats
// Based on GL_array_binding_data in gropengltnl.cpp
const VertexFormatMapping VERTEX_FORMAT_MAPPINGS[] = {
	// Position formats
	{ vertex_format_data::POSITION4,  vk::Format::eR32G32B32A32Sfloat, VertexAttributeLocation::Position, 4, 16 },
	{ vertex_format_data::POSITION3,  vk::Format::eR32G32B32Sfloat,    VertexAttributeLocation::Position, 3, 12 },
	{ vertex_format_data::POSITION2,  vk::Format::eR32G32Sfloat,       VertexAttributeLocation::Position, 2, 8 },

	// Color formats
	{ vertex_format_data::COLOR3,     vk::Format::eR8G8B8Unorm,        VertexAttributeLocation::Color, 3, 3 },
	{ vertex_format_data::COLOR4,     vk::Format::eR8G8B8A8Unorm,      VertexAttributeLocation::Color, 4, 4 },
	{ vertex_format_data::COLOR4F,    vk::Format::eR32G32B32A32Sfloat, VertexAttributeLocation::Color, 4, 16 },

	// Texture coordinate formats
	{ vertex_format_data::TEX_COORD2, vk::Format::eR32G32Sfloat,       VertexAttributeLocation::TexCoord, 2, 8 },
	{ vertex_format_data::TEX_COORD4, vk::Format::eR32G32B32A32Sfloat, VertexAttributeLocation::TexCoord, 4, 16 },

	// Normal/tangent formats
	{ vertex_format_data::NORMAL,     vk::Format::eR32G32B32Sfloat,    VertexAttributeLocation::Normal, 3, 12 },
	{ vertex_format_data::TANGENT,    vk::Format::eR32G32B32A32Sfloat, VertexAttributeLocation::Tangent, 4, 16 },

	// Instance/particle formats
	{ vertex_format_data::MODEL_ID,   vk::Format::eR32Sfloat,          VertexAttributeLocation::ModelId, 1, 4 },
	{ vertex_format_data::RADIUS,     vk::Format::eR32Sfloat,          VertexAttributeLocation::Radius, 1, 4 },
	{ vertex_format_data::UVEC,       vk::Format::eR32G32B32Sfloat,    VertexAttributeLocation::Uvec, 3, 12 },

	// Matrix format (mat4 = 4 vec4s, uses locations 8-11)
	{ vertex_format_data::MATRIX4,    vk::Format::eR32G32B32A32Sfloat, VertexAttributeLocation::ModelMatrix, 16, 64 },
};

const size_t VERTEX_FORMAT_MAPPINGS_COUNT = sizeof(VERTEX_FORMAT_MAPPINGS) / sizeof(VERTEX_FORMAT_MAPPINGS[0]);

const VertexFormatMapping* getVertexFormatMapping(vertex_format_data::vertex_format format)
{
	for (size_t i = 0; i < VERTEX_FORMAT_MAPPINGS_COUNT; ++i) {
		if (VERTEX_FORMAT_MAPPINGS[i].format == format) {
			return &VERTEX_FORMAT_MAPPINGS[i];
		}
	}
	return nullptr;
}

void VertexInputConfig::updatePointers()
{
	createInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
	createInfo.pVertexBindingDescriptions = bindings.empty() ? nullptr : bindings.data();
	createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	createInfo.pVertexAttributeDescriptions = attributes.empty() ? nullptr : attributes.data();
}

const VertexInputConfig& VulkanVertexFormatCache::getVertexInputConfig(const vertex_layout& layout)
{
	size_t hash = layout.hash();

	auto it = m_cache.find(hash);
	if (it != m_cache.end()) {
		return it->second;
	}

	// Create new configuration
	auto result = m_cache.emplace(hash, createVertexInputConfig(layout));
	return result.first->second;
}

void VulkanVertexFormatCache::clear()
{
	m_cache.clear();
}

VertexInputConfig VulkanVertexFormatCache::createVertexInputConfig(const vertex_layout& layout)
{
	VertexInputConfig config;

	// Track which bindings we've already added
	SCP_unordered_map<size_t, uint32_t> bufferBindings; // buffer_number -> binding index

	size_t numComponents = layout.get_num_vertex_components();

	for (size_t i = 0; i < numComponents; ++i) {
		const vertex_format_data* component = layout.get_vertex_component(i);
		const VertexFormatMapping* mapping = getVertexFormatMapping(component->format_type);

		if (!mapping) {
			mprintf(("VulkanVertexFormat: Unknown vertex format %d\n", static_cast<int>(component->format_type)));
			continue;
		}

		// Track which locations the layout natively provides
		uint32_t loc = static_cast<uint32_t>(mapping->location);
		config.providedInputMask |= (1u << loc);

		// Get or create binding for this buffer
		uint32_t bindingIndex;
		auto bindingIt = bufferBindings.find(component->buffer_number);
		if (bindingIt == bufferBindings.end()) {
			bindingIndex = static_cast<uint32_t>(config.bindings.size());
			bufferBindings[component->buffer_number] = bindingIndex;

			vk::VertexInputBindingDescription binding;
			binding.binding = bindingIndex;
			binding.stride = static_cast<uint32_t>(component->stride);
			binding.inputRate = (component->divisor > 0) ?
				vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex;
			config.bindings.push_back(binding);
		} else {
			bindingIndex = bindingIt->second;
		}

		// Handle MATRIX4 specially - it needs 4 attribute locations
		if (component->format_type == vertex_format_data::MATRIX4) {
			// mat4 requires 4 vec4 attributes at consecutive locations
			for (uint32_t row = 0; row < 4; ++row) {
				vk::VertexInputAttributeDescription attr;
				attr.location = static_cast<uint32_t>(mapping->location) + row;
				attr.binding = bindingIndex;
				attr.format = vk::Format::eR32G32B32A32Sfloat;
				attr.offset = static_cast<uint32_t>(component->offset) + (row * 16);
				config.attributes.push_back(attr);
			}
			// Mark all 4 matrix locations as provided
			config.providedInputMask |= (1u << (loc + 1)) | (1u << (loc + 2)) | (1u << (loc + 3));
		} else {
			vk::VertexInputAttributeDescription attr;
			attr.location = static_cast<uint32_t>(mapping->location);
			attr.binding = bindingIndex;
			attr.format = mapping->vkFormat;
			attr.offset = static_cast<uint32_t>(component->offset);
			config.attributes.push_back(attr);
		}
	}

	// Only add fallback bindings when the layout has actual vertex components.
	// Empty layouts (e.g. fullscreen triangles) generate vertices in the shader
	// and don't need any vertex input bindings.
	uint32_t colorBit = 1u << static_cast<uint32_t>(VertexAttributeLocation::Color);
	if (!(config.providedInputMask & colorBit) && numComponents > 0) {
		// Add binding for fallback color buffer (instanced so one value applies to all vertices)
		vk::VertexInputBindingDescription colorBinding;
		colorBinding.binding = FALLBACK_COLOR_BINDING;
		colorBinding.stride = 16;  // vec4 = 16 bytes
		colorBinding.inputRate = vk::VertexInputRate::eInstance;  // Same color for all vertices
		config.bindings.push_back(colorBinding);

		vk::VertexInputAttributeDescription colorAttr;
		colorAttr.location = static_cast<uint32_t>(VertexAttributeLocation::Color);
		colorAttr.binding = FALLBACK_COLOR_BINDING;
		colorAttr.format = vk::Format::eR32G32B32A32Sfloat;
		colorAttr.offset = 0;
		config.attributes.push_back(colorAttr);
	}

	// If no texcoord attribute, add a fallback providing (0,0,0,0)
	// In OpenGL, missing vertex attributes default to (0,0,0,1); Vulkan requires explicit input
	uint32_t texCoordBit = 1u << static_cast<uint32_t>(VertexAttributeLocation::TexCoord);
	if (!(config.providedInputMask & texCoordBit) && numComponents > 0) {
		// Add binding for fallback texcoord buffer (instanced so one value applies to all vertices)
		vk::VertexInputBindingDescription texCoordBinding;
		texCoordBinding.binding = FALLBACK_TEXCOORD_BINDING;
		texCoordBinding.stride = 16;  // vec4 = 16 bytes
		texCoordBinding.inputRate = vk::VertexInputRate::eInstance;
		config.bindings.push_back(texCoordBinding);

		vk::VertexInputAttributeDescription texCoordAttr;
		texCoordAttr.location = static_cast<uint32_t>(VertexAttributeLocation::TexCoord);
		texCoordAttr.binding = FALLBACK_TEXCOORD_BINDING;
		texCoordAttr.format = vk::Format::eR32G32B32A32Sfloat;
		texCoordAttr.offset = 0;
		config.attributes.push_back(texCoordAttr);
	}

	// Update the createInfo pointers
	config.updatePointers();

	return config;
}

} // namespace vulkan
} // namespace graphics
