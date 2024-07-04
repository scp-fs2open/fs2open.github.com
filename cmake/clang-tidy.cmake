option(ENABLE_CLANG_TIDY "Enable source code checking using clang-tidy" OFF)

find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
)

function(enable_clang_tidy target)
    # Not enabled in this configuration
endfunction()

if(NOT ENABLE_CLANG_TIDY)
    return()
endif()

if(NOT EXISTS "${CLANG_TIDY_EXE}")
    message(SEND_ERROR "clang-tidy: Could not find clang-tidy executable (value of variable is \"${CLANG_TIDY_EXE}\".")
    return()
endif()

if(CMAKE_VERSION VERSION_LESS "3.6.0")
    message(SEND_ERROR "Clang-tidy support requires CMake 3.6")
    return()
endif()

message(STATUS "clang-tidy: Using \"${CLANG_TIDY_EXE}\"")

function(enable_clang_tidy target)
    SET(TIDY_OPTIONS "${CLANG_TIDY_EXE}")
    if(FSO_FATAL_WARNINGS)
        set(TIDY_OPTIONS "${TIDY_OPTIONS};-warnings-as-errors=*")
    endif()

    set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${TIDY_OPTIONS}")
endfunction()

