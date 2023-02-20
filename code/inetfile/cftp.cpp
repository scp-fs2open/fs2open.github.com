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
#endif

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "inetfile/cftp.h"
#include "network/psnet2.h"


int FTPObjThread(void *obj)
{
	(reinterpret_cast<CFtpGet *>(obj))->WorkerThread();

	return (reinterpret_cast<CFtpGet *>(obj))->GetStatus();
}

void CFtpGet::AbortGet()
{
	m_Aborting = true;

	while ( !m_Aborted ) {
		os_sleep(10); // Wait for the thread to end
	}

	if (LOCALFILE != nullptr) {
		fclose(LOCALFILE);
		LOCALFILE = nullptr;
	}
}

CFtpGet::CFtpGet(const char *URL, const char *localfile, const char *Username, const char *Password)
{
	m_State = FTP_STATE_STARTUP;

	m_DataSock = INVALID_SOCKET;
	m_ControlSock = INVALID_SOCKET;
	m_iBytesIn = 0;
	m_iBytesTotal = 0;
	m_Aborting = false;
	m_Aborted = false;

	LOCALFILE = fopen(localfile, "wb");

	if (LOCALFILE == nullptr) {
		m_State = FTP_STATE_CANT_WRITE_FILE;
		m_Aborted = true;

		return;
	}

	if (Username) {
		m_szUserName = Username;
	} else {
		m_szUserName = "anonymous";
	}

	if (Password) {
		m_szPassword = Password;
	} else {
		m_szPassword = "pxouser@pxo.net";
	}

	m_ControlSock = socket(AF_INET6, SOCK_STREAM, 0);

	if (m_ControlSock == INVALID_SOCKET) {
		m_State = FTP_STATE_SOCKET_ERROR;
		m_Aborted = true;

		return;
	}

	// make sure we are in dual-stack mode (not the default on Windows)
	int i_opt = 0;
	setsockopt(m_ControlSock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&i_opt), sizeof(i_opt));

	// Parse the URL
	// Get rid of any extra ftp:// stuff
	const char *pURL = URL;

	if ( strnicmp(URL, "ftp:", 4) == 0 ) {
		pURL += 4;

		while (*pURL == '/') {
			pURL++;
		}
	}

	// There shouldn't be any : in this string
	if ( strchr(pURL, ':') ) {
		m_State = FTP_STATE_URL_PARSING_ERROR;
		m_Aborted = true;
		return;
	}

	// parse out host, directory, and filename
	SCP_string url_str(pURL);

	size_t filestart = url_str.find_last_of('/');
	size_t dirstart = url_str.find_first_of('/');

	if ( (filestart == SCP_string::npos) || (dirstart == SCP_string::npos) ) {
		m_State = FTP_STATE_URL_PARSING_ERROR;
		m_Aborted = true;
		return;
	}

	m_szHost = url_str.substr(0, dirstart);

	if (dirstart != filestart) {
		m_szDir = url_str.substr(dirstart+1, filestart);
	}

	m_szFilename = url_str.substr(filestart+1);


	SDL_Thread *thread = SDL_CreateThread(FTPObjThread, "FTPObjThread", this);

	if (thread == nullptr) {
		m_State = FTP_STATE_INTERNAL_ERROR;
		m_Aborted = true;
		return;
	} else {
		SDL_DetachThread(thread);
	}

	m_State = FTP_STATE_CONNECTING;
}



