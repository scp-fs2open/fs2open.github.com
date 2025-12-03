
#include "VulkanShader.h"

#ifdef WITH_VULKAN

#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "def_files/def_files.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "graphics/util/ShaderPreprocessor.h"
#include "mod_table/mod_table.h"
#include "options/Option.h"
#define MODEL_SDR_FLAG_MODE_CPP
#include "def_files/data/effects/model_shader_flags.h"
#include <md5.h>

#include <sstream>
#include <fstream>

#ifdef FSO_HAVE_SPIRV_CROSS
#ifdef FSO_SPIRV_CROSS_C_API
// Use C API for Windows shared library compatibility
#include <spirv_cross/spirv_cross_c.h>
#else
// Use C++ API directly for static linking
#include <spirv_cross/spirv_cross.hpp>
#endif
#endif

namespace graphics {
namespace vulkan {

// Shader type to filename mapping - mirrors GL_shader_types from gropenglshader.cpp
struct ShaderTypeInfo {
	shader_type type;
	const char* vert;
	const char* frag;
	const char* geo;  // nullptr if no geometry shader
};

static const ShaderTypeInfo SHADER_TYPE_INFO[] = {
	{ SDR_TYPE_MODEL, "main-v.sdr", "main-f.sdr", "main-g.sdr" },
	{ SDR_TYPE_EFFECT_PARTICLE, "effect-v.sdr", "effect-f.sdr", "effect-g.sdr" },
	{ SDR_TYPE_EFFECT_DISTORTION, "effect-distort-v.sdr", "effect-distort-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_MAIN, "post-v.sdr", "post-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_BLUR, "post-v.sdr", "blur-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_BLOOM_COMP, "post-v.sdr", "bloom-comp-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS, "post-v.sdr", "brightpass-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_FXAA, "fxaa-v.sdr", "fxaa-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS, "post-v.sdr", "fxaapre-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS, "post-v.sdr", "ls-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_TONEMAPPING, "post-v.sdr", "tonemapping-f.sdr", nullptr },
	{ SDR_TYPE_DEFERRED_LIGHTING, "deferred-v.sdr", "deferred-f.sdr", nullptr },
	{ SDR_TYPE_DEFERRED_CLEAR, "deferred-clear-v.sdr", "deferred-clear-f.sdr", nullptr },
	{ SDR_TYPE_VIDEO_PROCESS, "video-v.sdr", "video-f.sdr", nullptr },
	{ SDR_TYPE_PASSTHROUGH_RENDER, "passthrough-v.sdr", "passthrough-f.sdr", nullptr },
	{ SDR_TYPE_SHIELD_DECAL, "shield-impact-v.sdr", "shield-impact-f.sdr", nullptr },
	{ SDR_TYPE_BATCHED_BITMAP, "batched-v.sdr", "batched-f.sdr", nullptr },
	{ SDR_TYPE_DEFAULT_MATERIAL, "default-material.vert", "default-material.frag", nullptr },
	{ SDR_TYPE_NANOVG, "nanovg-v.sdr", "nanovg-f.sdr", nullptr },
	{ SDR_TYPE_DECAL, "decal-v.sdr", "decal-f.sdr", nullptr },
	{ SDR_TYPE_SCENE_FOG, "post-v.sdr", "fog-f.sdr", nullptr },
	{ SDR_TYPE_VOLUMETRIC_FOG, "post-v.sdr", "volumetric-f.sdr", nullptr },
	{ SDR_TYPE_ROCKET_UI, "rocketui-v.sdr", "rocketui-f.sdr", nullptr },
	{ SDR_TYPE_COPY, "post-v.sdr", "copy-f.sdr", nullptr },
	{ SDR_TYPE_COPY_WORLD, "passthrough-v.sdr", "copy-f.sdr", nullptr },
	{ SDR_TYPE_MSAA_RESOLVE, "post-v.sdr", "msaa-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_SMAA_EDGE, "smaa-edge-v.sdr", "smaa-edge-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_SMAA_BLENDING_WEIGHT, "smaa-blend-v.sdr", "smaa-blend-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_SMAA_NEIGHBORHOOD_BLENDING, "smaa-neighbour-v.sdr", "smaa-neighbour-f.sdr", nullptr },
	{ SDR_TYPE_ENVMAP_SPHERE_WARP, "post-v.sdr", "envmap-sphere-warp-f.sdr", nullptr },
	{ SDR_TYPE_IRRADIANCE_MAP_GEN, "post-v.sdr", "irrmap-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_TAA, "taa-v.sdr", "taa-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_SSAO, "post-v.sdr", "ssao-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_SSAO_BLUR, "post-v.sdr", "ssao-blur-f.sdr", nullptr },
	{ SDR_TYPE_POST_PROCESS_LUMINANCE, "post-v.sdr", "luminance-f.sdr", nullptr },
};

static const ShaderTypeInfo* findShaderTypeInfo(shader_type type) {
	for (const auto& info : SHADER_TYPE_INFO) {
		if (info.type == type) {
			return &info;
		}
	}
	return nullptr;
}

// ============================================================================
// VulkanShaderCache
// ============================================================================

VulkanShaderCache::VulkanShaderCache() = default;

bool VulkanShaderCache::initialize() {
	// Use same cache location as OpenGL shader cache
	m_cacheDir = "vulkan_shaders";
	m_initialized = true;

#ifdef FSO_HAVE_SHADERC
	// Get shaderc version for cache keying
	// The shaderc library doesn't expose a version function directly,
	// but we can use a hash of compile options as a proxy
	m_shadercVersion = 1;  // Increment this when shaderc is updated
#endif

	mprintf(("Vulkan shader cache initialized at: %s\n", m_cacheDir.c_str()));
	return true;
}

SCP_string VulkanShaderCache::computeCacheKey(const SCP_string& sourceHash, shader_type type, uint32_t flags, ShaderStage stage) {
	const char* stageStr;
	switch (stage) {
		case ShaderStage::Vertex: stageStr = "v"; break;
		case ShaderStage::Fragment: stageStr = "f"; break;
		case ShaderStage::Geometry: stageStr = "g"; break;
		default: stageStr = "x"; break;
	}

	char buffer[256];
	snprintf(buffer, sizeof(buffer), "vk_%d_%08x_%s_%s_%u",
		static_cast<int>(type), flags, stageStr, sourceHash.c_str(), m_shadercVersion);

	return SCP_string(buffer);
}

SCP_string VulkanShaderCache::getCacheFilePath(const SCP_string& cacheKey) {
	return cacheKey + ".spv";
}

bool VulkanShaderCache::hasCachedSpirv(const SCP_string& cacheKey) {
	if (!m_initialized || Cmdline_noshadercache) {
		return false;
	}

	auto filepath = getCacheFilePath(cacheKey);
	CFILE* cf = cfopen(filepath.c_str(), "rb", CF_TYPE_CACHE, false,
		CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	if (cf) {
		cfclose(cf);
		return true;
	}
	return false;
}

std::vector<uint32_t> VulkanShaderCache::loadCachedSpirv(const SCP_string& cacheKey) {
	if (!m_initialized || Cmdline_noshadercache) {
		return {};
	}

	auto filepath = getCacheFilePath(cacheKey);
	CFILE* cf = cfopen(filepath.c_str(), "rb", CF_TYPE_CACHE, false,
		CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	if (!cf) {
		return {};
	}

	int length = cfilelength(cf);
	if (length <= 0 || length % sizeof(uint32_t) != 0) {
		cfclose(cf);
		return {};
	}

	std::vector<uint32_t> spirv(length / sizeof(uint32_t));
	cfread(spirv.data(), 1, length, cf);
	cfclose(cf);

	nprintf(("VulkanShader", "Loaded cached SPIR-V: %s (%d bytes)\n", cacheKey.c_str(), length));
	return spirv;
}

void VulkanShaderCache::saveSpirvToCache(const SCP_string& cacheKey, const std::vector<uint32_t>& spirv) {
	if (!m_initialized || Cmdline_noshadercache || spirv.empty()) {
		return;
	}

	auto filepath = getCacheFilePath(cacheKey);
	CFILE* cf = cfopen(filepath.c_str(), "wb", CF_TYPE_CACHE, false,
		CF_LOCATION_ROOT_USER | CF_LOCATION_TYPE_ROOT);

	if (!cf) {
		mprintf(("VulkanShader: Failed to create cache file: %s\n", filepath.c_str()));
		return;
	}

	cfwrite(spirv.data(), sizeof(uint32_t), static_cast<int>(spirv.size()), cf);
	cfclose(cf);

	nprintf(("VulkanShader", "Saved SPIR-V to cache: %s (%zu words)\n", cacheKey.c_str(), spirv.size()));
}

// ============================================================================
// VulkanShaderCompiler
// ============================================================================

#ifdef FSO_HAVE_SHADERC

VulkanShaderCompiler::VulkanShaderCompiler() {
	// Set up compile options for Vulkan target
	m_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
	m_options.SetSourceLanguage(shaderc_source_language_glsl);
	m_options.SetOptimizationLevel(shaderc_optimization_level_performance);

	// Note: shaderc automatically defines VULKAN=1 when targeting Vulkan environment
	// Do not manually add it or you'll get "Macro redefined" errors
}

uint32_t VulkanShaderCompiler::getShadercVersion() const {
	// Return a version number - increment when shaderc is updated
	return 1;
}

shaderc_shader_kind VulkanShaderCompiler::stageToKind(ShaderStage stage) {
	switch (stage) {
		case ShaderStage::Vertex: return shaderc_vertex_shader;
		case ShaderStage::Fragment: return shaderc_fragment_shader;
		case ShaderStage::Geometry: return shaderc_geometry_shader;
		default: return shaderc_glsl_infer_from_source;
	}
}

std::vector<uint32_t> VulkanShaderCompiler::compileGlslToSpirv(const SCP_string& source, ShaderStage stage, const SCP_string& filename) {
	auto kind = stageToKind(stage);

	auto result = m_compiler.CompileGlslToSpv(source.c_str(), source.size(), kind, filename.c_str(), m_options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		mprintf(("VulkanShader: Compilation failed for %s:\n%s\n", filename.c_str(), result.GetErrorMessage().c_str()));
		return {};
	}

	if (result.GetNumWarnings() > 0) {
		nprintf(("VulkanShader", "Compilation warnings for %s:\n%s\n", filename.c_str(), result.GetErrorMessage().c_str()));
	}

	return std::vector<uint32_t>(result.cbegin(), result.cend());
}

#endif // FSO_HAVE_SHADERC

// ============================================================================
// VulkanDescriptorSetLayoutBuilder
// ============================================================================

VulkanDescriptorSetLayoutBuilder::VulkanDescriptorSetLayoutBuilder(vk::Device device)
	: m_device(device) {
}

DescriptorSetLayouts VulkanDescriptorSetLayoutBuilder::build(const ShaderReflectionData& reflectionData) {
	DescriptorSetLayouts result;

	if (reflectionData.descriptors.empty() && reflectionData.pushConstants.empty()) {
		// No resources to bind - return empty but valid layouts
		return result;
	}

	// Group bindings by descriptor set number
	std::map<uint32_t, SCP_vector<vk::DescriptorSetLayoutBinding>> bindingsBySet;

	for (const auto& descriptor : reflectionData.descriptors) {
		vk::DescriptorSetLayoutBinding binding;
		binding.binding = descriptor.binding;
		binding.descriptorType = descriptor.type;
		binding.descriptorCount = descriptor.count;
		binding.stageFlags = descriptor.stageFlags;
		binding.pImmutableSamplers = nullptr;

		bindingsBySet[descriptor.set].push_back(binding);

		nprintf(("VulkanDescriptor", "Set %u Binding %u: %s (%s, count=%u)\n",
			descriptor.set, descriptor.binding, descriptor.name.c_str(),
			vk::to_string(descriptor.type).c_str(), descriptor.count));
	}

	// Create descriptor set layouts for each set (including empty sets to maintain indexing)
	uint32_t maxSet = reflectionData.maxDescriptorSet;
	result.layouts.resize(maxSet + 1);

	for (uint32_t setIndex = 0; setIndex <= maxSet; ++setIndex) {
		vk::DescriptorSetLayoutCreateInfo createInfo;

		auto it = bindingsBySet.find(setIndex);
		if (it != bindingsBySet.end() && !it->second.empty()) {
			// Sort bindings by binding number for consistency
			auto& bindings = it->second;
			std::sort(bindings.begin(), bindings.end(),
				[](const auto& a, const auto& b) { return a.binding < b.binding; });

			createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			createInfo.pBindings = bindings.data();
		} else {
			// Empty set layout
			createInfo.bindingCount = 0;
			createInfo.pBindings = nullptr;
		}

		try {
			result.layouts[setIndex] = m_device.createDescriptorSetLayoutUnique(createInfo);
			nprintf(("VulkanDescriptor", "Created descriptor set layout for set %u with %u bindings\n",
				setIndex, createInfo.bindingCount));
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanDescriptorSetLayoutBuilder: Failed to create layout for set %u: %s\n",
				setIndex, e.what()));
			result.layouts.clear();
			return result;
		}
	}

	// Convert push constants
	for (const auto& pc : reflectionData.pushConstants) {
		vk::PushConstantRange range;
		range.stageFlags = pc.stageFlags;
		range.offset = pc.offset;
		range.size = pc.size;
		result.pushConstantRanges.push_back(range);

		nprintf(("VulkanDescriptor", "Push constant range: offset=%u, size=%u\n", pc.offset, pc.size));
	}

	return result;
}

vk::UniquePipelineLayout VulkanDescriptorSetLayoutBuilder::createPipelineLayout(const DescriptorSetLayouts& layouts) {
	// Convert UniqueDescriptorSetLayout to raw handles
	SCP_vector<vk::DescriptorSetLayout> rawLayouts;
	rawLayouts.reserve(layouts.layouts.size());
	for (const auto& layout : layouts.layouts) {
		rawLayouts.push_back(layout.get());
	}

	vk::PipelineLayoutCreateInfo createInfo;
	createInfo.setLayoutCount = static_cast<uint32_t>(rawLayouts.size());
	createInfo.pSetLayouts = rawLayouts.empty() ? nullptr : rawLayouts.data();
	createInfo.pushConstantRangeCount = static_cast<uint32_t>(layouts.pushConstantRanges.size());
	createInfo.pPushConstantRanges = layouts.pushConstantRanges.empty() ? nullptr : layouts.pushConstantRanges.data();

	try {
		auto pipelineLayout = m_device.createPipelineLayoutUnique(createInfo);
		nprintf(("VulkanDescriptor", "Created pipeline layout with %zu set layouts and %zu push constant ranges\n",
			rawLayouts.size(), layouts.pushConstantRanges.size()));
		return pipelineLayout;
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanDescriptorSetLayoutBuilder: Failed to create pipeline layout: %s\n", e.what()));
		return vk::UniquePipelineLayout();
	}
}

// ============================================================================
// VulkanShaderManager
// ============================================================================

VulkanShaderManager::VulkanShaderManager(vk::Device device)
	: m_device(device) {
}

VulkanShaderManager::~VulkanShaderManager() {
	shutdown();
}

bool VulkanShaderManager::initialize() {
	if (!m_cache.initialize()) {
		mprintf(("VulkanShaderManager: Failed to initialize shader cache\n"));
		// Continue anyway - we can still compile without caching
	}

	mprintf(("VulkanShaderManager initialized\n"));
	return true;
}

void VulkanShaderManager::shutdown() {
	// Clear all shader modules (they're UniqueShaderModule so will auto-destroy)
	m_shaderModules.clear();
}

SCP_string VulkanShaderManager::makeShaderKey(shader_type type, uint32_t flags, ShaderStage stage) {
	const char* stageStr;
	switch (stage) {
		case ShaderStage::Vertex: stageStr = "v"; break;
		case ShaderStage::Fragment: stageStr = "f"; break;
		case ShaderStage::Geometry: stageStr = "g"; break;
		default: stageStr = "x"; break;
	}

	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%d_%08x_%s", static_cast<int>(type), flags, stageStr);
	return SCP_string(buffer);
}

vk::ShaderModule VulkanShaderManager::getShader(shader_type type, uint32_t flags, ShaderStage stage) {
	auto key = makeShaderKey(type, flags, stage);

	// Check in-memory cache first
	auto it = m_shaderModules.find(key);
	if (it != m_shaderModules.end()) {
		return it->second.get();
	}

	// Need to compile/load this shader
	std::vector<uint32_t> spirv;

#ifdef FSO_HAVE_SHADERC
	// Preprocess the shader
	auto source = preprocessShader(type, flags, stage);
	if (source.empty()) {
		mprintf(("VulkanShaderManager: Failed to preprocess shader type=%d flags=0x%x stage=%d\n",
			static_cast<int>(type), flags, static_cast<int>(stage)));
		return nullptr;
	}

	// Compute source hash for caching
	auto sourceHash = computeSourceHash(source);
	auto cacheKey = m_cache.computeCacheKey(sourceHash, type, flags, stage);

	// Try to load from disk cache
	spirv = m_cache.loadCachedSpirv(cacheKey);

	if (spirv.empty()) {
		// Cache miss - compile with shaderc
		auto filename = getShaderFilename(type, stage);
		nprintf(("VulkanShader", "Compiling shader: %s (type=%d, flags=0x%x)\n", filename, static_cast<int>(type), flags));

		spirv = m_compiler.compileGlslToSpirv(source, stage, filename ? filename : "unknown");

		if (!spirv.empty()) {
			// Save to cache for next time
			m_cache.saveSpirvToCache(cacheKey, spirv);
		}
	}
#else
	mprintf(("VulkanShaderManager: shaderc not available, cannot compile shader\n"));
#endif

	if (spirv.empty()) {
		mprintf(("VulkanShaderManager: Failed to get SPIR-V for shader type=%d flags=0x%x stage=%d\n",
			static_cast<int>(type), flags, static_cast<int>(stage)));
		return nullptr;
	}

	// Create the shader module
	auto module = createShaderModule(spirv);
	if (!module) {
		mprintf(("VulkanShaderManager: Failed to create shader module\n"));
		return nullptr;
	}

	auto result = module.get();
	m_shaderModules[key] = std::move(module);
	return result;
}

vk::ShaderModule VulkanShaderManager::getVertexShader(shader_type type, uint32_t flags) {
	return getShader(type, flags, ShaderStage::Vertex);
}

vk::ShaderModule VulkanShaderManager::getFragmentShader(shader_type type, uint32_t flags) {
	return getShader(type, flags, ShaderStage::Fragment);
}

vk::ShaderModule VulkanShaderManager::getGeometryShader(shader_type type, uint32_t flags) {
	if (!requiresGeometryShader(type, flags)) {
		return nullptr;
	}
	return getShader(type, flags, ShaderStage::Geometry);
}

bool VulkanShaderManager::requiresGeometryShader(shader_type type, uint32_t flags) {
	auto info = findShaderTypeInfo(type);
	if (!info || !info->geo) {
		return false;
	}

	// Check if any variant requires geometry shader
	// TODO: This needs to check the specific flags like OpenGL does
	// For now, assume geometry shader is needed if it exists and MODEL_SDR_FLAG_SHADOW_MAP is set
	if (type == SDR_TYPE_MODEL && (flags & MODEL_SDR_FLAG_SHADOW_MAP)) {
		return true;
	}
	if (type == SDR_TYPE_EFFECT_PARTICLE && (flags & SDR_FLAG_PARTICLE_POINT_GEN)) {
		return true;
	}

	return false;
}

const char* VulkanShaderManager::getShaderFilename(shader_type type, ShaderStage stage) {
	auto info = findShaderTypeInfo(type);
	if (!info) {
		return nullptr;
	}

	switch (stage) {
		case ShaderStage::Vertex: return info->vert;
		case ShaderStage::Fragment: return info->frag;
		case ShaderStage::Geometry: return info->geo;
		default: return nullptr;
	}
}

SCP_string VulkanShaderManager::loadShaderSource(const char* filename) {
	SCP_string content;

	// Try to load from external file first (for modding)
	if (Enable_external_shaders) {
		CFILE* cf = cfopen(filename, "rt", CF_TYPE_EFFECTS);
		if (cf) {
			int len = cfilelength(cf);
			content.resize(len);
			cfread(&content[0], 1, len, cf);
			cfclose(cf);
			return content;
		}
	}

	// Fall back to embedded defaults
	auto def_file = defaults_get_file(filename);
	if (def_file.data && def_file.size > 0) {
		content.assign(static_cast<const char*>(def_file.data), def_file.size);
	}

	return content;
}

SCP_string VulkanShaderManager::generateShaderHeader(shader_type type, uint32_t flags, bool hasGeometryShader) {
	SCP_stringstream header;

	// Use GLSL 450 for Vulkan (SPIR-V compatible)
	header << "#version 450 core\n";
	header << "#ifndef VULKAN\n#define VULKAN 1\n#endif\n";

	// Vulkan-specific extensions
	header << "#extension GL_ARB_separate_shader_objects : enable\n";

	// Lighting model
	if (Detail.lighting < 3) {
		header << "#define FLAG_LIGHT_MODEL_BLINN_PHONG\n";
	}

	if (hasGeometryShader) {
		header << "#define HAS_GEOMETRY_SHADER\n";
	}

	// Add variant flags as defines
	// This mirrors the OpenGL behavior from opengl_shader_get_header()
	// The shaders check #ifdef MODEL_SDR_FLAG_XXX, so output the full flag name

	// Model shader flags - from model_shader_flags.h
	if (type == SDR_TYPE_MODEL) {
		if (flags & MODEL_SDR_FLAG_LIGHT) header << "#define MODEL_SDR_FLAG_LIGHT\n";
		if (flags & MODEL_SDR_FLAG_DEFERRED) header << "#define MODEL_SDR_FLAG_DEFERRED\n";
		if (flags & MODEL_SDR_FLAG_HDR) header << "#define MODEL_SDR_FLAG_HDR\n";
		if (flags & MODEL_SDR_FLAG_DIFFUSE) header << "#define MODEL_SDR_FLAG_DIFFUSE\n";
		if (flags & MODEL_SDR_FLAG_GLOW) header << "#define MODEL_SDR_FLAG_GLOW\n";
		if (flags & MODEL_SDR_FLAG_SPEC) header << "#define MODEL_SDR_FLAG_SPEC\n";
		if (flags & MODEL_SDR_FLAG_NORMAL) header << "#define MODEL_SDR_FLAG_NORMAL\n";
		if (flags & MODEL_SDR_FLAG_AMBIENT) header << "#define MODEL_SDR_FLAG_AMBIENT\n";
		if (flags & MODEL_SDR_FLAG_MISC) header << "#define MODEL_SDR_FLAG_MISC\n";
		if (flags & MODEL_SDR_FLAG_TEAMCOLOR) header << "#define MODEL_SDR_FLAG_TEAMCOLOR\n";
		if (flags & MODEL_SDR_FLAG_FOG) header << "#define MODEL_SDR_FLAG_FOG\n";
		if (flags & MODEL_SDR_FLAG_TRANSFORM) header << "#define MODEL_SDR_FLAG_TRANSFORM\n";
		if (flags & MODEL_SDR_FLAG_SHADOWS) header << "#define MODEL_SDR_FLAG_SHADOWS\n";
		if (flags & MODEL_SDR_FLAG_THRUSTER) header << "#define MODEL_SDR_FLAG_THRUSTER\n";
		if (flags & MODEL_SDR_FLAG_ALPHA_MULT) header << "#define MODEL_SDR_FLAG_ALPHA_MULT\n";
		if (flags & MODEL_SDR_FLAG_SHADOW_MAP) header << "#define MODEL_SDR_FLAG_SHADOW_MAP\n";
		if (flags & MODEL_SDR_FLAG_THICK_OUTLINES) header << "#define MODEL_SDR_FLAG_THICK_OUTLINES\n";
	}

	// Particle effect flags
	if (type == SDR_TYPE_EFFECT_PARTICLE) {
		if (flags & SDR_FLAG_PARTICLE_POINT_GEN) header << "#define FLAG_EFFECT_GEOMETRY\n";
	}

	// Blur direction flags - the shader uses PASS_0/PASS_1
	if (type == SDR_TYPE_POST_PROCESS_BLUR) {
		if (flags & SDR_FLAG_BLUR_HORIZONTAL) header << "#define PASS_0\n";
		if (flags & SDR_FLAG_BLUR_VERTICAL) header << "#define PASS_1\n";
	}

	return header.str();
}

SCP_string VulkanShaderManager::handleIncludes(const char* filename, const SCP_string& source) {
	// Create a preprocessor with Vulkan-specific callbacks
	graphics::ShaderPreprocessorCallbacks callbacks;

	// Capture 'this' to use loadShaderSource
	callbacks.loadSource = [this](const char* fname) { return loadShaderSource(fname); };

	// Check capabilities via gr_is_capable to determine which shader variants to use
	// This ensures shaders use the correct code paths (e.g., LARGE_SHADER vs small variants)
	callbacks.checkCapability = [](const SCP_string& capabilityName) {
		auto capability = std::find_if(&gr_capabilities[0], &gr_capabilities[gr_capabilities_num],
		                               [&capabilityName](const gr_capability_def& cap_def) {
			                               return !stricmp(cap_def.parse_name, capabilityName.c_str());
		                               });

		if (capability == &gr_capabilities[gr_capabilities_num]) {
			// Capability not found - treat as unsupported
			mprintf(("VulkanShaderManager: Unknown capability '%s', defaulting to false\n", capabilityName.c_str()));
			return false;
		}

		bool supported = gr_is_capable(capability->capability);
		mprintf(("VulkanShaderManager: Capability '%s' is %s\n", capabilityName.c_str(), supported ? "supported" : "not supported"));
		return supported;
	};

	graphics::ShaderPreprocessor preprocessor(std::move(callbacks));
	return preprocessor.handleIncludes(filename, source);
}

SCP_string VulkanShaderManager::handlePredefines(const char* filename, const SCP_string& source) {
	// Create a preprocessor with Vulkan-specific callbacks
	graphics::ShaderPreprocessorCallbacks callbacks;

	callbacks.loadSource = [this](const char* fname) { return loadShaderSource(fname); };
	callbacks.checkCapability = nullptr; // Not needed for predefines

	graphics::ShaderPreprocessor preprocessor(std::move(callbacks));
	return preprocessor.handlePredefines(filename, source);
}

SCP_string VulkanShaderManager::preprocessShader(shader_type type, uint32_t flags, ShaderStage stage) {
	auto filename = getShaderFilename(type, stage);
	if (!filename) {
		return "";
	}
	mprintf(("VulkanShaderManager: preprocess type=%d flags=0x%x stage=%d file=%s\n",
		static_cast<int>(type), flags, static_cast<int>(stage), filename));

	// Load raw shader source
	auto source = loadShaderSource(filename);
	if (source.empty()) {
		mprintf(("VulkanShaderManager: Failed to load shader: %s\n", filename));
		return "";
	}

	// Check if shader already has an active #version directive (Vulkan-native shader)
	// Ignore commented lines like "//? #version 150" which are only hints for OpenGL
	auto containsActiveVersion = [](const SCP_string& text) {
		size_t searchPos = text.find("#version");
		while (searchPos != SCP_string::npos) {
			// Check for preceding line comment on the same line
			size_t lineStart = text.rfind('\n', searchPos);
			lineStart = (lineStart == SCP_string::npos) ? 0 : lineStart + 1;
			auto lineCommentPos = text.find("//", lineStart);
			bool inLineComment = (lineCommentPos != SCP_string::npos && lineCommentPos < searchPos);

			// Check if inside an open block comment
			bool inBlockComment = false;
			size_t blockStart = text.rfind("/*", searchPos);
			if (blockStart != SCP_string::npos) {
				size_t blockEnd = text.rfind("*/", searchPos);
				if (blockEnd == SCP_string::npos || blockEnd < blockStart) {
					inBlockComment = true;
				}
			}

			if (!inLineComment && !inBlockComment) {
				return true;
			}

			searchPos = text.find("#version", searchPos + 8);
		}
		return false;
	};

	bool hasVersion = containsActiveVersion(source);
	if (hasVersion) {
		// Vulkan-native shader - process includes and predefines but skip header generation
		nprintf(("VulkanShader", "Shader %s has #version, processing includes/predefines only\n", filename));
		auto processed = handleIncludes(filename, source);
		return handlePredefines(filename, processed);
	}

	// Legacy FSO shader - needs header and full preprocessing
	// Determine if we need geometry shader
	bool hasGeo = requiresGeometryShader(type, flags);

	// Generate header with version and defines
	auto header = generateShaderHeader(type, flags, hasGeo);

	// Process includes
	auto processed = handleIncludes(filename, source);

	// Process predefines
	processed = handlePredefines(filename, processed);

	// Combine header and processed source
	return header + processed;
}

SCP_string VulkanShaderManager::computeSourceHash(const SCP_string& source) {
	MD5 md5;
	md5.update(source.c_str(), static_cast<MD5::size_type>(source.size()));
	md5.finalize();
	return md5.hexdigest();
}

vk::UniqueShaderModule VulkanShaderManager::createShaderModule(const std::vector<uint32_t>& spirv) {
	vk::ShaderModuleCreateInfo createInfo;
	createInfo.codeSize = spirv.size() * sizeof(uint32_t);
	createInfo.pCode = spirv.data();

	try {
		return m_device.createShaderModuleUnique(createInfo);
	}
	catch (const vk::SystemError& e) {
		mprintf(("VulkanShaderManager: Failed to create shader module: %s\n", e.what()));
		return vk::UniqueShaderModule{};
	}
}

// ============================================================================
// VulkanShaderReflection
// ============================================================================

#ifdef FSO_HAVE_SPIRV_CROSS

namespace {

vk::ShaderStageFlags stageToVkFlags(ShaderStage stage) {
	switch (stage) {
		case ShaderStage::Vertex: return vk::ShaderStageFlagBits::eVertex;
		case ShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
		case ShaderStage::Geometry: return vk::ShaderStageFlagBits::eGeometry;
		default: return {};
	}
}

#ifdef FSO_SPIRV_CROSS_C_API
// Helper to convert spvc_resource_type to vk::DescriptorType
vk::DescriptorType resourceTypeToDescriptorType(spvc_resource_type type) {
	switch (type) {
		case SPVC_RESOURCE_TYPE_UNIFORM_BUFFER: return vk::DescriptorType::eUniformBuffer;
		case SPVC_RESOURCE_TYPE_STORAGE_BUFFER: return vk::DescriptorType::eStorageBuffer;
		case SPVC_RESOURCE_TYPE_SAMPLED_IMAGE: return vk::DescriptorType::eCombinedImageSampler;
		case SPVC_RESOURCE_TYPE_SEPARATE_IMAGE: return vk::DescriptorType::eSampledImage;
		case SPVC_RESOURCE_TYPE_SEPARATE_SAMPLERS: return vk::DescriptorType::eSampler;
		case SPVC_RESOURCE_TYPE_STORAGE_IMAGE: return vk::DescriptorType::eStorageImage;
		default: return vk::DescriptorType::eUniformBuffer;
	}
}

// Helper to process resources of a specific type using C API
void processResourcesC(spvc_compiler compiler, spvc_resources resources, spvc_resource_type resType,
                       vk::ShaderStageFlags stageFlags, ShaderReflectionData& result) {
	const spvc_reflected_resource* list = nullptr;
	size_t count = 0;

	if (spvc_resources_get_resource_list_for_type(resources, resType, &list, &count) != SPVC_SUCCESS) {
		return;
	}

	for (size_t i = 0; i < count; ++i) {
		DescriptorBindingInfo info;
		info.set = spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationDescriptorSet);
		info.binding = spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding);
		info.type = resourceTypeToDescriptorType(resType);
		info.stageFlags = stageFlags;
		info.name = list[i].name ? list[i].name : "";
		info.count = 1;

		// Get size for buffers
		if (resType == SPVC_RESOURCE_TYPE_UNIFORM_BUFFER || resType == SPVC_RESOURCE_TYPE_STORAGE_BUFFER) {
			spvc_type type = spvc_compiler_get_type_handle(compiler, list[i].base_type_id);
			if (type) {
				size_t structSize = 0;
				if (spvc_compiler_get_declared_struct_size(compiler, type, &structSize) == SPVC_SUCCESS) {
					info.size = structSize;
				}
			}
		}

		result.descriptors.push_back(info);
		result.maxDescriptorSet = std::max(result.maxDescriptorSet, info.set);

		nprintf(("VulkanReflection", "Resource: %s (set=%u, binding=%u)\n",
			info.name.c_str(), info.set, info.binding));
	}
}
#endif

} // anonymous namespace

ShaderReflectionData VulkanShaderReflection::reflect(const std::vector<uint32_t>& spirv, ShaderStage stage) {
	ShaderReflectionData result;

	if (spirv.empty()) {
		return result;
	}

	auto stageFlags = stageToVkFlags(stage);

#ifdef FSO_SPIRV_CROSS_C_API
	// Use C API for Windows shared library
	spvc_context context = nullptr;
	spvc_parsed_ir ir = nullptr;
	spvc_compiler compiler = nullptr;
	spvc_resources resources = nullptr;

	if (spvc_context_create(&context) != SPVC_SUCCESS) {
		mprintf(("VulkanShaderReflection: Failed to create SPIRV-Cross context\n"));
		return result;
	}

	// Parse SPIR-V
	if (spvc_context_parse_spirv(context, spirv.data(), spirv.size(), &ir) != SPVC_SUCCESS) {
		mprintf(("VulkanShaderReflection: Failed to parse SPIR-V\n"));
		spvc_context_destroy(context);
		return result;
	}

	// Create compiler (use NONE backend for reflection only)
	if (spvc_context_create_compiler(context, SPVC_BACKEND_NONE, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler) != SPVC_SUCCESS) {
		mprintf(("VulkanShaderReflection: Failed to create SPIRV-Cross compiler\n"));
		spvc_context_destroy(context);
		return result;
	}

	// Get shader resources
	if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
		mprintf(("VulkanShaderReflection: Failed to get shader resources\n"));
		spvc_context_destroy(context);
		return result;
	}

