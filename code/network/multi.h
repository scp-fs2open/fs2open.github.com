/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTI_H
#define _MULTI_H

#include "globalincs/pstypes.h"
#include "network/psnet2.h"					// for PSNET_SOCKET		
#include "network/multi_ping.h"
#include "network/multi_options.h"
#include "io/keycontrol.h"
#include "globalincs/globals.h"

class p_object;
struct CFILE;
class player;

// ----------------------------------------------------------------------------------------
// Basic defines
//
//

// defines for checking PXO valid missions
#ifdef NDEBUG		
	// NEVER COMMENT OUT THIS LINE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	#define PXO_CHECK_VALID_MISSIONS				// always check for valid missions in a debug build
#else
	// #define PXO_CHECK_VALID_MISSIONS				// comment this in and out as necessary (for testing or not)
#endif

// name of the validated mission file for PXO missions
#define MULTI_VALID_MISSION_FILE		"mvalid.cfg"

// server version and compatible version
// to join a game - your LOCAL.MULTI_FS_SERVER_COMPATIBLE_VERSION must be >= GAME_SERVER.MULTI_FS_SERVER_VERSION
// Version #. Please put the date down when you up these values
// NOTE: always keep SERVER_VERSION and SERVER_COMPATIBLE_VERSION the same
//
// version 32 - 1/29/99
// version 33 - 2/22/99
// version 34 - 4/8/99
// version 35 - 4/21/99
// version 36 - 4/28/99
// version 37 - 4/29/99
// version 38 - 5/17/77
// version 39 - 7/3/99
// version 40 - 7/7/99
// version 41 - 7/22/99
// version 42 - 7/26/99 (ingame join stuff)
// version 43 - 7/30/99
// version 44 - 8/24/99
// version 46 - 8/30/99
// version 47 - 11/11/2003 (FS2OpenPXO, FS2 Open Changes - FS2Open 3.6)
// revert  46 - 9/7/2006 (the 47 bump wasn't needed, reverting to retail version for compatibility reasons)
// STANDALONE_ONLY

#define MULTI_FS_SERVER_VERSION							147

#define MULTI_FS_SERVER_COMPATIBLE_VERSION			MULTI_FS_SERVER_VERSION

// version defines (i.e. demo, full version, special OEM version
// id's should not be > 255!!!!
#define NG_VERSION_ID_FULL						1

// set the next define to be what version you want to build for.
#define NG_VERSION_ID							NG_VERSION_ID_FULL

// the max # of active players (flying ships)
#define MULTI_MAX_PLAYERS					12

// the total max # of connections (players + observers + (possibly)standalone server)
#define MULTI_MAX_CONNECTIONS				16

// the max # of observers ever allowed
#define MAX_OBSERVERS						4

#define LOGIN_LEN								33

// string length defines
#define MAX_GAMENAME_LEN					32				// maximum length in characters of a game name
#define DESCRIPT_LENGTH						512			// maximum length of a mission description (as specified by Fred)
#define MAX_PASSWD_LEN						16				// maximum length of the password for a netgame

// low level networking defines
#define IP_ADDRESS_LENGTH					4				// length of the address field for an IP address
#define IP_PORT_LENGTH						2				// length of the port field for an IP address
#define IPX_ADDRESS_LENGTH					6				// length of the address field for an IPX address
#define IPX_PORT_LENGTH						2				// length of the port field for an IPX address

// netgame defines
#define MP_SINGLE_MISSION					0				// not playing a campaign - single mission
#define MP_CAMPAIGN							1				// playing a campaign

// respawn defines
#define RESPAWN_INVUL_TIMESTAMP				5000			// how long a player is invulnerable after he respawns
#define MAX_RESPAWN_POINTS					25				// the max # of respawn points we'll keep track of for any mission

// player information defines
#define BUTTON_INFO_SAVE_COUNT			30				// how many buttons infos we keep track of for sending critical keypresses to the server
#define MAX_PINGS								10				// how many pings we keep track of for averaging player pings

// reliable connect wait
#define MULTI_RELIABLE_CONNECT_WAIT		15

// tracker mission validation status
#define MVALID_STATUS_UNKNOWN					-1
#define MVALID_STATUS_VALID					0
#define MVALID_STATUS_INVALID					1

// ----------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------
// Network macros
//
//

// netplayer management
#define NET_PLAYER_INDEX(np)	(np-Net_players)
#define NET_PLAYER_NUM(np)		(NET_PLAYER_INDEX(np))
#define MY_NET_PLAYER_NUM		(NET_PLAYER_INDEX(Net_player))

// determine what the status of this machine is
#define MULTIPLAYER_MASTER			( (Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) )
#define MULTIPLAYER_HOST			( (Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_GAME_HOST) )
#define MULTIPLAYER_CLIENT			( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) )
#define MULTIPLAYER_STANDALONE		( (Game_mode & GM_MULTIPLAYER) && (Net_players[0].flags & NETINFO_FLAG_AM_MASTER) && !(Net_players[0].flags & NETINFO_FLAG_GAME_HOST) )

// determine the status of the passed player
#define MULTI_CONNECTED(np)			(np.flags & NETINFO_FLAG_CONNECTED)
#define MULTI_HOST(np)				(np.flags & NETINFO_FLAG_GAME_HOST)
#define MULTI_SERVER(np)			(np.flags & NETINFO_FLAG_AM_MASTER)
#define MULTI_STANDALONE(np)		((np.flags & NETINFO_FLAG_AM_MASTER) && !(np.flags & NETINFO_FLAG_GAME_HOST))
#define MULTI_OBSERVER(np)			(np.flags & NETINFO_FLAG_OBSERVER)
#define MULTI_TEMP_OBSERVER(np)		((np.flags & NETINFO_FLAG_OBSERVER) && (np.flags & NETINFO_FLAG_OBS_PLAYER))
#define MULTI_PERM_OBSERVER(np)		((np.flags & NETINFO_FLAG_OBSERVER) && !(np.flags & NETINFO_FLAG_OBS_PLAYER))

