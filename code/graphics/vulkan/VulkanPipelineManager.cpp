#include "VulkanPipelineManager.h"

#ifdef WITH_VULKAN

#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "cfile/cfile.h"
#include "osapi/osapi.h"

#include <fstream>
#include <set>

namespace graphics {
namespace vulkan {

// Global instance
VulkanPipelineManager* g_vulkanPipelineManager = nullptr;

// ============================================================================
// PipelineKey Implementation
// ============================================================================

bool PipelineKey::operator==(const PipelineKey& other) const
{
	// Compare all fields that affect pipeline state
	if (shaderType != other.shaderType) return false;
	if (shaderFlags != other.shaderFlags) return false;
	if (vertexLayoutHash != other.vertexLayoutHash) return false;
	if (primitiveType != other.primitiveType) return false;
	if (cullEnabled != other.cullEnabled) return false;
	if (cullMode != other.cullMode) return false;
	if (polygonMode != other.polygonMode) return false;
	if (depthMode != other.depthMode) return false;
	if (stencilEnabled != other.stencilEnabled) return false;

	// Only compare stencil state if stencil is enabled
	if (stencilEnabled) {
		if (stencilFunc != other.stencilFunc) return false;
		if (stencilFailOp != other.stencilFailOp) return false;
		if (stencilDepthFailOp != other.stencilDepthFailOp) return false;
		if (stencilPassOp != other.stencilPassOp) return false;
	}

	if (blendMode != other.blendMode) return false;
	if (hasPerBufferBlend != other.hasPerBufferBlend) return false;

	// Only compare per-buffer blends if enabled
	if (hasPerBufferBlend) {
		if (bufferBlendModes != other.bufferBlendModes) return false;
	}

	if (colorMask.x != other.colorMask.x || colorMask.y != other.colorMask.y ||
	    colorMask.z != other.colorMask.z || colorMask.w != other.colorMask.w) return false;

	// Dynamic rendering formats (Vulkan 1.3+)
	if (colorFormat != other.colorFormat) return false;
	if (depthFormat != other.depthFormat) return false;

	if (sampleCount != other.sampleCount) return false;

	return true;
}

size_t PipelineKey::hash() const
{
	// FNV-1a hash
	size_t h = 14695981039346656037ULL;

	auto hashCombine = [&h](size_t value) {
		h ^= value;
		h *= 1099511628211ULL;
	};

	hashCombine(static_cast<size_t>(shaderType));
	hashCombine(static_cast<size_t>(shaderFlags));
	hashCombine(vertexLayoutHash);
	hashCombine(static_cast<size_t>(primitiveType));
	hashCombine(static_cast<size_t>(cullEnabled));
	hashCombine(static_cast<size_t>(static_cast<VkCullModeFlags>(cullMode)));
	hashCombine(static_cast<size_t>(polygonMode));
	hashCombine(static_cast<size_t>(depthMode));
	hashCombine(static_cast<size_t>(stencilEnabled));

	if (stencilEnabled) {
		hashCombine(static_cast<size_t>(stencilFunc));
		hashCombine(static_cast<size_t>(stencilFailOp));
		hashCombine(static_cast<size_t>(stencilDepthFailOp));
		hashCombine(static_cast<size_t>(stencilPassOp));
	}

	hashCombine(static_cast<size_t>(blendMode));
	hashCombine(static_cast<size_t>(hasPerBufferBlend));

	if (hasPerBufferBlend) {
		for (size_t i = 0; i < material::NUM_BUFFER_BLENDS; ++i) {
			hashCombine(static_cast<size_t>(bufferBlendModes[i]));
		}
	}

	hashCombine(static_cast<size_t>(colorMask.x));
	hashCombine(static_cast<size_t>(colorMask.y));
	hashCombine(static_cast<size_t>(colorMask.z));
	hashCombine(static_cast<size_t>(colorMask.w));

	// Dynamic rendering formats (Vulkan 1.3+)
	hashCombine(static_cast<size_t>(colorFormat));
	hashCombine(static_cast<size_t>(depthFormat));

	hashCombine(static_cast<size_t>(sampleCount));

	return h;
}

// ============================================================================
// VulkanPipelineManager Implementation
// ============================================================================

bool VulkanPipelineManager::initialize(vk::Device device, vk::PhysicalDevice physicalDevice,
                                        VulkanShaderManager* shaderManager)
{
	if (m_initialized) {
		mprintf(("VulkanPipelineManager: Already initialized\n"));
		return true;
	}

	m_device = device;
	m_physicalDevice = physicalDevice;
	m_shaderManager = shaderManager;

	if (!m_device) {
		mprintf(("VulkanPipelineManager: Invalid device\n"));
		return false;
	}

	if (!m_shaderManager) {
		mprintf(("VulkanPipelineManager: Invalid shader manager\n"));
		return false;
	}

	// Create VkPipelineCache for driver-level binary caching
	vk::PipelineCacheCreateInfo cacheInfo;
	// initialData will be set if we load from disk
	m_vkPipelineCache = m_device.createPipelineCacheUnique(cacheInfo);

	if (!m_vkPipelineCache) {
		mprintf(("VulkanPipelineManager: Failed to create pipeline cache\n"));
		return false;
	}

	if (!createMaterialDescriptorLayout()) {
		return false;
	}

	if (!createUniformDescriptorLayout()) {
		return false;
	}

	m_initialized = true;
	mprintf(("VulkanPipelineManager: Initialized successfully\n"));
	return true;
}

void VulkanPipelineManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	mprintf(("VulkanPipelineManager: Shutting down with %zu cached pipelines, %zu layouts, %zu vertex layouts\n",
	         m_pipelineCache.size(), m_layoutCache.size(), m_vertexLayoutCache.size()));

