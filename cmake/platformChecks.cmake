
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(CheckPrototypeDefinition)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckTypeSize)

CHECK_INCLUDE_FILE("execinfo.h" HAVE_EXECINFO_H)
CHECK_INCLUDE_FILE_CXX("cxxabi.h" HAVE_CXXAPI_H)

CHECK_TYPE_SIZE("max_align_t" MAX_ALIGN_T)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_REQUIRED_FLAGS "-std=c++11") # required for g++ <= 5
endif()

set(CMAKE_EXTRA_INCLUDE_FILES "cstddef")
CHECK_TYPE_SIZE("std::max_align_t" STD_MAX_ALIGN_T LANGUAGE CXX)
set(CMAKE_EXTRA_INCLUDE_FILES "type_traits")
CHECK_TYPE_SIZE("std::is_trivially_copyable<int>" STD_IS_TRIVIALLY_COPYABLE LANGUAGE CXX)
set(CMAKE_EXTRA_INCLUDE_FILES)

CHECK_TYPE_SIZE("char32_t" CHAR32_T LANGUAGE CXX)
CHECK_TYPE_SIZE("U'b'" UNICODE_CHAR_LITERAL LANGUAGE CXX)

set(CMAKE_REQUIRED_FLAGS)


CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)

CHECK_FUNCTION_EXISTS(_stricmp HAVE__STRICMP)
CHECK_FUNCTION_EXISTS(_strnicmp HAVE__STRNICMP)

CHECK_FUNCTION_EXISTS(strlwr HAVE_STRLWR)

CHECK_INCLUDE_FILE("strings.h" HAVE_STRINGS_H)

check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(_snprintf "stdio.h" HAVE__SNPRINTF)

set(PLATFORM_CHECK_HEADER "${GENERATED_SOURCE_DIR}/platformChecks.h")
CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/platformChecks.h.in "${PLATFORM_CHECK_HEADER}")
