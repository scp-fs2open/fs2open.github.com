/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "controlconfig/controlsconfig.h"
#include "freespace.h"
#include "gamehelp/gameplayhelp.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "io/key.h"
#include "missionui/missionscreencommon.h"
#include "sound/audiostr.h"
#include "ui/ui.h"
#include "weapon/weapon.h"



// text positioning constants
#define TITLE_Y			35
#define Y_START			70
#define X_OFFSET_1		70

// pixel offset for description of key from where key binding starts
// increased this from 193 to 250 to account for allowing multiple keys -Mjn
#define KEY_DESCRIPTION_OFFSET	250	

// different gameplay help pages
enum : int {
	// The first 9 are the retail pages
	GP_HELP_BASIC_KEYS,
	GP_HELP_MOVEMENT_KEYS,
	GP_HELP_COMMON_TARGET_KEYS,
	GP_HELP_ADVANCED_TARGET_KEYS,
	GP_HELP_MESSAGING,
	GP_HELP_WEAPON_KEYS,
	GP_HELP_THROTTLE_AND_ETS_KEYS,
	GP_HELP_VIEW_KEYS,
	GP_HELP_MISC_KEYS,
	GP_HELP_MULTI_KEYS,

	// Any others defined here will have unique sections within the vector
	// but will need special handling to display in the retail UI
	GP_HELP_FUNCTION_KEYS
};

#define GP_FIRST_SCREEN									0		// keep up to date
#define GP_LAST_SCREEN_SINGLE							8		// keep up to date
#define GP_LAST_SCREEN_MULTI							9		// keep up to date
#define GP_NUM_HELP_SECTIONS							10		// keep up to date

// set this to GP_LAST_SCREEN_SINGLE or GP_LAST_SCREEN_MULTI based upon what game mode we're in
static int Gp_last_screen;

#define NUM_BUTTONS						3
#define PREVIOUS_PAGE_BUTTON			0
#define NEXT_PAGE_BUTTON				1
#define CONTINUE_BUTTON					2

struct gameplay_help_buttons {
	const char *filename;
	int x, y;
	int hotspot;
	int tab;
	int flags;
	UI_BUTTON button;  // because we have a class inside this struct, we need the constructor below..

	gameplay_help_buttons(const char *name, int x1, int y1, int h) : filename(name), x(x1), y(y1), hotspot(h) {}
};

static gameplay_help_buttons Buttons[GR_NUM_RESOLUTIONS][NUM_BUTTONS] = {
//XSTR:OFF
	{
		gameplay_help_buttons("F1B_00",	15,	389,	0),
		gameplay_help_buttons("F1B_01",	60,	389,	1),
		gameplay_help_buttons("F1B_02",	574,	431,	2)
	},
	{
		gameplay_help_buttons("2_F1B_00",	24,	622,	0),
		gameplay_help_buttons("2_F1B_01",	96,	622,	1),
		gameplay_help_buttons("2_F1B_02",	919,	689,	2)
	},
//XSTR:ON
};

