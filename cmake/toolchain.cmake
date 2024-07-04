
# File to execute compiler specific commands

add_library(compiler INTERFACE)

target_compile_definitions(compiler INTERFACE "$<$<CONFIG:Release>:NDEBUG>;$<$<CONFIG:Debug>:_DEBUG>;$<$<CONFIG:FastDebug>:_DEBUG>")

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
