/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include <limits.h>		// this is need even when not building debug!!

#include "globalincs/globals.h"
#include "object/object.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multiui.h"
#include "mission/missionparse.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "network/multi_ingame.h"
#include "missionui/missionscreencommon.h"
#include "popup/popup.h"
#include "network/multi_observer.h"
#include "network/multi_xfer.h"
#include "network/multi_kick.h"
#include "menuui/mainhallmenu.h"
#include "stats/stats.h"
#include "network/multiteamselect.h"
#include "missionui/missionweaponchoice.h"
#include "network/multi_endgame.h"
#include "hud/hudshield.h"
#include "mission/missionhotkey.h"
#include "globalincs/alphacolors.h"
#include "io/timer.h"
#include "playerman/player.h"
#include "network/multi_log.h"


// --------------------------------------------------------------------------------------------------
// DAVE's BIGASS INGAME JOIN WARNING/DISCLAIMER
//
// Ingame joining is another delicate system. Although not as delicate as server transfer, it will
// help to take as many precautions as possible when handling ingame joins. Please be sure to follow
// all the same rules as explained in multi_strans.h
//
// --------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------
// INGAME JOIN DESCRIPTION
//
// 1.) Joiner sends a JOIN packet to the server
// 2.) If the server accepts him, he receives an ACCEPT packet in return
// 3.) The client then moves into the INGAME_SYNC state to begin receiving data from the server
// 4.) The first thing he does on this screen is send his filesig packet to the server. At which 
//     point the server will either let him in or deny him. There are no file transfers ingame.
// 5.) The server calls multi_handle_ingame_joiners() once per frame, through multi_do_frame()
// 6.) After verifiying or kicking the player because of his file signature, the server tells the
//     player to load the mission
// 7.) When the mission is loaded, the server, sends a netgame update to the client
// 8.) Without waiting, the server then begins sending data ship packets to the player
// 9.) Upon confirmation of receiving these packets, the server sends wing data packets
// 10.) Upon completion of this, the server sends respawn point packets
// 11.) Upon completion of this, the server sends a post briefing data block packet containing ship class and 
//      weapon information
// 12.) After this, the server sends a player settings packet (to all players for good measure)
// 13.) At this point, the server sends a jump into mission packet
// 14.) Upon receipt of this packet, the client moves into the ingame ship select state
// 15.) The first thing the client does in this state is load the mission data (textures, etc)
// 16.) The player is presented with a list of ships he can choose from. He selects one and sends
//      an INGAME_SHIP_REQUEST to the server. 
// 17.) The server checks to see if this request is acceptable and sends an INGAME_SHIP_REQUEST back
//      with the appropriate data.
// 18.) If the client received an affirmative, he selects the ship and jumps into the mission, otherwise
//      he removes it from the list and tries for another ship
// --------------------------------------------------------------------------------------------------


LOCAL	int	Ingame_ships_deleted = 0;
//LOCAL	int	Ingame_ships_to_delete[MAX_SHIPS];


// --------------------------------------------------------------------------------------------------
// INGAME JOIN FORWARD DECLARATIONS
//

void multi_ingame_send_ship_update(net_player *p);

void multi_ingame_join_check_buttons();
void multi_ingame_join_button_pressed(int n);



// --------------------------------------------------------------------------------------------------
// INGAME JOIN COMMON DEFINITIONS
//


// --------------------------------------------------------------------------------------------------
// INGAME JOIN SERVER FUNCTIONS
//

// called on the server to process ingame joiners and move them through the motions of ingame joining
void multi_handle_ingame_joiners()
{
	int idx;

	Assert( MULTIPLAYER_MASTER );

	// if my ingame joining flag isn't set, then don't do anything.
	if ( !(Netgame.flags & NG_FLAG_INGAME_JOINING) ){
		return;
	}

	// traverse through all the players
	for(idx = 0; idx<MAX_PLAYERS; idx++){
		// only process ingame joiners
		if( !(Net_players[idx].flags & NETINFO_FLAG_RELIABLE_CONNECTED) || !(Net_players[idx].flags & NETINFO_FLAG_INGAME_JOIN)){
			continue;
		}
		
		// if we're waiting for players to receive files, then check on their status
		if(Net_players[idx].s_info.ingame_join_flags & INGAME_JOIN_FLAG_FILE_XFER){						
			switch(multi_xfer_get_status(Net_players[idx].s_info.xfer_handle)){
			// if it has successfully completed, set his ok flag
			case MULTI_XFER_SUCCESS :
				// set his ok flag
				Net_players[idx].flags |= NETINFO_FLAG_MISSION_OK;

				// release the xfer instance handle
				multi_xfer_release_handle(Net_players[idx].s_info.xfer_handle);
				Net_players[idx].s_info.xfer_handle = -1;
				break;
			// if it has failed or timed-out, kick the player
			case MULTI_XFER_TIMEDOUT:
			case MULTI_XFER_FAIL:
				// release the xfer handle
				multi_xfer_release_handle(Net_players[idx].s_info.xfer_handle);
				Net_players[idx].s_info.xfer_handle = -1;
						
				// kick the loser
				multi_kick_player(idx, 0, KICK_REASON_BAD_XFER);
				break;
			}						
		}		
		
		// if the player has verified his file signature then send him the packet to load the mission and mark this down
		if((Net_players[idx].flags & NETINFO_FLAG_MISSION_OK) && !(Net_players[idx].s_info.ingame_join_flags & INGAME_JOIN_FLAG_LOADING_MISSION)){
			// send the netgame update here as well
			send_netgame_update_packet(&Net_players[idx]);
			send_netplayer_update_packet(&Net_players[idx]);

			// send the packet and mark it down
			send_netplayer_load_packet(&Net_players[idx]);			
			Net_players[idx].s_info.ingame_join_flags |= INGAME_JOIN_FLAG_LOADING_MISSION;						
		}

		// once he has finished loading the mission, start sending him ship data packets and mark this down
		if((Net_players[idx].state == NETPLAYER_STATE_MISSION_LOADED) && !(Net_players[idx].s_info.ingame_join_flags & INGAME_JOIN_FLAG_SENDING_SHIPS)){
			int i;

			// send the packet and mark it down
			for (i = 0; i < Num_teams; i++ ) {
				if(Game_mode & GM_STANDALONE_SERVER){
					send_wss_slots_data_packet(i, 1, &Net_players[idx], 0);
				} else {
					send_wss_slots_data_packet(i, 1, &Net_players[idx]);
				}
			}

			// mark the netgame as a critical stage in ingame joining so that I don't evaluate mission event/
			// goals, ship arrivals, etc.
			Netgame.flags |= NG_FLAG_INGAME_JOINING_CRITICAL;

			// send the packet and mark it down
			send_ingame_ships_packet(&Net_players[idx]);
			Net_players[idx].s_info.ingame_join_flags |= INGAME_JOIN_FLAG_SENDING_SHIPS;
		}

		// once he has finished receiving the ship data, start sending him wing data and mark this down
		/*
		if((Net_players[idx].state == NETPLAYER_STATE_INGAME_SHIPS) && !(Net_players[idx].s_info.ingame_join_flags & INGAME_JOIN_FLAG_SENDING_WINGS)){
			// setup the list of wings to send
			Net_players[idx].s_info.wing_index = 0;
			Net_players[idx].s_info.wing_index_backup = 0;

			// send the packet and mark it down
			send_ingame_wings_packet(&Net_players[idx]);
			Net_players[idx].s_info.ingame_join_flags |= INGAME_JOIN_FLAG_SENDING_WINGS;
		}
		*/

		// once he has received the respawn packet, send him the player settings for all the players in the game and mark this down
		if((Net_players[idx].state == NETPLAYER_STATE_INGAME_SHIPS) && !(Net_players[idx].s_info.ingame_join_flags & INGAME_JOIN_FLAG_SENDING_POST)){
			// reset the critical ingame joining flag so that I as server, will start evaluating mission
			// things again
			Netgame.flags &= ~NG_FLAG_INGAME_JOINING_CRITICAL;

			// send the packet and mark it down
			if(Game_mode & GM_STANDALONE_SERVER){
				send_post_sync_data_packet(&Net_players[idx],0);
			} else {
				send_post_sync_data_packet(&Net_players[idx]);
			}
			Net_players[idx].s_info.ingame_join_flags |= INGAME_JOIN_FLAG_SENDING_POST;
		}				

		// once the settings have been received, send him the jump into mission packet and mark this down. now the joiner
		// moves into the ship select state (ingame)
		if((Net_players[idx].state == NETPLAYER_STATE_POST_DATA_ACK) && !(Net_players[idx].s_info.ingame_join_flags & INGAME_JOIN_FLAG_PICK_SHIP)){			
			// if this guy is an obsever, create his observer object and be done!
			if(Net_players[idx].flags & NETINFO_FLAG_OBSERVER){
				multi_obs_create_observer(&Net_players[idx]);
				Net_players[idx].flags &= ~(NETINFO_FLAG_INGAME_JOIN);
				Netgame.flags &= ~(NG_FLAG_INGAME_JOINING_CRITICAL | NG_FLAG_INGAME_JOINING);
			}
		
			// send the packet and mark it down
			send_jump_into_mission_packet(&Net_players[idx]);
			Net_players[idx].s_info.ingame_join_flags |= INGAME_JOIN_FLAG_PICK_SHIP;
		}

		// check to see if his timestamp for ship update (hull, shields, etc) has popped. If so, send some info and reset
		if(timestamp_elapsed(Net_players[idx].s_info.last_full_update_time)){
			// send the ships
			multi_ingame_send_ship_update(&Net_players[idx]);

			// reset the timestamp
			Net_players[idx].s_info.last_full_update_time = timestamp(INGAME_SHIP_UPDATE_TIME);
		}

		// once he has received the weapon state packet, send him the player settings for all the players in the game and mark this down
		if(!(Net_players[idx].s_info.ingame_join_flags & INGAME_JOIN_FLAG_SENDING_SETS)){			
			// send the packet and mark it down
			// this will update _ALL_ players in the game which is important
			send_player_settings_packet();
			send_player_settings_packet( &Net_players[idx] );		// send directly so he gets the packet
		
			Net_players[idx].s_info.ingame_join_flags |= INGAME_JOIN_FLAG_SENDING_SETS;
		}		
	}
}

