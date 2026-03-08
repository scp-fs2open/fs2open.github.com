#include "VulkanShaderCompiler.h"
#include "VulkanShader.h"

#include "def_files/def_files.h"
#include "external_dll/externalcode.h"
#include "graphics/shader_preprocess.h"
#include "globalincs/systemvars.h"
#include "graphics/post_processing.h"

#include <md5.h>
#include <shaderc/shaderc.h>

namespace graphics::vulkan {

// ========== VulkanShadercLibrary ==========

class VulkanShadercLibrary : public SCP_ExternalCode {
public:
	VulkanShadercLibrary();
	bool isLoaded() const { return m_loaded; }

	decltype(&shaderc_compiler_initialize)                    compiler_initialize = nullptr;
	decltype(&shaderc_compiler_release)                       compiler_release = nullptr;
	decltype(&shaderc_compile_options_initialize)              compile_options_initialize = nullptr;
	decltype(&shaderc_compile_options_release)                 compile_options_release = nullptr;
	decltype(&shaderc_compile_options_set_target_env)          compile_options_set_target_env = nullptr;
	decltype(&shaderc_compile_options_set_optimization_level)  compile_options_set_optimization_level = nullptr;
	decltype(&shaderc_compile_options_set_generate_debug_info) compile_options_set_generate_debug_info = nullptr;
	decltype(&shaderc_compile_into_spv)                       compile_into_spv = nullptr;
	decltype(&shaderc_result_release)                         result_release = nullptr;
	decltype(&shaderc_result_get_compilation_status)           result_get_compilation_status = nullptr;
	decltype(&shaderc_result_get_error_message)                result_get_error_message = nullptr;
	decltype(&shaderc_result_get_num_warnings)                 result_get_num_warnings = nullptr;
	decltype(&shaderc_result_get_bytes)                       result_get_bytes = nullptr;
	decltype(&shaderc_result_get_length)                      result_get_length = nullptr;

private:
	bool m_loaded = false;
};

VulkanShadercLibrary::VulkanShadercLibrary()
{
	// Try platform-specific library names
#if defined(_WIN32)
	const char* names[] = {"shaderc_shared.dll", "shaderc.dll"};
#elif defined(__APPLE__)
	const char* names[] = {"libshaderc.dylib"};
#else
	const char* names[] = {"libshaderc.so", "libshaderc.so.1"};
#endif

	bool loaded = false;
	for (const auto* name : names) {
		if (LoadExternal(name)) {
			loaded = true;
			mprintf(("VulkanShadercLibrary: Loaded '%s'\n", name));
			break;
		}
	}

	if (!loaded) {
		mprintf(("VulkanShadercLibrary: Could not load shaderc shared library\n"));
		return;
	}

	// Load all required function pointers
	compiler_initialize                = LoadFunction<decltype(compiler_initialize)>("shaderc_compiler_initialize");
	compiler_release                   = LoadFunction<decltype(compiler_release)>("shaderc_compiler_release");
	compile_options_initialize         = LoadFunction<decltype(compile_options_initialize)>("shaderc_compile_options_initialize");
	compile_options_release            = LoadFunction<decltype(compile_options_release)>("shaderc_compile_options_release");
	compile_options_set_target_env     = LoadFunction<decltype(compile_options_set_target_env)>("shaderc_compile_options_set_target_env");
	compile_options_set_optimization_level = LoadFunction<decltype(compile_options_set_optimization_level)>("shaderc_compile_options_set_optimization_level");
	compile_options_set_generate_debug_info = LoadFunction<decltype(compile_options_set_generate_debug_info)>("shaderc_compile_options_set_generate_debug_info");
	compile_into_spv                   = LoadFunction<decltype(compile_into_spv)>("shaderc_compile_into_spv");
	result_release                     = LoadFunction<decltype(result_release)>("shaderc_result_release");
	result_get_compilation_status      = LoadFunction<decltype(result_get_compilation_status)>("shaderc_result_get_compilation_status");
	result_get_error_message           = LoadFunction<decltype(result_get_error_message)>("shaderc_result_get_error_message");
	result_get_num_warnings            = LoadFunction<decltype(result_get_num_warnings)>("shaderc_result_get_num_warnings");
	result_get_bytes                   = LoadFunction<decltype(result_get_bytes)>("shaderc_result_get_bytes");
	result_get_length                  = LoadFunction<decltype(result_get_length)>("shaderc_result_get_length");

	// Verify all required functions were loaded
	m_loaded = compiler_initialize && compiler_release
		&& compile_options_initialize && compile_options_release
		&& compile_options_set_target_env && compile_options_set_optimization_level
		&& compile_options_set_generate_debug_info
		&& compile_into_spv
		&& result_release && result_get_compilation_status
		&& result_get_error_message && result_get_num_warnings
		&& result_get_bytes && result_get_length;

	if (!m_loaded) {
		mprintf(("VulkanShadercLibrary: Library loaded but some functions are missing!\n"));
	}
}

// ========== VulkanShaderCompiler ==========

VulkanShaderCompiler::VulkanShaderCompiler() = default;
VulkanShaderCompiler::~VulkanShaderCompiler() = default;

bool VulkanShaderCompiler::init(SCP_vector<ShaderVariant> variants)
{
	if (m_initialized) {
		return true;
	}

	m_shaderc = std::make_unique<VulkanShadercLibrary>();
	if (!m_shaderc->isLoaded()) {
		mprintf(("VulkanShaderCompiler: shaderc library not available!\n"
		         "  Install the Vulkan SDK or shaderc shared library:\n"
		         "    Debian/Ubuntu: sudo apt install libshaderc-dev\n"
		         "    Vulkan SDK: https://vulkan.lunarg.com/sdk/home\n"));
		m_shaderc.reset();
		return false;
	}

	m_variants = std::move(variants);
	m_initialized = true;
	mprintf(("VulkanShaderCompiler: Initialized (runtime shaderc compilation)\n"));
	return true;
}

void VulkanShaderCompiler::shutdown()
{
	m_shaderc.reset();
	m_initialized = false;
}

bool VulkanShaderCompiler::isAvailable() const
{
	return m_initialized && m_shaderc && m_shaderc->isLoaded();
}

SCP_string VulkanShaderCompiler::buildHeader(vk::ShaderStageFlagBits /*stage*/, shader_type sdrType,
                                              unsigned int flags) const
{
	SCP_string header;
	header.reserve(512);

	// Required for layout(location=N) on varyings in Vulkan GLSL.
	// Injected here so individual shaders don't need it.
	header += "#extension GL_ARB_separate_shader_objects : enable\n";

	// shaderc automatically predefines VULKAN=100 when targeting Vulkan,
	// so we do NOT define it here — doing so causes a "Macro redefined" error.

	// Blinn-Phong lighting model (matches OpenGL's opengl_shader_get_header)
	if (Detail.lighting < 3) {
		header += "#define FLAG_LIGHT_MODEL_BLINN_PHONG\n";
	}

	// Post-processing shaders need special header injection (matching OpenGL's
	// opengl_post_shader_header). Effect indices map to #define names, and
	// lightshafts needs the sample count.
	if (sdrType == SDR_TYPE_POST_PROCESS_MAIN || sdrType == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS) {
		if (graphics::Post_processing_manager) {
			if (sdrType == SDR_TYPE_POST_PROCESS_MAIN) {
				const auto& postEffects = graphics::Post_processing_manager->getPostEffects();
				for (size_t idx = 0; idx < postEffects.size(); idx++) {
					if (flags & (1 << idx)) {
						header += "#define ";
						header += postEffects[idx].define_name;
						header += "\n";
					}
				}
			} else if (sdrType == SDR_TYPE_POST_PROCESS_LIGHTSHAFTS) {
				const auto& ls_params = graphics::Post_processing_manager->getLightshaftParams();
				char temp[64];
				snprintf(temp, sizeof(temp), "#define SAMPLE_NUM %d\n", ls_params.samplenum);
				header += temp;
			}
		}
	} else {
		// Inject variant-specific #defines based on flags
		for (const auto& variant : m_variants) {
			if (variant.type == sdrType && (flags & variant.flag)) {
				header += "#define ";
				header += variant.define;
				header += "\n";
			}
		}
	}

	return header;
}

SCP_string VulkanShaderCompiler::computeSourceHash(const SCP_string& header, const SCP_string& source)
{
	MD5 md5;
	md5.update(header.c_str(), static_cast<MD5::size_type>(header.size()));
	md5.update(source.c_str(), static_cast<MD5::size_type>(source.size()));

	// Include a version tag so cache is invalidated on engine updates
	static const char VERSION_TAG[] = "vk_shader_v1";
	md5.update(VERSION_TAG, sizeof(VERSION_TAG) - 1);

	md5.finalize();
	return md5.hexdigest();
}

SCP_vector<uint32_t> VulkanShaderCompiler::compile(const SCP_string& filename,
                                                     vk::ShaderStageFlagBits stage,
                                                     shader_type sdrType,
                                                     unsigned int flags)
{
	if (!m_initialized || !m_shaderc) {
		mprintf(("VulkanShaderCompiler: Not initialized!\n"));
		return {};
	}

	// Load and preprocess GLSL source
	SCP_string source = shader_load_source(filename);
	if (source.empty()) {
		mprintf(("VulkanShaderCompiler: Failed to load GLSL source: %s\n", filename.c_str()));
		return {};
	}

	source = shader_preprocess_includes(filename, source);
	source = shader_preprocess_defines(filename, source);

	// Build preprocessor header with #defines for variant flags
	SCP_string header = buildHeader(stage, sdrType, flags);

	// Compute hash for disk cache (uses preprocessed source)
	SCP_string hash = computeSourceHash(header, source);
	SCP_string cacheFilename = "vk_shader-" + hash + ".spv";

	// Check disk cache
	CFILE* cacheFile = cfopen(cacheFilename.c_str(), "rb", CF_TYPE_CACHE);
	if (cacheFile != nullptr) {
		int fileSize = cfilelength(cacheFile);
		if (fileSize > 0 && (fileSize % 4) == 0) {
			SCP_vector<uint32_t> spirv(fileSize / 4);
			if (cfread(spirv.data(), 1, fileSize, cacheFile) == fileSize) {
				cfclose(cacheFile);
				nprintf(("Vulkan", "VulkanShaderCompiler: Cache hit for %s (flags=0x%x)\n",
				         filename.c_str(), flags));
				return spirv;
			}
		}
		cfclose(cacheFile);
	}

	// Cache miss — compile with shaderc
	mprintf(("VulkanShaderCompiler: Compiling %s (flags=0x%x)...\n", filename.c_str(), flags));

	// Assemble: #version + header (extension + defines) + source
	SCP_string fullSource;
	fullSource.reserve(header.size() + source.size() + 32);
	fullSource += "#version 450\n";
	fullSource += header;
	fullSource += source;

	auto* sc = m_shaderc.get();

	shaderc_compiler_t compiler = sc->compiler_initialize();
	if (!compiler) {
		mprintf(("VulkanShaderCompiler: Failed to initialize shaderc compiler!\n"));
		return {};
	}

	shaderc_compile_options_t opts = sc->compile_options_initialize();
	sc->compile_options_set_target_env(opts, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
	sc->compile_options_set_optimization_level(opts, shaderc_optimization_level_performance);
	sc->compile_options_set_generate_debug_info(opts);

	shaderc_shader_kind kind;
	if (stage == vk::ShaderStageFlagBits::eVertex) {
		kind = shaderc_vertex_shader;
	} else {
		kind = shaderc_fragment_shader;
	}

	shaderc_compilation_result_t result = sc->compile_into_spv(
	    compiler, fullSource.c_str(), fullSource.size(), kind, filename.c_str(), "main", opts);

	SCP_vector<uint32_t> spirv;
	auto status = sc->result_get_compilation_status(result);

	if (status != shaderc_compilation_status_success) {
		const char* errMsg = sc->result_get_error_message(result);
		mprintf(("VulkanShaderCompiler: COMPILATION FAILED for %s (flags=0x%x):\n%s\n",
		         filename.c_str(), flags, errMsg ? errMsg : "(no error message)"));
	} else {
		if (sc->result_get_num_warnings(result) > 0) {
			const char* errMsg = sc->result_get_error_message(result);
			mprintf(("VulkanShaderCompiler: Warnings for %s:\n%s\n",
			         filename.c_str(), errMsg ? errMsg : ""));
		}

		size_t byteLen = sc->result_get_length(result);
		const char* bytes = sc->result_get_bytes(result);

		if (bytes && byteLen > 0 && (byteLen % 4) == 0) {
			spirv.resize(byteLen / 4);
			std::memcpy(spirv.data(), bytes, byteLen);

			mprintf(("VulkanShaderCompiler: Compiled %s -> %zu bytes SPIR-V\n",
			         filename.c_str(), byteLen));

			// Save to disk cache
			cacheFile = cfopen(cacheFilename.c_str(), "wb", CF_TYPE_CACHE);
			if (cacheFile != nullptr) {
				cfwrite(spirv.data(), static_cast<int>(spirv.size() * sizeof(uint32_t)), 1, cacheFile);
				cfclose(cacheFile);
			}
		}
	}

	sc->result_release(result);
	sc->compile_options_release(opts);
	sc->compiler_release(compiler);

	return spirv;
}

void VulkanShaderCompiler::purgeOldCache()
{
	const SCP_string PREFIX = "vk_shader-";
	const auto TIMEOUT = 2.0 * 30.0 * 24.0 * 60.0 * 60.0; // ~2 months in seconds

	SCP_vector<SCP_string> cache_files;
	SCP_vector<file_list_info> file_info;
	cf_get_file_list(cache_files, CF_TYPE_CACHE, "*.spv", CF_SORT_NONE, &file_info,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	Assertion(cache_files.size() == file_info.size(),
	          "cf_get_file_list returned different sizes for file names and file informations!");

	auto now = std::time(nullptr);
	for (size_t i = 0; i < cache_files.size(); ++i) {
		auto& name = cache_files[i];

		if (name.compare(0, PREFIX.size(), PREFIX) != 0) {
			continue; // Not our cache file
		}

		auto diff = std::difftime(now, file_info[i].write_time);
		if (diff > TIMEOUT) {
			auto full_name = name + ".spv";
			cf_delete(full_name.c_str(), CF_TYPE_CACHE);
		}
	}
}

} // namespace graphics::vulkan
