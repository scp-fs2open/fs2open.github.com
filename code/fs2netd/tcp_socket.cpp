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
#include <signal.h>
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
	char host_str[254]; // 253 is max domain name length: http://en.wikipedia.org/wiki/Domain_Name_System#Domain_name_syntax
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

		// Where SO_NOSIGPIPE is defined (currently OS X):
		// prevent SIGPIPE, which is generated when sending after receiver disconnected
		// on other *nix use MSG_NOSIGNAL flag when sending data, or, failing that, use signal masking
		// see FS2NetD_SendData()
#if defined(SCP_UNIX) && defined(SO_NOSIGPIPE)
		int nosigpipe = 1;
		if (setsockopt(mySocket, SOL_SOCKET, SO_NOSIGPIPE, (void*)&nosigpipe, sizeof(nosigpipe)) == -1) {
			my_error = errno;
			ml_printf("FS2NetD ERROR: Couldn't set SO_NOSIGPIPE on socket (\"%s\")!", strerror(my_error));
		}
#endif // SCP_UNIX && SO_NOSIGPIPE

		// blasted MS, probably need to use getaddrinfo() here for Win32, but I
		// want to keep this as clean and simple as possible and that means
		// using an actual standard :)

		strcpy_s( host_str, host );

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
		if ( select(static_cast<int>(mySocket + 1), NULL, &write_fds, NULL, &timeout) > 0 ) {
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
		if ( select(static_cast<int>(mySocket + 1), NULL, NULL, &error_fds, &timeout) )
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

	// on Unix with MSG_NOSIGNAL, prevent SIGPIPE, which is generated if receiver has disconnected
	// the MSG_NOSIGNAL flag doesn't work on OS X or Solaris
	// on OS X, use setsockopt() with SO_NOSIGPIPE instead, see above
#if defined(SCP_UNIX) && defined(MSG_NOSIGNAL)
	flags |= MSG_NOSIGNAL;
#endif // SCP_UNIX && MSG_NOSIGNAL

	socklen_t to_len = sizeof(sockaddr);

#if defined(SCP_UNIX) && !defined(SO_NOSIGPIPE) && !defined(MSG_NOSIGNAL) && _POSIX_REALTIME_SIGNALS > 0
		// Unix platforms do not ignore SIGPIPE like Windows does.  This results in the standalone crashing eventually.
		// Failing to use MSG_NOSIGNAL or SO_NOSIGPIPE, both of which are only on some platforms, we'll use this.
		// Borrowed from:
		// http://www.microhowto.info/howto/ignore_sigpipe_without_affecting_other_threads_in_a_process.html

		// Step 1: Add SIGPIPE to the signal mask using pthread_sigmask.
		sigset_t sigpipe_mask;
		sigemptyset(&sigpipe_mask);
		sigaddset(&sigpipe_mask, SIGPIPE);
		sigset_t saved_mask;
		if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) == -1) {
			ml_printf("Could not set mask with pthread_sigmask in FS2NetD_SendData");
			exit(1);
		}
#endif // SCP_UNIX && !SO_NOSIGPIPE && !MSG_NOSIGNAL && _POSIX_REALTIME_SIGNALS > 0

		// Step 2: Execute the library code which might raise SIGPIPE.
		auto sent_chars = sendto(mySocket, buffer, blen, flags, (sockaddr*)&FS2NetD_addr, to_len);

#if defined(SCP_UNIX) && !defined(SO_NOSIGPIPE) && !defined(MSG_NOSIGNAL) && _POSIX_REALTIME_SIGNALS > 0
		// Step 3: Accept any pending SIGPIPE using sigtimedwait.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
		// Some older GCC versions seem to incorrectly warn on this initialization.
		// The pragmas should be removable once GCC 5+ is in use everywhere.
		// Reference: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61489
		struct timespec zerotime = {};
#pragma GCC diagnostic pop
		if (sigtimedwait(&sigpipe_mask, 0, &zerotime) == -1) {
			ml_printf("Could not accept possible pending SIGPIPE with sigtimedwait in FS2NetD_SendData");
			exit(1);
		}

		// Step 4: Restore the signal mask to its original state.
		if (pthread_sigmask(SIG_SETMASK, &saved_mask, 0) == -1) {
			ml_printf("Could not restore mask with pthread_sigmask in FS2NetD_SendData");
			exit(1);
		}
#endif // SCP_UNIX && !SO_NOSIGPIPE && !MSG_NOSIGNAL && _POSIX_REALTIME_SIGNALS > 0

	return sent_chars;
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
