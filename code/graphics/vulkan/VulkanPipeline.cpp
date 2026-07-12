#include "VulkanPipeline.h"
#include "VulkanConvert.h"

#include "cfile/cfile.h"


namespace graphics::vulkan {

// Global pipeline manager pointer
static VulkanPipelineManager* g_pipelineManager = nullptr;

VulkanPipelineManager* getPipelineManager()
{
	Assertion(g_pipelineManager != nullptr, "Vulkan PipelineManager not initialized!");
	return g_pipelineManager;
}

void setPipelineManager(VulkanPipelineManager* manager)
{
	g_pipelineManager = manager;
}

bool PipelineConfig::operator==(const PipelineConfig& other) const
{
	return shaderType == other.shaderType &&
	       shaderFlags == other.shaderFlags &&
	       vertexLayoutHash == other.vertexLayoutHash &&
	       primitiveType == other.primitiveType &&
	       depthMode == other.depthMode &&
	       blendMode == other.blendMode &&
	       cullEnabled == other.cullEnabled &&
	       frontFaceCW == other.frontFaceCW &&
	       depthWriteEnabled == other.depthWriteEnabled &&
	       stencilEnabled == other.stencilEnabled &&
	       stencilFunc == other.stencilFunc &&
	       stencilMask == other.stencilMask &&
	       frontStencilOp.stencilFailOperation == other.frontStencilOp.stencilFailOperation &&
	       frontStencilOp.depthFailOperation == other.frontStencilOp.depthFailOperation &&
	       frontStencilOp.successOperation == other.frontStencilOp.successOperation &&
	       backStencilOp.stencilFailOperation == other.backStencilOp.stencilFailOperation &&
	       backStencilOp.depthFailOperation == other.backStencilOp.depthFailOperation &&
	       backStencilOp.successOperation == other.backStencilOp.successOperation &&
	       colorWriteMask.x == other.colorWriteMask.x &&
	       colorWriteMask.y == other.colorWriteMask.y &&
	       colorWriteMask.z == other.colorWriteMask.z &&
	       colorWriteMask.w == other.colorWriteMask.w &&
	       fillMode == other.fillMode &&
	       depthBiasEnabled == other.depthBiasEnabled &&
	       renderPass == other.renderPass &&
	       subpass == other.subpass &&
	       colorAttachmentCount == other.colorAttachmentCount &&
	       sampleCount == other.sampleCount &&
	       perAttachmentBlendEnabled == other.perAttachmentBlendEnabled &&
	       [&]() {
	           if (!perAttachmentBlendEnabled) return true;
	           for (uint32_t i = 0; i < colorAttachmentCount; ++i) {
	               if (attachmentBlends[i].blendMode != other.attachmentBlends[i].blendMode ||
	                   attachmentBlends[i].writeMask.x != other.attachmentBlends[i].writeMask.x ||
	                   attachmentBlends[i].writeMask.y != other.attachmentBlends[i].writeMask.y ||
	                   attachmentBlends[i].writeMask.z != other.attachmentBlends[i].writeMask.z ||
	                   attachmentBlends[i].writeMask.w != other.attachmentBlends[i].writeMask.w)
	                   return false;
	           }
	           return true;
	       }();
}

namespace {
// boost::hash_combine-style 64-bit fold (same pattern as vertex_layout::hash()).
// Order-dependent and avalanches every field into all bits, unlike the previous
// `h ^= hash(field) << shift` scheme whose overlapping/wrapping shifts collided
// heavily. operator== remains the authority for equality; a better hash only
// reduces bucket collisions (fewer operator== calls per lookup).
inline void hashCombine(uint64_t& seed, uint64_t value)
{
	seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
}
inline uint64_t maskBits(bool x, bool y, bool z, bool w)
{
	return (x ? 1u : 0u) | (y ? 2u : 0u) | (z ? 4u : 0u) | (w ? 8u : 0u);
}
} // namespace

uint64_t PipelineConfig::hash() const
{
	uint64_t h = 0;

	// Fold every field operator== compares, so equal configs always hash equal.
	hashCombine(h, static_cast<uint64_t>(shaderType));
	hashCombine(h, shaderFlags);
	hashCombine(h, vertexLayoutHash);
	hashCombine(h, static_cast<uint64_t>(primitiveType));
	hashCombine(h, static_cast<uint64_t>(depthMode));
	hashCombine(h, static_cast<uint64_t>(blendMode));
	hashCombine(h, cullEnabled ? 1u : 0u);
	hashCombine(h, frontFaceCW ? 1u : 0u);
	hashCombine(h, depthWriteEnabled ? 1u : 0u);
	hashCombine(h, stencilEnabled ? 1u : 0u);
	hashCombine(h, static_cast<uint64_t>(stencilFunc));
	hashCombine(h, stencilMask);
	hashCombine(h, static_cast<uint64_t>(frontStencilOp.stencilFailOperation));
	hashCombine(h, static_cast<uint64_t>(frontStencilOp.depthFailOperation));
	hashCombine(h, static_cast<uint64_t>(frontStencilOp.successOperation));
	hashCombine(h, static_cast<uint64_t>(backStencilOp.stencilFailOperation));
	hashCombine(h, static_cast<uint64_t>(backStencilOp.depthFailOperation));
	hashCombine(h, static_cast<uint64_t>(backStencilOp.successOperation));
	hashCombine(h, maskBits(colorWriteMask.x, colorWriteMask.y, colorWriteMask.z, colorWriteMask.w));
	hashCombine(h, static_cast<uint64_t>(fillMode));
	hashCombine(h, depthBiasEnabled ? 1u : 0u);
	hashCombine(h, reinterpret_cast<uint64_t>(static_cast<VkRenderPass>(renderPass)));
	hashCombine(h, subpass);
	hashCombine(h, colorAttachmentCount);
	hashCombine(h, static_cast<uint64_t>(sampleCount));
	hashCombine(h, perAttachmentBlendEnabled ? 1u : 0u);
	if (perAttachmentBlendEnabled) {
		for (uint32_t i = 0; i < colorAttachmentCount; ++i) {
			hashCombine(h, static_cast<uint64_t>(attachmentBlends[i].blendMode));
			hashCombine(h, maskBits(attachmentBlends[i].writeMask.x, attachmentBlends[i].writeMask.y,
			                        attachmentBlends[i].writeMask.z, attachmentBlends[i].writeMask.w));
		}
	}

	return h;
}

bool VulkanPipelineManager::init(vk::Device device, VulkanShaderManager* shaderManager,
                                  VulkanDescriptorManager* descriptorManager)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;
	m_shaderManager = shaderManager;
	m_descriptorManager = descriptorManager;

