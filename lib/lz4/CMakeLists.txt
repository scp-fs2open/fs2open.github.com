
IF(NOT FSO_BUILD_INCLUDED_LIBS)
	FIND_PACKAGE(LZ4 QUIET)
ENDIF(NOT FSO_BUILD_INCLUDED_LIBS)

IF (FSO_BUILD_INCLUDED_LIBS OR NOT LZ4_FOUND)
	MESSAGE(STATUS "Building lz4 from source")

	SET(LZ4_SOURCES
		lz4.c
		lz4frame.c
		lz4hc.c
		xxhash.c
		lz4.h
		lz4frame.h
		lz4hc.h
		xxhash.h)

	ADD_LIBRARY(lz4 ${LZ4_SOURCES})

	target_include_directories(lz4 SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

	set_target_properties(lz4
		PROPERTIES
			FOLDER "3rdparty"
			DEFINE_SYMBOL LZ4_DLL # This will only be defined if we are building a shared library
	)

    # Disable warnings if building from source
    suppress_warnings(lz4)
    
	target_link_libraries(lz4 PUBLIC compiler)

	IF(BUILD_SHARED_LIBS)
		target_compile_definitions(lz4 INTERFACE LZ4_DLL)
	ENDIF(BUILD_SHARED_LIBS)

	IF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)
		INSTALL(
			TARGETS lz4
			RUNTIME DESTINATION ${BINARY_DESTINATION}
			LIBRARY DESTINATION ${BINARY_DESTINATION}
			)
	ENDIF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)

ELSE(FSO_BUILD_INCLUDED_LIBS OR NOT LZ4_FOUND)
	INCLUDE(util)

	ADD_IMPORTED_LIB(lz4 ${LZ4_INCLUDE_DIR} ${LZ4_LIBRARIES})

ENDIF(FSO_BUILD_INCLUDED_LIBS OR NOT LZ4_FOUND)
