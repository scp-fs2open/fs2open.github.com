/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include <limits.h>		// this is need even when not building debug!!

#include "gamesnd/eventmusic.h"
#include "io/key.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "missionui/missionweaponchoice.h"
#include "missionui/missionbrief.h"
#include "io/timer.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "palman/palman.h"
#include "io/mouse.h"
#include "gamehelp/contexthelp.h"
#include "cmdline/cmdline.h"
#include "globalincs/linklist.h"
#include "popup/popup.h"
#include "hud/hudwingmanstatus.h"
#include "ui/uidefs.h"
#include "anim/animplay.h"
#include "ship/ship.h"
#include "render/3d.h"
#include "lighting/lighting.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multiteamselect.h"
#include "network/multi_endgame.h"
#include "missionui/chatbox.h"
#include "cutscene/movie.h"
#include "cutscene/cutscenes.h"
#include "parse/sexp.h"
#include "graphics/2d.h"
#include "graphics/gropenglshader.h"

//////////////////////////////////////////////////////////////////
// Game Globals
//////////////////////////////////////////////////////////////////

int Common_select_inited = 0;

// Dependent on when mouse button goes up
int Drop_icon_mflag, Drop_on_wing_mflag, Brief_mouse_up_flag;

int Mouse_down_last_frame = 0;

// Timers used to flash buttons after timeouts
#define MSC_FLASH_AFTER_TIME	60000		//	time before flashing a button
#define MSC_FLASH_INTERVAL		200		// time between flashes
int Flash_timer;								//	timestamp used to start flashing
int Flash_toggle;								// timestamp used to toggle flashing
int Flash_bright;								// state of button to flash

//////////////////////////////////////////////////////////////////
// Global to modulde
//////////////////////////////////////////////////////////////////
int Current_screen;
int Next_screen;
static int InterfacePaletteBitmap = -1; // PCX file that holds the interface palette
color Icon_colors[NUM_ICON_FRAMES];
shader Icon_shaders[NUM_ICON_FRAMES];

loadout_data Player_loadout;	// what the ship and weapon loadout is... used since we want to use the 
								// same loadout if the mission is played again

//wss_unit	Wss_slots[MAX_WSS_SLOTS];				// slot data struct
//int		Wl_pool[MAX_WEAPON_TYPES];				// weapon pool 
//int		Ss_pool[MAX_SHIP_CLASSES];				// ship pool
//int		Wss_num_wings;								// number of player wings

wss_unit	Wss_slots_teams[MAX_TVT_TEAMS][MAX_WSS_SLOTS];
int		Wl_pool_teams[MAX_TVT_TEAMS][MAX_WEAPON_TYPES];
int		Ss_pool_teams[MAX_TVT_TEAMS][MAX_SHIP_CLASSES];
int		Wss_num_wings_teams[MAX_TVT_TEAMS];

wss_unit	*Wss_slots = NULL;
int		*Wl_pool = NULL;
int		*Ss_pool = NULL;
int		Wss_num_wings;

//////////////////////////////////////////////////////////////////
// Externs
//////////////////////////////////////////////////////////////////
extern void ss_set_team_pointers(int team);
extern void ss_reset_team_pointers();
extern void wl_set_team_pointers(int team);
extern void wl_reset_team_pointers();
extern int anim_timer_start;

//////////////////////////////////////////////////////////////////
// UI 
//////////////////////////////////////////////////////////////////
UI_WINDOW	*Active_ui_window;

brief_common_buttons Common_buttons[3][GR_NUM_RESOLUTIONS][NUM_COMMON_BUTTONS] = {	
	{	// UGH
		{ // GR_640
			brief_common_buttons("CB_00",			7,		3,		37,	7,		0),
			brief_common_buttons("CB_01",			7,		19,	37,	23,	1),
			brief_common_buttons("CB_02",			7,		35,	37,	39,	2),
			brief_common_buttons("CB_05",			571,	425,	572,	413,	5),
			brief_common_buttons("CB_06",			533,	425,	500,	440,	6),
			brief_common_buttons("CB_07",			533,	455,	479,	464,	7),			
		}, 
		{ // GR_1024			
			brief_common_buttons("2_CB_00",		12,	5,		59,	12,	0),
			brief_common_buttons("2_CB_01",		12,	31,	59,	37,	1),
			brief_common_buttons("2_CB_02",		12,	56,	59,	62,	2),
			brief_common_buttons("2_CB_05",		914,	681,	937,	671,	5),
			brief_common_buttons("2_CB_06",		854,	681,	822,	704,	6),
			brief_common_buttons("2_CB_07",		854,	724,	800,	743,	7),			
		}
	},	
	{	// UGH
		{ // GR_640
			brief_common_buttons("CB_00",			7,		3,		37,	7,		0),
			brief_common_buttons("CB_01",			7,		19,	37,	23,	1),
			brief_common_buttons("CB_02",			7,		35,	37,	39,	2),
			brief_common_buttons("CB_05",			571,	425,	572,	413,	5),
			brief_common_buttons("CB_06",			533,	425,	500,	440,	6),
			brief_common_buttons("CB_07",			533,	455,	479,	464,	7),			
		}, 
		{ // GR_1024			
			brief_common_buttons("2_CB_00",		12,	5,		59,	12,	0),
			brief_common_buttons("2_CB_01",		12,	31,	59,	37,	1),
			brief_common_buttons("2_CB_02",		12,	56,	59,	62,	2),
			brief_common_buttons("2_CB_05",		914,	681,	937,	671,	5),
			brief_common_buttons("2_CB_06",		854,	681,	822,	704,	6),
			brief_common_buttons("2_CB_07",		854,	724,	800,	743,	7),			
		}
	},	
	{	// UGH
		{ // GR_640
			brief_common_buttons("CB_00",			7,		3,		37,	7,		0),
			brief_common_buttons("CB_01",			7,		19,	37,	23,	1),
			brief_common_buttons("CB_02",			7,		35,	37,	39,	2),
			brief_common_buttons("CB_05",			571,	425,	572,	413,	5),
			brief_common_buttons("CB_06",			533,	425,	500,	440,	6),
			brief_common_buttons("CB_07",			533,	455,	479,	464,	7),			
		}, 
		{ // GR_1024			
			brief_common_buttons("2_CB_00",		12,	5,		59,	12,	0),
			brief_common_buttons("2_CB_01",		12,	31,	59,	37,	1),
			brief_common_buttons("2_CB_02",		12,	56,	59,	62,	2),
			brief_common_buttons("2_CB_05",		914,	681,	937,	671,	5),
			brief_common_buttons("2_CB_06",		854,	681,	822,	704,	6),
			brief_common_buttons("2_CB_07",		854,	724,	800,	743,	7),			
		}
	}
};

#define COMMON_BRIEFING_BUTTON					0
#define COMMON_SS_BUTTON							1
#define COMMON_WEAPON_BUTTON						2
#define COMMON_COMMIT_BUTTON						3
#define COMMON_HELP_BUTTON							4
#define COMMON_OPTIONS_BUTTON						5

int Background_playing;			// Flag to indicate background animation is playing
static anim *Background_anim;	// Ids for the anim data that is loaded

// value for which Team_data entry to use
int	Common_team;

// Ids for the instance of the anim that is playing
//static anim_instance *Background_anim_instance;

int Wing_slot_empty_bitmap;
int Wing_slot_disabled_bitmap;

// prototypes
int wss_slots_all_empty();

