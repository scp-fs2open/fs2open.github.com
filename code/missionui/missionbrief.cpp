/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "freespace2/freespace.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "mission/missiongoals.h"
#include "gamesequence/gamesequence.h"
#include "ship/ship.h"
#include "io/key.h"
#include "render/3d.h"
#include "model/model.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "io/mouse.h"
#include "hud/hud.h"
#include "sound/audiostr.h"
#include "gamesnd/gamesnd.h"
#include "gamesnd/eventmusic.h"
#include "mission/missioncampaign.h"
#include "menuui/snazzyui.h"
#include "missionui/missionbrief.h"
#include "mission/missionbriefcommon.h"
#include "cmdline/cmdline.h"
#include "gamehelp/contexthelp.h"
#include "asteroid/asteroid.h"
#include "popup/popup.h"
#include "parse/sexp.h"
#include "globalincs/alphacolors.h"
#include "graphics/font.h"
#include "mission/missionmessage.h"
#include "playerman/player.h"
#include "sound/fsspeech.h"
#include "parse/parselo.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiteamselect.h"
#include "network/multiui.h"
#include "missionui/chatbox.h"


static int Brief_goals_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		65,152,508,211		// GR_640
	},
	{
		104,243,813,332		// GR_1024
	}
};

static int	Current_brief_stage;	// what stage of the briefing we're on
static int	Last_brief_stage;
static int	Num_brief_stages;
static int	Brief_multiplayer = FALSE;

static int	Brief_last_auto_advance = 0;	// timestamp of last auto-advance

// for managing the scene cut transition
static int	Quick_transition_stage;
static int	Start_fade_up_anim, Start_fade_down_anim;
static int	Brief_playing_fade_sound;
hud_anim		Fade_anim;

int	Briefing_music_handle = -1;
int	Briefing_music_begin_timestamp = 0;

// --------------------------------------------------------------------------------------
// Module scope globals
// --------------------------------------------------------------------------------------

static MENU_REGION	Briefing_select_region[NUM_BRIEFING_REGIONS];
static int				Num_briefing_regions;

// For closeup display 
#define					ONE_REV_TIME		6		// time (sec) for one revolution
#define					MAX_ANG_CHG			0.15f

static int Closeup_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		203, 151, 200, 213	// GR_640
	},
	{
		325, 241, 200, 213	// GR_1024
	}
};

/* no longer used - taylor
static int Closeup_img_h[GR_NUM_RESOLUTIONS] = {
	150,	// GR_640
	150		// GR_1024
};

static int Closeup_text_h[GR_NUM_RESOLUTIONS][4] = {
	63,		// GR_640
	63		// GR_1024
}; */

static int Brief_infobox_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		0, 391
	},
	{ // GR_1024
		0, 627
	}
};

static char *Brief_infobox_filename[GR_NUM_RESOLUTIONS] = {
	"InfoBox",
	"2_Infobox"
};

static char *Brief_filename[GR_NUM_RESOLUTIONS] = {
	"Brief",
	"2_Brief"
};

static char *Brief_multi_filename[GR_NUM_RESOLUTIONS] = {
	"BriefMulti",
	"2_BriefMulti"
};

static char *Brief_mask_filename[GR_NUM_RESOLUTIONS] = {
	"Brief-m",
	"2_Brief-m"
};

static char *Brief_multi_mask_filename[GR_NUM_RESOLUTIONS] = {
	"BriefMulti-m",
	"2_BriefMulti-m"
};


static char *Brief_win_filename[GR_NUM_RESOLUTIONS] = {
	"Briefwin",
	"2_Briefwin"
};

// coordinate inidices
#define BRIEF_X_COORD 0
#define BRIEF_Y_COORD 1
#define BRIEF_W_COORD 2
#define BRIEF_H_COORD 3

//static int Closeup_region[4] = {220,132,420,269};
int Closeup_region[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		211, 158, 215, 157
	}, 
	{ // GR_1024
		337, 253, 345, 252
	}, 
};

char *Closeup_background_filename[GR_NUM_RESOLUTIONS] = {
	NOX("BriefPop"),	// GR_640
	NOX("2_BriefPop")	// GR_1024
};

char *Closeup_button_filename[GR_NUM_RESOLUTIONS] = {
	NOX("BPB_00"),		// GR_640
	NOX("2_BPB_00"),		// GR_1024
};

int Closeup_button_hotspot = 14;

//static int			Closeup_button_coords[2] = {CLOSEUP_X+164,CLOSEUP_Y+227};
int Closeup_button_coords[GR_NUM_RESOLUTIONS][2] = {	
	{ 374, 316 },		// GR_640	
	{ 599, 506 }		// GR_1024	
};

UI_BUTTON	Closeup_close_button;
int Closeup_bitmap=-1;
int Closeup_one_revolution_time=ONE_REV_TIME;

brief_icon *Closeup_icon;
angles Closeup_angles;
matrix Closeup_orient;
vec3d Closeup_pos;
int Closeup_font_height;
int Closeup_x1, Closeup_y1;

// used for the 3d view of a closeup ship
float Closeup_zoom;
vec3d Closeup_cam_pos;

// Mask bitmap pointer and Mask bitmap_id
ubyte* Brief_ui_mask_data;		// pointer to actual bitmap data
static int Brief_ui_mask_w = -1;
static int Brief_ui_mask_h = -1;
int Brief_inited = FALSE;

int Briefing_paused = 0;	// for stopping audio and stage progression

int Max_brief_Lines;

// --------------------------------------------------------------------------------------
// Briefing specific UI
// --------------------------------------------------------------------------------------
#define	BRIEF_LAST_STAGE_MASK			7
#define	BRIEF_NEXT_STAGE_MASK			8
#define	BRIEF_PREV_STAGE_MASK			9
#define	BRIEF_FIRST_STAGE_MASK			10
#define	BRIEF_TEXT_SCROLL_UP_MASK		11
#define	BRIEF_TEXT_SCROLL_DOWN_MASK	12
#define	BRIEF_SKIP_TRAINING_MASK		15
#define	BRIEF_PAUSE_MASK					16

struct brief_buttons {	
	char *filename;
	int x, y;
	int xt, yt;
	int hotspot;
	int repeat;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	brief_buttons(char *name, int x1, int y1, int xt1, int yt1, int h, int r = 0) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), repeat(r) {}
};

int	Brief_grid_bitmap = -1;
int	Brief_text_bitmap = -1;

int	Brief_multitext_bitmap = -1;
int	Brief_background_bitmap =-1;

UI_WINDOW Brief_ui_window;

// Briefing specific buttons
#define NUM_BRIEF_BUTTONS 10

brief_buttons	Brief_buttons[GR_NUM_RESOLUTIONS][NUM_BRIEF_BUTTONS] = {
	{ // GR_640
		brief_buttons("BRB_08",		110,	116,	117,	157,	8),
		brief_buttons("BRB_09",		84,	116,	117,	157,	9),
		brief_buttons("BRB_10",		29,	116,	117,	157,	10),
		brief_buttons("BRB_11",		4,		116,	117,	157,	11),
		brief_buttons("BRB_12",		0,		405,	117,	157,	12),
		brief_buttons("BRB_13",		0,		447,	117,	157,	13),			
		brief_buttons("BRB_15",		562,	0,		117,	157,	15),			// skip training
		brief_buttons("BRB_16",		56,	116,	117,	157,	16),
		brief_buttons("TSB_34",		603,	374,	117,	157,	34),	
		brief_buttons("BRB_15",		562,	0,		117,	157,	15)			// exit loop	
	}, 
	{ // GR_1024
		brief_buttons("2_BRB_08",		175,	187,	117,	157,	8),
		brief_buttons("2_BRB_09",		135,	187,	117,	157,	9),
		brief_buttons("2_BRB_10",		47,	187,	117,	157,	10),
		brief_buttons("2_BRB_11",		8,		187,	117,	157,	11),
		brief_buttons("2_BRB_12",		0,		649,	117,	157,	12),
		brief_buttons("2_BRB_13",		0,		716,	117,	157,	13),			
		brief_buttons("2_BRB_15",		900,	0,		117,	157,	15),		// skip training
		brief_buttons("2_BRB_16",		91,	187,	117,	157,	16),
		brief_buttons("2_TSB_34",		966,	599,	117,	157,	34),			
		brief_buttons("2_BRB_15",		900,	0,		117,	157,	15)			// exit loop	
	}, 	
};

