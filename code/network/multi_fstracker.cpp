/*
 * Copyright (C) Volition, Inc. 2005.  All rights reserved.
 * 
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _WIN32
#include <netinet/in.h>
#endif

#include "freespace.h"
#include "io/timer.h"
#include "gamesequence/gamesequence.h"
#include "popup/popup.h"
#include "network/psnet2.h"
#include "network/valid.h"						// tracker API
#include "network/gtrack.h"						// tracker API
#include "network/ptrack.h"						// tracker API
#include "network/multi.h"
#include "network/multi_fstracker.h"
#include "network/multiutil.h"
#include "network/multiui.h"
#include "network/multimsgs.h"
#include "network/multi_log.h"
#include "network/stand_gui.h"
#include "network/multi_pmsg.h"
#include "playerman/player.h"
#include "pilotfile/pilotfile.h"
#include "stats/medals.h"

// -----------------------------------------------------------------------------------
// FREESPACE MASTER TRACKER DEFINES/VARS
//

// if the fs tracker module has been successfully initialized
static int Multi_fs_tracker_inited = 0;

// if we're currently performing some operation with the tracker
static int Multi_fs_tracker_busy = 0;

// channel to associate when creating a server
char Multi_fs_tracker_channel[MAX_PATH] = "";

// channel to use when polling the tracker for games
char Multi_fs_tracker_filter[MAX_PATH] = "";

// used for mod detection
short Multi_fs_tracker_game_id = -1;
SCP_string Multi_fs_tracker_game_name;
SCP_string Multi_fs_tracker_game_tag;

enum PROBE_FLAGS {
	PENDING	= (1<<0),
	SUCCESS	= (1<<1),
	FAILURE	= (1<<2)
};

// -----------------------------------------------------------------------------------
// FREESPACE MASTER TRACKER FORWARD DECLARATIONS
//

// used with popup_till_condition() for validating freespace pilots
#define MT_VALIDATE_NOT_DONE				0								// still in the process of validation
#define MT_VALIDATE_SUCCEED				1								// successfully validated the pilot
#define MT_VALIDATE_FAIL					2								// failed in validating the pilot
#define MT_VALIDATE_TIMEOUT				3								// timedout on contacting the tracker
#define MT_VALIDATE_CANCEL					4								// if the action was cancelled
#define MT_PILOT_VAL_TIMEOUT				5000							// timeout for validating a pilot
static int Multi_validate_mode;													// 0 == getting player id, 1 == getting player stats
int multi_fs_validate_process();

// used with popup_till_condition() for logging in freespace games
#define MT_LOGIN_NOT_DONE					0								// still in the process of logging in
#define MT_LOGIN_SUCCEED					1								// successfully logged the game in
#define MT_LOGIN_TIMEOUT					2								// timedout on contacting the tracker

// used with popup_till_condition() for storing player stats at the end of a freespace game
#define MT_STATS_NOT_DONE					0								// still in the process of storing stats
#define MT_STATS_SUCCEED					1								// successfully logged all player stats

// used with popup_till_condition() for validating missions
#define MT_MVALID_NOT_DONE					0								// still in the process of validating
#define MT_MVALID_VALID						1								// mission is valid
#define MT_MVALID_INVALID					2								// mission is invalid
#define MT_MVALID_ERROR						3								// error while performing operation. assume invalid

// store stats mode defined
#define MT_STORE_STATS_VALIDATE			0
#define MT_STORE_STATS_GET_STATS			1
#define MT_STORE_STATS_ACCEPT				2
#define MT_STORE_STATS_SEND_STATS		3

static int Multi_store_stats_mode;												// 0 == initial request for player stats, 1 == waiting for player stats, 2 == tallying stats locally, 3 == sending stats to tracker
static int Multi_store_stats_player_index;										// player we're currently working with
static int Multi_store_stats_player_flag;										// if we're finished with the current guy
static vmt_stats_struct Multi_store_stats_stats;						//
int multi_fs_store_stats_do();											// manage all master tracker stats storing
int multi_fs_store_stats_get_next_player(int cur_player);	

static int Multi_tracker_player_is_valid = 0;
static int Multi_tracker_got_response = 0;

// copy a freespace stats struct to a tracker-freespace stats struct
void multi_stats_fs_to_tracker(scoring_struct *fs, vmt_stats_struct *vmt, player *pl, int tracker_id);

// copy a tracker-freespace stats struct to a freespace stats struct
void multi_stats_tracker_to_fs(vmt_stats_struct *vmt, scoring_struct *fs);

// process an incoming active game item
void multi_fs_tracker_process_game_item(game_list *gl);

// verify that there are no duplicate tracker id's to this one
void multi_fs_tracker_check_dup(int tracker_id,int player_index);

// verify that there are no duplicate pilot callsigns
void multi_fs_tracker_check_dup_callsign(net_player *player,int player_index);

// report on the results of the stats store procedure
void multi_fs_tracker_report_stats_results();

// tracker specific data structures
static pxo_net_game_data Multi_tracker_game_data;
static vmt_stats_struct Multi_tracker_fs_pilot;
static squad_war_response Multi_tracker_sw_response;

#define PXO_DEFAULT_TRACKER "tracker.pxo.nottheeye.com"
#define PXO_WEBSITE_URL "http://pxo.nottheeye.com"	// NOTE: *must* be http: for compatiblity

// -----------------------------------------------------------------------------------
// FREESPACE MASTER TRACKER DEFINITIONS
//

// give some processor time to the tracker API
void multi_fs_tracker_process()
{
	game_list *gl;

	PSNET_TOP_LAYER_PROCESS();

	if(Multi_fs_tracker_inited){
		// pilot validation system
		ValidIdle();

		// pilot tracing system
		PollPTrackNet();

		// game tracking system
		IdleGameTracker();

		// set if we've got any pending game list items
		gl = GetGameList();
		if(gl != nullptr){
			multi_fs_tracker_process_game_item(gl);
			gl = nullptr;
		}
	}
}

// initialize the master tracker API for Freespace
void multi_fs_tracker_init()
{	
	// don't do anything if we're already initialized
	if(Multi_fs_tracker_inited){
		return;
	}	
	
	// initialize the low-level validation stuff
	if(!InitValidateClient()){
		ml_printf("Error initializing tracker api (validateclient)");
		return;
	}

	// initialize the low-level pilot tracking stuff	
	if(!InitPilotTrackerClient()){		
		ml_printf("Error initializing tracker api (pilotclient)");
		return;
	}	

	// intialize the low-level game tracking stuff
	if(!InitGameTrackerClient(GT_FS2OPEN)){
		ml_printf("Error initializing tracker api (gameclient)");
		return;
	}	

	nprintf(("Network","Successfully initialized tracker api\n"));

	// we've successfully initialized the tracker stuff
	Multi_fs_tracker_inited = 1;
}

// validate the current player with the master tracker (will create the pilot on the MT if necessary)
int multi_fs_tracker_validate(int show_error)
{
	validate_id_request vir;	

	if(!Multi_fs_tracker_inited){
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("Warning, Parallax Online startup failed. Will not be able to play tracker games!",666));
		return 0;
	}
	
	// set this to false for now
	Multi_tracker_player_is_valid = 0;
	Multi_tracker_got_response = 0;

	// mark the module as busy
	Multi_fs_tracker_busy = 1;		

	while (true) {
		// validate our pilot on the master tracker if possible
		memset(&vir,0,sizeof(vir));
		SDL_zero(Multi_tracker_id_string);
		SDL_strlcpy(vir.login, Multi_tracker_login, SDL_arraysize(vir.login));
		SDL_strlcpy(vir.password, Multi_tracker_passwd, SDL_arraysize(vir.password));
		ValidateUser(&vir,Multi_tracker_id_string);
		
		// set validation mode
		Multi_validate_mode = 0;
			
		int rval = popup_till_condition(multi_fs_validate_process,XSTR("&Cancel",667),XSTR("Attempting to validate pilot ...",668));
		switch(rval){
		// if we failed for one reason or another
		case MT_VALIDATE_FAIL :
			// if we're supposed to show error codes
			if(show_error){
				popup(PF_USE_AFFIRMATIVE_ICON | PF_BODY_BIG,1,XSTR("&Ok",669),XSTR("Pilot rejected by Parallax Online!",670));
			}

			Multi_validate_mode = -1;

			Multi_fs_tracker_busy = 0;
			return 0;
			
		case MT_VALIDATE_SUCCEED :
			// notify the user
			if(Multi_tracker_fs_pilot.virgin_pilot){
				multi_common_add_notify(XSTR("Successfully created and validated new pilot!",671));
			} else {
				multi_common_add_notify(XSTR("Parallax Online pilot validation succeeded!",672));
			}

			// copy my statistics into my pilot file
			multi_stats_tracker_to_fs(&Multi_tracker_fs_pilot,&Player->stats);
			Pilot.set_multi_stats(&Player->stats);

			Multi_validate_mode = -1;

			Multi_fs_tracker_busy = 0;
			return 1;
			
		case MT_VALIDATE_TIMEOUT :
			rval = popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_BODY_BIG,2,XSTR("&Abort",673),XSTR("&Retry",674),XSTR("Validation timed out",675));			
			
			// if the user clicked abort, then leave. otherwise try again
			if(rval == 0){
				Multi_validate_mode = -1;

				Multi_fs_tracker_busy = 0;
				return 0;
			}
			break;
		
		default : 
			Multi_validate_mode = -1;

			Multi_fs_tracker_busy = 0;
		
			// essentially, cancel
			return -1;
		}
	}
}

// attempt to log the current game server in with the master tracker
void multi_fs_tracker_login_freespace()
{	
	if(!Multi_fs_tracker_inited){
		if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
			popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("Warning, Parallax Online startup failed. Will not be able to play tracker games!",666));
		}

		return;
	}

	// if we're already logged into a game, don't do anything
	if ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) || (Net_player->flags & NETINFO_FLAG_MT_CONNECTED) ) {
		return;
	}

	// pretty much all we do is make 1 call
	memset(&Multi_tracker_game_data, 0, sizeof(Multi_tracker_game_data));
	SDL_strlcpy(Multi_tracker_game_data.game_name, Netgame.name, SDL_arraysize(Multi_tracker_game_data.game_name));
	Multi_tracker_game_data.type = Netgame.type_flags;
	Multi_tracker_game_data.state = Netgame.game_state;
	Multi_tracker_game_data.max_players = MAX_PLAYERS;
	Multi_tracker_game_data.current_num_players = 0;

	// repurpose difficulty field to be netgame mode (both otherwise unused)
	if (Netgame.mode == NG_MODE_RANK_BELOW) {
		Multi_tracker_game_data.difficulty = -(100 + Netgame.rank_base);
	} else if (Netgame.mode == NG_MODE_RANK_ABOVE) {
		Multi_tracker_game_data.difficulty = (100 + Netgame.rank_base);
	} else {
		Multi_tracker_game_data.difficulty = Netgame.mode;
	}

	// if we have a valid channel string, use it		
	if(strlen(Multi_fs_tracker_channel)){
		SDL_strlcpy(Multi_tracker_game_data.channel, Multi_fs_tracker_channel, SDL_arraysize(Multi_tracker_game_data.channel));
	}	
	
	StartTrackerGame(&Multi_tracker_game_data);
	Net_player->flags |= NETINFO_FLAG_MT_CONNECTED;	

	// NETLOG
	ml_string(NOX("Server connected to Game Tracker"));
}

// attempt to update all player statistics and scores on the tracker
int multi_fs_tracker_store_stats()
{		
	int idx;

	// NETLOG
	ml_string(NOX("Server storing stats on User Tracker"));

	// retrieve stats from tracker
	Multi_store_stats_mode = MT_STORE_STATS_VALIDATE;
	
	// multi_fs_store_stats_do() will handle all details of negotiating stats transfer with the tracker
	Multi_store_stats_player_index = -1;
	Multi_store_stats_player_flag = 1;

	// mark the module as busy
	Multi_fs_tracker_busy = 1;

	// unmark everyone's GET_FAILED flag
	for(idx=0;idx<MAX_PLAYERS;idx++){
		Net_players[idx].flags &= ~(NETINFO_FLAG_MT_GET_FAILED);
		Net_players[idx].flags &= ~(NETINFO_FLAG_MT_SEND_FAILED);
		Net_players[idx].flags &= ~(NETINFO_FLAG_MT_DONE);
	}

	// if playing with any tables that are hacked/invalid
	if ( game_hacked_data() ) {
		send_game_chat_packet(Net_player, XSTR("<Server detected a hacked ships.tbl. Stats will not be saved>", 1044), MULTI_MSG_ALL, nullptr, nullptr, 1);
		multi_display_chat_msg(XSTR("<Server detected a hacked ships.tbl. Stats will not be saved>", 1044), 0, 0);
		popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("You are playing with a hacked ships.tbl, your stats will not be saved", 1045) );
		multi_fs_tracker_report_stats_results();
		Multi_fs_tracker_busy = 0;
		return 0;
	}

	// if there is only 1 player, don't store the stats
	if((multi_num_players() <= 1) && (Multi_num_players_at_start <= 1)){
		send_game_chat_packet(Net_player, XSTR("<Not enough players were present at game start or end, stats will not be saved>", 1048), MULTI_MSG_ALL, nullptr, nullptr, 1);
		multi_display_chat_msg(XSTR("<Not enough players were present at game start or end, stats will not be saved>", 1048), 0, 0);
		multi_fs_tracker_report_stats_results();
		Multi_fs_tracker_busy = 0;
		return 0;
	}	

	// if any players have hacked info
	for(idx=0; idx<MAX_PLAYERS; idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_HAXOR)){
			return 0;
		}
	}

	// check to see if the mission is valid
	if(multi_fs_tracker_validate_mission(Game_current_mission_filename) != MVALID_STATUS_VALID){
		send_game_chat_packet(Net_player, XSTR("<Server detected a non PXO validated mission. Stats will not be saved>", 1049), MULTI_MSG_ALL, nullptr, nullptr, 1);
		multi_display_chat_msg(XSTR("<Server detected a non PXO validated mission. Stats will not be saved>", 1049), 0, 0);
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK, XSTR("This is not a PXO validated mission, your stats will not be saved", 1050));
		Multi_fs_tracker_busy = 0;
		return 0;
	}

	popup_till_condition(multi_fs_store_stats_do,XSTR("&Cancel",667), XSTR("Sending player stats requests ...",676));	

	// send appropriate chat messages indicating stats store failure
	multi_fs_tracker_report_stats_results();

	// mark the module as not busy anymore
	Multi_fs_tracker_busy = 0;

	return 1;
}

// attempt to update all player statistics (standalone mode)
int multi_fs_std_tracker_store_stats()
{	
	int ret_val;
	int idx;

	// don't do anything if this is a tracker game
	if(!(MULTI_IS_TRACKER_GAME)){
		return 0;
	}

	if(!Multi_fs_tracker_inited){
		return 0;
	}

	// NETLOG
	ml_string(NOX("Standalone server storing stats on User Tracker"));

	// retrieve stats from tracker
	Multi_store_stats_mode = MT_STORE_STATS_VALIDATE;
	
	// multi_fs_store_stats_do() will handle all details of negotiating stats transfer with the tracker
	Multi_store_stats_player_index = -1;
	Multi_store_stats_player_flag = 1;

	// mark the module as busy
	Multi_fs_tracker_busy = 1;

	// unmark everyone's GET_FAILED flag
	for(idx=0;idx<MAX_PLAYERS;idx++){
		Net_players[idx].flags &= ~(NETINFO_FLAG_MT_GET_FAILED);
		Net_players[idx].flags &= ~(NETINFO_FLAG_MT_SEND_FAILED);
		Net_players[idx].flags &= ~(NETINFO_FLAG_MT_DONE);
	}

	// if playing with an invalid ships.tbl
	if(!Game_ships_tbl_valid){	
		send_game_chat_packet(Net_player, XSTR("<Server detected a hacked ships.tbl. Stats will not be saved>", 1044), MULTI_MSG_ALL, nullptr, nullptr, 1);
		multi_display_chat_msg(XSTR("<Server detected a hacked ships.tbl. Stats will not be saved>", 1044), 0, 0);
		multi_fs_tracker_report_stats_results();
		Multi_fs_tracker_busy = 0;
		return 0;
	}

	// if playing with an invalid weapons.tbl
	if(!Game_weapons_tbl_valid){	
		send_game_chat_packet(Net_player, XSTR("<Server detected a hacked weapons.tbl. Stats will not be saved>", 1046), MULTI_MSG_ALL, nullptr, nullptr, 1);
		multi_display_chat_msg(XSTR("<Server detected a hacked weapons.tbl. Stats will not be saved>", 1046), 0, 0);
		multi_fs_tracker_report_stats_results();
		Multi_fs_tracker_busy = 0;
		return 0;
	}

	// if any players have hacked info
	for(idx=0; idx<MAX_PLAYERS; idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_HAXOR)){
			return 0;
		}
	}

	// if there is only 1 player, don't store the stats	
	if((multi_num_players() <= 1) && (Multi_num_players_at_start <= 1)){
		send_game_chat_packet(Net_player, XSTR("<Not enough players were present at game start or end, stats will not be saved>", 1048), MULTI_MSG_ALL, nullptr, nullptr, 1);
		multi_display_chat_msg(XSTR("<Not enough players were present at game start or end, stats will not be saved>", 1048), 0, 0);
		multi_fs_tracker_report_stats_results();
		Multi_fs_tracker_busy = 0;
		return 0;
	}

	// check to see if the mission is valid	
	if(multi_fs_tracker_validate_mission(Game_current_mission_filename) != MVALID_STATUS_VALID){		
		send_game_chat_packet(Net_player, XSTR("<Server detected a non PXO validated mission. Stats will not be saved>", 1049), MULTI_MSG_ALL, nullptr, nullptr, 1);
		multi_display_chat_msg(XSTR("<Server detected a non PXO validated mission. Stats will not be saved>", 1049), 0, 0);
		Multi_fs_tracker_busy = 0;
		return 0;
	}
	
	// multi_fs_store_stats_do() will handle all details of negotiating stats transfer with the tracker
	do {		
		ret_val = multi_fs_store_stats_do();
		game_set_frametime(GS_STATE_STANDALONE_POSTGAME);
		multi_do_frame();
	} while(ret_val == MT_STATS_NOT_DONE);

	// report on the results		
	multi_fs_tracker_report_stats_results();

	// mark the module as no longer busy
	Multi_fs_tracker_busy = 0;

	return 1;
}

// log freespace out of the tracker
void multi_fs_tracker_logout()
{
	if(!Multi_fs_tracker_inited){
		return;
	}
	
	// make sure we're connected
	if(!(Net_player->flags & NETINFO_FLAG_AM_MASTER) || !(Net_player->flags & NETINFO_FLAG_MT_CONNECTED)){
		return;
	}

	// otherwise, log us out
	SendGameOver();

	// clear our data
	memset(&Multi_tracker_game_data, 0, sizeof(Multi_tracker_game_data));
	Net_player->flags &= ~(NETINFO_FLAG_MT_CONNECTED);

	// NETLOG
	ml_string(NOX("Server disconnecting from Game Tracker"));
}

// send a request for a list of games
void multi_fs_tracker_send_game_request()
{
	filter_game_list_struct filter;
	size_t len;
	
	// if we're not initialized, don't do anything
	if(!Multi_fs_tracker_inited){
		return;
	}	

	// if we have a valid filter, use that instead		
	len = strlen(Multi_fs_tracker_filter);
	if((len > 0) && (len < CHANNEL_LEN) ){
		memset(&filter,0,sizeof(filter_game_list_struct));		

		SDL_strlcpy(filter.channel, Multi_fs_tracker_filter, SDL_arraysize(filter.channel));
		RequestGameListWithFilter(&filter);
	} else {	
		// simple API call
		RequestGameList();
	}
}

// if the API has successfully been initialized and is running
int multi_fs_tracker_inited()
{
	return (Multi_fs_tracker_inited && Multi_options_g.pxo);
}

// update our settings on the tracker regarding the current netgame stuff
void multi_fs_tracker_update_game(netgame_info *ng)
{
	if(!Multi_fs_tracker_inited){
		return;
	}

	// copy in the relevant data
	SDL_strlcpy(Multi_tracker_game_data.game_name, ng->name, SDL_arraysize(Multi_tracker_game_data.game_name));

	Multi_tracker_game_data.type = ng->type_flags;
	Multi_tracker_game_data.state = ng->game_state;
	Multi_tracker_game_data.max_players = ng->max_players;
	Multi_tracker_game_data.current_num_players = multi_num_players();

	// repurpose difficulty field to be netgame mode (both otherwise unused)
	if (ng->mode == NG_MODE_RANK_BELOW) {
		Multi_tracker_game_data.difficulty = -(100 + ng->rank_base);
	} else if (ng->mode == NG_MODE_RANK_ABOVE) {
		Multi_tracker_game_data.difficulty = (100 + ng->rank_base);
	} else {
		Multi_tracker_game_data.difficulty = ng->mode;
	}

	SDL_strlcpy(Multi_tracker_game_data.mission_name, ng->mission_name, SDL_arraysize(Multi_tracker_game_data.mission_name));

	// NETLOG
	ml_string(NOX("Server updating netgame info for Game Tracker"));

	UpdateGameData(&Multi_tracker_game_data);
}

// if we're currently busy performing some tracker operation (ie, you should wait or not)
int multi_fs_tracker_busy()
{
	return Multi_fs_tracker_busy;
}


// -----------------------------------------------------------------------------------
// FREESPACE MASTER TRACKER FORWARD DEFINITIONS
//

// used with popup_till_condition() for validating freespace pilots
int multi_fs_validate_process()
{						
	// should never be here if this is not true
	Assert(Multi_fs_tracker_inited);

	PSNET_TOP_LAYER_PROCESS();

	// if we're still in player validation mode
	if(Multi_validate_mode == 0){
		switch(ValidateUser(nullptr,nullptr)){
		// timeout on waiting for response
		case -2 :
			return MT_VALIDATE_TIMEOUT;			

		// user invalid
		case -1:
			// set tracker id to -1
			SDL_strlcpy(Multi_tracker_id_string, "-1", SDL_arraysize(Multi_tracker_id_string));
			Multi_tracker_id = -1;
			return MT_VALIDATE_FAIL;			

		// still waiting
		case 0:
			return MT_VALIDATE_NOT_DONE;
	
		// user valid
		case 1:						
			// now we need to try and receive stats			

			// mark me as being valid
			Multi_tracker_player_is_valid = 1;

			// change the popup text
			popup_change_text(XSTR("Attempting to get pilot stats ...",679));

			// get my tracker id#
			Multi_tracker_id = atoi(Multi_tracker_id_string);			
			Assert(Multi_tracker_id != -1);

			GetFSPilotData(reinterpret_cast<vmt_stats_struct *>(static_cast<uintptr_t>(0xffffffff)),nullptr,nullptr,0);
			GetFSPilotData(&Multi_tracker_fs_pilot,Player->callsign,Multi_tracker_id_string,1);
				
			// set to mode 1
			Multi_validate_mode = 1;				
			return MT_VALIDATE_NOT_DONE;	
			
		default :
			Int3();
		}
	} else {				
		switch(GetFSPilotData(nullptr,nullptr,nullptr,0)){
		// timedout
		case -1:
			return MT_VALIDATE_TIMEOUT;			
				
		// still waiting
		case 0:
			return MT_VALIDATE_NOT_DONE;
			
		// got data
		case 1:
			return MT_VALIDATE_SUCCEED;			

		// failure
		case 3:
			return MT_VALIDATE_FAIL;
		}			
	}

	// we're not done yet - probably should never get here
	return MT_VALIDATE_NOT_DONE;
}

// used with popup_till_condition() for storing player stats at the end of a freespace game
int multi_fs_store_stats_do()
{			
	char tracker_id_string[512];
	char popup_text[100];

	Assert(Multi_fs_tracker_inited);

	PSNET_TOP_LAYER_PROCESS();

	switch(Multi_store_stats_mode){
	// get stats for all players
	case MT_STORE_STATS_VALIDATE : 
		Multi_store_stats_mode = MT_STORE_STATS_GET_STATS;
		break;

	case MT_STORE_STATS_GET_STATS:
		// if we need to get the next player		
		if(Multi_store_stats_player_flag){
			Multi_store_stats_player_index = multi_fs_store_stats_get_next_player(Multi_store_stats_player_index);
			
			// if it returns < 0 we're done with all players and should move onto the next stage (applying mission stats)
			if(Multi_store_stats_player_index < 0){
				Multi_store_stats_mode = MT_STORE_STATS_ACCEPT;
				return MT_STATS_NOT_DONE;
			}

			// unset this flag so we process the request
			Multi_store_stats_player_flag = 0;

			// fill out the information request
			memset(tracker_id_string,0,512);
			Assert(Net_players[Multi_store_stats_player_index].tracker_player_id > 0);

			// verify that there are no duplicate tracker id's to this one
			multi_fs_tracker_check_dup(Net_players[Multi_store_stats_player_index].tracker_player_id,Multi_store_stats_player_index);
			multi_fs_tracker_check_dup_callsign(&Net_players[Multi_store_stats_player_index],Multi_store_stats_player_index);

			SDL_snprintf(tracker_id_string, SDL_arraysize(tracker_id_string), "%d", Net_players[Multi_store_stats_player_index].tracker_player_id);
			Net_players[Multi_store_stats_player_index].s_info.tracker_security_last = -1;
			Net_players[Multi_store_stats_player_index].s_info.tracker_checksum = 0;

			// send the request itself
			GetFSPilotData(reinterpret_cast<vmt_stats_struct *>(static_cast<uintptr_t>(0xffffffff)), nullptr, nullptr,0);
			memset(&Multi_store_stats_stats, 0, sizeof(Multi_store_stats_stats));
			if(GetFSPilotData(&Multi_store_stats_stats, Net_players[Multi_store_stats_player_index].m_player->callsign,tracker_id_string,1) != 0){
				Int3();

				// move onto the next player
				Multi_store_stats_player_flag = 1;
				return MT_STATS_NOT_DONE;
			}

			// set the popup text
			if(!(Game_mode & GM_STANDALONE_SERVER)){
				SDL_snprintf(popup_text, SDL_arraysize(popup_text), XSTR("Getting player stats for %s...\n", 680), Net_players[Multi_store_stats_player_index].m_player->callsign);
				popup_change_text(popup_text);
			}
			return MT_STATS_NOT_DONE;
		}

		// process the request
		switch(GetFSPilotData(nullptr,nullptr,nullptr,0)){
		// got data
		case 1:
			// copy his stats, then flag him as done so we move onto the next guys
			multi_stats_tracker_to_fs(&Multi_store_stats_stats,&Net_players[Multi_store_stats_player_index].m_player->stats);

			// make sure we apply his mission stats now
			scoring_do_accept(&Net_players[Multi_store_stats_player_index].m_player->stats);

#ifndef NDEBUG
			{
				// debug code to check for bogus stats
				scoring_struct *ssp = &(Net_players[Multi_store_stats_player_index].m_player->stats);
				vmt_stats_struct *vmt = &Multi_store_stats_stats;
				
				if ( (ssp->missions_flown < vmt->missions_flown) || (ssp->flight_time < vmt->flight_time) || (ssp->kill_count < vmt->kill_count) ) {
					Int3();
				}
			}
#endif

			// flag him as being completed
			Multi_store_stats_player_flag = 1;

			// also store this last security value so we can properly update him
			Net_players[Multi_store_stats_player_index].s_info.tracker_security_last = Multi_store_stats_stats.security;
			Net_players[Multi_store_stats_player_index].s_info.tracker_checksum = Multi_store_stats_stats.checksum;
			break;

		// in progress
		case 0:
			break;

		// failure
		case 3: case -2: case 2: case -3: case -1:
			// this shouldn't be happening under most conditions. For debugging....
			Int3();

			// flag him as done so we move onto the next guy
			Multi_store_stats_player_flag = 1;
			Net_players[Multi_store_stats_player_index].s_info.tracker_security_last = -1;
			Net_players[Multi_store_stats_player_index].s_info.tracker_checksum = 0;

			// mark down that the stats get for him failed
			Net_players[Multi_store_stats_player_index].flags |= NETINFO_FLAG_MT_GET_FAILED;
			Net_players[Multi_store_stats_player_index].flags |= NETINFO_FLAG_MT_DONE;
			break;										
		}
		break;

	// update all stats for all players locally and on client machines
	case MT_STORE_STATS_ACCEPT :
		// tell everyone to save their stats
		send_store_stats_packet(1);

		// reset status flags and indices
		Multi_store_stats_player_index = -1;
		Multi_store_stats_player_flag = 1;

		Multi_store_stats_mode = MT_STORE_STATS_SEND_STATS;
		break;

	// send stats to the tracker
	case MT_STORE_STATS_SEND_STATS:
		// if we need to get the next player		
		if(Multi_store_stats_player_flag){
			Multi_store_stats_player_index = multi_fs_store_stats_get_next_player(Multi_store_stats_player_index);
			
			// if it returns < 0 we need to move onto the next player
			if(Multi_store_stats_player_index < 0){				
				return MT_STATS_SUCCEED;
			}
		
			Multi_store_stats_player_flag = 0;

			// fill in the information
			memset(&Multi_store_stats_stats,0,sizeof(Multi_store_stats_stats));
				
			Assert(Net_players[Multi_store_stats_player_index].tracker_player_id > 0);

			// verify that there are no duplicate tracker id's to this one
			multi_fs_tracker_check_dup(Net_players[Multi_store_stats_player_index].tracker_player_id,Multi_store_stats_player_index);
			multi_fs_tracker_check_dup_callsign(&Net_players[Multi_store_stats_player_index],Multi_store_stats_player_index);
			multi_stats_fs_to_tracker(&Net_players[Multi_store_stats_player_index].m_player->stats,&Multi_store_stats_stats,Net_players[Multi_store_stats_player_index].m_player,Net_players[Multi_store_stats_player_index].tracker_player_id);
			
			Multi_store_stats_stats.security = Net_players[Multi_store_stats_player_index].s_info.tracker_security_last;

			// Assert(Net_players[Multi_store_stats_player_index].s_info.tracker_checksum != 0);
			Multi_store_stats_stats.checksum = Net_players[Multi_store_stats_player_index].s_info.tracker_checksum;
				
			// send the request
			SendFSPilotData(reinterpret_cast<vmt_stats_struct *>(static_cast<uintptr_t>(0xffffffff)));
			if(SendFSPilotData(&Multi_store_stats_stats) != 0){
				Int3();

				// failed to send, try another player the next time around
				Multi_store_stats_player_flag = 1;
				return MT_STATS_NOT_DONE;
			}
			
			// set the popup text
			if(!(Game_mode & GM_STANDALONE_SERVER)){
				SDL_snprintf(popup_text, SDL_arraysize(popup_text), XSTR("Updating player stats for %s...\n", 681), Net_players[Multi_store_stats_player_index].m_player->callsign);
				popup_change_text(popup_text);
			}

			return MT_STATS_NOT_DONE;
		}
		
		// otherwise check on his status			
		switch(SendFSPilotData(nullptr)){
		// error
		case -1: case -2: case -3: case 2: case 3:
			// flag him as done so we move onto the next guy
			Multi_store_stats_player_flag = 1;					

			Net_players[Multi_store_stats_player_index].flags |= NETINFO_FLAG_MT_SEND_FAILED;
			Net_players[Multi_store_stats_player_index].flags |= NETINFO_FLAG_MT_DONE;
			break;
			
		// got data
		case 1:
			// flag him as done so we move onto the next guys					
			Multi_store_stats_player_flag = 1;
			Net_players[Multi_store_stats_player_index].flags |= NETINFO_FLAG_MT_DONE;
			break;			
		}		
		
		break;
	}	
		
	// return not done yet
	return MT_STATS_NOT_DONE;	
}

// copy a freespace stats struct to a tracker-freespace stats struct
void multi_stats_fs_to_tracker(scoring_struct *fs, vmt_stats_struct *vmt, player *pl, int tracker_id)
{
	// tracker id
	vmt->tracker_id = tracker_id;

	// pilot callsign
	SDL_strlcpy(vmt->pilot_name, pl->callsign, SDL_arraysize(vmt->pilot_name));

	// score and rank
	vmt->score = fs->score;
	vmt->rank = fs->rank;

	// kills and assists
	vmt->assists = fs->assists;
	vmt->kill_count = fs->kill_count;
	vmt->kill_count_ok = fs->kill_count_ok;


	// shot statistics
	vmt->p_shots_fired = fs->p_shots_fired;
	vmt->s_shots_fired = fs->s_shots_fired;
	vmt->p_shots_hit = fs->p_shots_hit;
	vmt->s_shots_hit = fs->s_shots_hit;
	vmt->p_bonehead_hits = fs->p_bonehead_hits;
	vmt->s_bonehead_hits = fs->s_bonehead_hits;
	vmt->bonehead_kills = fs->bonehead_kills;

	// missions flown information
	vmt->missions_flown = fs->missions_flown;
	vmt->flight_time = fs->flight_time;
	vmt->last_flown = static_cast<unsigned int>(fs->last_flown);

	// medals and ship kills are stored in a single array, medals first
	Assert(fs->medal_counts.size() >= Medals.size());
	vmt->num_medals = static_cast<unsigned char>(Medals.size());

	if (vmt->num_medals > MAX_FS2OPEN_COUNTS) {
		vmt->num_medals = MAX_FS2OPEN_COUNTS;
	}

	// find only up to last in array with at least 1 kill
	vmt->num_ships = MAX_FS2OPEN_COUNTS - vmt->num_medals;

	for (int idx = vmt->num_ships-1; idx >= 0; --idx) {
		if (fs->kills[idx] > 0) {
			break;
		}

		--vmt->num_ships;
	}

	// medals should always fit here, ships might get cut off on large mods
	const size_t count = vmt->num_medals + vmt->num_ships;

	for (size_t idx = 0, idx2 = 0; idx < count; ++idx) {
		if (idx < vmt->num_medals) {
			vmt->counts[idx] = static_cast<unsigned short>(fs->medal_counts[idx]);
		} else {
			vmt->counts[idx] = static_cast<unsigned short>(fs->kills[idx2++]);
		}
	}
}

// copy a tracker-freespace stats struct to a freespace stats struct
void multi_stats_tracker_to_fs(vmt_stats_struct *vmt,scoring_struct *fs)
{
	// score and rank
	fs->score = vmt->score;
	fs->rank = vmt->rank;

	// kills and assists
	fs->assists = vmt->assists;
	fs->kill_count = vmt->kill_count;
	fs->kill_count_ok = vmt->kill_count_ok;

	// shot statistics
	fs->p_shots_fired = vmt->p_shots_fired;
	fs->s_shots_fired = vmt->s_shots_fired;
	fs->p_shots_hit = vmt->p_shots_hit;
	fs->s_shots_hit = vmt->s_shots_hit;
	fs->p_bonehead_hits = vmt->p_bonehead_hits;
	fs->s_bonehead_hits = vmt->s_bonehead_hits;
	fs->bonehead_kills = vmt->bonehead_kills;

	// missions flown information
	fs->missions_flown = vmt->missions_flown;
	fs->flight_time = vmt->flight_time;
	fs->last_flown = static_cast<_fs_time_t>(vmt->last_flown);
	if(fs->last_flown < 0){
		fs->last_flown = 0;
	}

	fs->last_backup = fs->last_flown;

	// medals and ship kills are stored in a single array, medals first
	const size_t count = vmt->num_medals + vmt->num_ships;

	Assert(count <= MAX_FS2OPEN_COUNTS);

	const size_t max_medals = std::max(Medals.size(), static_cast<size_t>(vmt->num_medals));
	fs->medal_counts.assign(max_medals, 0);

	for (size_t idx = 0, idx2 = 0; idx < count; ++idx) {
		if (idx < vmt->num_medals) {
			if (idx < max_medals) {
				fs->medal_counts[idx] = static_cast<int>(vmt->counts[idx]);
			}
		} else {
			fs->kills[idx2++] = static_cast<int>(vmt->counts[idx]);
		}
	}
}

// process an incoming active game item
void multi_fs_tracker_process_game_item(game_list *gl)
{
	active_game ag;	
	int idx;

	for(idx=0;idx<MAX_GAME_LISTS_PER_PACKET;idx++){
		// skip null server addresses
		if (IN6_IS_ADDR_UNSPECIFIED(&gl->game_server[idx])) {
			continue;
		}

		// package up the game information
		ag.init();
		SDL_strlcpy(ag.name, gl->game_name[idx], SDL_arraysize(ag.name));

		memcpy(&ag.server_addr.addr, &gl->game_server[idx], sizeof(ag.server_addr.addr));
		ag.server_addr.port = ntohs(gl->port[idx]); //DEFAULT_GAME_PORT;

		// add to the active game list
		// multi_update_active_games(&ag);

		// query this server
		send_server_query(&ag.server_addr);
	}
}

int multi_fs_store_stats_get_next_player(int cur_player)
{
	int idx;

	// if we're at the end of the list
	if(cur_player == (MAX_PLAYERS - 1)){
		return -2;
	}

	// find the next player
	for(idx=cur_player+1;idx<MAX_PLAYERS;idx++){
		// STANDALONE_ONLY
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].tracker_player_id != -1) && !(Net_players[idx].flags & NETINFO_FLAG_MT_GET_FAILED) ){
			return idx;
		}
	}

	// couldn't find one
	return -2;
}

// verify that there are no duplicate tracker id's to this one
void multi_fs_tracker_check_dup(int tracker_id,int player_index)
{
	int idx;

	// compare against all players except the passed player_index	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(idx == player_index){
			continue;
		}
		
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].tracker_player_id != -1)){
			Assert(Net_players[idx].tracker_player_id != tracker_id);
		}
	}
}

// verify that there are no duplicate pilot callsigns
void multi_fs_tracker_check_dup_callsign(net_player *player,int player_index)
{
	int idx;

	// compare against all players except the passed player_index	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(idx == player_index){
			continue;
		}
		
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].tracker_player_id != -1)){
			Assert(strcmp(player->m_player->callsign,Net_players[idx].m_player->callsign));
		}
	}
}

// return an MVALID_STATUS_* constant
int multi_fs_tracker_validate_mission_std()
{
	int ret_val;
	
	// wait for a response from the tracker
	do {		
		ret_val = ValidateMission(nullptr);
	} while(ret_val == 0);

	// report on the results
	switch(ret_val){
	// timeout
	case -2:
		// consider timeout to be fatal and cancel validation
		return -2;	//MVALID_STATUS_UNKNOWN;

	// invalid
	case -1:
		return MVALID_STATUS_INVALID;

	// valid, success
	case 1:
		return MVALID_STATUS_VALID;
	}

	Int3();
	return 0;
}

// special return values :
// 1 for timeout
// 2 for invalid
// 3 for valid
int multi_fs_tracker_validate_mission_normal()
{	
	switch(ValidateMission(nullptr)){
	// timeout
	case -2:
		return 1;

	// invalid
	case -1 :
		return 2;

	// valid
	case 1:
		return 3;
	}

	// not done yet
	return 0;
}	

// return an MVALID_STATUS_* (see multiui.h) value, or -2 if the user has "cancelled"
int multi_fs_tracker_validate_mission(char *filename)
{	
	vmt_validate_mission_req_struct mission;	
	char popup_string[512] = "";

	if(!Multi_fs_tracker_inited){
		return MVALID_STATUS_UNKNOWN;
	}

	mprintf(("Standalone: Validating '%s'\n", filename));
	
	// get the checksum of the local file	
	memset(&mission, 0, sizeof(mission));
	SDL_strlcpy(mission.file_name, filename, SDL_arraysize(mission.file_name));
	if(!cf_chksum_long(mission.file_name, reinterpret_cast<uint *>(&mission.checksum))){
		return MVALID_STATUS_UNKNOWN;
	}	

	// try and validate the mission
	if(ValidateMission(&mission) != 0){
		return MVALID_STATUS_UNKNOWN;
	}

	// do frames for standalone and non-standalone
	if(Game_mode & GM_STANDALONE_SERVER){		
		int ret_code;

		// set the filename in the dialog
		std_gen_set_text(filename, 2);

		// validate the mission
		ret_code = multi_fs_tracker_validate_mission_std();

		// if the dialog is no longer active, cancel everything
		//if(!std_gen_is_active()){
		//	return MVALID_STATUS_UNKNOWN;
		//}		

		return ret_code;
	} else {
		SDL_snprintf(popup_string, SDL_arraysize(popup_string), XSTR("Validating mission %s", 1074), filename);

		// run a popup
		switch ( popup_conditional_do(multi_fs_tracker_validate_mission_normal, popup_string) ) {
		// cancel 
		case 0: 
			// bash some API values here so that next time we try and verify, everything works
			MissionValidState = VALID_STATE_IDLE;
			return -2;

		// timeout
		case 1:
			return MVALID_STATUS_UNKNOWN;

		// invalid
		case 2:
			return MVALID_STATUS_INVALID;

		// valid
		case 3:
			return MVALID_STATUS_VALID;
		}
	}

	return MVALID_STATUS_UNKNOWN;
}

// return a MVALID_STATUS_* constant
int multi_fs_tracker_validate_data_std()
{
	int ret_val;

	// wait for a response from the tracker
	do {
		ret_val = ValidateData(nullptr);
	} while (ret_val == 0);

	// report on the results
	switch (ret_val) {
		// timeout
		case -2:
			// consider timeout to be fatal and cancel validation
			return -2;	//MVALID_STATUS_UNKNOWN;

		// invalid
		case -1:
			return MVALID_STATUS_INVALID;

		// valid, success
		case 1:
			return MVALID_STATUS_VALID;
	}

	Int3();

	return 0;
}

// special return values :
// 1 for timeout
// 2 for invalid
// 3 for valid
int multi_fs_tracker_validate_data_normal()
{
	switch ( ValidateData(nullptr) ) {
		// timeout
		case -2:
			return 1;

		// invalid
		case -1 :
			return 2;

		// valid
		case 1:
			return 3;
	}

	// not done yet
	return 0;
}

int multi_fs_tracker_validate_data(const vmt_valid_data_req_struct *vdr, const char *popup_text = nullptr)
{
	if ( !Multi_fs_tracker_inited ) {
		return MVALID_STATUS_UNKNOWN;
	}

	if (ValidateData(vdr) != 0) {
		return MVALID_STATUS_UNKNOWN;
	}

	if (Game_mode & GM_STANDALONE_SERVER) {
		std_gen_set_text(popup_text, 1);

		// validate the data
		int ret_code = multi_fs_tracker_validate_data_std();

		return ret_code;
	} else {
		switch ( popup_conditional_do(multi_fs_tracker_validate_data_normal, popup_text) ) {
			// cancel
			case 0:
				DataValidState = VALID_STATE_IDLE;
				return -2;

			// timeout
			case 1:
				DataValidState = VALID_STATE_IDLE;
				return -3;

			// invalid
			case 2:
				return MVALID_STATUS_INVALID;

			// valid
			case 3:
				return MVALID_STATUS_VALID;
		}
	}

	return MVALID_STATUS_UNKNOWN;
}

// batch validate missions and fill passed struct with results
// returns:
//		true if process completed successfully
//		false if process was terminated from timeout or by cancellation
bool multi_fs_tracker_validate_mission_list(SCP_vector<multi_create_info> &file_list)
{
	vmt_valid_data_req_struct vdr;
	char popup_string[512] = "";
	int rval;

	vdr.type = VDR_TYPE_MISSION;
	vdr.flags = VDR_FLAG_STATUS;
	vdr.num_files = 0;

	size_t packet_size = 3;	// type, flags, num_files

	for (size_t idx = 0; idx < file_list.size(); ) {
		auto &entry = file_list[idx];
		const size_t len = sizeof(uint32_t) + strlen(entry.filename) + 1;

		if (packet_size+len < MAX_UDP_DATA_LENGH) {
			valid_data_item item;

			cf_chksum_long(entry.filename, &item.crc);
			item.name = entry.filename;

			vdr.files.push_back(item);
			vdr.num_files++;

			packet_size += len;

			++idx;
		} else {
			// so we don't start at 0%
			if (idx != vdr.num_files) {
				SDL_snprintf(popup_string, SDL_arraysize(popup_string), XSTR("Validating missions ... %d%%", -1), static_cast<int>(((idx - vdr.num_files) / static_cast<float>(file_list.size())) * 100));
			}

			// send packet
			rval = multi_fs_tracker_validate_data(&vdr, popup_string);

			// if it was cancelled then just bail out
			if (rval == -2 || rval == -3) {
				return false;
			}

			// set status for each file checked
			auto istart = idx - vdr.num_files;

			for (auto p = 0; p < vdr.num_files; ++p) {
				file_list[istart+p].valid_status = IsDataIndexValid(p) ? MVALID_STATUS_VALID : MVALID_STATUS_INVALID;
			}

			// reset counts for next run
			packet_size = 3;	// type, flags, num_files
			vdr.files.clear();
			vdr.num_files = 0;
		}
	}

	// final packet
	if (vdr.num_files) {
		// so we don't start at 0%
		if (file_list.size() != vdr.num_files) {
			SDL_snprintf(popup_string, SDL_arraysize(popup_string), XSTR("Validating missions ... %d%%", -1), static_cast<int>(((file_list.size() - vdr.num_files) / static_cast<float>(file_list.size())) * 100));
		}

		// send packet
		rval = multi_fs_tracker_validate_data(&vdr, popup_string);

		// if it was cancelled then just bail out
		if (rval == -2) {
			return false;
		}

		// set status for each file checked
		auto istart = file_list.size() - vdr.num_files;

		for (auto p = 0; p < vdr.num_files; ++p) {
			file_list[istart+p].valid_status = IsDataIndexValid(p) ? MVALID_STATUS_VALID : MVALID_STATUS_INVALID;
		}
	}

	// done!
	return true;
}

static int validate_table_list(const SCP_vector<SCP_string> &table_list, int &game_data_status)
{
	vmt_valid_data_req_struct vdr;
	char popup_string[512] = "";
	int rval;

	strcpy_s(popup_string, XSTR("Validating tables ...", -1));

	vdr.type = VDR_TYPE_TABLE;
	vdr.flags = VDR_FLAG_IDENT;
	vdr.num_files = 0;

	size_t packet_size = 3;	// type, flags, num_files

	for (size_t idx = 0; idx < table_list.size(); ) {
		const auto &tbl = table_list[idx];
		const size_t len = sizeof(uint32_t) + tbl.length() + 1;

		if (packet_size+len < MAX_UDP_DATA_LENGH) {
			valid_data_item item;

			cf_chksum_long(tbl.c_str(), &item.crc);
			item.name = tbl;

			vdr.files.push_back(item);
			vdr.num_files++;

			packet_size += len;

			++idx;
		} else {
			// so we don't start at 0%
			if (idx != vdr.num_files) {
				SDL_snprintf(popup_string, SDL_arraysize(popup_string), XSTR("Validating tables ... %d%%", -1), static_cast<int>(((idx - vdr.num_files) / static_cast<float>(table_list.size())) * 100));
			}

			// send packet
			rval = multi_fs_tracker_validate_data(&vdr, popup_string);

			// reset counts for next run
			packet_size = 3;	// type, flags, num_files
			vdr.files.clear();
			vdr.num_files = 0;

			// if anything is valid then update our default
			if ( (rval == MVALID_STATUS_VALID) && (game_data_status == MVALID_STATUS_UNKNOWN) ) {
				game_data_status = MVALID_STATUS_VALID;
			}
			// continue processing after invalid for mod ident to fully work
			else if (rval == MVALID_STATUS_INVALID) {
				game_data_status = MVALID_STATUS_INVALID;
			}
			// if the popup was canceled then force a recheck on next attempt
			else if (rval == -2) {
				game_data_status = MVALID_STATUS_UNKNOWN;
				break;
			}
			// if we timed out then log it and return error
			else if (rval == -3) {
				game_data_status = MVALID_STATUS_UNKNOWN;

				if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
					popup_conditional_close();
				}

				ml_printf("PXO Game Ident timed out!  Unable to connect to server: %s", Multi_options_g.user_tracker_ip);

				return -1;
			}
		}
	}

	// final packet
	if (vdr.num_files) {
		// so we don't start at 0%
		if (table_list.size() != vdr.num_files) {
			SDL_snprintf(popup_string, SDL_arraysize(popup_string), XSTR("Validating tables ... %d%%", -1), static_cast<int>(((table_list.size() - vdr.num_files) / static_cast<float>(table_list.size())) * 100));
		}

		rval = multi_fs_tracker_validate_data(&vdr, popup_string);

		// if anything is valid then update our default
		if ( (rval == MVALID_STATUS_VALID) && (game_data_status == MVALID_STATUS_UNKNOWN) ) {
			game_data_status = MVALID_STATUS_VALID;
		}
		// continue processing after invalid for mod ident to fully work
		else if (rval == MVALID_STATUS_INVALID) {
			game_data_status = MVALID_STATUS_INVALID;
		}
		// if the popup was canceled then force a recheck on next attempt
		else if (rval == -2) {
			game_data_status = MVALID_STATUS_UNKNOWN;
		}
		// if we timed out then log it and return error
		else if (rval == -3) {
			game_data_status = MVALID_STATUS_UNKNOWN;

			if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
				popup_conditional_close();
			}

			ml_printf("PXO Game Ident timed out!  Unable to connect to server: %s", Multi_options_g.user_tracker_ip);

			return -1;
		}
	}

	return 0;
}

// check all tables with tracker
// this is hacked data check as well as mod ident (done server side)
// returns:
//     1 if hacked (or no ident) - stats won't save
//     0 if ident and not hacked - stats can save
//    -1 if server connection failed
int multi_fs_tracker_validate_game_data()
{
	// should only do this only once per game session
	static int game_data_status = MVALID_STATUS_UNKNOWN;
	char popup_string[512] = "";

	multi_fs_tracker_init();

	if ( !Multi_fs_tracker_inited ) {
		return 1; // assume hacked
	}

	if (game_data_status != MVALID_STATUS_UNKNOWN) {
		return (game_data_status != MVALID_STATUS_VALID);
	}


	SCP_vector<SCP_string> table_list;
	size_t tbl_idx, tbm_idx;
	int rval;

	// grab all tbl and tbm files and send the checksum to tracker
	// cf_get_file_list() strips ext so we have to do this in parts
	// let the tracker figure out which files to validate against

	// get all tbl files first
	cf_get_file_list(table_list, CF_TYPE_TABLES, "*.tbl");

	// add ext back on filenames
	for (tbl_idx = 0; tbl_idx < table_list.size(); ++tbl_idx) {
		table_list[tbl_idx].append(".tbl");
	}

	// next grab any tbm files
	cf_get_file_list(table_list, CF_TYPE_TABLES, "*.tbm");

	// and add ext
	for (tbm_idx = tbl_idx; tbm_idx < table_list.size(); ++tbm_idx) {
		table_list[tbm_idx].append(".tbm");
	}

	// now check with tracker
	strcpy_s(popup_string, XSTR("Validating tables ...", -1));

	if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
		popup_conditional_create(0, XSTR("&Cancel", 667), popup_string);
	}

	rval = validate_table_list(table_list, game_data_status);

	// if the connection timed out then maybe try a fallback...
	if ( (rval == -1) && (psnet_get_ip_mode() == PSNET_IP_MODE_DUAL) ) {
		// the default preference is IPv6, so fall back to IPv4
		// otherwise move to 4 or 6 depending on current preference

		if ( !Cmdline_prefer_ipv4 && !Cmdline_prefer_ipv6 ) {
			Cmdline_prefer_ipv4 = true;
		} else if (Cmdline_prefer_ipv4) {
			Cmdline_prefer_ipv4 = false;
			Cmdline_prefer_ipv6 = true;
		} else if (Cmdline_prefer_ipv6) {
			Cmdline_prefer_ipv4 = true;
			Cmdline_prefer_ipv6 = false;
		}

		// now we have to re-init the low-level tracker connections
		InitValidateClient();
		InitPilotTrackerClient();
		InitGameTrackerClient(GT_FS2OPEN);

		// and try to validate again
		validate_table_list(table_list, game_data_status);
	}

	if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
		popup_conditional_close();
	}

	// we should hopefully have a mod id now, so log it
	if (Multi_fs_tracker_game_id < 0) {
		ml_printf("PXO Game Ident => FAILED!");
	} else {
		ml_printf("PXO Game Ident => %d: %s", Multi_fs_tracker_game_id, Multi_fs_tracker_game_name.c_str());
	}

	return (game_data_status != MVALID_STATUS_VALID);
}

// report on the results of the stats store procedure
void multi_fs_tracker_report_stats_results()
{
	int idx;
	char str[512] = "";

	// tell everyone stats store is complete
	SDL_strlcpy(str, XSTR("<PXO stats store process complete>", 1001), SDL_arraysize(str));
	send_game_chat_packet(Net_player, str, MULTI_MSG_ALL, nullptr, nullptr, 1);
	multi_display_chat_msg(str, 0, 0);	
	ml_string(str);

	// for all players
	for(idx=0; idx<MAX_PLAYERS; idx++){		
		// for all players who we care about
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			// if the stats get or send failed for any player, report as such
			if(((Net_players[idx].tracker_player_id <= 0) || (Net_players[idx].flags & NETINFO_FLAG_MT_GET_FAILED) || (Net_players[idx].flags & NETINFO_FLAG_MT_SEND_FAILED) || !(Net_players[idx].flags & NETINFO_FLAG_MT_DONE)) && (Net_players[idx].m_player != nullptr)){
				SDL_snprintf(str, SDL_arraysize(str), XSTR("<PXO stats store failed for player %s>", 1002), Net_players[idx].m_player->callsign);
				send_game_chat_packet(Net_player, str, MULTI_MSG_ALL, nullptr, nullptr, 1);
				
				multi_display_chat_msg(str, 0, 0);
				ml_string(str);
			}
		}
	}
}

// report the status of PXO game probe (firewall check)
void multi_fs_tracker_report_probe_status(int flags, int next_try)
{
	static int last_flags = 0;
	SCP_string str;

	// if the flags haven't changed since last time then just bail
	// *except* if the probe failed since we always want that message
	// (don't & the flag check here, needs to be exact)
	if ( (flags == last_flags) && (flags != PROBE_FLAGS::FAILURE) ) {
		return;
	}

	if (flags & PROBE_FLAGS::PENDING) {
		// if we've been here before just bail (to avoid log spam)
		if (last_flags) {
			last_flags = flags;
			return;
		}

		str = "<PXO firewall probe in progress...>";
	} else if (flags & PROBE_FLAGS::FAILURE) {
		char t_str[64];

		str = "<PXO firewall probe failed! Next attempt in ";

		if (next_try < 120) {
			SDL_snprintf(t_str, SDL_arraysize(t_str), "%d seconds...>", next_try);
		} else if (next_try < (60*60+1)) {
			int minutes = next_try / 60;
			SDL_snprintf(t_str, SDL_arraysize(t_str), "%d minutes...>", minutes);
		} else {
			int hours = next_try / (60*60);
			SDL_snprintf(t_str, SDL_arraysize(t_str), "%d hours...>", hours);
		}

		str += t_str;
	} else if (flags & PROBE_FLAGS::SUCCESS) {
		str = "<PXO firewall probe was a success!>";
	} else {
		// getting here shouldn't happen, but it's not technically fatal
		return;
	}

	last_flags = flags;

	multi_display_chat_msg(str.c_str(), 0, 0);
	ml_string(str.c_str());
}

// return an MSW_STATUS_* constant
int multi_fs_tracker_validate_sw_std()
{
	int ret_val;
	
	// wait for a response from the tracker
	do {		
		ret_val = ValidateSquadWar(nullptr, &Multi_tracker_sw_response);
	} while(ret_val == 0);

	// report on the results
	switch(ret_val){
	// timeout
	case -2:		
		return MVALID_STATUS_UNKNOWN;

	// invalid
	case -1:		
		return MVALID_STATUS_INVALID;

	// valid, success
	case 1:		
		return MVALID_STATUS_VALID;
	}
	
	return MVALID_STATUS_UNKNOWN;
}

// special return values :
// 1 for timeout
// 2 for invalid
// 3 for valid
int multi_fs_tracker_validate_sw_normal()
{	
	switch(ValidateSquadWar(nullptr, &Multi_tracker_sw_response)){
	// timeout
	case -2:
		return 1;

	// invalid
	case -1 :
		return 2;

	// valid
	case 1:
		return 3;
	}

	// not done yet
	return 0;
}	

#define STUFF_SW_RESPONSE(_c, _len) do {\
	SDL_strlcpy(_c, "", _len);\
	int _idx;\
	int _bogus = 1;\
	for(_idx=0; _idx<MAX_SQUAD_RESPONSE_LEN; _idx++){\
		if(Multi_tracker_sw_response.reason[_idx] == '\0'){\
			_bogus = 0;\
			break;\
		}\
	}\
	if(!_bogus){\
		SDL_strlcpy(_c, Multi_tracker_sw_response.reason, _len);\
	}\
} while(false);

// return an MSW_STATUS_* value
int multi_fs_tracker_validate_sw(squad_war_request *sw_req, char *bad_reply, const size_t max_reply_len)
{
	char popup_string[512] = "";

	if(!Multi_fs_tracker_inited){
		return MSW_STATUS_UNKNOWN;
	}	

	// zero the response
	memset(&Multi_tracker_sw_response, 0, sizeof(Multi_tracker_sw_response));
	
	// try and validate the mission
	if(ValidateSquadWar(sw_req, &Multi_tracker_sw_response) != 0){
		SDL_strlcpy(bad_reply, "Error sending request for Squad War validation", max_reply_len);

		return MSW_STATUS_UNKNOWN;
	}

	// do frames for standalone and non-standalone
	if(Game_mode & GM_STANDALONE_SERVER){		
		int ret_code;		

		// validate the mission
		ret_code = multi_fs_tracker_validate_sw_std();		

		// copy the return code
		STUFF_SW_RESPONSE(bad_reply, max_reply_len);

		return ret_code;
	} else {
		SDL_strlcpy(popup_string, XSTR("Validating squad war", 1075), SDL_arraysize(popup_string));

		// run a popup
		switch(popup_till_condition(multi_fs_tracker_validate_sw_normal, XSTR("&Cancel", 645), popup_string)){
		// cancel 
		case -1:
		case 0: 
			// bash some API values here so that next time we try and verify, everything works
			SquadWarValidState = VALID_STATE_IDLE;

			// copy the return code
			STUFF_SW_RESPONSE(bad_reply, max_reply_len);
			return -2;

		// timeout
		case 1:
			// copy the return code			
			SDL_strlcpy(bad_reply, "Timeout", max_reply_len);
			return MSW_STATUS_UNKNOWN;

		// invalid
		case 2:
			// copy the return code
			STUFF_SW_RESPONSE(bad_reply, max_reply_len);
			return MSW_STATUS_INVALID;

		// valid
		case 3:
			// copy the return code
			STUFF_SW_RESPONSE(bad_reply, max_reply_len);
			return MSW_STATUS_VALID;
		}
	}

	SDL_strlcpy(bad_reply, "Unknown error", max_reply_len);
	return MSW_STATUS_UNKNOWN;
}

// popup do function
// -3	Error -- Called with NULL, but no request is waiting
// -2	Error -- Already sending data (hasn't timed out yet)
// -1	Timeout trying to send pilot data
// 0	Sending
// 1	Data succesfully sent
// 2	Send Cancelled (data may still have been written already, we just haven't been ACK'd yet)
// 3	Pilot not written (for some reason)
  
int multi_fs_tracker_store_sw_do()
{
	switch(SendSWData(nullptr, &Multi_tracker_sw_response)){
	// failure
	case -3:
	case -2:
	case -1:
	case 2:
	case 3:
		return 1;

	// success
	case 1:
		return 10;
	}

	// not done
	return 0;
}

// store the results of a squad war mission on PXO, return 1 on success
int multi_fs_tracker_store_sw(squad_war_result *sw_res, char * /*bad_reply*/, const size_t /*max_reply_len*/)
{
	char popup_string[512] = "";

	// clear any old requests
	SendSWData(reinterpret_cast<squad_war_result *>(static_cast<uintptr_t>(0xffffffff)), nullptr);

	// send this new request
	SendSWData(sw_res, &Multi_tracker_sw_response);

	// standalone
	if(Game_mode & GM_STANDALONE_SERVER){
		int ret_code;
		do {
			ret_code = SendSWData(nullptr, &Multi_tracker_sw_response);
		} while(ret_code == 0);

		// success
		if(ret_code == 1){
			return 1;
		}
	}
	// non-standalone
	else {
		SDL_strlcpy(popup_string, XSTR("Storing SquadWar results", 1078), SDL_arraysize(popup_string));

		// wait for a response
		if(popup_till_condition(multi_fs_tracker_store_sw_do, XSTR("&Cancel", 645), popup_string) == 10){
			// success
			return 1;
		}
	}

	// failure
	return 0;
}