// the final step for an ingame joining observer - create my observer object, unflag myself as joining and jump into mission
void multi_ingame_observer_finish()
{
	// create my local observer object
	multi_obs_create_observer_client();

	// unflag myself as being an ingame joiner
	Net_player->flags &= ~(NETINFO_FLAG_INGAME_JOIN);

	// set my state to be in-mission
	Net_player->state = NETPLAYER_STATE_IN_MISSION;
	send_netplayer_update_packet();

	// jump into the game
	gameseq_post_event(GS_EVENT_ENTER_GAME);
}

// --------------------------------------------------------------------------------------------------
// INGAME DATA SYNC SCREEN 
//

// mission sync screen init function for ingame joining
void multi_ingame_sync_init()
{	
	// if we couldn't get the file signature correctly. send some bogus values
	multi_get_mission_checksum(Game_current_mission_filename);
	
	// everyone should re-initialize these 
	init_multiplayer_stats();

	// reset all sequencing info
	multi_oo_reset_sequencing();

	// send the file signature to the host for possible mission file transfer
	strcpy_s(Netgame.mission_name,Game_current_mission_filename);
	send_file_sig_packet(Multi_current_file_checksum,Multi_current_file_length);
	
	Ingame_ships_deleted = 0;
}

// mission sync screen do function for ingame joining
void multi_ingame_sync_do()
{	
}

// mission sync screen do function for ingame joining
void multi_ingame_sync_close()
{
}


// --------------------------------------------------------------------------------------------------
// INGAME SHIP SELECT SCREEN 
//
static char *Multi_ingame_join_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MultiIngame",				// GR_640
	"2_MultiIngame"			// GR_1024
};

static char *Multi_ingame_join_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MultiIngame-M",			// GR_640
	"2_MultiIngame-M"			// GR_1024
};


// button defs
#define MULTI_INGAME_JOIN_NUM_BUTTONS       2
#define MIJ_CANCEL		0
#define MIJ_JOIN			1

ui_button_info Multi_ingame_join_buttons[GR_NUM_RESOLUTIONS][MULTI_INGAME_JOIN_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info( "MIB_00",	532,	434,	510,	413,	0 ),						// cancel
		ui_button_info( "MIB_01",	572,	428,	585,	413,	1 ),						// join
	},
	{ // GR_1024
		ui_button_info( "2_MIB_00",	851,	695,	916,	685,	0 ),						// cancel
		ui_button_info( "2_MIB_01",	916,	685,	950,	665,	1 ),						// join
	}
};

#define MULTI_INGAME_JOIN_NUM_TEXT			8

UI_XSTR Multi_ingame_join_text[GR_NUM_RESOLUTIONS][MULTI_INGAME_JOIN_NUM_TEXT] = {
	{ // GR_640		
		{"Cancel",							387,	510,	413,	UI_XSTR_COLOR_PINK, -1, &Multi_ingame_join_buttons[GR_640][MIJ_CANCEL].button},	
		{"Join",								1303,	585,	413,	UI_XSTR_COLOR_PINK, -1, &Multi_ingame_join_buttons[GR_640][MIJ_JOIN].button},
		{"Select Ship",					317,	39,	6,		UI_XSTR_COLOR_PINK, -1, NULL},
		{"name",								1423,	39,	28,	UI_XSTR_COLOR_GREEN, -1, NULL},
		{"class",							1424,	145,	28,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"status",							1425,	214,	28,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"primary",							1426,	295,	28,	UI_XSTR_COLOR_GREEN, -1, NULL},
		{"secondary",						1427,	440,	28,	UI_XSTR_COLOR_GREEN, -1, NULL}
	},
	{ // GR_1024		
		{"Cancel",							387,	843,	665,	UI_XSTR_COLOR_PINK, -1, &Multi_ingame_join_buttons[GR_1024][MIJ_CANCEL].button},	
		{"Join",								1303,	950,	665,	UI_XSTR_COLOR_PINK, -1, &Multi_ingame_join_buttons[GR_1024][MIJ_JOIN].button},
		{"Select Ship",					317,	63,	14,	UI_XSTR_COLOR_PINK, -1, NULL},
		{"name",								1423,	63,	45,	UI_XSTR_COLOR_GREEN, -1, NULL},
		{"class",							1424,	233,	45,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"status",							1425,	343,	45,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"primary",							1426,	473,	45,	UI_XSTR_COLOR_GREEN, -1, NULL},
		{"secondary",						1427,	704,	45,	UI_XSTR_COLOR_GREEN, -1, NULL}
	}
};

#define MI_FIELD_X		0
#define MI_FIELD_Y		1
#define MI_FIELD_W		2
#define MI_FIELD_H		3

static int Mi_width[GR_NUM_RESOLUTIONS] = { 
	569,		// GR_640
	910		// GR_1024
};

static int Mi_height[GR_NUM_RESOLUTIONS] = {
	339,		// GR_640
	542		// GR_1024
};

static int Mi_spacing[GR_NUM_RESOLUTIONS] = {
	30,
	48
};

static int Mi_name_field[GR_NUM_RESOLUTIONS][4] = {
	// GR_640
	{
		33,			// x
		49,			// y
		100,			// width
		339			// height
	},
	// GR_1024
	{
		53,			// x
		78,			// y
		160,			// width
		542			// height
	}
};

static int Mi_class_field[GR_NUM_RESOLUTIONS][4] = {
	// GR_640
	{
		140,			// x
		49,			// y
		59,			// width
		339			// height
	},
	// GR_1024
	{
		224,			// x
		78,			// y
		94,			// width
		542			// height
	}
};

static int Mi_status_field[GR_NUM_RESOLUTIONS][4] = {
	// GR_640
	{
		209,			// x
		49,			// y
		69,			// width
		339			// height
	},
	// GR_1024
	{
		334,			// x
		78,			// y
		110,			// width
		542			// height
	}
};

static int Mi_primary_field[GR_NUM_RESOLUTIONS][4] = {
	// GR_640
	{
		287,			// x
		49,			// y
		145,			// width
		339			// height
	},
	// GR_1024
	{
		459,			// x
		78,			// y
		232,			// width
		542			// height
	}
};

static int Mi_secondary_field[GR_NUM_RESOLUTIONS][4] = {
	// GR_640
	{
		441,			// x
		49,			// y
		145,			// width
		339			// height
	},
	// GR_1024
	{
		706,			// x
		78,			// y
		232,			// width
		542			// height
	}
};

// for timing a player out
static int Multi_ingame_timer_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		// GR_640
		26,
		411
	},
	{
		// GR_1024
		42,
		658
	}
};

//#define MULTI_INGAME_TIME_LEFT_X			26
//#define MULTI_INGAME_TIME_LEFT_Y			411

#define MULTI_INGAME_TIME_SECONDS		(1000 * 15)
LOCAL int Ingame_time_left;

