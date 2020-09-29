
set(_EMBEDD_FILE_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(target_embed_files _target)
    set(options)
    set(oneValueArgs RELATIVE_TO PATH_TYPE_PREFIX)
    set(multiValueArgs FILES)
    cmake_parse_arguments(EMBED "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT DEFINED EMBED_RELATIVE_TO)
        set(EMBED_RELATIVE_TO "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    if (NOT IS_ABSOLUTE "${EMBED_RELATIVE_TO}")
        get_filename_component(EMBED_RELATIVE_TO "${EMBED_RELATIVE_TO}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    foreach (file ${EMBED_FILES})
        get_filename_component(_absoluteFile "${file}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

        file(RELATIVE_PATH _pathType "${EMBED_RELATIVE_TO}" "${_absoluteFile}")
        get_filename_component(_pathType ${_pathType} DIRECTORY)
        if (DEFINED EMBED_PATH_TYPE_PREFIX)
            if (_pathType)
                set(_pathType "${EMBED_PATH_TYPE_PREFIX}/${_pathType}")
            else()
                # Avoid trailing / when we are in the same directory
                set(_pathType "${EMBED_PATH_TYPE_PREFIX}")
            endif()
        endif()

        if (NOT _pathType)
            # If it is still empty then we need to work around yet another annoying CMake behavior where it ignores
            # "empty" property strings
            set(_pathType ".")
        endif()

        set_property(TARGET ${_target} APPEND PROPERTY EMBEDDED_FILES "${_absoluteFile}")
        set_property(TARGET ${_target} APPEND PROPERTY EMBEDDED_FILES_BASE_DIR "${EMBED_RELATIVE_TO}")
        set_property(TARGET ${_target} APPEND PROPERTY EMBEDDED_FILES_PATH_TYPE "${_pathType}")
    endforeach()
endfunction()

function(handle_embedded_files _target)
    get_target_property(_filesToEmbed ${_target} EMBEDDED_FILES)
    get_target_property(_baseDirs ${_target} EMBEDDED_FILES_BASE_DIR)
    get_target_property(_pathTypes ${_target} EMBEDDED_FILES_PATH_TYPE)

    SET(DEF_OUT_FILES)

    set(INCLUDE_LIST)
    set(ARRAY_ELEMENTS)

    list(LENGTH _filesToEmbed _numFiles)

    FOREACH(i RANGE ${_numFiles})
        # Why must CMake always do the unexpected thing? The foreach variant includes the last index as opposed to every
        # other sensible language which does not include it...
        if (i GREATER_EQUAL _numFiles)
            break()
        endif()

        list(GET _filesToEmbed ${i} _filePath)
        list(GET _baseDirs ${i} _baseDir)
        list(GET _pathTypes ${i} _pathType)

        if (_pathType STREQUAL ".")
            # Handle the workaround we had to do for empty path types
            set(_pathType "")
        endif()

        file(RELATIVE_PATH _relativeFilePath "${_baseDir}" "${_filePath}")
        SET(_outputBasePath "${CMAKE_CURRENT_BINARY_DIR}/embedded_files/${_relativeFilePath}")

        # For some reason this is needed...
        get_filename_component(_outputDirectory ${_outputBasePath} DIRECTORY)
        file(MAKE_DIRECTORY ${_outputDirectory})

        # Retrieve the path type name from the relative path
        file(TO_NATIVE_PATH "${_pathType}" _pathType)
        if (MINGW)
            # There is a bug in CMake where it thinks MinGW uses forward slashes for paths but we need backslashes for Windows builds
            string(REPLACE "/" "\\" _pathType "${_pathType}")
        endif()
        # Properly escape back slashes in strings
        string(REPLACE "\\" "\\\\" _pathType "${_pathType}")

        get_filename_component(FILE_NAME "${_relativeFilePath}" NAME)

        string(MAKE_C_IDENTIFIER "${_relativeFilePath}" FIELD_NAME)

        set(HEADER_FILE "${_outputBasePath}.h")
        set(SOURCE_FILE "${_outputBasePath}.cpp")

        set(ALL_OUTPUTS "${HEADER_FILE}" "${SOURCE_FILE}")
        set(FIELD_NAME "Embedded_${FIELD_NAME}")

        set(INCLUDE_LIST "${INCLUDE_LIST}\n#include \"${HEADER_FILE}\"")
        set(ARRAY_ELEMENTS "${ARRAY_ELEMENTS}\n\t{ \"${_pathType}\", \"${FILE_NAME}\" , ${FIELD_NAME} , ${FIELD_NAME}_size },")

        add_custom_command(
            OUTPUT ${ALL_OUTPUTS}
            COMMAND embedfile "${_filePath}" "${_outputBasePath}" "${FIELD_NAME}"
            MAIN_DEPENDENCY "${_filePath}"
            COMMENT "Generating string file for ${_filePath}"
        )

        list(APPEND DEF_OUT_FILES ${ALL_OUTPUTS})
    ENDFOREACH()

    configure_file("${_EMBEDD_FILE_BASE_DIR}/embedded_files.inc.in" "${CMAKE_CURRENT_BINARY_DIR}/embedded_files/embedded_files.inc")
    target_include_directories(${_target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/embedded_files")
    target_sources(${_target} PRIVATE ${DEF_OUT_FILES})
    source_group("Generated Files\\Embedded Files" FILES ${DEF_OUT_FILES})
endfunction()
