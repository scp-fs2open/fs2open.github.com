#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

#ifdef WITH_VULKAN

#include <vulkan/vulkan.hpp>
#include <map>
#include <vector>

#ifdef FSO_HAVE_SHADERC
#include <shaderc/shaderc.hpp>
#endif

namespace graphics {
namespace vulkan {

/**
 * @brief Shader stage types for Vulkan
 */
enum class ShaderStage {
	Vertex,
	Fragment,
	Geometry
};

/**
 * @brief Information about a single descriptor binding
 */
struct DescriptorBindingInfo {
	uint32_t set = 0;
	uint32_t binding = 0;
	vk::DescriptorType type = vk::DescriptorType::eUniformBuffer;
	uint32_t count = 1;
	vk::ShaderStageFlags stageFlags = {};
	SCP_string name;
	size_t size = 0;  // For uniform buffers
};

/**
 * @brief Information about push constant ranges
 */
struct PushConstantInfo {
	vk::ShaderStageFlags stageFlags = {};
	uint32_t offset = 0;
	uint32_t size = 0;
};

/**
 * @brief Result of shader reflection containing all resource bindings
 */
struct ShaderReflectionData {
	SCP_vector<DescriptorBindingInfo> descriptors;
	SCP_vector<PushConstantInfo> pushConstants;
	uint32_t maxDescriptorSet = 0;  // Highest set number used
};

/**
 * @brief Result of creating descriptor set layouts
 */
struct DescriptorSetLayouts {
	SCP_vector<vk::UniqueDescriptorSetLayout> layouts;  // Indexed by set number
	SCP_vector<vk::PushConstantRange> pushConstantRanges;
};

/**
 * @brief Creates Vulkan descriptor set layouts from reflection data
 *
 * Takes ShaderReflectionData and creates the necessary vk::DescriptorSetLayout
 * objects for use in pipeline layout creation.
 */
class VulkanDescriptorSetLayoutBuilder {
public:
	explicit VulkanDescriptorSetLayoutBuilder(vk::Device device);
	~VulkanDescriptorSetLayoutBuilder() = default;

	/**
	 * @brief Build descriptor set layouts from reflection data
	 * @param reflectionData Merged reflection data from all shader stages
	 * @return Created descriptor set layouts (empty on failure)
	 */
	DescriptorSetLayouts build(const ShaderReflectionData& reflectionData);

	/**
	 * @brief Create a pipeline layout from descriptor set layouts
	 * @param layouts The descriptor set layouts to use
	 * @return Pipeline layout (may be null on failure)
	 */
	vk::UniquePipelineLayout createPipelineLayout(const DescriptorSetLayouts& layouts);

private:
	vk::Device m_device;
};

#ifdef FSO_HAVE_SPIRV_CROSS
/**
 * @brief Reflects SPIR-V shaders to extract resource bindings
 *
 * Uses SPIRV-Cross to parse compiled SPIR-V and extract:
 * - Uniform buffer bindings (set, binding, size)
 * - Sampled image bindings (textures)
 * - Push constant ranges
 */
class VulkanShaderReflection {
public:
	VulkanShaderReflection() = default;
	~VulkanShaderReflection() = default;

	/**
	 * @brief Reflect a SPIR-V shader module
	 * @param spirv SPIR-V binary data
	 * @param stage Shader stage for setting stageFlags
	 * @return Reflection data (empty on failure)
	 */
	ShaderReflectionData reflect(const std::vector<uint32_t>& spirv, ShaderStage stage);

	/**
	 * @brief Merge reflection data from multiple shader stages
	 * @param stages Vector of reflection data from each stage
	 * @return Combined reflection data with merged stage flags
	 */
	static ShaderReflectionData mergeStages(const SCP_vector<ShaderReflectionData>& stages);
};
#endif // FSO_HAVE_SPIRV_CROSS

/**
 * @brief Handles disk caching of compiled SPIR-V shaders
 *
 * Cache key includes: source hash + variant flags + shaderc version
 * to ensure stale cache bugs are avoided.
 */
class VulkanShaderCache {
public:
	VulkanShaderCache();
	~VulkanShaderCache() = default;

	/**
	 * @brief Initialize the cache directory
	 * @return true if cache directory is available
	 */
	bool initialize();

	/**
	 * @brief Compute cache key from all factors
	 * @param sourceHash MD5 hash of preprocessed GLSL source
	 * @param type Shader type (SDR_TYPE_*)
	 * @param flags Variant flags
	 * @param stage Shader stage (vertex/fragment/geometry)
	 * @return Cache key string
	 */
	SCP_string computeCacheKey(const SCP_string& sourceHash, shader_type type, uint32_t flags, ShaderStage stage);

	/**
	 * @brief Check if cached SPIR-V exists
	 * @param cacheKey Key from computeCacheKey()
	 * @return true if cache file exists
	 */
	bool hasCachedSpirv(const SCP_string& cacheKey);

