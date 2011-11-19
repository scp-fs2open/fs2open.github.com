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
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#define WSAGetLastError()  (errno)
#endif
#include <stdio.h>
#include <limits.h>

#include "globalincs/pstypes.h"
#include "network/psnet2.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multilag.h"
#include "osapi/osregistry.h"
#include "io/timer.h"
#include "network/multi_log.h"
#include "network/multi_rate.h"
#include "cmdline/cmdline.h"

// -------------------------------------------------------------------------------------------------------
// PSNET 2 DEFINES/VARS
//

int		Psnet_my_addr_valid;
net_addr Psnet_my_addr;

ubyte Null_address[6];

int Socket_type;
int Can_broadcast;			// can we do broadcasting on our socket?
int Ipx_can_broadcast = 0;
int Tcp_can_broadcast = 0;

int Tcp_active = 0;
int Ipx_active = 0;

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
#define NETWORK_STATUS_NO_PROTOCOL		3			// TCP/IP doesn't appear to be loaded
#define NETWORK_STATUS_NO_RELIABLE		4
#define NETWORK_STATUS_RUNNING			5			// everything should be running

// defintion of structures that actually leave this machine.  psnet_send give us only
// the data that we want to send.  We will add a header onto this data (packet sequence
// number, possibly a checksum).  We must include a 2 byte flags variable into both structure
// since the receiving end of this packet must know whether or not to checksum the packet.

#define MAX_TOP_LAYER_PACKET_SIZE			680

// use the pack pragma to pack these structures to 2 byte aligment.  Really only needed for
// the naked packet.
#define MAX_PACKET_BUFFERS		75

#pragma pack(push, 2)

/**
 * Definition for a non-checksum packet
 */
typedef struct network_packet
{
	int		sequence_number;
	ushort	flags;
	ubyte		data[MAX_TOP_LAYER_PACKET_SIZE];
} network_naked_packet;

/**
 * Structure definition for our packet buffers
 */
typedef struct network_packet_buffer
{
	int		sequence_number;
	int		len;	
	net_addr	from_addr;
	ubyte		data[MAX_TOP_LAYER_PACKET_SIZE];
} network_packet_buffer;

/**
 * Structure for a bunch of network packet buffers
 */
typedef struct network_packet_buffer_list {
	network_packet_buffer psnet_buffers[MAX_PACKET_BUFFERS];
	int psnet_seq_number;
	int psnet_lowest_id;
	int psnet_highest_id;
} network_packet_buffer_list;

#pragma pack(pop)


#define MAXHOSTNAME			128

#define MAX_RECEIVE_BUFSIZE	4096	// 32 K, eh?
#define MAX_SEND_RETRIES		20			// number of retries when sending would block
#define MAX_LINGER_TIME			0			// in seconds -- when lingering to close a socket

//Reliable UDP stuff
//*******************************
#define MAXNETBUFFERS			150		// Maximum network buffers (For between network and upper level functions, which is 
													// required in case of out of order packets
#define NETRETRYTIME				0.75f		// Time after sending before we resend
#define MIN_NET_RETRYTIME		0.2f
#define NETTIMEOUT				30			// Time after receiving the last packet before we drop that user
#define NETHEARTBEATTIME		3			// How often to send a heartbeat
#define MAXRELIABLESOCKETS		40			// Max reliable sockets to open at once...
#define NETBUFFERSIZE			600		// Max size of a network packet

#define RELIABLE_CONNECT_TIME		7		// how long we'll wait for a response when doing a reliable connect

int Nettimeout = NETTIMEOUT;

// Reliable packet stuff
#define RNT_ACK				1				// ACK Packet
#define RNT_DATA				2				// Data Packet
#define RNT_DATA_COMP		3				// Compressed Data Packet
#define RNT_REQ_CONN			4				// Requesting a connection
#define RNT_DISCONNECT		5				// Disconnecting a connection
#define RNT_HEARTBEAT		6				// Heartbeat -- send every NETHEARTBEATTIME
#define RNT_I_AM_HERE		7

#pragma pack(push, 1)
typedef struct {
	ubyte			type;					// packet type
	ubyte			compressed;			//
	ushort		seq;					// sequence packet 0-65535 used for ACKing also
	ushort		data_len;			// length of data
	float			send_time;			// Time the packet was sent, if an ACK the time the packet being ACK'd was sent.
	ubyte		data[NETBUFFERSIZE];	// Packet data
} reliable_header;

#define RELIABLE_PACKET_HEADER_ONLY_SIZE (sizeof(reliable_header)-NETBUFFERSIZE)
#define MAX_PING_HISTORY	10

typedef struct {
	ubyte buffer[NETBUFFERSIZE];

} reliable_net_sendbuffer;

typedef struct {
	ubyte buffer[NETBUFFERSIZE];
} reliable_net_rcvbuffer;

typedef struct {
	reliable_net_sendbuffer *sbuffers[MAXNETBUFFERS];	// This is an array of pointers for quick sorting
	unsigned short ssequence[MAXNETBUFFERS];				// This is the sequence number of the given packet
	float timesent[MAXNETBUFFERS];
	int send_len[MAXNETBUFFERS];
	reliable_net_rcvbuffer  *rbuffers[MAXNETBUFFERS];
	int recv_len[MAXNETBUFFERS];
	unsigned short rsequence[MAXNETBUFFERS];				// This is the sequence number of the given packet
	float last_packet_received;								// For a given connection, this is the last packet we received
	float last_packet_sent;
	SOCKADDR addr;													// SOCKADDR of our peer
	ushort status;													// Status of this connection
	unsigned short oursequence;								// This is the next sequence number the application is expecting
	unsigned short theirsequence;								// This is the next sequence number the peer is expecting
	net_addr	m_net_addr;											// A FS2 network address structure
	ubyte connection_type;										// IPX, IP, modem, etc.
	float pings[MAX_PING_HISTORY];
	ubyte ping_pos;
	unsigned int num_ping_samples;
	float mean_ping;	
} reliable_socket;

reliable_socket Reliable_sockets[MAXRELIABLESOCKETS];

// sockets for IPX and TCP (unreliable)
SOCKET IPX_socket;
SOCKET TCP_socket;

// the sockets that the game will use when selecting network type
SOCKET Unreliable_socket = INVALID_SOCKET;

float First_sent_iamhere = 0;
float Last_sent_iamhere = 0;

#define CONNECTSEQ 0x142										// Magic number for starting a connection, just so it isn't 0

unsigned int Serverconn = 0xffffffff;

#pragma pack(pop)
//*******************************

// top layer buffers
network_packet_buffer_list Psnet_top_buffers[PSNET_NUM_TYPES];

// -------------------------------------------------------------------------------------------------------
// PSNET 2 FORWARD DECLARATIONS
//

// if the string is a legally formatted ip string
int psnet_is_valid_numeric_ip(char *ip);

#ifdef _WIN32
// functions to get the status of a RAS connection
unsigned int psnet_ras_status();
#endif

// set some options on a socket
void psnet_socket_options( SOCKET sock );

// initialize ipx socket
int psnet_init_ipx();

// initialize tcp socket
int psnet_init_tcp();

// get time in seconds
float psnet_get_time();

// returns the ip address of this machine. use for calling bind() with to associate with the proper
// IP address and network device.
int psnet_get_ip();

// initialize reliable sockets
int psnet_init_rel_tcp(int port, int should_listen);
int psnet_init_rel_ipx(int port, int should_listen);

// shutdown reliable sockets
void psnet_rel_close();

// initialize the buffering system
void psnet_buffer_init(network_packet_buffer_list *l);

// buffer a packet (maintain order!)
void psnet_buffer_packet(network_packet_buffer_list *l, ubyte *data, int length, net_addr *from);

// get the index of the next packet in order!
int psnet_buffer_get_next(network_packet_buffer_list *l, ubyte *data, int *length, net_addr *from);


// -------------------------------------------------------------------------------------------------------
// PSNET 2 TOP LAYER FUNCTIONS - these functions simply buffer and store packets based upon type (see PSNET_TYPE_* defines)
//

/**
 * Wrappers around select() and recvfrom() for lagging/losing data
 */
int RECVFROM(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen, int psnet_type)
{
	network_packet_buffer_list *l;
	net_addr addr;
	int ret;
	int ret_len;

	// bad type
	Assert((psnet_type >= 0) && (psnet_type < PSNET_NUM_TYPES));
	if((psnet_type < 0) || (psnet_type >= PSNET_NUM_TYPES)){
		return -1;
	}	
	l = &Psnet_top_buffers[psnet_type];

	// if we have no buffer! The user should have made sure this wasn't the case by calling SELECT()
	ret = psnet_buffer_get_next(l, (ubyte*)buf, &ret_len, &addr);
	if(!ret){
		Int3();
		return -1;
	}

	// otherwise, stuff the outgoing data
	switch ( Socket_type ) {
#ifdef _WIN32
	case NET_IPX:			
		((SOCKADDR_IPX*)from)->sa_socket = htons(addr.port);
		memcpy(((SOCKADDR_IPX*)from)->sa_nodenum, addr.addr, 6);
		memcpy(((SOCKADDR_IPX*)from)->sa_netnum, addr.net_id, 4);		
		((SOCKADDR_IPX*)from)->sa_family = AF_IPX;		
		*fromlen = sizeof(SOCKADDR_IPX);
		break;
#endif

	case NET_TCP:			
		((SOCKADDR_IN*)from)->sin_port = htons(addr.port);
#ifdef _WIN32
		memcpy(&((SOCKADDR_IN*)from)->sin_addr.S_un.S_addr, addr.addr, 4);		
#else
		memcpy(&((SOCKADDR_IN*)from)->sin_addr.s_addr, addr.addr, 4);		
#endif
		((SOCKADDR_IN*)from)->sin_family = AF_INET;
		*fromlen = sizeof(SOCKADDR_IN);
		break;

	default:
		Assert(0);
		break;
	}

	// return bytes read
	return ret_len;
}

/**
 * Wrappers around select() and recvfrom() for lagging/losing data
 */
