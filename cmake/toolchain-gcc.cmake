
include(EnableExtraCompilerWarnings)
include(CheckCCompilerFlag)
include(util)

MESSAGE(STATUS "Doing configuration specific to gcc...")

option(GCC_ENABLE_LEAK_CHECK "Enable -fsanitize=leak" OFF)
option(GCC_ENABLE_ADDRESS_SANITIZER "Enable -fsanitize=address" OFF)
option(GCC_ENABLE_SANITIZE_UNDEFINED "Enable -fsanitize=undefined" OFF)
option(GCC_USE_GOLD "Use the gold linker instead of the standard linker" OFF)
option(GCC_GENERATE_GDB_INDEX "Adds linker option to generate the gdb index for debug builds" OFF)

# These are the default values
set(C_BASE_FLAGS "-pipe")
set(CXX_BASE_FLAGS "-pipe")

if(IS_X86)
	if(FORCED_NATIVE_SIMD_INSTRUCTIONS)
		set(GCC_EXTENSIONS "-march=native")
	elseif (FSO_INSTRUCTION_SET STREQUAL "")
		set(GCC_EXTENSIONS "-march=x86-64")
	elseif (FSO_INSTRUCTION_SET STREQUAL "SSE")
		set(GCC_EXTENSIONS "-march=x86-64 -msse -mfpmath=sse")
	elseif (FSO_INSTRUCTION_SET STREQUAL "SSE2")
		set(GCC_EXTENSIONS "-march=x86-64 -msse -msse2 -mfpmath=sse")
	elseif (FSO_INSTRUCTION_SET STREQUAL "AVX")
		set(GCC_EXTENSIONS "-march=x86-64-v2 -msse -msse2 -mavx -mfpmath=sse")
	elseif (FSO_INSTRUCTION_SET STREQUAL "AVX2")
		set(GCC_EXTENSIONS "-march=x86-64-v3 -msse -msse2 -mavx -mavx2 -mfpmath=sse")
	else ()
		message( FATAL_ERROR "Unknown instruction set encountered for GCC. Update toolchain-gcc.cmake!" )
	endif()

	set(C_BASE_FLAGS "${C_BASE_FLAGS} ${GCC_EXTENSIONS}")
	set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} ${GCC_EXTENSIONS}")

	if (IS_64BIT)
		set(C_BASE_FLAGS "${C_BASE_FLAGS} -m64")
		set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} -m64")
	endif()
elseif(IS_ARM)
	if(FORCED_NATIVE_SIMD_INSTRUCTIONS)
		set(C_BASE_FLAGS "${C_BASE_FLAGS} -march=native")
		set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} -march=native")
    endif ()
elseif(IS_RISCV)
    # You do not need to pass a -march, passing nothing will make gcc to choose itself.
    # If you want a specific set of instructions like vectors, set -march in CFLAGS and CXXFLAGS env variables
    # Example for vectors: -march=rv64gcv
    # https://gcc.gnu.org/onlinedocs/gcc/RISC-V-Options.html
    # As such, default C/CXX_BASE_FLAGS are sufficient.
endif()

if (USE_STATIC_LIBCXX)
	set(CXX_BASE_FLAGS "${CXX_BASE_FLAGS} -static-libstdc++")
endif()

# For C and C++, the values can be overwritten independently
if(DEFINED ENV{CFLAGS})
	set(C_BASE_FLAGS $ENV{CFLAGS})
endif()
if(DEFINED ENV{CXXFLAGS})
	set(CXX_BASE_FLAGS $ENV{CXXFLAGS})
endif()

# Initialize with an empty string to make sure we always get a clean start
set(COMPILER_FLAGS "")
set(LINKER_FLAGS "")

# Don't ignore user-set LDFLAGS
if(DEFINED ENV{LDFLAGS})
	set(LINKER_FLAGS $ENV{LDFLAGS})
endif()

if (GCC_USE_GOLD)
	OPTION(GCC_INCREMENTAL_LINKING "Use incremental linking" OFF)
	set(LINKER_FLAGS "${LINKER_FLAGS} -fuse-ld=gold")

	if (GCC_INCREMENTAL_LINKING)
		set(LINKER_FLAGS "${LINKER_FLAGS} -fno-use-linker-plugin -Wl,-z,norelro -Wl,--incremental")
	endif()
endif()

# This is a slight hack since our flag setup is a bit more complicated
_enable_extra_compiler_warnings_flags()
set(COMPILER_FLAGS "${COMPILER_FLAGS} ${_flags}")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -fsigned-char -Wno-unknown-pragmas")

