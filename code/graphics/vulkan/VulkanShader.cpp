#include "VulkanShader.h"
#include "VulkanVertexFormat.h"

#include "def_files/def_files.h"

namespace graphics {
namespace vulkan {

// Global shader manager pointer
static VulkanShaderManager* g_shaderManager = nullptr;

VulkanShaderManager* getShaderManager()
{
	Assertion(g_shaderManager != nullptr, "Vulkan ShaderManager not initialized!");
	return g_shaderManager;
}

void setShaderManager(VulkanShaderManager* manager)
{
	g_shaderManager = manager;
}

// ========== gr_screen function pointer implementations ==========

int vulkan_maybe_create_shader(shader_type shader_t, unsigned int flags)
{
	auto* shaderManager = getShaderManager();
	return shaderManager->maybeCreateShader(shader_t, flags);
}

void vulkan_recompile_all_shaders(const std::function<void(size_t, size_t)>& progressCallback)
{
	auto* shaderManager = getShaderManager();
	shaderManager->recompileAllShaders(progressCallback);
}

// Shader type definitions - maps shader_type to SPIR-V filenames
// Vertex input location bits
// Values reflect what actually survives SPIR-V dead-code elimination, not just what's
// declared in GLSL. Used to filter out fallback vertex attributes the shader doesn't consume.
static constexpr uint32_t VTX_POSITION = 1u << static_cast<uint32_t>(VertexAttributeLocation::Position);
static constexpr uint32_t VTX_COLOR    = 1u << static_cast<uint32_t>(VertexAttributeLocation::Color);
static constexpr uint32_t VTX_TEXCOORD = 1u << static_cast<uint32_t>(VertexAttributeLocation::TexCoord);
static constexpr uint32_t VTX_NORMAL   = 1u << static_cast<uint32_t>(VertexAttributeLocation::Normal);
static constexpr uint32_t VTX_TANGENT  = 1u << static_cast<uint32_t>(VertexAttributeLocation::Tangent);
static constexpr uint32_t VTX_MODELID  = 1u << static_cast<uint32_t>(VertexAttributeLocation::ModelId);
static constexpr uint32_t VTX_RADIUS   = 1u << static_cast<uint32_t>(VertexAttributeLocation::Radius);
static constexpr uint32_t VTX_UVEC     = 1u << static_cast<uint32_t>(VertexAttributeLocation::Uvec);
static constexpr uint32_t VTX_MATRIX   = (15u << static_cast<uint32_t>(VertexAttributeLocation::ModelMatrix)); // Four consecutive locations
static constexpr uint32_t VTX_NONE     = 0;

// Filenames match the compiled SPIR-V files: {basename}.{stage}.spv
const VulkanShaderTypeInfo VULKAN_SHADER_TYPES[] = {
	{ SDR_TYPE_MODEL,                              "main",           "main",           "Model rendering",              VTX_POSITION | VTX_TEXCOORD | VTX_NORMAL | VTX_TANGENT | VTX_MODELID },
	{ SDR_TYPE_EFFECT_PARTICLE,                    "effect",         "effect",         "Particle effects",             VTX_POSITION | VTX_COLOR | VTX_TEXCOORD | VTX_RADIUS },
	{ SDR_TYPE_EFFECT_DISTORTION,                  "effect-distort", "effect-distort", "Distortion effects",           VTX_POSITION | VTX_COLOR | VTX_TEXCOORD | VTX_RADIUS },
	{ SDR_TYPE_POST_PROCESS_MAIN,                  "postprocess",    "post",           "Post-processing main",         VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_BLUR,                  "postprocess",    "blur",           "Gaussian blur",                VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_BLOOM_COMP,            "postprocess",    "bloom-comp",     "Bloom composition",            VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS,            "postprocess",    "brightpass",     "Bright pass filter",           VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_FXAA,                  "postprocess",    "fxaa",           "FXAA anti-aliasing",           VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS,          "postprocess",    "fxaapre",        "FXAA luma prepass",            VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS,           "postprocess",    "lightshafts",    "Light shafts",                 VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_TONEMAPPING,           "postprocess",    "tonemapping",    "Tonemapping",                  VTX_NONE },
	{ SDR_TYPE_DEFERRED_LIGHTING,                  "deferred",       "deferred",       "Deferred lighting",            VTX_POSITION },
	{ SDR_TYPE_VIDEO_PROCESS,                      "video",          "video",          "Video playback",               VTX_POSITION | VTX_TEXCOORD },
	{ SDR_TYPE_PASSTHROUGH_RENDER,                 "passthrough",    "passthrough",    "Passthrough rendering",        VTX_POSITION | VTX_TEXCOORD },
	{ SDR_TYPE_SHIELD_DECAL,                       "shield-impact",  "shield-impact",  "Shield impact",                VTX_POSITION | VTX_NORMAL },
	{ SDR_TYPE_BATCHED_BITMAP,                     "batched",        "batched",        "Batched bitmaps",              VTX_POSITION | VTX_COLOR | VTX_TEXCOORD },
	{ SDR_TYPE_DEFAULT_MATERIAL,                   "default-material", "default-material", "Default material",             VTX_POSITION | VTX_COLOR | VTX_TEXCOORD },
	{ SDR_TYPE_NANOVG,                             "nanovg",         "nanovg",         "NanoVG UI",                    VTX_POSITION | VTX_TEXCOORD },
	{ SDR_TYPE_DECAL,                              "decal",          "decal",          "Decals",                       VTX_POSITION | VTX_MATRIX },
	{ SDR_TYPE_SCENE_FOG,                          "fog",            "fog",            "Scene fog",                    VTX_NONE },
	{ SDR_TYPE_VOLUMETRIC_FOG,                     "volumetric-fog", "volumetric-fog", "Volumetric fog",               VTX_NONE },
	{ SDR_TYPE_ROCKET_UI,                          "rocketui",       "rocketui",       "Rocket UI",                    VTX_POSITION | VTX_COLOR | VTX_TEXCOORD },
	{ SDR_TYPE_COPY,                               "copy",           "copy",           "Texture copy",                 VTX_NONE },
	{ SDR_TYPE_MSAA_RESOLVE,                       "msaa-resolve",   "msaa-resolve",   "MSAA resolve",                 VTX_NONE },
	{ SDR_TYPE_IRRADIANCE_MAP_GEN,                 "irradiance",     "irradiance",     "Irradiance map generation",    VTX_NONE },
	{ SDR_TYPE_SHADOW_MAP,                         "shadow",         "shadow",         "Shadow map generation",        VTX_POSITION | VTX_MODELID },
};

const size_t VULKAN_SHADER_TYPES_COUNT = sizeof(VULKAN_SHADER_TYPES) / sizeof(VULKAN_SHADER_TYPES[0]);

bool VulkanShaderManager::init(vk::Device device)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;
	m_initialized = true;

