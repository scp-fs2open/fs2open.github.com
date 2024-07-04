
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/graphics/shaders")
# This is the legacy location of shader code. To avoid duplicating included files, this is added as an include directory
set(LEGACY_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/def_files/data/effects")

set(SHADERS
	${SHADER_DIR}/default-material.frag
	${SHADER_DIR}/default-material.vert
	${SHADER_DIR}/vulkan.frag
	${SHADER_DIR}/vulkan.vert
)

target_sources(code PRIVATE ${SHADERS})
source_group("Graphics\\Shaders" FILES ${SHADERS})

set(_structHeaderList)

foreach (_shader ${SHADERS})
	if ("${_shader}" MATCHES "\\.glsl$")
		# Ignore include files since they will only be used but not compiled
		continue()
	endif ()

	get_filename_component(_fileName "${_shader}" NAME)

	# We write the compiled/generated shader files to the source directory so that they can be included in the VCS
	# This way, it is not necessary to have the tools for compiling shaders when doing non-shader related work
	set(_shaderOutputDir "${CMAKE_CURRENT_SOURCE_DIR}/graphics/shaders/compiled")
	set(_spirvFile "${_shaderOutputDir}/${_fileName}.spv")

	get_filename_component(_baseShaderName "${_shader}" NAME_WE)
	get_filename_component(_shaderExt "${_shader}" EXT)

	if (TARGET glslc)
		set(_depFileDir "${CMAKE_CURRENT_BINARY_DIR}/shaders")
		set(_depFile "${_depFileDir}/${_fileName}.spv.d")
		file(RELATIVE_PATH _relativeSpirvPath "${CMAKE_BINARY_DIR}" "${_spirvFile}")

		set(DEPFILE_PARAM)
		if (CMAKE_GENERATOR STREQUAL "Ninja")
			set(DEPFILE_PARAM DEPFILE "${_depFile}")
		endif ()

		add_custom_command(OUTPUT "${_spirvFile}"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${_depFileDir}"
			COMMAND glslc "${_shader}" -o "${_spirvFile}" --target-env=vulkan1.0 -O -g "-I${SHADER_DIR}"
				"-I${LEGACY_SHADER_DIR}" -MD -MF "${_depFile}" -MT "${_relativeSpirvPath}" -Werror -x glsl
			MAIN_DEPENDENCY "${shader}"
			COMMENT "Compiling shader ${_fileName}"
			${DEPFILE_PARAM}
			)

		target_embed_files(code FILES "${_spirvFile}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")

		set(_glslOutput "${_spirvFile}.glsl")
		set(_structOutput "${_shaderOutputDir}/${_baseShaderName}_structs${_shaderExt}.h")

		list(APPEND _structHeaderList "${_structOutput}")

		add_custom_command(OUTPUT "${_glslOutput}" "${_structOutput}"
			COMMAND shadertool --glsl "--glsl-output=${_glslOutput}" --structs "--structs-output=${_structOutput}" ${_spirvFile}
			MAIN_DEPENDENCY "${_spirvFile}"
			COMMENT "Processing shader ${_spirvFile}"
			)

		target_embed_files(code FILES "${_glslOutput}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")
	else()
		target_embed_files(code FILES "${_spirvFile}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")

		set(_glslOutput "${_spirvFile}.glsl")
		set(_structOutput "${_shaderOutputDir}/${_baseShaderName}_structs${_shaderExt}.h")

		list(APPEND _structHeaderList "${_structOutput}")

		target_embed_files(code FILES "${_glslOutput}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")
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