	// Clear all caches - unique handles will destroy automatically
	m_pipelineCache.clear();
	m_vertexLayoutCache.clear();
	m_layoutCache.clear();
	m_vkPipelineCache.reset();

	m_device = nullptr;
	m_physicalDevice = nullptr;
	m_shaderManager = nullptr;
	m_initialized = false;
}

vk::Pipeline VulkanPipelineManager::getOrCreatePipeline(const PipelineKey& key)
{
	if (!m_initialized) {
		return nullptr;
	}

	// Check cache first
	auto it = m_pipelineCache.find(key);
	if (it != m_pipelineCache.end()) {
		return it->second.get();
	}

	// Create new pipeline
	vk::UniquePipeline pipeline = createPipeline(key);
	if (!pipeline) {
		mprintf(("VulkanPipelineManager: Failed to create pipeline for shader type %d\n",
		         static_cast<int>(key.shaderType)));
		return nullptr;
	}

	vk::Pipeline result = pipeline.get();
	m_pipelineCache[key] = std::move(pipeline);
	return result;
}

vk::Pipeline VulkanPipelineManager::getOrCreatePipeline(const material& mat,
                                                          const vertex_layout& layout,
                                                          primitive_type primType,
                                                          vk::Format colorFormat,
                                                          vk::Format depthFormat)
{
	PipelineKey key = buildKeyFromMaterial(mat, layout, primType, colorFormat, depthFormat);
	return getOrCreatePipeline(key);
}

vk::PipelineLayout VulkanPipelineManager::getPipelineLayout(shader_type type, uint32_t flags)
{
	return getOrCreateLayout(type, flags);
}

vk::DescriptorSetLayout VulkanPipelineManager::getMaterialDescriptorSetLayout() const
{
	return m_materialDescriptorSetLayout ? m_materialDescriptorSetLayout.get() : vk::DescriptorSetLayout();
}

bool VulkanPipelineManager::createUniformDescriptorLayout()
{
	if (!m_device) {
		return false;
	}

	// Create bindings for all uniform block types
	// IMPORTANT: Using eUniformBufferDynamic because FSO uses ring-buffer suballocation.
	// UniformBufferManager creates a single large buffer (triple-buffered) that all uniform
	// block types share. Each block gets a range within this buffer via offset.
	// This matches OpenGL's glBindBufferRange() pattern (see gropengltnl.cpp:390).
	// Therefore we use dynamic offsets passed via pDynamicOffsets when binding descriptor sets.
	SCP_vector<vk::DescriptorSetLayoutBinding> bindings;
	bindings.reserve(static_cast<size_t>(uniform_block_type::NUM_BLOCK_TYPES));

	for (int i = 0; i < static_cast<int>(uniform_block_type::NUM_BLOCK_TYPES); ++i) {
		vk::DescriptorSetLayoutBinding binding;
		binding.binding = static_cast<uint32_t>(i);
		binding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		binding.descriptorCount = 1;
		binding.stageFlags = vk::ShaderStageFlagBits::eVertex | 
		                     vk::ShaderStageFlagBits::eFragment | 
		                     vk::ShaderStageFlagBits::eGeometry;
		binding.pImmutableSamplers = nullptr;
		bindings.push_back(binding);
	}

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	try {
		m_uniformDescriptorSetLayout = m_device.createDescriptorSetLayoutUnique(layoutInfo);
		mprintf(("VulkanPipelineManager: Created uniform buffer descriptor set layout with %zu bindings (dynamic offsets)\n", 
		         bindings.size()));
		return true;
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPipelineManager: Failed to create uniform descriptor set layout: %s\n", e.what()));
		return false;
	}
}

vk::DescriptorSetLayout VulkanPipelineManager::getUniformDescriptorSetLayout() const
{
	return m_uniformDescriptorSetLayout ? m_uniformDescriptorSetLayout.get() : vk::DescriptorSetLayout();
}

void VulkanPipelineManager::clearCache()
{
	mprintf(("VulkanPipelineManager: Clearing %zu cached pipelines, %zu vertex layouts\n",
	         m_pipelineCache.size(), m_vertexLayoutCache.size()));
	m_pipelineCache.clear();
	m_vertexLayoutCache.clear();
}

void VulkanPipelineManager::invalidatePipelinesForRenderPass(vk::RenderPass /*renderPass*/)
{
	// With dynamic rendering (Vulkan 1.3+), render passes no longer exist.
	// Pipelines are keyed by attachment formats, not render passes.
	// This function is retained for API compatibility but is now a no-op.
	// If format-based invalidation is needed, call clearCache() instead.
}

void VulkanPipelineManager::getCacheStats(size_t& numPipelines, size_t& numLayouts) const
{
	numPipelines = m_pipelineCache.size();
	numLayouts = m_layoutCache.size();
}

bool VulkanPipelineManager::loadPipelineCacheFromDisk(const SCP_string& path)
{
	if (!m_initialized) {
		return false;
	}

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		mprintf(("VulkanPipelineManager: No pipeline cache file found at %s\n", path.c_str()));
		return false;
	}

	std::streamsize size = file.tellg();
	if (size <= 0) {
		return false;
	}

	file.seekg(0, std::ios::beg);

