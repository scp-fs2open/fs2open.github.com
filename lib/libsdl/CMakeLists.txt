
IF(WIN32)
	IF(IS_64BIT)
		# 64-bit
		set(ARCH "x64")
	ELSE()
		# 32-bit
		set(ARCH "x86")
	ENDIF()

	add_library(sdlmain INTERFACE)
	IF(MSVC)
		add_library(sdl2 SHARED IMPORTED GLOBAL)
		set_target_properties(sdl2
			PROPERTIES
				IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH}/SDL2.dll"
				IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH}/SDL2.lib"
		)
		target_link_libraries(sdlmain INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH}/SDL2main.lib")
	ELSE(MSVC) #MINGW
		add_library(sdl2 INTERFACE)
		target_link_libraries(sdl2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH}/SDL2.dll")
		target_link_libraries(sdlmain INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH}/libSDL2main.a")
	ENDIF(MSVC)
	set_target_properties(sdl2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")

	add_target_copy_files("${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH}/SDL2.dll")
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README-SDL.txt"
			DESTINATION ${BINARY_DESTINATION}
	)
elseif(PLATFORM_MAC)
	add_library(sdl2 INTERFACE)
    get_prebuilt_path(PREBUILT_PATH)
	unset(SDL2_LIBRARY CACHE)
	find_library(SDL2_LIBRARY SDL2 PATHS "${PREBUILT_PATH}" NO_DEFAULT_PATH)

	target_link_libraries(sdl2 INTERFACE "${SDL2_LIBRARY}")
	target_include_directories(sdl2 INTERFACE "${SDL2_LIBRARY}/Headers")

	add_target_copy_files("${SDL2_LIBRARY}")
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README-SDL.txt"
			DESTINATION ${BINARY_DESTINATION}
	)
ELSE()
	# CMake can't check for SDL2 so we'll just use PkgConfig
	find_package(PkgConfig)
	INCLUDE(util)

	PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

	PKG_CONFIG_LIB_RESOLVE(SDL2 SDL2_LIB)
	ADD_IMPORTED_LIB(sdl2 "${SDL2_INCLUDE_DIRS}" "${SDL2_LIB}")
ENDIF()
