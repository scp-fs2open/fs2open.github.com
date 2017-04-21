
target_compile_definitions(platform INTERFACE SCP_SOLARIS)

find_library(SOCKET_LIB socket)

if (NOT SOCKET_LIB)
    message(FATAL_ERROR "Couldn't find libsocket! Please install it and rerun the configuration.")
else()
    target_link_libraries(platform INTERFACE "${SOCKET_LIB}")
endif()

find_library(NSL_LIB nsl)

if (NOT NSL_LIB)
    message(FATAL_ERROR "Couldn't find libnsl! Please install it and rerun the configuration.")
else()
    target_link_libraries(platform INTERFACE "${NSL_LIB}")
endif()
