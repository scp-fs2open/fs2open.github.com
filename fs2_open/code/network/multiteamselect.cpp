/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "network/multiteamselect.h"
#include "network/multi.h"
#include "missionui/chatbox.h"
#include "gamesnd/gamesnd.h"
#include "io/key.h"
#include "globalincs/linklist.h"
#include "gamesequence/gamesequence.h"
#include "graphics/font.h"
#include "network/multiutil.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "missionui/missionweaponchoice.h"
#include "missionui/missionbrief.h"
#include "network/multimsgs.h"
#include "menuui/snazzyui.h"
#include "io/mouse.h"
#include "popup/popup.h"
#include "network/multiui.h"
#include "network/multi_endgame.h"
#include "globalincs/alphacolors.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "mission/missionparse.h"


// ------------------------------------------------------------------------------------------------------
// TEAM SELECT DEFINES/VARS
//

// mission screen common data
extern int Next_screen;

//XSTR:OFF

// bitmap defines
#define MULTI_TS_PALETTE							"InterfacePalette"

char *Multi_ts_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"TeamSelect",		// GR_640
	"2_TeamSelect"		// GR_1024
};

char *Multi_ts_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"TeamSelect-M",	// GR_640
	"2_TeamSelect-M"		// GR_1024
};

// constants for coordinate lookup
#define MULTI_TS_X_COORD 0
#define MULTI_TS_Y_COORD 1
#define MULTI_TS_W_COORD 2
#define MULTI_TS_H_COORD 3

#define MULTI_TS_NUM_BUTTONS						7
#define MULTI_TS_BRIEFING							0					// go to the briefing
#define MULTI_TS_SHIP_SELECT						1					// this screen
#define MULTI_TS_WEAPON_SELECT					2					// go to the weapon select screen
#define MULTI_TS_SHIPS_UP							3					// scroll the ships list up
#define MULTI_TS_SHIPS_DOWN						4					// scroll the ships list down
#define MULTI_TS_COMMIT								5					// commit
#define MULTI_TS_LOCK								6					// lock (free) ship/weapon select

ui_button_info Multi_ts_buttons[GR_NUM_RESOLUTIONS][MULTI_TS_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("CB_00",	7,		3,		37,	7,		0),
		ui_button_info("CB_01",	7,		19,	37,	23,	1),
		ui_button_info("CB_02",	7,		35,	37,	39,	2),
		ui_button_info("TSB_03",	5,		303,	-1,	-1,	3),
		ui_button_info("TSB_04",	5,		454,	-1,	-1,	4),
		ui_button_info("TSB_09",	571,	425,	572,	413,	9),
		ui_button_info("TSB_34",	603,	374,	602,	364,	34)
	},
	{ // GR_1024

		ui_button_info("2_CB_00",	12,	5,		59,	12,	0),
		ui_button_info("2_CB_01",	12,	31,	59,	37,	1),
		ui_button_info("2_CB_02",	12,	56,	59,	62,	2),
		ui_button_info("2_TSB_03",	8,		485,	-1,	-1,	3),
		ui_button_info("2_TSB_04",	8,		727,	-1,	-1,	4),
		ui_button_info("2_TSB_09",	914,	681,	937,	660,	9),
		ui_button_info("2_TSB_34",	966,	599,	964,	584,	34)
	},
};


// players locked ani graphic
#define MULTI_TS_NUM_LOCKED_BITMAPS				3

char *Multi_ts_bmap_names[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		"TSB_340000",
		"TSB_340001",
		"TSB_340002"
	}, 
	{ // GR_1024
		"2_TSB_340000",
		"2_TSB_340001",
		"2_TSB_340002"
	}
};
int Multi_ts_locked_bitmaps[MULTI_TS_NUM_LOCKED_BITMAPS];


// snazzy menu regions
#define TSWING_0_SHIP_0								10
#define TSWING_0_SHIP_1								12
#define TSWING_0_SHIP_2								14
#define TSWING_0_SHIP_3								16
#define TSWING_1_SHIP_0								18
#define TSWING_1_SHIP_1								20
#define TSWING_1_SHIP_2								22
#define TSWING_1_SHIP_3								24
#define TSWING_2_SHIP_0								26
#define TSWING_2_SHIP_1								28
#define TSWING_2_SHIP_2								30
#define TSWING_2_SHIP_3								32

#define TSWING_0_NAME_0								11
#define TSWING_0_NAME_1								13
#define TSWING_0_NAME_2								15
#define TSWING_0_NAME_3								17
#define TSWING_1_NAME_0								19
#define TSWING_1_NAME_1								21
#define TSWING_1_NAME_2								23
#define TSWING_1_NAME_3								25
#define TSWING_2_NAME_0								27
#define TSWING_2_NAME_1								29
#define TSWING_2_NAME_2								31
#define TSWING_2_NAME_3								33

#define TSWING_LIST_0								5
#define TSWING_LIST_1								6
#define TSWING_LIST_2								7
#define TSWING_LIST_3								8

#define MULTI_TS_SLOT_LIST							0
#define MULTI_TS_PLAYER_LIST						1
#define MULTI_TS_AVAIL_LIST						2
	
// interface data
#define MULTI_TS_NUM_SNAZZY_REGIONS				28
int Multi_ts_bitmap;
int Multi_ts_mask;
int Multi_ts_inited = 0;
int Multi_ts_snazzy_regions;
ubyte *Multi_ts_mask_data;	
int Multi_ts_mask_w, Multi_ts_mask_h;
MENU_REGION	Multi_ts_region[MULTI_TS_NUM_SNAZZY_REGIONS];
UI_WINDOW Multi_ts_window;

// ship slot data
#define MULTI_TS_NUM_SHIP_SLOTS_TEAM	4														// # of ship slots in team v team
#define MULTI_TS_FLAG_NONE					-2														// never has any ships
#define MULTI_TS_FLAG_EMPTY				-1														// currently empty

static int Multi_ts_slot_icon_coords[MULTI_TS_NUM_SHIP_SLOTS][GR_NUM_RESOLUTIONS][2] = {							// x,y
	{		// 	
			{128,301},		// GR_640
			{205,482}		// GR_1024
	},
	{		// 
			{91,347},		// GR_640
			{146,555}		// GR_1024
	},
	{		// 
			{166,347},		// GR_640
			{266,555}		// GR_1024
	},
	{		// alpha
			{128,395},		// GR_640
			{205,632}		// GR_1024
	},
	{		//  
			{290,301},		// GR_640
			{464,482}		// GR_1024
	},
	{		// 
			{253,347},		// GR_640
			{405,555}		// GR_1024
	},
	{		// 
			{328,347},		// GR_640
			{525,555}		// GR_1024
	},
	{		// beta
			{290,395},		// GR_640
			{464,632}		// GR_1024
	},
	{		// 
			{453,301},		// GR_640
			{725,482}		// GR_1024
	},
	{		// 
			{416,347},		// GR_640
			{666,555}		// GR_1024
	},
	{		//
			{491,347},		// GR_640
			{786,555}		// GR_1024
	},
	{		// gamma
			{453,395},		// GR_640
			{725,632}		// GR_1024
	}
};

static int Multi_ts_slot_text_coords[MULTI_TS_NUM_SHIP_SLOTS][GR_NUM_RESOLUTIONS][3] = {	// x,y,width
	{		// alpha
		{112,330,181-112},	// GR_640
		{187,517,181-112}		// GR_1024
	},
	{		// alpha
		{74,377,143-74},	// GR_640
		{126,592,143-74}	// GR_1024
	},
	{		// alpha 
		{149,377,218-149},// GR_640
		{248,592,218-149}	// GR_1024
	},
	{		// alpha
		{112,424,181-112},// GR_640
		{187,667,181-112}	// GR_1024
	},
	{		// beta
		{274,330,343-274},// GR_640
		{446,517,343-274}	// GR_1024
	},
	{		// beta
		{236,377,305-236},// GR_640
		{385,592,305-236}	// GR_1024
	},
	{		// beta
		{311,377,380-311},// GR_640
		{507,592,380-311}	// GR_1024
	},
	{		// beta
		{274,424,343-274},// GR_640
		{446,667,343-274}	// GR_1024
	},
	{		// gamma
		{437,330,506-437},// GR_640
		{707,517,506-437}	// GR_1024
	},
	{		// gamma
		{399,377,468-399},// GR_640
		{646,592,468-399}	// GR_1024
	},
	{		// gamma
		{474,377,543-474},// GR_640
		{768,592,543-474}	// GR_1024
	},
	{		// gamma
		{437,424,506-437},// GR_640
		{707,667,506-437}	// GR_1024
	}
};

// avail ship list data
#define MULTI_TS_AVAIL_MAX_DISPLAY		4
static int Multi_ts_avail_coords[MULTI_TS_AVAIL_MAX_DISPLAY][GR_NUM_RESOLUTIONS][2] = {							// x,y coords
	{		// 
			{23,331},	// GR_640
			{37,530}		// GR_1024
	},
	{		// 
			{23,361},	// GR_640
			{37,578}		// GR_1024
	},
	{		// 
			{23,391},	// GR_640
			{37,626}		// GR_1024
	},
	{		//
			{23,421},	// GR_640
			{37,674}		// GR_1024
	}
};
int Multi_ts_avail_start = 0;																		// starting index of where we will display the available ships
int Multi_ts_avail_count = 0;																		// the # of available ship classes

// ship information stuff
#define MULTI_TS_SHIP_INFO_MAX_LINE_LEN				150
#define MULTI_TS_SHIP_INFO_MAX_LINES					10
#define MULTI_TS_SHIP_INFO_MAX_TEXT						(MULTI_TS_SHIP_INFO_MAX_LINE_LEN * MULTI_TS_SHIP_INFO_MAX_LINES)

static int Multi_ts_ship_info_coords[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		33, 150, 387
	},
	{ // GR_1024
		53, 240, 618
	}
};

char Multi_ts_ship_info_lines[MULTI_TS_SHIP_INFO_MAX_LINES][MULTI_TS_SHIP_INFO_MAX_LINE_LEN];
char Multi_ts_ship_info_text[MULTI_TS_SHIP_INFO_MAX_TEXT];
int Multi_ts_ship_info_line_count;

// status bar mode
static int Multi_ts_status_coords[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		95, 467, 426
	},
	{ // GR_1024
		152, 747, 688
	}
};

int Multi_ts_status_bar_mode = 0;

// carried icon information
int Multi_ts_carried_flag = 0;
int Multi_ts_clicked_flag = 0;
int Multi_ts_clicked_x,Multi_ts_clicked_y;
int Multi_ts_carried_ship_class;
int Multi_ts_carried_from_type = 0;
int Multi_ts_carried_from_index = 0;

// selected ship types (for informational purposes)
int Multi_ts_select_type = -1;
int Multi_ts_select_index = -1;
int Multi_ts_select_ship_class = -1;

// per-frame mouse hotspot vars
int Multi_ts_hotspot_type = -1;
int Multi_ts_hotspot_index = -1;

// operation types			
#define TS_GRAB_FROM_LIST									0
#define TS_SWAP_LIST_SLOT									1
#define TS_SWAP_SLOT_SLOT									2
#define TS_DUMP_TO_LIST										3
#define TS_SWAP_PLAYER_PLAYER								4
#define TS_MOVE_PLAYER										5

// packet codes
#define TS_CODE_LOCK_TEAM									0						// the specified team's slots are locked
#define TS_CODE_PLAYER_UPDATE								1						// a player slot update for the specified team

// team data
#define MULTI_TS_FLAG_NONE									-2						// slot is _always_ empty
#define MULTI_TS_FLAG_EMPTY								-1						// flag is temporarily empty
typedef struct ts_team_data {
	int multi_ts_objnum[MULTI_TS_NUM_SHIP_SLOTS];							// objnums for all slots in this team
	net_player *multi_ts_player[MULTI_TS_NUM_SHIP_SLOTS];					// net players corresponding to the same slots
	int multi_ts_flag[MULTI_TS_NUM_SHIP_SLOTS];								// flags indicating the "status" of a slot
	int multi_players_locked;														// are the players locked into place
} ts_team_data;
ts_team_data Multi_ts_team[MULTI_TS_MAX_TVT_TEAMS];								// data for all teams

// deleted ship objnums
int Multi_ts_deleted_objnums[MULTI_TS_MAX_TVT_TEAMS * MULTI_TS_NUM_SHIP_SLOTS];
int Multi_ts_num_deleted;

//XSTR:ON

// ------------------------------------------------------------------------------------------------------
// TEAM SELECT FORWARD DECLARATIONS
//

// check for button presses
void multi_ts_check_buttons();

// act on a button press
void multi_ts_button_pressed(int n);

// initialize all screen data, etc
void multi_ts_init_graphics();

// blit all of the icons representing all wings
void multi_ts_blit_wings();

// blit all of the player callsigns under the correct ships
void multi_ts_blit_wing_callsigns();

// blit the ships on the avail list
void multi_ts_blit_avail_ships();

// initialize the snazzy menu stuff for dragging ships,players around
void multi_ts_init_snazzy();

// what type of region the index is (0 == ship avail list, 1 == ship slots, 2 == player slot)
int multi_ts_region_type(int region);

// convert the region num to a ship slot index
int multi_ts_slot_index(int region);

// convert the region num to an avail list index
int multi_ts_avail_index(int region);

// convert the region num to a player slot index
int multi_ts_player_index(int region);

// blit the status bar
void multi_ts_blit_status_bar();

