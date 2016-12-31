# Clang
include(EnableExtraCompilerWarnings)
include(CheckCXXCompilerFlag)

MESSAGE(STATUS "Doing configuration specific to clang...")

option(CLANG_ENABLE_LEAK_CHECK "Enable -fsanitize=leak" OFF)
option(CLANG_ENABLE_ADDRESS_SANITIZER "Enable -fsanitize=address" OFF)

unset(COMPILER_FLAGS)
if(DEFINED ENV{CXXFLAGS})
	set(COMPILER_FLAGS $ENV{CXXFLAGS})
endif()

if(NOT COMPILER_FLAGS)
	set(COMPILER_FLAGS "-march=native -pipe")
endif()

# This is a slight hack since our flag setup is a bit more complicated
_enable_extra_compiler_warnings_flags()
set(COMPILER_FLAGS "${COMPILER_FLAGS} ${_flags}")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -funroll-loops -fsigned-char -Wno-unknown-pragmas")

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

# Omit "deprecated conversion from string constant to 'char*'" warnings.
set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-write-strings")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-unused-function")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-deprecated -Wno-char-subscripts")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-unused-parameter")

check_cxx_compiler_flag(-Wshift-negative-value SUPPORTS_SHIFT_NEGATIVE_VALUE)
if(SUPPORTS_SHIFT_NEGATIVE_VALUE)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-shift-negative-value")
endif()

set(COMPILER_FLAGS_RELEASE "-O2 -Wno-unused-variable")

set(COMPILER_FLAGS_DEBUG "-O0 -g -Wshadow")

set(CMAKE_CXX_FLAGS ${COMPILER_FLAGS})
set(CMAKE_C_FLAGS ${COMPILER_FLAGS})

set(CMAKE_CXX_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})

set(CMAKE_CXX_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})


set(CMAKE_EXE_LINKER_FLAGS "")

if (SANITIZE_FLAGS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAGS}")
endif()

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g -rdynamic")

if (FSO_FATAL_WARNINGS)
	# Make warnings fatal if the right variable is set
	target_compile_options(compiler INTERFACE "-Werror")
endif()