// briefing UI
#define BRIEF_SELECT_NUM_TEXT			3
UI_XSTR Brief_select_text[GR_NUM_RESOLUTIONS][BRIEF_SELECT_NUM_TEXT] = {
	{ // GR_640
		{ "Lock",				1270,	602,	364,	UI_XSTR_COLOR_GREEN, -1, &Brief_buttons[0][BRIEF_BUTTON_MULTI_LOCK].button },
		{ "Skip Training",	1442,	467,	7,		UI_XSTR_COLOR_GREEN, -1, &Brief_buttons[0][BRIEF_BUTTON_SKIP_TRAINING].button },
		{ "Exit Loop",			1477,	490,	7,		UI_XSTR_COLOR_GREEN, -1, &Brief_buttons[0][BRIEF_BUTTON_EXIT_LOOP].button }
	}, 
	{ // GR_1024
		{ "Lock",				1270,	964,	584,	UI_XSTR_COLOR_GREEN, -1, &Brief_buttons[1][BRIEF_BUTTON_MULTI_LOCK].button },
		{ "Skip Training",	1442,	805,	12,	UI_XSTR_COLOR_GREEN, -1, &Brief_buttons[1][BRIEF_BUTTON_SKIP_TRAINING].button },
		{ "Exit Loop",			1477,	830,	12,	UI_XSTR_COLOR_GREEN, -1, &Brief_buttons[1][BRIEF_BUTTON_EXIT_LOOP].button }
	}
};

// coordinates for briefing title -- the x value is for the RIGHT side of the text
static int Title_coords[GR_NUM_RESOLUTIONS][2] = {
	{575, 117},		// GR_640
	{918, 194}		// GR_1024
};

// coordinates for briefing title in multiplayer briefings -- the x value is for the LEFT side of the text
// third coord is max width of area for it to fit into (it is force fit there)
static int Title_coords_multi[GR_NUM_RESOLUTIONS][3] = {
	{1, 105, 190},		// GR_640
	{1, 174, 304}		// GR_1024
};

// briefing line widths
int Brief_max_line_width[GR_NUM_RESOLUTIONS] = {
	MAX_BRIEF_LINE_W_640, MAX_BRIEF_LINE_W_1024
};

//stuff for ht&l. vars and such
extern int Cmdline_nohtl;


// --------------------------------------------------------------------------------------
// Forward declarations
// --------------------------------------------------------------------------------------
int brief_setup_closeup(brief_icon *bi);
void brief_maybe_blit_scene_cut(float frametime);
void brief_transition_reset();

char *brief_tooltip_handler(char *str)
{
	if (!stricmp(str, NOX("@close"))) {
		if (Closeup_icon)
			return XSTR( "Close", 428);
	}

	return NULL;
}

// brief_skip_training_pressed()
//
// called when the skip training button on the briefing screen is hit.  When this happens,
// do a popup, then move to the next mission in the campaign.
void brief_skip_training_pressed()
{
	int val;

	val = popup(PF_USE_NEGATIVE_ICON | PF_USE_AFFIRMATIVE_ICON,2,POPUP_NO,POPUP_YES,XSTR( "Skip Training\n\n\n\nAre you sure you want to skip this training mission?", 429));

	// val is 0 when we hit no (first on the list)
	// AL: also, -1 is returned when ESC is hit
	if ( val <= 0 ){
		return;
	}

	// page out mission messages
	message_mission_shutdown();

	if ( !(Game_mode & GM_CAMPAIGN_MODE) ){
		gameseq_post_event( GS_EVENT_MAIN_MENU );
	}

	// tricky part.  Need to move to the next mission in the campaign.
	mission_goal_mark_objectives_complete();
	mission_goal_fail_incomplete();
	mission_campaign_store_goals_and_events_and_variables();

	mission_campaign_eval_next_mission();
	mission_campaign_mission_over();	

	gameseq_post_event( GS_EVENT_START_GAME );
}

// --------------------------------------------------------------------------------------
//	brief_do_next_pressed()
//
//
void brief_do_next_pressed(int play_sound)
{
	int now;
	now = timer_get_milliseconds();

	if ( (now - Brief_last_auto_advance) < 500 ) {
		return;
	}

	Current_brief_stage++;
	if ( Current_brief_stage >= Num_brief_stages ) {
		Current_brief_stage = Num_brief_stages - 1;
		gamesnd_play_iface(SND_GENERAL_FAIL);
		if ( Quick_transition_stage != -1 )
			brief_transition_reset();
	} else {
		if ( play_sound ) {
			gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
		}
	}

	Assert(Current_brief_stage >= 0);
}

// --------------------------------------------------------------------------------------
//	brief_do_prev_pressed()
//
//
void brief_do_prev_pressed()
{
	Current_brief_stage--;
	if ( Current_brief_stage < 0 ) {
		Current_brief_stage = 0;
		gamesnd_play_iface(SND_GENERAL_FAIL);
		if ( Quick_transition_stage != -1 )
			brief_transition_reset();
	} else {
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
	}
	Assert(Current_brief_stage >= 0);
}


