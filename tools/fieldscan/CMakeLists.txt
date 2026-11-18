IF(NOT CMAKE_CROSSCOMPILING)
	SET(FIELDSCAN_SOURCES fieldscan.cpp)

	ADD_EXECUTABLE(fieldscan EXCLUDE_FROM_ALL ${FIELDSCAN_SOURCES})

	target_link_libraries(fieldscan compiler platform)

	set_target_properties(fieldscan
		PROPERTIES
			FOLDER "Tools"
	)

	if (PLATFORM_MAC)
		# we build for two archs on mac, but this needs to always match the host system
		set_target_properties(fieldscan
			PROPERTIES
			OSX_ARCHITECTURES ${CMAKE_HOST_SYSTEM_PROCESSOR}
		)
	endif()

	enable_clang_tidy(fieldscan)

	EXPORT(TARGETS fieldscan APPEND FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake)
ENDIF(NOT CMAKE_CROSSCOMPILING)
