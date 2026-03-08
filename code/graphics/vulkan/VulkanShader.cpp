#include "VulkanShader.h"
#include "VulkanShaderCompiler.h"
#include "VulkanVertexFormat.h"


namespace graphics::vulkan {

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

// Shader type definitions - maps shader_type to GLSL filenames
// Vertex input location bits
// Values reflect what the shader declares. Used to filter out fallback
// vertex attributes the shader doesn't consume.
static constexpr uint32_t VTX_POSITION = 1u << static_cast<uint32_t>(VertexAttributeLocation::Position);
static constexpr uint32_t VTX_COLOR    = 1u << static_cast<uint32_t>(VertexAttributeLocation::Color);
static constexpr uint32_t VTX_TEXCOORD = 1u << static_cast<uint32_t>(VertexAttributeLocation::TexCoord);
static constexpr uint32_t VTX_NORMAL   = 1u << static_cast<uint32_t>(VertexAttributeLocation::Normal);
static constexpr uint32_t VTX_TANGENT  = 1u << static_cast<uint32_t>(VertexAttributeLocation::Tangent);
static constexpr uint32_t VTX_MODELID  = 1u << static_cast<uint32_t>(VertexAttributeLocation::ModelId);
static constexpr uint32_t VTX_RADIUS   = 1u << static_cast<uint32_t>(VertexAttributeLocation::Radius);
// static constexpr uint32_t VTX_UVEC     = 1u << static_cast<uint32_t>(VertexAttributeLocation::Uvec);
static constexpr uint32_t VTX_MATRIX   = (15u << static_cast<uint32_t>(VertexAttributeLocation::ModelMatrix)); // Four consecutive locations
static constexpr uint32_t VTX_NONE     = 0;

// Filenames are full .sdr names matching the def_files/data/effects/ directory.
// Unified shaders share source with OpenGL via #ifdef VULKAN / #ifdef OPENGL guards.
// Vulkan-only shaders have "-vulkan" in the name.
const VulkanShaderTypeInfo VULKAN_SHADER_TYPES[] = {
	{ SDR_TYPE_MODEL,                              "main-v.sdr",              "main-f.sdr",              "Model rendering",              VTX_POSITION | VTX_TEXCOORD | VTX_NORMAL | VTX_TANGENT | VTX_MODELID },
	{ SDR_TYPE_EFFECT_PARTICLE,                    "effect-v.sdr",            "effect-f.sdr",            "Particle effects",             VTX_POSITION | VTX_COLOR | VTX_TEXCOORD | VTX_RADIUS },
	{ SDR_TYPE_EFFECT_DISTORTION,                  "effect-distort-v.sdr",    "effect-distort-f.sdr",    "Distortion effects",           VTX_POSITION | VTX_COLOR | VTX_TEXCOORD | VTX_RADIUS },
	{ SDR_TYPE_POST_PROCESS_MAIN,                  "post-v.sdr",              "post-f.sdr",              "Post-processing main",         VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_BLUR,                  "post-v.sdr",              "blur-f.sdr",              "Gaussian blur",                VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_BLOOM_COMP,            "post-v.sdr",              "bloom-comp-f.sdr",        "Bloom composition",            VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_BRIGHTPASS,            "post-v.sdr",              "brightpass-f.sdr",        "Bright pass filter",           VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_FXAA,                  "post-v.sdr",              "fxaa-vulkan-f.sdr",       "FXAA anti-aliasing",           VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_FXAA_PREPASS,          "post-v.sdr",              "fxaapre-f.sdr",           "FXAA luma prepass",            VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_LIGHTSHAFTS,           "post-v.sdr",              "ls-f.sdr",                "Light shafts",                 VTX_NONE },
	{ SDR_TYPE_POST_PROCESS_TONEMAPPING,           "post-v.sdr",              "tonemapping-f.sdr",       "Tonemapping",                  VTX_NONE },
	{ SDR_TYPE_DEFERRED_LIGHTING,                  "deferred-v.sdr",          "deferred-f.sdr",          "Deferred lighting",            VTX_POSITION },
	{ SDR_TYPE_VIDEO_PROCESS,                      "video-v.sdr",             "video-f.sdr",             "Video playback",               VTX_POSITION | VTX_TEXCOORD },
	{ SDR_TYPE_PASSTHROUGH_RENDER,                 "passthrough-v.sdr",       "passthrough-f.sdr",       "Passthrough rendering",        VTX_POSITION | VTX_COLOR | VTX_TEXCOORD },
	{ SDR_TYPE_SHIELD_DECAL,                       "shield-impact-v.sdr",     "shield-impact-f.sdr",     "Shield impact",                VTX_POSITION | VTX_NORMAL },
	{ SDR_TYPE_BATCHED_BITMAP,                     "batched-v.sdr",           "batched-f.sdr",           "Batched bitmaps",              VTX_POSITION | VTX_COLOR | VTX_TEXCOORD },
	{ SDR_TYPE_DEFAULT_MATERIAL,                   "default-material-v.sdr",  "default-material-f.sdr",  "Default material",             VTX_POSITION | VTX_COLOR | VTX_TEXCOORD },
	{ SDR_TYPE_NANOVG,                             "nanovg-v.sdr",            "nanovg-f.sdr",            "NanoVG UI",                    VTX_POSITION | VTX_TEXCOORD },
	{ SDR_TYPE_DECAL,                              "decal-v.sdr",             "decal-f.sdr",             "Decals",                       VTX_POSITION | VTX_MATRIX },
	{ SDR_TYPE_SCENE_FOG,                          "post-v.sdr",              "fog-f.sdr",               "Scene fog",                    VTX_NONE },
	{ SDR_TYPE_VOLUMETRIC_FOG,                     "post-v.sdr",              "volumetric-f.sdr",        "Volumetric fog",               VTX_NONE },
	{ SDR_TYPE_ROCKET_UI,                          "rocketui-v.sdr",          "rocketui-f.sdr",          "Rocket UI",                    VTX_POSITION | VTX_COLOR | VTX_TEXCOORD },
	{ SDR_TYPE_COPY,                               "post-v.sdr",              "copy-f.sdr",              "Texture copy",                 VTX_NONE },
	{ SDR_TYPE_MSAA_RESOLVE,                       "msaa-resolve-vulkan-v.sdr", "msaa-resolve-vulkan-f.sdr", "MSAA resolve",             VTX_NONE },
	{ SDR_TYPE_IRRADIANCE_MAP_GEN,                 "post-v.sdr",              "irrmap-f.sdr",            "Irradiance map generation",    VTX_NONE },
	{ SDR_TYPE_SHADOW_MAP,                         "shadow-vulkan-v.sdr",     "shadow-vulkan-f.sdr",     "Shadow map generation",        VTX_POSITION | VTX_MODELID },
};

// ========== Variant table ==========
// Mirrors OpenGL's GL_shader_variants[] — same (type, flag, define) triples.
// MODEL flags come from model_shader_flags.h; others from 2d.h defines.

const ShaderVariant VULKAN_SHADER_VARIANTS[] = {
	// --- MODEL shader flags (from model_shader_flags.h) ---
	{ SDR_TYPE_MODEL, (1 << 0),  "MODEL_SDR_FLAG_LIGHT" },
	{ SDR_TYPE_MODEL, (1 << 1),  "MODEL_SDR_FLAG_DEFERRED" },
	{ SDR_TYPE_MODEL, (1 << 2),  "MODEL_SDR_FLAG_HDR" },
	{ SDR_TYPE_MODEL, (1 << 3),  "MODEL_SDR_FLAG_DIFFUSE" },
	{ SDR_TYPE_MODEL, (1 << 4),  "MODEL_SDR_FLAG_GLOW" },
	{ SDR_TYPE_MODEL, (1 << 5),  "MODEL_SDR_FLAG_SPEC" },
	{ SDR_TYPE_MODEL, (1 << 6),  "MODEL_SDR_FLAG_NORMAL" },
	{ SDR_TYPE_MODEL, (1 << 7),  "MODEL_SDR_FLAG_AMBIENT" },
	{ SDR_TYPE_MODEL, (1 << 8),  "MODEL_SDR_FLAG_MISC" },
	{ SDR_TYPE_MODEL, (1 << 9),  "MODEL_SDR_FLAG_TEAMCOLOR" },
	{ SDR_TYPE_MODEL, (1 << 10), "MODEL_SDR_FLAG_FOG" },
	{ SDR_TYPE_MODEL, (1 << 11), "MODEL_SDR_FLAG_TRANSFORM" },
	{ SDR_TYPE_MODEL, (1 << 12), "MODEL_SDR_FLAG_SHADOWS" },
	{ SDR_TYPE_MODEL, (1 << 13), "MODEL_SDR_FLAG_THRUSTER" },
	{ SDR_TYPE_MODEL, (1 << 14), "MODEL_SDR_FLAG_ALPHA_MULT" },

	// --- Non-MODEL shader flags (from 2d.h) ---
	{ SDR_TYPE_EFFECT_PARTICLE,          SDR_FLAG_PARTICLE_POINT_GEN,            "FLAG_EFFECT_GEOMETRY" },
	{ SDR_TYPE_DEFERRED_LIGHTING,        SDR_FLAG_ENV_MAP,                       "ENV_MAP" },
	{ SDR_TYPE_POST_PROCESS_BLUR,        SDR_FLAG_BLUR_HORIZONTAL,              "PASS_0" },
	{ SDR_TYPE_POST_PROCESS_BLUR,        SDR_FLAG_BLUR_VERTICAL,                "PASS_1" },
	{ SDR_TYPE_NANOVG,                   SDR_FLAG_NANOVG_EDGE_AA,               "EDGE_AA" },
	{ SDR_TYPE_DECAL,                    SDR_FLAG_DECAL_USE_NORMAL_MAP,         "USE_NORMAL_MAP" },
	{ SDR_TYPE_MSAA_RESOLVE,             SDR_FLAG_MSAA_SAMPLES_4,               "SAMPLES_4" },
	{ SDR_TYPE_MSAA_RESOLVE,             SDR_FLAG_MSAA_SAMPLES_8,               "SAMPLES_8" },
	{ SDR_TYPE_MSAA_RESOLVE,             SDR_FLAG_MSAA_SAMPLES_16,              "SAMPLES_16" },
	{ SDR_TYPE_VOLUMETRIC_FOG,           SDR_FLAG_VOLUMETRICS_DO_EDGE_SMOOTHING, "DO_EDGE_SMOOTHING" },
	{ SDR_TYPE_VOLUMETRIC_FOG,           SDR_FLAG_VOLUMETRICS_NOISE,            "NOISE" },
	{ SDR_TYPE_POST_PROCESS_TONEMAPPING, SDR_FLAG_TONEMAPPING_LINEAR_OUT,       "LINEAR_OUT" },
};

const size_t VULKAN_SHADER_VARIANTS_COUNT = sizeof(VULKAN_SHADER_VARIANTS) / sizeof(VULKAN_SHADER_VARIANTS[0]);

VulkanShaderManager::VulkanShaderManager() = default;
VulkanShaderManager::~VulkanShaderManager() = default;

bool VulkanShaderManager::init(vk::Device device)
{
	if (m_initialized) {
		return true;
	}

	m_device = device;

	// Initialize runtime shader compiler
	m_compiler = std::make_unique<VulkanShaderCompiler>();
	if (!m_compiler->init(SCP_vector<ShaderVariant>(VULKAN_SHADER_VARIANTS, VULKAN_SHADER_VARIANTS + VULKAN_SHADER_VARIANTS_COUNT))) {
		mprintf(("VulkanShaderManager: Failed to initialize shader compiler!\n"));
		return false;
	}

	VulkanShaderCompiler::purgeOldCache();

	m_initialized = true;

	mprintf(("VulkanShaderManager: Initialized with runtime shader compilation\n"));
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

	if (m_compiler) {
		m_compiler->shutdown();
		m_compiler.reset();
	}

	m_initialized = false;
	mprintf(("VulkanShaderManager: Shutdown complete\n"));
}

int VulkanShaderManager::maybeCreateShader(shader_type type, unsigned int flags)
{
	if (!m_initialized) {
		return -1;
	}

	shader_key_t key(static_cast<int>(type), flags);
	auto it = m_shaderMap.find(key);
	if (it != m_shaderMap.end()) {
		return static_cast<int>(it->second);
	}

	return loadShader(type, flags);
}

void VulkanShaderManager::recompileAllShaders(const std::function<void(size_t, size_t)>& progressCallback)
{
	if (!m_initialized || !m_compiler) {
		return;
	}

	size_t total = m_shaders.size();
	size_t current = 0;

	for (auto& shader : m_shaders) {
		if (shader.valid) {
			shader_type type = shader.type;
			unsigned int flags = shader.flags;

			// Release old modules
			shader.vertexModule.reset();
			shader.fragmentModule.reset();
			shader.valid = false;

			const VulkanShaderTypeInfo* typeInfo = getShaderTypeInfo(type);
			if (typeInfo) {
				SCP_string vertFile = typeInfo->vertexFile;
				auto vertSpirv = m_compiler->compile(vertFile, vk::ShaderStageFlagBits::eVertex, type, flags);
				if (!vertSpirv.empty()) {
					vk::ShaderModuleCreateInfo createInfo;
					createInfo.codeSize = vertSpirv.size() * sizeof(uint32_t);
					createInfo.pCode = vertSpirv.data();
					try {
						shader.vertexModule = m_device.createShaderModuleUnique(createInfo);
					} catch (const vk::SystemError& e) {
						mprintf(("VulkanShaderManager: Failed to create vertex module: %s\n", e.what()));
					}
				}
				shader.vertexInputMask = typeInfo->vertexInputMask;

				SCP_string fragFile = typeInfo->fragmentFile;
				auto fragSpirv = m_compiler->compile(fragFile, vk::ShaderStageFlagBits::eFragment, type, flags);
				if (!fragSpirv.empty()) {
					vk::ShaderModuleCreateInfo createInfo;
					createInfo.codeSize = fragSpirv.size() * sizeof(uint32_t);
					createInfo.pCode = fragSpirv.data();
					try {
						shader.fragmentModule = m_device.createShaderModuleUnique(createInfo);
					} catch (const vk::SystemError& e) {
						mprintf(("VulkanShaderManager: Failed to create fragment module: %s\n", e.what()));
					}
				}

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
	// Find the first shader of this type (any flags).
	// Used for vertex input mask queries where the specific variant doesn't matter.
	for (const auto& pair : m_shaderMap) {
		if (pair.first.first == static_cast<int>(type)) {
			return getShader(static_cast<int>(pair.second));
		}
	}
	return nullptr;
}

int VulkanShaderManager::loadShader(shader_type type, unsigned int flags)
{
	const VulkanShaderTypeInfo* typeInfo = getShaderTypeInfo(type);
	if (!typeInfo) {
		mprintf(("VulkanShaderManager: Unknown shader type: %d\n", static_cast<int>(type)));
		return -1;
	}

	VulkanShaderModule shader;
	shader.type = type;
	shader.flags = flags;
	shader.description = typeInfo->description;
	if (flags != 0) {
		shader.description += " (flags=0x";
		char buf[16];
		snprintf(buf, sizeof(buf), "%x", flags);
		shader.description += buf;
		shader.description += ")";
	}

	// Compile vertex shader
	SCP_string vertFile = typeInfo->vertexFile;
	auto vertSpirv = m_compiler->compile(vertFile, vk::ShaderStageFlagBits::eVertex, type, flags);
	if (!vertSpirv.empty()) {
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.codeSize = vertSpirv.size() * sizeof(uint32_t);
		createInfo.pCode = vertSpirv.data();
		try {
			shader.vertexModule = m_device.createShaderModuleUnique(createInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanShaderManager: Failed to create vertex module for %s: %s\n",
				vertFile.c_str(), e.what()));
		}
	}
	shader.vertexInputMask = typeInfo->vertexInputMask;

	// Compile fragment shader
	SCP_string fragFile = typeInfo->fragmentFile;
	auto fragSpirv = m_compiler->compile(fragFile, vk::ShaderStageFlagBits::eFragment, type, flags);
	if (!fragSpirv.empty()) {
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.codeSize = fragSpirv.size() * sizeof(uint32_t);
		createInfo.pCode = fragSpirv.data();
		try {
			shader.fragmentModule = m_device.createShaderModuleUnique(createInfo);
		} catch (const vk::SystemError& e) {
			mprintf(("VulkanShaderManager: Failed to create fragment module for %s: %s\n",
				fragFile.c_str(), e.what()));
		}
	}

	// Check if essential modules loaded
	shader.valid = shader.vertexModule && shader.fragmentModule;

	if (!shader.valid) {
		mprintf(("VulkanShaderManager: Failed to load shader type %d (flags=0x%x)\n",
			static_cast<int>(type), flags));
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
	shader_key_t key(static_cast<int>(type), flags);
	m_shaderMap[key] = index;

	if (m_shaders[index].valid) {
		nprintf(("Vulkan", "VulkanShaderManager: Created shader %zu: %s\n",
			index, m_shaders[index].description.c_str()));
	}

	return static_cast<int>(index);
}

const VulkanShaderTypeInfo* VulkanShaderManager::getShaderTypeInfo(shader_type type)
{
	for (const auto & i : VULKAN_SHADER_TYPES) {
		if (i.type == type) {
			return &i;
		}
	}
	return nullptr;
}

} // namespace graphics::vulkan