	std::vector<char> data(static_cast<size_t>(size));
	if (!file.read(data.data(), size)) {
		mprintf(("VulkanPipelineManager: Failed to read pipeline cache file\n"));
		return false;
	}

	// Validate cache header
	// VkPipelineCache data includes a header with:
	// - Header size (4 bytes)
	// - Header version (4 bytes)
	// - Vendor ID (4 bytes)
	// - Device ID (4 bytes)
	// - Pipeline cache UUID (16 bytes)
	if (data.size() < 32) {
		mprintf(("VulkanPipelineManager: Pipeline cache file too small\n"));
		return false;
	}

	// Get physical device properties for validation
	vk::PhysicalDeviceProperties props = m_physicalDevice.getProperties();

	// Extract header fields for validation
	// Header format: [headerSize(4), headerVersion(4), vendorId(4), deviceId(4), pipelineCacheUUID(16)]
	// We only validate vendorId, deviceId, and UUID - driver validates headerSize/headerVersion
	uint32_t vendorId = *reinterpret_cast<uint32_t*>(data.data() + 8);
	uint32_t deviceId = *reinterpret_cast<uint32_t*>(data.data() + 12);

	// Validate vendor and device match
	if (vendorId != props.vendorID || deviceId != props.deviceID) {
		mprintf(("VulkanPipelineManager: Pipeline cache vendor/device mismatch, ignoring\n"));
		return false;
	}

	// Validate UUID matches
	if (memcmp(data.data() + 16, props.pipelineCacheUUID, VK_UUID_SIZE) != 0) {
		mprintf(("VulkanPipelineManager: Pipeline cache UUID mismatch, ignoring\n"));
		return false;
	}

	// Recreate pipeline cache with loaded data
	vk::PipelineCacheCreateInfo cacheInfo;
	cacheInfo.initialDataSize = data.size();
	cacheInfo.pInitialData = data.data();

	try {
		m_vkPipelineCache = m_device.createPipelineCacheUnique(cacheInfo);
		mprintf(("VulkanPipelineManager: Loaded pipeline cache from %s (%zu bytes)\n",
		         path.c_str(), data.size()));
		return true;
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPipelineManager: Failed to create pipeline cache from file: %s\n", e.what()));
		// Create empty cache as fallback
		cacheInfo.initialDataSize = 0;
		cacheInfo.pInitialData = nullptr;
		m_vkPipelineCache = m_device.createPipelineCacheUnique(cacheInfo);
		return false;
	}
}

bool VulkanPipelineManager::savePipelineCacheToDisk(const SCP_string& path)
{
	if (!m_initialized || !m_vkPipelineCache) {
		return false;
	}

	try {
		std::vector<uint8_t> data = m_device.getPipelineCacheData(m_vkPipelineCache.get());

		if (data.empty()) {
			mprintf(("VulkanPipelineManager: Pipeline cache is empty, not saving\n"));
			return false;
		}

		std::ofstream file(path, std::ios::binary);
		if (!file.is_open()) {
			mprintf(("VulkanPipelineManager: Failed to open pipeline cache file for writing: %s\n",
			         path.c_str()));
			return false;
		}

		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		mprintf(("VulkanPipelineManager: Saved pipeline cache to %s (%zu bytes)\n",
		         path.c_str(), data.size()));
		return true;
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPipelineManager: Failed to get pipeline cache data: %s\n", e.what()));
		return false;
	}
}

void VulkanPipelineManager::setMsaaSamples(vk::SampleCountFlagBits samples)
{
	m_currentMsaaSamples = samples;
}

vk::SampleCountFlagBits VulkanPipelineManager::getMsaaSamples() const
{
	return m_currentMsaaSamples;
}

// ============================================================================
// Pipeline Creation
// ============================================================================

