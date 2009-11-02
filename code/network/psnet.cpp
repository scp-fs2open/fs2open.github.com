/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <wsipx.h>
#include <process.h>
#include <ras.h>
#include <raserror.h>
#endif
#include <stdio.h>
#include <limits.h>

#include "globalincs/pstypes.h"
#include "network/psnet.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multilag.h"
#include "osapi/osregistry.h"
#include "io/timer.h"
#include "network/multi_log.h"
#include "cmdline/cmdline.h"



#ifndef PSNET2

// old style packet buffering
// #define PSNET_BUFFER_OLD_SCHOOL

// sockets for IPX and TCP
SOCKET IPX_socket;
SOCKET IPX_reliable_socket;
SOCKET IPX_listen_socket;

SOCKET TCP_socket;
SOCKET TCP_reliable_socket;
SOCKET TCP_listen_socket;

// the sockets that the game will use when selecting network type
SOCKET Unreliable_socket = INVALID_SOCKET;
SOCKET Reliable_socket = INVALID_SOCKET;
SOCKET Listen_socket = INVALID_SOCKET;

BOOL		Psnet_my_addr_valid;
net_addr Psnet_my_addr;

ubyte Null_address[6];

int Socket_type;
int Can_broadcast;			// can we do broadcasting on our socket?
int Ipx_can_broadcast = 0;
int Tcp_can_broadcast = 0;

int Network_status;
int Tcp_failure_code = 0;
int Ipx_failure_code = 0;
int Ras_connected;
int Psnet_connection;

ushort	Psnet_default_port;

// specified their internet connnection type
#define NETWORK_CONNECTION_NONE			1
#define NETWORK_CONNECTION_DIALUP		2
#define NETWORK_CONNECTION_LAN			3

// defines and variables to indicate network connection status
#define NETWORK_STATUS_NOT_INITIALIZED	1
#define NETWORK_STATUS_NO_WINSOCK		2			// winsock failed to initialize
#define NETWORK_STATUS_NO_PROTOCOL		3			// specified protocol doesn't appear to be loaded
#define NETWORK_STATUS_NO_RELIABLE		4
#define NETWORK_STATUS_RUNNING			5			// everything should be running

// defintion of structures that actually leave this machine.  psnet_send give us only
// the data that we want to send.  We will add a header onto this data (packet sequence
// number, possibly a checksum).  We must include a 2 byte flags variable into both structure
// since the receiving end of this packet must know whether or not to checksum the packet.

// use the pack pragma to pack these structures to 2 byte aligment.  Really only needed for
// the naked packet.

#pragma pack(push, 2)

// packet def for a checksum'ed packet
#define MAX_CHECKSUM_PACKET_SIZE		1024
typedef struct network_checksum_packet
{
	int		sequence_number;
	ushort	flags;
	ushort	checksum;
	ubyte		data[MAX_CHECKSUM_PACKET_SIZE];
} network_checksum_packet;

// definition for a non-checksum packet
typedef struct network_packet
{
	int		sequence_number;
	ushort	flags;
	ubyte		data[MAX_PACKET_SIZE];
} network_naked_packet;

// structure definition for our packet buffers
typedef struct network_packet_buffer
{
	int		sequence_number;
	int		len;
	net_addr	from_addr;
	ubyte		data[MAX_PACKET_SIZE];
} network_packet_buffer;

#pragma pack(pop)

#define MAX_PACKET_BUFFERS		96

#ifdef PSNET_BUFFER_OLD_SCHOOL
	LOCAL network_packet_buffer packet_buffers[MAX_PACKET_BUFFERS];		// buffer to hold packets sent to us
	LOCAL short packet_free_list[MAX_PACKET_BUFFERS];							// contains id's of free packet buffers
	LOCAL Num_packet_buffers;
	LOCAL int Largest_packet_index = 0;

	int	Next_packet_id;
	int	Last_packet_id;
#endif

// variables to keep track of bytes read/written during the course of network play
#ifndef NDEBUG

#define PSNET_FRAME_FILTER		32
#define MAX_NET_STATS			32

typedef struct {
	int				in_use;
	net_addr			addr;						// stats for this address
	int				total_read;					// total bytes read
	int				total_written;				// total bytes read.
} net_stats;

typedef struct {
	net_addr			addr;
	SOCKET			socket;
} socket_xlate;

net_stats Psnet_stats[MAX_NET_STATS];
socket_xlate Psnet_socket_addr[MAX_NET_STATS];

int	psnet_bytes_read_frame;			// running count of bytes read this frame
int	psnet_bytes_written_frame;		// running count of bytes written this frame

int	Psnet_bytes_read;					// globally available numbers for printing on the hud
int	Psnet_bytes_written;

int	psnet_read_sizes[PSNET_FRAME_FILTER];
int	psnet_write_sizes[PSNET_FRAME_FILTER];

int	psnet_read_total;
int	psnet_write_total;
int	psnet_frame_int;
int	psnet_frame_count;

#endif

#define MAXHOSTNAME			128

#define MAX_RECEIVE_BUFSIZE	(1<<15)		// 32 K, eh?
#define MAX_SEND_RETRIES		20				// number of retries when sending would block
#define MAX_LINGER_TIME			0				// in seconds -- when lingering to close a socket

#ifndef NDEBUG

int psnet_same_no_port( net_addr * a1, net_addr * a2 );

// do stats for a particular address.  num_bytes is the number of bytes either read or
// written (depending on the read flag.  read flag is true when reading, 0 when writing
void psnet_do_net_stats( net_addr *addr, int num_bytes, int read )
{
	int i;	

	for ( i = 0; i < MAX_NET_STATS; i++ ) {
		if ( !Psnet_stats[i].in_use )
			continue;
		if ( addr && psnet_same_no_port(&Psnet_stats[i].addr, addr) )
			break;
	}

	if ( i == MAX_NET_STATS ) {
		// find the first NULL entry
		for ( i = 0; i < MAX_NET_STATS; i++ ) {
			if ( !Psnet_stats[i].in_use )
				break;
		}
		if ( i == MAX_NET_STATS ) {
			Int3();				// we should always have a slot
			return;
		}
		Psnet_stats[i].addr = *addr;
		Psnet_stats[i].in_use = 1;
	}
	if ( read )
		Psnet_stats[i].total_read += num_bytes;
	else
		Psnet_stats[i].total_written += num_bytes;
}

// returns the stats for the given network address.  returns 1 when (i.e. net_addr was found),
// 0 otherwise.  total read and total written are returned in the parameters
int psnet_get_stats( net_addr *addr, int *tr, int *tw )
{
	int i;

	for ( i = 0; i < MAX_NET_STATS; i++ ) {
		if ( !Psnet_stats[i].in_use )
			continue;

		if ( addr && psnet_same_no_port(&Psnet_stats[i].addr, addr) )
			break;
	}

	if ( i == MAX_NET_STATS )
		return 0;

	*tr = Psnet_stats[i].total_read;
	*tw = Psnet_stats[i].total_written;

	return 1;
}

void psnet_stats_init()
{
	int i;

	for ( i = 0; i < MAX_NET_STATS; i++ ) {
		Psnet_stats[i].in_use = 0;
		Psnet_stats[i].total_read = 0;
		Psnet_stats[i].total_written = 0;
		Psnet_socket_addr[i].socket = INVALID_SOCKET;
	}
}

#endif

#define NETWORK_STATUS_NOT_INITIALIZED	1
#define NETWORK_STATUS_NO_WINSOCK		2			// winsock failed to initialize
#define NETWORK_STATUS_NO_TCPIP			3			// TCP/IP doesn't appear to be loaded
#define NETWORK_STATUS_RUNNING			4			// everything should be running

// function called from high level FreeSpace code to determine the status of the networking
// code returns one of a handful of macros
int psnet_get_network_status()
{
	// first case is when "none" is selected
	if ( Psnet_connection == NETWORK_CONNECTION_NONE ) {
		return NETWORK_ERROR_NO_TYPE;
	}

	// first, check the connection status of the network
	if ( Network_status == NETWORK_STATUS_NO_WINSOCK )
		return NETWORK_ERROR_NO_WINSOCK;

	if ( Network_status == NETWORK_STATUS_NO_TCPIP )
		return NETWORK_ERROR_NO_TCPIP;
	
	// network is running -- be sure that the RAS people know to connect if they currently cannot.
	
	if ( Psnet_connection == NETWORK_CONNECTION_DIALUP ) {
		// if on a dialup connection, be sure that RAS is active.
		if ( !Ras_connected ) {
			return NETWORK_ERROR_CONNECT_TO_ISP;
		}
	} else if ( Psnet_connection == NETWORK_CONNECTION_LAN ) {
		// if on a LAN, and they have a dialup connection active, return error to indicate that they need
		// to pick the right connection type
		if ( Ras_connected ) {
			return NETWORK_ERROR_LAN_AND_RAS;
		}
	}
	return NETWORK_ERROR_NONE;
}

DWORD (__stdcall *pRasEnumConnections)(LPRASCONN lprasconn, LPDWORD lpcb, LPDWORD lpcConnections) = NULL;
DWORD (__stdcall *pRasGetConnectStatus)(HRASCONN hrasconn, LPRASCONNSTATUS lprasconnstatus ) = NULL;
DWORD (__stdcall *pRasGetProjectionInfo)(HRASCONN hrasconn, RASPROJECTION rasprojection, LPVOID lpprojection, LPDWORD lpcb ) = NULL;
 
// functions to get the status of a RAS connection
void psnet_ras_status()
{
	int rval;
	unsigned long size, num_connections, i;
	RASCONN rasbuffer[25];
	HINSTANCE ras_handle;

	Ras_connected = 0;

	// first, call a LoadLibrary to load the RAS api
	ras_handle = LoadLibrary( NOX("rasapi32.dll") );
	if ( ras_handle == NULL ) {
		return;
	}

	pRasEnumConnections = (DWORD (__stdcall *)(LPRASCONN, LPDWORD, LPDWORD))GetProcAddress(ras_handle, NOX("RasEnumConnectionsA"));
	if (!pRasEnumConnections)	{
		FreeLibrary( ras_handle );
		return;
	}
	pRasGetConnectStatus = (DWORD (__stdcall *)(HRASCONN, LPRASCONNSTATUS))GetProcAddress(ras_handle, NOX("RasGetConnectStatusA"));
	if (!pRasGetConnectStatus)	{
		FreeLibrary( ras_handle );
		return;
	}
	pRasGetProjectionInfo = (DWORD (__stdcall *)(HRASCONN, RASPROJECTION, LPVOID, LPDWORD))GetProcAddress(ras_handle, NOX("RasGetProjectionInfoA"));
	if (!pRasGetProjectionInfo)	{
		FreeLibrary( ras_handle );
		return;
	}

	size = sizeof(rasbuffer);
	rasbuffer[0].dwSize = sizeof(RASCONN);

	rval = pRasEnumConnections( rasbuffer, &size, &num_connections );
	if ( rval ) {
		FreeLibrary( ras_handle );
		return;
	}

	// JAS: My computer gets to this point, but I have no RAS connections,
	// so just exit
	if ( num_connections < 1 )	{
		nprintf(("Network", "Found no connections\n" )); 
		FreeLibrary( ras_handle );
		return;
	}

	nprintf(("Network", "Found %d connections\n", num_connections));

	for (i = 0; i < num_connections; i++ ) {
		RASCONNSTATUS status;
		RASPPPIP projection;
		unsigned long size;

		nprintf(("Network", "Connection %d:\n", i));
		nprintf(("Network", "Entry Name: %s\n", rasbuffer[i].szEntryName));
		nprintf(("Network", "Device Type: %s\n", rasbuffer[i].szDeviceType));
		nprintf(("Network", "Device Name: %s\n", rasbuffer[i].szDeviceName));

		// get the connection status
		status.dwSize = sizeof(RASCONNSTATUS);
		rval = pRasGetConnectStatus(rasbuffer[i].hrasconn, &status);
		if ( rval != 0 ) {
			FreeLibrary( ras_handle );
			return;
		}

		nprintf(("Network", "\tStatus: %s\n", (status.rasconnstate==RASCS_Connected)?"Connected":"Not Connected"));

		// get the projection informatiom
		size = sizeof(projection);
		projection.dwSize = size;
		rval = pRasGetProjectionInfo(rasbuffer[i].hrasconn, RASP_PppIp, &projection, &size );
		if ( rval != 0 ) {
			FreeLibrary( ras_handle );
			return;
		}

		printf(("Network", "\tIP Address: %s\n", projection.szIpAddress));
	}

	Ras_connected = 1;

	FreeLibrary( ras_handle );
}

