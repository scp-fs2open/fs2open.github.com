/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _PSNET2_H
#define _PSNET2_H


#ifdef _WIN32
#include <winsock.h>
#else
#include <errno.h>
#endif

#include "globalincs/pstypes.h"

// -------------------------------------------------------------------------------------------------------
// PSNET 2 DEFINES/VARS
//

#define NET_NONE		0		// if no protocol is active or none are selected
#define NET_TCP		1
#define NET_IPX		2
#define NET_VMT		3

// kazan - I think this should raise the ships limit across the network
//#define MAX_PACKET_SIZE 4096
#define MAX_PACKET_SIZE		512

#define DEFAULT_GAME_PORT 7808


typedef struct net_addr	{
	uint	type;			// See NET_ defines above
	ubyte	net_id[4];	// used for IPX only
	ubyte addr[6];		// address (first 4 used when IP, all 6 used when IPX)
	short port;			
} net_addr;

// define these in such a manner that a call to psnet_send_reliable is exactly the same and the new code in unobtrusive
typedef uint PSNET_SOCKET;
typedef uint PSNET_SOCKET_RELIABLE;
#if defined(_WIN32)
typedef int socklen_t;
#endif

#undef INVALID_SOCKET
#define INVALID_SOCKET (PSNET_SOCKET)(~0)

// defines for protocol overheads
#define UDP_HEADER_SIZE						34
#define TCP_HEADER_SIZE						40
#define TCP_HEADER_SIZE_COMPRESSED		6

// define values for network errors when trying to enter the ready room
#define NETWORK_ERROR_NONE					0
#define NETWORK_ERROR_NO_TYPE				-1
#define NETWORK_ERROR_NO_WINSOCK			-2
#define NETWORK_ERROR_NO_PROTOCOL		-3
#define NETWORK_ERROR_RELIABLE			-4
#define NETWORK_ERROR_CONNECT_TO_ISP	-5
#define NETWORK_ERROR_LAN_AND_RAS		-6

// psnet packet types
#define PSNET_NUM_TYPES						5
#define PSNET_TYPE_UNRELIABLE				0
#define PSNET_TYPE_RELIABLE				1
#define PSNET_TYPE_USER_TRACKER			2
#define PSNET_TYPE_GAME_TRACKER			3
#define PSNET_TYPE_VALIDATION				4

extern net_addr Psnet_my_addr;							// address information of this machine
extern uint Psnet_my_ip;
extern int Psnet_my_addr_valid;

extern int Network_status;
extern int Tcp_failure_code;
extern int Ipx_failure_code;

extern int Tcp_active;
extern int Ipx_active;

extern int Socket_type;										// protocol type in use (see NET_* defines above)

// specified their internet connnection type
#define NETWORK_CONNECTION_NONE			1
#define NETWORK_CONNECTION_DIALUP		2
#define NETWORK_CONNECTION_LAN			3

extern int Psnet_connection;

extern ushort Psnet_default_port;

// Reliable socket states
#define RNF_UNUSED			0		// Completely clean socket..
#define RNF_CONNECTED		1		// Connected and running fine
#define RNF_BROKEN			2		// Broken - disconnected abnormally
#define RNF_DISCONNECTED	3		// Disconnected cleanly
#define RNF_CONNECTING		4		// We received the connecting message, but haven't told the game yet.
#define RNF_LIMBO				5		// between connecting and connected

extern SOCKET Unreliable_socket;	// all PXO API modules should use this to send and receive on

// -------------------------------------------------------------------------------------------------------
// PSNET 2 TOP LAYER FUNCTIONS - these functions simply buffer and store packets based upon type (see PSNET_TYPE_* defines)
//

struct sockaddr;
#ifdef _WIN32
struct fd_set;
#endif
struct timeval;

// wrappers around select() and recvfrom() for lagging/losing data, and for sorting through different packet types
int RECVFROM(uint s, char * buf, int len, int flags, sockaddr *from, int *fromlen, int psnet_type);
int SELECT(int nfds, fd_set *readfds, fd_set *writefds, fd_set*exceptfds, const timeval* timeout, int psnet_type);

// wrappers around sendto to sorting through different packet types
int SENDTO(uint s, char * buf, int len, int flags, sockaddr * to, int tolen, int psnet_type);

// call this once per frame to read everything off of our socket
void PSNET_TOP_LAYER_PROCESS();


// -------------------------------------------------------------------------------------------------------
// PSNET 2 FUNCTIONS
//

// initialize psnet to use the specified port
void psnet_init(int protocol, int default_port);

// shutdown psnet
void psnet_close();

// set the protocol to use
int psnet_use_protocol(int type);

// get the status of the network
int psnet_get_network_status();

// convert a net_addr to a string
char *psnet_addr_to_string( char * text, net_addr * address );

// convert a string to a net addr
void psnet_string_to_addr( net_addr * address, char * text );

// compare 2 addresses
int psnet_same( net_addr * a1, net_addr * a2 );

// send data unreliably
int psnet_send( net_addr * who_to, void * data, int len, int np_index = -1 );

// get data from the unreliable socket
int psnet_get( void * data, net_addr * from_addr );

// broadcast data on unreliable socket
int psnet_broadcast( net_addr * who_to, void * data, int len );

// flush all sockets
void psnet_flush();

// if the passed string is a valid IP string
int psnet_is_valid_ip_string( char *ip_string, int allow_port=1 );

// mark a socket as having received data
void psnet_mark_received(PSNET_SOCKET_RELIABLE socket);


// -------------------------------------------------------------------------------------------------------
// PSNET 2 RELIABLE SOCKET FUNCTIONS
//

// shutdown a reliable socket
void psnet_rel_close_socket(PSNET_SOCKET_RELIABLE *sockp);

// obsolete function - left in for compatibility sake
int psnet_rel_check();

// send data on the reliable socket
int psnet_rel_send(PSNET_SOCKET_RELIABLE socket, ubyte *data, int length, int np_index = -1);

// Return codes:
// -1 socket not connected
// 0 No packet ready to receive
// >0 Buffer filled with the number of bytes recieved
int psnet_rel_get(PSNET_SOCKET_RELIABLE socket, ubyte *buffer, int max_length);

// process all active reliable sockets
void psnet_rel_work();

// get the status of a reliable socket, see RNF_* defines above
int psnet_rel_get_status(PSNET_SOCKET_RELIABLE sock);

// check the listen socket for pending reliable connections
int psnet_rel_check_for_listen(net_addr *addr);

// perform a reliable socket connect to the specified server
void psnet_rel_connect_to_server(PSNET_SOCKET_RELIABLE *s, net_addr *server_addr);

#endif