vk::UniquePipeline VulkanPipelineManager::createPipeline(const PipelineKey& key)
{
	// Get shader modules
	vk::ShaderModule vertShader = m_shaderManager->getVertexShader(key.shaderType, key.shaderFlags);
	vk::ShaderModule fragShader = m_shaderManager->getFragmentShader(key.shaderType, key.shaderFlags);

	if (!vertShader || !fragShader) {
		mprintf(("VulkanPipelineManager: Failed to get shaders for type %d, flags %u\n",
		         static_cast<int>(key.shaderType), key.shaderFlags));
		return vk::UniquePipeline();
	}

	// Shader stages
	SCP_vector<vk::PipelineShaderStageCreateInfo> shaderStages;

	vk::PipelineShaderStageCreateInfo vertStageInfo;
	vertStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertStageInfo.module = vertShader;
	vertStageInfo.pName = "main";
	shaderStages.push_back(vertStageInfo);

	vk::PipelineShaderStageCreateInfo fragStageInfo;
	fragStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragStageInfo.module = fragShader;
	fragStageInfo.pName = "main";
	shaderStages.push_back(fragStageInfo);

	// Check for geometry shader
	if (m_shaderManager->requiresGeometryShader(key.shaderType, key.shaderFlags)) {
		vk::ShaderModule geomShader = m_shaderManager->getGeometryShader(key.shaderType, key.shaderFlags);
		if (geomShader) {
			vk::PipelineShaderStageCreateInfo geomStageInfo;
			geomStageInfo.stage = vk::ShaderStageFlagBits::eGeometry;
			geomStageInfo.module = geomShader;
			geomStageInfo.pName = "main";
			shaderStages.push_back(geomStageInfo);
		}
	}

	// Vertex input state - look up cached layout from hash
	SCP_vector<vk::VertexInputBindingDescription> vertexBindings;
	SCP_vector<vk::VertexInputAttributeDescription> vertexAttributes;
	
	auto layoutIt = m_vertexLayoutCache.find(key.vertexLayoutHash);
	if (layoutIt != m_vertexLayoutCache.end()) {
		createVertexInputState(layoutIt->second, vertexBindings, vertexAttributes);
	} else if (key.vertexLayoutHash != 0) {
		// Hash is set but layout not found - this shouldn't happen if used correctly
		mprintf(("VulkanPipelineManager: Vertex layout hash %zu not found in cache\n", key.vertexLayoutHash));
	}
	// If hash is 0, we use empty vertex input (e.g., for fullscreen passes)
	
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
	vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
	vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

	// Input assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = convertPrimitiveType(key.primitiveType);
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport state - dynamic, so we just specify count
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	// pViewports and pScissors are null because they're dynamic

	// Rasterization state
	vk::PipelineRasterizationStateCreateInfo rasterizer = createRasterizationState(key);

	// Multisample state
	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = key.sampleCount;

	// Depth/stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil = createDepthStencilState(key);

	// Color blend state
	SCP_vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
	vk::PipelineColorBlendStateCreateInfo colorBlending;
	createColorBlendState(key, colorBlendAttachments, colorBlending);

	// Dynamic state - CRITICAL: viewport and scissor must be dynamic
	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::eLineWidth,
	};

	vk::PipelineDynamicStateCreateInfo dynamicState;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Get pipeline layout
	vk::PipelineLayout layout = getOrCreateLayout(key.shaderType, key.shaderFlags);
	if (!layout) {
		mprintf(("VulkanPipelineManager: Failed to get pipeline layout\n"));
		return vk::UniquePipeline();
	}

	// Dynamic rendering (Vulkan 1.3+) - specify attachment formats directly
	// This replaces VkRenderPass/VkFramebuffer with inline format specification
	vk::PipelineRenderingCreateInfo renderingCreateInfo;
	vk::Format colorFormats[] = { key.colorFormat };
	renderingCreateInfo.colorAttachmentCount = 1;
	renderingCreateInfo.pColorAttachmentFormats = colorFormats;
	renderingCreateInfo.depthAttachmentFormat = key.depthFormat;
	renderingCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;

	// Create the pipeline
	vk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.pNext = &renderingCreateInfo;  // Dynamic rendering via pNext chain
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = nullptr;  // No render pass with dynamic rendering
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = nullptr;
	pipelineInfo.basePipelineIndex = -1;

	try {
		auto result = m_device.createGraphicsPipelineUnique(m_vkPipelineCache.get(), pipelineInfo);
		return std::move(result.value);
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPipelineManager: Failed to create pipeline: %s\n", e.what()));
		return vk::UniquePipeline();
	}
}

PipelineKey VulkanPipelineManager::buildKeyFromMaterial(const material& mat,
                                                          const vertex_layout& layout,
                                                          primitive_type primType,
                                                          vk::Format colorFormat,
                                                          vk::Format depthFormat)
{
	PipelineKey key;

	// Shader identification
	key.shaderType = mat.get_shader_type();
	key.shaderFlags = mat.get_shader_flags();

	// Vertex layout hash and cache
	key.vertexLayoutHash = layout.hash();

	// Cache the layout so createPipeline can retrieve it
	if (m_vertexLayoutCache.find(key.vertexLayoutHash) == m_vertexLayoutCache.end()) {
		m_vertexLayoutCache[key.vertexLayoutHash] = layout;
	}

	// Primitive topology
	key.primitiveType = primType;

	// Rasterization
	key.cullEnabled = mat.get_cull_mode();
	key.cullMode = key.cullEnabled ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone;
	key.polygonMode = (mat.get_fill_mode() == GR_FILL_MODE_WIRE) ?
	                   vk::PolygonMode::eLine : vk::PolygonMode::eFill;

	// Depth mode
	key.depthMode = mat.get_depth_mode();

	// Stencil state
	key.stencilEnabled = mat.is_stencil_enabled();
	if (key.stencilEnabled) {
		const auto& stencilFunc = mat.get_stencil_func();
		key.stencilFunc = stencilFunc.compare;

		const auto& frontOp = mat.get_front_stencil_op();
		key.stencilFailOp = frontOp.stencilFailOperation;
		key.stencilDepthFailOp = frontOp.depthFailOperation;
		key.stencilPassOp = frontOp.successOperation;
	}

	// Blend mode
	key.blendMode = mat.get_blend_mode();
	key.hasPerBufferBlend = mat.has_buffer_blend_modes();
	if (key.hasPerBufferBlend) {
		for (size_t i = 0; i < material::NUM_BUFFER_BLENDS; ++i) {
			key.bufferBlendModes[i] = mat.get_blend_mode(static_cast<int>(i));
		}
	}

	// Color mask
	key.colorMask = mat.get_color_mask();

	// Dynamic rendering formats (Vulkan 1.3+)
	key.colorFormat = colorFormat;
	key.depthFormat = depthFormat;

	// MSAA
	key.sampleCount = m_currentMsaaSamples;

	return key;
}