	// Create empty pipeline cache
	vk::PipelineCacheCreateInfo cacheInfo;
	m_pipelineCache = m_device.createPipelineCacheUnique(cacheInfo);

	// Create common pipeline layout
	createPipelineLayout();

	m_initialized = true;
	nprintf(("vulkan", "VulkanPipelineManager: Initialized\n"));
	return true;
}

void VulkanPipelineManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Wait for device idle
	m_device.waitIdle();

	// Clear all pipelines
	m_pipelines.clear();
	m_pipelineLayout.reset();
	m_pipelineCache.reset();
	m_vertexFormatCache.clear();

	m_initialized = false;
	nprintf(("vulkan", "VulkanPipelineManager: Shutdown complete\n"));
}

vk::Pipeline VulkanPipelineManager::getPipeline(const PipelineConfig& config, const vertex_layout& vertexLayout)
{
	Assertion(m_initialized, "VulkanPipelineManager::getPipeline called before initialization!");

	// Update vertex layout hash in config
	PipelineConfig fullConfig = config;
	fullConfig.vertexLayoutHash = vertexLayout.hash();

	// Check cache
	auto it = m_pipelines.find(fullConfig);
	if (it != m_pipelines.end()) {
		return it->second.get();
	}

	// Create new pipeline
	auto pipeline = createPipeline(fullConfig, vertexLayout);
	if (!pipeline) {
		return {};
	}

	vk::Pipeline result = pipeline.get();
	m_pipelines[fullConfig] = std::move(pipeline);

	nprintf(("Vulkan", "VulkanPipelineManager: Created pipeline for shader type %d (hash 0x%" PRIx64 ")\n",
		static_cast<int>(config.shaderType), fullConfig.hash()));

	return result;
}

bool VulkanPipelineManager::loadPipelineCache(const SCP_string& filename)
{
	// Try to load cache file
	CFILE* fp = cfopen(filename.c_str(), "rb", CF_TYPE_CACHE);
	if (!fp) {
		nprintf(("Vulkan", "VulkanPipelineManager: No pipeline cache file found: %s\n", filename.c_str()));
		return false;
	}

	// Get file size
	int fileSize = cfilelength(fp);
	if (fileSize <= 0) {
		cfclose(fp);
		return false;
	}

	// Read cache data
	SCP_vector<uint8_t> cacheData(fileSize);
	if (cfread(cacheData.data(), 1, fileSize, fp) != fileSize) {
		cfclose(fp);
		return false;
	}
	cfclose(fp);

	// Create new pipeline cache with data
	vk::PipelineCacheCreateInfo cacheInfo;
	cacheInfo.initialDataSize = cacheData.size();
	cacheInfo.pInitialData = cacheData.data();

	try {
		auto newCache = m_device.createPipelineCacheUnique(cacheInfo);
		m_pipelineCache = std::move(newCache);
		nprintf(("vulkan", "VulkanPipelineManager: Loaded pipeline cache: %s (%d bytes)\n",
			filename.c_str(), fileSize));
		return true;
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanPipelineManager: Failed to load pipeline cache: %s\n", e.what()));
		return false;
	}
}

