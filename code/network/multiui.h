/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/MultiUI.h $
 * $Revision: 2.3 $
 * $Date: 2004-03-09 00:02:16 $
 * $Author: Kazan $
 *
 * Header file for the UI of the various multiplayer screens
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2004/03/05 09:02:02  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 7     5/04/99 5:20p Dave
 * Fixed up multiplayer join screen and host options screen. Should both
 * be at 100% now.
 * 
 * 6     2/23/99 2:29p Dave
 * First run of oldschool dogfight mode. 
 * 
 * 5     2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 4     11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 68    9/17/98 3:08p Dave
 * PXO to non-pxo game warning popup. Player icon stuff in create and join
 * game screens. Upped server count refresh time in PXO to 35 secs (from
 * 20).
 * 
 * 67    9/10/98 1:17p Dave
 * Put in code to flag missions and campaigns as being MD or not in Fred
 * and Freespace. Put in multiplayer support for filtering out MD
 * missions. Put in multiplayer popups for warning of non-valid missions.
 * 
 * 66    9/04/98 3:51p Dave
 * Put in validated mission updating and application during stats
 * updating.
 * 
 * 65    8/12/98 4:53p Dave
 * Put in 32 bit checksumming for PXO missions. No validation on the
 * actual tracker yet, though.
 * 
 * 64    6/05/98 9:54a Lawrance
 * OEM changes
 * 
 * 63    5/23/98 3:31p Dave
 * Tweaked pxo code. Fixed observer HUD stuff.
 * 
 * 62    5/20/98 2:25a Dave
 * Fixed server side voice muting. Tweaked multi debrief/endgame
 * sequencing a bit. Much friendlier for stats tossing/accepting now.
 * 
 * 61    5/15/98 12:09a Dave
 * New tracker api code. New game tracker code. Finished up first run of
 * the PXO screen. Fixed a few game server list exceptions.
 * 
 * 60    5/11/98 11:40p Dave
 * Stuff.
 * 
 * 59    5/09/98 7:16p Dave
 * Put in CD checking. Put in standalone host password. Made pilot into
 * popup scrollable.
 * 
 * 58    5/08/98 5:05p Dave
 * Go to the join game screen when quitting multiplayer. Fixed mission
 * text chat bugs. Put mission type symbols on the create game list.
 * Started updating standalone gui controls.
 * 
 * 57    5/04/98 10:39p Dave
 * Put in endgame sequencing.  Need to check campaign situations.
 * Realigned ship info on team select screen.
 * 
 * 56    5/04/98 1:44p Dave
 * Fixed up a standalone resetting problem. Fixed multiplayer stats
 * collection for clients. Make sure all multiplayer ui screens have the
 * correct palette at all times.
 * 
 * 55    4/30/98 12:57a Dave
 * Put in new mode for ship/weapon selection. Rearranged how game querying
 * is done a bit.
 * 
 * 54    4/23/98 6:19p Dave
 * Store ETS values between respawns. Put kick feature in the text
 * messaging system. Fixed text messaging system so that it doesn't
 * process or trigger ship controls. Other UI fixes.
 * 
 * 53    4/23/98 1:28a Dave
 * Seemingly nailed the current_primary_bank and current_secondary_bank -1
 * problem. Made sure non-critical button presses are _never_ sent to the
 * server.
 * 
 * 52    4/21/98 11:56p Dave
 * Put in player deaths statskeeping. Use arrow keys in the ingame join
 * ship select screen. Don't quit the game if in the debriefing and server
 * leaves.
 * 
 * 51    4/15/98 5:03p Dave
 * Put in a rough countdown to mission start on final sync screen. Fixed
 * several team vs. team bugs on the ship/team select screen.
 * 
 * 50    4/14/98 12:19p Dave
 * Revised the pause system yet again. Seperated into its own module.
 * 
 * 49    4/13/98 4:50p Dave
 * Maintain status of weapon bank/links through respawns. Put # players on
 * create game mission list. Make observer not have engine sounds. Make
 * oberver pivot point correct. Fixed respawn value getting reset every
 * time host options screen started.
 * 
 * 48    4/07/98 5:42p Dave
 * Put in support for ui display of voice system status (recording,
 * playing back, etc). Make sure main hall music is stopped before
 * entering a multiplayer game via ingame join.
 * 
 * 47    4/06/98 10:25p Dave
 * Fixed up Netgame.respawn for the standalone case.
 * 
 * 46    4/02/98 6:29p Lawrance
 * compile out multilag code for demo
 * 
 * 45    4/01/98 11:19p Dave
 * Put in auto-loading of xferred pilot pic files. Grey out background
 * behind pinfo popup. Put a chatbox message in when players are kicked.
 * Moved mission title down in briefing. Other ui fixes.
 * 
 * 44    3/28/98 3:20p Dave
 * Made pilot select popup handle large #'s of pilots correctly. Made
 * create game screen display/select missions correctly. Made temp
 * observers get stats after a mission. Numerous small ui fixes.
 * 
 * 43    3/26/98 5:24p Allender
 * put in respawn edit box into mission notes dialog.  Made loading of
 * missions/campaign happen when first entering the game setup screen.
 * 
 * 42    3/18/98 5:52p Dave
 * Put in netgame password popup. Numerous ui changes. Laid groundwork for
 * streamed multi_voice data.
 * 
 * 41    2/23/98 11:09p Dave
 * Finished up multiplayer campaign support. Seems bug-free.
 * 
 * 40    2/20/98 4:47p Allender
 * beefed up the multiplayer host interface when selection missions.
 * Don't reload all the missions everytime a new filter is set
 * 
 * 39    2/15/98 4:28p Dave
 * Made multiplayer mission lists display mission titles and descriptions
 * as well as filename. Removed some unneeded code in main hall.
 * 
 * 38    2/11/98 5:35p Dave
 * Standalone debugging. Changed how support ships warping in are handled.
 * Some UI code tidying up.
 * 
 * 37    2/10/98 8:39p Dave
 * Fixed bugs. Made endgame sequencing more clear.
 * 
 * 36    1/31/98 4:32p Dave
 * Put in new support for VMT player validation, game logging in, and game
 * logging out. Need to finish stats transfer.
 * 
 * 35    1/23/98 5:43p Dave
 * Finished bringing standalone up to speed. Coded in new host options
 * screen.
 * 
 * 34    1/22/98 5:26p Dave
 * Modified some pregame sequencing packets. Starting to repair broken
 * standalone stuff.
 * 
 * 33    1/21/98 5:58p Dave
 * Finished ingame join. Coded in multiplayer interface artwork changes.
 * 
 * 32    1/20/98 5:42p Dave
 * Moved ingame join to its own module. Improved it a bit.
 * 
 * 31    1/16/98 2:34p Dave
 * Made pause screen work properly (multiplayer). Changed how chat packets
 * work.
 * 
 * 30    1/15/98 6:12p Dave
 * Fixed weapons loadout bugs with multiplayer respawning. Added
 * multiplayer start screen. Fixed a few chatbox bugs.
 * 
 * 29    12/18/97 8:59p Dave
 * Finished putting in basic support for weapon select and ship select in
 * multiplayer.
 * 
 * 28    12/13/97 8:01p Dave
 * Installed multiplayer create game screen.
 * 
 * 27    12/13/97 3:01p Dave
 * Finished polishing up multiplayer join screen.
 * 
 * 26    12/13/97 1:59a Dave
 * Installed multiplayer join screen
 * 
 * 25    12/03/97 5:04p Sandeep
 * Fixed TCP/IP connections and you can play games off master tracker now
 * 
 * 24    11/15/97 2:37p Dave
 * More multiplayer campaign support.
 * 
 * 23    11/12/97 4:43p Hoffoss
 * Implemented new pause screens, moved old pause to debug pause.
 * 
 * 22    11/04/97 3:37p Dave
 * More sequencing overhauls. Respawning, and server transfer. 
 * 
 * 21    10/30/97 5:47p Dave
 * Smoothed transition between multiple multiplayer missions. Nailed a few
 * timestamp problems.
 * 
 * 20    10/29/97 5:18p Dave
 * More debugging of server transfer. Put in debrief/brief 
 * transition for multiplayer (w/standalone)
 * 
 * 19    10/22/97 6:26p Dave
 * Cleanup up yet more pregame sequencing. Got standalone up to working
 * condition.
 * 
 * 18    10/21/97 5:21p Dave
 * Fixed pregame mission load/file verify debacle. Added single vs.
 * multiplayer stats system.
 * 
 * 17    10/20/97 5:00p Dave
 * Installed new de-luxe ack system. Fixed a few potential chat/stats
 * bugs. Cleaned out some old code.
 * 
 * 16    10/09/97 5:24p Allender
 * fix misison time display.  Pass frametime into UI menus
 * 
 * 15    10/02/97 4:53p Dave
 * Finished all leave/join problems. Fixed file xfer with new ingame and
 * non-ingame situations. Fixed oddball timestamp problem.
 * 
 * 14    9/30/97 5:07p Dave
 * Finished up client-server remote commands. Finished up client-server
 * squadmate messaging. Added periodic auto-subsystem updates. Began work
 * on adapting ingame join to new player start system
 * 
 * 13    9/30/97 4:58p Allender
 * object updates on client side now happen according to different levels.
 * prediction code needs to be put in still
 * 
 * 12    9/15/97 4:43p Dave
 * Got basic observer mode working. Seems bug free so far.
 * 
 * 11    8/29/97 5:01p Dave
 * Put in multiplayer pause screen/feartures.
 * 
 * 10    8/20/97 4:21p Dave
 * Spliced out busy ACCEPT wait loop to work within the existing state
 * loop. Fixed display of mission description.
 * 
 * 9     7/24/97 10:06a Dave
 * Added scrollable game server list into multi_join_tracker_*
 * 
 * 8     7/23/97 4:54p Dave
 * Added multi_join_tracker_* functions for dealing with the tracker.
 * 
 * 7     7/02/97 12:59p Allender
 * chat hooks -- fixed a couple of chat bugs and reformetted the screen
 * slightly
 * 
 * 6     6/12/97 9:13a Allender
 * added sequencing state to the end of ship selection.  Changed some
 * packet names and host sequencing
 * 
 * 5     6/10/97 9:56p Allender
 * get multiplayer mission selection working.  Host can select mission and
 * have himself and clients load the mission -- no sequencing past this
 * point however
 * 
 * 4     6/06/97 10:40a Allender
 * added 'type' to mission (single/multi/etc).  Added a couple of new game
 * states for allowing to choose mission for multiplayer game
 * 
 * 3     1/01/97 6:45p Lawrance
 * added more multiplayer messages, improved code
 * 
 * 2     12/30/96 10:18a Lawrance
 * split up multiplayer code into manageable files
 *
 * $NoKeywords: $
 */


