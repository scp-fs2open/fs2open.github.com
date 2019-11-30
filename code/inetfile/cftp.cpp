/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <cstdio>

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

#define WSAGetLastError()  (errno)
#endif

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "inetfile/cftp.h"


int FTPObjThread( void *obj )
{
	((CFtpGet *)obj)->WorkerThread();

	return (reinterpret_cast<CFtpGet *>(obj))->GetStatus();
}

void CFtpGet::AbortGet()
{
	m_Aborting = true;

	while(!m_Aborted) os_sleep(10); //Wait for the thread to end

	if(LOCALFILE != nullptr)
	{
		fclose(LOCALFILE);
		LOCALFILE = nullptr;
	}
}

CFtpGet::CFtpGet(char *URL, char *localfile, char *Username, char *Password)
{
	SOCKADDR_IN listensockaddr;
	m_State = FTP_STATE_STARTUP;

	m_ListenSock = INVALID_SOCKET;
	m_DataSock = INVALID_SOCKET;
	m_ControlSock = INVALID_SOCKET;
	m_iBytesIn = 0;
	m_iBytesTotal = 0;
	m_Aborting = false;
	m_Aborted = false;

	LOCALFILE = fopen(localfile, "wb");

	if (NULL == LOCALFILE) {
		m_State = FTP_STATE_CANT_WRITE_FILE;
		m_Aborted = true;
		return;
	}

	if(Username)
	{
		strcpy_s(m_szUserName,Username);
	}
	else
	{
		strcpy_s(m_szUserName,"anonymous");
	}
	if(Password)
	{
		strcpy_s(m_szPassword,Password);
	}
	else
	{
		strcpy_s(m_szPassword,"pxouser@pxo.net");
	}
	m_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_ListenSock)
	{
		m_State = FTP_STATE_SOCKET_ERROR;
		m_Aborted = true;
		return;
	}
	else
	{
		memset(&listensockaddr, 0, sizeof(SOCKADDR_IN));
		listensockaddr.sin_family = AF_INET;		
		listensockaddr.sin_port = 0;
		listensockaddr.sin_addr.s_addr = INADDR_ANY;
							
		// Bind the listen socket
		if (bind(m_ListenSock, (SOCKADDR *)&listensockaddr, sizeof(SOCKADDR)))
		{
			//Couldn't bind the socket
			m_State = FTP_STATE_SOCKET_ERROR;
			m_Aborted = true;
			return;
		}

		// Listen for the server connection
		if (listen(m_ListenSock, 1))	
		{
			//Couldn't listen on the socket
			m_State = FTP_STATE_SOCKET_ERROR;
			m_Aborted = true;
			return;
		}
	}
	m_ControlSock = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_ControlSock)
	{
		m_State = FTP_STATE_SOCKET_ERROR;
		m_Aborted = true;
		return;
	}
	//Parse the URL
	//Get rid of any extra ftp:// stuff
	char *pURL = URL;
	if(strnicmp(URL,"ftp:",4)==0)
	{
		pURL +=4;
		while(*pURL == '/')
		{
			pURL++;
		}
	}
	//There shouldn't be any : in this string
	if(strchr(pURL,':'))
	{
		m_State = FTP_STATE_URL_PARSING_ERROR;
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
		m_State = FTP_STATE_URL_PARSING_ERROR;
		m_Aborted = true;
		return;
	}
	else
	{
		strncpy(m_szDir,dirstart,(filestart-dirstart));
		m_szDir[(filestart-dirstart)] = '\0';
		strncpy(m_szHost,pURL,(dirstart-pURL));
		m_szHost[(dirstart-pURL)-1] = '\0';
	}
	//At this point we should have a nice host,dir and filename
	
	SDL_Thread *thread = SDL_CreateThread(FTPObjThread, "FTPObjThread", this);

	if(thread == nullptr)
	{
		m_State = FTP_STATE_INTERNAL_ERROR;
		m_Aborted = true;
		return;
	}
	else
	{
		SDL_DetachThread(thread);
	}

	m_State = FTP_STATE_CONNECTING;
}



CFtpGet::~CFtpGet()
{
	if(m_ListenSock != INVALID_SOCKET)
	{
		shutdown(m_ListenSock,2);
		closesocket(m_ListenSock);
	}
	if(m_DataSock != INVALID_SOCKET)
	{
		shutdown(m_DataSock,2);
		closesocket(m_DataSock);
	}
	if(m_ControlSock != INVALID_SOCKET)
	{
		shutdown(m_ControlSock,2);
		closesocket(m_ControlSock);
	}

	if(LOCALFILE != nullptr)
	{
		fclose(LOCALFILE);
		LOCALFILE = nullptr;
	}
}

//Returns a value to specify the status (ie. connecting/connected/transferring/done)
int CFtpGet::GetStatus()
{
	return m_State;
}

uint CFtpGet::GetBytesIn()
{
	return m_iBytesIn;
}

