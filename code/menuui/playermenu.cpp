/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#include <limits.h>
#include <ctype.h>


#include "menuui/playermenu.h"
#include "ui/ui.h"
#include "gamesnd/gamesnd.h"
#include "playerman/player.h"
#include "io/key.h"
#include "playerman/managepilot.h"
#include "pilotfile/pilotfile.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "cmdline/cmdline.h"
#include "osapi/osregistry.h"
#include "menuui/mainhallmenu.h"
#include "popup/popup.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "mission/missioncampaign.h"
#include "parse/parselo.h"
#include "cfile/cfile.h"
#include "network/multi.h"
#include "debugconsole/console.h"


// --------------------------------------------------------------------------------------------------------
// PLAYER SELECT defines
//

int Max_lines;  //Max number of pilots displayed in Window. Gets set in player_select_draw_list()

// button control defines
#define NUM_PLAYER_SELECT_BUTTONS	8		// button control defines

#define CREATE_PILOT_BUTTON			0		//
#define CLONE_BUTTON				1		//
#define DELETE_BUTTON				2		//
#define SCROLL_LIST_UP_BUTTON		3		//
#define SCROLL_LIST_DOWN_BUTTON		4		//
#define ACCEPT_BUTTON				5		//
#define SINGLE_BUTTON				6		//
#define MULTI_BUTTON				7		//

// list text display area
int Choose_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		114, 117, 400, 87
	},
	{ // GR_1024
		183, 186, 640, 139
	}
};

char *Player_select_background_bitmap_name[GR_NUM_RESOLUTIONS] = {
	"ChoosePilot",
	"2_ChoosePilot"
};
char *Player_select_background_mask_bitmap[GR_NUM_RESOLUTIONS] = {
	"ChoosePilot-m",
	"2_ChoosePilot-m"
};
// #define PLAYER_SELECT_PALETTE			NOX("ChoosePilotPalette")	// palette for the screen

#define PLAYER_SELECT_MAIN_HALL_OVERLAY		NOX("MainHall1")			// main hall help overlay

// convenient struct for handling all button controls
struct barracks_buttons {
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	barracks_buttons(char *name, int x1, int y1, int xt1, int yt1, int h) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h) {}
};

static barracks_buttons Player_select_buttons[GR_NUM_RESOLUTIONS][NUM_PLAYER_SELECT_BUTTONS] = {
	{ // GR_640
		// create, clone and delete (respectively)
		barracks_buttons("CPB_00",		114,	205,	117,	240,	0),
		barracks_buttons("CPB_01",		172,	205,	175,	240,	1),
		barracks_buttons("CPB_02",		226,	205,	229,	240,	2),

		// scroll up, scroll down, and accept (respectively)
		barracks_buttons("CPB_03",		429,	213,	 -1,	 -1,	3),
		barracks_buttons("CPB_04",		456,	213,	 -1,	 -1,	4),
		barracks_buttons("CPB_05",		481,	207,	484,	246,	5),
		
		// single player select and multiplayer select, respectively
		barracks_buttons("CPB_06",		428,	 82,	430,	108,	6),
		barracks_buttons("CPB_07",		477,	 82,	481,	108,	7)
	}, 
	{ // GR_1024
		// create, clone and delete (respectively)
		barracks_buttons("2_CPB_00",	182,	328,	199,	384,	0),
		barracks_buttons("2_CPB_01",	275,	328,	292,	384,	1),
		barracks_buttons("2_CPB_02",	361,	328,	379,	384,	2),

		// scroll up, scroll down, and accept (respectively)
		barracks_buttons("2_CPB_03",	686,	341,	 -1,	 -1,	3),
		barracks_buttons("2_CPB_04",	729,	341,	 -1,	 -1,	4),
		barracks_buttons("2_CPB_05",	770,	332,	787,	394,	5),
		
		// single player select and multiplayer select, respectively
		barracks_buttons("2_CPB_06",	685,	132,	700,	173,	6),
		barracks_buttons("2_CPB_07",	764,	132,	782,	173,	7)
	}
};

// FIXME add to strings.tbl
#define PLAYER_SELECT_NUM_TEXT			1
UI_XSTR Player_select_text[GR_NUM_RESOLUTIONS][PLAYER_SELECT_NUM_TEXT] = {
	{ // GR_640
		{ "Choose Pilot",	1436,	122,	90,	UI_XSTR_COLOR_GREEN, -1, NULL }
	}, 
	{ // GR_1024
		{ "Choose Pilot",	1436,	195,	143,	UI_XSTR_COLOR_GREEN, -1, NULL }
	}
};

UI_WINDOW Player_select_window;				// ui window for this screen
UI_BUTTON Player_select_list_region;		// button for detecting mouse clicks on this screen
UI_INPUTBOX Player_select_input_box;		// input box for adding new pilot names

// #define PLAYER_SELECT_PALETTE_FNAME		NOX("InterfacePalette")
int Player_select_background_bitmap;		// bitmap for this screen
// int Player_select_palette;				// palette bitmap for this screen
int Player_select_autoaccept = 0;
// int Player_select_palette_set = 0;

// flag indicating if this is the absolute first pilot created and selected. Used to determine
// if the main hall should display the help overlay screen
int Player_select_very_first_pilot = 0;
int Player_select_initial_count = 0;
char Player_select_very_first_pilot_callsign[CALLSIGN_LEN + 2];

extern int Main_hall_bitmap;						// bitmap handle to the main hall bitmap