// Display the no ships selected error
void common_show_no_ship_error()
{
	popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR( "At least one ship must be selected before proceeding to weapons loadout", 460));
}

// Check the status of the buttons common to the loadout screens
void common_check_buttons()
{
	int			i;
	UI_BUTTON	*b;

	for ( i = 0; i < NUM_COMMON_BUTTONS; i++ ) {
		b = &Common_buttons[Current_screen-1][gr_screen.res][i].button;
		if ( b->pressed() ) {
			
			common_button_do(i);
		}
	}

/*
	// AL 11-23-97: let a joystick button press commit
	if ( joy_down_count(0) || joy_down_count(1) ) {
		Commit_pressed = 1;
	}
*/

}

// -------------------------------------------------------------------
// common_redraw_pressed_buttons()
//
// Redraw any common buttons that are pressed down.  This function is needed
// since we sometimes need to draw pressed buttons last to ensure the entire
// button gets drawn (and not overlapped by other buttons)
//
void common_redraw_pressed_buttons()
{
	int			i;
	UI_BUTTON	*b;

	for ( i = 0; i < NUM_COMMON_BUTTONS; i++ ) {
		b = &Common_buttons[Current_screen-1][gr_screen.res][i].button;
		if ( b->button_down() ) {
			b->draw_forced(2);
		}
	}
}

void common_buttons_maybe_reload(UI_WINDOW *ui_window)
{
	UI_BUTTON	*b;
	int			i;

	for ( i = 0; i < NUM_COMMON_BUTTONS; i++ ) {
		b = &Common_buttons[Current_screen-1][gr_screen.res][i].button;
		b->set_bmaps(Common_buttons[Current_screen-1][gr_screen.res][i].filename);
	}
}

void common_buttons_init(UI_WINDOW *ui_window)
{
	UI_BUTTON	*b;
	int			i;

	for ( i = 0; i < NUM_COMMON_BUTTONS; i++ ) {
		b = &Common_buttons[Current_screen-1][gr_screen.res][i].button;
		b->create( ui_window, "", Common_buttons[Current_screen-1][gr_screen.res][i].x, Common_buttons[Current_screen-1][gr_screen.res][i].y,  60, 30, 0, 1);
		// set up callback for when a mouse first goes over a button
		b->set_highlight_action( common_play_highlight_sound );
		b->set_bmaps(Common_buttons[Current_screen-1][gr_screen.res][i].filename);
		b->link_hotspot(Common_buttons[Current_screen-1][gr_screen.res][i].hotspot);
	}	

	// add some text	
	ui_window->add_XSTR("Briefing", 1504, Common_buttons[Current_screen-1][gr_screen.res][COMMON_BRIEFING_BUTTON].xt, Common_buttons[Current_screen-1][gr_screen.res][COMMON_BRIEFING_BUTTON].yt, &Common_buttons[Current_screen-1][gr_screen.res][COMMON_BRIEFING_BUTTON].button, UI_XSTR_COLOR_GREEN);
	ui_window->add_XSTR("Ship Selection", 1067, Common_buttons[Current_screen-1][gr_screen.res][COMMON_SS_BUTTON].xt, Common_buttons[Current_screen-1][gr_screen.res][COMMON_SS_BUTTON].yt, &Common_buttons[Current_screen-1][gr_screen.res][COMMON_SS_BUTTON].button, UI_XSTR_COLOR_GREEN);
	ui_window->add_XSTR("Weapon Loadout", 1068, Common_buttons[Current_screen-1][gr_screen.res][COMMON_WEAPON_BUTTON].xt, Common_buttons[Current_screen-1][gr_screen.res][COMMON_WEAPON_BUTTON].yt, &Common_buttons[Current_screen-1][gr_screen.res][COMMON_WEAPON_BUTTON].button, UI_XSTR_COLOR_GREEN);
	ui_window->add_XSTR("Commit", 1062, Common_buttons[Current_screen-1][gr_screen.res][COMMON_COMMIT_BUTTON].xt, Common_buttons[Current_screen-1][gr_screen.res][COMMON_COMMIT_BUTTON].yt, &Common_buttons[Current_screen-1][gr_screen.res][COMMON_COMMIT_BUTTON].button, UI_XSTR_COLOR_PINK);
	ui_window->add_XSTR("Help", 928, Common_buttons[Current_screen-1][gr_screen.res][COMMON_HELP_BUTTON].xt, Common_buttons[Current_screen-1][gr_screen.res][COMMON_HELP_BUTTON].yt, &Common_buttons[Current_screen-1][gr_screen.res][COMMON_HELP_BUTTON].button, UI_XSTR_COLOR_GREEN);
	ui_window->add_XSTR("Options", 1036, Common_buttons[Current_screen-1][gr_screen.res][COMMON_OPTIONS_BUTTON].xt, Common_buttons[Current_screen-1][gr_screen.res][COMMON_OPTIONS_BUTTON].yt, &Common_buttons[Current_screen-1][gr_screen.res][COMMON_OPTIONS_BUTTON].button, UI_XSTR_COLOR_GREEN);

	common_reset_buttons();

	Common_buttons[Current_screen-1][gr_screen.res][COMMON_COMMIT_BUTTON].button.set_hotkey(KEY_CTRLED+KEY_ENTER);
	Common_buttons[Current_screen-1][gr_screen.res][COMMON_HELP_BUTTON].button.set_hotkey(KEY_F1);
	Common_buttons[Current_screen-1][gr_screen.res][COMMON_OPTIONS_BUTTON].button.set_hotkey(KEY_F2);

	// for scramble or training missions, disable the ship/weapon selection regions
	if ( brief_only_allow_briefing() ) {
		Common_buttons[Current_screen-1][gr_screen.res][COMMON_SS_REGION].button.disable();
		Common_buttons[Current_screen-1][gr_screen.res][COMMON_WEAPON_REGION].button.disable();
	}
}

void set_active_ui(UI_WINDOW *ui_window)
{
	Active_ui_window = ui_window;
}

void common_music_init(int score_index)
{
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	if ( score_index >= NUM_SCORES ) {
		Int3();
		return;
	}

	if ( Mission_music[score_index] < 0 ) {
		if ( Num_music_files > 0 ) {
			Mission_music[score_index] = 0;
			nprintf(("Sound","No briefing music is selected, so play first briefing track: %s\n",Spooled_music[Mission_music[score_index]].name));
		} else {
			return;
		}
	}

	briefing_load_music( Spooled_music[Mission_music[score_index]].filename );
	// Use this id to trigger the start of music playing on the briefing screen
	Briefing_music_begin_timestamp = timestamp(BRIEFING_MUSIC_DELAY);
}

void common_music_do()
{
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	// Use this id to trigger the start of music playing on the briefing screen
	if ( timestamp_elapsed( Briefing_music_begin_timestamp) ) {
		Briefing_music_begin_timestamp = 0;
		briefing_start_music();
	}
}

void common_music_close()
{
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	if ( Num_music_files <= 0 )
		return;

	briefing_stop_music();
}

void common_maybe_play_cutscene(int movie_type)
{
	for (uint i = 0; i < The_mission.cutscenes.size(); i++) {
		if (movie_type == The_mission.cutscenes[i].type) {
			if (!eval_sexp( The_mission.cutscenes[i].formula )) {
				continue; 
			}

			if ( strlen(The_mission.cutscenes[i].cutscene_name) ) {
				common_music_close(); 
				movie_play( The_mission.cutscenes[i].cutscene_name );	//Play the movie!
				cutscene_mark_viewable( The_mission.cutscenes[i].cutscene_name );
			}
		}
	}
}

