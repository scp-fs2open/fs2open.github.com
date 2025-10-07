/*
 Copyright (c) 2014 by Cisco Systems, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#include "default_config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include "pcp_win_defines.h"

#include <mswsock.h>
#include <ws2def.h>
#else // WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#ifndef PCP_SOCKET_IS_VOIDPTR
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif // PCP_SOCKET_IS_VOIDPTR
#endif //! WIN32
#include "pcpnatpmp.h"

#include "pcp_socket.h"
#include "pcp_utils.h"
#include "unp.h"

static PCP_SOCKET pcp_socket_create_impl(int domain, int type, int protocol);
static ssize_t pcp_socket_recvfrom_impl(PCP_SOCKET sock, void *buf, size_t len,
                                        int flags, struct sockaddr *src_addr,
                                        socklen_t *addrlen,
                                        struct sockaddr_in6 *dst_addr);
static ssize_t pcp_socket_sendto_impl(PCP_SOCKET sock, const void *buf,
                                      size_t len, int flags,
                                      const struct sockaddr_in6 *src_addr,
                                      struct sockaddr *dest_addr,
                                      socklen_t addrlen);
static int pcp_socket_close_impl(PCP_SOCKET sock);

pcp_socket_vt_t default_socket_vt = {
    pcp_socket_create_impl, pcp_socket_recvfrom_impl, pcp_socket_sendto_impl,
    pcp_socket_close_impl};

#ifdef WIN32
// function calling WSAStartup (used in pcp-server and pcp_app)
int pcp_win_sock_startup() {
    int err;
    WORD wVersionRequested;
    WSADATA wsaData;
    OSVERSIONINFOEX osvi;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        perror("WSAStartup failed with error");
        return 1;
    }
    // find windows version
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (!GetVersionEx((LPOSVERSIONINFO)(&osvi))) {
        printf("pcp_app: GetVersionEx failed");
        return 1;
    }

    return 0;
}

/* function calling WSACleanup
 *  returns 0 on success and 1 on failure
 */
int pcp_win_sock_cleanup() {
    if (WSACleanup() == PCP_SOCKET_ERROR) {
        printf("WSACleanup failed.\n");
        return 1;
    }
    return 0;
}
#endif

void pcp_fill_in6_addr(struct in6_addr *dst_ip6, uint16_t *dst_port,
                       uint32_t *dst_scope_id, struct sockaddr *src) {
    if (src->sa_family == AF_INET) {
        struct sockaddr_in *src_ip4 = (struct sockaddr_in *)src;

        if (dst_ip6) {
            S6_ADDR32(dst_ip6)[0] = 0;
            S6_ADDR32(dst_ip6)[1] = 0;
            S6_ADDR32(dst_ip6)[2] = htonl(0xFFFF);
            S6_ADDR32(dst_ip6)[3] = src_ip4->sin_addr.s_addr;
        }
        if (dst_port) {
            *dst_port = src_ip4->sin_port;
        }
        if (dst_scope_id) {
            *dst_scope_id = 0;
        }
    } else if (src->sa_family == AF_INET6) {
        struct sockaddr_in6 *src_ip6 = (struct sockaddr_in6 *)src;

        if (dst_ip6) {
            memcpy(dst_ip6, src_ip6->sin6_addr.s6_addr, sizeof(*dst_ip6));
        }
        if (dst_port) {
            *dst_port = src_ip6->sin6_port;
        }
        if (dst_scope_id) {
            *dst_scope_id = src_ip6->sin6_scope_id;
        }
    }
}

void pcp_fill_sockaddr(struct sockaddr *dst, struct in6_addr *sip,
                       uint16_t sport, int ret_ipv6_mapped_ipv4,
                       uint32_t scope_id) {
    if ((!ret_ipv6_mapped_ipv4) && (IN6_IS_ADDR_V4MAPPED(sip))) {
        struct sockaddr_in *s = (struct sockaddr_in *)dst;

        s->sin_family = AF_INET;
        s->sin_addr.s_addr = S6_ADDR32(sip)[3];
        s->sin_port = sport;
        SET_SA_LEN(s, sizeof(struct sockaddr_in));
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)dst;

        s->sin6_family = AF_INET6;
        s->sin6_addr = *sip;
        s->sin6_port = sport;
        s->sin6_scope_id = scope_id;
        SET_SA_LEN(s, sizeof(struct sockaddr_in6));
    }
}