int Player_select_mode;								// single or multiplayer - never set directly. use player_select_init_player_stuff()
int Player_select_num_pilots;						// # of pilots on the list
int Player_select_list_start;						// index of first list item to start displaying in the box
int Player_select_pilot;							// index into the Pilot array of which is selected as the active pilot
int Player_select_input_mode;						// 0 if the player _isn't_ typing a callsign, 1 if he is
char Pilots_arr[MAX_PILOTS][MAX_FILENAME_LEN];
char *Pilots[MAX_PILOTS];
int Player_select_clone_flag;						// clone the currently selected pilot
char Player_select_last_pilot[CALLSIGN_LEN + 10];	// callsign of the last used pilot, or none if there wasn't one
int Player_select_last_is_multi;

SCP_string Player_select_force_main_hall = "";

static int Player_select_no_save_pilot = 0;		// to skip save of pilot in pilot_select_close()

int Player_select_screen_active = 0;	// for pilot savefile loading - taylor

// notification text areas

static int Player_select_bottom_text_y[GR_NUM_RESOLUTIONS] = {
	314, // GR_640
	502 // GR_1024
};

static int Player_select_middle_text_y[GR_NUM_RESOLUTIONS] = {
	253, // GR_640
	404 // GR_1024
};

char Player_select_bottom_text[150] = "";
char Player_select_middle_text[150] = "";


// FORWARD DECLARATIONS
void player_select_init_player_stuff(int mode);		// switch between single and multiplayer modes
void player_select_set_input_mode(int n);
void player_select_button_pressed(int n);
void player_select_scroll_list_up();
void player_select_scroll_list_down();
int player_select_create_new_pilot();
void player_select_delete_pilot();
void player_select_display_all_text();
void player_select_display_copyright();
void player_select_set_bottom_text(const char *txt);
void player_select_set_middle_text(const char *txt);
void player_select_set_controls(int gray);
void player_select_draw_list();
void player_select_process_noninput(int k);
void player_select_process_input(int k);
int player_select_pilot_file_filter(const char *filename);
int player_select_get_last_pilot_info();
void player_select_eval_very_first_pilot();
void player_select_commit();
void player_select_cancel_create();

extern int delete_pilot_file(char *pilot_name);

/*
 * validate that a pilot/player was created with the same language FSO is currently using
 *
 * @param pilots callsign
 * @note not longer needed if intel entry "primary keys" change to a non-translated value
 */
bool valid_pilot_lang(char *callsign)
{
	char pilot_lang[LCL_LANG_NAME_LEN+1], current_lang[LCL_LANG_NAME_LEN+1];
	SCP_string filename = callsign;

	filename += ".plr";
	lcl_get_language_name(current_lang);

	if (Pilot.verify(filename.c_str(), NULL, pilot_lang)) {
		if (!strcmp(current_lang, pilot_lang)) {
			return true;
		}
	}
	return false;
}

// basically, gray out all controls (gray == 1), or ungray the controls (gray == 0)
void player_select_set_controls(int gray)
{
	int idx;

	for(idx=0;idx<NUM_PLAYER_SELECT_BUTTONS;idx++) {
		if(gray) {
			Player_select_buttons[gr_screen.res][idx].button.disable();
		} else {
			Player_select_buttons[gr_screen.res][idx].button.enable();
		}
	}
}

// functions for selecting single/multiplayer pilots at the very beginning of FreeSpace
void player_select_init()
{
	int i;
	barracks_buttons *b;
	UI_WINDOW *w;

	// start a looping ambient sound
	main_hall_start_ambient();

	Player_select_force_main_hall = "";

	Player_select_screen_active = 1;

	// create the UI window
	Player_select_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Player_select_window.set_mask_bmap(Player_select_background_mask_bitmap[gr_screen.res]);
	
	// initialize the control buttons
	for (i=0; i<NUM_PLAYER_SELECT_BUTTONS; i++) {
		b = &Player_select_buttons[gr_screen.res][i];

		// create the button
		b->button.create(&Player_select_window, NULL, b->x, b->y, 60, 30, 1, 1);

		// set its highlight action
		b->button.set_highlight_action(common_play_highlight_sound);

		// set its animation bitmaps
		b->button.set_bmaps(b->filename);

		// link the mask hotspot
		b->button.link_hotspot(b->hotspot);
	}

	// add some text
	w = &Player_select_window;
	w->add_XSTR("Create", 1034, Player_select_buttons[gr_screen.res][CREATE_PILOT_BUTTON].xt, Player_select_buttons[gr_screen.res][CREATE_PILOT_BUTTON].yt, &Player_select_buttons[gr_screen.res][CREATE_PILOT_BUTTON].button, UI_XSTR_COLOR_GREEN);	
	w->add_XSTR("Clone", 1040, Player_select_buttons[gr_screen.res][CLONE_BUTTON].xt, Player_select_buttons[gr_screen.res][CLONE_BUTTON].yt, &Player_select_buttons[gr_screen.res][CLONE_BUTTON].button, UI_XSTR_COLOR_GREEN);	
	w->add_XSTR("Remove", 1038, Player_select_buttons[gr_screen.res][DELETE_BUTTON].xt, Player_select_buttons[gr_screen.res][DELETE_BUTTON].yt, &Player_select_buttons[gr_screen.res][DELETE_BUTTON].button, UI_XSTR_COLOR_GREEN);	

	w->add_XSTR("Select", 1039, Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].xt, Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].yt, &Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].button, UI_XSTR_COLOR_PINK);	
	w->add_XSTR("Single", 1041, Player_select_buttons[gr_screen.res][SINGLE_BUTTON].xt, Player_select_buttons[gr_screen.res][SINGLE_BUTTON].yt, &Player_select_buttons[gr_screen.res][SINGLE_BUTTON].button, UI_XSTR_COLOR_GREEN);	
	w->add_XSTR("Multi", 1042, Player_select_buttons[gr_screen.res][MULTI_BUTTON].xt, Player_select_buttons[gr_screen.res][MULTI_BUTTON].yt, &Player_select_buttons[gr_screen.res][MULTI_BUTTON].button, UI_XSTR_COLOR_GREEN);	
	for(i=0; i<PLAYER_SELECT_NUM_TEXT; i++) {
		w->add_XSTR(&Player_select_text[gr_screen.res][i]);
	}


	// create the list button text select region
	Player_select_list_region.create(&Player_select_window, "", Choose_list_coords[gr_screen.res][0], Choose_list_coords[gr_screen.res][1], Choose_list_coords[gr_screen.res][2], Choose_list_coords[gr_screen.res][3], 0, 1);
	Player_select_list_region.hide();

	// create the pilot callsign input box
	Player_select_input_box.create(&Player_select_window, Choose_list_coords[gr_screen.res][0], Choose_list_coords[gr_screen.res][1], Choose_list_coords[gr_screen.res][2] , CALLSIGN_LEN - 1, "", UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_KEYTHRU | UI_INPUTBOX_FLAG_LETTER_FIRST);
	Player_select_input_box.set_valid_chars(VALID_PILOT_CHARS);
	Player_select_input_box.hide();
	Player_select_input_box.disable();

	// not currently entering any text
	Player_select_input_mode = 0;

	// set up hotkeys for buttons so we draw the correct animation frame when a key is pressed
	Player_select_buttons[gr_screen.res][SCROLL_LIST_UP_BUTTON].button.set_hotkey(KEY_UP);
	Player_select_buttons[gr_screen.res][SCROLL_LIST_DOWN_BUTTON].button.set_hotkey(KEY_DOWN);
	Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].button.set_hotkey(KEY_ENTER);
	Player_select_buttons[gr_screen.res][CREATE_PILOT_BUTTON].button.set_hotkey(KEY_C);

	// attempt to load in the background bitmap
	Player_select_background_bitmap = bm_load(Player_select_background_bitmap_name[gr_screen.res]);
	Assert(Player_select_background_bitmap >= 0);

	// load in the palette for the screen
	// Player_select_palette = bm_load(PLAYER_SELECT_PALETTE);
	// Player_select_palette_set = 0;

	// unset the very first pilot data
	Player_select_very_first_pilot = 0;
	Player_select_initial_count = -1;
	memset(Player_select_very_first_pilot_callsign, 0, CALLSIGN_LEN + 2);