// assign the correct players to the correct slots
void multi_ts_init_players();

// assign the correct objnums to the correct slots
void multi_ts_init_objnums();

// assign the correct flags to the correct slots
void multi_ts_init_flags();

// get the proper team and slot index for the given ship name
void multi_ts_get_team_and_slot(char *ship_name,int *team_index,int *slot_index);

// handle an available ship scroll down button press
void multi_ts_avail_scroll_down();

// handle an available ship scroll up button press
void multi_ts_avail_scroll_up();

// handle all mouse events (clicking, dragging, and dropping)
void multi_ts_handle_mouse();

// can the specified player perform the action he is attempting
int multi_ts_can_perform(int from_type,int from_index,int to_type,int to_index,int ship_class,int player_index = -1);

// determine the kind of drag and drop operation this is
int multi_ts_get_dnd_type(int from_type,int from_index,int to_type,int to_index,int player_index = -1);

// swap two player positions
int multi_ts_swap_player_player(int from_index,int to_index,int *sound,int player_index = -1);

// move a player
int multi_ts_move_player(int from_index,int to_index,int *sound,int player_index = -1);

// get the ship class of the current index in the avail list or -1 if none exists
int multi_ts_get_avail_ship_class(int index);

// blit the currently carried icon (if any)
void multi_ts_blit_carried_icon();

// if the (console) player is allowed to grab a player slot at this point
int multi_ts_can_grab_player(int slot_index,int player_index = -1);

// return the bitmap index into the ships icon array (in ship select) which should be displayed for the given slot
int multi_ts_slot_bmap_num(int slot_index);

// blit any active ship information text
void multi_ts_blit_ship_info();

// select the given slot and setup any information, etc
void multi_ts_select_ship();				

// is it ok for this player to commit 
int multi_ts_ok_to_commit();

// return the bitmap index into the ships icon array (in ship select) which should be displayed for the given slot
int multi_ts_avail_bmap_num(int slot_index);

// set the status bar to reflect the status of wing slots (free or not free). 0 or 1 are valid values for now
void multi_ts_set_status_bar_mode(int m);

// check to see that no illegal ship settings have occurred
void multi_ts_check_errors();

// ------------------------------------------------------------------------------------------------------
// TEAM SELECT FUNCTIONS
//

// initialize the team select screen (always call, even when switching between weapon select, etc)
void multi_ts_init()
{
	// if we haven't initialized at all yet, then do it
	if(!Multi_ts_inited){
		multi_ts_init_graphics();
		Multi_ts_inited = 1;
	}	

	// use the common interface palette
	multi_common_set_palette();

	// set the interface palette
	// common_set_interface_palette(MULTI_TS_PALETTE);

	Net_player->state = NETPLAYER_STATE_SHIP_SELECT;

	Current_screen = ON_SHIP_SELECT;
}

// initialize all critical internal data structures
void multi_ts_common_init()
{
	int idx;	
	
	// reset timestamps here. they seem to get hosed by the loadinh of the mission file
	multi_reset_timestamps();

	// saying "not allowed to mess with ships"
	Multi_ts_status_bar_mode = 0;		

	// intialize ship info stuff
	memset(Multi_ts_ship_info_text,0,MULTI_TS_SHIP_INFO_MAX_TEXT);
	memset(Multi_ts_ship_info_lines,0,MULTI_TS_SHIP_INFO_MAX_TEXT);
	Multi_ts_ship_info_line_count = 0;			

	// initialize carried icon information	
	Multi_ts_carried_flag = 0;
	Multi_ts_clicked_flag = 0;
	Multi_ts_clicked_x = 0;
	Multi_ts_clicked_y = 0;
	Multi_ts_carried_ship_class = -1;
	Multi_ts_carried_from_type = 0;
	Multi_ts_carried_from_index = 0;

	// selected slot information (should be default player ship)
	if(!MULTI_PERM_OBSERVER(Net_players[MY_NET_PLAYER_NUM])){
		Multi_ts_select_type = MULTI_TS_SLOT_LIST;
		Multi_ts_select_index = Net_player->p_info.ship_index;
		
		// select this ship and setup his info
		Multi_ts_select_ship_class = Wss_slots[Multi_ts_select_index].ship_class;
		multi_ts_select_ship();
	} else {
		Multi_ts_select_type = -1;
		Multi_ts_select_index = -1;

		// no ship class selected for information purposes
		Multi_ts_select_ship_class = -1;
	}

	// deleted ship information
	memset(Multi_ts_deleted_objnums,0,sizeof(int) * MULTI_TS_MAX_TVT_TEAMS * MULTI_TS_NUM_SHIP_SLOTS);
	Multi_ts_num_deleted = 0;

	// mouse hotspot information
	Multi_ts_hotspot_type = -1;
	Multi_ts_hotspot_index = -1;

	// initialize avail ship list data
	Multi_ts_avail_start = 0;

	// load the locked button bitmaps bitmaps
	for(idx=0;idx<MULTI_TS_NUM_LOCKED_BITMAPS;idx++){
		Multi_ts_locked_bitmaps[idx] = -1;	
		Multi_ts_locked_bitmaps[idx] = bm_load(Multi_ts_bmap_names[gr_screen.res][idx]);
	}	

	// blast the team data clean
	memset(Multi_ts_team,0,sizeof(ts_team_data) * MULTI_TS_MAX_TVT_TEAMS);	

	// assign the correct players to the correct slots
	multi_ts_init_players();

	// assign the correct objnums to the correct slots
	multi_ts_init_objnums();

	// sync the interface as normal
	multi_ts_sync_interface();	
}

// do frame for team select
void multi_ts_do()
{	
	int k = chatbox_process();
	k = Multi_ts_window.process(k);

	// process any keypresses
	switch(k){
	case KEY_ESC :		
		gamesnd_play_iface(SND_USER_SELECT);
		multi_quit_game(PROMPT_ALL);
		break;	

	// cycle to the weapon select screen
	case KEY_TAB :
		gamesnd_play_iface(SND_USER_SELECT);
		Next_screen = ON_WEAPON_SELECT;
		gameseq_post_event(GS_EVENT_WEAPON_SELECTION);
		break;

	case KEY_ENTER|KEY_CTRLED:
	//	multi_ts_commit_pressed();
		Commit_pressed = 1;
		break;
	}		

	// check any button presses
	multi_ts_check_buttons();	

	// handle all mouse related events
	multi_ts_handle_mouse();

	// check for errors
	multi_ts_check_errors();
	
	// draw the background, etc
	gr_reset_clip();	
	GR_MAYBE_CLEAR_RES(Multi_ts_bitmap);
	if(Multi_ts_bitmap != -1){
		gr_set_bitmap(Multi_ts_bitmap);
		gr_bitmap(0,0);
	}
	Multi_ts_window.draw();

	// render all wings
	multi_ts_blit_wings();

	// blit all callsigns
	multi_ts_blit_wing_callsigns();

	// blit the ships on the available list
	multi_ts_blit_avail_ships();

	// force draw the ship select button
	Multi_ts_buttons[gr_screen.res][MULTI_TS_SHIP_SELECT].button.draw_forced(2);	

	// force draw the "locked" button if necessary
	if(multi_ts_is_locked()){
		Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button.draw_forced(2);
	} else {
		if( ((Netgame.type_flags & NG_TYPE_TEAM) && !(Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN)) ||
			 ((Netgame.type_flags & NG_TYPE_TEAM) && !(Net_player->flags & NETINFO_FLAG_GAME_HOST)) ){
			Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button.draw_forced(0);
		} else {
			Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button.draw();
		}
	}

	// blit any active ship information
	multi_ts_blit_ship_info();

	// blit the status bar
	multi_ts_blit_status_bar();	

	// render the chatbox
	chatbox_render();	

	// render tooltips
	Multi_ts_window.draw_tooltip();

	// display the status of the voice system
	multi_common_voice_display_status();

	// blit any carried icons
	multi_ts_blit_carried_icon();
	
	// flip the buffer
	gr_flip();

	// If the commit button was pressed, do the commit button actions.  Done at the end of the
	// loop since we are a bit special and have to close out in the proper order.
	if ( Commit_pressed ) {		
		multi_ts_commit_pressed();		
		Commit_pressed = 0;
	}
}

// close the team select screen (always call, even when switching between weapon select, etc)
void multi_ts_close()
{
	int idx;
	
	if(!Multi_ts_inited){
		return;
	}

	Multi_ts_inited = 0;
	
	// shut down the snazzy menu
	snazzy_menu_close();
	
	// unload any bitmaps
	if(!bm_unload(Multi_ts_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_ts_bitmap_fname[gr_screen.res]));
	}		
	for(idx=0;idx<MULTI_TS_NUM_LOCKED_BITMAPS;idx++){
		if(Multi_ts_locked_bitmaps[idx] != -1){
			bm_release(Multi_ts_locked_bitmaps[idx]);
			Multi_ts_locked_bitmaps[idx] = -1;
		}
	}
	
	// destroy the UI_WINDOW
	Multi_ts_window.destroy();
}

// is the given slot disabled for the specified player
int multi_ts_disabled_slot(int slot_num, int player_index)
{
	net_player *pl;

	// get the appropriate net player
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}

	// if the player is an observer, its _always_ disabled
	if(pl->flags & NETINFO_FLAG_OBSERVER){
		return 1;
	}

	// if the flag for this team isn't set to "free" we can't do anything
	if(!Multi_ts_team[pl->p_info.team].multi_players_locked){
		return 1;
	}

	// if the "leaders" only flag is set
	if(Netgame.options.flags & MSO_FLAG_SS_LEADERS){
		// in a team vs. team situation
		if(Netgame.type_flags & NG_TYPE_TEAM){
			if(pl->flags & NETINFO_FLAG_TEAM_CAPTAIN){
				return 0;
			}
		} 
		// in a non team vs. team situation
		else {
			if(pl->flags & NETINFO_FLAG_GAME_HOST){
				return 0;
			}
		}
	} else {	
		// in a team vs. team situation
		if(Netgame.type_flags & NG_TYPE_TEAM){
			// if i'm the team captain I can mess with my own ships as well as those of the ai ships on my team
			if(pl->flags & NETINFO_FLAG_TEAM_CAPTAIN){
				if((Multi_ts_team[pl->p_info.team].multi_ts_player[slot_num] != NULL) && (Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[slot_num]].flags & OF_PLAYER_SHIP) && (slot_num != pl->p_info.ship_index)){
					return 1;
				}

				return 0;
			}
		}
		// in a non team vs. team situation
		else {			
			// if we're the host, we can our own ship and ai ships
			if(pl->flags & NETINFO_FLAG_GAME_HOST){
				// can't grab player ships
				if((Multi_ts_team[pl->p_info.team].multi_ts_player[slot_num] != NULL) && (Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[slot_num]].flags & OF_PLAYER_SHIP) && (slot_num != pl->p_info.ship_index)){
					return 1;
				}

				return 0;
			}
		}

		// if this is our slot, then we can grab it
		if(slot_num == pl->p_info.ship_index){
			return 0;
		}
	}

	return 1;
}

// is the given slot disabled for the specified player, _and_ it is his ship as well
int multi_ts_disabled_high_slot(int slot_index,int player_index)
{
	net_player *pl;

	// get the appropriate net player
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}

	// if this is disabled for him and its also _his_ slot
	if(multi_ts_disabled_slot(slot_index,player_index) && !Multi_ts_team[pl->p_info.team].multi_players_locked && (slot_index == pl->p_info.ship_index)){
		return 1;
	}

	return 0;
}

// resynch all display/interface elements based upon all the ship/weapon pool values
void multi_ts_sync_interface()
{
	int idx;
	
	// item 1 - determine how many ship types are available in the ship pool
	Multi_ts_avail_count = 0;
	for(idx=0;idx<Num_ship_classes;idx++){
		if(Ss_pool[idx] > 0){
			Multi_ts_avail_count++;
		}
	}

	// item 2 - make sure our local Multi_ts_slot_flag array is up to date
	multi_ts_init_flags();
	
	// item 3 - set/unset any necessary flags in underlying ship select data structures
	for(idx=0;idx<MAX_WSS_SLOTS;idx++){
		switch(Multi_ts_team[Net_player->p_info.team].multi_ts_flag[idx]){
		case MULTI_TS_FLAG_EMPTY :		
			ss_make_slot_empty(idx);
			break;
		case MULTI_TS_FLAG_NONE :
			break;
		default :		
			ss_make_slot_full(idx);
			break;
		}
	}

	// item 4 - reset the locked/unlocked status of all ships in the weapon select screen
	ss_recalc_multiplayer_slots();
}

