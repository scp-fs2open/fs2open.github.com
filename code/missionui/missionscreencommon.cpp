/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MissionUI/MissionScreenCommon.cpp $
 * $Revision: 2.10 $
 * $Date: 2005-01-29 08:09:47 $
 * $Author: wmcoolmon $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.9  2005/01/15 05:53:18  wmcoolmon
 * Current version of the new techroom code -C
 *
 * Revision 2.8  2004/07/26 20:47:40  Kazan
 * remove MCD complete
 *
 * Revision 2.7  2004/07/17 18:46:08  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.6  2004/07/12 16:32:55  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.5  2004/03/05 09:01:55  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.4  2003/11/11 02:15:44  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.3  2003/09/13 06:02:06  Goober5000
 * clean rollback of all of argv's stuff
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.5  2002/05/16 06:07:38  mharris
 * more ifndef NO_SOUND
 *
 * Revision 1.4  2002/05/13 15:11:03  mharris
 * More NO_NETWORK ifndefs added
 *
 * Revision 1.3  2002/05/10 20:42:44  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 13    10/14/99 2:51p Jefff
 * localiztion fixes
 * 
 * 12    8/05/99 3:40p Jefff
 * hi-res text adjustments
 * 
 * 11    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 10    2/01/99 5:55p Dave
 * Removed the idea of explicit bitmaps for buttons. Fixed text
 * highlighting for disabled gadgets.
 * 
 * 9     1/30/99 7:33p Neilk
 * Fixed coords problems for mission briefing screens
 * 
 * 8     1/30/99 5:08p Dave
 * More new hi-res stuff.Support for nice D3D textures.
 * 
 * 7     1/29/99 4:17p Dave
 * New interface screens.
 * 
 * 6     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 5     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 4     11/17/98 11:12a Dave
 * Removed player identification by address. Now assign explicit id #'s.
 * 
 * 3     10/16/98 9:40a Andsager
 * Remove ".h" files from model.h
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 85    6/09/98 10:31a Hoffoss
 * Created index numbers for all xstr() references.  Any new xstr() stuff
 * added from here on out should be added to the end if the list.  The
 * current list count can be found in FreeSpace.cpp (search for
 * XSTR_SIZE).
 * 
 * 84    5/08/98 7:52p Lawrance
 * fix bug where wing slot icons were not getting loaded
 * 
 * 83    5/08/98 10:17a Lawrance
 * don't play briefing slide-in animation if detail level is below high
 * 
 * 82    5/06/98 11:50p Lawrance
 * Clean up help overlay code for loadout screens
 * 
 * 81    5/03/98 1:55a Lawrance
 * Fix some sound problems with loadout screens
 * 
 * 80    4/30/98 6:03p Lawrance
 * Make drag and drop work better.
 * 
 * 79    4/22/98 5:52p Dave
 * Large reworking of endgame sequencing. Updated host options screen for
 * new artwork. Put in checks to end game if host leaves or if team
 * captains leave mid-game. 
 * 
 * 78    4/07/98 7:51p Hoffoss
 * Implemented changed to options screen due to artwork changes.
 * 
 * 77    4/07/98 5:42p Dave
 * Put in support for ui display of voice system status (recording,
 * playing back, etc). Make sure main hall music is stopped before
 * entering a multiplayer game via ingame join.
 * 
 * 76    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 75    4/01/98 11:19p Dave
 * Put in auto-loading of xferred pilot pic files. Grey out background
 * behind pinfo popup. Put a chatbox message in when players are kicked.
 * Moved mission title down in briefing. Other ui fixes.
 * 
 * 74    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 73    3/31/98 11:47p Lawrance
 * Fix some bugs related to wingmen selection when doing a quick mission
 * restart.
 * 
 * 72    3/31/98 5:31p Lawrance
 * Fix sound bug that happenned when entering briefing
 * 
 * 71    3/30/98 5:16p Lawrance
 * centralize a check for disabled loadout screens
 * 
 * 70    3/30/98 12:18a Lawrance
 * change some DEMO_RELEASE code to not compile code rather than return
 * early
 * 
 * 69    3/29/98 10:16p Allender
 * scramble missions need to have ship/weapon selection disabled (this
 * code was somehow lost)
 * 
 * 68    3/29/98 1:24p Dave
 * Make chatbox not clear between multiplayer screens. Select player ship
 * as default in mp team select and weapons select screens. Made create
 * game mission list use 2 fixed size columns.
 * 
 * 67    3/29/98 12:55a Lawrance
 * Get demo build working with limited set of data.
 * 
 * 66    3/25/98 8:43p Hoffoss
 * Changed anim_play() to not be so complex when you try and call it.
 * 
 * 65    3/24/98 4:59p Dave
 * Fixed several ui bugs. Put in pre and post voice stream playback sound
 * fx. Put in error specific popups for clients getting dropped from games
 * through actions other than their own.
 * 
 * 64    3/19/98 5:32p Lawrance
 * Added music to the background of command brief screen.
 * 
 * 63    3/05/98 5:02p Dave
 * More work on team vs. team support for multiplayer. Need to fix bugs in
 * weapon select.
 * 
 * 62    3/03/98 8:55p Dave
 * Finished pre-briefing team vs. team support.
 * 
 * 61    3/03/98 8:12p Lawrance
 * Double timeout before flashing buttons
 * 
 * 60    3/02/98 3:27p Lawrance
 * Don't call muti_ts_init() in single player
 * 
 * 59    3/01/98 3:26p Dave
 * Fixed a few team select bugs. Put in multiplayer intertface sounds.
 * Corrected how ships are disabled/enabled in team select/weapon select
 * screens.
 * 
 * 58    2/27/98 4:55p Sandeep
 * Fixed a signed/unsigned bug in the wss packet type
 * 
 * 57    2/26/98 4:59p Allender
 * groundwork for team vs team briefings.  Moved weaponry pool into the
 * Team_data structure.  Added team field into the p_info structure.
 * Allow for mutliple structures in the briefing code.
 * 
 * 56    2/25/98 10:24a Lawrance
 * Don't ask for confirmation when ESC is pressed.
 * 
 * 55    2/24/98 12:22a Lawrance
 * New coords for revamped briefing graphics
 * 
 * 54    2/22/98 4:17p John
 * More string externalization classification... 190 left to go!
 * 
 * 53    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 52    2/21/98 2:50p Lawrance
 * Tell players that their campaign position will get saved if they return
 * to main hall from loadout screens.
 * 
 * 51    2/19/98 6:26p Dave
 * Fixed a few file xfer bugs. Tweaked mp team select screen. Put in
 * initial support for player data uploading.
 * 
 * 50    2/18/98 3:56p Dave
 * Several bugs fixed for mp team select screen. Put in standalone packet
 * routing for team select.
 * 
 * 49    2/17/98 6:07p Dave
 * Tore out old multiplayer team select screen, installed new one.
 * 
 * 48    2/13/98 3:46p Dave
 * Put in dynamic chatbox sizing. Made multiplayer file lookups use cfile
 * functions.
 * 
 * 47    2/10/98 8:40p Dave
 * Fixed some debriefing bugs.
 * 
 * 46    2/04/98 11:06p Lawrance
 * Fix a couple of bugs with save/restore of player weapon loadout.
 * 
 * 45    2/04/98 4:32p Allender
 * support for multiple briefings and debriefings.  Changes to mission
 * type (now a bitfield).  Bitfield defs for multiplayer modes
 * 
 * 44    1/30/98 10:40a Lawrance
 * remove any binding references to hotkey screen
 * 
 * 43    1/20/98 5:52p Lawrance
 * prompt user before returning to main hall (in single player)
 * 
 * 42    1/17/98 5:51p Dave
 * Bug fixes for bugs generated by multiplayer testing.
 * 
 * 41    1/14/98 11:39a Dave
 * Polished up a bunch of popup support items.
 * 
 * 40    1/13/98 5:35p Lawrance
 * Show popup when trying to go to weapons loadout without any ships
 * selected.
 * 
 * 39    1/10/98 5:48p Lawrance
 * Don't allow Tab to go to ship/weapon loadout screen in training, play
 * negative feedback sounds if user tries to.
 * 
 * 38    1/10/98 12:45a Lawrance
 * Don't restore loadout if mission was modified.
 * 
 * 37    1/09/98 6:06p Dave
 * Put in network sound support for multiplayer ship/weapon select
 * screens. Made clients exit game correctly through warp effect. Fixed
 * main hall menu help overlay bug.
 * 
 * 36    1/09/98 11:04a Lawrance
 * Fix bug that prevented buttons from flashing in the loadout screens.
 * 
 * 35    1/07/98 6:45p Lawrance
 * Play first briefing music if none specified.
 * 
 * 34    1/05/98 4:57p Lawrance
 * reset flash timers when a button is pressed or a key is pressed
 * 
 * 33    12/30/97 5:46p Lawrance
 * Rename rnd() to rand_alt().
 * 
 * 32    12/30/97 4:27p Lawrance
 * Add new rnd() function that doesn't affect rand() sequence.
 * 
 * 31    12/29/97 4:21p Lawrance
 * Flash buttons on briefing/ship select/weapons loadout when enough time
 * has elapsed without activity.
 * 
 * 30    12/28/97 5:52p Lawrance
 * Add support for debriefing success/fail music.
 * 
 * 29    12/24/97 8:54p Lawrance
 * Integrating new popup code
 * 
 * 28    12/24/97 1:19p Lawrance
 * fix some bugs with the multiplayer ship/weapons loadout
 * 
 * 27    12/23/97 11:58a Allender
 * changes to ship/wespon selection for multplayer.  added sounds to some
 * interface screens.  Update and modiied end-of-briefing packets -- yet
 * to be tested.
 *
 * $NoKeywords: $
 *
 */

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

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "network/multiteamselect.h"
#include "network/multi_endgame.h"
#include "missionui/chatbox.h"
#endif