// -------------------------------------------------------------------------------------------------
// netmisc_calc_checksum() calculates the checksum of a block of memory.
//
//
ushort psnet_calc_checksum( void * vptr, int len )
{
	ubyte * ptr = (ubyte *)vptr;
	unsigned int sum1,sum2;

	sum1 = sum2 = 0;

	while(len--)	{
		sum1 += *ptr++;
		if (sum1 >= 255 ) sum1 -= 255;
		sum2 += sum1;
	}
	sum2 %= 255;
	
	return (unsigned short)((sum1<<8)+ sum2);
}

// -------------------------------------------------------------------------------------------------
// psnet_set_socket_mode()
//
//

uint psnet_set_socket_mode(SOCKET sock_id, int opt, BOOL toggle)
{
	return (setsockopt(sock_id, SOL_SOCKET, opt, (LPSTR)&toggle, sizeof(toggle)));
}


// -------------------------------------------------------------------------------------------------
// sock_get_ip() returns the IP address of this computer.
//
//

uint sock_get_ip()
{
	char LclHost[MAXHOSTNAME];
	LPHOSTENT Hostent;
	SOCKADDR_IN LclAddr;
	SOCKADDR_IN RmtAddr;	
	SOCKET hSock;
	int nRet;

	// Init local address to zero
	LclAddr.sin_addr.s_addr = INADDR_ANY;
	
	// Get the local host name
	nRet = gethostname(LclHost, MAXHOSTNAME );
	if (nRet != SOCKET_ERROR )	{
		// Resolve host name for local address
		Hostent = gethostbyname((LPSTR)LclHost);
		if ( Hostent )
			LclAddr.sin_addr.s_addr = *((u_long FAR *)(Hostent->h_addr));
	}
			
	return LclAddr.sin_addr.s_addr;

	RmtAddr = RmtAddr;
	hSock = hSock;
}

// psnet_get_ip() attempts to get the local IP address of this machine
// returns 0 on failure, and 1 on success
int psnet_get_ip( SOCKET s )
{
	int rval, len;
	SOCKADDR_IN local_addr;

	if ( Psnet_my_addr_valid ){
		return 1;
	}

	memset(&local_addr, 0, sizeof(SOCKADDR_IN));
	len = sizeof(SOCKADDR_IN);
	rval = getsockname(s, (SOCKADDR *)&local_addr, &len );
	if ( rval == SOCKET_ERROR )
		return 0;

	// now we should have an IP address of this machine.
	memcpy(Psnet_my_addr.addr, &(local_addr.sin_addr), 4);
	Psnet_my_addr.port = ntohs(local_addr.sin_port);	

	Psnet_my_addr_valid = 1;
	return 1;
}

// -------------------------------------------------------------------------------------------------
// psnet_close()
//
//

void psnet_close()
{
#ifndef PSNET_RELIABLE_OLD_SCHOOL
	psnet_reliable_close();	
#endif

	if ( Network_status != NETWORK_STATUS_RUNNING )
		return;

	WSACancelBlockingCall();

	if ( TCP_listen_socket != INVALID_SOCKET ) {
		shutdown( TCP_listen_socket, 1 );
		closesocket( TCP_listen_socket );
	}

	if ( TCP_reliable_socket != INVALID_SOCKET ) {
		shutdown( TCP_reliable_socket, 1 );
		closesocket( TCP_reliable_socket );
	}

	if ( TCP_socket != INVALID_SOCKET ) {
		shutdown( TCP_socket, 1 );
		closesocket( TCP_socket );
	}

	if (WSACleanup())	{
		//Warning( LOCATION, "Error closing wsock!\n" );
	}
	
	Network_status = NETWORK_STATUS_NOT_INITIALIZED;
}

// function to initialize a reliable socket.  All it does is call the socket() command.
// returns 1 on success, 0 on failure.
int psnet_init_stream( SOCKET *s, int type )
{
	SOCKET sock;

	switch( type ) {
	case NET_IPX:
		sock = socket(AF_IPX,SOCK_STREAM,NSPROTO_SPX);
		if ( sock == INVALID_SOCKET )
			return 0;
		break;

	case NET_TCP:
		sock = socket(AF_INET,SOCK_STREAM,0);
		if ( sock == INVALID_SOCKET )
			return 0;
		break;

	default:
		Int3();
		return 0;
	}

	*s = sock;

	return 1;
}

// called by psnet_init to initialize the listen socket used by a host/server
int psnet_init_reliable(ushort port, int should_listen, int type)
{
	SOCKET sock = 0;		// JAS: Get rid of optimized warning
	SOCKADDR_IN sockaddr;		// UDP/TCP socket structure
	SOCKADDR_IPX ipx_addr;		// IPX socket structure

	// set up the reliable TCP transport socket
	if ( !psnet_init_stream(&sock, type) ) {
		nprintf(("Network", "Unable to initialize reliable socket on port %d (%d)!\n"  , port, WSAGetLastError() )); 
		return INVALID_SOCKET;
	}

	// bind the socket to the port
	switch( type ) {
	case NET_IPX:
		memset(&ipx_addr, 0, sizeof(SOCKADDR_IPX));
		ipx_addr.sa_socket = htons( port );
		ipx_addr.sa_family = AF_IPX;
		if (bind(sock, (SOCKADDR *)&ipx_addr, sizeof(SOCKADDR_IPX)) == SOCKET_ERROR) {
			nprintf(("Network", "Unable to bind reliable socket on port %d (%d)!\n"  , port, WSAGetLastError() )); 
			return INVALID_SOCKET;
		}
		break;

	case NET_TCP:
		memset( &sockaddr, 0, sizeof(SOCKADDR_IN) );
		sockaddr.sin_family = AF_INET; 
		sockaddr.sin_addr.s_addr = INADDR_ANY;  //fill in with our IP
		sockaddr.sin_port = htons( port );
		if ( bind(sock, (SOCKADDR*)&sockaddr, sizeof (sockaddr)) == SOCKET_ERROR) {	
			nprintf(("Network", "Unable to bind reliable socket on port %d (%d)!\n"  , port, WSAGetLastError() )); 
			return INVALID_SOCKET;
		}
		break;

	default:
		Int3();
		return INVALID_SOCKET;
	}


	// set up the listen on the Reliable_socket port
	if ( should_listen && listen(sock, 5) ) {
		nprintf(("Network", "Unable to listen on Listen Socket (%d)!\n"  , WSAGetLastError() )); 
		return INVALID_SOCKET;
	}

	// make any reliable sockets which we create that aren't listening non-blocking sockets
	if ( !should_listen ) {
		int error;
		unsigned long arg;

		arg = TRUE;
		error = ioctlsocket( sock, FIONBIO, &arg );
		if ( error == SOCKET_ERROR ) {
			nprintf(("Network", "Unable to make reliable socket non-blocking -- %d", WSAGetLastError() ));
			return INVALID_SOCKET;
		}

		// set the reuse option
		if ( psnet_set_socket_mode(sock, SO_REUSEADDR, 1) == SOCKET_ERROR ) {
			error = WSAGetLastError();
			nprintf(("Network", "Error setting socket to reuse address -- %d\n", error));
		}

	}

	return sock;
}

void psnet_socket_options( SOCKET sock )
{
	int broadcast;
	int ret, cursize, cursizesize, bufsize, trysize;

	// Set the mode of the socket to allow broadcasting.  We need to be able to broadcast
	// when a game is searched for in IPX mode.
	broadcast = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (LPSTR)&broadcast, sizeof(broadcast) )){
		Can_broadcast = 0;
	} else {
		Can_broadcast = 1;
	}

	// try and increase the size of my receive buffer
	bufsize = MAX_RECEIVE_BUFSIZE;
	
	// set the current size of the receive buffer
	cursizesize = sizeof(int);
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (LPSTR)&cursize, &cursizesize);
	for ( trysize = bufsize; trysize >= cursize; trysize >>= 1 ) {
		ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (LPSTR)&trysize, sizeof(trysize));
		if ( ret == SOCKET_ERROR ) {
			int wserr;

			wserr = WSAGetLastError();
			if ( (wserr == WSAENOPROTOOPT) || (wserr == WSAEINVAL) )
				break;
		} else
			break;
	}
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (LPSTR)&cursize, &cursizesize);
	nprintf(("Network", "Receive buffer set to %d\n", cursize));

	// set the current size of the send buffer
	cursizesize = sizeof(int);
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (LPSTR)&cursize, &cursizesize);
	for ( trysize = bufsize; trysize >= cursize; trysize >>= 1 ) {
		ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (LPSTR)&trysize, sizeof(trysize));
		if ( ret == SOCKET_ERROR ) {
			int wserr;

			wserr = WSAGetLastError();
			if ( (wserr == WSAENOPROTOOPT) || (wserr == WSAEINVAL) )
				break;
		} else
			break;
	}
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (LPSTR)&cursize, &cursizesize);
	nprintf(("Network", "Send buffer set to %d\n", cursize));
}


