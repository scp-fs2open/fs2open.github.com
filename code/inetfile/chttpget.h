/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _CHTTPGET_HEADER_
#define _CHTTPGET_HEADER_

#define HTTP_STATE_INTERNAL_ERROR		0
#define HTTP_STATE_SOCKET_ERROR			1
#define HTTP_STATE_URL_PARSING_ERROR	2
#define HTTP_STATE_CONNECTING				3
#define HTTP_STATE_HOST_NOT_FOUND		4
#define HTTP_STATE_CANT_CONNECT			5
#define HTTP_STATE_CONNECTED				6
#define HTTP_STATE_FILE_NOT_FOUND		10
#define HTTP_STATE_RECEIVING				11
#define HTTP_STATE_FILE_RECEIVED			12
#define HTTP_STATE_UNKNOWN_ERROR			13
#define HTTP_STATE_RECV_FAILED			14
#define HTTP_STATE_CANT_WRITE_FILE		15
#define HTTP_STATE_STARTUP					16

#define MAX_URL_LEN	300

class ChttpGet
{
public:
	ChttpGet(char *URL,char *localfile);
	ChttpGet(char *URL,char *localfile,char *proxyip,unsigned short proxyport);
	~ChttpGet();
	void GetFile(char *URL,char *localfile);
	int GetStatus();
	uint GetBytesIn();
	uint GetTotalBytes();
	void AbortGet();
	void WorkerThread();
	bool m_Aborted;

protected:
	int ConnectSocket();
	char *GetHTTPLine();
	uint ReadDataChannel();
	uint m_iBytesIn;
	uint m_iBytesTotal;
	uint m_State;
	bool m_ProxyEnabled;
	char *m_ProxyIP;
	char m_URL[MAX_URL_LEN];
	ushort m_ProxyPort;

	char m_szUserName[100];
	char m_szPassword[100];
	char m_szHost[200];
	char m_szDir[200];
	char m_szFilename[100];
	
	bool m_Aborting;


	SOCKET m_DataSock;
	
	FILE *LOCALFILE;
	char recv_buffer[1000];

#ifdef SCP_UNIX
	SDL_Thread *thread_id;
#endif
};










#endif