// Map vertex format type to shader location
// This matches the opengl_vert_attrib enum order used by FSO shaders
static uint32_t getVertexFormatLocation(vertex_format_data::vertex_format format) {
	switch (format) {
		// POSITION variants -> location 0
		case vertex_format_data::POSITION4:
		case vertex_format_data::POSITION3:
		case vertex_format_data::POSITION2:
		case vertex_format_data::SCREEN_POS:
			return 0;
		// COLOR variants -> location 1
		case vertex_format_data::COLOR3:
		case vertex_format_data::COLOR4:
		case vertex_format_data::COLOR4F:
			return 1;
		// TEXCOORD variants -> location 2
		case vertex_format_data::TEX_COORD2:
		case vertex_format_data::TEX_COORD4:
			return 2;
		// NORMAL -> location 3
		case vertex_format_data::NORMAL:
			return 3;
		// TANGENT -> location 4
		case vertex_format_data::TANGENT:
			return 4;
		// MODEL_ID -> location 5
		case vertex_format_data::MODEL_ID:
			return 5;
		// RADIUS -> location 6
		case vertex_format_data::RADIUS:
			return 6;
		// UVEC -> location 7
		case vertex_format_data::UVEC:
			return 7;
		// MATRIX4 -> locations 8-11 (uses 4 locations)
		case vertex_format_data::MATRIX4:
			return 8;
		default:
			return 0;
	}
}

void VulkanPipelineManager::createVertexInputState(const vertex_layout& layout,
                                                     SCP_vector<vk::VertexInputBindingDescription>& bindings,
                                                     SCP_vector<vk::VertexInputAttributeDescription>& attributes)
{
	bindings.clear();
	attributes.clear();

	// Track which binding indices we've seen
	std::set<size_t> seenBindings;

	for (size_t i = 0; i < layout.get_num_vertex_components(); ++i) {
		const vertex_format_data* component = layout.get_vertex_component(i);

		// Add binding description if not already added
		uint32_t bindingIndex = static_cast<uint32_t>(component->buffer_number);
		if (seenBindings.find(bindingIndex) == seenBindings.end()) {
			vk::VertexInputBindingDescription binding;
			binding.binding = bindingIndex;
			binding.stride = static_cast<uint32_t>(component->stride);
			binding.inputRate = (component->divisor > 0) ?
			                     vk::VertexInputRate::eInstance :
			                     vk::VertexInputRate::eVertex;
			bindings.push_back(binding);
			seenBindings.insert(bindingIndex);
		}

		// Add attribute description
		// Location is determined by the vertex format type, not the component index
		vk::VertexInputAttributeDescription attr;
		attr.binding = bindingIndex;
		attr.location = getVertexFormatLocation(component->format_type);
		attr.format = convertVertexFormat(component->format_type);
		attr.offset = static_cast<uint32_t>(component->offset);
		attributes.push_back(attr);
	}
}

vk::PrimitiveTopology VulkanPipelineManager::convertPrimitiveType(primitive_type primType)
{
	switch (primType) {
	case PRIM_TYPE_POINTS:
		return vk::PrimitiveTopology::ePointList;
	case PRIM_TYPE_LINES:
		return vk::PrimitiveTopology::eLineList;
	case PRIM_TYPE_LINESTRIP:
		return vk::PrimitiveTopology::eLineStrip;
	case PRIM_TYPE_TRIS:
		return vk::PrimitiveTopology::eTriangleList;
	case PRIM_TYPE_TRISTRIP:
		return vk::PrimitiveTopology::eTriangleStrip;
	case PRIM_TYPE_TRIFAN:
		return vk::PrimitiveTopology::eTriangleFan;
	default:
		return vk::PrimitiveTopology::eTriangleList;
	}
}

vk::PipelineRasterizationStateCreateInfo VulkanPipelineManager::createRasterizationState(const PipelineKey& key)
{
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = key.polygonMode;
	rasterizer.lineWidth = 1.0f;  // Dynamic state
	rasterizer.cullMode = key.cullEnabled ? key.cullMode : vk::CullModeFlagBits::eNone;
	// Use clockwise winding because we flip Y-axis via negative viewport height
	// This reverses the apparent winding order, so we compensate here
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;  // Could be enabled for shadow maps
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	return rasterizer;
}

vk::PipelineDepthStencilStateCreateInfo VulkanPipelineManager::createDepthStencilState(const PipelineKey& key)
{
	vk::PipelineDepthStencilStateCreateInfo depthStencil;

	// Convert depth mode
	vk::CompareOp compareOp;
	bool depthWrite, depthTest;
	convertDepthMode(key.depthMode, compareOp, depthWrite, depthTest);

	depthStencil.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
	depthStencil.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
	depthStencil.depthCompareOp = compareOp;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;

	// Stencil state
	depthStencil.stencilTestEnable = key.stencilEnabled ? VK_TRUE : VK_FALSE;

	if (key.stencilEnabled) {
		vk::StencilOpState stencilOp;
		stencilOp.failOp = convertStencilOp(key.stencilFailOp);
		stencilOp.depthFailOp = convertStencilOp(key.stencilDepthFailOp);
		stencilOp.passOp = convertStencilOp(key.stencilPassOp);
		stencilOp.compareOp = convertCompareFunc(key.stencilFunc);
		stencilOp.compareMask = 0xFF;
		stencilOp.writeMask = 0xFF;
		stencilOp.reference = 1;

		depthStencil.front = stencilOp;
		depthStencil.back = stencilOp;
	}

	return depthStencil;
}

