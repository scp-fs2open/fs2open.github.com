/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/PlayerMenu.cpp $
 * $Revision: 2.19 $
 * $Date: 2005-03-10 08:00:08 $
 * $Author: taylor $
 *
 * Code to drive the Player Select initial screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.18  2005/03/02 21:24:44  taylor
 * more NO_NETWORK/INF_BUILD goodness for Windows, takes care of a few warnings too
 *
 * Revision 2.17  2005/01/31 23:27:54  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.16  2004/12/22 21:49:05  taylor
 * add a popup to make sure people know about pilot upgrade
 *
 * Revision 2.15  2004/12/05 23:47:18  taylor
 * fix old V bug when cancelling the pilot select screen
 *
 * Revision 2.14  2004/10/31 21:53:23  taylor
 * new pilot code support, no-multiplayer and compiler warning fixes, center mouse cursor for redalert missions
 *
 * Revision 2.13  2004/07/26 20:47:37  Kazan
 * remove MCD complete
 *
 * Revision 2.12  2004/07/17 18:46:07  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.11  2004/07/12 16:32:53  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.10  2004/03/28 17:49:55  taylor
 * runtime language selection, mantis:0000133
 *
 * Revision 2.9  2004/03/05 09:01:53  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.8  2004/02/20 04:29:55  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.7  2003/11/11 02:15:43  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.6  2003/10/27 23:04:22  randomtiger
 * Added -no_set_gamma flags
 * Fixed up some more non standard res stuff
 * Improved selection of device type, this includes using a pure device when allowed which means dev should not use Get* functions in D3D
 * Made fade in credits work
 * Stopped a call to gr_reser_lighting() in non htl mode when the pointer was NULL, was causing a crash loading a fogged level
 * Deleted directx8 directory content, has never been needed.
 *
 * Revision 2.5  2003/08/20 08:11:00  wmcoolmon
 * Added error screens to the barracks and start screens when a pilot file can't be deleted
 *
 * Revision 2.4  2003/03/18 10:07:03  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.3  2003/01/14 04:00:15  Goober5000
 * allowed for up to 256 main halls
 * --Goober5000
 *
 * Revision 2.2.2.1  2002/09/24 18:56:43  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.2  2002/08/04 05:12:42  penguin
 * Display fs2_open version instead of "Freespace 2"
 *
 * Revision 2.1  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/10 20:42:44  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 43    11/02/99 11:42a Jefff
 * fixed copyright symbol in german fonts
 * 
 * 42    10/27/99 12:27a Jefff
 * localized tips correctly
 * 
 * 41    9/13/99 4:52p Dave
 * RESPAWN FIX
 * 
 * 40    9/02/99 11:10a Jefff
 * fixed 1024 list display bug - was only showing 8 pilots at a time
 * 
 * 39    8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 38    8/26/99 9:45a Dave
 * First pass at easter eggs and cheats.
 * 
 * 37    8/16/99 6:39p Jefff
 * 
 * 36    8/16/99 6:37p Jefff
 * minor string alterations
 * 
 * 35    8/05/99 4:17p Dave
 * Tweaks to client interpolation.
 * 
 * 34    8/05/99 11:29a Mikek
 * Jacked up number of comments from 20 to 40, thereby doubling the
 * quality of our game.
 * 
 * 33    8/04/99 10:53a Dave
 * Added title to the user tips popup.
 * 
 * 32    8/03/99 3:21p Jefff
 * 
 * 31    8/03/99 10:32a Jefff
 * raised location of bottom_text to not interfere w/ logo.  changed
 * "please enter callsign" to "type callsign and press enter"
 * 
 * 30    8/02/99 9:13p Dave
 * Added popup tips.
 * 
 * 29    7/30/99 10:29a Jefff
 * fixed colors of bottom display texts
 * 
 * 28    7/27/99 7:17p Jefff
 * Replaced some art text with XSTR() text.
 * 
 * 27    7/19/99 2:06p Jasons
 * Remove all palette stuff from player select menu.
 * 
 * 26    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 25    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 24    6/11/99 11:13a Dave
 * last minute changes before press tour build.
 * 
 * 23    5/21/99 6:45p Dave
 * Sped up ui loading a bit. Sped up localization disk access stuff. Multi
 * start game screen, multi password, and multi pxo-help screen.
 * 
 * 22    4/25/99 3:02p Dave
 * Build defines for the E3 build.
 * 
 * 21    3/25/99 2:31p Neilk
 * Coordinate changes to handle new artwork
 * 
 * 20    2/25/99 4:19p Dave
 * Added multiplayer_beta defines. Added cd_check define. Fixed a few
 * release build warnings. Added more data to the squad war request and
 * response packets.
 * 
 * 19    2/01/99 5:55p Dave
 * Removed the idea of explicit bitmaps for buttons. Fixed text
 * highlighting for disabled gadgets.
 * 
 * 18    1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 17    1/30/99 1:53a Dave
 * Fix some harcoded coords.
 * 
 * 16    1/30/99 1:28a Dave
 * 1024x768 full support.
 * 
 * 15    1/29/99 1:25p Dave
 * New code for choose pilot screen.
 * 
 * 14    1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 13    1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 12    12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 11    12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 10    12/01/98 6:20p Dave
 * Removed tga test bitmap code.
 * 
 * 9     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 * 
 * 8     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 7     11/20/98 11:16a Dave
 * Fixed up IPX support a bit. Making sure that switching modes and
 * loading/saving pilot files maintains proper state.
 * 
 * 6     11/19/98 4:19p Dave
 * Put IPX sockets back in psnet. Consolidated all multiplayer config
 * files into one.
 * 
 * 5     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 4     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 *
 * $NoKeywords: $
 *
 */