// --------------------------------------------------------------------------------------
//	brief_do_start_pressed()
//
//
void brief_do_start_pressed()
{
	if ( Current_brief_stage != 0 ) {
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
		Current_brief_stage = 0;
		if ( Quick_transition_stage != -1 )
			brief_transition_reset();
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
	Assert(Current_brief_stage >= 0);
}

// --------------------------------------------------------------------------------------
//	brief_do_end_pressed()
//
//
void brief_do_end_pressed()
{
	if ( Current_brief_stage != Num_brief_stages - 1 ) {
		gamesnd_play_iface(SND_BRIEF_STAGE_CHG);
		Current_brief_stage = Num_brief_stages - 1;
		if ( Quick_transition_stage != -1 )
			brief_transition_reset();

	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
	Assert(Current_brief_stage >= 0);
}


void brief_scroll_up_text()
{
	Top_brief_text_line--;
	if ( Top_brief_text_line < 0 ) {
		Top_brief_text_line = 0;
		gamesnd_play_iface(SND_GENERAL_FAIL);
	} else {
		gamesnd_play_iface(SND_SCROLL);
	}
}

void brief_scroll_down_text()
{
	Top_brief_text_line++;
	if ( (Num_brief_text_lines[0] - Top_brief_text_line) < Max_brief_Lines) {
		Top_brief_text_line--;
		gamesnd_play_iface(SND_GENERAL_FAIL);
	} else {
		gamesnd_play_iface(SND_SCROLL);
	}
}


// handles the exit loop option
void brief_exit_loop_pressed()
{
	int val = popup(PF_USE_NEGATIVE_ICON | PF_USE_AFFIRMATIVE_ICON, 2, POPUP_NO, POPUP_YES, XSTR( "Exit Loop\n\n\n\nAre you sure you want to leave the mission loop?", 1489));

	// bail if esc hit or no clicked
	if (val <= 0) {
		return;
	}

	// handle the details
	// this also posts the start game event
	mission_campaign_exit_loop();
}


// -------------------------------------------------------------------------------------
// brief_select_button_do() do the button action for the specified pressed button
//
void brief_button_do(int i)
{
	switch ( i ) {
		case BRIEF_BUTTON_LAST_STAGE:
			brief_do_end_pressed();
			break;

		case BRIEF_BUTTON_NEXT_STAGE:
			brief_do_next_pressed(1);
			break;

		case BRIEF_BUTTON_PREV_STAGE:
			brief_do_prev_pressed();
			break;

		case BRIEF_BUTTON_FIRST_STAGE:
			brief_do_start_pressed();
			break;

		case BRIEF_BUTTON_SCROLL_UP:
			brief_scroll_up_text();
			break;

		case BRIEF_BUTTON_SCROLL_DOWN:
			brief_scroll_down_text();
			break;

		case BRIEF_BUTTON_PAUSE:
			gamesnd_play_iface(SND_USER_SELECT);
			fsspeech_pause(Player->auto_advance != 0);
			Player->auto_advance ^= 1;
			break;

		case BRIEF_BUTTON_SKIP_TRAINING:
			fsspeech_stop();
			brief_skip_training_pressed();
			break;

		case BRIEF_BUTTON_EXIT_LOOP:
			fsspeech_stop();
			brief_exit_loop_pressed();
			break;

		case BRIEF_BUTTON_MULTI_LOCK:
			Assert(Game_mode & GM_MULTIPLAYER);			
			// the "lock" button has been pressed
			multi_ts_lock_pressed();

			// disable the button if it is now locked
			if(multi_ts_is_locked()){
				Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.disable();
			}
			break;
	} // end switch
}

// -------------------------------------------------------------------
// brief_check_buttons()
//
// Iterate through the briefing buttons, checking if they are pressed
//
void brief_check_buttons()
{
	int			i;
	UI_BUTTON	*b;

	for (i=0; i<NUM_BRIEF_BUTTONS; i++) {
		b = &Brief_buttons[gr_screen.res][i].button;
		if ( b->pressed() ) {
			common_flash_button_init();
			brief_button_do(i);
		}
	}

	if (Closeup_close_button.pressed()) {
		brief_turn_off_closeup_icon();
	}
}

// -------------------------------------------------------------------
// brief_redraw_pressed_buttons()
//
// Redraw any briefing buttons that are pressed down.  This function is needed
// since we sometimes need to draw pressed buttons last to ensure the entire
// button gets drawn (and not overlapped by other buttons)
//
void brief_redraw_pressed_buttons()
{
	int			i;
	UI_BUTTON	*b;
	
	common_redraw_pressed_buttons();

	for ( i = 0; i < NUM_BRIEF_BUTTONS; i++ ) {
		b = &Brief_buttons[gr_screen.res][i].button;
		if ( b->button_down() ) {
			b->draw_forced(2);
		}
	}

	if ( !Player->auto_advance ) {
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_PAUSE].button.draw_forced(2);
	}
}

void brief_buttons_init()
{
	UI_BUTTON	*b;
	int			i;

	//if ( Briefing->num_stages <= 0 )
	//	return;

	for ( i = 0; i < NUM_BRIEF_BUTTONS; i++ ) {
		b = &Brief_buttons[gr_screen.res][i].button;
		b->create( &Brief_ui_window, "", Brief_buttons[gr_screen.res][i].x, Brief_buttons[gr_screen.res][i].y, 60, 30, 0, 1 );
		// set up callback for when a mouse first goes over a button
		b->set_highlight_action( common_play_highlight_sound );

		if ((i == BRIEF_BUTTON_SKIP_TRAINING) || (i == BRIEF_BUTTON_EXIT_LOOP)) {
			b->set_bmaps(Brief_buttons[gr_screen.res][i].filename, 3, 0);
		} else {
			b->set_bmaps(Brief_buttons[gr_screen.res][i].filename);
		}
		b->link_hotspot(Brief_buttons[gr_screen.res][i].hotspot);
	}

	// add all xstrs
	for(i=0; i<BRIEF_SELECT_NUM_TEXT; i++) {
		Brief_ui_window.add_XSTR(&Brief_select_text[gr_screen.res][i]);
	}

	// Hide the 'skip training' button by default.  Only enable and unhide if we are playing a training
	// mission
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_SKIP_TRAINING].button.disable();
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_SKIP_TRAINING].button.hide();
	if ( (Game_mode & GM_NORMAL) && (The_mission.game_type & MISSION_TYPE_TRAINING) ) {
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_SKIP_TRAINING].button.enable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_SKIP_TRAINING].button.unhide();
	}

	// Hide the 'exit loop' button by default.  Only enable and unhide if we are playing a loop
	// mission
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_EXIT_LOOP].button.disable();
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_EXIT_LOOP].button.hide();
	if ( (Game_mode & GM_NORMAL) && (Campaign.loop_enabled) && (Game_mode & GM_CAMPAIGN_MODE) ) {
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_EXIT_LOOP].button.enable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_EXIT_LOOP].button.unhide();
	}

	// maybe disable the multi-lock button
	if(!(Game_mode & GM_MULTIPLAYER)){
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.hide();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.disable();
	} else {
		// if we're not the host of the game (or a tema captain in team vs. team mode), disable the lock button
		if(Netgame.type_flags & NG_TYPE_TEAM){
			if(!(Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN)){
				Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.disable();
			}
		} else {
			if(!(Net_player->flags & NETINFO_FLAG_GAME_HOST)){
				Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.disable();
			}
		}
	}

	// create close button for closeup popup
	Closeup_close_button.create( &Brief_ui_window, "", Closeup_button_coords[gr_screen.res][BRIEF_X_COORD], Closeup_button_coords[gr_screen.res][BRIEF_Y_COORD], 60, 30, 0, 1 );
	Closeup_close_button.set_highlight_action( common_play_highlight_sound );
	Closeup_close_button.set_bmaps(Closeup_button_filename[gr_screen.res]);
	Closeup_close_button.link_hotspot(Closeup_button_hotspot);

	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_LAST_STAGE].button.set_hotkey(KEY_SHIFTED|KEY_RIGHT);
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_NEXT_STAGE].button.set_hotkey(KEY_RIGHT);
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_PREV_STAGE].button.set_hotkey(KEY_LEFT);
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_FIRST_STAGE].button.set_hotkey(KEY_SHIFTED|KEY_LEFT);
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_SCROLL_UP].button.set_hotkey(KEY_UP);
	Brief_buttons[gr_screen.res][BRIEF_BUTTON_SCROLL_DOWN].button.set_hotkey(KEY_DOWN);

	Closeup_close_button.disable();
	Closeup_close_button.hide();

	// if we have no briefing stages, hide and disable briefing buttons
	if(Num_brief_stages <= 0){
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_LAST_STAGE].button.disable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_LAST_STAGE].button.hide();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_NEXT_STAGE].button.disable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_NEXT_STAGE].button.hide();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_PREV_STAGE].button.disable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_PREV_STAGE].button.hide();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_FIRST_STAGE].button.disable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_FIRST_STAGE].button.hide();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_SCROLL_UP].button.disable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_SCROLL_UP].button.hide();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_SCROLL_DOWN].button.disable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_SCROLL_DOWN].button.hide();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_PAUSE].button.disable();
		Brief_buttons[gr_screen.res][BRIEF_BUTTON_PAUSE].button.hide();		
	}
}

// --------------------------------------------------------------------------------------
//	brief_get_closeup_icon()
//
//
// changed from uint for 64-bit support - taylor
brief_icon *brief_get_closeup_icon()
{
	return Closeup_icon;
}

// stop showing the closeup view of an icon
void brief_turn_off_closeup_icon()
{
	// turn off closup
	if ( Closeup_icon != NULL ) {
		gamesnd_play_iface(SND_BRIEF_ICON_SELECT);
		Closeup_icon = NULL;
		Closeup_close_button.disable();
		Closeup_close_button.hide();
	}
}

// --------------------------------------------------------------------------------------
//	brief_load_bitmaps()
//
//
void brief_load_bitmaps()
{
	Brief_text_bitmap = bm_load(Brief_infobox_filename[gr_screen.res]);
	Brief_grid_bitmap = bm_load(Brief_win_filename[gr_screen.res]);
	
	if ( Closeup_bitmap == -1 ) {
		Closeup_bitmap = bm_load(Closeup_background_filename[gr_screen.res]);
	}
}

// --------------------------------------------------------------------------------------
//	brief_ui_init()
//
//
void brief_ui_init()
{
	if(Game_mode & GM_MULTIPLAYER) {
		Brief_background_bitmap = bm_load(Brief_multi_filename[gr_screen.res]);
	} else {
		Brief_background_bitmap = bm_load(Brief_filename[gr_screen.res]);	
	}

	if ( Num_brief_stages <= 0 ){
		return;
	}

	brief_load_bitmaps();
}