// uses MULTI_JOIN_REFRESH_TIME as its timestamp
UI_WINDOW Multi_ingame_window;											// the window object for the join screen
UI_BUTTON Multi_ingame_select_button;									// for selecting list items
int Multi_ingame_bitmap;													// the background bitmap

// ship class icons
#define MULTI_INGAME_MAX_SHIP_ICONS			40
typedef struct is_icon {
	int bmaps[NUM_ICON_FRAMES];
	int ship_class;
} is_icon;
is_icon Multi_ingame_ship_icon[MULTI_INGAME_MAX_SHIP_ICONS];
int Multi_ingame_num_ship_icons;

// # of available ships (also == the # currently being displayed)
int Multi_ingame_num_avail;

// signatures for each of the available ships
ushort Multi_ingame_ship_sigs[MAX_PLAYERS];

// net signature of the ship we've requested to grab as an ingame joiner (waiting for server response if >= 0)
ushort Multi_ingame_join_sig;

// the index into the list of the ship currently selected
int Multi_ingame_ship_selected;

// temporary stuff - used only until we come up with a more permanent interface for this screen
#define MAX_INGAME_SHIPS 50
#define INGAME_FINAL_TIMEOUT 4000

ushort Ingame_ship_signatures[MAX_INGAME_SHIPS];
//LOCAL int Ingame_final_timeout;

//XSTR:ON

// local variables to hold ship/obj/ai information for the joiner.  We need to
// create a bogus ship so that packets that the joiner receives during his join
// have valid Player_ship, Player_obj, and Player_ai to work with
int Ingame_shipnum;

// display the available ships (OF_COULD_BE_PLAYER flagged)
void multi_ingame_join_display_avail();

// try and scroll the selected ship up
void multi_ingame_scroll_select_up();

// try and scroll the selected ship down
void multi_ingame_scroll_select_down();

// handle all timeout details
void multi_ingame_handle_timeout();

int multi_ingame_get_ship_class_icon(int ship_class)
{
	int idx;

	// lookup through all available ship icons
	for(idx=0;idx<Multi_ingame_num_ship_icons;idx++){
		if(Multi_ingame_ship_icon[idx].ship_class == ship_class){
			return idx;
		}
	}

	// couldn't find it 
	return -1;
}

void multi_ingame_load_icons()
{
	int first_frame, num_frames, idx, s_idx;

	// zero out the icon handles	
	for(idx=0;idx<MULTI_INGAME_MAX_SHIP_ICONS;idx++){
		Multi_ingame_ship_icon[idx].ship_class = -1;		
		for(s_idx=0;s_idx<NUM_ICON_FRAMES;s_idx++){
			Multi_ingame_ship_icon[idx].bmaps[s_idx] = -1;		
		}
	}		
	Multi_ingame_num_ship_icons = 0;

	// traverse through all ship types
	for(idx=0;idx<Num_ship_classes;idx++){
		// if there is a valid icon for this ship
		if((Ship_info[idx].icon_filename[0] != '\0') && (Multi_ingame_num_ship_icons < MULTI_INGAME_MAX_SHIP_ICONS)){
			// set the ship class
			Multi_ingame_ship_icon[Multi_ingame_num_ship_icons].ship_class = idx;

			// load in the animation frames for the icon	
			first_frame = bm_load_animation(Ship_info[idx].icon_filename, &num_frames);
			if ( first_frame == -1 ) {
				Int3();	// Could not load in icon frames.. get Dave
			}	
			for ( s_idx = 0; s_idx < num_frames; s_idx++ ) {
				Multi_ingame_ship_icon[Multi_ingame_num_ship_icons].bmaps[s_idx] = first_frame+s_idx;
			}

			Multi_ingame_num_ship_icons++;
		}
	}
}

void multi_ingame_unload_icons()
{
	int idx,s_idx;

	// unload all the bitmaps
	for(idx=0;idx<Multi_ingame_num_ship_icons;idx++){
		for(s_idx=0;s_idx<NUM_ICON_FRAMES;s_idx++){
			if(Multi_ingame_ship_icon[idx].bmaps[s_idx] != -1){
				bm_release(Multi_ingame_ship_icon[idx].bmaps[s_idx]);
				Multi_ingame_ship_icon[idx].bmaps[s_idx] = -1;
			}
		}
	}
}