void VulkanPipelineManager::createColorBlendState(const PipelineKey& key,
                                                    SCP_vector<vk::PipelineColorBlendAttachmentState>& attachments,
                                                    vk::PipelineColorBlendStateCreateInfo& blendInfo)
{
	attachments.clear();

	// Determine number of color attachments
	// For now, assume 1 attachment unless per-buffer blending is enabled
	size_t numAttachments = key.hasPerBufferBlend ? material::NUM_BUFFER_BLENDS : 1;

	for (size_t i = 0; i < numAttachments; ++i) {
		gr_alpha_blend mode = key.hasPerBufferBlend ? key.bufferBlendModes[i] : key.blendMode;

		vk::BlendFactor srcColor, dstColor, srcAlpha, dstAlpha;
		convertBlendMode(mode, srcColor, dstColor, srcAlpha, dstAlpha);

		vk::PipelineColorBlendAttachmentState attachment;
		attachment.colorWriteMask =
			(key.colorMask.x ? vk::ColorComponentFlagBits::eR : vk::ColorComponentFlags{}) |
			(key.colorMask.y ? vk::ColorComponentFlagBits::eG : vk::ColorComponentFlags{}) |
			(key.colorMask.z ? vk::ColorComponentFlagBits::eB : vk::ColorComponentFlags{}) |
			(key.colorMask.w ? vk::ColorComponentFlagBits::eA : vk::ColorComponentFlags{});

		attachment.blendEnable = (mode != ALPHA_BLEND_NONE) ? VK_TRUE : VK_FALSE;
		attachment.srcColorBlendFactor = srcColor;
		attachment.dstColorBlendFactor = dstColor;
		attachment.colorBlendOp = vk::BlendOp::eAdd;
		attachment.srcAlphaBlendFactor = srcAlpha;
		attachment.dstAlphaBlendFactor = dstAlpha;
		attachment.alphaBlendOp = vk::BlendOp::eAdd;

		attachments.push_back(attachment);
	}

	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.logicOp = vk::LogicOp::eCopy;
	blendInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	blendInfo.pAttachments = attachments.data();
	blendInfo.blendConstants[0] = 0.0f;
	blendInfo.blendConstants[1] = 0.0f;
	blendInfo.blendConstants[2] = 0.0f;
	blendInfo.blendConstants[3] = 0.0f;
}

bool VulkanPipelineManager::createMaterialDescriptorLayout()
{
	if (!m_device) {
		return false;
	}

	// Create bindings for all texture slots used by materials
	// Binding order matches texture unit usage in shaders
	SCP_vector<vk::DescriptorSetLayoutBinding> bindings;
	
	// Base texture (diffuseMap) - binding 0
	bindings.push_back({
		0,  // binding
		vk::DescriptorType::eCombinedImageSampler,
		1,  // descriptorCount
		vk::ShaderStageFlagBits::eFragment,
		nullptr  // pImmutableSamplers
	});
	
	// Glow texture (glowMap) - binding 1
	bindings.push_back({
		1,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});
	
	// Normal texture (normalMap) - binding 2
	bindings.push_back({
		2,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});
	
	// Specular texture (specularMap) - binding 3
	bindings.push_back({
		3,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});
	
	// Environment map (sEnvmap) - binding 4
	bindings.push_back({
		4,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});
	
	// Irradiance map (sIrrmap) - binding 5
	bindings.push_back({
		5,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});
	
	// BRDF LUT (sBRDFLUT) - binding 6
	bindings.push_back({
		6,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});
	
	// SSAO texture (ssaoTex) - binding 7
	bindings.push_back({
		7,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});
	
	// Shadow map (Shadow_map_texture) - binding 8
	bindings.push_back({
		8,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});

	// Ambient occlusion texture (sAmbientmap) - binding 9
	bindings.push_back({
		9,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});

	// Misc/utility texture (sMiscmap) - binding 10
	bindings.push_back({
		10,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});

	// Scene framebuffer texture (sFramebuffer) - binding 11
	bindings.push_back({
		11,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	});

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	try {
		m_materialDescriptorSetLayout = m_device.createDescriptorSetLayoutUnique(layoutInfo);
		mprintf(("VulkanPipelineManager: Created material descriptor set layout with %zu bindings\n", 
		         bindings.size()));
		return true;
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPipelineManager: Failed to create material descriptor set layout: %s\n", e.what()));
		return false;
	}
}