// are we playing on a master tracker registered server
extern int Om_tracker_flag;
#define MULTI_IS_TRACKER_GAME    (Om_tracker_flag)
// ----------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------
// Packet definitions and additional packet data types
//
//

#define NETPLAYER_SLOTS_P        0x01     // data telling clients what player has what object number ...
#define FIRING_INFO					0x02		// firing info packet
#define INGAME_SHIP_UPDATE       0x03     // ship status-like update for ingame joiners choosing ships.
#define INGAME_SHIP_REQUEST      0x04     // used for requesting a replying (confirm or no) ships for an ingame joiner
#define TEAM_SELECT_UPDATE       0x05     // update from host to players in team select regarding who has what ship
#define FILE_SIG_INFO            0x06     // file signature info ( as calculated by get_bigass_file_signature() )
#define RESPAWN_NOTICE           0x07     // from client to server, or server to client
#define LOAD_MISSION_NOW         0x08     // clients should load the mission and return an ack
#define FILE_SIG_REQUEST         0x09     // request from server to client for filesig info
#define JUMP_INTO_GAME           0x0A     // from server to client telling him to jump into the mission
#define RESPAWN_POINTS           0x0B     // from server to ingame joiners, giving respawn data
#define CLIENT_REPAIR_INFO       0x0C     // passing various data to clients indicating their repair status
#define CARGO_REVEALED				0x0E		// cargo is known
#define SHIELD_EXPLOSION			0x0F		// shield explosion

#define SUBSYSTEM_DESTROYED		0x10		// update about a subsystem update
#define MISSION_SYNC_DATA        0x11		// this is a unique packet from the host to the server in a standalone game
#define STORE_MISSION_STATS      0x12		// sent to client indicating the host has hit "accept" and they should update their alltime stats
#define DEBRIS_UPDATE            0x13		// debris position, velocity, orientation, etc update
#define SHIP_WSTATE_CHANGE			0x14		// used to tell clients that a ship's primary/secondary state changed	
#define WSS_REQUEST_PACKET			0x15		// from client to server, requesting to do an operation
#define WSS_UPDATE_PACKET			0x16		// from server to client, given any update
#define KICK_PLAYER					0x17		// kick a specific player (sent to server by those who are allowed to do this)
#define MISSION_GOAL_INFO			0x18		// update of mission goal info
#define ASTEROID_INFO				0x19		// update of asteroid stuff
#define NETPLAYER_PAIN				0x1A		// to notify the player of hits which he may not otherwise see
#define OBJECT_UPDATE_NEW			0x1B
#define SUBSYS_CARGO_REVEALED		0X1C		// Capital ship cargo subsystem is known

#define POST_SYNC_DATA				0x20		// a very large packet containing all the data players will need before going into the mission itself
#define PLAYER_SETTINGS				0x21		// player settings for each individual net player
#define WSS_SLOTS_DATA				0x22		// all weapon slots information for the starting wings (needs to be synched)
#define PLAYER_STATS					0x23		// stats for a given player 
#define SLOT_UPDATE					0x24		// an player slot position update in multiplayer ship/team select
#define TEAM_UPDATE					0x25		// used for performing misc operations on pregame interface screens for team vs. team games
#define INGAME_EXTRA					0x26		// extra data for ingame joiners
#define HOST_RESTR_QUERY			0x27		// query the host when a player joins a restricted game
#define OPTIONS_UPDATE				0x28		// options (netgame or local) update packet
#define CLIENT_UPDATE				0x29		// sent from server to client periodically to update important info (pause status, etc)
#define CD_VERIFY						0x2A		// cd verification update
#define PRIMARY_FIRED_NEW			0x2B		// for client-side firing - highly streamlined
#define COUNTERMEASURE_NEW			0x2C		// for client-side firing
#define EVENT_UPDATE					0x2D		// event change

#define SECONDARY_FIRED_AI			0xA0		// fired a secondary weapon (ai ship)
#define SECONDARY_FIRED_PLR		0xA1		// fired a secondary weapon (player ship)
#define COUNTERMEASURE_FIRED		0xA2		// countermeasure was fired
#define FIRE_TURRET_WEAPON			0xA3		// a turret weapon was fired
#define SHIP_STATUS_CHANGE       0xA4     // any of the relevant ship status buttons have been hit (need ack from server)
#define PLAYER_ORDER_PACKET      0xA5     // ship and wing commands sent from client to server
#define AI_INFO_UPDATE				0xA6		// update ai information for the given ship
#define CAMPAIGN_UPDATE				0xA7		// one of several campaign informational packets
#define CAMPAIGN_UPDATE_INGAME	0xA8		// campaign info for ingame joiner
#define HOMING_WEAPON_UPDATE		0xA9		// update homing object and subsystem for homing missile
#define FLAK_FIRED					0xAA		// flak gun fired
#define SELF_DESTRUCT				0xAB		// self destruct

#define JOIN							0xB1		// a join request to a server
#define ACCEPT							0xB2		// acceptance of a join packet
#define DENY							0xB3		// a join request is denied
#define NOTIFY_NEW_PLAYER			0xB4		// notify players of a new player
#define MISSION_REQUEST				0xB5     // request a list of missions on the master.
#define MISSION_ITEM					0xB6		// a bundle of mission filenames
#define GAME_INFO						0xB7		// game information packet for an active server
#define MULTI_PAUSE_REQUEST		0xB8		// send a request to the server to pause or unpause the game
#define TRANSFER_HOST				0xB9     // transfer host status to the receiver
#define CHANGE_SERVER_ADDR			0xBA     // change your host_addr to this value 
#define ACCEPT_PLAYER_DATA			0xBB		// player data -- sent after guy is accepted
#define BEAM_FIRED					0xBC		// a beam weapon was fired
#define SW_STD_QUERY					0xBD		// query from host to standalone server to query PXO about a squad war match