// ingame join ship selection screen init
void multi_ingame_select_init()
{
	/// int objnum, wingnum_save,idx, goals_save;
	// ushort net_signature;
	int idx;

	Multi_ingame_join_sig = 0;
	Net_player->m_player->objnum = -1;

	// create a ship, then find a ship to copy crucial information from.  Save and restore the wing
	// number to be safe.
	/*
	wingnum_save = Player_start_pobject.wingnum;
	net_signature = Player_start_pobject.net_signature;
	goals_save = Player_start_pobject.ai_goals;
	Player_start_pobject.wingnum = -1;
	Player_start_pobject.net_signature = 0;
	Player_start_pobject.ai_goals = -1;
	objnum = parse_create_object( &Player_start_pobject );
	Player_start_pobject.wingnum = wingnum_save;
	Player_start_pobject.net_signature = net_signature;
	Player_start_pobject.ai_goals = goals_save;

	if ( objnum == -1 ) {
		nprintf(("Network", "Bailing ingame join because unable to create parse object player ship\n"));
		multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_INGAME_SHIP);
		return;
	}

	// make it invalid
	Player_obj = &Objects[objnum];
	Player_obj->net_signature = 0;						
	Player_ship = &Ships[Player_obj->instance];
	strcpy_s(Player_ship->ship_name, NOX("JIP Ship"));
	Player_ai = &Ai_info[Player_ship->ai_index];
	*/

	// load the temp ship icons
	multi_ingame_load_icons();

	// blast all the ingame ship signatures
	memset(Multi_ingame_ship_sigs,0,sizeof(ushort) * MAX_PLAYERS);

	// the index into the list of the ship currently selected
	Multi_ingame_ship_selected = -1;

	// initialize the time he has left to select a ship
	Ingame_time_left = timestamp(MULTI_INGAME_TIME_SECONDS);

	// initialize GUI data	

	// create the interface window
	Multi_ingame_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_ingame_window.set_mask_bmap(Multi_ingame_join_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_ingame_bitmap = bm_load(Multi_ingame_join_bitmap_fname[gr_screen.res]);
	if(Multi_ingame_bitmap < 0)
		Error(LOCATION, "Couldn't load background bitmap for ingame join");	
	
	// create the interface buttons
	for(idx=0; idx<MULTI_INGAME_JOIN_NUM_BUTTONS; idx++) {
		// create the object
		Multi_ingame_join_buttons[gr_screen.res][idx].button.create(&Multi_ingame_window, "", Multi_ingame_join_buttons[gr_screen.res][idx].x, Multi_ingame_join_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_ingame_join_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_ingame_join_buttons[gr_screen.res][idx].button.set_bmaps(Multi_ingame_join_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Multi_ingame_join_buttons[gr_screen.res][idx].button.link_hotspot(Multi_ingame_join_buttons[gr_screen.res][idx].hotspot);
	}	
	
	// create all xstrs
	for(idx=0; idx<MULTI_INGAME_JOIN_NUM_TEXT; idx++) {
		Multi_ingame_window.add_XSTR(&Multi_ingame_join_text[gr_screen.res][idx]);
	}

	// create the list item select button
	Multi_ingame_select_button.create(&Multi_ingame_window, "", Mi_name_field[gr_screen.res][MI_FIELD_X], Mi_name_field[gr_screen.res][MI_FIELD_Y], Mi_width[gr_screen.res], Mi_height[gr_screen.res], 0, 1);
	Multi_ingame_select_button.hide();			

	// load freespace stuff
	// JAS: Code to do paging used to be here.
}

// process all ship list related details
void multi_ingame_ship_list_process()
{
	int select_index,y;

	// if we currently don't have any ships selected, but we've got items on the list, select the first one
	if((Multi_ingame_ship_selected == -1) && (Multi_ingame_num_avail > 0)){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_ingame_ship_selected = 0;
	}

	// if we currently have a ship selected, but it disappears, select the next ship (is possible0
	if((Multi_ingame_ship_selected >= 0) && (Multi_ingame_ship_selected >= Multi_ingame_num_avail)){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_ingame_ship_selected = Multi_ingame_num_avail-1;
	}
	
	// if the player clicked on the select button, see if the selection has changed
	if(Multi_ingame_select_button.pressed()){
		Multi_ingame_select_button.get_mouse_pos(NULL,&y);
		select_index = y / Mi_spacing[gr_screen.res];

		// if we've selected a valid item
		if((select_index >= 0) && (select_index < Multi_ingame_num_avail)){
			// if we're not selected the same item, play a sound
			if(Multi_ingame_ship_selected != select_index){
				gamesnd_play_iface(SND_USER_SELECT);
			}

			// select the item
			Multi_ingame_ship_selected = select_index;
		}
	}
}


// determines if a button was pressed, and acts accordingly
void multi_ingame_join_check_buttons()
{
	int idx;
	for(idx=0; idx<MULTI_INGAME_JOIN_NUM_BUTTONS; idx++) {
		// we only really need to check for one button pressed at a time,
		// so we can break after finding one.
		if(Multi_ingame_join_buttons[gr_screen.res][idx].button.pressed()) {
			multi_ingame_join_button_pressed(idx);
			break;
		}
	}
}

// a button was pressed, so make it do its thing
// this is the "acting accordingly" part
void multi_ingame_join_button_pressed(int n)
{
	switch(n) {
	case MIJ_CANCEL:
		multi_quit_game(PROMPT_CLIENT);
		break;
	case MIJ_JOIN:
		// don't do further processing if the game is paused
		if ( Netgame.game_state == NETGAME_STATE_PAUSED )
			return;

		if(Multi_ingame_join_sig == 0) {
			// if he has a valid ship selected
			if(Multi_ingame_ship_selected >= 0) {
				gamesnd_play_iface(SND_USER_SELECT);
			
				// select the sig of this ship and send a request for it
				Multi_ingame_join_sig = Multi_ingame_ship_sigs[Multi_ingame_ship_selected];
				
				// send a request to the
				send_ingame_ship_request_packet(INGAME_SR_REQUEST,Multi_ingame_join_sig);
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}

		break;
	default:
		// how did this happen?
		Int3();
	}
}


// ingame join ship selection screen do
void multi_ingame_select_do()
{	


	int k = Multi_ingame_window.process();

	// process any keypresses
	switch(k){
	case KEY_ESC :
		multi_quit_game(PROMPT_CLIENT);		
		break;

	case KEY_UP:
		multi_ingame_scroll_select_up();
		break;

	case KEY_DOWN:
		multi_ingame_scroll_select_down();
		break;
	}	

	// process button presses
	// multi_ingame_process_buttons();
	multi_ingame_join_check_buttons();
	
	// process any ship list related events
	multi_ingame_ship_list_process();	
	
	// draw the background, etc
	gr_reset_clip();	
	GR_MAYBE_CLEAR_RES(Multi_ingame_bitmap);
	if(Multi_ingame_bitmap != -1){
		gr_set_bitmap(Multi_ingame_bitmap);
		gr_bitmap(0,0);
	}
	Multi_ingame_window.draw();

	// handle all timeout details. blitting, etc
	multi_ingame_handle_timeout();

	// display the available ships
	multi_ingame_join_display_avail();		

	// flip the buffer
	gr_flip();	
}

// ingame join ship select close
void multi_ingame_select_close()
{	
	// unload any bitmaps
	if(!bm_unload(Multi_ingame_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_ingame_join_bitmap_fname[gr_screen.res]));
	}	

	// unload all the ship class icons
	multi_ingame_unload_icons();
	
	// destroy the UI_WINDOW
	Multi_ingame_window.destroy();	

	// stop main hall music
	main_hall_stop_music();	
}

// display an individual ships information, starting at the indicated y pixel value
void multi_ingame_join_display_ship(object *objp,int y_start)
{
	int icon_num,idx;
	ship_info *sip;
	int y_spacing;
	ship_weapon *wp;

	sip = &Ship_info[Ships[objp->instance].ship_info_index];
	
	// blit the ship name itself
	gr_set_color_fast(&Color_normal);
	gr_string(Mi_name_field[gr_screen.res][MI_FIELD_X],y_start+10, Ships[objp->instance].ship_name);
	
	// blit the ship class icon
	icon_num = multi_ingame_get_ship_class_icon(Ships[objp->instance].ship_info_index);
	if(icon_num != -1){
		gr_set_bitmap(Multi_ingame_ship_icon[icon_num].bmaps[0]);
		gr_bitmap(Mi_class_field[gr_screen.res][MI_FIELD_X] + 15, y_start);
	}
	
	gr_set_color_fast(&Color_bright);
	wp = &Ships[objp->instance].weapons;
	
	// blit the ship's primary weapons	
	y_spacing = (Mi_spacing[gr_screen.res] - (wp->num_primary_banks * 10)) / 2;
	for(idx=0;idx<wp->num_primary_banks;idx++){
		gr_string(Mi_primary_field[gr_screen.res][MI_FIELD_X], y_start + y_spacing + (idx * 10), Weapon_info[wp->primary_bank_weapons[idx]].name);
	}

	// blit the ship's secondary weapons	
	y_spacing = (Mi_spacing[gr_screen.res] - (wp->num_secondary_banks * 10)) / 2;
	for(idx=0;idx<wp->num_secondary_banks;idx++){
		gr_string(Mi_secondary_field[gr_screen.res][MI_FIELD_X], y_start + y_spacing + (idx * 10), Weapon_info[wp->secondary_bank_weapons[idx]].name);
	}	

	// blit the shield/hull integrity
	hud_shield_show_mini(objp, Mi_status_field[gr_screen.res][MI_FIELD_X] + 15, y_start + 3,5,7);
}

// display the available ships (OF_COULD_BE_PLAYER flagged)
void multi_ingame_join_display_avail()
{		
	ship_obj *moveup;	

	// recalculate this # every frame
	Multi_ingame_num_avail = 0;	

	// display a background highlight rectangle for any selected lines
	if(Multi_ingame_ship_selected != -1){		
		int y_start = (Mi_name_field[gr_screen.res][MI_FIELD_Y] + (Multi_ingame_ship_selected * Mi_spacing[gr_screen.res]));		

		// draw the border
		gr_set_color_fast(&Color_bright_blue);
		gr_line(Mi_name_field[gr_screen.res][MI_FIELD_X]-1,y_start-1, (Mi_name_field[gr_screen.res][MI_FIELD_X]-1) + (Mi_width[gr_screen.res]+2),y_start-1);
		gr_line(Mi_name_field[gr_screen.res][MI_FIELD_X]-1,y_start + Mi_spacing[gr_screen.res] - 2, (Mi_name_field[gr_screen.res][MI_FIELD_X]-1) + (Mi_width[gr_screen.res]+2),y_start + Mi_spacing[gr_screen.res] - 2);
		gr_line(Mi_name_field[gr_screen.res][MI_FIELD_X]-1,y_start, Mi_name_field[gr_screen.res][MI_FIELD_X]-1, y_start + Mi_spacing[gr_screen.res] - 2);
		gr_line((Mi_name_field[gr_screen.res][MI_FIELD_X]-1) + (Mi_width[gr_screen.res]+2), y_start,(Mi_name_field[gr_screen.res][MI_FIELD_X]-1) + (Mi_width[gr_screen.res]+2),y_start + Mi_spacing[gr_screen.res] - 2);
	}

	moveup = GET_FIRST(&Ship_obj_list);	
	while(moveup != END_OF_LIST(&Ship_obj_list)){
		if( !(Ships[Objects[moveup->objnum].instance].flags & (SF_DYING|SF_DEPARTING)) && (Objects[moveup->objnum].flags & OF_COULD_BE_PLAYER) ) {
			// display the ship
			multi_ingame_join_display_ship(&Objects[moveup->objnum],Mi_name_field[gr_screen.res][MI_FIELD_Y] + (Multi_ingame_num_avail * Mi_spacing[gr_screen.res]));

			// set the ship signature
			Multi_ingame_ship_sigs[Multi_ingame_num_avail] = Objects[moveup->objnum].net_signature;
			
			// inc the # available
			Multi_ingame_num_avail++;
		}
		moveup = GET_NEXT(moveup);
	}		
}

// try and scroll the selected ship up
void multi_ingame_scroll_select_up()
{
	if(Multi_ingame_ship_selected > 0){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_ingame_ship_selected--;
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}	
}

// try and scroll the selected ship down
void multi_ingame_scroll_select_down()
{
	if(Multi_ingame_ship_selected < (Multi_ingame_num_avail - 1)){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_ingame_ship_selected++;
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// handle all timeout details
void multi_ingame_handle_timeout()
{
	/*
	// uncomment this block to disable the timer
	gr_set_color_fast(&Color_bright_red);
	gr_string(Multi_ingame_timer_coords[gr_screen.res][0], Multi_ingame_timer_coords[gr_screen.res][1], "Timer disabled!!");
	return;
	*/

	// if we've timed out, leave the game
	if( timestamp_elapsed(Ingame_time_left) ) {
		multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_INGAME_TIMEOUT, MULTI_END_ERROR_NONE);
		return;
	}

	// otherwise, blit how much time we have left
	int time_left = timestamp_until(Ingame_time_left) / 1000;
	char tl_string[100];
	gr_set_color_fast(&Color_bright);
	memset(tl_string,0,100);
	sprintf(tl_string,XSTR("Time remaining : %d s\n",682),time_left);	
	gr_string(Multi_ingame_timer_coords[gr_screen.res][0], Multi_ingame_timer_coords[gr_screen.res][1], tl_string);
}


// --------------------------------------------------------------------------------------------------
// PACKET HANDLER functions
// these are also defined in multimsgs.h, but the implementations are in the module for the sake of convenience
//

#define INGAME_PACKET_SLOP		75				// slop value used for packets to ingame joiner

void process_ingame_ships_packet( ubyte *data, header *hinfo )
{
	int offset, sflags, sflags2, oflags, team, j;
	ubyte p_type;
	ushort net_signature;	
	short wing_data;	
	int team_val, slot_index, idx;
	char ship_name[255] = "";
	object *objp;
	int net_sig_modify;

	// go through the ship obj list and delete everything. YEAH
	if(!Ingame_ships_deleted){

		// no player object
		Player_obj = NULL;
		Player_ship = NULL;
		Player_ai = NULL;

		// delete all ships
		for(idx=0; idx<MAX_SHIPS; idx++){
			if((Ships[idx].objnum >= 0) && (Ships[idx].objnum < MAX_OBJECTS)){
				obj_delete(Ships[idx].objnum);
			}
		}

		Ingame_ships_deleted = 1;
	}

	offset = HEADER_LENGTH;

	// go
	GET_DATA( p_type );	
	while ( p_type == INGAME_SHIP_NEXT ) {
		p_object *p_objp;
		int ship_num, objnum;

		GET_STRING( ship_name );
		GET_USHORT( net_signature );
		GET_INT( sflags );
		GET_INT( sflags2 );
		GET_INT( oflags );
		GET_INT( team );		
		GET_SHORT( wing_data );
		net_sig_modify = 0;
		if(wing_data >= 0){
			GET_INT(Wings[wing_data].current_wave);			
			net_sig_modify = Wings[wing_data].current_wave - 1;
		}

		// lookup ship in the original ships array
		p_objp = mission_parse_get_parse_object(net_signature);
		if(p_objp == NULL){
			// if this ship is part of wing not on its current wave, look for its "original" by subtracting out wave #
			p_objp = mission_parse_get_arrival_ship((ushort)(net_signature - (ushort)net_sig_modify));
		}
		if(p_objp == NULL){
			Int3();
			nprintf(("Network", "Couldn't find ship %s in either arrival list or in mission"));
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_INGAME_BOGUS);
			return;
		}

		// go ahead and create the parse object.  Set the network signature of this guy before
		// creation

		// multi_set_network_signature uncommented by Kazan on 11-04-2004
		 multi_set_network_signature( net_signature, MULTI_SIG_SHIP );
		objnum = parse_create_object( p_objp );
		ship_num = Objects[objnum].instance;
		Objects[objnum].flags = oflags;
		Objects[objnum].net_signature = net_signature;

		// assign any common data
		strcpy_s(Ships[ship_num].ship_name, ship_name);
		Ships[ship_num].flags = sflags;
		Ships[ship_num].flags2 = sflags2;
		Ships[ship_num].team = team;
		Ships[ship_num].wingnum = (int)wing_data;				

		GET_DATA( p_type );
	}

	PACKET_SET_SIZE();

	// if we have reached the end of the list and change our network state
	if ( p_type == INGAME_SHIP_LIST_EOL ) {		
		// merge all created list
		obj_merge_created_list();

		// fixup player ship stuff
		for(idx=0; idx<MAX_SHIPS; idx++){
			if(Ships[idx].objnum < 0){	
				continue;
			}

			// get the team and slot.  Team will be -1 when it isn't a part of player wing.  So, if
			// not -1, then be sure we have a valid slot, then change the ship type, etc.
			objp = &Objects[Ships[idx].objnum];		
			multi_ts_get_team_and_slot(Ships[idx].ship_name, &team_val, &slot_index);
			if ( team_val != -1 ) {
				Assert( slot_index != -1 );

				// change the ship type and the weapons
				change_ship_type(objp->instance, Wss_slots_teams[team_val][slot_index].ship_class);
				wl_bash_ship_weapons(&Ships[idx].weapons, &Wss_slots_teams[team_val][slot_index]);
	
				// Be sure to mark this ship as as a could_be_player
				obj_set_flags( objp, objp->flags | OF_COULD_BE_PLAYER );
				objp->flags &= ~OF_PLAYER_SHIP;
			}

			// if this is a player ship, make sure we find out who's it is and set their objnum accordingly
			if(team_val != -1){
				for( j = 0; j < MAX_PLAYERS; j++){
					if(MULTI_CONNECTED(Net_players[j]) && (Net_players[j].m_player->objnum == Objects[Ships[idx].objnum].net_signature)) {						
						// nprintf(("Network", "Making %s ship for %s\n", Ships[shipnum].ship_name, Net_players[j].player->callsign));
						multi_assign_player_ship( j, objp, Ships[idx].ship_info_index );
						objp->flags |= OF_PLAYER_SHIP;
						objp->flags &= ~OF_COULD_BE_PLAYER;
						break;
					}
				}
			}
		}

		// notify the server that we're all good.
		Net_player->state = NETPLAYER_STATE_INGAME_SHIPS;
		send_netplayer_update_packet();

		// add some mission sync text
		multi_common_add_text(XSTR("Ships packet ack (ingame)\n",683));
	}
}

void send_ingame_ships_packet(net_player *player)
{
	ubyte data[MAX_PACKET_SIZE];
	ubyte p_type;
	ship_obj *so;
	int packet_size;
	short wing_data;

	BUILD_HEADER( SHIPS_INGAME_PACKET );

	// essentially, we are going to send a list of ship names to the joiner for ships that are not
	// in wings.  The joiner will take the list, create any ships which should be created, and delete all
	// other ships after the list is sent.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		ship *shipp;

		shipp = &Ships[Objects[so->objnum].instance];

		// skip all wings.
		// if ( shipp->wingnum != -1 ){
			// continue;
		// }

		if ( Objects[so->objnum].net_signature == STANDALONE_SHIP_SIG ){
			continue;
		}

		//  add the ship name and other information such as net signature, ship and object(?) flags.
		p_type = INGAME_SHIP_NEXT;
		ADD_DATA( p_type );
		ADD_STRING( shipp->ship_name );
		ADD_USHORT( Objects[so->objnum].net_signature );
		ADD_INT( shipp->flags );
		ADD_INT( shipp->flags2 );
		ADD_INT( Objects[so->objnum].flags );
		ADD_INT( shipp->team );
		wing_data = (short)shipp->wingnum;
		ADD_SHORT(wing_data);
		if(wing_data >= 0){
			ADD_INT(Wings[wing_data].current_wave);
		}

		// don't send anymore data if we are getting close to the maximum size of this packet.  Send it off and
		// keep going
		if ( packet_size > (MAX_PACKET_SIZE - INGAME_PACKET_SLOP) ) {
			p_type = INGAME_SHIP_LIST_EOP;
			ADD_DATA( p_type );			
			multi_io_send_reliable(player, data, packet_size);
			BUILD_HEADER( SHIPS_INGAME_PACKET );
		}
	}

	// end of the ship list!!!
	p_type = INGAME_SHIP_LIST_EOL;
	ADD_DATA( p_type );	
	multi_io_send_reliable(player, data, packet_size);
}

void process_ingame_wings_packet( ubyte *data, header *hinfo )
{
	Int3();
}
/*
// code to process the wing data from a server.
void process_ingame_wings_packet( ubyte *data, header *hinfo )
{
	int offset, wingnum;
	ubyte p_type, what;

	offset = HEADER_LENGTH;

	GET_DATA( p_type );

	// p_type tells us whether to stop or not
	while ( p_type == INGAME_WING_NEXT ) {
		wing *wingp;

		// get the wingnum and a pointer to it.  The game stores data for all wings always, so this
		// is perfectly valid
		GET_DATA( wingnum );
		wingp = &Wings[wingnum];

		GET_DATA( what );
		if ( what == INGAME_WING_NOT_ARRIVED ) {
			Assert( wingp->total_arrived_count == 0 );			// this had better be true!!!
		} else if ( what == INGAME_WING_DEPARTED ) {
			// mark the wing as gone.  if it isn't, it soon will be.  Maybe we should send more information
			// about these wings later (like total_arrived_count, etc), but we will see.
			wingp->flags |= WF_WING_GONE;
		} else {
			int total_arrived_count, current_count, current_wave, i, j;
			ushort signature;
			int shipnum;

			// the wing is present in the mission on the server.  Get the crucial information about the
			// wing.  Then get the ships for this wing in order on the client machine
			GET_DATA( total_arrived_count );
			GET_DATA( current_count );
			GET_DATA( current_wave );

			Assert( current_wave > 0 );
			Assert( total_arrived_count > 0 );

			// for this wing, strip it down to nothing.  Let the parse object ocde recreate the
			// wing from the parse objects, then bash any weapons, etc for player wings.  We need
			// to do this because we might actually wind up with > MAX_SHIPS_PER_WING if we
			// don't delete them all first, and have a > 0 threshold, and are on something other
			// than the first wave.  Only do this for non-player wings.

			nprintf(("Network", "Clearing %s -- %d ships\n", wingp->name, wingp->current_count));
			for ( i = 0; i < wingp->current_count; i++ ) {
				int index, objnum;

				index = wingp->ship_index[i];
				Assert( index != -1 );
				objnum = Ships[index].objnum;
				Assert( objnum != -1 );

				// delete the object since we are filling the wing again anyway.
				obj_delete( objnum );
				Objects[objnum].net_signature = 0;				// makes this object "invalid" until dead.
				if ( Objects[objnum].type == OBJ_GHOST ) {
					nprintf(("Network", "Marking ghost objnum %d as dead\n", objnum));
					Objects[objnum].flags |= OF_SHOULD_BE_DEAD;
				}
				Ingame_ships_to_delete[index] = 0;		// be sure that this guy doesn't get deleted, since we already deleted it
				wingp->ship_index[i] = -1;
			}
			wingp->current_count = 0;
			wingp->total_arrived_count = 0;

			// now, recreate all the ships needed
			for (i = 0; i < current_count; i++ ) {
				int which_one, team, slot_index, specific_instance;
				ship *shipp;
				object *objp;

				GET_DATA( signature );

				// assign which_one to be the given signature - wing's base signature.  This let's us
				// know which ship to create (i.e. the total_arrivel_count);
				which_one = signature - wingp->net_signature;
				Assert( (which_one >= 0) && (which_one < (wingp->net_signature + (wingp->wave_count*wingp->num_waves))) );
				wingp->total_arrived_count = (ushort)which_one;

				// determine which ship in the ahip arrival list this guy is.  It is a 0 based index
				specific_instance = which_one % wingp->wave_count;

				// call parse_wing_create_ships making sure that we only ever create 1 ship at a time.  We don't
				// want parse_wing_create_ships() to assign network signature either.  We will directly
				// assign it here.

				wingp->current_wave = 0;						// make it the first wave.  Ensures that ships don't get removed off the list
				parse_wing_create_ships( wingp, 1, 1, specific_instance );
				shipnum = wingp->ship_index[wingp->current_count-1];
				Ingame_ships_to_delete[shipnum] = 0;			// "unmark" this ship so it doesn't get deleted.

				// kind of stupid, but bash the name since it won't get recreated properly from
				// the parse_wing_create_ships call.
				shipp = &Ships[shipnum];
				sprintf(shipp->ship_name, NOX("%s %d"), wingp->name, which_one + 1);
				nprintf(("Network", "Created %s\n", shipp->ship_name));

				objp = &Objects[shipp->objnum];
				objp->net_signature = (ushort)(wingp->net_signature + which_one);

				// get the team and slot.  Team will be -1 when it isn't a part of player wing.  So, if
				// not -1, then be sure we have a valid slot, then change the ship type, etc.
				multi_ts_get_team_and_slot(shipp->ship_name, &team, &slot_index);
				if ( team != -1 ) {
					Assert( slot_index != -1 );

					// change the ship type and the weapons
					change_ship_type(objp->instance, Wss_slots_teams[team][slot_index].ship_class);
					wl_bash_ship_weapons(&shipp->weapons,&Wss_slots_teams[team][slot_index]);

					// Be sure to mark this ship as as a could_be_player
					obj_set_flags( objp, objp->flags | OF_COULD_BE_PLAYER );
					objp->flags &= ~OF_PLAYER_SHIP;
				}

				// if this is a player ship, make sure we find out who's it is and set their objnum accordingly
				for( j = 0; j < MAX_PLAYERS; j++){
					if(MULTI_CONNECTED(Net_players[j]) && (Net_players[j].player->objnum == signature)) {
						Assert( team != -1 );		// to help trap errors!!!
						nprintf(("Network", "Making %s ship for %s\n", Ships[shipnum].ship_name, Net_players[j].player->callsign));
						multi_assign_player_ship( j, objp, Ships[shipnum].ship_info_index );
						objp->flags |= OF_PLAYER_SHIP;
						objp->flags &= ~OF_COULD_BE_PLAYER;
						break;
					}
				}
			}


			// we will have no ships in any wings at this point (we didn't create any when we loaded the
			// mission).  Set the current wave of this wing to be 1 less than was passed in since this value
			// will get incremented in parse_wing_create_ships;
			wingp->current_wave = current_wave;
			wingp->total_arrived_count = total_arrived_count;
		}

		GET_DATA( p_type );
	}

   PACKET_SET_SIZE();

	// if we have reached the end of the list change our network state
	if ( p_type == INGAME_WING_LIST_EOL ) {
		Net_player->state = NETPLAYER_STATE_INGAME_WINGS;
		send_netplayer_update_packet();

		// add some mission sync text
		multi_common_add_text(XSTR("Wings packet (ingame)\n",684));
	}
	
}

// function to send information about wings.  We need to send enough information to let the client
// construct or reconstruct any wings in the mission.  We will rely on the fact that the host wing array
// will exactly match the client wing array (in terms of number, and wing names)
void send_ingame_wings_packet( net_player *player )
{
	ubyte data[MAX_PACKET_SIZE];
	ubyte p_type;
	int packet_size, i;
	ubyte what;

	BUILD_HEADER( WINGS_INGAME_PACKET );

	// iterate through the wings list
	for ( i = 0; i < Num_wings; i++ ) {
		wing *wingp;

		wingp = &Wings[i];

		p_type = INGAME_WING_NEXT;
		ADD_DATA( p_type );

		ADD_DATA( i );

		// add wing data that the client needs.  There are several conditions to send to clients:
		//
		// 1. wing hasn't arrived -- total_arrived_count will be 0
		// 2. wing is done (or currently departing)
		// 3. wing is present (any wave, any number of ships).
		//
		// 1 and 2 are easy to handle.  (3) is the hardest.
		if ( wingp->total_arrived_count == 0 ) {
			what = INGAME_WING_NOT_ARRIVED;
			ADD_DATA( what );
		} else if ( wingp->flags & (WF_WING_GONE | WF_WING_DEPARTING) ) {
			what = INGAME_WING_DEPARTED;
			ADD_DATA( what );
		} else {
			int j;

			// include to code to possibly send more wing data here in this part of the if/else
			// chain.  We can do this because MAX_WINGS * 8 (8 being the number of byte for a minimum
			// description of a wing) is always less than MAX_PACKET_SIZE.  Checking here ensures that
			// we have enough space for *this* wing in the packet, and not the largest wing.  The
			// formula below looks at number of ships in the wing, the name length, length of the signature,
			// and the size of the bytes added before the ship names.  32 accounts for a little slop
			if ( packet_size > (MAX_PACKET_SIZE - (wingp->current_count * (NAME_LENGTH+2) + 32)) ) {
				p_type = INGAME_WING_LIST_EOP;
				ADD_DATA( p_type );				
				multi_io_send_reliable(player, data, packet_size);
				BUILD_HEADER( WINGS_INGAME_PACKET );
			}
			what = INGAME_WING_PRESENT;
			ADD_DATA( what );
			ADD_DATA( wingp->total_arrived_count );
			ADD_DATA( wingp->current_count );
			ADD_DATA( wingp->current_wave );

			// add the ship name and net signature of all ships currently in the wing.
			for ( j = 0; j < wingp->current_count; j++ ) {
				ship *shipp;

				shipp = &Ships[wingp->ship_index[j]];
				//ADD_STRING( shipp->ship_name );
				ADD_DATA( Objects[shipp->objnum].net_signature );
			}
		}

	}

	p_type = INGAME_WING_LIST_EOL;
	ADD_DATA( p_type );
	
	multi_io_send_reliable(player, data, packet_size);
}
*/

// send a request or a reply regarding ingame join ship choice
void send_ingame_ship_request_packet(int code,int rdata,net_player *pl)
{
	ubyte data[MAX_PACKET_SIZE],val;
	ship *shipp;
	ship_info *sip;
	int i, packet_size = 0;
	ushort signature;
	p_object *pobj;

	// add the data
	BUILD_HEADER(INGAME_SHIP_REQUEST);

	// add the code
	ADD_INT(code);
	
	// add any code specific data
	switch(code){
	case INGAME_SR_REQUEST:
		// add the net signature of the ship we're requesting
		signature = (ushort)rdata;
		ADD_USHORT( signature );
		break;
	case INGAME_SR_CONFIRM:
		// get a pointer to the ship
		shipp = &Ships[Objects[rdata].instance];
		sip = &Ship_info[shipp->ship_info_index];

		// add the most recent position and orientation for the requested ship
		ADD_VECTOR(Objects[rdata].pos);
		ADD_ORIENT(Objects[rdata].orient);
		ADD_INT( Missiontime ); // NOTE: this is a long so careful with swapping in 64-bit platforms - taylor

		// add the # of respawns this ship has left
		pobj = mission_parse_get_arrival_ship( Objects[rdata].net_signature );
		Assert(pobj != NULL);
		ADD_DATA(pobj->respawn_count);

		// add the ships ets settings
		val = (ubyte)shipp->weapon_recharge_index;
		ADD_DATA(val);
		val = (ubyte)shipp->shield_recharge_index;
		ADD_DATA(val);
		val = (ubyte)shipp->engine_recharge_index;
		ADD_DATA(val);

		// add the ballistic primary flag - Goober5000
		val = 0;
		if(sip->flags & SIF_BALLISTIC_PRIMARIES){
			val |= (1<<0);
		}
		ADD_DATA(val);

		// add current primary and secondary banks, and add link status
		val = (ubyte)shipp->weapons.current_primary_bank;
		ADD_DATA(val);
		val = (ubyte)shipp->weapons.current_secondary_bank;
		ADD_DATA(val);		

		// add the current ammo count for secondary banks;
		val = (ubyte)shipp->weapons.num_secondary_banks;		// for sanity checking
		ADD_DATA(val);
		for ( i = 0; i < shipp->weapons.num_secondary_banks; i++ ) {
			Assert( shipp->weapons.secondary_bank_ammo[i] < UCHAR_MAX );
			val = (ubyte)shipp->weapons.secondary_bank_ammo[i];
			ADD_DATA(val);
		}

		// add the current ammo count for primary banks - copied from above - Goober5000
		val = (ubyte)shipp->weapons.num_primary_banks;		// for sanity checking
		ADD_DATA(val);
		for ( i = 0; i < shipp->weapons.num_primary_banks; i++ )
		{
			Assert( shipp->weapons.primary_bank_ammo[i] < UCHAR_MAX );
			val = (ubyte)shipp->weapons.primary_bank_ammo[i];
			ADD_DATA(val);
		}

		// add the link status of weapons
		// primary link status	
		val = 0;
		if(shipp->flags & SF_PRIMARY_LINKED){
			val |= (1<<0);
		}
		if(shipp->flags & SF_SECONDARY_DUAL_FIRE){
			val |= (1<<1);
		}
		ADD_DATA(val);		
		break;
	}

	// send the packet
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		Assert(pl != NULL);		
		multi_io_send_reliable(pl, data, packet_size);
	} else {		
		multi_io_send_reliable(Net_player, data, packet_size);
	}

	// if this is a confirm to a player -- send data to the other players in the game telling them
	if ( (code == INGAME_SR_CONFIRM) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) ) {
		int player_num;

		player_num = NET_PLAYER_NUM(pl);
		code = INGAME_PLAYER_CHOICE;
		BUILD_HEADER(INGAME_SHIP_REQUEST);
		ADD_INT(code);
		ADD_INT(player_num);
		ADD_USHORT(Objects[rdata].net_signature);
		for (i = 0; i < MAX_PLAYERS; i++ ) {
			if(MULTI_CONNECTED(Net_players[i]) && (&Net_players[i] != Net_player) && (i != player_num) ) {				
				multi_io_send_reliable(&Net_players[i], data, packet_size);
			}
		}
	}
}

