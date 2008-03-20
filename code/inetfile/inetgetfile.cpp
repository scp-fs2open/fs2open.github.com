/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

 /*
 * $Logfile: /Freespace2/code/Inetfile/inetgetfile.cpp $
 * $Revision: 2.3.2.1 $
 * $Date: 2007-10-15 06:43:14 $
 * $Author: taylor $
 *
 * InternetGetFile Class
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/07/26 20:47:33  Kazan
 * remove MCD complete
 *
 * Revision 2.2  2004/07/12 16:32:51  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/08 02:35:09  mharris
 * porting
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     8/22/99 1:19p Dave
 * Fixed up http proxy code. Cleaned up scoring code. Reverse the order in
 * which d3d cards are detected.
 * 
 * 3     6/16/99 10:27a Andsager
 * Make directory if it does not already exist.
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

#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif
#include <stdio.h>
#include <string.h>

#include "inetfile/cftp.h"
#include "inetfile/chttpget.h"

#include "inetfile/inetgetfile.h"



#define INET_STATE_CONNECTING		1
#define INET_STATE_ERROR			2
#define INET_STATE_RECEIVING		3
#define INET_STATE_GOT_FILE			4


void InetGetFile::AbortGet()
{
#ifdef USE_INETFILE
	if (m_bUseHTTP) {
		http->AbortGet();
	} else {
		ftp->AbortGet();
	}
#endif
}

InetGetFile::InetGetFile(char *URL, char *localfile)
{
#ifdef USE_INETFILE
	m_HardError = 0;
	http = NULL;
	ftp = NULL;

	if ( (URL == NULL) || (localfile == NULL) )
		m_HardError = INET_ERROR_BADPARMS;

	// create directory if not already there.
	char dir_name[256], *end;

	// make sure localfile has \ in it or we'll be here a long time.
	if ( strstr(localfile, DIR_SEPARATOR_STR) ) {
		strcpy(dir_name, localfile);
		int len = strlen(localfile);
		end = dir_name + len;

		// start from end of localfile and go to first \ to get dirname
		while ( *end != DIR_SEPARATOR_CHAR ) {
			end--;
		}

		*end = '\0';

		if ( _mkdir(dir_name) == 0 )	{
			mprintf(( "CFILE: Created new directory '%s'\n", dir_name ));
		}
	}
printf("URL: %s\n", URL);
	if ( strstr(URL, "http:") ) {
		m_bUseHTTP = true;
printf("using http!\n");
		// using http proxy?
		extern char Multi_options_proxy[512];
		extern ushort Multi_options_proxy_port;

		if ( strlen(Multi_options_proxy) > 0 ) {
			http = new ChttpGet(URL, localfile, Multi_options_proxy, Multi_options_proxy_port);
		} else {
			http = new ChttpGet(URL, localfile);
		}

		if (http == NULL) {
			m_HardError = INET_ERROR_NO_MEMORY;
		}
	} else if ( strstr(URL, "ftp:") ) {
		m_bUseHTTP = FALSE;
printf("using ftp! (%s)\n", URL);
		ftp = new CFtpGet(URL,localfile);

		if (ftp == NULL) {
			m_HardError = INET_ERROR_NO_MEMORY;
		}
	} else {
		m_HardError = INET_ERROR_CANT_PARSE_URL;
	}

	Sleep(1000);
#endif
}

InetGetFile::~InetGetFile()
{
	if (http != NULL)
		delete http;

	if (ftp != NULL)
		delete ftp;
}

bool InetGetFile::IsConnecting()
{
#ifdef USE_INETFILE
	int state;

	if (m_bUseHTTP) {
		state = http->GetStatus();
	} else {
		state = ftp->GetStatus();
	}

	if (state == FTP_STATE_CONNECTING) {
		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

bool InetGetFile::IsReceiving()
{
#ifdef USE_INETFILE
	int state;

	if (m_bUseHTTP) {
		state = http->GetStatus();
	} else {
		state = ftp->GetStatus();
	}

	if (state == FTP_STATE_RECEIVING) {
		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

bool InetGetFile::IsFileReceived()
{
#ifdef USE_INETFILE
	int state;

	if (m_bUseHTTP) {
		state = http->GetStatus();
	} else {
		state = ftp->GetStatus();
	}

	if (state == FTP_STATE_FILE_RECEIVED) {
		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

bool InetGetFile::IsFileError()
{
#ifdef USE_INETFILE
	int state;

	if (m_HardError)
		return true;

	if (m_bUseHTTP) {
		state = http->GetStatus();
	} else {
		state = ftp->GetStatus();
	}
printf("state: %i\n", state);
	switch (state)
	{
		case FTP_STATE_URL_PARSING_ERROR:
		case FTP_STATE_HOST_NOT_FOUND:
		case FTP_STATE_DIRECTORY_INVALID:
		case FTP_STATE_FILE_NOT_FOUND:
		case FTP_STATE_CANT_CONNECT:
		case FTP_STATE_LOGIN_ERROR:
		case FTP_STATE_INTERNAL_ERROR:
		case FTP_STATE_SOCKET_ERROR:
		case FTP_STATE_UNKNOWN_ERROR:
		case FTP_STATE_RECV_FAILED:
		case FTP_STATE_CANT_WRITE_FILE:
			return true;

		case FTP_STATE_CONNECTING:
			return false;

		default:
			return false;
	}
#endif
	return false;
}

int InetGetFile::GetErrorCode()
{
#ifdef USE_INETFILE
	int state;

	if (m_HardError)
		return m_HardError;

	if (m_bUseHTTP) {
		state = http->GetStatus();
	} else {
		state = ftp->GetStatus();
	}

	switch (state)
	{
		case FTP_STATE_URL_PARSING_ERROR:
			return INET_ERROR_CANT_PARSE_URL;

		case FTP_STATE_HOST_NOT_FOUND:
			return INET_ERROR_HOST_NOT_FOUND;

		case FTP_STATE_DIRECTORY_INVALID:
		case FTP_STATE_FILE_NOT_FOUND:
			return INET_ERROR_BAD_FILE_OR_DIR;

		case FTP_STATE_CANT_CONNECT:
		case FTP_STATE_LOGIN_ERROR:
		case FTP_STATE_INTERNAL_ERROR:
		case FTP_STATE_SOCKET_ERROR:
		case FTP_STATE_UNKNOWN_ERROR:
		case FTP_STATE_RECV_FAILED:
			return INET_ERROR_UNKNOWN_ERROR;

		case FTP_STATE_CANT_WRITE_FILE:
			return INET_ERROR_CANT_WRITE_FILE;

		default:
			return INET_ERROR_NO_ERROR;
	}
#endif
	return INET_ERROR_NO_ERROR;
}

int InetGetFile::GetTotalBytes()
{
#ifdef USE_INETFILE
	if (m_bUseHTTP) {
		return http->GetTotalBytes();
	} else {
		return ftp->GetTotalBytes();
	}
#else
	return 0;
#endif
}

int InetGetFile::GetBytesIn()
{
#ifdef USE_INETFILE
	if (m_bUseHTTP) {
		return http->GetBytesIn();
	} else {
		return ftp->GetBytesIn();
	}
#else
	return 0;
#endif
}
