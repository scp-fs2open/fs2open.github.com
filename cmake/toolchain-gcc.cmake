
include(EnableExtraCompilerWarnings)
include(util)

MESSAGE(STATUS "Doing configuration specific to gcc...")

option(GCC_ENABLE_LEAK_CHECK "Enable the -fsanitize=leak" OFF)

unset(COMPILER_FLAGS)
if(DEFINED ENV{CXXFLAGS})
	set(COMPILER_FLAGS $ENV{CXXFLAGS})
endif()

if(NOT COMPILER_FLAGS)
	set(COMPILER_FLAGS "-march=native -pipe")
endif()

globally_enable_extra_compiler_warnings()
set(COMPILER_FLAGS "${COMPILER_FLAGS} -funroll-loops -fsigned-char -Wno-unknown-pragmas")

if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
	# Force color diagnostics for Ninja generator
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -fdiagnostics-color")
endif()

# Omit "deprecated conversion from string constant to 'char*'" warnings.
set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-write-strings")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-unused-function")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-deprecated -Wno-char-subscripts")

set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wno-unused-parameter")

set(COMPILER_FLAGS_RELEASE "-O2 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-array-bounds -Wno-empty-body -Wno-clobbered")

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

if (GCC_ENABLE_LEAK_CHECK)
	check_linker_flag("-fsanitize=leak" SUPPORTS_FSANITIZE_LEAK)

	if (SUPPORTS_FSANITIZE_LEAK)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=leak")
	endif()
endif()

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g -rdynamic")

IF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
	SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-zignore")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
