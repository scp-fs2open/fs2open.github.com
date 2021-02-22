/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _CFTP_HEADER_
#define _CFTP_HEADER_

#ifndef _WIN32
#include "netinet/in.h"
#endif

#include "globalincs/pstypes.h"

#include <cstdio>

#ifdef WIN32
#include <winsock2.h>
#endif
 
#define FTP_STATE_INTERNAL_ERROR		0
#define FTP_STATE_SOCKET_ERROR		1
#define FTP_STATE_URL_PARSING_ERROR	2
#define FTP_STATE_CONNECTING			3
#define FTP_STATE_HOST_NOT_FOUND		4
#define FTP_STATE_CANT_CONNECT		5
#define FTP_STATE_LOGGING_IN			6
#define FTP_STATE_LOGIN_ERROR			7
#define FTP_STATE_LOGGED_IN			8
#define FTP_STATE_DIRECTORY_INVALID	9
#define FTP_STATE_FILE_NOT_FOUND		10
#define FTP_STATE_RECEIVING			11
#define FTP_STATE_FILE_RECEIVED		12
#define FTP_STATE_UNKNOWN_ERROR		13
#define FTP_STATE_RECV_FAILED			14
#define FTP_STATE_CANT_WRITE_FILE	15
#define FTP_STATE_STARTUP				16


extern int FTPObjThread( void *obj );

class CFtpGet
{

public:
	CFtpGet(const char *URL, const char *localfile, const char *Username = nullptr, const char *Password = nullptr);
	~CFtpGet();
	int GetStatus();
	uint GetBytesIn();
	uint GetTotalBytes();
	void AbortGet();

	void WorkerThread();

protected:
	
	int ConnectControlSocket();
	int LoginHost();
	void LogoutHost();
	int SendFTPCommand(const char *command, SCP_string *response = nullptr);
	int ReadFTPServerReply(SCP_string *str_reply);
	uint GetFile();
	bool IssuePasv();
	uint ReadDataChannel();

	uint m_iBytesIn;
	uint m_iBytesTotal;
	int m_State;

	bool m_Aborting;
	bool m_Aborted;

	SCP_string m_szUserName;
	SCP_string m_szPassword;
	SCP_string m_szHost;
	SCP_string m_szDir;
	SCP_string m_szFilename;


	SOCKADDR_STORAGE m_ServerAddr;

	SOCKET m_DataSock;
	SOCKET m_ControlSock;

	FILE *LOCALFILE;

	SDL_Thread *thread_id;
};


#endif
