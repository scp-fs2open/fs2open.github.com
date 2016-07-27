
IF(UNIX AND NOT APPLE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(OGG REQUIRED ogg)
	pkg_check_modules(THEORA REQUIRED theora)
	pkg_check_modules(VORBIS REQUIRED vorbis)
	pkg_check_modules(VORBISFILE REQUIRED vorbisfile)

	INCLUDE(util)
	PKG_CONFIG_LIB_RESOLVE(OGG OGG_LIB)
	PKG_CONFIG_LIB_RESOLVE(THEORA THEORA_LIB)
	PKG_CONFIG_LIB_RESOLVE(VORBIS VORBIS_LIB)
	PKG_CONFIG_LIB_RESOLVE(VORBISFILE VORBISFILE_LIB)

	ADD_IMPORTED_LIB(ogg "${OGG_INCLUDEDIR}" "${OGG_LIB}")
	ADD_IMPORTED_LIB(theora "${THEORA_INCLUDEDIR}" "${THEORA_LIB}")
	ADD_IMPORTED_LIB(vorbis "${VORBIS_INCLUDEDIR}" "${VORBIS_LIB}")
	ADD_IMPORTED_LIB(vorbisfile "${VORBISFILE_INCLUDEDIR}" "${VORBISFILE_LIB}")

	add_library(oggvorbis INTERFACE)
	target_link_libraries(oggvorbis INTERFACE ogg theora vorbis vorbisfile)
ELSEIF (WIN32)
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		# 64-bit
		set(ARCH "x64")
	ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
		# 32-bit
		set(ARCH "x86")
	ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

	# We have precompiled libs for windows
	add_library(ogg STATIC IMPORTED GLOBAL)
	set_target_properties(ogg
		PROPERTIES
			IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libogg.lib
			INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include
	)

	add_library(vorbis STATIC IMPORTED GLOBAL)
	set_target_properties(vorbis
		PROPERTIES
			IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libvorbis.lib
			INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include
	)

	add_library(vorbisfile STATIC IMPORTED GLOBAL)
	set_target_properties(vorbisfile
		PROPERTIES
			IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libvorbisfile.lib
			INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include
	)

	add_library(theora STATIC IMPORTED GLOBAL)
	set_target_properties(theora
		PROPERTIES
			IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libtheora.lib
			INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include
	)

	add_library(oggvorbis INTERFACE)
	target_link_libraries(oggvorbis INTERFACE ogg theora vorbis vorbisfile)

	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		# 64-bit
		add_target_copy_files("${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libogg.dll")
		add_target_copy_files("${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libtheora.dll")
		add_target_copy_files("${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libvorbis.dll")
		add_target_copy_files("${CMAKE_CURRENT_LIST_DIR}/lib/${ARCH}/libvorbisfile.dll")
	ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
ELSEIF(APPLE)
	macro(find_lib varname libname)
		find_library(${varname} ${libname} PATHS "${FSO_MAC_FRAMEWORKS}")

		if (NOT ${varname})
			message(FATAL_ERROR "Couldn't find ${libname} libraries!")
		endif()
	endmacro(find_lib)

	find_lib(OGG_LIBRARY Ogg)
	find_lib(THEORA_LIBRARY Theora)
	find_lib(VORBIS_LIBRARY Vorbis)

	add_library(oggvorbis INTERFACE)
	target_link_libraries(oggvorbis INTERFACE ${OGG_LIBRARY} ${THEORA_LIBRARY}
		${VORBIS_LIBRARY})

	if (NOT "${OGG_LIBRARY}" MATCHES ".*\\.framework$")
		message(STATUS "Using non-framework version of ogg/vorbis/theora libraries")

		# The non-framework version needs vorbisfile
		find_lib(VORBISFILE_LIBRARY vorbisfile)

		target_link_libraries(oggvorbis INTERFACE ${VORBISFILE_LIBRARY})

		find_path(OGG_INCLUDE_DIR ogg.h PATH_SUFFIXES ogg)
		find_path(THEORA_INCLUDE_DIR theora.h PATH_SUFFIXES theora)
		find_path(VORBIS_INCLUDE_DIR vorbisenc.h PATH_SUFFIXES vorbis)
		find_path(VORBISFILE_INCLUDE_DIR vorbisfile.h PATH_SUFFIXES vorbis)

		target_include_directories(oggvorbis INTERFACE ${OGG_INCLUDE_DIR}
			${THEORA_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR} ${VORBISFILE_INCLUDE_DIR})
	endif()
ELSE()
	MESSAGE(FATAL_ERROR "Unconfigured oggvorbis platform, please report!")
ENDIF()