#define HUD_MSG						0xC1		// a hud message
#define LEAVE_GAME					0xC2		// indication that a player is leaving the game
#define GAME_CHAT						0xC3		// this chat packet used for FreeSpace
#define MISSION_MESSAGE				0xC4		// a message (squadmate, etc)
#define SHIP_DEPART					0xC5		// a ship has left the building
#define SHIPS_INGAME_PACKET		0xC6     // ingame join ship data packet
#define WINGS_INGAME_PACKET		0xC7		// ingame join wing data packet
#define MISSION_END					0xC8     // sent from host(master) to clients indicating they should go to the debriefing
#define INGAME_NAK					0xC9     // opposite of INGAME_ACK. Uses the ACK_* defines as well.
#define OBSERVER_UPDATE				0xCA     // sent from observers to server to give position and orientation
#define SQUADMSG_PLAYER				0xCB		// a squadmate message command has been sent to a netplayer
#define OBJ_UPDATE_SYNC				0xCC		// object update timebase syncing code
#define FORCE_MISSION_END			0xCD	// Force all the players to skip straight to the debrief screen

#define WEAPON_DET					0xD1		// a weapon has detonated, possibly with child weapons
#define SHIP_KILL						0xD2		// a ship was killed
#define WING_CREATE					0xD3		// create and warp in a wing of ships
#define SHIP_CREATE					0xD4		// create and wrap in a ship
#define PING					      0xD5     // ping
#define PONG					      0xD6		// pong
#define XFER_PACKET			      0xD7		// file xfer data of one kind or another
#define VOICE_PACKET					0xD8		// a voice streaming packet of one kind or another
#define NETGAME_END_ERROR			0xD9		// the netgame has been ended by the server because of an error (passed error code)
#define COUNTERMEASURE_SUCCESS	0xDA		// countermeasure was successful for this player.
#define REINFORCEMENT_AVAIL		0xDB		// a reinforcement is available
#define LIGHTNING_PACKET			0xDC		// lightning bolt packet for multiplayer nebula
#define BYTES_SENT					0xDD		// how much data we've sent/received

#define GAME_ACTIVE					0xE1		// info on an active game server
#define GAME_QUERY					0xE2		// request for a list of active game servers
#define GAME_UPDATE					0xE3		// update info on an active game server
#define NETPLAYER_UPDATE			0xE4		// a player update packet
#define OBJECT_UPDATE				0xE6		// an object update packet from server to all clients
#define MISSION_LOG_ENTRY			0xE7		// ad an item into the mission log
#define UPDATE_DESCRIPT				0xE8		// update the netgame description
#define COUNTDOWN						0xE9		// countdown timer for starting a game (pretty benign)
#define DEBRIEF_INFO					0xEA		// end of mission debriefing information
#define EMP_EFFECT					0xEB		// EMP effect (mission disk only)
#define CHANGE_IFF					0xEC		// change iff (1.04+ only)

#define CARGO_HIDDEN				0xED		// cargo is hidden
#define SUBSYS_CARGO_HIDDEN			0xEE		// Capital ship cargo subsystem is hidden

#define CHANGE_AI_CLASS				0xEF		// change ai class
#define NO_SUBSYS_STRING			"NO-SUBSYS"	// see change_ai_class - because same routine is used
												// for both ships and subsystems
#define VARIABLE_UPDATE				0xF0		// Karajorma - a variable update packet from server to all clients
#define WEAPON_OR_AMMO_CHANGED		0xF1		// Karajorma - The weapon or ammo of a fighter/bomber has been changed
#define SEXP						0xF2		// Karajorma - a general packet which can be used to update clients with changes caused by SEXPs
#define CHANGE_IFF_COLOR			0xF3		// Wanderer - change iff color

#define MAX_TYPE_ID					0xFF		// better not try to send > 255 in a single byte buddy

// ingame ack data codes
#define ACK_RESPAWN_POINTS			0x1		// from ingame joiner to server, indicating he got the respawn points packet
#define ACK_FILE_ACCEPTED        0x2		// server to client saying their file is valid
#define ACK_FILE_REJECTED        0x3		// server to client saying their file is not valid

// join request denial codes
#define JOIN_DENY_JR_STATE				0		// join request is rejected because the game is not in the proper state
#define JOIN_DENY_JR_TRACKER_INVAL		1		// join request is rejected because the game is an MT game and the passed player info is invalid
#define JOIN_DENY_JR_PASSWD				2		// join request is rejected because the game is password protected and the password sent is incorrect
#define JOIN_DENY_JR_CLOSED				3		// join request is rejected because the game is closed and is currently ingame
#define JOIN_DENY_JR_RANK_HIGH			4		// join request is rejected because the game is rank based and the passed rank is too high
#define JOIN_DENY_JR_RANK_LOW			5		// join request is rejected because the game is rank based and the passed rank is too low
#define JOIN_DENY_JR_DUP				6		// join request is denied because there is an exiting netplayer with matching characteristics
#define JOIN_DENY_JR_FULL				7		// join request is denied because the game is full
#define JOIN_DENY_JR_TEMP_CLOSED		8		// join request is denied because the _forming_ netgame has been toggled closed
#define JOIN_DENY_JR_BANNED				9		// join request is denied because the player has been banned for the duration of the game
#define JOIN_DENY_JR_NOOBS				10		// join request is denied because observers are not allowed
#define JOIN_DENY_JR_INGAME_JOIN		11		// join request is denied because someone else is already ingame joining
#define JOIN_DENY_JR_BAD_VERSION		12		// incompatible version types
#define JOIN_QUERY_RESTRICTED			13		// poll the host of the game to see if he accepts this player
#define JOIN_DENY_JR_TYPE				14		// cannot ingame join anything but dogfight games

// repair info codes
#define REPAIR_INFO_BEGIN			0x1		// server to client - set your REPAIRING flags
#define REPAIR_INFO_END				0x2		// server to client - unset your REPAIRING flags
#define REPAIR_INFO_UPDATE			0x3		// server to client - here's some repair update info (not currently used)
#define REPAIR_INFO_QUEUE			0x4		// server to client - client is queued for rearming.
#define REPAIR_INFO_ABORT			0x5		// server to client - client has aborted a rearm/repair
#define REPAIR_INFO_BROKEN			0x6		// server to client - client is breaking repair -- might be reestablished
#define REPAIR_INFO_WARP_ADD		0x7		// server to client - add client onto list of people for arriving support ship
#define REPAIR_INFO_WARP_REMOVE		0x8		// server to client - remove client from list of people for arriving support ship
#define REPAIR_INFO_ONWAY			0x9		// server to client - repair ship on way
#define REPAIR_INFO_KILLED			0xa		// server to client - repair ship was killed on way to rearm player
#define REPAIR_INFO_COMPLETE		0xb		// server to client - repair of your ship is complete

