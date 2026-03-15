#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/shader_types.h"

#include <vulkan/vulkan.hpp>
#include <functional>
#include <memory>


namespace graphics::vulkan {

class VulkanShaderCompiler;

/**
 * @brief Holds SPIR-V shader modules for a single shader program (or variant)
 *
 * Corresponds to an OpenGL shader program (vertex + fragment).
 * With runtime compilation, each unique (type, flags) pair produces a distinct
 * VulkanShaderModule — matching OpenGL's compile-time variant system.
 */
struct VulkanShaderModule {
	vk::UniqueShaderModule vertexModule;
	vk::UniqueShaderModule fragmentModule;

	shader_type type = SDR_TYPE_NONE;
	unsigned int flags = 0;  // SDR_FLAG_* bitmask for this variant

	SCP_string description;
	bool valid = false;

	// Bitmask of vertex input locations this shader declares (bit N = location N).
	// Used at pipeline creation to filter out fallback attributes the shader
	// doesn't consume. Computed from ShaderTypeInfo::attributes at load time.
	uint32_t vertexInputMask = 0;
};

/**
 * @brief Manages Vulkan shader modules with runtime GLSL→SPIR-V compilation
 *
 * Provides the implementation for gr_screen.gf_maybe_create_shader and
 * gr_screen.gf_recompile_all_shaders function pointers.
 *
 * Shader variants are handled via compile-time #defines, matching OpenGL's
 * system. Each unique (type, flags) pair produces a distinct compiled shader.
 */
class VulkanShaderManager {
public:
	VulkanShaderManager();
	~VulkanShaderManager();

	// Non-copyable
	VulkanShaderManager(const VulkanShaderManager&) = delete;
	VulkanShaderManager& operator=(const VulkanShaderManager&) = delete;

	/**
	 * @brief Initialize the shader manager and runtime compiler
	 * @param device Vulkan logical device
	 * @return true on success
	 */
	bool init(vk::Device device);

	/**
	 * @brief Shutdown and release all shader modules
	 */
	void shutdown();

	/**
	 * @brief Get or create a shader program variant
	 *
	 * Implements gr_screen.gf_maybe_create_shader.
	 * Each unique (type, flags) pair produces a distinct compiled shader.
	 *
	 * @param type Shader type
	 * @param flags SDR_FLAG_* bitmask for variant selection
	 * @return Shader handle (index), or -1 on failure
	 */
	int maybeCreateShader(shader_type type, unsigned int flags);

	/**
	 * @brief Recompile all loaded shader variants
	 *
	 * Implements gr_screen.gf_recompile_all_shaders
	 *
	 * @param progressCallback Called with (current, total) progress
	 */
	void recompileAllShaders(const std::function<void(size_t, size_t)>& progressCallback);

	/**
	 * @brief Get a shader by handle
	 * @param handle Shader handle from maybeCreateShader
	 * @return Pointer to shader module, or nullptr if invalid
	 */
	const VulkanShaderModule* getShader(int handle) const;

	/**
	 * @brief Get a shader by handle (alias for getShader)
	 */
	const VulkanShaderModule* getShaderByHandle(int handle) const { return getShader(handle); }

	/**
	 * @brief Get any shader of the given type (ignoring flags).
	 *
	 * Used for vertex input mask queries where the specific variant doesn't matter.
	 * @param type Shader type
	 * @return Pointer to shader module, or nullptr if not found
	 */
	const VulkanShaderModule* getShaderByType(shader_type type) const;

	/**
	 * @brief Get total number of loaded shaders
	 */
	size_t getShaderCount() const { return m_shaders.size(); }

private:
	/**
	 * @brief Load and compile a shader variant
	 * @param type Shader type
	 * @param flags SDR_FLAG_* bitmask
	 * @return Index of new shader, or -1 on failure
	 */
	int loadShader(shader_type type, unsigned int flags);

	vk::Device m_device;

	// Shader lookup: (type, flags) -> index in m_shaders
	typedef std::pair<int, unsigned int> shader_key_t;
	struct key_hasher {
		size_t operator()(const shader_key_t& k) const {
			return std::hash<int>()(k.first) ^ (std::hash<unsigned int>()(k.second) << 16);
		}
	};
	SCP_unordered_map<shader_key_t, size_t, key_hasher> m_shaderMap;

	// All loaded shaders
	SCP_vector<VulkanShaderModule> m_shaders;

	// Free list for shader slot reuse
	SCP_vector<size_t> m_freeSlots;

	// Runtime GLSL→SPIR-V compiler
	std::unique_ptr<VulkanShaderCompiler> m_compiler;

	bool m_initialized = false;
};

// Global shader manager access
VulkanShaderManager* getShaderManager();
void setShaderManager(VulkanShaderManager* manager);

// ========== gr_screen function pointer implementations ==========

int vulkan_maybe_create_shader(shader_type shader_t, unsigned int flags);
void vulkan_recompile_all_shaders(const std::function<void(size_t, size_t)>& progressCallback);

} // namespace graphics::vulkan
