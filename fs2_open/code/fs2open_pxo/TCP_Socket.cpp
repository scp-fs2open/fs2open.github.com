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
 * $Revision: 1.18 $
 * $Date: 2006-01-20 07:10:33 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
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
 * for #pragma warning disable to indicate the message being disabled
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


#include "fs2open_pxo/TCP_Socket.h"
#include "globalincs/pstypes.h"

#include <iostream>

#ifdef SCP_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


// 4100 = unreferenced formal parameter
// 4511 = copy constructor could not be generated
// 4512 = assignment operator could not be generated
#pragma warning(disable:4100 4511 4512)

using namespace std;

//**************************************************************************
// TCP_Socket Implementation
//**************************************************************************

extern void ml_printf(char *, ...);

#if defined(FS2_TCP_RMultithread)
void _cdecl tcp_socket_mt_run(void *arg)
{
	TCP_Socket *socket = (TCP_Socket *)arg;


	while (!socket->MtDie)
	{
		if (socket->DataReady())
		{
			//ml_printf("Got Some Data via multithreads!");
			if (socket->GetDataFromNetwork() == 0)
			{
				// socket has died!
				socket->IgnorePackets();
				socket->Close();
				socket->Initialized = false; 
				break;
			}
		}
	}
	_endthread();


}
#endif


// arguments are ignored if we're in ServerMode
bool TCP_Socket::InitSocket(std::string rem_host, int rem_port)
{

#if defined(FS2_TCP_RMultithread)
		QueueLen = 0;

		memset(ReceiveBuffer, 0, FS2_TCP_MaxWaiting * FS2_TCP_ReceiveBuffer);
		memset(ReceiveLen, 0, sizeof(int) * FS2_TCP_MaxWaiting);
		Lock = false;
		MtDie = false;
#endif

	sockaddr_in adr_inet;

	
	memset(&adr_inet, 0, sizeof(adr_inet));
	adr_inet.sin_family = AF_INET;

	if (ServerMode) //adr_inet will be MY address
	{
		adr_inet.sin_port = htons((unsigned short)port);
		adr_inet.sin_addr.s_addr = INADDR_ANY;
	}
	else // adr_inet is the remote address
	{
		adr_inet.sin_port = htons((unsigned short)rem_port);
		adr_inet.sin_addr.s_addr = inet_addr(rem_host.c_str());
	}


#if defined(WIN32)
	// Windows Version
	int rc=startWinsock();
	if(rc != 0)
		return false;

	mySocket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(mySocket == INVALID_SOCKET)
		return false;

	/*
	int timeout = TCP_SOCKET_TIMEOUT;
	setsockopt(mySocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	setsockopt(mySocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int)); */

	if (ServerMode)
	{
		if (bind(mySocket, (sockaddr *)&adr_inet, sizeof(adr_inet)) == SOCKET_ERROR)
			return false;
		
		if (listen(mySocket, 10) == SOCKET_ERROR)
			return false;
	}
	else
	{
		if (connect(mySocket, (sockaddr *)&adr_inet, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
				return false;		
		}
		

		
	}
#else
	// Unix Version



	mySocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySocket == -1)
	{
		cout << "Couldn't Get Socket (" << strerror(errno) << ")" << endl;
		return false;
	}


	if (ServerMode)
	{
		if (bind(mySocket, (sockaddr *)&adr_inet, sizeof(adr_inet)) == -1)
		{
			cout << "Couldn't Bind Socket" << endl;
			return false;
		}

		if (listen(mySocket, 10) == -1)
		{
			cout << "Couldn't create Listen Socket" << endl;
			return false;
		}
	}
	else
	{
		if (connect(mySocket, (sockaddr *)&adr_inet, sizeof(sockaddr_in)) == -1)
		{
			cout << "Couldn't Connect to Remote system" << endl;
			return false;
		}
	}
#endif

#if defined(FS2_TCP_RMultithread)
	_beginthread(tcp_socket_mt_run, 0, (void *)this);
#endif

	Initialized = true;
	return true;

}

#pragma warning(push)
#pragma warning(disable:4127)	// conditional expression is constant
bool TCP_Socket::DataReady()
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

#if defined(FS2_TCP_RMultithread)
	return (status != 0 && status != SOCKET_ERROR) || this->MT_DataReady();
#else
	return (status != 0 && status != SOCKET_ERROR && FD_ISSET(mySocket, &recvs));
#endif
}