#include <ctype.h>

#include "PreProcDefines.h"

#include "menuui/playermenu.h"
#include "ui/ui.h"
#include "gamesnd/gamesnd.h"
#include "playerman/player.h"
#include "io/key.h"
#include "playerman/managepilot.h"
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

#ifndef NO_NETWORK
#include "network/multi.h"
#endif



// --------------------------------------------------------------------------------------------------------
// Demo title screen
#ifdef FS2_DEMO
static int Demo_title_active = 0;
static int Demo_title_bitmap = -1;
static int Demo_title_expire_timestamp = 0;
static int Demo_title_need_fade_in = 1;
static char *Demo_title_bitmap_filename = NOX("DemoTitle1");
#endif

// --------------------------------------------------------------------------------------------------------
// PLAYER SELECT defines
//

//#define MAX_PLAYER_SELECT_LINES		8							// max # of pilots displayed at once
int Player_select_max_lines[GR_NUM_RESOLUTIONS] = {			// max # of pilots displayed at once
	8,			// GR_640
	15			// GR_1024
};

// button control defines
#define NUM_PLAYER_SELECT_BUTTONS	8							// button control defines

#define CREATE_PILOT_BUTTON			0							//	
#define CLONE_BUTTON						1							//
#define DELETE_BUTTON					2							//
#define SCROLL_LIST_UP_BUTTON			3							//
#define SCROLL_LIST_DOWN_BUTTON		4							//
#define ACCEPT_BUTTON					5							//
#define SINGLE_BUTTON					6							//
#define MULTI_BUTTON						7							//

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
// #define PLAYER_SELECT_PALETTE							NOX("ChoosePilotPalette")	// palette for the screen	

#define PLAYER_SELECT_MAIN_HALL_OVERLAY         NOX("MainHall1")				// main hall help overlay

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

		// scroll up, scroll down,	and accept (respectively)
		barracks_buttons("CPB_03",		429,	213,	-1,	-1,	3),
		barracks_buttons("CPB_04",		456,	213,	-1,	-1,	4),
		barracks_buttons("CPB_05",		481,  207,	484,	246,	5),	
		
		// single player select and multiplayer select, respectively
		barracks_buttons("CPB_06",		428,	82,	430,	108,	6),
		barracks_buttons("CPB_07",		477,	82,	481,	108,	7)
	}, 
	{ // GR_1024
		// create, clone and delete (respectively)
		barracks_buttons("2_CPB_00",	182,  328,	199,	384,	0),
		barracks_buttons("2_CPB_01",	275,	328,	292,	384,	1),
		barracks_buttons("2_CPB_02",	361,	328,	379,	384,	2),

		// scroll up, scroll down, and accept (respectively)
		barracks_buttons("2_CPB_03",	686,	341,	-1,	-1,	3),
		barracks_buttons("2_CPB_04",	729,	341,	-1,	-1,	4),
		barracks_buttons("2_CPB_05",	770,  332,	787,	394,	5),	
		
		// single player select and multiplayer select, respectively
		barracks_buttons("2_CPB_06",	685,	132,	700,	173,	6),
		barracks_buttons("2_CPB_07",	764,	132,	782,	173,	7)
	}
};

// FIXME add to strings.tbl
#define PLAYER_SELECT_NUM_TEXT			1
UI_XSTR Player_select_text[GR_NUM_RESOLUTIONS][PLAYER_SELECT_NUM_TEXT] = {
	{ // GR_640
		{ "Choose Pilot",		1436,		122,	90,	UI_XSTR_COLOR_GREEN, -1, NULL }
	}, 
	{ // GR_1024
		{ "Choose Pilot",		1436,		195,	143,	UI_XSTR_COLOR_GREEN, -1, NULL }
	}
};

