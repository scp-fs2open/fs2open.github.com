cmake_minimum_required(VERSION 3.16)
project(ffx_portable LANGUAGES CXX)

set(FFX_PORTABLE_BACKEND "AUTO" CACHE STRING "How FSR is provided: AUTO, DYNAMIC (load AMD's library at runtime on Windows), SHIM (load AMD's Windows DLL on Linux) or DIRECT (link AMD's library)")
set_property(CACHE FFX_PORTABLE_BACKEND PROPERTY STRINGS AUTO DYNAMIC SHIM DIRECT UNAVAILABLE)
set(FFX_PORTABLE_AMD_LIBRARY "" CACHE FILEPATH "DIRECT: AMD library to link. Empty uses AMD's Windows import library")
set(FFX_PORTABLE_AMD_RUNTIME_FILES "" CACHE STRING "DIRECT: runtime files to deploy. Empty uses AMD's Windows DLLs")
option(FFX_PORTABLE_BUILD_TESTS "Build the ffx-portable smoke client" OFF)

# Mirrors the root of AMD's SDK, because the file list in AMD's license.md uses paths relative to that root.
set(FFX_PORTABLE_AMD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/amd)
set(FFX_PORTABLE_AMD_INCLUDE_DIR ${FFX_PORTABLE_AMD_DIR}/Kits)

# AMD FSR SDK 2.4.0. The hashes are what's trusted, so a mirror can be used without weakening anything.
set(FFX_PORTABLE_DOWNLOAD_URL "https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/7539ae8b6568ad29ed78a753404351daf3b5cca7/Kits/FidelityFX/signedbin"
    CACHE STRING "Where AMD's binaries are downloaded from")
set(FFX_PORTABLE_UPSCALER_SHA256 "4a7e895af6e0b615d40a84d59eb8c2b1054ad91cdd0c739567940270d51bdab7")
set(FFX_PORTABLE_LOADER_SHA256 "38a56a01fb3565c0757d9f5c1e4e69de4b65e5f2e5769b323b200dc1f142bab9")
set(FFX_PORTABLE_LOADER_LIB_SHA256 "7f0b0186c7c207779bee56f44ee6a7881b9c6ebb32038a95e8b960ccce9b3689")

set(FFX_PORTABLE_CACHE_DIR "${CMAKE_BINARY_DIR}/ffx-portable-cache" CACHE PATH "Where downloaded AMD binaries are kept between configures")

# Cached by hash, so different versions coexist and an existing file only needs verifying, which works offline.
function(ffx_portable_fetch name expected_hash out_var)
    set(cached "${FFX_PORTABLE_CACHE_DIR}/${expected_hash}/${name}")
    if(EXISTS "${cached}")
        file(SHA256 "${cached}" actual_hash)
        if(actual_hash STREQUAL expected_hash)
            set(${out_var} "${cached}" PARENT_SCOPE)
            return()
        endif()
        message(STATUS "ffx-portable: discarding corrupt cached ${name}")
        file(REMOVE "${cached}")
    endif()

    message(STATUS "ffx-portable: downloading ${name}")
    string(RANDOM LENGTH 12 suffix)
    set(partial "${cached}.${suffix}.part")
    # Retried so a network blip doesn't fail the configure. A download that keeps failing still does: the build
    # must not silently go ahead without what it was configured with.
    set(attempts 5)
    foreach(attempt RANGE 1 ${attempts})
        file(DOWNLOAD "${FFX_PORTABLE_DOWNLOAD_URL}/${name}" "${partial}" TLS_VERIFY ON STATUS status)
        list(GET status 0 status_code)
        list(GET status 1 status_text)
        if(status_code EQUAL 0)
            break()
        endif()
        file(REMOVE "${partial}")
        message(STATUS "ffx-portable: downloading ${name} failed (attempt ${attempt} of ${attempts}): ${status_text}")
    endforeach()
    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "ffx-portable: could not download ${name}: ${status_text}\n"
            "URL: ${FFX_PORTABLE_DOWNLOAD_URL}/${name}\n"
            "To build offline, place the file with SHA-256 ${expected_hash} at ${cached}\n"
            "To build without FSR instead, configure with -DFSO_BUILD_WITH_FFX=OFF")
    endif()
    file(SHA256 "${partial}" actual_hash)
    if(NOT actual_hash STREQUAL expected_hash)
        file(REMOVE "${partial}")
        message(FATAL_ERROR "ffx-portable: ${name} from ${FFX_PORTABLE_DOWNLOAD_URL} has the wrong SHA-256\n"
            "Expected: ${expected_hash}\nActual: ${actual_hash}")
    endif()
    file(RENAME "${partial}" "${cached}")
    set(${out_var} "${cached}" PARENT_SCOPE)
