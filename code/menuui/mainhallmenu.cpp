/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
 */

#include <stdlib.h>
#include <limits.h>

#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "cmdline/cmdline.h"
#include "freespace2/freespace.h"
#include "gamehelp/contexthelp.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "graphics/generic.h"
#include "io/key.h"
#include "io/mouse.h"
#include "io/timer.h"
#include "menuui/fishtank.h"
#include "menuui/mainhallmenu.h"
#include "menuui/playermenu.h"
#include "menuui/snazzyui.h"
#include "mission/missioncampaign.h"
#include "network/multi.h"
#include "network/multi_voice.h"
#include "network/multiui.h"
#include "network/multiutil.h"
#include "palman/palman.h"
#include "parse/parselo.h"
#include "parse/scripting.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "sound/audiostr.h"

#ifndef NDEBUG
#include "cutscene/movie.h"
#include "mission/missionload.h"
#endif


// A reference to io/keycontrol.cpp
extern void game_process_cheats(int k);

// ----------------------------------------------------------------------------
// MAIN HALL DATA DEFINES
//
#define MISC_ANIM_MODE_LOOP			0		// loop the animation
#define MISC_ANIM_MODE_HOLD			1		// play to the end and hold the animation
#define MISC_ANIM_MODE_TIMED		2		// uses timestamps to determine when a finished anim should be checked again
#define NUM_REGIONS					7		// (6 + 1 for multiplayer equivalent of campaign room)
#define MAIN_HALL_MAX_CHEAT_LEN		40		// cheat buffer length (also maximum cheat length)

SCP_vector< SCP_vector<main_hall_defines> > Main_hall_defines;

static main_hall_defines *Main_hall = NULL;

static int Main_hall_music_index = -1;

int Vasudan_funny = 0;
int Vasudan_funny_plate = -1;

SCP_string Main_hall_cheat = "";

// ----------------------------------------------------------------------------
// MISC interface data
//
// is the main hall inited (for reentrancy)
int Main_hall_inited = 0;

// handle to the playing music
int Main_hall_music_handle = -1;

// background bitmap handle
int Main_hall_bitmap;

// background bitmap dimensions
int Main_hall_bitmap_w;
int Main_hall_bitmap_h;

// background bitmap mask handle
int Main_hall_mask;

// bitmap struct for th background mask bitmap
bitmap *Main_hall_mask_bitmap;

// actual data for the background mask bitmap
ubyte *Main_hall_mask_data;

int Main_hall_mask_w, Main_hall_mask_h;


// ----------------------------------------------------------------------------
// MOUSE clicking stuff
//
// indicates whether a right click occured
int Main_hall_right_click;

// use this to cycle through the selectable regions instead of the mouse's current region
int Main_hall_last_clicked_region;

// use this to determine how long the cursor has to linger on a region before it starts playing
#define MAIN_HALL_REGION_LINGER		175		// in ms
int Main_hall_region_linger_stamp = -1;

// handle any right clicks which may have occured
void main_hall_handle_right_clicks();


// ----------------------------------------------------------------------------
// RANDOM intercom sounds
//

// next random intercom sound to play
int Main_hall_next_intercom_sound = 0;

// delay for the next intercom sound
int Main_hall_next_intercom_sound_stamp = -1;

// handle to any playing instance of a random intercom sound
int Main_hall_intercom_sound_handle = -1;

// handle any details related to random intercom sounds
void main_hall_handle_random_intercom_sounds();


// ----------------------------------------------------------------------------
// MISC animations
//

// the misc animations themselves
SCP_vector<generic_anim> Main_hall_misc_anim;

// render all playing misc animations
void main_hall_render_misc_anims(float frametime, bool over_doors);


// ----------------------------------------------------------------------------
// DOOR animations (not all of these are doors anymore, but they're doorlike _regions_)
//
#define DOOR_TEXT_X 100
#define DOOR_TEXT_Y 450

// the door animations themselves
SCP_vector<generic_anim> Main_hall_door_anim;

// render all playing door animations
void main_hall_render_door_anims(float frametime);


// ----------------------------------------------------------------------------
// SNAZZY MENU stuff
//
#define NUM_MAIN_HALL_MAX_REGIONS 20

// region mask #'s (identifiers)
#define EXIT_REGION				0
#define BARRACKS_REGION			1
#define READY_ROOM_REGION		2
#define TECH_ROOM_REGION		3
#define OPTIONS_REGION			4
#define CAMPAIGN_ROOM_REGION	5
#define MULTIPLAYER_REGION		10
#define LOAD_MISSION_REGION		11
#define QUICK_START_REGION		12
#define SKILL_LEVEL_REGION		13
#define SCRIPT_REGION			14
#define START_REGION			15

struct main_hall_region_info {
	int mask;
	char *name;
};

main_hall_region_info Main_hall_region_map[] = {
	{ EXIT_REGION, "Exit" },
	{ BARRACKS_REGION, "Barracks" },
	{ READY_ROOM_REGION, "Readyroom" },
	{ TECH_ROOM_REGION, "Techroom" },
	{ OPTIONS_REGION, "Options" },
	{ CAMPAIGN_ROOM_REGION, "Campaigns" },
	{ MULTIPLAYER_REGION, "Multiplayer" },
	{ LOAD_MISSION_REGION, "Load Mission" },
	{ QUICK_START_REGION, "Quickstart" },
	{ SKILL_LEVEL_REGION, "Skilllevel" },
	{ SCRIPT_REGION, "Script" },
	{ START_REGION, "Start" },
	{ -1, NULL }
};

// all the menu regions in the main hall
MENU_REGION Main_hall_region[NUM_MAIN_HALL_MAX_REGIONS];

// region over which the mouse is currently residing, or -1 if over no region
// NOTE : you should nevery change this directly. Always use main_hall_handle_mouse_location(int)
//        to do this. Otherwise, the door opening and closing animations will get screwed up
int Main_hall_mouse_region;

// set this to skip a frame
int Main_hall_frame_skip;

// do any necessary processing based upon the mouse location
void main_hall_handle_mouse_location(int cur_region);

// if the mouse has moved off of the currently active region, handle the anim accordingly
void main_hall_mouse_release_region(int region);

// if the mouse has moved on this region, handle it accordingly
void main_hall_mouse_grab_region(int region);


// ----------------------------------------------------------------------------
// SOUND data / handlers
// -

// toaster oven room sound idex
#define TOASTER_REGION		3

// everyone's favorite desk guardian
#define ALLENDER_REGION		4

// handles to the sound instances of the doors opening/closing
SCP_vector<int> Main_hall_door_sound_handles;

// sound handle for looping ambient sound
int Main_hall_ambient_loop = -1;

// cull any door sounds that have finished playing
void main_hall_cull_door_sounds();

// handle starting, stopping and reversing "door" animations
void main_hall_handle_region_anims();

// to determine if we should continue playing sounds and random animations
static int Main_hall_paused = 0;


// ----------------------------------------------------------------------------
// warning/notification messages
//
#define MAIN_HALL_NOTIFY_TIME	3500

// timestamp for the notification messages
int Main_hall_notify_stamp = -1;

// text to display as the current notification message
char Main_hall_notify_text[300]="";

// set the current notification string and the associated timestamp
void main_hall_set_notify_string(const char *str);

// handle any drawing, culling, etc of notification messages
void main_hall_notify_do();


// ----------------------------------------------------------------------------
// MISC functions
//

// upper _RIGHT_ corner for the version text
#define MAIN_HALL_VERSION_X		630
#define MAIN_HALL_VERSION_Y		467

// main hall help overlay ID
int Main_hall_overlay_id;

// blit the freespace version #
void main_hall_blit_version();

// blit any necessary tooltips
void main_hall_maybe_blit_tooltips();

// shader for behind tooltips
shader Main_hall_tooltip_shader;

// num pixels shader is above/below tooltip text
static int Main_hall_default_tooltip_padding[GR_NUM_RESOLUTIONS] = {
	4,		// GR_640
	7,		// GR_1024
};
static int Main_hall_f1_text_frame = 0;
static int F1_text_done = 0;

// "press f1" for help stuff
#define MAIN_HALL_HELP_TIME		5000
int Main_hall_help_stamp = -1;
void main_hall_process_help_stuff();


// ----------------------------------------------------------------------------
// VOICE RECORDING STUFF
//

// are we currently recording voice?
int Recording = 0;

/*
 * Called when multiplayer clicks on the ready room door.  May pop up dialog depending on network
 * connection status and errors
 */
void main_hall_do_multi_ready()
{
	int error;

	error = psnet_get_network_status();
	switch( error ) {
	case NETWORK_ERROR_NO_TYPE:
		popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have not defined your type of Internet connection.  Please run the Launcher, hit the setup button, and go to the Network tab and choose your connection type.", 360));
		break;
	case NETWORK_ERROR_NO_WINSOCK:
		popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "Winsock is not installed.  You must have TCP/IP and Winsock installed to play multiplayer FreeSpace.", 361));
		break;
	case NETWORK_ERROR_NO_PROTOCOL:
		if (Multi_options_g.protocol == NET_TCP) {
			popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "TCP/IP protocol not found.  This protocol is required for multiplayer FreeSpace.", 1602));
		} else {
			Assert(Multi_options_g.protocol == NET_IPX);
			popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "IPX protocol not found.  This protocol is required for multiplayer FreeSpace.", 1603));
		}
		break;
	case NETWORK_ERROR_CONNECT_TO_ISP:
		popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected Dial Up Networking as your type of connection to the Internet.  You are not currently connected.  You must connect to your ISP before continuing on past this point.", 363));
		break;
	case NETWORK_ERROR_LAN_AND_RAS:
		popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have indicated that you use a LAN for networking.  You also appear to be dialed into your ISP.  Please disconnect from your service provider, or choose Dial Up Networking.", 364));
		break;

	case NETWORK_ERROR_NONE:
	default:
		break;
	}

	// if our selected protocol is not active
	if ((Multi_options_g.protocol == NET_TCP) && !Tcp_active) {
		if (Tcp_failure_code == WSAEADDRINUSE) {
			popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected TCP/IP for multiplayer FreeSpace, but the TCP socket is already in use.  Check for another instance and/or use the \"-port <port_num>\" command line option to select an available port.", 1604));
		} else {
			popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected TCP/IP for multiplayer FreeSpace, but the TCP/IP protocol was not detected on your machine.", 362));
		}
		return;
	}

	if ((Multi_options_g.protocol == NET_IPX) && !Ipx_active) {
		popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected IPX for multiplayer FreeSpace, but the IPX protocol was not detected on your machine.", 1402));
		return;
	}

	if (error != NETWORK_ERROR_NONE) {
		return;
	}

	// 7/9/98 -- MWA.  Deal with the connection speed issue.  make a call to the multiplayer code to
	// determine is a valid connection setting exists
	if (Multi_connection_speed == CONNECTION_SPEED_NONE) {
		popup( PF_USE_AFFIRMATIVE_ICON | PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You must define your connection speed.  Please run the Launcher, hit the setup button, and go to the Network tab and choose your connection speed.", 986) );
		return;
	}

	// go to parallax online
	if (Om_tracker_flag) {
		Multi_options_g.protocol = NET_TCP;
		gameseq_post_event(GS_EVENT_PXO);
	} else {
		// go to the regular join game screen 
		gameseq_post_event(GS_EVENT_MULTI_JOIN_GAME);
	}

	// select protocol
	psnet_use_protocol(Multi_options_g.protocol);
}


