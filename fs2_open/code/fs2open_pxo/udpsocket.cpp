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
 * $Revision: 1.5 $
 * $Date: 2004-03-31 05:42:26 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
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

#include "udpsocket.h"

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
		Out << "Couldn't Get Socket (" << strerror(errno) << ")" << endl;
		return false;
	}


	if (bind(this->socket, (sockaddr *)&adr_inet, sizeof(adr_inet)) == -1)
	{
		Out << "Couldn't Bind Socket" << endl;
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

#if defined(WIN32)
	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 1;

	fd_set recvs;
	recvs.fd_count = 1;
	recvs.fd_array[0] = this->socket;

	int status = select(NULL, &recvs, NULL, NULL, &wait);
	if (status != 0 && status != SOCKET_ERROR)
		recvbytes = recvfrom(this->socket, buffer, blen, 0, (sockaddr *)&from_addr, &from_size);

#else
	// +#@$++#@$+ +#@$++#@$+ CRITICAL FIX REQURED +#@$++#@$+ +#@$++#@$+ 
	// FIX THIS FOR IT NOT TO BLOCK LIKE I DID ON WINDOWS! 
	// +#@$++#@$+ +#@$++#@$+ CRITICAL FIX REQURED +#@$++#@$+ +#@$++#@$+ 
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