endfunction()

function(ffx_portable_fetch_windows_runtime out_var)
    ffx_portable_fetch(amd_fidelityfx_loader_vk.dll ${FFX_PORTABLE_LOADER_SHA256} loader)
    ffx_portable_fetch(amd_fidelityfx_upscaler_vk.dll ${FFX_PORTABLE_UPSCALER_SHA256} upscaler)
    set(${out_var} "${loader};${upscaler}" PARENT_SCOPE)
endfunction()

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" FFX_PORTABLE_ARCH)
if(FFX_PORTABLE_ARCH MATCHES "^(x86_64|amd64)$" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FFX_PORTABLE_X64 ON)
else()
    set(FFX_PORTABLE_X64 OFF)
endif()

set(FFX_PORTABLE_SELECTED "${FFX_PORTABLE_BACKEND}")
if(FFX_PORTABLE_SELECTED STREQUAL "AUTO")
    if(WIN32)
        set(FFX_PORTABLE_SELECTED DYNAMIC)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND FFX_PORTABLE_X64)
        set(FFX_PORTABLE_SELECTED SHIM)
    else()
        set(FFX_PORTABLE_SELECTED UNAVAILABLE)
    endif()
endif()

set(FFX_PORTABLE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(FFX_PORTABLE_SELECTED STREQUAL "DIRECT")
    if(FFX_PORTABLE_AMD_LIBRARY)
        set(FFX_PORTABLE_LINK ${FFX_PORTABLE_AMD_LIBRARY})
    elseif(WIN32)
        ffx_portable_fetch(amd_fidelityfx_loader_vk.lib ${FFX_PORTABLE_LOADER_LIB_SHA256} FFX_PORTABLE_LINK)
    else()
        message(FATAL_ERROR "FFX_PORTABLE_BACKEND=DIRECT needs FFX_PORTABLE_AMD_LIBRARY on this platform")
    endif()
    if(FFX_PORTABLE_AMD_RUNTIME_FILES)
        set(FFX_PORTABLE_RUNTIME ${FFX_PORTABLE_AMD_RUNTIME_FILES})
    elseif(WIN32)
        ffx_portable_fetch_windows_runtime(FFX_PORTABLE_RUNTIME)
    endif()

    add_library(ffx_portable INTERFACE)
    target_include_directories(ffx_portable INTERFACE ${FFX_PORTABLE_INCLUDES})
    # SYSTEM: warnings in AMD's headers must not fail a client's -Werror build.
    target_include_directories(ffx_portable SYSTEM INTERFACE ${FFX_PORTABLE_AMD_INCLUDE_DIR})
    target_compile_definitions(ffx_portable INTERFACE FFX_PORTABLE_DIRECT=1)
    target_link_libraries(ffx_portable INTERFACE ${FFX_PORTABLE_LINK})
elseif(FFX_PORTABLE_SELECTED STREQUAL "DYNAMIC")
    if(NOT WIN32)
        message(FATAL_ERROR "FFX_PORTABLE_BACKEND=DYNAMIC only supports Windows")
    endif()
    add_library(ffx_portable STATIC src/windows/ffx_exports.cpp)
    ffx_portable_fetch_windows_runtime(FFX_PORTABLE_RUNTIME)
elseif(FFX_PORTABLE_SELECTED STREQUAL "SHIM")
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT FFX_PORTABLE_X64)
        message(FATAL_ERROR "FFX_PORTABLE_BACKEND=SHIM only supports x86-64 Linux")
    endif()
    find_package(Threads REQUIRED)
    add_library(ffx_portable STATIC
        src/linux/ffx_exports.cpp
        src/linux/pe_image.cpp
        src/linux/sha256.cpp
        src/linux/win32_shims.cpp)
    target_link_libraries(ffx_portable PUBLIC ${CMAKE_DL_LIBS} Threads::Threads)
    target_compile_definitions(ffx_portable PRIVATE FFX_PORTABLE_UPSCALER_SHA256="${FFX_PORTABLE_UPSCALER_SHA256}")
    ffx_portable_fetch(amd_fidelityfx_upscaler_vk.dll ${FFX_PORTABLE_UPSCALER_SHA256} FFX_PORTABLE_RUNTIME)
