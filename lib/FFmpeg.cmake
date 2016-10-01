
add_library(ffmpeg_defs INTERFACE)

IF(PLATFORM_WINDOWS)
    get_prebuilt_path(PREBUILT_PATH)
    set(FFMPEG_ROOT_DIR "${PREBUILT_PATH}/ffmpeg")

    macro(add_av_lib name libname)
        add_library(${name} SHARED IMPORTED GLOBAL)

        set(include_dir "${FFMPEG_ROOT_DIR}/include/lib${name}")
        set(dll_name "${FFMPEG_ROOT_DIR}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}${libname}${CMAKE_SHARED_LIBRARY_SUFFIX}")
        set(lib_name "${FFMPEG_ROOT_DIR}/lib/${CMAKE_IMPORT_LIBRARY_PREFIX}${name}${CMAKE_IMPORT_LIBRARY_SUFFIX}")
        if (NOT EXISTS "${dll_name}")
            set(dll_name "${FFMPEG_ROOT_DIR}/bin/${libname}${CMAKE_SHARED_LIBRARY_SUFFIX}")
        endif()
        if (NOT EXISTS "${lib_name}")
            set(lib_name "${FFMPEG_ROOT_DIR}/bin/${CMAKE_IMPORT_LIBRARY_PREFIX}${name}${CMAKE_IMPORT_LIBRARY_SUFFIX}")
        endif()

        if (NOT IS_DIRECTORY "${include_dir}")
            message("Couldn't find include directory for ${name} in \"${FFMPEG_ROOT_DIR}\".")
        endif ()

        set_target_properties(${name} PROPERTIES
            IMPORTED_LOCATION "${dll_name}"
            IMPORTED_IMPLIB "${lib_name}"
        )

        set_target_properties(${name} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_ROOT_DIR}/include"
        )

        add_target_copy_files("${dll_name}")
    endmacro(add_av_lib)

    add_av_lib(avcodec avcodec-57)
    add_av_lib(avformat avformat-57)
    add_av_lib(avutil avutil-55)
    add_av_lib(swscale swscale-4)

    add_av_lib(swresample swresample-2)

    add_library(ffmpeg INTERFACE)
    target_link_libraries(ffmpeg INTERFACE avcodec)
    target_link_libraries(ffmpeg INTERFACE avformat)
    target_link_libraries(ffmpeg INTERFACE avutil)
    target_link_libraries(ffmpeg INTERFACE swscale)
    target_link_libraries(ffmpeg INTERFACE swresample)
ELSE(WIN32)
    option(FFMPEG_USE_PRECOMPILED "Use precompiled version of FFmpeg. If disabled the system libraries will be used." OFF)

    # CMake can't check for ffmpeg so we'll just use PkgConfig
    INCLUDE(FindPkgConfig REQUIRED)
    INCLUDE(util)

    set(USING_PREBUILT_LIBS FALSE)
    set(FFMPEG_PATH)
    if (FFMPEG_USE_PRECOMPILED)
        get_prebuilt_path(PREBUILT_PATH)
        set(FFMPEG_PATH "${PREBUILT_PATH}/ffmpeg")
        set(USING_PREBUILT_LIBS TRUE)
    else()
        # Check if we have ffmpeg
        PKG_SEARCH_MODULE(avcodec "libavcodec")
        if (NOT avcodec_FOUND)
            message("FFmpeg libraries could not be found. Using prebuilt libraries...")

            get_prebuilt_path(PREBUILT_PATH)
            set(FFMPEG_PATH "${PREBUILT_PATH}/ffmpeg")
            set(USING_PREBUILT_LIBS TRUE)
        endif()
    endif()

    if (USING_PREBUILT_LIBS)
        message(STATUS "Using pre-built LGPL FFmpeg libraries.")
    else()
        message(STATUS "Using system FFmpeg libraries. Don't distribute these FSO binaries if these are the GPL libraries!")
    endif()

    # I need to do some workarounds to be able to use our prebuilt LGPL builds
    # so we can distribute that in releases
    # The ffmpeg libs specify a SONAME that is different from the name of the file
    # so we also need to copy and install that symlink in addition to the actual library
    macro(add_av_lib name)
        if (NOT USING_PREBUILT_LIBS)
            PKG_SEARCH_MODULE(${name} REQUIRED "lib${name}")

            PKG_CONFIG_LIB_RESOLVE(${name} ${name}_LIB)

            ADD_IMPORTED_LIB("${name}" "${${name}_INCLUDE_DIRS}" "${${name}_LIB}" SHARED)
        else()
            # Use our libraries
            find_library(${name}_LOCATION ${name}
                PATHS "${FFMPEG_PATH}/lib"
                NO_DEFAULT_PATH)

            file(GLOB ${name}_LIBS "${FFMPEG_PATH}/lib/lib${name}*")

            get_filename_component(FULL_LIB_PATH "${${name}_LOCATION}" REALPATH)

            ADD_IMPORTED_LIB("${name}" "${FFMPEG_PATH}/include" "${FULL_LIB_PATH}")

            add_target_copy_files("${${name}_LIBS}")
        endif()
    endmacro(add_av_lib)

    add_av_lib(avcodec)
    add_av_lib(avformat)
    add_av_lib(avutil)
    add_av_lib(swresample)
    add_av_lib(swscale)

    add_library(ffmpeg INTERFACE)
    target_compile_definitions(ffmpeg INTERFACE __STDC_CONSTANT_MACROS)

    target_link_libraries(ffmpeg INTERFACE avcodec)
    target_link_libraries(ffmpeg INTERFACE avformat)
    target_link_libraries(ffmpeg INTERFACE avutil)
    target_link_libraries(ffmpeg INTERFACE swscale)
    target_link_libraries(ffmpeg INTERFACE swresample)
ENDIF()
