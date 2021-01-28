
IF(NOT CMAKE_CROSSCOMPILING)
	SET(EMBEDFILE_SOURCES embedfile.cpp)

	ADD_EXECUTABLE(embedfile EXCLUDE_FROM_ALL ${EMBEDFILE_SOURCES})

	target_link_libraries(embedfile compiler platform)

	set_target_properties(embedfile
		PROPERTIES
			FOLDER "Tools"
	)

	enable_clang_tidy(embedfile)

	EXPORT(TARGETS embedfile FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake)
ENDIF(NOT CMAKE_CROSSCOMPILING)