int SELECT(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout, int psnet_type)
{
	network_packet_buffer_list *l;

	// if this is a check for writability, just return the select 
	if(writefds != NULL){
		return select(nfds, readfds, writefds, exceptfds, timeout);
	}	
	
	// bad type
	Assert((psnet_type >= 0) && (psnet_type < PSNET_NUM_TYPES));
	if((psnet_type < 0) || (psnet_type >= PSNET_NUM_TYPES)){
		return -1;
	}	
	l = &Psnet_top_buffers[psnet_type];	

	// do we have any buffers in here?	
	if((l->psnet_lowest_id == -1) || (l->psnet_lowest_id > l->psnet_highest_id)){
		return 0;
	}

	// yo
	return 1;
}

/**
 * Wrappers around sendto to sorting through different packet types
 */
int SENDTO(SOCKET s, char * buf, int len, int flags, sockaddr *to, int tolen, int psnet_type)
{	
	char outbuf[MAX_TOP_LAYER_PACKET_SIZE + 150];		

	// stuff type
	outbuf[0] = (char)psnet_type;
	memcpy(&outbuf[1], buf, len);
	
	// send it
	return sendto(s, outbuf, len + 1, flags, (SOCKADDR*)to, tolen);
}

/**
 * Call this once per frame to read everything off of our socket
 */
void PSNET_TOP_LAYER_PROCESS()
{
	// read socket stuff
	SOCKADDR_IN ip_addr;				// UDP/TCP socket structure
#ifdef _WIN32
	SOCKADDR_IPX ipx_addr;			// IPX socket structure
#endif
	fd_set	rfds;
	timeval	timeout;
	int		read_len;
   socklen_t from_len;
	net_addr	from_addr;	
	network_naked_packet packet_read;		

	// clear the addresses to remove compiler warnings
	memset(&ip_addr, 0, sizeof(SOCKADDR_IN));
#ifdef _WIN32
	memset(&ipx_addr, 0, sizeof(SOCKADDR_IPX));
#endif

	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		ml_string("Network ==> socket not inited in PSNET_TOP_LAYER_PROCESS");
		return;
	}

	while ( 1 ) {		
		// check if there is any data on the socket to be read.  The amount of data that can be 
		// atomically read is stored in len.

		FD_ZERO(&rfds);
		FD_SET( Unreliable_socket, &rfds );
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

#ifdef _WIN32
		if ( select( -1, &rfds, NULL, NULL, &timeout) == SOCKET_ERROR ) {
#else
		if ( select( Unreliable_socket + 1, &rfds, NULL, NULL, &timeout) == SOCKET_ERROR ) {
#endif
			ml_printf("Error %d doing a socket select on read", WSAGetLastError());
			break;
		}

		// if the read file descriptor is not set, then bail!
		if ( !FD_ISSET(Unreliable_socket, &rfds) ){
			return;
		}

		// get data off the socket and process
		read_len = SOCKET_ERROR;
		switch ( Socket_type ) {
#ifdef _WIN32
		case NET_IPX:
			from_len = sizeof(SOCKADDR_IPX);			
			read_len = recvfrom( Unreliable_socket, (char*)packet_read.data, MAX_TOP_LAYER_PACKET_SIZE, 0,  (SOCKADDR*)&ipx_addr, &from_len);
			break;
#endif

		case NET_TCP:
			from_len = sizeof(SOCKADDR_IN);			
			read_len = recvfrom( Unreliable_socket, (char*)packet_read.data, MAX_TOP_LAYER_PACKET_SIZE, 0,  (SOCKADDR*)&ip_addr, &from_len);
			break;
		
		default:
			Assert(0);
			return;
		}

		// set the from_addr for storage into the packet buffer structure
		from_addr.type = Socket_type;

		switch ( Socket_type ) {
#ifdef _WIN32
		case NET_IPX:			
			from_addr.port = ntohs( ipx_addr.sa_socket );			
			memcpy(from_addr.addr, ipx_addr.sa_nodenum, 6 );
			memcpy(from_addr.net_id, ipx_addr.sa_netnum, 4 );
			break;
#endif

		case NET_TCP:			
			from_addr.port = ntohs( ip_addr.sin_port );			
			memset(from_addr.addr, 0x00, 6);
#ifdef _WIN32
			memcpy(from_addr.addr, &ip_addr.sin_addr.S_un.S_addr, 4); //-V512
#else
			memcpy(from_addr.addr, &ip_addr.sin_addr.s_addr, 4); //-V512
#endif
			break;

		default:
			Assert(0);
			return;
			// break;
		}

		if ( read_len == SOCKET_ERROR ) {
			ml_string("Socket error on socket_get_data()");
			break;
		}		

		// determine the packet type
		int packet_type = packet_read.data[0];		
		if((packet_type < 0) || (packet_type >= PSNET_NUM_TYPES)){
			Int3();
		} else {		
			// buffer the packet
			psnet_buffer_packet(&Psnet_top_buffers[packet_type], packet_read.data + 1, read_len - 1, &from_addr);
		}
	}
}


// -------------------------------------------------------------------------------------------------------
// PSNET 2 FUNCTIONS
//

/**
 * Initialize psnet to use the specified port
 */