//	if(Player_select_num_pilots == 0){
//		Player_select_autoaccept = 1;
//	}

// if we found a pilot
	if ( player_select_get_last_pilot_info() ) {
		if (Player_select_last_is_multi && !Networking_disabled) {
			player_select_init_player_stuff(PLAYER_SELECT_MODE_MULTI);
		} else {
			player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);
		}
	} else { // otherwise go to the single player mode by default
		player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);
	}

	if ( (Player_select_num_pilots == 1) && Player_select_input_mode ) {
		Player_select_autoaccept = 1;
	}
}

// no need to reset this to false because we only ever see player_select once per game run
static bool Startup_warning_dialog_displayed = false;

void player_select_do()
{
	int k;

	// Goober5000 - display a popup warning about problems in the mod
	if ((Global_warning_count > 10 || Global_error_count > 0) && !Startup_warning_dialog_displayed) {
		char text[512];
		sprintf(text, "Warning!\n\nThe currently active mod has generated %d warnings and/or errors during program startup.  These could have been caused by anything from incorrectly formated table files to corrupt models.  While FreeSpace Open will attempt to compensate for these issues, it cannot guarantee a trouble-free gameplay experience.  Source Code Project staff cannot provide assistance or support for these problems, as they are caused by the mod's data files, not FreeSpace Open's source code.", Global_warning_count + Global_error_count);
		popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, text);
		Startup_warning_dialog_displayed = true;
	}
		
	// set the input box at the "virtual" line 0 to be active so the player can enter a callsign
	if (Player_select_input_mode) {
		Player_select_input_box.set_focus();
	}

	// process any ui window stuff
	k = Player_select_window.process();

	if (k) {
		extern void game_process_cheats(int k);
		game_process_cheats(k);
	}

	switch (k) {
		// switch between single and multiplayer modes
		case KEY_TAB: {
			if (Player_select_input_mode) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			// play a little sound
			gamesnd_play_iface(SND_USER_SELECT);

			if (Player_select_mode == PLAYER_SELECT_MODE_MULTI) {
				player_select_set_bottom_text(XSTR( "Single-Player Mode", 376));

				// reinitialize as single player mode
				player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);
			} else if (Player_select_mode == PLAYER_SELECT_MODE_SINGLE) {
				player_select_set_bottom_text(XSTR( "Multiplayer Mode", 377));

				// reinitialize as multiplayer mode
				player_select_init_player_stuff(PLAYER_SELECT_MODE_MULTI);
			}

			break;
		}

		case KEY_ESC: {
			// we can hit ESC to get out of text input mode, and we don't want
			// to set this var in that case since it will crash on a NULL Player
			// ptr when going to the mainhall
			if ( !Player_select_input_mode ) {
				Player_select_no_save_pilot = 1;
			}

			break;
		}
	}

	// draw the player select pseudo-dialog over it
	GR_MAYBE_CLEAR_RES(Player_select_background_bitmap);
	gr_set_bitmap(Player_select_background_bitmap);
	gr_bitmap(0,0,GR_RESIZE_MENU);

	// press the accept button
	if (Player_select_autoaccept) {
		Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].button.press_button();
	}
	
	// draw any ui window stuf
	Player_select_window.draw();

	// light up the correct mode button (single or multi)
	if (Player_select_mode == PLAYER_SELECT_MODE_SINGLE) {
		Player_select_buttons[gr_screen.res][SINGLE_BUTTON].button.draw_forced(2);
	} else {
		Player_select_buttons[gr_screen.res][MULTI_BUTTON].button.draw_forced(2);
	}

	// draw the pilot list text
	player_select_draw_list();

	// draw copyright message on the bottom on the screen
	player_select_display_copyright();

	if (!Player_select_input_mode) {
		player_select_process_noninput(k);
	} else {
		player_select_process_input(k);
	}
	
	// draw any pending messages on the bottom or middle of the screen
	player_select_display_all_text();

	gr_flip();
}