UI_WINDOW Player_select_window;								// ui window for this screen
UI_BUTTON Player_select_list_region;						// button for detecting mouse clicks on this screen
UI_INPUTBOX Player_select_input_box;						// input box for adding new pilot names				

// #define PLAYER_SELECT_PALETTE_FNAME					NOX("InterfacePalette")
int Player_select_background_bitmap;						// bitmap for this screen
// int Player_select_palette;										// palette bitmap for this screen
int Player_select_autoaccept = 0;
// int Player_select_palette_set = 0;

// flag indicating if this is the absolute first pilot created and selected. Used to determine
// if the main hall should display the help overlay screen
int Player_select_very_first_pilot = 0;			
int Player_select_initial_count = 0;
char Player_select_very_first_pilot_callsign[CALLSIGN_LEN + 2];

extern int Main_hall_bitmap;									// bitmap handle to the main hall bitmap

int Player_select_mode;											// single or multiplayer - never set directly. use player_select_init_player_stuff()
int Player_select_num_pilots;									// # of pilots on the list
int Player_select_list_start;									// index of first list item to start displaying in the box
int Player_select_pilot;									    // index into the Pilot array of which is selected as the active pilot
int Player_select_input_mode;						   			// 0 if the player _isn't_ typing a callsign, 1 if he is
char Pilots_arr[MAX_PILOTS][MAX_FILENAME_LEN];		
char *Pilots[MAX_PILOTS];
int Player_select_clone_flag;									// clone the currently selected pilot
char Player_select_last_pilot[CALLSIGN_LEN + 10];		// callsign of the last used pilot, or none if there wasn't one
int Player_select_last_is_multi;

int Player_select_force_main_hall = 0;

static int Player_select_no_save_pilot = 0;		// to skip save of pilot in pilot_select_close()

int Player_select_screen_active = 0;	// for pilot savefile loading - taylor

// notification text areas

static int Player_select_bottom_text_y[GR_NUM_RESOLUTIONS] = {
	314,	// GR_640
	502	// GR_1024
};

static int Player_select_middle_text_y[GR_NUM_RESOLUTIONS] = {
	253,	// GR_640
	404	// GR_1024
};

char Player_select_bottom_text[150] = "";
char Player_select_middle_text[150] = "";
void player_select_set_bottom_text(char *txt);
void player_select_set_middle_text(char *txt);


// FORWARD DECLARATIONS
void player_select_init_player_stuff(int mode);			// switch between single and multiplayer modes
void player_select_set_input_mode(int n);					
void player_select_button_pressed(int n);
void player_select_scroll_list_up();
void player_select_scroll_list_down();
int player_select_create_new_pilot();
void player_select_delete_pilot();
void player_select_display_all_text();
void player_select_display_copyright();
void player_select_set_bottom_text(char *txt);
void player_select_set_controls(int gray);
void player_select_draw_list();
void player_select_process_noninput(int k);
void player_select_process_input(int k);
int player_select_pilot_file_filter(char *filename);
int player_select_get_last_pilot_info();
void player_select_eval_very_first_pilot();
void player_select_commit();
void player_select_cancel_create();


// basically, gray out all controls (gray == 1), or ungray the controls (gray == 0) 
void player_select_set_controls(int gray)
{
	int idx;
	
	for(idx=0;idx<NUM_PLAYER_SELECT_BUTTONS;idx++){
		if(gray){
			Player_select_buttons[gr_screen.res][idx].button.disable();
		} else {
			Player_select_buttons[gr_screen.res][idx].button.enable();
		}
	}
}

