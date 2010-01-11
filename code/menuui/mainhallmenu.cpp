/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "menuui/mainhallmenu.h"
#include "palman/palman.h"
#include "gamesequence/gamesequence.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "io/key.h"
#include "io/timer.h"
#include "menuui/snazzyui.h"
#include "playerman/player.h"
#include "sound/audiostr.h"
#include "gamesnd/gamesnd.h"
#include "gamesnd/eventmusic.h"
#include "io/mouse.h"
#include "gamehelp/contexthelp.h"
#include "cmdline/cmdline.h"
#include "popup/popup.h"
#include "menuui/playermenu.h"
#include "freespace2/freespace.h"
#include "globalincs/alphacolors.h"
#include "graphics/generic.h"
#include "menuui/fishtank.h"
#include "mission/missioncampaign.h"
#include "parse/parselo.h"
#include "network/multiui.h"
#include "network/multiutil.h"
#include "network/multi_voice.h"
#include "network/multi.h"

#ifndef NDEBUG
#include "cutscene/movie.h"
#include "demo/demo.h"
#include "mission/missionload.h"
#endif



// ----------------------------------------------------------------------------
// MAIN HALL DATA DEFINES
//
#define MAX_RANDOM_INTERCOM_SOUNDS				10
#define NUM_RANDOM_INTERCOM_SOUNDS_0			3
#define NUM_RANDOM_INTERCOM_SOUNDS_1			3

#define MAX_MISC_ANIMATIONS						10
#define NUM_MISC_ANIMATIONS_0						2
#define NUM_MISC_ANIMATIONS_1						4

#define MAX_DOOR_ANIMATIONS						10
#define NUM_DOOR_ANIMATIONS_0						6
#define NUM_DOOR_ANIMATIONS_1						6

#define MAX_DOOR_SOUNDS								10
#define NUM_DOOR_SOUNDS_0							6
#define NUM_DOOR_SOUNDS_1							6

#define MISC_ANIM_MODE_LOOP						0				// loop the animation
#define MISC_ANIM_MODE_HOLD						1				// play to the end and hold the animation
#define MISC_ANIM_MODE_TIMED						2				// uses timestamps to determine when a finished anim should be checked again

#define NUM_REGIONS									7				// (6 + 1 for multiplayer equivalent of campaign room)
typedef struct main_hall_defines {
	// bitmap and mask
	char bitmap[MAX_FILENAME_LEN];
	char mask[MAX_FILENAME_LEN];

	// music
	char music_name[MAX_FILENAME_LEN];
	char substitute_music_name[MAX_FILENAME_LEN];

	// intercom defines -------------------
	
	// # of intercom sounds
	int num_random_intercom_sounds;
	
	// random (min/max) delays between playing intercom sounds
	int intercom_delay[MAX_RANDOM_INTERCOM_SOUNDS][2];
	
	// intercom sounds themselves
	int intercom_sounds[MAX_RANDOM_INTERCOM_SOUNDS];

	// intercom sound pan values
	float intercom_sound_pan[MAX_RANDOM_INTERCOM_SOUNDS];


	// misc animations --------------------

	// # of misc animations
	int num_misc_animations;

	// filenames of the misc animations
	char misc_anim_name[MAX_MISC_ANIMATIONS][MAX_FILENAME_LEN];

	// Time until we will next play a given misc animation, min delay, and max delay
	int misc_anim_delay[MAX_MISC_ANIMATIONS][3];

	//	coords of where to play the misc anim
	int misc_anim_coords[MAX_MISC_ANIMATIONS][2];
	
	// misc anim play modes (see MISC_ANIM_MODE_* above)
	int misc_anim_modes[MAX_MISC_ANIMATIONS];

	// panning values for each of the misc anims
	float misc_anim_sound_pan[MAX_MISC_ANIMATIONS];

	// [N][0] == # of sounds, [N][1-9] sound index
	int misc_anim_special_sounds[MAX_MISC_ANIMATIONS][10];

	// [N][0] == # of triggers, [N][1-9] >= frame num
	int misc_anim_special_trigger[MAX_MISC_ANIMATIONS][10];

	// [N][0] == # of handles, [N][1-9] == sound handle num
	int misc_anim_sound_handles[MAX_MISC_ANIMATIONS][10];

	// [N][0] == # of handles, [N][1-9] == sound "should" be playing
	int misc_anim_sound_flag[MAX_MISC_ANIMATIONS][10];	


	// door animations --------------------

	// # of door animations
	int num_door_animations;
	
	// filenames of the door animations
	char door_anim_name[MAX_DOOR_ANIMATIONS][MAX_FILENAME_LEN];	

	// first pair : coords of where to play a given door anim
	// second pair : center of a given door anim in windowed mode
	int door_anim_coords[MAX_DOOR_ANIMATIONS][4];
	

	// door sounds ------------------------

	// # of door sounds
	int num_door_sounds;

	// sounds for each region (open/close)
	int door_sounds[MAX_DOOR_SOUNDS][2];

	// pan values for the door sounds
	float door_sound_pan[MAX_DOOR_SOUNDS];

	
	// region descriptions ----------------
	
	// text (tooltip) description
	char *region_descript[NUM_REGIONS];

	// y coord of where to draw tooltip text
	int region_yval;

} main_hall_defines;


// use main hall 0 by default
main_hall_defines Main_hall_defines[GR_NUM_RESOLUTIONS][MAIN_HALLS_MAX];
main_hall_defines *Main_hall = &Main_hall_defines[0][0];

int Num_main_halls = 0;

int Vasudan_funny = 0;
int Vasudan_funny_plate = -1;

char Main_hall_campaign_cheat[512] = "";
	
// ----------------------------------------------------------------------------
// MISC interface data
//
// is the main hall inited (for reentrancy)
int Main_hall_inited = 0;

// handle to the playing music
int Main_hall_music_handle = -1;

// background bitmap handle
int Main_hall_bitmap;

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
#define MAIN_HALL_REGION_LINGER				175				// in ms
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
//anim *Main_hall_misc_anim[MAX_MISC_ANIMATIONS];
generic_anim Main_hall_misc_anim[MAX_MISC_ANIMATIONS];

// the instance of a given misc animation
//anim_instance *Main_hall_misc_anim_instance[MAX_MISC_ANIMATIONS];

// handle starting, stopping and randomizing misc animations
void main_hall_handle_misc_anims();									

// cull any finished misc animation instances
void main_hall_cull_misc_anim_instances();								

// render all playing misc animations
void main_hall_render_misc_anims(float frametime);


// ----------------------------------------------------------------------------
// DOOR animations (not all of these are doors anymore, but they're doorlike _regions_)
//
#define DOOR_TEXT_X 100
#define DOOR_TEXT_Y 450

// the door animations themselves
//anim *Main_hall_door_anim[MAX_DOOR_ANIMATIONS];
generic_anim Main_hall_door_anim[MAX_DOOR_ANIMATIONS];

// the instance of a given door animation
//anim_instance *Main_hall_door_anim_instance[MAX_DOOR_ANIMATIONS];

// render all playing door animations
void main_hall_render_door_anims(float frametime);


// ----------------------------------------------------------------------------
// SNAZZY MENU stuff
//
#define NUM_MAIN_HALL_REGIONS 10
#define NUM_MAIN_HALL_MOUSE_REGIONS 6

// region mask #'s (identifiers)
#define EXIT_REGION				 0
#define BARRACKS_REGION			 1
#define READY_ROOM_REGION		 2
#define TECH_ROOM_REGION		 3
#define OPTIONS_REGION			 4
#define CAMPAIGN_ROOM_REGION	 5
#define MULTIPLAYER_REGION     10
#define LOAD_MISSION_REGION    11
#define QUICK_START_REGION     12
#define SKILL_LEVEL_REGION     13