// function to validate all players in the game according to their team select index.  If discrepancies
// are found, this function should be able to fix them up.
void multi_ingame_validate_players()
{
	int i;

	for ( i = 0; i < MAX_PLAYERS; i++ ) {
		if( MULTI_CONNECTED(Net_players[i]) && (Net_player != &Net_players[i]) && !MULTI_STANDALONE(Net_players[i]) ) {
			char ship_name[NAME_LENGTH];
			int shipnum, objnum, player_objnum;

			player_objnum = Net_players[i].m_player->objnum;
			if ( (Objects[player_objnum].type != OBJ_SHIP) && (Objects[player_objnum].type != OBJ_GHOST) ) {
				Int3();
			}

			multi_ts_get_shipname( ship_name, Net_players[i].p_info.team, Net_players[i].p_info.ship_index );
			Assert( ship_name != NULL );
			shipnum = ship_name_lookup( ship_name );
			if ( shipnum == -1 ) {
				// ship could be respawning
				continue;
			}
			objnum = Ships[shipnum].objnum;
			Assert( objnum != -1 );

			// if this guy's objnum isn't a ship, then it should proably be a ghost!!
			if ( Objects[objnum].type == OBJ_SHIP ) {
				if ( objnum != Net_players[i].m_player->objnum ) {
					Int3();
					Net_players[i].m_player->objnum = objnum;
				}
			} else {
				Assert( Objects[objnum].type == OBJ_GHOST );
			}
		}
	}
}