if (NOT GCC_INCREMENTAL_LINKING)
	# Place each function and data in its own section so the linker can
	# perform dead code elimination
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -fdata-sections -ffunction-sections")
endif()

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

# Dear GCC, please tell us if a function does not return a value since that part of the standard is stupid!
set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wreturn-type")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-deprecated -Wno-char-subscripts")

# These two warnings cause a lot of false-positives in the FSO code base
check_cxx_compiler_flag(-Wstringop-truncation SUPPORTS_STRINGOP_TRUNCATION)
if(SUPPORTS_STRINGOP_TRUNCATION)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-stringop-truncation")
endif()

check_cxx_compiler_flag(-Wstringop-overflow SUPPORTS_STRINGOP_OVERFLOW)
if(SUPPORTS_STRINGOP_TRUNCATION)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-stringop-overflow")
endif()

# Check if there is a user-set optimisation flag
string(REGEX MATCH "-O[a-zA-Z|0-9]+" CXX_OPT_FLAG ${CXX_BASE_FLAGS})
string(REGEX MATCH "-O[a-zA-Z|0-9]+" C_OPT_FLAG ${C_BASE_FLAGS})

# If no user-set opt flag, set -O3 and -Og
if ("${CXX_OPT_FLAG}" STREQUAL "")
	set(CXX_OPT_FLAG_RELEASE "-O3")
	set(CXX_OPT_FLAG_DEBUG "-O0")
else()
	set(CXX_OPT_FLAG_RELEASE "${CXX_OPT_FLAG}")
	set(CXX_OPT_FLAG_DEBUG "${CXX_OPT_FLAG}")
endif()
if ("${C_OPT_FLAG}" STREQUAL "")
	set(C_OPT_FLAG_RELEASE "-O3")
	set(C_OPT_FLAG_DEBUG "-O0")
else()
	set(C_OPT_FLAG_RELEASE "${C_OPT_FLAG}")
	set(C_OPT_FLAG_DEBUG "${C_OPT_FLAG}")
endif()

set(CXX_FLAGS_RELEASE "${CXX_OPT_FLAG_RELEASE} -Wno-unused-variable -Wno-unused-but-set-variable -Wno-array-bounds -Wno-empty-body -Wno-clobbered -Wno-unused-parameter")
set(C_FLAGS_RELEASE "${C_OPT_FLAG_RELEASE} -Wno-unused-variable -Wno-unused-but-set-variable -Wno-array-bounds -Wno-empty-body -Wno-clobbered -Wno-unused-parameter")

set(CXX_FLAGS_DEBUG "${CXX_OPT_FLAG_DEBUG} -g -Wshadow")
set(C_FLAGS_DEBUG "${C_OPT_FLAG_DEBUG} -g -Wshadow")

# Always use the base flags and add our compiler flags at the back
set(CMAKE_CXX_FLAGS "${CXX_BASE_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${C_BASE_FLAGS} ${COMPILER_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE ${CXX_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASE ${C_FLAGS_RELEASE})

set(CMAKE_CXX_FLAGS_DEBUG ${CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUG ${C_FLAGS_DEBUG})

set(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}")

IF (MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++ -Wl,--enable-auto-import")
	target_compile_definitions(compiler INTERFACE __USE_MINGW_ANSI_STDIO=1)
ENDIF(MINGW)

if (SANITIZE_FLAGS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAGS}")
endif()

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")

if (GCC_GENERATE_GDB_INDEX)
	# For pure debug binaries, generate a gdb index for better debugging
	SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--gdb-index")
endif()

IF(NOT MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -rdynamic")
	# Allow the linker to perform dead code elimination; this is considered
	# experimental for COFF and PE formats and thus disabled for Windows builds
	# (https://sourceware.org/binutils/docs/ld/Options.html)
	if (NOT GCC_INCREMENTAL_LINKING AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
	endif()
ENDIF(NOT MINGW)

IF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-zignore")
ENDIF()

if (FSO_FATAL_WARNINGS)
	# Make warnings fatal if the right variable is set
	target_compile_options(compiler INTERFACE "-Werror")
endif()

# Always define this to make sure that the fixed-width format macros are available
target_compile_definitions(compiler INTERFACE __STDC_FORMAT_MACROS=1)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
	# GNU ar: Create thin archive files.
	# Requires binutils-2.19 or later.
	set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_C_ARCHIVE_APPEND   "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
	set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