// all the menu regions in the main hall
MENU_REGION Main_hall_region[NUM_MAIN_HALL_REGIONS];

// # of regions (options) on this screen. parsed from a table
int Main_hall_num_options;

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
int Main_hall_door_sound_handles[MAX_DOOR_SOUNDS] = {		
	-1,-1,-1,-1,-1,-1
};

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
#define MAIN_HALL_NOTIFY_TIME  3500

// timestamp for the notification messages
int Main_hall_notify_stamp = -1;

// text to display as the current notification message
char Main_hall_notify_text[300]="";

// set the current notification string and the associated timestamp
void main_hall_set_notify_string(char *str);

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
static int Main_hall_tooltip_padding[GR_NUM_RESOLUTIONS] = {
	4,		// GR_640
	7,		// GR_1024
};
static int Main_hall_f1_text_frame = 0;
static int F1_text_done = 0;

// read in main hall table
void main_hall_read_table();

// "press f1" for help stuff
#define MAIN_HALL_HELP_TIME		5000
int Main_hall_help_stamp = -1;
void main_hall_process_help_stuff();


// ----------------------------------------------------------------------------
// VOICE RECORDING STUFF
//

// are we currently recording voice?
int Recording = 0;


// called when multiplayer clicks on the ready room door.  May pop up dialog depending on network
// connection status and errors
void main_hall_do_multi_ready()
{
	int error;

	error = psnet_get_network_status();
	switch( error ) {
	case NETWORK_ERROR_NO_TYPE:
		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have not defined your type of Internet connection.  Please run the Launcher, hit the setup button, and go to the Network tab and choose your connection type.", 360));
		break;
	case NETWORK_ERROR_NO_WINSOCK:
		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "Winsock is not installed.  You must have TCP/IP and Winsock installed to play multiplayer FreeSpace.", 361));
		break;
	case NETWORK_ERROR_NO_PROTOCOL:
		if(Multi_options_g.protocol == NET_TCP){
			popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "TCP/IP protocol not found.  This protocol is required for multiplayer FreeSpace.", -1));
		} else {
			Assert(Multi_options_g.protocol == NET_IPX);
			popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "IPX protocol not found.  This protocol is required for multiplayer FreeSpace.", -1));
		}
		break;
	case NETWORK_ERROR_CONNECT_TO_ISP:
		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected Dial Up Networking as your type of connection to the Internet.  You are not currently connected.  You must connect to your ISP before continuing on past this point.", 363));
		break;
	case NETWORK_ERROR_LAN_AND_RAS:
		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have indicated that you use a LAN for networking.  You also appear to be dialed into your ISP.  Please disconnect from your service provider, or choose Dial Up Networking.", 364));
		break;

	case NETWORK_ERROR_NONE:
	default:
		break;
	}

	// if our selected protocol is not active
	if((Multi_options_g.protocol == NET_TCP) && !Tcp_active){
		if (Tcp_failure_code == WSAEADDRINUSE) {
			popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected TCP/IP for multiplayer FreeSpace, but the TCP socket is already in use.  Check for another instance and/or use the \"-port <port_num>\" command line option to select an available port.", -1));
		} else {
			popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected TCP/IP for multiplayer FreeSpace, but the TCP/IP protocol was not detected on your machine.", 362));
		}

		return;
	} 
	if((Multi_options_g.protocol == NET_IPX) && !Ipx_active){		
		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You have selected IPX for multiplayer FreeSpace, but the IPX protocol was not detected on your machine.", 1402));
		return;
	} 

	if ( error != NETWORK_ERROR_NONE ){
		return;
	}

	// 7/9/98 -- MWA.  Deal with the connection speed issue.  make a call to the multiplayer code to
	// determine is a valid connection setting exists
	if ( Multi_connection_speed == CONNECTION_SPEED_NONE ) {
		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "You must define your connection speed.  Please run the Launcher, hit the setup button, and go to the Network tab and choose your connection speed.", 986) );
		return;
	}

	// go to parallax online
#ifdef MULTIPLAYER_BETA_BUILD // do we want this for FS2_DEMO
	Multi_options_g.pxo = 1;
	Multi_options_g.protocol = NET_TCP;	
	gameseq_post_event( GS_EVENT_PXO );
#else
	if (Om_tracker_flag) {
		Multi_options_g.protocol = NET_TCP;
		gameseq_post_event(GS_EVENT_PXO);
	} else {
		// go to the regular join game screen 	
		gameseq_post_event( GS_EVENT_MULTI_JOIN_GAME );	
	}
#endif	//MULTIPLAYER_BETA_BUILD

	// select protocol
	psnet_use_protocol(Multi_options_g.protocol);
}


// blit some small color indicators to show whether ships.tbl and weapons.tbl are valid
// green == valid, red == invalid.
// ships.tbl will be on the left, weapons.tbl on the right
int Mh_ship_table_status[GR_NUM_RESOLUTIONS][2] = {
	{ 1, 479 },
	{ 1, 767 }
};
int Mh_weapon_table_status[GR_NUM_RESOLUTIONS][2] = {
	{ 3, 479 },
	{ 3, 767 }
};
void main_hall_blit_table_status()
{
	// blit ship table status
	gr_set_color_fast(Game_ships_tbl_valid ? &Color_bright_green : &Color_bright_red);
	gr_line(Mh_ship_table_status[gr_screen.res][0], Mh_ship_table_status[gr_screen.res][1], Mh_ship_table_status[gr_screen.res][0], Mh_ship_table_status[gr_screen.res][1]);

	// blit weapon table status
	gr_set_color_fast(Game_weapons_tbl_valid ? &Color_bright_green : &Color_bright_red);
	gr_line(Mh_weapon_table_status[gr_screen.res][0], Mh_weapon_table_status[gr_screen.res][1], Mh_weapon_table_status[gr_screen.res][0], Mh_ship_table_status[gr_screen.res][1]);
}

// bash the player to a specific mission in a campaign
void main_hall_campaign_cheat()
{
	char *ret = popup_input(0, XSTR("Enter mission name.\n\n* This will destroy all legitimate progress in this campaign. *", -1));

	// yay
	if(ret != NULL) {
		// strcpy_s(Main_hall_campaign_cheat, ret);		
		mission_campaign_jump_to_mission(ret);
	}
}

// -------------------------------------------------------------------------------------------------------------------
// FUNCTION DEFINITIONS BEGIN
//