// debris update codes
#define DEBRIS_UPDATE_UPDATE			0x1		// update a piece
#define DEBRIS_UPDATE_REMOVE			0x2		// remove a piece of debris
#define DEBRIS_UPDATE_NUKE				0x3		// blow up a piece of debris
#define DEBRIS_UPDATE_CREATE_HULL		0x4		// create a piece of debris

// weapon select/ship select update packets
#define WSS_WEAPON_SELECT			0x1		// ship select stuff
#define WSS_SHIP_SELECT				0x2		// weapon select stuff

// accept packet codes
#define ACCEPT_INGAME				(1<<0)	// accept the player as an ingame joiner
#define ACCEPT_HOST					(1<<1)	// accept the player as the host of the game
#define ACCEPT_OBSERVER				(1<<2)	// accept the player as an observer
#define ACCEPT_CLIENT				(1<<3)	// accept the player as a normal, non ingame join, non host player

// accept player data codes
#define APD_NEXT						0			// there is still more player data
#define APD_END_PACKET					1			// end of this packet
#define APD_END_DATA					2			// end of the data

// ingame ship request codes		
#define INGAME_SR_REQUEST			0x1		// request for the ship with the given net signature
#define INGAME_SR_CONFIRM			0x2		// confirmation to the client that he can use the requested ship
#define INGAME_SR_DENY				0x3		// deny the request the ingame joiner made for the ship
#define INGAME_PLAYER_CHOICE		0x4		// sent to other players informing them of ingame joiners choice

// ai info update codes
#define AI_UPDATE_DOCK				0x1		// server tells clients which ships are now docked
#define AI_UPDATE_UNDOCK			0x2		// server tells clients which ships have undocked
#define AI_UPDATE_ORDERS			0x3		// server tells clients about new AI order for the ship

// requests to the standalone
#define MISSION_LIST_REQUEST		0x1		// ask for list of missions
#define CAMPAIGN_LIST_REQUEST		0x2		// ask for list of campaigns

// asteroid stuff
#define ASTEROID_CREATE				0x1		// create an asteroid
#define ASTEROID_THROW				0x2		// throw an asteroid
#define ASTEROID_HIT				0x3		// asteroid hit occured

// commands for squadmate messages
#define SQUAD_MSG_SHIP				0x1
#define SQUAD_MSG_WING				0x2
#define SQUAD_MSG_ALL				0x3
#define SQUAD_MSG_REINFORCEMENT		0x4

// SW_STD_QUERY codes
#define SW_STD_START					0x1		// from host to standalone saying "query the tracker"
#define SW_STD_OK						0x2		// from standalone to host - "everything is cool"
#define SW_STD_BAD						0x3		// from standalone to host - "everything is bad"

// stats block packet
#define STATS_MISSION				0			// all stats for the mission (except kills), for one player
#define STATS_ALLTIME				1			// alltime stats, for one player
#define STATS_MISSION_KILLS			2			// mission kills and assists
#define STATS_DOGFIGHT_KILLS		3			// same as mission kills, but also sends per-player kills
#define STATS_MISSION_CLASS_KILLS	4			// kills for the mission, for one player
#define STATS_ALLTIME_KILLS			5			// alltime kills, for one player

#define MAX_SHIPS_PER_PACKET		64			// Number of ships in a STATS_MISSION_KILLS or STATS_ALLTIME_KILLS packet

// ----------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------
// Multiplayer structure definitions
//
//

// definition of header packet used in any protocol
typedef struct header {
	int		bytes_processed;											// used to determine how many bytes this packet was
	ubyte		net_id[4];													// obtained from network layer header
	ubyte		addr[6];														// obtained from network-layer header
	short		port;															// obtained from network-layer header
	short		id;															// will be stuffed with player_id (short)
} header;

// NETPLAYER INFORMATION THE SERVER AND THE INDIVIDUAL CLIENT MUST HAVE
typedef struct net_player_server_info {		
	ping_struct		ping;													// evaluated by the ping module
	int				wing_index_backup;								// in case of fail on the last packet
	int				wing_index;											// index of the next wing data item to be sent
	int				ingame_join_flags;								// status flags for an ingame joiner
	int				invul_timestamp;									// invulnerability flag timestamp (for respawning after dying)
	button_info		last_buttons[BUTTON_INFO_SAVE_COUNT];		// button info for sending critical control packets to the server
	int				last_buttons_id[BUTTON_INFO_SAVE_COUNT];	//
	fix				last_buttons_time[BUTTON_INFO_SAVE_COUNT];//
	int				num_last_buttons;									//
	fix				last_full_update_time;							// time when server last updated this player position/orientation
	int				xfer_handle;										// handle to the file xfer handle (-1 if no file xfer is taking place)
	int				kick_timestamp;									// timestamp with which we'll disconnect a player if he hasn't reponded to a kick packet
	int				kick_reason;										// reason he was kicked
	int            voice_token_timestamp;							// timestamp set when a player loses a token (so we can prevent him from getting it again too quickly)
	int				reliable_connect_time;							// after sending an accept packet, wait for this long for the guy to connect on the reliable socket

	// weapon select/linking information (maintained on the server and passed on respawn to all clients)
	char				cur_primary_bank;									// currently selected primary bank
	char				cur_secondary_bank;								// currently selected secondary bank
	ubyte				cur_link_status;									// if (1<<0) - primaries linked. if (1<<1) - secondaries are linked

	// information regarding the current view position of this player.
	vec3d			eye_pos;								// eye position and orientation
	matrix			eye_orient;

	// ets information
	ushort			ship_ets;							// ets settings (sigh......)

	// tracker information
	int				tracker_security_last;			// this is the value returned when getting tracker data. it must be used when "sending" tracker data
	unsigned int	tracker_checksum;					// tracker checksum

	// common targeting information
	int				target_objnum;

	// rate limiting information
	int				rate_stamp;							// rate limiting timestamp
	int				rate_bytes;							// bytes sent this "second"

	// firing info (1<<0) for primary fire, (1<<1) for secondary fired, (1<<2) for countermeasure fired, (1<<3) for afterburner on
	// basically, we set these bits if necessary between control info sends from the client. once sent, these values are
	// cleared until the next send time
	ubyte				accum_buttons;				
	
	// buffered packet info
	ubyte					unreliable_buffer[MAX_PACKET_SIZE];	// buffer used to buffer unreliable packets before sending as a single UDP packet
	int					unreliable_buffer_size;					// length (in bytes) of data in unreliable send_buffer
	ubyte					reliable_buffer[MAX_PACKET_SIZE];	// buffer used to buffer reliable packets before sending as a single UDP packet
	int					reliable_buffer_size;					// length (in bytes) of data in reliable send_buffer
} net_player_server_info;