// process an ingame ship request packet
void process_ingame_ship_request_packet(ubyte *data, header *hinfo)
{
	int code;
	object *objp;
	int offset = HEADER_LENGTH;
	int team, slot_index, i;
	uint respawn_count;
	ubyte val, num_secondary_banks, num_primary_banks;
	p_object *pobj;
	int player_num;

	// get the code
	GET_INT(code);

	switch(code){
	// a request for a ship from an ingame joiner
	case INGAME_SR_REQUEST:				
		ushort sig_request;

		// lookup the player and make sure he doesn't already have an objnum (along with possible error conditions)
		GET_USHORT(sig_request);
		PACKET_SET_SIZE();
			
		player_num = find_player_id(hinfo->id);	
		if(player_num == -1){
			nprintf(("Network","Received ingame ship request packet from unknown player!!\n"));		
			break;
		}
		
		// make sure this player doesn't already have an object
		Assert(MULTI_CONNECTED(Net_players[player_num]));
		if(Net_players[player_num].m_player->objnum != -1){
			send_ingame_ship_request_packet(INGAME_SR_DENY,0,&Net_players[player_num]);
			break;
		}
		
		// try and find the object
		objp = NULL;
		objp = multi_get_network_object(sig_request);
		if(objp == NULL || !(objp->flags & OF_COULD_BE_PLAYER)){
			send_ingame_ship_request_packet(INGAME_SR_DENY,0,&Net_players[player_num]);
			break;
		}		

		// Assign the player this objnum and ack him
		Net_players[player_num].m_player->objnum = OBJ_INDEX(objp);
		Net_players[player_num].state = NETPLAYER_STATE_IN_MISSION;                   // since he'll do this anyway...
		Net_players[player_num].flags &= ~(NETINFO_FLAG_INGAME_JOIN);
		multi_assign_player_ship( player_num, objp, Ships[objp->instance].ship_info_index );

		// update his ets and link status stuff
		multi_server_update_player_weapons(&Net_players[player_num],&Ships[objp->instance]);

		objp->flags &= ~(OF_COULD_BE_PLAYER);
		objp->flags |= OF_PLAYER_SHIP;

		// send a player settings packet to update all other players of this guy's final choices
		send_player_settings_packet();

		// initialize datarate limiting for this guy
		multi_oo_rate_init(&Net_players[player_num]);
		
		// ack him
		send_ingame_ship_request_packet(INGAME_SR_CONFIRM,OBJ_INDEX(objp),&Net_players[player_num]);

		// clear my ingame join flag so that others may join
		Netgame.flags &= ~NG_FLAG_INGAME_JOINING;

		// clear his net stats
		scoring_level_init( &(Net_players[player_num].m_player->stats) );
		break;

		// a denial for the ship we requested from the server
	case INGAME_SR_DENY :
		PACKET_SET_SIZE();

		// set this to -1 so we can pick again
		Multi_ingame_join_sig = 0;

		// display a popup
		popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("You have been denied the requested ship",686));
		break;

	// a confirmation that we can use the selected ship
	case INGAME_SR_CONFIRM:
		// object *temp_objp;

		// delete the ship this ingame joiner was using.  Unassign Player_obj so that this object
		// doesn't become a ghost.
		// temp_objp = Player_obj;
		// Player_obj = NULL;
		// obj_delete( OBJ_INDEX(temp_objp) );

		// get the object itself
		objp = multi_get_network_object(Multi_ingame_join_sig);
		Assert(objp != NULL);

		// get its most recent position and orientation
		GET_VECTOR(objp->pos);
		GET_ORIENT(objp->orient);
		GET_INT( Missiontime ); // NOTE: this is a long so careful with swapping in 64-bit platforms - taylor
		GET_UINT( respawn_count );
				
		// tell the server I'm in the mission
		Net_player->state = NETPLAYER_STATE_IN_MISSION;
		send_netplayer_update_packet();

		// setup our object				
		Net_player->m_player->objnum = OBJ_INDEX(objp);			
		Player_obj = objp;
		Player_obj->flags &= ~(OF_COULD_BE_PLAYER);
		Player_obj->flags |= OF_PLAYER_SHIP;
		multi_assign_player_ship( MY_NET_PLAYER_NUM, objp, Ships[objp->instance].ship_info_index );

		// must change the ship type and weapons.  An ingame joiner know about the default class
		// and weapons for a ship, but these could have changed.
		multi_ts_get_team_and_slot(Player_ship->ship_name, &team, &slot_index);
		Assert( team != -1 );
		Assert( slot_index != -1 );
		change_ship_type(objp->instance, Wss_slots_teams[team][slot_index].ship_class);
		wl_bash_ship_weapons(&Player_ship->weapons,&Wss_slots_teams[team][slot_index]);

		// get the parse object for it and assign the respawn count
		pobj = mission_parse_get_arrival_ship( objp->net_signature );
		Assert(pobj != NULL);
		pobj->respawn_count = respawn_count;

		// get the ships ets settings
		GET_DATA(val);
		Player_ship->weapon_recharge_index = val;
		GET_DATA(val);
		Player_ship->shield_recharge_index = val;
		GET_DATA(val);
		Player_ship->engine_recharge_index = val;		

		// handle the ballistic primary flag - Goober5000
		GET_DATA(val);
		if(val & (1<<0)){
			Player_ship->flags |= SIF_BALLISTIC_PRIMARIES;
		}

		// get current primary and secondary banks, and add link status
		GET_DATA(val);
		Player_ship->weapons.current_primary_bank = val;
		GET_DATA(val);
		Player_ship->weapons.current_secondary_bank = val;				

		// secondary bank ammo data
		GET_DATA( num_secondary_banks );
		Assert( num_secondary_banks == Player_ship->weapons.num_secondary_banks );
		for ( i = 0; i < Player_ship->weapons.num_secondary_banks; i++ ) {
			GET_DATA(val);
			Player_ship->weapons.secondary_bank_ammo[i] = val;
		}

		// primary bank ammo data - copied from above - Goober5000
		GET_DATA( num_primary_banks );
		Assert( num_primary_banks == Player_ship->weapons.num_primary_banks );
		for ( i = 0; i < Player_ship->weapons.num_primary_banks; i++ )
		{
			GET_DATA(val);
			Player_ship->weapons.primary_bank_ammo[i] = val;
		}

		// get the link status of weapons
		GET_DATA(val);
		if(val & (1<<0)){
			Player_ship->flags |= SF_PRIMARY_LINKED;
		}
		if(val & (1<<1)){
			Player_ship->flags |= SF_SECONDARY_DUAL_FIRE;
		}		
		PACKET_SET_SIZE();					

		// be sure that this ships current primary/secondary weapons are valid.  Easiest is to just
		// bash the values to 0!
		/*
		if ( Player_ship->weapons.current_primary_bank == -1 )
			Player_ship->weapons.current_primary_bank = 0;
		if ( Player_ship->weapons.current_secondary_bank == -1 )
			Player_ship->weapons.current_secondary_bank = 0;
		*/
		
		Net_player->flags &= ~(NETINFO_FLAG_INGAME_JOIN);

		// clear all object collision pairs, then add myself to the list
		extern void obj_reset_all_collisions();
		obj_reset_all_collisions();
		// obj_reset_pairs();
		// obj_add_pairs( OBJ_INDEX(Player_obj) );

		mission_hotkey_set_defaults();

		//multi_ingame_validate_players();

		// jump into the mission 
		// NOTE : we check this flag because its possible that the player could have received an endgame packet in the same
		//        frame as getting this confirmation. In that case, he should be quitting to the main menu. We must not make
		//        him continue on into the mission
		if(!multi_endgame_ending()){
			gameseq_post_event(GS_EVENT_ENTER_GAME);								
		} 
		break;

	case INGAME_PLAYER_CHOICE: {
		ushort net_signature;

		// get the player number of this guy, and the net signature of the ship he has chosen
		GET_INT(player_num);
		GET_USHORT(net_signature);
		PACKET_SET_SIZE();

		objp = multi_get_network_object(net_signature);
		if ( objp == NULL ) {
			// bogus!!!  couldn't find the object -- we cannot connect his -- this is really bad!!!
			nprintf(("Network", "Couldn't find ship for ingame joiner %s\n", Net_players[player_num].m_player->callsign));
			break;
		}
		objp->flags |= OF_PLAYER_SHIP;
		objp->flags &= ~OF_COULD_BE_PLAYER;

		multi_assign_player_ship( player_num, objp, Ships[objp->instance].ship_info_index );

		break;
		}
	}
}