// initialize the main hall proper
void main_hall_init(int main_hall_num)
{
	ubyte bg_type;
	if ( Main_hall_inited ) {
		return;
	}

	int idx,s_idx;
	char temp[100], whee[100];	

	// read in the main hall table
	main_hall_read_table();

	if(!Num_main_halls) {
		Error(LOCATION, "No main halls were loaded to initialize.");
	}

	if ((main_hall_num < 0) || (main_hall_num >= Num_main_halls)) {
		Warning(LOCATION, "Tried to load a main hall %d, but valid main halls are only 0 through %d; defaulting to main hall 0", main_hall_num, Num_main_halls-1);
		main_hall_num = 0;
	}

	// create the snazzy interface and load up the info from the table
	snazzy_menu_init();
	read_menu_tbl(NOX("MAIN HALL"), temp, whee, Main_hall_region, &Main_hall_num_options, 0);

	// assign the proper main hall data
	Assert((main_hall_num >= 0) && (main_hall_num < Num_main_halls));
	Main_hall = &Main_hall_defines[gr_screen.res][main_hall_num];	

	// tooltip strings
	Main_hall->region_descript[0] = XSTR( "Exit FreeSpace 2", 353);
	Main_hall->region_descript[1] = XSTR( "Barracks - Manage your FreeSpace 2 pilots", 354);
	Main_hall->region_descript[2] = XSTR( "Ready room - Start or continue a campaign", 355);
	Main_hall->region_descript[3] = XSTR( "Tech room - View specifications of FreeSpace 2 ships and weaponry", 356);
	Main_hall->region_descript[4] = XSTR( "Options - Change your FreeSpace 2 options", 357);
	Main_hall->region_descript[5] = XSTR( "Campaign Room - View all available campaigns", 358);
	Main_hall->region_descript[6] = XSTR( "Multiplayer - Start or join a multiplayer game", 359);
	
	// init tooltip shader													// nearly black
	gr_create_shader(&Main_hall_tooltip_shader, 5, 5, 5, 168);

	// load the background bitmap
	Main_hall_bitmap = bm_load(Main_hall->bitmap);
	bg_type = bm_get_type(Main_hall_bitmap);
	if(Main_hall_bitmap < 0){
		nprintf(("General","WARNING! Couldn't load main hall background bitmap %s\n", Main_hall->bitmap));
	}

	// set the interface palette 
#ifndef HARDWARE_ONLY
	palette_use_bm_palette(Main_hall_bitmap);	
#endif

	Main_hall_mask_w = -1;
	Main_hall_mask_h = -1;
		
	// load the mask
	Main_hall_mask = bm_load(Main_hall->mask);
	if (Main_hall_mask < 0) {
		nprintf(("General","WARNING! Couldn't load main hall background mask %s\n", Main_hall->mask));
		if (gr_screen.res == 0) {
			Error(LOCATION,"Could not load in main hall mask '%s'!\n\n(This error most likely means that you are missing required 640x480 interface art.)", Main_hall->mask);
		} else {
			Error(LOCATION,"Could not load in main hall mask '%s'!\n\n(This error most likely means that you are missing required 1024x768 interface art.)", Main_hall->mask);
		}
	} else {
		// get a pointer to bitmap by using bm_lock(), so we can feed it to he snazzy menu system
		Main_hall_mask_bitmap = bm_lock(Main_hall_mask, 8, BMP_AABITMAP);
		Main_hall_mask_data = (ubyte*)Main_hall_mask_bitmap->data;
		bm_get_info(Main_hall_mask, &Main_hall_mask_w, &Main_hall_mask_h);
	}

	// load up the misc animations, and nullify all the delay timestamps for the misc animations
	for(idx=0;idx<Main_hall->num_misc_animations;idx++) {
		generic_anim_init(&Main_hall_misc_anim[idx], Main_hall->misc_anim_name[idx]);
		Main_hall_misc_anim[idx].ani.bg_type = bg_type;
		if(generic_anim_stream(&Main_hall_misc_anim[idx]) == -1) {
			nprintf(("General","WARNING!, Could not load misc %s anim in main hall\n",Main_hall->misc_anim_name));
		}
		else {
			//start paused
			Main_hall_misc_anim[idx].direction |= GENERIC_ANIM_DIRECTION_PAUSED;
			if(Main_hall->misc_anim_modes[idx] == MISC_ANIM_MODE_HOLD)
				Main_hall_misc_anim[idx].direction |= GENERIC_ANIM_DIRECTION_NOLOOP;
		}
		
		// null out the delay timestamps
		Main_hall->misc_anim_delay[idx][0] = -1;
	}	

	// load up the door animations
	for(idx=0;idx<Main_hall->num_door_animations;idx++) {
		generic_anim_init(&Main_hall_door_anim[idx], Main_hall->door_anim_name[idx]);
		Main_hall_door_anim[idx].ani.bg_type = bg_type;
		if(generic_anim_stream(&Main_hall_door_anim[idx]) == -1) {
			nprintf(("General","WARNING!, Could not load door anim %s in main hall\n",Main_hall->door_anim_name[idx]));
		}
		else
			Main_hall_door_anim[idx].direction = GENERIC_ANIM_DIRECTION_BACKWARDS | GENERIC_ANIM_DIRECTION_NOLOOP;
	}	

	// load in help overlay bitmap		
	if(Main_hall == &Main_hall_defines[gr_screen.res][0]) {
		Main_hall_overlay_id = MH_OVERLAY;
	} else {
//		Assert(Main_hall == &Main_hall_defines[gr_screen.res][1]);
		Main_hall_overlay_id = MH2_OVERLAY;
	}
	help_overlay_load(Main_hall_overlay_id);
	help_overlay_set_state(Main_hall_overlay_id,0);		

	// check to see if the "very first pilot" flag is set, and load the overlay if so
	if (!F1_text_done) {
		if (Main_hall_f1_text_frame == 0) {
			Main_hall_help_stamp = timestamp(MAIN_HALL_HELP_TIME);
		} else {
			F1_text_done = 1;
		}
	}

/*
	if(Player_select_very_first_pilot) {				
		Main_hall_help_stamp = timestamp(MAIN_HALL_HELP_TIME);
		
		// don't display the "press f1" message more than once
		Player_select_very_first_pilot = 0;
	} else {
		Main_hall_help_stamp = -1;
	}
*/
	Main_hall_region_linger_stamp = -1;

	strcpy_s(Main_hall_campaign_cheat, "");

	// zero out the door sounds
	for(idx=0;idx<Main_hall->num_door_sounds;idx++){
		Main_hall_door_sound_handles[idx] = -1;
	}

	// zero out the misc anim sounds
	for(idx=0;idx<Main_hall->num_misc_animations;idx++){
		for(s_idx = 1;s_idx < 10;s_idx++){
			Main_hall->misc_anim_sound_handles[idx][s_idx] = -1;
			Main_hall->misc_anim_sound_flag[idx][s_idx] = 0;
		}
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

void main_hall_exit_game()
{
#if defined(NDEBUG) || defined(INTERPLAYQA)
	int choice;

	// stop music first
	main_hall_stop_music();
	main_hall_stop_ambient();
	choice = popup( PF_NO_NETWORKING | PF_BODY_BIG, 2, POPUP_NO, POPUP_YES, XSTR( "Exit Game?", 365));
	if ( choice == 1 ) {
		gameseq_post_event(GS_EVENT_QUIT_GAME);
	} else {
		main_hall_start_music();
		main_hall_start_ambient();
	}
#else
	gameseq_post_event(GS_EVENT_QUIT_GAME);
#endif
}


// do a frame for the main hall
void main_hall_do(float frametime)
{
	int code, key, snazzy_action;	

	// need to ensure ambient is playing, since it may be stopped by a playing movie
	main_hall_start_ambient();

	// handle any random intercom sound details
	main_hall_handle_random_intercom_sounds();

	// handle any mouse clicks
	main_hall_handle_right_clicks();	

	// handle any sound details
	main_hall_cull_door_sounds();	

	// process any keypresses/mouse events
	snazzy_action = -1;
	code = snazzy_menu_do(Main_hall_mask_data, Main_hall_mask_w, Main_hall_mask_h, Main_hall_num_options, Main_hall_region, &snazzy_action, 1, &key);

	if(key){
		extern void game_process_cheats(int k);
		game_process_cheats(key);
	}
	switch(key){
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
	case KEY_DEBUGGED + KEY_D:
		demo_start_playback("test.fsd");
		break;
	}
#else 
	}
#endif

	// do any processing based upon what happened to the snazzy menu
	switch (snazzy_action) {
	case SNAZZY_OVER:
		main_hall_handle_mouse_location(code);
		break;

	case SNAZZY_CLICKED:
		switch (code) {
		// clicked on the exit region
		case EXIT_REGION:
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			main_hall_exit_game();
			break;

		// clicked on the readyroom region
		case READY_ROOM_REGION:
			if (Campaign_file_missing) {
				// error popup for a missing campaign file, don't try to enter ready room in this case
				popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "The currently active campaign cannot be found.\n\n Please select another in the Campaign Room.", -1));
				break;
			} else if ( !(Player->flags & PLAYER_FLAGS_IS_MULTI) && !strlen(Campaign.filename) ) {
				// no campaign loaded...
				popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "No active campaign is available.  Please choose one in the Campaign Room.", -1));
				break;
			}

