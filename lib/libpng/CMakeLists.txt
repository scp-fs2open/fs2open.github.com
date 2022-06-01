
IF(NOT FSO_BUILD_INCLUDED_LIBS)
	FIND_PACKAGE(PNG QUIET)
ENDIF(NOT FSO_BUILD_INCLUDED_LIBS)

IF (FSO_BUILD_INCLUDED_LIBS OR NOT PNG_FOUND)
	MESSAGE(STATUS "Building libpng from source")

	SET(PNG_SOURCES
		png.c
		pngerror.c
		pngget.c
		pngmem.c
		pngpread.c
		pngread.c
		pngrio.c
		pngrtran.c
		pngrutil.c
		pngset.c
		pngtrans.c
		pngwio.c
		pngwrite.c
		pngwtran.c
		pngwutil.c
		png.h
		pngconf.h
		pngdebug.h
		pnginfo.h
		pnglibconf.h
		pngpriv.h
		pngstruct.h)

	ADD_LIBRARY(png ${PNG_SOURCES})

	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

	set_target_properties(png
		PROPERTIES
			DEFINE_SYMBOL PNG_BUILD_DLL # This will only be defined if we are building a shared library
	)
	target_include_directories(png SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

	target_compile_definitions(png PRIVATE "_CRT_SECURE_NO_WARNINGS;$<$<CONFIG:Debug>:PNG_DEBUG=1>")
	target_compile_definitions(png PRIVATE PNG_ARM_NEON_IMPLEMENTATION=0) # Disable NEON to fix Mac ARM builds
	target_compile_definitions(png PRIVATE PNG_ARM_NEON_OPT=0)

	IF(BUILD_SHARED_LIBS)
		target_compile_definitions(png INTERFACE PNG_BUILD_DLL)
	ENDIF(BUILD_SHARED_LIBS)

	set_target_properties(png
		PROPERTIES
			FOLDER "3rdparty"
	)
    
    # Disable warnings if building from source
    suppress_warnings(png)

	target_link_libraries(png PUBLIC ${ZLIB_LIBS})
	target_link_libraries(png PUBLIC compiler)

	SET(PNG_LIBS png CACHE INTERNAL "PNG library")

	IF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)
		INSTALL(
			TARGETS png
			RUNTIME DESTINATION ${BINARY_DESTINATION}
			LIBRARY DESTINATION ${BINARY_DESTINATION}
		)
	ENDIF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)
ELSE(FSO_BUILD_INCLUDED_LIBS OR NOT PNG_FOUND)
	INCLUDE(util)

	ADD_IMPORTED_LIB(png "${PNG_INCLUDE_DIRS}" "${PNG_LIBRARIES}")
	set_target_properties(png
		PROPERTIES
			INTERFACE_COMPILE_DEFINITIONS "${PNG_DEFINITIONS}"
	)

	SET(PNG_LIBS png CACHE INTERNAL "png library" FORCE)
ENDIF(FSO_BUILD_INCLUDED_LIBS OR NOT PNG_FOUND)
