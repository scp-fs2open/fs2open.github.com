/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multi_pause.h"
#include "missionui/chatbox.h"
#include "io/key.h"
#include "popup/popup.h"
#include "gamesequence/gamesequence.h"
#include "network/stand_gui.h"
#include "gamesnd/gamesnd.h"
#include "network/multiutil.h"
#include "network/multiui.h"
#include "network/multimsgs.h"
#include "network/multi_endgame.h"
#include "network/multi_pmsg.h"
#include "playerman/player.h"
#include "network/multi.h"
#include "globalincs/alphacolors.h"
#include "io/timer.h"



// ----------------------------------------------------------------------------------
// PAUSE DEFINES/VARS
//

// state of the game (paused or not) on _my_ machine. Obviously this is important for the server
// call multi_pause_reset() to reinitialize
int Multi_pause_status = 0;

// who paused the game
net_player *Multi_pause_pauser = NULL;

// timestamp for eating keypresses for a while after 
float Multi_pause_eat = -1.0f;

// pause ui screen stuff
#define MULTI_PAUSED_NUM_BUTTONS		3

// button defs
#define MP_SCROLL_UP					0
#define MP_SCROLL_DOWN				1
#define MP_EXIT_MISSION				2

char *Multi_paused_bg_fname[GR_NUM_RESOLUTIONS] = {
	"MPPause",
	"2_MPPause"
};

char *Multi_paused_bg_mask[GR_NUM_RESOLUTIONS] = {
	"MPPause-m",
	"2_MPPause-m"
};

// where to place the pauser's callsign
int Mp_callsign_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		110, 132
	},
	{ // GR_1024
		171, 218
	}
};

ui_button_info Multi_paused_buttons[GR_NUM_RESOLUTIONS][MULTI_PAUSED_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("PB00",		519,	212,	-1,	-1,	0),
		ui_button_info("PB01",		519,	252,	-1,	-1,	1),
		ui_button_info("PB02",		488,	321,	-1,	-1,	2),
	},
	{ // GR_1024
		ui_button_info("2_PB00",		831,	339,	-1,	-1,	0),
		ui_button_info("2_PB01",		831,	403,	-1,	-1,	1),
		ui_button_info("2_PB02",		781,	514,	-1,	-1,	2),
	}
};

// text
#define MULTI_PAUSED_NUM_TEXT				3
UI_XSTR Multi_paused_text[GR_NUM_RESOLUTIONS][MULTI_PAUSED_NUM_BUTTONS] = {
	{ // GR_640
		{ "Exit",				1059,	493,	297,	UI_XSTR_COLOR_PINK,	-1, &Multi_paused_buttons[0][MP_EXIT_MISSION].button },
		{ "Mission",			1063,	482,	306,	UI_XSTR_COLOR_PINK,	-1, &Multi_paused_buttons[0][MP_EXIT_MISSION].button },
		{ "Mission Paused",	1440,	107,	356,	UI_XSTR_COLOR_PINK,	-1, NULL },
	},
	{ // GR_1024
		{ "Exit",				1059,	787,	478,	UI_XSTR_COLOR_PINK,	-1, &Multi_paused_buttons[1][MP_EXIT_MISSION].button },
		{ "Mission",			1063,	778,	490,	UI_XSTR_COLOR_PINK,	-1, &Multi_paused_buttons[1][MP_EXIT_MISSION].button },
		{ "Mission Paused",	1440,	171,	567,	UI_XSTR_COLOR_PINK,	-1, NULL },
	}
};


UI_WINDOW Multi_paused_window;
int Multi_paused_screen_id = -1;		// backed up screen data
int Multi_paused_background = -1;		// pause background

void multi_pause_check_buttons();
void multi_pause_button_pressed(int n);

// (server) evaluate a pause request from the given player (should call for himself as well)
void multi_pause_server_eval_request(net_player *pl, int pause);

// if this player can unpause
int multi_pause_can_unpause(net_player *p);

// render the callsign of the guy who paused
void multi_pause_render_callsign();

int Multi_paused = 0;

// ----------------------------------------------------------------------------------
// EXTERNAL FUNCTIONS/VARIABLES
//

extern void game_flush();

extern void weapon_pause_sounds();
extern void weapon_unpause_sounds();

extern void audiostream_pause_all();
extern void audiostream_unpause_all();

// ----------------------------------------------------------------------------------
// PAUSE FUNCTIONS
//

// re-initializes the pause system. call before entering the mission to reset
void multi_pause_reset()
{
	// set the pause status to 0
	Multi_pause_status = 0;

	// null out the pause pointer
	Multi_pause_pauser = NULL;

	// eat keys timestamp
	Multi_pause_eat = -1.0f;
}