	mprintf(("VulkanShaderManager: Initialized\n"));
	return true;
}

void VulkanShaderManager::shutdown()
{
	if (!m_initialized) {
		return;
	}

	// Clear all shaders (unique_ptrs will clean up)
	m_shaders.clear();
	m_shaderMap.clear();
	m_freeSlots.clear();

	m_initialized = false;
	mprintf(("VulkanShaderManager: Shutdown complete\n"));
}

int VulkanShaderManager::maybeCreateShader(shader_type type, unsigned int /*flags*/)
{
	if (!m_initialized) {
		return -1;
	}

	// Flags are ignored — Vulkan uses pre-compiled SPIR-V with runtime UBO flags
	int key = static_cast<int>(type);
	auto it = m_shaderMap.find(key);
	if (it != m_shaderMap.end()) {
		return static_cast<int>(it->second);
	}

	return loadShader(type);
}

void VulkanShaderManager::recompileAllShaders(const std::function<void(size_t, size_t)>& progressCallback)
{
	if (!m_initialized) {
		return;
	}

	size_t total = m_shaders.size();
	size_t current = 0;

	for (auto& shader : m_shaders) {
		if (shader.valid) {
			// Re-load this shader
			shader_type type = shader.type;

			// Release old modules
			shader.vertexModule.reset();
			shader.fragmentModule.reset();
			shader.valid = false;

			const VulkanShaderTypeInfo* typeInfo = getShaderTypeInfo(type);
			if (typeInfo) {
				SCP_string vertFile = SCP_string(typeInfo->vertexFile) + ".vert";
				shader.vertexModule = loadSpirvModule(vertFile);
				shader.vertexInputMask = typeInfo->vertexInputMask;

				SCP_string fragFile = SCP_string(typeInfo->fragmentFile) + ".frag";
				shader.fragmentModule = loadSpirvModule(fragFile);

				shader.valid = shader.vertexModule && shader.fragmentModule;
			}
		}

		++current;
		if (progressCallback) {
			progressCallback(current, total);
		}
	}

	mprintf(("VulkanShaderManager: Recompiled %zu shaders\n", total));
}

const VulkanShaderModule* VulkanShaderManager::getShader(int handle) const
{
	if (handle < 0 || static_cast<size_t>(handle) >= m_shaders.size()) {
		return nullptr;
	}

	const VulkanShaderModule& shader = m_shaders[handle];
	return shader.valid ? &shader : nullptr;
}

const VulkanShaderModule* VulkanShaderManager::getShaderByType(shader_type type) const
{
	int key = static_cast<int>(type);
	auto it = m_shaderMap.find(key);
	if (it == m_shaderMap.end()) {
		return nullptr;
	}

	return getShader(static_cast<int>(it->second));
}

bool VulkanShaderManager::isShaderTypeSupported(shader_type type) const
{
	return getShaderTypeInfo(type) != nullptr;
}

vk::UniqueShaderModule VulkanShaderManager::loadSpirvModule(const SCP_string& filename)
{
	// Try to load from def_files
	SCP_string fullName = filename + ".spv";

	const auto def_file = defaults_get_file(fullName.c_str());
	if (def_file.data == nullptr || def_file.size == 0) {
		mprintf(("VulkanShaderManager: Could not load SPIR-V file: %s\n", fullName.c_str()));
		return {};
	}

	// Validate SPIR-V magic number
	if (def_file.size < 4) {
		mprintf(("VulkanShaderManager: SPIR-V file too small: %s\n", fullName.c_str()));
		return {};
	}

	const uint32_t* spirvData = static_cast<const uint32_t*>(def_file.data);
	if (spirvData[0] != 0x07230203) {
		mprintf(("VulkanShaderManager: Invalid SPIR-V magic number in: %s\n", fullName.c_str()));
		return {};
	}

	vk::ShaderModuleCreateInfo createInfo;
	createInfo.codeSize = def_file.size;
	createInfo.pCode = spirvData;

	try {
		auto module = m_device.createShaderModuleUnique(createInfo);
		mprintf(("VulkanShaderManager: Loaded SPIR-V: %s (size=%zu)\n", fullName.c_str(), def_file.size));
		return module;
	} catch (const vk::SystemError& e) {
		mprintf(("VulkanShaderManager: Failed to create shader module from %s: %s\n",
			fullName.c_str(), e.what()));
		return {};
	}
}

int VulkanShaderManager::loadShader(shader_type type)
{
	const VulkanShaderTypeInfo* typeInfo = getShaderTypeInfo(type);
	if (!typeInfo) {
		mprintf(("VulkanShaderManager: Unknown shader type: %d\n", static_cast<int>(type)));
		return -1;
	}

	VulkanShaderModule shader;
	shader.type = type;
	shader.description = typeInfo->description;

	// Load vertex shader
	SCP_string vertFile = SCP_string(typeInfo->vertexFile) + ".vert";
	shader.vertexModule = loadSpirvModule(vertFile);
	shader.vertexInputMask = typeInfo->vertexInputMask;

	// Load fragment shader
	SCP_string fragFile = SCP_string(typeInfo->fragmentFile) + ".frag";
	shader.fragmentModule = loadSpirvModule(fragFile);

	// Check if essential modules loaded
	shader.valid = shader.vertexModule && shader.fragmentModule;

	if (!shader.valid) {
		mprintf(("VulkanShaderManager: Failed to load shader type %d\n", static_cast<int>(type)));
	}

	// Find or allocate slot
	size_t index;
	if (!m_freeSlots.empty()) {
		index = m_freeSlots.back();
		m_freeSlots.pop_back();
		m_shaders[index] = std::move(shader);
	} else {
		index = m_shaders.size();
		m_shaders.push_back(std::move(shader));
	}

	// Add to lookup map
	m_shaderMap[static_cast<int>(type)] = index;

	if (m_shaders[index].valid) {
		nprintf(("Vulkan", "VulkanShaderManager: Created shader %zu: %s\n",
			index, typeInfo->description));
	}

	return static_cast<int>(index);
}

const VulkanShaderTypeInfo* VulkanShaderManager::getShaderTypeInfo(shader_type type) const
{
	for (size_t i = 0; i < VULKAN_SHADER_TYPES_COUNT; ++i) {
		if (VULKAN_SHADER_TYPES[i].type == type) {
			return &VULKAN_SHADER_TYPES[i];
		}
	}
	return nullptr;
}

} // namespace vulkan
} // namespace graphics
