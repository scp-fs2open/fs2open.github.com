/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <cerrno>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#ifdef SCP_SOLARIS
#include <sys/filio.h>
#endif
#endif

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cerrno>

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "inetfile/inetgetfile.h"
#include "inetfile/chttpget.h"
#include "io/timer.h"
#include "network/psnet2.h"


int HTTPObjThread( void *obj )
{
	((ChttpGet *)obj)->WorkerThread();
	((ChttpGet *)obj)->m_Aborted = true;

	return (reinterpret_cast<ChttpGet *>(obj))->GetStatus();
}

void ChttpGet::AbortGet()
{
	m_Aborting = true;
	while(!m_Aborted) os_sleep(50); //Wait for the thread to end
}

ChttpGet::ChttpGet(const char *URL, const char *localfile, const char *proxyip, unsigned short proxyport)
{
	m_ProxyEnabled = true;
	m_ProxyIP = proxyip;
	m_ProxyPort = proxyport;
	GetFile(URL,localfile);
}

ChttpGet::ChttpGet(const char *URL, const char *localfile)
{
	m_ProxyEnabled = false;
	GetFile(URL,localfile);
}


void ChttpGet::GetFile(const char *URL, const char *localfile)
{
	m_DataSock = INVALID_SOCKET;
	m_iBytesIn = 0;
	m_iBytesTotal = 0;
	m_State = HTTP_STATE_STARTUP;
	m_Aborting = false;
	m_Aborted = false;

	strncpy(m_URL,URL,MAX_URL_LEN-1);
	m_URL[MAX_URL_LEN-1] = 0;

	LOCALFILE = fopen(localfile,"wb");
	if(NULL == LOCALFILE)
	{
		m_State = HTTP_STATE_CANT_WRITE_FILE;
		m_Aborted = true;
		return;
	}
	m_DataSock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET == m_DataSock)
	{
		m_State = HTTP_STATE_SOCKET_ERROR;
		m_Aborted = true;
		return;
	}

	// make sure we are in dual-stack mode (not the default on Windows)
	int i_opt = 0;
	setsockopt(m_DataSock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&i_opt), sizeof(i_opt));

	unsigned long arg = 1;
	ioctlsocket( m_DataSock, FIONBIO, &arg );

	const char *pURL = URL;
	if(strnicmp(URL,"http:",5)==0)
	{
		pURL +=5;
		while(*pURL == '/')
		{
			pURL++;
		}
	}
	//There shouldn't be any : in this string
	if(strchr(pURL,':'))
	{
		m_State = HTTP_STATE_URL_PARSING_ERROR;
		m_Aborted = true;
		return;
	}
	//read the filename by searching backwards for a /
	//then keep reading until you find the first /
	//when you found it, you have the host and dir
	const char *filestart = nullptr;
	const char *dirstart = nullptr;
	for(size_t i = strlen(pURL);;i--)
	{
		if(pURL[i]== '/')
		{
			if(!filestart)
			{
				filestart = pURL+i+1;
				dirstart = pURL+i+1;
				strcpy_s(m_szFilename,filestart);
			}
			else
			{
				dirstart = pURL+i+1;
			}
		}
		if (i == 0) {
			break;
		}
	}
	if((dirstart==NULL) || (filestart==NULL))
	{
		m_State = HTTP_STATE_URL_PARSING_ERROR;
		m_Aborted = true;
		return;
	}
	else
	{
		strcpy_s(m_szDir,dirstart);//,(filestart-dirstart));
		//m_szDir[(filestart-dirstart)] = NULL;
		strncpy(m_szHost,pURL,(dirstart-pURL));
		m_szHost[(dirstart-pURL)-1] = '\0';
	}

	SDL_Thread *thread = SDL_CreateThread(HTTPObjThread, "HTTPObjThread", this);

	if(thread == nullptr)
	{
		m_State = HTTP_STATE_INTERNAL_ERROR;
		m_Aborted = true;
	}
	else
	{
		SDL_DetachThread(thread);
	}
}


ChttpGet::~ChttpGet()
{
	if(m_DataSock != INVALID_SOCKET)
	{
		shutdown(m_DataSock,2);
		closesocket(m_DataSock);
	}

	if(LOCALFILE != nullptr)
	{
		fclose(LOCALFILE);
	}
}

