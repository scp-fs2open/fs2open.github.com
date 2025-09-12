
SET(BINARY_DESTINATION ".")
SET(LIBRAY_DESTINATION "${BINARY_DESTINATION}")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64BIT TRUE)
else()
    set(IS_64BIT FALSE)
endif()

set(IS_ARM64 FALSE)
set(IS_ARMV7A FALSE)

if (NOT "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "")   # needed to cover Visual Studio generator
    if(CMAKE_GENERATOR_PLATFORM MATCHES "^(aarch64|arm64|ARM64)")
        set(IS_ARM64 TRUE)
	elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^(armv7)")
        set(IS_ARMV7A TRUE)
    endif()
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)")
        set(IS_ARM64 TRUE)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv7)")
        set(IS_ARMV7A TRUE)
    endif()
endif()

set(IS_RISCV FALSE)

if (NOT "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "")   # needed to cover Visual Studio generator
    if(CMAKE_GENERATOR_PLATFORM MATCHES "^(riscv64|RISCV64|riscv32|RISCV32)")
        set(IS_RISCV TRUE)
    endif()
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(riscv64|RISCV64|riscv32|RISCV32)")
        set(IS_RISCV TRUE)
    endif()
endif()

set(IS_X86 FALSE)
if (NOT IS_ARM64 AND NOT IS_RISCV AND NOT IS_ARMV7A)
    set(IS_X86 TRUE)
endif()
