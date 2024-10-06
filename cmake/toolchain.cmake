
# File to execute compiler specific commands

add_library(compiler INTERFACE)

target_compile_definitions(compiler INTERFACE "$<$<CONFIG:Release>:NDEBUG>;$<$<CONFIG:Debug>:_DEBUG>;$<$<CONFIG:FastDebug>:_DEBUG>")

set(FORCED_NATIVE_SIMD_INSTRUCTIONS ON CACHE BOOL "Override instruction set detection and compile with the maximum possible instructions for the current system.")
IF(IS_X86)
	SET(POSSIBLE_INSTRUCTION_SETS "" SSE SSE2 AVX AVX2)
	detect_simd_instructions(DETECTED_SIMD_INSTRUCTIONS)

	set(FORCED_SIMD_INSTRUCTIONS "" CACHE STRING "Overrides any instruction set settings (including forced-native). Any of ${POSSIBLE_INSTRUCTION_SETS}.")
	if (NOT FORCED_SIMD_INSTRUCTIONS STREQUAL "" AND NOT FORCED_NATIVE_SIMD_INSTRUCTIONS)
		LIST(FIND POSSIBLE_INSTRUCTION_SETS "${FORCED_SIMD_INSTRUCTIONS}" SET_INDEX)

		if (SET_INDEX LESS 0)
			MESSAGE(STATUS "An invalid instruction set was specified, defaulting to no special compiler options.")
			set(FSO_INSTRUCTION_SET "")
		else ()
			LIST(FIND POSSIBLE_INSTRUCTION_SETS "${DETECTED_SIMD_INSTRUCTIONS}" SET_INDEX)
			if (SET_INDEX LESS 0)
				MESSAGE(STATUS "An instruction set was specified which is unsupported on this platform. Compiled binary might not be executable.")
			endif()

			set(FSO_INSTRUCTION_SET ${FORCED_SIMD_INSTRUCTIONS})
		endif()
	else()
		SET(FSO_INSTRUCTION_SET ${DETECTED_SIMD_INSTRUCTIONS})
	endif()
	MESSAGE(STATUS "Using instruction set level ${FSO_INSTRUCTION_SET}. Optimizations for host-system specifically: ${FORCED_NATIVE_SIMD_INSTRUCTIONS}.")

	if (FSO_INSTRUCTION_SET STREQUAL "SSE")
		add_compile_definitions(FSO_SIMD_SSE)
	elseif (FSO_INSTRUCTION_SET STREQUAL "SSE2")
		add_compile_definitions(FSO_SIMD_SSE)
		add_compile_definitions(FSO_SIMD_SSE2)
	elseif (FSO_INSTRUCTION_SET STREQUAL "AVX")
		add_compile_definitions(FSO_SIMD_SSE)
		add_compile_definitions(FSO_SIMD_SSE2)
		add_compile_definitions(FSO_SIMD_AVX)
	elseif (FSO_INSTRUCTION_SET STREQUAL "AVX2")
		add_compile_definitions(FSO_SIMD_SSE)
		add_compile_definitions(FSO_SIMD_SSE2)
		add_compile_definitions(FSO_SIMD_AVX)
		add_compile_definitions(FSO_SIMD_AVX2)
	endif()
ENDIF()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	include(toolchain-gcc)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	include(toolchain-msvc)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	include(toolchain-clang)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	include(toolchain-apple-clang)
ELSE("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	MESSAGE(STATUS "No special handling for this compiler present, good luck!")
ENDIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

find_package(Threads REQUIRED)
target_link_libraries(compiler INTERFACE Threads::Threads)

# Copy release settings to FastDebug
set(CMAKE_C_FLAGS_FASTDEBUG "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_FASTDEBUG "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_FASTDEBUG "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_STATIC_LINKER_FLAGS_FASTDEBUG "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}")

if (MSVC)
	# Adjust runtime library
	string(REPLACE "/MD" "/MDd" CMAKE_C_FLAGS_FASTDEBUG "${CMAKE_C_FLAGS_FASTDEBUG}")
	string(REPLACE "/MT" "/MTd" CMAKE_C_FLAGS_FASTDEBUG "${CMAKE_C_FLAGS_FASTDEBUG}")
	
	string(REPLACE "/MD" "/MDd" CMAKE_CXX_FLAGS_FASTDEBUG "${CMAKE_CXX_FLAGS_FASTDEBUG}")
	string(REPLACE "/MT" "/MTd" CMAKE_CXX_FLAGS_FASTDEBUG "${CMAKE_CXX_FLAGS_FASTDEBUG}")
endif()