// NETPLAYER INFORMATION ALL COMPUTERS IN THE GAME MUST HAVE
typedef struct net_player_info {
	p_object			*p_objp;								// pointer to parse object for my ship -- used with respawns
	int				team;									// valid for team v. team games -- which team is this guy on
	int				ship_index;							// index into the ship choices in team select/ship select (out of 12 choices)
	int				ship_class;							// the ship class of the players ship
	multi_local_options options;						// players options settings	
	net_addr			addr;
	char				pxo_squad_name[LOGIN_LEN];		// PXO squadron name
} net_player_info;

// NETPLAYER COMMON INFORMATION
typedef struct net_player {
	player			*m_player;								// stuff pertaining directly to the player (callsign, etc).
	short				player_id;							// player id (always use this instead of ip address for identification purposes)
	int				tracker_player_id;            // the tracker id for this player, only matters in
																// tracker games.	
	int				flags;								// tells us interesting information about this player
	int				state;								// one of the NETGAME_STATE_* flags below -- used for sequencing
	PSNET_SOCKET_RELIABLE	reliable_socket;		// reliable socket to server
	
	ushort			client_cinfo_seq;					// sequence # for client control info packets
	ushort			client_server_seq;				// sequence # for incoming object update packets		
	
	fix				last_heard_time;					// time when last heard from this player

	net_player_server_info	s_info;					// server critical info
	net_player_info			p_info;					// player critical info

	// bytes sent and received
	// SERVER-side
	int				sv_bytes_sent;						// bytes we've sent to this guy (on the server)	
	int				sv_last_pl;							// packet loss

	// CLIENT-side
	int				cl_bytes_recvd;					// bytes we've received (as a client)		
	int				cl_last_pl;							// packet loss
} net_player;

// structure which describes the state of the multiplayer game currently being played
typedef struct netgame_info {
	char		name[MAX_GAMENAME_LEN+1];		// name of the netgame (host can set this!)
	char		mission_name[NAME_LENGTH+1];	// current mission name (filename)
	char		title[NAME_LENGTH+1];			// title of the mission (as appears in the mission file)
	char		campaign_name[NAME_LENGTH+1];	// current campaign name	
	char		passwd[MAX_PASSWD_LEN+1];		// password for the game
	int		version_info;						// version info for this game.
	int		type_flags;							// see NG_TYPE_* defines
	int		mode;									// see NG_MODE_* defines
	int		flags;								// see NG_FLAG_* defines
	int		rank_base;							// used to compare against connecting players (rank above/rank below)	
	int      max_players;		
	int		game_state;							// state (briefing, in mission, etc) this game is in
	int		security;							// some random number that should hopefully be unique for each game started
														// I'm also using this value to use as a starting base for the net_signature
														// for object synchronization.
	float    ping_time;							// ping time to this server
	net_addr	server_addr;						// address of the server
	net_player *host;
	net_player *server;							// pointer to the server		

	uint respawn;

	int campaign_mode;							// 0 == single mission mode, 1 == starting campaign

	ushort   server_update_seq;				// the current object update reference count (server-side only)
	int      server_update_frame_ref;		// used to determine when we should increment the server_update_seq value

	multi_server_options options;				// server options settings

	ubyte		debug_flags;						// special debug flags (see NETD_FLAG_* defines)
} netgame_info;

// structure for active games -- kind of like Descent, but using the linked list thing, we will
// be able to support many more games in the list.
#define AG_FLAG_COOP								(1<<0)			// is a coop game
#define AG_FLAG_TEAMS							(1<<1)			// is a team vs. team game
#define AG_FLAG_DOGFIGHT						(1<<2)			// is a dogfight game
#define AG_FLAG_FORMING							(1<<3)			// game is currently forming
#define AG_FLAG_BRIEFING						(1<<4)			// game is in the briefing state
#define AG_FLAG_DEBRIEF							(1<<5)			// game is in the debriefing state
#define AG_FLAG_PAUSE							(1<<6)			// game is paused
#define AG_FLAG_IN_MISSION						(1<<7)			// game is in mission
#define AG_FLAG_PASSWD							(1<<8)			// is a password protected game
#define AG_FLAG_STANDALONE						(1<<9)			// this is a standalone server
#define AG_FLAG_CAMPAIGN						(1<<10)			// the server is playing in campaign mode
#define AG_FLAG_TRACKER							(1<<11)

// flags for defining the connection speed
#define AG_FLAG_CONNECTION_SPEED_MASK		((1<<12)|(1<<13)|(1<<14))	// mask for the connection speed

#define AG_FLAG_VALID_MISSION					(1<<15)			// the mission is a "valid" tracker mission

#define AG_FLAG_CONNECTION_BIT				12						// number of bits to shift right or left to get speed