#ifdef MULTIPLAYER_BETA_BUILD
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			Player->flags |= PLAYER_FLAGS_IS_MULTI;
			main_hall_do_multi_ready();
#elif defined(E3_BUILD) || defined(PRESS_TOUR_BUILD)									
			gameseq_post_event(GS_EVENT_NEW_CAMPAIGN);			
#else

			if (Player->flags & PLAYER_FLAGS_IS_MULTI){
				gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
				main_hall_do_multi_ready();
			} else {				
				if(strlen(Main_hall_campaign_cheat)){
					gameseq_post_event(GS_EVENT_CAMPAIGN_CHEAT);
				} else {
					gameseq_post_event(GS_EVENT_NEW_CAMPAIGN);				
				}
				gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);				
			}
#endif
			break;

		// clicked on the tech room region
		case TECH_ROOM_REGION:
#if defined(FS2_DEMO)
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			game_feature_not_in_demo_popup();
#else
		//	if (Campaign_file_missing) {
		//		// error popup for a missing campaign file, don't try to enter tech room in this case
		//		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "The currently active campaign cannot be found.  Please select another in the Campaign Room.", -1));
		//		break;
		//	}
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			gameseq_post_event( GS_EVENT_TECH_MENU );
#endif
			break;

		// clicked on the options region
		case OPTIONS_REGION:
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			gameseq_post_event(GS_EVENT_OPTIONS_MENU);
			break;

		// clicked on the campaign toom region
		case CAMPAIGN_ROOM_REGION:
#if !defined(MULTIPLAYER_BETA_BUILD) && !defined(E3_BUILD) && !defined(PRESS_TOUR_BUILD)

#ifdef FS2_DEMO
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			{
			//game_feature_not_in_demo_popup();
			int reset_campaign = popup(PF_USE_AFFIRMATIVE_ICON|PF_BODY_BIG, 2, "Exit", "Restart Campaign", "Campaign Room only available in full version. However, you may restart the campaign.");
			if (reset_campaign == 1) {
				mission_campaign_savefile_delete(Campaign.filename);
				mission_campaign_load(Campaign.filename);
				mission_campaign_next_mission();
			}
			}

#else
			if(Player->flags & PLAYER_FLAGS_IS_MULTI){
				gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
				main_hall_set_notify_string(XSTR( "Campaign Room not valid for multiplayer pilots", 366));
			} else {
				gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
				gameseq_post_event(GS_EVENT_CAMPAIGN_ROOM);			
			}
#endif

#endif
			break;

		// clicked on the multiplayer region
		case MULTIPLAYER_REGION:
#if defined(DEMO) || defined(OEM_BUILD) // not for FS2_DEMO
			game_feature_not_in_demo_popup();
#else
			if (Player->flags & PLAYER_FLAGS_IS_MULTI){
				// NOTE : this isn't a great thing to be calling this anymore. But we'll leave it for now
				gameseq_post_event( GS_EVENT_MULTI_JOIN_GAME );
			} else {
				main_hall_set_notify_string(XSTR( "Not a valid multiplayer pilot!!", 367));
			}
#endif
			break;

		// load mission key was pressed
		case LOAD_MISSION_REGION:
#ifdef RELEASE_REAL
#else
	#if !(defined(MULTIPLAYER_BETA_BUILD) || defined(FS2_DEMO))
	//#if !defined(NDEBUG) || defined(INTERPLAYQA)
				if (Player->flags & PLAYER_FLAGS_IS_MULTI){
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					main_hall_set_notify_string(XSTR( "Load Mission not valid for multiplayer pilots", 368));
				} else {
	#ifdef GAME_CD_CHECK
					// if ( !game_do_cd_check() ) {
						// break;
					// }
	#endif
					gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
					gameseq_post_event( GS_EVENT_LOAD_MISSION_MENU );
				}
	//#endif
	#endif
#endif
			break;

		// quick start a game region
		case QUICK_START_REGION:
#if !defined(NDEBUG) && !defined(FS2_DEMO)
			if (Player->flags & PLAYER_FLAGS_IS_MULTI){
				main_hall_set_notify_string(XSTR( "Quick Start not valid for multiplayer pilots", 369));
			} else {

				if (Num_recent_missions > 0)	{
					strncpy( Game_current_mission_filename, Recent_missions[0], MAX_FILENAME_LEN );
				} else {
					if ( mission_load_up_campaign() )
						main_hall_set_notify_string(XSTR( "Campaign file is currently unavailable", -1));

					strncpy( Game_current_mission_filename, Campaign.missions[0].name, MAX_FILENAME_LEN );
				}

				Campaign.current_mission = -1;
				gameseq_post_event(GS_EVENT_START_GAME_QUICK);
			}
#endif
			break;

		// clicked on the barracks region
		case BARRACKS_REGION:			
		//	if (Campaign_file_missing) {
		//		// error popup for a missing campaign file, don't try to enter barracks in this case
		//		popup( PF_NO_NETWORKING, 1, POPUP_OK, XSTR( "The currently active campaign cannot be found.  Please select another in the Campaign Room.", -1));
		//		break;
		//	}
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			gameseq_post_event( GS_EVENT_BARRACKS_MENU );
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
			if(!help_overlay_active(Main_hall_overlay_id)){
				gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
				main_hall_exit_game();
			}
			// kill the overlay
			else {
				help_overlay_set_state(Main_hall_overlay_id,0);
			}
			break;
		}

		// if the escape key wasn't pressed handle any mouse position related events
		if (code != ESC_PRESSED){
			main_hall_handle_mouse_location(code);
		}
		break;

		default:
			main_hall_handle_mouse_location(-1);
			break;
	}

	if ( mouse_down(MOUSE_LEFT_BUTTON) ) {
		help_overlay_set_state(Main_hall_overlay_id, 0);
	}

	// draw the background bitmap	
	gr_reset_clip();	
	GR_MAYBE_CLEAR_RES(Main_hall_bitmap);
	if(Main_hall_bitmap >= 0){
		gr_set_bitmap(Main_hall_bitmap);
		gr_bitmap(0, 0);
	}

	// draw any pending notification messages
	main_hall_notify_do();			

	// render misc animations
	main_hall_render_misc_anims(frametime);

	// render door animtions
	main_hall_render_door_anims(frametime);	

	// blit any appropriate tooltips
	main_hall_maybe_blit_tooltips();

	// fishtank
	fishtank_process();

	// process any help "hit f1" timestamps and display any messages if necessary
	if (!F1_text_done) {
		main_hall_process_help_stuff();
	}

	// blit help overlay if active
	help_overlay_maybe_blit(Main_hall_overlay_id);

	// blit the freespace version #
	main_hall_blit_version();

	// blit ship and weapon table status
#ifndef NDEBUG
	main_hall_blit_table_status();