void multi_ts_assign_players_all()
{
	int idx,team_index,slot_index,found,player_count,shipnum;	
	char name_lookup[100];
	object *objp;	
	
	// set all player ship indices to -1
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			Net_players[idx].p_info.ship_index = -1;
		}
	}

	// merge the created object list with the actual object list so we have all available ships
	obj_merge_created_list();		

	// get the # of players currently in the game
	player_count = multi_num_players();

	// always assign the host to the wing leader of one of the TVT wings
	// this is valid for coop games as well because the first starting wing
	// and the first tvt wing must have the same name
	memset(name_lookup,0,100);

	// To account for cases where <Wingname> 1 is not a player ship
	for (int i = 0; i < MAX_SHIPS_PER_WING; i++) {
		if(Netgame.type_flags & NG_TYPE_TEAM) {
			sprintf(name_lookup, "%s %d", TVT_wing_names[Netgame.host->p_info.team], i + 1);
		} else {
			sprintf(name_lookup, "%s %d", TVT_wing_names[0], i + 1);
		}

		if (!stricmp(name_lookup, Player_start_shipname))
			break;
	}
		
	shipnum = ship_name_lookup(name_lookup);
	
	// if we couldn't find the ship for the host
	if(shipnum == -1){
		// Netgame.flags |= NG_FLAG_QUITTING;
		multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_SHIP_ASSIGN);
		return;
	}

	multi_ts_get_team_and_slot(Ships[shipnum].ship_name,&team_index,&slot_index);
	multi_assign_player_ship(NET_PLAYER_INDEX(Netgame.host),&Objects[Ships[shipnum].objnum],Ships[shipnum].ship_info_index);
	Netgame.host->p_info.ship_index = slot_index;
	Assert(Netgame.host->p_info.ship_index >= 0);
	Netgame.host->p_info.ship_class = Ships[shipnum].ship_info_index;
	Netgame.host->m_player->objnum = Ships[shipnum].objnum;						

	// for each netplayer, try and find a ship
	objp = GET_FIRST(&obj_used_list);
	while(objp != END_OF_LIST(&obj_used_list)){
		// find a valid player ship - ignoring the ship which was assigned to the host
		if((objp->flags & OF_PLAYER_SHIP) && stricmp(Ships[objp->instance].ship_name,name_lookup)){
			// determine what team and slot this ship is				
			multi_ts_get_team_and_slot(Ships[objp->instance].ship_name,&team_index,&slot_index);
			Assert((team_index != -1) && (slot_index != -1));

			// in a team vs. team situation
			if(Netgame.type_flags & NG_TYPE_TEAM){
				// find a player on this team who needs a ship
				found = 0;
				for(idx=0;idx<MAX_PLAYERS;idx++){
					if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && (Net_players[idx].p_info.ship_index == -1) && (Net_players[idx].p_info.team == team_index)){
						found = 1;
						break;
					}
				}			
			}
			// in a non team vs. team situation			
			else {
				// find any player on this who needs a ship
				found = 0;
				for(idx=0;idx<MAX_PLAYERS;idx++){
					if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx]) && (Net_players[idx].p_info.ship_index == -1)){					
						found = 1;
						break;
					}
				}			
			}

			// if we found a player
			if(found){
				multi_assign_player_ship(idx,objp,Ships[objp->instance].ship_info_index);
				Net_players[idx].p_info.ship_index = slot_index;
				Assert(Net_players[idx].p_info.ship_index >= 0);
				Net_players[idx].p_info.ship_class = Ships[objp->instance].ship_info_index;
				Net_players[idx].m_player->objnum = OBJ_INDEX(objp);					
				
				// decrement the player count
				player_count--;
			} else {
				objp->flags &= ~OF_PLAYER_SHIP;
				obj_set_flags( objp, objp->flags | OF_COULD_BE_PLAYER );
			}

			// if we've assigned all players, we're done
			if(player_count <= 0){
				break;
			}
		}		
		
		// move to the next item
		objp = GET_NEXT(objp);		
	}	
	
	// go through and change any ships marked as player ships to be COULD_BE_PLAYER
	if ( objp != END_OF_LIST(&obj_used_list) ) {
		for ( objp = GET_NEXT(objp); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if ( objp->flags & OF_PLAYER_SHIP ){
				objp->flags &= ~OF_PLAYER_SHIP;
				obj_set_flags( objp, objp->flags | OF_COULD_BE_PLAYER );
			}
		}
	}	
	
	if(Game_mode & GM_STANDALONE_SERVER){
		Player_obj = NULL;
		//Net_player->m_player->objnum = -1;
	}

	// check to make sure all players were assigned correctly
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_OBSERVER(Net_players[idx])){
			// if this guy never got assigned a player ship, there's a mission problem
			if(Net_players[idx].p_info.ship_index == -1){
				// Netgame.flags |= NG_FLAG_QUITTING;
				multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_SHIP_ASSIGN);
				return;
			}
		}
	}
}

// delete ships which have been removed from the game, tidy things 
void multi_ts_create_wings()
{
	int idx,s_idx;
	
	// the standalone never went through this screen so he should never call this function!
	// the standalone and all other clients will have this equivalent function performed whey they receive
	// the post_sync_data_packet!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Assert(!(Game_mode & GM_STANDALONE_SERVER));	
	
	// check status of all ships and delete or change ship type as necessary
	Multi_ts_num_deleted = 0;
	for(idx=0;idx<MULTI_TS_MAX_TVT_TEAMS;idx++){
		for(s_idx=0;s_idx<MULTI_TS_NUM_SHIP_SLOTS;s_idx++){	
			// otherwise if there's a valid ship in this spot
			if(Multi_ts_team[idx].multi_ts_flag[s_idx] >= 0){
				int objnum;

				// set the ship type appropriately
				Assert(Wss_slots_teams[idx][s_idx].ship_class >= 0);

				objnum = Multi_ts_team[idx].multi_ts_objnum[s_idx];
				change_ship_type(Objects[objnum].instance,Wss_slots_teams[idx][s_idx].ship_class);

				// set the ship weapons correctly
				wl_update_ship_weapons(objnum,&Wss_slots_teams[idx][s_idx]);
				
				// assign ts_index of the ship to point to the proper Wss_slots slot
				Ships[Objects[objnum].instance].ts_index = s_idx;
			} else if(Multi_ts_team[idx].multi_ts_flag[s_idx] == MULTI_TS_FLAG_EMPTY){		
				Assert(Multi_ts_team[idx].multi_ts_objnum[s_idx] >= 0);			

				// mark the object as having been deleted
				Multi_ts_deleted_objnums[Multi_ts_num_deleted] = Multi_ts_team[idx].multi_ts_objnum[s_idx];

				// delete the ship
				ship_add_exited_ship( &Ships[Objects[Multi_ts_deleted_objnums[Multi_ts_num_deleted]].instance], SEF_PLAYER_DELETED );
				obj_delete(Multi_ts_deleted_objnums[Multi_ts_num_deleted]);			
				ship_wing_cleanup(Objects[Multi_ts_deleted_objnums[Multi_ts_num_deleted]].instance,&Wings[Ships[Objects[Multi_ts_team[idx].multi_ts_objnum[s_idx]].instance].wingnum]);

				// increment the # of ships deleted
				Multi_ts_num_deleted++;
			}
		}
	}	
}

// do any necessary processing for players who have left the game
void multi_ts_handle_player_drop()
{
	int idx,s_idx;

	// find the player
	for(idx=0;idx<MULTI_TS_MAX_TVT_TEAMS;idx++){
		for(s_idx=0;s_idx<MULTI_TS_NUM_SHIP_SLOTS;s_idx++){
			// if we found him, clear his player slot and set his object back to being  OF_COULD_BE_PLAYER
			if((Multi_ts_team[idx].multi_ts_player[s_idx] != NULL) && !MULTI_CONNECTED((*Multi_ts_team[idx].multi_ts_player[s_idx]))){
				Assert(Multi_ts_team[idx].multi_ts_objnum[s_idx] != -1);
				Multi_ts_team[idx].multi_ts_player[s_idx] = NULL;
				Objects[Multi_ts_team[idx].multi_ts_objnum[s_idx]].flags &= ~(OF_PLAYER_SHIP);
				obj_set_flags( &Objects[Multi_ts_team[idx].multi_ts_objnum[s_idx]], Objects[Multi_ts_team[idx].multi_ts_objnum[s_idx]].flags | OF_COULD_BE_PLAYER);
			}
		}
	}
}

// set the status bar to reflect the status of wing slots (free or not free). 0 or 1 are valid values for now
void multi_ts_set_status_bar_mode(int m)
{
	Multi_ts_status_bar_mode = m;
}

// blit the proper "locked" button - used for weapon select and briefing screens
void multi_ts_blit_locked_button()
{		
	// if we're locked down and we have a valid bitmap
	if((Multi_ts_team[Net_player->p_info.team].multi_players_locked) && (Multi_ts_locked_bitmaps[2] != -1)){
		gr_set_bitmap(Multi_ts_locked_bitmaps[2]);
		gr_bitmap(Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].x, Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].y);
	}
	// draw as "not locked" if possible
	else if(Multi_ts_locked_bitmaps[0] != -1){
		gr_set_bitmap(Multi_ts_locked_bitmaps[0]);
		gr_bitmap( Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].x, Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].y);
	}
}

// the "lock" button has been pressed
void multi_ts_lock_pressed()
{
	// do nothing if the button has already been pressed
	if(multi_ts_is_locked()){
		gamesnd_play_iface(SND_GENERAL_FAIL);
		return;
	}
	
	if(Netgame.type_flags & NG_TYPE_TEAM){
		Assert(Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN);
	} else {
		Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);
	}
	gamesnd_play_iface(SND_USER_SELECT);

	// send a final player slot update packet		
	send_pslot_update_packet(Net_player->p_info.team,TS_CODE_LOCK_TEAM,-1);				
	Multi_ts_team[Net_player->p_info.team].multi_players_locked = 1;

	// sync interface stuff
	multi_ts_set_status_bar_mode(1);
	multi_ts_sync_interface();
	ss_recalc_multiplayer_slots();		

	// disable this button now
	Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button.disable();
}

// if i'm "locked"
int multi_ts_is_locked()
{
	return Multi_ts_team[Net_player->p_info.team].multi_players_locked;
}

// show a popup saying "only host and team captains can modify, etc, etc"
void multi_ts_maybe_host_only_popup()
{
/*
	// if this is because the "host modifies" option is set				
				if((Netgame.options.flags & MSO_FLAG_SS_LEADERS) && !(Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
					multi_ts_host_only_popup();					
				}

	if(Netgame.type == NG_TYPE_TEAM){
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,"Only team captains may modify ships and weapons in this game");
	} else {
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,"Only the host may modify ships and weapons in this game");
	}
	*/
}


// ------------------------------------------------------------------------------------------------------
// TEAM SELECT FORWARD DEFINITIONS
//

// check for button presses
void multi_ts_check_buttons()
{
	int idx;
	for(idx=0;idx<MULTI_TS_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Multi_ts_buttons[gr_screen.res][idx].button.pressed()){
			multi_ts_button_pressed(idx);
			break;
		}
	}
}

// act on a button press
void multi_ts_button_pressed(int n)
{
	switch(n){
	// back to the briefing screen
	case MULTI_TS_BRIEFING :
		gamesnd_play_iface(SND_USER_SELECT);
		Next_screen = ON_BRIEFING_SELECT;
		gameseq_post_event( GS_EVENT_START_BRIEFING );
		break;
	// already on this screen
	case MULTI_TS_SHIP_SELECT:
		gamesnd_play_iface(SND_GENERAL_FAIL);
		break;
	// back to the weapon select screen
	case MULTI_TS_WEAPON_SELECT:
		gamesnd_play_iface(SND_USER_SELECT);
		Next_screen = ON_WEAPON_SELECT;
		gameseq_post_event(GS_EVENT_WEAPON_SELECTION);
		break;
	// scroll the available ships list down
	case MULTI_TS_SHIPS_DOWN:		
		multi_ts_avail_scroll_down();
		break;
	// scroll the available ships list up
	case MULTI_TS_SHIPS_UP:		
		multi_ts_avail_scroll_up();
		break;
	// free ship/weapon select
	case MULTI_TS_LOCK:				
		Assert(Game_mode & GM_MULTIPLAYER);			
		// the "lock" button has been pressed
		multi_ts_lock_pressed();

		// disable the button if it is now locked
		if(multi_ts_is_locked()){
			Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button.disable();
		}
		break;
	// commit button
	case MULTI_TS_COMMIT :				
	//	multi_ts_commit_pressed();
		Commit_pressed = 1;
		break;
	default :
		gamesnd_play_iface(SND_GENERAL_FAIL);		
		break;
	}
}