uint CFtpGet::GetTotalBytes()
{

	return m_iBytesTotal;
}

//This function does all the work -- connects on a blocking socket
//then sends the appropriate user and password commands
//and then the cwd command, the port command then get and finally the quit
void CFtpGet::WorkerThread()
{
	ConnectControlSocket();
	if(m_State != FTP_STATE_LOGGING_IN)
	{
		return;
	}
	LoginHost();
	if(m_State != FTP_STATE_LOGGED_IN)
	{
		return;
	}
	GetFile();

	//We are all done now, and state has the current state.
	m_Aborted = true;
}

uint CFtpGet::GetFile()
{
	//Start off by changing into the proper dir.
	char szCommandString[200];
	int rcode;
	
	sprintf(szCommandString,"TYPE I\r\n");
	rcode = SendFTPCommand(szCommandString);
	if(rcode >=400)
	{
		m_State = FTP_STATE_UNKNOWN_ERROR;	
		return 0;
	}
	if(m_Aborting)
		return 0;
	if(m_szDir[0])
	{
		if (snprintf(szCommandString, sizeof(szCommandString), "CWD %s\r\n",m_szDir) >= (int)sizeof(szCommandString)) {
			// Make sure the string is null terminated
			szCommandString[sizeof(szCommandString) - 1] = '\0';
		}
		rcode = SendFTPCommand(szCommandString);
		if(rcode >=400)
		{
			m_State = FTP_STATE_DIRECTORY_INVALID;	
			return 0;
		}
	}
	if(m_Aborting)
		return 0;
	if(!IssuePort())
	{
		m_State = FTP_STATE_UNKNOWN_ERROR;
		return 0;
	}
	if(m_Aborting)
		return 0;
	sprintf(szCommandString,"RETR %s\r\n",m_szFilename);
	rcode = SendFTPCommand(szCommandString);
	if(rcode >=400)
	{
		m_State = FTP_STATE_FILE_NOT_FOUND;	
		return 0;
	}
	if(m_Aborting)
		return 0;
	//Now we will try to determine the file size...
	char *p,*s;
	p = strchr(recv_buffer,'(');
	p++;
	if(p)
	{
		s = strchr(p,' ');
		*s = '\0';
		m_iBytesTotal = atoi(p);
	}
	if(m_Aborting)
		return 0;

	m_DataSock = accept(m_ListenSock, NULL,NULL);
	// Close the listen socket
	closesocket(m_ListenSock);
	if (m_DataSock == INVALID_SOCKET)
	{
		m_State = FTP_STATE_SOCKET_ERROR;
		return 0;
	}
	if(m_Aborting)
		return 0;
	ReadDataChannel();
	
	m_State = FTP_STATE_FILE_RECEIVED;
	return 1;
}

uint CFtpGet::IssuePort()
{

	char szCommandString[200];
	SOCKADDR_IN listenaddr;					// Socket address structure
	SOCKLEN_T iLength;						// Length of the address structure
	uint32_t nLocalPort;							// Local port for listening
	uint32_t nReplyCode;							// FTP server reply code


   // Get the address for the hListenSocket
	iLength = sizeof(listenaddr);
	if (getsockname(m_ListenSock, (LPSOCKADDR)&listenaddr,&iLength) == SOCKET_ERROR)
	{
		m_State = FTP_STATE_SOCKET_ERROR;
		return 0;
	}

	// Extract the local port from the hListenSocket
	nLocalPort = listenaddr.sin_port;
							
	// Now, reuse the socket address structure to 
	// get the IP address from the control socket.
	if (getsockname(m_ControlSock, (LPSOCKADDR)&listenaddr,&iLength) == SOCKET_ERROR)
	{
		m_State = FTP_STATE_SOCKET_ERROR;
		return 0;
	}
				
	// Format the PORT command with the correct numbers.
	sprintf(szCommandString, "PORT %d,%d,%d,%d,%d,%d\r\n",
				static_cast<int>((listenaddr.sin_addr.s_addr >> 0)  & 0xFF),
				static_cast<int>((listenaddr.sin_addr.s_addr >> 8)  & 0xFF),
				static_cast<int>((listenaddr.sin_addr.s_addr >> 16) & 0xFF),
				static_cast<int>((listenaddr.sin_addr.s_addr >> 24) & 0xFF),
				nLocalPort >> 8,
				nLocalPort & 0xFF);

	// Tell the server which port to use for data.
	nReplyCode = SendFTPCommand(szCommandString);
	if (nReplyCode != 200)
	{
		m_State = FTP_STATE_SOCKET_ERROR;
		return 0;
	}
	return 1;
}