#define AG_FLAG_TYPE_MASK						(AG_FLAG_COOP|AG_FLAG_TEAMS|AG_FLAG_DOGFIGHT)
#define AG_FLAG_STATE_MASK						(AG_FLAG_FORMING|AG_FLAG_BRIEFING|AG_FLAG_DEBRIEF|AG_FLAG_PAUSE|AG_FLAG_IN_MISSION)

typedef struct active_game {
	active_game		*next, *prev;				// next and previous elements in the list	
	int				heard_from_timer;			// when we last heard from the game	
	
	char		name[MAX_GAMENAME_LEN+1];
	char		mission_name[NAME_LENGTH+1];
	char		title[NAME_LENGTH+1];	
	ubyte		num_players;
	net_addr	server_addr;	
	ushort	flags;								// see above AG_FLAG_* defines
	ubyte		version,comp_version;			// version and compatible version
	ping_struct ping;								// ping time to the server
} active_game;

// permanent server list (read from tcp.cfg)
typedef struct server_item {
	server_item *next, *prev;

	net_addr server_addr;
} server_item;

// sent to the server on a join request with various data
#define JOIN_FLAG_AS_OBSERVER			(1<<0)	// wants to join as an aboserver
#define JOIN_FLAG_HAXOR					(1<<2)	// if the player has hacked data

typedef struct join_request {
	char passwd[MAX_PASSWD_LEN+1];				// password for a password protected game
	char callsign[CALLSIGN_LEN+1];				// player's callsign
	char image_filename[MAX_FILENAME_LEN+1];	// player's image filename
	char squad_filename[MAX_FILENAME_LEN+1];	// player's squad filename	
	ubyte player_rank;								// the rank of the requesting player
	ubyte flags;										// misc flags associated with this guy
	int tracker_id;									// player's tracker ID #
	multi_local_options player_options;			// player's options
	ubyte version, comp_version;					// local version and comp_version

	// multiplayer squad war info
	char pxo_squad_name[LOGIN_LEN];				// squad name
} join_request;

// network buffer for sending and receiving packets
typedef struct network_buffer {
	int	size;										// size of the buffer
	ubyte	data[MAX_PACKET_SIZE];				// MAX_PACKET_SIZE from psnet2.h
} network_buffer;
// -------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------
// Multiplayer structure flags/settings
//
//

// flags used for the net_player structure
#define NETINFO_FLAG_CONNECTED				(1<<0)		// if this player connected
#define NETINFO_FLAG_AM_MASTER				(1<<1)		// is this player the master
#define NETINFO_FLAG_MT_CONNECTED			(1<<2)      // if everything is hunky dory with the tracker connection
#define NETINFO_FLAG_MT_FAILED				(1<<3)      // all attempts to connect have failed
#define NETINFO_FLAG_MT_STARTUP				(1<<4)      // the initial state (ie, we haven't tried anything yet)
#define NETINFO_FLAG_GAME_HOST				(1<<5)      // I'm the host
#define NETINFO_FLAG_INGAME_JOIN			(1<<6)      // means he is still in the process of joining ingame
#define NETINFO_FLAG_OBSERVER				(1<<7)      // means he's an observer
#define NETINFO_FLAG_OBS_PLAYER				(1<<8)      // means he's an observer, but he was formerly a player (should show his stats)
#define NETINFO_FLAG_LIMBO					(1<<9)      // (client side) means he has to choose whether to be an observer or quit (no more respawns)
#define NETINFO_FLAG_MISSION_OK				(1<<10)     // this client's mission has been verified
#define NETINFO_FLAG_RESPAWNING				(1<<11)     // so that we wait on a keypress, or other user input before respawning
#define NETINFO_FLAG_DO_NETWORKING			(1<<12)		// set when we can send/receive data
#define NETINFO_FLAG_TEAM_LOCKED			(1<<13)		// if this is set, only the host can modify the team settings for this player
#define NETINFO_FLAG_TEAM_CAPTAIN			(1<<14)		// this player is the captain of his team
#define NETINFO_FLAG_KICKED					(1<<15)		// this player was kicked
#define NETINFO_FLAG_ACCEPT_INGAME			(1<<16)		// accepted ingame
#define NETINFO_FLAG_ACCEPT_HOST			(1<<17)		// accetped as host
#define NETINFO_FLAG_ACCEPT_OBSERVER		(1<<18)		// accepted as observer
#define NETINFO_FLAG_ACCEPT_CLIENT			(1<<19)		// accepted as client
#define NETINFO_FLAG_WARPING_OUT			(1<<20)		// clients keep track of this for themselves to know if they should be leaving
#define NETINFO_FLAG_HAS_CD					(1<<21)		// the player has a CD in the drive
#define NETINFO_FLAG_RELIABLE_CONNECTED		(1<<22)		// reliable socket is now active
#define NETINFO_FLAG_MT_GET_FAILED			(1<<23)		// set during MT stats update process indicating we didn't properly get his stats
#define NETINFO_FLAG_MT_SEND_FAILED			(1<<24)		// set during MT stats update process indicating we didn't properly send his stats
#define NETINFO_FLAG_MT_DONE				(1<<25)		// set when a player has been processed for stats (fail, succeed, or otherwise)
#define NETINFO_FLAG_HAXOR					(1<<26)		// the player has some form of hacked client data

#define NETPLAYER_IS_OBSERVER(player)		(player->flags & (NETINFO_FLAG_OBSERVER|NETINFO_FLAG_OBS_PLAYER))
#define NETPLAYER_IS_DEAD(player)			(player->flags & (NETINFO_FLAG_LIMBO|NETINFO_FLAG_RESPAWNING))

// netgame modes
#define NG_MODE_OPEN							1				// an open game
#define NG_MODE_CLOSED							2				// a closed game
#define NG_MODE_PASSWORD						3				// a password protected game
#define NG_MODE_RESTRICTED						4				// a restricted game
#define NG_MODE_RANK_ABOVE						5				// ranks above a certain rank are allowed
#define NG_MODE_RANK_BELOW						6				// ranks below a certain rank are allowed

