
if(PLATFORM_WINDOWS OR PLATFORM_MAC)
	add_library(freetype INTERFACE)

	# We use prebuilt binaries for windows and mac
	get_prebuilt_path(PREBUILT_PATH)
	set(FREETYPE_ROOT_DIR "${PREBUILT_PATH}/freetype")

	set(SEARCH_PATH "${FREETYPE_ROOT_DIR}/lib")

	if(PLATFORM_WINDOWS)
		set(LIBNAME "freetype*.dll")
	else()
		set(LIBNAME "libfreetype*")
	endif()

	target_include_directories(freetype INTERFACE "${FREETYPE_ROOT_DIR}/include")

	find_library(freetype_LOCATION
		NAMES freetype freetype281
		PATHS "${SEARCH_PATH}"
		NO_DEFAULT_PATH)

	file(GLOB freetype_LIBS "${SEARCH_PATH}/${LIBNAME}")

	get_filename_component(FULL_LIB_PATH "${freetype_LOCATION}" REALPATH)
	target_link_libraries(freetype INTERFACE "${FULL_LIB_PATH}")

	add_target_copy_files("${freetype_LIBS}")
else()
	option(FREETYPE_USE_PRECOMPILED "Use precompiled version of Freetype. If disabled the system libraries will be used." OFF)

	include(util)

	set(USING_PREBUILT_LIBS FALSE)
	set(FREETYPE_ROOT_DIR)

	if(FREETYPE_USE_PRECOMPILED)
		get_prebuilt_path(PREBUILT_PATH)
		set(FREETYPE_ROOT_DIR "${PREBUILT_PATH}/freetype")
		set(USING_PREBUILT_LIBS TRUE)
	else()
		find_package(Freetype)

		if(FREETYPE_FOUND)
			ADD_IMPORTED_LIB(freetype "${FREETYPE_INCLUDE_DIRS}" "${FREETYPE_LIBRARIES}")
		else()
			message("Freetype library could not be found. Using prebuilt library...")

			get_prebuilt_path(PREBUILT_PATH)
			set(FREETYPE_ROOT_DIR "${PREBUILT_PATH}/freetype")
			set(USING_PREBUILT_LIBS TRUE)
		endif()
	endif()

	if(USING_PREBUILT_LIBS)
		message(STATUS "Using pre-built Freetype library.")

		unset(FREETYPE_LOCATION CACHE)
		find_library(FREETYPE_LOCATION freetype PATHS "${FREETYPE_ROOT_DIR}/lib" NO_DEFAULT_PATH)

		get_filename_component(FULL_LIB_PATH "${FREETYPE_LOCATION}" REALPATH)
		ADD_IMPORTED_LIB(freetype "${FREETYPE_ROOT_DIR}/include" "${FULL_LIB_PATH}")

		file(GLOB FREETYPE_LIBS "${FREETYPE_ROOT_DIR}/lib/libfreetype*")
		add_target_copy_files("${FREETYPE_LIBS}")
	endif()
endif()