int ChttpGet::GetStatus()
{
	return m_State;
}

unsigned int ChttpGet::GetBytesIn()
{
	return m_iBytesIn;
}

unsigned int ChttpGet::GetTotalBytes()
{
	return m_iBytesTotal;
}


void ChttpGet::WorkerThread()
{
	char szCommand[1000];
	char *p;
	int irsp = 0;
	ConnectSocket();
	if(m_Aborting)
	{
		fclose(LOCALFILE);
		LOCALFILE = nullptr;
		return;
	}
	if(m_State != HTTP_STATE_CONNECTED)
	{
		fclose(LOCALFILE);
		LOCALFILE = nullptr;
		return;
	}
	sprintf(szCommand,"GET %s%s HTTP/1.1\nAccept: */*\nAccept-Encoding: deflate\nHost: %s\n\n\n",m_ProxyEnabled?"":"/",m_ProxyEnabled?m_URL:m_szDir,m_szHost);
	send(m_DataSock,szCommand,static_cast<int>(strlen(szCommand)),0);
	p = GetHTTPLine();
	if (!p) return;
	if(strnicmp("HTTP/",p,5)==0)
	{
		char *pcode;
		pcode = strchr(p,' ');
		if(pcode == nullptr)
		{
			m_State = HTTP_STATE_UNKNOWN_ERROR;	
			fclose(LOCALFILE);
			LOCALFILE = nullptr;
			return;

		}
		pcode++;
		pcode[3] = '\0';
		irsp = atoi(pcode);

		if(irsp == 0)
		{
			m_State = HTTP_STATE_UNKNOWN_ERROR;	
			fclose(LOCALFILE);
			LOCALFILE = nullptr;
			return;
		}
		if(irsp==200)
		{
			do
			{
				p = GetHTTPLine();
				if(p==NULL)
				{
					m_State = HTTP_STATE_UNKNOWN_ERROR;	
					fclose(LOCALFILE);
					LOCALFILE = nullptr;
					return;
				}
				if(*p=='\0')
				{
					break;
				}
				if(strnicmp(p,"Content-Length:",strlen("Content-Length:"))==0)
				{
					char *s = strchr(p,' ')+1;
					p = s;
					if(s)
					{
						while(*s)
						{
							if(!isdigit(*s))
							{
								*s='\0';
							}
							s++;
						};
						m_iBytesTotal = atoi(p);
					}

				}

				os_sleep(1);
			}while(true);
		ReadDataChannel();
		return;
		}
		m_State = HTTP_STATE_FILE_NOT_FOUND;
		fclose(LOCALFILE);
		LOCALFILE = nullptr;
		return;
	}
	else
	{
		m_State = HTTP_STATE_UNKNOWN_ERROR;
		fclose(LOCALFILE);
		LOCALFILE = nullptr;
		return;
	}
}

int ChttpGet::ConnectSocket()
{
	SOCKADDR_STORAGE hostaddr;
	int rcode;

	if (m_Aborting) {
		return 0;
	}

	if (m_ProxyEnabled) {
		if (m_ProxyPort) {
			rcode = psnet_get_addr(m_ProxyIP, m_ProxyPort, &hostaddr);
		} else {
			rcode = psnet_get_addr(m_ProxyIP, "http", &hostaddr);
		}
	} else {
		rcode = psnet_get_addr(m_szHost, "http", &hostaddr);
	}

	if ( !rcode ) {
		m_State = HTTP_STATE_HOST_NOT_FOUND;

		return 0;
	}

	//Now we will connect to the host					
	fd_set	wfds;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int serr = connect(m_DataSock, reinterpret_cast<LPSOCKADDR>(&hostaddr), sizeof(hostaddr));
	int cerr = WSAGetLastError();
	if (serr) {
		// fail after 20 seconds
		int failtime = timer_get_seconds() + 20;

		while ( (cerr == WSAEALREADY) || (cerr == WSAEINVAL) || NETCALL_WOULDBLOCK(cerr) ) {
			FD_ZERO(&wfds);
			FD_SET( m_DataSock, &wfds );

			if ( select(static_cast<int>(m_DataSock+1), nullptr, &wfds, nullptr, &timeout) )
			{
				int error_code = 0;
				SOCKLEN_T error_code_size = sizeof(error_code);

				// check to make sure socket is *really* connected
				int rc = getsockopt(m_DataSock, SOL_SOCKET, SO_ERROR, (char *)&error_code, &error_code_size);

				if(!rc && !error_code)
				{
					serr = 0;
				}

				break;
			}

			if (m_Aborting)
				return 0;

			serr = connect(m_DataSock, reinterpret_cast<LPSOCKADDR>(&hostaddr), sizeof(hostaddr));

			if (serr == 0)
				break;

			cerr = WSAGetLastError();
	
			if (cerr == WSAEISCONN) {
				serr = 0;
				break;
			}

			if (timer_get_seconds() > failtime) {
				break;
			}

			os_sleep(1);
		}
	}

	if (serr) {
		m_State = HTTP_STATE_CANT_CONNECT;
		return 0;
	}

	m_State = HTTP_STATE_CONNECTED;

	return 1;
}