elseif(FFX_PORTABLE_SELECTED STREQUAL "UNAVAILABLE")
    add_library(ffx_portable STATIC src/unavailable.cpp)
else()
    message(FATAL_ERROR "Unknown FFX_PORTABLE_BACKEND: ${FFX_PORTABLE_BACKEND}")
endif()

message(STATUS "ffx-portable backend: ${FFX_PORTABLE_SELECTED}")

if(NOT FFX_PORTABLE_SELECTED STREQUAL "DIRECT")
    target_include_directories(ffx_portable PUBLIC ${FFX_PORTABLE_INCLUDES})
    target_include_directories(ffx_portable SYSTEM PUBLIC ${FFX_PORTABLE_AMD_INCLUDE_DIR})
    set_target_properties(ffx_portable PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
    if(NOT WIN32)
        # ffx_api.h marks its functions __declspec(dllexport) unconditionally; AMD's headers stay unmodified.
        target_compile_options(ffx_portable PUBLIC "-D__declspec(x)=")
    endif()
    if(TARGET Vulkan::Headers)
        target_link_libraries(ffx_portable PUBLIC Vulkan::Headers)
    endif()
endif()

# Stored globally because the functions below run in the client's CMake scope, not this one.
set_property(GLOBAL PROPERTY FFX_PORTABLE_RUNTIME_FILES "${FFX_PORTABLE_RUNTIME}")
set_property(GLOBAL PROPERTY FFX_PORTABLE_NOTICE_FILES
    ${FFX_PORTABLE_AMD_DIR}/Kits/FidelityFX/docs/license.md
    ${FFX_PORTABLE_AMD_DIR}/3rdpartynotice.md)

# For clients with their own deployment logic. Runtime files belong next to the executable, or in a "lib"
# directory beside it or one level up on Linux. Notice files must ship with any redistribution.
function(ffx_portable_get_runtime_files out_var)
    get_property(files GLOBAL PROPERTY FFX_PORTABLE_RUNTIME_FILES)
    set(${out_var} "${files}" PARENT_SCOPE)
endfunction()

function(ffx_portable_get_notice_files out_var)
    get_property(files GLOBAL PROPERTY FFX_PORTABLE_NOTICE_FILES)
    set(${out_var} "${files}" PARENT_SCOPE)
endfunction()

# Copies runtime files next to the executable and notices to licenses/ffx-portable/ after each build. With
# INSTALL_DESTINATION, `cmake --install` does the same relative to that directory.
function(ffx_portable_deploy target)
    cmake_parse_arguments(ARG "" "INSTALL_DESTINATION" "" ${ARGN})
    ffx_portable_get_runtime_files(runtime)
    ffx_portable_get_notice_files(notices)
    set(output_dir "$<TARGET_FILE_DIR:${target}>")
    if(runtime)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${runtime} "${output_dir}"
            VERBATIM)
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}/licenses/ffx-portable"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${notices} "${output_dir}/licenses/ffx-portable"
        VERBATIM)
    if(ARG_INSTALL_DESTINATION)
        if(runtime)
            install(FILES ${runtime} DESTINATION "${ARG_INSTALL_DESTINATION}")
        endif()
        install(FILES ${notices} DESTINATION "${ARG_INSTALL_DESTINATION}/licenses/ffx-portable")
    endif()
endfunction()

if(FFX_PORTABLE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