#endif

	gr_flip();

	// see if we have a missing campaign and force the player to select a new campaign if so
	extern bool Campaign_room_no_campaigns;
	if ( !(Player->flags & PLAYER_FLAGS_IS_MULTI) && Campaign_file_missing && !Campaign_room_no_campaigns ) {
		int rc = popup(0, 3, XSTR("Go to Campaign Room", -1), XSTR("Select another pilot", -1), XSTR("Exit Game", -1), XSTR("The currently active campaign cannot be found.  Please select another...", -1));

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
// #if defined(FS2_DEMO) && defined(NDEBUG)
	player_tips_popup();
// #endif

	// if we were supposed to skip a frame, then stop doing it after 1 frame
	if(Main_hall_frame_skip){
		Main_hall_frame_skip = 0;
	}
}

// close the main hall proper
void main_hall_close()
{
	int idx,s_idx;

	if(!Main_hall_inited){
		return;
	}	

	// unload the main hall bitmap
	if(Main_hall_bitmap != -1){
		bm_release(Main_hall_bitmap);
	}

	// unload any bitmaps
	if(Main_hall_mask >= 0){		
		// make sure we unlock the mask bitmap so it can be unloaded
		bm_unlock(Main_hall_mask);
		bm_release(Main_hall_mask);
	}

	// free up any (possibly) playing misc animation handles
	for(idx=0;idx<Main_hall->num_misc_animations;idx++){
		if(Main_hall_misc_anim[idx].num_frames > 0){
			generic_anim_unload(&Main_hall_misc_anim[idx]);
		}
	}
	
	// free up any (possibly) playing door animation handles
	for(idx=0;idx<Main_hall->num_door_animations;idx++){
		if(Main_hall_door_anim[idx].num_frames > 0){
			generic_anim_unload(&Main_hall_door_anim[idx]);
		}		
	}	

	// stop any playing door sounds
	for(idx=0;idx<Main_hall->num_door_sounds-2;idx++){	// don't cut off the glow sounds (requested by Dan)
		if((Main_hall_door_sound_handles[idx] != -1) && snd_is_playing(Main_hall_door_sound_handles[idx])){
			snd_stop(Main_hall_door_sound_handles[idx]);
			Main_hall_door_sound_handles[idx] = -1;
		}
	}	

	// stop any playing misc animation sounds
	for(idx=0;idx<Main_hall->num_misc_animations;idx++){
		for(s_idx=1;s_idx<10;s_idx++){
			if(snd_is_playing(Main_hall->misc_anim_sound_handles[idx][s_idx])){
				snd_stop(Main_hall->misc_anim_sound_handles[idx][s_idx]);
				Main_hall->misc_anim_sound_handles[idx][s_idx] = -1;
			}
		}
	}

	// unload the overlay bitmap
	help_overlay_unload(Main_hall_overlay_id);

	// close any snazzy menu details
	snazzy_menu_close();

	// restore
	palette_restore_palette();

	// no fish
	fishtank_stop();	

	// unpause
	Main_hall_paused = 0;

	// not inited anymore
	Main_hall_inited = 0;
}

int main_hall_get_music_index(int main_hall_num)
{
	int index;
	main_hall_defines *hall;

	if (main_hall_num < 0)
		return -1;

	hall = &Main_hall_defines[gr_screen.res][main_hall_num];

	// Goober5000 - try substitute first
	index = event_music_get_spooled_music_index(hall->substitute_music_name);
	if ((index >= 0) && (Spooled_music[index].flags & SMF_VALID))
		return index;

	// now try regular
	index = event_music_get_spooled_music_index(hall->music_name);
	if ((index >= 0) && (Spooled_music[index].flags & SMF_VALID))
		return index;

	return -1;
}

// start the main hall music playing
void main_hall_start_music()
{
	int index;
	char *filename;

	// start a looping ambient sound
	main_hall_start_ambient();

	// if we have selected no music, then don't do this
	if (Cmdline_freespace_no_music)
		return;

	// already playing?
	if (Main_hall_music_handle >= 0)
		return;

	// get music
	index = main_hall_get_music_index(Main_hall-Main_hall_defines[gr_screen.res]);
	if (index < 0)
	{
		nprintf(("Warning", "No music file exists to play music at the main menu!\n"));
		return;
	}

	filename = Spooled_music[index].filename;
	Assert(filename != NULL);

	// get handle
	Main_hall_music_handle = audiostream_open(filename, ASF_MENUMUSIC);
	if (Main_hall_music_handle < 0)
	{
		nprintf(("Warning", "No music file exists to play music at the main menu!\n"));
		return;
	}

	audiostream_play(Main_hall_music_handle, Master_event_music_volume, 1);
}

// stop the main hall music
void main_hall_stop_music()
{
	if ( Main_hall_music_handle != -1 ) {
		audiostream_close_file(Main_hall_music_handle, 1);
		Main_hall_music_handle = -1;
	}
}

// render all playing misc animations
void main_hall_render_misc_anims(float frametime)
{
	int idx,s_idx;
/*
	// HACKETY HACK HACK - always render misc anim 3 first, if it is playing
	if(Main_hall_misc_anim_instance[2] != NULL){
		anim_render_one(GS_STATE_MAIN_MENU,Main_hall_misc_anim_instance[2],frametime);
	}
*/
	// render all other animations
	for(idx=0;idx<MAX_MISC_ANIMATIONS;idx++){
/*
		// skip anim 3, which was previously rendered, if at all
		if(idx == 2){
			continue;
		}
*/
		// render it
		if(Main_hall_misc_anim[idx].num_frames > 0){
			//animation is paused
			if(Main_hall_misc_anim[idx].direction & GENERIC_ANIM_DIRECTION_PAUSED) {
			// if the timestamp is -1, then reset it to some random value (based on MIN and MAX) and continue
			if(Main_hall->misc_anim_delay[idx][0] == -1){
				Main_hall->misc_anim_delay[idx][0] = timestamp(Main_hall->misc_anim_delay[idx][1] + 
					 									      (int)(((float)rand()/(float)RAND_MAX) * (float)(Main_hall->misc_anim_delay[idx][2] - Main_hall->misc_anim_delay[idx][1])));

			// if the timestamp is not -1 and has popped, play the anim and make the timestap -1
				} else if (timestamp_elapsed(Main_hall->misc_anim_delay[idx][0])) {
					Main_hall_misc_anim[idx].direction &= ~GENERIC_ANIM_DIRECTION_PAUSED;
					Main_hall_misc_anim[idx].current_frame = 0;
					Main_hall_misc_anim[idx].anim_time = 0.0;
				
				// kill the timestamp	
				Main_hall->misc_anim_delay[idx][0] = -1;				

				// reset the "should be playing" flags
				for(s_idx=1;s_idx<10;s_idx++){
					Main_hall->misc_anim_sound_flag[idx][s_idx] = 0;
				}
			}
		} 		
		else {
			for(s_idx=Main_hall->misc_anim_special_sounds[idx][0]; s_idx > 0; s_idx--){
				// if we've passed the trigger point, then play the sound and break out of the loop
					if((Main_hall_misc_anim[idx].current_frame >= Main_hall->misc_anim_special_trigger[idx][s_idx]) && !Main_hall->misc_anim_sound_flag[idx][s_idx]){
					Main_hall->misc_anim_sound_flag[idx][s_idx] = 1;

					// if the sound is already playing, then kill it. This is a pretty safe thing to do since we can assume that
					// by the time we get to this point again, the sound will have been long finished
					if(snd_is_playing(Main_hall->misc_anim_sound_handles[idx][s_idx])){
						snd_stop(Main_hall->misc_anim_sound_handles[idx][s_idx]);
						Main_hall->misc_anim_sound_handles[idx][s_idx] = -1;
					}
					// play the sound
					Main_hall->misc_anim_sound_handles[idx][s_idx] = snd_play(&Snds_iface[Main_hall->misc_anim_special_sounds[idx][s_idx]],Main_hall->misc_anim_sound_pan[idx]);					
					break;
				}
			}
				if(Main_hall_misc_anim[idx].current_frame == Main_hall_misc_anim[idx].num_frames - 1) {
				Main_hall->misc_anim_delay[idx][0] = -1;				

					//this helps the above code reset the timers
					//MISC_ANIM_MODE_HOLD simply stops on the last frame, so we don't care
					//MISC_ANIM_MODE_LOOPED just loops so we don't care either
					if(Main_hall->misc_anim_modes[idx] == MISC_ANIM_MODE_TIMED) {
						Main_hall_misc_anim[idx].direction |= GENERIC_ANIM_DIRECTION_PAUSED;
					}
					//don't reset sound for MISC_ANIM_MODE_HOLD
					if(Main_hall->misc_anim_modes[idx] != MISC_ANIM_MODE_HOLD) {
				// reset the "should be playing" flags
				for(s_idx=1;s_idx<10;s_idx++){
					Main_hall->misc_anim_sound_flag[idx][s_idx] = 0;
				}
			}			

			}
		}			

			if(Main_hall_frame_skip || Main_hall_paused)
				frametime = 0;
			generic_anim_render(&Main_hall_misc_anim[idx], frametime, Main_hall->misc_anim_coords[idx][0], Main_hall->misc_anim_coords[idx][1]);
		}
	}
}