// blit some small color indicators to show whether ships.tbl and weapons.tbl are valid
// green == valid, red == invalid.
// ships.tbl will be on the left, weapons.tbl on the right
void main_hall_blit_table_status()
{
	// blit ship table status
	gr_set_color_fast(Game_ships_tbl_valid ? &Color_bright_green : &Color_bright_red);
	gr_line(1, gr_screen.max_h_unscaled_zoomed - 1, 1, gr_screen.max_h_unscaled_zoomed - 1, GR_RESIZE_MENU_ZOOMED);

	// blit weapon table status
	gr_set_color_fast(Game_weapons_tbl_valid ? &Color_bright_green : &Color_bright_red);
	gr_line(3, gr_screen.max_h_unscaled_zoomed - 1, 3, gr_screen.max_h_unscaled_zoomed - 1, GR_RESIZE_MENU_ZOOMED);
}

/**
 * Bash the player to a specific mission in a campaign
 */
void main_hall_campaign_cheat()
{
	char *ret = popup_input(0, XSTR("Enter mission name.\n\n* This will destroy all legitimate progress in this campaign. *", 1605));

	// yay
	if (ret != NULL) {
		mission_campaign_jump_to_mission(ret);
	}
}

// -------------------------------------------------------------------------------------------------------------------
// FUNCTION DEFINITIONS BEGIN
//

/**
 * Initialize the main hall proper
 *
 * @param main_hall_name Name of main hall to initialise
 */
void main_hall_init(const SCP_string &main_hall_name)
{
	BM_TYPE bg_type;
	if (Main_hall_inited) {
		return;
	}

	int idx;
	SCP_string main_hall_to_load;

	// reparse the table here if the relevant cmdline flag is set
	if (Cmdline_reparse_mainhall) {
		main_hall_table_init();
	}

	// sanity checks
	if (Main_hall_defines.size() == 0) {
		Error(LOCATION, "No main halls were loaded to initialize.");
	} else if (main_hall_name == "") {
		Warning(LOCATION, "main_hall_init() was passed a blank main hall name; loading first available main hall.");
		main_hall_get_name(main_hall_to_load, 0);
	} else if (main_hall_get_pointer(main_hall_name) == NULL) {
		Warning(LOCATION, "Tried to load a main hall called '%s', but it does not exist; loading first available main hall.", main_hall_name.c_str());
		main_hall_get_name(main_hall_to_load, 0);
	} else {
		main_hall_to_load = main_hall_name;
	}

	// if we're switching to a different mainhall we may need to change music
	if (main_hall_get_music_index(main_hall_get_index(main_hall_to_load)) != Main_hall_music_index) {
		main_hall_stop_music(true);
	}

	// create the snazzy interface and load up the info from the table
	snazzy_menu_init();
	
	// assign the proper main hall data
	Main_hall = main_hall_get_pointer(main_hall_to_load);
	Assertion(Main_hall != NULL, "Failed to obtain pointer to main hall '%s'; get a coder!\n", main_hall_to_load.c_str());

	// check if we have to change the ready room's description
	if(Main_hall->default_readyroom) {
		if (Player->flags & PLAYER_FLAGS_IS_MULTI) {
			Main_hall->regions[2].description = XSTR( "Multiplayer - Start or join a multiplayer game", 359);
		} else {
			Main_hall->regions[2].description = XSTR( "Ready room - Start or continue a campaign", 355);
		}
	}
	
	// Read the menu regions from mainhall.tbl
	SCP_vector<main_hall_region>::iterator it;
	for (it = Main_hall->regions.begin(); Main_hall->regions.end() != it; ++it) {
		snazzy_menu_add_region(&Main_hall_region[it - Main_hall->regions.begin()], it->description.c_str(), it->mask, it->key, -1);
	}

	// init tooltip shader						// nearly black
	gr_create_shader(&Main_hall_tooltip_shader, 5, 5, 5, 168);

	// are we funny?
	if (Vasudan_funny && main_hall_is_vasudan()) {
		if (!stricmp(Main_hall->bitmap.c_str(), "vhall")) {
			Main_hall->door_sounds.at(OPTIONS_REGION).at(0) = SND_VASUDAN_BUP;
			Main_hall->door_sounds.at(OPTIONS_REGION).at(1) = SND_VASUDAN_BUP;
			
			// set head anim. hehe
			Main_hall->door_anim_name.at(OPTIONS_REGION) = "vhallheads";
			
			// set the background
			Main_hall->bitmap = "vhallhead";
		} else if (!stricmp(Main_hall->bitmap.c_str(), "2_vhall")) {
			Main_hall->door_sounds.at(OPTIONS_REGION).at(0) = SND_VASUDAN_BUP;
			Main_hall->door_sounds.at(OPTIONS_REGION).at(1) = SND_VASUDAN_BUP;
			
			// set head anim. hehe
			Main_hall->door_anim_name.at(OPTIONS_REGION) = "2_vhallheads";
			
			// set the background
			Main_hall->bitmap = "2_vhallhead";
		}
	}

	Main_hall_bitmap_w = -1;
	Main_hall_bitmap_h = -1;

	// load the background bitmap
	Main_hall_bitmap = bm_load(Main_hall->bitmap);
	if (Main_hall_bitmap < 0) {
		nprintf(("General","WARNING! Couldn't load main hall background bitmap %s\n", Main_hall->bitmap.c_str()));
	} else {
		bm_get_info(Main_hall_bitmap, &Main_hall_bitmap_w, &Main_hall_bitmap_h);
	}
	bg_type = bm_get_type(Main_hall_bitmap);

	Main_hall_mask_w = -1;
	Main_hall_mask_h = -1;

	// load the mask
	Main_hall_mask = bm_load(Main_hall->mask);
	if (Main_hall_mask < 0) {
		nprintf(("General","WARNING! Couldn't load main hall background mask %s\n", Main_hall->mask.c_str()));
		if (gr_screen.res == 0) {
			Error(LOCATION,"Could not load in main hall mask '%s'!\n\n(This error most likely means that you are missing required 640x480 interface art.)", Main_hall->mask.c_str());
		} else {
			Error(LOCATION,"Could not load in main hall mask '%s'!\n\n(This error most likely means that you are missing required 1024x768 interface art.)", Main_hall->mask.c_str());
		}
	} else {
		// get a pointer to bitmap by using bm_lock(), so we can feed it to he snazzy menu system
		Main_hall_mask_bitmap = bm_lock(Main_hall_mask, 8, BMP_AABITMAP);
		Main_hall_mask_data = (ubyte*)Main_hall_mask_bitmap->data;
		bm_get_info(Main_hall_mask, &Main_hall_mask_w, &Main_hall_mask_h);
	}

	// make sure the zoom area is completely within the background bitmap
	if (Main_hall->zoom_area_width > Main_hall_bitmap_w) {
		Main_hall->zoom_area_width = Main_hall_bitmap_w;
	}
	if (Main_hall->zoom_area_height > Main_hall_bitmap_h) {
		Main_hall->zoom_area_height = Main_hall_bitmap_h;
	}

	// get the default value for tooltip padding if necessary
	if (Main_hall->tooltip_padding == -1) {
		Main_hall->tooltip_padding = Main_hall_default_tooltip_padding[gr_get_resolution_class(Main_hall_bitmap_w, Main_hall_bitmap_h)];
	}

	// In case we're re-entering the mainhall
	Main_hall_misc_anim.clear();

	// load up the misc animations, and nullify all the delay timestamps for the misc animations
	for (idx = 0; idx < Main_hall->num_misc_animations; idx++) {
		generic_anim temp_anim;
		generic_anim_init(&temp_anim, Main_hall->misc_anim_name.at(idx));
		Main_hall_misc_anim.push_back(temp_anim);
		Main_hall_misc_anim.at(idx).ani.bg_type = bg_type;
		if (generic_anim_stream(&Main_hall_misc_anim.at(idx)) == -1) {
			nprintf(("General","WARNING!, Could not load misc %s anim in main hall\n",Main_hall->misc_anim_name.at(idx).c_str()));
		} else {
			// start paused
			if (Main_hall->misc_anim_modes.at(idx) == MISC_ANIM_MODE_HOLD)
				Main_hall_misc_anim.at(idx).direction |= GENERIC_ANIM_DIRECTION_NOLOOP;
		}

		// null out the delay timestamps
		Main_hall->misc_anim_delay.at(idx).at(0) = -1;

		// start paused
		Main_hall->misc_anim_paused.at(idx) = true;
	}

	// In case we're re-entering the mainhall
	Main_hall_door_anim.clear();

	// load up the door animations
	for (idx = 0; idx < Main_hall->num_door_animations; idx++) {
		generic_anim temp_anim;
		generic_anim_init(&temp_anim, Main_hall->door_anim_name.at(idx));
		Main_hall_door_anim.push_back(temp_anim);
		Main_hall_door_anim.at(idx).ani.bg_type = bg_type;
		if (generic_anim_stream(&Main_hall_door_anim.at(idx)) == -1) {
			nprintf(("General","WARNING!, Could not load door anim %s in main hall\n",Main_hall->door_anim_name.at(idx).c_str()));
		} else {
			Main_hall_door_anim.at(idx).direction = GENERIC_ANIM_DIRECTION_BACKWARDS | GENERIC_ANIM_DIRECTION_NOLOOP;
		}
	}

	// load in help overlay bitmap
	if (!Main_hall->help_overlay_name.empty()) {
		Main_hall_overlay_id = help_overlay_get_index(Main_hall->help_overlay_name.c_str());
	} else if (main_hall_id() == 0) {
		Main_hall_overlay_id = help_overlay_get_index(MH_OVERLAY);
	} else {
		Main_hall_overlay_id = help_overlay_get_index(MH2_OVERLAY);
	}
	help_overlay_set_state(Main_hall_overlay_id,gr_screen.res,0);

	// check to see if the "very first pilot" flag is set, and load the overlay if so
	if (!F1_text_done) {
		if (Main_hall_f1_text_frame == 0) {
			Main_hall_help_stamp = timestamp(MAIN_HALL_HELP_TIME);
		} else {
			F1_text_done = 1;
		}
	}

	Main_hall_region_linger_stamp = -1;

	// initialize door sound handles
	Main_hall_door_sound_handles.clear();
	for (idx = 0; idx < Main_hall->num_door_animations; idx++) {
		Main_hall_door_sound_handles.push_back(-1);
	}

	// skip the first frame
	Main_hall_frame_skip = 1;

	// initialize the music
	main_hall_start_music();

	// initialize the main hall notify text
	Main_hall_notify_stamp = 1;

	// initialize the random intercom sound stuff
	Main_hall_next_intercom_sound = 0;
	Main_hall_next_intercom_sound_stamp = -1;
	Main_hall_intercom_sound_handle = -1;

	// set the placement of the mouse cursor (start at the ready room)
	Main_hall_mouse_region = -1;
	Main_hall_last_clicked_region = READY_ROOM_REGION;

	Main_hall_inited = 1;

	// determine if we have a right click
	Main_hall_right_click = mouse_down(MOUSE_RIGHT_BUTTON);
}

