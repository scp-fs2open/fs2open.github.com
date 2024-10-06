# Clang
include(EnableExtraCompilerWarnings)
include(CheckCXXCompilerFlag)

MESSAGE(STATUS "Doing configuration specific to clang...")

option(CLANG_ENABLE_LEAK_CHECK "Enable -fsanitize=leak" OFF)
option(CLANG_ENABLE_ADDRESS_SANITIZER "Enable -fsanitize=address" OFF)

option(CLANG_USE_LIBCXX "Use libc++" OFF)

# These are the default values
set(C_BASE_FLAGS "-pipe")
set(CXX_BASE_FLAGS "-pipe")

if(IS_X86)
	if(FORCED_NATIVE_SIMD_INSTRUCTIONS)
		set(CLANG_EXTENSIONS "-march=native")
	elseif (FSO_INSTRUCTION_SET STREQUAL "")
		set(CLANG_EXTENSIONS "-march=x86-64")
	elseif (FSO_INSTRUCTION_SET STREQUAL "SSE")
		set(CLANG_EXTENSIONS "-march=x86-64 -msse -mfpmath=sse")
	elseif (FSO_INSTRUCTION_SET STREQUAL "SSE2")
		set(CLANG_EXTENSIONS "-march=x86-64 -msse -msse2 -mfpmath=sse")
	elseif (FSO_INSTRUCTION_SET STREQUAL "AVX")
		set(CLANG_EXTENSIONS "-march=x86-64-v2 -msse -msse2 -mavx -mfpmath=sse")
	elseif (FSO_INSTRUCTION_SET STREQUAL "AVX2")
		set(CLANG_EXTENSIONS "-march=x86-64-v3 -msse -msse2 -mavx -mavx2 -mfpmath=sse")
	else ()
		message( FATAL_ERROR "Unknown instruction set encountered for clang. Update toolchain-clang.cmake!" )
	endif()

	set(C_BASE_FLAGS "${C_BASE_FLAGS} ${CLANG_EXTENSIONS}")
	set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} ${CLANG_EXTENSIONS}")
elseif(IS_ARM)
	if(FORCED_NATIVE_SIMD_INSTRUCTIONS)
		set(C_BASE_FLAGS "${C_BASE_FLAGS} -march=native")
		set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} -march=native")
	endif ()
endif()

if (USE_STATIC_LIBCXX)
	set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} -static-libstdc++ -Qunused-arguments")
	set(CLANG_USE_LIBCXX ON)
endif()

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

# Check if there is a user-set optimisation flag
string(REGEX MATCH "-O[a-zA-Z|0-9]+" CXX_OPT_FLAG ${CXX_BASE_FLAGS})
string(REGEX MATCH "-O[a-zA-Z|0-9]+" C_OPT_FLAG ${C_BASE_FLAGS})

# If no user-set opt flag, set -O2 and -Og
if ("${CXX_OPT_FLAG}" STREQUAL "")
	set(CXX_OPT_FLAG_RELEASE "-O2")
	set(CXX_OPT_FLAG_DEBUG "-Og")
else()
	set(CXX_OPT_FLAG_RELEASE "${CXX_OPT_FLAG}")
	set(CXX_OPT_FLAG_DEBUG "${CXX_OPT_FLAG}")
endif()
if ("${C_OPT_FLAG}" STREQUAL "")
	set(C_OPT_FLAG_RELEASE "-O2")
	set(C_OPT_FLAG_DEBUG "-Og")
else()
	set(C_OPT_FLAG_RELEASE "${C_OPT_FLAG}")
	set(C_OPT_FLAG_DEBUG "${C_OPT_FLAG}")
endif()

set(CXX_FLAGS_RELEASE "${CXX_OPT_FLAG_RELEASE} -Wno-unused-variable -Wno-unused-parameter")
set(C_FLAGS_RELEASE "${C_OPT_FLAG_RELEASE} -Wno-unused-variable -Wno-unused-parameter")

set(CXX_FLAGS_DEBUG "${CXX_OPT_FLAG_DEBUG} -g -Wshadow")
set(C_FLAGS_DEBUG "${C_OPT_FLAG_DEBUG} -g -Wshadow")

# Always use the base flags and add our compiler flags at the back
set(CMAKE_CXX_FLAGS "${CXX_BASE_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${C_BASE_FLAGS} ${COMPILER_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE ${CXX_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASE ${C_FLAGS_RELEASE})

set(CMAKE_CXX_FLAGS_DEBUG ${CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUG ${C_FLAGS_DEBUG})

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

# Always define this to make sure that the fixed-width format macros are available
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