// function that sets the current palette to the interface palette.  This function
// needs to be followed by common_free_interface_palette() to restore the game palette.
void common_set_interface_palette(char *filename)
{
	static char buf[MAX_FILENAME_LEN + 1] = {0};

	if (!filename)
		filename = NOX("palette01");

	Assert(strlen(filename) <= MAX_FILENAME_LEN);
	if ( (InterfacePaletteBitmap != -1) && !stricmp(filename, buf) )
		return;  // already set to this palette

	strcpy_s(buf, filename);

	// unload the interface bitmap from memory
	if (InterfacePaletteBitmap != -1) {
		bm_release(InterfacePaletteBitmap);
		InterfacePaletteBitmap = -1;
	}

	// ugh - we don't need this anymore
	/*
	InterfacePaletteBitmap = bm_load(filename);
	if (InterfacePaletteBitmap < 0) {
		Error(LOCATION, "Could not load in \"%s\"!", filename);
	}
	*/

#ifndef HARDWARE_ONLY
	palette_use_bm_palette(InterfacePaletteBitmap);
#endif
}

// release the interface palette .pcx file, and restore the game palette
void common_free_interface_palette()
{
	// unload the interface bitmap from memory
	if (InterfacePaletteBitmap != -1) {
		bm_release(InterfacePaletteBitmap);
		InterfacePaletteBitmap = -1;
	}

	// restore the normal game palette
	palette_restore_palette();
}

// Init timers used for flashing buttons
void common_flash_button_init()
{
	Flash_timer = timestamp(MSC_FLASH_AFTER_TIME);
	Flash_toggle = 1;
	Flash_bright = 0;
}

// determine if we should draw a button as bright
int common_flash_bright()
{
	if ( timestamp_elapsed(Flash_timer) ) {
		if ( timestamp_elapsed(Flash_toggle) ) {
			Flash_toggle = timestamp(MSC_FLASH_INTERVAL);
			Flash_bright ^= 1;
		}
	}

	return Flash_bright;
}

// set the necessary pointers
void common_set_team_pointers(int team)
{
	Assert( (team >= 0) && (team < MAX_TVT_TEAMS) );

	Wss_slots = Wss_slots_teams[team];
	Ss_pool = Ss_pool_teams[team];
	Wl_pool = Wl_pool_teams[team];

	ss_set_team_pointers(team);
	wl_set_team_pointers(team);
}

// reset the necessary pointers to defaults
void common_reset_team_pointers()
{
	ss_reset_team_pointers();
	wl_reset_team_pointers();

	// these are done last so that we can make use of the Assert()'s in the above
	// functions to make sure the screens are exited and this is safe
	Wss_slots = NULL;
	Ss_pool = NULL;
	Wl_pool = NULL;
}

// common_select_init() will load in animations and bitmaps that are common to the 
// briefing/ship select/weapon select screens.  The global Common_select_inited is set
// after this function is called once, and is only cleared when common_select_close()
// is called.  This prevents multiple loadings of animations/bitmaps.
//
// This function also sets the palette based on the file palette01.pcx
void common_select_init()
{
	if ( Common_select_inited ) {
		nprintf(("Alan","common_select_init() returning without doing anything\n"));
		return;
	}

	nprintf(("Alan","entering common_select_init()\n"));

	// No anims are playing
	Background_playing = 0;
	Background_anim = NULL;

	Current_screen = Next_screen = ON_BRIEFING_SELECT;

	// load in the icons for the wing slots
	load_wing_icons(NOX("iconwing01"));

	Current_screen = Next_screen = ON_BRIEFING_SELECT;
	
	Commit_pressed = 0;

	Common_select_inited = 1;

	// this handles the case where the player played a multiplayer game but now is in single player (in one instance
	// of FreeSpace)
	if(!(Game_mode & GM_MULTIPLAYER)){
		chatbox_close();
	}

	// get the value of the team
	Common_team = 0;							// assume the first team -- we'll change this value if we need to

	if ( (Game_mode & GM_MULTIPLAYER) && IS_MISSION_MULTI_TEAMS )
		Common_team = Net_player->p_info.team;

	common_set_team_pointers(Common_team);

	ship_select_common_init();	
	weapon_select_common_init();
	common_flash_button_init();

	if ( Game_mode & GM_MULTIPLAYER ) {
		multi_ts_common_init();
	}

	// restore loadout from Player_loadout if this is the same mission as the one previously played
	if ( !(Game_mode & GM_MULTIPLAYER) ) {
		if ( !stricmp(Player_loadout.filename, Game_current_mission_filename) ) {
			wss_restore_loadout();
			ss_synch_interface();
			wl_synch_interface();
		}
	}
	
	Drop_icon_mflag = 0;
	Drop_on_wing_mflag = 0;

	//init colors
	gr_init_alphacolor(&Icon_colors[ICON_FRAME_NORMAL], 32, 128, 128, 255);
	gr_init_alphacolor(&Icon_colors[ICON_FRAME_HOT], 48, 160, 160, 255);
	gr_init_alphacolor(&Icon_colors[ICON_FRAME_SELECTED], 64, 192, 192, 255);
	gr_init_alphacolor(&Icon_colors[ICON_FRAME_PLAYER], 192, 128, 64, 255);
	gr_init_alphacolor(&Icon_colors[ICON_FRAME_DISABLED], 175, 175, 175, 255);
	gr_init_alphacolor(&Icon_colors[ICON_FRAME_DISABLED_HIGH], 100, 100, 100, 255);
	//init shaders
	gr_create_shader(&Icon_shaders[ICON_FRAME_NORMAL], 32, 128, 128, 255);
	gr_create_shader(&Icon_shaders[ICON_FRAME_HOT], 48, 160, 160, 255);
	gr_create_shader(&Icon_shaders[ICON_FRAME_SELECTED], 64, 192, 192, 255);
	gr_create_shader(&Icon_shaders[ICON_FRAME_PLAYER], 192, 128, 64, 255);
	gr_create_shader(&Icon_shaders[ICON_FRAME_DISABLED], 175, 175, 175, 255);
	gr_create_shader(&Icon_shaders[ICON_FRAME_DISABLED_HIGH], 100, 100, 100, 255);
}

void common_reset_buttons()
{
	int			i;
	UI_BUTTON	*b;

	for ( i = 0; i < NUM_COMMON_BUTTONS; i++ ) {
		b = &Common_buttons[Current_screen-1][gr_screen.res][i].button;
		b->reset_status();
	}

	switch(Current_screen) {
	case ON_BRIEFING_SELECT:
		Common_buttons[Current_screen-1][gr_screen.res][COMMON_BRIEFING_REGION].button.skip_first_highlight_callback();
		break;
	case ON_SHIP_SELECT:
		Common_buttons[Current_screen-1][gr_screen.res][COMMON_SS_REGION].button.skip_first_highlight_callback();
		break;
	case ON_WEAPON_SELECT:
		Common_buttons[Current_screen-1][gr_screen.res][COMMON_WEAPON_REGION].button.skip_first_highlight_callback();
		break;
	}
}

