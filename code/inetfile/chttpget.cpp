/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define WSAGetLastError()  (errno)
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "inetfile/inetgetfile.h"
#include "inetfile/chttpget.h"



#define NW_AGHBN_CANCEL		1
#define NW_AGHBN_LOOKUP		2
#define NW_AGHBN_READ		3

#ifdef WIN32
void __cdecl http_gethostbynameworker(void *parm);
#else
int http_gethostbynameworker(void *parm);
#endif

int http_Asyncgethostbyname(unsigned int *ip,int command, char *hostname);

#ifdef WIN32
void HTTPObjThread( void *obj )
#else
int HTTPObjThread( void *obj )
#endif
{
	((ChttpGet *)obj)->WorkerThread();
	((ChttpGet *)obj)->m_Aborted = true;
	//OutputDebugString("http transfer exiting....\n");

#ifdef SCP_UNIX
	return 0;
#endif
}

void ChttpGet::AbortGet()
{
#ifdef WIN32
	OutputDebugString("Aborting....\n");
#endif

	m_Aborting = true;
	while(!m_Aborted) Sleep(50); //Wait for the thread to end

#ifdef WIN32
	OutputDebugString("Aborted....\n");
#endif
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
#ifdef SCP_UNIX
	thread_id = NULL;
#endif

	strncpy(m_URL,URL,MAX_URL_LEN-1);
	m_URL[MAX_URL_LEN-1] = 0;

	LOCALFILE = fopen(localfile,"wb");
	if(NULL == LOCALFILE)
	{
		m_State = HTTP_STATE_CANT_WRITE_FILE;
		return;
	}
	m_DataSock = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_DataSock)
	{
		m_State = HTTP_STATE_SOCKET_ERROR;
		return;
	}

//	uint arg = 1;
//	ioctlsocket( m_DataSock, FIONBIO, &arg );

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
		return;
	}
	//read the filename by searching backwards for a /
	//then keep reading until you find the first /
	//when you found it, you have the host and dir
	char *filestart = NULL;
	char *dirstart = NULL;
	for(int i = strlen(pURL);i>=0;i--)
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
	}
	if((dirstart==NULL) || (filestart==NULL))
	{
		m_State = HTTP_STATE_URL_PARSING_ERROR;
		return;
	}
	else
	{
		strcpy_s(m_szDir,dirstart);//,(filestart-dirstart));
		//m_szDir[(filestart-dirstart)] = NULL;
		strncpy(m_szHost,pURL,(dirstart-pURL));
		m_szHost[(dirstart-pURL)-1] = '\0';
	}

#ifdef WIN32
	if ( _beginthread(HTTPObjThread,0,this) == NULL )
#else
	if ( (thread_id = SDL_CreateThread(HTTPObjThread, this)) == NULL )
#endif
	{
		m_State = HTTP_STATE_INTERNAL_ERROR;
		return;
	}
}