	// Process each resource type
	processResourcesC(compiler, resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, stageFlags, result);
	processResourcesC(compiler, resources, SPVC_RESOURCE_TYPE_STORAGE_BUFFER, stageFlags, result);
	processResourcesC(compiler, resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, stageFlags, result);
	processResourcesC(compiler, resources, SPVC_RESOURCE_TYPE_SEPARATE_IMAGE, stageFlags, result);
	processResourcesC(compiler, resources, SPVC_RESOURCE_TYPE_SEPARATE_SAMPLERS, stageFlags, result);
	processResourcesC(compiler, resources, SPVC_RESOURCE_TYPE_STORAGE_IMAGE, stageFlags, result);

	// Process push constants
	const spvc_reflected_resource* pcList = nullptr;
	size_t pcCount = 0;
	if (spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_PUSH_CONSTANT, &pcList, &pcCount) == SPVC_SUCCESS) {
		for (size_t i = 0; i < pcCount; ++i) {
			PushConstantInfo info;
			info.stageFlags = stageFlags;
			info.offset = 0;

			spvc_type type = spvc_compiler_get_type_handle(compiler, pcList[i].base_type_id);
			if (type) {
				size_t structSize = 0;
				if (spvc_compiler_get_declared_struct_size(compiler, type, &structSize) == SPVC_SUCCESS) {
					info.size = static_cast<uint32_t>(structSize);
				}
			}

			result.pushConstants.push_back(info);
			nprintf(("VulkanReflection", "Push Constant: %s (size=%u)\n",
				pcList[i].name ? pcList[i].name : "unnamed", info.size));
		}
	}