// functions for selecting single/multiplayer pilots at the very beginning of Freespace
void player_select_init()
{			
	int i;
	barracks_buttons *b;   
	UI_WINDOW *w;

	// start a looping ambient sound
	main_hall_start_ambient();

	Player_select_force_main_hall = 0;

	Player_select_screen_active = 1;

#ifdef FS2_DEMO
	/*
	Demo_title_bitmap = bm_load(Demo_title_bitmap_filename);
	if ( Demo_title_bitmap >= 0 ) {
#ifndef HARDWARE_ONLY
		palette_use_bm_palette(Demo_title_bitmap);
#endif
		Demo_title_active = 1;
		Demo_title_expire_timestamp = timestamp(5000);
	} else {
		Demo_title_active = 0;
	}
	*/
	Demo_title_active = 0;
#endif

	// create the UI window
	Player_select_window.create(0, 0, gr_screen.max_w, gr_screen.max_h, 0);
	Player_select_window.set_mask_bmap(Player_select_background_mask_bitmap[gr_screen.res]);
	
	// initialize the control buttons
	for (i=0; i<NUM_PLAYER_SELECT_BUTTONS; i++) {
		b = &Player_select_buttons[gr_screen.res][i];

		// create the button
		if ( (i == SCROLL_LIST_UP_BUTTON) || (i == SCROLL_LIST_DOWN_BUTTON) )
			b->button.create(&Player_select_window, NULL, b->x, b->y, 60, 30, 1, 1);
		else
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

	// disable the single player button in the multiplayer beta
#ifdef MULTIPLAYER_BETA_BUILD
	Player_select_buttons[gr_screen.res][SINGLE_BUTTON].button.hide();
	Player_select_buttons[gr_screen.res][SINGLE_BUTTON].button.disable();
#elif defined(E3_BUILD) || defined(PRESS_TOUR_BUILD)
	Player_select_buttons[gr_screen.res][MULTI_BUTTON].button.hide();
	Player_select_buttons[gr_screen.res][MULTI_BUTTON].button.disable();
#endif


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
#if defined(DEMO) || defined(OEM_BUILD) || defined(E3_BUILD) || defined(PRESS_TOUR_BUILD) || defined(NO_NETWORK) // not for FS2_DEMO
	player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);	
#elif defined(MULTIPLAYER_BETA_BUILD)
	player_select_init_player_stuff(PLAYER_SELECT_MODE_MULTI);	
#else
	if (player_select_get_last_pilot_info()) {
		if (Player_select_last_is_multi) {
			player_select_init_player_stuff(PLAYER_SELECT_MODE_MULTI);
		} else {
			player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);
		}
	} 
	// otherwise go to the single player mode by default
	else {
		player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);
	}
#endif	

	if((Player_select_num_pilots == 1) && Player_select_input_mode){
		Player_select_autoaccept = 1;
	}	
}

#ifdef FS2_DEMO
// Display the demo title screen
void demo_title_blit()
{
	int k;

	Mouse_hidden = 1;

	if ( timestamp_elapsed(Demo_title_expire_timestamp) ) {
		Demo_title_active = 0;
	}

	k = game_poll();
	if ( k > 0 ) {
		Demo_title_active = 0;
	}

	if ( Demo_title_need_fade_in ) {
		gr_fade_out(0);
	}
	
	gr_set_bitmap(Demo_title_bitmap);
	gr_bitmap(0,0);

	gr_flip();

	if ( Demo_title_need_fade_in ) {
		gr_fade_in(0);
		Demo_title_need_fade_in = 0;
	}

	if ( !Demo_title_active ) {
		gr_fade_out(0);
		Mouse_hidden = 0;
	}
}

#endif

void player_select_do()
{
	int k;

#ifdef FS2_DEMO
	if ( Demo_title_active ) {
		// demo_title_blit();
		return;
	}
#endif

	//if ( !Player_select_palette_set ) {
	//	Assert(Player_select_palette >= 0);
//#ifndef HARDWARE_ONLY
//		palette_use_bm_palette(Player_select_palette);
//#endif
//		Player_select_palette_set = 1;
//	}
		
	// set the input box at the "virtual" line 0 to be active so the player can enter a callsign
	if (Player_select_input_mode){
		Player_select_input_box.set_focus();
	}

	// process any ui window stuff
	k = Player_select_window.process();
	if(k){
		extern void game_process_cheats(int k);
		game_process_cheats(k);
	}
	switch(k){
	// switch between single and multiplayer modes
	case KEY_TAB : 
#if defined(DEMO) || defined(OEM_BUILD) // not for FS2_DEMO
		break;
#else

		if(Player_select_input_mode){
			gamesnd_play_iface(SND_GENERAL_FAIL);
			break;
		}
		// play a little sound
		gamesnd_play_iface(SND_USER_SELECT);
		if(Player_select_mode == PLAYER_SELECT_MODE_MULTI){					
			player_select_set_bottom_text(XSTR( "Single-Player Mode", 376));
				
			// reinitialize as single player mode
			player_select_init_player_stuff(PLAYER_SELECT_MODE_SINGLE);
		} else if(Player_select_mode == PLAYER_SELECT_MODE_SINGLE){										
			player_select_set_bottom_text(XSTR( "Multiplayer Mode", 377));
				
			// reinitialize as multiplayer mode
			player_select_init_player_stuff(PLAYER_SELECT_MODE_MULTI);
		}
		break;	
#endif
		case KEY_ESC:
			Player_select_no_save_pilot = 1;
			break;
	}	

	// draw the player select pseudo-dialog over it
	gr_set_bitmap(Player_select_background_bitmap);
	gr_bitmap(0,0);

	// press the accept button
	if (Player_select_autoaccept) {
		Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].button.press_button();
	}
	
	// draw any ui window stuf
	Player_select_window.draw();

	// light up the correct mode button (single or multi)
	if (Player_select_mode == PLAYER_SELECT_MODE_SINGLE){
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

#ifndef RELEASE_REAL
	// gr_set_color_fast(&Color_bright_green);
	// gr_string(0x8000, 10, "Development version - DO NOT RELEASE");
#endif
	
	/*
	gr_set_color(255, 0, 0);
	vector whee[5];
	vector *arr[5] = {&whee[0], &whee[1], &whee[2], &whee[3], &whee[4]};
	whee[0].x = 10; whee[0].y = 10; whee[0].z = 0.0f;
	whee[1].x = 50; whee[1].y = 50; whee[1].z = 0.0f;
	whee[2].x = 50; whee[2].y = 90; whee[2].z = 0.0f;
	whee[3].x = 90; whee[3].y = 130; whee[3].z = 0.0f;
	whee[4].x = 180; whee[4].y = 130; whee[4].z = 0.0f;
	gr_pline_special(arr, 5, 2);
	*/
	

	gr_flip();
}