/**
 * Exit the game
 *
 * @note In Debug mode, the confirmation popup does not get used, and the game usefully exits immediately
 */
void main_hall_exit_game()
{
#if defined(NDEBUG)
	int choice;

	// stop music first
	main_hall_stop_music(true);
	main_hall_stop_ambient();
	choice = popup( PF_NO_NETWORKING | PF_BODY_BIG, 2, POPUP_NO, POPUP_YES, XSTR( "Exit Game?", 365));
	if (choice == 1) {
		gameseq_post_event(GS_EVENT_QUIT_GAME);
	} else {
		main_hall_start_music();
		main_hall_start_ambient();
	}
#else
	gameseq_post_event(GS_EVENT_QUIT_GAME);
#endif
}

/**
 * Do a frame for the main hall
 *
 * @param frametime Animation frame time
 */
void main_hall_do(float frametime)
{
	int code, key, snazzy_action, region_action = -1;
	SCP_vector<main_hall_region>::iterator it;

	// set the screen scale to the main hall's dimensions
	gr_set_screen_scale(Main_hall_bitmap_w, Main_hall_bitmap_h, Main_hall->zoom_area_width, Main_hall->zoom_area_height);

	// need to ensure ambient is playing, since it may be stopped by a playing movie
	main_hall_start_ambient();

	// handle any random intercom sound details
	main_hall_handle_random_intercom_sounds();

	// handle any mouse clicks
	main_hall_handle_right_clicks();

	// handle any sound details
	main_hall_cull_door_sounds();

	// do any campaign load failure handling
	mission_campaign_load_failure_popup();

	// process any keypresses/mouse events
	snazzy_action = -1;
	code = snazzy_menu_do(Main_hall_mask_data, Main_hall_mask_w, Main_hall_mask_h, (int)Main_hall->regions.size(), Main_hall_region, &snazzy_action, 1, &key);

	if (key) {
		game_process_cheats(key);

		Main_hall_cheat += (char) key_to_ascii(key);
		if(Main_hall_cheat.size() > MAIN_HALL_MAX_CHEAT_LEN) {
			Main_hall_cheat = Main_hall_cheat.substr(Main_hall_cheat.size() - MAIN_HALL_MAX_CHEAT_LEN);
		}

		int cur_frame;
		float anim_time;
		bool cheat_anim_found, cheat_found = false;

		for (int c_idx = 0; c_idx < (int) Main_hall->cheat.size(); c_idx++) {
			cheat_anim_found = false;

			if(Main_hall_cheat.find(Main_hall->cheat.at(c_idx)) != SCP_string::npos) {
				cheat_found = true;
				// switch animations

				for (int idx = 0; idx < Main_hall->num_misc_animations; idx++) {
					if (Main_hall->misc_anim_name.at(idx) == Main_hall->cheat_anim_from.at(c_idx)) {
						Main_hall->misc_anim_name.at(idx) = Main_hall->cheat_anim_to.at(c_idx);

						cur_frame = Main_hall_misc_anim.at(idx).current_frame;
						anim_time = Main_hall_misc_anim.at(idx).anim_time;

						generic_anim_unload(&Main_hall_misc_anim.at(idx));
						generic_anim_init(&Main_hall_misc_anim.at(idx), Main_hall->misc_anim_name.at(idx));

						if (generic_anim_stream(&Main_hall_misc_anim.at(idx)) == -1) {
							nprintf(("General","WARNING! Could not load misc %s anim in main hall\n", Main_hall->misc_anim_name.at(idx).c_str()));
						} else {
							// start paused
							if (Main_hall->misc_anim_modes.at(idx) == MISC_ANIM_MODE_HOLD)
								Main_hall_misc_anim.at(idx).direction |= GENERIC_ANIM_DIRECTION_NOLOOP;
						}

						Main_hall_misc_anim.at(idx).current_frame = cur_frame;
						Main_hall_misc_anim.at(idx).anim_time = anim_time;

						// null out the delay timestamps
						Main_hall->misc_anim_delay.at(idx).at(0) = -1;

						cheat_anim_found = true;
						break;
					}
				}

				if (!cheat_anim_found) {
					for (int idx = 0; idx < Main_hall->num_door_animations; idx++) {
						if (Main_hall->door_anim_name.at(idx) == Main_hall->cheat_anim_from.at(c_idx)) {
							Main_hall->door_anim_name.at(idx) = Main_hall->cheat_anim_to.at(c_idx);

							cur_frame = Main_hall_door_anim.at(idx).current_frame;
							anim_time = Main_hall_door_anim.at(idx).anim_time;

							generic_anim_unload(&Main_hall_door_anim.at(idx));
							generic_anim_init(&Main_hall_door_anim.at(idx), Main_hall->door_anim_name.at(idx));

							if (generic_anim_stream(&Main_hall_door_anim.at(idx)) == -1) {
								nprintf(("General","WARNING! Could not load door anim %s in main hall\n", Main_hall->door_anim_name.at(idx).c_str()));
							} else {
								Main_hall_door_anim.at(idx).direction = GENERIC_ANIM_DIRECTION_BACKWARDS | GENERIC_ANIM_DIRECTION_NOLOOP;
							}

							Main_hall_door_anim.at(idx).current_frame = cur_frame;
							Main_hall_door_anim.at(idx).anim_time = anim_time;

							cheat_anim_found = true;
							break;
						}
					}
				}

				if (!cheat_anim_found) {
					// Note: This can also happen if the cheat triggers a second time since the animations are already switched at that point.
					nprintf(("General", "Could not find animation '%s' for cheat '%s'!", Main_hall->cheat_anim_from.at(c_idx).c_str(), Main_hall->cheat.at(c_idx).c_str()));
				}
			}
		}

		if(cheat_found) {
			// Found a cheat, clear the buffer.

			Main_hall_cheat = "";
		}
	}

	switch(key) {
		case KEY_ENTER:
			snazzy_action = SNAZZY_CLICKED;
			break;
		case KEY_F3:
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			gameseq_post_event(GS_EVENT_LAB);
			break;
	#ifndef NDEBUG
		case KEY_1:
			// no soup for you!
			movie_play("endprt2b.mve");
			break;
		case KEY_2:
			// no soup for you!
			movie_play_two("endprt2a.mve", "endprt2b.mve");
			break;
		case KEY_3:
			main_hall_campaign_cheat();
			break;
	#endif
	}

	// do any processing based upon what happened to the snazzy menu
	switch (snazzy_action) {
		case SNAZZY_OVER:
			for (it = Main_hall->regions.begin(); Main_hall->regions.end() != it; ++it) {
				if (it->mask == code) {
					main_hall_handle_mouse_location(it - Main_hall->regions.begin());
					break;
				}
			}
			
			break;

		case SNAZZY_CLICKED:
			if (code == ESC_PRESSED) {
				region_action = ESC_PRESSED;
			} else {
				if (code == -1) {
					// User didn't click on a valid button, just ignore the event
					break;
				}

				for (it = Main_hall->regions.begin(); Main_hall->regions.end() != it; ++it) {
					if (it->mask == code) {
						region_action = it->action;
						break;
					}
				}
				
				if (region_action == -1) {
					Error(LOCATION, "Region %d doesn't have an action!", code);
				} else if (region_action == START_REGION) {
					if (Player->flags & PLAYER_FLAGS_IS_MULTI) {
						region_action = MULTIPLAYER_REGION;
					} else {
						region_action = READY_ROOM_REGION;
					}
				}
			}
			
			switch (region_action) {
				// clicked on the exit region
				case EXIT_REGION:
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					main_hall_exit_game();
					break;

				// clicked on the readyroom region
				case READY_ROOM_REGION:
					// Make sure we aren't in multi mode.
					Player->flags &= ~PLAYER_FLAGS_IS_MULTI;
					Game_mode = GM_NORMAL;
					
					gameseq_post_event(GS_EVENT_NEW_CAMPAIGN);
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					break;

				// clicked on the tech room region
				case TECH_ROOM_REGION:
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					gameseq_post_event(GS_EVENT_TECH_MENU);
					break;

				// clicked on the options region
				case OPTIONS_REGION:
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					gameseq_post_event(GS_EVENT_OPTIONS_MENU);
					break;

				// clicked on the campaign toom region
				case CAMPAIGN_ROOM_REGION:
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					gameseq_post_event(GS_EVENT_CAMPAIGN_ROOM);
					break;

				// clicked on the multiplayer region
				case MULTIPLAYER_REGION:
					// Make sure we are in multi mode.
					Player->flags |= PLAYER_FLAGS_IS_MULTI;
					Game_mode = GM_MULTIPLAYER;
					
					main_hall_do_multi_ready();
					
					// NOTE : this isn't a great thing to be calling this anymore. But we'll leave it for now
					gameseq_post_event(GS_EVENT_MULTI_JOIN_GAME);
					break;

				// load mission key was pressed
				case LOAD_MISSION_REGION:
					break;

				// quick start a game region
				case QUICK_START_REGION:
			#if !defined(NDEBUG)
					if (Num_recent_missions > 0) {
						strcpy_s(Game_current_mission_filename, Recent_missions[0]);
					} else {
						if (mission_load_up_campaign()) {
							main_hall_set_notify_string(XSTR( "Campaign file is currently unavailable", 1606));
						}
						strcpy_s(Game_current_mission_filename, Campaign.missions[0].name);
					}
					Campaign.current_mission = -1;
					gameseq_post_event(GS_EVENT_START_GAME_QUICK);
			#endif
					break;

				// clicked on the barracks region
				case BARRACKS_REGION:
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					gameseq_post_event(GS_EVENT_BARRACKS_MENU);
					break;

				// increate the skill level
				case SKILL_LEVEL_REGION:
					char temp[100];
					game_increase_skill_level();
					sprintf(temp, XSTR( "Skill level set to %s.", 370), Skill_level_names(Game_skill_level));
					main_hall_set_notify_string(temp);
					break;

				// escape was pressed
				case ESC_PRESSED:
					// if there is a help overlay active, then don't quit the game - just kill the overlay
					if (!help_overlay_active(Main_hall_overlay_id)) {
						gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
						main_hall_exit_game();
					} else { // kill the overlay
						help_overlay_set_state(Main_hall_overlay_id,gr_screen.res,0);
					}
					break;
				
				// custom action
				case SCRIPT_REGION:
					const char *lua = it->lua_action.c_str();
					bool success = Script_system.EvalString(lua, NULL, NULL, lua);
					if(!success)
						Warning(LOCATION, "mainhall '+Door Action / $Script' failed to evaluate \"%s\"; check your syntax", lua);
					break;
			} // END switch (code)

			// if the escape key wasn't pressed handle any mouse position related events
			if (code != ESC_PRESSED) {
				main_hall_handle_mouse_location((region_action == -1 ? -1 : it - Main_hall->regions.begin()));
			}
			break;

			default:
				main_hall_handle_mouse_location(-1);
				break;
	} // END switch (snazzy_action)

	if (mouse_down(MOUSE_LEFT_BUTTON)) {
		help_overlay_set_state(Main_hall_overlay_id, main_hall_get_overlay_resolution_index(), 0);
	}

	// draw the background bitmap
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Main_hall_bitmap);
	if (Main_hall_bitmap >= 0) {
		gr_set_bitmap(Main_hall_bitmap);
		gr_bitmap(0, 0, GR_RESIZE_MENU);
	}

	// render misc animations
	main_hall_render_misc_anims(frametime, false);

	// render door animtions
	main_hall_render_door_anims(frametime);

	// render misc animations (over doors)
	main_hall_render_misc_anims(frametime, true);

	// blit any appropriate tooltips
	main_hall_maybe_blit_tooltips();

	// fishtank
	fishtank_process();

	// draw any pending notification messages
	main_hall_notify_do();

	// process any help "hit f1" timestamps and display any messages if necessary
	if (!F1_text_done) {
		main_hall_process_help_stuff();
	}

	// blit help overlay if active
	help_overlay_maybe_blit(Main_hall_overlay_id, main_hall_get_overlay_resolution_index());

	// blit the freespace version #
	main_hall_blit_version();

	// blit ship and weapon table status