// netgame option flags
#define NG_FLAG_TEMP_CLOSED					(1<<0)		// a forming netgame is temporarily closed (should not be checked otherwise)
#define NG_FLAG_SERVER_LOST					(1<<1)		// client has temporarily lost contact with the server
#define NG_FLAG_INGAME_JOINING				(1<<2)		// someone is ingame joining.
#define NG_FLAG_INGAME_JOINING_CRITICAL		(1<<3)		// someone is ingame joining and at the critical point where we cannot do certain things.
#define NG_FLAG_STORED_MT_STATS				(1<<4)		// stored tracker stats in the debriefing already
#define NG_FLAG_HACKED_SHIPS_TBL			(1<<5)		// set when the server is playing with a hacked ships.tbl (only needed to notify hosts playing on a standalone)
#define NG_FLAG_HACKED_WEAPONS_TBL			(1<<6)		// set when the server is playing with a hacked weapons.tbl (only needed to notify hosts playing on a standalone)

// netgame type flags
#define NG_TYPE_COOP							(1<<0)		// cooperative mode
#define NG_TYPE_TVT								(1<<1)		// team vs. team mode
#define NG_TYPE_SW								(1<<2)		// squad war
#define NG_TYPE_TEAM							( NG_TYPE_TVT | NG_TYPE_SW )
#define NG_TYPE_DOGFIGHT						(1<<3)		// plain old dogfight mode

// some quick macros for the above
#define MULTI_COOP						((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_COOP))
#define MULTI_TEAM						((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM))
#define MULTI_DOGFIGHT					((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT))

// and some to handle everything but
#define MULTI_NOT_COOP					((Game_mode & GM_MULTIPLAYER) && !(Netgame.type_flags & NG_TYPE_COOP))
#define MULTI_NOT_TEAM					((Game_mode & GM_MULTIPLAYER) && !(Netgame.type_flags & NG_TYPE_TEAM))
#define MULTI_NOT_DOGFIGHT				((Game_mode & GM_MULTIPLAYER) && !(Netgame.type_flags & NG_TYPE_DOGFIGHT))

// state defines for netgame states
#define NETGAME_STATE_FORMING					1				// players are joining, host is selecting missions, etc
#define NETGAME_STATE_BRIEFING					2				// players are reading the mission briefing
#define NETGAME_STATE_IN_MISSION				3				// the mission itself is being played
#define NETGAME_STATE_SERVER_TRANSFER			4				// server status is being transferred from one computer to another
#define NETGAME_STATE_PAUSED					5				// the netgame is paused
#define NETGAME_STATE_DEBRIEF					6				// the debriefing screen			
#define NETGAME_STATE_MISSION_SYNC				7				// client/server data sync screens before and after the briefing stage
#define NETGAME_STATE_ENDGAME					8				// game is moving from gameplay to the debriefing state
#define NETGAME_STATE_STD_HOST_SETUP			9				// the host is on the setup netgame screen for the standalone server
#define NETGAME_STATE_HOST_SETUP				10				// the host is on the setup netgame screen _non_ standalone

// state defines for netplayer states
#define NETPLAYER_STATE_JOINING				0				// joining the netgame
#define NETPLAYER_STATE_JOINED				1				// has joined and opened a reliable socket connection
#define NETPLAYER_STATE_MISSION_LOADING		2				// in the process of loading the mission
#define NETPLAYER_STATE_MISSION_LOADED		3				// mission loaded
#define NETPLAYER_STATE_BRIEFING			4				// in the briefing
#define NETPLAYER_STATE_SHIP_SELECT			5				// in the ship selection screen
#define NETPLAYER_STATE_WEAPON_SELECT		6				// in the weapon selection screen
#define NETPLAYER_STATE_DATA_LOAD			7				// loading specific data (textures, etc)
#define NETPLAYER_STATE_WAITING				8				// waiting to do something (like enter the mission)
#define NETPLAYER_STATE_SLOT_ACK			9				// got player ship slot packets
#define NETPLAYER_STATE_IN_MISSION			10				// in the mission itself
#define NETPLAYER_STATE_INGAME_SHIPS		11				// player is receiving ingame join ship data
#define NETPLAYER_STATE_INGAME_WINGS		12				// player is receiving ingame join wing data
#define NETPLAYER_STATE_INGAME_RPTS			13				// player is receiving ingame join respawn data
#define NETPLAYER_STATE_INGAME_SHIP_SELECT	14				// player is in the ship select screen for ingame join
#define NETPLAYER_STATE_DEBRIEF				15				// player is in the debrief state (screen)
#define NETPLAYER_STATE_MISSION_SYNC		16				// player is in the mission sync screen
#define NETPLAYER_STATE_STD_HOST_SETUP		17				// the host is on the setup netgame screen for the standalone server
#define NETPLAYER_STATE_HOST_SETUP			18				// the host is on the setup netgame screen _non_ standalone
#define NETPLAYER_STATE_SETTINGS_ACK		19				// the player has received the player settings data for all players
#define NETPLAYER_STATE_SLOTS_ACK			20				// the player has received wss slots data (ingame join only)
#define NETPLAYER_STATE_POST_DATA_ACK		21				// the player has received the post briefing data block
#define NETPLAYER_STATE_WSS_ACK				22				// have received weapon slot information
#define NETPLAYER_STATE_FLAG_ACK			23				// the player has received his flag change information
#define NETPLAYER_STATE_MT_STATS			24				// the server is in the process of requesting or updating stats from the MT
#define NETPLAYER_STATE_MISSION_XFER		25				// the player is in the process of receiving a mission file
#define NETPLAYER_STATE_INGAME_STUFF		26				// received ingame "stuff"  happens just before ship selection
#define NETPLAYER_STATE_DEBRIEF_ACCEPT		27				// the player has hit the accept button in the debrief and is good to go
#define NETPLAYER_STATE_DEBRIEF_REPLAY		28				// set on the host instead of NETPLAYER_STATE_DEBRIEF_ACCEPT to indicate he wants to replay the mission
#define NETPLAYER_STATE_CPOOL_ACK			29				// player has acked all campaign pool status data
#define NETPLAYER_STATE_INGAME_CINFO		30				// player has received campaign information (ingame join only)