#if !defined(FS2_TCP_RMultithread)
void TCP_Socket::IgnorePackets()
{
	if (DataReady())
	{
			// let's say most that is going to be backed up is 32K
		char *bitbox = new char[1024*32];

		GetData(bitbox, 1024*32);

		delete[] bitbox;
	}

}
#endif


bool TCP_Socket::OOBDataReady()
{
	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 1;

	fd_set recvs;

	FD_ZERO(&recvs);
	FD_SET(mySocket, &recvs);

#ifndef SCP_UNIX
	int status = select(1, NULL, NULL, &recvs, &wait);
#else
	int status = select(mySocket+1, NULL, NULL, &recvs, &wait);
#endif

	return (status != 0 && status != SOCKET_ERROR);
}
#pragma warning(pop)

#if defined(FS2_TCP_RMultithread)


int TCP_Socket::GetDataFromNetwork()
{
	char buffer[FS2_TCP_ReceiveBuffer];
	memset(buffer, 0, FS2_TCP_ReceiveBuffer);

	
	int size = recv(mySocket, buffer, FS2_TCP_ReceiveBuffer, 0);

	if (size > 0)
	{
		while (Lock);
		Lock = true;

		while (QueueLen >= FS2_TCP_MaxWaiting)
			RemoveFirstPacket();

		memcpy(ReceiveBuffer[QueueLen], buffer, size);
		ReceiveLen[QueueLen] = size;
		QueueLen++;

		Lock = false;
	}

	return size;
}


void TCP_Socket::RemoveFirstPacket()
{

	for (int i = 0; i < QueueLen-1; i++)
	{
		memcpy(ReceiveBuffer[i], ReceiveBuffer[i+1], FS2_TCP_ReceiveBuffer);
		ReceiveLen[i] = ReceiveLen[i+1];
	}
	memset(ReceiveBuffer[QueueLen-1], 0, FS2_TCP_ReceiveBuffer);
	QueueLen--;
}
#endif

int TCP_Socket::GetData(char *buffer, int blen, bool OOB)
{
	int flags = 0;

	if (OOB)
		flags = flags | MSG_OOB;

#if defined(FS2_TCP_RMultithread)
	if (MT_DataReady())
	{
		while (Lock);
		Lock = true;

		int cpylen = ReceiveLen[0];
		if (cpylen > blen)
			cpylen = blen;

		memcpy(buffer, ReceiveBuffer[0], cpylen);
	

		RemoveFirstPacket();
		Lock = false;

		return cpylen;
	}
	else
		return 0;
#else
	// clear the buffer
	memset(buffer, 0, blen);



	return recv(mySocket, buffer, blen, flags);
#endif
	
}

int TCP_Socket::SendData(char *buffer, int msg_len, bool OOB)
{
	int flags = 0;

	if (OOB)
		flags = flags | MSG_OOB;

	
	return send(mySocket, buffer, msg_len, flags);
}

#if defined(WIN32) && defined(MT_TCP_Socket)
void __cdecl StartServerProc(void *arg)
{

	TCP_Socket *sock = (TCP_Socket *)arg;
	sock->nServProc(sock->nServerSocket);
}
#endif


STYPE TCP_Socket::AcceptConnections()
{
	sockaddr_in adr_client;
	int len = sizeof(sockaddr_in);
#ifndef SCP_UNIX
	STYPE client = accept(mySocket, (sockaddr *) &adr_client, &len);
#else
	STYPE client = accept(mySocket, (sockaddr *) &adr_client, (socklen_t*)&len);
#endif
	
	return client; // some sort of error
}

#if defined(MT_TCP_Socket)
bool TCP_Socket::AcceptConnections(sproc server_process)
{

	sockaddr_in adr_client;
	int len = sizeof(sockaddr_in);
	STYPE client = accept(mySocket, (sockaddr *) &adr_client, &len);

	
	if (client == -1) //SOCKET_ERROR on windows
		return false; // some sort of error

	#if defined(WIN32)
		// --------- Windows ---------
	nServerSocket = client;
	_beginthread(StartServerProc, 0, NULL);
	#else
	// --------- Unix ---------
	pid_t pid;

	switch(pid=fork()){
		case -1: 
			cout << "can't fork\n";
			exit(-1);

		case 0 : 
			nServProc(client);
			exit(0);

		default: // this is the code the parent runs 
			close(client); // we don't want to keep a copy around for us
			break; 
	}
	#endif


	return true;
}

#endif