ChttpGet::~ChttpGet()
{
#ifdef WIN32
	_endthread();
#else
	if (thread_id)
		SDL_WaitThread(thread_id, NULL);
#endif

	if (m_DataSock != INVALID_SOCKET) {
		shutdown(m_DataSock, 2);
		closesocket(m_DataSock);
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
		return;
	}
	if(m_State != HTTP_STATE_CONNECTED)
	{
		fclose(LOCALFILE);
		return;
	}
	sprintf(szCommand,"GET %s%s HTTP/1.1\nAccept: */*\nAccept-Encoding: deflate\nHost: %s\n\n\n",m_ProxyEnabled?"":"/",m_ProxyEnabled?m_URL:m_szDir,m_szHost);
	send(m_DataSock,szCommand,strlen(szCommand),0);
	p = GetHTTPLine();
if (!p) return;
	if(strnicmp("HTTP/",p,5)==0)
	{
		char *pcode;
		pcode = strchr(p,' ')+1;
		if(!pcode)
		{
			m_State = HTTP_STATE_UNKNOWN_ERROR;	
			fclose(LOCALFILE);
			return;

		}
		pcode[3] = '\0';
		irsp = atoi(pcode);

		if(irsp == 0)
		{
			m_State = HTTP_STATE_UNKNOWN_ERROR;	
			fclose(LOCALFILE);
			return;
		}
		if(irsp==200)
		{
			int idataready=0;
			do
			{
				p = GetHTTPLine();
				if(p==NULL)
				{
					m_State = HTTP_STATE_UNKNOWN_ERROR;	
					fclose(LOCALFILE);
					return;
				}
				if(*p=='\0')
				{
					idataready = 1;
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

				Sleep(1);
			}while(!idataready);
		ReadDataChannel();
		return;
		}
		m_State = HTTP_STATE_FILE_NOT_FOUND;
		fclose(LOCALFILE);
		return;
	}
	else
	{
		m_State = HTTP_STATE_UNKNOWN_ERROR;
		fclose(LOCALFILE);
		return;
	}
}

int ChttpGet::ConnectSocket()
{
	//HOSTENT *he;
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

			Sleep(1);
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
	//ip = htonl(ip);
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

				Sleep(1);
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
//printf("cerr: %s\n", strerror(cerr));
	if (serr) {
		while ( (cerr == WSAEALREADY) || (cerr == WSAEINVAL) || (cerr == WSAEWOULDBLOCK) ) {
			FD_ZERO(&wfds);
			FD_SET( m_DataSock, &wfds );

#ifdef WIN32
			if ( select(0, NULL, &wfds, NULL, &timeout) )
#else
			if ( select(m_DataSock+1, NULL, &wfds, NULL, &timeout) )
#endif
			{
				serr = 0;
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

			Sleep(1);
		}
	}

	if (serr) {
//printf("1-serr: %i\n", serr);
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
//printf("0 - iBytesRead: %i  ==>  (%i) %s\n", iBytesRead, error, strerror(error));
				if(WSAEWOULDBLOCK==error)
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

			Sleep(1);
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
					if(WSAEWOULDBLOCK==error)
					{
//printf("1 - iBytesRead: %i  ==>  %s\n", iBytesRead, strerror(error));
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

				Sleep(1);
			}while(!gotdata);
			igotcrlf = 1;	
		}
		else
		{	chunk[1] = '\0';
			strcat_s(recv_buffer,chunk);
		}
		
		Sleep(1);
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

#ifdef WIN32
		select(0, &wfds, NULL, NULL, &timeout);
#else
		select(m_DataSock+1, &wfds, NULL, NULL, &timeout);
#endif
	
    	if (m_Aborting) {
			fclose(LOCALFILE);
			return 0;		
		}

		nBytesRecv = recv(m_DataSock, (char *)&sDataBuffer, sizeof(sDataBuffer), 0);
	
    	if (m_Aborting) {
			fclose(LOCALFILE);
			return 0;
		}
	
		if (SOCKET_ERROR == nBytesRecv) {	
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nBytesRecv = 1;
				continue;
			}
		}

		m_iBytesIn += nBytesRecv;

		if (nBytesRecv > 0 ) {
			fwrite(sDataBuffer, nBytesRecv, 1, LOCALFILE);
			//Write sDataBuffer, nBytesRecv
    	}
		
		Sleep(1);
	} while (nBytesRecv > 0);

	fclose(LOCALFILE);	
	
	// Close the file and check for error returns.
	if (nBytesRecv == SOCKET_ERROR) { 
		//Ok, we got a socket error -- xfer aborted?
		m_State = HTTP_STATE_RECV_FAILED;
		return 0;
	} else {
		//OutputDebugString("HTTP File complete!\n");
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

#ifdef WIN32
void __cdecl http_gethostbynameworker(void *parm);
#else
int http_gethostbynameworker(void *parm);
#endif

int http_Asyncgethostbyname(unsigned int *ip,int command, char *hostname)
{
	
	if(command==NW_AGHBN_LOOKUP)
	{
		if(http_lastaslu)
			http_lastaslu->abort = true;

		async_dns_lookup *newaslu;
		newaslu = (async_dns_lookup *)vm_malloc(sizeof(async_dns_lookup));
		memset(&newaslu->ip,0,sizeof(unsigned int));
		newaslu->host = hostname;
		newaslu->done = false;
		newaslu->error = false;
		newaslu->abort = false;
		http_lastaslu = newaslu;
		httpaslu.done = false;

#ifdef WIN32
		_beginthread(http_gethostbynameworker, 0, newaslu);
#else
		HOSTENT *he = gethostbyname(hostname);

		if (he == NULL) {
			newaslu->error = true;
		} else if ( !newaslu->abort ) {
			memcpy( &newaslu->ip, he->h_addr_list[0], sizeof(uint) );
			newaslu->done = true;
			memcpy( &httpaslu,newaslu, sizeof(async_dns_lookup) );
		}
	//	struct hostent *he;
	//	he = gethostbyname(hostname);
	//	newaslu->ip = (uint)((in_addr *)(he->h_addr))->s_addr;
	//	newaslu->done = true;
	//	httpaslu.done = true;
	//	thread_id
#endif

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
			//vm_free(http_lastaslu);
			http_lastaslu = NULL;
			memcpy(ip,&httpaslu.ip,sizeof(unsigned int));
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
#ifdef WIN32
void __cdecl http_gethostbynameworker(void *parm)
#else
int http_gethostbynameworker(void *parm)
#endif
{
#ifdef SCP_UNIX
//	df_pthread_detach(df_pthread_self());
#endif
	async_dns_lookup *lookup = (async_dns_lookup *)parm;
	HOSTENT *he = gethostbyname(lookup->host);
	if(he==NULL)
	{
		lookup->error = true;
#ifdef WIN32
		return;
#else
		return 0;
#endif
	}
	else if(!lookup->abort)
	{
		memcpy(&lookup->ip,he->h_addr_list[0],sizeof(unsigned int));
		lookup->done = true;
		memcpy(&httpaslu,lookup,sizeof(async_dns_lookup));
	}
	vm_free(lookup);

#ifdef SCP_UNIX
	return 0;
#endif
}
