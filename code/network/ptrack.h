/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _pilot_track_header
#define _pilot_track_header

#include "stats/scoring.h"					// for medals count

//Pilot tracker client header
#define REGPORT						9243

#define MAX_NET_RETRIES				30
#define NET_ACK_TIMEOUT				2500

#define MAX_PXO_FILENAME_LEN		32

//This is for type 
#define UNT_CONTROL					0
#define UNT_NEW_ID_REQUEST			1
#define UNT_VALIDAT_ID_REQUEST	2
#define UNT_LOOKUP_ID_REQUEST		3
#define UNT_UPDATE_ID_REQUEST		4
#define UNT_MOTD_REQUEST			5
#define UNT_MOTD_RESPONSE			6
#define UNT_PILOT_DATA_READ		7		// Request from the game for pilot info
#define UNT_PILOT_DATA_RESPONSE	8		// Mastertracker's response to a read request (has pilot data in this packet)
#define UNT_PILOT_DATA_WRITE		9		// Request from the server to write a user record
#define UNT_PILOT_WRITE_FAILED	10		// Server didn't update the pilots record for some reason
#define UNT_PILOT_WRITE_SUCCESS	11		// Server updated the pilots record
#define UNT_PILOT_READ_FAILED		12		// Couldn't find the record
#define UNT_LOGIN_AUTH_REQUEST	13		// Request login authentication by login/password only (returns tracker id)
#define UNT_LOGIN_NO_AUTH			14		// Couldn't login this user (code has reason)
#define UNT_LOGIN_AUTHENTICATED	15		// User was authenticated (data has sz string with tracker id)
#define UNT_VALID_FS_MSN_REQ		18		// Client asking if this is a valid mission
#define UNT_VALID_FS_MSN_RSP		19		// Server Response (code has the answer)

#define UNT_PILOT_DATA_READ_NEW	20		// New packet for requesting reads (tracker will get new security field when receiving this)
#define UNT_PILOT_DATA_WRITE_NEW	21		// New packet for requesting writes (tracker will compare security fields if this packet is used)

// 22 through 45 are D3 specific codes
#define UNT_VALID_FS2_MSN_REQ		46		// validated mission request to PXO for FS2
#define UNT_VALID_FS2_MSN_RSP		47		// validated mission response from PXO for FS2

// validate a squad war mission
#define UNT_VALID_SW_MSN_REQ		48		// send info about a squad war mission, wait for tracker response
#define UNT_VALID_SW_MSN_RSP		49		// response from the tracker
#define UNT_SW_RESULT_WRITE		50		// report on a finished squad war mission to the tracker
#define UNT_SW_RESULT_RESPONSE	51		// response on a squad war mission write request

#define UNT_CONTROL_VALIDATION	70		// UNT_CONTROL for validation packets

#define UNT_VALID_MSN_REQ			71		// Client asking if this is a valid mission
#define UNT_VALID_MSN_RSP			72		// Server response (code has the answer)
#define UNT_VALID_TBL_REQ			73		// Client asking if this is a valid table
#define UNT_VALID_TBL_RSP			74		// Server response (code has the answer)

#define UNT_VALID_DATA_REQ			75		// client asking for data validity check
#define UNT_VALID_DATA_RSP			76		// server response


//This is for code
#define CMD_NEW_USER_ACK			1
#define CMD_NEW_USER_NAK			2
#define CMD_VALIDATED_USER_ACK	3
#define CMD_UPDATED_USER_ACK		4
#define CMD_CLIENT_RECEIVED		5
#define CMD_FIND_USER_NAK			6
#define CMD_FIND_USER_ACK			7
#define CMD_UPDATED_USER_NAK		8
#define CMD_VALIDATED_USER_NAK	9
	//Game designators for UNT_PILOT_DATA_REQUEST and UNT_PILOT_DATA_RESPONSE
#define CMD_GAME_FREESPACE			10						
#define CMD_GAME_DESCENT3			11
#define CMD_GAME_FREESPACE2			12
#define CMD_GAME_FS2OPEN			13

//This is for xcode
#define REG_NAK_EMAIL				0								// failed to register the guy because of an invalid email address
#define REG_NAK_LOGIN				1								// failed to register the guy because an existing login exists
#define REG_NAK_ERROR				2								// failed to register because of an error on the tracker
#define REG_NAK_STRINGS				3								// failed to validate because of invalid password/login match
#define REG_NAK_UNKNOWN				4								// failed to validate because the player is unknown
#define REG_NAK_UPDATE_PL			5								// update info failed because login/passwd were not correct
#define REG_NAK_UPDATE_GEN			6								// update info failed in general (tracker problem)
#define REG_NAK_UPDATE_LOG			7								// update failed because login not found
#define REG_ACK_NEW_ID				8								// New id created, just used for return code, not net packets.

#define MAX_UDP_DATA_LENGH			500
#define PACKED_HEADER_ONLY_SIZE	(sizeof(udp_packet_header)-MAX_UDP_DATA_LENGH)
//sizeof(update_id_request)	//The largest packet