#ifndef NDEBUG
	main_hall_blit_table_status();
#endif

	gr_flip();
	gr_reset_screen_scale();

	// see if we have a missing campaign and force the player to select a new campaign if so
	extern bool Campaign_room_no_campaigns;
	if ( !(Player->flags & PLAYER_FLAGS_IS_MULTI) && Campaign_file_missing && !Campaign_room_no_campaigns ) {
		int rc = popup(0, 3, XSTR("Go to Campaign Room", 1607), XSTR("Select another pilot", 1608), XSTR("Exit Game", 1609), XSTR("The currently active campaign cannot be found.  Please select another...", 1600));

		switch (rc) {
			case 0:
				gameseq_post_event(GS_EVENT_CAMPAIGN_ROOM);
				break;

			case 1:
				gameseq_post_event(GS_EVENT_INITIAL_PLAYER_SELECT);
				break;

			case 2:
				main_hall_exit_game();
				break;

			default:
				gameseq_post_event(GS_EVENT_CAMPAIGN_ROOM);
				break;
		}
	}

	// maybe run the player tips popup
	player_tips_popup();

	// if we were supposed to skip a frame, then stop doing it after 1 frame
	if (Main_hall_frame_skip) {
		Main_hall_frame_skip = 0;
	}
}

/**
 * Close the main hall proper
 */
void main_hall_close()
{
	int idx, s_idx;

	if (!Main_hall_inited) {
		return;
	}

	// unload the main hall bitmap
	if (Main_hall_bitmap != -1) {
		bm_release(Main_hall_bitmap);
	}

	// unload any bitmaps
	if (Main_hall_mask >= 0) {
		// make sure we unlock the mask bitmap so it can be unloaded
		bm_unlock(Main_hall_mask);
		bm_release(Main_hall_mask);
	}

	// free up any (possibly) playing misc animation handles
	for (idx = 0; idx < Main_hall->num_misc_animations; idx++) {
		if (Main_hall_misc_anim.at(idx).num_frames > 0) {
			generic_anim_unload(&Main_hall_misc_anim.at(idx));
		}
	}

	// free up any (possibly) playing door animation handles
	for (idx = 0; idx < Main_hall->num_door_animations; idx++) {
		if (Main_hall_door_anim.at(idx).num_frames > 0) {
			generic_anim_unload(&Main_hall_door_anim.at(idx));
		}
	}

	// stop any playing door sounds
	for (idx = 0; idx < Main_hall->num_door_animations-2; idx++) {
		if ( (Main_hall_door_sound_handles.at(idx) != -1) && snd_is_playing(Main_hall_door_sound_handles.at(idx)) ) {
			snd_stop(Main_hall_door_sound_handles.at(idx));
			Main_hall_door_sound_handles.at(idx) = -1;
		}
	}

	// stop any playing misc animation sounds
	for (idx = 0; idx < Main_hall->num_misc_animations; idx++) {
		// if this goes wrong, the int cast could overflow
		Assert(Main_hall->misc_anim_special_sounds.at(idx).size() < INT_MAX);

		for (s_idx = 0; s_idx < (int)Main_hall->misc_anim_special_sounds.at(idx).size(); s_idx++) {
			if (snd_is_playing(Main_hall->misc_anim_special_sounds.at(idx).at(s_idx))) {
				snd_stop(Main_hall->misc_anim_special_sounds.at(idx).at(s_idx));
			}
		}
	}

	// close any snazzy menu details
	snazzy_menu_close();

	// no fish
	fishtank_stop();

	// unpause
	Main_hall_paused = 0;

	// not inited anymore
	Main_hall_inited = 0;
}

/**
 * Return music index
 *
 * @param main_hall_num Index to seek
 * @return Index into spooled music
 */
int main_hall_get_music_index(int main_hall_num)
{
	main_hall_defines *hall;
	int index;

	if (main_hall_num < 0) {
		return -1;
	}

	hall = &Main_hall_defines.at(main_hall_num).at(main_hall_get_resolution_index(main_hall_num));

	// Goober5000 - try substitute first
	index = event_music_get_spooled_music_index(hall->substitute_music_name);
	if ( (index >= 0) && (Spooled_music[index].flags & SMF_VALID) ) {
		return index;
	}

	// now try regular
	index = event_music_get_spooled_music_index(hall->music_name);
	if ( (index >= 0) && (Spooled_music[index].flags & SMF_VALID) ) {
		return index;
	}

	return -1;
}

/**
 * Start the main hall music playing
 */
void main_hall_start_music()
{
	char *filename;

	// start a looping ambient sound
	main_hall_start_ambient();

	// if we have selected no music, then don't do this
	if (Cmdline_freespace_no_music) {
		return;
	}

	// already playing?
	if (Main_hall_music_handle >= 0) {
		return;
	}

	// get music
	Main_hall_music_index = main_hall_get_music_index(main_hall_id());
	if (Main_hall_music_index < 0) {
		nprintf(("Warning", "No music file exists to play music at the main menu!\n"));
		return;
	}

	filename = Spooled_music[Main_hall_music_index].filename;
	Assert(filename != NULL);

	// get handle
	Main_hall_music_handle = audiostream_open(filename, ASF_MENUMUSIC);
	if (Main_hall_music_handle < 0) {
		nprintf(("Warning", "No music file exists to play music at the main menu!\n"));
		return;
	}

	audiostream_play(Main_hall_music_handle, Master_event_music_volume, 1);
}

/**
 * Stop the main hall music
 */
void main_hall_stop_music(bool fade)
{
	if (Main_hall_music_handle != -1) {
		audiostream_close_file(Main_hall_music_handle, fade);
		Main_hall_music_handle = -1;
	}
}

/**
 * Render all playing misc animations
 * 
 * @param frametime Animation frame time
 */
