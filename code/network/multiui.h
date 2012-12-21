/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef MULTI_UI_H
#define MULTI_UI_H

#include "globalincs/globals.h"
#include "ui/ui.h"
#include "network/multi.h"


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


typedef struct multi_create_info {
	char		filename[MAX_FILENAME_LEN];	// filename of the mission
	char		name[NAME_LENGTH];				// name of the mission
	int		flags;								// flags to tell what type of multiplayer game (coop, team v. team)
	uint     respawn;								//	mission specified respawn count
	ubyte		max_players;						// max players allowed for this file	
	char		valid_status;						// see MVALID_* defines above

	multi_create_info( )
		: flags( 0 ), respawn( 0 ), max_players( 0 ), valid_status( MVALID_STATUS_UNKNOWN )
	{
		filename[ 0 ] = 0;
		name[ 0 ] = 0;
	}
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
extern SCP_vector<multi_create_info> Multi_create_mission_list;
extern SCP_vector<multi_create_info> Multi_create_campaign_list;

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


#endif