// common_select_do() is called once per loop in the interface screens and is used
// for drawing and changing the common animations and blitting common bitmaps.
int common_select_do(float frametime)
{
	int	k, new_k;


	if ( help_overlay_active(BR_OVERLAY) || help_overlay_active(SS_OVERLAY) || help_overlay_active(WL_OVERLAY) ) {
		Common_buttons[0][gr_screen.res][COMMON_HELP_BUTTON].button.reset_status();
		Common_buttons[1][gr_screen.res][COMMON_HELP_BUTTON].button.reset_status();
		Common_buttons[2][gr_screen.res][COMMON_HELP_BUTTON].button.reset_status();
		Active_ui_window->set_ignore_gadgets(1);
	} else {
		Active_ui_window->set_ignore_gadgets(0);
	}

	k = chatbox_process();

	if ( Game_mode & GM_NORMAL ) {
		new_k = Active_ui_window->process(k);
	} else {
		new_k = Active_ui_window->process(k, 0);
	}

	if ( (k > 0) || (new_k > 0) || B1_JUST_RELEASED ) {
		if ( help_overlay_active(BR_OVERLAY) || help_overlay_active(SS_OVERLAY) || help_overlay_active(WL_OVERLAY) ) {
			help_overlay_set_state(BR_OVERLAY, 0);
			help_overlay_set_state(SS_OVERLAY, 0);
			help_overlay_set_state(WL_OVERLAY, 0);
			Active_ui_window->set_ignore_gadgets(0);
			k = 0;
			new_k = 0;
		}
	}

	// test for mouse buttons,  must be done after Active_ui_window->process()
	// has been called to work properly
	//
	Drop_icon_mflag = 0;
	Drop_on_wing_mflag = 0;
	Brief_mouse_up_flag = 0;
	Mouse_down_last_frame = 0;

	// if the left mouse button was released...
	if ( B1_RELEASED ) {
		Drop_icon_mflag = 1;
		Drop_on_wing_mflag = 1;
	}

	// if the left mouse button was pressed...
	if ( B1_PRESSED ) {
		Mouse_down_last_frame = 1;
	}

	// basically a "click", only check for the click here to avoid action-on-over on briefing map
	if ( B1_JUST_PRESSED ) {
		Brief_mouse_up_flag = 1;
	}

	// reset timers for flashing buttons if key pressed
	if ( (k>0) || (new_k>0) ) {
		common_flash_button_init();
	}

	common_music_do();

	/*
	if ( Background_playing ) {

		if ( Background_anim_instance->frame_num == BUTTON_SLIDE_IN_FRAME ) {
			gamesnd_play_iface(SND_BTN_SLIDE);
		}	
	
		if ( Background_anim_instance->frame_num == Background_anim_instance->stop_at ) {
			// Free up the big honking background animation, since we won't be playing it again
			anim_release_render_instance(Background_anim_instance);
			anim_free(Background_anim);

			Background_playing = 0;		
			Current_screen = Next_screen = ON_BRIEFING_SELECT;
		}
	}
	*/

	if ( Current_screen != Next_screen ) {
		switch( Next_screen ) {
			case ON_BRIEFING_SELECT:
				gameseq_post_event( GS_EVENT_START_BRIEFING );
				break;

			case ON_SHIP_SELECT:
				// go to the specialized multiplayer team/ship select screen
				if(Game_mode & GM_MULTIPLAYER){
					gameseq_post_event(GS_EVENT_TEAM_SELECT);
				}
				// go to the normal ship select screen
				else {
					gameseq_post_event(GS_EVENT_SHIP_SELECTION);
				}
				break;

			case ON_WEAPON_SELECT:
				if ( !wss_slots_all_empty() ) {
					gameseq_post_event(GS_EVENT_WEAPON_SELECTION);
				} else {
					common_show_no_ship_error();
				}
				break;
		} // end switch
	}

   return new_k;
}

// -------------------------------------------------------------------------------------
// common_render()
//
void common_render(float frametime)
{
	if ( !Background_playing ) {
		gr_set_bitmap(Brief_background_bitmap);
		gr_bitmap(0, 0);
	}

	anim_render_all(0, frametime);
	anim_render_all(ON_SHIP_SELECT, frametime);
}

// -------------------------------------------------------------------------------------
// common_render_selected_screen_button()
//
//	A very ugly piece of special purpose code.  This is used to draw the pressed button
// frame for whatever stage of the briefing/ship select/weapons loadout we are on. 
//
void common_render_selected_screen_button()
{
	Common_buttons[Next_screen-1][gr_screen.res][Next_screen-1].button.draw_forced(2);
}

// -------------------------------------------------------------------------------------
// common_button_do() do the button action for the specified pressed button
//
void common_button_do(int i)
{
	if ( i == COMMON_COMMIT_BUTTON ) {
		Commit_pressed = 1;
		return;
	}

	if ( Background_playing )
		return;

	switch ( i ) {

	case COMMON_BRIEFING_BUTTON:
		if ( Current_screen != ON_BRIEFING_SELECT ) {
			gamesnd_play_iface(SND_SCREEN_MODE_PRESSED);
			Next_screen = ON_BRIEFING_SELECT;
		}
		break;

	case COMMON_WEAPON_BUTTON:
		if ( Current_screen != ON_WEAPON_SELECT ) {
			if ( !wss_slots_all_empty() ) {
				gamesnd_play_iface(SND_SCREEN_MODE_PRESSED);
				Next_screen = ON_WEAPON_SELECT;
			} else {
				common_show_no_ship_error();
			}
		}
		break;

	case COMMON_SS_BUTTON:
		if ( Current_screen != ON_SHIP_SELECT ) {
			gamesnd_play_iface(SND_SCREEN_MODE_PRESSED);
			Next_screen = ON_SHIP_SELECT;
		}
		break;

	case COMMON_OPTIONS_BUTTON:
		gamesnd_play_iface(SND_SWITCH_SCREENS);
		gameseq_post_event( GS_EVENT_OPTIONS_MENU );
		break;

	case COMMON_HELP_BUTTON:
		gamesnd_play_iface(SND_HELP_PRESSED);
		launch_context_help();
		break;

	} // end switch
}