#define LOGIN_LEN						33
#define REAL_NAME_LEN				66
#define PASSWORD_LEN					17
#define EMAIL_LEN						100
#define TRACKER_ID_LEN				10
#define PILOT_NAME_LEN				20
#define MATCH_CODE_LEN				34

#define MAX_SQUAD_PLAYERS			4
#define MAX_SQUAD_RESPONSE_LEN	255

// data could be one of the following:

// type == UNT_NEW_ID_REQUEST
// Respond with ACK
typedef struct {
	char first_name[REAL_NAME_LEN];		// Real Name
	char last_name[REAL_NAME_LEN];		// Real Name
	char login[LOGIN_LEN];					// Login id
	char password[PASSWORD_LEN];			// password
	char email[EMAIL_LEN];					// Email Address
	unsigned char showemail;				// 0==don't show 1 == show
	unsigned char showname;					// 0==don't show 1 == show
} new_id_request;


// type == UNT_VALIDAT_ID_REQUEST or UNT_LOOKUP_ID_REQUEST
typedef struct {
	char login[LOGIN_LEN];					// Login id
	char password[PASSWORD_LEN];			// password
	char tracker_id[TRACKER_ID_LEN];		// Tracker ID
} validate_id_request;

// type == UNT_UPDATE_ID_REQUEST
typedef struct {
	char old_login[LOGIN_LEN];				// Login before it's changed.
	char old_password[PASSWORD_LEN];		// Password before it's changed
	char tracker_id[TRACKER_ID_LEN];		// Tracker ID (not sure if we need it for updating, but maybe)
	char first_name[REAL_NAME_LEN];		// Real Name
	char last_name[REAL_NAME_LEN];		// Real Name
	char login[LOGIN_LEN];					// Login id (new)
	char password[PASSWORD_LEN];			// password (new)
	char email[EMAIL_LEN];					// Email Address (new)
	unsigned char showemail;				// 0==don't show 1 == show
	unsigned char showname;					// 0==don't show 1 == show
} update_id_request;

typedef struct {
	char pilot_name[PILOT_NAME_LEN];		// Login id
	char tracker_id[TRACKER_ID_LEN];		// Tracker ID
	char pad[2];							// 2-bytes padding
} pilot_request;

// type == UNT_VALID_SW_MSN_REQ
typedef struct squad_war_request {		
	int squad_plr1[MAX_SQUAD_PLAYERS];	// id #'s for all squad members in the match
	int squad_plr2[MAX_SQUAD_PLAYERS];	// id #'s for all squad memebrs in the match

	ubyte squad_count1;						// # of players present in squad 1
	ubyte squad_count2;						// # of players present in squad 2	

	char match_code[MATCH_CODE_LEN];		// code for the match	

	char mission_filename[MAX_PXO_FILENAME_LEN];		// filename of mission
	int mission_checksum;									// mission checksum
} squad_war_request;

// type == UNT_SW_RESULT_WRITE
typedef struct squad_war_result {
	char match_code[MATCH_CODE_LEN];			// code for the match
	ubyte result;									// result of the match, 0 == tie, 1 == one team won
	ubyte squad_count1;							// # of players in winning squad
	ubyte squad_count2;							// # of players in the losing squad
	char pad[3];							// 3-bytes padding
	int squad_winners[MAX_SQUAD_PLAYERS];	// list of players on the winning team
	int squad_losers[MAX_SQUAD_PLAYERS];	// list of players on the losing team
} squad_war_result;

// type == UNT_VALID_SW_MSN_RSP and UNT_SW_RESULT_RESPONSE
typedef struct squad_war_response {
	char reason[MAX_SQUAD_RESPONSE_LEN];
	unsigned char accepted;
} squad_war_response;

// type == UNT_VALID_DATA_REQ and UNT_VAID_DATA_RSP
enum {
	VDR_TYPE_TABLE		= 0,
	VDR_TYPE_MISSION	= 1,
	VDR_TYPE_SCRIPT		= 2
};

enum {
	VDR_FLAG_IDENT		= 1<<0,		// include mod ident in response
	VDR_FLAG_STATUS		= 1<<1		// include individual file valid status in response
};

struct valid_data_item {
	uint32_t crc;
	SCP_string name;
};

// NOTE: This is packed manually and should not be associated/copied/cast to the data block!!
struct vmt_valid_data_req_struct {
	uint8_t type;
	uint8_t flags;
	uint8_t num_files;

	SCP_vector<valid_data_item> files;
};

// NOTE: This is not used directly but listed here to document the packet
/*
struct vmt_valid_data_rsp_struct {
	uint8_t flags;

	// if VDR_FLAG_IDENT is set
	short game_id;
	SCP_string game_tag;
	SCP_string game_name;

	// if VDR_FLAG_STATUS is set
	uint8_t status_count;
	SCP_vector<uint32_t> status;
};
*/