// --------------------------------------------------------------------------------------------------
// INGAME JOIN FORWARD DEFINITIONS
//

void multi_ingame_send_ship_update(net_player *p)
{
	ship_obj *moveup;
	
	// get the first object on the list
	moveup = GET_FIRST(&Ship_obj_list);
	
	// go through the list and send all ships which are mark as OF_COULD_BE_PLAYER
	while(moveup!=END_OF_LIST(&Ship_obj_list)){
		//Make sure the object can be a player and is on the same team as this guy
		if(Objects[moveup->objnum].flags & OF_COULD_BE_PLAYER && obj_team(&Objects[moveup->objnum]) == p->p_info.team){
			// send the update
			send_ingame_ship_update_packet(p,&Ships[Objects[moveup->objnum].instance]);
		}

		// move to the next item
		moveup = GET_NEXT(moveup);
	}
}

// for now, I guess we'll just send hull and shield % values
void send_ingame_ship_update_packet(net_player *p,ship *sp)
{
	ubyte data[MAX_PACKET_SIZE];
	object *objp;
	int idx;
	int packet_size = 0;
	float f_tmp;

	BUILD_HEADER(INGAME_SHIP_UPDATE);
	
	// just send net signature, shield and hull percentages
	objp = &Objects[sp->objnum];
	ADD_USHORT(objp->net_signature);
	ADD_UINT(objp->flags);
	ADD_FLOAT(objp->hull_strength);
	
	// shield percentages
	for(idx=0; idx<MAX_SHIELD_SECTIONS; idx++){
		f_tmp = objp->shield_quadrant[idx];
		ADD_FLOAT(f_tmp);
	}
	
	multi_io_send_reliable(p, data, packet_size);
}

void process_ingame_ship_update_packet(ubyte *data, header *hinfo)
{
	int offset;
	float garbage;
	int flags;
	int idx;
	ushort net_sig;
	object *lookup;
	float f_tmp;
	
	offset = HEADER_LENGTH;
	// get the net sig for the ship and do a lookup
	GET_USHORT(net_sig);
	GET_INT(flags);
   
	// get the object
	lookup = multi_get_network_object(net_sig);
	if(lookup == NULL){
		// read in garbage values if we can't find the ship
		nprintf(("Network","Got ingame ship update for unknown object\n"));
		GET_FLOAT(garbage);
		for(idx=0;idx<MAX_SHIELD_SECTIONS;idx++){
			GET_FLOAT(garbage);
		}

		PACKET_SET_SIZE();
		return;
	}
	// otherwise read in the ship values
	lookup->flags = flags;
 	GET_FLOAT(lookup->hull_strength);
	for(idx=0;idx<MAX_SHIELD_SECTIONS;idx++){
		GET_FLOAT(f_tmp);
		lookup->shield_quadrant[idx] = f_tmp;
	}

	PACKET_SET_SIZE();
}
