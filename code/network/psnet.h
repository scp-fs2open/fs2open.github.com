/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/PsNet.h $
 * $Revision: 2.3 $
 * $Date: 2004-03-09 00:02:16 $
 * $Author: Kazan $
 *
 * Header file for the application level network-interface.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2003/11/14 22:47:37  Kazan
 * *WARNING* Multi Compatability with previous versions now in question *WARNING*
 * [But that's ok, i incremented the multi version number the other day so that it will say "you don't have a compatable version"]
 *
 * I upped the MAX_PACKET_SIZE from 512 to 4096 -- try and bust multi with more than ?130? ships now :D
 *
 * Revision 2.1  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 4     11/19/98 8:04a Dave
 * Full support for D3-style reliable sockets. Revamped packet lag/loss
 * system, made it receiver side and at the lowest possible level.
 * 
 * 3     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 41    10/02/98 3:22p Allender
 * fix up the -connect option and fix the -port option
 * 
 * 40    9/14/98 11:28a Allender
 * support for server bashing of address when received from client.  Added
 * a cmdline.cfg file to process command line arguments from a file
 * 
 * 39    8/07/98 10:40a Allender
 * new command line flags for starting netgames.  Only starting currently
 * works, and PXO isn't implemented yet
 * 
 * 38    6/30/98 2:17p Dave
 * Revised object update system. Removed updates for all weapons. Put
 * button info back into control info packet.
 * 
 * 37    6/04/98 11:04a Allender
 * object update level stuff.  Don't reset to high when becoming an
 * observer of any type.  default to low when guy is a dialup customer
 * 
 * 36    5/22/98 10:54a Allender
 * new dialog information for networking to tell user if connection type
 * doesn't match the setup specified type
 * 
 * 35    5/21/98 3:31p Allender
 * more RAS stuff -- fix possible problem when unable to read data on
 * reliable socket
 * 
 * 34    5/18/98 4:10p Allender
 * ditch any reference to IPX initialziation.  put in (butt comment out)
 * the RAS code
 * 
 * 33    5/01/98 3:15a Dave
 * Tweaked object update system. Put in new packet buffering system.
 * 
 * 32    4/04/98 4:22p Dave
 * First rev of UDP reliable sockets is done. Seems to work well if not
 * overly burdened.
 * 
 * 31    4/03/98 1:03a Dave
 * First pass at unreliable guaranteed delivery packets.
 * 
 * 30    3/11/98 11:42p Allender
 * more ingame join stuff.  Fix to networking code to possibly
 * reinitialize reliable socket when entering join screen
 * 
 * 29    2/05/98 11:44a Allender
 * enhcanced network statistic collection.  Made callback in debug console
 * to do networking if player is in the console
 * 
 * 28    2/04/98 6:35p Dave
 * Changed psnet to use raw data with no headers. Started putting in
 * support for master tracker security measures.
 * 
 * 27    1/11/98 10:03p Allender
 * removed <winsock.h> from headers which included it.  Made psnet_socket
 * type which is defined just as SOCKET type is.
 * 
 * 26    12/16/97 6:19p Dave
 * Put in primary weapon support for multiplayer weapon select.
 * 
 * 25    12/16/97 5:24p Allender
 * changed to options menu to allow FQDN's.  changes to player booting
 * code (due to timeout).  more work still needs to be done though.
 * 
 * 24    12/11/97 8:15p Dave
 * Put in network options screen. Xed out olf protocol selection screen.
 * 
 * 23    12/03/97 11:48a Allender
 * overhaul on reliable socket code.  Made reliable sockets non-blocking
 * (listen socket still blocks).  Made player timeouts work correctly with
 * certain winsock errors on the reliable sockets
 * 
 * 22    11/11/97 11:55a Allender
 * initialize network at beginning of application.  create new call to set
 * which network protocol to use
 * 
 * 21    11/04/97 3:50p Allender
 * more reliable socket stuff.  Removed admin port.  Cleaner sequencing,
 * etc.
 * 
 * 20    11/03/97 8:25p Dave
 * Got client side reliable sockets working. Got reliable/unreliable
 * pakcet interleaving done.
 * 
 * 19    11/03/97 5:09p Allender
 * added reliable transport system -- still in infancy.  Each netplayer
 * has reliable socket to server (and server to clients).
 * 
 * 18    10/24/97 6:22p Sandeep
 * added checknet
 * 
 * 17    9/17/97 9:09a Dave
 * Observer mode work, put in standalone controls. Fixed up some stuff for
 * ingame join broken by recent code checkins.
 * 
 * 16    8/26/97 5:02p Dave
 * Put in admin socket handling thread. Modified all functions to support
 * this.
 * 
 * 15    8/20/97 4:34p Dave
 * Added admin socket init to psnet_init()
 * 
 * 14    8/13/97 4:55p Dave
 * Moved PSNET_FLAG_CHECKSUM bit #define into psnet.h
 * 
 * 13    8/11/97 3:18p Dave
 * Added administration client socket.
 * 
 * 12    8/04/97 9:41p Allender
 * revamped packet structure for netgames.  No more magic number.  psnet*
 * functions can now do checksumming at it's level.
 * 
 * 11    7/30/97 9:39a Allender
 * network debug stuff -- showing net read/write stats on hud
 * 
 * 10    6/11/97 1:38p Allender
 * added basic sequencing through ship selection.  Put My_addr structure
 * into psnet.cpp where it should be
 * 
 * 9     1/03/97 12:01p Lawrance
 * Now getting return address and port from the network layer header.
 * 
 * 8     1/02/97 2:07p Lawrance
 * fixed problem with reading socket data
 *
 * 7     1/01/97 6:45p Lawrance
 * added support for IPX
 *
 * $NoKeywords: $
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