void main_hall_render_misc_anims(float frametime, bool over_doors)
{
	std::deque<bool> group_anims_weve_checked;
	int idx, s_idx, jdx;

	// render all misc animations
	for (idx = 0; idx < Main_hall->num_misc_animations; idx++) {
		// give it a spot in the vector
		group_anims_weve_checked.push_back(false);

		// render it
		if (Main_hall_misc_anim.at(idx).num_frames > 0 && Main_hall->misc_anim_over_doors.at(idx) == over_doors) {
			// animation is paused
			if (Main_hall->misc_anim_paused.at(idx)) {
				// if the timestamp is -1, then regenerate it
				if (Main_hall->misc_anim_delay.at(idx).at(0) == -1) {
					int regen_idx = -1;

					// if this is part of a group, we should do additional checking
					if (Main_hall->misc_anim_group.at(idx) >= 0) {

						// make sure we haven't already checked it
						if (group_anims_weve_checked.at(idx) == false) {
							SCP_vector<int> group_indexes; //stores indexes of which anims are part of a group
							bool all_neg1 = true;

							// okay... now we need to make sure all anims in this group are paused and -1
							for (jdx = 0; jdx < Main_hall->num_misc_animations; jdx++) {
								if (Main_hall->misc_anim_group.at(jdx) == Main_hall->misc_anim_group.at(idx)) {
									Assert(group_anims_weve_checked.size() < INT_MAX);
									if ((int)group_anims_weve_checked.size() <= jdx) {
										group_anims_weve_checked.push_back(true);
									} else {
										group_anims_weve_checked.at(jdx) = true;
									}

									group_indexes.push_back(jdx);

									if (!Main_hall->misc_anim_paused.at(jdx) || Main_hall->misc_anim_delay.at(jdx).at(0) != -1) {
										all_neg1 = false;
									}
								}
							}

							// if the entire group is paused and off, pick a random one to regenerate
							if (all_neg1) {
								Assert(group_indexes.size() < INT_MAX);
								regen_idx = group_indexes[rand() % (int)group_indexes.size()];
							}
						}
					} else { // not part of a group, so just handle this index
						regen_idx = idx;
					}

					// reset it to some random value (based on MIN and MAX) and continue
					if (regen_idx >= 0) {
						int min = Main_hall->misc_anim_delay.at(regen_idx).at(1);
						int max = Main_hall->misc_anim_delay.at(regen_idx).at(2);
						Main_hall->misc_anim_delay.at(regen_idx).at(0) = timestamp(min + (int) (frand() * (max - min)));
					}

				// if the timestamp is not -1 and has popped, play the anim and make the timestamp -1
				} else if (timestamp_elapsed(Main_hall->misc_anim_delay.at(idx).at(0))) {
					Main_hall->misc_anim_paused.at(idx) = false;
					Main_hall_misc_anim.at(idx).current_frame = 0;
					Main_hall_misc_anim.at(idx).anim_time = 0.0;

					// kill the timestamp
					Main_hall->misc_anim_delay.at(idx).at(0) = -1;

					// reset the "should be playing" flags
					Assert(Main_hall->misc_anim_sound_flag.at(idx).size() < INT_MAX);
					for (s_idx = 0; s_idx < (int)Main_hall->misc_anim_sound_flag.at(idx).size(); s_idx++) {
						Main_hall->misc_anim_sound_flag.at(idx).at(s_idx) = 0;
					}
				}
			} else { // animation is not paused
				Assert(Main_hall->misc_anim_special_sounds.at(idx).size() < INT_MAX);
				for (s_idx = 0; s_idx < (int)Main_hall->misc_anim_special_sounds.at(idx).size(); s_idx++) {
					// if we've passed the trigger point, then play the sound and break out of the loop
					if ( (Main_hall_misc_anim.at(idx).current_frame >= Main_hall->misc_anim_special_trigger.at(idx).at(s_idx)) 
							&& !Main_hall->misc_anim_sound_flag.at(idx).at(s_idx) ) {
						Main_hall->misc_anim_sound_flag.at(idx).at(s_idx) = 1;

						// if the sound is already playing, then kill it.
						// This is a pretty safe thing to do since we can assume that by the time we get to this point again,
						// the sound will have been long finished
						if (snd_is_playing(Main_hall->misc_anim_special_sounds.at(idx).at(s_idx))) {
							snd_stop(Main_hall->misc_anim_special_sounds.at(idx).at(s_idx));
						}

						int sound = Main_hall->misc_anim_special_sounds.at(idx).at(s_idx);

						// Check if the sound is valid
						if (sound >= 0)
						{
							// play the sound
							snd_play(&Snds_iface[sound],Main_hall->misc_anim_sound_pan.at(idx));
						}
						break;
					}
				}

				// animation has reached the last frame
				if (Main_hall_misc_anim.at(idx).current_frame == Main_hall_misc_anim.at(idx).num_frames - 1) {
					Main_hall->misc_anim_delay.at(idx).at(0) = -1;

					// this helps the above code reset the timers
					// MISC_ANIM_MODE_HOLD simply stops on the last frame, so we don't care
					// MISC_ANIM_MODE_LOOPED just loops so we don't care either
					if (Main_hall->misc_anim_modes.at(idx) == MISC_ANIM_MODE_TIMED) {
						Main_hall->misc_anim_paused.at(idx) = true;
					}
					// don't reset sound for MISC_ANIM_MODE_HOLD
					if (Main_hall->misc_anim_modes.at(idx) != MISC_ANIM_MODE_HOLD) {
						// reset the "should be playing" flags
						Assert(Main_hall->misc_anim_sound_flag.at(idx).size() < INT_MAX);
						for (s_idx = 0; s_idx < (int)Main_hall->misc_anim_sound_flag.at(idx).size(); s_idx++) {
							Main_hall->misc_anim_sound_flag.at(idx).at(s_idx) = 0;
						}
					}
				}

				// actually render it
				if (Main_hall_frame_skip || Main_hall_paused) {
					frametime = 0;
				}
				generic_anim_render(&Main_hall_misc_anim.at(idx), frametime, Main_hall->misc_anim_coords.at(idx).at(0), Main_hall->misc_anim_coords.at(idx).at(1), true);
			}
		}
	}
}

/**
 * Render all playing door animations
 * @param frametime Animation frame time
 */
void main_hall_render_door_anims(float frametime)
{
	int idx;

	// render all door animations
	Assert(Main_hall_door_anim.size() < INT_MAX);
	for (idx = 0; idx < (int)Main_hall_door_anim.size(); idx++) {
		if (Main_hall_door_anim.at(idx).num_frames > 0) {
		// first pair : coords of where to play a given door anim
		// second pair : center of a given door anim in windowed mode
			generic_anim_render(&Main_hall_door_anim.at(idx), frametime, Main_hall->door_anim_coords.at(idx).at(0), Main_hall->door_anim_coords.at(idx).at(1), true);
		}
	}
}

/**
 * Any necessary processing based upon the mouse location
 * @param cur_region Region of current mouse location
 */
void main_hall_handle_mouse_location(int cur_region)
{
	if (Main_hall_frame_skip) {
		return;
	}
	
	if (cur_region >= (int) Main_hall->regions.size()) {
		// MWA -- inserted return since Int3() was tripped when hitting L from main menu.
		return;
	}

	// if the mouse is now over a resgion
	if (cur_region != -1) {
		// if we're still over the same region we were last frame, check stuff
		if (cur_region == Main_hall_mouse_region) {
			// if we have a linger timestamp set and it has expired, then get moving
			if ((Main_hall_region_linger_stamp != -1) && timestamp_elapsed(Main_hall_region_linger_stamp)) {
				main_hall_mouse_grab_region(cur_region);

				// release the region linger stamp
				Main_hall_region_linger_stamp = -1;
			}
		} else {
			// if we're currently on another region, release it
			if ( (Main_hall_mouse_region != -1) && (cur_region != Main_hall_mouse_region) ) {
				main_hall_mouse_release_region(Main_hall_mouse_region);
			}
		
			// set the linger time
			if (Main_hall_region_linger_stamp == -1) {
				Main_hall_mouse_region = cur_region;
				Main_hall_region_linger_stamp = timestamp(MAIN_HALL_REGION_LINGER);
			}
		}
	} else { // if it was over a region but isn't anymore, release that region
		if (Main_hall_mouse_region != -1) {
			main_hall_mouse_release_region(Main_hall_mouse_region);
			Main_hall_mouse_region = -1;

			// release the region linger timestamp
			Main_hall_region_linger_stamp = -1;
		}
	}
}

/**
 * If the mouse has moved off of the currently active region, handle the anim accordingly
 * @param region Region of prior mouse location
 */
void main_hall_mouse_release_region(int region)
{
	if (Main_hall_frame_skip) {
		return;
	}
	// don't do anything if there are no animations to play
	else if (region >= (int) Main_hall_door_anim.size()) {
		return;
	}

	// run backwards and stop at the first frame
	Main_hall_door_anim.at(region).direction = GENERIC_ANIM_DIRECTION_BACKWARDS | GENERIC_ANIM_DIRECTION_NOLOOP;

	// check for door sounds, ignoring the OPTIONS_REGION (which isn't a door)
	if (Main_hall_door_anim.at(region).num_frames > 0) {
		// don't stop the toaster oven or microwave regions from playing all the way through
		if (Main_hall_door_sound_handles.at(region) != -1) {
			snd_stop(Main_hall_door_sound_handles.at(region));
		}

		int sound = Main_hall->door_sounds.at(region).at(1);

		if (sound >= 0)
		{
			Main_hall_door_sound_handles.at(region) = snd_play(&Snds_iface[sound], Main_hall->door_sound_pan.at(region));
		}

		// TODO: track current frame
		snd_set_pos(Main_hall_door_sound_handles.at(region), &Snds_iface[SND_MAIN_HALL_DOOR_CLOSE], 
			(float)((Main_hall_door_anim.at(region).keyframe) ? Main_hall_door_anim.at(region).keyframe : 
				Main_hall_door_anim.at(region).num_frames - Main_hall_door_anim.at(region).current_frame) / 
					(float)Main_hall_door_anim.at(region).num_frames, 1);
	}
}

/**
 * If the mouse has moved on this region, handle it accordingly
 * @param region Region of current mouse location
 */
void main_hall_mouse_grab_region(int region)
{
	if (Main_hall_frame_skip) {
		return;
	}
	// don't do anything if there are no animations to play
	else if (region >= (int) Main_hall_door_anim.size()) {
		return;
	}

	// run forwards
	Main_hall_door_anim.at(region).direction = GENERIC_ANIM_DIRECTION_FORWARDS;
	// stay on last frame if we have no keyframe
	if (!Main_hall_door_anim.at(region).keyframe) {
		Main_hall_door_anim.at(region).direction += GENERIC_ANIM_DIRECTION_NOLOOP;
	}

	// check for opening/starting sounds
	// kill the currently playing sounds if necessary
	if (Main_hall_door_sound_handles.at(region) != -1) {
		snd_stop(Main_hall_door_sound_handles.at(region));
	}


	int sound = Main_hall->door_sounds.at(region).at(0);

	if (sound >= 0)
	{
		Main_hall_door_sound_handles.at(region) = snd_play(&Snds_iface[sound],Main_hall->door_sound_pan.at(region));
	}

	// start the sound playing at the right spot relative to the completion of the animation
	if ( (Main_hall_door_anim.at(region).num_frames > 0) && (Main_hall_door_anim.at(region).current_frame != -1) ) {
		snd_set_pos(Main_hall_door_sound_handles.at(region),&Snds_iface[SND_MAIN_HALL_DOOR_OPEN], 
			(float)Main_hall_door_anim.at(region).current_frame / (float)Main_hall_door_anim.at(region).num_frames,1);
	}
}

/**
 * Handle any right clicks which may have occured
 */
void main_hall_handle_right_clicks()
{
	int new_region;

	if (Main_hall_frame_skip) {
		return;
	}

	// check to see if the button has been clicked
	if (!Main_hall_right_click) {
		if (mouse_down(MOUSE_RIGHT_BUTTON)) {
			// cycle through the available regions
			if (Main_hall_last_clicked_region == (int) Main_hall_door_anim.size() - 1) {
				new_region = 0;
			} else {
				new_region = Main_hall_last_clicked_region + 1;
			}

			// set the position of the mouse cursor and the newly clicked region
			int mx = Main_hall->door_anim_coords.at(new_region).at(2);
			int my = Main_hall->door_anim_coords.at(new_region).at(3);
			gr_resize_screen_pos( &mx, &my, NULL, NULL, GR_RESIZE_MENU );

			if (mx < 0) {
				mx = 0;
			}
			if (mx >= gr_screen.max_w) {
				mx = gr_screen.max_w - 1;
			}
			if (my < 0) {
				my = 0;
			}
			if (my >= gr_screen.max_h) {
				my = gr_screen.max_h - 1;
			}

			mouse_set_pos( mx, my );

			main_hall_handle_mouse_location(new_region);
			Main_hall_last_clicked_region = new_region;

			// set the mouse as being clicked
			Main_hall_right_click = 1;
		}
	// set the mouse as being unclicked
	} else if (Main_hall_right_click && !(mouse_down(MOUSE_RIGHT_BUTTON))) {
		Main_hall_right_click = 0;
	}
}