char *ChttpGet::GetHTTPLine()
{
	int iBytesRead;
	char chunk[2];
	uint igotcrlf = 0;
	memset(recv_buffer,0,1000);
	do
	{
		chunk[0]='\0';
		bool gotdata = false;
		do
		{
			iBytesRead = recv(m_DataSock,chunk,1,0);

			if(SOCKET_ERROR == iBytesRead)
			{
				int error = WSAGetLastError();
				if( NETCALL_WOULDBLOCK(error) )
				{
					gotdata = false;
					continue;
				}
				else
					return NULL;
			}
			else
			{
				gotdata = true;
			}

			os_sleep(1);
		}while(!gotdata);
		
		if(chunk[0]==0x0d)
		{
			//This should always read a 0x0a
			do
			{
				iBytesRead = recv(m_DataSock,chunk,1,0);

				if(SOCKET_ERROR == iBytesRead)
				{	
					int error = WSAGetLastError();
					if( NETCALL_WOULDBLOCK(error) )
					{
						gotdata = false;
						continue;
					}
					else
						return NULL;
				}
				else
				{
					gotdata = true;
				}

				os_sleep(1);
			}while(!gotdata);
			igotcrlf = 1;	
		}
		else
		{	chunk[1] = '\0';
			strcat_s(recv_buffer,chunk);
		}
		
		os_sleep(1);
	}while(igotcrlf==0);
	return recv_buffer;	
}

uint ChttpGet::ReadDataChannel()
{
	char sDataBuffer[4096];		// Data-storage buffer for the data channel
	int nBytesRecv = 0;						// Bytes received from the data channel

	fd_set	wfds;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;

	m_State = HTTP_STATE_RECEIVING;			

	do {
		FD_ZERO(&wfds);
		FD_SET( m_DataSock, &wfds );

		if ( (m_iBytesTotal) && (m_iBytesIn == m_iBytesTotal) )
			break;

		select(static_cast<int>(m_DataSock+1), &wfds, nullptr, nullptr, &timeout);
	
    	if (m_Aborting) {
			fclose(LOCALFILE);
			LOCALFILE = nullptr;
			return 0;		
		}

		nBytesRecv = recv(m_DataSock, (char *)&sDataBuffer, sizeof(sDataBuffer), 0);
	
    	if (m_Aborting) {
			fclose(LOCALFILE);
			LOCALFILE = nullptr;
			return 0;
		}
	
		if (SOCKET_ERROR == nBytesRecv) {	
			int error = WSAGetLastError();

			if ( NETCALL_WOULDBLOCK(error) ) {
				nBytesRecv = 1;
				continue;
			}
		}

		m_iBytesIn += nBytesRecv;

		if (nBytesRecv > 0 ) {
			fwrite(sDataBuffer, nBytesRecv, 1, LOCALFILE);
    	}
		
		os_sleep(1);
	} while (nBytesRecv > 0);

	fclose(LOCALFILE);	
	LOCALFILE = nullptr;
	
	// Close the file and check for error returns.
	if (nBytesRecv == SOCKET_ERROR) { 
		//Ok, we got a socket error -- xfer aborted?
		m_State = HTTP_STATE_RECV_FAILED;
		return 0;
	} else {
		//done!
		m_State = HTTP_STATE_FILE_RECEIVED;
		return 1;
	}
}