#ifndef MULTI_UI_H
#define MULTI_UI_H

#include "globalincs/globals.h"
#include "ui/ui.h"

struct net_player;
struct net_addr;

void multi_common_add_text(char *txt,int auto_scroll = 0);
void multi_common_set_text(char *str,int auto_scroll = 0);

// time between sending refresh packets to known servers
#define MULTI_JOIN_REFRESH_TIME			45000			
#define MULTI_JOIN_REFRESH_TIME_LOCAL	5000
// this time must be longer than the MULTI_JOIN_REFRESH_TIME but shorter than twice the MULTI_JOIN_REFRESH_TIME
// so that it does not time out between refresh times, but cannot last more than 2 complete refreshed without
// timing out. Just trust me - DB
#define MULTI_JOIN_SERVER_TIMEOUT			(MULTI_JOIN_REFRESH_TIME + (MULTI_JOIN_REFRESH_TIME /2))
#define MULTI_JOIN_SERVER_TIMEOUT_LOCAL	(MULTI_JOIN_REFRESH_TIME_LOCAL + (MULTI_JOIN_REFRESH_TIME_LOCAL / 2))

// maximum number of items which can be on the list
#if defined(DEMO) || defined(OEM_BUILD) // not for FS2_DEMO
	#define MULTI_CREATE_MAX_LIST_ITEMS			1