// psnet_init called at game startup time and initializes all sockets that the game might use (i.e. for both
// IPX and TCP).
void psnet_init( int protocol, int port_num )
{	
	char *internet_connection;
	WSADATA wsa_data; 	

	// UDP/TCP socket structure
	SOCKADDR_IN sockaddr; 

	// IPX socket structure
	//SOCKADDR_IPX ipx_addr;

#if defined(DEMO) || defined(OEM_BUILD) // not for FS2_DEMO
	return;
#endif

	// GAME PORT INITIALIZATION STUFF
	if ( Network_status == NETWORK_STATUS_RUNNING )
		return;

// sort of a hack; assume unix users are always on LAN :)
#ifdef _WIN32
	internet_connection = os_config_read_string(NULL, "NetworkConnection", "none");
	if ( !stricmp(internet_connection, NOX("dialup")) ) {
		Psnet_connection = NETWORK_CONNECTION_DIALUP;
	} else if ( !stricmp(internet_connection, NOX("lan")) ) {
		Psnet_connection = NETWORK_CONNECTION_LAN;
	} else {
		Psnet_connection = NETWORK_CONNECTION_NONE;
	}
#else
	Psnet_connection = NETWORK_CONNECTION_LAN;
#endif

	Network_status = NETWORK_STATUS_NO_WINSOCK;
	if (WSAStartup(0x101, &wsa_data )){
		return;
	}

	// get the port for running this game on.  Be careful that it cannot be out of bounds
	Psnet_default_port = DEFAULT_GAME_PORT;
	if ( (port_num > 1023) && (port_num < USHRT_MAX) ) {
		Psnet_default_port = (ushort)port_num;
	}

#ifndef PSNET_RELIABLE_OLD_SCHOOL
	if(!psnet_reliable_init()){
		nprintf(("Network","PSNET RELIABLE : init failed - no networking available!\n"));
		return;
	}
#endif

	// initialize TCP now
	TCP_socket = INVALID_SOCKET;
	TCP_reliable_socket = INVALID_SOCKET;
	TCP_listen_socket = INVALID_SOCKET;

	Network_status = NETWORK_STATUS_NO_TCPIP;
	TCP_socket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( TCP_socket == INVALID_SOCKET ) {
		Tcp_failure_code = WSAGetLastError();
		nprintf(("Network", "Error on TCP startup %d\n", Tcp_failure_code));
		return;
	}

	// bind the socket
	memset(&sockaddr,0,sizeof(SOCKADDR_IN));
	sockaddr.sin_family = AF_INET; 

	sockaddr.sin_addr.s_addr = INADDR_ANY;  //fill in with our IP

	sockaddr.sin_port = htons( Psnet_default_port );
	if ( bind(TCP_socket, (SOCKADDR*)&sockaddr, sizeof (sockaddr)) == SOCKET_ERROR) {	
		Tcp_failure_code = WSAGetLastError();
		nprintf(( "Network", "Couldn't bind TCP socket (%d)! Invalidating TCP\n", Tcp_failure_code )); 
		return;
	}

#ifdef PSNET_RELIABLE_OLD_SCHOOL
	// set up reliable socket (using SPX).
	TCP_reliable_socket = psnet_init_reliable( 0, 0, NET_TCP );
	if ( TCP_reliable_socket == INVALID_SOCKET ) {
		Tcp_failure_code = WSAGetLastError();
		nprintf(( "Network", "Couldn't initialize TCP reliable socket (OLD SCHOOL) (%d)! Invalidating TCP\n", Tcp_failure_code )); 
		return;
	}

	TCP_listen_socket = psnet_init_reliable( (u_short)(Psnet_default_port-1), 1, NET_TCP );
	if ( TCP_listen_socket == INVALID_SOCKET ) {
		Tcp_failure_code = WSAGetLastError();
		nprintf(( "Network", "Couldn't initialize TCP listen socket (OLD SCHOOL) (%d)! Invalidating TCP\n", Tcp_failure_code )); 
		return;
	}
#endif

	psnet_socket_options( TCP_socket );		
	Tcp_can_broadcast = Can_broadcast;
	Can_broadcast = 0;

// fire up a second UDP socket for "reliable" transport
#ifndef PSNET_RELIABLE_OLD_SCHOOL
	TCP_reliable_socket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( TCP_reliable_socket != INVALID_SOCKET )	{
		// bind the socket
		memset(&sockaddr,0,sizeof(SOCKADDR_IN));
		sockaddr.sin_family = AF_INET; 

		sockaddr.sin_addr.s_addr = INADDR_ANY;  //fill in with our IP
		//unsigned long my_ip = sock_get_ip();
		//memcpy(&sockaddr.sin_addr.s_addr,&my_ip,sizeof(uint));

		sockaddr.sin_port = htons( (ushort)(port+1) );
		if ( bind(TCP_reliable_socket, (SOCKADDR*)&sockaddr, sizeof (sockaddr)) == SOCKET_ERROR) {	
			nprintf(( "Network", "Couldn't bind TCP socket (%d)! Invalidating TCP\n", WSAGetLastError() )); 
			goto tcp_done;
		}

		psnet_socket_options( TCP_reliable_socket );		
		Can_broadcast = 0;
	}
#endif

	psnet_ras_status();

	Network_status = NETWORK_STATUS_RUNNING;

	nprintf(("Network","TCP Initialized\n"));

	// with TCP initialized, see if the RAS services are installed


#ifndef NDEBUG
	psnet_frame_int = -1;		// for counting bytes per frame
#endif

#ifdef PSNET_BUFFER_OLD_SCHOOL
	Next_packet_id = 0;
	Last_packet_id = -1;

	// initialize the packet buffer stuff.  We use an array (the packet_free_list) to determine
	// which of the packet buffers we can use next.
	int i;
	for (i=0; i<MAX_PACKET_BUFFERS; i++ )	{
		packet_buffers[i].sequence_number = -1;
		packet_free_list[i] = (short)i;
	}
	Num_packet_buffers = 0;
	Largest_packet_index = 0;
#else
	// initialize the new buffering system
	psnet_buffer_init();
#endif
}

#define MAX_TMPBUF_SIZE	1024

#ifdef PSNET_RELIABLE_OLD_SCHOOL
// function to shutdown and close the given socket.  It takes a couple of things into consideration
// when closing, such as possibly reiniting reliable sockets if they are closed here.
void psnet_rel_close_socket( PSNET_SOCKET *p_sockp )
{
	int reinit_type;
	int error, rval;
	SOCKET *sockp;

	sockp = (SOCKET *)p_sockp;

	if ( *sockp == INVALID_SOCKET )
		return;

#ifndef NDEBUG
	if ( *sockp == IPX_reliable_socket )
		nprintf(("Network", "Closing my reliable IPX socket\n"));
	else if ( *sockp == TCP_reliable_socket )
		nprintf(("network", "Closing my relibale TCP socket\n"));
	else 
		nprintf(("Network", "Closing client's reliable socket\n"));
#endif

	// make the socket blocking
	//unsigned long val = 0;
	//rval = ioctlsocket(*sockp,FIONBIO,&val);
	//if ( rval == SOCKET_ERROR ) {
	//	error = WSAGetLastError();
	//	nprintf(("Network", "Error %d when trying to make reliable socket blocking.\n", error));
	//}

	// set the socket to linger with a timeout of 0
	linger l_val;
	l_val.l_onoff = 1;
	l_val.l_linger = 0;
	rval = setsockopt(*sockp,SOL_SOCKET,SO_LINGER,(char*)&l_val,sizeof(linger));
	if ( rval == SOCKET_ERROR ) {
		error = WSAGetLastError();
		nprintf(("Network", "Error %d when trying to set linger value.\n", error));
	}

	// see if this socket is my Reliable_socket.  If so, I will have to reinitialize it!
	reinit_type = -1;
	if ( *sockp == IPX_reliable_socket )
		reinit_type = NET_IPX;
	else if ( *sockp == TCP_reliable_socket )
		reinit_type = NET_TCP;

	/*
	rval = shutdown( *sockp, 2);		// shut the whole thing down.
	if ( rval == SOCKET_ERROR ) {
		error = WSAGetLastError();
		nprintf(("Network", "Error %d on socket shutdown\n", error));
	}
	*/

	shutdown( *sockp, 1 );
	rval = 1;
	while ( rval && (rval != SOCKET_ERROR) ) {
		char tmp_buf[MAX_TMPBUF_SIZE];

		rval = recv( *sockp, tmp_buf, MAX_TMPBUF_SIZE, 0 );
		if ( rval == SOCKET_ERROR ) {
			int error;

			error = WSAGetLastError();
			if ( error == WSAEWOULDBLOCK )
				break;
		}
	}

	rval = closesocket( *sockp );
	if ( rval == SOCKET_ERROR ) {
		error = WSAGetLastError();
		nprintf(("Network", "Error %d on closing socket\n", error ));
	}

	*sockp = INVALID_SOCKET;

	// see if we need to reinitialize
	if ( reinit_type != -1 ) {
		ushort port;

		port = Psnet_default_port;
		if ( reinit_type == NET_IPX ){
			IPX_reliable_socket = psnet_init_reliable( 0, 0, NET_IPX);
			Reliable_socket = IPX_reliable_socket;
		}
		else if ( reinit_type == NET_TCP){
			TCP_reliable_socket = psnet_init_reliable( 0, 0, NET_TCP);
			Reliable_socket = TCP_reliable_socket;
		}
	}
}
#else
// function to shutdown and close the given socket.  It takes a couple of things into consideration
// when closing, such as possibly reiniting reliable sockets if they are closed here.
void psnet_close_socket( PSNET_SOCKET *p_sockp )
{
	net_addr remove;

	// copy the address to be removed
	memcpy(&remove,&((net_addr*)(*p_sockp)),sizeof(net_addr));

	// remove it from the reliable address list
	psnet_reliable_notify_drop_addr(&remove);

	// invalidate the passed in "socket"
	*p_sockp = NULL;
}
#endif

// function to check the status of the reliable socket and try to re-initialize it if necessary.
// win95 seems to have trouble doing a reinit of the socket immediately after close, so this
// function exists to check the status, and reinitialize if we need to
int psnet_rel_check()
{
	// if our Reliable_socket is valid, simply return true to indicate all is well.
	if ( Reliable_socket != INVALID_SOCKET )
		return 1;

	// based on our protocol type, try to reinitialize the socket we need.
	switch( Socket_type ) {
		
	case NET_IPX:
		IPX_reliable_socket = psnet_init_reliable( 0, 0, NET_IPX);
		Reliable_socket = IPX_reliable_socket;
		break;

	case NET_TCP:
		TCP_reliable_socket = psnet_init_reliable( 0, 0, NET_TCP);
		Reliable_socket = TCP_reliable_socket;
		break;

	}

	// return our status.
	if ( Reliable_socket == INVALID_SOCKET )
		return 0;

	return 1;
}

// this function is called from a high level at FreeSpace to set the protocol that the user wishes to use
int psnet_use_protocol( int protocol )
{
	int len;
	SOCKADDR_IPX	ipx_addr;

	// zero out my address
	Psnet_my_addr_valid = 0;
	memset( &Psnet_my_addr, 0, sizeof(Psnet_my_addr) );

	// wait until we choose a protocol to determine if we can broadcast
	Can_broadcast = 0;

	switch ( protocol ) {
	case NET_IPX:
		return 0;

		// assign the IPX_* sockets to the socket values used elsewhere
		Unreliable_socket = IPX_socket;
		Reliable_socket = IPX_reliable_socket;
		Listen_socket = IPX_listen_socket;

		Can_broadcast = Ipx_can_broadcast;
		if(Can_broadcast){
			nprintf(("Network","Psnet : IPX broadcast\n"));
		}

		// get the socket name for the IPX_socket, and put it into My_addr
		len = sizeof(SOCKADDR_IPX);
		if ( getsockname(IPX_socket, (SOCKADDR *)&ipx_addr, &len) == SOCKET_ERROR ) {
			nprintf(("Network", "Unable to get sock name for IPX unreliable socket (%d)\n", WSAGetLastError() ));
			return 0;
		}

		memcpy(Psnet_my_addr.net_id, ipx_addr.sa_netnum, 4);
		memcpy(Psnet_my_addr.addr, ipx_addr.sa_nodenum, 6);
		Psnet_my_addr.port = DEFAULT_GAME_PORT;		

		nprintf(("Network","Psnet using - NET_IPX\n"));
		break;

	case NET_TCP:
		if ( Network_status != NETWORK_STATUS_RUNNING )
			return 0;

		// assign the TCP_* sockets to the socket values used elsewhere
		Unreliable_socket = TCP_socket;
		Reliable_socket = TCP_reliable_socket;
		Listen_socket = TCP_listen_socket;

		Can_broadcast = Tcp_can_broadcast;
		if(Can_broadcast){
			nprintf(("Network","Psnet : TCP broadcast\n"));
		}

		nprintf(("Network","Psnet using - NET_TCP\n"));
		break;

	default:
		Int3();
		return 0;
	}

	Psnet_my_addr.type = protocol;
	Socket_type = protocol;

	return 1;
}

#define MAX_CONNECT_TRIES	10

// attacmpt to connect() to the server's tcp socket.  socket parameter is simply assigned to the
// Reliable_socket socket created in psnet_init
void psnet_rel_connect_to_server( PSNET_SOCKET *psocket, net_addr *server_addr)
{
	SOCKADDR_IN sockaddr;				// UDP/TCP socket structure
	SOCKADDR_IPX ipx_addr;				// IPX socket structure
	SOCKADDR *addr;						// pointer to SOCKADDR to make coding easier
	SOCKET *socket;
	ubyte iaddr[6];
	u_short port;
	int name_length, val;

	memset(iaddr, 0x00, 6);
	memcpy(iaddr, server_addr->addr, 6);
	port = (u_short)(server_addr->port - 1);

	socket = (SOCKET *)psocket;

	switch( Socket_type ) {
	case NET_IPX:
		ipx_addr.sa_family = AF_IPX;
		memcpy(ipx_addr.sa_nodenum, iaddr, 6);
		memcpy(ipx_addr.sa_netnum, server_addr->net_id, 4);
		ipx_addr.sa_socket = htons(port);
		addr = (SOCKADDR *)&ipx_addr;
		name_length = sizeof(ipx_addr);
		break;

	case NET_TCP:
		sockaddr.sin_family = AF_INET; 
		memcpy(&sockaddr.sin_addr.s_addr, iaddr, 4);
		sockaddr.sin_port = htons(port); 
		addr = (SOCKADDR *)&sockaddr;
		name_length = sizeof(sockaddr);
		break;

	default:
		Int3();
		return;
	}

	// NETLOG
	ml_string(NOX("Attempting to perform reliable TCP connect to server"));

	// connect this guys Reliable_socket to the socket parameter passed in
	*socket = INVALID_SOCKET;
	val = connect(Reliable_socket, addr, name_length);

	 // I suppose that we could have a valid socket right away
	if ( val != SOCKET_ERROR ) {
		*socket = Reliable_socket;
	} else {
		int error;

		error = WSAGetLastError();
		if ( error == WSAEWOULDBLOCK ) {			// use select to find out when socket is writeable
			int is_set, num_tries;
			fd_set wfds;
			struct timeval timeout;

			// call select -- and return if an error leaving our connection to the client invalid
			num_tries = 0;
			do {
				FD_ZERO(&wfds);
				FD_SET( Reliable_socket, &wfds );
				timeout.tv_sec = 0;
				timeout.tv_usec = 500000;			//500000 micro seconds is 1/2 second

				is_set = select( -1, NULL, &wfds, NULL, &timeout);
				// check for error on select first
				if ( is_set == SOCKET_ERROR ) {
					nprintf(("Network", "Error on select for connect %d\n", WSAGetLastError() ));
					return;
				} else if (is_set) {			// if set, then we have connection, move forward
					break;
				} else {
					Sleep(10);					// sleep for 10 ms and try again
					num_tries++;
				}
			} while ( num_tries < MAX_CONNECT_TRIES );

			if ( num_tries == MAX_CONNECT_TRIES )
				return;

			*socket = Reliable_socket;

		} else {
			nprintf(("Network", "Connecting with server failed %d\n", error ));
			*socket = INVALID_SOCKET;
		}
	}

	// if we have a connection, get my address from getsockname
	if ( *socket == Reliable_socket ) {
		if ( !psnet_get_ip(Reliable_socket) ) {
			*socket = INVALID_SOCKET;
			return;
		}
	}
}