bool VulkanPipelineManager::savePipelineCache(const SCP_string& filename)
{
	if (!m_pipelineCache) {
		return false;
	}

	// Get cache data
	auto cacheData = m_device.getPipelineCacheData(m_pipelineCache.get());
	if (cacheData.empty()) {
		return false;
	}

	// Write to file
	CFILE* fp = cfopen(filename.c_str(), "wb", CF_TYPE_CACHE);
	if (!fp) {
		nprintf(("vulkan", "VulkanPipelineManager: Could not create cache file: %s\n", filename.c_str()));
		return false;
	}

	bool success = (cfwrite(cacheData.data(), 1, static_cast<int>(cacheData.size()), fp) ==
		static_cast<int>(cacheData.size()));
	cfclose(fp);

	if (success) {
		nprintf(("vulkan", "VulkanPipelineManager: Saved pipeline cache: %s (%zu bytes)\n",
			filename.c_str(), cacheData.size()));
	}

	return success;
}

bool VulkanPipelineManager::needsFallbackAttribute(const vertex_layout& vertexLayout, shader_type shaderType,
                                                     VertexAttributeLocation location)
{
	// Empty layouts (fullscreen triangle etc.) don't use fallbacks
	if (vertexLayout.get_num_vertex_components() == 0) return false;

	const VertexInputConfig& config = m_vertexFormatCache.getVertexInputConfig(vertexLayout);
	uint32_t bit = 1u << location;

	// Layout natively provides this attribute — no fallback needed
	if (config.providedInputMask & bit) return false;

	// Fallback needed only if the shader actually consumes this attribute
	const VulkanShaderModule* shader = m_shaderManager->getShaderByType(shaderType);
	if (shader && shader->vertexInputMask != 0) {
		return (shader->vertexInputMask & bit) != 0;
	}
	return true;
}

void VulkanPipelineManager::createPipelineLayout()
{
	// Get descriptor set layouts from descriptor manager
	const auto& uniqueLayouts = m_descriptorManager->getAllSetLayouts();
	std::array<vk::DescriptorSetLayout, static_cast<size_t>(DescriptorSetIndex::Count)> setLayouts;
	for (size_t i = 0; i < uniqueLayouts.size(); ++i) {
		setLayouts[i] = uniqueLayouts[i].get();
	}

	vk::PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	layoutInfo.pSetLayouts = setLayouts.data();
	layoutInfo.pushConstantRangeCount = 0;
	layoutInfo.pPushConstantRanges = nullptr;

	m_pipelineLayout = m_device.createPipelineLayoutUnique(layoutInfo);

	nprintf(("vulkan", "VulkanPipelineManager: Created pipeline layout with %zu descriptor sets\n",
		setLayouts.size()));
}