// initialize all screen data, etc
void multi_ts_init_graphics()
{
	int idx;
	
	// create the interface window
	Multi_ts_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_ts_window.set_mask_bmap(Multi_ts_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_ts_bitmap = bm_load(Multi_ts_bitmap_fname[gr_screen.res]);
	if(Multi_ts_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}
			
	// create the interface buttons
	for(idx=0;idx<MULTI_TS_NUM_BUTTONS;idx++){
		// create the object
		if((idx == MULTI_TS_SHIPS_UP) || (idx == MULTI_TS_SHIPS_DOWN)){
			Multi_ts_buttons[gr_screen.res][idx].button.create(&Multi_ts_window, "", Multi_ts_buttons[gr_screen.res][idx].x, Multi_ts_buttons[gr_screen.res][idx].y, 1, 1, 1, 1);
		} else {
			Multi_ts_buttons[gr_screen.res][idx].button.create(&Multi_ts_window, "", Multi_ts_buttons[gr_screen.res][idx].x, Multi_ts_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);
		}

		// set the sound to play when highlighted
		Multi_ts_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_ts_buttons[gr_screen.res][idx].button.set_bmaps(Multi_ts_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Multi_ts_buttons[gr_screen.res][idx].button.link_hotspot(Multi_ts_buttons[gr_screen.res][idx].hotspot);
	}		

	// add some text	
	Multi_ts_window.add_XSTR("Briefing", 765, Multi_ts_buttons[gr_screen.res][MULTI_TS_BRIEFING].xt, Multi_ts_buttons[gr_screen.res][MULTI_TS_BRIEFING].yt, &Multi_ts_buttons[gr_screen.res][MULTI_TS_BRIEFING].button, UI_XSTR_COLOR_GREEN);
	Multi_ts_window.add_XSTR("Ship Selection", 1067, Multi_ts_buttons[gr_screen.res][MULTI_TS_SHIP_SELECT].xt, Multi_ts_buttons[gr_screen.res][MULTI_TS_SHIP_SELECT].yt, &Multi_ts_buttons[gr_screen.res][MULTI_TS_SHIP_SELECT].button, UI_XSTR_COLOR_GREEN);
	Multi_ts_window.add_XSTR("Weapon Loadout", 1068, Multi_ts_buttons[gr_screen.res][MULTI_TS_WEAPON_SELECT].xt, Multi_ts_buttons[gr_screen.res][MULTI_TS_WEAPON_SELECT].yt, &Multi_ts_buttons[gr_screen.res][MULTI_TS_WEAPON_SELECT].button, UI_XSTR_COLOR_GREEN);
	Multi_ts_window.add_XSTR("Commit", 1062, Multi_ts_buttons[gr_screen.res][MULTI_TS_COMMIT].xt, Multi_ts_buttons[gr_screen.res][MULTI_TS_COMMIT].yt, &Multi_ts_buttons[gr_screen.res][MULTI_TS_COMMIT].button, UI_XSTR_COLOR_PINK);
	Multi_ts_window.add_XSTR("Lock", 1270, Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].xt, Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].yt, &Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button, UI_XSTR_COLOR_GREEN);

//	Multi_ts_window.add_XSTR("Help", 928, Multi_ts_buttons[Current_screen-1][gr_screen.res][COMMON_HELP_BUTTON].xt, Multi_ts_buttons[Current_screen-1][gr_screen.res][COMMON_HELP_BUTTON].yt, &Multi_ts_buttons[Current_screen-1][gr_screen.res][COMMON_HELP_BUTTON].button, UI_XSTR_COLOR_GREEN);
//	Multi_ts_window.add_XSTR("Options", 1036, Multi_ts_buttons[Current_screen-1][gr_screen.res][COMMON_OPTIONS_BUTTON].xt, Multi_ts_buttons[Current_screen-1][gr_screen.res][COMMON_OPTIONS_BUTTON].yt, &Multi_ts_buttons[Current_screen-1][gr_screen.res][COMMON_OPTIONS_BUTTON].button, UI_XSTR_COLOR_GREEN);



	// make the ship scrolling lists

	// if we're not the host of the game (or a tema captain in team vs. team mode), disable the lock button
	if (Netgame.type_flags & NG_TYPE_TEAM) {
		if(!(Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
			Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button.disable();
		}
	} else {
		if(!(Net_player->flags & NETINFO_FLAG_GAME_HOST)){
			Multi_ts_buttons[gr_screen.res][MULTI_TS_LOCK].button.disable();
		}
	}

	// initialize the snazzy menu stuff (for grabbing ships, names, etc)
	multi_ts_init_snazzy();

	// create the chatbox (again, should not be necessary at this point)	
	chatbox_create();	

	// sync the interface as normal
	multi_ts_sync_interface();
}

// blit all of the icons representing all wings
void multi_ts_blit_wings()
{
	int idx;

	// blit them all blindly for now
	for(idx=0;idx<MAX_WSS_SLOTS;idx++){
		// if this ship doesn't exist, then continue
		if(Multi_ts_team[Net_player->p_info.team].multi_ts_flag[idx] == MULTI_TS_FLAG_NONE){
			continue;
		}

		// otherwise blit the ship icon or the "empty" icon
		if(Multi_ts_team[Net_player->p_info.team].multi_ts_flag[idx] == MULTI_TS_FLAG_EMPTY){
			ss_blit_ship_icon(Multi_ts_slot_icon_coords[idx][gr_screen.res][MULTI_TS_X_COORD],Multi_ts_slot_icon_coords[idx][gr_screen.res][MULTI_TS_Y_COORD],-1,0);
		} else {
			ss_blit_ship_icon(Multi_ts_slot_icon_coords[idx][gr_screen.res][MULTI_TS_X_COORD],Multi_ts_slot_icon_coords[idx][gr_screen.res][MULTI_TS_Y_COORD],Wss_slots[idx].ship_class,multi_ts_slot_bmap_num(idx));

			// if this is a team vs team game, and the slot is occupised by a team captain, put a c there
			if((Netgame.type_flags & NG_TYPE_TEAM) && (Multi_ts_team[Net_player->p_info.team].multi_ts_player[idx] != NULL) && (Multi_ts_team[Net_player->p_info.team].multi_ts_player[idx]->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				gr_set_color_fast(&Color_bright);
				gr_string(Multi_ts_slot_icon_coords[idx][gr_screen.res][MULTI_TS_X_COORD] - 5,Multi_ts_slot_icon_coords[idx][gr_screen.res][MULTI_TS_Y_COORD] - 5, XSTR("C",737));  // [[ Team captain ]]
			}
		}
	}	
}

// blit all of the player callsigns under the correct ships
void multi_ts_blit_wing_callsigns()
{
	int idx,callsign_w;
	char callsign[CALLSIGN_LEN+2];
	p_object *pobj;

	// blit them all blindly for now
	for(idx=0;idx<MAX_WSS_SLOTS;idx++){		
		// if this ship doesn't exist, then continue
		if(Multi_ts_team[Net_player->p_info.team].multi_ts_flag[idx] == MULTI_TS_FLAG_NONE){
			continue;
		}		

		// if there is a player in the slot
		if(Multi_ts_team[Net_player->p_info.team].multi_ts_player[idx] != NULL){
			// make sure the string fits
			strcpy_s(callsign,Multi_ts_team[Net_player->p_info.team].multi_ts_player[idx]->m_player->callsign);
		} else {
			// determine if this is a locked AI ship
			pobj = mission_parse_get_arrival_ship(Ships[Objects[Multi_ts_team[Net_player->p_info.team].multi_ts_objnum[idx]].instance].ship_name);			
			if((pobj == NULL) || !(pobj->flags & OF_PLAYER_SHIP)){
				strcpy_s(callsign, NOX("<"));
				strcat_s(callsign, XSTR("AI",738));  // [[ Artificial Intellegence ]]						
				strcat_s(callsign, NOX(">"));
			} else {
				strcpy_s(callsign, XSTR("AI",738));  // [[ Artificial Intellegence ]]						
			}
		}
			
		gr_force_fit_string(callsign, CALLSIGN_LEN, Multi_ts_slot_text_coords[idx][gr_screen.res][MULTI_TS_W_COORD]);

		// get the final length
		gr_get_string_size(&callsign_w, NULL, callsign);

		// blit the string
		if((Multi_ts_hotspot_type == MULTI_TS_PLAYER_LIST) && (Multi_ts_hotspot_index == idx) && (Multi_ts_team[Net_player->p_info.team].multi_ts_player[idx] != NULL)){
			gr_set_color_fast(&Color_text_active_hi);
		} else {
			gr_set_color_fast(&Color_normal);
		}
		gr_string(Multi_ts_slot_text_coords[idx][gr_screen.res][MULTI_TS_X_COORD] + ((Multi_ts_slot_text_coords[idx][gr_screen.res][MULTI_TS_W_COORD] - callsign_w)/2),Multi_ts_slot_text_coords[idx][gr_screen.res][MULTI_TS_Y_COORD],callsign);								
	}	
}

// blit the ships on the avail list
void multi_ts_blit_avail_ships()
{
	int display_count,ship_count,idx;
	char count[6];

	// blit the availability of all ship counts
	display_count = 0;
	ship_count = 0;
	for(idx=0;idx<Num_ship_classes;idx++){
		if(Ss_pool[idx] > 0){
			// if our starting display index is after this, then skip it
			if(ship_count < Multi_ts_avail_start){
				ship_count++;
			} else {
				// blit the icon 
				ss_blit_ship_icon(Multi_ts_avail_coords[display_count][gr_screen.res][MULTI_TS_X_COORD],Multi_ts_avail_coords[display_count][gr_screen.res][MULTI_TS_Y_COORD],idx,multi_ts_avail_bmap_num(display_count));

				// blit the ship count available
				sprintf(count,"%d",Ss_pool[idx]);
				gr_set_color_fast(&Color_normal);
				gr_string(Multi_ts_avail_coords[display_count][gr_screen.res][MULTI_TS_X_COORD] - 20,Multi_ts_avail_coords[display_count][gr_screen.res][MULTI_TS_Y_COORD],count);

				// increment the counts
				display_count++;				
				ship_count++;
			}
		}

		// if we've reached the max amount we can display, then stop
		if(display_count >= MULTI_TS_AVAIL_MAX_DISPLAY){
			return;
		}
	}
}

// initialize the snazzy menu stuff for dragging ships,players around
void multi_ts_init_snazzy()
{	
	// initialize the snazzy menu
	snazzy_menu_init();

	// blast the data
	Multi_ts_snazzy_regions = 0;
	memset(Multi_ts_region,0,sizeof(MENU_REGION) * MULTI_TS_NUM_SNAZZY_REGIONS);	

	// get a pointer to the mask bitmap data
	Multi_ts_mask_data = Multi_ts_window.get_mask_data(&Multi_ts_mask_w, &Multi_ts_mask_h);

	// add the wing slots information
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_SHIP_0,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_SHIP_1,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_SHIP_2,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_SHIP_3,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_SHIP_0,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_SHIP_1,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_SHIP_2,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_SHIP_3,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_SHIP_0,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_SHIP_1,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_SHIP_2,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_SHIP_3,		0);

	// add the name slots information
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_NAME_0,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_NAME_1,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_NAME_2,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_0_NAME_3,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_NAME_0,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_NAME_1,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_NAME_2,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_1_NAME_3,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_NAME_0,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_NAME_1,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_NAME_2,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_2_NAME_3,		0);

	// add the available ships region
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_LIST_0,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_LIST_1,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_LIST_2,		0);
	snazzy_menu_add_region(&Multi_ts_region[Multi_ts_snazzy_regions++], "",	TSWING_LIST_3,		0);	
}

// what type of region the index is (0 == ship avail list, 1 == ship slots, 2 == player slot)
int multi_ts_region_type(int region)
{
	if((region == TSWING_0_SHIP_0) || (region == TSWING_0_SHIP_1) || (region == TSWING_0_SHIP_2) || (region == TSWING_0_SHIP_3) ||
		(region == TSWING_1_SHIP_0) || (region == TSWING_1_SHIP_1) || (region == TSWING_1_SHIP_2) || (region == TSWING_1_SHIP_3) ||
		(region == TSWING_2_SHIP_0) || (region == TSWING_2_SHIP_1) || (region == TSWING_2_SHIP_2) || (region == TSWING_2_SHIP_3) ){
		return MULTI_TS_SLOT_LIST;
	}

	if((region == TSWING_0_NAME_0) || (region == TSWING_0_NAME_1) || (region == TSWING_0_NAME_2) || (region == TSWING_0_NAME_3) ||
		(region == TSWING_1_NAME_0) || (region == TSWING_1_NAME_1) || (region == TSWING_1_NAME_2) || (region == TSWING_1_NAME_3) ||
		(region == TSWING_2_NAME_0) || (region == TSWING_2_NAME_1) || (region == TSWING_2_NAME_2) || (region == TSWING_2_NAME_3) ){
		return MULTI_TS_PLAYER_LIST;
	}

	if((region == TSWING_LIST_0) || (region == TSWING_LIST_1) || (region == TSWING_LIST_2) || (region == TSWING_LIST_3)){
		return MULTI_TS_AVAIL_LIST;
	}

	return -1;
}

// convert the region num to a ship slot index
int multi_ts_slot_index(int region)
{
	switch(region){
	case TSWING_0_SHIP_0:
		return 0;
	case TSWING_0_SHIP_1:
		return 1;
	case TSWING_0_SHIP_2:
		return 2;
	case TSWING_0_SHIP_3:
		return 3;
	case TSWING_1_SHIP_0:
		return 4;
	case TSWING_1_SHIP_1:
		return 5;
	case TSWING_1_SHIP_2:
		return 6;
	case TSWING_1_SHIP_3:
		return 7;
	case TSWING_2_SHIP_0:
		return 8;
	case TSWING_2_SHIP_1:
		return 9;
	case TSWING_2_SHIP_2:
		return 10;
	case TSWING_2_SHIP_3:
		return 11;
	}

	return -1;	
}

// convert the region num to an avail list index (starting from absolute 0)
int multi_ts_avail_index(int region)
{
	switch(region){
	case TSWING_LIST_0:
		return 0;
	case TSWING_LIST_1:
		return 1;
	case TSWING_LIST_2:
		return 2;
	case TSWING_LIST_3:
		return 3;
	}

	return -1;
}

// convert the region num to a player slot index
int multi_ts_player_index(int region)
{
	switch(region){
	case TSWING_0_NAME_0:
		return 0;
	case TSWING_0_NAME_1:
		return 1;
	case TSWING_0_NAME_2:
		return 2;
	case TSWING_0_NAME_3:
		return 3;
	case TSWING_1_NAME_0:
		return 4;
	case TSWING_1_NAME_1:
		return 5;
	case TSWING_1_NAME_2:
		return 6;
	case TSWING_1_NAME_3:
		return 7;
	case TSWING_2_NAME_0:
		return 8;
	case TSWING_2_NAME_1:
		return 9;
	case TSWING_2_NAME_2:
		return 10;
	case TSWING_2_NAME_3:
		return 11;
	}

	return -1;	
}