int CFtpGet::ConnectControlSocket()
{
	HOSTENT *he;
	SERVENT *se;
	SOCKADDR_IN hostaddr;
	he = gethostbyname(m_szHost);
	if(he == NULL)
	{
		m_State = FTP_STATE_HOST_NOT_FOUND;
		return 0;
	}
	//m_ControlSock
	if(m_Aborting)
		return 0;

	memset(&hostaddr, 0, sizeof(SOCKADDR_IN));
    
	se = getservbyname("ftp", NULL);
    
	if(se == NULL)
	{
		hostaddr.sin_port = htons(21);
	}
	else
	{
		hostaddr.sin_port = se->s_port;
	}
	hostaddr.sin_family = AF_INET;		
	memcpy(&hostaddr.sin_addr,he->h_addr_list[0],4);
	if(m_Aborting)
		return 0;
	//Now we will connect to the host					
	if(connect(m_ControlSock, (SOCKADDR *)&hostaddr, sizeof(SOCKADDR)))
	{
		m_State = FTP_STATE_CANT_CONNECT;
		return 0;
	}
	m_State = FTP_STATE_LOGGING_IN;
	return 1;
}


int CFtpGet::LoginHost()
{
	char szLoginString[200];
	int rcode;
	
	sprintf(szLoginString,"USER %s\r\n",m_szUserName);
	rcode = SendFTPCommand(szLoginString);
	if(rcode >=400)
	{
		m_State = FTP_STATE_LOGIN_ERROR;	
		return 0;
	}
	sprintf(szLoginString,"PASS %s\r\n",m_szPassword);
	rcode = SendFTPCommand(szLoginString);
	if(rcode >=400)
	{
		m_State = FTP_STATE_LOGIN_ERROR;	
		return 0;
	}

	m_State = FTP_STATE_LOGGED_IN;
	return 1;
}


uint CFtpGet::SendFTPCommand(char *command)
{

	FlushControlChannel();
	// Send the FTP command
	if (SOCKET_ERROR ==(send(m_ControlSock,command,static_cast<int>(strlen(command)), 0)))
		{
		  // Return 999 to indicate an error has occurred
			return(999);
		} 
		
	// Read the server's reply and return the reply code as an integer
	return(ReadFTPServerReply());	            
}	



uint CFtpGet::ReadFTPServerReply()
{
	uint rcode;
	int iBytesRead;
	char chunk[2];
	char szcode[4];
	uint igotcrlf = 0;
	memset(recv_buffer,0,1000);
	do
	{
		chunk[0] = '\0';
		iBytesRead = recv(m_ControlSock,chunk,1,0);

		if (iBytesRead == SOCKET_ERROR)
		{
		  // Return 999 to indicate an error has occurred
			return(999);
		}
		
		if((chunk[0]==0x0a) || (chunk[0]==0x0d))
		{
			if(recv_buffer[0] != '\0') 
			{
				igotcrlf = 1;	
			}
		}
		else
		{	chunk[1] = '\0';
			strcat_s(recv_buffer,chunk);
		}
		
		os_sleep(1);	
	}while(igotcrlf==0);
					
	if(recv_buffer[3] == '-')
	{
		//Hack -- must be a MOTD
		return ReadFTPServerReply();
	}
	if(recv_buffer[3] != ' ')
	{
		//We should have 3 numbers then a space
		return 999;
	}
	memcpy(szcode,recv_buffer,3);
	szcode[3] = '\0';
	rcode = atoi(szcode);
    // Extract the reply code from the server reply and return as an integer
	return(rcode);	            
}	


uint CFtpGet::ReadDataChannel()
{
	char sDataBuffer[4096];		// Data-storage buffer for the data channel
	int nBytesRecv;						// Bytes received from the data channel
	m_State = FTP_STATE_RECEIVING;			
   if(m_Aborting)
		return 0;
	do	
   {
		if(m_Aborting)
			return 0;
		nBytesRecv = (int)recv(m_DataSock, sDataBuffer, sizeof(sDataBuffer), 0);
    					
		m_iBytesIn += nBytesRecv;
		if (nBytesRecv > 0 )
		{
			fwrite(sDataBuffer,nBytesRecv,1,LOCALFILE);
    	}

		os_sleep(1);
	}while (nBytesRecv > 0);
	fclose(LOCALFILE);							
	// Close the file and check for error returns.
	if (nBytesRecv == SOCKET_ERROR)
	{ 
		//Ok, we got a socket error -- xfer aborted?
		m_State = FTP_STATE_RECV_FAILED;
		return 0;
	}
	else
	{
		//done!
		m_State = FTP_STATE_FILE_RECEIVED;
		return 1;
	}
}	


void CFtpGet::FlushControlChannel()
{
	fd_set read_fds;	           
	struct timeval timeout;   	
	char flushbuff[3];

	timeout.tv_sec = 0;            
	timeout.tv_usec = 0;
	
	FD_ZERO(&read_fds);
	FD_SET(m_ControlSock, &read_fds);    

	while ( select(static_cast<int>(m_ControlSock+1), &read_fds, nullptr, nullptr, &timeout) )
	{
		recv(m_ControlSock,flushbuff,1,0);

		os_sleep(1);
	}
}