	spvc_context_destroy(context);

#else
	// Use C++ API for non-Windows builds
	try {
		spirv_cross::Compiler compiler(spirv);
		auto resources = compiler.get_shader_resources();

		// Process uniform buffers
		for (const auto& ubo : resources.uniform_buffers) {
			DescriptorBindingInfo info;
			info.set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
			info.binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
			info.type = vk::DescriptorType::eUniformBuffer;
			info.count = 1;
			info.stageFlags = stageFlags;
			info.name = ubo.name;

			const auto& type = compiler.get_type(ubo.base_type_id);
			info.size = compiler.get_declared_struct_size(type);

			result.descriptors.push_back(info);
			result.maxDescriptorSet = std::max(result.maxDescriptorSet, info.set);

			nprintf(("VulkanReflection", "UBO: %s (set=%u, binding=%u, size=%zu)\n",
				info.name.c_str(), info.set, info.binding, info.size));
		}

		// Process storage buffers (SSBOs)
		for (const auto& ssbo : resources.storage_buffers) {
			DescriptorBindingInfo info;
			info.set = compiler.get_decoration(ssbo.id, spv::DecorationDescriptorSet);
			info.binding = compiler.get_decoration(ssbo.id, spv::DecorationBinding);
			info.type = vk::DescriptorType::eStorageBuffer;
			info.count = 1;
			info.stageFlags = stageFlags;
			info.name = ssbo.name;

			const auto& type = compiler.get_type(ssbo.base_type_id);
			info.size = compiler.get_declared_struct_size(type);

			result.descriptors.push_back(info);
			result.maxDescriptorSet = std::max(result.maxDescriptorSet, info.set);

			nprintf(("VulkanReflection", "SSBO: %s (set=%u, binding=%u, size=%zu)\n",
				info.name.c_str(), info.set, info.binding, info.size));
		}

		// Process sampled images (combined image samplers in GLSL)
		for (const auto& image : resources.sampled_images) {
			DescriptorBindingInfo info;
			info.set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			info.binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			info.type = vk::DescriptorType::eCombinedImageSampler;
			info.stageFlags = stageFlags;
			info.name = image.name;

			const auto& type = compiler.get_type(image.type_id);
			if (!type.array.empty()) {
				info.count = type.array[0];
			} else {
				info.count = 1;
			}

			result.descriptors.push_back(info);
			result.maxDescriptorSet = std::max(result.maxDescriptorSet, info.set);

			nprintf(("VulkanReflection", "Sampler: %s (set=%u, binding=%u, count=%u)\n",
				info.name.c_str(), info.set, info.binding, info.count));
		}

		// Process separate images (texture2D without sampler)
		for (const auto& image : resources.separate_images) {
			DescriptorBindingInfo info;
			info.set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			info.binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			info.type = vk::DescriptorType::eSampledImage;
			info.stageFlags = stageFlags;
			info.name = image.name;

			const auto& type = compiler.get_type(image.type_id);
			if (!type.array.empty()) {
				info.count = type.array[0];
			} else {
				info.count = 1;
			}

			result.descriptors.push_back(info);
			result.maxDescriptorSet = std::max(result.maxDescriptorSet, info.set);

			nprintf(("VulkanReflection", "Image: %s (set=%u, binding=%u, count=%u)\n",
				info.name.c_str(), info.set, info.binding, info.count));
		}

		// Process separate samplers
		for (const auto& sampler : resources.separate_samplers) {
			DescriptorBindingInfo info;
			info.set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
			info.binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
			info.type = vk::DescriptorType::eSampler;
			info.stageFlags = stageFlags;
			info.name = sampler.name;
			info.count = 1;

			result.descriptors.push_back(info);
			result.maxDescriptorSet = std::max(result.maxDescriptorSet, info.set);

			nprintf(("VulkanReflection", "Sampler: %s (set=%u, binding=%u)\n",
				info.name.c_str(), info.set, info.binding));
		}

		// Process storage images (imageLoad/imageStore)
		for (const auto& image : resources.storage_images) {
			DescriptorBindingInfo info;
			info.set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
			info.binding = compiler.get_decoration(image.id, spv::DecorationBinding);
			info.type = vk::DescriptorType::eStorageImage;
			info.stageFlags = stageFlags;
			info.name = image.name;

			const auto& type = compiler.get_type(image.type_id);
			if (!type.array.empty()) {
				info.count = type.array[0];
			} else {
				info.count = 1;
			}

			result.descriptors.push_back(info);
			result.maxDescriptorSet = std::max(result.maxDescriptorSet, info.set);

			nprintf(("VulkanReflection", "Storage Image: %s (set=%u, binding=%u, count=%u)\n",
				info.name.c_str(), info.set, info.binding, info.count));
		}

		// Process push constants
		for (const auto& pc : resources.push_constant_buffers) {
			PushConstantInfo info;
			info.stageFlags = stageFlags;
			info.offset = 0;

			const auto& type = compiler.get_type(pc.base_type_id);
			info.size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

			result.pushConstants.push_back(info);

			nprintf(("VulkanReflection", "Push Constant: %s (size=%u)\n", pc.name.c_str(), info.size));
		}
	}
	catch (const spirv_cross::CompilerError& e) {
		mprintf(("VulkanShaderReflection: SPIRV-Cross error: %s\n", e.what()));
		return ShaderReflectionData{};
	}
#endif

