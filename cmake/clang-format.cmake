option(ENABLE_CLANG_FORMAT "Enable source code formatting using clang-format." OFF)

find_program(
        CLANG_FORMAT_EXE
        NAMES "clang-format"
        DOC "Path to clang-format executable"
)

function(enable_clang_format target)
    # Not enabled in this configuration
endfunction()

if(NOT ENABLE_CLANG_FORMAT)
    return()
endif()

if(NOT EXISTS "${CLANG_FORMAT_EXE}")
    message(SEND_ERROR "clang-format: Could not find clang-format executable (value of variable is \"${CLANG_FORMAT_EXE}\".")
    return()
endif()

message(STATUS "clang-tidy: Using \"${CLANG_FORMAT_EXE}\"")

# This dummy target serves as the common dependency for all clang-format targets
add_custom_target(clang-format)

function(enable_clang_format target)
    get_target_property(TARGET_SOURCE_DIR ${target} SOURCE_DIR)
    get_target_property(TARGET_BINARY_DIR ${target} BINARY_DIR)
    get_target_property(TARGET_SOURCES ${target} SOURCES)

    set(RESOLVED_SOURCES)
    foreach (source ${TARGET_SOURCES})
        # This should match what CMake does
        if (IS_ABSOLUTE "${source}")
            set(ABSOLUTE_PATH "${source}")
        else()
            set(ABSOLUTE_PATH "${TARGET_SOURCE_DIR}/${source}")
        endif()

        file(RELATIVE_PATH SOURCE_RELATIVE "${TARGET_SOURCE_DIR}" "${ABSOLUTE_PATH}")

        get_source_file_property(LANGUAGE "${TARGET_SOURCE_DIR}/${source}" LANGUAGE)

        # This should exclude most generated files that are not relevant here
        if (NOT ("${SOURCE_RELATIVE}" MATCHES "^\\.\\.") AND "${source}" MATCHES "(\\.h|\\.cpp)$")
            set(RESOLVED_SOURCES ${RESOLVED_SOURCES} "${ABSOLUTE_PATH}")
        endif()
    endforeach ()

    add_custom_target(clang-format-${target}
            COMMAND ${CLANG_FORMAT_EXE} -i -sort-includes ${RESOLVED_SOURCES}
            COMMENT "Formatting code of ${target} using clang-format"
            VERBATIM)
    add_dependencies(clang-format clang-format-${target})
endfunction()