#ifndef PCP_SOCKET_IS_VOIDPTR
static pcp_errno pcp_get_error() {
#ifdef WIN32
    int errnum = WSAGetLastError();

    switch (errnum) {
    case WSAEADDRINUSE:
        return PCP_ERR_ADDRINUSE;
    case WSAEWOULDBLOCK:
        return PCP_ERR_WOULDBLOCK;
    default:
        return PCP_ERR_UNKNOWN;
    }
#else
    switch (errno) {
    case EADDRINUSE:
        return PCP_ERR_ADDRINUSE;
        //        case EAGAIN:
    case EWOULDBLOCK:
        return PCP_ERR_WOULDBLOCK;
    default:
        return PCP_ERR_UNKNOWN;
    }
#endif
}
#endif

PCP_SOCKET pcp_socket_create(struct pcp_ctx_s *ctx, int domain, int type,
                             int protocol) {
    assert(ctx && ctx->virt_socket_tb && ctx->virt_socket_tb->sock_create);

    return ctx->virt_socket_tb->sock_create(domain, type, protocol);
}

ssize_t pcp_socket_recvfrom(struct pcp_ctx_s *ctx, void *buf, size_t len,
                            int flags, struct sockaddr *src_addr,
                            socklen_t *addrlen, struct sockaddr_in6 *dst_addr) {
    assert(ctx && ctx->virt_socket_tb && ctx->virt_socket_tb->sock_recvfrom);

    return ctx->virt_socket_tb->sock_recvfrom(ctx->socket, buf, len, flags,
                                              src_addr, addrlen, dst_addr);
}

ssize_t pcp_socket_sendto(struct pcp_ctx_s *ctx, const void *buf, size_t len,
                          int flags, struct sockaddr_in6 *src_addr,
                          struct sockaddr *dest_addr, socklen_t addrlen) {
    assert(ctx && ctx->virt_socket_tb && ctx->virt_socket_tb->sock_sendto);

    return ctx->virt_socket_tb->sock_sendto(ctx->socket, buf, len, flags,
                                            src_addr, dest_addr, addrlen);
}

int pcp_socket_close(struct pcp_ctx_s *ctx) {
    assert(ctx && ctx->virt_socket_tb && ctx->virt_socket_tb->sock_close);

    return ctx->virt_socket_tb->sock_close(ctx->socket);
}

static PCP_SOCKET pcp_socket_create_impl(int domain, int type, int protocol) {
#ifdef PCP_SOCKET_IS_VOIDPTR
    return PCP_INVALID_SOCKET;
#else
    PCP_SOCKET s;
    uint32_t flg;
    unsigned long iMode = 1;
    struct sockaddr_storage sas;
    struct sockaddr_in *sin = (struct sockaddr_in *)&sas;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&sas;

    OSDEP(iMode);
    OSDEP(flg);

    memset(&sas, 0, sizeof(sas));
    sas.ss_family = domain;
    if (domain == AF_INET) {
        sin->sin_port = htons(5350);
        SET_SA_LEN(sin, sizeof(struct sockaddr_in));
    } else if (domain == AF_INET6) {
        sin6->sin6_port = htons(5350);
        SET_SA_LEN(sin6, sizeof(struct sockaddr_in6));
    } else {
        PCP_LOG(PCP_LOGLVL_ERR, "Unsupported socket domain:%d", domain);
    }

    s = (PCP_SOCKET)socket(domain, type, protocol);
    if (s == PCP_INVALID_SOCKET)
        return PCP_INVALID_SOCKET;

#ifdef WIN32
    if (ioctlsocket(s, FIONBIO, &iMode)) {
        PCP_LOG(PCP_LOGLVL_ERR, "%s",
                "Unable to set nonblocking mode for socket.");
        CLOSE(s);
        return PCP_INVALID_SOCKET;
    }
#else  // WIN32
    flg = fcntl(s, F_GETFL, 0);
    if (fcntl(s, F_SETFL, flg | O_NONBLOCK)) {
        PCP_LOG(PCP_LOGLVL_ERR, "%s",
                "Unable to set nonblocking mode for socket.");
        CLOSE(s);
        return PCP_INVALID_SOCKET;
    }
#endif //! WIN32
#ifdef PCP_USE_IPV6_SOCKET
    flg = 0;
    if (PCP_SOCKET_ERROR ==
        setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&flg, sizeof(flg))) {
        PCP_LOG(PCP_LOGLVL_ERR, "%s",
                "Dual-stack sockets are not supported on this platform. "
                "Recompile library with disabled IPv6 support.");
        CLOSE(s);
        return PCP_INVALID_SOCKET;
    }