#else
	#define MULTI_CREATE_MAX_LIST_ITEMS			200
#endif

typedef struct {
	char		filename[MAX_FILENAME_LEN];	// filename of the mission
	char		name[NAME_LENGTH];				// name of the mission
	int		flags;								// flags to tell what type of multiplayer game (coop, team v. team)
	uint     respawn;								//	mission specified respawn count
	ubyte		max_players;						// max players allowed for this file	
	char		valid_status;						// see MVALID_* defines above
} multi_create_info;

// load all common icons
#define MULTI_NUM_COMMON_ICONS		12
#define MICON_VOICE_DENIED				0
#define MICON_VOICE_RECORDING			1
#define MICON_TEAM0						2
#define MICON_TEAM0_SELECT				3
#define MICON_TEAM1						4
#define MICON_TEAM1_SELECT				5
#define MICON_COOP						6
#define MICON_TVT							7
#define MICON_DOGFIGHT					8
#define MICON_VOLITION					9
#define MICON_VALID						10
#define MICON_CD							11

// common icon stuff
extern int Multi_common_icons[MULTI_NUM_COMMON_ICONS];
extern int Multi_common_icon_dims[MULTI_NUM_COMMON_ICONS][2];
void multi_load_common_icons();
void multi_unload_common_icons();

