
if(PLATFORM_WINDOWS)
    add_library(sdlmain INTERFACE)

    get_prebuilt_path(PREBUILT_PATH)
    set(SDL2_ROOT_DIR "${PREBUILT_PATH}/sdl2")

    if(MSVC)
        add_library(sdl2 SHARED IMPORTED GLOBAL)
        set_target_properties(sdl2
            PROPERTIES
            IMPORTED_LOCATION "${SDL2_ROOT_DIR}/lib/SDL2.dll"
            IMPORTED_IMPLIB "${SDL2_ROOT_DIR}/lib/SDL2.lib"
        )
        target_link_libraries(sdlmain INTERFACE "${SDL2_ROOT_DIR}/lib/SDL2main.lib")
    else(MSVC) # MINGW
        add_library(sdl2 INTERFACE)
        target_link_libraries(sdl2 INTERFACE "${SDL2_ROOT_DIR}/lib/SDL2.dll")
        target_link_libraries(sdlmain INTERFACE "${SDL2_ROOT_DIR}/lib/libSDL2main.a")
    endif(MSVC)

    set_target_properties(sdl2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL2_ROOT_DIR}/include")

    add_target_copy_files("${SDL2_ROOT_DIR}/lib/SDL2.dll")
    install(FILES "${SDL2_ROOT_DIR}/README-SDL.txt"
        DESTINATION ${BINARY_DESTINATION}
    )
elseif(PLATFORM_MAC)
    message(STATUS "Using pre-built SDL2 framework.")

    add_library(sdl2 INTERFACE)
    get_prebuilt_path(PREBUILT_PATH)
    unset(SDL2_LIBRARY CACHE)
    find_library(SDL2_LIBRARY SDL2 PATHS "${PREBUILT_PATH}" NO_DEFAULT_PATH)

    target_link_libraries(sdl2 INTERFACE "${SDL2_LIBRARY}")
    target_include_directories(sdl2 SYSTEM INTERFACE "${SDL2_LIBRARY}/Headers")

    add_target_copy_files("${SDL2_LIBRARY}")
else()
    option(SDL2_USE_PRECOMPILED "Use precompiled version of SDL2. If disabled the system libraries will be used." OFF)

    include(util)

    set(USING_PREBUILT_LIBS FALSE)
    set(SDL2_ROOT_DIR)

    if(SDL2_USE_PRECOMPILED)
        get_prebuilt_path(PREBUILT_PATH)
        set(SDL2_ROOT_DIR "${PREBUILT_PATH}/sdl2")
        set(USING_PREBUILT_LIBS TRUE)
    else()
        # CMake can't check for SDL2 so we'll just use PkgConfig
        find_package(PkgConfig)

        PKG_SEARCH_MODULE(SDL2 "sdl2 >= 2.26.5")
        if(SDL2_FOUND)
            PKG_CONFIG_LIB_RESOLVE(SDL2 SDL2_LIB)
            ADD_IMPORTED_LIB(sdl2 "${SDL2_INCLUDE_DIRS}" "${SDL2_LIB}")
        else()
            message("Suitable SDL2 library could not be found. Using prebuilt library...")

            get_prebuilt_path(PREBUILT_PATH)
            set(SDL2_ROOT_DIR "${PREBUILT_PATH}/sdl2")
            set(USING_PREBUILT_LIBS TRUE)
        endif()
    endif()

    if(USING_PREBUILT_LIBS)
        message(STATUS "Using pre-built SDL2 library.")
        unset(SDL2_LOCATION CACHE)
        if(NOT ANDROID)
        	find_library(SDL2_LOCATION NAMES SDL2 SDL2-2.0 PATHS "${SDL2_ROOT_DIR}/lib" NO_DEFAULT_PATH)
	
        	get_filename_component(FULL_LIB_PATH "${SDL2_LOCATION}" REALPATH)
        	ADD_IMPORTED_LIB(sdl2 "${SDL2_ROOT_DIR}/include" "${FULL_LIB_PATH}")
			message("${SDL2_LOCATION}")
        	file(GLOB SDL2_LIBS "${SDL2_ROOT_DIR}/lib/libSDL2-2*")
        	add_target_copy_files("${SDL2_LIBS}")
        else()
        	# workaround, find_library() was not finding anything
        	ADD_IMPORTED_LIB(sdl2 "${SDL2_ROOT_DIR}/include/SDL2" "${SDL2_ROOT_DIR}/lib/libSDL2.so")
        	file(GLOB SDL2_LIBS "${SDL2_ROOT_DIR}/lib/libSDL2.so")
        	add_target_copy_files("${SDL2_LIBS}")
        endif()

    endif()
endif()
