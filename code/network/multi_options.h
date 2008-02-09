/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_options.h $
 * $Revision: 2.4 $
 * $Date: 2005-07-13 03:35:32 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2003/09/23 02:42:54  Kazan
 * ##KAZAN## - FS2NetD Support! (FS2 Open PXO) -- Game Server Listing, and mission validation completed - stats storing to come - needs fs2open_pxo.cfg file [VP-able]
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 6     4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 5     2/19/99 2:55p Dave
 * Temporary checking to report the winner of a squad war match.
 * 
 * 4     2/12/99 6:16p Dave
 * Pre-mission Squad War code is 95% done.
 * 
 * 3     11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 12    7/07/98 2:49p Dave
 * UI bug fixes.
 * 
 * 11    5/03/98 7:04p Dave
 * Make team vs. team work mores smoothly with standalone. Change how host
 * interacts with standalone for picking missions. Put in a time limit for
 * ingame join ship select. Fix ingame join ship select screen for Vasudan
 * ship icons.
 * 
 * 10    5/03/98 2:52p Dave
 * Removed multiplayer furball mode.
 * 
 * 9     4/30/98 12:57a Dave
 * Put in new mode for ship/weapon selection. Rearranged how game querying
 * is done a bit.
 * 
 * 8     4/22/98 5:53p Dave
 * Large reworking of endgame sequencing. Updated multi host options
 * screen for new artwork. Put in checks for host or team captains leaving
 * midgame.
 * 
 * 7     4/16/98 11:39p Dave
 * Put in first run of new multiplayer options screen. Still need to
 * complete final tab.
 * 
 * 6     4/09/98 11:01p Dave
 * Put in new multi host options screen. Tweaked multiplayer options a
 * bit.
 * 
 * 5     4/09/98 5:43p Dave
 * Remove all command line processing from the demo. Began work fixing up
 * the new multi host options screen.
 * 
 * 4     4/06/98 10:24p Dave
 * Fixed up Netgame.respawn for the standalone case.
 * 
 * 3     4/06/98 6:37p Dave
 * Put in max_observers netgame server option. Make sure host is always
 * defaulted to alpha 1 or zeta 1. Changed create game so that MAX_PLAYERS
 * can always join but need to be kicked before commit can happen. Put in
 * support for server ending a game and notifying clients of a special
 * condition.
 * 
 * 2     3/31/98 4:51p Dave
 * Removed medals screen and multiplayer buttons from demo version. Put in
 * new pilot popup screen. Make ships in mp team vs. team have proper team
 * ids. Make mp respawns a permanent option saved in the player file.
 * 
 * 1     3/30/98 6:24p Dave
 *  
 * 
 * $NoKeywords: $
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
	char		pxo_ip[MULTI_OPTIONS_STRING_LEN];					// ip address of pxo chat server
	char		pxo_rank_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo rankings page
	char		pxo_create_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo create account page
	char		pxo_verify_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo account validation page
	char		pxo_banner_url[MULTI_OPTIONS_STRING_LEN];			// URL of pxo banner files

	// standalone only options
	int		std_max_players;											// max players allowed on the standalone
	int		std_datarate;												// some OBJ_UPDATE_* value
	int		std_voice;													// should standalone allow voice
	char		std_passwd[STD_PASSWD_LEN];							// standalone host password
	char		std_pname[STD_NAME_LEN];								// permanent name for the standalone - if any
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