CFtpGet::~CFtpGet()
{
	if (m_DataSock != INVALID_SOCKET) {
		shutdown(m_DataSock, 2);
		closesocket(m_DataSock);
	}

	if (m_ControlSock != INVALID_SOCKET) {
		shutdown(m_ControlSock, 2);
		closesocket(m_ControlSock);
	}

	if (LOCALFILE != nullptr) {
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

	if (m_State != FTP_STATE_LOGGING_IN) {
		return;
	}

	LoginHost();

	if (m_State != FTP_STATE_LOGGED_IN) {
		return;
	}

	GetFile();

	LogoutHost();

	// We are all done now, and state has the current state.
	m_Aborted = true;
}

uint CFtpGet::GetFile()
{
	// Start off by changing into the proper dir.
	SCP_string response;
	SCP_string szCommandString;
	int rcode;

	rcode = SendFTPCommand("TYPE I");

	if (rcode >=400) {
		m_State = FTP_STATE_UNKNOWN_ERROR;
		return 0;
	}

	if (m_Aborting) {
		return 0;
	}

	if ( !m_szDir.empty() ) {
		szCommandString = "CWD ";	// end space is intentional!
		szCommandString += m_szDir;

		rcode = SendFTPCommand(szCommandString.c_str());

		if (rcode >=400) {
			m_State = FTP_STATE_DIRECTORY_INVALID;
			return 0;
		}
	}

	if (m_Aborting) {
		return 0;
	}

	if ( !IssuePasv() ) {
		m_State = FTP_STATE_UNKNOWN_ERROR;
		return 0;
	}

	if (m_Aborting) {
		return 0;
	}

	szCommandString = "RETR ";	// end space is intentional!
	szCommandString += m_szFilename;

	rcode = SendFTPCommand(szCommandString.c_str(), &response);

	if (rcode >=400) {
		m_State = FTP_STATE_FILE_NOT_FOUND;
		return 0;
	}

	// 150 Opening BINARY mode data connection for filename (num bytes)
	size_t offset = response.find('(');

	if (offset != SCP_string::npos) {
		SCP_string val = response.substr(offset+1);

		sscanf(val.c_str(), "%u ", &m_iBytesTotal);
	}

	if (m_Aborting) {
		return 0;
	}

	ReadDataChannel();

	m_State = FTP_STATE_FILE_RECEIVED;

	return 1;
}

bool CFtpGet::IssuePasv()
{
	SOCKADDR_STORAGE dataaddr;
	SCP_string response;
	SCP_string val;
	int rcode;
	uint16_t port;

	// try extended passive mode first, then fall back to regular,
	if ( (rcode = SendFTPCommand("EPSV", &response)) >= 400 ) {
		if ( (rcode = SendFTPCommand("PASV", &response)) >= 400 ) {
			return false;
		}
	}

	if (m_Aborting) {
		return false;
	}

	size_t offset = response.find('(');

	if (offset == SCP_string::npos) {
		return false;
	}

	val = response.substr(offset+1);

	// 229 Entering Extended Passive Mode (|||port|)
	if (rcode == 229) {
		unsigned int i_port;

		sscanf(val.c_str(), "|||%u|", &i_port);
		port = static_cast<uint16_t>(i_port);
	}
	// 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
	else if (rcode == 227) {
		unsigned int p1, p2;

		// per the spec for IPv6 compat we'll skip host and only grab port
		sscanf(val.c_str(), ",%u,%u)", &p1, &p2);
		port = static_cast<uint16_t>(p1 * 256 + p2);
	} else {
		return false;
	}

	if (m_Aborting) {
		return false;
	}

	memcpy(&dataaddr, &m_ServerAddr, sizeof(dataaddr));

	switch (dataaddr.ss_family) {
		case AF_INET: {
			auto *sa4 = reinterpret_cast<SOCKADDR_IN *>(&dataaddr);

			sa4->sin_port = htons(port);

			break;
		}

		case AF_INET6: {
			auto *sa6 = reinterpret_cast<SOCKADDR_IN6 *>(&dataaddr);

			sa6->sin6_port = htons(port);

			break;
		}

		default:
			Int3();
	}

	if (m_Aborting) {
		return false;
	}

	m_DataSock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	if (m_DataSock == INVALID_SOCKET) {
		return false;
	}

	// make sure we are in dual-stack mode (not the default on Windows)
	int i_opt = 0;
	setsockopt(m_DataSock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&i_opt), sizeof(i_opt));

	if (m_Aborting) {
		return false;
	}

	if ( connect(m_DataSock, reinterpret_cast<LPSOCKADDR>(&dataaddr), psnet_get_sockaddr_len(&dataaddr)) == SOCKET_ERROR ) {
		return false;
	}

	return true;
}

int CFtpGet::ConnectControlSocket()
{
	// server address will be needed again for data socket so be sure it's saved
	if ( !psnet_get_addr(m_szHost.c_str(), "ftp", &m_ServerAddr) ) {
		m_State = FTP_STATE_HOST_NOT_FOUND;
		return 0;
	}

	if (m_Aborting) {
		return 0;
	}

	// Now we will connect to the host
	if ( connect(m_ControlSock, reinterpret_cast<LPSOCKADDR>(&m_ServerAddr), psnet_get_sockaddr_len(&m_ServerAddr)) ) {
		m_State = FTP_STATE_CANT_CONNECT;
		return 0;
	}

	m_State = FTP_STATE_LOGGING_IN;

	return 1;
}


