// udpsocket.h
// UDP Socket class for FS2Open PXO
// Derek Meek
// 2-14-2003

/*
 * $Logfile: /Freespace2/code/fs2open_pxo/udpsocket.h $
 * $Revision: 1.2 $
 * $Date: 2003-10-13 06:02:50 $
 * $Author: Kazan $
 *
 *
 * $Log: not supported by cvs2svn $
 *
 *
 */

#if !defined(__protocol_h_)
#define __protocol_h_

#if defined(WIN32)
// Windows Version
#include <windows.h>
#include <string>
//#include <winsock2.h>
#define STYPE SOCKET
#define CLOSEFUNC closesocket

#else
// Unix Version
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#define STYPE int
#define CLOSEFUNC close

#endif

class UDP_Socket
{
	private:

		bool Initialized;
		STYPE socket;
		int port;


	public:
		UDP_Socket() : Initialized(false), port(0) 
			{}
		UDP_Socket(int aport) : Initialized(false), port(aport) 
			{}

		~UDP_Socket() { Close(); }
		
		bool isInitialized() { return Initialized; }
		void SetPort(int aport) { port = aport; } 
		bool InitSocket();
		int GetPacket(char *buffer, int blen, std::string &from);
		int SendPacket(char *buffer, int plen, std::string &to, int toport);

		void Close() { CLOSEFUNC(socket); }


};

#endif
