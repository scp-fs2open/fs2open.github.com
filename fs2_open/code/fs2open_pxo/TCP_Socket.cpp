// TCP_Socket.cpp
// TCP_Socket for FS2Open PXO
// Derek Meek
// 2-14-2003


#include "TCP_Socket.h"

#include <iostream.h>
//**************************************************************************
// TCP_Socket Implementation
//**************************************************************************


// arguments are ignored if we're in ServerMode
bool TCP_Socket::InitSocket(std::string rem_host, int rem_port)
{

	sockaddr_in adr_inet;

	
	memset(&adr_inet, 0, sizeof(adr_inet));
	adr_inet.sin_family = AF_INET;

	if (ServerMode) //adr_inet will be MY address
	{
		adr_inet.sin_port = htons(port);
		adr_inet.sin_addr.s_addr = INADDR_ANY;
	}
	else // adr_inet is the remote address
	{
		adr_inet.sin_port = htons(rem_port);
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
		if (connect(mySocket, (sockaddr *)&adr_inet, sizeof(sockaddr_in) == -1)
		{
			cout << "Couldn't Connect to Remote system" << endl;
			return fasle;
		}
	}
#endif

	Initialized = true;
	return true;

}


bool TCP_Socket::DataReady()
{
#if defined(WIN32)
	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 1;

	fd_set recvs;

	/*
	recvs.fd_count = 1;
	recvs.fd_array[0] = mySocket;*/
	FD_ZERO(&recvs);
	FD_SET(mySocket, &recvs);

	int status = select(NULL, &recvs, NULL, NULL, &wait);

	return (status != 0 && status != SOCKET_ERROR);
#else
	// --------- IMPLEMENT ME!!! ------------
	return false; 
#endif
}

bool TCP_Socket::OOBDataReady()
{
#if defined(WIN32)
	timeval wait;
	wait.tv_sec = 0;
	wait.tv_usec = 1;

	fd_set recvs;

	/*
	recvs.fd_count = 1;
	recvs.fd_array[0] = mySocket;*/
	FD_ZERO(&recvs);
	FD_SET(mySocket, &recvs);

	int status = select(NULL, NULL, NULL, &recvs, &wait);

	return (status != 0 && status != SOCKET_ERROR);
#else
	// --------- IMPLEMENT ME!!! ------------
	return false; 
#endif
}



int TCP_Socket::GetData(char *buffer, int blen, bool OOB)
{
	// clear the buffer
	memset(buffer, 0, blen);

	int flags = 0;

	if (OOB)
		flags = flags | MSG_OOB;

	return recv(mySocket, buffer, blen, flags);

	
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