// verify and possibly update Multi_options_g with sane PXO values
void multi_fs_tracker_verify_options()
{
	bool override = false;

	// if any tracker ip is missing, force update
	if ( !strlen(Multi_options_g.user_tracker_ip) || !strlen(Multi_options_g.game_tracker_ip)
		 || !strlen(Multi_options_g.pxo_ip)	)
	{
		override = true;
	}

	// if user tracker ip is set to retail setting, force update
	if ( !override && !strcmp(Multi_options_g.user_tracker_ip, "ut.pxo.net") ) {
		override = true;
	}

	// if user tracker ip is set to FS2NetD, force update
	if ( !override && !strcmp(Multi_options_g.user_tracker_ip, "fs2netd.game-warden.com") ) {
		override = true;
	}

	if ( !override ) {
		return;
	}

	//
	// update required options
	//

	strcpy_s(Multi_options_g.user_tracker_ip, PXO_DEFAULT_TRACKER);
	strcpy_s(Multi_options_g.game_tracker_ip, PXO_DEFAULT_TRACKER);
	strcpy_s(Multi_options_g.pxo_ip, PXO_DEFAULT_TRACKER);

	//
	// update optional urls
	//

	strcpy_s(Multi_options_g.pxo_rank_url, PXO_WEBSITE_URL);
	strcat_s(Multi_options_g.pxo_rank_url, "/rankings");

	strcpy_s(Multi_options_g.pxo_create_url, PXO_WEBSITE_URL);
	strcat_s(Multi_options_g.pxo_create_url, "/register");

	strcpy_s(Multi_options_g.pxo_verify_url, PXO_WEBSITE_URL);
	strcat_s(Multi_options_g.pxo_verify_url, "/account");

	strcpy_s(Multi_options_g.pxo_banner_url, PXO_WEBSITE_URL);
	strcat_s(Multi_options_g.pxo_banner_url, "/files/banners");
}