// initialize/display all bitmaps, etc related to displaying the voice system status
void multi_common_voice_display_status();

// multiplayer screen common palettes
void multi_common_load_palette();
void multi_common_set_palette();
void multi_common_unload_palette();

// call this to verify if we have a CD in the drive or not
void multi_common_verify_cd();

// variables to hold the mission and campaign lists
extern int Multi_create_mission_count;										// how many we have
extern int Multi_create_campaign_count;
extern multi_create_info Multi_create_mission_list[MULTI_CREATE_MAX_LIST_ITEMS];
extern multi_create_info Multi_create_campaign_list[MULTI_CREATE_MAX_LIST_ITEMS];

extern char Multi_create_files_array[MULTI_CREATE_MAX_LIST_ITEMS][MAX_FILENAME_LEN];
extern int Multi_create_files_array_count;

void multi_create_list_load_missions();
void multi_create_list_load_campaigns();

// returns an index into Multi_create_mission_list
int multi_create_lookup_mission(char *fname);

// returns an index into Multi_create_campaign_list
int multi_create_lookup_campaign(char *fname);

void multi_sg_rank_build_name(char *in,char *out);

void multi_join_game_init();
void multi_join_game_close();
void multi_join_game_do_frame();
void multi_join_eval_pong(net_addr *addr, fix pong_time);
void multi_join_reset_join_stamp();
void multi_join_clear_game_list();
void multi_join_notify_new_game();

void multi_start_game_init();
void multi_start_game_do();
void multi_start_game_close();

void multi_create_game_init();
void multi_create_game_do();
void multi_create_game_close();
void multi_create_game_add_mission(char *fname,char *name, int flags);

#define MULTI_CREATE_SHOW_MISSIONS			0
#define MULTI_CREATE_SHOW_CAMPAIGNS			1
void multi_create_setup_list_data(int mode);

void multi_create_handle_join(net_player *pl);

void multi_jw_handle_join(net_player *pl);

void multi_host_options_init();
void multi_host_options_do();
void multi_host_options_close();

void multi_game_client_setup_init();
void multi_game_client_setup_do_frame();
void multi_game_client_setup_close();

#define MULTI_SYNC_PRE_BRIEFING		0		// moving from the join to the briefing stage
#define MULTI_SYNC_POST_BRIEFING		1		// moving from the briefing to the gameplay stage
#define MULTI_SYNC_INGAME				2		// ingame joiners data sync
extern int Multi_sync_mode;					// should always set this var before calling GS_EVENT_MULTI_MISSION_SYNC
extern int Multi_sync_countdown;				// time in seconds until the mission is going to be launched
void multi_sync_init();
void multi_sync_do();
void multi_sync_close();
void multi_sync_start_countdown();			// start the countdown to launch when the launch button is pressed

// note : these functions are called from within missiondebrief.cpp - NOT from freespace.cpp
void multi_debrief_init();
void multi_debrief_do_frame();
void multi_debrief_close();
void multi_debrief_accept_hit();						// handle the accept button being hit
void multi_debrief_esc_hit();							// handle the ESC button being hit
void multi_debrief_replay_hit();						// handle the replay button being hit
void multi_debrief_server_left();					// call this when the server has left and we would otherwise be saying "contact lost with server"
void multi_debrief_stats_accept();					// call this to insure that stats are not undone when we leave the debriefing
void multi_debrief_stats_toss();						// call this to "toss" the stats packet
int multi_debrief_stats_accept_code();				// call this to determine the status of multiplayer stats acceptance
void multi_debrief_server_process();				// process all details regarding moving the netgame to its next state

// add a notification string, drawing appropriately depending on the state/screen we're in
void multi_common_add_notify(char *str);

// bring up the password string popup, fill in passwd (return 1 if accept was pressed, 0 if cancel was pressed)
int multi_passwd_popup(char *passwd);

// #Kazan#
void multi_servers_query();
void fs2netd_maybe_init();

#endif
