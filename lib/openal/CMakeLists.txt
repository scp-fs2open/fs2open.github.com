
IF(FSO_USE_OPENALSOFT)
	INCLUDE(ExternalProject)
	MESSAGE(STATUS "Using OpenAL Soft for OpenAL support")

	IF(FSO_DEVELOPMENT_MODE)
		SET(CONFIG Debug)
	ELSE(FSO_DEVELOPMENT_MODE)
		SET(CONFIG Release)
	ENDIF(FSO_DEVELOPMENT_MODE)

	ExternalProject_Add(OpenALsoft
		PREFIX OpenALsoft
		# Downloading
		URL http://kcat.strangesoft.net/openal-releases/openal-soft-1.15.1.tar.bz2 # Full path or URL of source
		URL_MD5 ea83dec3b9655a27d28e7bc7cae9cd71               # MD5 checksum of file at URL


		CMAKE_CACHE_ARGS -DUTILS:BOOL=OFF -DEXAMPLES:BOOL=OFF
							-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DCMAKE_BUILD_TYPE:STRING=${CONFIG}

		BUILD_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config ${CONFIG}

		INSTALL_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config ${CONFIG} --target install
	)

	set_target_properties(OpenALsoft
		PROPERTIES
			FOLDER "3rdparty"
	)

	ExternalProject_Get_Property(OpenALsoft install_dir)

	add_library(openalsoft_lib SHARED IMPORTED GLOBAL)
	ADD_DEPENDENCIES(openalsoft_lib OpenALsoft)

	IF(WIN32)
		set_target_properties(openalsoft_lib
			PROPERTIES
				IMPORTED_LOCATION ${install_dir}/bin/OpenAL32.dll
				IMPORTED_IMPLIB ${install_dir}/lib/OpenAL32.lib
				INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
		)

		INSTALL(FILES ${install_dir}/bin/OpenAL32.dll
				DESTINATION ${BINARY_DESTINATION}
		)

		add_target_copy_files("${install_dir}/bin/OpenAL32.dll")
	ELSE(WIN32)
		set_target_properties(openalsoft_lib
			PROPERTIES
				IMPORTED_LOCATION ${install_dir}/lib/libopenal.so
				INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
		)

		INSTALL(FILES ${install_dir}/lib/libopenal.so
				DESTINATION ${BINARY_DESTINATION}
		)
	ENDIF(WIN32)

	SET(OPENAL_LIBS openalsoft_lib CACHE INTERNAL "OpenAL library")
ELSE(FSO_USE_OPENALSOFT)
	IF(NOT WIN32)
		FIND_PACKAGE(OpenAL REQUIRED)
	ENDIF(NOT WIN32)

	# We have precompiled libs for windows
	IF (WIN32)
		MESSAGE(STATUS "Could not find OpenAL libraries on system, using included pre-built libraries")

		add_library(openal STATIC IMPORTED GLOBAL)
		IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
			# 64-bit
			set_target_properties(openal
				PROPERTIES
					IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/win64/openal32.lib
					INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
			)
		ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
			# 32-bit
			set_target_properties(openal
				PROPERTIES
					IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/win32/openal32.lib
					INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
			)
		ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

		SET(OPENAL_LIBS openal CACHE INTERNAL "OpenAL library")
	ELSE(WIN32)
		INCLUDE(util)

		ADD_IMPORTED_LIB(openal ${OPENAL_INCLUDE_DIR} ${OPENAL_LIBRARY})

		SET(OPENAL_LIBS openal CACHE INTERNAL "OpenAL library" FORCE)
	ENDIF(WIN32)
ENDIF(FSO_USE_OPENALSOFT)