int CFtpGet::LoginHost()
{
	SCP_string szLoginString;
	int rcode;

	szLoginString = "USER ";	// end space is intentional!
	szLoginString += m_szUserName;

	rcode = SendFTPCommand(szLoginString.c_str());

	if (rcode >= 400) {
		m_State = FTP_STATE_LOGIN_ERROR;
		return 0;
	}

	szLoginString = "PASS ";	// end space is intentional!
	szLoginString += m_szPassword;

	rcode = SendFTPCommand(szLoginString.c_str());

	if (rcode >= 400) {
		m_State = FTP_STATE_LOGIN_ERROR;
		return 0;
	}

	m_State = FTP_STATE_LOGGED_IN;

	return 1;
}

void CFtpGet::LogoutHost()
{
	SendFTPCommand("QUIT");
}

int CFtpGet::SendFTPCommand(const char *command, SCP_string *response)
{
	const SCP_string eol = "\r\n";	// CRLF
	SCP_string cmd;

	if (command == nullptr) {
		return 999;
	}

	cmd = command;
	cmd += eol;

	// Send the FTP command
	if ( send(m_ControlSock, cmd.c_str(), static_cast<int>(cmd.length()), 0) == SOCKET_ERROR ) {
		// Return 999 to indicate an error has occurred
		return 999;
	}

	// Read the server's reply and return the reply code as an integer
	return ReadFTPServerReply(response);
}

int CFtpGet::ReadFTPServerReply(SCP_string *str_reply)
{
	SSIZE_T rval;
	SCP_string code_str;
	const size_t RECV_BUF_SIZE = 1024;
	char recv_buffer[RECV_BUF_SIZE];
	bool multi_line = false;
	int code;

	char *p_buf = nullptr;
	char *p_save = nullptr;

	do {
		memset(recv_buffer, 0, RECV_BUF_SIZE);

		rval = recv(m_ControlSock, recv_buffer, RECV_BUF_SIZE, 0);

		if (rval <= 0) {
			break;
		}

		if (m_Aborting) {
			break;
		}

		// buffer could contain multiple lines so loop through them all
		p_buf = strtok_r(recv_buffer, "\r", &p_save);

		while (p_buf) {
			// response eol is CRLF, so skip the LF too
			if (*p_buf == '\n') {
				++p_buf;
			}

			if ( strlen(p_buf) < 4 ) {
				break;
			}

			// skip muli-line responses
			// 230-start
			// ...
			// 230 end
			if (p_buf[3] == '-') {
				multi_line = true;
			}
			// normal response or end of multi-line
			else if (p_buf[3] == ' ') {
				if (multi_line) {
					multi_line = false;
				} else {
					code_str = SCP_string(p_buf, 0, 3);
					code = std::stoi(code_str);

					if (str_reply) {
						str_reply->assign(p_buf+4);
					}

					return code;
				}
			}

			p_buf = strtok_r(nullptr, "\r", &p_save);
		}

		os_sleep(1);
	} while (true);

	return 999;
}

uint CFtpGet::ReadDataChannel()
{
	const size_t BUF_SIZE = 4096;
	char sDataBuffer[BUF_SIZE];
	SSIZE_T nBytesRecv;

	m_State = FTP_STATE_RECEIVING;

	if (m_Aborting) {
		return 0;
	}

	do {
		if (m_Aborting) {
			return 0;
		}

		nBytesRecv = recv(m_DataSock, sDataBuffer, BUF_SIZE, 0);

		if (nBytesRecv > 0) {
			m_iBytesIn += static_cast<uint>(nBytesRecv);
			fwrite(sDataBuffer, static_cast<size_t>(nBytesRecv), 1, LOCALFILE);
		}

		os_sleep(1);
	} while (nBytesRecv > 0);

	fclose(LOCALFILE);
	LOCALFILE = nullptr;

	// Close the file and check for error returns.
	if (nBytesRecv == SOCKET_ERROR) {
		// Ok, we got a socket error -- xfer aborted?
		m_State = FTP_STATE_RECV_FAILED;
		return 0;
	} else {
		// done!
		m_State = FTP_STATE_FILE_RECEIVED;
		return 1;
	}
}