// blit any active ship information text
void multi_ts_blit_ship_info()
{
	int y_start;
	ship_info *sip;
	char str[100];

	// if we don't have a valid ship selected, do nothing
	if(Multi_ts_select_ship_class == -1){
		return;
	}

	// get the ship class
	sip = &Ship_info[Multi_ts_select_ship_class];

	// starting line
	y_start = Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_Y_COORD];

	memset(str,0,100);

	// blit the ship class (name)
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Class",739));
	if(strlen((sip->alt_name[0]) ? sip->alt_name : sip->name)){
		gr_set_color_fast(&Color_bright);

		// Goober5000
		char temp[NAME_LENGTH];
		strcpy_s(temp, (sip->alt_name[0]) ? sip->alt_name : sip->name);
		end_string_at_first_hash_symbol(temp);

		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, temp);
	}
	y_start += 10;

	// blit the ship type
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Type",740));
	if((sip->type_str != NULL) && strlen(sip->type_str)){
		gr_set_color_fast(&Color_bright);
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, sip->type_str);
	}
	y_start += 10;

	// blit the ship length
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Length",741));
	if((sip->ship_length != NULL) && strlen(sip->ship_length)){
		gr_set_color_fast(&Color_bright);
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, sip->ship_length);
	}
	y_start += 10;

	// blit the max velocity
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Max Velocity",742));	
	sprintf(str, XSTR("%d m/s",743),(int)sip->max_vel.xyz.z);
	gr_set_color_fast(&Color_bright);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start,str);	
	y_start += 10;

	// blit the maneuverability
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Maneuverability",744));
	if((sip->maneuverability_str != NULL) && strlen(sip->maneuverability_str)){
		gr_set_color_fast(&Color_bright);
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, sip->maneuverability_str);
	}
	y_start += 10;

	// blit the armor
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Armor",745));
	if((sip->armor_str != NULL) && strlen(sip->armor_str)){
		gr_set_color_fast(&Color_bright);
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, sip->armor_str);
	}
	y_start += 10;

	// blit the gun mounts 
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Gun Mounts",746));
	if((sip->gun_mounts != NULL) && strlen(sip->gun_mounts)){
		gr_set_color_fast(&Color_bright);
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, sip->gun_mounts);
	}
	y_start += 10;

	// blit the missile banke
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Missile Banks",747));
	if((sip->missile_banks != NULL) && strlen(sip->missile_banks)){
		gr_set_color_fast(&Color_bright);
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, sip->missile_banks);
	}
	y_start += 10;

	// blit the manufacturer
	gr_set_color_fast(&Color_normal);
	gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, XSTR("Manufacturer",748));
	if((sip->manufacturer_str != NULL) && strlen(sip->manufacturer_str)){
		gr_set_color_fast(&Color_bright);
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD] + 150, y_start, sip->manufacturer_str);
	}
	y_start += 10;

	// blit the _short_ text description
	
	Assert(Multi_ts_ship_info_line_count < 3);
	gr_set_color_fast(&Color_normal);
	for(int idx=0;idx<Multi_ts_ship_info_line_count;idx++){
		gr_string(Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_X_COORD], y_start, Multi_ts_ship_info_lines[idx]);
		y_start += 10;
	}
	
}


// blit the status bar
void multi_ts_blit_status_bar()
{
	char text[50];
	int text_w;
	int blit = 0;

	// mode specific text
	switch(Multi_ts_status_bar_mode){
	case 0 :
		strcpy_s(text, XSTR("Ships/Weapons Locked",749));
		blit = 1;
		break;
	case 1 :
		strcpy_s(text, XSTR("Ships/Weapons Are Now Free",750));
		blit = 1;
		break;
	}

	// if we should be blitting
	if(blit){	
		gr_get_string_size(&text_w,NULL,text);
		gr_set_color_fast(&Color_bright_blue);
		gr_string(Multi_ts_status_coords[gr_screen.res][MULTI_TS_X_COORD] + ((Multi_ts_status_coords[gr_screen.res][MULTI_TS_W_COORD] - text_w)/2),Multi_ts_status_coords[gr_screen.res][MULTI_TS_Y_COORD],text);
	}
}

// assign the correct players to the correct slots
void multi_ts_init_players()
{
	int idx;

	// if i'm an observer, i have no ship
	if(Net_player->flags & NETINFO_FLAG_OBSERVER){
		Net_player->p_info.ship_index = -1;
	}

	// initialize all players and observer
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			if(MULTI_OBSERVER(Net_players[idx])){
				Net_players[idx].p_info.ship_index = -1;
			} else {
				Multi_ts_team[Net_players[idx].p_info.team].multi_ts_player[Net_players[idx].p_info.ship_index] = &Net_players[idx];
			}
		}
	}
}

// assign the correct objnums to the correct slots
void multi_ts_init_objnums()
{
	int idx,s_idx,team_index,slot_index;
	object *objp;

	// zero out the indices
	for(idx=0;idx<MULTI_TS_MAX_TVT_TEAMS;idx++){
		for(s_idx=0;s_idx<MULTI_TS_NUM_SHIP_SLOTS;s_idx++){
			Multi_ts_team[idx].multi_ts_objnum[s_idx] = -1;
		}		
	}

	// set all the objnums
	objp = GET_FIRST(&obj_used_list);
	while(objp != END_OF_LIST(&obj_used_list)){
		// if its a ship, get its slot index (if any)
		if(objp->type == OBJ_SHIP){
			multi_ts_get_team_and_slot(Ships[objp->instance].ship_name,&team_index,&slot_index);			
			if((slot_index != -1) && (team_index != -1)){
				Multi_ts_team[team_index].multi_ts_objnum[slot_index] = Ships[objp->instance].objnum;				
			}
		}

		objp = GET_NEXT(objp);
	}		
}

bool multi_ts_validate_ship(char *shipname, char *wingname) 
{
	int wing_number, wing_idx; 

	// check name isn't too short to be valid
	if (strlen(shipname) < (strlen (wingname) + 2) ) {		
		return false;
	}

	wing_idx = wing_lookup(wingname); 
	Assert (wing_idx >= 0 && wing_idx < MAX_WINGS); 
	wing_number = atoi(shipname+strlen(wingname)); 

	if (wing_number > 0 && wing_number <= Wings[wing_idx].wave_count) {
		return true; 
	}

	return false;
}

// get the proper team and slot index for the given ship name
void multi_ts_get_team_and_slot(char *ship_name,int *team_index,int *slot_index)
{
	int idx; 

	// set the return values to default values
	*team_index = -1;
	*slot_index = -1;

	// if we're in team vs. team mode
	if(Netgame.type_flags & NG_TYPE_TEAM){
		Assert(MAX_TVT_WINGS == MULTI_TS_MAX_TVT_TEAMS);
		for (idx = 0; idx < MAX_TVT_WINGS; idx++) {
			// get team (wing)
			if ( !strnicmp(ship_name, TVT_wing_names[idx], strlen(TVT_wing_names[idx])) && multi_ts_validate_ship(ship_name, TVT_wing_names[idx]) ) {				
				*team_index = idx;
				*slot_index = (ship_name[strlen(ship_name)-1] - '1');

				// just Assert(), if this is wrong then we're pretty much screwed either way
				Assert( (*slot_index >= 0) && (*slot_index < MAX_WSS_SLOTS) );
			}
		}
	} 
	// if we're _not_ in team vs. team mode
	else {
		int wing, ship;
		for (idx = 0; idx < MAX_STARTING_WINGS; idx++) {
			// get wing
			if ( !strnicmp(ship_name, Starting_wing_names[idx], strlen(Starting_wing_names[idx])) && multi_ts_validate_ship(ship_name, Starting_wing_names[idx]) ) {
				wing = idx;
				ship = (ship_name[strlen(ship_name)-1] - '1');

				// just Assert(), if this is wrong then we're pretty much screwed either way
				Assert( (ship >= 0) && (ship < MULTI_TS_NUM_SHIP_SLOTS_TEAM) );

				// team is 0, slot is the starting slot for all ships
				*team_index = 0;
				*slot_index = wing * MULTI_TS_NUM_SHIP_SLOTS_TEAM + ship;
			}
		}
	}
}

// function to return the shipname of the ship in the slot designated by the team and slot
// parameters
void multi_ts_get_shipname( char *ship_name, int team, int slot_index )
{
	if ( Netgame.type_flags & NG_TYPE_TEAM ) {
		Assert( (team >= 0) && (team < MULTI_TS_MAX_TVT_TEAMS) );
		sprintf(ship_name, "%s %d", TVT_wing_names[team], slot_index);
	} else {
		Assert( team == 0 );
		sprintf(ship_name, "%s %d", Starting_wing_names[slot_index / MULTI_TS_NUM_SHIP_SLOTS_TEAM], slot_index % MULTI_TS_NUM_SHIP_SLOTS_TEAM);
	}
}

// assign the correct flags to the correct slots
void multi_ts_init_flags()
{
	int idx,s_idx;

	// zero out the flags
	for(idx=0;idx<MULTI_TS_MAX_TVT_TEAMS;idx++){
		for(s_idx=0;s_idx<MULTI_TS_NUM_SHIP_SLOTS;s_idx++){
			Multi_ts_team[idx].multi_ts_flag[s_idx] = MULTI_TS_FLAG_NONE;			
		}
	}

	// in a team vs. team situation
	if(Netgame.type_flags & NG_TYPE_TEAM){
		for(idx=0;idx<MULTI_TS_MAX_TVT_TEAMS;idx++){
			for(s_idx=0;s_idx<MULTI_TS_NUM_SHIP_SLOTS_TEAM;s_idx++){
				// if the there is an objnum here but no ship class, we know its currently empty
				if((Multi_ts_team[idx].multi_ts_objnum[s_idx] != -1) && (Wss_slots_teams[idx][s_idx].ship_class == -1)){
					Multi_ts_team[idx].multi_ts_flag[s_idx] = MULTI_TS_FLAG_EMPTY;
				} else if((Multi_ts_team[idx].multi_ts_objnum[s_idx] != -1) && (Wss_slots_teams[idx][s_idx].ship_class != -1)){
					Multi_ts_team[idx].multi_ts_flag[s_idx] = Wss_slots_teams[idx][s_idx].ship_class;
				}
			}
		}
	}
	// in a non team vs. team situation
	else {
		for(idx=0;idx<MULTI_TS_NUM_SHIP_SLOTS;idx++){
			// if the there is an objnum here but no ship class, we know its currently empty
			if((Multi_ts_team[0].multi_ts_objnum[idx] != -1) && (Wss_slots[idx].ship_class == -1)){
				Multi_ts_team[0].multi_ts_flag[idx] = MULTI_TS_FLAG_EMPTY;
			} else if((Multi_ts_team[0].multi_ts_objnum[idx] != -1) && (Wss_slots[idx].ship_class != -1)){
				Multi_ts_team[0].multi_ts_flag[idx] = Wss_slots[idx].ship_class;
			}			
		}
	}
}

