/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _INET_GETFILE_HEADER_
#define _INET_GETFILE_HEADER_

//At the end of this file is an example of usage

#include "inetfile/cftp.h"
#include "inetfile/chttpget.h"

#define INET_ERROR_NO_ERROR			0
#define INET_ERROR_BADPARMS			1
#define INET_ERROR_CANT_WRITE_FILE	2
#define INET_ERROR_CANT_PARSE_URL	3
#define INET_ERROR_BAD_FILE_OR_DIR	4
#define INET_ERROR_HOST_NOT_FOUND	5
#define INET_ERROR_UNKNOWN_ERROR		6
#define INET_ERROR_NO_MEMORY        7

class InetGetFile
{
public:
	InetGetFile(char *URL,char *localfile);
	~InetGetFile();
	bool IsFileReceived();
	bool IsFileError();
	bool IsConnecting();
	bool IsReceiving();
	int GetErrorCode();
	int GetBytesIn();
	int GetTotalBytes();
	void AbortGet();

protected:
	CFtpGet *ftp;
	ChttpGet *http;
	bool m_bUseHTTP;
	int m_ErrorCode;
	int m_State;
	int m_HardError;

};

#endif

/*

#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#include "inetfile/inetgetfile.h"

int main(int argc,char **argv)
{
	unsigned int LastPrintbytes = time(NULL);
	InetGetFile *inetfile;
	WSADATA	ws_data;
	WORD ver=MAKEWORD(1,1);
	
	int error=WSAStartup(ver,&ws_data);
	inetfile = new InetGetFile("http://www.volition-inc.com/images/download/freespace/fsdemo1x-12u.exe","e:\\fsdemo1x-12u.exe");
	do
	{
		if(inetfile->IsFileReceived())
		{
			printf("File received\n");
			break;
		}
		if(inetfile->IsFileError())
		{
			printf("File not received. Error code: %d\n",inetfile->GetErrorCode());
			break;
		}
		if(time(NULL)-LastPrintbytes>=1)
		{
			int ipct = 0;
			if(inetfile->GetTotalBytes())
			{
				ipct = 100*(float)((float)inetfile->GetBytesIn()/(float)inetfile->GetTotalBytes());
			}
			printf("Received %d Bytes out of %d (%d%%).\n",inetfile->GetBytesIn(),inetfile->GetTotalBytes(),ipct);
			LastPrintbytes = time(NULL);
		}


	}while(!kbhit());
	return 0;

}



 */
