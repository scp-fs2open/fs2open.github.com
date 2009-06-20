/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifndef _PSNET_H
#define _PSNET_H

// use PSNET 2
#define PSNET2

#ifdef PSNET2
	#include "network/psnet2.h"
#else 

// use Berkeley reliable sockets if defined - otherwise use Volition reliable sockets
#define PSNET_RELIABLE_OLD_SCHOOL

#define NET_NONE		0		// if no protocol is active or none are selected
#define NET_TCP		1
#define NET_IPX		2
#define NET_VMT		3

// kazan - I think this should raise the ships limit across the network
//#define MAX_PACKET_SIZE 4096
#define MAX_PACKET_SIZE		512

#define PSNET_FLAG_CHECKSUM	(1<<0)		// this packet is checksummed
#define PSNET_FLAG_RAW			(1<<1)		// send or receive raw data. don't do any checksumming, sequencing, etc

#define DEFAULT_GAME_PORT 7802

typedef struct net_addr	{
	uint	type;			// See NET_ defines above
	ubyte	net_id[4];	// used for IPX only
	ubyte addr[6];		// address (first 4 used when IP, all 6 used when IPX)
	short port;			
} net_addr;

// define these in such a manner that a call to psnet_send_reliable is exactly the same and the new code in unobtrusive
#ifdef PSNET_RELIABLE_OLD_SCHOOL
	typedef uint PSNET_SOCKET;
	typedef uint PSNET_SOCKET_RELIABLE;

	#undef INVALID_SOCKET
	#define INVALID_SOCKET (PSNET_SOCKET)(~0)
#else
	typedef net_addr* PSNET_SOCKET;
	typedef net_addr* PSNET_SOCKET_RELIABLE

	#undef INVALID_SOCKET
	#define INVALID_SOCKET NULL

#endif

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

extern net_addr Psnet_my_addr;							// address information of this machine
extern uint Psnet_my_ip;
extern int Psnet_my_addr_valid;

extern int Network_status;
extern int Tcp_failure_code;
extern int Ipx_failure_code;

extern int Tcp_active;

// specified their internet connnection type
#define NETWORK_CONNECTION_NONE			1
#define NETWORK_CONNECTION_DIALUP		2
#define NETWORK_CONNECTION_LAN			3

extern int Psnet_connection;

extern ushort Psnet_default_port;


#ifndef NDEBUG

extern int	Psnet_bytes_read;					// globally available numbers for printing on the hud
extern int	Psnet_bytes_written;
extern void psnet_calc_socket_stats();		// routine to calc stats for this frame.
#endif

void ipx_ntoa(net_addr *addr, char *text); // this is a HUGE hack right now. Just the 6 byte equivalent of inet_ntoa

extern void psnet_init( int protocol, int default_port );
extern void psnet_close();
extern int psnet_use_protocol( int type );
extern void psnet_rel_close_socket( PSNET_SOCKET *sockp );
extern int psnet_rel_check();
extern int psnet_get_network_status();

extern void psnet_whoami( net_addr * my_address );
extern char* psnet_addr_to_string( char * text, net_addr * address );
extern void psnet_string_to_addr( net_addr * address, char * text );
extern int psnet_same( net_addr * a1, net_addr * a2 );

extern int psnet_send( net_addr * who_to, void * data, int len, int flags = PSNET_FLAG_RAW, int reliable_socket = 0 );
extern int psnet_get( void * data, net_addr * from_addr, int flags = PSNET_FLAG_RAW );
extern int psnet_broadcast( net_addr * who_to, void * data, int len,int flags = PSNET_FLAG_RAW );

// functions for reliable socket stuff
extern int psnet_rel_send( PSNET_SOCKET socket, ubyte *data, int length, int flags = PSNET_FLAG_RAW );
extern int psnet_rel_get( PSNET_SOCKET socket, ubyte *buffer, int max_length, int flags = PSNET_FLAG_RAW);

extern int psnet_rel_check_for_listen( net_addr *addr );
extern void psnet_rel_connect_to_server( PSNET_SOCKET *s, net_addr *server_addr );

extern void psnet_flush();
extern int psnet_is_valid_ip_string( char *ip_string, int allow_port=1 );

// initialize the buffering system
extern void psnet_buffer_init();

// buffer a packet (maintain order!)
extern void psnet_buffer_packet(ubyte *data, int length, net_addr *from);

// get the index of the next packet in order!
extern int psnet_buffer_get_next(ubyte *data, int *length, net_addr *from);


// -------------------------------------------------------------------------------------
// PSNET RELIABLE UDP STUFF
//

// initialize the psnet reliable system (return 0 on fail, 1 on success)
int psnet_reliable_init();

// shutdown the reliable system (free up buffers, etc)
void psnet_reliable_close();

// notify the reliable system of a new address at index N
void psnet_reliable_notify_new_addr(net_addr *addr,int index);

// notify the reliable system of a drop at index N
void psnet_reliable_notify_drop_addr(net_addr *addr);

// send a reliable data packet
int psnet_reliable_send(ubyte *data,int packet_size,net_addr *addr);

// process frame for all reliable stuff (call once per frame)
void psnet_reliable_process();

// determine if the passed in reliable data should be processed, and sends an ack if necessary
// return # of bytes which should be stripped off the data (reliable data header)
int psnet_reliable_should_process(net_addr *addr,ubyte *data,int packet_size);


#ifndef NDEBUG
extern void psnet_stats_init();
extern int psnet_get_stats( net_addr *addr, int *tr, int *tw );
#endif

#endif // #ifdef PSNET2

#endif