// --------------------------------------------------------------------------------------
//	brief_set_default_closeup()
//
//
void brief_set_default_closeup()
{
	brief_stage		*bs;
	int				i;

	bs = &Briefing->stages[0];

	if ( Briefing->num_stages <= 0 ) {
		Closeup_icon = NULL;
		return;
	}

	if ( bs->num_icons <= 0 ) {
		Closeup_icon = NULL;
		return;
	}

	// check for the first highlighted icons to have as the default closeup
	for ( i = 0; i < bs->num_icons; i++ ) {
		if ( bs->icons[i].flags & BI_HIGHLIGHT )
			break;
	}
	
	if ( i == bs->num_icons ) {
		brief_setup_closeup(&bs->icons[0]);
	}
	else {
		brief_setup_closeup(&bs->icons[i]);
	}
}

//  funciton to evaluate the sexpressions of the briefing stages eliminating those stages
// which shouldn't get shown
void brief_compact_stages()
{
	int num, before, result, i;

	/*
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.campaign_mode == MP_CAMPAIGN) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		Game_mode |= GM_CAMPAIGN_MODE;
	}
	*/

	before = Briefing->num_stages;

	num = 0;
	while ( num < Briefing->num_stages ) {
		result = eval_sexp( Briefing->stages[num].formula );
		if ( !result ) {
			if ( Briefing->stages[num].new_text != NULL ) {
				vm_free( Briefing->stages[num].new_text );
				Briefing->stages[num].new_text = NULL;
			}

			if ( Briefing->stages[num].icons != NULL ) {
				vm_free( Briefing->stages[num].icons );
				Briefing->stages[num].icons = NULL;
			}


			if ( Briefing->stages[num].lines != NULL ) {
				vm_free( Briefing->stages[num].lines );
				Briefing->stages[num].lines = NULL;
			}

			Briefing->stages[num].num_icons = 0;
			for ( i = num+1; i < Briefing->num_stages; i++ ) {
				Briefing->stages[i-1] = Briefing->stages[i];
			}
			Briefing->num_stages--;
			continue;
		}
		num++;
	}

	// completely clear out the old entries (if any) so we don't access them by mistake - taylor
	if ( before > Briefing->num_stages ) {
		for ( i = Briefing->num_stages; i < before; i++ ) {
			memset( &Briefing->stages[i], 0, sizeof(brief_stage) );
		}
	}

	/*
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.campaign_mode == MP_CAMPAIGN) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		Game_mode &= ~(GM_CAMPAIGN_MODE);
	}
	*/
}


// --------------------------------------------------------------------------------------
// brief_init() 
//
	int red_alert_mission(void);
//
void brief_init()
{
	int i;

	// for multiplayer, change the state in my netplayer structure
	// and initialize the briefing chat area thingy
	if ( Game_mode & GM_MULTIPLAYER ){
		Net_player->state = NETPLAYER_STATE_BRIEFING;
	}

	// Non standard briefing in red alert mission
	if ( red_alert_mission() ) {
		Int3();	// since we shouldn't be here
		gameseq_post_event(GS_EVENT_RED_ALERT);
		return;
	}

	// get a pointer to the appropriate briefing structure
	if(MULTI_TEAM){
		Briefing = &Briefings[Net_player->p_info.team];
	} else {
		Briefing = &Briefings[0];			
	}

	// Goober5000 - replace any variables (probably persistent variables) with their values
	for (i = 0; i < Briefing->num_stages; i++)
	{
		if (Briefing->stages[i].new_text)
			sexp_replace_variable_names_with_values(Briefing->stages[i].new_text, MAX_BRIEF_LEN);
	}

	Brief_last_auto_advance = 0;

	brief_compact_stages();			// compact the briefing array to eliminate unused stages

	common_set_interface_palette("BriefingPalette");

	set_active_ui(&Brief_ui_window);
	Current_screen = ON_BRIEFING_SELECT;
	brief_restart_text_wipe();
	common_flash_button_init();
	common_music_init(SCORE_BRIEFING);

	help_overlay_set_state(BR_OVERLAY,0);

	if ( Brief_inited == TRUE ) {
		common_buttons_maybe_reload(&Brief_ui_window);	// AL 11-21-97: this is necessary since we may returning from the hotkey
																		// screen, which can release common button bitmaps.
		common_reset_buttons();
		nprintf(("Alan","brief_init() returning without doing anything\n"));
		return;
	}

	if (The_mission.game_type & MISSION_TYPE_TRAINING)
		Num_brief_stages = Briefing->num_stages;
	else
		Num_brief_stages = Briefing->num_stages + 1;

	Current_brief_stage = 0;
	Last_brief_stage = 0;

	// init the scene-cut data
	brief_transition_reset();

	hud_anim_init(&Fade_anim, Brief_static_coords[gr_screen.res][0], Brief_static_coords[gr_screen.res][1], Brief_static_name[gr_screen.res]);
	hud_anim_load(&Fade_anim);

	nprintf(("Alan","Entering brief_init()\n"));
	common_select_init();

	help_overlay_load(BR_OVERLAY);

	// Set up the mask regions
   // initialize the different regions of the menu that will react when the mouse moves over it
	Num_briefing_regions = 0;

	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	COMMON_BRIEFING_REGION,				0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	COMMON_SS_REGION,						0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	COMMON_WEAPON_REGION,				0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	COMMON_COMMIT_REGION,				0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	COMMON_HELP_REGION,					0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	COMMON_OPTIONS_REGION,				0);

	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	BRIEF_LAST_STAGE_MASK,			0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	BRIEF_NEXT_STAGE_MASK,			0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	BRIEF_PREV_STAGE_MASK,			0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	BRIEF_FIRST_STAGE_MASK,			0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	BRIEF_TEXT_SCROLL_UP_MASK,		0);
	snazzy_menu_add_region(&Briefing_select_region[Num_briefing_regions++], "",	BRIEF_TEXT_SCROLL_DOWN_MASK,	0);

	// init common UI
	Brief_ui_window.create( 0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0 );

	if(Game_mode & GM_MULTIPLAYER){
		Brief_ui_window.set_mask_bmap(Brief_multi_mask_filename[gr_screen.res]);
	} else {
		Brief_ui_window.set_mask_bmap(Brief_mask_filename[gr_screen.res]);
	}

	// get a pointer to the mask data
	Brief_ui_mask_data = Brief_ui_window.get_mask_data( &Brief_ui_mask_w, &Brief_ui_mask_h );

	Brief_ui_window.tooltip_handler = brief_tooltip_handler;
	common_buttons_init(&Brief_ui_window);
	brief_buttons_init();

	// if multiplayer, initialize a few other systems
	if(Game_mode & GM_MULTIPLAYER){		
		// again, should not be necessary, but we'll leave it for now
		chatbox_create();

		// force the chatbox to be small
		chatbox_force_small();
	}

	// set up the screen regions
	brief_init_screen(Brief_multiplayer);

	// init briefing specific UI
	brief_ui_init();

	// init the briefing map
	brief_init_map();

	// init the briefing voice playback
	brief_voice_init();
	brief_voice_load_all();

	// init objectives display stuff
	ML_objectives_init(Brief_goals_coords[gr_screen.res][BRIEF_X_COORD], Brief_goals_coords[gr_screen.res][BRIEF_Y_COORD], Brief_goals_coords[gr_screen.res][BRIEF_W_COORD], Brief_goals_coords[gr_screen.res][BRIEF_H_COORD]);

	// set the camera target
	if ( Briefing->num_stages > 0 ) {
		brief_set_new_stage(&Briefing->stages[0].camera_pos, &Briefing->stages[0].camera_orient, 0, Current_brief_stage);
		brief_reset_icons(Current_brief_stage);
	}

	Brief_playing_fade_sound = 0;
	Brief_mouse_up_flag	= 0;
	Closeup_font_height = gr_get_font_height();
	Closeup_icon = NULL;
   Brief_inited = TRUE;
}