#ifndef NDEBUG
void psnet_calc_bytes_per_frame()
{
}
#endif

// -------------------------------------------------------------------------------------------------
// psnet_same()
//
//

int psnet_same_no_port( net_addr * a1, net_addr * a2 )
{
	if ( !memcmp(a1->addr, a2->addr, 6) )
		return 1;
	return 0;
}

int psnet_same( net_addr * a1, net_addr * a2 )
{
	return psnet_same_no_port( a1, a2 );
/*
	if ( !memcmp(a1->addr, a2->addr, 6)  && a1->port == a2->port)
		return 1;
	return 0;
*/
}

// -------------------------------------------------------------------------------------------------
// psnet_describe()
//
//

char* psnet_addr_to_string( char * text, net_addr * address )
{

	if ( Network_status != NETWORK_STATUS_RUNNING )		{
		strcpy_s( text, XSTR("[no networking]",910) );
		return text;
	}

	in_addr temp_addr;

	switch ( address->type ) {
		case NET_IPX:
			sprintf(text, "%x %x %x %x: %x %x %x %x %x %x", address->net_id[0],
																			address->net_id[1],
																			address->net_id[2],
																			address->net_id[3],
																			address->addr[0],
																			address->addr[1],
																			address->addr[2],
																			address->addr[3],
																			address->addr[4],
																			address->addr[5]);
			break;

		case NET_TCP:
			memcpy(&temp_addr.s_addr, address->addr, 4);
			strcpy_s( text, inet_ntoa(temp_addr) );
			break;

		default:
			// Assert(0);
			break;

	} // end switch
	
	return text;
}


// -------------------------------------------------------------------------------------------------
// psnet_undescribe()
//
//

void psnet_string_to_addr( net_addr * address, char * text )
{
	struct hostent *he;
	char str[255], *c, *port;
	in_addr addr;

	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		strcpy_s( text, XSTR("[no networking]",910) );
		return;
	}

	// copy the text string to local storage to look for ports
	Assert( strlen(text) < 255 );
	strcpy_s(str, text);
	c = strrchr(str, ':');
	port = NULL;
	if ( c ) {
		*c = '\0';
		port = c+1;
	}

	switch ( address->type ) {
		case NET_IPX:	      
			Int3();		// no support for this yet
			break;

		case NET_TCP:
			addr.s_addr = inet_addr(str);
			// if we get INADDR_NONE returns, then we need to try and resolve the host
			// name
			if ( addr.s_addr == INADDR_NONE ) {
				he = gethostbyname( str );
				// returns a non-null pointer if successful, so get the address
				if ( he ) {
					addr.s_addr = ((in_addr *)(he->h_addr))->s_addr;			// this is the address in network byte order
				} else {
					addr.s_addr = INADDR_NONE;
				}
			}

			memset(address->addr, 0x00, 6);
			memcpy(address->addr, &addr.s_addr, 4);
			if ( port )
				address->port = (ushort)(atoi(port));
			break;

		default:
			Assert(0);
			break;

	} // end switch

}

// psnet_get_socket_data() will get data out of the socket and stuff it into the packet_buffers
// The original psnet_get() now calls this function, then determines which of the packet buffers
// to package up and use
void psnet_get_socket_data(SOCKET socket, int flags = PSNET_FLAG_RAW)
{
	SOCKADDR_IN ip_addr;				// UDP/TCP socket structure
	SOCKADDR_IPX ipx_addr;			// IPX socket structure
	fd_set	rfds;
	timeval	timeout;
	int		read_len, from_len;
	net_addr	from_addr;	
	network_checksum_packet packet_read;	
	network_checksum_packet packet_data;

	// clear the addresses to remove compiler warnings
	memset(&ip_addr, 0, sizeof(SOCKADDR_IN));
	memset(&ipx_addr, 0, sizeof(SOCKADDR_IPX));

	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		nprintf(("Network","Network ==> socket not inited in psnet_get\n"));
		return;
	}

	while ( 1 ) {		
#ifdef PSNET_BUFFER_OLD_SCHOOL
		int packet_id, id;
		// if there are no more packet buffers that we can use, then we must bail here before reading
		// any data!!!!
		if ( Num_packet_buffers >= MAX_PACKET_BUFFERS ) {
			nprintf(("Network", "Packet buffer overrun in psnet_get()\n"));
			break;
		}
#endif

		// check if there is any data on the socket to be read.  The amount of data that can be 
		// atomically read is stored in len.

		FD_ZERO(&rfds);
		FD_SET( socket, &rfds );
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		if ( select( -1, &rfds, NULL, NULL, &timeout) == SOCKET_ERROR ) {
			nprintf(("Network", "Error %d doing a socket select on read\n", WSAGetLastError()));
			break;
		}

		// if the read file descriptor is not set, then bail!
		if ( !FD_ISSET(socket, &rfds ) )
			return;

		// get data off the socket and process
		read_len = SOCKET_ERROR;
		switch ( Socket_type ) {
		case NET_IPX:
			from_len = sizeof(SOCKADDR_IPX);
			if(flags & PSNET_FLAG_RAW){
				read_len = recvfrom( socket, (char*)packet_read.data, MAX_PACKET_SIZE, 0,  (SOCKADDR*)&ipx_addr, &from_len );
			} else {
				read_len = recvfrom( socket, (char *)&packet_read, sizeof(packet_data), 0,  (SOCKADDR*)&ipx_addr, &from_len );
			}
			break;

		case NET_TCP:
			from_len = sizeof(SOCKADDR_IN);
			if(flags & PSNET_FLAG_RAW){
				read_len = recvfrom( socket, (char*)packet_read.data, MAX_PACKET_SIZE, 0,  (SOCKADDR*)&ip_addr, &from_len );
			} else {
				read_len = recvfrom( socket, (char *)&packet_read, sizeof(packet_data), 0,  (SOCKADDR*)&ip_addr, &from_len );
			}
			break;
		
		default:
			Int3();
			break;
		}

		// set the from_addr for storage into the packet buffer structure
		from_addr.type = Socket_type;

		switch ( Socket_type ) {
		case NET_IPX:
			if(socket == Reliable_socket){
				from_addr.port = Psnet_default_port;
			} else {
				from_addr.port = ntohs( ipx_addr.sa_socket );
			}
			memcpy(from_addr.addr, ipx_addr.sa_nodenum, 6 );
			memcpy(from_addr.net_id, ipx_addr.sa_netnum, 4 );
			break;

		case NET_TCP:
			if(socket == Reliable_socket){
				from_addr.port = Psnet_default_port;
			} else {
				from_addr.port = ntohs( ip_addr.sin_port );
			}
			memset(from_addr.addr, 0x00, 6);
			memcpy(from_addr.addr, &ip_addr.sin_addr.S_un.S_addr, 4);
			break;

		default:
			Assert(0);
			break;
		}

		if ( read_len == SOCKET_ERROR ) {
			int x = WSAGetLastError();
			nprintf(("Network", "Read error on socket.  Winsock error %d \n", x));
			break;
		}		

#ifndef PSNET_RELIABLE_OLD_SCHOOL
		int shave_size;
		// now we check to see if this is a reliable packet, and act accordindly
		if(socket == Reliable_socket){
			// this function processes the incoming packet, and determines if the system should continue to process the data
			shave_size = psnet_reliable_should_process(&from_addr,packet_read.data,read_len);
			if(!shave_size){				
				continue;
			} else {
				// copy in from data+2, so we skip the reliable data header
				memcpy(packet_data.data,packet_read.data + shave_size,read_len);

				// subtract out the size of the reliable header
				read_len -= shave_size;
			}
		} else {
			memcpy(packet_data.data,packet_read.data,read_len);
		}
#else
		memcpy(packet_data.data,packet_read.data,read_len);
#endif

#ifdef PSNET_BUFFER_OLD_SCHOOL
		ubyte		*data;
		int len;

		// if we had no error reading from the socket, then determine if we need to calculate a
		// checksum on the packet		
		if ( !(flags & PSNET_FLAG_RAW) && (packet_data.flags & PSNET_FLAG_CHECKSUM) ) {
			ushort checksum;			

			// calculate the size of the data that is actual real data
			len = read_len - (sizeof(packet_data) - MAX_CHECKSUM_PACKET_SIZE);
			checksum = psnet_calc_checksum( packet_data.data, len );
			if ( checksum != packet_data.checksum ) {
				nprintf(("Network", "bad checksum on incoming packet -- discarding\n"));
				return;
			}
			data = packet_data.data;
			packet_id = packet_data.sequence_number;
		} else {
			network_naked_packet *packet;

			// just read in the raw socket data and length
			if(flags & PSNET_FLAG_RAW){
				data = packet_data.data;
				len = read_len;

				packet_id = 0;
			} else {
				// we don't have a checksum packet.  cast the data to the naked packet type and
				// copy it over to passed parameters
				packet = (network_naked_packet *)&packet_data;				
	
				len = read_len - (sizeof(network_naked_packet) - MAX_PACKET_SIZE);
				data = packet->data;				

				packet_id = packet->sequence_number;
			}			
		}

		// look at the sequence number compared to what we last received		
		if ( Last_packet_id > -1 ) {
			if ( packet_id != (Last_packet_id+1) ) {
				//if ( packet_id < Last_packet_id )
				//	nprintf(("network", "packet %d came in delayed (last id was %d\n", packet_id, Last_packet_id));
				//else if ( packet_id > (Last_packet_id+1) )
				//	nprintf(("Network", "missed %d packet(s).  last id %d.  This id %d\n", (packet_id - Last_packet_id), Last_packet_id, packet_id));
			}
		}
		Last_packet_id = packet_id;				

#ifndef NDEBUG
		psnet_do_net_stats( &from_addr, read_len, 1 );
		psnet_bytes_read_frame += read_len;
#endif

		// put all of the data (length, who from, etc.) into the next available packet buffer
		// slot.  We should be assured of a slot here because of the check at the beginning
		// of the while loop
		Assert ( Num_packet_buffers < MAX_PACKET_BUFFERS );
		id = packet_free_list[ Num_packet_buffers++ ];
		if (id > Largest_packet_index ) Largest_packet_index = id;
		packet_buffers[id].len = len;		// use the flags field of the packet structure to hold the data length
		packet_buffers[id].sequence_number =  packet_id;
		packet_buffers[id].from_addr = from_addr;
		memcpy( packet_buffers[id].data, data, len );		
#else
#ifndef NDEBUG
		psnet_do_net_stats( &from_addr, read_len, 1 );
		psnet_bytes_read_frame += read_len;
#endif

		// buffer the packet
		psnet_buffer_packet(packet_data.data,read_len,&from_addr);
#endif
	}
}

// -------------------------------------------------------------------------------------------------
// psnet_send()
//
//

