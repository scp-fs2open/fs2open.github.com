
include(EnableExtraCompilerWarnings)
include(CheckCCompilerFlag)
include(util)

MESSAGE(STATUS "Doing configuration specific to gcc...")

option(GCC_ENABLE_LEAK_CHECK "Enable -fsanitize=leak" OFF)
option(GCC_ENABLE_ADDRESS_SANITIZER "Enable -fsanitize=address" OFF)

unset(COMPILER_FLAGS)
if(DEFINED ENV{CXXFLAGS})
	set(COMPILER_FLAGS $ENV{CXXFLAGS})
endif()

if(NOT COMPILER_FLAGS)
	set(COMPILER_FLAGS "-march=native -pipe")
endif()

globally_enable_extra_compiler_warnings()
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
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fsanitize=address")
	endif()

	set(SANITIZE_FLAGS ${SANITIZE_FLAGS} "address")
endif()
if (GCC_ENABLE_LEAK_CHECK)
	check_linker_flag("-fsanitize=leak" SUPPORTS_FSANITIZE_LEAK)

	if (SUPPORTS_FSANITIZE_LEAK)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=leak")
	endif()

	set(SANITIZE_FLAGS ${SANITIZE_FLAGS} "leak")
endif()

string(REPLACE ";" "," SANITIZE_FLAGS "${SANITIZE_FLAGS}")
if (NOT "${SANITIZE_FLAGS}" STREQUAL "")
	set(SANITIZE_FLAGS "-fsanitize=${SANITIZE_FLAGS}")
endif()

set(COMPILER_FLAGS "${COMPILER_FLAGS} ${SANITIZE_FLAGS}")

# Omit "deprecated conversion from string constant to 'char*'" warnings.
set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-write-strings")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-unused-function")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-deprecated -Wno-char-subscripts")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-unused-parameter")

set(COMPILER_FLAGS_RELEASE "-O2 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-array-bounds -Wno-empty-body -Wno-clobbered")

set(COMPILER_FLAGS_DEBUG "-O0 -g -Wshadow")

set(CMAKE_CXX_FLAGS ${COMPILER_FLAGS})
set(CMAKE_C_FLAGS ${COMPILER_FLAGS})

set(CMAKE_CXX_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})

set(CMAKE_CXX_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})

set(CMAKE_EXE_LINKER_FLAGS "")

IF (MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++ -Wl,--enable-auto-import")
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
