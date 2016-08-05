
IF(NOT CMAKE_CROSSCOMPILING)
	SET(EMBEDFILE_SOURCES embedfile.cpp)

	ADD_EXECUTABLE(embedfile EXCLUDE_FROM_ALL ${EMBEDFILE_SOURCES})

	target_compile_features(embedfile PUBLIC cxx_auto_type)

	set_target_properties(embedfile
		PROPERTIES
			FOLDER "Tools"
	)

	EXPORT(TARGETS embedfile FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake)
ENDIF(NOT CMAKE_CROSSCOMPILING)
