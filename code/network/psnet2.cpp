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

#include <cstdio>
#include <climits>
#include <algorithm>
#include <sstream>

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

net_addr Psnet_my_addr;
static SCP_vector<in6_addr> Psnet_my_ip;

static int Psnet_ip_mode = PSNET_IP_MODE_UNKNOWN;

static bool Psnet_active = false;

static int Network_status;
int Psnet_failure_code = 0;
int Psnet_connection;

uint16_t Psnet_default_port;

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

// use the pack pragma to pack these structures to 2 byte aligment.  Really only needed for
// the naked packet.
#define MAX_PACKET_BUFFERS		75

#pragma pack(push, 2)

/**
 * Structure definition for our packet buffers
 */
typedef struct network_packet_buffer
{
	int		sequence_number;
	SSIZE_T		len;
	SOCKADDR_IN6	from_addr;
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

#define RELIABLE_CONNECT_TIME		7		// how long we'll wait for a response when doing a reliable connect

static int Nettimeout = NETTIMEOUT;

// Reliable packet stuff
#define RNT_ACK				1				// ACK Packet
#define RNT_DATA				2				// Data Packet
#define RNT_DATA_COMP		3				// Compressed Data Packet
#define RNT_REQ_CONN			4				// Requesting a connection
#define RNT_DISCONNECT		5				// Disconnecting a connection
#define RNT_HEARTBEAT		6				// Heartbeat -- send every NETHEARTBEATTIME
#define RNT_I_AM_HERE		7

#pragma pack(push, 1)
typedef struct reliable_header {
	ubyte		type;					// packet type
	ubyte		compressed;				//
	ushort		seq;					// sequence packet 0-65535 used for ACKing also
	ushort		data_len;				// length of data
	float		send_time;				// Time the packet was sent, if an ACK the time the packet being ACK'd was sent.
	ubyte		data[MAX_PACKET_SIZE];	// Packet data

	reliable_header() : type(0), compressed(0), seq(0), data_len(0), send_time(0.0f) {}
} reliable_header;
#pragma pack(pop)

// Psnet adds 1 byte for type ident, so make sure we've got a little headroom
static_assert(sizeof(reliable_header) < MAX_TOP_LAYER_PACKET_SIZE, "reliable_header is larger than max packet size!");

#define RELIABLE_PACKET_HEADER_ONLY_SIZE (sizeof(reliable_header)-MAX_PACKET_SIZE)
#define MAX_PING_HISTORY	10

typedef struct {
	ubyte buffer[MAX_PACKET_SIZE];
} reliable_net_buffer;

#pragma pack(push, 1)
typedef struct {
	reliable_net_buffer *sbuffers[MAXNETBUFFERS];	// This is an array of pointers for quick sorting
	reliable_net_buffer  *rbuffers[MAXNETBUFFERS];
	int send_len[MAXNETBUFFERS];
	int recv_len[MAXNETBUFFERS];
	unsigned short ssequence[MAXNETBUFFERS];
	unsigned short rsequence[MAXNETBUFFERS];				// This is the sequence number of the given packet
	float timesent[MAXNETBUFFERS];
	float last_packet_received;								// For a given connection, this is the last packet we received
	float last_packet_sent;
	SOCKADDR_IN6 addr;													// SOCKADDR of our peer
	ushort status;													// Status of this connection
	unsigned short oursequence;								// This is the next sequence number the application is expecting
	unsigned short theirsequence;								// This is the next sequence number the peer is expecting
	unsigned short ping_pos;
	float pings[MAX_PING_HISTORY];
	unsigned int num_ping_samples;
	float mean_ping;
} reliable_socket;
#pragma pack(pop)

static reliable_socket Reliable_sockets[MAXRELIABLESOCKETS];

// the sockets that the game will use when selecting network type
SOCKET Psnet_socket = INVALID_SOCKET;

static float First_sent_iamhere = 0;
static float Last_sent_iamhere = 0;

#define CONNECTSEQ 0x142										// Magic number for starting a connection, just so it isn't 0

unsigned int Serverconn = 0xffffffff;

// bad packet type debugging
static constexpr size_t MAX_BAD_PACKETS_WINDOW = 3600;		// in seconds, 1 hour
static constexpr size_t MAX_BAD_PACKETS_PER_WINDOW = 20;

static size_t Psnet_bad_packet_count = 0;
static time_t Psnet_bad_packet_time = 0;

//*******************************

// top layer buffers
static network_packet_buffer_list Psnet_top_buffers[PSNET_NUM_TYPES];

// -------------------------------------------------------------------------------------------------------
// PSNET 2 FORWARD DECLARATIONS
//

// set some options on a socket
void psnet_set_socket_options();

// initialize socket
bool psnet_init_socket();

// initilize my addr
bool psnet_init_my_addr();

// get time in seconds
float psnet_get_time();

// initialize reliable sockets
void psnet_init_rel_tcp();

// shutdown reliable sockets
void psnet_rel_close();

// initialize the buffering system
void psnet_buffer_init(network_packet_buffer_list *l);

// buffer a packet (maintain order!)
static void psnet_buffer_packet(network_packet_buffer_list *l, const ubyte *data, const SSIZE_T length, const SOCKADDR_IN6 *from);

// get the index of the next packet in order!
int psnet_buffer_get_next(network_packet_buffer_list *l, ubyte *data, SSIZE_T *length, SOCKADDR_IN6 *from);

// ip string parsing helpers
static bool psnet_is_ip_notation(int af, const char *ip_string);
static bool psnet_explode_ip_string(const char *ip_string, SCP_string &host, SCP_string &port);

// conversions
static void psnet_sockaddr_to_addr(const SOCKADDR_IN6 *sockaddr, net_addr *addr);
static void psnet_addr_to_sockaddr(const net_addr *addr, SOCKADDR_IN6 *sockaddr);

// debugging / testing
static void psnet_debug_bad_packet(const int packet_type, const uint8_t *packet_data, const SSIZE_T read_len, const SOCKADDR_IN6 *from_addr);

// -------------------------------------------------------------------------------------------------------
// PSNET 2 TOP LAYER FUNCTIONS - these functions simply buffer and store packets based upon type (see PSNET_TYPE_* defines)
//

/**
 * Wrappers around select() and recvfrom() for lagging/losing data
 */
int RECVFROM(SOCKET  /*s*/, char *buf, int  /*len*/, int  /*flags*/, SOCKADDR *from, int *fromlen, int psnet_type)
{
	network_packet_buffer_list *l;
	SOCKADDR_IN6 addr;
	int ret;
	SSIZE_T ret_len;

	// bad type
	Assert( (psnet_type >= 0) && (psnet_type < PSNET_NUM_TYPES) );

	if ( (psnet_type < 0) || (psnet_type >= PSNET_NUM_TYPES) ) {
		return -1;
	}

	// make sure we have enough storage size for return addr
	if ( *fromlen < static_cast<int>(sizeof(addr)) ) {
		Int3();
		return -1;
	}

	l = &Psnet_top_buffers[psnet_type];

	// if we have no buffer! The user should have made sure this wasn't the case by calling SELECT()
	ret = psnet_buffer_get_next(l, reinterpret_cast<ubyte *>(buf), &ret_len, &addr);

	if ( !ret ) {
		Int3();
		return -1;
	}

	// otherwise, stuff the outgoing data
	memcpy(from, &addr, sizeof(addr));
	*fromlen = sizeof(addr);

	// return bytes read
	return static_cast<int>(ret_len);
}

/**
 * Wrappers around select() and recvfrom() for lagging/losing data
 */
int SELECT(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout, int psnet_type)
{
	network_packet_buffer_list *l;

	// if this is a check for writability, just return the select 
	if (writefds != nullptr) {
		return select(nfds, readfds, writefds, exceptfds, timeout);
	}
	
	// bad type
	Assert( (psnet_type >= 0) && (psnet_type < PSNET_NUM_TYPES) );

	if ( (psnet_type < 0) || (psnet_type >= PSNET_NUM_TYPES) ) {
		return -1;
	}

	l = &Psnet_top_buffers[psnet_type];

	// do we have any buffers in here?
	if ( (l->psnet_lowest_id == -1) || (l->psnet_lowest_id > l->psnet_highest_id) ) {
		if (readfds) {
			FD_ZERO(readfds);
		}

		if (exceptfds) {
			FD_ZERO(exceptfds);
		}

		return 0;
	}

	// yo
	return 1;
}

/**
 * Wrappers around sendto to sorting through different packet types
 */
int SENDTO(SOCKET s, char * buf, int len, int flags, SOCKADDR *to, int tolen, int psnet_type)
{
	char outbuf[MAX_TOP_LAYER_PACKET_SIZE];

	Assert(len < MAX_TOP_LAYER_PACKET_SIZE);

	// stuff type
	outbuf[0] = static_cast<char>(psnet_type);
	memcpy(&outbuf[1], buf, static_cast<size_t>(len));

	// send it
	return static_cast<int>( sendto(s, outbuf, len + 1, flags, reinterpret_cast<LPSOCKADDR>(to), tolen) );
}

/**
 * Call this once per frame to read everything off of our socket
 */
void PSNET_TOP_LAYER_PROCESS()
{
	// read socket stuff
	fd_set rfds;
	timeval timeout;
	SSIZE_T read_len;
	socklen_t from_len;
	SOCKADDR_IN6 from_addr;
	uint8_t packet_data[MAX_TOP_LAYER_PACKET_SIZE];

	if ( !Psnet_active ) {
		return;
	}

	// clear the addresses to remove compiler warnings
	memset(&from_addr, 0, sizeof(from_addr));

	while (true) {
		// check if there is any data on the socket to be read.  The amount of data that can be 
		// atomically read is stored in len.

		FD_ZERO(&rfds);
		FD_SET(Psnet_socket, &rfds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		if ( select(static_cast<int>(Psnet_socket + 1), &rfds, nullptr, nullptr, &timeout) == SOCKET_ERROR ) {
			ml_printf("Error %d doing a socket select on read", WSAGetLastError());
			break;
		}

		// if the read file descriptor is not set, then bail!
		if ( !FD_ISSET(Psnet_socket, &rfds) ) {
			return;
		}

		// get data off the socket and process
		from_len = sizeof(from_addr);
		read_len = recvfrom(Psnet_socket, reinterpret_cast<char *>(packet_data), sizeof(packet_data),
							0, reinterpret_cast<LPSOCKADDR>(&from_addr), &from_len);

		if (read_len <= 0) {
			if (read_len == -1) {
				ml_string("Socket error on socket_get_data()");
			}

			break;
		}

		// determine the packet type
		int packet_type = packet_data[0];

		if ( (packet_type >= 0) && (packet_type < PSNET_NUM_TYPES) ) {
			// buffer the packet
			psnet_buffer_packet(&Psnet_top_buffers[packet_type], packet_data + 1, read_len - 1, &from_addr);
		} else {
			// got something that's definitely not from a psnet client, so dump it
			psnet_debug_bad_packet(packet_type, packet_data, from_len, &from_addr);
		}
	}
}


// -------------------------------------------------------------------------------------------------------
// PSNET 2 FUNCTIONS
//

/**
 * Initialize psnet to use the specified port
 */
void psnet_init(uint16_t port_num)
{	
	int idx;

	if (Psnet_active) {
		return;
	}

	Psnet_connection = NETWORK_CONNECTION_LAN;

	Network_status = NETWORK_STATUS_NO_PROTOCOL;

#ifdef _WIN32
	WSADATA wsa_data;
	WORD ws_ver;

	ws_ver = MAKEWORD(2, 2);

	if ( WSAStartup(ws_ver, &wsa_data) ) {
		return;
	}
#endif

	// get the port for running this game on.  Be careful that it cannot be out of bounds
	Psnet_default_port = DEFAULT_GAME_PORT;

	if (port_num > 1023) {
		Psnet_default_port = port_num;
	}

	// initialize all packet type buffers
	for (idx = 0; idx < PSNET_NUM_TYPES; idx++) {
		psnet_buffer_init(&Psnet_top_buffers[idx]);
	}

	// do this before socket init
	psnet_init_my_addr();

	// initialize UDP now
	if ( !psnet_init_socket() ) {
		ml_printf("Error on UDP startup %d", Psnet_failure_code);
		return;
	}

	psnet_init_rel_tcp();

	Psnet_active = true;

	// specified network timeout
	Nettimeout = NETTIMEOUT;

	if (Cmdline_timeout > 0) {
		Nettimeout = Cmdline_timeout;
	}

	Network_status = NETWORK_STATUS_RUNNING;
}

/**
 * Shutdown psnet
 */
void psnet_close()
{
	if ( !Psnet_active ) {
		return;
	}

	// close down all reliable sockets - this forces them to
	// send a disconnect to any remote machines
	psnet_rel_close();

	if (Psnet_socket != INVALID_SOCKET) {
		shutdown(Psnet_socket, 1);
		closesocket(Psnet_socket);
	}

	Psnet_active = false;
	Network_status = NETWORK_STATUS_NOT_INITIALIZED;

#ifdef _WIN32
	WSACleanup();
#endif
}

/**
 * Test whether psnet is ready or not
 */
bool psnet_is_active()
{
	return Psnet_active;
}

/**
 * Initialize my addr, ip, etc.
 */
bool psnet_init_my_addr()
{
	SOCKADDR_STORAGE remote_addr;
	SOCKADDR_IN6 local_addr_ipv4, local_addr_ipv6;
	char ip_string[INET6_ADDRSTRLEN];
	socklen_t addrlen;
	SOCKET tsock;
	int rval;
	const char *local_ip;

	// zero out my address
	Psnet_my_ip.clear();
	memset(&Psnet_my_addr, 0, sizeof(Psnet_my_addr));
	Psnet_my_addr.port = Psnet_default_port;	// set this just in case we bail

	local_addr_ipv4.sin6_family = AF_UNSPEC;
	local_addr_ipv4.sin6_addr = IN6ADDR_ANY_INIT;
	local_addr_ipv6.sin6_family = AF_UNSPEC;
	local_addr_ipv6.sin6_addr = IN6ADDR_ANY_INIT;

	//
	// run through some public DNS servers to try and populate the routing table
	// which should load the socket with our local interface IP address
	//
	// IPv6 and IPv4 checks are separate so we can get both addresses
	// it's safe for either one to fail
	//

	// IPv6 & IPv4 -> Cloudflare, Google Public DNS, Quad9
	const SCP_vector<SCP_string> remote_hosts_ipv6 = { "2606:4700:4700::1111", "2001:4860:4860::8888", "2620:fe::fe" };
	const SCP_vector<SCP_string> remote_hosts_ipv4 = { "1.1.1.1", "8.8.8.8", "9.9.9.9" };

	// IPv6 check (should be done first!!)
	tsock = socket(AF_INET6, SOCK_DGRAM, 0);

	if (tsock == INVALID_SOCKET) {
		ml_string("Error creating IPv6 socket, unable to determine local IP");
	} else {
		for (const auto &host : remote_hosts_ipv6) {
			if ( !psnet_get_addr(host.c_str(), 53, &remote_addr) ) {
				continue;
			}

			rval = connect(tsock, reinterpret_cast<LPSOCKADDR>(&remote_addr), sizeof(remote_addr));

			if (rval) {
				continue;
			}

			// we've connected, so the socket should have a routable IP now
			addrlen = sizeof(local_addr_ipv6);
			rval = getsockname(tsock, reinterpret_cast<LPSOCKADDR>(&local_addr_ipv6), &addrlen);

			if ( !rval ) {
				break;
			}
		}

		if ( !IN6_IS_ADDR_UNSPECIFIED(&local_addr_ipv6.sin6_addr) ) {
			// add to ip list
			Psnet_my_ip.push_back(local_addr_ipv6.sin6_addr);
		}

		shutdown(tsock, 1);
		closesocket(tsock);
	}

	// IPv4 check
	tsock = socket(AF_INET6, SOCK_DGRAM, 0);

	if (tsock == INVALID_SOCKET) {
		ml_string("Error creating IPv4 socket, unable to determine local IP");
	} else {
		// make sure we are in dual-stack mode (not the default on Windows)
		int i_opt = 0;
		setsockopt(tsock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&i_opt), sizeof(i_opt));

		for (const auto &host : remote_hosts_ipv4) {
			if ( !psnet_get_addr(host.c_str(), 53, &remote_addr) ) {
				continue;
			}

			rval = connect(tsock, reinterpret_cast<LPSOCKADDR>(&remote_addr), sizeof(remote_addr));

			if (rval) {
				continue;
			}

			// we've connected, so the socket should have a routable IP now
			addrlen = sizeof(local_addr_ipv4);
			rval = getsockname(tsock, reinterpret_cast<LPSOCKADDR>(&local_addr_ipv4), &addrlen);

			if ( !rval ) {
				break;
			}
		}

		if ( !IN6_IS_ADDR_UNSPECIFIED(&local_addr_ipv4.sin6_addr) ) {
			// add to ip list
			Psnet_my_ip.push_back(local_addr_ipv4.sin6_addr);
		}

		shutdown(tsock, 1);
		closesocket(tsock);
	}

	//
	// rest of this just normalizes and populates the internal structures
	//

	// ?? oops
	if ( Psnet_my_ip.empty() ) {
		ml_string("Local interface IP address => <undetermined>");
		return false;
	}

	if (Psnet_my_ip.size() > 1) {
		Psnet_ip_mode = PSNET_IP_MODE_DUAL;
	} else if ( IN6_IS_ADDR_V4MAPPED(&Psnet_my_ip[0]) ) {
		Psnet_ip_mode = PSNET_IP_MODE_V4;
	} else {
		Psnet_ip_mode = PSNET_IP_MODE_V6;
	}

	// prefer IPv4 address if avaiable (last entry), for maximum compatibility
	// TODO: something more robust probably needs to happen with this
	memcpy(&Psnet_my_addr.addr, &Psnet_my_ip.back(), sizeof(in6_addr));
	Psnet_my_addr.port = Psnet_default_port;

	// log results to multi.log
	for (auto &in6 : Psnet_my_ip) {
		local_ip = inet_ntop(AF_INET6, psnet_mask_addr(&in6), ip_string, sizeof(ip_string));

		if (local_ip) {
			ml_printf("Local interface address => %s", local_ip);
		}
	}

	if ( (Psnet_ip_mode == PSNET_IP_MODE_DUAL) && (Cmdline_prefer_ipv4 || Cmdline_prefer_ipv6) ) {
		ml_printf("Prefering IPv%d connections where possible", Cmdline_prefer_ipv4 ? 4 : 6);
	}

	return true;
}

/**
 * Get the status of the network
 */
int psnet_get_network_status()
{
	// first case is when "none" is selected
	if (Psnet_connection == NETWORK_CONNECTION_NONE) {
		return NETWORK_ERROR_NO_TYPE;
	}

	// first, check the connection status of the network
	if (Network_status == NETWORK_STATUS_NO_WINSOCK) {
		return NETWORK_ERROR_NO_WINSOCK;
	}

	if (Network_status == NETWORK_STATUS_NO_PROTOCOL) {
		return NETWORK_ERROR_NO_PROTOCOL;
	}

	return NETWORK_ERROR_NONE;
}

/**
 * Convert a ::net_addr to a string
 */
const char *psnet_addr_to_string(const net_addr *address, char *text, size_t max_len)
{
	in6_addr addr6;

	memcpy(&addr6, &address->addr, sizeof(addr6));

	if ( inet_ntop(AF_INET6, &addr6, text, static_cast<socklen_t>(max_len)) == nullptr ) {
		strncpy(text, "", max_len);
	}

	return text;
}

/**
 * Convert a string to a net addr
 */
bool psnet_string_to_addr(const char *text, net_addr *address)
{
	SCP_string host, port;
	SOCKADDR_STORAGE sockaddr;
	auto *addr6 = reinterpret_cast<SOCKADDR_IN6 *>(&sockaddr);

	memset(address, 0, sizeof(*address));

	if ( !psnet_explode_ip_string(text, host, port) ) {
		return false;
	}

	if ( !psnet_get_addr(host.c_str(), DEFAULT_GAME_PORT, &sockaddr) ) {
		return false;
	}

	memcpy(&address->addr, &addr6->sin6_addr, sizeof(address->addr));

	if ( !port.empty() ) {
		try {
			int port_num = std::stoi(port);

			if ( (port_num >= 0) && (port_num <= USHRT_MAX) ) {
				address->port = static_cast<uint16_t>(port_num);
			}
		} catch (...) {
			mprintf(("Invalid port number in psnet_string_to_addr()\n"));
		}
	}

	return true;
}

/**
 * Compare 2 addresses
 */
int psnet_same(SOCKADDR_IN6 *a1, SOCKADDR_IN6 *a2)
{
	return (a1->sin6_port == a2->sin6_port) && IN6_ARE_ADDR_EQUAL(&a1->sin6_addr, &a2->sin6_addr);
}

int psnet_same(net_addr *a1, net_addr *a2)
{
	return (a1->port == a2->port) && !memcmp(&a1->addr, &a2->addr, sizeof(a1->addr));
}

/**
 * Compare against possible address of this machine
 */

bool psnet_is_local_addr(const net_addr *addr)
{
	// if port is different then skip the rest of the tests
	if (addr->port != Psnet_default_port) {
		return false;
	}

	auto *sin6_addr = reinterpret_cast<const in6_addr *>(&addr->addr);

	// IPv6 loopback
	if (IN6_IS_ADDR_LOOPBACK(sin6_addr)) {
		return true;
	}

	// IPv4 loopback
	if (IN6_IS_ADDR_V4MAPPED(sin6_addr)) {
		if (reinterpret_cast<const uint32_t *>(sin6_addr)[3] == INADDR_LOOPBACK) {
			return true;
		}
	}

	// identified local interfaces
	for (auto &in6 : Psnet_my_ip) {
		if (IN6_ARE_ADDR_EQUAL(sin6_addr, &in6)) {
			return true;
		}
	}

	return false;
}

/**
 * Get IP mode (IPv4, IPv6, dual stack...)
 */
int psnet_get_ip_mode()
{
	return Psnet_ip_mode;
}

/**
 * Send data unreliably
 */
int psnet_send(net_addr *who_to_addr, void *data, int len, int np_index)	// NOLINT(misc-unused-parameters)
{
	// send data unreliably
	int ret;
	fd_set wfds;
	struct timeval timeout;
	SOCKADDR_IN6 who_to;

	if ( !Psnet_active ) {
		ml_string("Network ==> Socket not inited in psnet_send");
		return 0;
	}

	psnet_addr_to_sockaddr(who_to_addr, &who_to);

	if ( IN6_IS_ADDR_UNSPECIFIED(&who_to.sin6_addr) ) {
		ml_string("Network ==> send to address is 0 in psnet_send");
		return 0;
	}

	if (who_to.sin6_port == 0) {
		ml_printf("Network ==> destination port %d invalid in psnet_send", ntohs(who_to.sin6_port));
		return 0;
	}

	if ( psnet_is_local_addr(who_to_addr) ) {
		return 0;
	}

	FD_ZERO(&wfds);
	FD_SET(Psnet_socket, &wfds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if ( SELECT(static_cast<int>(Psnet_socket+1), nullptr, &wfds, nullptr, &timeout, PSNET_TYPE_UNRELIABLE) == SOCKET_ERROR ) {
		ml_printf("Error on blocking select for write %d", WSAGetLastError());
		return 0;
	}

	// if the write file descriptor is not set, then bail!
	if ( !FD_ISSET(Psnet_socket, &wfds) ) {
		return 0;
	}

	multi_rate_add(np_index, "udp(h)", len + UDP_HEADER_SIZE);
	multi_rate_add(np_index, "udp", len);

	ret = SENDTO(Psnet_socket, reinterpret_cast<char *>(data), len, 0,
				 reinterpret_cast<LPSOCKADDR>(&who_to), sizeof(who_to),
				 PSNET_TYPE_UNRELIABLE);

	if (ret != SOCKET_ERROR) {
		return 1;
	}

	return 0;
}

/**
 * Get data from the unreliable socket
 */
int psnet_get(void *data, net_addr *addr)
{
	SSIZE_T buffer_size;
	SOCKADDR_IN6 from_addr;

	if ( !Psnet_active ) {
		return 0;
	}

	// try and get a free buffer and return its size
	if ( psnet_buffer_get_next(&Psnet_top_buffers[PSNET_TYPE_UNRELIABLE], reinterpret_cast<ubyte *>(data), &buffer_size, &from_addr) ) {
		psnet_sockaddr_to_addr(&from_addr, addr);
		return static_cast<int>(buffer_size);
	}

	// return nothing
	return 0;
}

/**
 * Flush all sockets
 */
void psnet_flush()
{
	ubyte data[MAX_TOP_LAYER_PACKET_SIZE];
	net_addr from_addr;

	if ( !Psnet_active ) {
		return;
	}

	while ( psnet_get(data, &from_addr) > 0 );
}

/**
 * Convert from sockaddr_in6 to net_addr struct
 */
static void psnet_sockaddr_to_addr(const SOCKADDR_IN6 *sockaddr, net_addr *addr)
{
	memcpy(&addr->addr, &sockaddr->sin6_addr, sizeof(addr->addr));
	addr->port = ntohs(sockaddr->sin6_port);
}

/**
 * Convert from net_addr struct to sockaddr_in6
 */
static void psnet_addr_to_sockaddr(const net_addr *addr, SOCKADDR_IN6 *sockaddr)
{
	memset(sockaddr, 0, sizeof(*sockaddr));

	memcpy(&sockaddr->sin6_addr, &addr->addr, sizeof(sockaddr->sin6_addr));

	sockaddr->sin6_family = AF_INET6;
	sockaddr->sin6_port = htons(addr->port);
}

/**
 * Convert from sockaddr_storage struct to sockaddr_in6
 */
static void psnet_sockaddr_storage_to_in6(const SOCKADDR_STORAGE *addr, SOCKADDR_IN6 *addr_in6)
{
	if (addr->ss_family == AF_INET6) {
		memcpy(addr_in6, addr, sizeof(SOCKADDR_IN6));
	} else {
		Assertion(addr->ss_family == AF_INET, "Invalid sockaddr type (%d)!", addr->ss_family);
		auto *in4 = reinterpret_cast<const SOCKADDR_IN *>(addr);

		memset(addr_in6, 0, sizeof(*addr_in6));

		addr_in6->sin6_family = AF_INET6;
		addr_in6->sin6_port = in4->sin_port;
		psnet_map4to6(&in4->sin_addr, &addr_in6->sin6_addr);
	}
}

/**
 * Helper to map IPv4 to IPv6
 */
void psnet_map4to6(const in_addr *in4, in6_addr *in6)
{
	if (in4->s_addr == INADDR_ANY) {
		memcpy(in6, &in6addr_any, sizeof(in6_addr));
	} else {
		reinterpret_cast<uint32_t *>(in6)[0] = 0;
		reinterpret_cast<uint32_t *>(in6)[1] = 0;
		reinterpret_cast<uint32_t *>(in6)[2] = htonl(0xffff);
		reinterpret_cast<uint32_t *>(in6)[3] = in4->s_addr;
	}
}

/**
 * Helper to map IPv6 to IPv4
 */
bool psnet_map6to4(const in6_addr *in6, in_addr *in4)
{
	if ( !IN6_IS_ADDR_V4MAPPED(in6) ) {
		return false;
	}

	in4->s_addr = reinterpret_cast<const uint32_t *>(in6)[3];

	return true;
}

/**
 * Helper to anonymize IP address for logging purposes
 */
in6_addr *psnet_mask_addr(const in6_addr *inaddr)
{
	static in6_addr addr;
	size_t nbytes = 0;

	memcpy(&addr, inaddr, sizeof(in6_addr));

	if ( IN6_IS_ADDR_V4MAPPED(&addr) ) {
		// zero last octet of IPv4 address
		// (last byte)
		nbytes = 1;
	} else {
		// zero last 80 bits of IPv6 address
		// (last 10 bytes)
		nbytes = 10;
	}

	memset(addr.s6_addr+(sizeof(addr)-nbytes), 0, nbytes);

	return &addr;
}

/**
 * Helper to quickly test if ip string is in standard notation
 */
static bool psnet_is_ip_notation(int af, const char *ip_string)
{
	uint8_t addr[sizeof(in6_addr)];

	if (ip_string == nullptr) {
		return false;
	}

	switch (af) {
		case AF_INET6:
		case AF_INET: {
			if ( inet_pton(af, ip_string, addr) == 1 ) {
				return true;
			}

			break;
		}

		default: {
			if ( inet_pton(AF_INET6, ip_string, addr) == 1 ) {
				return true;
			}

			if ( inet_pton(AF_INET, ip_string, addr) == 1 ) {
				return true;
			}

			break;
		}
	}

	return false;
}

/**
 * Helper to extract the ip address and port number from a string
 */
static bool psnet_explode_ip_string(const char *ip_string, SCP_string &host, SCP_string &port)
{
	SCP_string ip;

	host.clear();
	port.clear();

	if (ip_string == nullptr) {
		return false;
	}

	ip = ip_string;

	auto obracket = ip.find_first_of('[');
	auto colon = ip.find_first_of(':');
	auto dot = ip.find_first_of('.');

	//
	// look for IPv6 formatted string first
	//

	// check for brackets and colon (ip with port)
	if ( (obracket != SCP_string::npos) && (colon != SCP_string::npos) ) {
		auto cbracket = ip.find_last_of(']');

		// no closing bracket... fail
		if (cbracket == SCP_string::npos) {
			return false;
		}

		// closing bracket before opening bracket... fail
		if (cbracket < obracket) {
			return false;
		}

		// colon not between brackets... fail
		if ( (colon < obracket) || (colon > cbracket) ) {
			return false;
		}

		// ok, assume ipv6 and grab inside of brackets as address
		host = ip.substr(obracket + 1, cbracket - obracket - 1);

		// now check for port number after closing bracket
		auto lcolon = ip.find_last_of(':');

		if ( (lcolon != SCP_string::npos) && (cbracket < lcolon) ) {
			port = ip.substr(lcolon + 1);
		}

		return true;
	}

	// if it has no dots then assume it's just IPv6
	if (dot == SCP_string::npos) {
		host = ip;
		return true;
	}

	// if colon before dots then it's likely mapped IPv4
	if ( (colon != SCP_string::npos) && (dot != SCP_string::npos) && (colon < dot) ) {
		host = ip;
		return true;
	}

	// otherwise it should be IPv4
	// if it has a port then split it off
	if (colon != SCP_string::npos) {
		host = ip.substr(0, colon);
		port = ip.substr(colon + 1);
	} else {
		host = ip;
	}

	return true;
}

/**
 * If the passed string is a valid IP string
 */
bool psnet_is_valid_ip_string(const char *ip_string)
{
	SCP_string ip;
	SCP_string host, port;

	if (ip_string == nullptr) {
		return false;
	}

	// split string into host & port
	if ( !psnet_explode_ip_string(ip_string, host, port) ) {
		return false;
	}

	// if host is standard notation then we're done
	if ( psnet_is_ip_notation(AF_UNSPEC, ip_string) ) {
		return true;
	}

	// final check, may have to do DNS lookup on it
	return psnet_get_addr(host.c_str(), nullptr, nullptr);
}

/**
 * Get valid sockaddr structure
 */
bool psnet_get_addr(const char *host, const char *port, SOCKADDR_STORAGE *addr, int flags)
{
	struct addrinfo hints, *srvinfo = nullptr;
	bool success = false;
	SOCKADDR_IN6 si4to6;
	int rval;

	memset(&si4to6, 0, sizeof(si4to6));

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_flags = AI_V4MAPPED;

	if (flags & ADDR_FLAG_NUMERIC_SERVICE) {
		hints.ai_flags |= AI_NUMERICSERV;
	}

	if (host == nullptr) {
		hints.ai_flags |= AI_PASSIVE;
	} else {
		// avoid dns lookups if we can
		if ( psnet_is_ip_notation(AF_UNSPEC, host) ) {
			hints.ai_flags |= AI_NUMERICHOST;
		}
		// skip AAAA lookups if we can't use them
		else if ( !(Psnet_ip_mode & PSNET_IP_MODE_V6) ) {
			hints.ai_family = AF_INET;
		}
		// skip A lookups if we can't use them
		else if ( !(Psnet_ip_mode & PSNET_IP_MODE_V4) ) {
			hints.ai_family = AF_INET6;
		}
	}

	if ( (rval = getaddrinfo(host, port, &hints, &srvinfo)) != 0 ) {
		ml_printf("getaddrinfo() => %s", gai_strerror(rval));

		if (srvinfo) {
			freeaddrinfo(srvinfo);
		}

		return false;
	}

	if (Cmdline_prefer_ipv4) {
		flags |= ADDR_FLAG_PREFER_IPV4;
	} else if (Cmdline_prefer_ipv6) {
		flags &= ~ADDR_FLAG_PREFER_IPV4;
	}

	const bool prefer_v6 = ((Psnet_ip_mode == PSNET_IP_MODE_DUAL) && Cmdline_prefer_ipv6);
	const bool prefer_v4 = ((Psnet_ip_mode == PSNET_IP_MODE_DUAL) && (flags & ADDR_FLAG_PREFER_IPV4));

	for (auto *srv = srvinfo; srv != nullptr; srv = srv->ai_next) {
		if ( (srv->ai_family == AF_INET) || (srv->ai_family == AF_INET6) ) {
			if (addr) {
				// map ipv4 to ipv6
				if (srv->ai_family == AF_INET) {
					auto *si4 = reinterpret_cast<SOCKADDR_IN *>(srv->ai_addr);

					si4to6.sin6_family = AF_INET6;
					si4to6.sin6_port = si4->sin_port;

					psnet_map4to6(&si4->sin_addr, &si4to6.sin6_addr);

					memcpy(addr, &si4to6, sizeof(si4to6));
				} else if ( !success ) {
					memcpy(addr, srv->ai_addr, srv->ai_addrlen);
				}
			}

			success = true;

			// if we would prefer an IPv6 address then maybe keep looking
			if ( prefer_v6 && addr && (srv->ai_family == AF_INET) ) {
				continue;
			}

			// if we would prefer an IPv4 address then maybe keep looking
			if ( prefer_v4 && addr && (srv->ai_family == AF_INET6) ) {
				continue;
			}

			break;
		}
	}

	freeaddrinfo(srvinfo);

	return success;
}

bool psnet_get_addr(const char *host, const uint16_t port, SOCKADDR_STORAGE *addr, int flags)
{
	SCP_string port_str = std::to_string(port);

	return psnet_get_addr(host, port_str.c_str(), addr, flags | ADDR_FLAG_NUMERIC_SERVICE);
}

const in6_addr *psnet_get_local_ip(int af_type)
{
	switch (af_type) {
		case AF_INET: {
			if (Psnet_ip_mode == PSNET_IP_MODE_DUAL) {
				return &Psnet_my_ip[1];
			} else if (Psnet_ip_mode & PSNET_IP_MODE_V4) {
				return &Psnet_my_ip[0];
			}

			break;
		}

		case AF_INET6: {
			if (Psnet_ip_mode & PSNET_IP_MODE_V6) {
				return &Psnet_my_ip[0];
			}

			break;
		}

		default:
			break;
	}

	return nullptr;
}

/**
 * Log invalid packets for debug purposes.
 */
static void psnet_debug_bad_packet(const int packet_type, const uint8_t *packet_data, const SSIZE_T read_len, const SOCKADDR_IN6 *from_addr)
{
	// this could just be harmless junk, or not, but let's try to deal with it as gracefully as we can
	// to avoid log spam from bad actors we restrict logging to a limited number of packets per time window

	const time_t thistime = time(nullptr);

	++Psnet_bad_packet_count;

	if ( (Psnet_bad_packet_count > MAX_BAD_PACKETS_PER_WINDOW) && (Psnet_bad_packet_time >= thistime) ) {
		// packet flood, break!
		return;
	}

	// in case of flooding, log number of packets we've skipped during the previous window
	if (Psnet_bad_packet_count > MAX_BAD_PACKETS_PER_WINDOW) {
		ml_printf("WARNING: Invalid packet log window reset ... %lu non-logged packets received during previous window!", Psnet_bad_packet_count - MAX_BAD_PACKETS_PER_WINDOW);

		// reset count
		Psnet_bad_packet_count = 1;
	}

	if (Psnet_bad_packet_time <= thistime) {
		Psnet_bad_packet_time = thistime + MAX_BAD_PACKETS_WINDOW;
	}

	char from_string[INET6_ADDRSTRLEN] = "";
	std::stringstream dbg_string;

	inet_ntop(AF_INET6, &from_addr->sin6_addr, from_string, INET6_ADDRSTRLEN);

	dbg_string << "WARNING: Invalid packet type " << packet_type << " with length " << read_len << " received from [" << from_string << "]:" << ntohs(from_addr->sin6_port) << " ... ";

	// dump first 11 bytes for debugging (packet_type + 10 bytes)
	for (auto i = 0; (i < read_len) && (i < 11); ++i) {
		dbg_string << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(packet_data[i]);
	}

	ml_string(dbg_string.str().c_str());

	if (Psnet_bad_packet_count == MAX_BAD_PACKETS_PER_WINDOW) {
		ml_string("WARNING: Max invalid packet limit reached for this log window!");
	}
}

// -------------------------------------------------------------------------------------------------------
// PSNET 2 RELIABLE SOCKET FUNCTIONS
//

void psnet_rel_send_ack(SOCKADDR_IN6 *raddr, ushort sig, float time_sent)
{
	int ret;
	ushort sig_tmp;
	reliable_header ack_header;

	ack_header.type = RNT_ACK;
	ack_header.data_len = sizeof(ushort);
	ack_header.send_time = INTEL_FLOAT(&time_sent);

	sig_tmp = INTEL_SHORT(sig);
	memcpy(&ack_header.data, &sig_tmp, sizeof(ushort));

	ret = SENDTO(Psnet_socket, reinterpret_cast<char *>(&ack_header),
				 RELIABLE_PACKET_HEADER_ONLY_SIZE + ack_header.data_len, 0,
				 reinterpret_cast<LPSOCKADDR>(raddr), sizeof(*raddr), PSNET_TYPE_RELIABLE);

	if (ret == -1) {
		ml_string("TCP SENDTO failed in rel_send_ack()");
	}
}

/**
 * Function to shutdown and close the given socket.
 *
 * It takes a couple of things into consideration when closing, such as possibly 
 * reiniting reliable sockets if they are closed here.
 */
void psnet_rel_close_socket(PSNET_SOCKET_RELIABLE socketid)
{
	reliable_header diss_conn_header;
	reliable_socket *rsocket = nullptr;

	// if the socket is out of range
	if (socketid >= MAXRELIABLESOCKETS) {
		if (socketid != PSNET_INVALID_SOCKET) {
			ml_printf("Invalid socket id passed to psnet_rel_close_socket() -- %d", socketid);
		}

		return;
	}

	ml_printf("Closing socket %d", socketid);

	rsocket = &Reliable_sockets[socketid];

	// go through every buffer and "free it up(tm)"
	for (auto i = 0; i < MAXNETBUFFERS; i++) {
		if (rsocket->rbuffers[i]) {
			vm_free(rsocket->rbuffers[i]);

			rsocket->rbuffers[i] = nullptr;
			rsocket->rsequence[i] = 0;
		}

		if (rsocket->sbuffers[i]) {
			vm_free(rsocket->sbuffers[i]);

			rsocket->sbuffers[i] = nullptr;
			rsocket->ssequence[i] = 0;
		}
	}

	// send a disconnect packet to the socket on the other end
	diss_conn_header.type = RNT_DISCONNECT;
	diss_conn_header.seq = CONNECTSEQ;
	diss_conn_header.data_len = 0;

	if (socketid == Serverconn) {
		Serverconn = 0xffffffff;
	}

	SENDTO(Psnet_socket, reinterpret_cast<char *>(&diss_conn_header), RELIABLE_PACKET_HEADER_ONLY_SIZE,
		   0, reinterpret_cast<LPSOCKADDR>(&rsocket->addr), sizeof(rsocket->addr), PSNET_TYPE_RELIABLE);

	memset(rsocket, 0, sizeof(reliable_socket));
	rsocket->status = RNF_UNUSED;
}

/**
 * Send data reliably
 */
int psnet_rel_send(PSNET_SOCKET_RELIABLE socketid, ubyte *data, int length, int np_index)	// NOLINT(misc-unused-parameters)
{
	int i;
	int bytesout = 0;
	reliable_socket *rsocket;
	
	if (socketid >= MAXRELIABLESOCKETS) {
		ml_printf("Invalid socket id passed to psnet_rel_send() -- %d", socketid);
		return -1;
	}

	Assert(length <= MAX_PACKET_SIZE);

	psnet_rel_work();

	rsocket = &Reliable_sockets[socketid];

	if (rsocket->status != RNF_CONNECTED) {
		// We can't send because this isn't a connected reliable socket.
		ml_printf("Can't send packet because of status %d in psnet_rel_send(). socket = %d", rsocket->status, socketid);
		return -1;
	}
	
	// Add the new packet to the sending list and send it.
	for (i = 0; i < MAXNETBUFFERS; i++) {
		if (rsocket->sbuffers[i] == nullptr) {
			reliable_header send_header;
			int send_this_packet = 1;

			rsocket->send_len[i] = length;
			rsocket->sbuffers[i] = reinterpret_cast<reliable_net_buffer *>(vm_malloc(sizeof(reliable_net_buffer)));

			memcpy(rsocket->sbuffers[i]->buffer, data, static_cast<size_t>(length));

			send_header.seq = INTEL_SHORT(rsocket->theirsequence);
			rsocket->ssequence[i] = rsocket->theirsequence;

			memcpy(send_header.data, data, static_cast<size_t>(length));

			send_header.data_len = INTEL_SHORT( static_cast<ushort>(length) );
			send_header.type = RNT_DATA;
			send_header.send_time = psnet_get_time();
			send_header.send_time = INTEL_FLOAT( &send_header.send_time ) ;

			if (send_this_packet) {
				multi_rate_add(np_index, "tcp(h)", RELIABLE_PACKET_HEADER_ONLY_SIZE+rsocket->send_len[i]);

				bytesout = SENDTO(Psnet_socket, reinterpret_cast<char *>(&send_header),
								  static_cast<int>(RELIABLE_PACKET_HEADER_ONLY_SIZE) + rsocket->send_len[i], 0,
								  reinterpret_cast<LPSOCKADDR>(&rsocket->addr), sizeof(rsocket->addr),
								  PSNET_TYPE_RELIABLE);
			}

			if ( (bytesout == SOCKET_ERROR) && (WSAGetLastError() == WSAEWOULDBLOCK) ) {
				// This will cause it to try to send again next frame. (or sooner)
				rsocket->timesent[i] = psnet_get_time() - (NETRETRYTIME*4);
			} else {
				rsocket->timesent[i] = psnet_get_time();
			}

			rsocket->theirsequence++;

			return bytesout;
		}
	}

	ml_printf("PSNET RELIABLE SEND BUFFER OVERRUN. socket = %d", socketid);

	return 0;
}

// Return codes:
// -1 socket not connected
// 0 No packet ready to receive
// >0 Buffer filled with the number of bytes recieved
int psnet_rel_get(PSNET_SOCKET socketid, ubyte *buffer, int max_length)
{
	reliable_socket *rsocket = nullptr;

	psnet_rel_work();

	if (socketid >= MAXRELIABLESOCKETS) {
		ml_printf("Invalid socket id passed to psnet_rel_get() -- %d", socketid);
		return -1;
	}

	rsocket = &Reliable_sockets[socketid];

	if ( (rsocket->status != RNF_CONNECTED) && (rsocket->status != RNF_LIMBO) ) {
		ml_printf("Can't receive packet because it isn't connected in psnet_rel_get(). socket = %d", socketid);
		return 0;
	}

	// If the buffer position is the position we are waiting for, fill in
	// the buffer we received in the call to this function and return true

	for (auto i = 0;  i < MAXNETBUFFERS; i++) {
		if ( (rsocket->rsequence[i] == rsocket->oursequence) && rsocket->rbuffers[i] ) {
			Assert(max_length >= rsocket->recv_len[i]);

			if (max_length < rsocket->recv_len[i]) {
				ml_printf("Data too large for buffer - size %d, max %d. socket = %d",
						  rsocket->recv_len[i], max_length, socketid);

				return 0;
			}

			memcpy(buffer, rsocket->rbuffers[i]->buffer, static_cast<size_t>(rsocket->recv_len[i]));

			vm_free(rsocket->rbuffers[i]);
			rsocket->rbuffers[i] = nullptr;

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
	int i, j;
	int rcode = -1;
	fd_set read_fds;
	struct timeval timeout;
	reliable_header rcv_buff;
	SOCKADDR_STORAGE rcv_addr;
	auto *addr6 = reinterpret_cast<SOCKADDR_IN6 *>(&rcv_addr);
	char addr_string[INET6_ADDRSTRLEN];

	if ( !Psnet_active ) {
		return;
	}

	PSNET_TOP_LAYER_PROCESS();

	// negotitate initial connection with the server
	reliable_socket *rsocket = nullptr;

	if (Serverconn != 0xffffffff) {
		// Check to see if we need to send a packet out.
		if ( (Reliable_sockets[Serverconn].status == RNF_LIMBO) &&
			 ((Serverconn != 0xffffffff) && fl_abs((psnet_get_time() - Last_sent_iamhere)) > NETRETRYTIME) )
		{
			rsocket = &Reliable_sockets[Serverconn];

			reliable_header conn_header;

			// Now send I_AM_HERE packet
			conn_header.type = RNT_I_AM_HERE;
			conn_header.seq = static_cast<ushort>(~CONNECTSEQ);
			conn_header.data_len = 0;

			Last_sent_iamhere = psnet_get_time();

			int ret = SENDTO(Psnet_socket, reinterpret_cast<char *>(&conn_header), RELIABLE_PACKET_HEADER_ONLY_SIZE, 0,
							 reinterpret_cast<LPSOCKADDR>(&rsocket->addr), sizeof(rsocket->addr), PSNET_TYPE_RELIABLE);


			if ( (ret == SOCKET_ERROR) && (WSAGetLastError() == WSAEWOULDBLOCK) ) {
				rsocket->last_packet_sent = psnet_get_time() - NETRETRYTIME;
			} else {
				rsocket->last_packet_sent = psnet_get_time();
			}
		}
	}

	do {
		rsocket = nullptr;

		// Check UDP

		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(Psnet_socket, &read_fds);

		if ( SELECT(static_cast<int>(Psnet_socket+1), &read_fds, nullptr, nullptr, &timeout, PSNET_TYPE_RELIABLE) == SOCKET_ERROR ) {
			break;
		}

		// if the file descriptor is not set, then bail!
		if ( !FD_ISSET(Psnet_socket, &read_fds) ) {
			break;
		}

		memset(&rcv_addr, 0, sizeof(rcv_addr));
		int addrlen = sizeof(rcv_addr);

		int bytesin = RECVFROM(Psnet_socket, reinterpret_cast<char *>(&rcv_buff), sizeof(reliable_header), 0,
							   reinterpret_cast<LPSOCKADDR>(&rcv_addr), &addrlen, PSNET_TYPE_RELIABLE);

		if (bytesin == 0) {
			break;
		}

		if (bytesin == -1) {
			ml_printf("recvfrom returned an error! -- %d",WSAGetLastError());

			break;
		}

		rcv_buff.seq = INTEL_SHORT( rcv_buff.seq ); //-V570
		rcv_buff.data_len = INTEL_SHORT( rcv_buff.data_len ); //-V570
		rcv_buff.send_time = INTEL_FLOAT( &rcv_buff.send_time );

		// Someone wants to connect, so find a slot
		if (rcv_buff.type == RNT_REQ_CONN) {
			for (i = 1; i < MAXRELIABLESOCKETS; i++) {
				if ( (Reliable_sockets[i].status == RNF_CONNECTED) || (Reliable_sockets[i].status == RNF_LIMBO) ) {
					if ( psnet_same(addr6, &Reliable_sockets[i].addr) ) {
						// We already have a reliable link to this user, so we will ignore it...
						ml_printf("Received duplicate connection request. %d",i);

						psnet_rel_send_ack(&Reliable_sockets[i].addr, rcv_buff.seq, rcv_buff.send_time);

						// We will change this as a hack to prevent later code from hooking us up
						rcv_buff.type = 0xff;

						continue;
					}
				}
			}

			for (i = 1; i < MAXRELIABLESOCKETS; i++) {
				if (Reliable_sockets[i].status == RNF_UNUSED) {
					// Add the new connection here.
					rsocket = &Reliable_sockets[i];

					psnet_sockaddr_storage_to_in6(&rcv_addr, &Reliable_sockets[i].addr);

					rsocket->ping_pos = 0;
					rsocket->num_ping_samples = 0;
					rsocket->status = RNF_LIMBO;
					rsocket->last_packet_received = psnet_get_time();

					const char *ip_string = inet_ntop(AF_INET6, psnet_mask_addr(&addr6->sin6_addr), addr_string, sizeof(addr_string));
					ml_printf("Connect from [%s]:%d", ip_string ? ip_string : "unknown", ntohs(addr6->sin6_port));

					break;
				}
			}

			if (i == MAXRELIABLESOCKETS) {
				// No more connections!
				ml_string("Out of incoming reliable connection sockets");

				continue;
			}

			psnet_rel_send_ack(&rsocket->addr, rcv_buff.seq, rcv_buff.send_time);
		}

		// Find out if this is a packet from someone we were expecting a packet.
		for (i = 1; i < MAXRELIABLESOCKETS; i++) {
			if ( psnet_same(addr6, &Reliable_sockets[i].addr) ) {
				rsocket = &Reliable_sockets[i];

				break;
			}
		}

		if (rsocket == nullptr) {
			const char *ip_string = inet_ntop(AF_INET6, psnet_mask_addr(&addr6->sin6_addr), addr_string, sizeof(addr_string));
			ml_string("Received reliable data from unconnected client.");
			ml_printf("Received from [%s]:%d\n", ip_string ? ip_string : "unknown", ntohs(addr6->sin6_port));

			continue;
		}

		rsocket->last_packet_received = psnet_get_time();

		if (rsocket->status != RNF_CONNECTED) {
			// Get out of limbo
			if (rsocket->status == RNF_LIMBO) {
				// this is our connection to the server
				if (Serverconn != 0xffffffff) {
					if (rcv_buff.type == RNT_ACK) {
						auto *acknum = reinterpret_cast<ushort *>(&rcv_buff.data);

						if (INTEL_SHORT(*acknum) == (~CONNECTSEQ & 0xffff)) {
							rsocket->status = RNF_CONNECTED;
							ml_string("Got ACK for IAMHERE!");
						}

						continue;
					}
				} else if (rcv_buff.type == RNT_I_AM_HERE) {
					rsocket->status = RNF_CONNECTING;
					psnet_rel_send_ack(&rsocket->addr, rcv_buff.seq, rcv_buff.send_time);
					ml_string("Got IAMHERE!");

					continue;
				}
			}

			if ( (rcv_buff.type == RNT_DATA) && (Serverconn != 0xffffffff) ) {
				rsocket->status = RNF_CONNECTED;
			} else {
				rsocket->last_packet_received = psnet_get_time();

				continue;
			}
		}

		// Update the last recv variable so we don't need a heartbeat
		rsocket->last_packet_received = psnet_get_time();

		if (rcv_buff.type == RNT_HEARTBEAT) {
			continue;
		}

		if (rcv_buff.type == RNT_ACK) {
			// Update ping time
			rsocket->num_ping_samples++;

			rsocket->pings[rsocket->ping_pos] = rsocket->last_packet_received - rcv_buff.send_time;

			if (rsocket->num_ping_samples >= MAX_PING_HISTORY) {
				float sort_ping[MAX_PING_HISTORY];

				for (auto a = 0; a < MAX_PING_HISTORY; a++) {
					sort_ping[a] = rsocket->pings[a];
				}

				std::sort(sort_ping, sort_ping + MAX_PING_HISTORY);
				rsocket->mean_ping = ( (sort_ping[MAX_PING_HISTORY/2] + sort_ping[(MAX_PING_HISTORY/2) + 1]) ) / 2;
			}

			rsocket->ping_pos++;

			if (rsocket->ping_pos >= MAX_PING_HISTORY) {
				rsocket->ping_pos = 0;
			}

			// if this is an ack for a send buffer on the socket, kill the send buffer. its done
			auto *acksig = reinterpret_cast<ushort *>(&rcv_buff.data);

			for (i = 0; i < MAXNETBUFFERS; i++) {
				if ( rsocket->sbuffers[i] && (rsocket->ssequence[i] == INTEL_SHORT(*acksig)) ) {
					vm_free(rsocket->sbuffers[i]);
					rsocket->sbuffers[i] = nullptr;
					rsocket->ssequence[i] = 0;
				}
			}

			// remove that packet from the send buffer
			rsocket->last_packet_received = psnet_get_time();

			continue;
		}

		if (rcv_buff.type == RNT_DATA_COMP) {
			// More2Come
			// Decompress it. Put it back in the buffer. Process it as RNT_DATA
			rcv_buff.type = RNT_DATA;
		}

		if (rcv_buff.type == RNT_DATA) {
			// If the data is out of order by >= MAXNETBUFFERS-1 ignore that packet for now
			int seqdelta = rcv_buff.seq - rsocket->oursequence;

			if (seqdelta < 0) {
				seqdelta = -seqdelta;
			}

			if ( seqdelta >= (MAXNETBUFFERS-1) ) {
				ml_string("Received reliable packet out of order!");

				// It's out of order, so we won't ack it, which will mean we will get it again soon.
				continue;
			}

			//e lse move data into the proper buffer position
			int savepacket = 1;

			if ( rsocket->oursequence < (0xffff - (MAXNETBUFFERS-1)) ) {
				if (rsocket->oursequence > rcv_buff.seq) {
					savepacket = 0;
				}
			} else {
				// Sequence is high, so prepare for wrap around
				if ( static_cast<unsigned short>(rcv_buff.seq + rsocket->oursequence) > (MAXNETBUFFERS-1) ) {
					savepacket = 0;
				}
			}

			for (i = 0; i < MAXNETBUFFERS; i++) {
				if ( (rsocket->rbuffers[i] != nullptr) && (rsocket->rsequence[i] == rcv_buff.seq) ) {
					// Received duplicate packet!
					savepacket = 0;
				}
			}

			if (savepacket) {
				if (rcv_buff.data_len > MAX_PACKET_SIZE) {
					ml_string("Received oversized reliable packet!");

					// don't ack it, which will mean we will get it again soon.
					continue;
				}

				for (i = 0; i < MAXNETBUFFERS; i++) {
					if (rsocket->rbuffers[i] == nullptr) {
						rsocket->recv_len[i] = rcv_buff.data_len;

						rsocket->rbuffers[i] = reinterpret_cast<reliable_net_buffer *>(vm_malloc(sizeof(reliable_net_buffer)));

						memcpy(rsocket->rbuffers[i]->buffer, rcv_buff.data, static_cast<size_t>(rsocket->recv_len[i]));
						rsocket->rsequence[i] = rcv_buff.seq;

						break;
					}
				}
			}

			psnet_rel_send_ack(&rsocket->addr, rcv_buff.seq, rcv_buff.send_time);
		}
	} while (true);
	
	// Go through each reliable socket that is connected and do any needed work.
	for (j = 0; j < MAXRELIABLESOCKETS; j++) {
		rsocket = &Reliable_sockets[j];

		if (Serverconn == 0xffffffff) {
			if (rsocket->status == RNF_LIMBO) {
				if ( fl_abs((psnet_get_time() - rsocket->last_packet_received)) > Nettimeout ) {
					ml_printf("Reliable (but in limbo) socket (%d) timed out in psnet_rel_work().", j);

					for (auto a = 0; a < MAXNETBUFFERS; a++) {
						if (rsocket->sbuffers[a] != nullptr) {
							vm_free(rsocket->sbuffers[a]);
						}

						if (rsocket->rbuffers[a] != nullptr) {
							vm_free(rsocket->rbuffers[a]);
						}
					}

					memset(rsocket, 0, sizeof(reliable_socket));
					rsocket->status = RNF_UNUSED; // Won't work if this is an outgoing connection.
				}
			}
		} else {
			if ( (rsocket->status == RNF_LIMBO) && (fl_abs((psnet_get_time() - First_sent_iamhere)) > Nettimeout) ) {
				rsocket->status = RNF_BROKEN;
				ml_printf("Reliable socket (%d) timed out in psnet_rel_work().", j);
			}
		}

		if (rsocket->status == RNF_CONNECTED) {
			float retry_packet_time;

			if ( ((rsocket->mean_ping < 0.00001f) && (rsocket->mean_ping > -0.00001f)) || (rsocket->mean_ping > (NETRETRYTIME*4)) ) {
				retry_packet_time = NETRETRYTIME;
			} else {
				if (rsocket->mean_ping < MIN_NET_RETRYTIME) {
					retry_packet_time = static_cast<float>(MIN_NET_RETRYTIME);
				} else {
					retry_packet_time = rsocket->mean_ping * 1.25f;
				}
			}

			// Iterate through send buffers.
			for (i = 0;i < MAXNETBUFFERS; i++) {
				// send again
				if ( rsocket->sbuffers[i] && (fl_abs((psnet_get_time() - rsocket->timesent[i])) >= retry_packet_time) ) {
					reliable_header send_header;

					memcpy(send_header.data, rsocket->sbuffers[i]->buffer, static_cast<size_t>(rsocket->send_len[i]));

					send_header.data_len = INTEL_SHORT( static_cast<ushort>(rsocket->send_len[i]) );
					send_header.send_time = psnet_get_time();
					send_header.send_time = INTEL_FLOAT( &send_header.send_time );
					send_header.seq = INTEL_SHORT( rsocket->ssequence[i] );
					send_header.type = RNT_DATA;

					rcode = SENDTO(Psnet_socket, reinterpret_cast<char *>(&send_header),
								   static_cast<int>(RELIABLE_PACKET_HEADER_ONLY_SIZE) + rsocket->send_len[i], 0,
								   reinterpret_cast<LPSOCKADDR>(&rsocket->addr), sizeof(rsocket->addr),
								   PSNET_TYPE_RELIABLE);

					if ( (rcode == SOCKET_ERROR) && (WSAGetLastError() == WSAEWOULDBLOCK) ) {
						// The packet didn't get sent, flag it to try again next frame
						rsocket->timesent[i] = psnet_get_time() - (NETRETRYTIME*4);
					} else {
						rsocket->last_packet_sent = psnet_get_time();
						rsocket->timesent[i] = psnet_get_time();
					}
				}
			}

			if ( (rsocket->status == RNF_CONNECTED) && (fl_abs((psnet_get_time() - rsocket->last_packet_sent)) > NETHEARTBEATTIME) ) {
				reliable_header send_header;

				send_header.send_time = psnet_get_time();
				send_header.send_time = INTEL_FLOAT( &send_header.send_time );
				send_header.seq = 0;
				send_header.data_len = 0;
				send_header.type = RNT_HEARTBEAT;

				rcode = SENDTO(Psnet_socket, reinterpret_cast<char *>(&send_header),
							   RELIABLE_PACKET_HEADER_ONLY_SIZE, 0,
							   reinterpret_cast<LPSOCKADDR>(&rsocket->addr), sizeof(rsocket->addr),
							   PSNET_TYPE_RELIABLE);

				if ( (rcode != SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK) ) {
					// It must have been sent
					rsocket->last_packet_sent = psnet_get_time();
				}
			}

			if ( (rsocket->status == RNF_CONNECTED) && (fl_abs((psnet_get_time() - rsocket->last_packet_received))>Nettimeout) ) {
				// This socket is hosed.....inform someone?
				ml_printf("Reliable Socket (%d) timed out in psnet_rel_work().", j);

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
	if (socketid >= MAXRELIABLESOCKETS) {
		return -1;
	}

	return Reliable_sockets[socketid].status;
}

/**
 * Checks the Listen_socket for possibly incoming requests to be connected.
 * @return 0 on error or nothing waiting.  1 if we should try to accept
 */
PSNET_SOCKET psnet_rel_check_for_listen(net_addr *from_addr)
{
	psnet_rel_work();

	for (unsigned int i = 1; i < MAXRELIABLESOCKETS; i++) {
		if (Reliable_sockets[i].status == RNF_CONNECTING) {
			Reliable_sockets[i].status = RNF_CONNECTED;

			ml_string("New reliable connection in psnet_rel_check_for_listen().");

			psnet_sockaddr_to_addr(&Reliable_sockets[i].addr, from_addr);

			return i;
		}
	}

	return PSNET_INVALID_SOCKET;
}

/**
 * Attempt to connect() to the server's tcp socket.  socket parameter is simply assigned to the
 * Reliable_socket socket created in psnet_init
 */
void psnet_rel_connect_to_server(PSNET_SOCKET *socket, net_addr *server_addr)
{
	// Send out a RNT_REQ_CONN packet, and wait for it to be acked.
	SOCKADDR_STORAGE rcv_addr;
	SOCKADDR_IN6 srv_addr;
	int addrlen;
	int rcode;
	float first_sent_req = 0;
	reliable_header rheader;
	int bytesin;
	struct timeval timeout;
	fd_set read_fds;
	reliable_socket *rsocket;

	*socket = PSNET_INVALID_SOCKET;

	psnet_addr_to_sockaddr(server_addr, &srv_addr);


	// Flush out any left overs
	do {
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(Psnet_socket, &read_fds);

		if ( SELECT(static_cast<int>(Psnet_socket+1), &read_fds, nullptr, nullptr, &timeout, PSNET_TYPE_RELIABLE) == SOCKET_ERROR ) {
			break;
		}

		// if the file descriptor is not set, then bail!
		if ( !FD_ISSET(Psnet_socket, &read_fds) ) {
			break;
		}

		addrlen = sizeof(rcv_addr);
		bytesin = RECVFROM(Psnet_socket, reinterpret_cast<char *>(&rheader), sizeof(reliable_header), 0,
						   reinterpret_cast<LPSOCKADDR>(&rcv_addr), &addrlen, PSNET_TYPE_RELIABLE);

		if (bytesin < 0) {
			ml_printf("UDP recvfrom returned an error! -- %d", WSAGetLastError());
			break;
		}
	} while (true);


	rheader.type = RNT_REQ_CONN;
	rheader.seq = CONNECTSEQ;
	rheader.data_len = 0;

	rcode = SENDTO(Psnet_socket, reinterpret_cast<char *>(&rheader), RELIABLE_PACKET_HEADER_ONLY_SIZE, 0,
				   reinterpret_cast<LPSOCKADDR>(&srv_addr), sizeof(srv_addr), PSNET_TYPE_RELIABLE);

	if (rcode == SOCKET_ERROR) {
		ml_printf("Unable to send UDP packet in psnet_rel_connect_to_server()! -- %d", WSAGetLastError());
		return;
	}

	first_sent_req = psnet_get_time();


	// Wait until we get a response from the server or we timeout
	do {
		PSNET_TOP_LAYER_PROCESS();

		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(Psnet_socket, &read_fds);

		if ( SELECT(static_cast<int>(Psnet_socket+1), &read_fds, nullptr, nullptr, &timeout, PSNET_TYPE_RELIABLE) == SOCKET_ERROR ) {
			break;
		}

		// if the file descriptor is not set, then bail!
		if ( !FD_ISSET(Psnet_socket, &read_fds) ) {
			continue;
		}

		ml_string("selected() in psnet_rel_connect_to_server()");

		addrlen = sizeof(rcv_addr);
		bytesin = RECVFROM(Psnet_socket, reinterpret_cast<char *>(&rheader), sizeof(reliable_header), 0,
						   reinterpret_cast<LPSOCKADDR>(&rcv_addr) ,&addrlen, PSNET_TYPE_RELIABLE);

		if (bytesin == 0) {
			ml_string("Received 0 bytes from recvfrom() in psnet_rel_connect_to_server().");
			continue;
		}

		if (bytesin == -1) {
			ml_printf("recvfrom returned an error! -- %d", WSAGetLastError());
			Int3();
			return;
		}

		ml_string("received data after select in psnet_rel_connect_to_server()");

		ml_string("about to check ack_header.type");

		if (rheader.type != RNT_ACK) {
			ml_string("Received something that isn't an ACK in psnet_rel_connect_to_server().");
			continue;
		}

		auto *acknum = reinterpret_cast<ushort *>(&rheader.data);

		if (INTEL_SHORT(*acknum) != CONNECTSEQ) {
			ml_string("Received out of sequence ACK in psnet_rel_connect_to_server().");
			continue;
		}


		for (unsigned int i = 1; i < MAXRELIABLESOCKETS; i++) {
			rsocket = &Reliable_sockets[i];

			if (rsocket->status == RNF_UNUSED) {
				// Add the new connection here.
				memset(rsocket, 0, sizeof(reliable_socket));

				rsocket->last_packet_received = psnet_get_time();
				psnet_sockaddr_storage_to_in6(&rcv_addr, &rsocket->addr);
				rsocket->status = RNF_LIMBO;

				ml_string("Successfully connected to server in psnet_rel_connect_to_server().");

				First_sent_iamhere = psnet_get_time();
				Last_sent_iamhere = psnet_get_time();

				// Now send I_AM_HERE packet
				rheader.type = RNT_I_AM_HERE;
				rheader.seq = static_cast<ushort>(~CONNECTSEQ);
				rheader.data_len = 0;

				rcode = SENDTO(Psnet_socket, reinterpret_cast<char *>(&rheader), RELIABLE_PACKET_HEADER_ONLY_SIZE, 0,
							   reinterpret_cast<LPSOCKADDR>(&srv_addr), sizeof(srv_addr), PSNET_TYPE_RELIABLE);

				if (rcode == SOCKET_ERROR) {
					*socket = PSNET_INVALID_SOCKET;

					rsocket->status = RNF_UNUSED;
					memset(rsocket, 0, sizeof(reliable_socket));

					ml_string("Unable to send packet in psnet_rel_connect_to_server()");

					return;
				}

				*socket = i;
				Serverconn = i;

				rsocket->last_packet_sent = psnet_get_time();

				float f = psnet_get_time();

				while ( (fl_abs((psnet_get_time() - f)) < 2) && (rsocket->status != RNF_CONNECTING) ) {
					psnet_rel_work();
				}

				return;
			}
		}

		ml_string("Out of reliable socket space in psnet_rel_connect_to_server().");

		return;
	} while(fl_abs((psnet_get_time() - first_sent_req)) < RELIABLE_CONNECT_TIME);
}

/**
 * Initialize reliable sockets
 */
void psnet_init_rel_tcp()
{
	// clear reliable sockets
	for (auto &rsocket : Reliable_sockets) {
		memset(&rsocket, 0, sizeof(reliable_socket));
	}
}

void psnet_rel_close()
{
	PSNET_SOCKET_RELIABLE sock;

	// kill all sockets
	for (auto idx = 0; idx < MAXRELIABLESOCKETS; idx++) {
		if (Reliable_sockets[idx].status != RNF_UNUSED) {
			sock = static_cast<PSNET_SOCKET_RELIABLE>(idx);
			psnet_rel_close_socket(sock);
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
	for (idx = 0;idx < MAX_PACKET_BUFFERS; idx++) {
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
static void psnet_buffer_packet(network_packet_buffer_list *l, const ubyte *data, const SSIZE_T length, const SOCKADDR_IN6 *from)
{
	int idx;
	bool found_buf = false;

	Assert(length > 0);

	// find the first empty packet
	for (idx = 0; idx < MAX_PACKET_BUFFERS; idx++) {
		if (l->psnet_buffers[idx].sequence_number == -1) {
			found_buf = true;
			break;
		}
	}

	// if we didn't find the buffer, report an overrun
	if ( !found_buf ) {
		ml_string("WARNING - Buffer overrun in psnet");
	} else {
		// copy in the data
		memcpy(l->psnet_buffers[idx].data, data, static_cast<size_t>(length));
		l->psnet_buffers[idx].len = length;
		l->psnet_buffers[idx].sequence_number = l->psnet_seq_number;
		memcpy(&l->psnet_buffers[idx].from_addr, from, sizeof(l->psnet_buffers[idx].from_addr));

		// keep track of the highest id#
		l->psnet_highest_id = l->psnet_seq_number++;

		// set the lowest id# for the first time
		if (l->psnet_lowest_id == -1) {
			l->psnet_lowest_id = l->psnet_highest_id;
		}
	}
}

/**
 * Get the index of the next packet in order!
 */
int psnet_buffer_get_next(network_packet_buffer_list *l, ubyte *data, SSIZE_T *length, SOCKADDR_IN6 *from)
{	
	int idx;
	int found_buf = 0;

	// if there are no buffers, do nothing
	if ( (l->psnet_lowest_id == -1) || (l->psnet_lowest_id > l->psnet_highest_id) ) {
		return 0;
	}

	// search until we find the lowest packet index id#
	for (idx = 0; idx < MAX_PACKET_BUFFERS; idx++) {
		// if we found the buffer
		if (l->psnet_buffers[idx].sequence_number == l->psnet_lowest_id) {
			found_buf = 1;
			break;
		}
	}

	// at this point, we should _always_ have found the buffer
	Assert(found_buf);

	Assert(l->psnet_buffers[idx].len > 0);

	// copy out the buffer data
	memcpy(data, l->psnet_buffers[idx].data, static_cast<size_t>(l->psnet_buffers[idx].len));
	*length = l->psnet_buffers[idx].len;
	memcpy(from, &l->psnet_buffers[idx].from_addr, sizeof(*from));

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
 * Set some options on a socket
 */
void psnet_set_socket_options()
{
	int cursize, bufsize, i_opt;
	socklen_t cursizesize;

	// try and increase the size of my receive buffer
	bufsize = MAX_RECEIVE_BUFSIZE;
	
	// set the current size of the receive buffer
	cursizesize = sizeof(int);
	getsockopt(Psnet_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&cursize), &cursizesize);

	if ( cursize < (MAX_RECEIVE_BUFSIZE*2) ) {
		setsockopt(Psnet_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&bufsize), sizeof(bufsize));
		cursizesize = sizeof(int);
		getsockopt(Psnet_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char *>(&cursize), &cursizesize);
	}

	ml_printf("Receive buffer set to %d", cursize);

	// set the current size of the send buffer
	cursizesize = sizeof(int);
	getsockopt(Psnet_socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&cursize), &cursizesize);

	if ( cursize < (MAX_RECEIVE_BUFSIZE*2) ) {
		setsockopt(Psnet_socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char *>(&bufsize), sizeof(bufsize));
		cursizesize = sizeof(int);
		getsockopt(Psnet_socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&cursize), &cursizesize);
	}

	ml_printf("Send buffer set to %d", cursize);

	// make sure we are in dual-stack mode (not the default on Windows)
	i_opt = 0;
	setsockopt(Psnet_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char *>(&i_opt), sizeof(i_opt));
	cursizesize = sizeof(int);
	getsockopt(Psnet_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char *>(&i_opt), &cursizesize);

	ml_printf("Socket set to %s mode", i_opt ? "IPv6-only" : "dual-stack");
}

/**
 * Initialize socket
 */
bool psnet_init_socket()
{
	SOCKADDR_STORAGE sockaddr;

	Psnet_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	if (Psnet_socket == INVALID_SOCKET) {
		Psnet_failure_code = WSAGetLastError();
		ml_printf("Error on TCP startup %d", Psnet_failure_code);

		return false;
	}

	// set socket options
	psnet_set_socket_options();

	// bind the socket
	psnet_get_addr(nullptr, Psnet_default_port, &sockaddr);

	if ( bind(Psnet_socket, reinterpret_cast<LPSOCKADDR>(&sockaddr), sizeof(sockaddr)) == SOCKET_ERROR) {
		Psnet_failure_code = WSAGetLastError();

		if (Psnet_failure_code == WSAEADDRINUSE) {
			ml_printf("Socket already in use!  Another instance running?  (Try using the \"-port %i\" cmdline option)", Psnet_default_port + 1);
		} else {
			ml_printf("Couldn't bind socket (%d)!", Psnet_failure_code );
		}

		return false;
	}

	// success
	return true;
}

/**
 * Get time in seconds
 */
float psnet_get_time()
{
	return static_cast<float>(timer_get_milliseconds()) / 1000.0f;
}

/**
 * Mark a socket as having received data
 */
void psnet_mark_received(PSNET_SOCKET_RELIABLE socketid)
{
	// valid socket?
	if (socketid >= MAXRELIABLESOCKETS) {
		return;
	}

	// mark it
	Reliable_sockets[socketid].last_packet_received = psnet_get_time();
}
