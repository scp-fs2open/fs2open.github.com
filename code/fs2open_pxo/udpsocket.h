// udpsocket.h
// UDP Socket class for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/udpsocket.h $
 * $Revision: 1.5 $
 * $Date: 2004-08-11 05:06:23 $
 * $Author: Kazan $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2004/03/31 05:42:26  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * for #pragma warning disable to indicate the message being disabled
 * --Goober5000
 *
 * Revision 1.3  2004/02/21 00:59:43  Kazan
 * FS2NETD License Comments
 *
 * Revision 1.2  2003/10/13 06:02:50  Kazan
 * Added Log Comment Thingy to these files
 *
 *
 *
 */

#include "PreProcDefines.h"
#if !defined(__protocol_h_)
#define __protocol_h_

#if defined(WIN32)
// Windows Version
#include <windows.h>

#pragma warning(push, 2)	// ignore all those warnings for Microsoft stuff
#include <string>
#pragma warning(pop)

//#include <winsock2.h>
#define STYPE SOCKET
#define CLOSEFUNC closesocket

#else
// Unix Version
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#define STYPE int
#define CLOSEFUNC close

#endif

class UDP_Socket
{
	private:

		bool Initialized;
		STYPE socket;
		int port;


	public:
		UDP_Socket() : Initialized(false), port(0) 
			{}
		UDP_Socket(int aport) : Initialized(false), port(aport) 
			{}

		~UDP_Socket() { Close(); }
		
		bool isInitialized() { return Initialized; }
		void SetPort(int aport) { port = aport; } 
		bool InitSocket();
		int GetPacket(char *buffer, int blen, std::string &from);
		int SendPacket(char *buffer, int plen, std::string &to, int toport);

		void Close() { CLOSEFUNC(socket); }


};

#endif