	return result;
}

ShaderReflectionData VulkanShaderReflection::mergeStages(const SCP_vector<ShaderReflectionData>& stages) {
	ShaderReflectionData result;

	// Merge descriptors, combining stage flags for same set/binding
	std::map<std::pair<uint32_t, uint32_t>, DescriptorBindingInfo> mergedDescriptors;

	for (const auto& stage : stages) {
		for (const auto& desc : stage.descriptors) {
			auto key = std::make_pair(desc.set, desc.binding);
			auto it = mergedDescriptors.find(key);

			if (it != mergedDescriptors.end()) {
				// Merge stage flags
				it->second.stageFlags |= desc.stageFlags;
				// Verify type matches
				if (it->second.type != desc.type) {
					mprintf(("VulkanShaderReflection: Type mismatch at set=%u binding=%u\n", desc.set, desc.binding));
				}
			} else {
				mergedDescriptors[key] = desc;
			}
		}

		result.maxDescriptorSet = std::max(result.maxDescriptorSet, stage.maxDescriptorSet);
	}

	// Convert map back to vector
	for (const auto& kv : mergedDescriptors) {
		result.descriptors.push_back(kv.second);
	}

	// Merge push constants (combine ranges and flags)
	std::map<uint32_t, PushConstantInfo> mergedPushConstants;

	for (const auto& stage : stages) {
		for (const auto& pc : stage.pushConstants) {
			auto it = mergedPushConstants.find(pc.offset);
			if (it != mergedPushConstants.end()) {
				it->second.stageFlags |= pc.stageFlags;
				it->second.size = std::max(it->second.size, pc.size);
			} else {
				mergedPushConstants[pc.offset] = pc;
			}
		}
	}

	for (const auto& kv : mergedPushConstants) {
		result.pushConstants.push_back(kv.second);
	}

	return result;
}

#endif // FSO_HAVE_SPIRV_CROSS

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