// handle an available ship scroll down button press
void multi_ts_avail_scroll_down()
{	
	if((Multi_ts_avail_count - Multi_ts_avail_start) > MULTI_TS_AVAIL_MAX_DISPLAY){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_ts_avail_start++;		
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// handle an available ship scroll up button press
void multi_ts_avail_scroll_up()
{
	if(Multi_ts_avail_start > 0){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_ts_avail_start--;		
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// handle all mouse events (clicking, dragging, and dropping)
void multi_ts_handle_mouse()
{
	int snazzy_region,snazzy_action;
	int region_type,region_index,region_empty;
	int mouse_x,mouse_y,ship_class;

	// get the mouse coords
	mouse_get_pos_unscaled(&mouse_x,&mouse_y);

	// do frame for the snazzy menu
	snazzy_region = snazzy_menu_do(Multi_ts_mask_data, Multi_ts_mask_w, Multi_ts_mask_h, Multi_ts_snazzy_regions, Multi_ts_region, &snazzy_action, 0);

	region_type = -1;
	region_index = -1;
	region_empty = 1;
	ship_class = -1;
	if(snazzy_region != -1){
		region_type = multi_ts_region_type(snazzy_region);
		Assert(region_type != -1);

		// determine what type of region the mouse is over and the appropriate index
		switch(region_type){
		case MULTI_TS_AVAIL_LIST:
			region_index = multi_ts_avail_index(snazzy_region);
			ship_class = multi_ts_get_avail_ship_class(region_index);

			if(ship_class == -1){
				region_empty = 1;
			} else {
				region_empty = (Ss_pool[ship_class] > 0) ? 0 : 1;
			}
			break;
		case MULTI_TS_SLOT_LIST:
			region_index = multi_ts_slot_index(snazzy_region);
			region_empty = (Multi_ts_team[Net_player->p_info.team].multi_ts_flag[region_index] >= 0) ? 0 : 1;
			if(!region_empty){
				ship_class = Wss_slots[region_index].ship_class;
			}
			break;
		case MULTI_TS_PLAYER_LIST:
			region_index = multi_ts_player_index(snazzy_region);
			region_empty = (Multi_ts_team[Net_player->p_info.team].multi_ts_player[region_index] != NULL) ? 0 : 1;
			break;
		}
	}	

	// maybe play a "highlight" sound
	switch(region_type){
	case MULTI_TS_PLAYER_LIST:
		if((Multi_ts_hotspot_index != region_index) && (region_index >= 0) && (Multi_ts_team[Net_player->p_info.team].multi_ts_player[region_index] != NULL)){
			gamesnd_play_iface(SND_USER_SELECT);			
		}
		break;
	}

	// set the current frame mouse hotspot vars
	Multi_ts_hotspot_type = region_type;
	Multi_ts_hotspot_index = region_index;

	// if we currently have clicked on something and have just released it
	if(!Multi_ts_carried_flag && Multi_ts_clicked_flag && !mouse_down(MOUSE_LEFT_BUTTON)){
		Multi_ts_clicked_flag = 0;
	}

	// if we're currently not carrying anything and the user has clicked
	if(!Multi_ts_carried_flag && !Multi_ts_clicked_flag && mouse_down(MOUSE_LEFT_BUTTON) && !region_empty){
		// set the "clicked" flag
		Multi_ts_clicked_flag = 1;

		// check to see if he clicked on a ship type and highlight if necessary
		switch(region_type){
		// selected a ship in the wing slots
		case MULTI_TS_SLOT_LIST:
			Multi_ts_select_type = MULTI_TS_SLOT_LIST;
			Multi_ts_select_index = region_index;
			multi_ts_select_ship();
			break;

		// selected a ship on the avail list
		case MULTI_TS_AVAIL_LIST:
			Multi_ts_select_type = MULTI_TS_AVAIL_LIST;
			Multi_ts_select_index = region_index;
			multi_ts_select_ship();
			break;
		
		// selected something else - unselect
		default :
			Multi_ts_select_type = -1;
			Multi_ts_select_index = -1;
			Multi_ts_select_ship_class = -1;
			break;
		}

		Multi_ts_clicked_x = mouse_x;
		Multi_ts_clicked_y = mouse_y;
	}

	// if we had something clicked and have started dragging it
	if(!Multi_ts_carried_flag && Multi_ts_clicked_flag && mouse_down(MOUSE_LEFT_BUTTON) && ((Multi_ts_clicked_x != mouse_x) || (Multi_ts_clicked_y != mouse_y))){
		// if this player is an observer, he shouldn't be able to do jack
		if(Net_player->flags & NETINFO_FLAG_OBSERVER){
			return;
		}

		// first we check for illegal conditions (any case where he cannot grab what he is attempting to grab)
		switch(region_type){
		case MULTI_TS_AVAIL_LIST :
			// if players are not yet locked, can't grab ships
			if(!Multi_ts_team[Net_player->p_info.team].multi_players_locked){
				return;
			}
			
			if(region_empty){
				return;
			}
			break;
		case MULTI_TS_SLOT_LIST:
			// if players are not yet locked, can't grab ships
			if(!Multi_ts_team[Net_player->p_info.team].multi_players_locked){
				return;
			}

			if(multi_ts_disabled_slot(region_index)){
				multi_ts_maybe_host_only_popup();			
				return;
			}
			if(Multi_ts_team[Net_player->p_info.team].multi_ts_flag[region_index] < 0){
				return;
			}
			break;
		case MULTI_TS_PLAYER_LIST:
			if(!multi_ts_can_grab_player(region_index)){
				return;
			}
			break;
		}
		
		
		Multi_ts_clicked_flag = 0;
		Multi_ts_carried_flag = 1;

		// set up the carried icon here
		Multi_ts_carried_from_type = region_type;
		Multi_ts_carried_from_index = region_index;
		Multi_ts_carried_ship_class = ship_class;
	}		

	// if we were carrying something but have dropped it
	if(Multi_ts_carried_flag && !mouse_down(MOUSE_LEFT_BUTTON)){
		Multi_ts_carried_flag = 0;
		Multi_ts_clicked_flag = 0;		

		// if we're not allowed to drop onto this slot
		if((region_type == MULTI_TS_SLOT_LIST) && multi_ts_disabled_slot(region_index)){
			multi_ts_maybe_host_only_popup();			
		}

		// if we're over some kind of valid region, apply		
		multi_ts_drop(Multi_ts_carried_from_type,Multi_ts_carried_from_index,region_type,region_index,Multi_ts_carried_ship_class);		
	}
}

// can the specified player perform the action he is attempting
int multi_ts_can_perform(int from_type,int from_index,int to_type,int to_index,int ship_class,int player_index)
{
	net_player *pl;
	int op_type;
	p_object *pobj;

	// get the appropriate player
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}

	// get the operation type
	op_type = multi_ts_get_dnd_type(from_type,from_index,to_type,to_index,player_index);

	// if either of the indices are bogus, then bail
	if((from_index == -1) || (to_index == -1)){
		return 0;
	}

	switch(op_type){
	case TS_GRAB_FROM_LIST:
		// if there are no more of this ship class, its no go
		if(Ss_pool_teams[pl->p_info.team][ship_class] <= 0){
			return 0;
		}

		// if he's not allowed to touch the wing slot
		if(multi_ts_disabled_slot(to_index,player_index)){
			return 0;
		}

		// if the slot he's trying to drop it on is "permanently" empty
		if(Multi_ts_team[pl->p_info.team].multi_ts_flag[to_index] == MULTI_TS_FLAG_NONE){
			return 0;
		}
		break;

	case TS_SWAP_LIST_SLOT:
		// if there are no more of this ship class, its no go
		if(Ss_pool_teams[pl->p_info.team][ship_class] <= 0){
			return 0;
		}

		// if he's not allowed to touch the wing slot
		if(multi_ts_disabled_slot(to_index,player_index)){
			return 0;
		}

		// if the slot we're trying to move to is invalid, then do nothing
		if(Multi_ts_team[pl->p_info.team].multi_ts_flag[to_index] == MULTI_TS_FLAG_NONE){
			return 0;
		}

		// if the slot he's trying to drop it on is "permanently" empty
		if(Multi_ts_team[pl->p_info.team].multi_ts_flag[to_index] == MULTI_TS_FLAG_NONE){
			return 0;
		}
		break;

	case TS_SWAP_SLOT_SLOT:
		// if he's not allowed to touch one of the slots, its no go
		if(multi_ts_disabled_slot(from_index,player_index) || multi_ts_disabled_slot(to_index,player_index)){
			return 0;
		}

		// if the slot we're taking from is invalid
		if(Multi_ts_team[pl->p_info.team].multi_ts_flag[to_index] == MULTI_TS_FLAG_NONE){
			return 0;
		}

		// if the slot he's trying to drop it on is "permanently" empty
		if(Multi_ts_team[pl->p_info.team].multi_ts_flag[to_index] == MULTI_TS_FLAG_NONE){
			return 0;
		}
		break;

	case TS_DUMP_TO_LIST:
		// if he's not allowed to be touching the slot to begin with, it no go
		if(multi_ts_disabled_slot(from_index,player_index)){
			return 0;
		}

		// if the slot we're trying to move to is invalid, then do nothing
		if(Multi_ts_team[pl->p_info.team].multi_ts_flag[to_index] == MULTI_TS_FLAG_NONE){
			return 0;
		}
		break;

	case TS_SWAP_PLAYER_PLAYER:
		// if his team is already locked, he cannot do this
		if(Multi_ts_team[pl->p_info.team].multi_players_locked){
			return 0;
		}

		// if there isn't a player at one of the positions
		if((Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] == NULL) || (Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] == NULL)){
			return 0;
		}

		// if this is not a player ship type object
		if(Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index] != -1){
			pobj = mission_parse_get_arrival_ship(Ships[Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].instance].ship_name);
			if((pobj == NULL) || !(pobj->flags & OF_PLAYER_SHIP)){
				return 0;
			}
		}		

		if(Netgame.type_flags & NG_TYPE_TEAM){
			// if he's not the team captain, he cannot do this
			if(!(pl->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				return 0;
			}
		} else {
			// if he's not the host, he cannot do this
			if(!(pl->flags & NETINFO_FLAG_GAME_HOST)){
				return 0;
			}
		}
		break;		

	case TS_MOVE_PLAYER:
		// if his team is already locked, he cannot do this
		if(Multi_ts_team[pl->p_info.team].multi_players_locked){
			return 0;
		}

		// if there isn't a player at the _from_
		if(Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] == NULL){
			return 0;
		}

		// if there is no ship at the _to_ location
		if(Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index] < 0){
			return 0;
		}

		// if this is not a player ship type object
		if(Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index] != -1){
			pobj = mission_parse_get_arrival_ship(Ships[Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].instance].ship_name);
			if((pobj == NULL) || !(pobj->flags & OF_PLAYER_SHIP)){
				return 0;
			}
		}

		if(Netgame.type_flags & NG_TYPE_TEAM){
			// if he's not the team captain, he cannot do this
			if(!(pl->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				return 0;
			}
		} else {
			// if he's not the host, he cannot do this
			if(!(pl->flags & NETINFO_FLAG_GAME_HOST)){
				return 0;
			}
		}
		break;

	default : 
		return 0;
		break;
	}
	
	return 1;
}

// determine the kind of drag and drop operation this is
int multi_ts_get_dnd_type(int from_type,int from_index,int to_type,int to_index,int player_index)
{	
	net_player *pl;

	// get the appropriate player
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}
	
	switch(from_type){
	// came from the ship avail list
	case MULTI_TS_AVAIL_LIST :	
		// do nothing
		if(to_type == MULTI_TS_AVAIL_LIST){
			return -1;
		}
		
		// if placing it on a slot
		if(to_type == MULTI_TS_SLOT_LIST){
			if(Wss_slots_teams[pl->p_info.team][to_index].ship_class == -1){				
				return TS_GRAB_FROM_LIST;				
			} else {								
				return TS_SWAP_LIST_SLOT;
			}
		}
		break;
	
	// came from the ship slots 
	case MULTI_TS_SLOT_LIST :
		if(to_type == MULTI_TS_SLOT_LIST){
			return TS_SWAP_SLOT_SLOT;
		}
		if(to_type == MULTI_TS_AVAIL_LIST){
			return TS_DUMP_TO_LIST;
		}
		break;	

	// came from the player lists
	case MULTI_TS_PLAYER_LIST :
		if(to_type == MULTI_TS_PLAYER_LIST){
			if(Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] == NULL){
				return TS_MOVE_PLAYER;
			} else {
				return TS_SWAP_PLAYER_PLAYER;
			}
		}
		break;
	}

	return -1;
}

void multi_ts_apply(int from_type,int from_index,int to_type,int to_index,int ship_class,int player_index)
{
	int size,update,sound;
	ubyte wss_data[MAX_PACKET_SIZE-20];	
	net_player *pl;
	
	// determine what kind of operation this is
	int type = multi_ts_get_dnd_type(from_type,from_index,to_type,to_index,player_index);	

	// get the proper net player
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}

	// set the proper pool pointers
	common_set_team_pointers(pl->p_info.team);

	sound = -1;
	switch(type){
	case TS_SWAP_SLOT_SLOT :
		nprintf(("Network","Apply swap slot slot %d %d\n",from_index,to_index));
		update = ss_swap_slot_slot(from_index,to_index,&sound);
		break;
	case TS_DUMP_TO_LIST	:
		nprintf(("Network","Apply dump to list %d %d\n",from_index,to_index));
		update = ss_dump_to_list(from_index,ship_class,&sound);
		break;
	case TS_SWAP_LIST_SLOT :
		nprintf(("Network","Apply swap list slot %d %d\n",from_index,to_index));
		update = ss_swap_list_slot(ship_class,to_index,&sound);
		break;
	case TS_GRAB_FROM_LIST :
		nprintf(("Network","Apply grab from list %d %d\n",from_index,to_index));		
		update = ss_grab_from_list(ship_class,to_index,&sound);
		break;
	case TS_SWAP_PLAYER_PLAYER :
		nprintf(("Network","Apply swap player player %d %d\n",from_index,to_index));
		update = multi_ts_swap_player_player(from_index,to_index,&sound,player_index);
		break;
	case TS_MOVE_PLAYER :
		nprintf(("Network","Apply move player %d %d\n",from_index,to_index));
		update = multi_ts_move_player(from_index,to_index,&sound,player_index);
	default :
		update = 0;
		break;
	}
	
	if(update){
		// if we're the host, send an update to all players
		if ( MULTIPLAYER_HOST ) {						
			// send the correct type of update
			if(type == TS_SWAP_PLAYER_PLAYER){
			} else {				
				size = store_wss_data(wss_data, MAX_PACKET_SIZE-20, sound, player_index);
				send_wss_update_packet(pl->p_info.team,wss_data, size);			

				// send a player slot update packet as well, so ship class information, etc is kept correct
				send_pslot_update_packet(pl->p_info.team,TS_CODE_PLAYER_UPDATE,-1);				
			}

			// if the player index == -1, it means the action was done locally - so play a sound
			if((player_index == -1) && (sound != -1)){
				gamesnd_play_iface(sound);
			}
		}

		// sync the interface screen up ourselves, if necessary
		if(Net_player->p_info.team == pl->p_info.team){
			multi_ts_sync_interface();	
		}

		// make sure all flags are set properly for all teams
		multi_ts_init_flags();
	}	

	// set the proper pool pointers
	common_set_team_pointers(Net_player->p_info.team);
}

