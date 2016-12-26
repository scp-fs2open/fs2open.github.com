
IF(FSO_USE_LUAJIT)
	MESSAGE(STATUS "Using luajit as lua interpreter")

	FIND_PACKAGE(PkgConfig)

	IF(PKG_CONFIG_FOUND AND NOT FSO_BUILD_INCLUDED_LIBS)
		pkg_check_modules(LUAJIT luajit)
	ENDIF(PKG_CONFIG_FOUND AND NOT FSO_BUILD_INCLUDED_LIBS)

	IF(LUAJIT_FOUND)
		PKG_CONFIG_LIB_RESOLVE(LUAJIT LUAJIT_LIB)

		ADD_IMPORTED_LIB(luajit "${LUAJIT_INCLUDE_DIRS}" "${LUAJIT_LIB}")

		SET(LUA_LIBS luajit CACHE INTERNAL "LuaJIT library" FORCE)
	ELSE(LUAJIT_FOUND)
		INCLUDE(ExternalProject)
		INCLUDE(util)

		IF(WIN32)
			IF(MSVC)
				SET(BUILD_COMMAND cd src && msvcbuild)
			ELSEIF(MINGW)
				SET(BUILD_COMMAND mingw32-make)
			ELSEIF(CYGWIN)
				SET(BUILD_COMMAND make)
			ELSE(MSVC)
				MESSAGE(ERROR "Unknown compiler! Cannot build luajit!")
			ENDIF(MSVC)
		ELSE(WIN32)
			SET(BUILD_COMMAND make)
		ENDIF(WIN32)

		IF(WIN32)
			EP_CHECK_FILE_EXISTS("src/luajit/src/lua51.dll" BUILD_COMMAND luajit test_compile.cmake "${BUILD_COMMAND}" "src/luajit")
		ELSE(WIN32)
			EP_CHECK_FILE_EXISTS("src/luajit/src/libluajit.a" BUILD_COMMAND luajit test_compile.cmake "${BUILD_COMMAND}" "src/luajit")
		ENDIF(WIN32)

		ExternalProject_Add(luajit
			PREFIX luajit
			# Downloading
			URL http://luajit.org/download/LuaJIT-2.0.2.tar.gz # Full path or URL of source
			URL_MD5 112dfb82548b03377fbefbba2e0e3a5b               # MD5 checksum of file at URL
			# Configuring
			CONFIGURE_COMMAND ""
			# Building
			BUILD_COMMAND ${BUILD_COMMAND}
			BUILD_IN_SOURCE 1
			# Installing
			INSTALL_COMMAND ""
		)

		set_target_properties(luajit
			PROPERTIES
				FOLDER "3rdparty"
		)

		ExternalProject_Get_Property(luajit source_dir)

		add_library(luajit_lib SHARED IMPORTED GLOBAL)
		ADD_DEPENDENCIES(luajit_lib luajit)

		IF(WIN32)
			set_target_properties(luajit_lib
				PROPERTIES
					IMPORTED_LOCATION ${source_dir}/src/lua51.dll
					IMPORTED_IMPLIB ${source_dir}/src/lua51.lib
			)

			add_target_copy_files("${source_dir}/src/lua51.dll")
		ELSE(WIN32)
			set_target_properties(luajit_lib
				PROPERTIES
					IMPORTED_LOCATION ${source_dir}/src/libluajit.a
			)
		ENDIF(WIN32)

		set_target_properties(luajit_lib
			PROPERTIES
				FOLDER "3rdparty"
				INTERFACE_COMPILE_DEFINITIONS LUA_BUILD_AS_DLL
				INTERFACE_INCLUDE_DIRECTORIES "${source_dir}/src"
		)

		FILE(MAKE_DIRECTORY "${source_dir}/src")

		SET(LUA_LIBS luajit_lib CACHE INTERNAL "LuaJIT library")
	ENDIF(LUAJIT_FOUND)