void player_select_close()
{
	// destroy the player select window
	Player_select_window.destroy();

	// if we're in input mode - we should undo the pilot create reqeust
	if(Player_select_input_mode) {
		player_select_cancel_create();
	}

	// if we are just exiting then don't try to save any pilot files - taylor
	if (Player_select_no_save_pilot) {
		Player = NULL;
		return;
	}

	// actually set up the Player struct here
	if ( (Player_select_pilot == -1) || (Player_select_num_pilots == 0) ) {
		nprintf(("General","WARNING! No pilot selected! We should be exiting the game now!\n"));
		return;
	}

	// unload all bitmaps
	if(Player_select_background_bitmap >= 0) {
		bm_release(Player_select_background_bitmap);
		Player_select_background_bitmap = -1;
	} 
	// if(Player_select_palette >= 0){
	// 	bm_release(Player_select_palette);
		//Player_select_palette = -1;
	// }

	// setup the player  struct
	Player_num = 0;
	Player = &Players[0];
	Player->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;

	// New pilot file makes no distinction between multi pilots and regular ones, so let's do this here.
	if (Player_select_mode == PLAYER_SELECT_MODE_MULTI) {
		Player->flags |= PLAYER_FLAGS_IS_MULTI;
	}

	// WMC - Set appropriate game mode
	if ( Player->flags & PLAYER_FLAGS_IS_MULTI ) {
		Game_mode = GM_MULTIPLAYER;
	} else {
		Game_mode = GM_NORMAL;
	}

	// now read in a the pilot data
	if ( !Pilot.load_player(Pilots[Player_select_pilot], Player) ) {
		Error(LOCATION,"Couldn't load pilot file, bailing");
		Player = NULL;
	} else {
		// NOTE: this may fail if there is no current campaign, it's not fatal
		Pilot.load_savefile(Player->current_campaign);
	}

	if (Player_select_force_main_hall != "") {
		main_hall_init(Player_select_force_main_hall);
	}

	// free memory from all parsing so far, all tbls found during game_init()
	// and the current campaign which we loaded here
	stop_parse();

	Player_select_screen_active = 0;
}

void player_select_set_input_mode(int n)
{
	int i;

	// set the input mode
	Player_select_input_mode = n;

	// enable all the player select buttons
	for (i=0; i<NUM_PLAYER_SELECT_BUTTONS; i++) {
		Player_select_buttons[gr_screen.res][i].button.enable(!n);
	}

	Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].button.set_hotkey(n ? -1 : KEY_ENTER);
	Player_select_buttons[gr_screen.res][CREATE_PILOT_BUTTON].button.set_hotkey(n ? -1 : KEY_C);

	// enable the player select input box
	if (Player_select_input_mode) {
		Player_select_input_box.enable();
		Player_select_input_box.unhide();
	} else {
		Player_select_input_box.hide();
		Player_select_input_box.disable();
	}
}

