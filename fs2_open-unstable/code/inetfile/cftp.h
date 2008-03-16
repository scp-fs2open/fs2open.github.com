/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

 /*
 * $Logfile: /Freespace2/code/Inetfile/CFtp.h $
 * $Revision: 2.3.2.1 $
 * $Date: 2007-10-15 06:43:13 $
 * $Author: taylor $
 *
 * FTP Client class (get only)
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2005/07/13 03:15:50  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.2  2004/08/11 05:06:25  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/05 09:01:55  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 1     4/20/99 4:37p Dave
 * 
 * Initial version
 *
 * $NoKeywords: $
 */

#ifndef _CFTP_HEADER_
#define _CFTP_HEADER_

#include "globalincs/pstypes.h"

#include <stdio.h>

#ifdef WIN32
#include <winsock.h>
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


#ifdef WIN32
extern void FTPObjThread( void *obj );
#else
extern int FTPObjThread( void *obj );
#endif

class CFtpGet
{

public:
	CFtpGet(char *URL, char *localfile, char *Username = NULL, char *Password = NULL);
	~CFtpGet();
	int GetStatus();
	uint GetBytesIn();
	uint GetTotalBytes();
	void AbortGet();

	void WorkerThread();

protected:
	
	int ConnectControlSocket();
	int LoginHost();	
	uint SendFTPCommand(char *command);
	uint ReadFTPServerReply();
	uint GetFile();
	uint IssuePort();
	uint ReadDataChannel();
	void FlushControlChannel();

	uint m_iBytesIn;
	uint m_iBytesTotal;
	uint m_State;

	bool m_Aborting;
	bool m_Aborted;

	char m_szUserName[100];
	char m_szPassword[100];
	char m_szHost[200];
	char m_szDir[200];
	char m_szFilename[100];
	
	char recv_buffer[1000];

	SOCKET m_ListenSock;
	SOCKET m_DataSock;
	SOCKET m_ControlSock;

	FILE *LOCALFILE;

#ifdef SCP_UNIX
	SDL_Thread *thread_id;
#endif
};


#endif