// (client) call when receiving a packet indicating we should pause
void multi_pause_pause()
{
	int idx;

	// if we're already paused, don't do anything
	if(Multi_pause_status){
		return;
	}

	// sanity check
	Assert(!Multi_pause_status);

	// mark the game as being paused
	Multi_pause_status = 1;

	// if we're not already in the pause state
	if(gameseq_get_state() != GS_STATE_MULTI_PAUSED){
		// jump into the paused state 
		gameseq_post_event(GS_EVENT_MULTI_PAUSE);

		// mark the netgame state
		Netgame.game_state = NETGAME_STATE_PAUSED;					
	}

	// if we're the server of the game, send a packet which will pause the clients in the game now
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){
				send_client_update_packet(&Net_players[idx]);
			}
		}
	}
}

// (client) call when receiving a packet indicating we should unpause
void multi_pause_unpause()
{
	int idx;

	// if we're already unpaused, don't do anything
	if(!Multi_pause_status){
		return;
	}

	// sanity check
	Assert(Multi_pause_status);

	// mark the game as being unpaused
	Multi_pause_status = 0;

	// pop us out of any necessary states (including the pause state !!)
	multi_handle_state_special();
	
	// mark the netgame state
	Netgame.game_state = NETGAME_STATE_IN_MISSION;			

	// if we're the server of the game, send a packet which will unpause the clients in the game now
	// if we're the server of the game, send a packet which will pause the clients in the game now
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx])){
				send_client_update_packet(&Net_players[idx]);
			}
		}
	}
}

// send a request to pause or unpause a game (all players should use this function)
void multi_pause_request(int pause)
{
	// if i'm the server, run it through the eval function right now
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_pause_server_eval_request(Net_player,pause);
	} 
	// otherwise, send a reliable request packet to the server
	else {
		send_multi_pause_packet(pause);		
	}
}

// (server) evaluate a pause request from the given player (should call for himself as well)
void multi_pause_server_eval_request(net_player *pl, int pause)
{
	int cur_state;
	
	// if this is a pause request and we're already in the pause state, do nothing
	if(pause && Multi_pause_status){
		return;
	}

	// if this is an unpause request and we're already unpaused, do nothing
	if(!pause && !Multi_pause_status){
		return;
	}

	// get the current state (don't allow pausing from certain states
	cur_state = gameseq_get_state();
	if((cur_state == GS_STATE_DEBRIEF) || (cur_state == GS_STATE_MULTI_MISSION_SYNC) || (cur_state == GS_STATE_BRIEFING) ||
		(cur_state == GS_STATE_STANDALONE_POSTGAME) || (cur_state == GS_STATE_MULTI_STD_WAIT) || (cur_state == GS_STATE_WEAPON_SELECT) ||
		(cur_state == GS_STATE_TEAM_SELECT) || (cur_state == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)){
		return;
	}
	
	// if this is a pause request
	if(pause){
		// record who the pauser is
		Multi_pause_pauser = pl;

		// pause the game
		multi_pause_pause();		
	}
	// if this is an unpause request
	else {
		// if this guy is allowed to unpause the game, do so
		if(multi_pause_can_unpause(pl)){
			// unmark the "pauser"
			Multi_pause_pauser = NULL;

			// unpause the game
			multi_pause_unpause();			
		}
	}	
}

// if this player can unpause
int multi_pause_can_unpause(net_player *p)
{			 	
	if(!(p->flags & NETINFO_FLAG_GAME_HOST) && (p != Multi_pause_pauser)){
		return 0;
	}

	return 1;
}

// if we still want to eat keys 
int multi_pause_eat_keys()
{
	// if the eat timestamp is negative, don't eat keys
	if(Multi_pause_eat < 0.0f){
		return 0;
	} 

	// if less than 1 second has passed, continue eating keys
	if((f2fl(timer_get_fixed_seconds()) - Multi_pause_eat) < 1.0f){
		nprintf(("Network","PAUSE EATING KEYS\n"));

		control_config_clear_used_status();
		key_flush();

		return 1;
	}

	// otherwise, disable the timestamp
	Multi_pause_eat = -1.0f;

	return 0;
}


// ----------------------------------------------------------------------------------
// PAUSE UI FUNCTIONS
//

void multi_pause_init()
{
	int i;

	// if we're already paused. do nothing
	if ( Multi_paused ) {
		return;
	}

	Assert( Game_mode & GM_MULTIPLAYER );

	if ( !(Game_mode & GM_MULTIPLAYER) )
		return;

	// pause all beam weapon sounds
	weapon_pause_sounds();

	// standalone shouldn't be doing any freespace interface stuff
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_debug_set_standalone_state_string("Multi paused do");
	} 
	// everyone else should be doing UI stuff
	else {
		// pause all game music
		audiostream_pause_all();

		// switch off the text messaging system if it is active
		multi_msg_text_flush();

		if ( Multi_paused_screen_id == -1 )
			Multi_paused_screen_id = gr_save_screen();

		// create ui window
		Multi_paused_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
		Multi_paused_window.set_mask_bmap(Multi_paused_bg_mask[gr_screen.res]);
		Multi_paused_background = bm_load(Multi_paused_bg_fname[gr_screen.res]);

		for (i=0; i<MULTI_PAUSED_NUM_BUTTONS; i++) {
			// create the button
			Multi_paused_buttons[gr_screen.res][i].button.create(&Multi_paused_window, "", Multi_paused_buttons[gr_screen.res][i].x, Multi_paused_buttons[gr_screen.res][i].y, 1, 1, 0, 1);

			// set the highlight action
			Multi_paused_buttons[gr_screen.res][i].button.set_highlight_action(common_play_highlight_sound);

			// set the ani
			Multi_paused_buttons[gr_screen.res][i].button.set_bmaps(Multi_paused_buttons[gr_screen.res][i].filename);

			// set the hotspot
			Multi_paused_buttons[gr_screen.res][i].button.link_hotspot(Multi_paused_buttons[gr_screen.res][i].hotspot);
		}	

		// add text
		for(i=0; i<MULTI_PAUSED_NUM_TEXT; i++){
			Multi_paused_window.add_XSTR(&Multi_paused_text[gr_screen.res][i]);
		}
		
		// close any instances of a chatbox
		chatbox_close();

		// intialize our custom chatbox
		chatbox_create(CHATBOX_FLAG_MULTI_PAUSED);		
	}

	Multi_paused = 1;

	// reset timestamps
	multi_reset_timestamps();
}

