# Clang
include(EnableExtraCompilerWarnings)
include(CheckCXXCompilerFlag)

MESSAGE(STATUS "Doing configuration specific to clang...")

unset(CMAKE_CXX_FLAGS)
	message(STATUS "$ENV{CXXFLAGS}")
if(DEFINED ENV{CXXFLAGS})
	message(STATUS "$ENV{CXXFLAGS}")
	set(CMAKE_CXX_FLAGS $ENV{CXXFLAGS})
endif()

if(NOT CMAKE_CXX_FLAGS)
	set(CMAKE_CXX_FLAGS "-march=native -pipe")
endif(NOT CMAKE_CXX_FLAGS)

globally_enable_extra_compiler_warnings()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -fsigned-char -Wno-unknown-pragmas")

# Omit "argument unused during compilation" when clang is used with ccache.
if(${CMAKE_CXX_COMPILER} MATCHES "ccache")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
endif()
# Omit "conversion from string literal to 'char *' is deprecated" warnings.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-char-subscripts")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")

check_cxx_compiler_flag(-Wshift-negative-value SUPPORTS_SHIFT_NEGATIVE_VALUE)
if(SUPPORTS_SHIFT_NEGATIVE_VALUE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shift-negative-value")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wno-unused-variable")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wshadow")

if (FSO_FATAL_WARNINGS)
	# Make warnings fatal if the right variable is set
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

set(CMAKE_EXE_LINKER_FLAGS "")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g -rdynamic")