vk::UniquePipeline VulkanPipelineManager::createPipeline(const PipelineConfig& config,
                                                          const vertex_layout& vertexLayout)
{
	// Ensure shader variant is loaded (lazy creation on first use)
	int shaderHandle = m_shaderManager->maybeCreateShader(config.shaderType, config.shaderFlags);

	// Get shader modules
	const VulkanShaderModule* shader = (shaderHandle >= 0) ? m_shaderManager->getShader(shaderHandle) : nullptr;
	if (!shader || !shader->valid) {
		nprintf(("vulkan", "VulkanPipelineManager: Shader not available for type %d\n",
			static_cast<int>(config.shaderType)));
		return {};
	}

	// Debug: Log which shader and vertex layout is being used
	nprintf(("vulkan", "VulkanPipelineManager: Creating pipeline for shader type %d (%s)\n",
		static_cast<int>(config.shaderType), shader->description.c_str()));
	nprintf(("vulkan", "  Vertex layout has %zu components:\n", vertexLayout.get_num_vertex_components()));
	for (size_t i = 0; i < vertexLayout.get_num_vertex_components(); ++i) {
		const vertex_format_data* comp = vertexLayout.get_vertex_component(i);
		nprintf(("vulkan", "    [%zu] format=%d offset=%zu stride=%zu\n", i,
			static_cast<int>(comp->format_type), comp->offset, comp->stride));
	}

	// Shader stages
	SCP_vector<vk::PipelineShaderStageCreateInfo> shaderStages;

	vk::PipelineShaderStageCreateInfo vertStage;
	vertStage.stage = vk::ShaderStageFlagBits::eVertex;
	vertStage.module = shader->vertexModule.get();
	vertStage.pName = "main";
	shaderStages.push_back(vertStage);

	vk::PipelineShaderStageCreateInfo fragStage;
	fragStage.stage = vk::ShaderStageFlagBits::eFragment;
	fragStage.module = shader->fragmentModule.get();
	fragStage.pName = "main";
	shaderStages.push_back(fragStage);

	// Vertex input state — filter out attributes the shader doesn't consume.
	// The vertex format cache may add fallback color/texcoord attributes that
	// shaders like NanoVG don't declare; the SPIR-V compiler strips unused
	// inputs, so we must match the pipeline to the actual shader inputs.
	VertexInputConfig vertexInputConfig = m_vertexFormatCache.getVertexInputConfig(vertexLayout);
	if (shader->vertexInputMask != 0) {
		uint32_t mask = shader->vertexInputMask;
		auto& attrs = vertexInputConfig.attributes;
		SCP_unordered_set<uint32_t> usedBindings;

		// Remove attributes at locations the shader doesn't use
		attrs.erase(std::remove_if(attrs.begin(), attrs.end(),
			[mask](const vk::VertexInputAttributeDescription& a) {
				return (mask & (1u << a.location)) == 0;
			}), attrs.end());

		// Collect bindings still referenced by remaining attributes
		for (auto& a : attrs) {
			usedBindings.insert(a.binding);
		}

		// Remove orphaned bindings
		auto& binds = vertexInputConfig.bindings;
		binds.erase(std::remove_if(binds.begin(), binds.end(),
			[&usedBindings](const vk::VertexInputBindingDescription& b) {
				return usedBindings.count(b.binding) == 0;
			}), binds.end());

		vertexInputConfig.updatePointers();
	}

	// Input assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = convertPrimitiveType(config.primitiveType);
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport state (dynamic)
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr;  // Dynamic
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;   // Dynamic

	// Rasterization state
	vk::PipelineRasterizationStateCreateInfo rasterizer = createRasterizationState(
		config.cullEnabled, config.fillMode, config.frontFaceCW, config.depthBiasEnabled);

	// Multisample state
	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling.rasterizationSamples = config.sampleCount;
	multisampling.sampleShadingEnable = VK_FALSE;

	// Depth stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil = createDepthStencilState(
		config.depthMode,
		config.stencilEnabled,
		config.stencilFunc,
		config.stencilEnabled ? &config.frontStencilOp : nullptr,
		config.stencilEnabled ? &config.backStencilOp : nullptr,
		config.stencilMask);

	// Override depth write if specified
	if (!config.depthWriteEnabled) {
		depthStencil.depthWriteEnable = VK_FALSE;
	}

	// Color blend state
	SCP_vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
	for (uint32_t i = 0; i < config.colorAttachmentCount; ++i) {
		if (config.perAttachmentBlendEnabled) {
			colorBlendAttachments.push_back(createColorBlendAttachment(
				config.attachmentBlends[i].blendMode, config.attachmentBlends[i].writeMask));
		} else {
			colorBlendAttachments.push_back(createColorBlendAttachment(config.blendMode, config.colorWriteMask));
		}
	}

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlending.pAttachments = colorBlendAttachments.data();

	// Dynamic state
	std::array<vk::DynamicState, 5> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::eLineWidth,
		vk::DynamicState::eDepthBias,
		vk::DynamicState::eStencilReference,
	};

	vk::PipelineDynamicStateCreateInfo dynamicState;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Create pipeline
	vk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputConfig.createInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout.get();
	pipelineInfo.renderPass = config.renderPass;
	pipelineInfo.subpass = config.subpass;
	pipelineInfo.basePipelineHandle = nullptr;
	pipelineInfo.basePipelineIndex = -1;

	try {
		auto result = m_device.createGraphicsPipelineUnique(m_pipelineCache.get(), pipelineInfo);
		return std::move(result.value);
	} catch (const vk::SystemError& e) {
		nprintf(("vulkan", "VulkanPipelineManager: Failed to create pipeline: %s\n", e.what()));
		return {};
	}
}

} // namespace graphics::vulkan