// common_check_keys() will check for keypresses common to all the interface screens.
void common_check_keys(int k)
{
	switch (k) {

		case KEY_ESC: {

			if ( Current_screen == ON_BRIEFING_SELECT ) {
				if ( brief_get_closeup_icon() != NULL ) {
					brief_turn_off_closeup_icon();
					break;
				}
			}

			// prompt the host of a multiplayer game
			if(Game_mode & GM_MULTIPLAYER){
				multi_quit_game(PROMPT_ALL);
			} else {
				// go through the single player quit process
				// return to the main menu
/*
				int return_to_menu, pf_flags;
				pf_flags = PF_USE_AFFIRMATIVE_ICON|PF_USE_NEGATIVE_ICON;
				return_to_menu = popup(pf_flags, 2, POPUP_NO, POPUP_YES, XSTR( "Do you want to return to the Main Hall?\n(Your campaign position will be saved)", -1));
				if ( return_to_menu == 1 ) {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				}
*/
				gameseq_post_event(GS_EVENT_MAIN_MENU);
			}			
			break;
		}

		case KEY_CTRLED + KEY_ENTER:
			Commit_pressed = 1;
			break;

		case KEY_B:
			if ( Current_screen != ON_BRIEFING_SELECT && !Background_playing ) {
				Next_screen = ON_BRIEFING_SELECT;
			}
			break;

		case KEY_W:
			if ( brief_only_allow_briefing() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			if ( Current_screen != ON_WEAPON_SELECT && !Background_playing ) {
				if ( !wss_slots_all_empty() ) {
					Next_screen = ON_WEAPON_SELECT;
				} else {
					common_show_no_ship_error();
				}
			}

			break;

		case KEY_S:

			if ( brief_only_allow_briefing() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			if ( Current_screen != ON_SHIP_SELECT && !Background_playing ) {
				Next_screen = ON_SHIP_SELECT;
			}

			break;

		case KEY_SHIFTED+KEY_TAB:

			if ( brief_only_allow_briefing() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			if ( !Background_playing ) {
				switch ( Current_screen ) {
					case ON_BRIEFING_SELECT:
						if ( !wss_slots_all_empty() ) {
							Next_screen = ON_WEAPON_SELECT;
						} else {
							common_show_no_ship_error();
						}
						break;

					case ON_SHIP_SELECT:
						Next_screen = ON_BRIEFING_SELECT;
						break;

					case ON_WEAPON_SELECT:
						Next_screen = ON_SHIP_SELECT;
						break;
					default:
						Int3();
						break;
				}	// end switch
			}

			break;

		case KEY_TAB:

			if ( brief_only_allow_briefing() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			if ( !Background_playing ) {
				switch ( Current_screen ) {
					case ON_BRIEFING_SELECT:
						Next_screen = ON_SHIP_SELECT;
						break;

					case ON_SHIP_SELECT:
						if ( !wss_slots_all_empty() ) {
							Next_screen = ON_WEAPON_SELECT;
						} else {
							common_show_no_ship_error();
						}
						break;

					case ON_WEAPON_SELECT:
						Next_screen = ON_BRIEFING_SELECT;
						break;
					default:
						Int3();
						break;
				}	// end switch
			}

			break;

		case KEY_P:
			if ( Anim_paused )
				Anim_paused = 0;
			else
				Anim_paused = 1;
			break;
	} // end switch
}

// common_select_close() will release the memory for animations and bitmaps that
// were loaded in common_select_init().  This function will abort if the Common_select_inited
// flag is not set.  The last thing common_select_close() does in clear the Common_select_inited
// flag.  
//
// weapon_select_close() and ship_select_close() are both called, since common_select_close()
// is the function that is called the interface screens are finally exited.
void common_select_close()
{
	if ( !Common_select_inited ) {
		nprintf(("Alan","common_select_close() returning without doing anything\n"));
		return;
	}

	nprintf(("Alan","entering common_select_close()\n"));

	// catch open anims that weapon_select_init_team() opened when not in weapon_select - taylor
	// *** not the same as weapon_select_close() ***
	weapon_select_close_team();

	weapon_select_close();

	if(Game_mode & GM_MULTIPLAYER){
		multi_ts_close();
	} 

	ship_select_close();
	brief_close();	

	common_free_interface_palette();

	// release the bitmpas that were previously extracted from anim files
	unload_wing_icons();

	// Release any instances that may still exist
	anim_release_all_instances();

	// free the anim's that were loaded into memory
	/*
	if ( Background_anim ) {
		anim_free(Background_anim);
		Background_anim = NULL;
	}
	*/

	common_music_close();

	common_reset_team_pointers();

	Common_select_inited = 0;
}

// ------------------------------------------------------------------------
//	load_wing_icons() creates the bitmaps for wing icons 
//
void load_wing_icons(char *filename)
{
	int first_frame, num_frames;

	first_frame = bm_load_animation(filename, &num_frames);
	if ( first_frame == -1 ) {
		Error(LOCATION, "Could not load icons from %s\n", filename);
		return;
	}

	Wing_slot_disabled_bitmap = first_frame;
	Wing_slot_empty_bitmap = first_frame + 1;
//	Wing_slot_player_empty_bitmap = first_frame + 2;
}

// ------------------------------------------------------------------------
//	common_scroll_up_pressed()
//
int common_scroll_up_pressed(int *start, int size, int max_show)
{
	// check if we even need to scroll at all
	if ( size <= max_show ) {
		return 0;
	}

	if ( (size - *start) > max_show ) {
		*start += 1;
		return 1;
	}
	return 0;
}

// ------------------------------------------------------------------------
//	common_scroll_down_pressed()
//
int common_scroll_down_pressed(int *start, int size, int max_show)
{
	// check if we even need to scroll at all
	if ( size <= max_show ) {
		return 0;
	}

	if ( *start > 0 ) {
		*start -= 1;
		return 1;
	}
	return 0;
}

// NEWSTUFF BEGIN

// save ship selection loadout to the Player_loadout struct
void wss_save_loadout()
{
	int i,j;

	Assert( (Ss_pool != NULL) && (Wl_pool != NULL) && (Wss_slots != NULL) );

	// save the ship pool
	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		Player_loadout.ship_pool[i] = Ss_pool[i]; 
	}

	// save the weapons pool
	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		Player_loadout.weapon_pool[i] = Wl_pool[i]; 
	}

	// save the ship class / weapons for each slot
	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		Player_loadout.unit_data[i].ship_class = Wss_slots[i].ship_class;

		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			Player_loadout.unit_data[i].wep[j] = Wss_slots[i].wep[j];
			Player_loadout.unit_data[i].wep_count[j] = Wss_slots[i].wep_count[j];
		}
	}
}

// restore ship/weapons loadout from the Player_loadout struct
void wss_restore_loadout()
{
	int i,j;
	wss_unit	*slot;

	Assert( (Ss_pool != NULL) && (Wl_pool != NULL) && (Wss_slots != NULL) );

	// only restore if mission hasn't changed
	if ( stricmp(Player_loadout.last_modified, The_mission.modified) ) {
		return;
	}

	// restore the ship pool
	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		Ss_pool[i] = Player_loadout.ship_pool[i]; 
	}

	// restore the weapons pool
	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		Wl_pool[i] = Player_loadout.weapon_pool[i]; 
	}

	// restore the ship class / weapons for each slot
	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		slot = &Player_loadout.unit_data[i];
		Wss_slots[i].ship_class = slot->ship_class;

		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			Wss_slots[i].wep[j]= slot->wep[j];
			Wss_slots[i].wep_count[j] = slot->wep_count[j];
		}
	}
}

