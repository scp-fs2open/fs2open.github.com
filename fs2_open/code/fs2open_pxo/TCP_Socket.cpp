// TCP_Socket.cpp
// TCP_Socket for FS2Open PXO
// Derek Meek
// 2-14-2003

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/TCP_Socket.cpp $
 * $Revision: 1.5 $
 * $Date: 2003-11-11 02:15:42 $
 * $Author: Goober5000 $
 *
 *
 * $Log: not supported by cvs2svn $
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

#pragma warning(disable:4100)
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#pragma warning(disable:4711)

#include "TCP_Socket.h"
#include <process.h>
#include <iostream.h>


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
			return fasle;
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
#pragma warning(disable:4127)
bool TCP_Socket::DataReady()
{
	timeval wait;
	wait.tv_sec = 1;
	wait.tv_usec = 0;

	fd_set recvs;

	FD_ZERO(&recvs);
	FD_SET(mySocket, &recvs);

	int status = select(1, &recvs, NULL, NULL, &wait);

	return (status != 0 && status != SOCKET_ERROR);
}

bool TCP_Socket::OOBDataReady()
{
	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 1;

	fd_set recvs;

	FD_ZERO(&recvs);
	FD_SET(mySocket, &recvs);

	int status = select(1, NULL, NULL, &recvs, &wait);

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
	STYPE client = accept(mySocket, (sockaddr *) &adr_client, &len);

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