void player_select_button_pressed(int n)
{
	int ret;

	switch (n) {
	case SCROLL_LIST_UP_BUTTON:
		player_select_set_bottom_text("");

		player_select_scroll_list_up();
		break;

	case SCROLL_LIST_DOWN_BUTTON:
		player_select_set_bottom_text("");

		player_select_scroll_list_down();
		break;

	case ACCEPT_BUTTON:
		// make sure he has a valid pilot selected
		if (Player_select_pilot < 0) {
			popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR( "You must select a valid pilot first", 378));
		} else {
			if (valid_pilot_lang(Pilots[Player_select_pilot])) {
				player_select_commit();
			} else {
				popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR(
					"Selected pilot was created with a different language\n"
					"to the currently active language.\n\n"
					"Please select a different pilot or change the language", 1637));
			}
		}
		break;

	case CLONE_BUTTON:
		// if we're at max-pilots, don't allow another to be added
		if (Player_select_num_pilots >= MAX_PILOTS) {
			player_select_set_bottom_text(XSTR( "You already have the maximum # of pilots!", 379));

			gamesnd_play_iface(SND_GENERAL_FAIL);
			break;
		}

		if (Player_select_pilot >= 0) {
			// first we have to make sure this guy is actually loaded for when we create the clone
			if (Player == NULL) {
				Player = &Players[0];
				Player->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;
			}

			// attempt to read in the pilot file of the guy to be cloned
			if ( !Pilot.load_player(Pilots[Player_select_pilot], Player) ) {
				Error(LOCATION,"Couldn't load pilot file, bailing");
				Player = NULL;
				Int3();
			}

			// set the clone flag
			Player_select_clone_flag = 1;

			// create the new pilot (will be cloned with Player_select_clone_flag_set)
			if ( !player_select_create_new_pilot() ) {
				player_select_set_bottom_text(XSTR( "Error creating new pilot file!", 380));
				Player_select_clone_flag = 0;
				Player->reset();
				Player = NULL;
				break;
			}

			// display some text on the bottom of the dialog
			player_select_set_bottom_text(XSTR( "Type Callsign and Press Enter", 381));
			
			// gray out all controls in the dialog
			player_select_set_controls(1);
		}
		break;

	case CREATE_PILOT_BUTTON:
		// if we're at max-pilots, don't allow another to be added
		if(Player_select_num_pilots >= MAX_PILOTS) {
			player_select_set_bottom_text(XSTR( "You already have the maximum # of pilots!", 379));

			gamesnd_play_iface(SND_GENERAL_FAIL);
			break;
		}

		// create a new pilot
		if ( !player_select_create_new_pilot() ) {
			player_select_set_bottom_text(XSTR( "Type Callsign and Press Enter", 381));
		}

		// don't clone anyone
		Player_select_clone_flag = 0;

		// display some text on the bottom of the dialog
		player_select_set_bottom_text(XSTR( "Type Callsign and Press Enter", 381));

		// gray out all controls
		player_select_set_controls(1);
		break;

	case DELETE_BUTTON:
		player_select_set_bottom_text("");

		if (Player_select_pilot >= 0) {
			if (Player_select_mode == PLAYER_SELECT_MODE_MULTI) {
				popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("Disabled!\n\nMulti and single player pilots are now identical. "
							"Deleting a multi-player pilot will also delete all single-player data for that pilot.\n\nAs a safety precaution, pilots can only be "
							"deleted from the single-player menu.", 1610));
			} else {
				// display a popup requesting confirmation
				ret = popup(PF_TITLE_BIG | PF_TITLE_RED, 2, POPUP_NO, POPUP_YES, XSTR( "Warning!\n\nAre you sure you wish to delete this pilot?", 382));

				// delete the pilot
				if (ret == 1) {
					player_select_delete_pilot();
				}
			}
		}
		break;

	case SINGLE_BUTTON:
		player_select_set_bottom_text("");

		Player_select_autoaccept = 0;
		// switch to single player mode
		if (Player_select_mode != PLAYER_SELECT_MODE_SINGLE) {
			// play a little sound
			gamesnd_play_iface(SND_USER_SELECT);

			player_select_set_bottom_text(XSTR( "Single Player Mode", 376));

			// reinitialize as single player mode
			player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	case MULTI_BUTTON:
		player_select_set_bottom_text("");

		Player_select_autoaccept = 0;
		if ( Networking_disabled ) {
			game_feature_disabled_popup();
			break;
		}

		// switch to multiplayer mode
		if (Player_select_mode != PLAYER_SELECT_MODE_MULTI) {
			// play a little sound
			gamesnd_play_iface(SND_USER_SELECT);

			player_select_set_bottom_text(XSTR( "Multiplayer Mode", 377));

			// reinitialize as multiplayer mode
			player_select_init_player_stuff(PLAYER_SELECT_MODE_MULTI);
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;
	}
}

int player_select_create_new_pilot()
{
	int idx;

	// make sure we haven't reached the max
	if (Player_select_num_pilots >= MAX_PILOTS) {
		gamesnd_play_iface(SND_GENERAL_FAIL);
		return 0;
	}

	int play_scroll_sound = 1;

	if ( play_scroll_sound ) {
		gamesnd_play_iface(SND_SCROLL);
	}

	idx = Player_select_num_pilots;	

	// move all the pilots in the list up
	while (idx--) {
		strcpy(Pilots[idx + 1], Pilots[idx]);
	}

	// by default, set the default netgame protocol to be VMT
	Multi_options_g.protocol = NET_TCP;

	// select the beginning of the list
	Player_select_pilot = 0;
	Player_select_num_pilots++;
	Pilots[Player_select_pilot][0] = 0;
	Player_select_list_start= 0;

	// set us to be in input mode
	player_select_set_input_mode(1);

	// set the input box to have focus
	Player_select_input_box.set_focus();
	Player_select_input_box.set_text("");
	Player_select_input_box.update_dimensions(Choose_list_coords[gr_screen.res][0], Choose_list_coords[gr_screen.res][1], Choose_list_coords[gr_screen.res][2], gr_get_font_height());	

	return 1;
}

void player_select_delete_pilot()
{
	char filename[MAX_PATH_LEN + 1];
	int i, del_rval;

	// tack on the full path and the pilot file extension
	// build up the path name length
	// make sure we do this based upon whether we're in single or multiplayer mode
	strcpy_s( filename, Pilots[Player_select_pilot] );

	del_rval = delete_pilot_file(filename);

	if ( !del_rval ) {
		popup(PF_USE_AFFIRMATIVE_ICON | PF_TITLE_BIG | PF_TITLE_RED, 1, POPUP_OK, XSTR("Error\nFailed to delete pilot file. File may be read-only.", 1599));
		return;
	}

	// move all the players down
	for ( i=Player_select_pilot; i<Player_select_num_pilots-1; i++ ) {
		strcpy(Pilots[i], Pilots[i + 1]);
	}

	// correcly set the # of pilots and the currently selected pilot
	Player_select_num_pilots--;
	if (Player_select_pilot >= Player_select_num_pilots) {
		Player_select_pilot = Player_select_num_pilots - 1;
	}

}

// scroll the list of players up
void player_select_scroll_list_up()
{
	if (Player_select_pilot == -1) {
		return;
	}

	// change the pilot selected index and play the appropriate sound
	if (Player_select_pilot) {
		Player_select_pilot--;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}

	if (Player_select_pilot < Player_select_list_start) {
		Player_select_list_start = Player_select_pilot;
	}
}

// scroll the list of players down
void player_select_scroll_list_down()
{
	// change the pilot selected index and play the appropriate sound
	if ( Player_select_pilot < Player_select_num_pilots - 1 ) {
		Player_select_pilot++;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}

	if ( Player_select_pilot >= (Player_select_list_start + Max_lines) ) {
		Player_select_list_start++;
	}
}