// Do a direct restore of the Player_loadout ship/weapon data to the wings
void wss_direct_restore_loadout()
{
	int				i, j;
	wing				*wp;
	wss_unit			*slot;

	// only restore if mission hasn't changed
	if ( stricmp(Player_loadout.last_modified, The_mission.modified) ) {
		return;
	}

	for ( i = 0; i < MAX_WING_BLOCKS; i++ ) {

		if ( Starting_wings[i] < 0 )
			continue;

		wp = &Wings[Starting_wings[i]];

		// If this wing is still on the arrival list, then update the parse objects
		if ( wp->ship_index[0] == -1 ) {
			p_object *p_objp;
			j=0;
			for ( p_objp = GET_FIRST(&Ship_arrival_list); p_objp != END_OF_LIST(&Ship_arrival_list); p_objp = GET_NEXT(p_objp) ) {
				slot = &Player_loadout.unit_data[i*MAX_WING_SLOTS+j];
				if ( p_objp->wingnum == WING_INDEX(wp) ) {
					p_objp->ship_class = slot->ship_class;
					wl_update_parse_object_weapons(p_objp, slot);
					j++;
				}
			}
		} else {
			int	k;
			int cleanup_ship_index[MAX_WING_SLOTS];
			ship	*shipp;

			for ( k = 0; k < MAX_WING_SLOTS; k++ ) {
				cleanup_ship_index[k] = -1;
			}

			// This wing is already created, so directly update the ships
			for ( j = 0; j < MAX_WING_SLOTS; j++ ) {
				slot = &Player_loadout.unit_data[i*MAX_WING_SLOTS+j];
				shipp = &Ships[wp->ship_index[j]];
				if ( shipp->ship_info_index != slot->ship_class ) {

					if ( wp->ship_index[j] == -1 ) {
						continue;
					}

					if ( slot->ship_class == -1 ) {
						cleanup_ship_index[j] = wp->ship_index[j];
						ship_add_exited_ship( shipp, SEF_PLAYER_DELETED );
						obj_delete(shipp->objnum);
						hud_set_wingman_status_none( shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
						continue;
					} else {
						change_ship_type(wp->ship_index[j], slot->ship_class);
					}
				}
				wl_bash_ship_weapons(&Ships[wp->ship_index[j]].weapons, slot);
			}

			for ( k = 0; k < MAX_WING_SLOTS; k++ ) {
				if ( cleanup_ship_index[k] != -1 ) {
					ship_wing_cleanup( cleanup_ship_index[k], wp );
				}
			}

		}
	} // end for 
}
int wss_slots_all_empty()
{
	int i;

	Assert( Wss_slots != NULL );

	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		if ( Wss_slots[i].ship_class >= 0 ) 
			break;
	}

	if ( i == MAX_WSS_SLOTS )
		return 1;
	else
		return 0;
}

// determine the mode (WSS_...) based on slot/list index values
int wss_get_mode(int from_slot, int from_list, int to_slot, int to_list, int wl_ship_slot)
{
	int mode, to_slot_empty=0;

	Assert( Wss_slots != NULL );

	if ( wl_ship_slot >= 0 ) {
		// weapons loadout
		if ( to_slot >= 0 ) {
			if ( Wss_slots[wl_ship_slot].wep_count[to_slot] == 0 ) {
				to_slot_empty = 1;
			}
		}
	} else {
		// ship select
		if ( to_slot >= 0 ) {
			if ( Wss_slots[to_slot].ship_class == -1 ){
				to_slot_empty = 1;
			}
		}
	}

	// determine mode
	if ( from_slot >= 0 && to_slot >= 0 ) {
		mode = WSS_SWAP_SLOT_SLOT;
	} else if ( from_slot >= 0 && to_list >= 0 ) {
		mode = WSS_DUMP_TO_LIST;
	} else if ( (from_list >= 0) && (to_slot >= 0) && (to_slot_empty) ) {
		mode = WSS_GRAB_FROM_LIST;
	} else if ( (from_list >= 0) && (to_slot >= 0) && (!to_slot_empty) ) {
		mode = WSS_SWAP_LIST_SLOT;
	} else {
		mode = -1;	// no changes required
	}

	return mode;
}

// store all the unit data and pool data 
int store_wss_data(ubyte *block, int max_size, int sound,int player_index)
{
	int j, i,offset=0;	
	short player_id;	
	short ishort;

	// this function assumes that the data is going to be used over the network
	// so make a non-network version of this function if needed
	Assert( Game_mode & GM_MULTIPLAYER );
	Assert( (Ss_pool != NULL) && (Wl_pool != NULL) && (Wss_slots != NULL) );

	if ( !(Game_mode & GM_MULTIPLAYER) )
		return 0;


	// write the ship pool 
	for ( i = 0; i < MAX_SHIP_CLASSES; i++ ) {
		if ( Ss_pool[i] > 0 ) {	
			block[offset++] = (ubyte)i;
			Assert( Ss_pool[i] < UCHAR_MAX );
			
			// take care of sign issues
			if(Ss_pool[i] == -1){
				block[offset++] = 0xff;
			} else {
				block[offset++] = (ubyte)Ss_pool[i];
			}
		}
	}

	block[offset++] = 0xff;	// signals start of weapons pool

	// write the weapon pool
	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		if ( Wl_pool[i] > 0 ) {
			block[offset++] = (ubyte)i;
			ishort = INTEL_SHORT( (short)Wl_pool[i] );
			memcpy(block+offset, &ishort, sizeof(short));
			offset += sizeof(short);
		}
	}

	// write the unit data

	block[offset++] = 0xff; // signals start of unit data

	for ( i=0; i<MAX_WSS_SLOTS; i++ ) {
		Assert( Wss_slots[i].ship_class < UCHAR_MAX );
		if(Wss_slots[i].ship_class == -1){
			block[offset++] = 0xff;
		} else {
			block[offset++] = (ubyte)(Wss_slots[i].ship_class);
		}
		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			// take care of sign issues
			Assert( Wss_slots[i].wep[j] < UCHAR_MAX );			
			if(Wss_slots[i].wep[j] == -1){
				block[offset++] = 0xff;
			} else {
				block[offset++] = (ubyte)(Wss_slots[i].wep[j]);
			}

			Assert( Wss_slots[i].wep_count[j] < SHRT_MAX );
			ishort = INTEL_SHORT( (short)Wss_slots[i].wep_count[j] );

			memcpy(&(block[offset]), &(ishort), sizeof(short) );
			offset += sizeof(short);
		}

		// mwa -- old way below -- too much space
		//memcpy(block+offset, &Wss_slots[i], sizeof(wss_unit));
		//offset += sizeof(wss_unit);
	}

	// any sound index
	if(sound == -1){
		block[offset++] = 0xff;
	} else {
		block[offset++] = (ubyte)sound;
	}

	// add a netplayer address to identify who should play the sound
	player_id = -1;

	if(player_index != -1){
		player_id = Net_players[player_index].player_id;		
	}

	player_id = INTEL_SHORT( player_id );
	memcpy(block+offset,&player_id,sizeof(player_id));
	offset += sizeof(player_id);

	Assert( offset < max_size );
	return offset;
}

