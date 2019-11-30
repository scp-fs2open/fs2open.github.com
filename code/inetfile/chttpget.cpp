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

#define WSAGetLastError()  (errno)
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



#define NW_AGHBN_CANCEL		1
#define NW_AGHBN_LOOKUP		2
#define NW_AGHBN_READ		3

int http_gethostbynameworker(void *parm);

int http_Asyncgethostbyname(unsigned int *ip,int command, char *hostname);

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

ChttpGet::ChttpGet(char *URL,char *localfile,char *proxyip,unsigned short proxyport)
{
	m_ProxyEnabled = true;
	m_ProxyIP = proxyip;
	m_ProxyPort = proxyport;
	GetFile(URL,localfile);
}

ChttpGet::ChttpGet(char *URL,char *localfile)
{
	m_ProxyEnabled = false;
	GetFile(URL,localfile);
}


void ChttpGet::GetFile(char *URL,char *localfile)
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
	m_DataSock = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_DataSock)
	{
		m_State = HTTP_STATE_SOCKET_ERROR;
		m_Aborted = true;
		return;
	}

	unsigned long arg = 1;
	ioctlsocket( m_DataSock, FIONBIO, &arg );

	char *pURL = URL;
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
	char *filestart = NULL;
	char *dirstart = NULL;
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
	unsigned int ip;
	SERVENT *se;
	SOCKADDR_IN hostaddr;
	if(m_Aborting){
		return 0;
	}
	
	ip = inet_addr((const char *)m_szHost);

	int rcode = 0;
	if(ip==INADDR_NONE)
	{
		http_Asyncgethostbyname(&ip,NW_AGHBN_LOOKUP,m_szHost);		
		do
		{	
			if(m_Aborting)
			{
				http_Asyncgethostbyname(&ip,NW_AGHBN_CANCEL,m_szHost);
				return 0;
			}
			rcode = http_Asyncgethostbyname(&ip,NW_AGHBN_READ,m_szHost);

			os_sleep(1);
		}while(rcode==0);
	}
	
	if(rcode == -1)
	{
		m_State = HTTP_STATE_HOST_NOT_FOUND;
		return 0;
	}
	//m_ControlSock
	if(m_Aborting)
		return 0;
	se = getservbyname("http", NULL);
	if(m_Aborting)
		return 0;
	if(se == NULL)
	{
		hostaddr.sin_port = htons(80);
	}
	else
	{
		hostaddr.sin_port = se->s_port;
	}
	hostaddr.sin_family = AF_INET;
	memcpy(&hostaddr.sin_addr, &ip, 4);

	if(m_ProxyEnabled)
	{
		//This is on a proxy, so we need to make sure to connect to the proxy machine
		ip = inet_addr((const char *)m_ProxyIP);
				
		if(ip==INADDR_NONE)
		{
			http_Asyncgethostbyname(&ip,NW_AGHBN_LOOKUP,m_ProxyIP);
			rcode = 0;
			do
			{	
				if(m_Aborting)
				{
					http_Asyncgethostbyname(&ip,NW_AGHBN_CANCEL,m_ProxyIP);
					return 0;
				}
				rcode = http_Asyncgethostbyname(&ip,NW_AGHBN_READ,m_ProxyIP);

				os_sleep(1);
			}while(rcode==0);
			
			
			if(rcode == -1)
			{
				m_State = HTTP_STATE_HOST_NOT_FOUND;
				return 0;
			}

		}
		//Use either the proxy port or 80 if none specified
		hostaddr.sin_port = htons((ushort)(m_ProxyPort ? m_ProxyPort : 80));
		//Copy the proxy address...
		memcpy(&hostaddr.sin_addr,&ip,4);

	}
	//Now we will connect to the host					
	fd_set	wfds;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int serr = connect(m_DataSock, (SOCKADDR *)&hostaddr, sizeof(SOCKADDR));
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

			serr = connect(m_DataSock, (SOCKADDR *)&hostaddr, sizeof(SOCKADDR));

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


typedef struct _async_dns_lookup
{
	unsigned int ip;	//resolved host. Write only to worker thread.
	char * host;//host name to resolve. read only to worker thread
	bool done;	//write only to the worker thread. Signals that the operation is complete
	bool error; //write only to worker thread. Thread sets this if the name doesn't resolve
	bool abort;	//read only to worker thread. If this is set, don't fill in the struct.
}async_dns_lookup;

async_dns_lookup httpaslu;
async_dns_lookup *http_lastaslu = NULL;

int http_gethostbynameworker(void *parm);

int http_Asyncgethostbyname(unsigned int *ip,int command, char *hostname)
{
	
	if(command==NW_AGHBN_LOOKUP)
	{
		if(http_lastaslu)
			http_lastaslu->abort = true;

		async_dns_lookup *newaslu;
		newaslu = (async_dns_lookup *)vm_malloc(sizeof(async_dns_lookup));
		newaslu->ip = 0;
		newaslu->host = hostname;
		newaslu->done = false;
		newaslu->error = false;
		newaslu->abort = false;
		http_lastaslu = newaslu;
		httpaslu.done = false;

		SDL_CreateThread(http_gethostbynameworker, "GetHostByNameWorker", newaslu);

		return 1;
	}
	else if(command==NW_AGHBN_CANCEL)
	{
		if(http_lastaslu)
			http_lastaslu->abort = true;
		http_lastaslu = NULL;
	}
	else if(command==NW_AGHBN_READ)
	{
		if(!http_lastaslu)
			return -1;
		if(httpaslu.done)
		{
			http_lastaslu = NULL;
			*ip = httpaslu.ip;
			return 1;
		}
		else if(httpaslu.error)
		{
			vm_free(http_lastaslu);
			http_lastaslu = NULL;
			return -1;
		}
		else return 0;
	}
	return -2;

}

// This is the worker thread which does the lookup.
int http_gethostbynameworker(void *parm)
{
	async_dns_lookup *lookup = reinterpret_cast<async_dns_lookup *>(parm);
	struct hostent *he = gethostbyname(lookup->host);
	if(he==nullptr)
	{
		lookup->error = true;
		return 1;
	}
	else if(!lookup->abort)
	{
		memcpy(&lookup->ip,he->h_addr_list[0],sizeof(unsigned int));
		lookup->done = true;
		memcpy(&httpaslu,lookup,sizeof(async_dns_lookup));
	}
	vm_free(lookup);

	return 0;
}