	/**
	 * @brief Load SPIR-V from cache
	 * @param cacheKey Key from computeCacheKey()
	 * @return SPIR-V binary data (empty if not found)
	 */
	std::vector<uint32_t> loadCachedSpirv(const SCP_string& cacheKey);

	/**
	 * @brief Save compiled SPIR-V to cache
	 * @param cacheKey Key from computeCacheKey()
	 * @param spirv Compiled SPIR-V binary
	 */
	void saveSpirvToCache(const SCP_string& cacheKey, const std::vector<uint32_t>& spirv);

private:
	SCP_string m_cacheDir;
	uint32_t m_shadercVersion = 0;
	bool m_initialized = false;

	SCP_string getCacheFilePath(const SCP_string& cacheKey);
};

#ifdef FSO_HAVE_SHADERC
/**
 * @brief Compiles GLSL to SPIR-V using shaderc
 */
class VulkanShaderCompiler {
public:
	VulkanShaderCompiler();
	~VulkanShaderCompiler() = default;

	/**
	 * @brief Compile GLSL source to SPIR-V
	 * @param source GLSL source code
	 * @param stage Shader stage
	 * @param filename Source filename for error messages
	 * @return SPIR-V binary (empty on failure)
	 */
	std::vector<uint32_t> compileGlslToSpirv(const SCP_string& source, ShaderStage stage, const SCP_string& filename);

	/**
	 * @brief Get the shaderc version for cache keying
	 * @return shaderc version number
	 */
	uint32_t getShadercVersion() const;

private:
	shaderc::Compiler m_compiler;
	shaderc::CompileOptions m_options;

	shaderc_shader_kind stageToKind(ShaderStage stage);
};
#endif // FSO_HAVE_SHADERC

/**
 * @brief Manages Vulkan shader modules
 *
 * Handles preprocessing, compilation, caching, and loading of shaders.
 */
class VulkanShaderManager {
public:
	VulkanShaderManager(vk::Device device);
	~VulkanShaderManager();

	/**
	 * @brief Initialize the shader manager
	 * @return true on success
	 */
	bool initialize();

	/**
	 * @brief Shutdown and cleanup
	 */
	void shutdown();

	/**
	 * @brief Get or create a shader module for the given type and flags
	 * @param type Shader type (SDR_TYPE_*)
	 * @param flags Variant flags
	 * @param stage Shader stage
	 * @return Shader module (may be null if compilation failed)
	 */
	vk::ShaderModule getShader(shader_type type, uint32_t flags, ShaderStage stage);

	/**
	 * @brief Get vertex shader for type and flags
	 */
	vk::ShaderModule getVertexShader(shader_type type, uint32_t flags);

	/**
	 * @brief Get fragment shader for type and flags
	 */
	vk::ShaderModule getFragmentShader(shader_type type, uint32_t flags);

	/**
	 * @brief Get geometry shader for type and flags (may return null)
	 */
	vk::ShaderModule getGeometryShader(shader_type type, uint32_t flags);

	/**
	 * @brief Check if a shader type requires a geometry shader for given flags
	 */
	bool requiresGeometryShader(shader_type type, uint32_t flags);

private:
	vk::Device m_device;
	VulkanShaderCache m_cache;

#ifdef FSO_HAVE_SHADERC
	VulkanShaderCompiler m_compiler;
#endif

	// Cache of loaded shader modules: key = "type_flags_stage"
	std::map<SCP_string, vk::UniqueShaderModule> m_shaderModules;

	/**
	 * @brief Generate cache/lookup key for a shader
	 */
	SCP_string makeShaderKey(shader_type type, uint32_t flags, ShaderStage stage);

	/**
	 * @brief Preprocess shader source (header + includes + defines)
	 * @param type Shader type
	 * @param flags Variant flags
	 * @param stage Shader stage
	 * @return Preprocessed GLSL source
	 */
	SCP_string preprocessShader(shader_type type, uint32_t flags, ShaderStage stage);

	/**
	 * @brief Get shader filename for type and stage
	 */
	const char* getShaderFilename(shader_type type, ShaderStage stage);

	/**
	 * @brief Load shader source from file or embedded defaults
	 */
	SCP_string loadShaderSource(const char* filename);

	/**
	 * @brief Generate shader header with version and defines
	 */
	SCP_string generateShaderHeader(shader_type type, uint32_t flags, bool hasGeometryShader);

	/**
	 * @brief Process #include directives
	 */
	SCP_string handleIncludes(const char* filename, const SCP_string& source);

	/**
	 * @brief Process #predefine/#prereplace directives
	 */
	SCP_string handlePredefines(const char* filename, const SCP_string& source);

	/**
	 * @brief Compute MD5 hash of shader source
	 */
	SCP_string computeSourceHash(const SCP_string& source);

	/**
	 * @brief Create VkShaderModule from SPIR-V binary
	 */
	vk::UniqueShaderModule createShaderModule(const std::vector<uint32_t>& spirv);
};

} // namespace vulkan
} // namespace graphics

#endif // WITH_VULKAN
