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
 * $Revision: 1.6 $
 * $Date: 2005-02-04 20:06:03 $
 * $Author: taylor $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2004/08/11 05:06:23  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
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

#ifdef _WIN32
#include <windows.h>

#pragma warning(push, 2)	// ignore all those warnings for Microsoft stuff
#include <string>
#pragma warning(pop)

//#include <winsock2.h>
#define STYPE SOCKET
#define CLOSEFUNC closesocket

#else
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <cerrno>
#endif
#include <string>

#include "globalincs/pstypes.h"

#define CLOSEFUNC closesocket

#if !defined(STYPE)
#define STYPE SOCKET
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
