
IF(NOT FSO_BUILD_INCLUDED_LIBS)
	FIND_PACKAGE(ZLIB QUIET)
ENDIF(NOT FSO_BUILD_INCLUDED_LIBS)

IF (FSO_BUILD_INCLUDED_LIBS OR NOT ZLIB_FOUND)
	MESSAGE(STATUS "Building zlib from source")

	SET(ZLIB_SOURCES
		adler32.c
		compress.c
		crc32.c
		deflate.c
		infback.c
		inffast.c
		inflate.c
		inftrees.c
		trees.c
		uncompr.c
		zutil.c
		crc32.h
		deflate.h
		inffast.h
		inffixed.h
		inflate.h
		inftrees.h
		trees.h
		zconf.h
		zlib.h
		zutil.h)

	ADD_LIBRARY(zlib ${ZLIB_SOURCES})

	target_include_directories(zlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

	set_target_properties(zlib
		PROPERTIES
			FOLDER "3rdparty"
			DEFINE_SYMBOL ZLIB_DLL # This will only be defined if we are building a shared library
	)

    # Disable warnings if building from source
    suppress_warnings(zlib)
    
	target_link_libraries(zlib PUBLIC compiler)

	IF(BUILD_SHARED_LIBS)
		target_compile_definitions(zlib INTERFACE ZLIB_DLL)
	ENDIF(BUILD_SHARED_LIBS)

	IF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)
		INSTALL(
			TARGETS zlib
			RUNTIME DESTINATION ${BINARY_DESTINATION}
			LIBRARY DESTINATION ${BINARY_DESTINATION}
			)
	ENDIF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)

	SET(ZLIB_LIBS zlib CACHE INTERNAL "zlib library" FORCE)
ELSE(FSO_BUILD_INCLUDED_LIBS OR NOT ZLIB_FOUND)
	INCLUDE(util)

	ADD_IMPORTED_LIB(zlib ${ZLIB_INCLUDE_DIR} ${ZLIB_LIBRARIES})

	SET(ZLIB_LIBS zlib CACHE INTERNAL "zlib library" FORCE)
ENDIF(FSO_BUILD_INCLUDED_LIBS OR NOT ZLIB_FOUND)
