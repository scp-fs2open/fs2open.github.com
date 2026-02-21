
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/graphics/shaders")
# This is the legacy location of shader code. To avoid duplicating included files, this is added as an include directory
set(LEGACY_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/def_files/data/effects")

set(SHADERS
	${SHADER_DIR}/default-material.frag
	${SHADER_DIR}/default-material.vert
	${SHADER_DIR}/passthrough.frag
	${SHADER_DIR}/passthrough.vert
	${SHADER_DIR}/batched.frag
	${SHADER_DIR}/batched.vert
	${SHADER_DIR}/video.frag
	${SHADER_DIR}/video.vert
	${SHADER_DIR}/rocketui.frag
	${SHADER_DIR}/rocketui.vert
	${SHADER_DIR}/main.frag
	${SHADER_DIR}/main.vert
	${SHADER_DIR}/nanovg.frag
	${SHADER_DIR}/nanovg.vert
	${SHADER_DIR}/decal.frag
	${SHADER_DIR}/decal.vert
	${SHADER_DIR}/postprocess.vert
	${SHADER_DIR}/tonemapping.frag
	${SHADER_DIR}/brightpass.frag
	${SHADER_DIR}/blur.frag
	${SHADER_DIR}/bloom-comp.frag
	${SHADER_DIR}/fxaapre.frag
	${SHADER_DIR}/fxaa.frag
	${SHADER_DIR}/post.frag
	${SHADER_DIR}/lightshafts.frag
	${SHADER_DIR}/effect.vert
	${SHADER_DIR}/effect.frag
	${SHADER_DIR}/effect-distort.vert
	${SHADER_DIR}/effect-distort.frag
	${SHADER_DIR}/deferred.vert
	${SHADER_DIR}/deferred.frag
	${SHADER_DIR}/shadow.vert
	${SHADER_DIR}/shadow.frag
	${SHADER_DIR}/irradiance.vert
	${SHADER_DIR}/irradiance.frag
	${SHADER_DIR}/fog.vert
	${SHADER_DIR}/fog.frag
	${SHADER_DIR}/volumetric-fog.vert
	${SHADER_DIR}/volumetric-fog.frag
	${SHADER_DIR}/copy.frag
	${SHADER_DIR}/copy.vert
	${SHADER_DIR}/shield-impact.frag
	${SHADER_DIR}/shield-impact.vert
	${SHADER_DIR}/msaa-resolve.vert
	${SHADER_DIR}/msaa-resolve.frag
)

# Shaders shared with the OpenGL backend. These get GLSL decompilation (.spv.glsl)
# and the decompiled GLSL is embedded for runtime use.
# All other shaders are Vulkan-only: SPIR-V compilation and embedding only.
set(SHADERS_GL_SHARED
	${SHADER_DIR}/default-material.frag
	${SHADER_DIR}/default-material.vert
)

# Shaders that need C++ struct header generation from SPIR-V reflection.
# Generated structs are included via shader_structs.h for compile-time layout validation.
set(SHADERS_NEED_STRUCT_GEN
	${SHADER_DIR}/default-material.frag
	${SHADER_DIR}/default-material.vert
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

	list(FIND SHADERS_GL_SHARED "${_shader}" _isGlShared)
	list(FIND SHADERS_NEED_STRUCT_GEN "${_shader}" _needStructs)

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
			MAIN_DEPENDENCY "${_shader}"
			COMMENT "Compiling shader ${_fileName}"
			${DEPFILE_PARAM}
			)

		target_embed_files(code FILES "${_spirvFile}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")

		# Build shadertool arguments based on what this shader needs
		set(_glslOutput)
		set(_shadertoolArgs)
		set(_shadertoolOutputs)

		if (_isGlShared GREATER -1)
			set(_glslOutput "${_spirvFile}.glsl")
			list(APPEND _shadertoolArgs --glsl "--glsl-output=${_glslOutput}")
			list(APPEND _shadertoolOutputs "${_glslOutput}")
		endif()
		if (_needStructs GREATER -1)
			set(_structOutput "${_shaderOutputDir}/${_baseShaderName}_structs${_shaderExt}.h")
			list(APPEND _shadertoolArgs --structs "--structs-output=${_structOutput}")
			list(APPEND _shadertoolOutputs "${_structOutput}")
			list(APPEND _structHeaderList "${_structOutput}")
		endif()

		if (_shadertoolArgs)
			add_custom_command(OUTPUT ${_shadertoolOutputs}
				COMMAND shadertool ${_shadertoolArgs} ${_spirvFile}
				MAIN_DEPENDENCY "${_spirvFile}"
				COMMENT "Processing shader ${_spirvFile}"
				)
		endif()

		if (_glslOutput)
			target_embed_files(code FILES "${_glslOutput}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")
		endif()
	else()
		# No shader compiler available — use pre-compiled files from VCS
		target_embed_files(code FILES "${_spirvFile}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")

		if (_needStructs GREATER -1)
			set(_structOutput "${_shaderOutputDir}/${_baseShaderName}_structs${_shaderExt}.h")
			list(APPEND _structHeaderList "${_structOutput}")
		endif()

		if (_isGlShared GREATER -1)
			set(_glslOutput "${_spirvFile}.glsl")
			target_embed_files(code FILES "${_glslOutput}" RELATIVE_TO "${_shaderOutputDir}" PATH_TYPE_PREFIX "data/effects")
		endif()
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
