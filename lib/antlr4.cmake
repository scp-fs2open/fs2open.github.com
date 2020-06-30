
if (MSVC)
    if (MSVC_USE_RUNTIME_DLL)
        set(WITH_STATIC_CRT OFF)
    else()
        set(WITH_STATIC_CRT ON)
    endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CLANG_USE_LIBCXX)
        set(WITH_LIBCXX ON)
    else()
        set(WITH_LIBCXX OFF)
    endif()
endif()

set(WITH_DEMO FALSE)
set(ANTLR4_INSTALL FALSE)

add_subdirectory(antlr4-cpp-runtime)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime/cmake" PARENT_SCOPE)
suppress_warnings(antlr4_static)

target_include_directories(antlr4_static PUBLIC "${CMAKE_CURRENT_LIST_DIR}/antlr4-cpp-runtime/runtime/src")

set_target_properties(antlr4_static
    PROPERTIES
    FOLDER "3rdparty/antlr-runtime"
)