#define GAME_HELP_NUM_TEXT		2
static UI_XSTR Game_help_text[GR_NUM_RESOLUTIONS][GAME_HELP_NUM_TEXT] = {
	{ // GR_640
		{ "Press ESC to return to the game",	1441,	263,	389,	UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Continue",		1069,		571,	413,	UI_XSTR_COLOR_GREEN, -1, &Buttons[gr_screen.res][CONTINUE_BUTTON].button }
	}, 
	{ // GR_1024
		{ "Press ESC to return to the game",	1441,	421,	622,	UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Continue",		1069,		928,	663,	UI_XSTR_COLOR_GREEN, -1, &Buttons[gr_screen.res][CONTINUE_BUTTON].button }
	}
};

static const char *Game_help_filename[GR_NUM_RESOLUTIONS] = {
	"F1",
	"2_F1"
};

static const char *Game_help_mask_filename[GR_NUM_RESOLUTIONS] = {
	"F1-m",
	"2_F1-m"
};

static UI_WINDOW Ui_window;
static int Background_bitmap;
static int Gameplay_help_inited = 0;

static int Current_help_page;

static SCP_vector<gameplay_help_section> Help_text;

void gameplay_help_init_control_line(int id, gameplay_help_section &thisHelp)
{
	char buf[256];

	auto ci = &Control_config[id];

	buf[0] = 0;

	strcpy_s(buf, ci->first.textify().c_str());

	if (!ci->second.empty()) {
		strcat_s(buf, XSTR(", ", 129));
		strcat_s(buf, ci->second.textify().c_str());
	}

	if (ci->empty()) {
		strcpy_s(buf, XSTR("no binding", 130));
	}

	thisHelp.key.push_back(buf);
	thisHelp.text.push_back(XSTR(ci->text.c_str(), CONTROL_CONFIG_XSTR + id));
}

void gameplay_help_init_control_line_raw(const char* control_text, const char* control_description, gameplay_help_section &thisHelp)
{
	thisHelp.key.push_back(control_text);
	thisHelp.text.push_back(control_description);
}

// displays the given help key and text at the specified
// x and y coordinates
void gameplay_help_blit_help_text(int x, int y, const SCP_string *control_text, const SCP_string *control_description)
{
	gr_string(x, y, control_text->c_str(), GR_RESIZE_MENU);
	gr_string(x + KEY_DESCRIPTION_OFFSET, y, control_description->c_str(), GR_RESIZE_MENU);
}

// displays a help section header at the specified y coordinate and an x coordinate
// calculated from the header string's length
void gameplay_help_blit_header_text(int y_offset, const SCP_string *header)
{
	gr_set_color_fast(&Color_bright);
	int w;
	gr_get_string_size(&w, nullptr, header->c_str());
	gr_printf_menu((gr_screen.clip_width - w) / 2, y_offset, "%s", header->c_str());
	gr_set_color_fast(&Color_normal);
}

// game_play_help_set_title() will display the title for the help screen and
// set the font for the rest of the screen
void gameplay_help_set_title(const SCP_string *title)
{
	int sy=TITLE_Y;
	char buf[128];

	gr_set_color_fast(&Color_bright);
	int w;
	gr_get_string_size(&w, nullptr, title->c_str());

	gr_printf_menu((gr_screen.clip_width_unscaled - w) / 2,sy,"%s", title->c_str());

	sprintf(buf, XSTR("Page %d of %d", 132), Current_help_page + 1, Gp_last_screen + 1);
	gr_get_string_size(&w, NULL, buf);
	gr_printf_menu((gr_screen.clip_width_unscaled - w) / 2, sy + gr_get_font_height() + 2, "%s", buf);
	gr_set_color_fast(&Color_normal);
}

// Inits all the gameplay help text and returns it
// in a nice organized vector of help sections
SCP_vector<gameplay_help_section> gameplay_help_init_text()
{
	SCP_vector<gameplay_help_section> complete_help_text;

	for (int i = 0; i <= GP_NUM_HELP_SECTIONS; i++) {

		gameplay_help_section thisHelp;

		switch (i) {

		case GP_HELP_BASIC_KEYS:
			thisHelp.title = XSTR("Basic Keys", 133);

			thisHelp.header = "";

			gameplay_help_init_control_line(END_MISSION, thisHelp);
			gameplay_help_init_control_line(FIRE_PRIMARY, thisHelp);
			gameplay_help_init_control_line(MAX_THROTTLE, thisHelp);
			gameplay_help_init_control_line(ZERO_THROTTLE, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV, thisHelp);
			gameplay_help_init_control_line(TOGGLE_AUTO_TARGETING, thisHelp);

			break;

		case GP_HELP_MOVEMENT_KEYS:
			thisHelp.title = XSTR("Movement Keys", 147);

			thisHelp.header = "";

			gameplay_help_init_control_line(FORWARD_THRUST, thisHelp);
			gameplay_help_init_control_line(MAX_THROTTLE, thisHelp);
			gameplay_help_init_control_line(ONE_THIRD_THROTTLE, thisHelp);
			gameplay_help_init_control_line(TWO_THIRDS_THROTTLE, thisHelp);
			gameplay_help_init_control_line(MINUS_5_PERCENT_THROTTLE, thisHelp);
			gameplay_help_init_control_line(PLUS_5_PERCENT_THROTTLE, thisHelp);
			gameplay_help_init_control_line(ZERO_THROTTLE, thisHelp);
			gameplay_help_init_control_line(AFTERBURNER, thisHelp);
			gameplay_help_init_control_line(REVERSE_THRUST, thisHelp);
			gameplay_help_init_control_line(PITCH_FORWARD, thisHelp);
			gameplay_help_init_control_line(PITCH_BACK, thisHelp);
			gameplay_help_init_control_line(YAW_LEFT, thisHelp);
			gameplay_help_init_control_line(YAW_RIGHT, thisHelp);
			gameplay_help_init_control_line(BANK_LEFT, thisHelp);
			gameplay_help_init_control_line(BANK_RIGHT, thisHelp);
			gameplay_help_init_control_line(BANK_WHEN_PRESSED, thisHelp);

			break;

		case GP_HELP_COMMON_TARGET_KEYS:
			thisHelp.title = XSTR("Basic Targeting Keys", 148);

			thisHelp.header = "";

			gameplay_help_init_control_line(TARGET_NEXT, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV, thisHelp);
			gameplay_help_init_control_line(TOGGLE_AUTO_TARGETING, thisHelp);
			gameplay_help_init_control_line(TARGET_SHIP_IN_RETICLE, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT_CLOSEST_HOSTILE, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV_CLOSEST_HOSTILE, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT_CLOSEST_FRIENDLY, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV_CLOSEST_FRIENDLY, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT_SUBOBJECT, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV_SUBOBJECT, thisHelp);
			gameplay_help_init_control_line(TARGET_SUBOBJECT_IN_RETICLE, thisHelp);
			gameplay_help_init_control_line(MATCH_TARGET_SPEED, thisHelp);
			gameplay_help_init_control_line(TOGGLE_AUTO_MATCH_TARGET_SPEED, thisHelp);

			break;

		case GP_HELP_ADVANCED_TARGET_KEYS:
			thisHelp.title = XSTR("Advanced Targeting Keys", 149);

			thisHelp.header = "";

			gameplay_help_init_control_line(TARGET_CLOSEST_SHIP_ATTACKING_SELF, thisHelp);
			gameplay_help_init_control_line(TARGET_CLOSEST_SHIP_ATTACKING_TARGET, thisHelp);
			gameplay_help_init_control_line(TARGET_LAST_TRANMISSION_SENDER, thisHelp);
			gameplay_help_init_control_line(TARGET_TARGETS_TARGET, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT_ESCORT_SHIP, thisHelp);
			gameplay_help_init_control_line(TARGET_CLOSEST_REPAIR_SHIP, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT_UNINSPECTED_CARGO, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV_UNINSPECTED_CARGO, thisHelp);
			gameplay_help_init_control_line(TARGET_NEWEST_SHIP, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT_BOMB, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV_BOMB, thisHelp);
			gameplay_help_init_control_line(STOP_TARGETING_SHIP, thisHelp);
			gameplay_help_init_control_line(TARGET_NEXT_LIVE_TURRET, thisHelp);
			gameplay_help_init_control_line(TARGET_PREV_LIVE_TURRET, thisHelp);
			gameplay_help_init_control_line(STOP_TARGETING_SUBSYSTEM, thisHelp);
			gameplay_help_init_control_line_raw(XSTR("F5...F12", 143), XSTR("Select target assigned to that hotkey", 150), thisHelp);
			gameplay_help_init_control_line_raw(XSTR("Shift-F5...F12", 151), XSTR("Add/remove target from that hotkey", 152), thisHelp);
			gameplay_help_init_control_line_raw(XSTR("Alt-Shift-F5...F12", 153), XSTR("Clear that hotkey", 154), thisHelp);

			break;

		case GP_HELP_MESSAGING:
			thisHelp.title = XSTR("Messaging Keys", 155);

			thisHelp.header = "";

			gameplay_help_init_control_line(SQUADMSG_MENU, thisHelp);
			gameplay_help_init_control_line(REARM_MESSAGE, thisHelp);
			gameplay_help_init_control_line(ATTACK_MESSAGE, thisHelp);
			gameplay_help_init_control_line(DISARM_MESSAGE, thisHelp);
			gameplay_help_init_control_line(DISABLE_MESSAGE, thisHelp);
			gameplay_help_init_control_line(CAPTURE_MESSAGE, thisHelp);
			gameplay_help_init_control_line(ENGAGE_MESSAGE, thisHelp);
			gameplay_help_init_control_line(FORM_MESSAGE, thisHelp);
			gameplay_help_init_control_line(IGNORE_MESSAGE, thisHelp);
			gameplay_help_init_control_line(PROTECT_MESSAGE, thisHelp);
			gameplay_help_init_control_line(COVER_MESSAGE, thisHelp);
			gameplay_help_init_control_line(WARP_MESSAGE, thisHelp);
			gameplay_help_init_control_line_raw(XSTR("F5...F12", 143),
				XSTR("send specified order to these target(s)", 156),
				thisHelp);

			break;

		case GP_HELP_WEAPON_KEYS:
			thisHelp.title = XSTR("Weapon Keys", 157);

			thisHelp.header = "";

			gameplay_help_init_control_line(FIRE_PRIMARY, thisHelp);
			gameplay_help_init_control_line(FIRE_SECONDARY, thisHelp);
			gameplay_help_init_control_line(CYCLE_NEXT_PRIMARY, thisHelp);
			gameplay_help_init_control_line(CYCLE_PREV_PRIMARY, thisHelp);
			gameplay_help_init_control_line(CYCLE_SECONDARY, thisHelp);
			gameplay_help_init_control_line(CYCLE_NUM_MISSLES, thisHelp);
			gameplay_help_init_control_line(LAUNCH_COUNTERMEASURE, thisHelp);

			break;

		case GP_HELP_THROTTLE_AND_ETS_KEYS:
			thisHelp.title = XSTR("Energy Management Keys", 158);

			thisHelp.header = "";

			gameplay_help_init_control_line(SHIELD_EQUALIZE, thisHelp);
			gameplay_help_init_control_line(SHIELD_XFER_TOP, thisHelp);
			gameplay_help_init_control_line(SHIELD_XFER_BOTTOM, thisHelp);
			gameplay_help_init_control_line(SHIELD_XFER_LEFT, thisHelp);
			gameplay_help_init_control_line(SHIELD_XFER_RIGHT, thisHelp);
			gameplay_help_init_control_line(INCREASE_WEAPON, thisHelp);
			gameplay_help_init_control_line(DECREASE_WEAPON, thisHelp);
			gameplay_help_init_control_line(INCREASE_SHIELD, thisHelp);
			gameplay_help_init_control_line(DECREASE_SHIELD, thisHelp);
			gameplay_help_init_control_line(INCREASE_ENGINE, thisHelp);
			gameplay_help_init_control_line(DECREASE_ENGINE, thisHelp);
			gameplay_help_init_control_line(ETS_EQUALIZE, thisHelp);
			/*
			gameplay_help_init_control_line(XFER_LASER, thisHelp);
			gameplay_help_init_control_line(XFER_SHIELD, thisHelp);
			*/

			break;

		case GP_HELP_MISC_KEYS:
			thisHelp.title = XSTR("Miscellaneous Keys", 159);

			thisHelp.header = "";

			// ending mission
			gameplay_help_init_control_line(END_MISSION, thisHelp);
			gameplay_help_init_control_line_raw(XSTR("ESC", 160), XSTR("invoke abort mission popup", 161), thisHelp);

			// time compression
			gameplay_help_init_control_line(TIME_SPEED_UP, thisHelp);
			gameplay_help_init_control_line(TIME_SLOW_DOWN, thisHelp);

			// radar keys
			gameplay_help_init_control_line(RADAR_RANGE_CYCLE, thisHelp);

			// escort view keys
			gameplay_help_init_control_line(ADD_REMOVE_ESCORT, thisHelp);
			gameplay_help_init_control_line(ESCORT_CLEAR, thisHelp);

			// Pause, Print Screenshot
			gameplay_help_init_control_line_raw(XSTR("Pause", 162), XSTR("pause game", 163), thisHelp);
			gameplay_help_init_control_line_raw(XSTR("Print Scrn", 164), XSTR("take screen shot", 165), thisHelp);

			break;

		case GP_HELP_VIEW_KEYS:
			thisHelp.title = XSTR("View Keys", 166);

			thisHelp.header = "";

			gameplay_help_init_control_line(PADLOCK_UP, thisHelp);
			gameplay_help_init_control_line(PADLOCK_DOWN, thisHelp);
			gameplay_help_init_control_line(PADLOCK_LEFT, thisHelp);
			gameplay_help_init_control_line(PADLOCK_RIGHT, thisHelp);
			gameplay_help_init_control_line(VIEW_CHASE, thisHelp);
			gameplay_help_init_control_line(VIEW_EXTERNAL, thisHelp);
			gameplay_help_init_control_line(VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK, thisHelp);
			gameplay_help_init_control_line(VIEW_SLEW, thisHelp);
			gameplay_help_init_control_line(VIEW_DIST_INCREASE, thisHelp);
			gameplay_help_init_control_line(VIEW_DIST_DECREASE, thisHelp);
			gameplay_help_init_control_line(VIEW_CENTER, thisHelp);
			gameplay_help_init_control_line(VIEW_OTHER_SHIP, thisHelp);

			break;

		case GP_HELP_MULTI_KEYS:
			thisHelp.title = XSTR("Multiplayer Keys", 167);

			thisHelp.header = XSTR("Ingame messaging keys (tap for text, hold for voice)", 168);

			gameplay_help_init_control_line(MULTI_MESSAGE_ALL, thisHelp);
			gameplay_help_init_control_line(MULTI_MESSAGE_FRIENDLY, thisHelp);
			gameplay_help_init_control_line(MULTI_MESSAGE_HOSTILE, thisHelp);
			gameplay_help_init_control_line(MULTI_MESSAGE_TARGET, thisHelp);

			break;

		case GP_HELP_FUNCTION_KEYS:
			thisHelp.title = XSTR("Function Keys", 134);

			thisHelp.header = "";

			gameplay_help_init_control_line_raw(XSTR("F1", 135), XSTR("context-sensitive help", 136), thisHelp);
			gameplay_help_init_control_line_raw(XSTR("F2", 137),
				XSTR("options screen (available anywhere in game)", 138),
				thisHelp);
			gameplay_help_init_control_line_raw(XSTR("F3", 139), XSTR("hotkey assignment", 140), thisHelp);
			gameplay_help_init_control_line_raw(XSTR("F4", 141), XSTR("HUD message scroll-back", 142), thisHelp);
			gameplay_help_init_control_line_raw(XSTR("F5...F12", 143), XSTR("hotkeys", 144), thisHelp);
			gameplay_help_init_control_line_raw(XSTR("Shift-Esc", 145),
				XSTR("quit FreeSpace 2 immediately", 146),
				thisHelp);

			break;

		}

		complete_help_text.push_back(thisHelp);
	}

	return complete_help_text;
}

// called once when the gameplay help state is entered
void gameplay_help_init()
{
	int i;
	gameplay_help_buttons *b;

	if ( Gameplay_help_inited ) {
		return;
	}

	// init all the help text
	Help_text = gameplay_help_init_text();

	common_set_interface_palette("InterfacePalette");  // set the interface palette
	Ui_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Ui_window.set_mask_bmap(Game_help_mask_filename[gr_screen.res]);

	for (i=0; i<NUM_BUTTONS; i++) {
		b = &Buttons[gr_screen.res][i];

		b->button.create(&Ui_window, "", b->x, b->y, 60, 30, 0, 1);
		// set up callback for when a mouse first goes over a button
		b->button.set_highlight_action(common_play_highlight_sound);
		b->button.set_bmaps(b->filename);
		b->button.link_hotspot(b->hotspot);
	}

	// add all xstrs
	for (i=0; i<GAME_HELP_NUM_TEXT; i++)
	{
		Ui_window.add_XSTR(&Game_help_text[gr_screen.res][i]);
	}

	// set the proper last screen # based upon game mode
	if(Game_mode & GM_MULTIPLAYER){
		Gp_last_screen = GP_LAST_SCREEN_MULTI;
	} else {
		Gp_last_screen = GP_LAST_SCREEN_SINGLE;
	}

	// setup hotkeys so lights flash when keys are pressed
	Buttons[gr_screen.res][CONTINUE_BUTTON].button.set_hotkey(KEY_CTRLED | KEY_ENTER);
	Buttons[gr_screen.res][PREVIOUS_PAGE_BUTTON].button.set_hotkey(KEY_LEFT);
	Buttons[gr_screen.res][NEXT_PAGE_BUTTON].button.set_hotkey(KEY_RIGHT);

	Background_bitmap = bm_load(Game_help_filename[gr_screen.res]);

	Current_help_page = GP_FIRST_SCREEN;
	Gameplay_help_inited = 1;
}

// called when moving to the previous help page
void gameplay_help_goto_prev_screen()
{
	Current_help_page--;
	if (Current_help_page < GP_FIRST_SCREEN) {
		Current_help_page = Gp_last_screen;
	}
	gamesnd_play_iface(InterfaceSounds::SWITCH_SCREENS);

}

// called when proceeding to the next help page
void gameplay_help_goto_next_screen()
{
	Current_help_page++;
	if (Current_help_page > Gp_last_screen) {
		Current_help_page = GP_FIRST_SCREEN;
	}
	gamesnd_play_iface(InterfaceSounds::SWITCH_SCREENS);
}

// called when the screen is exited
void gameplay_help_leave()
{
	// unpause all game sounds
	weapon_unpause_sounds();
	audiostream_unpause_all();

	gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	game_flush();
}

// deal with a keypress on the gameplay help screen
void gameplay_help_process_key(int k)
{
	switch ( k ) {
		case KEY_ESC:
			gameplay_help_leave();
			break;

		case KEY_ENTER:
		case KEY_SPACEBAR:
		case KEY_TAB:
			//gameplay_help_goto_next_screen();
			Buttons[gr_screen.res][NEXT_PAGE_BUTTON].button.press_button();
			break;

		case KEY_SHIFTED | KEY_TAB:
			Buttons[gr_screen.res][PREVIOUS_PAGE_BUTTON].button.press_button();
//			gameplay_help_goto_prev_screen();
			break;

		default:
			break;

	} // end switch
}

// deal with buttons being pressed on the gameplay help screen
void gameplay_help_button_pressed(int n)
{
	switch (n) {

	case PREVIOUS_PAGE_BUTTON:
		gameplay_help_goto_prev_screen();
		break;

	case NEXT_PAGE_BUTTON:
		gameplay_help_goto_next_screen();
		break;

	case CONTINUE_BUTTON:
		gameplay_help_leave();
		gamesnd_play_iface(InterfaceSounds::COMMIT_PRESSED);
		break;

	default:
		Int3();
		break;
	}
}

// Draw the help text onto the screen
void gameplay_help_draw_text()
{
	int x_offset, y_offset, separation;

	separation = gr_get_font_height() + 3;

	gameplay_help_section thisHelp = Help_text[Current_help_page];

	gameplay_help_set_title(&thisHelp.title);
	x_offset = X_OFFSET_1;
	y_offset = Y_START;

	// if we have a header then display it
	if (thisHelp.header.length() > 0) {
		gameplay_help_blit_header_text(y_offset, &thisHelp.header);
		y_offset += separation;
	}

	// blit help text for each key
	for (int i = 0; i < (int)thisHelp.key.size(); i++) {
		y_offset += separation;
		gameplay_help_blit_help_text(x_offset, y_offset, &thisHelp.key[i], &thisHelp.text[i]);
	}

	// Special handling of function keys to display on the same page as the basic keys
	if (Current_help_page == GP_HELP_BASIC_KEYS) {

		y_offset += separation;
		y_offset += separation;

		thisHelp = Help_text[GP_HELP_FUNCTION_KEYS];

		gameplay_help_blit_header_text(y_offset, &thisHelp.title);

		y_offset += separation;
		y_offset += separation;

		// blit help text for each key
		for (int i = 0; i < (int)thisHelp.key.size(); i++) {
			y_offset += separation;
			gameplay_help_blit_help_text(x_offset, y_offset, &thisHelp.key[i], &thisHelp.text[i]);
		}
	}		
}

// gameplay_help_do_frame() is the function that displays help when acutally playing the game
void gameplay_help_do_frame(float  /*frametime*/)
{
	int i, k;	

	// ensure the gameplay help interface has been initialized
	if (!Gameplay_help_inited) {
		Int3();
		return;
	}

	// make sure game sounds are paused
	weapon_pause_sounds();
	audiostream_pause_all();

	k = Ui_window.process() & ~KEY_DEBUGGED;
	gameplay_help_process_key(k);

	for (i=0; i<NUM_BUTTONS; i++){
		if (Buttons[gr_screen.res][i].button.pressed()){
			gameplay_help_button_pressed(i);
		}
	}
	
	GR_MAYBE_CLEAR_RES(Background_bitmap);
	if (Background_bitmap >= 0) {
		gr_set_bitmap(Background_bitmap);
		gr_bitmap(0, 0, GR_RESIZE_MENU);
	}

	Ui_window.draw();

	gameplay_help_draw_text();
	gr_flip();
}