// drop a carried icon 
void multi_ts_drop(int from_type,int from_index,int to_type,int to_index,int ship_class,int player_index)
{
	// if I'm the host, apply immediately
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
		// if this is a legal operation
		if(multi_ts_can_perform(from_type,from_index,to_type,to_index,ship_class,player_index)){
			multi_ts_apply(from_type,from_index,to_type,to_index,ship_class,player_index);
		} else {
			nprintf(("Network","Could not apply operation!\n"));
		}
	} 
	// otherwise send a request to the host
	else {
		send_wss_request_packet(Net_player->player_id, from_type, from_index, to_type, to_index, -1, ship_class, WSS_SHIP_SELECT);
	}
}

// swap two player positions
int multi_ts_swap_player_player(int from_index,int to_index,int *sound,int player_index)
{
	net_player *pl,*temp;

	// get the proper player pointer
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}

	// swap the players
	temp = Multi_ts_team[pl->p_info.team].multi_ts_player[to_index];
	Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] = Multi_ts_team[pl->p_info.team].multi_ts_player[from_index];
	Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] = temp;

	// update netplayer information if necessary
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] != NULL){
		Multi_ts_team[pl->p_info.team].multi_ts_player[from_index]->p_info.ship_index = from_index;
		Multi_ts_team[pl->p_info.team].multi_ts_player[from_index]->p_info.ship_class = Wss_slots_teams[pl->p_info.team][from_index].ship_class;

		multi_assign_player_ship(NET_PLAYER_INDEX(Multi_ts_team[pl->p_info.team].multi_ts_player[from_index]),&Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]],Wss_slots_teams[pl->p_info.team][from_index].ship_class);
	}
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] != NULL){
		Multi_ts_team[pl->p_info.team].multi_ts_player[to_index]->p_info.ship_index = to_index;
		Multi_ts_team[pl->p_info.team].multi_ts_player[to_index]->p_info.ship_class = Wss_slots_teams[pl->p_info.team][to_index].ship_class;

		multi_assign_player_ship(NET_PLAYER_INDEX(Multi_ts_team[pl->p_info.team].multi_ts_player[to_index]),&Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]],Wss_slots_teams[pl->p_info.team][to_index].ship_class);
	}

	// update ship flags
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].flags &= ~(OF_COULD_BE_PLAYER);
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].flags &= ~(OF_PLAYER_SHIP);
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] != NULL){		
		Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].flags |= OF_PLAYER_SHIP;
	}		
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]].flags &= ~(OF_COULD_BE_PLAYER);
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]].flags &= ~(OF_PLAYER_SHIP);
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] != NULL){		
		Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]].flags |= OF_PLAYER_SHIP;
	}		

	// recalcalate which slots are locked/unlocked, etc
	ss_recalc_multiplayer_slots();

	// send an update packet to all players
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
		send_pslot_update_packet(pl->p_info.team,TS_CODE_PLAYER_UPDATE,SND_ICON_DROP_ON_WING);
		gamesnd_play_iface(SND_ICON_DROP_ON_WING);
	}

	*sound = SND_ICON_DROP;

	return 1;
}

// move a player
int multi_ts_move_player(int from_index,int to_index,int *sound,int player_index)
{
	net_player *pl;

	// get the proper player pointer
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}

	// swap the players	
	Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] = Multi_ts_team[pl->p_info.team].multi_ts_player[from_index];
	Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] = NULL;

	// update netplayer information if necessary
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] != NULL){
		Multi_ts_team[pl->p_info.team].multi_ts_player[from_index]->p_info.ship_index = from_index;
		Multi_ts_team[pl->p_info.team].multi_ts_player[from_index]->p_info.ship_class = Wss_slots_teams[pl->p_info.team][from_index].ship_class;

		multi_assign_player_ship(NET_PLAYER_INDEX(Multi_ts_team[pl->p_info.team].multi_ts_player[from_index]),&Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]],Wss_slots_teams[pl->p_info.team][from_index].ship_class);
	}
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] != NULL){
		Multi_ts_team[pl->p_info.team].multi_ts_player[to_index]->p_info.ship_index = to_index;
		Multi_ts_team[pl->p_info.team].multi_ts_player[to_index]->p_info.ship_class = Wss_slots_teams[pl->p_info.team][to_index].ship_class;

		multi_assign_player_ship(NET_PLAYER_INDEX(Multi_ts_team[pl->p_info.team].multi_ts_player[to_index]),&Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]],Wss_slots_teams[pl->p_info.team][to_index].ship_class);
	}

	// update ship flags
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].flags &= ~(OF_COULD_BE_PLAYER);
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].flags &= ~(OF_PLAYER_SHIP);
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[to_index] != NULL){		
		Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[to_index]].flags |= OF_PLAYER_SHIP;
	}		
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]].flags &= ~(OF_COULD_BE_PLAYER);
	Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]].flags &= ~(OF_PLAYER_SHIP);
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[from_index] != NULL){		
		Objects[Multi_ts_team[pl->p_info.team].multi_ts_objnum[from_index]].flags |= OF_PLAYER_SHIP;
	}		

	// recalcalate which slots are locked/unlocked, etc
	ss_recalc_multiplayer_slots();

	// send an update packet to all players
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
		send_pslot_update_packet(pl->p_info.team,TS_CODE_PLAYER_UPDATE,SND_ICON_DROP_ON_WING);
		gamesnd_play_iface(SND_ICON_DROP_ON_WING);
	}

	*sound = SND_ICON_DROP;

	return 1;
}

// get the ship class of the current index in the avail list or -1 if none exists
int multi_ts_get_avail_ship_class(int index)
{
	int ship_count,class_index;

	ship_count = index + Multi_ts_avail_start;
	class_index = 0;
	while((ship_count >= 0) && (class_index < MAX_SHIP_CLASSES)){
		if(Ss_pool[class_index] > 0){
			ship_count--;
		}

		if(ship_count >= 0){
			class_index++;
		}
	}

	if(ship_count < 0){
		return class_index;
	}

	return -1;
}

// blit the currently carried icon (if any)
void multi_ts_blit_carried_icon()
{
	int x,y;
	int offset_x,offset_y,callsign_w;
	char callsign[CALLSIGN_LEN+2];

	// if we're not carrying anything, then return
	if(!Multi_ts_carried_flag){
		return;
	}	

	// get the mouse position
	mouse_get_pos_unscaled(&x,&y);

	// if we're carrying an icon of some kind
	switch(Multi_ts_carried_from_type){
	case MULTI_TS_SLOT_LIST:	
		offset_x = Multi_ts_slot_icon_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_X_COORD] - Multi_ts_clicked_x;
		offset_y = Multi_ts_slot_icon_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_Y_COORD] - Multi_ts_clicked_y;

		// blit the icon		
		ss_blit_ship_icon(x + offset_x,y + offset_y,Multi_ts_carried_ship_class,0);
		break;
	case MULTI_TS_AVAIL_LIST:
		offset_x = Multi_ts_avail_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_X_COORD] - Multi_ts_clicked_x;
		offset_y = Multi_ts_avail_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_Y_COORD] - Multi_ts_clicked_y;

		// blit the icon		
		ss_blit_ship_icon(x + offset_x,y + offset_y,Multi_ts_carried_ship_class,0);
		break;
	case MULTI_TS_PLAYER_LIST:
		// get the final length of the string so we can calculate a valid offset
		strcpy_s(callsign,Multi_ts_team[Net_player->p_info.team].multi_ts_player[Multi_ts_carried_from_index]->m_player->callsign);
		gr_force_fit_string(callsign,CALLSIGN_LEN,Multi_ts_slot_text_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_W_COORD]);						
		gr_get_string_size(&callsign_w,NULL,callsign);

		// calculate the offsets
		offset_x = (Multi_ts_slot_text_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_X_COORD] - Multi_ts_clicked_x) + ((Multi_ts_slot_text_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_W_COORD] - callsign_w)/2);
		offset_y = Multi_ts_slot_text_coords[Multi_ts_carried_from_index][gr_screen.res][MULTI_TS_Y_COORD] - Multi_ts_clicked_y;

		gr_set_color_fast(&Color_normal);
		gr_string(x + offset_x,y + offset_y,Multi_ts_team[Net_player->p_info.team].multi_ts_player[Multi_ts_carried_from_index]->m_player->callsign);
		break;
	default : 
		break;			
	}
}