#endif // PCP_USE_IPV6_SOCKET
#if defined(IP_PKTINFO) || defined(IPV6_RECVPKTINFO) || defined(IPV6_PKTINFO)
    {
        int optval = 1;
#if defined WIN32 && defined IPV6_PKTINFO
        if (setsockopt(s, IPPROTO_IPV6, IPV6_PKTINFO, (char *)&optval,
                       sizeof(optval)) < 0) {
            PCP_LOG(PCP_LOGLVL_ERR, "%s",
                    "Unable to set IPV6_PKTINFO option for socket.");
            CLOSE(s);
            return PCP_INVALID_SOCKET;
        }
#endif // WIN32 && IPV6_PKTINFO
#ifdef IP_PKTINFO
        if (setsockopt(s, IPPROTO_IP, IP_PKTINFO, (char *)&optval,
                       sizeof(optval)) < 0) {
            PCP_LOG(PCP_LOGLVL_ERR, "%s",
                    "Unable to set IP_PKTINFO option for socket.");
        }
#endif // IP_PKTINFO
#ifdef IPV6_RECVPKTINFO
        if (setsockopt(s, IPPROTO_IPV6, IPV6_RECVPKTINFO, (char *)&optval,
                       sizeof(optval)) < 0) {
            PCP_LOG(PCP_LOGLVL_ERR, "%s",
                    "Unable to set IPV6_RECVPKTINFO option for socket.");
            CLOSE(s);
            return PCP_INVALID_SOCKET;
        }
#endif // IPV6_RECVPKTINFO
    }
#endif // IP_PKTINFO && IPV6_RECVPKTINFO
    while (bind(s, (struct sockaddr *)&sas, SA_LEN((struct sockaddr *)&sas)) ==
           PCP_SOCKET_ERROR) {
        if (pcp_get_error() == PCP_ERR_ADDRINUSE) {
            if (sas.ss_family == AF_INET) {
                sin->sin_port = htons(ntohs(sin->sin_port) + 1);
            } else {
                sin6->sin6_port = htons(ntohs(sin6->sin6_port) + 1);
            }
        } else {
            PCP_LOG(PCP_LOGLVL_ERR, "%s", "bind error");
            CLOSE(s);
            return PCP_INVALID_SOCKET;
        }
    }
    PCP_LOG(PCP_LOGLVL_DEBUG, "%s: return %d", __FUNCTION__, s);
    return s;
#endif
}

#ifdef WIN32
#define PCP_CMSG_DATA(msg) (WSA_CMSG_DATA(msg))
#else // WIN32
#define PCP_CMSG_DATA(msg) (CMSG_DATA(msg))
#endif // WIN32

static ssize_t pcp_socket_recvfrom_impl(PCP_SOCKET sock, void *buf, size_t len,
                                        int flags, struct sockaddr *src_addr,
                                        socklen_t *addrlen,
                                        struct sockaddr_in6 *dst_addr) {
    ssize_t ret = -1;

#ifndef PCP_SOCKET_IS_VOIDPTR
#if defined(IPV6_PKTINFO) && defined(IP_PKTINFO)
    char control_buf[1024] = {0};

#ifndef WIN32
    struct msghdr msg;
    struct iovec iov;
    struct cmsghdr *cmsg;

    iov.iov_base = buf;
    iov.iov_len = len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = src_addr;
    msg.msg_namelen = addrlen ? *addrlen : 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);

    // Receive message with IP_PKTINFO
    ret = recvmsg(sock, &msg, 0);

