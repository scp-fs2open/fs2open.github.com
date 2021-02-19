/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef PSNET2_H
#define PSNET2_H


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <cerrno>
#endif

#include "globalincs/pstypes.h"

// -------------------------------------------------------------------------------------------------------
// PSNET 2 DEFINES/VARS
//

#define NET_NONE		0		// if no protocol is active or none are selected
#define NET_TCP		1
#define NET_VMT		3

// adjust this value to change max packet size
//
// 1280 = min safe size required by IPv6 spec
//  -40 = IPv6 header
//   -8 = UDP header
#define MAX_TOP_LAYER_PACKET_SIZE			1232

// MAX_PACKET_SIZE must be set to at least 11 bytes less than MAX_TOP_LAYER_PACKET_SIZE
//   10 bytes required for reliable packet header
//   1 byte added to every packet for psnet ident
#define MAX_PACKET_SIZE		(MAX_TOP_LAYER_PACKET_SIZE-11)

#define DEFAULT_GAME_PORT 7808


typedef struct net_addr {
	uint8_t addr[sizeof(in6_addr)];
	uint16_t port;
	uint8_t __pad[2];	// alignment
} net_addr;


// define these in such a manner that a call to psnet_send_reliable is exactly the same and the new code in unobtrusive
typedef uint PSNET_SOCKET;
typedef uint PSNET_SOCKET_RELIABLE;

#define PSNET_INVALID_SOCKET static_cast<PSNET_SOCKET>(~0)

// defines for protocol overheads
#define UDP_HEADER_SIZE						34

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

extern int Psnet_failure_code;

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

extern SOCKET Psnet_socket;	// all PXO API modules should use this to send and receive on

extern unsigned int Serverconn;

#define PSNET_IP_MODE_UNKNOWN	0
#define PSNET_IP_MODE_V4		(1<<0)
#define PSNET_IP_MODE_V6		(1<<1)
#define PSNET_IP_MODE_DUAL		(PSNET_IP_MODE_V4|PSNET_IP_MODE_V6)

// -------------------------------------------------------------------------------------------------------
// PSNET 2 TOP LAYER FUNCTIONS - these functions simply buffer and store packets based upon type (see PSNET_TYPE_* defines)
//

struct sockaddr;
struct timeval;

// wrappers around select() and recvfrom() for lagging/losing data, and for sorting through different packet types
int RECVFROM(SOCKET s, char * buf, int len, int flags, sockaddr *from, int *fromlen, int psnet_type);
int SELECT(int nfds, fd_set *readfds, fd_set *writefds, fd_set*exceptfds, struct timeval* timeout, int psnet_type);

// wrappers around sendto to sorting through different packet types
int SENDTO(SOCKET s, char * buf, int len, int flags, sockaddr * to, int tolen, int psnet_type);

// call this once per frame to read everything off of our socket
void PSNET_TOP_LAYER_PROCESS();


// -------------------------------------------------------------------------------------------------------
// PSNET 2 FUNCTIONS
//

// initialize psnet to use the specified port
void psnet_init(uint16_t default_port = 0);

// is psnet initted properly
bool psnet_is_active();

// shutdown psnet
void psnet_close();

// get the status of the network
int psnet_get_network_status();

// convert a net_addr to a string
const char *psnet_addr_to_string(const net_addr *address, char *text, size_t max_len);

// convert a string to a net addr
bool psnet_string_to_addr(const char *text, net_addr *address);

// compare 2 addresses
int psnet_same(SOCKADDR_IN6 *a1, SOCKADDR_IN6 *a2);
int psnet_same(net_addr *a1, net_addr *a2);

// check if address is local machine instance
bool psnet_is_local_addr(const net_addr *addr);

// send data unreliably
int psnet_send(net_addr *who_to, void *data, int len, int np_index = -1);

// get data from the unreliable socket
int psnet_get(void *data, net_addr *from_addr);

// flush all sockets
void psnet_flush();

// if the passed string is a valid IP string
bool psnet_is_valid_ip_string(const char *ip_string);

// mark a socket as having received data
void psnet_mark_received(PSNET_SOCKET_RELIABLE socketid);

// fill sockaddr structure with usable data
bool psnet_get_addr(const char *host, const char *port, SOCKADDR_STORAGE *addr, int flags = 0);
bool psnet_get_addr(const char *host, uint16_t port, SOCKADDR_STORAGE *addr, int flags = 0);

#define ADDR_FLAG_NUMERIC_SERVICE	(1<<0)
#define ADDR_FLAG_PREFER_IPV4		(1<<1)

// map an IPv4 address to IPv6
void psnet_map4to6(const in_addr *in4, in6_addr *in6);

// map an IPv4-mapped IPv6 address to IPv4
bool psnet_map6to4(const in6_addr *in6, in_addr *in4);

// anonymize address for logging
in6_addr *psnet_mask_addr(const in6_addr *inaddr);

const in6_addr *psnet_get_local_ip(int af_type);

int psnet_get_ip_mode();

// -------------------------------------------------------------------------------------------------------
// PSNET 2 RELIABLE SOCKET FUNCTIONS
//

// shutdown a reliable socket
void psnet_rel_close_socket(PSNET_SOCKET_RELIABLE socketid);

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
PSNET_SOCKET psnet_rel_check_for_listen(net_addr *addr);

// perform a reliable socket connect to the specified server
void psnet_rel_connect_to_server(PSNET_SOCKET_RELIABLE *s, net_addr *server_addr);

#endif
