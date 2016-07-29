
include(CheckCCompilerFlag)

FUNCTION(MAKE_CACHE_INTERNAL VARIABLE)
	SET(${VARIABLE} ${${VARIABLE}} CACHE INTERNAL "Internal cache variable")
ENDFUNCTION(MAKE_CACHE_INTERNAL)

FUNCTION(ADD_IMPORTED_LIB NAME INCLUDES LIBS)
	ADD_LIBRARY(${NAME} INTERFACE)

	target_link_libraries(${NAME} INTERFACE "${LIBS}")

	target_include_directories(${NAME} INTERFACE "${INCLUDES}")
ENDFUNCTION(ADD_IMPORTED_LIB)

MACRO(PKG_CONFIG_LIB_RESOLVE NAME OUTVAR)
	SET(${OUTVAR} "")
	foreach(lib ${${NAME}_LIBRARIES})
		find_library(${lib}_LIBRARY
					NAMES ${lib}
					HINTS ${${NAME}_LIBDIR} ${${NAME}_LIBRARY_DIRS}
		)

		if (NOT ${${lib}_LIBRARY} MATCHES ".*NOTFOUND.*")
			SET(${OUTVAR} ${${OUTVAR}} ${${lib}_LIBRARY})
		endif (NOT ${${lib}_LIBRARY} MATCHES ".*NOTFOUND.*")
	endforeach(lib)
ENDMACRO(PKG_CONFIG_LIB_RESOLVE)

# Copy from http://cmake.3232098.n2.nabble.com/RFC-cmake-analog-to-AC-SEARCH-LIBS-td7585423.html
INCLUDE (CheckFunctionExists)
INCLUDE (CheckLibraryExists)

MACRO (CMAKE_SEARCH_LIBS v_func v_lib func)
    CHECK_FUNCTION_EXISTS (${func} ${v_func})
    IF (NOT ${v_func})
        FOREACH (lib ${ARGN})
            CHECK_LIBRARY_EXISTS (${lib} ${func} "" "HAVE_${func}_IN_${lib}")
            IF (${HAVE_${func}_IN_${lib}})
                SET (${v_func} TRUE)
                SET (${v_lib} "${lib}" CACHE INTERNAL "Library providing ${func}")
                BREAK()
            ENDIF (${HAVE_${func}_IN_${lib}})
        ENDFOREACH (lib)
    ENDIF (NOT ${v_func})
ENDMACRO (CMAKE_SEARCH_LIBS)
# End copy

FUNCTION(EP_CHECK_FILE_EXISTS FILE OUTVAR TARGET NAME COMMAND WORKING_DIR)
	string (REPLACE ";" " " COMMAND_STR "${COMMAND}")
	IF(WIN32)
		# Do something special for windows...
		SET(COMMAND "cmd /C \"${COMMAND_STR}\"")
	ELSE(WIN32)
		SET(COMMAND "${COMMAND_STR}")
	ENDIF(WIN32)

	FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${NAME}" "
IF(EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${FILE}\")
	MESSAGE(\"'${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${FILE}' already exists, nothing to be done.\")
ELSE(EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${FILE}\")
	execute_process(COMMAND ${COMMAND} WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${WORKING_DIR}\")
ENDIF(EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${FILE}\")
")

	SET(${OUTVAR} "${CMAKE_COMMAND}" -P ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}/${NAME} PARENT_SCOPE)
ENDFUNCTION(EP_CHECK_FILE_EXISTS)

MACRO(COPY_FILES_TO_TARGET _target)
	FOREACH(file IN LISTS TARGET_COPY_FILES)
		ADD_CUSTOM_COMMAND(
			TARGET ${_target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${file}"  "$<TARGET_FILE_DIR:${_target}>"
			COMMENT "copying '${file}'..."
		)
	ENDFOREACH(file)
ENDMACRO(COPY_FILES_TO_TARGET)

macro(set_policy policy value)
	if (POLICY ${policy})
		cmake_policy(SET ${policy} ${value})
	endif ()
endmacro(set_policy)

macro(set_if_not_defined VAR VALUE)
    if (NOT DEFINED ${VAR})
        set(${VAR} ${VALUE})
    endif()
endmacro(set_if_not_defined)

macro(configure_cotire target)
	IF(COTIRE_ENABLE)
		# Disable unity build as it doesn't work well for us
		set_target_properties(${target} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)

		# add ignored paths for the precompiled header here
		set_target_properties(code PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH
			"${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR}")

		IF(DEFINED CMAKE_CONFIGURATION_TYPES)
			cotire(${target} CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
		ELSE(DEFINED CMAKE_CONFIGURATION_TYPES)
			cotire(${target})
		ENDIF(DEFINED CMAKE_CONFIGURATION_TYPES)
	ENDIF(COTIRE_ENABLE)
endmacro(configure_cotire)

macro(add_target_copy_files)
	INSTALL(FILES ${ARGN}
			DESTINATION ${BINARY_DESTINATION}
	)

	SET(TARGET_COPY_FILES ${TARGET_COPY_FILES} ${ARGN} CACHE INTERNAL "" FORCE)
endmacro(add_target_copy_files)

function(detect_simd_instructions _out_var)
	if(CMAKE_CROSSCOMPILING)
		set(${_out_var} "" PARENT_SCOPE)
	else()
		message(STATUS "Detecting SIMD features of current CPU...")
		TRY_RUN(RUN_RESULT COMPILE_RESULT "${CMAKE_BINARY_DIR}/temp" "${CMAKE_SOURCE_DIR}/cmake/cpufeatures.cpp"
			RUN_OUTPUT_VARIABLE FEATURE_OUTPUT)
		
		IF(COMPILE_RESULT)
			MESSAGE(STATUS "Detected compatibility for the ${FEATURE_OUTPUT} feature set.")
			SET(${_out_var} ${FEATURE_OUTPUT} PARENT_SCOPE)
		ELSE(COMPILE_RESULT)
			MESSAGE("Compilation of CPU feature detector failed, please set the instruction set manually.")
			set(${_out_var} "" PARENT_SCOPE)
		ENDIF(COMPILE_RESULT)
	endif()
endfunction()

function (check_linker_flag _flag _out_var)
	SET(CMAKE_REQUIRED_FLAGS "${_flag}")
	CHECK_C_COMPILER_FLAG("" ${_out_var})
endfunction(check_linker_flag)
