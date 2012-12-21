/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTI_NETGAME_OPTIONS_HEADER_FILE
#define _MULTI_NETGAME_OPTIONS_HEADER_FILE

// ----------------------------------------------------------------------------------
// MULTI OPTIONS DEFINES/VARS
//
#include "globalincs/pstypes.h"

struct header;
struct netgame_info;
struct net_player;

// global options
#define STD_PASSWD_LEN			16
#define STD_NAME_LEN				32
#define MULTI_OPTIONS_STRING_LEN			256
typedef struct multi_global_options {
	// common options
	int		protocol;													// selected network protocol	
	ushort	port;															// port we're running on - for allowing multiple servers on one machine
	int		log;															// use a logfile	
	int		datarate_cap;												// datarate cap for OBJ_UPDATE_HIGH

	char		user_tracker_ip[MULTI_OPTIONS_STRING_LEN];		// ip address of user tracker
	char		game_tracker_ip[MULTI_OPTIONS_STRING_LEN];		// ip address of game tracker
	char		tracker_port[STD_NAME_LEN];						// ip address to use for user/game tracker (used for FS2NetD only)
	int			pxo;
	char		pxo_ip[MULTI_OPTIONS_STRING_LEN];					// ip address of pxo chat server
	char		pxo_rank_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo rankings page
	char		pxo_create_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo create account page
	char		pxo_verify_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo account validation page
	char		pxo_banner_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo banner files

	// standalone only options
	int		std_max_players;											// max players allowed on the standalone
	int		std_datarate;												// some OBJ_UPDATE_* value
	int		std_voice;													// should standalone allow voice
	char		std_passwd[STD_PASSWD_LEN+1];							// standalone host password
	char		std_pname[STD_NAME_LEN+1];								// permanent name for the standalone - if any
	char		std_pxo_login[MULTI_OPTIONS_STRING_LEN];				// pxo login to use
	char		std_pxo_password[MULTI_OPTIONS_STRING_LEN];				// pxo password to use
	int		std_framecap;												// standalone frame cap
} multi_global_options;

extern multi_global_options Multi_options_g;

// local (netplayer - nonserver) options - maintained on individual clients and on the server (no need for other clients to know this guy's settings)
#define MAX_OBJ_UPDATE_LEVELS						4					// the # of object update levels there are
#define OBJ_UPDATE_LOW								0					// low object updates
#define OBJ_UPDATE_MEDIUM							1					// medium object updates
#define OBJ_UPDATE_HIGH								2					// high object updates
#define OBJ_UPDATE_LAN								3					// ultra-high updates - no capping at all

#define MLO_FLAG_ACCEPT_PIX						(1<<0)			// accept pix from server (pilot pics, squadron logos, etc)
#define MLO_FLAG_NO_VOICE							(1<<1)			// turn off voice altogether
#define MLO_FLAG_LOCAL_BROADCAST					(1<<2)			// broadcast on the local subnet when looking for games
#define MLO_FLAG_FLUSH_CACHE						(1<<3)			// flush the multidata cache before every game
#define MLO_FLAG_XFER_MULTIDATA					(1<<4)			// xfer mission files to the multidata cache directory
#define MLO_FLAG_TEMP_CLOSED						(1<<5)			// send to standalone to tell him to toggle the temp closed status

// BE AWARE : any changes made to this structure will mess with the player file. it will have to be upped!!!!
typedef struct multi_local_options {
	int flags;																	// misc player options	
	int obj_update_level;													// one off the flags above indicating how often to refresh objects
} multi_local_options;	

// server options - maintained on the server _and_ clients
#define MSO_SQUAD_RANK								0						// only highest ranking players can message
#define MSO_SQUAD_LEADER							1						// only wingleaders can message
#define MSO_SQUAD_ANY								2						// anyone can message
#define MSO_SQUAD_HOST								3						// only the host can message

#define MSO_END_RANK									0						// only the highest ranking players and the host can end the mission
#define MSO_END_LEADER								1						// only team/wing leaders and the host can end the mission
#define MSO_END_ANY									2						// any player can end the mission
#define MSO_END_HOST									3						// only the host can end the mission

#define MSO_FLAG_INGAME_XFER						(1<<0)				// netgame allows file xfers to ingame joiners
#define MSO_FLAG_ACCEPT_PIX						(1<<1)				// netgame allows pilot pix, squad logos
#define MSO_FLAG_NO_VOICE							(1<<2)				// netgame is disallowing voice altogether
#define MSO_FLAG_SS_LEADERS						(1<<3)				// in ship/weapon select, only host or team captains can modify ships

// BE AWARE : any changes made to this structure will mess with the player file. it will have to be upped!!!!
typedef struct multi_server_options {
	// misc settings and flags
	ubyte squad_set;															// see MSO_SQUAD_*
	ubyte endgame_set;														// see MSO_END_*
	int flags;																	// see MSO_FLAG_*

	// default respawn count
	uint respawn;

	// default max # of observers
	ubyte max_observers;

	// default skill level
	ubyte skill_level;
	
	// voice settings
	ubyte voice_qos;															// voice quality of sound
	int voice_token_wait;													// min time between token gets for a given player
	int voice_record_time;													// max duration for voice recording (in ms)

	// time limit
	fix mission_time_limit;													// mission time limit (set to -1 for no limit)

	// kill limit
	int kill_limit;															// kill limit for a furball mission
} multi_server_options;


// ----------------------------------------------------------------------------------
// MULTI OPTIONS FUNCTIONS
//

// load in the config file
void multi_options_read_config();

// set netgame defaults 
// NOTE : should be used when creating a newpilot
void multi_options_set_netgame_defaults(multi_server_options *options);

// set local netplayer defaults
// NOTE : should be used when creating a newpilot
void multi_options_set_local_defaults(multi_local_options *options);

// fill in the passed netgame options struct with the data from my player file data (only host/server should do this)
void multi_options_netgame_load(multi_server_options *options);

// fill in the passed local options struct with the data from my player file data (all machines except standalone should do this)
void multi_options_local_load(multi_local_options *options, net_player *pxo_pl);

// update everyone on the current netgame options
void multi_options_update_netgame();

// update everyone with my local settings
void multi_options_update_local();

// update the standalone with the settings I have picked at the "start game" screen
void multi_options_update_start_game(netgame_info *ng);

// update the standalone with the mission settings I have picked (mission filename, etc)
void multi_options_update_mission(netgame_info *ng, int campaign_mode);


// ----------------------------------------------------------------------------------
// MULTI OPTIONS FUNCTIONS
//

// process an incoming multi options packet
void multi_options_process_packet(unsigned char *data, header *hinfo);


#endif