int psnet_send( net_addr * who_to, void * data, int len, int flags, int reliable_socket )
{
	
	SOCKET send_sock;
	SOCKADDR_IN sockaddr;				// UDP/TCP socket structure
	SOCKADDR_IPX ipx_addr;				// IPX socket structure
	int ret, send_len;
	ubyte iaddr[6], *send_data;
	short port;
	fd_set	wfds;
	struct timeval timeout;

	if(!reliable_socket){
		send_sock = Unreliable_socket;
	} else {
		send_sock = Reliable_socket;
	}

	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		nprintf(("Network","Network ==> Socket not inited in psnet_send\n"));
		return 0;
	}

	if ( psnet_same( who_to, &Psnet_my_addr) ){
		return 0;
	}

	memset(iaddr, 0x00, 6);
	memcpy(iaddr, who_to->addr, 6);

	if ( memcmp(iaddr, Null_address, 6) == 0) {
		nprintf(("Network","Network ==> send to address is 0 in psnet_send\n"));
		return 0;
	}

	if(send_sock == Unreliable_socket){
		port = who_to->port;
	} else if(send_sock == Reliable_socket){
		port = DEFAULT_GAME_PORT + 1;
	} else {
		port = who_to->port;
	}
		
	if ( port == 0) {
		nprintf(("Network","Network ==> destination port %d invalid in psnet_send\n", port));
		return 0;
	}

#ifdef PSNET_BUFFER_OLD_SCHOOL
	network_checksum_packet		Send_network_checksum_packet;
	network_naked_packet			Send_network_naked_packet;

	// determine from the flags whether or not this packet should have a checksum.
	if ( flags & PSNET_FLAG_CHECKSUM ) {      
		// can't send raw data with a checksum!
		Assert(!(flags & PSNET_FLAG_RAW));

		Send_network_checksum_packet.sequence_number = Next_packet_id++;
		Send_network_checksum_packet.flags = PSNET_FLAG_CHECKSUM;
						
		Send_network_checksum_packet.checksum = psnet_calc_checksum(data, len);
		memcpy( Send_network_checksum_packet.data, data, len );
		send_len = sizeof(Send_network_checksum_packet) - MAX_CHECKSUM_PACKET_SIZE + len;
		send_data = (ubyte *)&Send_network_checksum_packet;
	} else {		
		// send standard psnet stuff
		if(!(flags & PSNET_FLAG_RAW)){
			Send_network_naked_packet.sequence_number = Next_packet_id++;		
			Send_network_naked_packet.flags = 0;
						
			memcpy(Send_network_naked_packet.data, data, len);
			send_len = sizeof(Send_network_naked_packet) - MAX_PACKET_SIZE + len;		// gets us the real size of the structure
			send_data = (ubyte *)&Send_network_naked_packet;
		}
		// send raw data
		else {
			send_data = (ubyte*)data;
			send_len = len;
		}
	}
#else
	send_data = (ubyte*)data;
	send_len = len;
#endif


	FD_ZERO(&wfds);
	FD_SET( send_sock, &wfds );
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if ( select( -1, NULL, &wfds, NULL, &timeout) == SOCKET_ERROR ) {
		nprintf(("Network", "Error on blocking select for write %d\n", WSAGetLastError() ));
		return 0;
	}

	// if the write file descriptor is not set, then bail!
	if ( !FD_ISSET(send_sock, &wfds ) )
		return 0;

	ret = SOCKET_ERROR;
	switch ( who_to->type ) {

		case NET_IPX:
			ipx_addr.sa_socket = htons(port);
			ipx_addr.sa_family = AF_IPX;
			memcpy(ipx_addr.sa_nodenum, iaddr, 6);
			memcpy(ipx_addr.sa_netnum, who_to->net_id, 4);
				
			ret = sendto(send_sock, (char *)send_data, send_len, 0, (SOCKADDR*)&ipx_addr, sizeof(ipx_addr));
			if ( (ret != SOCKET_ERROR) && (ret != send_len) )
				nprintf(("Network", "requested to send %d bytes -- sent %d instead!!!\n", send_len, ret));
			break;



		case NET_TCP:
			sockaddr.sin_family = AF_INET; 
			memcpy(&sockaddr.sin_addr.s_addr, iaddr, 4);
			sockaddr.sin_port = htons(port); 

			ret = sendto( send_sock, (char *)send_data, send_len, 0, (SOCKADDR*)&sockaddr, sizeof(sockaddr) );
			break;

		default:
			Assert(0);	// unknown protocol
			break;

	} // end switch

	if ( ret != SOCKET_ERROR )	{
#ifndef NDEBUG
		psnet_bytes_written_frame += send_len;
		psnet_do_net_stats( who_to, send_len, 0 );
#endif
		return 1;
	}
	//Warning( LOCATION, "Couldn't send data (0x%x)!\n", WSAGetLastError() ); 
	return 0;
}

#ifdef PSNET_BUFFER_OLD_SCHOOL
// routine to "free" a packet buffer
void free_packet( int id )
{
	packet_buffers[id].sequence_number = -1;
	packet_free_list[ --Num_packet_buffers ] = (short)id;
	if ( Largest_packet_index == id)
		while ((--Largest_packet_index>0) && (packet_buffers[Largest_packet_index].sequence_number == -1 ));
}
#endif

#ifdef PSNET_RELIABLE_OLD_SCHOOL
// psnet_send_reliable sends the given data through the given reliable socket.

#define MAX_RSEND_BUFFER		2048
ubyte rsend_buffer[MAX_RSEND_BUFFER];

int psnet_rel_send( PSNET_SOCKET psocket, ubyte *data, int length, int flags )
{
	ubyte *send_data;
	int num_sent, total_sent, error, retries;
	SOCKET socket;
	unsigned short s_length;

	socket = (SOCKET)psocket;	

	// basic checks
	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		nprintf(("Network","Network ==> Socket not inited in psnet_send\n"));
		return 0;
	}

	if ( socket == INVALID_SOCKET )		// might happen in race conditions -- should get cleaned up.
		return 0;

	Assert( length < MAX_RSEND_BUFFER );

	// copy the length of the data into the beginning of the buffer.  then put the data into the buffer
	// after the length value
	Assert( length > 0 );
	s_length = (ushort)length;
	memcpy( &rsend_buffer[0], &s_length, sizeof(s_length) );
	memcpy( &rsend_buffer[2], data, length );

	retries = 0;
	total_sent = 0;
	send_data = data;
	do {
		num_sent = send( socket, (char *)rsend_buffer, length+sizeof(s_length), 0 );
		if ( num_sent == SOCKET_ERROR ) {
			error = WSAGetLastError();
			if ( (error != WSAEWOULDBLOCK) || (retries > MAX_SEND_RETRIES) )	{		// means that we would block on send -- not really an error

				// if error is would block, then set error to aborted connection
				if ( error == WSAEWOULDBLOCK )
					error = WSAECONNABORTED;

				multi_eval_socket_error(socket, error);
				return 0;
			}
			retries++;											// keep a try count
		} else {
			length -= num_sent;
			send_data += num_sent;
			total_sent += num_sent;
		}
	} while ( length > 0 );

#ifndef NDEBUG
	psnet_bytes_written_frame += total_sent;

	//int index;
	//index = psnet_find_stats_loc( who_to );
	//if ( index != -1 )
	//	Psnet_stats[index].total_written += send_len;
#endif
	return 1;
}

// get data off the reliable socket
int psnet_rel_get( PSNET_SOCKET psocket, ubyte *buffer, int max_len, int flags)
{
	int from_len, total_read, error;
	SOCKET socket;
	fd_set rfds;
	struct timeval timeout;
	short read_len;
	ubyte rread_buffer[2];

	socket = (SOCKET)psocket;

	// see if there is data to be read
	FD_ZERO(&rfds);
	FD_SET( socket, &rfds );
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if ( select( -1, &rfds, NULL, NULL, &timeout) == SOCKET_ERROR ) {
		nprintf(("Network", "Error on select for read reliable: %d\n", WSAGetLastError() ));
		return 0;
	}

	// if no data, then we can leave.
	if ( !FD_ISSET(socket, &rfds ) )
		return 0;

	// we know we have data to read.  We must read the two byte length of the packet first
	total_read = 0;
	read_len = 2;
	do {
		from_len = recv(socket, (char *)(&rread_buffer[total_read]), read_len - total_read, 0);

		// from_len will be 0 when back end gracefully closes connection.  We will assume that since
		// the close is graceful, we will info from him telling us he's left.  So we'll ignore
		// this condition here.
		if ( from_len == 0 ) {
			nprintf(("Network", "Dumping player because recv returned 0\n"));
			multi_eval_socket_error( socket, WSAECONNRESET );		// this error drops player from game.
			return 0;
		}

		// on a socket error, we need to check for WSAEWOULDBLOCK meaning that there is no more
		// data to be read.
		else if ( from_len == SOCKET_ERROR ) {
			error = WSAGetLastError();
			if ( error != WSAEWOULDBLOCK )
				multi_eval_socket_error( socket, error );
			return 0;		// get it next frame?
		}

		total_read += from_len;
	} while ( total_read < read_len );


	total_read = 0;
	memcpy(&read_len, &rread_buffer[0], 2);
	Assert( (read_len > 0) && (read_len < max_len) );
	if ( read_len == 0 )
		return 0;

	do {
		from_len = recv(socket, (char *)(buffer + total_read), read_len - total_read, 0);

		// from_len will be 0 when back end gracefully closes connection.  We will assume that since
		// the close is graceful, we will info from him telling us he's left.  So we'll ignore
		// this condition here.
		if ( from_len == 0 ) {
			nprintf(("Network", "Dumping player because recv returned 0\n"));
			multi_eval_socket_error( socket, WSAECONNRESET );		// this error drops player from game.
			return 0;
		}

		// on a socket error, we need to check for WSAEWOULDBLOCK meaning that there is no more
		// data to be read.
		else if ( from_len == SOCKET_ERROR ) {
			error = WSAGetLastError();
			if ( error != WSAEWOULDBLOCK ) {
				multi_eval_socket_error( socket, error );
				return 0;
			}
			continue;
		}

		total_read += from_len;
	} while ( total_read < read_len );

	return total_read;
}
#else
// psnet_send_reliable sends the given data through the given reliable socket.
int psnet_send_reliable( PSNET_SOCKET psocket, ubyte *data, int length, int flags )
{
	// don't do anything if the socket is null
	if(psocket == NULL){
		return 0;
	}
	
	// send a reliable data packet
	return psnet_reliable_send(data,length,(net_addr*)psocket);
}

int psnet_get_reliable( PSNET_SOCKET psocket, ubyte *buffer, int max_len, int flags)

{
	int best, best_id, i, n, size;

	// call the routine to read data out of the socket (which stuffs it into the packet buffers)
	psnet_get_socket_data(Reliable_socket,flags);

#ifdef PSNET_BUFFER_OLD_SCHOOL
	// now determine which (if any) of the packet buffers we should look at!
	best = -1;
	n = 0;
	best_id = -1;

	for (i=0; i <= Largest_packet_index; i++ )	{
		if ( packet_buffers[i].sequence_number > -1 ) {
			n++;
			if ( best == -1 || (packet_buffers[i].sequence_number < best) ) {
				best = packet_buffers[i].sequence_number;
				best_id = i;
			}
		}			
	}

	//mprintf( (0, "Best id = %d, pn = %d, last_ecb = %x, len=%x, ne = %d\n", best_id, best, last_ecb, lastlen, neterrors ));
	//mprintf( (1, "<%d> ", neterrors ));

	if ( best_id < 0 ) return 0;

	size = packet_buffers[best_id].len;
	memcpy( buffer, packet_buffers[best_id].data, size );
	memcpy( psocket, &packet_buffers[best_id].from_addr, sizeof(net_addr) );
	free_packet(best_id);

	return size;	
#else
	return 0;
#endif
}
#endif

