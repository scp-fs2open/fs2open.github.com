
INCLUDE(util)

MESSAGE(STATUS "Configuring UNIX specific things and stuff...")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/finder")

include (FindLTTngUST REQUIRED)

target_link_libraries(platform INTERFACE LTTng::UST)

target_compile_definitions(platform INTERFACE SCP_UNIX USE_OPENAL)

# Set RPATH
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN")

set(PLATFORM_UNIX TRUE CACHE INTERNAL "" FORCE)