// fill in the data on the last played pilot (callsign and is_multi or not)
int player_select_get_last_pilot_info()
{
	// TODO: Replace this with a function that does this properly for the new pilot code.

	const char *last_player = os_config_read_string( NULL, "LastPlayer", NULL);

	if (last_player == NULL) {
		return 0;
	} else {
		strcpy_s(Player_select_last_pilot, last_player);
	}

	// handle changing from pre-pilot code to post-pilot code
	if (Player_select_last_pilot[strlen(Player_select_last_pilot)-1] == 'M' || Player_select_last_pilot[strlen(Player_select_last_pilot)-1] == 'S') {
		Player_select_last_pilot[strlen(Player_select_last_pilot)-1]='\0';	// chop off last char, M|P
	}

	if ( !Pilot.load_player(Player_select_last_pilot, Player) ) {
		Player_select_last_is_multi = 0;
	} else {
		Player_select_last_is_multi = Player->player_was_multi;
	}

	return 1;
}

int player_select_get_last_pilot()
{
	// if the player has the Cmdline_use_last_pilot command line option set, try and drop out quickly
	if (Cmdline_use_last_pilot) {
		int idx;

		if ( !player_select_get_last_pilot_info() ) {
			return 0;
		}

		Get_file_list_filter = player_select_pilot_file_filter;

		Player_select_num_pilots = cf_get_file_list_preallocated(MAX_PILOTS, Pilots_arr, Pilots, CF_TYPE_PLAYERS, NOX("*.plr"), CF_SORT_TIME);

		Player_select_pilot = -1;
		idx = 0;
		// pick the last player
		for (idx=0;idx<Player_select_num_pilots;idx++) {
			if (strcmp(Player_select_last_pilot,Pilots_arr[idx])==0) {
				Player_select_pilot = idx;
				break;
			}
		}

		// set this so that we don't incorrectly create a "blank" pilot - .plr
		// in the player_select_close() function
		Player_select_num_pilots = 0;

		// if we've actually found a valid pilot, load him up
		if (Player_select_pilot != -1) {
			Player = &Players[0];
			Pilot.load_player(Pilots_arr[idx], Player);
			Player->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;
			return 1;
		}
	}

	return 0;
}

void player_select_init_player_stuff(int mode)
{
	Player_select_list_start = 0;

	// set the select mode to single player for default
	Player_select_mode = mode;

	// load up the list of players based upon the Player_select_mode (single or multiplayer)
	Get_file_list_filter = player_select_pilot_file_filter;

	Player_select_num_pilots = cf_get_file_list_preallocated(MAX_PILOTS, Pilots_arr, Pilots, CF_TYPE_PLAYERS, NOX("*.plr"), CF_SORT_TIME);

	// if we have a "last_player", and they're in the list, bash them to the top of the list
	if (Player_select_last_pilot[0] != '\0') {
		int i,j;
		for (i = 0; i < Player_select_num_pilots; ++i) {
			if (!stricmp(Player_select_last_pilot,Pilots[i])) {
				break;
			}
		}
		if (i != Player_select_num_pilots) {
			for (j = i; j > 0; --j) {
				strncpy(Pilots[j], Pilots[j-1], strlen(Pilots[j-1])+1);
			}
			strncpy(Pilots[0], Player_select_last_pilot, strlen(Player_select_last_pilot)+1);
		}
	}

	Player = NULL;

	// if this value is -1, it means we should set it to the num pilots count
	if (Player_select_initial_count == -1) {
		Player_select_initial_count = Player_select_num_pilots;
	}

	// select the first pilot if any exist, otherwise set to -1
	if (Player_select_num_pilots == 0) {
		Player_select_pilot = -1;
		player_select_set_middle_text(XSTR( "Type Callsign and Press Enter", 381));
		player_select_set_controls(1);		// gray out the controls
		player_select_create_new_pilot();
	} else {
		Player_select_pilot = 0;
	}
}

void player_select_draw_list()
{
	int idx;

	if (gr_screen.res == 1) {
		Max_lines = 145/gr_get_font_height(); //Make the max number of lines dependent on the font height. 145 and 85 are magic numbers, based on the window size in retail. 
	} else {
		Max_lines = 85/gr_get_font_height();
	}

	for (idx=0; idx<Max_lines; idx++) {
		// only draw as many pilots as we have
		if ((idx + Player_select_list_start) == Player_select_num_pilots) {
			break;
		}

		// if the currently selected pilot is this line, draw it highlighted
		if ( (idx + Player_select_list_start) == Player_select_pilot) {
			// if he's the active pilot and is also the current selection, super-highlight him
			gr_set_color_fast(&Color_text_active);
		} else { // otherwise draw him normally
			gr_set_color_fast(&Color_text_normal);
		}
		// draw the actual callsign
		gr_printf_menu(Choose_list_coords[gr_screen.res][0], Choose_list_coords[gr_screen.res][1] + (idx * gr_get_font_height()), Pilots[idx + Player_select_list_start]);
	}
}

