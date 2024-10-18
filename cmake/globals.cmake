
SET(BINARY_DESTINATION ".")
SET(LIBRAY_DESTINATION "${BINARY_DESTINATION}")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64BIT TRUE)
else()
    set(IS_64BIT FALSE)
endif()

set(IS_ARM64 FALSE)

if (NOT "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "")   # needed to cover Visual Studio generator
    if(CMAKE_GENERATOR_PLATFORM MATCHES "^(aarch64|arm64|ARM64)")
        set(IS_ARM64 TRUE)
    endif()
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)")
        set(IS_ARM64 TRUE)
    endif()
endif()

set(IS_X86 FALSE)
if (NOT IS_ARM64)
    set(IS_X86 TRUE)
endif()