/**
 * Cull any door sounds that have finished playing
 */
void main_hall_cull_door_sounds()
{
	int idx;
	// basically just set the handle of any finished sound to be -1, so that we know its free any where else in the code we may need it
	Assert(Main_hall_door_sound_handles.size() < INT_MAX);
	for (idx = 0; idx < (int)Main_hall_door_sound_handles.size(); idx++) {
		if ( (Main_hall_door_sound_handles.at(idx) != -1) && !snd_is_playing(Main_hall_door_sound_handles.at(idx)) ) {
			Main_hall_door_sound_handles.at(idx) = -1;
		}
	}
}

/**
 * Insert random intercom sounds
 */
void main_hall_handle_random_intercom_sounds()
{
	if (Main_hall->num_random_intercom_sounds <= 0)
	{
		// If there are no intercom sounds then just skip this section
		return;
	}

	// if we have no timestamp for the next random sound, then set on
	if ( (Main_hall_next_intercom_sound_stamp == -1) && (Main_hall_intercom_sound_handle == -1) ) {
		Main_hall_next_intercom_sound_stamp = timestamp((int)((rand() * RAND_MAX_1f) * 
			(float)(Main_hall->intercom_delay.at(Main_hall_next_intercom_sound).at(1) 
				- Main_hall->intercom_delay.at(Main_hall_next_intercom_sound).at(0))) );
	}

	// if the there is no sound playing
	if (Main_hall_intercom_sound_handle == -1) {
		if (Main_hall_paused) {
			return;
		}

		// if the timestamp has popped, play a sound
		if ( (Main_hall_next_intercom_sound_stamp != -1) && (timestamp_elapsed(Main_hall_next_intercom_sound_stamp)) ) {
			int sound = Main_hall->intercom_sounds.at(Main_hall_next_intercom_sound);

			// Check if the sound is valid
			if (sound >= 0)
			{
				// play the sound
				Main_hall_intercom_sound_handle = snd_play(&Snds_iface.at(sound));

				// unset the timestamp
				Main_hall_next_intercom_sound_stamp = -1;
			}	

			// unset the timestamp
			Main_hall_next_intercom_sound_stamp = -1;
		}
	} else { // if the sound is playing
		// if the sound has finished, set the timestamp and continue
		if (!snd_is_playing(Main_hall_intercom_sound_handle)) {
			// increment the next sound
			if (Main_hall_next_intercom_sound >= (Main_hall->num_random_intercom_sounds-1)) {
				Main_hall_next_intercom_sound = 0;
			} else {
				Main_hall_next_intercom_sound++;
			}

			// set the timestamp
			Main_hall_next_intercom_sound_stamp = timestamp((int)((rand() * RAND_MAX_1f) * 
				(float)(Main_hall->intercom_delay.at(Main_hall_next_intercom_sound).at(1) 
					- Main_hall->intercom_delay.at(Main_hall_next_intercom_sound).at(0))) );

			// release the sound handle
			Main_hall_intercom_sound_handle = -1;
		}
	}
}

/**
 * Set the notification string with its decay timeout
 * @param str Notification string
 */
void main_hall_set_notify_string(const char *str)
{
	strcpy_s(Main_hall_notify_text,str);
	Main_hall_notify_stamp = timestamp(MAIN_HALL_NOTIFY_TIME);
}

/**
 * Handle any drawing, culling, etc of notification messages
 */
void main_hall_notify_do()
{
	// check to see if we should try and do something
	if (Main_hall_notify_stamp != -1) {
		// if the text time has expired
		if (timestamp_elapsed(Main_hall_notify_stamp)) {
			strcpy_s(Main_hall_notify_text,"");
			Main_hall_notify_stamp = -1;
		} else {
			int w,h;

			int old_font = gr_get_current_fontnum();

			gr_set_color_fast(&Color_bright);
			gr_set_font(Main_hall->font);

			gr_get_string_size(&w,&h,Main_hall_notify_text);
			gr_printf_menu_zoomed((gr_screen.max_w_unscaled_zoomed - w)/2, gr_screen.max_h_unscaled_zoomed - (h * 4 + 4), Main_hall_notify_text);

			gr_set_font(old_font);
		}
	}
}

/**
 * Start a looping ambient sound for main hall
 */
void main_hall_start_ambient()
{
	int play_ambient_loop = 0;

	if (Main_hall_paused) {
		return;
	}

	if (Main_hall_ambient_loop == -1) {
		play_ambient_loop = 1;
	} else {
		if (!snd_is_playing(Main_hall_ambient_loop)) {
			play_ambient_loop = 1;
		}
	}

	if (play_ambient_loop) {
		Main_hall_ambient_loop = snd_play_looping(&Snds_iface[SND_MAIN_HALL_AMBIENT]);
	}
}

/**
 * Stop a looping ambient sound for the main hall
 */
void main_hall_stop_ambient()
{
	if (Main_hall_ambient_loop != -1) {
		snd_stop(Main_hall_ambient_loop);
		Main_hall_ambient_loop = -1;
	}
}

/**
 * Reset the volume of the looping ambient sound.
 *
 * @note This is called from the options screen when the looping ambient sound might be playing.
 */
void main_hall_reset_ambient_vol()
{
	if (Main_hall_ambient_loop >= 0) {
		snd_set_volume(Main_hall_ambient_loop, Snds_iface[SND_MAIN_HALL_AMBIENT].default_volume);
	}
}

/**
 * Blit the freespace version number
 */
void main_hall_blit_version()
{
	int w, h;
	char version_string[100];

	// format the version string
	get_version_string(version_string, sizeof(version_string));

	int old_font = gr_get_current_fontnum();
	gr_set_font(Main_hall->font);

	// get the length of the string
	gr_get_string_size(&w,&h,version_string);

	// print the string near the lower left corner
	gr_set_color_fast(&Color_bright_white);
	gr_string(5, gr_screen.max_h_unscaled_zoomed - (h * 2 + 6), version_string, GR_RESIZE_MENU_ZOOMED);

	gr_set_font(old_font);
}

/**
 * Blit any necessary tooltips
 */
void main_hall_maybe_blit_tooltips()
{
	int w, h;

	// if we're over no region - don't blit anything
	if (Main_hall_mouse_region < 0) {
		return;
	}

	if (Main_hall_mouse_region >= (int) Main_hall->regions.size()) {
		Error(LOCATION, "Missing region description for index %d!\n", Main_hall_mouse_region);
	}

	// set the color and blit the string
	if (!help_overlay_active(Main_hall_overlay_id)) {
		const char* desc = Main_hall->regions[Main_hall_mouse_region].description.c_str();
		
		int old_font = gr_get_current_fontnum();
		gr_set_font(Main_hall->font);
		// get the width of the string
		gr_get_string_size(&w, &h, desc);
		int text_y;
		if (Main_hall->region_yval == -1) {
			text_y = gr_screen.max_h_unscaled - ((gr_screen.max_h_unscaled - gr_screen.max_h_unscaled_zoomed) / 2) - Main_hall->tooltip_padding - h;
		} else {
			text_y = Main_hall->region_yval;
		}
		int shader_y = text_y - (Main_hall->tooltip_padding);	// subtract more to pull higher
		
		gr_set_shader(&Main_hall_tooltip_shader);
		gr_shade(0, shader_y, gr_screen.max_w_unscaled, (gr_screen.max_h_unscaled - shader_y), GR_RESIZE_MENU);

		gr_set_color_fast(&Color_bright_white);
		gr_string((gr_screen.max_w_unscaled - w)/2, text_y, desc, GR_RESIZE_MENU);

		gr_set_font(old_font);
	}
}

/**
 * Process help messages
 */
void main_hall_process_help_stuff()
{
	int w, h;
	char str[255];

	// if the timestamp has popped, don't do anything
	if (Main_hall_help_stamp == -1) {
		return;
	}

	// if the timestamp has popped, advance frame
	if (timestamp_elapsed(Main_hall_help_stamp)) {
		Main_hall_f1_text_frame++;
	}

	int old_font = gr_get_current_fontnum();
	gr_set_font(Main_hall->font);

	// otherwise print out the message
	strcpy_s(str, XSTR( "Press F1 for help", 371));
	gr_get_string_size(&w, &h, str);

	int y_anim_offset = Main_hall_f1_text_frame;

	// if anim is off the screen finally, stop altogether
	if ( (y_anim_offset >= (2*Main_hall->tooltip_padding) + h) || (help_overlay_active(Main_hall_overlay_id)) ) {
		Main_hall_f1_text_frame = -1;
		Main_hall_help_stamp = -1;
		F1_text_done = 1;
		return;
	}

	// set the color and print out text and shader
	gr_set_color_fast(&Color_bright_white);
	gr_set_shader(&Main_hall_tooltip_shader);
	gr_shade(0, 0, gr_screen.max_w_unscaled_zoomed, (2*Main_hall->tooltip_padding) + h - y_anim_offset, GR_RESIZE_MENU_ZOOMED);
	gr_string((gr_screen.max_w_unscaled_zoomed - w)/2, Main_hall->tooltip_padding - y_anim_offset, str, GR_RESIZE_MENU_ZOOMED);

	gr_set_font(old_font);
}

/**
 * CommanderDJ - finds the mainhall struct whose name is equal to the passed string
 * @param name_to_find Name of mainhall we're searching for
 * 
 * \return pointer to mainhall if one with a matching name is found
 * \return NULL otherwise
 */
main_hall_defines* main_hall_get_pointer(const SCP_string &name_to_find)
{
	unsigned int i;

	for (i = 0; i < Main_hall_defines.size(); i++) {
		if (Main_hall_defines.at(i).at(0).name == name_to_find) {
			return &Main_hall_defines.at(i).at(main_hall_get_resolution_index(i));
		}
	}
	return NULL;
}

/**
 * CommanderDJ - finds the mainhall struct whose name is equal to the passed string
 * @param name_to_find Name of mainhall we're searching for
 *
 * \return index of mainhall in Main_hall_defines if one with a matching name is found
 * \return -1 otherwise
 */

int main_hall_get_index(const SCP_string &name_to_find)
{
	unsigned int i;

	for (i = 0; i < Main_hall_defines.size(); i++) {
		if (Main_hall_defines.at(i).at(0).name == name_to_find) {
			return i;
		}
	}
	return -1;
}

int main_hall_get_resolution_index(int main_hall_num)
{
	unsigned int i;
	float aspect_ratio = (float)gr_screen.center_w / (float)gr_screen.center_h;

	for (i = Main_hall_defines.at(main_hall_num).size() - 1; i >= 1; i--) {
		main_hall_defines* m = &Main_hall_defines.at(main_hall_num).at(i);
		if (gr_screen.center_w >= m->min_width && gr_screen.center_h >= m->min_height && aspect_ratio >= m->min_aspect_ratio) {
			return i;
		}
	}
	return 0;
}