#else // WIN32
    WSAMSG msg;
    WSABUF iov;
    WSACMSGHDR *cmsg;
    iov.buf = buf;
    iov.len = len;

    memset(&msg, 0, sizeof(msg));
    msg.name = (struct sockaddr *)src_addr;
    msg.namelen = addrlen ? *addrlen : 0;
    msg.lpBuffers = &iov;
    msg.dwBufferCount = 1;
    msg.Control.buf = control_buf;
    msg.Control.len = sizeof(control_buf);
    msg.dwFlags = 0;
    // Get WSARecvMsg function pointer
    LPFN_WSARECVMSG WSARecvMsg;
    GUID guid = WSAID_WSARECVMSG;
    DWORD bytesReturned;
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                 &WSARecvMsg, sizeof(WSARecvMsg), &bytesReturned, NULL,
                 NULL) == SOCKET_ERROR) {
        perror("WSAIoctl failed");
        return 1;
    }

    DWORD bytesReceived;
    if (WSARecvMsg(sock, &msg, &bytesReceived, NULL, NULL) == SOCKET_ERROR) {
        ret = -1;
    } else {
#ifdef _WIN32
        if (bytesReceived > INT32_MAX) {
            ret = -1; // Handle overflow error
        } else {
            ret = (ssize_t)bytesReceived;
        }
#else
        ret = bytesReceived;
#endif
    }
#endif // WIN32

    // Processing control message
    if (ret > 0) {
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
             cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IP &&
                cmsg->cmsg_type == IP_PKTINFO) {
                struct in_pktinfo *pktinfo =
                    (struct in_pktinfo *)PCP_CMSG_DATA(cmsg);
                S6_ADDR32(&dst_addr->sin6_addr)[0] = 0;
                S6_ADDR32(&dst_addr->sin6_addr)[1] = 0;
                S6_ADDR32(&dst_addr->sin6_addr)[2] = htonl(0xFFFF);
                S6_ADDR32(&dst_addr->sin6_addr)[3] = pktinfo->ipi_addr.s_addr;
                dst_addr->sin6_scope_id = 0;
                dst_addr->sin6_family = AF_INET6;
            }
            if (cmsg->cmsg_level == IPPROTO_IPV6 &&
                cmsg->cmsg_type == IPV6_PKTINFO) {
                struct in6_pktinfo *pktinfo6 =
                    (struct in6_pktinfo *)PCP_CMSG_DATA(cmsg);
                IPV6_ADDR_COPY(&dst_addr->sin6_addr, &pktinfo6->ipi6_addr);
                dst_addr->sin6_family = AF_INET6;
                if (IN6_IS_ADDR_LINKLOCAL(&pktinfo6->ipi6_addr)) {
                    dst_addr->sin6_scope_id = pktinfo6->ipi6_ifindex;
                } else {
                    dst_addr->sin6_scope_id = 0;
                }
            }
        }
    }
#else  // IPV6_PKTINFO && IP_PKTINFO
    ret = recvfrom(sock, buf, len, flags, src_addr, addrlen);
#endif // IPV6_PKTINFO && IP_PKTINFO
    if (ret == PCP_SOCKET_ERROR) {
        if (pcp_get_error() == PCP_ERR_WOULDBLOCK) {
            ret = PCP_ERR_WOULDBLOCK;
        } else {
            ret = PCP_ERR_RECV_FAILED;
        }
    }
#endif // PCP_SOCKET_IS_VOIDPTR

    return ret;
}

