
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(CheckPrototypeDefinition)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)

CHECK_INCLUDE_FILE("execinfo.h" HAVE_EXECINFO_H)
CHECK_INCLUDE_FILE_CXX("cxxabi.h" HAVE_CXXAPI_H)

# Check if we have glu.h
FIND_PACKAGE(OpenGL REQUIRED)
if (APPLE)
    set(GL_INCLUDE_DIR "${OPENGL_INCLUDE_DIR}/Headers")
else()
    set(GL_INCLUDE_DIR "${OPENGL_INCLUDE_DIR}/GL")
endif()
set(CMAKE_REQUIRED_INCLUDES "${GL_INCLUDE_DIR}")
CHECK_INCLUDE_FILE("glu.h" HAVE_GLU_H)

set(PLATFORM_CHECK_HEADER "${GENERATED_SOURCE_DIR}/platformChecks.h")
CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/platformChecks.h.in "${PLATFORM_CHECK_HEADER}")
