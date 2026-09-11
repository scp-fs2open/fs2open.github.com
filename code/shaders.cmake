
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/def_files/data/effects")

# All Vulkan GLSL shaders live alongside OpenGL shaders in def_files/data/effects/*.sdr,
# unified with #ifdef VULKAN / #ifdef OPENGL guards. They are embedded into the executable
# via source_groups.cmake (target_embed_files) and compiled to SPIR-V at runtime via shaderc.

# Shaders that need C++ struct header generation from SPIR-V reflection.
# Generated structs are included via shader_structs.h for compile-time layout validation.
set(SHADERS_NEED_STRUCT_GEN
	${SHADER_DIR}/default-material-f.sdr
	${SHADER_DIR}/default-material-v.sdr
)

# Struct header generation via shadertool (SPIR-V reflection).
# When SHADERS_ENABLE_COMPILATION is ON, shaders in SHADERS_NEED_STRUCT_GEN are
# compiled to temporary SPIR-V with glslc, then shadertool generates C++ struct
# headers from the reflection data. The generated headers are checked into VCS
# so that builds without glslc/shadertool still work.
set(_structHeaderList)
set(_shaderCompiledDir "${CMAKE_CURRENT_SOURCE_DIR}/graphics/shaders/compiled")

foreach (_shader ${SHADERS_NEED_STRUCT_GEN})
	get_filename_component(_fileName "${_shader}" NAME)
	get_filename_component(_baseShaderName "${_shader}" NAME_WE)

	# Determine shader stage from filename convention: *-v.sdr = vertex, *-f.sdr = fragment
	string(REGEX MATCH "-([vf])\\.sdr$" _match "${_fileName}")
	if (CMAKE_MATCH_1 STREQUAL "v")
		set(_stage "vertex")
		set(_structSuffix ".vert.h")
	else()
		set(_stage "fragment")
		set(_structSuffix ".frag.h")
	endif()

	# Map e.g. "default-material-f" to "default-material_structs.frag.h"
	string(REGEX REPLACE "-[vf]$" "" _baseName "${_baseShaderName}")
	set(_structOutput "${_shaderCompiledDir}/${_baseName}_structs${_structSuffix}")
	list(APPEND _structHeaderList "${_structOutput}")

	if (TARGET glslc)
		set(_spirvFile "${CMAKE_CURRENT_BINARY_DIR}/shaders/${_fileName}.spv")
		set(_depFileDir "${CMAKE_CURRENT_BINARY_DIR}/shaders")
		set(_depFile "${_depFileDir}/${_fileName}.spv.d")
		file(RELATIVE_PATH _relativeSpirvPath "${CMAKE_BINARY_DIR}" "${_spirvFile}")

		set(DEPFILE_PARAM)
		if (CMAKE_GENERATOR STREQUAL "Ninja")
			set(DEPFILE_PARAM DEPFILE "${_depFile}")
		endif ()

		add_custom_command(OUTPUT "${_spirvFile}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${_depFileDir}"
			COMMAND glslc -x glsl -fshader-stage=${_stage}
				"${_shader}" -o "${_spirvFile}" --target-env=vulkan1.0 -std=450 -O -g
				"-I${SHADER_DIR}"
				-MD -MF "${_depFile}" -MT "${_relativeSpirvPath}" -Werror
			MAIN_DEPENDENCY "${_shader}"
			COMMENT "Compiling shader ${_fileName} (for struct generation)"
			${DEPFILE_PARAM}
			)

		add_custom_command(OUTPUT "${_structOutput}"
			COMMAND shadertool --structs "--structs-output=${_structOutput}" "${_spirvFile}"
			MAIN_DEPENDENCY "${_spirvFile}"
			COMMENT "Generating struct header from ${_fileName}"
			)
	endif()
endforeach ()

set(_shaderHeaderPath "${CMAKE_CURRENT_BINARY_DIR}/shader_structs.h")
set(_headerContent "#pragma once")
foreach(_headerPath ${_structHeaderList})
	set(_headerContent "${_headerContent}\n#include \"${_headerPath}\"")
endforeach()
# Use the generate command to avoid rewriting the file if the contents did not actually change
file(GENERATE OUTPUT "${_shaderHeaderPath}"
	CONTENT "${_headerContent}")
target_include_directories(code PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

target_sources(code PRIVATE ${_structHeaderList} "${_shaderHeaderPath}")
source_group("Graphics\\Shader structs" FILES ${_structHeaderList} "${_shaderHeaderPath}")