// defines for connection speed
#define CONNECTION_SPEED_NONE					-1				// not really used except for error checking

#define CONNECTION_SPEED_288					0
#define CONNECTION_SPEED_56K					1
#define CONNECTION_SPEED_SISDN				2
#define CONNECTION_SPEED_CABLE				3
#define CONNECTION_SPEED_T1					4

// use this to check and see whether a netgame is anywhere in mission (paused, etc, etc)
#define MULTI_IN_MISSION						( (Netgame.game_state == NETGAME_STATE_IN_MISSION) || (Netgame.game_state == NETGAME_STATE_PAUSED) )

extern int Multi_connection_speed;

// -------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------
// Multiplayer global vars
//
//

// netplayer vars
extern net_player Net_players[MAX_PLAYERS];						// array of all netplayers in the game
extern net_player *Net_player;										// pointer to console's net_player entry

// network object management
#define SHIP_SIG_MIN				1
#define SHIP_SIG_MAX				(0x2fff)

#define STANDALONE_SHIP_SIG	(SHIP_SIG_MAX+1)
#define REAL_SHIP_SIG_MAX		(0x3fff)

#define DEBRIS_SIG_MIN			(REAL_SHIP_SIG_MAX+1)
#define DEBRIS_SIG_MAX			(0x5fff)

#define ASTEROID_SIG_MIN		(DEBRIS_SIG_MAX+1)
#define ASTEROID_SIG_MAX		(0x7fff)

#define NPERM_SIG_MIN			(ASTEROID_SIG_MAX+1)
#define NPERM_SIG_MAX			(0xffff)

extern ushort Next_ship_signature;									// next network signature to assign to an object
extern ushort Next_asteroid_signature;								// next asteroid signature
extern ushort Next_non_perm_signature;								// next non-permanent signature
extern ushort Next_debris_signature;								// next debris signature

// netgame vars
extern netgame_info Netgame;											// netgame information
extern int Multi_mission_loaded;										// flag, so that we don't load the mission more than once client side
extern ushort Multi_ingame_join_sig;									// signature for the player obj for use when joining ingame
extern int Multi_button_info_ok;										// flag saying it is ok to apply critical button info on a client machine
extern int Multi_button_info_id;										// identifier of the stored button info to be applying

// low level networking vars
extern int ADDRESS_LENGTH;												// will be 6 for IPX, 4 for IP
extern int PORT_LENGTH;													// will be 2 for IPX, 2 for IP
extern int HEADER_LENGTH;												// 1 byte (packet type)

// misc data
extern active_game* Active_game_head;								// linked list of active games displayed on Join screen
extern int Active_game_count;											// for interface screens as well
extern CFILE* Multi_chat_stream;										// for streaming multiplayer chat strings to a file
extern int Multi_has_cd;												// if this machine has a cd or not (call multi_common_verify_cd() to set this)
extern int Multi_num_players_at_start;								// the # of players present (kept track of only on the server) at the very start of the mission
extern short Multi_id_num;												// for assigning player id #'s

// permanent server list
extern server_item* Game_server_head;								// list of permanent game servers to be querying

// restricted game vars
#define MULTI_QUERY_RESTR_STAMP			5000						// 5 seconds to reply
#define MULTI_JOIN_RESTR_MODE_1			0							// mode 1 - normal restricted join
#define MULTI_JOIN_RESTR_MODE_2			1							// mode 2 - team vs. team, only team 0 has ships
#define MULTI_JOIN_RESTR_MODE_3			2							// mode 3 - team vs. team, only team 1 has ships
#define MULTI_JOIN_RESTR_MODE_4			3							// mode 4 - team vs. team, both teams have ships

extern int Multi_restr_query_timestamp;							// timestamp for querying the host to see if he will allow a new player to join ingame
extern join_request Multi_restr_join_request;					// join request for the query
extern net_addr Multi_restr_addr;									// net address of the join request
extern int Multi_join_restr_mode;									// what mode we're in

// non API master tracker vars
#define MULTI_TRACKER_STRING_LEN	100
extern char Multi_tracker_login[MULTI_TRACKER_STRING_LEN+1];
extern char Multi_tracker_passwd[MULTI_TRACKER_STRING_LEN+1];
extern char Multi_tracker_squad_name[MULTI_TRACKER_STRING_LEN+1];
extern int Multi_tracker_id;
extern char Multi_tracker_id_string[255];

// current file checksum
extern ushort Multi_current_file_checksum;
extern int Multi_current_file_length;


// ----------------------------------------------------------------------------------------
// Multiplayer main functions
//
//

// module handling functions -------------------

// called at game startup
void multi_init();

// called whenever a netgame is started
void multi_level_init();

// reset all relevant main networking loop timestamps
void multi_reset_timestamps();

// returns true is server hasn't been heard from in N seconds. false otherwise
int multi_client_server_dead();


// netgame data processing functions -----------

// do all network processing for this game frame
void multi_do_frame();

// analog of multi_do_frame() called when netgame is in the pause state
void multi_pause_do_frame();

// process all incoming packets
// void multi_process_bigdata(ubyte* data, int size, net_addr* from_addr);

// process all reliable socket details
void multi_process_reliable_details();


// standalone handling functions ---------------

// initialize the standalone
void standalone_main_init();

// do frame for the main standalone state
void standalone_main_do();

// close for the main standalone state
void standalone_main_close();

// reset all standalone stuff, including gui, networking, netgame, etc, and go into the main state
void multi_standalone_reset_all();

// init for the wait mode of the standalone (when waiting for players to finish the briefing)
void multi_standalone_wait_init();

// do frame for the standalone wait state (when waiting for players to finish the briefing)
void multi_standalone_wait_do();

// close for the standalone wait state (when waiting for players to finish the briefing)
void multi_standalone_wait_close();

// init for the standalone postgame state (when players are in the debriefing)
void multi_standalone_postgame_init();

// do frame for the standalone postgame state (when players are in the debriefing)
void multi_standalone_postgame_do();

// close for the standalone postgame state (when players are in the debriefing_
void multi_standalone_postgame_close();


#endif