vk::PipelineLayout VulkanPipelineManager::getOrCreateLayout(shader_type type, uint32_t flags)
{
	SCP_string key = makeLayoutKey(type, flags);

	auto it = m_layoutCache.find(key);
	if (it != m_layoutCache.end()) {
		return it->second.get();
	}

	// Create pipeline layout with descriptor sets
	// Set 0: Uniform buffers (global)
	// Set 1: Material textures (per-material)
	vk::PipelineLayoutCreateInfo layoutInfo;
	SCP_vector<vk::DescriptorSetLayout> setLayouts;

	// Set 0: Uniform buffers (global)
	if (m_uniformDescriptorSetLayout) {
		setLayouts.push_back(m_uniformDescriptorSetLayout.get());
	}

	// Set 1: Material textures (per-material)
	if (m_materialDescriptorSetLayout) {
		setLayouts.push_back(m_materialDescriptorSetLayout.get());
	}

	layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	layoutInfo.pSetLayouts = setLayouts.data();

	// Push constants for Buffer Device Address (BDA) uniform buffer addresses
	// Contains 9 x 8-byte GPU addresses (one per uniform_block_type)
	// This allows shaders to access uniform data via buffer_reference without descriptor sets
	vk::PushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex |
	                                vk::ShaderStageFlagBits::eFragment |
	                                vk::ShaderStageFlagBits::eGeometry;
	pushConstantRange.offset = 0;
	pushConstantRange.size = VulkanBufferManager::UniformAddressPushConstants::size();

	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &pushConstantRange;

	// TODO: Integrate shader reflection for dynamic descriptor set layout creation
	// Query shader reflection for descriptor set layouts to ensure pipeline layouts match shader expectations
	// This would use VulkanShaderManager::getShaderReflection(type, flags) and VulkanDescriptorSetLayoutBuilder
	// to dynamically build descriptor set layouts from SPIR-V reflection data.
	// For now, we use hardcoded layouts (uniform buffers at Set 0, material textures at Set 1) which work
	// for the current shader set, but reflection-based layout creation would be more robust and future-proof.
	//
	// Example integration (when reflection is available):
	// if (m_shaderManager) {
	//     auto reflection = m_shaderManager->getShaderReflection(type, flags);
	//     if (reflection && !reflection.descriptors.empty()) {
	//         VulkanDescriptorSetLayoutBuilder builder(m_device);
	//         auto layouts = builder.build(reflection);
	//         // Use reflection-based layouts instead of hardcoded ones
	//     }
	// }

	try {
		m_layoutCache[key] = m_device.createPipelineLayoutUnique(layoutInfo);
		mprintf(("VulkanPipelineManager: Created pipeline layout with %u push constant bytes (BDA uniform addresses)\n",
		         pushConstantRange.size));
		return m_layoutCache[key].get();
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanPipelineManager: Failed to create pipeline layout: %s\n", e.what()));
		return nullptr;
	}
}

SCP_string VulkanPipelineManager::makeLayoutKey(shader_type type, uint32_t flags)
{
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%d_%u", static_cast<int>(type), flags);
	return SCP_string(buffer);
}

// ============================================================================
// State Conversion Helpers
// ============================================================================

void VulkanPipelineManager::convertBlendMode(gr_alpha_blend mode,
                                               vk::BlendFactor& srcColor, vk::BlendFactor& dstColor,
                                               vk::BlendFactor& srcAlpha, vk::BlendFactor& dstAlpha)
{
	switch (mode) {
	case ALPHA_BLEND_NONE:
		// 1*SrcPixel + 0*DestPixel
		srcColor = vk::BlendFactor::eOne;
		dstColor = vk::BlendFactor::eZero;
		break;

	case ALPHA_BLEND_ADDITIVE:
		// 1*SrcPixel + 1*DestPixel
		srcColor = vk::BlendFactor::eOne;
		dstColor = vk::BlendFactor::eOne;
		break;

	case ALPHA_BLEND_ALPHA_ADDITIVE:
		// Alpha*SrcPixel + 1*DestPixel
		srcColor = vk::BlendFactor::eSrcAlpha;
		dstColor = vk::BlendFactor::eOne;
		break;

	case ALPHA_BLEND_ALPHA_BLEND_ALPHA:
		// Alpha*SrcPixel + (1-Alpha)*DestPixel
		srcColor = vk::BlendFactor::eSrcAlpha;
		dstColor = vk::BlendFactor::eOneMinusSrcAlpha;
		break;

	case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:
		// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
		srcColor = vk::BlendFactor::eSrcAlpha;
		dstColor = vk::BlendFactor::eOneMinusSrcColor;
		break;

	case ALPHA_BLEND_PREMULTIPLIED:
		// 1*SrcPixel + (1-Alpha)*DestPixel
		srcColor = vk::BlendFactor::eOne;
		dstColor = vk::BlendFactor::eOneMinusSrcAlpha;
		break;

	default:
		srcColor = vk::BlendFactor::eOne;
		dstColor = vk::BlendFactor::eZero;
		break;
	}

	// Alpha channel uses same factors
	srcAlpha = srcColor;
	dstAlpha = dstColor;
}

void VulkanPipelineManager::convertDepthMode(gr_zbuffer_type mode,
                                               vk::CompareOp& compareOp, bool& depthWrite, bool& depthTest)
{
	switch (mode) {
	case ZBUFFER_TYPE_NONE:
		compareOp = vk::CompareOp::eAlways;
		depthWrite = false;
		depthTest = false;
		break;

	case ZBUFFER_TYPE_READ:
		compareOp = vk::CompareOp::eLess;
		depthWrite = false;
		depthTest = true;
		break;

	case ZBUFFER_TYPE_WRITE:
		compareOp = vk::CompareOp::eAlways;
		depthWrite = true;
		depthTest = true;
		break;

	case ZBUFFER_TYPE_FULL:
	case ZBUFFER_TYPE_DEFAULT:
	default:
		compareOp = vk::CompareOp::eLess;
		depthWrite = true;
		depthTest = true;
		break;
	}
}

vk::CompareOp VulkanPipelineManager::convertCompareFunc(ComparisionFunction func)
{
	switch (func) {
	case ComparisionFunction::Never:
		return vk::CompareOp::eNever;
	case ComparisionFunction::Always:
		return vk::CompareOp::eAlways;
	case ComparisionFunction::Less:
		return vk::CompareOp::eLess;
	case ComparisionFunction::Greater:
		return vk::CompareOp::eGreater;
	case ComparisionFunction::Equal:
		return vk::CompareOp::eEqual;
	case ComparisionFunction::NotEqual:
		return vk::CompareOp::eNotEqual;
	case ComparisionFunction::LessOrEqual:
		return vk::CompareOp::eLessOrEqual;
	case ComparisionFunction::GreaterOrEqual:
		return vk::CompareOp::eGreaterOrEqual;
	default:
		return vk::CompareOp::eAlways;
	}
}