// -------------------------------------------------------------------------------------
// brief_render_closeup_text()
//
//
#define CLOSEUP_TEXT_OFFSET	10
void brief_render_closeup_text()
{
/*
	brief_icon	*bi;
	char			line[MAX_ICON_TEXT_LINE_LEN];
	int			n_lines, i, render_x, render_y;
	int			n_chars[MAX_ICON_TEXT_LINES];
	char			*p_str[MAX_ICON_TEXT_LINES];

	if ( Closeup_icon == NULL ) {
		Int3();
		return;
	}

	bi = Closeup_icon;

	render_x = Closeup_region[0];
	render_y = Closeup_region[1] + CLOSEUP_IMG_H;
	
	gr_set_clip(render_x+CLOSEUP_TEXT_OFFSET, render_y, CLOSEUP_W,CLOSEUP_TEXT_H);
	gr_set_color_fast(&Color_white);

//	n_lines = split_str(bi->text, CLOSEUP_W - 2*CLOSEUP_TEXT_OFFSET, n_chars, p_str, MAX_ICON_TEXT_LINES);
	Assert(n_lines != -1);

	for ( i = 0; i < n_lines; i++ ) {
		Assert(n_chars[i] < MAX_ICON_TEXT_LINE_LEN);
		strncpy(line, p_str[i], n_chars[i]);
		line[n_chars[i]] = 0;
		gr_printf(0,0+i*Closeup_font_height,line);
	}
*/
}