// render all playing door animations
void main_hall_render_door_anims(float frametime)
{
	int idx;	

	// render all door animations
	for(idx=0;idx<MAX_DOOR_ANIMATIONS;idx++){  
		if(Main_hall_door_anim[idx].num_frames > 0){
		// first pair : coords of where to play a given door anim
		// second pair : center of a given door anim in windowed mode
			generic_anim_render(&Main_hall_door_anim[idx], frametime, Main_hall->door_anim_coords[idx][0], Main_hall->door_anim_coords[idx][1]);
		}		
	}	
}

// do any necessary processing based upon the mouse location
void main_hall_handle_mouse_location(int cur_region)
{
   if(Main_hall_frame_skip)
		return;

	if(cur_region > NUM_MAIN_HALL_MOUSE_REGIONS) {
		// MWA -- inserted return since Int3() was tripped when hitting L from main
		// menu.
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
			if ((Main_hall_mouse_region != -1) && (cur_region != Main_hall_mouse_region)) {
				main_hall_mouse_release_region(Main_hall_mouse_region);
			}
		
			// set the linger time
			if (Main_hall_region_linger_stamp == -1) {
				Main_hall_mouse_region = cur_region;
				Main_hall_region_linger_stamp = timestamp(MAIN_HALL_REGION_LINGER);
			}			
		}
	}
	// if it was over a region but isn't anymore, release that region
	else {
		if (Main_hall_mouse_region != -1) {
			main_hall_mouse_release_region(Main_hall_mouse_region);
			Main_hall_mouse_region = -1;

			// release the region linger timestamp
			Main_hall_region_linger_stamp = -1;
		}
	}
}

// if the mouse has moved off of the currently active region, handle the anim accordingly
void main_hall_mouse_release_region(int region)
{
	if(Main_hall_frame_skip){
		return;
	}

	//run backwards and stop at the first frame
	Main_hall_door_anim[region].direction = GENERIC_ANIM_DIRECTION_BACKWARDS | GENERIC_ANIM_DIRECTION_NOLOOP;

	// check for door sounds, ignoring the OPTIONS_REGION (which isn't a door)
	if ((Main_hall_door_anim[region].num_frames > 0)) {
		// don't stop the toaster oven or microwave regions from playing all the way through
		if (Main_hall_door_sound_handles[region] != -1) {
			snd_stop(Main_hall_door_sound_handles[region]);
		}
		Main_hall_door_sound_handles[region] = snd_play(&Snds_iface[Main_hall->door_sounds[region][1]], Main_hall->door_sound_pan[region]);

		//TODO: track current frame
		snd_set_pos(Main_hall_door_sound_handles[region], &Snds_iface[SND_MAIN_HALL_DOOR_CLOSE],
						(float)((Main_hall_door_anim[region].keyframe) ? Main_hall_door_anim[region].keyframe : Main_hall_door_anim[region].num_frames - Main_hall_door_anim[region].current_frame) / (float)Main_hall_door_anim[region].num_frames, 1);
	}
}

// if the mouse has moved on this region, handle it accordingly
void main_hall_mouse_grab_region(int region)
{
	if (Main_hall_frame_skip) {
		return;
	}

	//run forwards
	Main_hall_door_anim[region].direction = GENERIC_ANIM_DIRECTION_FORWARDS;
	//stay on last frame if we have no keyframe
	if(!Main_hall_door_anim[region].keyframe)
		Main_hall_door_anim[region].direction += GENERIC_ANIM_DIRECTION_NOLOOP;

	// check for opening/starting sounds
	// kill the currently playing sounds if necessary
	if(Main_hall_door_sound_handles[region] != -1){			
		snd_stop(Main_hall_door_sound_handles[region]);
	}	
	Main_hall_door_sound_handles[region] = snd_play(&Snds_iface[Main_hall->door_sounds[region][0]],Main_hall->door_sound_pan[region]);				

	// start the sound playing at the right spot relative to the completion of the animation		
	if( (Main_hall_door_anim[region].num_frames > 0) && (Main_hall_door_anim[region].current_frame != -1) ) {
			snd_set_pos(Main_hall_door_sound_handles[region],&Snds_iface[SND_MAIN_HALL_DOOR_OPEN],
							(float)Main_hall_door_anim[region].current_frame / (float)Main_hall_door_anim[region].num_frames,1);
	}				
}

// handle any right clicks which may have occured
void main_hall_handle_right_clicks()
{
	int new_region;

	if(Main_hall_frame_skip)
		return;

	// check to see if the button has been clicked
	if(!Main_hall_right_click){
		if(mouse_down(MOUSE_RIGHT_BUTTON)){
			// cycle through the available regions
			if(Main_hall_last_clicked_region == NUM_MAIN_HALL_MOUSE_REGIONS - 1){
				new_region = 0;
			} else
				new_region = Main_hall_last_clicked_region + 1;

			// set the position of the mouse cursor and the newly clicked region			
			int mx = Main_hall->door_anim_coords[new_region][2];
			int my = Main_hall->door_anim_coords[new_region][3];
			gr_resize_screen_pos( &mx, &my );
			mouse_set_pos( mx, my );			

			main_hall_handle_mouse_location(new_region);
			Main_hall_last_clicked_region = new_region;
			
			// set the mouse as being clicked
			Main_hall_right_click = 1;
		}
	} 
	// set the mouse as being unclicked
	else if(Main_hall_right_click && !(mouse_down(MOUSE_RIGHT_BUTTON))){
		Main_hall_right_click = 0;
	}
}

// cull any door sounds that have finished playing
void main_hall_cull_door_sounds()
{
	int idx;
	// basically just set the handle of any finished sound to be -1, so that we know its free any where else in the code we may need it
	for(idx=0;idx<Main_hall->num_door_sounds;idx++){
		if((Main_hall_door_sound_handles[idx] != -1) && !snd_is_playing(Main_hall_door_sound_handles[idx])){			
			Main_hall_door_sound_handles[idx] = -1;
		}
	}
}