// function which checks the Listen_socket for possibly incoming requests to be connected.
// returns 0 on error or nothing waiting.  1 if we should try to accept
int psnet_rel_check_for_listen(net_addr *from_addr)
{
	fd_set	rfds;
	timeval	timeout;
	SOCKET	sock;				// when trying to accept, this is new socket
	SOCKADDR_IN ip_addr;				// UDP/TCP socket structure
	SOCKADDR_IPX ipx_addr;			// IPX socket structure
	int from_len, error;
	unsigned long arg;

	FD_ZERO(&rfds);
	FD_SET( Listen_socket, &rfds );
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if ( select(-1, &rfds, NULL, NULL, &timeout) == SOCKET_ERROR ) {
		nprintf(("Network", "Error %d doing select on listen socket\n", WSAGetLastError() ));
		return 0;
	}

	// check to see if Listen_socket has something -- if not, return
	if ( !FD_ISSET(Listen_socket, &rfds) )
		return INVALID_SOCKET;

	sock = INVALID_SOCKET;
	switch ( Socket_type ) {
	case NET_IPX:
		from_len = sizeof(SOCKADDR_IPX);
		sock = accept( Listen_socket, (SOCKADDR*)&ipx_addr, &from_len );
		from_addr->port = ntohs( ipx_addr.sa_socket );
		memcpy(from_addr->addr, ipx_addr.sa_nodenum, 6 );
		memcpy(from_addr->net_id, ipx_addr.sa_netnum, 4 );
		nprintf(("Network","Accepted SPX connection!!\n"));
		break;

	case NET_TCP:
		from_len = sizeof(SOCKADDR_IN);
		sock = accept( Listen_socket, (SOCKADDR*)&ip_addr, &from_len );
		from_addr->port = ntohs( ip_addr.sin_port );
		memset(from_addr->addr, 0x00, 6);
		memcpy(from_addr->addr, &ip_addr.sin_addr.S_un.S_addr, 4);
		nprintf(("Network","Accepted TCP connected!!\n"));
		break;
	
	default:
		Int3();
		break;
	}

	if ( !psnet_get_ip(sock) ) {
		return INVALID_SOCKET;
	}

	// make the new socket non-blocking
	if(sock != INVALID_SOCKET){
		arg = TRUE;
		error = ioctlsocket( sock, FIONBIO, &arg );
		if ( error == SOCKET_ERROR ) {
			nprintf(("Network", "Unable to make accepted socket non-blocking -- %d", WSAGetLastError() ));
			return INVALID_SOCKET;
		}
	}

	return sock;

}

// psnet_get() will call the above function to read data out of the socket.  It will then determine
// which of the buffers we should use and pass to the routine which called us
int psnet_get( void * data, net_addr* from_addr, int flags )
{			
	// USE THIS CODE TO TEST NON-BUFFERED SOCKET READS. OUT-OF-ORDER PACKETS DROP TO NEARLY 0
	// - Dave
	/*
	fd_set	rfds;
	timeval	timeout;
	int read_len,from_len;
	SOCKADDR_IN ip_addr;
	FD_ZERO(&rfds);
	FD_SET( Unreliable_socket, &rfds );
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if ( select( -1, &rfds, NULL, NULL, &timeout) == SOCKET_ERROR ) {
		nprintf(("Network", "Error %d doing a socket select on read\n", WSAGetLastError()));
		return 0;
	}

	// if the read file descriptor is not set, then bail!
	if ( !FD_ISSET(Unreliable_socket, &rfds ) )
		return 0;

	// get data off the socket and process
	read_len = SOCKET_ERROR;	
	from_len = sizeof(SOCKADDR_IN);	
	read_len = recvfrom( Unreliable_socket, (char*)data, MAX_PACKET_SIZE, 0,  (SOCKADDR*)&ip_addr, &from_len );	
	from_addr->port = ntohs( ip_addr.sin_port );	
	memset(from_addr->addr, 0x00, 6);
	memcpy(from_addr->addr, &ip_addr.sin_addr.S_un.S_addr, 4);	
	return read_len;	
	*/
	
	// call the routine to read data out of the socket (which stuffs it into the packet buffers)
	psnet_get_socket_data(Unreliable_socket,flags);

#ifdef PSNET_BUFFER_OLD_SCHOOL
	int best, best_id, i, n, size;

	// now determine which (if any) of the packet buffers we should look at!
	best = -1;
	n = 0;
	best_id = -1;

	for (i=0; i <= Largest_packet_index; i++ )	{
		if ( packet_buffers[i].sequence_number > -1 ) {
			n++;
			if ( best == -1 || (packet_buffers[i].sequence_number < best) ) {
				best = packet_buffers[i].sequence_number;
				best_id = i;
			}
		}			
	}

	//mprintf( (0, "Best id = %d, pn = %d, last_ecb = %x, len=%x, ne = %d\n", best_id, best, last_ecb, lastlen, neterrors ));
	//mprintf( (1, "<%d> ", neterrors ));

	if ( best_id < 0 ) return 0;

	size = packet_buffers[best_id].len;
	memcpy( data, packet_buffers[best_id].data, size );
	memcpy( from_addr, &packet_buffers[best_id].from_addr, sizeof(net_addr) );
	free_packet(best_id);

	return size;
#else
	int buffer_size;

	// try and get a free buffer and return its size
	if(psnet_buffer_get_next((ubyte*)data,&buffer_size,from_addr)){
		return buffer_size;
	}

	// return nothing
	return 0;
#endif
}



// -------------------------------------------------------------------------------------------------
// psnet_broadcast()
//
//

int psnet_broadcast( net_addr * who_to, void * data, int len, int flags )
{
	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		nprintf(("Network","Network ==> Socket not inited in psnet_broadcast\n"));
		return 0;
	}

	if ( !Can_broadcast ) {
		nprintf(("Network", "Cannot broadcast -- returning without doing anything\n"));
		return 0;
	}

	ubyte broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	// broadcasting works on a local subnet which is all we really want to do for now anyway.
	// we might keep this in as an option for freespace later.
	switch ( who_to->type ) {
		case NET_IPX:
		case NET_TCP:

			memcpy(who_to->addr, broadcast, 6);
			psnet_send(who_to, data, len, flags);
			break;
	
	} // end switch

	return 1;
}

// called to clear out the socket of any remaining data

void psnet_flush()
{
	ubyte data[MAX_PACKET_SIZE];
	net_addr from_addr;

	while ( psnet_get( data, &from_addr ) > 0 ) ;
}



#ifndef NDEBUG
// function to keep track of bytes read/written on average during the frame
void psnet_calc_socket_stats()
{
	if ( psnet_frame_int == -1 )	{
		int i;
		for ( i = 0; i < PSNET_FRAME_FILTER; i++ ) {
			psnet_read_sizes[i] = 0;
			psnet_write_sizes[i] = 0;
		}
		psnet_read_total = 0;
		psnet_write_total = 0;
		psnet_frame_int = 0;
	}
	psnet_read_total -= psnet_read_sizes[psnet_frame_int];
	psnet_write_total -= psnet_write_sizes[psnet_frame_int];

	psnet_read_total += psnet_bytes_read_frame;
	psnet_write_total += psnet_bytes_written_frame;

	psnet_read_sizes[psnet_frame_int] = psnet_bytes_read_frame;
	psnet_write_sizes[psnet_frame_int] = psnet_bytes_written_frame;

	psnet_frame_int = (psnet_frame_int + 1 ) % PSNET_FRAME_FILTER;

	if ( psnet_frame_count > 0 )	{
		if ( psnet_frame_count >= PSNET_FRAME_FILTER )
			Psnet_bytes_read = psnet_read_total / PSNET_FRAME_FILTER;
		else
			Psnet_bytes_read = psnet_read_total / psnet_frame_count;

		if ( psnet_frame_count >= PSNET_FRAME_FILTER )
			Psnet_bytes_written = psnet_write_total / PSNET_FRAME_FILTER;
		else
			Psnet_bytes_written = psnet_write_total / psnet_frame_count;
	}

	psnet_frame_count++;
	psnet_bytes_read_frame = 0;
	psnet_bytes_written_frame = 0;
}
#endif

int psnet_is_valid_numeric_ip(char *ip)
{
	char *token;
	char copy[100];
	int val1,val2,val3,val4;

	// get the first ip value
	strcpy_s(copy,ip);
	token = strtok(copy,".");
	if(token == NULL){
		return 0;
	} else {
		// get the value of the token
		val1 = atoi(token);
		if((val1 < 0) || (val1 > 255)){
			return 0;
		}
	}

	// second ip value
	token = strtok(NULL,".");
	if(token == NULL){
		return 0;
	} else {
		// get the value of the token
		val2 = atoi(token);
		if((val2 < 0) || (val2 > 255)){
			return 0;
		}
	}

	// third ip value
	token = strtok(NULL,".");
	if(token == NULL){
		return 0;
	} else {
		// get the value of the token
		val3 = atoi(token);
		if((val3 < 0) || (val3 > 255)){
			return 0;
		}
	}

	// third ip value
	token = strtok(NULL,"");
	if(token == NULL){
		return 0;
	} else {
		// get the value of the token
		val4 = atoi(token);
		if((val4 < 0) || (val4 > 255)){
			return 0;
		}
	}

	// make sure he hasn't entered all 0's
	if((val1 == 0) && (val2 == 0) && (val3 == 0) && (val4 == 0)){
		return 0;
	}

	// valid
	return 1;
}


// returns true or false if the given string is a valid ip string or not.
// allow port allows us to append the port number on the end of the ip string with
// <addr>:<port#>
// so we must be sure to remove the port number
int psnet_is_valid_ip_string( char *ip_string, int allow_port )
{
	in_addr addr;
	struct hostent *host_ent;
	char str[255], *c;

	// our addresses may have ports, so make local copy and remove port number
	Assert( strlen(ip_string) < 255 );
	strcpy_s(str, ip_string);
	c = strrchr(str, ':');
	if ( c ){
		*c = '\0';
	}	

	addr.s_addr = inet_addr(ip_string);
	if ( addr.s_addr != INADDR_NONE ){
		// make sure the ip string is a valid format string
		if(psnet_is_valid_numeric_ip(ip_string)){
			return 1;
		}
	}

	// try name resolution
	host_ent = gethostbyname( ip_string );
	if ( !host_ent ){
		return 0;
	}

	// valid host entry so return 1;
	return 1;
}


// ------------------------------------------------------------------------------------------------------
// PACKET BUFFERING FUNCTIONS
//

#ifndef PSNET_BUFFER_OLD_SCHOOL

// a sequence number of -1 will indicate that this packet is not valid
LOCAL network_packet_buffer Psnet_buffers[MAX_PACKET_BUFFERS];
LOCAL int Psnet_seq_number = 0;
LOCAL int Psnet_lowest_id = 0;
LOCAL int Psnet_highest_id = 0;

// initialize the buffering system
void psnet_buffer_init()
{
	int idx;
	
	// blast the buffer clean
	memset(Psnet_buffers,0,sizeof(network_packet_buffer) * MAX_PACKET_BUFFERS);
	
	// set all buffer sequence #'s to -1
	for(idx=0;idx<MAX_PACKET_BUFFERS;idx++){
		Psnet_buffers[idx].sequence_number = -1;
	}

	// initialize the sequence #
	Psnet_seq_number = 0;
	Psnet_lowest_id = -1;
	Psnet_highest_id = -1;
}

// buffer a packet (maintain order!)
void psnet_buffer_packet(ubyte *data, int length, net_addr *from)
{
	int idx;
	int found_buf = 0;
	
	// find the first empty packet
	for(idx=0;idx<MAX_PACKET_BUFFERS;idx++){
		if(Psnet_buffers[idx].sequence_number == -1){
			found_buf = 1;
			break;
		}
	}

	// if we didn't find the buffer, report an overrun
	if(!found_buf){
		nprintf(("Network","WARNING - Buffer overrun in psnet\n"));
	} else {
		// copy in the data
		memcpy(Psnet_buffers[idx].data,data,length);
		Psnet_buffers[idx].len = length;
		memcpy(&Psnet_buffers[idx].from_addr,from,sizeof(net_addr));
		Psnet_buffers[idx].sequence_number = Psnet_seq_number;
		
		// keep track of the highest id#
		Psnet_highest_id = Psnet_seq_number++;

		// set the lowest id# for the first time
		if(Psnet_lowest_id == -1){
			Psnet_lowest_id = Psnet_highest_id;
		}
	}
}