#ifndef NDEBUG
#include <limits.h>
#endif




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
static anim_instance *Background_anim_instance;

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

	#ifdef DEMO // allow for FS2_DEMO
		Common_buttons[Current_screen-1][gr_screen.res][COMMON_SS_REGION].button.disable();
		Common_buttons[Current_screen-1][gr_screen.res][COMMON_WEAPON_REGION].button.disable();
	#endif
}

void set_active_ui(UI_WINDOW *ui_window)
{
	Active_ui_window = ui_window;
}

void common_music_init(int score_index)
{
#ifndef NO_SOUND
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
#endif
}

void common_music_do()
{
#ifndef NO_SOUND
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	// Use this id to trigger the start of music playing on the briefing screen
	if ( timestamp_elapsed( Briefing_music_begin_timestamp) ) {
		Briefing_music_begin_timestamp = 0;
		briefing_start_music();
	}
#endif
}

void common_music_close()
{
#ifndef NO_SOUND
	if ( Cmdline_freespace_no_music ) {
		return;
	}

	if ( Num_music_files <= 0 )
		return;

	briefing_stop_music();
#endif
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

	strcpy(buf, filename);

	// unload the interface bitmap from memory
	if (InterfacePaletteBitmap != -1) {
		bm_unload(InterfacePaletteBitmap);
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
		bm_unload(InterfacePaletteBitmap);
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

	#ifndef DEMO // not for FS2_DEMO

	/*
	if ( current_detail_level() >= (NUM_DEFAULT_DETAIL_LEVELS-2) ) {

		anim_play_struct aps;

		// Load in the background transition anim
		if ( Game_mode & GM_MULTIPLAYER )
			Background_anim = anim_load("BriefTransMulti", 1);	// 1 as last parm means file is mem-mapped
		else  {
			Background_anim = anim_load("BriefTrans", 1);	// 1 as last parm means file is mem-mapped
		}

		Assert( Background_anim != NULL );
		anim_play_init(&aps, Background_anim, 0, 0);
		aps.framerate_independent = 1;
		aps.skip_frames = 0;
		Background_anim_instance = anim_play(&aps);
		Background_playing = 1;		// start playing the Background anim
	}
	*/
	Current_screen = Next_screen = ON_BRIEFING_SELECT;

	// load in the icons for the wing slots
	load_wing_icons(NOX("iconwing01"));


	#endif

	Current_screen = Next_screen = ON_BRIEFING_SELECT;
	
	Commit_pressed = 0;

	Common_select_inited = 1;

#ifndef NO_NETWORK
	// this handles the case where the player played a multiplayer game but now is in single player (in one instance
	// of Freespace)
	if(!(Game_mode & GM_MULTIPLAYER)){
		chatbox_close();
	}
#endif

	// get the value of the team
	Common_team = 0;							// assume the first team -- we'll change this value if we need to
#ifndef NO_NETWORK
	if ( (Game_mode & GM_MULTIPLAYER) && IS_MISSION_MULTI_TEAMS )
		Common_team = Net_player->p_info.team;
#endif

	ship_select_common_init();	
	weapon_select_common_init();
	common_flash_button_init();

#ifndef NO_NETWORK
	if ( Game_mode & GM_MULTIPLAYER ) {
		multi_ts_common_init();
	}
#endif

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

	// If the mouse went up, set flags.  We can't use mouse_up_count() more than once a frame,
	// since the count gets zeroed after the call.
	//
	Drop_icon_mflag = 0;
	Drop_on_wing_mflag = 0;
	Brief_mouse_up_flag = 0;

	if ( mouse_up_count(MOUSE_LEFT_BUTTON) ) {
		Drop_icon_mflag = 1;
		Drop_on_wing_mflag = 1;
		Brief_mouse_up_flag = 1;		
	}

	Mouse_down_last_frame = 0;
	if ( mouse_down_count(MOUSE_LEFT_BUTTON) ) {
		Mouse_down_last_frame = 1;
	}

	if ( help_overlay_active(BR_OVERLAY) || help_overlay_active(SS_OVERLAY) || help_overlay_active(WL_OVERLAY) ) {
		Common_buttons[0][gr_screen.res][COMMON_HELP_BUTTON].button.reset_status();
		Common_buttons[1][gr_screen.res][COMMON_HELP_BUTTON].button.reset_status();
		Common_buttons[2][gr_screen.res][COMMON_HELP_BUTTON].button.reset_status();
		Active_ui_window->set_ignore_gadgets(1);
	} else {
		Active_ui_window->set_ignore_gadgets(0);
	}

#ifndef NO_NETWORK
	k = chatbox_process();
#else
	k = 0;
#endif
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
				if ( brief_get_closeup_icon() != 0 ) {
					brief_turn_off_closeup_icon();
					break;
				}
			}

#ifndef NO_NETWORK
			// prompt the host of a multiplayer game
			if(Game_mode & GM_MULTIPLAYER){
				multi_quit_game(PROMPT_ALL);
			}
			else
#endif
			{
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

			#ifndef DEMO // not for FS2_DEMO
				if ( Current_screen != ON_WEAPON_SELECT && !Background_playing ) {
					if ( !wss_slots_all_empty() ) {
						Next_screen = ON_WEAPON_SELECT;
					} else {
						common_show_no_ship_error();
					}
				}
			#else
				gamesnd_play_iface(SND_GENERAL_FAIL);
			#endif

			break;

		case KEY_S:

			if ( brief_only_allow_briefing() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			#ifndef DEMO // not for FS2_DEMO
				if ( Current_screen != ON_SHIP_SELECT && !Background_playing ) {
					Next_screen = ON_SHIP_SELECT;
				}
			#else
				gamesnd_play_iface(SND_GENERAL_FAIL);
			#endif

			break;

		case KEY_SHIFTED+KEY_TAB:

			if ( brief_only_allow_briefing() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			#ifndef DEMO // not for FS2_DEMO
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
			#else
				gamesnd_play_iface(SND_GENERAL_FAIL);
			#endif

			break;

		case KEY_TAB:

			if ( brief_only_allow_briefing() ) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				break;
			}

			#ifndef DEMO // not for FS2_DEMO
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
			#else
				gamesnd_play_iface(SND_GENERAL_FAIL);
			#endif

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
#ifndef NO_NETWORK
	if(Game_mode & GM_MULTIPLAYER){
		multi_ts_close();
	} 
#endif
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

loadout_data Player_loadout;	// what the ship and weapon loadout is... used since we want to use the 
										// same loadout if the mission is played again

//wss_unit	Wss_slots[MAX_WSS_SLOTS];				// slot data struct
//int		Wl_pool[MAX_WEAPON_TYPES];				// weapon pool 
//int		Ss_pool[MAX_SHIP_TYPES];				// ship pool
//int		Wss_num_wings;								// number of player wings

wss_unit	Wss_slots_teams[MAX_TEAMS][MAX_WSS_SLOTS];
int		Wl_pool_teams[MAX_TEAMS][MAX_WEAPON_TYPES];
int		Ss_pool_teams[MAX_TEAMS][MAX_SHIP_TYPES];
int		Wss_num_wings_teams[MAX_TEAMS];

wss_unit	*Wss_slots;
int		*Wl_pool;
int		*Ss_pool;
int		Wss_num_wings;

// save ship selection loadout to the Player_loadout struct
void wss_save_loadout()
{
	int i,j;

	// save the ship pool
	for ( i = 0; i < MAX_SHIP_TYPES; i++ ) {
		Player_loadout.ship_pool[i] = Ss_pool[i]; 
	}

	// save the weapons pool
	for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
		Player_loadout.weapon_pool[i] = Wl_pool[i]; 
	}

	// save the ship class / weapons for each slot
	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		Player_loadout.unit_data[i].ship_class = Wss_slots[i].ship_class;

		for ( j = 0; j < MAX_WL_WEAPONS; j++ ) {
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

	// only restore if mission hasn't changed
	if ( stricmp(Player_loadout.last_modified, The_mission.modified) ) {
		return;
	}

	// restore the ship pool
	for ( i = 0; i < MAX_SHIP_TYPES; i++ ) {
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

		for ( j = 0; j < MAX_WL_WEAPONS; j++ ) {
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
			for ( p_objp = GET_FIRST(&ship_arrival_list); p_objp != END_OF_LIST(&ship_arrival_list); p_objp = GET_NEXT(p_objp) ) {
				slot = &Player_loadout.unit_data[i*4+j];
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
				slot = &Player_loadout.unit_data[i*4+j];
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

	// write the ship pool 
	for ( i = 0; i < MAX_SHIP_TYPES; i++ ) {
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
			ishort = (short)Wl_pool[i];
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
		for ( j = 0; j < MAX_WL_WEAPONS; j++ ) {
			// take care of sign issues
			Assert( Wss_slots[i].wep[j] < UCHAR_MAX );			
			if(Wss_slots[i].wep[j] == -1){
				block[offset++] = 0xff;
			} else {
				block[offset++] = (ubyte)(Wss_slots[i].wep[j]);
			}

			Assert( Wss_slots[i].wep_count[j] < SHRT_MAX );
			ishort = short(Wss_slots[i].wep_count[j]);

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
#ifndef NO_NETWORK
	if(player_index != -1){
		player_id = Net_players[player_index].player_id;		
	}
#endif

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

	// restore ship pool
	sanity=0;
	memset(Ss_pool, 0, MAX_SHIP_TYPES*sizeof(int));
	for (;;) {
		if ( sanity++ > MAX_SHIP_TYPES ) {
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
		Wl_pool[b1] = ishort;
	}

	for ( i=0; i<MAX_WSS_SLOTS; i++ ) {
		if(block[offset] == 0xff){
			Wss_slots[i].ship_class = -1;
		} else {
			Wss_slots[i].ship_class = block[offset];
		}
		offset++;		
		for ( j = 0; j < MAX_WL_WEAPONS; j++ ) {
			// take care of sign issues
			if(block[offset] == 0xff){
				Wss_slots[i].wep[j] = -1;
				offset++;
			} else {
				Wss_slots[i].wep[j] = (int)(block[offset++]);
			}
		
			memcpy( &ishort, &(block[offset]), sizeof(short) );
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
	offset += sizeof(short);
	
#ifndef NO_NETWORK
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
#endif

	return offset;
}

extern float View_zoom;
void draw_model_icon(int model_id, int flags, float closeup_zoom, int x, int y, int w, int h, ship_info *sip)
{
	matrix	object_orient	= IDENTITY_MATRIX;
	angles rot_angles = {0.0f,0.0f,0.0f};
	float zoom = closeup_zoom * 2.5f;

	if(sip == NULL)
	{
		//Assume it's a weapon
		rot_angles.h = -(PI/2);
	}
	else if(sip->flags & SIF_SMALL_SHIP)
	{
		rot_angles.p = -(PI/2);
	}
	else if((sip->max_speed <= 0.0f) && !(sip->flags & SIF_CARGO))
	{
		//Probably an installation or Knossos
		rot_angles.h = PI;
	}
	else
	{
		//Probably a capship
		rot_angles.h = PI/2;
	}
	vm_angles_2_matrix(&object_orient, &rot_angles);
	
	gr_set_clip(x, y, w, h);
	g3_start_frame(1);
	if(sip != NULL)
	{
		g3_set_view_matrix( &sip->closeup_pos, &vmd_identity_matrix, zoom);
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

		vector weap_closeup;
		float y_closeup;

		//Find the center of teh submodel
		weap_closeup.xyz.x = -(bs->min.xyz.z + (bs->max.xyz.z - bs->min.xyz.z)/2.0f);
		weap_closeup.xyz.y = bs->min.xyz.y + (bs->max.xyz.y - bs->min.xyz.y)/2.0f;
		weap_closeup.xyz.z = (weap_closeup.xyz.x/tan(zoom / 2.0f));

		y_closeup = -(weap_closeup.xyz.y/tan(zoom / 2.0f));
		if(y_closeup < weap_closeup.xyz.z)
		{
			weap_closeup.xyz.z = y_closeup;
		}
//		weap_closeup.xyz.x = bs->min.xyz.x + (bs->max.xyz.x - bs->min.xyz.x)/2.0f;
		g3_set_view_matrix( &weap_closeup, &vmd_identity_matrix, zoom);
	}

	model_set_detail_level(0);
	if (!Cmdline_nohtl) gr_set_proj_matrix( 0.5f*(4.0f/9.0f) * 3.14159f * View_zoom,  gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height, Min_draw_distance, Max_draw_distance);
	if (!Cmdline_nohtl)	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	if(!(flags & MR_NO_LIGHTING))
	{
		light_reset();
		vector light_dir = vmd_zero_vector;
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

// NEWSTUFF END