void multi_pause_do()
{
	int k;
	
	// make sure we don't enter this state unless we're in the mission itself	
	Netgame.game_state = NETGAME_STATE_PAUSED;

	// server of the game should periodically be sending pause packets for good measure
	if (Net_player->flags & NETINFO_FLAG_AM_MASTER) {		
	}

	if (!(Game_mode & GM_STANDALONE_SERVER)) {
		// restore saved screen data if any
		if (Multi_paused_screen_id >= 0) {
			gr_restore_screen(Multi_paused_screen_id);
		}

		// set the background image
		if (Multi_paused_background >= 0) {
			gr_set_bitmap(Multi_paused_background);
			gr_bitmap(0, 0, GR_RESIZE_MENU);
		}

		// if we're inside of popup code right now, don't process the window
		if(!popup_active()){
			// process chatbox and window stuff
			k = chatbox_process();
			k = Multi_paused_window.process(k);	
		
			switch (k) {
			case KEY_ESC:			
			case KEY_PAUSE:									
				multi_pause_request(0);
				break;
			}
		}

		// check for any button presses
		multi_pause_check_buttons();

		// render the callsign of the guy who paused
		multi_pause_render_callsign();
				
		// render the chatbox
		chatbox_render();
		
		// draw tooltips
		// Multi_paused_window.draw_tooltip();
		Multi_paused_window.draw();

		// display the voice status indicator
		multi_common_voice_display_status();

		// don't flip screen if we are in the popup code right now
		if (!popup_active()) {
			gr_flip();
		}
	}
	// standalone pretty much does nothing here
	else {
		Sleep(1);
	}
}

void multi_pause_close(int end_mission)
{
	if ( !Multi_paused )
		return;

	// set the standalonest
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_debug_set_standalone_state_string("Game play");
	} else {
		// free the screen up
		if ( end_mission && (Multi_paused_screen_id >= 0) ) {
			gr_free_screen(Multi_paused_screen_id);
			Multi_paused_screen_id = -1;
		}

		if (Multi_paused_background >= 0) {
			bm_release(Multi_paused_background);
			Multi_paused_background = -1;
		}

		Multi_paused_window.destroy();		
		game_flush();

		// unpause all the music
		audiostream_unpause_all();	
	}

	// unpause beam weapon sounds
	weapon_unpause_sounds();

	// eat keys timestamp
	Multi_pause_eat = f2fl(timer_get_fixed_seconds());
	
	// reset timestamps
	multi_reset_timestamps();

	// clear out control config and keypress info
	control_config_clear_used_status();
	key_flush();

	Multi_paused = 0;
}

void multi_pause_check_buttons()
{
	int idx;

	// process any pause buttons which may have been pressed
	for (idx=0; idx<MULTI_PAUSED_NUM_BUTTONS; idx++){
		if (Multi_paused_buttons[gr_screen.res][idx].button.pressed()){
			multi_pause_button_pressed(idx);
		}
	}
}

void multi_pause_button_pressed(int n)
{
	switch (n) {
	// the scroll up button
	case MP_SCROLL_UP :
		chatbox_scroll_up();
		break;

	// the scroll down button
	case MP_SCROLL_DOWN :
		chatbox_scroll_down();
		break;

	// the exit mission
	case MP_EXIT_MISSION :
		multi_quit_game(PROMPT_ALL);
		break;
	}
}

// render the callsign of the guy who paused
void multi_pause_render_callsign()
{
	char pause_str[100];

	// write out the callsign of the player who paused the game	
	if((Multi_pause_pauser != NULL) && (Multi_pause_pauser->m_player != NULL)){
		memset(pause_str,0,100);
		strcpy_s(pause_str,Multi_pause_pauser->m_player->callsign);

		// blit it
		gr_set_color_fast(&Color_bright);
		gr_string(Mp_callsign_coords[gr_screen.res][0], Mp_callsign_coords[gr_screen.res][1], pause_str, GR_RESIZE_MENU);
	} 	
}