static ssize_t pcp_socket_sendto_impl(PCP_SOCKET sock, const void *buf,
                                      size_t len, int flags UNUSED,
                                      const struct sockaddr_in6 *src_addr,
                                      struct sockaddr *dest_addr,
                                      socklen_t addrlen) {
    ssize_t ret = -1;

#ifndef PCP_SOCKET_IS_VOIDPTR

#if defined(IPV6_PKTINFO)
    if (src_addr) {
        struct in6_pktinfo ipi6 = {0};

#ifndef WIN32
        uint8_t c[CMSG_SPACE(sizeof(struct in6_pktinfo))] = {0};
        struct iovec iov;
        struct msghdr msg;
        struct cmsghdr *cmsg;

        iov.iov_base = (void *)buf;
        iov.iov_len = len;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        ipi6.ipi6_addr = src_addr->sin6_addr;
        ipi6.ipi6_ifindex = src_addr->sin6_scope_id;
        msg.msg_control = c;
        msg.msg_controllen = sizeof(c);
        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = IPPROTO_IPV6;
        cmsg->cmsg_type = IPV6_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(ipi6));
        memcpy(CMSG_DATA(cmsg), &ipi6, sizeof(ipi6));
        msg.msg_name = (void *)dest_addr;
        msg.msg_namelen = addrlen;
        ret = sendmsg(sock, &msg, flags);
#else  // WIN32
        WSABUF wsaBuf;
        wsaBuf.buf = buf;
        wsaBuf.len = len;
        uint8_t c[WSA_CMSG_SPACE(sizeof(struct in6_pktinfo))] = {0};

        WSAMSG wsaMsg;
        memset(&wsaMsg, 0, sizeof(wsaMsg));
        wsaMsg.name = (struct sockaddr *)dest_addr;
        wsaMsg.namelen = addrlen;
        wsaMsg.lpBuffers = &wsaBuf;
        wsaMsg.dwBufferCount = 1;
        wsaMsg.Control.buf = c;

        // Set the source address inside the control message
        if (IN6_IS_ADDR_V4MAPPED(&src_addr->sin6_addr)) {
            wsaMsg.Control.len = WSA_CMSG_SPACE(sizeof(struct in_pktinfo));
            struct cmsghdr *cmsg = WSA_CMSG_FIRSTHDR(&wsaMsg);
            cmsg->cmsg_level = IPPROTO_IP;
            cmsg->cmsg_type = IP_PKTINFO;
            cmsg->cmsg_len = WSA_CMSG_LEN(sizeof(struct in_pktinfo));
            struct in_pktinfo *pktinfo =
                (struct in_pktinfo *)WSA_CMSG_DATA(cmsg);
            pktinfo->ipi_addr.s_addr = S6_ADDR32(&src_addr->sin6_addr)[3];
        } else {
            wsaMsg.Control.len = WSA_CMSG_SPACE(sizeof(struct in6_pktinfo));
            struct cmsghdr *cmsg = WSA_CMSG_FIRSTHDR(&wsaMsg);
            cmsg->cmsg_level = IPPROTO_IPV6;
            cmsg->cmsg_type = IPV6_PKTINFO;
            cmsg->cmsg_len = WSA_CMSG_LEN(sizeof(struct in6_pktinfo));
            struct in6_pktinfo *pktinfo =
                (struct in6_pktinfo *)WSA_CMSG_DATA(cmsg);
            IPV6_ADDR_COPY(&pktinfo->ipi6_addr, &src_addr->sin6_addr);
            pktinfo->ipi6_ifindex = src_addr->sin6_scope_id;
        }

        LPFN_WSARECVMSG WSARecvMsg;
        GUID WSARecvMsg_GUID = WSAID_WSARECVMSG;
        DWORD dwBytesReturned;

        if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &WSARecvMsg_GUID,
                     sizeof(GUID), &WSARecvMsg, sizeof(WSARecvMsg),
                     &dwBytesReturned, NULL, NULL) == SOCKET_ERROR) {
            PCP_LOG(PCP_LOGLVL_PERR, "WSAIoctl failed");
            return 1;
        }

        // Send the packet
        DWORD bytesSent = 0;
        if (WSASendMsg(sock, &wsaMsg, 0, &bytesSent, NULL, NULL) ==
            SOCKET_ERROR) {
            PCP_LOG(PCP_LOGLVL_PERR, "WSASendMsg failed: %d",
                    WSAGetLastError());
        } else {
            ret = bytesSent;
        }
#endif // WIN32
    } else
#else  // IPV6_PKTINFO
    ret = sendto(sock, buf, len, 0, dest_addr, addrlen);
#endif /* IPV6_PKTINFO */

        if ((ret == PCP_SOCKET_ERROR) || (ret != (ssize_t)len)) {
            if (pcp_get_error() == PCP_ERR_WOULDBLOCK) {
                ret = PCP_ERR_WOULDBLOCK;
            } else {
                ret = PCP_ERR_SEND_FAILED;
            }
        }
#endif
    return ret;
}

static int pcp_socket_close_impl(PCP_SOCKET sock) {
#ifndef PCP_SOCKET_IS_VOIDPTR
    return CLOSE(sock);
#else
    return PCP_SOCKET_ERROR;
#endif
}
