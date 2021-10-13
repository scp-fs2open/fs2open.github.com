include(CheckStructHasMember)

check_struct_has_member("struct sockaddr" sa_len
        "sys/socket.h"
        HAVE_SOCKADDR_SA_LEN)

set(LIBPCP_INCLUDE_DIRECTORIES
    include
    src
    src/net
)

if(WIN32)
    set(LIBPCP_INCLUDE_DIRECTORIES
        ${LIBPCP_INCLUDE_DIRECTORIES}
        include/windows
        src/windows
        win_utils
    )
endif(WIN32)

# include directories with source and header files
include_directories(${LIBPCP_INCLUDE_DIRECTORIES} )


set(LIBPCP_SOURCES
    include/pcp.h
    src/net/gateway.c
    src/net/gateway.h
    src/net/findsaddr-udp.c
    src/pcp_api.c
    src/pcp_client_db.c
    src/pcp_client_db.h
    src/pcp_event_handler.c
    src/pcp_event_handler.h
    src/pcp_logger.c
    src/pcp_logger.h
    src/pcp_msg.c
    src/pcp_msg.h
    src/pcp_server_discovery.c
    src/pcp_server_discovery.h
    src/net/sock_ntop.c
    src/net/pcp_socket.c
    src/net/pcp_socket.h
    src/net/findsaddr.h
    src/net/unp.h
)

if(WIN32)
    set(LIBPCP_SOURCES
        ${LIBPCP_SOURCES}
        src/windows/pcp_gettimeofday.c
        src/windows/pcp_gettimeofday.h
        src/windows/pcp_win_defines.h
        src/windows/stdint.h
    )
endif(WIN32)


add_library(pcp STATIC ${LIBPCP_SOURCES})


if(WIN32)
    if(MINGW)
        target_compile_definitions(pcp PRIVATE HAVE_GETTIMEOFDAY)
    endif(MINGW)

    # target XP
    target_compile_definitions(pcp PRIVATE WIN32 NTDDI_VERSION=0x06000000 _WIN32_WINNT=0x0600)
    target_link_libraries(pcp INTERFACE iphlpapi ws2_32)
endif(WIN32)

suppress_warnings(pcp)

set_target_properties(pcp PROPERTIES FOLDER "3rdparty")
target_link_libraries(pcp PUBLIC compiler)

target_include_directories(pcp SYSTEM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