void main_hall_get_name(SCP_string &name, unsigned int index)
{
	if (index>=Main_hall_defines.size()) {
		name = "";
	} else {
		name = Main_hall_defines.at(index).at(0).name;
	}
}

int main_hall_get_overlay_id()
{
	if (Main_hall==NULL) {
		return -1;
	} else {
		return Main_hall_overlay_id;
	}
}

int main_hall_get_overlay_resolution_index()
{
	if (Main_hall==NULL) {
		return -1;
	} else {
		return Main_hall->help_overlay_resolution_index;
	}
}

// what main hall we're on
int main_hall_id()
{
	if (Main_hall==NULL) {
		return -1;
	} else {
		return main_hall_get_index(Main_hall->name);
	}
}

// CommanderDJ - helper function for initialising intercom sounds vectors based on number of sounds
// To be called after num_intercom_sounds has been parsed
void intercom_sounds_init(main_hall_defines &m)
{
	int idx;

	if (Cmdline_reparse_mainhall) {
		// we could be reparsing with a different number of intercom sounds, so clear these and reinitialise
		m.intercom_delay.clear();
		m.intercom_sounds.clear();
		m.intercom_sound_pan.clear();
	}

	SCP_vector<int> temp;

	for (idx = 0; idx < m.num_random_intercom_sounds; idx++) {
		// intercom_delay
		m.intercom_delay.push_back(temp);

		// each delay has a min and a max
		m.intercom_delay.at(idx).push_back(0);
		m.intercom_delay.at(idx).push_back(0);

		// intercom_sounds
		m.intercom_sounds.push_back(-1);

		// intercom_sound_pan
		m.intercom_sound_pan.push_back(0);
	}
}

/**
 * Iinitialising misc anim vectors based on number of anims
 * @note To be called after num_misc_animations has been parsed
 *
 * @param m Main hall defines
 */
void misc_anim_init(main_hall_defines &m)
{
	int idx;

	if (Cmdline_reparse_mainhall) {
		// we could be reparsing with a different number of misc anims, so clear these and reinitialise
		m.misc_anim_name.clear();
		m.misc_anim_delay.clear();
		m.misc_anim_paused.clear();
		m.misc_anim_group.clear();
		m.misc_anim_coords.clear();
		m.misc_anim_modes.clear();
		m.misc_anim_sound_pan.clear();
		m.misc_anim_special_sounds.clear();
		m.misc_anim_special_trigger.clear();
		m.misc_anim_sound_flag.clear();
	}

	SCP_vector<int> temp;
	SCP_string temp_string;

	for (idx = 0; idx < m.num_misc_animations; idx++) {

		// misc_anim_name
		m.misc_anim_name.push_back(temp_string);

		// misc_anim_delay
		m.misc_anim_delay.push_back(temp);

		// -1 default for the first entry, 0 for the others
		m.misc_anim_delay.at(idx).push_back(-1);
		m.misc_anim_delay.at(idx).push_back(0);
		m.misc_anim_delay.at(idx).push_back(0);

		// misc_anim_paused
		m.misc_anim_paused.push_back(1); // default is paused

		// misc_anim_group
		m.misc_anim_group.push_back(-1);

		// misc_anim_coords
		m.misc_anim_coords.push_back(temp);

		m.misc_anim_coords.at(idx).push_back(0);
		m.misc_anim_coords.at(idx).push_back(0);

		// misc_anim_modes
		m.misc_anim_modes.push_back(MISC_ANIM_MODE_LOOP);

		// misc_anim_sound_pan
		m.misc_anim_sound_pan.push_back(0.0f);

		// misc_anim_special_sounds
		// parse_sound_list deals with the rest of the initialisation for this one
		m.misc_anim_special_sounds.push_back(temp);

		// misc_anim_special_trigger
		m.misc_anim_special_trigger.push_back(temp);

		m.misc_anim_special_trigger.at(idx).push_back(0);

		// misc_anim_sound_flag
		m.misc_anim_sound_flag.push_back(temp);
	}
}

/**
 * Initialising door anim vectors based on number of anims
 * @note To be called after num_door_animations has been parsed
 *
 * @param m Main hall defines
 */
void door_anim_init(main_hall_defines &m)
{
	int idx;

	if (Cmdline_reparse_mainhall) {
		/* since we could be reparsing with a different number of door
		 anims, clear these and reinitialise. */
		m.door_anim_name.clear();
		m.door_anim_coords.clear();
		m.door_sounds.clear();
		m.door_sound_pan.clear();
	}

	SCP_vector<int> temp;
	SCP_string temp_string;

	for (idx = 0; idx < m.num_door_animations; idx++) {
		// door_anim_name
		m.door_anim_name.push_back(temp_string);

		// door_anim_coords
		m.door_anim_coords.push_back(temp);

		// we want two pairs of coordinates for each animation
		m.door_anim_coords.at(idx).push_back(0);
		m.door_anim_coords.at(idx).push_back(0);
		m.door_anim_coords.at(idx).push_back(0);
		m.door_anim_coords.at(idx).push_back(0);

		// door_sounds
		m.door_sounds.push_back(temp);

		// door_sound_pan
		m.door_sound_pan.push_back(0.0f);
	}
}

void region_info_init(main_hall_defines &m)
{
	if (Cmdline_reparse_mainhall) {
		m.regions.clear();
	}
	
	main_hall_region defaults[] = {
		main_hall_region(0,  0,  XSTR( "Exit FreeSpace 2", 353), EXIT_REGION, ""),
		main_hall_region(1, 'B', XSTR( "Barracks - Manage your FreeSpace 2 pilots", 354), BARRACKS_REGION, ""),
		main_hall_region(2, 'R', XSTR( "Ready room - Start or continue a campaign", 355), START_REGION, ""),
		main_hall_region(3, 'T', XSTR( "Tech room - View specifications of FreeSpace 2 ships and weaponry", 356), TECH_ROOM_REGION, ""),
		main_hall_region(4,  0,  XSTR( "Options - Change your FreeSpace 2 options", 357), OPTIONS_REGION, ""),
		main_hall_region(5, 'C', XSTR( "Campaign Room - View all available campaigns", 358), CAMPAIGN_ROOM_REGION, ""),
		main_hall_region(6, 'G', "Quick start", QUICK_START_REGION, "")
	};
	
	for (int idx = 0; idx < 7; idx++) {
		m.regions.push_back(defaults[idx]);
	}
	
	// XSTR( "Multiplayer - Start or join a multiplayer game", 359)
	m.default_readyroom = true;
}

/**
 * Read in main hall table
 */
void main_hall_table_init()
{
	// clear the main hall entries
	Main_hall_defines.clear();

	// if mainhall.tbl exists, parse it
	if (cf_exists_full("mainhall.tbl", CF_TYPE_TABLES)) {
		parse_main_hall_table("mainhall.tbl");
	}

	// parse any modular tables
	parse_modular_table("*-hall.tbm", parse_main_hall_table);
}