void player_select_process_noninput(int k)
{
	int idx;

	// check for pressed buttons
	for (idx=0; idx<NUM_PLAYER_SELECT_BUTTONS; idx++) {
		if (Player_select_buttons[gr_screen.res][idx].button.pressed()) {
			player_select_button_pressed(idx);
		}
	}

	// check for keypresses
	switch (k) {
	// quit the game entirely
	case KEY_ESC:
		gameseq_post_event(GS_EVENT_QUIT_GAME);
		break;

	case KEY_ENTER | KEY_CTRLED:
		player_select_button_pressed(ACCEPT_BUTTON);
		break;

	// delete the currently highlighted pilot
	case KEY_DELETE:
		player_select_button_pressed(DELETE_BUTTON);
		break;
	}

	// check to see if the user has clicked on the "list region" button
	// and change the selected pilot appropriately
	if (Player_select_list_region.pressed()) {
		int click_y;
		// get the mouse position
		Player_select_list_region.get_mouse_pos(NULL, &click_y);
		
		// determine what index to select
		//idx = (click_y+5) / 10;
		idx = click_y / gr_get_font_height();


		// if he selected a valid item
		if ( ((idx + Player_select_list_start) < Player_select_num_pilots) && (idx >= 0) ) {
			Player_select_pilot = idx + Player_select_list_start;
		}
	}

	// if the player has double clicked on a valid pilot, choose it and hit the accept button
	if (Player_select_list_region.double_clicked()) {
		if ((Player_select_pilot >= 0) && (Player_select_pilot < Player_select_num_pilots)) {
			player_select_button_pressed(ACCEPT_BUTTON);
		}
	}
}

void player_select_process_input(int k)
{
	char buf[CALLSIGN_LEN + 1];
	int idx,z;

	// if the player is in the process of typing in a new pilot name...
	switch (k) {
	// cancel create pilot
	case KEY_ESC:
		player_select_cancel_create();
		break;

	// accept a new pilot name
	case KEY_ENTER:
		Player_select_input_box.get_text(buf);
		drop_white_space(buf);
		z = 0;
		if (!isalpha(*buf)) {
			z = 1;
		} else {
			for (idx=1; buf[idx]; idx++) {
				if (!isalpha(buf[idx]) && !isdigit(buf[idx]) && !strchr(VALID_PILOT_CHARS, buf[idx])) {
					z = 1;
					break;
				}
			}
		}

		for (idx=1; idx<Player_select_num_pilots; idx++) {
			if (!stricmp(buf, Pilots[idx])) {
				// verify if it is ok to overwrite the file
				if (pilot_verify_overwrite() == 1) {
					// delete the pilot and select the beginning of the list
					Player_select_pilot = idx;
					player_select_delete_pilot();
					Player_select_pilot = 0;
					idx = Player_select_num_pilots;
					z = 0;

				} else
					z = 1;

				break;
			}
		}

		if (!*buf || (idx < Player_select_num_pilots)) {
			z = 1;
		}

		if (z) {
			gamesnd_play_iface(SND_GENERAL_FAIL);
			break;
		}

		// Create the new pilot, and write out his file
		strcpy(Pilots[0], buf);

		// if this is the first guy, we should set the Player struct
		if (Player == NULL) {
			Player = &Players[0];
			Player->reset();
			Player->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;
		}

		strcpy_s(Player->callsign, buf);
		init_new_pilot(Player, !Player_select_clone_flag);

		// set him as being a multiplayer pilot if we're in the correct mode
		if (Player_select_mode == PLAYER_SELECT_MODE_MULTI) {
			Player->flags |= PLAYER_FLAGS_IS_MULTI;
			Player->stats.flags |= STATS_FLAG_MULTIPLAYER;
		}

		// create his pilot file
		Pilot.save_player(Player);

		// unset the player
		Player->reset();
		Player = NULL;

		// make this guy the selected pilot and put him first on the list
		Player_select_pilot = 0;

		// unset the input mode
		player_select_set_input_mode(0);

		// clear any pending bottom text
		player_select_set_bottom_text("");

		// clear any pending middle text
		player_select_set_middle_text("");

		// ungray all the controls
		player_select_set_controls(0);

		// evaluate whether or not this is the very first pilot
		player_select_eval_very_first_pilot();
		break;

	case 0:
		break;

	// always kill middle text when a char is pressed in input mode
	default:
		player_select_set_middle_text("");
		break;
	}
}

// draw copyright message on the bottom on the screen
void player_select_display_copyright()
{
	int	sx, sy, w;
	char	Copyright_msg1[256], Copyright_msg2[256];
	
//	strcpy_s(Copyright_msg1, XSTR("Descent: FreeSpace - The Great War, Copyright c 1998, Volition, Inc.", -1));
	gr_set_color_fast(&Color_white);

//	sprintf(Copyright_msg1, NOX("FreeSpace 2"));
	get_version_string(Copyright_msg1, sizeof(Copyright_msg1));
	sprintf(Copyright_msg2, XSTR("Copyright %c 1999, Volition, Inc.  All rights reserved.", 385), Lcl_special_chars + 4);

	gr_get_string_size(&w, NULL, Copyright_msg1);
	sx = fl2i((gr_screen.max_w_unscaled / 2) - w/2.0f + 0.5f);
	sy = (gr_screen.max_h_unscaled - 2) - 2*gr_get_font_height();
	gr_string(sx, sy, Copyright_msg1, GR_RESIZE_MENU);

	gr_get_string_size(&w, NULL, Copyright_msg2);
	sx = fl2i((gr_screen.max_w_unscaled / 2) - w/2.0f + 0.5f);
	sy = (gr_screen.max_h_unscaled - 2) - gr_get_font_height();

	gr_string(sx, sy, Copyright_msg2, GR_RESIZE_MENU);
}

void player_select_display_all_text()
{
	int w, h;

	// only draw if we actually have a valid string
	if (strlen(Player_select_bottom_text)) {
		gr_get_string_size(&w, &h, Player_select_bottom_text);

		w = (gr_screen.max_w_unscaled - w) / 2;
		gr_set_color_fast(&Color_bright_white);
		gr_printf_menu(w, Player_select_bottom_text_y[gr_screen.res], Player_select_bottom_text);
	}

	// only draw if we actually have a valid string
	if (strlen(Player_select_middle_text)) {
		gr_get_string_size(&w, &h, Player_select_middle_text);

		w = (gr_screen.max_w_unscaled - w) / 2;
		gr_set_color_fast(&Color_bright_white);
		gr_printf_menu(w, Player_select_middle_text_y[gr_screen.res], Player_select_middle_text);
	}
}