// if the (console) player is allowed to grab a player slot at this point
int multi_ts_can_grab_player(int slot_index, int player_index)
{
	net_player *pl;

	// get a pointe rto the proper net player
	if(player_index == -1){
		pl = Net_player;
	} else {
		pl = &Net_players[player_index];
	}

	// if the players are locked in any case, he annot grab it
	if(Multi_ts_team[pl->p_info.team].multi_players_locked){
		return 0;
	}

	if(Netgame.type_flags & NG_TYPE_TEAM){
		// if he's not the team captain, he cannot do this
		if(!(pl->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
			return 0;
		}
	} else {
		// if he's not the host, he cannot do this
		if(!(pl->flags & NETINFO_FLAG_GAME_HOST)){
			return 0;
		}
	}

	// if the slot is empty
	if(Multi_ts_team[pl->p_info.team].multi_ts_player[slot_index] == NULL){
		return 0;
	}

	return 1;
}

// get the team # of the given ship
int multi_ts_get_team(char *ship_name)
{
	int idx;//,s_idx;

	// lookup through all team ship names
	Assert(MAX_TVT_WINGS == MULTI_TS_MAX_TVT_TEAMS);
	for(idx=0;idx<MAX_TVT_WINGS;idx++)
	{
		if (!strnicmp(ship_name, TVT_wing_names[idx], strlen(TVT_wing_names[idx])))
			return idx;
	}

	// always on team 0 if not found otherwise
	return 0;
}

// return the bitmap index into the ships icon array (in ship select) which should be displayed for the given slot
int multi_ts_avail_bmap_num(int slot_index)
{
	// if this slot has been highlighted for informational purposes
	if((Multi_ts_select_type == MULTI_TS_AVAIL_LIST) && (Multi_ts_select_index == slot_index)){
		return ICON_FRAME_SELECTED;
	}

	// if its otherwise being lit by the mouse
	if((Multi_ts_hotspot_type == MULTI_TS_AVAIL_LIST) && (Multi_ts_hotspot_index == slot_index)){
		return ICON_FRAME_HOT;
	}	

	return ICON_FRAME_NORMAL;
}

// return the bitmap index into the ships icon array (in ship select) which should be displayed for the given slot
int multi_ts_slot_bmap_num(int slot_index)
{	
	// special case - slot is disabled, its my ship and the host hasn't locked the ships yet
	if(multi_ts_disabled_high_slot(slot_index)){
		return ICON_FRAME_DISABLED_HIGH;
	}	
	
	// if this slot is disabled for us, then show it as such	
	if(multi_ts_disabled_slot(slot_index)){
		return ICON_FRAME_DISABLED;
	}

	// if this slot has been highlighted for informational purposes
	if((Multi_ts_select_type == MULTI_TS_SLOT_LIST) && (Multi_ts_select_index == slot_index)){
		return ICON_FRAME_SELECTED;
	}

	// if this is our ship, then highlight it as so
	if(Net_player->p_info.ship_index == slot_index){
		return ICON_FRAME_PLAYER;
	}

	// if its otherwise being lit by the mouse
	if((Multi_ts_hotspot_type == MULTI_TS_SLOT_LIST) && (Multi_ts_hotspot_index == slot_index)){
		return ICON_FRAME_HOT;
	}	

	// normal unhighlighted frame
	return ICON_FRAME_NORMAL;
}

// select the given slot and setup any information, etc
void multi_ts_select_ship()
{
	int n_lines;
	int n_chars[MAX_BRIEF_LINES];
	char ship_desc[1000];
	char *p_str[MAX_BRIEF_LINES];
	char *token;
	
	
	// blast all current text
	memset(Multi_ts_ship_info_lines,0,MULTI_TS_SHIP_INFO_MAX_TEXT);
	memset(Multi_ts_ship_info_text,0,MULTI_TS_SHIP_INFO_MAX_TEXT);

	// get the selected ship class
	Assert(Multi_ts_select_index >= 0);
	Multi_ts_select_ship_class = -1;
	switch(Multi_ts_select_type){
	case MULTI_TS_SLOT_LIST:
		Multi_ts_select_ship_class = Wss_slots[Multi_ts_select_index].ship_class;
		break;
	case MULTI_TS_AVAIL_LIST:
		Multi_ts_select_ship_class = multi_ts_get_avail_ship_class(Multi_ts_select_index);
		// if he has selected an empty slot, don't do anything
		if(Multi_ts_select_ship_class < 0){
			return;
		}
		break;
	default : 
		// should always have one of the 2 above types selected
		Int3();
		break;
	}
	
	// split the text info up	
	
	Assert(Multi_ts_select_ship_class >= 0);
//	Assert((Ship_info[Multi_ts_select_ship_class].desc != NULL) && strlen(Ship_info[Multi_ts_select_ship_class].desc));
	if (Ship_info[Multi_ts_select_ship_class].desc != NULL)
	{

		// strip out newlines
		memset(ship_desc,0,1000);
		strcpy_s(ship_desc,Ship_info[Multi_ts_select_ship_class].desc);
		token = strtok(ship_desc,"\n");
		if(token != NULL){
			strcpy_s(Multi_ts_ship_info_text,token);
			while(token != NULL){
				token = strtok(NULL,"\n");
				if(token != NULL){
					strcat_s(Multi_ts_ship_info_text," ");
					strcat_s(Multi_ts_ship_info_text,token);
				}
			}
		}
	
		if(Multi_ts_ship_info_text[0] != '\0'){
			// split the string into multiple lines
			n_lines = split_str(Multi_ts_ship_info_text, Multi_ts_ship_info_coords[gr_screen.res][MULTI_TS_W_COORD], n_chars, p_str, MULTI_TS_SHIP_INFO_MAX_LINES, 0);	

			// copy the split up lines into the text lines array
			for (int idx = 0;idx<n_lines;idx++ ) {
				Assert(n_chars[idx] < MULTI_TS_SHIP_INFO_MAX_LINE_LEN);
				strncpy(Multi_ts_ship_info_lines[idx], p_str[idx], n_chars[idx]);
				Multi_ts_ship_info_lines[idx][n_chars[idx]] = 0;
				drop_leading_white_space(Multi_ts_ship_info_lines[idx]);		
			}

			// get the line count
			Multi_ts_ship_info_line_count = n_lines;
		} else {
			// set the line count to 
			Multi_ts_ship_info_line_count = 0;
		}
	}	
}

// handle all details when the commit button is pressed (including possibly reporting errors/popups)
void multi_ts_commit_pressed()
{					
	// if my team's slots are still not "locked", we cannot commit unless we're the only player in the game
	if(!Multi_ts_team[Net_player->p_info.team].multi_players_locked){
		if(multi_num_players() != 1){
			popup(PF_USE_AFFIRMATIVE_ICON | PF_BODY_BIG,1,POPUP_OK, XSTR("Players have not yet been assigned to their ships",751));
			return;
		} else {
			Multi_ts_team[Net_player->p_info.team].multi_players_locked = 1;
		}
	}

	// check to see if its not ok for this player to commit
	switch(multi_ts_ok_to_commit()){
	// yes, it _is_ ok to commit
	case 0:
		extern void commit_pressed();
		commit_pressed();
		break;

	// player has not assigned all necessary ships
	case 1: 	
		gamesnd_play_iface(SND_GENERAL_FAIL);
		popup(PF_USE_AFFIRMATIVE_ICON | PF_BODY_BIG,1,POPUP_OK, XSTR("You have not yet assigned all necessary ships",752));
		break;
	
	// there are ships without primary weapons
	case 2: 
		gamesnd_play_iface(SND_GENERAL_FAIL);
		popup(PF_USE_AFFIRMATIVE_ICON | PF_BODY_BIG,1,POPUP_OK, XSTR("There are ships without primary weapons!",753));
		break;

	// there are ships without secondary weapons
	case 3: 
		gamesnd_play_iface(SND_GENERAL_FAIL);
		popup(PF_USE_AFFIRMATIVE_ICON | PF_BODY_BIG,1,POPUP_OK, XSTR("There are ships without secondary weapons!",754));
		break;
	}
}

// is it ok for this player to commit 
int multi_ts_ok_to_commit()
{
	int idx,s_idx;
	int primary_ok,secondary_ok;	

	// if this player is an observer, he can always commit
	if(Net_player->flags & NETINFO_FLAG_OBSERVER){
		return 0;
	}
	
	for(idx=0;idx<MULTI_TS_NUM_SHIP_SLOTS;idx++){
		// if this is a player slot this player can modify and it is empty, then he cannot continue
		// implies there is never an object in this slot
		if((Multi_ts_team[Net_player->p_info.team].multi_ts_objnum[idx] != -1) &&							
			// implies player can't touch this slot anyway			
			!multi_ts_disabled_slot(idx) &&								
			// implies that there should be a player ship here but there isn't
			((Multi_ts_team[Net_player->p_info.team].multi_ts_player[idx] != NULL) && (Multi_ts_team[Net_player->p_info.team].multi_ts_flag[idx] == MULTI_TS_FLAG_EMPTY)) ){
			return 1;
		}

		// if the ship in this slot has a ship which can be a player but has 0 primary or secondary weapons, then he cannot continue
		if( (Multi_ts_team[Net_player->p_info.team].multi_ts_objnum[idx] != -1) && 
			((Objects[Multi_ts_team[Net_player->p_info.team].multi_ts_objnum[idx]].flags & OF_COULD_BE_PLAYER) ||
			 (Objects[Multi_ts_team[Net_player->p_info.team].multi_ts_objnum[idx]].flags & OF_PLAYER_SHIP)) &&
			 !multi_ts_disabled_slot(idx)){
			
			primary_ok = 0;
			secondary_ok = 0;
			// go through all weapons in the list
			for(s_idx=0;s_idx<MAX_SHIP_WEAPONS;s_idx++){
				// if this slot has a weapon with a greater than 0 count, check
				if((Wss_slots_teams[Net_player->p_info.team][idx].wep[s_idx] >= 0) && (Wss_slots_teams[Net_player->p_info.team][idx].wep_count[s_idx] > 0)){
					switch(Weapon_info[Wss_slots_teams[Net_player->p_info.team][idx].wep[s_idx]].subtype){				
					case WP_LASER:
						primary_ok = 1;
						break;

					case WP_MISSILE:
						secondary_ok = 1;
						break;

					default :
						Int3();
					}
				}

				// if we've got both primary and secondary weapons
				if(primary_ok && secondary_ok){
					break;
				}
			}

			// if the ship doesn't have primary weapons
			if (!primary_ok && !(The_mission.ai_profile->flags & AIPF_MULTI_ALLOW_EMPTY_PRIMARIES)) {
				return 2;
			} 

			// if the ship doesn't have secondary weapons
			if (!secondary_ok && !(The_mission.ai_profile->flags & AIPF_MULTI_ALLOW_EMPTY_SECONDARIES)) {
				return 3;
			}
		}
	}
			
	return 0;
}

// check to see that no illegal ship settings have occurred
void multi_ts_check_errors()
{
	/*
	int idx;
	ship *shipp;

	for(idx=0;idx<MULTI_TS_NUM_SHIP_SLOTS;idx++){
		if(Multi_ts_team[0].multi_ts_objnum[idx] == -1){
			continue;
		}

		shipp = &Ships[Objects[Multi_ts_team[0].multi_ts_objnum[idx]].instance];
		Assert((shipp->weapons.current_primary_bank != -1) && (shipp->weapons.current_secondary_bank != -1));
	}
	*/
}


// ------------------------------------------------------------------------------------------------------
// TEAM SELECT PACKET HANDLERS
//

// send a player slot position update
void send_pslot_update_packet(int team,int code,int sound)
{
	ubyte data[MAX_PACKET_SIZE],stop,val;
	short s_sound;
	int idx;
	int packet_size = 0;
	int i_tmp;

	// build the header and add the data
	BUILD_HEADER(SLOT_UPDATE);	

	// add the opcode
	val = (ubyte)code;
	ADD_DATA(val);

	// add the team 
	val = (ubyte)team;
	ADD_DATA(val);

	// add the sound to play
	s_sound = (short)sound;
	ADD_SHORT(s_sound);
	
	// add data based upon the packet code
	switch(code){
	case TS_CODE_LOCK_TEAM:
		// don't have to do anything
		break;
	case TS_CODE_PLAYER_UPDATE:
		// only the host should ever be doing this
		Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);
			
		// add individual slot data
		for(idx=0;idx<MAX_WSS_SLOTS;idx++){
			if(Multi_ts_team[team].multi_ts_flag[idx] != MULTI_TS_FLAG_NONE){
				// add a stop byte
				stop = 0x0;
				ADD_DATA(stop);

				// add the slot #
				val = (ubyte)idx;
				ADD_DATA(val);

				// add the ship class
				val = (ubyte)Wss_slots_teams[team][idx].ship_class;
				ADD_DATA(val);

				// add the objnum we're working with
				i_tmp = Multi_ts_team[team].multi_ts_objnum[idx];
				ADD_INT(i_tmp);

				// add a byte indicating if a player is here or not
				if(Multi_ts_team[team].multi_ts_player[idx] == NULL){
					val = 0;
				} else {
					val = 1;
				} 
				ADD_DATA(val);

				// if there's a player, add his address
				if(val){
					ADD_SHORT(Multi_ts_team[team].multi_ts_player[idx]->player_id);

					// should also update his p_info settings locally
					Multi_ts_team[team].multi_ts_player[idx]->p_info.ship_class = Wss_slots_teams[team][idx].ship_class;
					Multi_ts_team[team].multi_ts_player[idx]->p_info.ship_index = idx;
				}
				
				// add a byte indicating what object flag should be set (0 == ~(OF_COULD_BE_PLAYER | OF_PLAYER_SHIP), 1 == player ship, 2 == could be player ship)				
				if(Objects[Multi_ts_team[team].multi_ts_objnum[idx]].flags & OF_COULD_BE_PLAYER){
					val = 2;
				} else if(Objects[Multi_ts_team[team].multi_ts_objnum[idx]].flags & OF_PLAYER_SHIP){
					val = 1;
				} else {
					val = 0;
				}
				ADD_DATA(val);
			}
		}
		// add a final stop byte
		val = 0xff;
		ADD_DATA(val);
		break;
	default :
		Int3();
		break;
	}
	
	// send the packet to the standalone	
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER) {
		multi_io_send_to_all_reliable(data, packet_size);		
	} else {
		multi_io_send_reliable(Net_player, data, packet_size);
	}
}

// process a player slot position update
void process_pslot_update_packet(ubyte *data, header *hinfo)
{
	int offset = HEADER_LENGTH;
	int my_index;
	int player_index,idx,team,code,objnum;
	short sound;
	short player_id;
	ubyte stop,val,slot_num,ship_class;

	my_index = Net_player->p_info.ship_index;

	// if we're the standalone, then we should be routing this data to all the other clients
	player_index = -1;
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// fill in the address information of where this came from		
		player_index = find_player_id(hinfo->id);
		Assert(player_index != -1);		
	}

	// get the opcode
	GET_DATA(val);
	code = (int)val;

	// get the team
	GET_DATA(val);
	team = (int)val;

	// get the sound to play
	GET_SHORT(sound);

	// process the different opcodes
	switch(code){
	case TS_CODE_LOCK_TEAM:
		// lock the team
		Multi_ts_team[team].multi_players_locked = 1;

		// if this was my team, sync stuff up
		if((team == Net_player->p_info.team) && !(Game_mode & GM_STANDALONE_SERVER)){
			multi_ts_set_status_bar_mode(1);
			multi_ts_sync_interface();
			ss_recalc_multiplayer_slots();
		}

		// if this is the standalone server, we need to re-route the packet here and there
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			// in team vs team mode, only a team captain should ever be sending this
			if(Netgame.type_flags & NG_TYPE_TEAM){
				Assert(Net_players[player_index].flags & NETINFO_FLAG_TEAM_CAPTAIN);
			}
			// in any other mode, it better be coming from the game host
			else {
				Assert(Net_players[player_index].flags & NETINFO_FLAG_GAME_HOST);
			}

			// re-route to all other players
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && (&Net_players[idx] != Net_player) && (&Net_players[idx] != &Net_players[player_index]) ){					
					multi_io_send_reliable(&Net_players[idx], data, offset);
				}
			}
		}
		break;
	case TS_CODE_PLAYER_UPDATE:			
		// get the first stop byte
		GET_DATA(stop);
		while(stop != 0xff){
			// get the slot #
			GET_DATA(slot_num);

			// get the ship class
			GET_DATA(ship_class);

			// get the objnum
			GET_INT(objnum);
	
			// flag indicating if a player is in this slot
			GET_DATA(val);
			if(val){
				// look the player up
				GET_SHORT(player_id);
				player_index = find_player_id(player_id);
			
				// if we couldn't find him
				if(player_index == -1){
					nprintf(("Network","Couldn't find player for pslot update!\n"));
					Multi_ts_team[team].multi_ts_player[slot_num] = NULL;
				} 
				// if we found him, assign him to this ship
				else {
					Net_players[player_index].p_info.ship_class = (int)ship_class;
					Net_players[player_index].p_info.ship_index = (int)slot_num;
					multi_assign_player_ship(player_index,&Objects[objnum],(int)ship_class);				

					// ui stuff
					Multi_ts_team[team].multi_ts_player[slot_num] = &Net_players[player_index];

					// if this was me and my ship index changed, update the weapon select screen
					if(my_index != Net_player->p_info.ship_index){
						wl_reset_selected_slot();

						my_index = Net_player->p_info.ship_index;
					}
				}
			} else {
				Multi_ts_team[team].multi_ts_player[slot_num] = NULL;
			}

			// get the ship flag byte
			GET_DATA(val);
			Objects[objnum].flags &= ~(OF_PLAYER_SHIP | OF_COULD_BE_PLAYER);
			switch(val){
			case 1 :
				Objects[objnum].flags |= OF_PLAYER_SHIP;
				break;
			case 2 :
				obj_set_flags( &Objects[objnum], Objects[objnum].flags | OF_COULD_BE_PLAYER );
				break;
			}

			// get the next stop byte
			GET_DATA(stop);
		}
		// if we have a sound we're supposed to play
		if((sound != -1) && !(Game_mode & GM_STANDALONE_SERVER) && (gameseq_get_state() == GS_STATE_TEAM_SELECT)){
			gamesnd_play_iface(sound);
		}

		// if i'm the standalone server, I should rebroadcast this packet
		if(Game_mode & GM_STANDALONE_SERVER){
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_HOST(Net_players[idx]) && (Net_player != &Net_players[idx])){					
					multi_io_send_reliable(&Net_players[idx], data, offset);
				}
			}
		}
		break;
	}	
	PACKET_SET_SIZE();				

	// recalculate stuff
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		ss_recalc_multiplayer_slots();
	}
}