// read in main hall table
void parse_main_hall_table(const char* filename)
{
	SCP_vector<main_hall_defines> temp_vector;
	main_hall_defines *m, temp;
	int idx, s_idx, m_idx;
	int num_resolutions = 2;
	unsigned int count;
	char temp_string[MAX_FILENAME_LEN];
	SCP_string temp_scp_string;

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);

		reset_parse();

		if (optional_string("$Num Resolutions:")) {
			stuff_int(&num_resolutions);
		}

		if (num_resolutions < 1) {
			Error(LOCATION, "$Num Resolutions in %s is %d. (Must be 1 or greater)", filename, num_resolutions);
		}

		// go for it
		count = Main_hall_defines.size();
		while (!optional_string("#end")) {
			Main_hall_defines.push_back(temp_vector);
			// read in all resolutions
			for (m_idx = 0; m_idx < num_resolutions; m_idx++) {
				Main_hall_defines.at(count).push_back(temp);
				m = &Main_hall_defines.at(count).at(m_idx);

				// ready
				required_string("$Main Hall");

				// Parse the entry name for the first resolution, checking for duplicates and erroring if necessary
				if (m_idx == 0) {
					if (optional_string("+Name:")) {
						stuff_string(temp_string, F_RAW, MAX_FILENAME_LEN);

						// we can't have two mainhalls with the same name
						if (main_hall_get_pointer(temp_string) == NULL) {
							m->name = temp_string;
						}
						else {
							Error(LOCATION, "A mainhall with the name '%s' already exists. All mainhalls must have unique names.", temp_string);
						}
					}
					else {
						snprintf(temp_string, MAX_FILENAME_LEN, "%u", count);
						m->name = temp_string;
					}
				}
				else {
					if (optional_string("+Name:")) {
						stuff_string(temp_string, F_RAW, MAX_FILENAME_LEN);

						/**
						 * the reason that this is an error is that even if we were to change the names to match
						 * it is very likely the user would get the wrong mainhall loaded since their campaign files
						 * may still refer to the entry with the incorrect name
						 */
						if (strcmp(temp_string, Main_hall_defines.at(count).at(0).name.c_str()) != 0) {
							Error(LOCATION, "The mainhall '%s' has different names for different resolutions. All resolutions must have the same name. Either remove the hi-res entries' names entirely or set them to match the first resolution entry's name.", Main_hall_defines.at(0).at(count).name.c_str());
						}
					}

					m->name = Main_hall_defines.at(count).at(0).name;
				}

				// add cheats
				while (optional_string("+Cheat String:")) {
					stuff_string(temp_scp_string, F_RAW);
					m->cheat.push_back(temp_scp_string);

					if (temp_scp_string.size() > MAIN_HALL_MAX_CHEAT_LEN) {
						// Since the value is longer than the cheat buffer it will never match.

						Warning(LOCATION, "The value '%s' for '+Cheat String:' is too long! It can be at most %d characters long.", temp_scp_string.c_str(), MAIN_HALL_MAX_CHEAT_LEN);
					}

					required_string("+Anim To Change:");
					stuff_string(temp_scp_string, F_NAME);
					m->cheat_anim_from.push_back(temp_scp_string);

					required_string("+Anim To Change To:");
					stuff_string(temp_scp_string, F_NAME);
					m->cheat_anim_to.push_back(temp_scp_string);
				}

				// minimum resolution
				if (optional_string("+Min Resolution:")) {
					stuff_int(&m->min_width);
					stuff_int(&m->min_height);
				}
				else if (m_idx == 0) {
					m->min_width = 0;
					m->min_height = 0;
				}
				else {
					m->min_width = GR_1024_THRESHOLD_WIDTH;
					m->min_height = GR_1024_THRESHOLD_HEIGHT;
				}

				// minimum aspect ratio
				if (optional_string("+Min Aspect Ratio:")) {
					stuff_float(&m->min_aspect_ratio);
				}
				else {
					m->min_aspect_ratio = 0.0f;
				}

				// bitmap and mask
				required_string("+Bitmap:");
				stuff_string(temp_string, F_NAME, MAX_FILENAME_LEN);
				m->bitmap = temp_string;

				required_string("+Mask:");
				stuff_string(temp_string, F_NAME, MAX_FILENAME_LEN);
				m->mask = temp_string;

				required_string("+Music:");
				stuff_string(temp_string, F_NAME, MAX_FILENAME_LEN);
				m->music_name = temp_string;

				// Goober5000
				if (optional_string("+Substitute Music:")) {
					stuff_string(temp_string, F_NAME, MAX_FILENAME_LEN);
					m->substitute_music_name = temp_string;
				}

				if (optional_string("+Help Overlay:")) {
					stuff_string(temp_string, F_NAME, MAX_FILENAME_LEN);
					m->help_overlay_name = temp_string;
				}

				if (optional_string("+Help Overlay Resolution Index:")) {
					stuff_int(&m->help_overlay_resolution_index);
				}
				else {
					m->help_overlay_resolution_index = m_idx;
				}

				// zoom area
				if (optional_string("+Zoom To:")) {
					stuff_int(&m->zoom_area_width);
					stuff_int(&m->zoom_area_height);
				}
				else {
					m->zoom_area_width = -1;
					m->zoom_area_height = -1;
				}

				// intercom sounds
				required_string("+Num Intercom Sounds:");
				stuff_int(&m->num_random_intercom_sounds);

				// initialise intercom sounds vectors
				intercom_sounds_init(*m);

				for (idx = 0; idx < m->num_random_intercom_sounds; idx++) {
					// intercom delay
					required_string("+Intercom delay:");
					stuff_int(&m->intercom_delay.at(idx).at(0));
					stuff_int(&m->intercom_delay.at(idx).at(1));
				}

				for (idx = 0; idx < m->num_random_intercom_sounds; idx++) {
					// intercom sound id
					parse_sound("+Intercom sound:", &m->intercom_sounds.at(idx), "+Intercom sound:", PARSE_SOUND_INTERFACE_SOUND);
				}

				for (idx = 0; idx < m->num_random_intercom_sounds; idx++) {
					// intercom pan
					required_string("+Intercom pan:");
					stuff_float(&m->intercom_sound_pan.at(idx));
				}

				// misc animations
				required_string("+Num Misc Animations:");
				stuff_int(&m->num_misc_animations);

				// initialise the misc anim vectors
				misc_anim_init(*m);

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim names
					required_string("+Misc anim:");
					stuff_string(temp_string, F_NAME, MAX_FILENAME_LEN);
					m->misc_anim_name.at(idx) = (SCP_string)temp_string;
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim groups, optionally
					if (optional_string("+Misc anim group:")) {
						stuff_int(&m->misc_anim_group.at(idx));
					}
					else {
						m->misc_anim_group.at(idx) = -1;
					}
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim delay
					required_string("+Misc anim delay:");
					stuff_int(&m->misc_anim_delay.at(idx).at(0));
					stuff_int(&m->misc_anim_delay.at(idx).at(1));
					stuff_int(&m->misc_anim_delay.at(idx).at(2));
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim coords
					required_string("+Misc anim coords:");
					stuff_int(&m->misc_anim_coords.at(idx).at(0));
					stuff_int(&m->misc_anim_coords.at(idx).at(1));
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim mode
					required_string("+Misc anim mode:");
					stuff_int(&m->misc_anim_modes.at(idx));
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim pan
					required_string("+Misc anim pan:");
					stuff_float(&m->misc_anim_sound_pan.at(idx));
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim sound id
					parse_sound_list("+Misc anim sounds:", m->misc_anim_special_sounds.at(idx), "+Misc anim sounds:", PARSE_SOUND_INTERFACE_SOUND);
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim sound triggers
					required_string("+Misc anim trigger:");
					int temp_int = 0;
					stuff_int(&temp_int);
					for (s_idx = 0; s_idx < temp_int; s_idx++) {
						m->misc_anim_special_trigger.at(idx).push_back(0);
						stuff_int(&m->misc_anim_special_trigger.at(idx).at(s_idx));
					}
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim sound handles - deprecated, but deal with it just in case
					if (optional_string("+Misc anim handles:")) {
						advance_to_eoln(NULL);
					}
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// anim sound flags - table flag deprecated, so ignore user input
					if (optional_string("+Misc anim flags:")) {
						advance_to_eoln(NULL);
					}

					// we need one flag for each sound
					Assert(m->misc_anim_special_sounds.at(idx).size() < INT_MAX);
					for (s_idx = 0; s_idx < (int)m->misc_anim_special_sounds.at(idx).size(); s_idx++) {
						m->misc_anim_sound_flag.at(idx).push_back(0);
					}
				}

				for (idx = 0; idx < m->num_misc_animations; idx++) {
					// render over doors - default to false

					if (optional_string("+Misc anim over doors:")) {
						bool temp_b;
						stuff_boolean(&temp_b);
						m->misc_anim_over_doors.push_back(temp_b);
					}
					else {
						m->misc_anim_over_doors.push_back(0);
					}
				}

				region_info_init(*m);

				// door animations
				required_string("+Num Door Animations:");
				stuff_int(&m->num_door_animations);

				// initialise the door anim vectors
				door_anim_init(*m);

				for (idx = 0; idx < m->num_door_animations; idx++) {
					// door name
					required_string("+Door anim:");
					stuff_string(temp_string, F_NAME, MAX_FILENAME_LEN);
					m->door_anim_name.at(idx) = (SCP_string)temp_string;
				}

				for (idx = 0; idx < m->num_door_animations; idx++) {
					// door coords
					required_string("+Door coords:");
					stuff_int(&m->door_anim_coords.at(idx).at(0));
					stuff_int(&m->door_anim_coords.at(idx).at(1));
					stuff_int(&m->door_anim_coords.at(idx).at(2));
					stuff_int(&m->door_anim_coords.at(idx).at(3));
				}

				for (idx = 0; idx < m->num_door_animations; idx++) {
					// door open and close sounds
					parse_sound_list("+Door sounds:", m->door_sounds.at(idx), "+Door sounds:", (parse_sound_flags)(PARSE_SOUND_INTERFACE_SOUND | PARSE_SOUND_SCP_SOUND_LIST));
				}

				for (idx = 0; idx < m->num_door_animations; idx++) {
					// door pan value
					required_string("+Door pan:");
					stuff_float(&m->door_sound_pan[idx]);
				}

				int mask;
				for (idx = 0; optional_string("+Door mask value:"); idx++) {
					// door mask
					stuff_string(temp_string, F_RAW, MAX_FILENAME_LEN);

					mask = (int)strtol(temp_string, NULL, 0);
					mask = 255 - mask;

					if (idx >= (int)m->regions.size()) {
						m->regions.resize(idx + 1);
					}
					m->regions[idx].mask = mask;
				}

				for (idx = 0; optional_string("+Door action:"); idx++) {
					// door action

					if (idx >= (int)m->regions.size()) {
						m->regions.resize(idx + 1);
					}

					if (optional_string("Script")) {
						m->regions[idx].action = SCRIPT_REGION;
						stuff_string(m->regions[idx].lua_action, F_RAW);
					}
					else {
						stuff_string(temp_scp_string, F_RAW);

						int action = -1;
						for (int i = 0; Main_hall_region_map[i].name != NULL; i++) {
							if (temp_scp_string == Main_hall_region_map[i].name) {
								action = Main_hall_region_map[i].mask;
								break;
							}
						}

						if (action == -1) {
							SCP_string err_msg = "";
							for (int i = 0; Main_hall_region_map[i].name != NULL; i++) {
								if (i != 0) {
									err_msg += ", ";
								}
								err_msg += Main_hall_region_map[i].name;
							}

							Error(LOCATION, "Unkown Door Region '%s'! Expected one of: %s", temp_scp_string.c_str(), err_msg.c_str());
						}

						m->regions[idx].action = action;
					}
				}

				for (idx = 0; optional_string("+Door key:"); idx++) {
					// door key
					stuff_string(temp_string, F_RAW, MAX_FILENAME_LEN);

					if ((int)m->regions.size() <= idx) {
						m->regions.resize(idx + 1);
					}
					m->regions[idx].key = temp_string[0];
				}

				for (idx = 0; optional_string("+Door description:"); idx++) {
					// region description (tooltip)
					stuff_string(temp_scp_string, F_MESSAGE);

					if (temp_scp_string != "default") {
						if (idx >= (int)m->regions.size()) {
							m->regions.resize(idx + 1);
						}

						m->regions[idx].description = temp_scp_string;

						if (idx == 2) {
							m->default_readyroom = false;
						}
					}
				}

				// font for tooltips and other text
				if (optional_string("+Font:")) {
					stuff_int(&m->font);
				}
				else {
					m->font = FONT1;
				}

				// tooltip padding
				if (optional_string("+Tooltip Padding:")) {
					stuff_int(&m->tooltip_padding);
				}
				else {
					m->tooltip_padding = -1; // we'll get the default value later
				}

				// tooltip y location
				if (optional_string("+Tooltip Y:")) {
					stuff_int(&m->region_yval);
				}
				else {
					m->region_yval = -1;
				}
			}

			count++;
		}

		// free up memory from parsing the mainhall tbl
		stop_parse();
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

/**
 * Make the vasudan main hall funny
 */
void main_hall_vasudan_funny()
{
	Vasudan_funny = 1;
}

/**
 * Lookup if Vasudan main hall, based upon music name
 * @return 1 if true, 0 if false
 */
int main_hall_is_vasudan()
{
	// kind of a hack for now
	return (!stricmp(Main_hall->music_name.c_str(), "Psampik") || !stricmp(Main_hall->music_name.c_str(), "Psamtik"));
}

/**
 * Silence sounds on mainhall if we hit a pause mode (ie. lost window focus, minimized, etc)
 */
void main_hall_pause()
{
	if (Main_hall_paused) {
		return;
	}

	Main_hall_paused = 1;

	audiostream_pause(Main_hall_music_handle);

	main_hall_stop_ambient();
}

/**
 * Recover from a paused state (ie. lost window focus, minimized, etc)
 */
void main_hall_unpause()
{
	if (!Main_hall_paused) {
		return;
	}

	Main_hall_paused = 0;

	audiostream_unpause(Main_hall_music_handle);

	main_hall_start_ambient();
}