ELSE(FSO_USE_LUAJIT)
	IF(NOT FSO_BUILD_INCLUDED_LIBS)
		FIND_PACKAGE(PkgConfig)
		if (PKG_CONFIG_FOUND)
			INCLUDE(util)
			
			# The standard FindLua51 doesn't work on Fedora, use pkg-config instead
			pkg_check_modules(LUA51 lua5.1<=5.1.5)
			if (NOT LUA51_FOUND)
				pkg_check_modules(LUA51 lua-5.1<=5.1.5)
				if (NOT LUA51_FOUND)
					pkg_check_modules(LUA51 lua51<=5.1.5)
					if (NOT LUA51_FOUND)
						pkg_check_modules(LUA51 lua<=5.1.5)
					endif()
				endif()
			endif()
			
			if (LUA51_FOUND)
				PKG_CONFIG_LIB_RESOLVE(LUA51 LUA51_LIB)
				ADD_IMPORTED_LIB(lua "${LUA51_INCLUDE_DIRS}" "${LUA51_LIB}")
		
				SET(LUA_LIBS lua CACHE INTERNAL "lua library" FORCE)
			endif()
		endif()
	ENDIF(NOT FSO_BUILD_INCLUDED_LIBS)

	IF (FSO_BUILD_INCLUDED_LIBS OR NOT LUA51_FOUND)
		MESSAGE(STATUS "Building lua from source")

		SET(LUA51_SOURCES
			lapi.c
			lauxlib.c
			lbaselib.c
			lcode.c
			ldblib.c
			ldebug.c
			ldo.c
			ldump.c
			lfunc.c
			lgc.c
			linit.c
			liolib.c
			llex.c
			lmathlib.c
			lmem.c
			loadlib.c
			lobject.c
			lopcodes.c
			loslib.c
			lparser.c
			lstate.c
			lstring.c
			lstrlib.c
			ltable.c
			ltablib.c
			ltm.c
			lundump.c
			lvm.c
			lzio.c
			print.c
			lapi.h
			lauxlib.h
			lcode.h
			ldebug.h
			ldo.h
			lfunc.h
			lgc.h
			llex.h
			llimits.h
			lmem.h
			lobject.h
			lopcodes.h
			lparser.h
			lstate.h
			lstring.h
			ltable.h
			ltm.h
			lua.h
			luaconf.h
			lualib.h
			lundump.h
			lvm.h
			lzio.h
			)

		ADD_LIBRARY(lua51 ${LUA51_SOURCES})

		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

		set_target_properties(lua51
			PROPERTIES
				COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS
				DEFINE_SYMBOL LUA_BUILD_AS_DLL # This will only be defined if we are building a shared library
				INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
		)

		IF(BUILD_SHARED_LIBS)
			set_target_properties(lua51
				PROPERTIES
					INTERFACE_COMPILE_DEFINITIONS LUA_BUILD_AS_DLL
			)
		ENDIF(BUILD_SHARED_LIBS)

		set_target_properties(lua51
			PROPERTIES
				FOLDER "3rdparty"
		)

		# Set the correct defines for the current platform
		if (PLATFORM_LINUX)
			target_compile_definitions(lua51 PUBLIC LUA_USE_LINUX)
		elseif(PLATFORM_MAC)
			target_compile_definitions(lua51 PUBLIC LUA_USE_MACOSX)
		endif()

		if (MSVC)
			# Disable warnings if building from source
			target_compile_options(lua51 PRIVATE "/W0")
		else()
			target_compile_options(lua51 PRIVATE "-w")
		endif()
		target_link_libraries(lua51 PUBLIC compiler)

		SET(LUA_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "Lua 5.1 include directory")

		SET(LUA_LIBS lua51 CACHE INTERNAL "Lua 5.1 library")

		IF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)
			INSTALL(
				TARGETS lua51
				RUNTIME DESTINATION ${BINARY_DESTINATION}
				LIBRARY DESTINATION ${BINARY_DESTINATION}
				)
		ENDIF(FSO_COPY_EXE AND BUILD_SHARED_LIBS)
	ENDIF(FSO_BUILD_INCLUDED_LIBS OR NOT LUA51_FOUND)

ENDIF(FSO_USE_LUAJIT)