// -------------------------------------------------------------------------------------
// brief_render_closeup()
//
//
void brief_render_closeup(int ship_class, float frametime)
{
	matrix	view_orient = IDENTITY_MATRIX;
	matrix	temp_matrix;
	float		ang;
	int		w,h;

	if (ship_class < 0)
		return;

	if (Closeup_bitmap < 0)
		return;

	ang = PI2 * frametime/Closeup_one_revolution_time;
	if ( ang > MAX_ANG_CHG )
		ang = MAX_ANG_CHG;

	Closeup_angles.h += ang;
	if ( Closeup_angles.h > PI2 )
		Closeup_angles.h -= PI2;
	vm_angles_2_matrix(&temp_matrix, &Closeup_angles );
	Closeup_orient = temp_matrix;

	w = Closeup_region[gr_screen.res][2];
	h = Closeup_region[gr_screen.res][3];
	gr_set_clip(Closeup_region[gr_screen.res][0], Closeup_region[gr_screen.res][1], w, h);

	g3_start_frame(1);
	g3_set_view_matrix(&Closeup_cam_pos, &view_orient, Closeup_zoom);


	if (!Cmdline_nohtl) {
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}
	
	model_clear_instance( Closeup_icon->modelnum );
	model_set_detail_level(0);

	int is_neb = The_mission.flags & MISSION_FLAG_FULLNEB;

	// maybe switch off nebula rendering
	if(is_neb){
		The_mission.flags &= ~MISSION_FLAG_FULLNEB;
	}

	int model_render_flags_local;
	if ( Closeup_icon->type == ICON_JUMP_NODE) {
		model_set_outline_color(HUD_color_red, HUD_color_green, HUD_color_blue);		
		model_render_flags_local = MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_POLYS | MR_SHOW_OUTLINE;
	} else {
		model_render_flags_local = MR_NO_LIGHTING | MR_LOCK_DETAIL | MR_AUTOCENTER;
	}

	model_render( Closeup_icon->modelnum, &Closeup_orient, &Closeup_pos, model_render_flags_local );

	if (is_neb) {
		The_mission.flags |= MISSION_FLAG_FULLNEB;
	}

	if (!Cmdline_nohtl)
	{
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	g3_end_frame();

	gr_set_color_fast(&Color_bright_white);

	gr_string(0x8000,2,Closeup_icon->closeup_label,false);
//	brief_render_closeup_text();

	Closeup_close_button.enable();
	Closeup_close_button.unhide();

	gr_reset_clip();
}

// -------------------------------------------------------------------------------------
// brief_render()
//
//	frametime is in seconds
void brief_render(float frametime)
{
	int z, w, h;

	if ( Num_brief_stages <= 0 ) {
		gr_set_color_fast(&Color_white);
		Assert( Game_current_mission_filename != NULL );
		gr_printf(0x8000,200,XSTR( "No Briefing exists for mission: %s", 430), Game_current_mission_filename);

		#ifndef NDEBUG
		gr_get_string_size(&w, &h, The_mission.name);
		gr_set_color_fast(&Color_normal);
		gr_printf(0x8000, 230, NOX("[filename: %s, last mod: %s]"), Mission_filename, The_mission.modified);
		#endif

		return;
	}

	gr_set_bitmap(Brief_grid_bitmap);
	gr_bitmap(Brief_bmap_coords[gr_screen.res][0], Brief_bmap_coords[gr_screen.res][1]);

	brief_render_map(Current_brief_stage, frametime);

	// draw the frame bitmaps
	gr_set_bitmap(Brief_text_bitmap);
	gr_bitmap(Brief_infobox_coords[gr_screen.res][0], Brief_infobox_coords[gr_screen.res][1]);
	brief_blit_stage_num(Current_brief_stage, Num_brief_stages);

	// only try to render text and play audio if there is really something here - taylor
	if (Briefing->num_stages > 0) {
		z = brief_render_text(Top_brief_text_line, Brief_text_coords[gr_screen.res][0], Brief_text_coords[gr_screen.res][1], Brief_text_coords[gr_screen.res][3], frametime);
		if (z) {
			brief_voice_play(Current_brief_stage);
		}

		if (gr_screen.res == 1) {
			Max_brief_Lines = 110/gr_get_font_height(); //Make the max number of lines dependent on the font height. 225 and 85 are magic numbers, based on the window size in retail. 
		} else {
			Max_brief_Lines = 60/gr_get_font_height();
		}

		// maybe output the "more" indicator
		if ( Max_brief_Lines < Num_brief_text_lines[0] ) {
			// can be scrolled down
			int more_txt_x = Brief_text_coords[gr_screen.res][0] + (Brief_max_line_width[gr_screen.res]/2) - 10;
			int more_txt_y = Brief_text_coords[gr_screen.res][1] + Brief_text_coords[gr_screen.res][3] - 2;				// located below brief text, centered

			gr_get_string_size(&w, &h, XSTR("more", 1469), strlen(XSTR("more", 1469)));
			gr_set_color_fast(&Color_black);
			gr_rect(more_txt_x-2, more_txt_y, w+3, h);
			gr_set_color_fast(&Color_red);
			gr_string(more_txt_x, more_txt_y, XSTR("more", 1469));  // base location on the input x and y?
		}
	}

	brief_maybe_blit_scene_cut(frametime);	

#if !defined(NDEBUG)
	gr_set_color_fast(&Color_normal);
	int title_y_offset = (Game_mode & GM_MULTIPLAYER) ? 20 : 10;
	gr_printf(Brief_bmap_coords[gr_screen.res][0], Brief_bmap_coords[gr_screen.res][1]-title_y_offset, NOX("[name: %s, mod: %s]"), Mission_filename, The_mission.modified);
#endif

	// output mission title
	gr_set_color_fast(&Color_bright_white);

	if (Game_mode & GM_MULTIPLAYER) {
		char buf[256];
		strncpy(buf, The_mission.name, 256);
		gr_force_fit_string(buf, 255, Title_coords_multi[gr_screen.res][2]);
		gr_string(Title_coords_multi[gr_screen.res][0], Title_coords_multi[gr_screen.res][1], buf);
	} else {
		gr_get_string_size(&w, NULL, The_mission.name);
		gr_string(Title_coords[gr_screen.res][0] - w, Title_coords[gr_screen.res][1], The_mission.name);
	}

	// maybe do objectives
	if (Current_brief_stage == Briefing->num_stages) {
		ML_objectives_do_frame(0);
	}	
}

// -------------------------------------------------------------------------------------
// brief_set_closeup_pos()
//
//
#define CLOSEUP_OFFSET 20
void brief_set_closeup_pos(brief_icon *bi)
{
	Closeup_y1 = 10;
	Closeup_x1 = fl2i(320 - Closeup_coords[gr_screen.res][BRIEF_W_COORD]/2.0f + 0.5f);
}

void brief_get_closeup_ship_modelnum(brief_icon *ci)
{
	object	*objp;
	ship		*sp;

	// find the model number for the ship to display
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {

		if ( objp == &obj_used_list || objp->type != OBJ_SHIP ) {
			continue;
		}
		
		sp = &Ships[objp->instance];
		if ( sp->ship_info_index == ci->ship_class ) {
			ci->ship_class = sp->ship_info_index;
			ci->modelnum = Ship_info[sp->ship_info_index].model_num;
			ci->radius = objp->radius;
			break;
		}
	}
}

// -------------------------------------------------------------------------------------
// brief_setup_closeup()
//
// exit: 0	=>		set-up icon sucessfully
//			-1	=>		could not setup closeup icon
int brief_setup_closeup(brief_icon *bi)
{
	char				pof_filename[NAME_LENGTH];
	ship_info		*sip=NULL;
	vec3d			tvec;

	Closeup_icon = bi;
	Closeup_icon->ship_class = bi->ship_class;
	Closeup_icon->modelnum = -1;

	Closeup_one_revolution_time = ONE_REV_TIME;

	switch(Closeup_icon->type) {
	case ICON_PLANET:
		Closeup_icon = NULL;
		return -1;
		/*
		strcpy_s(pof_filename, NOX("planet.pof"));
		strcpy_s(Closeup_icon->closeup_label, XSTR("planet",-1));
		vm_vec_make(&Closeup_cam_pos, 0.0f, 0.0f, -8300.0f);
		Closeup_zoom = 0.5f;
		Closeup_one_revolution_time = ONE_REV_TIME * 3;
		*/
		break;
	case ICON_ASTEROID_FIELD:
		strcpy_s(pof_filename, Asteroid_info[ASTEROID_TYPE_LARGE].pof_files[0]);
		strcpy_s(Closeup_icon->closeup_label, XSTR( "asteroid", 431));
		vm_vec_make(&Closeup_cam_pos, 0.0f, 0.0f, -334.0f);
		Closeup_zoom = 0.5f;
		break;
	case ICON_JUMP_NODE:
		strcpy_s(pof_filename, NOX("subspacenode.pof"));
		strcpy_s(Closeup_icon->closeup_label, XSTR( "jump node", 432));
		vm_vec_make(&Closeup_cam_pos, 0.0f, 0.0f, -2700.0f);
		Closeup_zoom = 0.5f;
		Closeup_one_revolution_time = ONE_REV_TIME * 3;
		break;
	case ICON_UNKNOWN:
	case ICON_UNKNOWN_WING:
		strcpy_s(pof_filename, NOX("unknownship.pof"));
		strcpy_s(Closeup_icon->closeup_label, XSTR( "unknown", 433));
		vm_vec_make(&Closeup_cam_pos, 0.0f, 0.0f, -22.0f);
		Closeup_zoom = 0.5f;
		break;
	default:
		brief_get_closeup_ship_modelnum(Closeup_icon);
		Assert( Closeup_icon->ship_class != -1 );
		sip = &Ship_info[Closeup_icon->ship_class];

		strcpy_s(Closeup_icon->closeup_label,(sip->alt_name[0]) ? sip->alt_name : sip->name);

		// cut any text off after (and including) '#' char
		end_string_at_first_hash_symbol(Closeup_icon->closeup_label);

		// Goober5000 - wcsaga doesn't want this
		if (Ship_types[sip->class_type].hud_bools & STI_HUD_NO_CLASS_DISPLAY ) {
			strcat_s(Closeup_icon->closeup_label, XSTR( " class", 434));
		}

		break;
	}
	
	if ( Closeup_icon->modelnum == -1 ) {
		if ( sip == NULL ) {
			Closeup_icon->modelnum = model_load(pof_filename, 0, NULL);
		} else {
			Closeup_icon->modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
		}
		Closeup_icon->radius = model_get_radius(Closeup_icon->modelnum);
	}

	vm_set_identity(&Closeup_orient);
	vm_vec_make(&tvec, 0.0f, 0.0f, -1.0f);
	Closeup_orient.vec.fvec = tvec;
	vm_vec_zero(&Closeup_pos);
	Closeup_angles.p  = 0.0f;
	Closeup_angles.b  = 0.0f;
	Closeup_angles.h  = PI;

	brief_set_closeup_pos(bi);

	if ( sip ) {
		Closeup_cam_pos = sip->closeup_pos;
		Closeup_zoom = sip->closeup_zoom;
	}

	return 0;
}

// -------------------------------------------------------------------------------------
// brief_update_closeup_icon()
//
//	input:	mode	=>		how to update the closeup view
//								0 -> disable
//
void brief_update_closeup_icon(int mode)
{
	brief_stage		*bs;
	brief_icon		*bi;
	int				i, closeup_index;
	

	if ( mode == 0 ) {
		// mode 0 means disable the closeup icon
		if ( Closeup_icon != NULL ) {
			brief_turn_off_closeup_icon();
		}
		return;
	}

	if ( Closeup_icon == NULL )
		return;

	bs = &Briefing->stages[Current_brief_stage];

	closeup_index = -1;
	// see if any icons are being highlighted this stage
	for ( i = 0; i < bs->num_icons; i++ ) {
		bi = &bs->icons[i];
		if ( bi->flags & BI_HIGHLIGHT ) {
			closeup_index = i;
			break;
		}
	}

	if ( closeup_index != -1 ) {
		bi = &bs->icons[closeup_index];
		brief_setup_closeup(bi);
	}
	else {
		Closeup_icon = NULL;
	}
}


// -------------------------------------------------------------------------------------
// brief_check_for_anim()
//
//
void brief_check_for_anim()
{
	int				mx, my, i, iw, ih, x, y;
	brief_stage		*bs;
	brief_icon		*bi = NULL;

	bs = &Briefing->stages[Current_brief_stage];
	mouse_get_pos_unscaled( &mx, &my );

	// if mouse click is over the VCR controls, don't launch an icon
	// FIXME - should prolly push these into defines instead of hardcoding this
//	if ( mx >= 0 && mx <= 115 && my >= 136 && my <= 148 ) {
//		return;
//	}

	// same as above but without the hardcoded values, which were wrong anyway.  don't know
	// how this will work longterm but will hopefully keep things working well - taylor
	for (i = 0; i <= BRIEF_BUTTON_FIRST_STAGE; i++) {
		Brief_buttons[gr_screen.res][i].button.get_dimensions(&x, &y, &iw, &ih);

		if (mx >= x && mx <= (x+iw) && my >= y && my <= (y+ih)) {
			return;
		}
 	}

	// if mouse coords are outside the briefing screen, then go away
	my -= bscreen.map_y1;
	mx -= bscreen.map_x1;
	if ( my < 0 || mx < 0 || mx > (bscreen.map_x2-bscreen.map_x1+1) || my > (bscreen.map_y2-bscreen.map_y1+1) )
		return;

	for ( i = 0; i < bs->num_icons; i++ ) {
		bi = &bs->icons[i];
		brief_common_get_icon_dimensions(&iw, &ih, bi->type, bi->ship_class);
		if ( mx < bi->x ) continue;
		if ( mx > (bi->x + iw) ) continue;
		if ( my < bi->y ) continue;
		if ( my > (bi->y + ih) ) continue;
		// if we've got here, must be a hit
		break;
	}

	if ( i == bs->num_icons ) {
		brief_turn_off_closeup_icon();
		return;
	}

	if ( brief_setup_closeup(bi) == 0 ) {
		gamesnd_play_iface(SND_BRIEF_ICON_SELECT);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// maybe flash a button if player hasn't done anything for a while
void brief_maybe_flash_button()
{
	UI_BUTTON *b;

	if ( Num_brief_stages <= 0 ) 
		return;

	if ( Closeup_icon != NULL ) {
		common_flash_button_init();
		return;
	}

	if ( common_flash_bright() ) {
		if ( Current_brief_stage == (Num_brief_stages-1) ) {
			// AL 30-3-98: Don't flash ship selection button if in a training mission, 
			if ( brief_only_allow_briefing() ) {
				return;
			}

			b = &Common_buttons[Current_screen-1][gr_screen.res][1].button;		// ship select button
		} else {
			b = &Brief_buttons[gr_screen.res][1].button;		// next stage button
		}

		if ( b->button_hilighted() ) {
			common_flash_button_init();
		} else {
			b->draw_forced(1);
		}
	}
}

// -------------------------------------------------------------------------------------
// brief_do_frame()
//
// frametime is in seconds
//
void brief_do_frame(float frametime)
{
	int k, brief_choice;

	if ( red_alert_mission() ) {
		return;
	}


	if ( !Brief_inited ){
		brief_init();
	}

	// commit if skipping briefing, but not in multi - Goober5000
	if (!(Game_mode & GM_MULTIPLAYER))
	{
		if (The_mission.flags & MISSION_FLAG_NO_BRIEFING)
		{
			commit_pressed();
			return;
		}
	}

	int snazzy_action = -1;
	brief_choice = snazzy_menu_do(Brief_ui_mask_data, Brief_ui_mask_w, Brief_ui_mask_h, Num_briefing_regions, Briefing_select_region, &snazzy_action, 0);

	k = common_select_do(frametime);

	if ( Closeup_icon ) {
		Brief_mouse_up_flag = 0;
	}

	if ( help_overlay_active(BR_OVERLAY) ) {
		common_flash_button_init();
		brief_turn_off_closeup_icon();
	}

	// Check common keypresses
	common_check_keys(k);

#ifndef NDEBUG
	int cam_change = 0;
#endif

	if ( Briefing->num_stages > 0 ) {

		// check for special keys
		switch(k) {

#ifndef NDEBUG			
			case KEY_CTRLED | KEY_PAGEUP: {
				if (Closeup_icon->ship_class) {
					Closeup_icon->ship_class--;

					ship_info *sip = &Ship_info[Closeup_icon->ship_class];
					if (sip->model_num < 0)
						sip->model_num = model_load(sip->pof_file, 0, NULL);

					mprintf(("Shiptype = %d (%s)\n", Closeup_icon->ship_class, sip->name));
					mprintf(("Modelnum = %d (%s)\n", sip->model_num, sip->pof_file));
					brief_setup_closeup(Closeup_icon);
				}

				break;
			}

			case KEY_CTRLED | KEY_PAGEDOWN: {
				if (Closeup_icon->ship_class < Num_ship_classes - 1) {
					Closeup_icon->ship_class++;

					ship_info *sip = &Ship_info[Closeup_icon->ship_class];
					if (sip->model_num < 0)
						sip->model_num = model_load(sip->pof_file, 0, NULL);

					mprintf(("Shiptype = %d (%s)\n", Closeup_icon->ship_class, sip->name));
					mprintf(("Modelnum = %d (%s)\n", sip->model_num, sip->pof_file));
					brief_setup_closeup(Closeup_icon);
				}

				break;
			}

			case KEY_A:
				Closeup_cam_pos.xyz.z += 1;
				cam_change = 1;
				break;

			case KEY_A + KEY_SHIFTED:
				Closeup_cam_pos.xyz.z += 10;
				cam_change = 1;
				break;

			case KEY_Z:
				Closeup_cam_pos.xyz.z -= 1;
				cam_change = 1;
				break;

			case KEY_Z + KEY_SHIFTED:
				Closeup_cam_pos.xyz.z -= 10;
				cam_change = 1;
				break;
			
			case KEY_Y:
				Closeup_cam_pos.xyz.y += 1;
				cam_change = 1;
				break;

			case KEY_Y + KEY_SHIFTED:
				Closeup_cam_pos.xyz.y += 10;
				cam_change = 1;
				break;

			case KEY_H:
				Closeup_cam_pos.xyz.y -= 1;
				cam_change = 1;
				break;

			case KEY_H + KEY_SHIFTED:
				Closeup_cam_pos.xyz.y -= 10;
				cam_change = 1;
				break;

			case KEY_COMMA:
				Closeup_zoom -= 0.1f;
				if ( Closeup_zoom < 0.1 ) 
					Closeup_zoom = 0.1f;
				cam_change = 1;
				break;

			case KEY_COMMA+KEY_SHIFTED:
				Closeup_zoom -= 0.5f;
				if ( Closeup_zoom < 0.1 ) 
					Closeup_zoom = 0.1f;
				cam_change = 1;
				break;

			case KEY_PERIOD:
				Closeup_zoom += 0.1f;
				cam_change = 1;
				break;

			case KEY_PERIOD+KEY_SHIFTED:
				Closeup_zoom += 0.5f;
				cam_change = 1;
				break;
#endif
			case 1000:		// need this to avoid warning about no case
				break;

			default:
				break;
		} // end switch
	}

#ifndef NDEBUG
	if ( cam_change ) {
		nprintf(("General","Camera pos: %.2f, %.2f %.2f // ", Closeup_cam_pos.xyz.x, Closeup_cam_pos.xyz.y, Closeup_cam_pos.xyz.z));
		nprintf(("General","Camera zoom: %.2f\n", Closeup_zoom));
	}
#endif

	if ( brief_choice > -1 && snazzy_action == SNAZZY_OVER ) {
		Brief_mouse_up_flag = 0;
		brief_choice = -1;
	}


	common_check_buttons();
	// if ( Briefing->num_stages > 0 )
	brief_check_buttons();

	if ( brief_choice != -1 ) {
		Brief_mouse_up_flag = 0;
	}

	gr_reset_clip();

	common_render(frametime);

	if ( Current_brief_stage < (Num_brief_stages-1) ) {
		if ( !help_overlay_active(BR_OVERLAY) && brief_time_to_advance(Current_brief_stage, frametime) ) {
			brief_do_next_pressed(0);
			common_flash_button_init();
			Brief_last_auto_advance = timer_get_milliseconds();
		}
	}

	if ( !Background_playing ) {
		int time = -1;
		int check_jump_flag = 1;
		if ( Current_brief_stage != Last_brief_stage ) {

			// Check if we have a quick transition pending
			if ( Quick_transition_stage != -1 ) {
					Quick_transition_stage = -1;
					brief_reset_last_new_stage();
					time = 0;
					check_jump_flag = 0;
			}

			if ( check_jump_flag ) {
				if ( abs(Current_brief_stage - Last_brief_stage) > 1 ) {
					Quick_transition_stage = Current_brief_stage;
					Current_brief_stage = Last_brief_stage;
					Assert(Current_brief_stage >= 0);
					Start_fade_up_anim = 1;
					goto Transition_done;
				}
			}

			if ( time != 0 ) {
				if ( Current_brief_stage > Last_brief_stage ) {
					if ( Briefing->stages[Last_brief_stage].flags & BS_FORWARD_CUT ) {
						Quick_transition_stage = Current_brief_stage;
						Current_brief_stage = Last_brief_stage;
						Assert(Current_brief_stage >= 0);
						Start_fade_up_anim = 1;
						goto Transition_done;
					} else {
						time = Briefing->stages[Current_brief_stage].camera_time;
					}
				}
				else {
					if ( Briefing->stages[Last_brief_stage].flags & BS_BACKWARD_CUT ) { 
						Quick_transition_stage = Current_brief_stage;
						Current_brief_stage = Last_brief_stage;
						Assert(Current_brief_stage >= 0);
						Start_fade_up_anim = 1;
						goto Transition_done;
					} else {
						time = Briefing->stages[Last_brief_stage].camera_time;
					}
				}
			}

			brief_voice_stop(Last_brief_stage);
			fsspeech_stop();

			if ( Current_brief_stage < 0 ) {
				Int3();
				Current_brief_stage=0;
			}

			// set the camera target
			brief_set_new_stage(&Briefing->stages[Current_brief_stage].camera_pos,
									  &Briefing->stages[Current_brief_stage].camera_orient,
									  time, Current_brief_stage);

			Brief_playing_fade_sound = 0;
			Last_brief_stage = Current_brief_stage;
			brief_reset_icons(Current_brief_stage);
			brief_update_closeup_icon(0);
		}

		Transition_done:

		if ( Brief_mouse_up_flag && !Closeup_icon) {
			brief_check_for_anim();
		}

		brief_render(frametime);
		brief_camera_move(frametime, Current_brief_stage);

		if (Closeup_icon && (Closeup_bitmap >= 0)) {
			// blit closeup background
			gr_set_bitmap(Closeup_bitmap);
			gr_bitmap(Closeup_coords[gr_screen.res][BRIEF_X_COORD], Closeup_coords[gr_screen.res][BRIEF_Y_COORD]);
		}

		Brief_ui_window.draw();
		brief_redraw_pressed_buttons();
		common_render_selected_screen_button();

		if (Closeup_icon) {
			brief_render_closeup(Closeup_icon->ship_class, frametime);
		}

		// render some extra stuff in multiplayer
		if (Game_mode & GM_MULTIPLAYER) {
			// should render this last so that it overlaps all controls
			chatbox_render();

			// render the status indicator for the voice system
			multi_common_voice_display_status();

			// maybe blit the multiplayer "locked" button	
			// if its locked, everyone blits it as such
			if(multi_ts_is_locked()){
				Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.draw_forced(2);
			} 
			// anyone who can't hit the button sees it off, otherwise
			else {
				if( ((Netgame.type_flags & NG_TYPE_TEAM) && !(Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN)) ||
					 ((Netgame.type_flags & NG_TYPE_TEAM) && !(Net_player->flags & NETINFO_FLAG_GAME_HOST)) ){
					Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.draw_forced(0);
				} else {
					Brief_buttons[gr_screen.res][BRIEF_BUTTON_MULTI_LOCK].button.draw();
				}
			}
		}
	}		

	// maybe flash a button if player hasn't done anything for a while
	brief_maybe_flash_button();

	// blit help overlay if active
	help_overlay_maybe_blit(BR_OVERLAY);	

	gr_flip();	

	// If the commit button was pressed, do the commit button actions.  Done at the end of the
	// loop so there isn't a skip in the animation (since ship_create() can take a long time if
	// the ship model is not in memory
	if (Commit_pressed) {
		if (Game_mode & GM_MULTIPLAYER) {
			multi_ts_commit_pressed();
		} else {
			commit_pressed();
		}

		Commit_pressed = 0;
	}
}

// --------------------------------------------------------------------------------------
//	brief_unload_bitmaps()
//
//
void brief_unload_bitmaps()
{	
	if ( Brief_text_bitmap != -1 ) {
		bm_release(Brief_text_bitmap);
		Brief_text_bitmap = -1;
	}

	if(Brief_grid_bitmap != -1){
		bm_release(Brief_grid_bitmap);
		Brief_grid_bitmap = -1;
	}

	if ( Brief_multitext_bitmap != -1 ) {
		bm_release(Brief_multitext_bitmap);
		Brief_multitext_bitmap = -1;
	}

	if ( Brief_background_bitmap != -1 ) {
		bm_release(Brief_background_bitmap);
		Brief_background_bitmap = -1;
	}

	help_overlay_unload(BR_OVERLAY);
}

// ------------------------------------------------------------------------------------
// brief_close()
//
//
void brief_close()
{
	if ( Brief_inited == FALSE ) {
		nprintf(("Warning","brief_close() returning without doing anything\n"));
		return;
	}

	nprintf(("Alan", "Entering brief_close()\n"));

	ML_objectives_close();
	fsspeech_stop();

	// unload the audio streams used for voice playback
	brief_voice_unload_all();

	bm_unload(Fade_anim.first_frame);

	Brief_ui_window.destroy();

	// unload the bitmaps
	brief_unload_bitmaps();

	brief_common_close();

	Briefing_paused = 0;

	Brief_inited = FALSE;
}

void briefing_stop_music()
{
	if ( Briefing_music_handle != -1 ) {
		audiostream_close_file(Briefing_music_handle, 1);
		Briefing_music_handle = -1;
	}
}

void briefing_load_music(char* fname)
{
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	if ( Briefing_music_handle != -1 )
		return;

	if ( fname )
		Briefing_music_handle = audiostream_open( fname, ASF_MENUMUSIC );
}

void briefing_start_music()
{
	if ( Briefing_music_handle != -1 ) {
		if ( !audiostream_is_playing(Briefing_music_handle) )
			audiostream_play(Briefing_music_handle, Master_event_music_volume, 1);
	}
	else {
		nprintf(("Warning", "No music file exists to play music at this briefing!\n"));
	}
}

void brief_stop_voices()
{
	brief_voice_stop(Current_brief_stage);
}

void brief_pause()
{
	if (Briefing_paused)
		return;

	Briefing_paused = 1;

	if (Briefing_music_handle >= 0) {
		audiostream_pause(Briefing_music_handle);
	}

	brief_voice_pause(Current_brief_stage);
}

void brief_unpause()
{
	if (!Briefing_paused)
		return;

	Briefing_paused = 0;

	if (Briefing_music_handle >= 0) {
		audiostream_unpause(Briefing_music_handle);
	}

	brief_voice_unpause(Current_brief_stage);
}

void brief_maybe_blit_scene_cut(float frametime)
{
	if ( Start_fade_up_anim ) {
		int framenum;

		Fade_anim.time_elapsed += frametime;

		if ( !Brief_playing_fade_sound ) {
			gamesnd_play_iface(SND_BRIEFING_STATIC);					
			Brief_playing_fade_sound = 1;
		}

		if ( Fade_anim.time_elapsed > Fade_anim.total_time ) {
			Fade_anim.time_elapsed = 0.0f;
			Start_fade_up_anim = 0;
			Start_fade_down_anim = 1;
			Current_brief_stage = Quick_transition_stage;
		
			if ( Current_brief_stage < 0 ) {
				brief_transition_reset();
				Current_brief_stage = Last_brief_stage;
			}

			Assert(Current_brief_stage >= 0);			
			goto Fade_down_anim_start;
		}

		// draw the correct frame of animation
		framenum = fl2i( (Fade_anim.time_elapsed * Fade_anim.num_frames) / Fade_anim.total_time );
		if ( framenum < 0 )
			framenum = 0;
		if ( framenum >= Fade_anim.num_frames )
			framenum = Fade_anim.num_frames-1;

		// Blit the bitmap for this frame
		gr_set_bitmap(Fade_anim.first_frame + framenum);
		gr_bitmap(Fade_anim.sx, Fade_anim.sy);
	}


	Fade_down_anim_start:
	if ( Start_fade_down_anim ) {
		int framenum;

		Fade_anim.time_elapsed += frametime;

		if ( Fade_anim.time_elapsed > Fade_anim.total_time ) {
			Fade_anim.time_elapsed = 0.0f;
			Start_fade_up_anim = 0;
			Start_fade_down_anim = 0;
			return;
		}

		// draw the correct frame of animation
		framenum = fl2i( (Fade_anim.time_elapsed * Fade_anim.num_frames) / Fade_anim.total_time );
		if ( framenum < 0 )
			framenum = 0;
		if ( framenum >= Fade_anim.num_frames )
			framenum = Fade_anim.num_frames-1;

		// Blit the bitmap for this frame
		gr_set_bitmap(Fade_anim.first_frame + (Fade_anim.num_frames-1) - framenum);
		gr_bitmap(Fade_anim.sx, Fade_anim.sy);
	}
}

void brief_transition_reset()
{
	Quick_transition_stage = -1;
	Start_fade_up_anim = 0;
	Start_fade_down_anim = 0;
	Fade_anim.time_elapsed = 0.0f;
}

// return 1 if this mission only allow players to use the briefing (and not ship or 
// weapon loadout).  Otherwise return 0.
int brief_only_allow_briefing()
{
	if ( The_mission.game_type & MISSION_TYPE_TRAINING ) {
		return 1;
	}

	if ( The_mission.flags & (MISSION_FLAG_SCRAMBLE | MISSION_FLAG_RED_ALERT) ) {
		return 1;
	}

	return 0;
}
