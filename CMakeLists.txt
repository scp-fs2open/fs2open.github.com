
CMAKE_MINIMUM_REQUIRED(VERSION 3.4)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure the source and binary directory are not the sample
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
	message(FATAL_ERROR "Binary and source directory can not be the same! Choose another build directory.")
endif()

include(cmake/util.cmake)
include(cmake/embed_file.cmake)

# Set a few policies
set_policy(CMP0051 NEW)
set_policy(CMP0053 NEW)
set_policy(CMP0054 NEW)

SET(FSO_PROJECT_DESCRIPTION "Open source project based on the original FreeSpace 2 source code.")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Executable output path")
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Library output path")

# CMAKE_INSTALL_PREFIX is defined but also overwritten by the PROJECT command so we need to set it later
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
	SET(RESET_INSTALL_PREFIX TRUE)
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)

# Reset this variable
SET(TARGET_COPY_FILES "" CACHE INTERNAL "" FORCE)
SET(TARGET_COPY_DIRS "" CACHE INTERNAL "" FORCE)

### Define options and editable variables in the section below:
# This is only needed for single configuration generators so don't annoy others
if(NOT CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build configuration used." FORCE)
	message(WARNING "CMAKE_BUILD_TYPE was not specified, defaulting to Release configuration.")
endif(NOT CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)

# Override standard CMake configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release FastDebug CACHE STRING "Available configurations" FORCE)

PROJECT(FS2_Open LANGUAGES C CXX OBJC OBJCXX)

find_program(CCACHE_PROGRAM ccache)
# Currently only GCC support ccache properly without generating additional warnings
# Enable ccache on Mac even for clang
if(CCACHE_PROGRAM AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR APPLE))
	if(NOT CMAKE_C_COMPILER_LAUNCHER)
		message(STATUS "Using ccache for C compilation: ${CCACHE_PROGRAM}")
		SET(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	endif()
	if(NOT CMAKE_CXX_COMPILER_LAUNCHER)
		message(STATUS "Using ccache for C++ compilation: ${CCACHE_PROGRAM}")
		SET(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
	endif()
endif()

IF(CMAKE_CROSSCOMPILING)
	SET(IMPORT_EXE_DIR "IMPORT_EXE_DIR-NOTFOUND" CACHE FILEPATH "Required for a cross compiling build, point to binary directory of a cmake build for the real OS")
	INCLUDE(${IMPORT_EXE_DIR}/ImportExecutables.cmake)
ENDIF(CMAKE_CROSSCOMPILING)

# Check if the external modules exists
IF(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/rpavlik-cmake-modules/launcher-templates")
	message(FATAL_ERROR "External submodules could not be found. Please make sure you have updated your submodules.")
endif()

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/finder")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/rpavlik-cmake-modules")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/external/cotire")

IF(RESET_INSTALL_PREFIX)
	IF(NOT $ENV{FS2PATH} STREQUAL "")
		IF(NOT IS_DIRECTORY $ENV{FS2PATH})
			MESSAGE(WARNING "The path '$ENV{FS2PATH}' of the environment variable FS2PATH does not point to a directory! Install prefix wasn't set.")
		ELSE(NOT IS_DIRECTORY $ENV{FS2PATH})
			SET(CMAKE_INSTALL_PREFIX "$ENV{FS2PATH}" CACHE FILEPATH "Install path" FORCE)
			FILE(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX)
		ENDIF(NOT IS_DIRECTORY $ENV{FS2PATH})
	ELSE(NOT $ENV{FS2PATH} STREQUAL "")
		MESSAGE(STATUS "NO FS2PATH environment variable found, you can define this to point to your freespace install but it is not required.")
	ENDIF(NOT $ENV{FS2PATH} STREQUAL "")
ENDIF(RESET_INSTALL_PREFIX)

IF(WIN32 OR APPLE)
	OPTION(FSO_USE_SPEECH "Use text-to-speach libraries" ON)
ENDIF(WIN32 OR APPLE)

IF (WIN32)
	OPTION(FSO_USE_VOICEREC "Enable voice recognition support" ON)

	OPTION(FSO_BUILD_FRED2 "Build FRED2 binary" ON)
elseif(UNIX)
	option(FSO_BUILD_APPIMAGE "Build an AppImage package" OFF)
ENDIF(WIN32)

OPTION(FSO_BUILD_TOOLS "Build tools related to FSO" OFF)

OPTION(FSO_BUILD_TESTS "Build unit tests" OFF)

OPTION(FSO_DEVELOPMENT_MODE "Generate binaries in development mode, only use if you know what you're doing!" OFF)

OPTION(FSO_BUILD_QTFRED "Build qtFRED2 binary" OFF)

IF(WIN32 OR APPLE)
	# On windows and mac the default should be to always build the included libraries
	SET(FSO_BUILD_INCLUDED_LIBS_DEFAULT ON)
ELSE()
	SET(FSO_BUILD_INCLUDED_LIBS_DEFAULT OFF)
ENDIF()

OPTION(FSO_BUILD_INCLUDED_LIBS "Build and use the included libraries istead of using the system headers and libraries" ${FSO_BUILD_INCLUDED_LIBS_DEFAULT})

OPTION(FSO_USE_LUAJIT "Use LuaJIT for Lua scripting, needs a working internet connection!" OFF)

OPTION(FSO_USE_OPENALSOFT "Download and build OpenAL Soft instead of using the system libraries" OFF)

OPTION(FSO_FATAL_WARNINGS "Determines if warnings in the build are considered fatal errors, primarily used for CI" OFF)

SET(FSO_FREESPACE_PATH "${CMAKE_INSTALL_PREFIX}" CACHE FILEPATH "The path of the FreeSpace directory you want to use. Please note that you will have to change CMAKE_INSTALL_PREFIX if you change this at some point.")

SET(FSO_RUN_ARGUMENTS "" CACHE STRING "Additional arguments passed to a generated executable when run with the generated build files.")

option(FSO_INSTALL_DEBUG_FILES "Install some debug files (currently only PDB files on windows)" OFF)

option(ENABLE_COTIRE "Enable cotire for faster compilation. Enabled by default." ON)

option(FSO_RELEASE_LOGGING "Enable logging output for release builds" OFF)

OPTION(FSO_BUILD_WITH_FFMPEG "Enable the usage of FFmpeg for sound and custscenes" ON)

OPTION(FSO_BUILD_WITH_OPENGL "Enable compilation of the OpenGL renderer" ON)

OPTION(FSO_BUILD_WITH_VULKAN "Enable compilation of the Vulkan renderer" ON)

MARK_AS_ADVANCED(FORCE FSO_CMAKE_DEBUG)
MARK_AS_ADVANCED(FORCE FSO_BUILD_INCLUDED_LIBS)
MARK_AS_ADVANCED(FORCE FSO_USE_OPENALSOFT)
MARK_AS_ADVANCED(FORCE FSO_USE_LUAJIT)
MARK_AS_ADVANCED(FORCE FSO_DEVELOPMENT_MODE)
MARK_AS_ADVANCED(FORCE FSO_FATAL_WARNINGS)
mark_as_advanced(FORCE FSO_INSTALL_DEBUG_FILES)
mark_as_advanced(FORCE ENABLE_COTIRE)
mark_as_advanced(FORCE FSO_RELEASE_LOGGING)
mark_as_advanced(FORCE FSO_BUILD_WITH_FFMPEG)
mark_as_advanced(FORCE FSO_BUILD_WITH_OPENGL)
mark_as_advanced(FORCE FSO_BUILD_WITH_VULKAN)

# Include cotire file from https://github.com/sakra/cotire/
include(cotire)

INCLUDE(globals)
INCLUDE(toolchain)
INCLUDE(platforms)
include(version)

INCLUDE(headers)

SET(GENERATED_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_source)

include(generateHeaders)

if(FSO_BUILD_APPIMAGE)
	include(appimage)
endif()

include(clang-tidy)

# This includes source code for some tools, either used in the build or also for something else
ADD_SUBDIRECTORY(tools)

ADD_SUBDIRECTORY(lib)

ADD_SUBDIRECTORY(parsers)

ADD_SUBDIRECTORY(code)

ADD_SUBDIRECTORY(freespace2)

IF(FSO_BUILD_FRED2)
	ADD_SUBDIRECTORY(fred2)
ENDIF(FSO_BUILD_FRED2)

IF(FSO_BUILD_QTFRED)
	ADD_SUBDIRECTORY(qtfred)
ENDIF()

if(FSO_BUILD_TESTS)
	add_subdirectory(test)
endif()

if ("${CMAKE_VERSION}" VERSION_GREATER "3.5")
	# Default to using Freespace2 as startup project if CMake supports it.
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Freespace2)
endif()

IF(MSVC_USE_RUNTIME_DLL)
	SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BINARY_DESTINATION}")

	IF(FSO_BUILD_FRED2)
		SET(CMAKE_INSTALL_MFC_LIBRARIES ON)
	ENDIF(FSO_BUILD_FRED2)

	INCLUDE(InstallRequiredSystemLibraries)
ENDIF(MSVC_USE_RUNTIME_DLL)

include(package)

include(doxygen)

# Print used options to log
IF(WIN32 OR APPLE)
	message(STATUS "Using text to speech: ${FSO_USE_SPEECH}")
ENDIF()
IF (WIN32)
	message(STATUS "Using voice recogition: ${FSO_USE_VOICEREC}")
	message(STATUS "Building FRED2: ${FSO_BUILD_FRED2}")
elseif(UNIX)
	message(STATUS "Building AppImage: ${FSO_BUILD_APPIMAGE}")
ENDIF()
message(STATUS "Building FSO tools: ${FSO_BUILD_TOOLS}")
message(STATUS "Building qtFRED: ${FSO_BUILD_QTFRED}")
message(STATUS "Fatal warnings: ${FSO_FATAL_WARNINGS}")
message(STATUS "Release logging: ${FSO_RELEASE_LOGGING}")
message(STATUS "With FFmpeg: ${FSO_BUILD_WITH_FFMPEG}")
message(STATUS "With OpenGL: ${FSO_BUILD_WITH_OPENGL}")
message(STATUS "With Vulkan: ${FSO_BUILD_WITH_VULKAN}")
