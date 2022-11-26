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
#endif

#include "globalincs/pstypes.h"

#include <cstdio>
#include <cstring>

#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "inetfile/cftp.h"
#include "inetfile/chttpget.h"
#include "inetfile/inetgetfile.h"
#include "osapi/osapi.h"


#define INET_STATE_CONNECTING		1
#define INET_STATE_ERROR			2
#define INET_STATE_RECEIVING		3
#define INET_STATE_GOT_FILE			4


void InetGetFile::AbortGet()
{
	if (m_bUseHTTP) {
		http->AbortGet();
	} else {
		ftp->AbortGet();
	}
}

InetGetFile::InetGetFile(const char *URL, const char *filename, int cf_type)
{
	m_HardError = 0;
	http = NULL;
	ftp = NULL;

	if ( (URL == nullptr) || (filename == nullptr) || !CF_TYPE_SPECIFIED(cf_type) ) {
		m_HardError = INET_ERROR_BADPARMS;
		return;
	}

	// create directory if not already there.
	cf_create_directory(cf_type);

	// create full path for file
	SCP_string localfile;
	cf_create_default_path_string(localfile, cf_type, filename);

	if ( strstr(URL, "http:") ) {
		m_bUseHTTP = true;

		// using http proxy?
		extern char Multi_options_proxy[512];
		extern ushort Multi_options_proxy_port;

		if ( strlen(Multi_options_proxy) > 0 ) {
			http = new ChttpGet(URL, localfile.c_str(), Multi_options_proxy, Multi_options_proxy_port);
		} else {
			http = new ChttpGet(URL, localfile.c_str());
		}

		if (http == NULL) {
			m_HardError = INET_ERROR_NO_MEMORY;
		}
	} else if ( strstr(URL, "ftp:") ) {
		m_bUseHTTP = FALSE;

		ftp = new CFtpGet(URL,localfile.c_str());

		if (ftp == NULL) {
			m_HardError = INET_ERROR_NO_MEMORY;
		}
	} else {
		m_HardError = INET_ERROR_CANT_PARSE_URL;
	}

	os_sleep(1000);
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
}

bool InetGetFile::IsReceiving()
{
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
}

bool InetGetFile::IsFileReceived()
{
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
}

bool InetGetFile::IsFileError()
{
	int state;

	if (m_HardError)
		return true;

	if (m_bUseHTTP) {
		state = http->GetStatus();
	} else {
		state = ftp->GetStatus();
	}

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
}

int InetGetFile::GetErrorCode()
{
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
}

int InetGetFile::GetTotalBytes()
{
	if (m_bUseHTTP) {
		return http->GetTotalBytes();
	} else {
		return ftp->GetTotalBytes();
	}
}

int InetGetFile::GetBytesIn()
{
	if (m_bUseHTTP) {
		return http->GetBytesIn();
	} else {
		return ftp->GetBytesIn();
	}
}
