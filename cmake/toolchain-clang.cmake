# Clang
include(EnableExtraCompilerWarnings)
include(CheckCXXCompilerFlag)

MESSAGE(STATUS "Doing configuration specific to clang...")

option(CLANG_ENABLE_LEAK_CHECK "Enable -fsanitize=leak" OFF)
option(CLANG_ENABLE_ADDRESS_SANITIZER "Enable -fsanitize=address" OFF)

option(CLANG_USE_LIBCXX "Use libc++" OFF)

# These are the default values
set(C_BASE_FLAGS "-march=native -pipe")
set(CXX_BASE_FLAGS "-march=native -pipe")

# For C and C++, the values can be overwritten independently
if(DEFINED ENV{CXXFLAGS})
	set(CXX_BASE_FLAGS $ENV{CXXFLAGS})
endif()
if(DEFINED ENV{CFLAGS})
	set(C_BASE_FLAGS $ENV{CFLAGS})
endif()

if (CLANG_USE_LIBCXX)
	set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} -stdlib=libc++")
	target_link_libraries(compiler INTERFACE "c++" "c++abi")
endif()

# Initialize with an empty string to make sure we always get a clean start
set(COMPILER_FLAGS "")

# This is a slight hack since our flag setup is a bit more complicated
_enable_extra_compiler_warnings_flags()
set(COMPILER_FLAGS "${COMPILER_FLAGS} ${_flags}")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -fsigned-char -Wno-unknown-pragmas")

# Omit "argument unused during compilation" when clang is used with ccache.
if(${CMAKE_CXX_COMPILER} MATCHES "ccache")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Qunused-arguments")
endif()

if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
	# Force color diagnostics for Ninja generator
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -fdiagnostics-color")
endif()

# Start with an empty list
set(SANITIZE_FLAGS)

if(CLANG_ENABLE_ADDRESS_SANITIZER)
	set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
	CHECK_C_COMPILER_FLAG("-fsanitize=address" SUPPORTS_SANITIZE_ADDRESS)

	if (SUPPORTS_SANITIZE_ADDRESS)
		set(SANITIZE_FLAGS ${SANITIZE_FLAGS} "address")
	endif()
endif()
if (CLANG_ENABLE_LEAK_CHECK)
	check_linker_flag("-fsanitize=leak" SUPPORTS_FSANITIZE_LEAK)

	if (SUPPORTS_FSANITIZE_LEAK)
		set(SANITIZE_FLAGS ${SANITIZE_FLAGS} "leak")
	endif()
endif()

string(REPLACE ";" "," SANITIZE_FLAGS "${SANITIZE_FLAGS}")
if (NOT "${SANITIZE_FLAGS}" STREQUAL "")
	set(SANITIZE_FLAGS "-fsanitize=${SANITIZE_FLAGS}")
endif()

set(COMPILER_FLAGS "${COMPILER_FLAGS} ${SANITIZE_FLAGS}")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wformat-security")

# Dear Clang, please tell us if a function does not return a value since that part of the standard is stupid!
set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wreturn-type")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-char-subscripts")

check_cxx_compiler_flag(-Wshift-negative-value SUPPORTS_SHIFT_NEGATIVE_VALUE)
if(SUPPORTS_SHIFT_NEGATIVE_VALUE)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-shift-negative-value")
endif()

set(COMPILER_FLAGS_RELEASE "-O2 -Wno-unused-variable -Wno-unused-parameter")

set(COMPILER_FLAGS_DEBUG "-Og -g -Wshadow")

# Always use the base flags and add our compiler flags at the bacl
set(CMAKE_CXX_FLAGS "${CXX_BASE_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${C_BASE_FLAGS} ${COMPILER_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})

set(CMAKE_CXX_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})


set(CMAKE_EXE_LINKER_FLAGS "")

if(DEFINED ENV{LDFLAGS})
    set(CMAKE_EXE_LINKER_FLAGS $ENV{LDFLAGS})
endif()

if (CLANG_USE_LIBCXX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
endif()

if (SANITIZE_FLAGS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAGS}")
endif()

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g -rdynamic")

if (FSO_FATAL_WARNINGS)
	# Make warnings fatal if the right variable is set
	target_compile_options(compiler INTERFACE "-Werror")
endif()

# Always define this to make sure that the fixed width format macros are available
target_compile_definitions(compiler INTERFACE __STDC_FORMAT_MACROS)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
	# GNU ar: Create thin archive files.
	# Requires binutils-2.19 or later.
	set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_C_ARCHIVE_APPEND   "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

target_link_libraries(compiler INTERFACE m)
