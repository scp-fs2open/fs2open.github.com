
add_library(freetype INTERFACE)

if(PLATFORM_WINDOWS)
	# We use prebuilt binaries for windows
	get_prebuilt_path(PREBUILT_PATH)
	set(FRRETYPE_ROOT_DIR "${PREBUILT_PATH}/freetype")

	set(SEARCH_PATH "${FRRETYPE_ROOT_DIR}/lib")

	set(dll_name "${SEARCH_PATH}/freetype281.dll")

	target_include_directories(freetype INTERFACE "${FRRETYPE_ROOT_DIR}/include")

	target_link_libraries(freetype INTERFACE "${SEARCH_PATH}/freetype281.lib")

	add_target_copy_files("${dll_name}")
else()
	find_package(Freetype REQUIRED)

	target_include_directories(freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
	target_link_libraries(freetype INTERFACE ${FREETYPE_LIBRARIES})
endif()
