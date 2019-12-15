AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/src/net -I$(srcdir)/src
AM_CPPFLAGS += $(PCP_CPPFLAGS)
AM_CFLAGS = $(PCP_CFLAGS)

pkginclude_HEADERS = include/pcp.h
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libpcp-client.pc

lib_LTLIBRARIES = libpcp-client.la
libpcp_client_la_SOURCES = src/pcp_logger.c\
                    src/pcp_server_discovery.c\
                    src/pcp_client_db.c\
                    src/pcp_msg.c\
                    src/pcp_event_handler.c\
                    src/net/gateway.c\
                    src/pcp_msg_structs.h\
                    src/pcp_api.c\
                    src/net/findsaddr-udp.c \
                    src/net/sock_ntop.c \
                    src/net/pcp_socket.c

noinst_HEADERS =    src/net/pcp_socket.h\
                    src/net/gateway.h\
                    src/pcp_event_handler.h\
                    src/pcp_msg.h\
                    src/pcp_client_db.h\
                    src/pcp_logger.h\
                    src/pcp_server_discovery.h\
                    src/pcp_utils.h \
                    src/net/findsaddr.h \
                    src/net/unp.h

libpcp_client_la_LDFLAGS = -version-info 0:0:0 -release 2 ${PCP_LDFLAGS}

libpcp_client_la_LIBADD = $(GCOVLIB)