int player_select_pilot_file_filter(const char *filename)
{
	return (int)Pilot.verify(filename);
}

void player_select_set_bottom_text(const char *txt)
{
	if (txt) {
		strncpy(Player_select_bottom_text, txt, 149);
	}
}

void player_select_set_middle_text(const char *txt)
{
	if (txt) {
		strncpy(Player_select_middle_text, txt, 149);
	}
}

void player_select_eval_very_first_pilot()
{	
	// never bring up the initial main hall help overlay
	// Player_select_very_first_pilot = 0;

	// if we already have this flag set, check to see if our callsigns match
	if(Player_select_very_first_pilot) {
		// if the callsign has changed, unset the flag
		if(strcmp(Player_select_very_first_pilot_callsign,Pilots[Player_select_pilot])){
			Player_select_very_first_pilot = 0;
		}
	} else { // otherwise check to see if there is only 1 pilot
		if((Player_select_num_pilots == 1) && (Player_select_initial_count == 0)){
			// set up the data
			Player_select_very_first_pilot = 1;
			strcpy_s(Player_select_very_first_pilot_callsign,Pilots[Player_select_pilot]);
		}
	}
}

void player_select_commit()
{
	// if we've gotten to this point, we should have ensured this was the case
	Assert(Player_select_num_pilots > 0);

	gameseq_post_event(GS_EVENT_MAIN_MENU);
	gamesnd_play_iface(SND_COMMIT_PRESSED);

	// evaluate if this is the _very_ first pilot
	player_select_eval_very_first_pilot();
} 

void player_select_cancel_create()
{
	int idx;

	Player_select_num_pilots--;

	// make sure we correct the Selected_pilot index to account for the cancelled action
	if (Player_select_num_pilots == 0) {
		Player_select_pilot = -1;
	}

	// move all pilots down
	for (idx=0; idx<Player_select_num_pilots; idx++) {
		strcpy(Pilots[idx], Pilots[idx + 1]);
	}

	// unset the input mode
	player_select_set_input_mode(0);

	// clear any bottom text
	player_select_set_bottom_text("");

	// clear any middle text
	player_select_set_middle_text("");

	// ungray all controls
	player_select_set_controls(0);

	// disable the autoaccept
	Player_select_autoaccept = 0;
}

DCF(bastion,"Sets the player to be on the bastion (or any other main hall)")
{
	int idx;
	
	if(gameseq_get_state() != GS_STATE_INITIAL_PLAYER_SELECT) {
		dc_printf("This command can only be run while in the initial player select screen.\n");
		return;
	}

	if (dc_optional_string_either("help", "--help")) {
		dc_printf("Usage: bastion [index]\n");
		dc_printf("    [index] -- optional main hall index; if not supplied, defaults to 1\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Player is on main hall '%s'\n", Player_select_force_main_hall.c_str());
		return;
	}

	if (dc_maybe_stuff_int(&idx)) {
		Assert(Main_hall_defines.size() < INT_MAX);
		if ((idx < 0) || (idx >= (int) Main_hall_defines.size())) {
			dc_printf("Main hall index out of range\n");

		} else {
			main_hall_get_name(Player_select_force_main_hall, idx);
			dc_printf("Player is now on main hall '%d'\n", Player_select_force_main_hall.c_str());
		}
	
	} else {
		// No argument passed
		Player_select_force_main_hall = "1";
		dc_printf("Player is now on the Bastion... hopefully\n");
	}
}

#define MAX_PLAYER_TIPS			40

char *Player_tips[MAX_PLAYER_TIPS];
int Num_player_tips;
int Player_tips_shown = 0;

// tooltips
void player_tips_init()
{
	Num_player_tips = 0;
	
	try
	{
		read_file_text("tips.tbl", CF_TYPE_TABLES);
		reset_parse();

		while (!optional_string("#end")) {
			required_string("+Tip:");

			if (Num_player_tips >= MAX_PLAYER_TIPS) {
				break;
			}
			Player_tips[Num_player_tips++] = stuff_and_malloc_string(F_NAME, NULL);
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", "tips.tbl", e.what()));
		return;
	}
}

// close out player tips - *only call from game_shutdown()*
void player_tips_close()
{
	int i;

	for (i=0; i<MAX_PLAYER_TIPS; i++) {
		if (Player_tips[i] != NULL) {
			vm_free(Player_tips[i]);
			Player_tips[i] = NULL;
		}
	}
}

void player_tips_popup()
{
	int tip, ret;

	// player has disabled tips
	if ( (Player != NULL) && !Player->tips ) {
		return;
	}
	// only show tips once per instance of FreeSpace
	if(Player_tips_shown == 1) {
		return;
	}
	Player_tips_shown = 1;

	// randomly pick one
	tip = (int)frand_range(0.0f, (float)Num_player_tips - 1.0f);

	char all_txt[2048];

	do {
		sprintf(all_txt, XSTR("NEW USER TIP\n\n%s", 1565), Player_tips[tip]);
		ret = popup(PF_NO_SPECIAL_BUTTONS | PF_TITLE | PF_TITLE_WHITE, 3, XSTR("&Ok", 669), XSTR("&Next", 1444), XSTR("Don't show me this again", 1443), all_txt);
	
		// now what?
		switch(ret){
		// next
		case 1:
			if(tip >= Num_player_tips - 1) {
				tip = 0;
			} else {
				tip++;
			}
			break;

		// don't show me this again
		case 2:
			ret = 0;
			Player->tips = 0;
			Pilot.save_player(Player);
			Pilot.save_savefile();
			break;
		}
	} while(ret > 0);
}