// get the index of the next packet in order!
int psnet_buffer_get_next(ubyte *data, int *length, net_addr *from)
{	
	int idx;
	int found_buf = 0;

	// if there are no buffers, do nothing
	if((Psnet_lowest_id == -1) || (Psnet_lowest_id > Psnet_highest_id)){
		return 0;
	}

	// search until we find the lowest packet index id#
	for(idx=0;idx<MAX_PACKET_BUFFERS;idx++){
		// if we found the buffer
		if(Psnet_buffers[idx].sequence_number == Psnet_lowest_id){
			found_buf = 1;
			break;
		}
	}

	// at this point, we should _always_ have found the buffer
	Assert(found_buf);
	
	// copy out the buffer data
	memcpy(data,Psnet_buffers[idx].data,Psnet_buffers[idx].len);
	*length = Psnet_buffers[idx].len;
	memcpy(from,&Psnet_buffers[idx].from_addr,sizeof(net_addr));

	// now we need to cleanup the packet list

	// mark the buffer as free
	Psnet_buffers[idx].sequence_number = -1;
	Psnet_lowest_id++;

	return 1;
}

#else 

// initialize the buffering system
void psnet_buffer_init() {}

// buffer a packet (maintain order!)
void psnet_buffer_packet(ubyte *data, int length) {}

// get the index of the next packet in order!
int psnet_buffer_get_next() {	return -1; }

#endif

// ------------------------------------------------------------------------------------------------------
// RELIABLE UDP FUNCTIONS
//

// verbose debug output
#define PSNET_RELIABLE_VERBOSE

// overall buffer space allocated to the buffers
#define PSNET_RELIABLE_MAX_OUT_BUFFER_SIZE							(1<<18)			// 256k
#define PSNET_RELIABLE_MAX_IN_BUFFER_SIZE								(1<<12)			// 4k

// outgoing reliable packets
typedef struct reliable_packet_out {
	ubyte			data[MAX_PACKET_SIZE];					// data in the original packet
	ushort		packet_size;								// size of the original packet
	ushort		player_flags;								// bitflags indexing (1<<N) into the Psnet_reliable_addr[N] array
	ushort		player_acks;								// bitflags indexing (1<<N) into the Psnet_reliable_addr[N] array
	int			player_stamps[MAX_PLAYERS];			// timeouts for each player
	ubyte			num_resends[MAX_PLAYERS];				// # of times we've resent to a given player
	ushort		out_id;										// identifier of the packet
	fix			age;											// how old this packet is
} reliable_packet_out;

// incoming reliable packets
typedef struct reliable_packet_in {
	ushort		in_id;										// identifier of the received packet
	fix			age;											// when we received the packet
} reliable_packet_in;

// # of outgoing and incoming buffers we'll allocate
#define PSNET_RELIABLE_NUM_OUT_BUFFERS					(PSNET_RELIABLE_MAX_OUT_BUFFER_SIZE / sizeof(reliable_packet_out))
#define PSNET_RELIABLE_NUM_IN_BUFFERS					(PSNET_RELIABLE_MAX_IN_BUFFER_SIZE / sizeof(reliable_packet_in))					

// timeout to be used for the packets
#define PSNET_RELIABLE_TIMEOUT							1500				// in ms

// # of repeats to use
#define PSNET_RELIABLE_REPEAT								8					// repeat this many times max

// invalid id#
#define PSNET_RELIABLE_INVALID							0		

// ack code
#define PSNET_RELIABLE_ACK									0xffff

// the outgoing and incoming buffers themselves
reliable_packet_out *Psnet_reliable_out[PSNET_RELIABLE_NUM_OUT_BUFFERS];
reliable_packet_in *Psnet_reliable_in[PSNET_RELIABLE_NUM_IN_BUFFERS];

// psnet reliable address list
net_addr Psnet_reliable_addr[MAX_PLAYERS];			// address of registered "reliable" addrs
int Psnet_reliable_addr_flags;							// bitflags indicating which of the above are valid

// my local identifier # (will only use lower 12 bits)
ushort Psnet_reliable_local_id = 0x1;

// is the reliable system initialized
int Psnet_reliable_inited = 0;

// # of times a packet has not been delivered
int Psnet_reliable_fail_count = 0;

// # of times packets have had to be resent
int Psnet_reliable_resend_count = 0;

// # of times we've run out of packets and had to overwrite existing ones
int Psnet_reliable_overwrite_count = 0;

// forward declarations ------------------------------------------------------

// free up all used buffers 
void psnet_reliable_free_all_buffers();

// get the index of the passed address or -1 if it doesn't exist
int psnet_reliable_addr_index(net_addr *addr);

// get the index into the address array from the passed bitflag
int psnet_reliable_index_addr(int flags);

// kill all outgoing packets belonging to the passed index
void psnet_reliable_kill_outgoing(int index);

// generate a unique id #, given the passed in value and the address it came from, return PSNET_RELIABLE_INVALID
// upper 12 bytes should be valid # and the lower 4 should be free for this function to fill in
ushort psnet_reliable_get_unique_id(ushort id_num,net_addr *from);

// get the upper 12 bit version of my id# and increment the original
ushort psnet_reliable_get_next_id();

// get the index of a free outgoing relible packet buffer, killing the oldest if necessary
int psnet_reliable_get_free_outgoing();

// get the index of a free incoming relible packet buffer, killing the oldest if necessary
int psnet_reliable_get_free_incoming();

// actually send the data contained with the reliable_packet_out
int psnet_reliable_send_packet(reliable_packet_out *out,int force_index = 0);

// evaluate the status of a reliable packet - and free it up if the thing is _done_
void psnet_reliable_evaluate_status(reliable_packet_out *out);

// determine if the passed id# exists in the in-packet list, return the instance where it exists, or -1 if it doesn't
int psnet_reliable_find_in_id(ushort id_num);

// determine if the passed id# exists in the out-packet list, return the instance where it exists, or -1 if it doesn't
int psnet_reliable_find_out_id(ushort id_num);

// send an ack to the specified address
void psnet_reliable_send_ack(net_addr *addr,ushort id_num);


// extern functions -----------------------------------------------------------

// initialize the psnet reliable system (return 0 on fail, 1 on success)
int psnet_reliable_init()
{
	int idx;

	// if the system is already inited, do nothing
	if(Psnet_reliable_inited){
		return 1;
	}
	
#ifdef PSNET_RELIABLE_VERBOSE
	nprintf(("Network","PSNET RELIABLE SIZES : \n   OUT BUFFER SIZE : %d\n   OUT BUFFER COUNT %d\n   IN BUFFER SIZE %d\n   IN BUFFER COUNT %d\n",
				PSNET_RELIABLE_MAX_OUT_BUFFER_SIZE,PSNET_RELIABLE_NUM_OUT_BUFFERS,PSNET_RELIABLE_MAX_IN_BUFFER_SIZE,PSNET_RELIABLE_NUM_IN_BUFFERS));
#endif

	// null all buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_OUT_BUFFERS;idx++){
		Psnet_reliable_out[idx] = NULL;
	}
	for(idx=0;idx<PSNET_RELIABLE_NUM_IN_BUFFERS;idx++){
		Psnet_reliable_in[idx] = NULL;
	}

	// initialize all outgoing buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_OUT_BUFFERS;idx++){
		Psnet_reliable_out[idx] = NULL;
		Psnet_reliable_out[idx] = (reliable_packet_out*)vm_malloc(sizeof(reliable_packet_out));

		// if we failed to allocate the buffer, return failure
		if(Psnet_reliable_out[idx] == NULL){
			psnet_reliable_free_all_buffers();
			return 0;
		}
		memset(Psnet_reliable_out[idx],0,sizeof(reliable_packet_out));
	}

	// initialize all incoming buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_IN_BUFFERS;idx++){
		Psnet_reliable_in[idx] = NULL;
		Psnet_reliable_in[idx] = (reliable_packet_in*)vm_malloc(sizeof(reliable_packet_in));

		// if we failed to allocate the buffer, return failure
		if(Psnet_reliable_in[idx] == NULL){
			psnet_reliable_free_all_buffers();
			return 0;
		}
		memset(Psnet_reliable_in[idx],0,sizeof(reliable_packet_in));
	}

	// blast the reliable address list free
	memset(Psnet_reliable_addr,0,sizeof(net_addr) * MAX_PLAYERS);
	Psnet_reliable_addr_flags = 0;

	// set the system to be initialized
	Psnet_reliable_inited = 1;

	// initialize my local id #
	Psnet_reliable_local_id = 0x1;

	// initialize the packet delivery fail count
	Psnet_reliable_fail_count = 0;

	// intialize the packet necessary resend count
	Psnet_reliable_resend_count = 0;

	// initialize # of times we've run out of packets and had to overwrite existing ones
	Psnet_reliable_overwrite_count = 0;

	// return success
	return 1;
}

// shutdown the reliable system (free up buffers, etc)
void psnet_reliable_close()
{
	// if the system is not initialized, don't do anything
	if(!Psnet_reliable_inited){
		return;
	}

	// free up all buffers
	psnet_reliable_free_all_buffers();

	// blast all addresses clean
	memset(Psnet_reliable_addr,0,sizeof(net_addr) * MAX_PLAYERS);
	Psnet_reliable_addr_flags = 0;

	// set the system as being uninitialized
	Psnet_reliable_inited = 0;
}

// notify the reliable system of a new address at index N
void psnet_reliable_notify_new_addr(net_addr *addr,int index)
{
	// copy in the address
	memcpy(&Psnet_reliable_addr[index],addr,sizeof(net_addr));

	// set the bit indicating its validity
	Psnet_reliable_addr_flags |= (1<<index);
}

// notify the reliable system of a drop at index N
void psnet_reliable_notify_drop_addr(net_addr *addr)
{
	int index;

	// do a lookup for the address
	index = psnet_reliable_addr_index(addr);
	if(index != -1){		
		// clear out all packets belonging exclusively to this address
		psnet_reliable_kill_outgoing(index);

		// clear the address and its existence bit
		memset(&Psnet_reliable_addr[index],0,sizeof(net_addr));
		Psnet_reliable_addr_flags &= ~(1<<index);
	}
}

// send a reliable data packet
int psnet_reliable_send(ubyte *data,int packet_size,net_addr *addr)
{
	int free_buffer;
	int to_index;
	reliable_packet_out *p;
	
	// if the system is not initialized, don't do anything
	if(!Psnet_reliable_inited){
		return 0;
	}

	// try and find a guy to send to 
	to_index = -1;
	to_index = psnet_reliable_addr_index(addr);
	if(to_index == -1){
		nprintf(("Network","PSNET RELIABLE : could not find player for outgoing packet!\n"));
		return 0;
	}

	// attempt to get a free buffer
	free_buffer = -1;
	free_buffer = psnet_reliable_get_free_outgoing();
	if(free_buffer == -1){
		Int3();						// should never happen - we should always overwrite the oldest buffer
	}

	// setup the data for the outgoing packet
	p = Psnet_reliable_out[free_buffer];
	memcpy(p->data,data,packet_size);
	p->packet_size = (ushort)packet_size;
	p->player_flags |= (1<<to_index);
	memset(p->player_stamps,0xf,sizeof(int) * MAX_PLAYERS);
	p->player_stamps[to_index] = timestamp(PSNET_RELIABLE_TIMEOUT);
	p->out_id = psnet_reliable_get_next_id();
	p->age = timer_get_fixed_seconds();

	// send the packet
	return psnet_reliable_send_packet(p);
}

// process frame for all reliable stuff (call once per frame)
void psnet_reliable_process()
{
	int idx,s_idx;
	reliable_packet_out *p;
	
	// if the system is not initialized, don't do anything
	if(!Psnet_reliable_inited){
		return;
	}

	// go through all active packets
	for(idx=0;idx<PSNET_RELIABLE_NUM_OUT_BUFFERS;idx++){
		// if the packet is active
		if(Psnet_reliable_out[idx]->out_id != PSNET_RELIABLE_INVALID){
			p = Psnet_reliable_out[idx];

			// loop through all active players and see if we need to resend
			for(s_idx=0;s_idx<MAX_PLAYERS;s_idx++){
				// if the packet is active for this player and he hasn't acked
				if(p->player_flags & (1<<s_idx) && !(p->player_acks & (1<<s_idx))){
					// if the timestamp has elapsed
					if((p->player_stamps[s_idx] != -1) && timestamp_elapsed(p->player_stamps[s_idx])){
						// if we are at max resends, bomb!
						if(p->num_resends[s_idx] >= PSNET_RELIABLE_REPEAT){
#ifdef PSNET_RELIABLE_VERBOSE
							nprintf(("Network","PSNET RELIABLE : packet failed to be delivered (%d retries) !!\n",PSNET_RELIABLE_REPEAT));
#endif
							p->player_flags &= ~(1<<s_idx);

							// increment the fail count
							Psnet_reliable_fail_count++;
						}
						// otherwise resend the packet
						else {
							// actually send the data contained with the reliable_packet_out
							psnet_reliable_send_packet(p,s_idx);

							// increment the resend count
							Psnet_reliable_resend_count++;

#ifdef PSNET_RELIABLE_VERBOSE
							nprintf(("Network","PSNET RELIABLE : resending packet\n"));
#endif
						}
					}
				}
			}

			// evaluate if this packet has completed
			psnet_reliable_evaluate_status(p);
		}
	}
}