vk::StencilOp VulkanPipelineManager::convertStencilOp(StencilOperation op)
{
	switch (op) {
	case StencilOperation::Keep:
		return vk::StencilOp::eKeep;
	case StencilOperation::Zero:
		return vk::StencilOp::eZero;
	case StencilOperation::Replace:
		return vk::StencilOp::eReplace;
	case StencilOperation::Increment:
		return vk::StencilOp::eIncrementAndClamp;
	case StencilOperation::IncrementWrap:
		return vk::StencilOp::eIncrementAndWrap;
	case StencilOperation::Decrement:
		return vk::StencilOp::eDecrementAndClamp;
	case StencilOperation::DecrementWrap:
		return vk::StencilOp::eDecrementAndWrap;
	case StencilOperation::Invert:
		return vk::StencilOp::eInvert;
	default:
		return vk::StencilOp::eKeep;
	}
}

vk::Format VulkanPipelineManager::convertVertexFormat(vertex_format_data::vertex_format format)
{
	switch (format) {
	case vertex_format_data::POSITION4:
		return vk::Format::eR32G32B32A32Sfloat;
	case vertex_format_data::POSITION3:
		return vk::Format::eR32G32B32Sfloat;
	case vertex_format_data::POSITION2:
		return vk::Format::eR32G32Sfloat;
	case vertex_format_data::SCREEN_POS:
		return vk::Format::eR32G32B32Sfloat;  // screen3d has sw component
	case vertex_format_data::COLOR3:
		return vk::Format::eR8G8B8Unorm;
	case vertex_format_data::COLOR4:
		return vk::Format::eR8G8B8A8Unorm;
	case vertex_format_data::COLOR4F:
		return vk::Format::eR32G32B32A32Sfloat;
	case vertex_format_data::TEX_COORD2:
		return vk::Format::eR32G32Sfloat;
	case vertex_format_data::TEX_COORD4:
		return vk::Format::eR32G32B32A32Sfloat;
	case vertex_format_data::NORMAL:
		return vk::Format::eR32G32B32Sfloat;
	case vertex_format_data::TANGENT:
		return vk::Format::eR32G32B32A32Sfloat;  // vec4 for bitangent sign
	case vertex_format_data::MODEL_ID:
		return vk::Format::eR32Sfloat;
	case vertex_format_data::RADIUS:
		return vk::Format::eR32Sfloat;
	case vertex_format_data::UVEC:
		return vk::Format::eR32G32B32Sfloat;
	case vertex_format_data::MATRIX4:
		// First row of 4x4 matrix - needs 4 consecutive attribute slots
		return vk::Format::eR32G32B32A32Sfloat;
	default:
		mprintf(("VulkanPipelineManager: Unknown vertex format %d\n", static_cast<int>(format)));
		return vk::Format::eUndefined;
	}
}

// ============================================================================
// Static Conversion Functions (for unit testing)
// ============================================================================

vk::Format VulkanPipelineManager::convertVertexFormatStatic(vertex_format_data::vertex_format format)
{
	switch (format) {
	case vertex_format_data::POSITION4:
		return vk::Format::eR32G32B32A32Sfloat;
	case vertex_format_data::POSITION3:
		return vk::Format::eR32G32B32Sfloat;
	case vertex_format_data::POSITION2:
		return vk::Format::eR32G32Sfloat;
	case vertex_format_data::SCREEN_POS:
		return vk::Format::eR32G32B32Sfloat;
	case vertex_format_data::COLOR3:
		return vk::Format::eR8G8B8Unorm;
	case vertex_format_data::COLOR4:
		return vk::Format::eR8G8B8A8Unorm;
	case vertex_format_data::COLOR4F:
		return vk::Format::eR32G32B32A32Sfloat;
	case vertex_format_data::TEX_COORD2:
		return vk::Format::eR32G32Sfloat;
	case vertex_format_data::TEX_COORD4:
		return vk::Format::eR32G32B32A32Sfloat;
	case vertex_format_data::NORMAL:
		return vk::Format::eR32G32B32Sfloat;
	case vertex_format_data::TANGENT:
		return vk::Format::eR32G32B32A32Sfloat;
	case vertex_format_data::MODEL_ID:
		return vk::Format::eR32Sfloat;
	case vertex_format_data::RADIUS:
		return vk::Format::eR32Sfloat;
	case vertex_format_data::UVEC:
		return vk::Format::eR32G32B32Sfloat;
	case vertex_format_data::MATRIX4:
		return vk::Format::eR32G32B32A32Sfloat;
	default:
		return vk::Format::eUndefined;
	}
}

vk::PrimitiveTopology VulkanPipelineManager::convertPrimitiveTypeStatic(primitive_type primType)
{
	switch (primType) {
	case PRIM_TYPE_POINTS:
		return vk::PrimitiveTopology::ePointList;
	case PRIM_TYPE_LINES:
		return vk::PrimitiveTopology::eLineList;
	case PRIM_TYPE_LINESTRIP:
		return vk::PrimitiveTopology::eLineStrip;
	case PRIM_TYPE_TRIS:
		return vk::PrimitiveTopology::eTriangleList;
	case PRIM_TYPE_TRISTRIP:
		return vk::PrimitiveTopology::eTriangleStrip;
	case PRIM_TYPE_TRIFAN:
		return vk::PrimitiveTopology::eTriangleFan;
	default:
		return vk::PrimitiveTopology::eTriangleList;
	}
}

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
