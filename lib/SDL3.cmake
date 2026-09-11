
if(PLATFORM_WINDOWS)
    get_prebuilt_path(PREBUILT_PATH)
    set(SDL3_ROOT_DIR "${PREBUILT_PATH}/sdl3")

    if(MSVC)
        add_library(sdl3 SHARED IMPORTED GLOBAL)
        set_target_properties(sdl3
            PROPERTIES
            IMPORTED_LOCATION "${SDL3_ROOT_DIR}/lib/SDL3.dll"
            IMPORTED_IMPLIB "${SDL3_ROOT_DIR}/lib/SDL3.lib"
        )
    else(MSVC) # MINGW
        add_library(sdl3 INTERFACE)
        target_link_libraries(sdl3 INTERFACE "${SDL3_ROOT_DIR}/lib/SDL3.dll")
    endif(MSVC)

    set_target_properties(sdl3 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL3_ROOT_DIR}/include")

    add_target_copy_files("${SDL3_ROOT_DIR}/lib/SDL3.dll")
    install(FILES "${SDL3_ROOT_DIR}/README.md"
        DESTINATION ${BINARY_DESTINATION}
    )
elseif(PLATFORM_MAC)
    message(STATUS "Using pre-built SDL3 framework.")

    add_library(sdl3 INTERFACE)
    get_prebuilt_path(PREBUILT_PATH)
    unset(SDL3_LIBRARY CACHE)
    find_library(SDL3_LIBRARY SDL3 PATHS "${PREBUILT_PATH}" NO_DEFAULT_PATH)

    target_link_libraries(sdl3 INTERFACE "${SDL3_LIBRARY}")
    target_include_directories(sdl3 SYSTEM INTERFACE "${SDL3_LIBRARY}/Headers")

    add_target_copy_files("${SDL3_LIBRARY}")
else()
    option(SDL3_USE_PRECOMPILED "Use precompiled version of SDL3. If disabled the system libraries will be used." OFF)

    include(util)

    set(USING_PREBUILT_LIBS FALSE)
    set(SDL3_ROOT_DIR)

    if(SDL3_USE_PRECOMPILED)
        get_prebuilt_path(PREBUILT_PATH)
        set(SDL3_ROOT_DIR "${PREBUILT_PATH}/sdl3")
        set(USING_PREBUILT_LIBS TRUE)
    else()
        # CMake can't check for SDL3 so we'll just use PkgConfig
        find_package(PkgConfig)

        PKG_SEARCH_MODULE(SDL3 "sdl3 >= 3.2.14")
        if(SDL3_FOUND)
            PKG_CONFIG_LIB_RESOLVE(SDL3 SDL3_LIB)
            ADD_IMPORTED_LIB(sdl3 "${SDL3_INCLUDE_DIRS}" "${SDL3_LIB}")
        else()
            message("Suitable SDL3 library could not be found. Using prebuilt library...")

            get_prebuilt_path(PREBUILT_PATH)
            set(SDL3_ROOT_DIR "${PREBUILT_PATH}/sdl3")
            set(USING_PREBUILT_LIBS TRUE)
        endif()
    endif()

    if(USING_PREBUILT_LIBS)
        message(STATUS "Using pre-built SDL3 library.")

        unset(SDL3_LOCATION CACHE)
        find_library(SDL3_LOCATION NAMES SDL3 SDL3-3.0 PATHS "${SDL3_ROOT_DIR}/lib" NO_DEFAULT_PATH)

        get_filename_component(FULL_LIB_PATH "${SDL3_LOCATION}" REALPATH)
        ADD_IMPORTED_LIB(sdl3 "${SDL3_ROOT_DIR}/include" "${FULL_LIB_PATH}")

        file(GLOB SDL3_LIBS "${SDL3_ROOT_DIR}/lib/libSDL3*")
        add_target_copy_files("${SDL3_LIBS}")
    endif()
endif()
