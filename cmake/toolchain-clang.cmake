# Clang
include(EnableExtraCompilerWarnings)
include(CheckCXXCompilerFlag)

MESSAGE(STATUS "Doing configuration specific to clang...")

unset(COMPILER_FLAGS)
if(DEFINED ENV{CXXFLAGS})
	set(COMPILER_FLAGS $ENV{CXXFLAGS})
endif()

if(NOT COMPILER_FLAGS)
	set(COMPILER_FLAGS "-march=native -pipe")
endif()

globally_enable_extra_compiler_warnings()
set(COMPILER_FLAGS "${COMPILER_FLAGS} -funroll-loops -fsigned-char -Wno-unknown-pragmas")

# Omit "argument unused during compilation" when clang is used with ccache.
if(${CMAKE_CXX_COMPILER} MATCHES "ccache")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Qunused-arguments")
endif()

if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
	# Force color diagnostics for Ninja generator
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -fdiagnostics-color")
endif()

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

if (FSO_FATAL_WARNINGS)
	# Make warnings fatal if the right variable is set
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -Werror")
endif()

set(CMAKE_CXX_FLAGS ${COMPILER_FLAGS})
set(CMAKE_C_FLAGS ${COMPILER_FLAGS})

set(CMAKE_CXX_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})

set(CMAKE_CXX_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})


set(CMAKE_EXE_LINKER_FLAGS "")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g -rdynamic")