int restore_wss_data(ubyte *block)
{
	int	i, j, sanity, offset=0;
	ubyte	b1, b2,sound;	
	short ishort;
	short player_id;	

	// this function assumes that the data is going to be used over the network
	// so make a non-network version of this function if needed
	Assert( Game_mode & GM_MULTIPLAYER );
	Assert( (Ss_pool != NULL) && (Wl_pool != NULL) && (Wss_slots != NULL) );

	if ( !(Game_mode & GM_MULTIPLAYER) )
		return 0;

	// restore ship pool
	sanity=0;
	memset(Ss_pool, 0, MAX_SHIP_CLASSES*sizeof(int));
	for (;;) {
		if ( sanity++ > MAX_SHIP_CLASSES ) {
			Int3();
			break;
		}

		b1 = block[offset++];
		if ( b1 == 0xff ) {
			break;
		}
	
		// take care of sign issues
		b2 = block[offset++];
		if(b2 == 0xff){
			Ss_pool[b1] = -1;
		} else {
			Ss_pool[b1] = b2;
		}
	}

	// restore weapons pool
	sanity=0;
	memset(Wl_pool, 0, MAX_WEAPON_TYPES*sizeof(int));
	for (;;) {
		if ( sanity++ > MAX_WEAPON_TYPES ) {
			Int3();
			break;
		}

		b1 = block[offset++];
		if ( b1 == 0xff ) {
			break;
		}
	
		memcpy(&ishort, block+offset, sizeof(short));
		offset += sizeof(short);
		Wl_pool[b1] = INTEL_SHORT( ishort );
	}

	for ( i=0; i<MAX_WSS_SLOTS; i++ ) {
		if(block[offset] == 0xff){
			Wss_slots[i].ship_class = -1;
		} else {
			Wss_slots[i].ship_class = block[offset];
		}
		offset++;		
		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			// take care of sign issues
			if(block[offset] == 0xff){
				Wss_slots[i].wep[j] = -1;
				offset++;
			} else {
				Wss_slots[i].wep[j] = (int)(block[offset++]);
			}
		
			memcpy( &ishort, &(block[offset]), sizeof(short) );
			ishort = INTEL_SHORT( ishort );
			Wss_slots[i].wep_count[j] = (int)ishort;
			offset += sizeof(short);
		}

		// mwa -- old way below
		//memcpy(&Wss_slots[i], block+offset, sizeof(wss_unit));
		//offset += sizeof(wss_unit);
	}

	// read in the sound data
	sound = block[offset++];					// the sound index

	// read in the player address
	memcpy(&player_id,block+offset,sizeof(player_id));
	player_id = INTEL_SHORT( player_id );
	offset += sizeof(short);
	
	// determine if I'm the guy who should be playing the sound
	if((Net_player != NULL) && (Net_player->player_id == player_id)){
		// play the sound
		if(sound != 0xff){
			gamesnd_play_iface((int)sound);
		}
	}

	if(!(Game_mode & GM_MULTIPLAYER)){
		ss_synch_interface();
	}	

	return offset;
}

