IF(NOT CMAKE_CROSSCOMPILING)
	cmake_policy(SET CMP0091 NEW) # Enable use of CMAKE_MSVC_RUNTIME_LIBRARY
	SET(EMBEDFILE_SOURCES embedfile.cpp)

	ADD_EXECUTABLE(embedfile EXCLUDE_FROM_ALL ${EMBEDFILE_SOURCES})

	target_link_libraries(embedfile compiler platform)

	set_target_properties(embedfile
		PROPERTIES
			FOLDER "Tools"
	)

	if (PLATFORM_MAC)
		# we build for two archs on mac, but this needs to always match the host system
		set_target_properties(embedfile
			PROPERTIES
			OSX_ARCHITECTURES ${CMAKE_HOST_SYSTEM_PROCESSOR}
		)
	endif()

	enable_clang_tidy(embedfile)

	EXPORT(TARGETS embedfile FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake)
ENDIF(NOT CMAKE_CROSSCOMPILING)
