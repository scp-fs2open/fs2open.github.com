# Generates the field lists the checkpoint codec uses to visit structs (see tools/fieldscan).
#
# target_checkpoint_fields(<target> OUTPUT <generated header name>)
#
# Scans the headers of the target for structs and enums marked with "// @checkpoint fields".  The generated header is
# placed in <binary dir>/checkpoint/ and regenerated whenever one of the headers changes.
function(target_checkpoint_fields _target)
	cmake_parse_arguments(ARG "" "OUTPUT" "" ${ARGN})

	set(_sourceRoot "${CMAKE_CURRENT_SOURCE_DIR}")
	set(_outputDir "${CMAKE_CURRENT_BINARY_DIR}/checkpoint")
	set(_output "${_outputDir}/${ARG_OUTPUT}")
	set(_stamp "${_outputDir}/${ARG_OUTPUT}.stamp")
	set(_headerList "${_outputDir}/${ARG_OUTPUT}.headers")

	# the target's headers, as listed in its sources
	set(_headers "$<FILTER:$<TARGET_PROPERTY:${_target},SOURCES>,INCLUDE,\\.h$>")
	file(GENERATE OUTPUT "${_headerList}" CONTENT "$<JOIN:${_headers},\n>\n")

	IF(CMAKE_CROSSCOMPILING)
		SET(FIELDSCAN_PATH "FIELDSCAN-NOTFOUND" CACHE FILEPATH "Required for a cross compiling build, point to a fieldscan of a cmake build for the host OS")
		IF(FIELDSCAN_PATH STREQUAL "FIELDSCAN-NOTFOUND")
			message(FATAL_ERROR "A manual fieldscan path must be configured for cross compilation. Set FIELDSCAN_PATH to the location of a fieldscan executable, found in the build directories for the host OS.")
		ENDIF()
		SET(_fieldscanDependency "${FIELDSCAN_PATH}")
	ELSE(CMAKE_CROSSCOMPILING)
		SET(FIELDSCAN_PATH "fieldscan")
		# a changed tool regenerates the lists
		SET(_fieldscanDependency fieldscan)
	ENDIF(CMAKE_CROSSCOMPILING)

	file(MAKE_DIRECTORY "${_outputDir}")

	# fieldscan only rewrites the header when its content changes; the stamp lets the build skip
	# recompiling dependents when it didn't
	add_custom_command(
		OUTPUT "${_stamp}"
		BYPRODUCTS "${_output}"
		COMMAND ${FIELDSCAN_PATH} "${_sourceRoot}" "${_headerList}" "${_output}"
		COMMAND ${CMAKE_COMMAND} -E touch "${_stamp}"
		DEPENDS "${_headerList}" "${_headers}" ${_fieldscanDependency}
		COMMAND_EXPAND_LISTS
		COMMENT "Generating checkpoint field lists"
	)

	target_sources(${_target} PRIVATE "${_stamp}")
	# only checkpoint/checkpoint_fields.cpp includes the generated header
	target_include_directories(${_target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
	source_group("Generated Files\\Checkpoint" FILES "${_stamp}" "${_output}")
endfunction()