void main_hall_handle_random_intercom_sounds()
{
	// if we have no timestamp for the next random sound, then set on
	if((Main_hall_next_intercom_sound_stamp == -1) && (Main_hall_intercom_sound_handle == -1)){
		Main_hall_next_intercom_sound_stamp = timestamp((int)(((float)rand()/(float)RAND_MAX) * 
			                                            (float)(Main_hall->intercom_delay[Main_hall_next_intercom_sound][1]
																	  - Main_hall->intercom_delay[Main_hall_intercom_sound_handle][0])) );		
	}

	// if the there is no sound playing
	if(Main_hall_intercom_sound_handle == -1){
		if (Main_hall_paused) {
			return;
		}

		// if the timestamp has popped, play a sound
		if((Main_hall_next_intercom_sound_stamp != -1) && (timestamp_elapsed(Main_hall_next_intercom_sound_stamp))){
			// play the sound
			Main_hall_intercom_sound_handle = snd_play(&Snds_iface[Main_hall->intercom_sounds[Main_hall_next_intercom_sound]]);			
			
			// unset the timestamp
			Main_hall_next_intercom_sound_stamp = -1;
		}
	}
	// if the sound is playing
	else {
		// if the sound has finished, set the timestamp and continue
		if(!snd_is_playing(Main_hall_intercom_sound_handle)){
			// increment the next sound
			if(Main_hall_next_intercom_sound >= (Main_hall->num_random_intercom_sounds-1)){
				Main_hall_next_intercom_sound = 0;
			} else {
				Main_hall_next_intercom_sound++;
			}

			// set the timestamp
			Main_hall_next_intercom_sound_stamp = timestamp((int)(((float)rand()/(float)RAND_MAX) * 
			                                            (float)(Main_hall->intercom_delay[Main_hall_next_intercom_sound][1]
																	  - Main_hall->intercom_delay[Main_hall_next_intercom_sound][0])) );

			// release the sound handle
			Main_hall_intercom_sound_handle = -1;
		}
	}
}

// set the notification string with its decay timeout
void main_hall_set_notify_string(char *str)
{
	strcpy_s(Main_hall_notify_text,str);
	Main_hall_notify_stamp = timestamp(MAIN_HALL_NOTIFY_TIME);
}

void main_hall_notify_do()
{
	// check to see if we should try and do something
	if(Main_hall_notify_stamp != -1){
	   // if the text time has expired
		if(timestamp_elapsed(Main_hall_notify_stamp)){
			strcpy_s(Main_hall_notify_text,"");
			Main_hall_notify_stamp = -1;
		} else {
			int w,h;
			gr_set_color_fast(&Color_bright);

			gr_get_string_size(&w,&h,Main_hall_notify_text);
			gr_printf((gr_screen.max_w - w)/2, gr_screen.max_h - 40, Main_hall_notify_text);
		}
	}
}

// start a looping ambient sound for main hall
void main_hall_start_ambient()
{
	int play_ambient_loop = 0;

	if (Main_hall_paused)
		return;

	if ( Main_hall_ambient_loop == -1 ) {
		play_ambient_loop = 1;
	} else {
		if ( !snd_is_playing(Main_hall_ambient_loop) ) {
			play_ambient_loop = 1;
		}
	}

	if ( play_ambient_loop ) {
		Main_hall_ambient_loop = snd_play_looping(&Snds_iface[SND_MAIN_HALL_AMBIENT]);
	}
}

// stop a looping ambient sound for the main hall
void main_hall_stop_ambient()
{
	if ( Main_hall_ambient_loop != -1 ) {
		snd_stop(Main_hall_ambient_loop);
		Main_hall_ambient_loop = -1;
	}
}

// Reset the volume of the looping ambient sound.  This is called from the options 
// screen when the looping ambient sound might be playing.
void main_hall_reset_ambient_vol()
{
	if ( Main_hall_ambient_loop >= 0 ) {
		snd_set_volume(Main_hall_ambient_loop, Snds_iface[SND_MAIN_HALL_AMBIENT].default_volume);
	}
}

// blit the freespace version #
void main_hall_blit_version()
{
	char version_string[100];
	int w;

	// format the version string
	get_version_string(version_string, sizeof(version_string));

	// get the length of the string
	gr_get_string_size(&w,NULL,version_string);

	// print the string near the lower left corner
	gr_set_color_fast(&Color_bright_white);
	gr_string(5, gr_screen.max_h_unscaled - 24, version_string);
}

// blit any necessary tooltips
void main_hall_maybe_blit_tooltips()
{
	int w;
	int text_index;

	// if we're over no region - don't blit anything
	if(Main_hall_mouse_region < 0) {
		return;
	}

	// get the index of the proper text to be using
	if(Main_hall_mouse_region == READY_ROOM_REGION) {
		// if this is a multiplayer pilot, the ready room region becomes the multiplayer region
		if(Player->flags & PLAYER_FLAGS_IS_MULTI){
			text_index = NUM_REGIONS - 1;
		} else {
			text_index = READY_ROOM_REGION;
		}
	} else {
		text_index = Main_hall_mouse_region;
	}

	// set the color and blit the string
	if(!help_overlay_active(Main_hall_overlay_id)) {
		int shader_y = (Main_hall->region_yval) - Main_hall_tooltip_padding[gr_screen.res];	// subtract more to pull higher
		// get the width of the string
		gr_get_string_size(&w, NULL, Main_hall->region_descript[text_index]);

		gr_set_shader(&Main_hall_tooltip_shader);
		gr_shade(0, shader_y, gr_screen.clip_width_unscaled, (gr_screen.clip_height_unscaled - shader_y));

		gr_set_color_fast(&Color_bright_white);
		gr_string((gr_screen.max_w_unscaled - w)/2, Main_hall->region_yval, Main_hall->region_descript[text_index]);
	}
}


void main_hall_process_help_stuff()
{
	int w, h;
	char str[255];
	
	// if the timestamp has popped, don't do anything
	if(Main_hall_help_stamp == -1) {
		return;
	}

	// if the timestamp has popped, advance frame
	if(timestamp_elapsed(Main_hall_help_stamp)) {
		Main_hall_f1_text_frame++;
	}

	// otherwise print out the message
	strcpy_s(str, XSTR( "Press F1 for help", 371));
	gr_get_string_size(&w, &h, str);

	int y_anim_offset = Main_hall_f1_text_frame;

	// if anim is off the screen finally, stop altogether
	if ( (y_anim_offset >= (2*Main_hall_tooltip_padding[gr_screen.res]) + h) || (help_overlay_active(Main_hall_overlay_id)) ) {
		Main_hall_f1_text_frame = -1;
		Main_hall_help_stamp = -1;
		F1_text_done = 1;
		return;
	}

	// set the color and print out text and shader
	gr_set_color_fast(&Color_bright_white);
	gr_shade(0, 0, gr_screen.max_w_unscaled, (2*Main_hall_tooltip_padding[gr_screen.res]) + h - y_anim_offset);
	gr_string((gr_screen.max_w_unscaled - w)/2, Main_hall_tooltip_padding[gr_screen.res] /*- y_anim_offset*/, str);
}

// what main hall we're on
int main_hall_id()
{
	return (Main_hall - &Main_hall_defines[gr_screen.res][0]);
} 

