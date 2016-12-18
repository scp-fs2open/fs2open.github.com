
IF(NOT FSO_BUILD_INCLUDED_LIBS)
	FIND_PACKAGE(JPEG QUIET)
ENDIF(NOT FSO_BUILD_INCLUDED_LIBS)

IF (FSO_BUILD_INCLUDED_LIBS OR NOT JPEG_FOUND)
	MESSAGE(STATUS "Building libjpeg from source")

	SET(JPEG_SOURCES
		jaricom.c
		jcapimin.c
		jcapistd.c
		jcomapi.c
		jcparam.c
		jctrans.c
		jdapimin.c
		jdapistd.c
		jdarith.c
		jdatasrc.c
		jdcoefct.c
		jdcolor.c
		jddctmgr.c
		jdhuff.c
		jdinput.c
		jdmainct.c
		jdmarker.c
		jdmaster.c
		jdmerge.c
		jdpostct.c
		jdsample.c
		jdtrans.c
		jerror.c
		jidctflt.c
		jidctfst.c
		jidctint.c
		jmemmgr.c
		jmemnobs.c
		jquant1.c
		jquant2.c
		jutils.c)

	ADD_LIBRARY(jpeg STATIC ${JPEG_SOURCES})

	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

	set_target_properties(jpeg
		PROPERTIES
			COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS
			INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
	)

	set_target_properties(jpeg
		PROPERTIES
			FOLDER "3rdparty"
	)
	
	if (MSVC)
		target_compile_options(jpeg PRIVATE "/W0")
	endif()
	target_link_libraries(jpeg PUBLIC compiler)

	SET(JPEG_LIBS jpeg CACHE INTERNAL "JPEG library")
ELSE(FSO_BUILD_INCLUDED_LIBS OR NOT JPEG_FOUND)
	INCLUDE(util)

	ADD_IMPORTED_LIB(jpeg "${JPEG_INCLUDE_DIR}" "${JPEG_LIBRARIES}")

	SET(JPEG_LIBS jpeg CACHE INTERNAL "zlib library" FORCE)
ENDIF(FSO_BUILD_INCLUDED_LIBS OR NOT JPEG_FOUND)
