// udpsocket.cpp
// UDP Socket class for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/udpsocket.cpp $
 * $Revision: 1.11 $
 * $Date: 2005-10-10 17:21:04 $
 * $Author: taylor $
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2005/07/13 02:50:49  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.9  2005/03/02 21:18:18  taylor
 * better support for Inferno builds (in PreProcDefines.h now, no networking support)
 * make sure NO_NETWORK builds are as friendly on Windows as it is on Linux/OSX
 * revert a timeout in Client.h back to the original value before Linux merge
 *
 * Revision 1.8  2005/02/04 20:06:03  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 1.7  2004/07/26 20:47:29  Kazan
 * remove MCD complete
 *
 * Revision 1.6  2004/07/12 16:32:46  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.5  2004/03/31 05:42:26  Goober5000
 * got rid of all those nasty warnings from xlocale and so forth; also added comments
 * for #pragma warning disable to indicate the message being disabled
 * --Goober5000
 *
 * Revision 1.4  2004/02/21 00:59:43  Kazan
 * FS2NETD License Comments
 *
 * Revision 1.3  2003/11/11 02:15:42  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 1.2  2003/10/13 06:02:50  Kazan
 * Added Log Comment Thingy to these files
 *
 *
 *
 */


#pragma warning(disable:4710)	// function not inlined
#pragma warning(disable:4711)	// function inlined

#include <iostream>

#ifdef SCP_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "fs2open_pxo/udpsocket.h"

using namespace std;




#if defined(WIN32)
//*******************************************************************************
// Windows Version
//*******************************************************************************
#include <windows.h>

int startWinsock()
{  
	WSADATA wsa;  
	return WSAStartup(MAKEWORD(2,0),&wsa);
}

#endif



bool UDP_Socket::InitSocket()
{
	if (Initialized)
		return true; // we're already initialized!

	sockaddr_in adr_inet;

	memset(&adr_inet, 0, sizeof(adr_inet));
	adr_inet.sin_family = AF_INET;
	adr_inet.sin_port = htons((unsigned short) port);
	adr_inet.sin_addr.s_addr = INADDR_ANY;

#if defined(WIN32)

	// Windows Version
	int rc = startWinsock();
	if(rc != 0)
		return false;

	this->socket = ::socket(AF_INET,SOCK_DGRAM,0);
	if(this->socket == INVALID_SOCKET)
		return false;

	if (bind(this->socket, (sockaddr *)&adr_inet, sizeof(adr_inet)) == SOCKET_ERROR)
		return false;
#else
	// Unix Version

	this->socket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (this->socket == -1)
	{
		cout << "Couldn't Get Socket (" << strerror(errno) << ")" << endl;
		return false;
	}


	if (bind(this->socket, (sockaddr *)&adr_inet, sizeof(adr_inet)) == -1)
	{
		cout << "Couldn't Bind Socket" << endl;
		return false;
	}

#endif

	Initialized = true;
	return true;
}




//*******************************************************************************
// Uniform Version
//*******************************************************************************

int UDP_Socket::GetPacket(char *buffer, int blen, std::string &from)
{
	if (!Initialized)
		return false;

	sockaddr_in from_addr;
	memset(&from_addr, 0, sizeof(from_addr));

	int recvbytes = -1;
	int from_size = sizeof(from_addr);

	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 1;

	fd_set recvs;
	FD_ZERO(&recvs);
	FD_SET(this->socket, &recvs);
	

	int status = select(-1, &recvs, NULL, NULL, &wait);
	if (status != 0 && status != SOCKET_ERROR)
#if defined(WIN32)
		recvbytes = recvfrom(this->socket, buffer, blen, 0, (sockaddr *)&from_addr, &from_size);
#else
		recvbytes = recvfrom(this->socket, buffer, blen, 0, (sockaddr *)&from_addr, (socklen_t *)&from_size);
#endif

	from = inet_ntoa(from_addr.sin_addr);

	return recvbytes;
}

int UDP_Socket::SendPacket(char *buffer, int plen, std::string &to, int toport)
{
	if (!Initialized)
		return false;

	sockaddr_in to_addr;
	int to_size = sizeof(to_addr);
	
	to_addr.sin_family	=	AF_INET;
	to_addr.sin_port	=	htons((unsigned short) toport);
#if !defined(WIN32)
	to_addr.sin_addr.s_addr	=	inet_addr(to.c_str());
#else
	
	to_addr.sin_addr.s_addr	=	inet_addr(to.c_str());
#endif

	int sent = sendto(this->socket, buffer, plen, 0, (sockaddr *)&to_addr, to_size);
	return sent;
}
