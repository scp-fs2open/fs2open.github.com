
include(EnableExtraCompilerWarnings)
include(CheckCCompilerFlag)
include(util)

MESSAGE(STATUS "Doing configuration specific to gcc...")

option(GCC_ENABLE_LEAK_CHECK "Enable -fsanitize=leak" OFF)
option(GCC_ENABLE_ADDRESS_SANITIZER "Enable -fsanitize=address" OFF)
option(GCC_ENABLE_SANITIZE_UNDEFINED "Enable -fsanitize=undefined" OFF)

# These are the default values
set(C_BASE_FLAGS "-march=native -pipe")
set(CXX_BASE_FLAGS "-march=native -pipe")

# For C and C++, the values can be overwritten independently
if(DEFINED ENV{CFLAGS})
	set(C_BASE_FLAGS $ENV{CFLAGS})
endif()
if(DEFINED ENV{CXXFLAGS})
	set(CXX_BASE_FLAGS $ENV{CXXFLAGS})
endif()

# Initialize with an empty string to make sure we always get a clean start
set(COMPILER_FLAGS "")

# This is a slight hack since our flag setup is a bit more complicated
_enable_extra_compiler_warnings_flags()
set(COMPILER_FLAGS "${COMPILER_FLAGS} ${_flags}")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -funroll-loops -fsigned-char -Wno-unknown-pragmas")

# Place each function and data in its own section so the linker can
# perform dead code elimination
set(COMPILER_FLAGS "${COMPILER_FLAGS} -fdata-sections -ffunction-sections")

if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
	# Force color diagnostics for Ninja generator
	CHECK_C_COMPILER_FLAG(-fdiagnostics-color SUPPORTS_DIAGNOSTIC_COLOR)

	if(SUPPORTS_DIAGNOSTIC_COLOR)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fdiagnostics-color")
	endif()
endif()

# Start with an empty list
set(SANITIZE_FLAGS)

if(GCC_ENABLE_ADDRESS_SANITIZER)
	set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
	CHECK_C_COMPILER_FLAG("-fsanitize=address" SUPPORTS_SANITIZE_ADDRESS)

	if (SUPPORTS_SANITIZE_ADDRESS)
		set(SANITIZE_FLAGS ${SANITIZE_FLAGS} "address")
	endif()
endif()
if (GCC_ENABLE_LEAK_CHECK)
	check_linker_flag("-fsanitize=leak" SUPPORTS_FSANITIZE_LEAK)

	if (SUPPORTS_FSANITIZE_LEAK)
		set(SANITIZE_FLAGS ${SANITIZE_FLAGS} "leak")
	endif()
endif()
if (GCC_ENABLE_SANITIZE_UNDEFINED)
	check_linker_flag("-fsanitize=undefined" SUPPORTS_SANITIZE_UNDEFINED)

	if (SUPPORTS_SANITIZE_UNDEFINED)
		set(SANITIZE_FLAGS ${SANITIZE_FLAGS} "undefined")
	endif()
endif()

string(REPLACE ";" "," SANITIZE_FLAGS "${SANITIZE_FLAGS}")
if (NOT "${SANITIZE_FLAGS}" STREQUAL "")
	set(SANITIZE_FLAGS "-fsanitize=${SANITIZE_FLAGS}")
endif()

set(COMPILER_FLAGS "${COMPILER_FLAGS} ${SANITIZE_FLAGS}")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wformat-security")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-unused-function")

# Dear GCC, please tell us if a function does not return a value since that part of the standard is stupid!
set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wreturn-type")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-deprecated -Wno-char-subscripts")

set(COMPILER_FLAGS_RELEASE "-O2 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-array-bounds -Wno-empty-body -Wno-clobbered  -Wno-unused-parameter")

set(COMPILER_FLAGS_DEBUG "-O0 -g -Wshadow")

# Always use the base flags and add our compiler flags at the bacl
set(CMAKE_CXX_FLAGS "${CXX_BASE_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${C_BASE_FLAGS} ${COMPILER_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})

set(CMAKE_CXX_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})

set(CMAKE_EXE_LINKER_FLAGS "")

IF (MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++ -Wl,--enable-auto-import")
	target_compile_definitions(compiler INTERFACE __USE_MINGW_ANSI_STDIO=1)
ENDIF(MINGW)

if (SANITIZE_FLAGS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAGS}")
endif()

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")
IF(NOT MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -rdynamic")
	# Allow the linker to perform dead code elimination; this is considered
	# experimental for COFF and PE formats and thus disabled for Windows builds
	# (https://sourceware.org/binutils/docs/ld/Options.html)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
ENDIF(NOT MINGW)

IF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
	SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-zignore")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")

if (FSO_FATAL_WARNINGS)
	# Make warnings fatal if the right variable is set
	target_compile_options(compiler INTERFACE "-Werror")
endif()

# Always define this to make sure that the fixed width format macros are available
target_compile_definitions(compiler INTERFACE __STDC_FORMAT_MACROS=1)