void draw_model_icon(int model_id, int flags, float closeup_zoom, int x, int y, int w, int h, ship_info *sip, bool resize)
{
	matrix	object_orient	= IDENTITY_MATRIX;
	angles rot_angles = {0.0f,0.0f,0.0f};
	float zoom = closeup_zoom * 2.5f;

	if(sip == NULL)
	{
		//Assume it's a weapon
		rot_angles.h = -(PI_2);
	}
	else if(sip->flags & SIF_SMALL_SHIP)
	{
		rot_angles.p = -(PI_2);
	}
	else if((sip->max_speed <= 0.0f) && !(sip->flags & SIF_CARGO))
	{
		//Probably an installation or Knossos
		rot_angles.h = PI;
	}
	else
	{
		//Probably a capship
		rot_angles.h = PI_2;
	}
	vm_angles_2_matrix(&object_orient, &rot_angles);

	gr_set_clip(x, y, w, h, resize);
	g3_start_frame(1);
	if(sip != NULL)
	{
		g3_set_view_matrix( &sip->closeup_pos, &vmd_identity_matrix, zoom);

		if (!Cmdline_nohtl) {
			gr_set_proj_matrix(0.5f*Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		}
	}
	else
	{
		polymodel *pm = model_get(model_id);
		bsp_info *bs = NULL;	//tehe
		for(int i = 0; i < pm->n_models; i++)
		{
			if(!pm->submodel[i].is_thruster)
			{
				bs = &pm->submodel[i];
				break;
			}
		}

		if(bs == NULL)
		{
			bs = &pm->submodel[0];
		}

		vec3d weap_closeup;
		float y_closeup;

		//Find the center of teh submodel
		weap_closeup.xyz.x = -(bs->min.xyz.z + (bs->max.xyz.z - bs->min.xyz.z)/2.0f);
		weap_closeup.xyz.y = -(bs->min.xyz.y + (bs->max.xyz.y - bs->min.xyz.y)/2.0f);
		//weap_closeup.xyz.z = (weap_closeup.xyz.x/tanf(zoom / 2.0f));
		weap_closeup.xyz.z = -(bs->rad/tanf(zoom/2.0f));

		y_closeup = -(weap_closeup.xyz.y/tanf(zoom / 2.0f));
		if(y_closeup < weap_closeup.xyz.z)
		{
			weap_closeup.xyz.z = y_closeup;
		}
		if(bs->min.xyz.x < weap_closeup.xyz.z)
		{
			weap_closeup.xyz.z = bs->min.xyz.x;
		}
//		weap_closeup.xyz.x = bs->min.xyz.x + (bs->max.xyz.x - bs->min.xyz.x)/2.0f;
		g3_set_view_matrix( &weap_closeup, &vmd_identity_matrix, zoom);

		if (!Cmdline_nohtl) {
			gr_set_proj_matrix(0.5f*Proj_fov, gr_screen.clip_aspect, 0.05f, 1000.0f);
		}
	}

	model_set_detail_level(0);

	if (!Cmdline_nohtl)	{
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}

	if(!(flags & MR_NO_LIGHTING))
	{
		light_reset();
		vec3d light_dir = vmd_zero_vector;
		light_dir.xyz.x = -0.5;
		light_dir.xyz.y = 2.0f;
		light_dir.xyz.z = -2.0f;	
		light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
		// light_filter_reset();
		light_rotate_all();
		// lighting for techroom
	}

	model_clear_instance(model_id);
	model_render(model_id, &object_orient, &vmd_zero_vector, flags, -1, -1);

	if (!Cmdline_nohtl) 
	{
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	g3_end_frame();
	gr_reset_clip();
}

void draw_model_rotating(int model_id, int x1, int y1, int x2, int y2, float *rotation_buffer, vec3d *closeup_pos, float closeup_zoom, float rev_rate, int flags, bool resize, int effect)
{
	//WMC - Can't draw a non-model
	if(model_id < 0)
		return;
	
	float time = (timer_get_milliseconds()-anim_timer_start)/1000.0f;
	angles rot_angles, view_angles;
	matrix model_orient;
	
	if(effect == 2)  // FS2 Effect; Phase 0 Expand scanline, Phase 1 scan the grid and wireframe, Phase 2 scan up and reveal the ship, Phase 3 tilt the camera, Phase 4 start rotating the ship
	{
		
		// rotate the ship as much as required for this frame
		if(time >= 3.6f) // Phase 4
			*rotation_buffer += PI2 * flFrametime / rev_rate;
		else
			*rotation_buffer = PI; // No rotation before Phase 4
		while (*rotation_buffer > PI2){
			*rotation_buffer -= PI2;
		}

		view_angles.p = -PI_2;
		if(time >= 3.0f) // Phase 3
		{
			if(time >= 3.6f) // done tilting
			{
				view_angles.p = -0.6f; 
			}
			else
			{
				view_angles.p = (PI_2-0.6f)*(time-3.0f)*1.66667f - PI_2; // Phase 3 Tilt animation
			}
		}

		view_angles.b = 0.0f;
		view_angles.h = 0.0f;
		vm_angles_2_matrix(&model_orient, &view_angles);

		rot_angles.p = 0.0f;
		rot_angles.b = 0.0f;
		rot_angles.h = *rotation_buffer;
		vm_rotate_matrix_by_angles(&model_orient, &rot_angles);
	
		gr_set_clip(x1, y1, x2, y2, resize);
		vec3d wire_normal,ship_normal,plane_point;
		// Clip the wireframe below the scanline
		wire_normal.xyz.x = 0.0f;
		wire_normal.xyz.y = 1.0f;
		wire_normal.xyz.z = 0.0f;
		
		// Clip the ship above the scanline 
		ship_normal.xyz.x = 0.0f;
		ship_normal.xyz.y = -1.0f;
		ship_normal.xyz.z = 0.0f;

		polymodel *pm = model_get(model_id);
		
		//Make the clipping plane
		float clip = -pm->rad*0.7f;
		if(time < 1.5f && time >= 0.5f) // Phase 1 Move down
			clip = pm->rad*(time-1.0f)*1.4f;
		if(time >= 1.5f)
			clip = pm->rad*(time-2.0f)*(-1.4f); // Phase 2 Move up
		vm_vec_scale_sub(&plane_point,&vmd_zero_vector,&wire_normal,clip);
		
		g3_start_frame(1);
		if(closeup_pos != NULL)
		{
			g3_set_view_matrix(closeup_pos, &vmd_identity_matrix, closeup_zoom);
		}
		else
		{
			vec3d pos = { { { 0.0f, 0.0f, -(pm->rad * 1.5f) } } };
			g3_set_view_matrix(&pos, &vmd_identity_matrix, closeup_zoom);
		}

		if (!Cmdline_nohtl) {
			gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
			gr_set_view_matrix(&Eye_position, &Eye_matrix);
		}

		vec3d start, stop;
		float size = pm->rad*0.7f;
		float start_scale = MIN(time,0.5f)*2.5f;
		float offset = size*0.5f*MIN(MAX(time-3.0f,0.0f),0.6f)*1.66667f;
		if(time < 1.5f && time >= 0.5f)  // Clip the grid if were in phase 1
			g3_start_user_clip_plane(&plane_point,&wire_normal);
		
		g3_start_instance_angles(&vmd_zero_vector,&view_angles);
		if( time < 0.5f ) // Do the expanding scanline in phase 0
		{
			gr_set_color(0,255,0);
			start.xyz.x = size*start_scale;
			start.xyz.y = 0.0f;
			start.xyz.z = -clip;
			stop.xyz.x = -size*start_scale;
			stop.xyz.y = 0.0f;
			stop.xyz.z = -clip;
			g3_draw_htl_line(&start,&stop);
		}
		g3_done_instance(true);

		gr_zbuffer_set(false); // Turn of Depthbuffer so we dont get gridlines over the ship or a disappearing scanline 
		if( time >= 0.5f) // Phase 1 onward draw the grid
		{
			int i;

			start.xyz.y = -offset;
			start.xyz.z = size+offset*0.5f;
			stop.xyz.y = -offset;
			stop.xyz.z = -size+offset*0.5f;
		
			gr_set_color(0,200,0);
		
			g3_start_instance_angles(&vmd_zero_vector,&view_angles);
			for(i = -3; i < 4; i++)
			{
				start.xyz.x = stop.xyz.x = size*0.333f*i;
				g3_draw_htl_line(&start,&stop);
			}
		
			start.xyz.x = size;
			stop.xyz.x = -size;
			for(i = -3; i < 4; i++)
			{
				start.xyz.z = stop.xyz.z = size*0.333f*i+offset*0.5f;
				g3_draw_htl_line(&start,&stop);
			}
		
			g3_done_instance(true);
			
			
			// lighting for techroom
			light_reset();
			vec3d light_dir = vmd_zero_vector;
			light_dir.xyz.y = 1.0f;	
			light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
			light_rotate_all();
			// lighting for techroom

			// render the ships
			model_clear_instance(model_id);
			model_set_detail_level(0);
			gr_set_color(80,49,160);
			opengl_shader_set_animated_effect(ANIMATED_SHADER_LOADOUTSELECT_FS2);
			opengl_shader_set_animated_timer(-clip);
			if(time < 2.5f && time >= 0.5f) // Phase 1 and 2 render the wireframe
			{
				if(time >= 1.5f) // Just clip the wireframe after Phase 1
					g3_start_user_clip_plane(&plane_point,&wire_normal);
				
				model_render(model_id, &model_orient, &vmd_zero_vector, flags | MR_SHOW_OUTLINE_HTL | MR_NO_POLYS | MR_ANIMATED_SHADER);
				g3_stop_user_clip_plane();
			}
			if(time >= 1.5f) // Render the ship in Phase 2 onwards
			{
				
				g3_start_user_clip_plane(&plane_point,&ship_normal);
				model_render(model_id, &model_orient, &vmd_zero_vector, flags | MR_ANIMATED_SHADER);
				g3_stop_user_clip_plane();
			}
			
			if( time < 2.5f ) // Render the scanline in Phase 1 and 2
			{
				gr_set_color(0,255,0);
				start.xyz.x = size*1.25f;
				start.xyz.y = 0.0f;
				start.xyz.z = -clip;
				stop.xyz.x = -size*1.25f;
				stop.xyz.y = 0.0f;
				stop.xyz.z = -clip;
				g3_start_instance_angles(&vmd_zero_vector,&view_angles);
				g3_draw_htl_line(&start,&stop);
				g3_done_instance(true);
			}
		
		}
		gr_zbuffer_set(true); // Turn of depthbuffer again
		if (!Cmdline_nohtl) 
		{
			gr_end_view_matrix();
			gr_end_proj_matrix();
		}

		g3_end_frame();
		gr_reset_clip();
	}
	else
	{
		// rotate the ship as much as required for this frame
		*rotation_buffer += PI2 * flFrametime / rev_rate;
		while (*rotation_buffer > PI2){
			*rotation_buffer -= PI2;
		}

		view_angles.p = -0.6f;
		view_angles.b = 0.0f;
		view_angles.h = 0.0f;
		vm_angles_2_matrix(&model_orient, &view_angles);

		rot_angles.p = 0.0f;
		rot_angles.b = 0.0f;
		rot_angles.h = *rotation_buffer;
		vm_rotate_matrix_by_angles(&model_orient, &rot_angles);
	
		gr_set_clip(x1, y1, x2, y2, resize);
		vec3d normal;
		normal.xyz.x = 0.0f;
		normal.xyz.y = 1.0f;
		normal.xyz.z = 0.0f;
		g3_start_frame(1);
		// render the ship
		
		if(closeup_pos != NULL)
		{
			g3_set_view_matrix(closeup_pos, &vmd_identity_matrix, closeup_zoom);
		}
		else
		{
			polymodel *pm = model_get(model_id);
			vec3d pos = { { { 0.0f, 0.0f, -(pm->rad * 1.5f) } } };
			g3_set_view_matrix(&pos, &vmd_identity_matrix, closeup_zoom);
		}

		if (!Cmdline_nohtl) {
			gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
			gr_set_view_matrix(&Eye_position, &Eye_matrix);
		}

		// lighting for techroom
		light_reset();
		vec3d light_dir = vmd_zero_vector;
		light_dir.xyz.y = 1.0f;	
		light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
		light_rotate_all();
		// lighting for techroom

		model_clear_instance(model_id);
		model_set_detail_level(0);
		gr_set_color(0,128,0);
		if(effect == 1) // FS1 effect
		{
			opengl_shader_set_animated_effect(ANIMATED_SHADER_LOADOUTSELECT_FS1);
			opengl_shader_set_animated_timer(MIN(time*0.5f,2.0f));
			model_render(model_id, &model_orient, &vmd_zero_vector, flags | MR_ANIMATED_SHADER);
		}
		else
			model_render(model_id, &model_orient, &vmd_zero_vector, flags);

		if (!Cmdline_nohtl) 
		{
			gr_end_view_matrix();
			gr_end_proj_matrix();
		}

		g3_end_frame();
		gr_reset_clip();
	}
}

// NEWSTUFF END