void psnet_init( int protocol, int port_num )
{	
	int idx;
	Tcp_active = 0;
#ifdef _WIN32
	char *internet_connection;
	WSADATA wsa_data; 		
	Ipx_active = 0;
#endif

	// GAME PORT INITIALIZATION STUFF
	if ( Network_status == NETWORK_STATUS_RUNNING ){
		ml_string("Skipping psnet_init() because network already running");
		return;
	}

// sort of a hack; assume unix users are always on LAN :)
#ifdef _WIN32
	internet_connection = os_config_read_string(NULL, "NetworkConnection", "none");
	if ( !stricmp(internet_connection, NOX("dialup")) ) {
		ml_string("psnet_init() detected dialup connection");

		Psnet_connection = NETWORK_CONNECTION_DIALUP;
	} else if ( !stricmp(internet_connection, NOX("lan")) ) {
		ml_string("psnet_init() detected lan connection");

		Psnet_connection = NETWORK_CONNECTION_LAN;
	} else {
		ml_string("psnet_init() detected no connection");

		Psnet_connection = NETWORK_CONNECTION_NONE;
	}
#else
	Psnet_connection = NETWORK_CONNECTION_LAN;
#endif

	Network_status = NETWORK_STATUS_NO_WINSOCK;
#ifdef _WIN32
	if (WSAStartup(0x101, &wsa_data )){
		return;
	}
#endif

	// get the port for running this game on.  Be careful that it cannot be out of bounds
	Psnet_default_port = DEFAULT_GAME_PORT;
	if ( (port_num > 1023) && (port_num < USHRT_MAX) ) {
		Psnet_default_port = (ushort)port_num;
	}

	// initialize TCP now	
	Tcp_active = 1;
	if(!psnet_init_tcp()){
		ml_printf("Error on TCP startup %d", Tcp_failure_code);		

		Tcp_active = 0;
	} else {
		if(!psnet_init_rel_tcp(Psnet_default_port + 1, 0)){
			ml_printf("Error on TCP startup %d", Tcp_failure_code);			

			Tcp_active = 0;
		}
	}

	// clear reliable sockets
	reliable_socket *rsocket;
	int j;	
	for(j=0; j<MAXRELIABLESOCKETS; j++){
		rsocket=&Reliable_sockets[j];
		memset(rsocket,0,sizeof(reliable_socket));
	}

#ifdef _WIN32
	// initialize IPX not
	Ipx_active = 1;
	if(!psnet_init_ipx()){
		ml_printf("Error on IPX startup %d", Ipx_failure_code);		

		Ipx_active = 0;
	} else {
		if(!psnet_init_rel_ipx(Psnet_default_port + 1, 0)){
			ml_printf("Error on IPX startup %d", Ipx_failure_code);			

			Ipx_active = 0;
		}
	}
#endif

	// determine if we've successfully initialized the protocol we want
#ifdef _WIN32
	if(!Tcp_active && !Ipx_active){	
#else
	if(!Tcp_active){	
#endif
		Network_status = NETWORK_STATUS_NO_PROTOCOL;		

		ml_string("No protocol in psnet_init()!");
	}

	// specified network timeout	
	Nettimeout = NETTIMEOUT;
	if(Cmdline_timeout > 0){
		Nettimeout = Cmdline_timeout;
	}

#ifdef _WIN32
	// set ras status
	psnet_ras_status();	
#endif

	if(Network_status != NETWORK_STATUS_NO_PROTOCOL){			
		// set network to be running
		Network_status = NETWORK_STATUS_RUNNING;	
	
		// determine if our socket can broadcast
#ifdef _WIN32
		Can_broadcast = Tcp_can_broadcast || Ipx_can_broadcast;
#else
		Can_broadcast = Tcp_can_broadcast;
#endif
	
		// initialize all packet type buffers
		for(idx=0; idx<PSNET_NUM_TYPES; idx++){
			psnet_buffer_init(&Psnet_top_buffers[idx]);
		}
	}
}

/**
 * Shutdown psnet
 */
void psnet_close()
{
	if ( Network_status != NETWORK_STATUS_RUNNING ){
		return;
	}

#ifdef _WIN32
	WSACancelBlockingCall();		

	if ( TCP_socket != INVALID_SOCKET ) {
		shutdown( TCP_socket, 1 );
		closesocket( TCP_socket );
	}

	if ( IPX_socket != INVALID_SOCKET ) {
		shutdown( IPX_socket, 1 );
		closesocket( IPX_socket );
	}

	if (WSACleanup())	{
	}
#else
	if ( TCP_socket != (int)INVALID_SOCKET ) {
		shutdown( TCP_socket, 1 );
		close( TCP_socket );
	}
#endif

	// close down all reliable sockets - this forces them to
	// send a disconnect to any remote machines	
	psnet_rel_close();
	
	Network_status = NETWORK_STATUS_NOT_INITIALIZED;
}

/**
 * Set the protocol to use
 */
int psnet_use_protocol( int protocol )
{
   socklen_t len;
#ifdef _WIN32
	SOCKADDR_IPX	ipx_addr;
#endif
	SOCKADDR_IN		ip_addr;
	char *custom_ip = NULL;

	// zero out my address
	Psnet_my_addr_valid = 0;
	memset( &Psnet_my_addr, 0, sizeof(Psnet_my_addr) );

	// wait until we choose a protocol to determine if we can broadcast
	Can_broadcast = 0;

	ml_string("In psnet_use_protocol()");

	switch ( protocol ) {
#ifdef _WIN32
	case NET_IPX:		
		ml_string("psnet_use_protocol() selecting IPX");

		// assign the IPX_* sockets to the socket values used elsewhere
		Unreliable_socket = IPX_socket;		

		Can_broadcast = Ipx_can_broadcast;
		if(Can_broadcast){
			ml_string("Psnet : IPX broadcast");
		}

		// get the socket name for the IPX_socket, and put it into My_addr
		len = sizeof(SOCKADDR_IPX);
		if ( getsockname(IPX_socket, (SOCKADDR *)&ipx_addr, &len) == SOCKET_ERROR ) {
			ml_printf("Unable to get sock name for IPX unreliable socket (%d)", WSAGetLastError() );
			return 0;
		}

		memcpy(Psnet_my_addr.net_id, ipx_addr.sa_netnum, 4);
		memcpy(Psnet_my_addr.addr, ipx_addr.sa_nodenum, 6);
		Psnet_my_addr.port = Psnet_default_port;

		ml_string("Psnet using - NET_IPX");
		break;
#endif

	case NET_TCP:
		if ( Network_status != NETWORK_STATUS_RUNNING ){
			ml_string("Network_status != NETWORK_STATUS_RUNNING in NET_TCP in psnet_use_protocol()");
			return 0;
		}

		// assign the TCP_* sockets to the socket values used elsewhere
		Unreliable_socket = TCP_socket;		

		Can_broadcast = Tcp_can_broadcast;
		if(Can_broadcast){
			ml_string("Psnet : TCP broadcast");
		}

		// get the socket name for the TCP_socket, and put it into My_addr
		len = sizeof(SOCKADDR_IN);
		if ( getsockname(TCP_socket, (SOCKADDR *)&ip_addr, &len) == SOCKET_ERROR ) {
			ml_printf("Unable to get sock name for TCP unreliable socket (%d)", WSAGetLastError() );			
			return 0;
		}

		// check user-specified IP for getting around NAT
		custom_ip = os_config_read_string( NOX("Network"), NOX("CustomIP"), NULL );

		if (custom_ip != NULL) {
			SOCKADDR_IN custom_address;

#ifndef WIN32
			if ( inet_aton(custom_ip, &custom_address.sin_addr) ) {
#else
			if ( (custom_address.sin_addr.s_addr = inet_addr(custom_ip)) != INADDR_NONE ) {
#endif
				memcpy(&ip_addr.sin_addr, &custom_address.sin_addr, sizeof(custom_address.sin_addr));
			} else {
				ml_printf("WARNING  =>  psnet_get_ip() custom IP is invalid: %s", custom_ip);
			}
		}

		memset(Psnet_my_addr.net_id, 0, 4);
		memcpy(Psnet_my_addr.addr, &ip_addr.sin_addr, 6);
		Psnet_my_addr.port = Psnet_default_port;

		ml_string("Psnet using - NET_TCP");
		break;

	default:
		Int3();
		return 0;
	}

	Psnet_my_addr.type = protocol;
	Socket_type = protocol;

	return 1;
}

/**
 * Get the status of the network
 */
int psnet_get_network_status()
{
	// first case is when "none" is selected
	if ( Psnet_connection == NETWORK_CONNECTION_NONE ) {
		return NETWORK_ERROR_NO_TYPE;
	}

	// first, check the connection status of the network
	if ( Network_status == NETWORK_STATUS_NO_WINSOCK )
		return NETWORK_ERROR_NO_WINSOCK;

	if ( Network_status == NETWORK_STATUS_NO_PROTOCOL ){
		return NETWORK_ERROR_NO_PROTOCOL;
	}
	
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

/**
 * Convert a ::net_addr to a string
 */
char* psnet_addr_to_string( char * text, net_addr * address )
{

	if ( Network_status != NETWORK_STATUS_RUNNING )		{
		strcpy( text, XSTR("[no networking]",910) );
		return text;
	}

	in_addr temp_addr;

	switch ( address->type ) {
#ifdef _WIN32
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
#endif

		case NET_TCP:
			memcpy(&temp_addr.s_addr, address->addr, 4);
			strcpy( text, inet_ntoa(temp_addr) );
			break;

		default:
			break;

	} // end switch
	
	return text;
}

/**
 * Convert a string to a net addr
 */
void psnet_string_to_addr( net_addr * address, char * text )
{
	struct hostent *he;
	char str[255], *c, *port;
	in_addr addr;

	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		strcpy( text, XSTR("[no networking]",910) );
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
#ifdef _WIN32
		case NET_IPX:	      
			Int3();		// no support for this yet
			break;
#endif

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
			memcpy(address->addr, &addr.s_addr, 4); //-V512
			if ( port ){
				address->port = (ushort)(atoi(port));
			}
			break;

		default:
			Assert(0);
			break;

	} // end switch
}

/**
 * Compare 2 addresses
 */
int psnet_same( net_addr * a1, net_addr * a2 )
{
	return !memcmp(a1->addr, a2->addr, 6);		
}

/**
 * Send data unreliably
 */
int psnet_send( net_addr * who_to, void * data, int len, int np_index )
{
	// send data unreliably
	SOCKET send_sock;
	SOCKADDR_IN sockaddr;				// UDP/TCP socket structure
#ifdef _WIN32
	SOCKADDR_IPX ipx_addr;				// IPX socket structure
#endif
	int ret, send_len;
	ubyte iaddr[6], *send_data;
	short port;
	fd_set	wfds;
	struct timeval timeout;	

	// always use the unreliable socket
	send_sock = Unreliable_socket;		

	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		ml_string("Network ==> Socket not inited in psnet_send");
		return 0;
	}

	if ( psnet_same( who_to, &Psnet_my_addr) ){
		return 0;
	}

	memset(iaddr, 0x00, 6);
	memcpy(iaddr, who_to->addr, 6);

	if ( memcmp(iaddr, Null_address, 6) == 0) {
		ml_string("Network ==> send to address is 0 in psnet_send");
		return 0;
	}
	
	port = who_to->port;
		
	if ( port == 0) {
		ml_printf("Network ==> destination port %d invalid in psnet_send", port);
		return 0;
	}

	// stuff the data with the type	
	send_data = (ubyte*)data;
	send_len = len;

	FD_ZERO(&wfds);
	FD_SET( send_sock, &wfds );
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

#ifdef _WIN32
	if ( SELECT( -1, NULL, &wfds, NULL, &timeout, PSNET_TYPE_UNRELIABLE) == SOCKET_ERROR ) {
#else
	if ( SELECT( send_sock+1, NULL, &wfds, NULL, &timeout, PSNET_TYPE_UNRELIABLE) == SOCKET_ERROR ) {
#endif
		ml_printf("Error on blocking select for write %d", WSAGetLastError() );
		return 0;
	}

	// if the write file descriptor is not set, then bail!
	if ( !FD_ISSET(send_sock, &wfds ) ){
		return 0;
	}

	ret = SOCKET_ERROR;
	switch ( who_to->type ) {
#ifdef _WIN32
		case NET_IPX:
			ipx_addr.sa_socket = htons(port);
			ipx_addr.sa_family = AF_IPX;
			memcpy(ipx_addr.sa_nodenum, iaddr, 6);
			memcpy(ipx_addr.sa_netnum, who_to->net_id, 4);
				
			ret = SENDTO(send_sock, (char *)send_data, send_len, 0, (SOCKADDR*)&ipx_addr, sizeof(ipx_addr), PSNET_TYPE_UNRELIABLE);

			multi_rate_add(np_index, "ipx", send_len);
			if ( (ret != SOCKET_ERROR) && (ret != send_len) ){
			}
			break;
#endif

		case NET_TCP:
			sockaddr.sin_family = AF_INET; 
			memcpy(&sockaddr.sin_addr.s_addr, iaddr, 4);
			sockaddr.sin_port = htons(port); 

			multi_rate_add(np_index, "udp(h)", send_len + UDP_HEADER_SIZE);
			multi_rate_add(np_index, "udp", send_len);
			ret = SENDTO( send_sock, (char *)send_data, send_len, 0, (SOCKADDR*)&sockaddr, sizeof(sockaddr), PSNET_TYPE_UNRELIABLE );
			break;

		default:
			Assert(0);	// unknown protocol
			break;

	} // end switch

	if ( ret != SOCKET_ERROR )	{
		return 1;
	}
	return 0;
}

/**
 * Get data from the unreliable socket
 */
int psnet_get( void * data, net_addr * from_addr )
{					
	int buffer_size;

	// try and get a free buffer and return its size
	if(psnet_buffer_get_next(&Psnet_top_buffers[PSNET_TYPE_UNRELIABLE], (ubyte*)data, &buffer_size, from_addr)){
		return buffer_size;
	}

	// return nothing
	return 0;
}

/**
 * Broadcast data on unreliable socket
 */
int psnet_broadcast( net_addr * who_to, void * data, int len )
{
	if ( Network_status != NETWORK_STATUS_RUNNING ) {
		ml_string("Network ==> Socket not inited in psnet_broadcast");
		return 0;
	}

	if ( !Can_broadcast ) {
		ml_string("Cannot broadcast -- returning without doing anything");
		return 0;
	}

	ubyte broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	// broadcasting works on a local subnet which is all we really want to do for now anyway.
	// we might keep this in as an option for freespace later.
	switch ( who_to->type ) {
		case NET_IPX:
		case NET_TCP:
			memcpy(who_to->addr, broadcast, 6);
			psnet_send(who_to, data, len);
			break;
	
	} // end switch

	return 1;
}

/**
 * Flush all sockets
 */
void psnet_flush()
{
	ubyte data[MAX_TOP_LAYER_PACKET_SIZE + 250];
	net_addr from_addr;

	while ( psnet_get( data, &from_addr ) > 0 ) ;
}

/**
 * If the passed string is a valid IP string
 */
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


// -------------------------------------------------------------------------------------------------------
// PSNET 2 RELIABLE SOCKET FUNCTIONS
//

/**
 * Compare 2 pings
 */
int psnet_rel_ping_compare( const void *arg1, const void *arg2 )
{
	float *ping1 = (float *)arg1;
	float *ping2 = (float *)arg2;
	
	if(*ping1==*ping2) return 0;
	else if(*ping1>*ping2) return 1;
	else if(*ping1<*ping2) return -1;

	return 0;
}

void psnet_rel_send_ack(SOCKADDR *raddr, unsigned int sig, ubyte link_type, float time_sent)
{
	int ret, sig_tmp;
	reliable_header ack_header;
	ack_header.type = RNT_ACK;	
	ack_header.data_len = sizeof(unsigned int);
	ack_header.send_time = time_sent;
	ack_header.send_time = INTEL_FLOAT(&ack_header.send_time);
	sig_tmp = INTEL_INT(sig);
	memcpy(&ack_header.data,&sig_tmp,sizeof(unsigned int));
	switch (link_type) {
#ifdef _WIN32
	case NET_IPX:
		if(!Ipx_active){
			ml_string("No IPX in rel_send_ack()");
			return;
		}		
		ret = SENDTO(Unreliable_socket, (char *)&ack_header, RELIABLE_PACKET_HEADER_ONLY_SIZE+ack_header.data_len, 0, raddr, sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
		break;
#endif
	case NET_TCP:
		if(!Tcp_active){
			ml_string("No TCP in rel_send_ack()");
			return;
		}
		ret = SENDTO(Unreliable_socket, (char *)&ack_header, RELIABLE_PACKET_HEADER_ONLY_SIZE+ack_header.data_len, 0, raddr, sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
		break;
	default:		
		ml_string("Unknown protocol type in nw_SendReliable()");
		break;
	}	
}

/**
 * Function to shutdown and close the given socket.
 *
 * It takes a couple of things into consideration when closing, such as possibly 
 * reiniting reliable sockets if they are closed here.
 */
void psnet_rel_close_socket( PSNET_SOCKET_RELIABLE *sockp )
{
	reliable_header diss_conn_header;

	// if the socket is out of range
	if (*sockp >= MAXRELIABLESOCKETS) {
		if (*sockp != INVALID_SOCKET) {
			ml_printf("Invalid socket id passed to nw_NewCloseSocket() -- %d",*sockp);
		}

		return;
	}	

	ml_printf("Closing socket %d",*sockp);
	
	// go through every buffer and "free it up(tm)"
	int i;
	for(i=0;i<MAXNETBUFFERS;i++){
		if(Reliable_sockets[*sockp].rbuffers[i]){
			if(Reliable_sockets[*sockp].rbuffers[i] != NULL){
				vm_free(Reliable_sockets[*sockp].rbuffers[i]);
			}
			Reliable_sockets[*sockp].rbuffers[i] = NULL;
			Reliable_sockets[*sockp].rsequence[i] = 0;
		}
		if(Reliable_sockets[*sockp].sbuffers[i]){
			if(Reliable_sockets[*sockp].sbuffers[i] != NULL){
				vm_free(Reliable_sockets[*sockp].sbuffers[i]);
			}
			Reliable_sockets[*sockp].sbuffers[i] = NULL;
			Reliable_sockets[*sockp].rsequence[i] = 0;
		}
	}

	// send a disconnect packet to the socket on the other end
	diss_conn_header.type = RNT_DISCONNECT;
	diss_conn_header.seq = CONNECTSEQ;
	diss_conn_header.data_len = 0;
	if(*sockp==Serverconn){
		Serverconn = 0xffffffff;
	}
	switch ( Reliable_sockets[*sockp].connection_type ) {
#ifdef _WIN32
		case NET_IPX:
			if(!Ipx_active){
				 return;
			}
			SENDTO(Unreliable_socket, (char *)&diss_conn_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,&Reliable_sockets[*sockp].addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
			break;
#endif
		case NET_TCP:
			if(!Tcp_active){
				return;
			}
			SENDTO(Unreliable_socket, (char *)&diss_conn_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,&Reliable_sockets[*sockp].addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
			break;
		default:
			ml_string("Unknown protocol type in nw_CloseSocket()!");			
			break;
	}
	memset(&Reliable_sockets[*sockp],0,sizeof(reliable_socket));
	Reliable_sockets[*sockp].status = RNF_UNUSED;	
}

// function to check the status of the reliable socket and try to re-initialize it if necessary.
// win95 seems to have trouble doing a reinit of the socket immediately after close, so this
// function exists to check the status, and reinitialize if we need to
int psnet_rel_check()
{
	return 1;
}

/**
 * Send data reliably
 */
int psnet_rel_send(PSNET_SOCKET_RELIABLE socketid, ubyte *data, int length, int np_index)
{		
	int i;
	int bytesout = 0;
	reliable_socket *rsocket;	
	
	if(socketid >= MAXRELIABLESOCKETS){
		ml_printf("Invalid socket id passed to psnet_rel_send() -- %d",socketid);
		return -1;
	}

	Assert(length < (int)sizeof(reliable_header));
	psnet_rel_work();

	rsocket=&Reliable_sockets[socketid];
	if(rsocket->status!=RNF_CONNECTED) {
		//We can't send because this isn't a connected reliable socket.
		ml_printf("Can't send packet because of status %d in nw_SendReliable(). socket = %d",rsocket->status,socketid);
		return -1;
	}
	
	// Add the new packet to the sending list and send it.
	for(i=0;i<MAXNETBUFFERS;i++){
		if(NULL==rsocket->sbuffers[i]){			
			reliable_header send_header;
			int send_this_packet=1;			
			
			rsocket->send_len[i] = length;
			rsocket->sbuffers[i] = (reliable_net_sendbuffer *)vm_malloc(sizeof(reliable_net_sendbuffer));
		
			memcpy(rsocket->sbuffers[i]->buffer,data,length);	

			send_header.seq = INTEL_SHORT( rsocket->theirsequence );
			rsocket->ssequence[i] = rsocket->theirsequence;
			
			memcpy(send_header.data,data,length);
			send_header.data_len = INTEL_SHORT( (ushort)length );
			send_header.type = RNT_DATA;
			send_header.send_time = psnet_get_time();
			send_header.send_time = INTEL_FLOAT( &send_header.send_time ) ;
					
			if (send_this_packet){
				switch ( rsocket->connection_type ){
#ifdef _WIN32
					case NET_IPX:
						if(!Ipx_active){
							return 0;
						}
						bytesout = SENDTO(Unreliable_socket, (char *)&send_header,RELIABLE_PACKET_HEADER_ONLY_SIZE+rsocket->send_len[i],0,&rsocket->addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
						break;
#endif
					case NET_TCP:
						if(!Tcp_active){
							return 0;
						}
						multi_rate_add(np_index, "tcp(h)", RELIABLE_PACKET_HEADER_ONLY_SIZE+rsocket->send_len[i]);
						bytesout = SENDTO(Unreliable_socket, (char *)&send_header,RELIABLE_PACKET_HEADER_ONLY_SIZE+rsocket->send_len[i],0,&rsocket->addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
						break;
					default:
						ml_string("Unknown protocol type in nw_SendReliable()!");
						Int3();
						break;
				}		
			}

			if((bytesout==SOCKET_ERROR)&&(WSAEWOULDBLOCK==WSAGetLastError())){
				//This will cause it to try to send again next frame. (or sooner)
				rsocket->timesent[i] = psnet_get_time()-(NETRETRYTIME*4);
			} else {
				rsocket->timesent[i] = psnet_get_time();
			}
			
						
			rsocket->theirsequence++;
			return bytesout;
		}
	}
	ml_printf("PSNET RELIABLE SEND BUFFER OVERRUN. socket = %d",socketid);	
	return 0;
}

// Return codes:
// -1 socket not connected
// 0 No packet ready to receive
// >0 Buffer filled with the number of bytes recieved
int psnet_rel_get(PSNET_SOCKET socketid, ubyte *buffer, int max_len)
{	
	int i;
	
	reliable_socket *rsocket = NULL;
	psnet_rel_work();
	if(socketid >= MAXRELIABLESOCKETS){
		ml_printf("Invalid socket id passed to nw_NewReceiveReliable() -- %d",socketid);
		return -1;
	}
	rsocket = &Reliable_sockets[socketid];
	if( (RNF_CONNECTED!=rsocket->status) && (RNF_LIMBO!=rsocket->status) ){
		ml_printf("Can't receive packet because it isn't connected in nw_ReceiveReliable(). socket = %d",socketid);
		return 0;
	}
	//If the buffer position is the position we are waiting for, fill in 
	//the buffer we received in the call to this function and return true			

	for(i=0; i<MAXNETBUFFERS; i++){
		if((rsocket->rsequence[i] == rsocket->oursequence) && rsocket->rbuffers[i]){
			memcpy(buffer,rsocket->rbuffers[i]->buffer, rsocket->recv_len[i]);
			vm_free(rsocket->rbuffers[i]);
			rsocket->rbuffers[i] = NULL;
			rsocket->rsequence[i] = 0;			
			rsocket->oursequence++;
			return rsocket->recv_len[i];
		}
	}

	return 0;	
}

/**
 * Process all active reliable sockets
 */
void psnet_rel_work()
{
	int i,j;
	int rcode = -1;
	int max_len = NETBUFFERSIZE;
	fd_set read_fds;	           
	TIMEVAL timeout; 
	static reliable_header rcv_buff;
	static SOCKADDR rcv_addr;
	int bytesin = 0;
	int addrlen = sizeof(SOCKADDR);
	unsigned int rcvid;//The id of who we actually received a packet from, as opposed to socketid parm
	timeout.tv_sec=0;            
	timeout.tv_usec=0;

	PSNET_TOP_LAYER_PROCESS();
		
	// negotitate initial connection with the server
	reliable_socket *rsocket = NULL;
	if(Serverconn != 0xffffffff){
		//Check to see if we need to send a packet out.
		if((Reliable_sockets[Serverconn].status==RNF_LIMBO) && ((Serverconn != 0xffffffff) && fl_abs((psnet_get_time() - Last_sent_iamhere))>NETRETRYTIME) ){
			reliable_header conn_header;
			//Now send I_AM_HERE packet
			conn_header.type = RNT_I_AM_HERE;
			conn_header.seq = (ushort)(~CONNECTSEQ);
			conn_header.data_len = 0;
			Last_sent_iamhere = psnet_get_time();
			int ret = SOCKET_ERROR;
			switch ( Reliable_sockets[Serverconn].connection_type ) {
#ifdef _WIN32
			case NET_IPX:
				if(!Ipx_active){
					ml_string("Unable to use this network connection type in nw_WorkReliable()");
					Int3();
					return;
				}
				ret = SENDTO(Unreliable_socket, (char *)&conn_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,&Reliable_sockets[Serverconn].addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
				break;
#endif
			case NET_TCP:
				if(!Tcp_active){
					ml_string("Unable to use this network connection type in nw_WorkReliable()");
					Int3();
					return;
				}
				ret = SENDTO(Unreliable_socket, (char *)&conn_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,&Reliable_sockets[Serverconn].addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
				break;
			default:
				ml_string("Unknown protocol type in nw_WorkReliable()!");
				Int3();
				break;
			}

			if((ret == SOCKET_ERROR) && (WSAEWOULDBLOCK == WSAGetLastError())){
				Reliable_sockets[Serverconn].last_packet_sent = psnet_get_time()-NETRETRYTIME;
			} else {
				Reliable_sockets[Serverconn].last_packet_sent = psnet_get_time();
			}
		}
	}

	ubyte link_type;
	net_addr d3_rcv_addr;
	SOCKADDR_IN *rcvaddr,*rsockaddr;
#ifdef _WIN32
	int ipx_has_data = 0;
#endif
	int udp_has_data = 0;
	do {		
		rsocket = NULL;
		//Check IPX and UDP
#ifdef _WIN32
		if(Ipx_active && (Socket_type == NET_IPX)){
			FD_ZERO(&read_fds);
			FD_SET(Unreliable_socket, &read_fds);    
			ipx_has_data = SELECT(0,&read_fds,NULL,NULL,&timeout, PSNET_TYPE_RELIABLE);
		}
#endif
		if(Tcp_active && (Socket_type == NET_TCP)){
			FD_ZERO(&read_fds);
			FD_SET(Unreliable_socket, &read_fds);    
#ifdef _WIN32
			udp_has_data = SELECT(0,&read_fds,NULL,NULL,&timeout, PSNET_TYPE_RELIABLE);
#else
			udp_has_data = SELECT(Unreliable_socket+1, &read_fds,NULL,NULL,&timeout, PSNET_TYPE_RELIABLE);
#endif
		}
		bytesin = 0;
		addrlen = sizeof(SOCKADDR);
#ifdef _WIN32
		if(ipx_has_data){
			SOCKADDR_IPX *ipx_addr = (SOCKADDR_IPX *)&rcv_addr;
			memset(&d3_rcv_addr,0,sizeof(net_addr));
			memset(&rcv_addr,0,sizeof(SOCKADDR));
			bytesin = RECVFROM(Unreliable_socket, (char *)&rcv_buff,sizeof(reliable_header),0,(SOCKADDR *)&rcv_addr,&addrlen, PSNET_TYPE_RELIABLE);
			memcpy(d3_rcv_addr.addr, ipx_addr->sa_nodenum, 6);
			memcpy(d3_rcv_addr.net_id, ipx_addr->sa_netnum, 4);				
			d3_rcv_addr.type = NET_IPX;
			link_type = NET_IPX;
		} else
#endif
      if(udp_has_data){
			SOCKADDR_IN *tcp_addr = (SOCKADDR_IN *)&rcv_addr;
			memset(&d3_rcv_addr,0,sizeof(net_addr));
			memset(&rcv_addr,0,sizeof(SOCKADDR));
			bytesin = RECVFROM(Unreliable_socket, (char *)&rcv_buff,sizeof(reliable_header), 0, (SOCKADDR *)&rcv_addr,&addrlen, PSNET_TYPE_RELIABLE);
			rcv_buff.seq = INTEL_SHORT( rcv_buff.seq ); //-V570
			rcv_buff.data_len = INTEL_SHORT( rcv_buff.data_len ); //-V570
			rcv_buff.send_time = INTEL_FLOAT( &rcv_buff.send_time );
			memcpy(d3_rcv_addr.addr, &tcp_addr->sin_addr.s_addr, 4); //-V512
			d3_rcv_addr.port = tcp_addr->sin_port;
			d3_rcv_addr.type = NET_TCP;
			link_type = NET_TCP;
		} else {
			//Neither socket had data waiting
			break;
		}		

		if(bytesin==-1){
			ml_printf("recvfrom returned an error! -- %d",WSAGetLastError());			
			return;
		}
		if(bytesin){
			//Someone wants to connect, so find a slot
			if(rcv_buff.type == RNT_REQ_CONN){
				for(i=1; i<MAXRELIABLESOCKETS; i++){
					if( (Reliable_sockets[i].status == RNF_CONNECTED) || (Reliable_sockets[i].status == RNF_LIMBO) ){
						if(memcmp(&d3_rcv_addr, &Reliable_sockets[i].m_net_addr, sizeof(net_addr)) == 0){
							//We already have a reliable link to this user, so we will ignore it...
							ml_printf("Received duplicate connection request. %d",i);
							psnet_rel_send_ack(&Reliable_sockets[i].addr, rcv_buff.seq, link_type, rcv_buff.send_time);
							//We will change this as a hack to prevent later code from hooking us up
							rcv_buff.type = 0xff;
							continue;
						}
					}
				}
				for(i=1; i<MAXRELIABLESOCKETS; i++){
					if(Reliable_sockets[i].status == RNF_UNUSED){
						//Add the new connection here.
						Reliable_sockets[i].connection_type=link_type;
						memcpy(&Reliable_sockets[i].m_net_addr, &d3_rcv_addr, sizeof(net_addr));
						memcpy(&Reliable_sockets[i].addr ,&rcv_addr, sizeof(SOCKADDR));
						Reliable_sockets[i].ping_pos = 0;
						Reliable_sockets[i].num_ping_samples = 0;
						Reliable_sockets[i].status = RNF_LIMBO;
						Reliable_sockets[i].last_packet_received = psnet_get_time();
						rsocket = &Reliable_sockets[i];
						rcvaddr = (SOCKADDR_IN *)&rcv_addr;
						ml_printf("Connect from %s:%d", inet_ntoa(rcvaddr->sin_addr), htons(rcvaddr->sin_port));
						break;
					}
				}
				if(i==MAXRELIABLESOCKETS){
					//No more connections!
					ml_string("Out of incoming reliable connection sockets");
					continue;
				}
				psnet_rel_send_ack(&rsocket->addr, rcv_buff.seq, link_type, rcv_buff.send_time);			
			}
			
			//Find out if this is a packet from someone we were expecting a packet.
			rcvaddr = (SOCKADDR_IN *)&rcv_addr;
			for(i=1; i<MAXRELIABLESOCKETS; i++){
				rsockaddr = (SOCKADDR_IN *)&Reliable_sockets[i].addr;
				if(memcmp(&d3_rcv_addr,&Reliable_sockets[i].m_net_addr,sizeof(net_addr)) == 0){
					rsocket=&Reliable_sockets[i];
					rcvid = i;
					break;
				}				
			}
			if(NULL == rsocket){
				ml_string("Received reliable data from unconnected client.");
				ml_printf("Received from %s:%d\n",inet_ntoa(rcvaddr->sin_addr),rcvaddr->sin_port);
				continue ;
			}
			rsocket->last_packet_received = psnet_get_time();
			
			if(rsocket->status != RNF_CONNECTED){
				//Get out of limbo
				if(rsocket->status == RNF_LIMBO){
					//this is our connection to the server
					if(Serverconn != 0xffffffff){
						if(rcv_buff.type == RNT_ACK){
							ushort *acknum = (ushort *)&rcv_buff.data;
							if(*acknum == (~CONNECTSEQ & 0xffff)){
								rsocket->status = RNF_CONNECTED;
								ml_string("Got ACK for IAMHERE!");
							}
							continue;
						}
					} else if(rcv_buff.type == RNT_I_AM_HERE){
						rsocket->status = RNF_CONNECTING;
						psnet_rel_send_ack(&rsocket->addr, rcv_buff.seq, link_type, rcv_buff.send_time);		
						ml_string("Got IAMHERE!");
						continue;
					}
				}
				if((rcv_buff.type == RNT_DATA) && (Serverconn != 0xffffffff)){
					rsocket->status = RNF_CONNECTED;
				} else {					
					rsocket->last_packet_received = psnet_get_time();
					continue;
				}				
			}
			//Update the last recv variable so we don't need a heartbeat
			rsocket->last_packet_received = psnet_get_time();

			if(rcv_buff.type == RNT_HEARTBEAT){
				continue;
			}
			if(rcv_buff.type == RNT_ACK){
				//Update ping time
				rsocket->num_ping_samples++;
				
				rsocket->pings[rsocket->ping_pos] = rsocket->last_packet_received - rcv_buff.send_time;				
				if(rsocket->num_ping_samples >= MAX_PING_HISTORY){
					float sort_ping[MAX_PING_HISTORY];
					for(int a=0;a<MAX_PING_HISTORY;a++){
						sort_ping[a] = rsocket->pings[a];
					}

					qsort(sort_ping ,MAX_PING_HISTORY, sizeof(float), psnet_rel_ping_compare);
					rsocket->mean_ping = ((sort_ping[MAX_PING_HISTORY/2]+sort_ping[(MAX_PING_HISTORY/2)+1]))/2;					
				}
				rsocket->ping_pos++;
				if(rsocket->ping_pos >= MAX_PING_HISTORY){
					rsocket->ping_pos=0;				
				}

				// if this is an ack for a send buffer on the socket, kill the send buffer. its done
				for(i=0; i<MAXNETBUFFERS; i++){
					unsigned int *acksig = (unsigned int *)&rcv_buff.data;
					if(rsocket){
						if(rsocket->sbuffers[i]){
							if(rsocket->ssequence[i] == INTEL_INT(*acksig)){								
								Assert(rsocket->sbuffers[i] != NULL);
								vm_free(rsocket->sbuffers[i]);
								rsocket->sbuffers[i] = NULL;	
								rsocket->ssequence[i] = 0;
							}
						}
					}
				}
				//remove that packet from the send buffer
				rsocket->last_packet_received = psnet_get_time();
				continue;
			}

			if(rcv_buff.type == RNT_DATA_COMP){
				//More2Come
				//Decompress it. Put it back in the buffer. Process it as RNT_DATA
				rcv_buff.type = RNT_DATA;
			}
			if(rcv_buff.type == RNT_DATA){				
				//If the data is out of order by >= MAXNETBUFFERS-1 ignore that packet for now
				int seqdelta;
				seqdelta = rcv_buff.seq - rsocket->oursequence;
				if(seqdelta<0) seqdelta = seqdelta*-1;
				if(seqdelta>=MAXNETBUFFERS - 1){
					ml_string("Received reliable packet out of order!");
					//It's out of order, so we won't ack it, which will mean we will get it again soon.
					continue;
				}
				//else move data into the proper buffer position
				int savepacket=1;
				
				if(rsocket->oursequence < (0xffff - (MAXNETBUFFERS-1))){
					if (rsocket->oursequence > rcv_buff.seq){
						savepacket = 0;
					}
				} else {
					//Sequence is high, so prepare for wrap around
					if( ((unsigned short)(rcv_buff.seq + rsocket->oursequence)) > (MAXNETBUFFERS-1)){
						savepacket = 0;	
					}
				}

				for(i=0; i<MAXNETBUFFERS; i++){
					if( (NULL != rsocket->rbuffers[i]) && (rsocket->rsequence[i] == rcv_buff.seq)){
						//Received duplicate packet!						
						savepacket = 0;
					}
				}
				if(savepacket){
					if(rcv_buff.data_len>max_len){
						ml_string("Received oversized reliable packet!");
						//don't ack it, which will mean we will get it again soon.
						continue;
					} 
					for(i=0; i<MAXNETBUFFERS; i++){
						if(NULL == rsocket->rbuffers[i]){							
							rsocket->recv_len[i] = rcv_buff.data_len; 
							rsocket->rbuffers[i] = (reliable_net_rcvbuffer *)vm_malloc(sizeof(reliable_net_rcvbuffer));
							memcpy(rsocket->rbuffers[i]->buffer,rcv_buff.data,rsocket->recv_len[i]);	
							rsocket->rsequence[i] = rcv_buff.seq;							
							break;
						}
					}
				}
				psnet_rel_send_ack(&rsocket->addr, rcv_buff.seq, link_type, rcv_buff.send_time);		
			}
			
		}
#ifdef _WIN32
	} while((ipx_has_data>0) || (udp_has_data>0));
#else
	} while (udp_has_data>0);
#endif
	
	// Go through each reliable socket that is connected and do any needed work.
	for(j=0; j<MAXRELIABLESOCKETS; j++){
		rsocket=&Reliable_sockets[j];

		if(Serverconn == 0xffffffff){
			if(rsocket->status==RNF_LIMBO){
				if(fl_abs((psnet_get_time() - rsocket->last_packet_received))>Nettimeout){
					ml_printf("Reliable (but in limbo) socket (%d) timed out in nw_WorkReliable().",j);
					memset(rsocket,0,sizeof(reliable_socket));
					rsocket->status = RNF_UNUSED;//Won't work if this is an outgoing connection.
				}
			}
		} else {
			if((rsocket->status == RNF_LIMBO) && (fl_abs((psnet_get_time() - First_sent_iamhere)) > Nettimeout)){
				rsocket->status = RNF_BROKEN;
				ml_printf("Reliable socket (%d) timed out in nw_WorkReliable().",j);
			}
		}
		
		if(rsocket->status == RNF_CONNECTED){
			float retry_packet_time;
			if((rsocket->mean_ping==0) || (rsocket->mean_ping > (NETRETRYTIME*4))){
				retry_packet_time = NETRETRYTIME;
			} else {
				if(rsocket->mean_ping<MIN_NET_RETRYTIME) {
					retry_packet_time = (float)MIN_NET_RETRYTIME;					
				} else {
					retry_packet_time = ((float)(float)rsocket->mean_ping * (float)1.25);					
				}
			}
			//Iterate through send buffers.  
			for(i=0;i<MAXNETBUFFERS;i++){
				// send again
				if((rsocket->sbuffers[i]) && (fl_abs((psnet_get_time() - rsocket->timesent[i])) >= retry_packet_time)) {
					reliable_header send_header;					
					send_header.send_time = psnet_get_time();
					send_header.send_time = INTEL_FLOAT( &send_header.send_time );
					send_header.seq = INTEL_SHORT( rsocket->ssequence[i] );
					memcpy(send_header.data,rsocket->sbuffers[i]->buffer,rsocket->send_len[i]);
					send_header.data_len = INTEL_SHORT( (ushort)rsocket->send_len[i] );
					send_header.type = RNT_DATA;
					if(rsocket->connection_type == NET_TCP){
						rcode = SENDTO(Unreliable_socket, (char *)&send_header,RELIABLE_PACKET_HEADER_ONLY_SIZE+rsocket->send_len[i],0,&rsocket->addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
					} 
#ifdef _WIN32
                 else if(rsocket->connection_type == NET_IPX){
						rcode = SENDTO(Unreliable_socket, (char *)&send_header,RELIABLE_PACKET_HEADER_ONLY_SIZE+rsocket->send_len[i],0,&rsocket->addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
					}
#endif

					if((rcode == SOCKET_ERROR) && (WSAEWOULDBLOCK == WSAGetLastError())){
						//The packet didn't get sent, flag it to try again next frame
						rsocket->timesent[i] = psnet_get_time()-(NETRETRYTIME*4);
					} else {
						rsocket->last_packet_sent = psnet_get_time();
						rsocket->timesent[i] = psnet_get_time();
					}
					
				}//getcwd
			}

			if((rsocket->status == RNF_CONNECTED) && (fl_abs((psnet_get_time() - rsocket->last_packet_sent)) > NETHEARTBEATTIME)) {
				reliable_header send_header;				
				send_header.send_time = psnet_get_time();
				send_header.send_time = INTEL_FLOAT( &send_header.send_time );
				send_header.seq = 0;
				send_header.data_len = 0;
				send_header.type = RNT_HEARTBEAT;

				rcode = -1;
				if(rsocket->connection_type == NET_TCP){
					rcode = SENDTO(Unreliable_socket, (char *)&send_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,&rsocket->addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
				} 
#ifdef _WIN32
            else if(rsocket->connection_type == NET_IPX){
					rcode = SENDTO(Unreliable_socket, (char *)&send_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,&rsocket->addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
				}
#endif

				if((rcode != SOCKET_ERROR) && (WSAEWOULDBLOCK != WSAGetLastError())){
					//It must have been sent
					rsocket->last_packet_sent = psnet_get_time();
				}
			}

			if((rsocket->status == RNF_CONNECTED) && (fl_abs((psnet_get_time() - rsocket->last_packet_received))>Nettimeout)){
				//This socket is hosed.....inform someone?
				ml_printf("Reliable Socket (%d) timed out in nw_WorkReliable().",j);
				rsocket->status = RNF_BROKEN;
			}
		}
	}	
}

/**
 * Get the status of a reliable socket, see RNF_* defines above
 */
int psnet_rel_get_status(PSNET_SOCKET_RELIABLE socketid)
{	
	if(socketid >= MAXRELIABLESOCKETS){		
		return -1;
	}

	return Reliable_sockets[socketid].status;
}

/**
 * Checks the Listen_socket for possibly incoming requests to be connected.
 * @return 0 on error or nothing waiting.  1 if we should try to accept
 */
int psnet_rel_check_for_listen(net_addr *from_addr)
{	
	SOCKADDR_IN *ip_addr;				// UDP/TCP socket structure
#ifdef _WIN32
	SOCKADDR_IPX *ipx_addr;			// IPX socket structure	
#endif
	
	psnet_rel_work();
	int i;
	for(i=1; i<MAXRELIABLESOCKETS; i++){
		if(Reliable_sockets[i].status == RNF_CONNECTING){
			Reliable_sockets[i].status = RNF_CONNECTED;
			ml_string("New reliable connection in nw_CheckListenSocket().");
			
			switch ( Reliable_sockets[i].connection_type ){
#ifdef _WIN32
			case NET_IPX:
				ipx_addr = (SOCKADDR_IPX *)&Reliable_sockets[i].addr;
				memset(from_addr, 0x00, sizeof(net_addr));
				from_addr->port = ntohs( ipx_addr->sa_socket );
				from_addr->type = NET_IPX;
				memcpy(from_addr->addr, ipx_addr->sa_nodenum, 6 );
				memcpy(from_addr->net_id, ipx_addr->sa_netnum, 4 );
				break;
#endif

			case NET_TCP:
				ip_addr = (SOCKADDR_IN *)&Reliable_sockets[i].addr;
				memset(from_addr, 0x00, sizeof(net_addr));
				from_addr->port = ntohs( ip_addr->sin_port );
				from_addr->type = NET_TCP;
#ifdef _WIN32
				memcpy(from_addr->addr, &ip_addr->sin_addr.S_un.S_addr, 4); //-V512
#else
				memcpy(from_addr->addr, &ip_addr->sin_addr.s_addr, 4); //-V512
#endif
				break;
			
			default:
				Int3();
				break;
			}
			return i;
		}
	}
	return INVALID_SOCKET;	
}

/**
 * Attempt to connect() to the server's tcp socket.  socket parameter is simply assigned to the
 * Reliable_socket socket created in psnet_init
 */
void psnet_rel_connect_to_server(PSNET_SOCKET *socket, net_addr *server_addr)
{	
	//Send out a RNT_REQ_CONN packet, and wait for it to be acked.
	SOCKADDR_IN sockaddr;				// UDP/TCP socket structure
#ifdef _WIN32
	SOCKADDR_IPX ipx_addr;				// IPX socket structure
#endif
	SOCKADDR *addr;						// pointer to SOCKADDR to make coding easier
	SOCKADDR rcv_addr;
	int addrlen;
	ubyte iaddr[6];
	ushort port;
	int name_length;
	float time_sent_req = 0;
	float first_sent_req = 0;
	static reliable_header conn_header;
	static reliable_header ack_header;
	int bytesin;
	struct timeval timeout;
	fd_set read_fds;
	int i;
	*socket = INVALID_SOCKET;	
	
	memset(iaddr, 0x00, 6);
	memcpy(iaddr, &server_addr->addr, 6);
	port = (ushort)(server_addr->port);	// Talk to the server listen port
	
	conn_header.type = RNT_REQ_CONN;
	conn_header.seq = CONNECTSEQ;
	conn_header.data_len = 0;
	
	timeout.tv_sec=0;            
	timeout.tv_usec=0;

#ifdef _WIN32
	if((server_addr->type == NET_IPX) && (!Ipx_active)){
		return;
	}
#endif
	if((server_addr->type == NET_TCP) && (!Tcp_active)){
		return;
	}
	//Flush out any left overs
	if(Tcp_active && (Socket_type == NET_TCP)){
		FD_ZERO(&read_fds);
		FD_SET(Unreliable_socket, &read_fds);    
#ifdef _WIN32
		while(SELECT(0, &read_fds, NULL, NULL, &timeout, PSNET_TYPE_RELIABLE)){
#else
		while(SELECT(Unreliable_socket+1, &read_fds, NULL, NULL, &timeout, PSNET_TYPE_RELIABLE)){
#endif
			addrlen = sizeof(SOCKADDR);
			bytesin = RECVFROM(Unreliable_socket, (char *)&ack_header,sizeof(reliable_header),0,(SOCKADDR *)&rcv_addr,&addrlen, PSNET_TYPE_RELIABLE);
			if(bytesin==-1){
				ml_printf("UDP recvfrom returned an error! -- %d",WSAGetLastError());
				break;
			}
			FD_ZERO(&read_fds);
			FD_SET(Unreliable_socket, &read_fds);    
		}
	}
#ifdef _WIN32
	if(Ipx_active && (Socket_type == NET_IPX)){
		FD_ZERO(&read_fds);
		FD_SET(Unreliable_socket, &read_fds);    
		while(SELECT(0, &read_fds, NULL, NULL, &timeout, PSNET_TYPE_RELIABLE))
		{
			addrlen = sizeof(SOCKADDR);
			bytesin = RECVFROM(Unreliable_socket, (char *)&ack_header,sizeof(reliable_header),0,(SOCKADDR *)&rcv_addr,&addrlen, PSNET_TYPE_RELIABLE);
			if(bytesin==-1)
			{
				ml_printf("IPX recvfrom returned an error! -- %d",WSAGetLastError());
				break;
			}
			FD_ZERO(&read_fds);
			FD_SET(Unreliable_socket, &read_fds);    
		}
	}
#endif
	memset(&ack_header,0,sizeof(reliable_header));
	bytesin = 0;
	SOCKET typeless_sock;
	net_addr d3_rcv_addr;
	memset(&d3_rcv_addr,0,sizeof(net_addr));

	switch ( server_addr->type ){
#ifdef _WIN32
		case NET_IPX:
			ipx_addr.sa_family = AF_IPX;
			memcpy(ipx_addr.sa_nodenum, iaddr, 6);
			memcpy(ipx_addr.sa_netnum, server_addr->net_id, 4);
			ipx_addr.sa_socket = htons(port);
			addr = (SOCKADDR *)&ipx_addr;
			name_length = sizeof(ipx_addr);
			if( SOCKET_ERROR == SENDTO(Unreliable_socket, (char *)&conn_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE) ){
				ml_printf("Unable to send IPX packet in nw_ConnectToServer()! -- %d",WSAGetLastError());
				return;
			}
			typeless_sock = Unreliable_socket;
			memcpy(d3_rcv_addr.addr,ipx_addr.sa_nodenum, 6);
			memcpy(d3_rcv_addr.net_id,ipx_addr.sa_netnum, 4);				
			d3_rcv_addr.type = NET_IPX;
			break;
#endif

		case NET_TCP:
			sockaddr.sin_family = AF_INET; 
			memcpy(&sockaddr.sin_addr.s_addr, iaddr, 4);
			sockaddr.sin_port = htons(port); 
			addr = (SOCKADDR *)&sockaddr;
			name_length = sizeof(sockaddr);
			if( SOCKET_ERROR == SENDTO(Unreliable_socket, (char *)&conn_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE) ){
				ml_printf("Unable to send UDP packet in nw_ConnectToServer()! -- %d",WSAGetLastError());
				return;
			}
			memcpy(d3_rcv_addr.addr, &sockaddr.sin_addr.s_addr, 4); //-V512
			d3_rcv_addr.port = sockaddr.sin_port;
			d3_rcv_addr.type = NET_TCP;
			typeless_sock = Unreliable_socket;
			break;

		default:
			ml_string("Unknown protocol type in nw_ConnectToServer()!");
			Int3();
			return;
	}		

	
	first_sent_req = psnet_get_time();
	time_sent_req = psnet_get_time();
	
	//Wait until we get a response from the server or we timeout
	
	do {
		PSNET_TOP_LAYER_PROCESS();

		FD_ZERO(&read_fds);
		FD_SET(typeless_sock, &read_fds);    		
#ifdef _WIN32
		if(SELECT(0, &read_fds, NULL,NULL,&timeout, PSNET_TYPE_RELIABLE)){
#else
		if(SELECT(typeless_sock+1, &read_fds, NULL,NULL,&timeout, PSNET_TYPE_RELIABLE)){
#endif
			ml_string("selected() in psnet_rel_connect_to_server()");

			addrlen = sizeof(SOCKADDR);
			bytesin = RECVFROM(typeless_sock,(char *)&ack_header,sizeof(reliable_header),0,(SOCKADDR *)&rcv_addr,&addrlen, PSNET_TYPE_RELIABLE);
			if(bytesin==-1){
				ml_printf("recvfrom returned an error! -- %d",WSAGetLastError());
				Int3();
				return;				
			}
			
			ml_string("received data after select in psnet_rel_connect_to_server()");
			if(bytesin){	
				ml_string("about to check ack_header.type");
				if(ack_header.type == RNT_ACK){
					short *acknum = (short *)&ack_header.data;
					if(*acknum == CONNECTSEQ){						
						for(i=1; i<MAXRELIABLESOCKETS; i++){
							if(Reliable_sockets[i].status==RNF_UNUSED){
								//Add the new connection here.
								memset(&Reliable_sockets[i],0,sizeof(reliable_socket));
								Reliable_sockets[i].connection_type = (ubyte)server_addr->type;
								memcpy(&Reliable_sockets[i].m_net_addr,&d3_rcv_addr,sizeof(net_addr));
								Reliable_sockets[i].last_packet_received = psnet_get_time();
								memcpy(&Reliable_sockets[i].addr,&rcv_addr,sizeof(SOCKADDR));
								Reliable_sockets[i].status = RNF_LIMBO;
								*socket = i;
								ml_string("Succesfully connected to server in nw_ConnectToServer().");
								//Now send I_AM_HERE packet
								conn_header.type = RNT_I_AM_HERE;
								conn_header.seq = (ushort)(~CONNECTSEQ);
								conn_header.data_len = 0;
								Serverconn = i;
								First_sent_iamhere = psnet_get_time();
								Last_sent_iamhere = psnet_get_time();
								int rcode = SENDTO(typeless_sock,(char *)&conn_header,RELIABLE_PACKET_HEADER_ONLY_SIZE,0,addr,sizeof(SOCKADDR), PSNET_TYPE_RELIABLE);
								if(rcode == SOCKET_ERROR){
									*socket = INVALID_SOCKET;
									Reliable_sockets[i].status = RNF_UNUSED;
									memset(&Reliable_sockets[i],0,sizeof(reliable_socket));
									ml_string("Unable to send packet in nw_ConnectToServer()");
									return;
								}
								Reliable_sockets[i].last_packet_sent = psnet_get_time();
								float f;
								f = psnet_get_time();
								while((fl_abs((psnet_get_time() - f))<2) && (Reliable_sockets[i].status != RNF_CONNECTING)){
									psnet_rel_work();
								}
									
								return;
							}
						}
						ml_string("Out of reliable socket space in nw_ConnectToServer().");
						return;						
					} else ml_string("Received out of sequence ACK in nw_ConnectToServer().");
				} else ml_string("Received something that isn't an ACK in nw_ConnectToServer().");
			} else ml_string("Received 0 bytes from recvfrom() in nw_ConnectToServer().");
		}
	} while(fl_abs((psnet_get_time() - first_sent_req)) < RELIABLE_CONNECT_TIME);	
}

/**
 * Returns the IP address of this computer
 */
int psnet_get_ip()
{	
	SOCKADDR_IN local_address;

#ifdef _WIN32
	if(Psnet_connection == NETWORK_CONNECTION_DIALUP){	
		local_address.sin_addr.s_addr = psnet_ras_status();
		if(local_address.sin_addr.s_addr == INADDR_NONE){
			local_address.sin_addr.s_addr = INADDR_ANY;
		}
	} 
	else
#endif   // FIXME - always returns INADDR_ANY for LAN
	{
		// Init local address to zero
		local_address.sin_addr.s_addr = INADDR_ANY;			
	}

	ml_printf("psnet_get_ip() reports IP : %s", inet_ntoa(local_address.sin_addr));
	
	return local_address.sin_addr.s_addr;
}

/**
 * Initialize reliable sockets
 */
int psnet_init_rel_tcp(int port, int should_listen)
{
	// success
	return 1;
}

/**
 * Initialize reliable sockets
 */
int psnet_init_rel_ipx(int port, int should_listen)
{	
	// success
	return 1;
}

void psnet_rel_close()
{
	int idx;
	PSNET_SOCKET_RELIABLE sock;

	// kill all sockets
	for(idx=0; idx<MAXRELIABLESOCKETS; idx++){
		if(Reliable_sockets[idx].status != RNF_UNUSED){
			sock = idx;
			psnet_rel_close_socket(&sock);
		}
	}
}

// ------------------------------------------------------------------------------------------------------
// PACKET BUFFERING FUNCTIONS
//

/**
 * Initialize the buffering system
 */
void psnet_buffer_init(network_packet_buffer_list *l)
{
	int idx;
	
	// blast the buffer clean
	memset(l->psnet_buffers, 0, sizeof(network_packet_buffer) * MAX_PACKET_BUFFERS);
	
	// set all buffer sequence #'s to -1
	for(idx=0;idx<MAX_PACKET_BUFFERS;idx++){		
		l->psnet_buffers[idx].sequence_number = -1;
	}

	// initialize the sequence #
	l->psnet_seq_number = 0;
	l->psnet_lowest_id = -1;
	l->psnet_highest_id = -1;
}

/**
 * Buffer a packet (maintain order!)
 */
void psnet_buffer_packet(network_packet_buffer_list *l, ubyte *data, int length, net_addr *from)
{
	int idx;
	int found_buf = 0;
	
	// find the first empty packet
	for(idx=0;idx<MAX_PACKET_BUFFERS;idx++){
		if(l->psnet_buffers[idx].sequence_number == -1){
			found_buf = 1;
			break;
		}
	}

	// if we didn't find the buffer, report an overrun
	if(!found_buf){
		ml_string("WARNING - Buffer overrun in psnet");
	} else {
		// copy in the data
		memcpy(l->psnet_buffers[idx].data, data, length);
		l->psnet_buffers[idx].len = length;
		memcpy(&l->psnet_buffers[idx].from_addr, from, sizeof(net_addr));
		l->psnet_buffers[idx].sequence_number = l->psnet_seq_number;
		
		// keep track of the highest id#
		l->psnet_highest_id = l->psnet_seq_number++;

		// set the lowest id# for the first time
		if(l->psnet_lowest_id == -1){
			l->psnet_lowest_id = l->psnet_highest_id;
		}
	}
}

/**
 * Get the index of the next packet in order!
 */
int psnet_buffer_get_next(network_packet_buffer_list *l, ubyte *data, int *length, net_addr *from)
{	
	int idx;
	int found_buf = 0;

	// if there are no buffers, do nothing
	if((l->psnet_lowest_id == -1) || (l->psnet_lowest_id > l->psnet_highest_id)){
		return 0;
	}

	// search until we find the lowest packet index id#
	for(idx=0;idx<MAX_PACKET_BUFFERS;idx++){
		// if we found the buffer
		if(l->psnet_buffers[idx].sequence_number == l->psnet_lowest_id){
			found_buf = 1;
			break;
		}
	}

	// at this point, we should _always_ have found the buffer
	Assert(found_buf);	
	
	// copy out the buffer data
	memcpy(data, l->psnet_buffers[idx].data, l->psnet_buffers[idx].len);
	*length = l->psnet_buffers[idx].len;
	memcpy(from, &l->psnet_buffers[idx].from_addr, sizeof(net_addr));

	// now we need to cleanup the packet list

	// mark the buffer as free
	l->psnet_buffers[idx].sequence_number = -1;
	l->psnet_lowest_id++;

	return 1;
}

// -------------------------------------------------------------------------------------------------------
// PSNET 2 FORWARD DEFINITIONS
//

/**
 * If the string is a legally formatted IP string
 */
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

#ifdef _WIN32 // Dial-Up Networking
// function called from high level FreeSpace code to determine the status of the networking
// code returns one of a handful of macros

DWORD (__stdcall *pRasEnumConnections)(LPRASCONN lprasconn, LPDWORD lpcb, LPDWORD lpcConnections) = NULL;
DWORD (__stdcall *pRasGetConnectStatus)(HRASCONN hrasconn, LPRASCONNSTATUS lprasconnstatus ) = NULL;
DWORD (__stdcall *pRasGetProjectionInfo)(HRASCONN hrasconn, RASPROJECTION rasprojection, LPVOID lpprojection, LPDWORD lpcb ) = NULL;

/**
 * Get the status of a RAS connection
 */
unsigned int psnet_ras_status()
{
	int rval;
	unsigned long size, num_connections, i, valid_connections = 0;
	RASCONN rasbuffer[25];
	HINSTANCE ras_handle;
	unsigned long rasip=0;
	RASPPPIP projection;

	Ras_connected = 0;

	// first, call a LoadLibrary to load the RAS api
	ras_handle = LoadLibrary( "rasapi32.dll" );
	if ( ras_handle == NULL ) {
		return INADDR_ANY;
	}

	pRasEnumConnections = (DWORD (__stdcall *)(LPRASCONN, LPDWORD, LPDWORD))GetProcAddress(ras_handle, "RasEnumConnectionsA");
	if (!pRasEnumConnections)	{
		FreeLibrary( ras_handle );
		return INADDR_ANY;
	}
	pRasGetConnectStatus = (DWORD (__stdcall *)(HRASCONN, LPRASCONNSTATUS))GetProcAddress(ras_handle, "RasGetConnectStatusA");
	if (!pRasGetConnectStatus)	{
		FreeLibrary( ras_handle );
		return INADDR_ANY;
	}
	pRasGetProjectionInfo = (DWORD (__stdcall *)(HRASCONN, RASPROJECTION, LPVOID, LPDWORD))GetProcAddress(ras_handle, "RasGetProjectionInfoA");
	if (!pRasGetProjectionInfo)	{
		FreeLibrary( ras_handle );
		return INADDR_ANY;
	}

	size = sizeof(rasbuffer);
	rasbuffer[0].dwSize = sizeof(RASCONN);

	rval = pRasEnumConnections( rasbuffer, &size, &num_connections );
	if ( rval ) {
		FreeLibrary( ras_handle );
		return INADDR_ANY;
	}

	// JAS: My computer gets to this point, but I have no RAS connections,
	// so just exit
	if ( num_connections < 1 )	{
		ml_string("Found no RAS connections");
		FreeLibrary( ras_handle );
		return INADDR_ANY;
	}

	ml_printf("Found %d connections", num_connections);

	for (i = 0; i < num_connections; i++ ) {
		RASCONNSTATUS status;
		unsigned long dummySize;

		// don't count VPNs with the non-LAN connections
		if ( !stricmp(rasbuffer[i].szDeviceType, "RASDT_Vpn") ) {
			continue;
		} else {
			valid_connections++;
		}

		ml_printf("Connection %d:", i);
		ml_printf("Entry Name: %s", rasbuffer[i].szEntryName);
		ml_printf("Device Type: %s", rasbuffer[i].szDeviceType);
		ml_printf("Device Name: %s", rasbuffer[i].szDeviceName);

		// get the connection status
		status.dwSize = sizeof(RASCONNSTATUS);
		rval = pRasGetConnectStatus(rasbuffer[i].hrasconn, &status);
		if ( rval != 0 ) {
			FreeLibrary( ras_handle );
			return INADDR_ANY;
		}

		// get the projection informatiom
		size = sizeof(projection);
		projection.dwSize = size;
		rval = pRasGetProjectionInfo(rasbuffer[i].hrasconn, RASP_PppIp, &projection, &dummySize );
		if ( rval != 0 ) {
			FreeLibrary( ras_handle );
			return INADDR_ANY;
		}

		ml_printf("IP Address: %s", projection.szIpAddress);
	}

	if (!valid_connections) {
		FreeLibrary( ras_handle );
		return INADDR_ANY;
	}

	Ras_connected = 1;

	FreeLibrary( ras_handle );
	rasip = inet_addr(projection.szIpAddress);
	if(rasip==INADDR_NONE){
		return INADDR_ANY;
	}

	//The ip of the RAS connection
	return rasip;
}
#endif // Dial-Up Networking

/**
 * Set some options on a socket
 */
void psnet_socket_options( SOCKET sock )
{
	int broadcast;
	int cursize, bufsize; 
	socklen_t cursizesize;

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
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (LPSTR)&bufsize, sizeof(bufsize));
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (LPSTR)&cursize, &cursizesize);
	ml_printf("Receive buffer set to %d", cursize);

	// set the current size of the send buffer
	cursizesize = sizeof(int);
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (LPSTR)&cursize, &cursizesize);
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (LPSTR)&bufsize, sizeof(bufsize));
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (LPSTR)&cursize, &cursizesize);
	ml_printf("Send buffer set to %d", cursize);
}

#ifdef _WIN32
/**
 * Initialize ipx socket
 */
int psnet_init_ipx()
{	
	SOCKADDR_IPX sockaddr;

	IPX_socket = INVALID_SOCKET;	
	
	IPX_socket = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
	if ( IPX_socket == INVALID_SOCKET ) {
		Ipx_failure_code = WSAGetLastError();
		ml_printf("Error on IPX startup %d", Ipx_failure_code);
		return 0;
	}

	// bind the socket
	memset(&sockaddr,0,sizeof(SOCKADDR_IPX));
	sockaddr.sa_family = AF_IPX; 	
	sockaddr.sa_socket = htons( Psnet_default_port );
	if ( bind(IPX_socket, (SOCKADDR*)&sockaddr, sizeof (SOCKADDR_IPX)) == SOCKET_ERROR) {	
		Ipx_failure_code = WSAGetLastError();
		ml_printf("Couldn't bind IPX socket (%d)! Invalidating IPX", Ipx_failure_code ); 
		return 0;
	}

	// set socket options
	psnet_socket_options( IPX_socket );		
	Ipx_can_broadcast = Can_broadcast;

	// success
	return 1;
}
#endif

/**
 * Initialize tcp socket
 */
int psnet_init_tcp()
{	
	SOCKADDR_IN sockaddr;

	TCP_socket = INVALID_SOCKET;	
	
	TCP_socket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( TCP_socket == (int)INVALID_SOCKET ) {
		Tcp_failure_code = WSAGetLastError();
		ml_printf("Error on TCP startup %d", Tcp_failure_code);
		return 0;
	}

	// bind the socket
	memset(&sockaddr,0,sizeof(SOCKADDR_IN));
	sockaddr.sin_family = AF_INET; 
	sockaddr.sin_addr.s_addr = psnet_get_ip();
	sockaddr.sin_port = htons( Psnet_default_port );
	if ( bind(TCP_socket, (SOCKADDR*)&sockaddr, sizeof (sockaddr)) == SOCKET_ERROR) {	
		Tcp_failure_code = WSAGetLastError();

		if (Tcp_failure_code == WSAEADDRINUSE) {
			ml_printf("TCP socket already in use!  Another instance running?  (Try using the \"-port %i\" cmdline option)", Psnet_default_port + 1);
		} else {
			ml_printf("Couldn't bind TCP socket (%d)! Invalidating TCP", Tcp_failure_code );
		}

		return 0;
	}

	// set socket options
	psnet_socket_options( TCP_socket );		
	Tcp_can_broadcast = Can_broadcast;

	// success
	return 1;
}

/**
 * Get time in seconds
 */
float psnet_get_time()
{		
	return (float)timer_get_milliseconds() / 1000.0f;
}

/**
 * Mark a socket as having received data
 */
void psnet_mark_received(PSNET_SOCKET_RELIABLE socket)
{
	// valid socket?
	if((socket == 0xffffffff) || (socket >= MAXRELIABLESOCKETS)){
		return;
	}

	// mark it
	Reliable_sockets[socket].last_packet_received = psnet_get_time();
}
