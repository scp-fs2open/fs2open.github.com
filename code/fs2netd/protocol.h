// Protocol.h
// Protocol Definitions for FS2Open PXO
// Derek Meek
// 2-14-2003

// ############## ATTENTION ##########
// Licensed under the Academic Free License version 2.0
// View License at http://www.opensource.org/licenses/afl-2.0.php
// ###################################



#if !defined(__pxo_protocol_h_)
#define __pxo_protocol_h_


#include "globalincs/globals.h"
#include "globalincs/pstypes.h"


#define PXO_PROTO_VER "2.0"


// Packet IDs
#define PCKT_ID_FIRST				0x00

#define PCKT_SLIST_REQUEST			0x01
#define PCKT_SLIST_REPLY			0x02
#define PCKT_SLIST_HB				0x03
#define PCKT_LOGIN_AUTH				0x04
#define PCKT_LOGIN_REPLY			0x05
#define PCKT_MISSIONS_RQST			0x06
#define PCKT_MISSIONS_REPLY			0x07
#define PCKT_TABLES_RQST			0x08
#define PCKT_TABLES_REPLY			0x09
#define PCKT_PILOT_GET				0x0a
#define PCKT_PILOT_REPLY			0x0b
#define PCKT_PILOT_UPDATE			0x0c
#define PCKT_PILOT_UREPLY			0x0d
#define PCKT_PING					0x0e
#define PCKT_PONG					0x0f

#define PCKT_MISSION_CHECK			0x10
#define PCKT_MCHECK_REPLY			0x11
#define PCKT_BANLIST_RQST			0x12
#define PCKT_BANLIST_RPLY			0x13
#define PCKT_NETOWRK_WALL			0x14
#define PCKT_VALID_SID_RQST			0x15
#define PCKT_VALID_SID_REPLY		0x16
#define PCKT_CHAT_CHANNEL_UPD		0x17
#define PCKT_CHAT_CHAN_COUNT_RQST	0x18
#define PCKT_CHAT_CHAN_COUNT_REPLY	0x19
#define PCKT_SLIST_HB_2				0x1a
#define PCKT_SERVER_DISCONNECT		0x1b
#define PCKT_DUP_LOGIN_RQST			0x1c
#define PCKT_DUP_LOGIN_REPLY		0x1d
#define PCKT_SLIST_REQUEST_FILTER	0x1e
#define PCKT_SERVER_START			0x1f

#define PCKT_SERVER_UPDATE			0x20

#define PCKT_ID_LAST				0x21

#define PCKT_INVALID				0xff

#define VALID_PACKET_ID(p)			(((p) > PCKT_ID_FIRST) && ((p) < PCKT_ID_LAST))



struct crc_valid_status {
	char name[NAME_LENGTH];
	uint crc32;
	ubyte valid;
};

struct file_record
{
	char name[33];
	uint crc32;
};

struct fs2open_ship_typekill
{
	char name[NAME_LENGTH];
	ushort kills;
};


#endif
