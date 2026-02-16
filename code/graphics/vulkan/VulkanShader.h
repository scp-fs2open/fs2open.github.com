#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

#include <vulkan/vulkan.hpp>
#include <functional>

namespace graphics {
namespace vulkan {

/**
 * @brief Holds SPIR-V shader modules for a single shader program
 *
 * Corresponds to an OpenGL shader program (vertex + fragment).
 * Vulkan uses pre-compiled SPIR-V with no runtime variants — all conditional
 * behavior is handled via UBO runtime flags (not compile-time defines).
 */
struct VulkanShaderModule {
	vk::UniqueShaderModule vertexModule;
	vk::UniqueShaderModule fragmentModule;

	shader_type type = SDR_TYPE_NONE;

	SCP_string description;
	bool valid = false;

	// Bitmask of vertex input locations this shader declares (bit N = location N).
	// Used at pipeline creation to filter out fallback attributes the shader
	// doesn't consume. Copied from VulkanShaderTypeInfo at load time.
	uint32_t vertexInputMask = 0;
};

/**
 * @brief Shader type definition - maps shader type to SPIR-V filenames
 *
 * Based on opengl_shader_type_t from gropenglshader.h
 */
struct VulkanShaderTypeInfo {
	shader_type type;
	const char* vertexFile;      // Vertex shader SPIR-V filename (without .spv)
	const char* fragmentFile;    // Fragment shader SPIR-V filename
	const char* description;
	uint32_t vertexInputMask;    // Bitmask of vertex input locations (bit N = location N)
};

/**
 * @brief Manages Vulkan shader modules (SPIR-V loading and caching)
 *
 * Provides the implementation for gr_screen.gf_maybe_create_shader and
 * gr_screen.gf_recompile_all_shaders function pointers.
 *
 * Unlike OpenGL, Vulkan shaders are pre-compiled to SPIR-V with no
 * runtime variant support. The flags parameter in maybeCreateShader is
 * accepted for API compatibility but ignored — all conditional behavior
 * is handled via UBO runtime flags in the shader code.
 */
class VulkanShaderManager {
public:
	VulkanShaderManager() = default;
	~VulkanShaderManager() = default;

	// Non-copyable
	VulkanShaderManager(const VulkanShaderManager&) = delete;
	VulkanShaderManager& operator=(const VulkanShaderManager&) = delete;

	/**
	 * @brief Initialize the shader manager
	 * @param device Vulkan logical device
	 * @return true on success
	 */
	bool init(vk::Device device);

	/**
	 * @brief Shutdown and release all shader modules
	 */
	void shutdown();

	/**
	 * @brief Get or create a shader program
	 *
	 * Implements gr_screen.gf_maybe_create_shader.
	 * The flags parameter is ignored — Vulkan uses pre-compiled SPIR-V
	 * with runtime UBO flags instead of compile-time variants.
	 *
	 * @param type Shader type
	 * @param flags Ignored (accepted for API compatibility)
	 * @return Shader handle (index), or -1 on failure
	 */
	int maybeCreateShader(shader_type type, unsigned int flags);

	/**
	 * @brief Recompile all loaded shaders
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
	 * @brief Get a shader by type
	 * @param type Shader type
	 * @return Pointer to shader module, or nullptr if not found
	 */
	const VulkanShaderModule* getShaderByType(shader_type type) const;

	/**
	 * @brief Get total number of loaded shaders
	 */
	size_t getShaderCount() const { return m_shaders.size(); }

	/**
	 * @brief Check if a shader type is supported
	 * @param type Shader type to check
	 * @return true if the shader type has SPIR-V files defined
	 */
	bool isShaderTypeSupported(shader_type type) const;

private:
	/**
	 * @brief Load a SPIR-V shader module from embedded files
	 * @param filename Base filename (e.g., "model.vert")
	 * @return Shader module, or empty unique_ptr on failure
	 */
	vk::UniqueShaderModule loadSpirvModule(const SCP_string& filename);

	/**
	 * @brief Load a shader for the given type
	 * @param type Shader type
	 * @return Index of new shader, or -1 on failure
	 */
	int loadShader(shader_type type);

	/**
	 * @brief Get shader type info for a shader type
	 * @param type Shader type
	 * @return Pointer to type info, or nullptr if not found
	 */
	const VulkanShaderTypeInfo* getShaderTypeInfo(shader_type type) const;

	vk::Device m_device;

	// Shader lookup: type -> index in m_shaders
	SCP_unordered_map<int, size_t> m_shaderMap;

	// All loaded shaders
	SCP_vector<VulkanShaderModule> m_shaders;

	// Free list for shader slot reuse
	SCP_vector<size_t> m_freeSlots;

	bool m_initialized = false;
};

// Global shader type definitions
extern const VulkanShaderTypeInfo VULKAN_SHADER_TYPES[];
extern const size_t VULKAN_SHADER_TYPES_COUNT;

// Global shader manager access
VulkanShaderManager* getShaderManager();
void setShaderManager(VulkanShaderManager* manager);

// ========== gr_screen function pointer implementations ==========

int vulkan_maybe_create_shader(shader_type shader_t, unsigned int flags);
void vulkan_recompile_all_shaders(const std::function<void(size_t, size_t)>& progressCallback);

} // namespace vulkan
} // namespace graphics