// determine if the passed in reliable data should be processed, and sends an ack if necessary
// return # of bytes which should be stripped off the data (reliable data header)
int psnet_reliable_should_process(net_addr *addr,ubyte *data,int packet_size)
{
	ushort id_num,ack_code,unique_id;
	int packet_index;
	int player_index;
	int free_index;
	reliable_packet_in *p;

	// get the reliable packet id #
	memcpy(&id_num,data,sizeof(ushort));

	// if the id# is an ack, get the id# do a lookup
	if(id_num == PSNET_RELIABLE_ACK){
#ifdef PSNET_RELIABLE_VERBOSE
		nprintf(("Network","PSNET RELIABLE : ACK 1\n"));
#endif

		ack_code = id_num;

		// get the id#
		memcpy(&id_num,data+2,sizeof(ushort));

		// get the packet index
		// unique_id = psnet_reliable_get_unique_id(id_num,addr);
		unique_id = id_num;
		packet_index = psnet_reliable_find_out_id(unique_id);
		player_index = psnet_reliable_addr_index(addr);			
		if((packet_index != -1) && (player_index != -1)){
#ifdef PSNET_RELIABLE_VERBOSE
			nprintf(("Network","PSNET RELIABLE : ACK 2\n"));
#endif

			Psnet_reliable_out[packet_index]->player_acks |= (1<<player_index);
			
			// check to see if this packet is _done_
			psnet_reliable_evaluate_status(Psnet_reliable_out[packet_index]);			
		}

		// return 4 bytes processed
		return 4;
	}

	// otherwise - see if this is a new packet
	packet_index = psnet_reliable_find_in_id(id_num);
	if(packet_index == -1){
		// get a free index
		free_index = -1;
		free_index = psnet_reliable_get_free_incoming();
		if(free_index == -1){
			Int3();
		}

		// setup the incoming packet
		p = Psnet_reliable_in[free_index];
		p->age = timer_get_fixed_seconds();
		p->in_id = psnet_reliable_get_unique_id(id_num,addr);

		// send an ack
		psnet_reliable_send_ack(addr,id_num);

		// return 2 bytes processed
		return 2;
	}
	// send another ack for good measure
	else {
		psnet_reliable_send_ack(addr,id_num);
	}

	return 0;	
}

// forward definitions --------------------------------------------

// free up all used buffers
void psnet_reliable_free_all_buffers()
{
	int idx;

	// free all outgoing buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_OUT_BUFFERS;idx++){
		// if the buffer is not null, free it
		if(Psnet_reliable_out[idx] != NULL){
			vm_free(Psnet_reliable_out[idx]);
			Psnet_reliable_out[idx] = NULL;
		}
	}

	// free all incoming buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_IN_BUFFERS;idx++){
		// if the buffer is not null, free it
		if(Psnet_reliable_in[idx] != NULL){
			vm_free(Psnet_reliable_in[idx]);
			Psnet_reliable_in[idx] = NULL;
		}
	}
}

// get the index of the passed address or -1 if it doesn't exist
int psnet_reliable_addr_index(net_addr *addr)
{
	int idx;

	// look through all valid addresses
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if((Psnet_reliable_addr_flags & (1<<idx)) && psnet_same(addr,&Psnet_reliable_addr[idx])){
			return idx;
		}
	}

	// couldn't find the address
	return -1;
}

// get the index into the address array from the passed bitflag
int psnet_reliable_index_addr(int flags)
{
	int idx;

	// look through all the bits in the flags
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(flags & (1<<idx)){
			return idx;
		}
	}

	// couldn't find any
	return -1;
}

// kill all outgoing packets belonging to the passed index
void psnet_reliable_kill_outgoing(int index)
{
	int idx;

	// go through all buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_OUT_BUFFERS;idx++){
		if(Psnet_reliable_out[idx]->out_id != PSNET_RELIABLE_INVALID){
			// if it is exclusively his, kill the whole packet
			if(Psnet_reliable_out[idx]->player_flags == (1<<index)){
				memset(Psnet_reliable_out[idx],0,sizeof(reliable_packet_out));
				continue;
			} 
			// if it belongs to him and other players, kill his entry
			else if(Psnet_reliable_out[idx]->player_flags & (1<<index)){
				Psnet_reliable_out[idx]->player_flags &= ~(1<<index);
				Psnet_reliable_out[idx]->num_resends[index] = 0;
			}
		}
	}
}

// generate a unique id #, given the passed in value and the address it came from, return PSNET_RELIABLE_INVALID
// upper 12 bytes should be valid # and the lower 4 should be free for this function to fill in
ushort psnet_reliable_get_unique_id(ushort id_num,net_addr *from)
{
	int idx;
	ushort cast;
	
	// if the lower 4 bits are not clear, we've got a problem
	if(id_num & 0xf){
		Int3();
	}

	// lookup through the Psnet_reliable_addr[] list and try and find the index
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if((idx == 1) /*(Psnet_reliable_addr_flags & (1<<idx)) && (psnet_same(from,&Psnet_reliable_addr[idx]))*/){
			// fill in the lower 4 bits
			cast = (ushort)idx;
			id_num |= (cast & 0xf);
			return id_num;
		}
	}

	// couldn't find an id#
	return PSNET_RELIABLE_INVALID;
}

// get the upper 12 bit version of my id# and increment the original
ushort psnet_reliable_get_next_id()
{
	ushort id_num = Psnet_reliable_local_id;

	// clear out the upper 4 bits
	id_num &= 0x0fff;

	// shift 4 bits to the left
	id_num <<= 4;

	// increment the local id #
	if(Psnet_reliable_local_id == 0x0fff){
		Psnet_reliable_local_id = 0x1;
	} else {
		Psnet_reliable_local_id++;
	}

	// return the shifted value
	return id_num;
}

// get the index of a free outgoing relible packet buffer, killing the oldest if necessary
int psnet_reliable_get_free_outgoing()
{
	fix oldest = -1;
	int oldest_index = -1;
	int idx;

	// search through all buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_OUT_BUFFERS;idx++){
		if(Psnet_reliable_out[idx]->out_id == PSNET_RELIABLE_INVALID){
			return idx;
		}

		// keep track of the oldest packet
		if((oldest_index == -1) || (Psnet_reliable_out[idx]->age < oldest)){
			oldest_index = idx;
			oldest = Psnet_reliable_out[idx]->age;
		}
	}

	// if we got here, all of our buffers are full, so we should kill the oldest one
	memset(Psnet_reliable_out[oldest_index],0,sizeof(reliable_packet_out));
	Psnet_reliable_overwrite_count = 0;
#ifdef PSNET_RELIABLE_VERBOSE
	nprintf(("Network","PSNET RELIABLE : overwriting old send buffer\n"));
#endif
	return oldest_index;
}

// get the index of a free incoming relible packet buffer, killing the oldest if necessary
int psnet_reliable_get_free_incoming()
{
	fix oldest = -1;
	int oldest_index = -1;
	int idx;

	// search through all buffers
	for(idx=0;idx<PSNET_RELIABLE_NUM_IN_BUFFERS;idx++){
		if(Psnet_reliable_in[idx]->in_id == PSNET_RELIABLE_INVALID){
			return idx;
		}

		// keep track of the oldest packet
		if((oldest_index == -1) || (Psnet_reliable_in[idx]->age < oldest)){
			oldest_index = idx;
			oldest = Psnet_reliable_in[idx]->age;
		}
	}

	// if we got here, all of our buffers are full, so we should kill the oldest one
	memset(Psnet_reliable_in[oldest_index],0,sizeof(reliable_packet_in));	
#ifdef PSNET_RELIABLE_VERBOSE
	nprintf(("Network","PSNET RELIABLE : overwriting old recv buffer\n"));
#endif
	return oldest_index;
}

// actually send the data contained with the reliable_packet_out
int psnet_reliable_send_packet(reliable_packet_out *out,int force_index)
{	
	ubyte data[MAX_PACKET_SIZE];	
	int idx,bytes_sent;
	int packet_size = 2 + out->packet_size;

	// stick the identifier on the front of the packet
	memcpy(data,&out->out_id,sizeof(ushort));

	// copy in the actual data
	memcpy(data+2,out->data,out->packet_size);

	// send the packet and update the timestamp for all players
	
	// send to one specified player in the packet
	bytes_sent = 0;
	if(force_index != 0){
		if(out->player_flags & (1<<force_index) && !(out->player_acks & (1<<force_index))){
			bytes_sent = psnet_send(&Psnet_reliable_addr[force_index],data,packet_size,PSNET_FLAG_RAW,1);			
			out->player_stamps[force_index] = timestamp(PSNET_RELIABLE_TIMEOUT);
			out->num_resends[force_index]++;
		}
	}
	// send to all players contained in the packet
	else {
		for(idx=0;idx<MAX_PLAYERS;idx++){
			// if this guy is flagged and he exists and he hasn't already acked
			if(out->player_flags & (1<<idx) && (Psnet_reliable_addr_flags & (1<<idx)) && !(out->player_acks & (1<<idx))){
				bytes_sent = psnet_send(&Psnet_reliable_addr[idx],data,packet_size,PSNET_FLAG_RAW,1);				
				out->player_stamps[idx] = timestamp(PSNET_RELIABLE_TIMEOUT);
				out->num_resends[idx]++;
			}
		}
	}

	// return success
	return bytes_sent;
}

// evaluate the status of a reliable packet - and free it up if the thing is _done_
void psnet_reliable_evaluate_status(reliable_packet_out *out)
{
	int idx,ok;

	// check to see if all players have acked or failed miserably
	ok = 1;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// if the packet is active for this guy but he has not acked yet
		if((out->player_flags & (1<<idx)) && !(out->player_acks & (1<<idx))){
			ok = 0;
			break;
		}
	}

	// if its ok to annihilate this packet, do so
	if(ok){
		memset(out,0,sizeof(reliable_packet_out));
	}
}

// determine if the passed id# exists in the in-packet list, return the instance where it exists, or -1 if it doesn't
int psnet_reliable_find_in_id(ushort id_num)
{
	int idx;

	// if the id is 0, its invalid
	if(id_num == PSNET_RELIABLE_INVALID){
		return -1;
	}

	// look through all in packets
	for(idx=0;idx<PSNET_RELIABLE_NUM_IN_BUFFERS;idx++){
		if(id_num == Psnet_reliable_in[idx]->in_id){
			return idx;
		}
	}

	// couldn't find it 
	return -1;
}

// determine if the passed id# exists in the out-packet list, return the instance where it exists, or -1 if it doesn't
int psnet_reliable_find_out_id(ushort id_num)
{
	int idx;

	// if the id is 0, its invalid
	if(id_num == PSNET_RELIABLE_INVALID){
		return -1;
	}

	// look through all in packets
	for(idx=0;idx<PSNET_RELIABLE_NUM_OUT_BUFFERS;idx++){
		if(id_num == Psnet_reliable_out[idx]->out_id){
			return idx;
		}
	}

	// couldn't find it 
	return -1;
}

// send an ack to the specified address
void psnet_reliable_send_ack(net_addr *addr,ushort id_num)
{
	ubyte data[10];
	ushort val;

	// add in the ack
	val = PSNET_RELIABLE_ACK;
	memcpy(data,&val,sizeof(ushort));

	// add in the id#
	memcpy(data+2,&id_num,sizeof(ushort));

	// send the data
	psnet_send(addr,data,4,PSNET_FLAG_RAW,Reliable_socket);
}

#endif  // #ifndef PSNET2
