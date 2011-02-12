// TCP_Socket.cpp
// TCP_Socket for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################




#include "fs2netd/tcp_socket.h"
#include "globalincs/pstypes.h"
#include "network/multi_log.h"


#ifdef SCP_UNIX
#include <sys/time.h>	// The OS X 10.3 SDK appears to need this for some reason (for timeval struct)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <cerrno>
#include <sys/ioctl.h>
#ifdef SCP_SOLARIS
#include <sys/filio.h>
#endif
#include <ctype.h>

#define WSAGetLastError()  (errno)
#else
#include <windows.h>
#include <process.h>
typedef int socklen_t;
#endif


static bool Connecting = false;
static bool Connected = false;
static SOCKET mySocket = INVALID_SOCKET;

static sockaddr_in FS2NetD_addr;


void FS2NetD_Disconnect()
{
	if (mySocket != INVALID_SOCKET) {
		shutdown(mySocket, 2);
		closesocket(mySocket);
		mySocket = INVALID_SOCKET;
	}

	Connected = false;
	Connecting = false;
}

int FS2NetD_ConnectToServer(const char *host, const char *port)
{
	struct hostent *my_host = NULL;
	char host_str[5];
#ifdef SCP_UNIX
	int my_error = 0;
#endif

	if ( !Connecting ) {
#ifdef WIN32
		WSADATA wsa;
		int rc = WSAStartup(MAKEWORD(2,0), &wsa);

		if (rc != 0) {
			ml_printf("FS2NetD ERROR:  Failed to start WinSock!");
			return -1;
		}
#endif

		memset(&FS2NetD_addr, 0, sizeof(sockaddr_in));
		FS2NetD_addr.sin_family = AF_INET;
		FS2NetD_addr.sin_addr.s_addr = INADDR_ANY;
		FS2NetD_addr.sin_port = 0;

		mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (mySocket == SOCKET_ERROR) {
#ifdef SCP_UNIX
			my_error = errno;
			ml_printf("FS2NetD ERROR: Couldn't get socket (\"%s\")!", strerror(my_error));
#else
			ml_printf("FS2NetD ERROR: Couldn't get socket!");
#endif
			return -1;
		}

		if ( bind(mySocket, (sockaddr *)&FS2NetD_addr, sizeof(sockaddr)) == SOCKET_ERROR ) {
#ifdef SCP_UNIX
			my_error = errno;
			ml_printf("FS2NetD ERROR: Couldn't bind socket (\"%s\")!", strerror(my_error));
#else
			ml_printf("FS2NetD ERROR: Couldn't bind socket!");
#endif
			return -1;
		}

		// set to non-blocking mode
		ulong arg = 1;
		ioctlsocket(mySocket, FIONBIO, &arg);

		// blasted MS, probably need to use getaddrinfo() here for Win32, but I
		// want to keep this as clean and simple as possible and that means
		// using an actual standard :)

		strncpy( host_str, host, sizeof(host_str) );

		// check that we aren't in a dotted format, some gethostbyname() implementations don't like that
		// (NOTE: Yes, I'm aware that this is problematic if a host name uses an initial digit)
		if ( !isdigit(host_str[0]) ) {
			my_host = gethostbyname( host );

			if (my_host == NULL) {
				// NOTE: that we don't do specific error reporting here since it's totally different
				//       on Win32, compared to everything else that is actually standard
				ml_printf("Failure from gethostbyname() for host '%s'", host);
				return -1;
			} else if (my_host->h_addrtype != AF_INET) {
				ml_printf("Invalid address type returned by gethostbyname()!");
				return -1;
			} else if (my_host->h_addr_list[0] == NULL) {
				ml_printf("Unable to determine IP from host name '%s'", host);
				return -1;
			}

			FS2NetD_addr.sin_addr.s_addr = ((in_addr *)(my_host->h_addr_list[0]))->s_addr;
		}
		// we might be in dotted format so try using it as such
		else {
			FS2NetD_addr.sin_addr.s_addr = inet_addr( host );
		}

		if (FS2NetD_addr.sin_addr.s_addr == INADDR_ANY) {
			ml_printf("No valid server address to connect with!");
			return -1;
		}

		// we need to set the correct port before moving on
		long m_port = strtol(port, (char**)NULL, 10);
		FS2NetD_addr.sin_port = htons( (ushort)m_port );


		if ( connect(mySocket, (sockaddr *)&FS2NetD_addr, sizeof(sockaddr)) == SOCKET_ERROR ) {
			if ( WSAGetLastError() == WSAEWOULDBLOCK ) {
				Connecting = true;
				return 0;
			} else {
#ifdef SCP_UNIX
				int errv = errno;
				ml_printf("FS2NetD ERROR: Couldn't connect to remote system at %s (\"%s\")!", inet_ntoa(FS2NetD_addr.sin_addr), strerror(errv));
#else
				ml_printf("FS2NetD ERROR: Couldn't connect to remote system at %s!", inet_ntoa(FS2NetD_addr.sin_addr));
#endif
			}

			return -1;
		}
		// technically there should always be an error since we should be non-blocking
		else {
			Connecting = true;
			Connected = true;

			return 1;
		}
	}
	// done starting the connection, so lets see if we are actually connected yet
	else {
		if (Connected) {
			Int3();
			return 1;
		}
	
		fd_set write_fds, error_fds;	           
		timeval timeout;

		timeout.tv_sec = 0;            
		timeout.tv_usec = 1;

		FD_ZERO(&write_fds);
		FD_SET(mySocket, &write_fds);   

		// if it's writeable then we are fully connected
		if ( select(mySocket+1, NULL, &write_fds, NULL, &timeout) > 0 ) {
			// make sure that we don't have any connect() errors (since it's non-blocking)
			int err_val = 0;
			size_t err_val_size = sizeof(err_val);
			getsockopt(mySocket, SOL_SOCKET, SO_ERROR, (char*)&err_val, (socklen_t*)&err_val_size);

			if (err_val) {
				// if we aren't still in blocking mode then we can't connect
				if (err_val != WSAEWOULDBLOCK) {
					return -1;
				}

				return 0;
			}

			Connected = true;
			return 1;
		}

		FD_ZERO(&error_fds);
		FD_SET(mySocket, &error_fds);

		// if it's in error then it has failed to connect
		if ( select(mySocket+1, NULL, NULL, &error_fds, &timeout) )
			return -1;

		// not connected, and haven't failed to connect, so keep in the loop
		return 0;
	}
}

int FS2NetD_GetData(char *buffer, int blen)
{
	int flags = 0;

	// clear the buffer
	memset(buffer, 0, blen);

	socklen_t from_len = sizeof(sockaddr);

	return recvfrom(mySocket, buffer, blen, flags, (sockaddr*)&FS2NetD_addr, &from_len);
}

int FS2NetD_SendData(char *buffer, int blen)
{
	int flags = 0;

	socklen_t to_len = sizeof(sockaddr);

	return sendto(mySocket, buffer, blen, flags, (sockaddr*)&FS2NetD_addr, to_len);
}

bool FS2NetD_DataReady()
{
	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 1;

	fd_set recvs;

	FD_ZERO(&recvs);
	FD_SET(mySocket, &recvs);

#ifndef SCP_UNIX
	int status = select(1, &recvs, NULL, NULL, &wait);
#else
	int status = select(mySocket+1, &recvs, NULL, NULL, &wait);
#endif

	return ( (status != 0) && (status != -1) && FD_ISSET(mySocket, &recvs) );
}
