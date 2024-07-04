
FIND_PACKAGE(Threads)

set(CMAKE_CXX_STANDARD 14)

set(BASE_RPC_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/discord_rpc.h
    discord_rpc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/discord_register.h
    rpc_connection.h
    rpc_connection.cpp
    serialization.h
    serialization.cpp
    connection.h
    backoff.h
    msg_queue.h
)

if (${BUILD_SHARED_LIBS})
    if(WIN32)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} dllmain.cpp)
    endif(WIN32)
endif(${BUILD_SHARED_LIBS})

if(WIN32)
    add_definitions(-DDISCORD_WINDOWS)
    set(BASE_RPC_SRC ${BASE_RPC_SRC} connection_win.cpp discord_register_win.cpp)
    add_library(discord-rpc ${BASE_RPC_SRC})
endif(WIN32)

if(UNIX)
    set(BASE_RPC_SRC ${BASE_RPC_SRC} connection_unix.cpp)

    if (APPLE)
        add_definitions(-DDISCORD_OSX)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} discord_register_osx.m)
    else (APPLE)
        add_definitions(-DDISCORD_LINUX)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} discord_register_linux.cpp)
    endif(APPLE)

    add_library(discord-rpc ${BASE_RPC_SRC})

    if (${BUILD_SHARED_LIBS})
        target_compile_options(discord-rpc PRIVATE -fPIC)
    endif (${BUILD_SHARED_LIBS})

    if (APPLE)
        target_link_libraries(discord-rpc PRIVATE "-framework AppKit")
    endif (APPLE)
endif(UNIX)

target_link_libraries(discord-rpc PUBLIC Threads::Threads)

target_link_libraries(discord-rpc PUBLIC compiler)

suppress_warnings(discord-rpc)

target_include_directories(discord-rpc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty")
target_include_directories(discord-rpc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")

if (${BUILD_SHARED_LIBS})
    target_compile_definitions(discord-rpc PUBLIC -DDISCORD_DYNAMIC_LIB)
    target_compile_definitions(discord-rpc PRIVATE -DDISCORD_BUILDING_SDK)
endif(${BUILD_SHARED_LIBS})
