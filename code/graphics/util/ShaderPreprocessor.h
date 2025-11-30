#pragma once

#include "globalincs/pstypes.h"

#include <functional>

namespace graphics {

/**
 * @brief Callback interface for shader preprocessing operations
 *
 * This allows the preprocessor to be backend-agnostic (OpenGL/Vulkan)
 * by delegating source loading and capability checking to the caller.
 */
struct ShaderPreprocessorCallbacks {
	/**
	 * @brief Load shader source from a file
	 * @param filename The shader filename to load
	 * @return The shader source content, or empty string on failure
	 */
	std::function<SCP_string(const char* filename)> loadSource;

	/**
	 * @brief Check if a graphics capability is supported
	 * @param capabilityName The capability name (e.g., "geometry_shader")
	 * @return true if the capability is supported
	 */
	std::function<bool(const SCP_string& capabilityName)> checkCapability;
};

/**
 * @brief Preprocesses shader source code
 *
 * Handles the following directives:
 * - #include "filename" - includes another shader file
 * - #conditional_include (+|-)"capability" "filename" - conditional include based on capability
 * - #predefine token replacePattern - defines a replacement pattern with %s placeholder
 * - #prereplace token argument - applies a predefine replacement
 *
 * This is a shared implementation used by both OpenGL and Vulkan backends.
 */
class ShaderPreprocessor {
public:
	/**
	 * @brief Construct a preprocessor with callbacks
	 * @param callbacks The callbacks for loading sources and checking capabilities
	 */
	explicit ShaderPreprocessor(ShaderPreprocessorCallbacks callbacks);
	~ShaderPreprocessor() = default;

	/**
	 * @brief Process #include and #conditional_include directives
	 *
	 * Recursively processes includes, detecting cyclic includes.
	 * Outputs #line directives to maintain proper error reporting.
	 *
	 * @param filename The current file being processed (for error messages)
	 * @param source The shader source to process
	 * @return Processed source with includes expanded
	 */
	SCP_string handleIncludes(const char* filename, const SCP_string& source);

	/**
	 * @brief Process #predefine and #prereplace directives
	 *
	 * #predefine creates a named pattern with a %s placeholder.
	 * #prereplace applies a predefine, substituting the argument for %s.
	 *
	 * @param filename The current file being processed (for error messages)
	 * @param source The shader source to process
	 * @return Processed source with predefines applied
	 */
	SCP_string handlePredefines(const char* filename, const SCP_string& source);

	/**
	 * @brief Perform full preprocessing (includes then predefines)
	 *
	 * Convenience method that calls handleIncludes followed by handlePredefines.
	 *
	 * @param filename The shader filename being processed
	 * @param source The raw shader source
	 * @return Fully preprocessed shader source
	 */
	SCP_string preprocess(const char* filename, const SCP_string& source);

private:
	ShaderPreprocessorCallbacks m_callbacks;

	/**
	 * @brief Internal recursive include handler
	 */
	void handleIncludesImpl(SCP_vector<SCP_string>& includeStack,
	                        SCP_stringstream& output,
	                        int& includeCounter,
	                        const SCP_string& filename,
	                        const SCP_string& source);
};

} // namespace graphics
