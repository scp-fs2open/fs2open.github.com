
add_library(freetype INTERFACE)

if(PLATFORM_WINDOWS OR PLATFORM_MAC)
	# We use prebuilt binaries for windows and mac
	get_prebuilt_path(PREBUILT_PATH)
	set(FREETYPE_ROOT_DIR "${PREBUILT_PATH}/freetype")

	set(SEARCH_PATH "${FREETYPE_ROOT_DIR}/lib")

	target_include_directories(freetype INTERFACE "${FREETYPE_ROOT_DIR}/include")

if(PLATFORM_WINDOWS)
	target_link_libraries(freetype INTERFACE "${SEARCH_PATH}/freetype281.lib")
	set(dll_name "${SEARCH_PATH}/freetype281.dll")
	add_target_copy_files("${dll_name}")
else()
	# mac freetype build uses only static linking
	target_link_libraries(freetype INTERFACE "${SEARCH_PATH}/libfreetype.a")
endif(PLATFORM_WINDOWS)

else()
	find_package(Freetype REQUIRED)

	target_include_directories(freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
	target_link_libraries(freetype INTERFACE ${FREETYPE_LIBRARIES})
endif()