// read in main hall table
void main_hall_read_table()
{
	main_hall_defines *m, temp;
	int count, idx, s_idx, m_idx, rval;

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", "mainhall.tbl", rval));
		return;
	}

	// read the file in
	read_file_text("mainhall.tbl", CF_TYPE_TABLES);
	reset_parse();

	// go for it
	count = 0;
	while(!optional_string("#end"))
	{
		// read in 2 resolutions
		for(m_idx=0; m_idx<GR_NUM_RESOLUTIONS; m_idx++)
		{
			// maybe use a temp main hall stuct
			if(count >= MAIN_HALLS_MAX)
			{
				m = &temp;
				Warning(LOCATION, "Number of main halls in mainhall.tbl has exceeded max of %d. All further main halls will be ignored.", MAIN_HALLS_MAX);
				//WMC - break, because there's nothing after this loop to parse
				break;
			} else {
				m = &Main_hall_defines[m_idx][count];
			}

			// ready
			required_string("$Main Hall");

			// bitmap and mask
			required_string("+Bitmap:");
			stuff_string(m->bitmap, F_NAME, MAX_FILENAME_LEN);
			required_string("+Mask:");
			stuff_string(m->mask, F_NAME, MAX_FILENAME_LEN);

#ifndef FS2_DEMO
			required_string("+Music:");
			stuff_string(m->music_name, F_NAME, MAX_FILENAME_LEN);

			// Goober5000
			if (optional_string("+Substitute Music:"))
				stuff_string(m->substitute_music_name, F_NAME, MAX_FILENAME_LEN);
#endif

			// intercom sounds
			required_string("+Num Intercom Sounds:");
			stuff_int(&m->num_random_intercom_sounds);		
			for(idx=0; idx<m->num_random_intercom_sounds; idx++){			
				// intercom delay
				required_string("+Intercom delay:");
				stuff_int(&m->intercom_delay[idx][0]);
				stuff_int(&m->intercom_delay[idx][1]);
			}
			for(idx=0; idx<m->num_random_intercom_sounds; idx++){			
				// intercom sound id
				required_string("+Intercom sound:");
				stuff_int(&m->intercom_sounds[idx]);			
			}			
			for(idx=0; idx<m->num_random_intercom_sounds; idx++){			
				// intercom pan
				required_string("+Intercom pan:");
				stuff_float(&m->intercom_sound_pan[idx]);			
			}			

			// misc animations
			required_string("+Num Misc Animations:");
			stuff_int(&m->num_misc_animations);
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim names
				required_string("+Misc anim:");
				stuff_string(m->misc_anim_name[idx], F_NAME, MAX_FILENAME_LEN);
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim delay
				required_string("+Misc anim delay:");
				stuff_int(&m->misc_anim_delay[idx][0]);
				stuff_int(&m->misc_anim_delay[idx][1]);
				stuff_int(&m->misc_anim_delay[idx][2]);
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim coords
				required_string("+Misc anim coords:");
				stuff_int(&m->misc_anim_coords[idx][0]);
				stuff_int(&m->misc_anim_coords[idx][1]);
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim mode
				required_string("+Misc anim mode:");
				stuff_int(&m->misc_anim_modes[idx]);			
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim pan
				required_string("+Misc anim pan:");
				stuff_float(&m->misc_anim_sound_pan[idx]);
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim sound id
				required_string("+Misc anim sounds:");
				stuff_int(&m->misc_anim_special_sounds[idx][0]);
				for(s_idx=0; s_idx<m->misc_anim_special_sounds[idx][0]; s_idx++){
					stuff_int(&m->misc_anim_special_sounds[idx][s_idx + 1]);
				}
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim sound triggers
				required_string("+Misc anim trigger:");
				stuff_int(&m->misc_anim_special_trigger[idx][0]);
				for(s_idx=0; s_idx<m->misc_anim_special_trigger[idx][0]; s_idx++){
					stuff_int(&m->misc_anim_special_trigger[idx][s_idx + 1]);
				}
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim sound handles
				required_string("+Misc anim handles:");
				stuff_int(&m->misc_anim_sound_handles[idx][0]);			
			}
			for(idx=0; idx<m->num_misc_animations; idx++){
				// anim sound flags
				required_string("+Misc anim flags:");
				stuff_int(&m->misc_anim_sound_flag[idx][0]);			
			}

			// door animations
			required_string("+Num Door Animations:");
			stuff_int(&m->num_door_animations);
			for(idx=0; idx<m->num_door_animations; idx++){
				// door name
				required_string("+Door anim:");
				stuff_string(m->door_anim_name[idx], F_NAME, MAX_FILENAME_LEN);
			}
			for(idx=0; idx<m->num_door_animations; idx++){
				// door coords
				required_string("+Door coords:");
				stuff_int(&m->door_anim_coords[idx][0]);
				stuff_int(&m->door_anim_coords[idx][1]);
				stuff_int(&m->door_anim_coords[idx][2]);
				stuff_int(&m->door_anim_coords[idx][3]);
			}
			for(idx=0; idx<m->num_door_animations; idx++){
				// door open and close sounds
				required_string("+Door sounds:");
				stuff_int(&m->door_sounds[idx][0]);
				stuff_int(&m->door_sounds[idx][1]);			
			}
			for(idx=0; idx<m->num_door_animations; idx++){
				// door pan value
				required_string("+Door pan:");
				stuff_float(&m->door_sound_pan[idx]);			
			}

			// tooltip y location
			required_string("+Tooltip Y:");
			stuff_int(&m->region_yval);
			for(idx=0; idx<NUM_REGIONS; idx++){
				m->region_descript[idx] = NULL;
			}
		}

		if(count < MAIN_HALLS_MAX){
			count++;
		}
	}

	Num_main_halls = count;

	// are we funny?
	if(Vasudan_funny){
		int hall = main_hall_id();

		Main_hall_defines[GR_640][hall].door_sounds[OPTIONS_REGION][0] = SND_VASUDAN_BUP;
		Main_hall_defines[GR_640][hall].door_sounds[OPTIONS_REGION][1] = SND_VASUDAN_BUP;
		Main_hall_defines[GR_1024][hall].door_sounds[OPTIONS_REGION][0] = SND_VASUDAN_BUP;
		Main_hall_defines[GR_1024][hall].door_sounds[OPTIONS_REGION][1] = SND_VASUDAN_BUP;

		// set head anim. hehe
		strcpy_s(Main_hall_defines[GR_640][hall].door_anim_name[OPTIONS_REGION], "vhallheads");
		strcpy_s(Main_hall_defines[GR_1024][hall].door_anim_name[OPTIONS_REGION], "2_vhallheads");

		// set the background
		strcpy_s(Main_hall_defines[GR_640][hall].bitmap, "vhallhead");
		strcpy_s(Main_hall_defines[GR_1024][hall].bitmap, "2_vhallhead");		
	}

	// free up memory from parsing the mainhall tbl
	stop_parse();
}

// make the vasudan main hall funny
void main_hall_vasudan_funny()
{
	Vasudan_funny = 1;
}

int main_hall_is_vasudan()
{
	// kind of a hack for now
	return (!stricmp(Main_hall->music_name, "Psampik") || !stricmp(Main_hall->music_name, "Psamtik"));
}

// silence sounds on mainhall if we hit a pause mode (ie. lost window focus, minimized, etc);
void main_hall_pause()
{
	if (Main_hall_paused)
		return;

	Main_hall_paused = 1;

	audiostream_pause(Main_hall_music_handle);

	main_hall_stop_ambient();
}

// recover from a paused state (ie. lost window focus, minimized, etc);
void main_hall_unpause()
{
	if (!Main_hall_paused)
		return;

	Main_hall_paused = 0;

	audiostream_unpause(Main_hall_music_handle);

	main_hall_start_ambient();
}
