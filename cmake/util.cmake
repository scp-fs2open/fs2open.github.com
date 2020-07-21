
include(CheckCCompilerFlag)

FUNCTION(MAKE_CACHE_INTERNAL VARIABLE)
	SET(${VARIABLE} ${${VARIABLE}} CACHE INTERNAL "Internal cache variable")
ENDFUNCTION(MAKE_CACHE_INTERNAL)

FUNCTION(ADD_IMPORTED_LIB NAME INCLUDES LIBS)
	ADD_LIBRARY(${NAME} INTERFACE)

	target_link_libraries(${NAME} INTERFACE "${LIBS}")

	target_include_directories(${NAME} SYSTEM INTERFACE "${INCLUDES}")
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

MACRO(COPY_FILE_TO_TARGET _target _file)
	if(UNIX)
		ADD_CUSTOM_COMMAND(
			TARGET ${_target} POST_BUILD
			COMMAND cp -a "${_file}"  "$<TARGET_FILE_DIR:${_target}>/${LIBRAY_DESTINATION}/"
			COMMENT "copying '${_file}'..."
		)
	else()
		ADD_CUSTOM_COMMAND(
			TARGET ${_target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_file}"  "$<TARGET_FILE_DIR:${_target}>/${LIBRAY_DESTINATION}/"
			COMMENT "copying '${_file}'..."
		)
	endif()
endmacro(COPY_FILE_TO_TARGET)

MACRO(COPY_FILES_TO_TARGET _target)
	if(UNIX)
		ADD_CUSTOM_COMMAND(
			TARGET ${_target} POST_BUILD
			COMMAND mkdir -p "$<TARGET_FILE_DIR:${_target}>/${LIBRAY_DESTINATION}/"
			COMMENT "Creating '$<TARGET_FILE_DIR:${_target}>/${LIBRAY_DESTINATION}/'..."
		)
	endif()
	
	FOREACH(file IN LISTS TARGET_COPY_FILES)
		COPY_FILE_TO_TARGET("${_target}" "${file}")
	ENDFOREACH(file)
ENDMACRO(COPY_FILES_TO_TARGET)

macro(_clear_old_libraries_state)
	set(is_debug FALSE)
	set(is_optimized FALSE)
endmacro(_clear_old_libraries_state)

function(CONVERT_OLD_LIBRARIES)
	_clear_old_libraries_state()
	set(out_list)
	foreach(lib ${ARGV})
		if ("${lib}" STREQUAL "debug")
			_clear_old_libraries_state()
			set(is_debug TRUE)
		elseif("${lib}" STREQUAL "optimized")
			_clear_old_libraries_state()
			set(is_optimized TRUE)
		elseif("${lib}" STREQUAL "general")
			_clear_old_libraries_state()
		else("${lib}" STREQUAL "debug")
			# Expecting normal library
			if(is_debug)
				list(APPEND out_list "$<$<CONFIG:Debug>:${lib}>" "$<$<CONFIG:FastDebug>:${lib}>")
			elseif(is_optimized)
				list(APPEND out_list "$<$<CONFIG:Release>:${lib}>")
			else(is_debug)
				list(APPEND out_list "${lib}")
			endif(is_debug)
		endif("${lib}" STREQUAL "debug")
	endforeach(lib)

	set(CONVERTED_LIBRARIES ${out_list} PARENT_SCOPE)
endfunction(CONVERT_OLD_LIBRARIES)

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

function(set_precompiled_header _target _headerPath)
	if (COMMAND target_precompile_headers)
		target_compile_definitions(${_target} PRIVATE CMAKE_PCH)
		target_precompile_headers(${_target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${_headerPath}>")
	else()
		if (ENABLE_COTIRE)
			message("${_headerPath}")
			set_target_properties(${_target} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${_headerPath}")

			# Disable unity build as it doesn't work well for us
			set_target_properties(${_target} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)

			cotire(${_target})
		else()
			if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
				target_compile_options(${_target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-include${_headerPath}>")
			elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
				target_compile_options(${_target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/FI ${_headerPath}>")
			elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
				target_compile_options(${_target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-include${_headerPath}>")
			elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
				target_compile_options(${_target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-include${_headerPath}>")
			ELSE()
				MESSAGE("Unknown compiler for global includes. This will probably break the build.")
			ENDIF()
		endif()
	endif()
endfunction()

macro(add_target_copy_files)
	foreach(file ${ARGN})
		if (IS_DIRECTORY "${file}")
			INSTALL(DIRECTORY ${file}
					DESTINATION ${LIBRAY_DESTINATION}
					)
		else()
			INSTALL(FILES ${file}
					DESTINATION ${LIBRAY_DESTINATION}
					)
		endif()

		SET(TARGET_COPY_FILES ${TARGET_COPY_FILES} ${file} CACHE INTERNAL "" FORCE)
	endforeach()
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
	SET(CMAKE_REQUIRED_LINK_OPTIONS "${_flag}")
	CHECK_C_COMPILER_FLAG("" ${_out_var})
endfunction(check_linker_flag)

# Suppresses warnings for the specified target
function(suppress_warnings _target)
    if (MSVC)
        target_compile_options(${_target} PRIVATE "/W0")
	else()
        # Assume everything else uses GCC style options
		target_compile_options(${_target} PRIVATE "-w")
    endif()
endfunction(suppress_warnings)

# Suppresses warnings for the specified files
function(suppress_file_warnings)
	if (MSVC)
		set_source_files_properties(
				${ARGN}
			PROPERTIES
				COMPILE_FLAGS "/W0")
	else()
		# Assume everything else uses GCC style options
		set_source_files_properties(
				${ARGN}
			PROPERTIES
				COMPILE_FLAGS "-w")
	endif()
endfunction(suppress_file_warnings)


function(list_target_dependencies _target _out_var)
	set(out_list)
	set(work_libs ${_target})
	list(LENGTH work_libs libs_length)

	while(libs_length GREATER 0)
		list(GET work_libs 0 current)
		list(REMOVE_AT work_libs 0)
		list(APPEND out_list ${current})

		get_target_property(current_libs ${current} INTERFACE_LINK_LIBRARIES)

		if(current_libs)
			foreach(lib ${current_libs})
				if (TARGET ${lib})
					list(APPEND work_libs ${lib})
				endif (TARGET ${lib})
			endforeach(lib)
		endif(current_libs)

		list(LENGTH work_libs libs_length)
	endwhile(libs_length GREATER 0)

	list(REMOVE_DUPLICATES out_list)
	set(${_out_var} ${out_list} PARENT_SCOPE)
endfunction(list_target_dependencies)

macro(add_file_folder FOLDER_NAME)
	string(MAKE_C_IDENTIFIER "${FOLDER_NAME}" VARIABLE_NAME)
	set(files_${VARIABLE_NAME} ${files_${VARIABLE_NAME}} ${ARGN})

	string(REPLACE "/" "\\" FIXED_FOLDER_FILE "${FOLDER_NAME}")

	source_group("${FIXED_FOLDER_FILE}" FILES ${ARGN})
	set(source_files ${source_files} ${ARGN})
endmacro(add_file_folder)
