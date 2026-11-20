# Generates the field lists the checkpoint codec uses to visit structs (see tools/fieldscan).
#
# target_checkpoint_fields(<target> LIST <struct list> OUTPUT <generated header name>)
#
# The struct list names "<struct> <header>" pairs, with headers relative to the directory of the list's target
# (code/). The generated header is placed in <binary dir>/checkpoint/ and regenerated whenever the list or one of
# the listed headers changes.
function(target_checkpoint_fields _target)
	cmake_parse_arguments(ARG "" "LIST;OUTPUT" "" ${ARGN})

	get_filename_component(_listPath "${ARG_LIST}" ABSOLUTE)
	set(_sourceRoot "${CMAKE_CURRENT_SOURCE_DIR}")
	set(_outputDir "${CMAKE_CURRENT_BINARY_DIR}/checkpoint")
	set(_output "${_outputDir}/${ARG_OUTPUT}")
	set(_stamp "${_outputDir}/${ARG_OUTPUT}.stamp")

	# the headers to depend on come from the list itself, so reconfigure when it changes
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_listPath}")
	file(STRINGS "${_listPath}" _lines)
	set(_headers)
	foreach(_line IN LISTS _lines)
		string(REGEX REPLACE "#.*" "" _line "${_line}")
		string(STRIP "${_line}" _line)
		if (_line STREQUAL "")
			continue()
		endif()
		string(REGEX REPLACE "^[^ \t]+[ \t]+" "" _header "${_line}")
		list(APPEND _headers "${_sourceRoot}/${_header}")
	endforeach()
	list(REMOVE_DUPLICATES _headers)

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
		COMMAND ${FIELDSCAN_PATH} "${_sourceRoot}" "${_listPath}" "${_output}"
		COMMAND ${CMAKE_COMMAND} -E touch "${_stamp}"
		DEPENDS "${_listPath}" ${_headers} ${_fieldscanDependency}
		COMMENT "Generating checkpoint field lists"
	)

	target_sources(${_target} PRIVATE "${_stamp}")
	# only checkpoint/checkpoint_fields.cpp includes the generated header
	target_include_directories(${_target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
	source_group("Generated Files\\Checkpoint" FILES "${_stamp}" "${_output}")
endfunction()
