#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace graphics::vulkan {

struct ShaderVariant;
class VulkanShadercLibrary;

/**
 * @brief Runtime GLSL->SPIR-V compiler using shaderc
 *
 * Replaces build-time glslc compilation. Compiles GLSL to SPIR-V at runtime
 * with disk caching, #define variant injection, and include resolution.
 * The shaderc library is loaded dynamically.
 */
class VulkanShaderCompiler {
public:
	VulkanShaderCompiler();
	~VulkanShaderCompiler();

	bool init(SCP_vector<ShaderVariant> variants);
	void shutdown();

	/**
	 * @brief Check if the shaderc library was loaded successfully
	 */
	bool isAvailable() const;

	/**
	 * @brief Compile GLSL to SPIR-V with disk caching
	 *
	 * @param filename  GLSL source filename, e.g. "main.frag"
	 * @param stage     Vertex or fragment
	 * @param sdrType   Shader type (for variant flag lookup)
	 * @param flags     SDR_FLAG_* bitmask — matching flags injected as #defines
	 * @return SPIR-V words, or empty vector on failure
	 */
	SCP_vector<uint32_t> compile(const SCP_string& filename,
	                             vk::ShaderStageFlagBits stage,
	                             shader_type sdrType,
	                             unsigned int flags);

	/**
	 * @brief Delete stale cache files older than ~2 months
	 */
	static void purgeOldCache();

private:
	/**
	 * @brief Build the GLSL header with #defines for variant flags
	 */
	SCP_string buildHeader(vk::ShaderStageFlagBits stage, shader_type sdrType, unsigned int flags) const;

	/**
	 * @brief Compute an MD5 hash of the header + source + version tag for disk caching
	 */
	static SCP_string computeSourceHash(const SCP_string& header, const SCP_string& source);

	SCP_vector<ShaderVariant> m_variants;
	std::unique_ptr<VulkanShadercLibrary> m_shaderc;
	bool m_initialized = false;
};

} // namespace graphics::vulkan
