
# We have precompiled libs for windows
IF (WIN32)
	get_prebuilt_path(PREBUILT_PATH)

	add_library(openal SHARED IMPORTED GLOBAL)

	set(dll_name "${PREBUILT_PATH}/openal/bin/OpenAL32.dll")

	set_target_properties(openal
		PROPERTIES
			IMPORTED_LOCATION "${dll_name}"
			IMPORTED_IMPLIB "${PREBUILT_PATH}/openal/libs/${CMAKE_IMPORT_LIBRARY_PREFIX}OpenAL32${CMAKE_IMPORT_LIBRARY_SUFFIX}"
			INTERFACE_INCLUDE_DIRECTORIES "${PREBUILT_PATH}/openal/include"
	)

	add_target_copy_files("${dll_name}")
ELSE(WIN32)
	FIND_PACKAGE(OpenAL REQUIRED)

	INCLUDE(util)

	ADD_IMPORTED_LIB(openal ${OPENAL_INCLUDE_DIR} ${OPENAL_LIBRARY})
ENDIF(WIN32)
