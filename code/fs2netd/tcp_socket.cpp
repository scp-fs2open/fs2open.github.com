// TCP_Socket.cpp
// TCP_Socket for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/TCP_Socket.cpp $
 * $Revision: 1.1.2.1 $
 * $Date: 2007-10-15 06:43:11 $
 * $Author: taylor $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.19.2.2  2006/09/20 04:55:48  taylor
 * some basic error message/handling fixage
 *
 * Revision 1.19.2.1  2006/09/08 06:07:59  taylor
 * add support for a server name in fs2open_pxo.cfg, rather than just an IP address for the server
 *
 * Revision 1.19  2006/01/26 03:23:29  Goober5000
 * pare down the pragmas some more
 * --Goober5000
 *
 * Revision 1.18  2006/01/20 07:10:33  Goober5000
 * reordered #include files to quash Microsoft warnings
 * --Goober5000
 *
 * Revision 1.17  2005/10/10 17:21:04  taylor
 * remove NO_NETWORK
 *
 * Revision 1.16  2005/07/13 02:50:49  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.15  2005/06/29 18:49:37  taylor
 * various FS2NetD fixes:
 *  - replace timer stuff with something that more accurately works cross-platform and without being affected by load
 *  - better sanity checking for the server list
 *  - working Linux compatibility that's not dog slow
 *  - when calling DataReady() make sure that the data is properly valid
 *  - fix messed up cvs merge cleanup from the Linux merge which did nasty things
 *
 * Revision 1.14  2005/03/02 21:18:18  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 1.13  2005/02/04 20:06:03  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.12  2004/11/18 00:05:36  Goober5000
 * #pragma'd a bunch of warnings
 * --Goober5000
 *
 * Revision 1.11  2004/07/26 20:47:29  Kazan
 * remove MCD complete
 *
 * Revision 1.10  2004/07/12 16:32:46  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.9  2004/03/31 05:42:26  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * to indicate which warnings were being disabled
 * --Goober5000
 *
 * Revision 1.8  2004/03/09 17:59:01  Kazan
 * Disabled multithreaded TCP_Socket in favor of safer single threaded
 * FS2NetD doesn't kill the game on connection failure now - just gives warning message and effectively dsiables itself until they try to connect again
 *
 * Revision 1.7  2004/03/05 09:01:56  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 1.6  2004/02/21 00:59:43  Kazan
 * FS2NETD License Comments
 *
 * Revision 1.5  2003/11/11 02:15:42  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 1.4  2003/11/06 20:22:05  Kazan
 * slight change to .dsp - leave the release target as fs2_open_r.exe already
 * added myself to credit
 * killed some of the stupid warnings (including doing some casting and commenting out unused vars in the graphics modules)
 * Release builds should have warning level set no higher than 2 (default is 1)
 * Why are we getting warning's about function selected for inline expansion... (killing them with warning disables)
 * FS2_SPEECH was not defined (source file doesn't appear to capture preproc defines correctly either)
 *
 * Revision 1.3  2003/10/13 06:02:50  Kazan
 * Added Log Comment Thingy to these files
 *
 *
 *
 */


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
#include <ctype.h>

#define WSAGetLastError()  (errno)
#else
#include <windows.h>
#include <process.h>
typedef int socklen_t;
#endif

#if !defined(CLOSEFUNC)
#define CLOSEFUNC closesocket
#endif

#if !defined(STYPE)
#define STYPE SOCKET
#endif


static bool Connecting = false;
static bool Connected = false;
static STYPE mySocket;

static sockaddr_in PXO_addr;


void FS2NetD_Disconnect()
{
	CLOSEFUNC(mySocket);

	Connected = false;
	Connecting = false;
}

int FS2NetD_ConnectToServer(char *host, ushort port)
{
	struct hostent *my_host = NULL;
	char host_str[5];
	int my_error = 0;


	if ( !Connecting ) {
#ifdef WIN32
		WSADATA wsa;
		int rc = WSAStartup(MAKEWORD(2,0), &wsa);

		if (rc != 0) {
			ml_printf("FS2NetD ERROR:  Failed to start WinSock!");
			return -1;
		}
#endif

		memset(&PXO_addr, 0, sizeof(sockaddr_in));
		PXO_addr.sin_family = AF_INET;
		PXO_addr.sin_addr.s_addr = INADDR_ANY;
		PXO_addr.sin_port = 0;

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

		if ( bind(mySocket, (sockaddr *)&PXO_addr, sizeof(sockaddr)) == SOCKET_ERROR ) {
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

			PXO_addr.sin_addr.s_addr = ((in_addr *)(my_host->h_addr_list[0]))->s_addr;
		}
		// we might be in dotted format so try using it as such
		else {
			PXO_addr.sin_addr.s_addr = inet_addr( host );
		}

		if (PXO_addr.sin_addr.s_addr == INADDR_ANY) {
			ml_printf("No valid server address to connect with!");
			return -1;
		}

		// we need to set the correct port before moving on
		PXO_addr.sin_port = htons(port);


		if ( connect(mySocket, (sockaddr *)&PXO_addr, sizeof(sockaddr)) == SOCKET_ERROR ) {
			if ( WSAGetLastError() == WSAEWOULDBLOCK ) {
				Connecting = true;
				return 0;
			} else {
#ifdef SCP_UNIX
				int errv = errno;
				ml_printf("FS2NetD ERROR: Couldn't connect to remote system at %s (\"%s\")!", inet_ntoa(PXO_addr.sin_addr), strerror(errv));
#else
				ml_printf("FS2NetD ERROR: Couldn't connect to remote system at %s!", inet_ntoa(PXO_addr.sin_addr));
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

	return -1;
}

int FS2NetD_GetData(char *buffer, int blen, bool OOB)
{
	int flags = 0;
	int status = SOCKET_ERROR;

	if (OOB)
		flags = flags | MSG_OOB;

	// clear the buffer
	memset(buffer, 0, blen);

	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 0;

	fd_set recvs;

	FD_ZERO(&recvs);
	FD_SET(mySocket, &recvs);

#ifndef SCP_UNIX
	status = select(1, &recvs, NULL, NULL, &wait);
#else
	status = select(mySocket+1, &recvs, NULL, NULL, &wait);
#endif

	if ( (status == SOCKET_ERROR) || !(FD_ISSET(mySocket, &recvs)) )
        return -1;

	socklen_t from_len = sizeof(sockaddr);

	return recvfrom(mySocket, buffer, blen, flags, (sockaddr*)&PXO_addr, &from_len);
}

int FS2NetD_SendData(char *buffer, int msg_len, bool OOB)
{
	int flags = 0;

	if (OOB)
		flags = flags | MSG_OOB;

	socklen_t to_len = sizeof(sockaddr);

	return sendto(mySocket, buffer, msg_len, flags, (sockaddr*)&PXO_addr, to_len);
}

void FS2NetD_IgnorePackets()
{
	// let's say most that is going to be backed up is 32K
	char *bitbox = new char[1024*32];

	FS2NetD_GetData(bitbox, 1024*32);

	delete[] bitbox;
}