void player_select_close()
{
	// destroy the player select window
	Player_select_window.destroy();

	// if we're in input mode - we should undo the pilot create reqeust
	if(Player_select_input_mode){
		player_select_cancel_create();
	}

	// if we are just exiting then don't try to save any pilot files - taylor
	if (Player_select_no_save_pilot) {
		Player = NULL;
		return;
	}

	// actually set up the Player struct here	
	if((Player_select_pilot == -1) || (Player_select_num_pilots == 0)){
		nprintf(("General","WARNING! No pilot selected! We should be exiting the game now!\n"));
		return;
	}

	// unload all bitmaps
	if(Player_select_background_bitmap >= 0){
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
		
	// now read in a the pilot data
	if (read_pilot_file(Pilots[Player_select_pilot], !Player_select_mode, Player) != 0) {
		Error(LOCATION,"Couldn't load pilot file, bailing");
		Player = NULL;
	} else {
		if (Player_select_mode == PLAYER_SELECT_MODE_SINGLE) {
			mission_load_up_campaign(); // load up campaign savefile - taylor
		}
	}

	if (Player_select_force_main_hall) {
		Player->main_hall = 1;
	}

	Player_select_screen_active = 0;
}

void player_select_set_input_mode(int n)
{
	int i;

	// set the input mode
	Player_select_input_mode = n;	
	
	// enable all the player select buttons
	for (i=0; i<NUM_PLAYER_SELECT_BUTTONS; i++){
		Player_select_buttons[gr_screen.res][i].button.enable(!n);
	}

	Player_select_buttons[gr_screen.res][ACCEPT_BUTTON].button.set_hotkey(n ? -1 : KEY_ENTER);
	Player_select_buttons[gr_screen.res][CREATE_PILOT_BUTTON].button.set_hotkey(n ? -1 : KEY_C);

	// enable the player select input box
	if(Player_select_input_mode){
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
			player_select_commit();				
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
			if (read_pilot_file(Pilots[Player_select_pilot], !Player_select_mode, Player) != 0) {
				Error(LOCATION,"Couldn't load pilot file, bailing");
				Player = NULL;
				Int3();
			} else {
				if (Player_select_mode == PLAYER_SELECT_MODE_SINGLE) {
					mission_load_up_campaign(); // get campaign file - taylor
				}
			}

			// set the clone flag
			Player_select_clone_flag = 1;

			// create the new pilot (will be cloned with Player_select_clone_flag_set)
			if (!player_select_create_new_pilot()) {					
				player_select_set_bottom_text(XSTR( "Error creating new pilot file!", 380));
				Player_select_clone_flag = 0;
				memset(Player,0,sizeof(player));
				Player = NULL;
				break;
			}				

			// clear the player out
			// JH: How do you clone a pilot if you clear out the source you are copying
			// from?  These next 2 lines are pure stupidity, so I commented them out!
//			memset(Player,0,sizeof(player));
//			Player = NULL;
				
			// display some text on the bottom of the dialog
			player_select_set_bottom_text(XSTR( "Type Callsign and Press Enter", 381));				
			
			// gray out all controls in the dialog
			player_select_set_controls(1);					
		}
		break;

	case CREATE_PILOT_BUTTON:
		// if we're at max-pilots, don't allow another to be added
		if(Player_select_num_pilots >= MAX_PILOTS){
			player_select_set_bottom_text(XSTR( "You already have the maximum # of pilots!", 379));

			gamesnd_play_iface(SND_GENERAL_FAIL);
			break;
		}

		// create a new pilot
		if (!player_select_create_new_pilot()) {
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
			// display a popup requesting confirmation
			ret = popup(PF_TITLE_BIG | PF_TITLE_RED, 2, POPUP_NO, POPUP_YES, XSTR( "Warning!\n\nAre you sure you wish to delete this pilot?", 382));

			// delete the pilot
			if(ret == 1){
				player_select_delete_pilot();
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
#if defined(DEMO) || defined(OEM_BUILD) // not for FS2_DEMO
		game_feature_not_in_demo_popup();
#elif defined(NO_NETWORK)
		game_feature_disabled_popup();
#else
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
#endif
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

#ifdef FS2_DEMO
	if ( Demo_title_active ) {
		play_scroll_sound = 0;
	}
#endif

	if ( play_scroll_sound ) {
		gamesnd_play_iface(SND_SCROLL);
	}

	idx = Player_select_num_pilots;	
	
	// move all the pilots in the list up
	while (idx--) {
		strcpy(Pilots[idx + 1], Pilots[idx]);		
	}	

#ifndef NO_NETWORK
	// by default, set the default netgame protocol to be VMT
	Multi_options_g.protocol = NET_TCP;	
#endif

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
	int i, deleted_cur_pilot;

	deleted_cur_pilot = 0;

	// tack on the full path and the pilot file extension
	// build up the path name length
	// make sure we do this based upon whether we're in single or multiplayer mode
	strcpy( filename, Pilots[Player_select_pilot] );
	strcat( filename, NOX(".pl2") );

	int del_rval;
	int popup_rval = 0;
	do {
		// attempt to delete the pilot
		if (Player_select_mode == PLAYER_SELECT_MODE_SINGLE) {
			del_rval = cf_delete( filename, CF_TYPE_SINGLE_PLAYERS );
		} else {
			del_rval = cf_delete( filename, CF_TYPE_MULTI_PLAYERS );
		}

		if(!del_rval) {
			popup_rval = popup(PF_TITLE_BIG | PF_TITLE_RED, 2, XSTR( "&Retry", -1), XSTR("&Cancel",-1),
				XSTR("Error\nFailed to delete pilot file.  File may be read-only.\n", -1));
		}

		//Abort
		if(popup_rval)
		{
			return;
		}

		//Try again
	} while (!del_rval);

	// delete all the campaign save files for this pilot.
	mission_campaign_delete_all_savefiles( Pilots[Player_select_pilot], (Player_select_mode != PLAYER_SELECT_MODE_SINGLE) );

	// move all the players down
	for (i=Player_select_pilot; i<Player_select_num_pilots-1; i++){
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
	if (Player_select_pilot == -1)
		return;

	// change the pilot selected index and play the appropriate sound
	if (Player_select_pilot) {
		Player_select_pilot--;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
		
	if (Player_select_pilot < Player_select_list_start){
		Player_select_list_start = Player_select_pilot;
	}
}

// scroll the list of players down
void player_select_scroll_list_down()
{	
	// change the pilot selected index and play the appropriate sound
	if (Player_select_pilot < Player_select_num_pilots - 1) {
		Player_select_pilot++;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
		
	if (Player_select_pilot >= (Player_select_list_start + Player_select_max_lines[gr_screen.res])){
		Player_select_list_start++;
	}
}

// fill in the data on the last played pilot (callsign and is_multi or not)
int player_select_get_last_pilot_info()
{
	char *last_player;

	last_player = os_config_read_string( NULL, "LastPlayer", NULL);
	
	if(last_player == NULL){
		return 0;		
	} else {
		strcpy(Player_select_last_pilot,last_player);
	}

	// determine if he was a single or multi-player based upon the last character in his callsign
	Player_select_last_is_multi = Player_select_last_pilot[strlen(Player_select_last_pilot)-1] == 'M' ? 1 : 0;
	Player_select_last_pilot[strlen(Player_select_last_pilot)-1]='\0';

	return 1;	
}

int player_select_get_last_pilot()
{
	// if the player has the Cmdline_use_last_pilot command line option set, try and drop out quickly
	if(Cmdline_use_last_pilot){			
		int idx;

		if(!player_select_get_last_pilot_info()){
			return 0;
		}

		Get_file_list_filter = player_select_pilot_file_filter;
		if (Player_select_last_is_multi) {
			Player_select_num_pilots = cf_get_file_list_preallocated(MAX_PILOTS, Pilots_arr, Pilots, CF_TYPE_MULTI_PLAYERS, NOX("*.plr"), CF_SORT_TIME);
		} else {
			int i,j, new_pilot_num = 0;
			int old_pilot_num = 0;
			char old_pilots_arr[MAX_PILOTS][MAX_FILENAME_LEN];
			char *old_pilots[MAX_PILOTS];

			Player_select_num_pilots = cf_get_file_list_preallocated(MAX_PILOTS, Pilots_arr, Pilots, CF_TYPE_SINGLE_PLAYERS, NOX("*.pl2"), CF_SORT_TIME);
			old_pilot_num = cf_get_file_list_preallocated(MAX_PILOTS, old_pilots_arr, old_pilots, CF_TYPE_SINGLE_PLAYERS, NOX("*.plr"), CF_SORT_TIME);

			new_pilot_num = Player_select_num_pilots + old_pilot_num;

			for (i = Player_select_num_pilots; i<new_pilot_num;) {
				for (j = 0; j<old_pilot_num; j++) {
					if ( i <= MAX_PILOTS ) {
						strcpy( Pilots[i], old_pilots[j] );
						Player_select_num_pilots++;
						i++;
					}
				}
			}
		}

		Player_select_pilot = -1;
		idx = 0;
		// pick the last player		
		for(idx=0;idx<Player_select_num_pilots;idx++){
			if(strcmp(Player_select_last_pilot,Pilots_arr[idx])==0){
				Player_select_pilot = idx;
				break;
			}
		}		

		// set this so that we don't incorrectly create a "blank" pilot - .plr
		// in the player_select_close() function
		Player_select_num_pilots = 0;

		// if we've actually found a valid pilot, load him up		
		if(Player_select_pilot != -1){
			Player = &Players[0];			
			read_pilot_file(Pilots_arr[idx],!Player_select_last_is_multi,Player);
			if (Player_select_mode == PLAYER_SELECT_MODE_SINGLE) {
				mission_load_up_campaign(); // load up campaign file now - taylor
			}
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
	if (mode == PLAYER_SELECT_MODE_SINGLE) {
		int i,j, new_pilot_num = 0;
		int old_pilot_num = 0;
		char old_pilots_arr[MAX_PILOTS][MAX_FILENAME_LEN];
		char *old_pilots[MAX_PILOTS];
	
		Player_select_num_pilots = cf_get_file_list_preallocated(MAX_PILOTS, Pilots_arr, Pilots, CF_TYPE_SINGLE_PLAYERS, NOX("*.pl2"), CF_SORT_TIME);
		old_pilot_num = cf_get_file_list_preallocated(MAX_PILOTS, old_pilots_arr, old_pilots, CF_TYPE_SINGLE_PLAYERS, NOX("*.plr"), CF_SORT_TIME);

		new_pilot_num = Player_select_num_pilots + old_pilot_num;

		for (i = Player_select_num_pilots; i<new_pilot_num;) {
			for (j = 0; j<old_pilot_num; j++) {
				if ( i <= MAX_PILOTS ) {
					strcpy( Pilots[i], old_pilots[j] );
					Player_select_num_pilots++;
					i++;
				}
			}
		}
	} else {
		Player_select_num_pilots = cf_get_file_list_preallocated(MAX_PILOTS, Pilots_arr, Pilots, CF_TYPE_MULTI_PLAYERS, NOX("*.plr"), CF_SORT_TIME);
	}

	Player = NULL;	

	// if this value is -1, it means we should set it to the num pilots count
	if(Player_select_initial_count == -1){
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

	for (idx=0; idx<Player_select_max_lines[gr_screen.res]; idx++) {
		// only draw as many pilots as we have
		if ((idx + Player_select_list_start) == Player_select_num_pilots)
			break;

		// if the currently selected pilot is this line, draw it highlighted
		if ( (idx + Player_select_list_start) == Player_select_pilot) {
			// if he's the active pilot and is also the current selection, super-highlight him									
			gr_set_color_fast(&Color_text_active);
		}
		// otherwise draw him normally
		else {
			gr_set_color_fast(&Color_text_normal);
		}
		
		// draw the actual callsign
		gr_printf(Choose_list_coords[gr_screen.res][0], Choose_list_coords[gr_screen.res][1] + (idx * gr_get_font_height()), Pilots[idx + Player_select_list_start]);
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
		if (Player_select_pilot >= 0) {
			int ret;

			// display a popup requesting confirmation
			ret = popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON,2,POPUP_NO,POPUP_YES,XSTR( "Are you sure you want to delete this pilot?", 383));										

			// delete the pilot
			if(ret == 1){
				player_select_delete_pilot();
			} 
		}
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
		if(((idx + Player_select_list_start) < Player_select_num_pilots) && (idx >= 0)){
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
			memset(Player, 0, sizeof(player));
			Player->flags |= PLAYER_FLAGS_STRUCTURE_IN_USE;
		}

		strcpy(Player->callsign, buf);
		init_new_pilot(Player, !Player_select_clone_flag);

		// set him as being a multiplayer pilot if we're in the correct mode
		if (Player_select_mode == PLAYER_SELECT_MODE_MULTI) {
			Player->flags |= PLAYER_FLAGS_IS_MULTI;
			Player->stats.flags |= STATS_FLAG_MULTIPLAYER;
		}

		// create his pilot file
		write_pilot_file(Player);

		// unset the player
		memset(Player, 0, sizeof(player));
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
	
//	strcpy(Copyright_msg1, XSTR("Descent: FreeSpace - The Great War, Copyright c 1998, Volition, Inc.", -1));
	gr_set_color_fast(&Color_white);

//	sprintf(Copyright_msg1, NOX("FreeSpace 2"));
	get_version_string(Copyright_msg1);
	if (Lcl_gr) {
		sprintf(Copyright_msg2, XSTR("Copyright %c 1999, Volition, Inc.  All rights reserved.", 385), '\xA8');
	} else {
		sprintf(Copyright_msg2, XSTR("Copyright %c 1999, Volition, Inc.  All rights reserved.", 385), '\x83');
	}

	gr_get_string_size(&w, NULL, Copyright_msg1);
	sx = fl2i((gr_screen.max_w / 2) - w/2.0f + 0.5f);
	sy = (gr_screen.max_h - 2) - 2*gr_get_font_height();
	gr_string(sx, sy, Copyright_msg1);

	gr_get_string_size(&w, NULL, Copyright_msg2);
	sx = fl2i((gr_screen.max_w / 2) - w/2.0f + 0.5f);
	sy = (gr_screen.max_h - 2) - gr_get_font_height();

	gr_string(sx, sy, Copyright_msg2);
}

void player_select_display_all_text()
{
	int w, h;

	// only draw if we actually have a valid string
	if (strlen(Player_select_bottom_text)) {
		gr_get_string_size(&w, &h, Player_select_bottom_text);
	
		w = (gr_screen.max_w - w) / 2;
		gr_set_color_fast(&Color_bright_white);
		gr_printf(w, Player_select_bottom_text_y[gr_screen.res], Player_select_bottom_text);
	}

	// only draw if we actually have a valid string
	if (strlen(Player_select_middle_text)) {
		gr_get_string_size(&w, &h, Player_select_middle_text);
	
		w = (gr_screen.max_w - w) / 2;
		gr_set_color_fast(&Color_bright_white);
		gr_printf(w, Player_select_middle_text_y[gr_screen.res], Player_select_middle_text);
	}
}

int player_select_pilot_file_filter(char *filename)
{
	return !verify_pilot_file(filename, Player_select_mode == PLAYER_SELECT_MODE_SINGLE);
}

void player_select_set_bottom_text(char *txt)
{
	if (txt) {
		strncpy(Player_select_bottom_text, txt, 149);
	}
}

void player_select_set_middle_text(char *txt)
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
	if(Player_select_very_first_pilot){
		// if the callsign has changed, unset the flag
		if(strcmp(Player_select_very_first_pilot_callsign,Pilots[Player_select_pilot])){
			Player_select_very_first_pilot = 0;
		}
	}
	// otherwise check to see if there is only 1 pilot
	else {
		if((Player_select_num_pilots == 1) && (Player_select_initial_count == 0)){
			// set up the data
			Player_select_very_first_pilot = 1;
			strcpy(Player_select_very_first_pilot_callsign,Pilots[Player_select_pilot]);
		}
	}
}

void player_select_commit()
{
	// if we've gotten to this point, we should have ensured this was the case
	Assert(Player_select_num_pilots > 0);

	// check to see if we are going to try and upgrade or not
	if ( pilot_file_upgrade_check(Pilots[Player_select_pilot], !Player_select_mode) )
		return;

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

DCF(bastion,"Sets the player to be on the bastion")
{
	if(gameseq_get_state() == GS_STATE_INITIAL_PLAYER_SELECT){
		Player_select_force_main_hall = 1;
		dc_printf("Player is now in the Bastion\n");
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

	// begin external localization stuff
	lcl_ext_open();

	read_file_text("tips.tbl");
	reset_parse();

	while(!optional_string("#end")){
		required_string("+Tip:");

		if(Num_player_tips >= MAX_PLAYER_TIPS){
			break;
		}
		Player_tips[Num_player_tips++] = stuff_and_malloc_string(F_NAME, NULL, 1024);				
	}

	// stop externalizing, homey
	lcl_ext_close();
}

// close out player tips - *only call from game_shutdown()*
void player_tips_close()
{
	int i;

	for (i=0; i<MAX_PLAYER_TIPS; i++) {
		if (Player_tips[i] != NULL) {
			free(Player_tips[i]);
			Player_tips[i] = NULL;
		}
	}
}

void player_tips_popup()
{
	int tip, ret;	
	
	// player has disabled tips
	if((Player != NULL) && !Player->tips){
		return;
	}
	// only show tips once per instance of Freespace
	if(Player_tips_shown == 1){
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
			if(tip >= Num_player_tips - 1){
				tip = 0;
			} else {
				tip++;
			}
			break;

		// don't show me this again
		case 2:
			ret = 0;
			Player->tips = 0;
			write_pilot_file(Player);
			break;
		}
	} while(ret > 0);
}