#pragma pack(push, 1)
	typedef struct {	
		unsigned char type;						// type
		unsigned short len;						//	Length of total packet, including this header
		unsigned int code;						// For control messages
		unsigned short xcode;					// For control/NAK messages and for sigs.
		unsigned int sig;						// To identify unique return ACKs
		unsigned int security;					// Just a random value, we store the last value used in the user record
														// So we don't process the same request twice.		
		unsigned char data[MAX_UDP_DATA_LENGH];
	} udp_packet_header;
#pragma pack(pop)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GAME SPECIFIC structures


// -----------------------------------------------------------------------------
// FS2 Open
// NOTE: struct is packed when serialized, padding bytes ignored
const unsigned short MAX_FS2OPEN_COUNTS	= 192;
struct vmt_fs2open_struct {
	int tracker_id;
	char pilot_name[PILOT_NAME_LEN];

	int score;
	int rank;
	int assists;
	int kill_count;					// total alltime kills
	int kill_count_ok;				// total valid alltime kills (no friendlies)
	unsigned int p_shots_fired;		// primary weapon
	unsigned int s_shots_fired;		// secondary weapon

	unsigned int p_shots_hit;		// primary
	unsigned int s_shots_hit;		// secondary

	unsigned int p_bonehead_hits;	// hits/kills on the players own team
	unsigned int s_bonehead_hits;
	int bonehead_kills;

	unsigned int missions_flown;	// # of missions flown to completion
	unsigned int flight_time;		// total hours of flight time
	unsigned int last_flown;		// data/time of last mission flown

	int security;
	unsigned int checksum;			// This value needs to be equal to whatever the checksum is once the packet is decoded

	unsigned char virgin_pilot;		// This pilot was just created if TRUE

	char pad;						// IGNORED!!  1-byte padding (alignment)

	unsigned char num_medals;
	unsigned char num_ships;

	unsigned short counts[MAX_FS2OPEN_COUNTS];	// <-- This *must* be last entry!!!!
};
const unsigned short FS2OPEN_BLOCK_SIZE = sizeof(vmt_fs2open_struct) - 1;	// ignore pad byte in size

#define vmt_stats_struct vmt_fs2open_struct
#define STATS_BLOCK_SIZE FS2OPEN_BLOCK_SIZE

//Function prototypes
int InitPilotTrackerClient();
void AckServer(unsigned int sig);

int SendFSPilotData(vmt_stats_struct *fs_pilot);
int GetFSPilotData(vmt_stats_struct *fs_pilot, const char *pilot_name, const char *tracker_id, int update_security);
int SendSWData(squad_war_result *sw_res, squad_war_response *sw_resp);
void PollPTrackNet();

//Definitions
#define STATE_IDLE						0
#define STATE_SENDING_PILOT			1
#define STATE_READING_PILOT			2
#define STATE_RECEIVED_PILOT			3
#define STATE_WROTE_PILOT				4
#define STATE_TIMED_OUT					5
#define STATE_PILOT_NOT_FOUND			6
#define STATE_WRITE_PILOT_FAILED		7

#define PILOT_REQ_TIMEOUT			30000
#define PILOT_REQ_RESEND_TIME		3500


#define PXO_ADD_DATA(d) do { memcpy(data+packet_size, &d, sizeof(d) ); packet_size += sizeof(d); } while (false)
#define PXO_ADD_SHORT(d) do { short swap = INTEL_SHORT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (false)
#define PXO_ADD_USHORT(d) do { ushort swap = INTEL_SHORT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (false)
#define PXO_ADD_INT(d) do { int swap = INTEL_INT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (false)
#define PXO_ADD_UINT(d) do { uint swap = INTEL_INT(d); memcpy(data+packet_size, &swap, sizeof(d) ); packet_size += sizeof(d); } while (false)
#define PXO_ADD_STRING(d, l) do { size_t len = SDL_strlcpy(reinterpret_cast<char *>(data+packet_size), reinterpret_cast<const char *>(d), l-packet_size); packet_size += static_cast<unsigned short>(SDL_min(len+1, l-packet_size)); } while (0)

#define PXO_GET_DATA(d) do { memcpy(&d, data+offset, sizeof(d) ); offset += sizeof(d); } while(false)
#define PXO_GET_SHORT(d) do { short swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_SHORT(swap); offset += sizeof(d); } while(false)
#define PXO_GET_USHORT(d) do { ushort swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_SHORT(swap); offset += sizeof(d); } while(false)
#define PXO_GET_INT(d) do { int swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_INT(swap); offset += sizeof(d); } while(false)
#define PXO_GET_UINT(d) do { uint swap; memcpy(&swap, data+offset, sizeof(d) ); d = INTEL_INT(swap); offset += sizeof(d); } while(false)
#define PXO_GET_STRING(d) do { size_t len = SDL_strlcpy(d, reinterpret_cast<const char *>(data+offset), SDL_arraysize(d)); offset += static_cast<unsigned short>(SDL_min(len+1, SDL_arraysize(d))); } while(0)

#endif
