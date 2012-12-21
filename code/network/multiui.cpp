/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifdef _WIN32
#include <winsock.h>	// for inet_addr()
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include "network/multiui.h"
#include "freespace2/freespace.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "io/key.h"
#include "io/timer.h"
#include "gamesequence/gamesequence.h"
#include "gamehelp/contexthelp.h"
#include "playerman/player.h"
#include "network/multi_xfer.h"
#include "cmdline/cmdline.h"
#include "network/stand_gui.h"
#include "network/multiteamselect.h"
#include "mission/missioncampaign.h"
#include "graphics/font.h"
#include "io/mouse.h"
#include "gamesnd/gamesnd.h"
#include "missionui/chatbox.h"
#include "popup/popup.h"
#include "missionui/missiondebrief.h"
#include "network/multi_ingame.h"
#include "network/multi_kick.h"
#include "network/multi_data.h"
#include "network/multi_campaign.h"
#include "network/multi_team.h"
#include "network/multi_pinfo.h"
#include "network/multi_observer.h"
#include "network/multi_voice.h"
#include "network/multi_endgame.h"
#include "playerman/managepilot.h"
#include "stats/stats.h"
#include "network/multi_pmsg.h"
#include "network/multi_obj.h"
#include "network/multi_log.h"
#include "globalincs/alphacolors.h"
#include "anim/animplay.h"
#include "network/multi_dogfight.h"
#include "missionui/missionpause.h"
#include "ship/ship.h"
#include "osapi/osregistry.h"
#include "mission/missionbriefcommon.h"
#include "parse/parselo.h"
#include "cfile/cfile.h"
#include "fs2netd/fs2netd_client.h"
#include "menuui/mainhallmenu.h"

#include <algorithm>


// -------------------------------------------------------------------------------------------------------------
// 
// MULTIPLAYER COMMON interface controls
//

// the common text info box stuff. This is lifted almost directly from Alans briefing code (minus the spiffy colored, scrolling
// text crap :)
int Multi_common_text_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		29, 396, 393, 76
	},
	{ // GR_1024
		47, 634, 630, 122
	}
};

int Multi_common_text_max_display[GR_NUM_RESOLUTIONS] = {
	8,		// GR_640
	12,	// GR_1024
};

#define MULTI_COMMON_TEXT_META_CHAR				'$'
#define MULTI_COMMON_TEXT_MAX_LINE_LENGTH		200
#define MULTI_COMMON_TEXT_MAX_LINES				20
#define MULTI_COMMON_MAX_TEXT						(MULTI_COMMON_TEXT_MAX_LINES * MULTI_COMMON_TEXT_MAX_LINE_LENGTH)

char Multi_common_all_text[MULTI_COMMON_MAX_TEXT];
char Multi_common_text[MULTI_COMMON_TEXT_MAX_LINES][MULTI_COMMON_TEXT_MAX_LINE_LENGTH];

int Multi_common_top_text_line = -1;		// where to start displaying from
int Multi_common_num_text_lines = 0;		// how many lines we have

void multi_common_scroll_text_up();
void multi_common_scroll_text_down();
void multi_common_move_to_bottom();
void multi_common_render_text();
void multi_common_split_text();

#define MAX_IP_STRING		255				// maximum length for ip string

void multi_common_scroll_text_up()
{
	Multi_common_top_text_line--;
	if ( Multi_common_top_text_line < 0 ) {
		Multi_common_top_text_line = 0;
		if ( !mouse_down(MOUSE_LEFT_BUTTON) )
			gamesnd_play_iface(SND_GENERAL_FAIL);

	} else {
		gamesnd_play_iface(SND_SCROLL);
	}
}

void multi_common_scroll_text_down()
{
	Multi_common_top_text_line++;
	if ( (Multi_common_num_text_lines - Multi_common_top_text_line) < Multi_common_text_max_display[gr_screen.res] ) {
		Multi_common_top_text_line--;
		if ( !mouse_down(MOUSE_LEFT_BUTTON) ){
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
	} else {
		gamesnd_play_iface(SND_SCROLL);
	}
}

void multi_common_move_to_bottom()
{
	// if there's nowhere to scroll down, do nothing
	if(Multi_common_num_text_lines <= Multi_common_text_max_display[gr_screen.res]){
		return;
	}
		
	Multi_common_top_text_line = Multi_common_num_text_lines - Multi_common_text_max_display[gr_screen.res];
}

void multi_common_set_text(char *str,int auto_scroll)
{
	// make sure it fits
	// store the entire string as well
	if(strlen(str) >= MULTI_COMMON_MAX_TEXT){
		return ;
	} else {
		strcpy_s(Multi_common_all_text,str);
	}
	
	// split the whole thing up
	multi_common_split_text();

	// scroll to the bottom if we're supposed to
	if(auto_scroll){
		multi_common_move_to_bottom();
	}
}

void multi_common_add_text(char *str,int auto_scroll)
{
	// make sure it fits
	// store the entire string as well
	if((strlen(str) + strlen(Multi_common_all_text)) >= MULTI_COMMON_MAX_TEXT){
		return ;
	} else {
		strcat_s(Multi_common_all_text,str);
	}
	
	// split the whole thing up
	multi_common_split_text();

	// scroll to the bottom if we're supposed to
	if(auto_scroll){
		multi_common_move_to_bottom();
	}
}

void multi_common_split_text()
{
	int	n_lines, i;
	int	n_chars[MAX_BRIEF_LINES];
	char	*p_str[MAX_BRIEF_LINES];

	n_lines = split_str(Multi_common_all_text, Multi_common_text_coords[gr_screen.res][2], n_chars, p_str, MULTI_COMMON_TEXT_MAX_LINES, MULTI_COMMON_TEXT_META_CHAR);
	Assert(n_lines != -1);

	for ( i = 0; i < n_lines; i++ ) {
		//The E -- This check is unnecessary, and will break when fonts that aren't bank gothic are used
		//split_str already ensured that everything will fit in the text window for us already.
		//Assert(n_chars[i] < MULTI_COMMON_TEXT_MAX_LINE_LENGTH); 
		strncpy(Multi_common_text[i], p_str[i], n_chars[i]);
		Multi_common_text[i][n_chars[i]] = 0;
		drop_leading_white_space(Multi_common_text[i]);		
	}

	Multi_common_top_text_line = 0;
	Multi_common_num_text_lines = n_lines;	
}

void multi_common_render_text()
{
	int i, fh, line_count;

	fh = gr_get_font_height();
	
	line_count = 0;
	gr_set_color_fast(&Color_text_normal);
	for ( i = Multi_common_top_text_line; i < Multi_common_num_text_lines; i++ ) {
		if ( line_count >= Multi_common_text_max_display[gr_screen.res] ){
			break;	
		}
		gr_string(Multi_common_text_coords[gr_screen.res][0], Multi_common_text_coords[gr_screen.res][1] + (line_count*fh), Multi_common_text[i]);		
		line_count++;
	}

	if ( (Multi_common_num_text_lines - Multi_common_top_text_line) > Multi_common_text_max_display[gr_screen.res] ) {
		gr_set_color_fast(&Color_bright_red);
		gr_string(Multi_common_text_coords[gr_screen.res][0], (Multi_common_text_coords[gr_screen.res][1] + Multi_common_text_coords[gr_screen.res][3])-5, XSTR("more",755));
	}
}

// common notification messaging stuff
#define MULTI_COMMON_NOTIFY_TIME		3500
int Multi_common_join_y[GR_NUM_RESOLUTIONS] = {
	375,		// GR_640
	605		// GR_1024
};
int Multi_common_create_y[GR_NUM_RESOLUTIONS] = {
	380,		// GR_640
	610		// GR_1024
};

int Multi_common_jw_y[GR_NUM_RESOLUTIONS] = {
	380,		// GR_640
	610		// GR_1024
};

int Multi_common_msg_y[GR_NUM_RESOLUTIONS] = {
	380,		// GR_640
	610		// GR_1024
};

char Multi_common_notify_text[200];
int Multi_common_notify_stamp;

void multi_common_notify_init()
{
	strcpy_s(Multi_common_notify_text,"");
	Multi_common_notify_stamp = -1;
}

// add a notification string, drawing appropriately depending on the state/screen we're in
void multi_common_add_notify(char *str)
{
	if(str){
		strcpy_s(Multi_common_notify_text,str);
		Multi_common_notify_stamp = timestamp(MULTI_COMMON_NOTIFY_TIME);
	}
}

// process/display notification messages
void multi_common_notify_do()
{
	if(Multi_common_notify_stamp != -1){
		if(timestamp_elapsed(Multi_common_notify_stamp)){
			Multi_common_notify_stamp = -1;
		} else {
			int w,h,y;
			gr_get_string_size(&w,&h,Multi_common_notify_text);
			gr_set_color_fast(&Color_white);
			
			// determine where it should be placed based upon which screen we're on
			y = -1;
			switch(gameseq_get_state()){
			case GS_STATE_MULTI_JOIN_GAME :
				y = Multi_common_join_y[gr_screen.res];
				break;
			case GS_STATE_MULTI_HOST_SETUP :
				y = Multi_common_create_y[gr_screen.res];
				break;
			case GS_STATE_MULTI_CLIENT_SETUP :
				y = Multi_common_jw_y[gr_screen.res];
				break;
			case GS_STATE_MULTI_START_GAME :
				y = Multi_common_msg_y[gr_screen.res];
				break;			
			}
			if(y != -1){
				gr_string((gr_screen.max_w_unscaled - w)/2, y, Multi_common_notify_text);
			}
		}
	}
}

// common icon stuff
int Multi_common_icons[MULTI_NUM_COMMON_ICONS];
//XSTR:OFF
char *Multi_common_icon_names[MULTI_NUM_COMMON_ICONS] = {
	"DotRed",				// voice denied
	"DotGreen",				// voice recording
	"OvalGreen",			// team 0
	"OvalGreen01",			// team 0 select
	"OvalRed",				// team 1
	"OvalRed01",			// team 1 select
	"mp_coop",				// coop mission
	"mp_teams",				// TvT mission
	"mp_furball",			// furball mission
	"icon-volition",		// volition mission	
	"icon-valid",			// mission is valid
	"cd"						// cd icon
};
//XSTR:ON
// width and height of the icons
int Multi_common_icon_dims[MULTI_NUM_COMMON_ICONS][2] = {
	{11, 11},				// voice denied
	{11, 11},				// voice recording
	{11, 11},				// team 0
	{11, 11},				// team 0 select
	{11, 11},				// team 1
	{11, 11},				// team 1 select
	{18, 11},				// mp coop
	{18, 11},				// mp TvT
	{18, 11},				// mp furball
	{9, 9},					// volition mission	
	{8, 8},					// mission is valid
	{8, 8}					// cd icon
};

void multi_load_common_icons()
{
	int idx;

	if (Is_standalone)
		return;

	// load all icons
	for(idx=0; idx<MULTI_NUM_COMMON_ICONS; idx++){
		Multi_common_icons[idx] = -1;
		Multi_common_icons[idx] = bm_load(Multi_common_icon_names[idx]);
	}
}

void multi_unload_common_icons()
{
	int idx;

	if (Is_standalone)
		return;

	// unload all icons
	for(idx=0; idx<MULTI_NUM_COMMON_ICONS; idx++){
		if(Multi_common_icons[idx] != -1){
			// don't bm_release() here, used in multiple places - taylor
			bm_unload(Multi_common_icons[idx]);
			Multi_common_icons[idx] = -1;
		}
	}
}

// display any relevant voice status icons
void multi_common_voice_display_status()
{	
	switch(multi_voice_status()){
	// i have been denied the voice token
	case MULTI_VOICE_STATUS_DENIED:
		if(Multi_common_icons[MICON_VOICE_DENIED] != -1){
			gr_set_bitmap(Multi_common_icons[MICON_VOICE_DENIED]);
			gr_bitmap(0,0);
		}
		break;

	// i am currently recording
	case MULTI_VOICE_STATUS_RECORDING:
		if(Multi_common_icons[MICON_VOICE_RECORDING] != -1){
			gr_set_bitmap(Multi_common_icons[MICON_VOICE_RECORDING]);
			gr_bitmap(0,0);
		}
		break;

	// i am currently playing back sound
	case MULTI_VOICE_STATUS_PLAYING:
		break;

	// the system is currently idle
	case MULTI_VOICE_STATUS_IDLE:
		break;
	}
}

//XSTR:OFF
// palette initialization stuff
#define MULTI_COMMON_PALETTE_FNAME			"InterfacePalette"
//XSTR:ON

int Multi_common_interface_palette = -1;

void multi_common_load_palette();
void multi_common_set_palette();
void multi_common_unload_palette();

// load in the palette if it doesn't already exist
void multi_common_load_palette()
{
	if(Multi_common_interface_palette != -1){
		return;
	}

	Multi_common_interface_palette = bm_load(MULTI_COMMON_PALETTE_FNAME);
	if(Multi_common_interface_palette == -1){
		nprintf(("Network","Error loading multiplayer common palette!\n"));
	}
}

// set the common palette to be the active one
void multi_common_set_palette()
{
	// if the palette is not loaded yet, do so now
	if(Multi_common_interface_palette == -1){
		multi_common_load_palette();
	}
	
	if(Multi_common_interface_palette != -1){
#ifndef HARDWARE_ONLY
		palette_use_bm_palette(Multi_common_interface_palette);
#endif
	}
}

// unload the bitmap palette
void multi_common_unload_palette()
{
	if(Multi_common_interface_palette != -1){
		bm_unload(Multi_common_interface_palette);
		Multi_common_interface_palette = -1;
	}
}

void multi_common_verify_cd()
{
#ifdef GAME_CD_CHECK
	// otherwise, call the freespace function to determine if we have a cd
	Multi_has_cd = 0;
	if((find_freespace_cd(FS_CDROM_VOLUME_1) >= 0) || (find_freespace_cd(FS_CDROM_VOLUME_2) >= 0) || (find_freespace_cd(FS_CDROM_VOLUME_3) >= 0) ){
		Multi_has_cd = 1;
	} 
#else
	Multi_has_cd = 1;
#endif
}


// -------------------------------------------------------------------------------------------------------------
// 
// MULTIPLAYER JOIN SCREEN 
//

#define MULTI_JOIN_NUM_BUTTONS	11

//XSTR:OFF
// bitmaps defs
#define MULTI_JOIN_PALETTE				"InterfacePalette"

static char *Multi_join_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MultiJoin",		// GR_640
	"2_MultiJoin"			// GR_1024
};

static char *Multi_join_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MultiJoin-M",		// GR_640
	"2_MultiJoin-M"		// GR_1024
};
//XSTR:ON

// slider
char *Mj_slider_name[GR_NUM_RESOLUTIONS] = {
	"slider",
	"2_slider"
};
int Mj_slider_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		2, 91, 15, 202
	},
	{ // GR_1024
		8, 147, 30, 322
	}
};

// button defs
#define MJ_SCROLL_UP				0
#define MJ_SCROLL_DOWN			1
#define MJ_REFRESH				2
#define MJ_SCROLL_INFO_UP		3
#define MJ_SCROLL_INFO_DOWN	4
#define MJ_JOIN_OBSERVER		5
#define MJ_START_GAME			6
#define MJ_CANCEL					7
#define MJ_HELP					8
#define MJ_OPTIONS				9
#define MJ_ACCEPT					10

// uses MULTI_JOIN_REFRESH_TIME as its timestamp
int Multi_join_glr_stamp;

#define MULTI_JOIN_PING_TIME     15000        // how often we ping all the known servers
int Multi_join_ping_stamp;
UI_WINDOW Multi_join_window;											// the window object for the join screen
UI_BUTTON Multi_join_select_button;									// for selecting list items
UI_SLIDER2 Multi_join_slider;											// handy dandy slider
int Multi_join_bitmap;													// the background bitmap

ui_button_info Multi_join_buttons[GR_NUM_RESOLUTIONS][MULTI_JOIN_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info( "MJ_00",	1,		57,	-1,	-1,	0 ),						// scroll up
		ui_button_info( "MJ_02",	1,		297,	-1,	-1,	2 ),						// scroll down
		ui_button_info( "MJ_03",	10,	338,	65,	364,	3 ),						// refresh
		ui_button_info( "MJ_04",	1,		405,	-1,	-1,	4 ),						// scroll info up
		ui_button_info( "MJ_05",	1,		446,	-1,	-1,	5 ),						// scroll info down
		ui_button_info( "MJ_06",	489,	339,	-1,	-1,	6 ),						// join as observer
		ui_button_info( "MJ_07",	538,	339,	-1,	-1,	7 ),						// create game
		ui_button_info( "MJ_08",	583,	339,	588,	376,	8 ),						// cancel
		ui_button_info( "MJ_09",	534,	426,	-1,	-1,	9 ),						// help
		ui_button_info( "MJ_10",	534,	454,	-1,	-1,	10 ),						// options
		ui_button_info( "MJ_11",	571,	426,	589,	416,	11 ),						// join
	},
	{ // GR_1024
		ui_button_info( "2_MJ_00",	2,		92,	-1,	-1,	0 ),						// scroll up
		ui_button_info( "2_MJ_02",	2,		475,	-1,	-1,	2 ),						// scroll down
		ui_button_info( "2_MJ_03",	16,	541,	104,	582,	3 ),						// refresh
		ui_button_info( "2_MJ_04",	2,		648,	-1,	-1,	4 ),						// scroll info up
		ui_button_info( "2_MJ_05",	2,		713,	-1,	-1,	5 ),						// scroll info down
		ui_button_info( "2_MJ_06",	783,	542,	-1,	-1,	6 ),						// join as observer
		ui_button_info( "2_MJ_07",	861,	542,	-1,	-1,	7 ),						// create game
		ui_button_info( "2_MJ_08",	933,	542,	588,	376,	8 ),						// cancel
		ui_button_info( "2_MJ_09",	854,	681,	-1,	-1,	9 ),						// help
		ui_button_info( "2_MJ_10",	854,	727,	-1,	-1,	10 ),						// options
		ui_button_info( "2_MJ_11",	914,	681,	937,	668,	11 ),						// join
	}
};

#define MULTI_JOIN_NUM_TEXT			13

UI_XSTR Multi_join_text[GR_NUM_RESOLUTIONS][MULTI_JOIN_NUM_TEXT] = {
	{ // GR_640		
		{"Refresh",							1299, 65,	364,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[0][MJ_REFRESH].button},
		{"Join as",							1300,	476,	376,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[0][MJ_JOIN_OBSERVER].button},
		{"Observer",						1301,	467,	385,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[0][MJ_JOIN_OBSERVER].button},
		{"Create",							1408,	535,	376,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[0][MJ_START_GAME].button},	
		{"Game",								1302,	541,	385,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[0][MJ_START_GAME].button},
		{"Cancel",							387,	588,	376,	UI_XSTR_COLOR_PINK, -1, &Multi_join_buttons[0][MJ_CANCEL].button},	
		{"Help",								928,	479,	436,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[0][MJ_HELP].button},
		{"Options",							1036,	479,	460,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[0][MJ_OPTIONS].button},
		{"Join",								1303,	589,	416,	UI_XSTR_COLOR_PINK, -1, &Multi_join_buttons[0][MJ_ACCEPT].button},
		{"Status",							1304,	37,	37,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"Server",							1305,	116,	37,	UI_XSTR_COLOR_GREEN, -1, NULL},
		{"Players",							1306,	471,	37,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"Ping",								1307,	555,	37,	UI_XSTR_COLOR_GREEN, -1, NULL}
	},
	{ // GR_1024		
		{"Refresh",							1299, 104,	582,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[1][MJ_REFRESH].button},
		{"Join as",							1300,	783,	602,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[1][MJ_JOIN_OBSERVER].button},
		{"Observer",						1301,	774,	611,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[1][MJ_JOIN_OBSERVER].button},		
		{"Create",							1408,	868,	602,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[1][MJ_START_GAME].button},
		{"Game",								1302,	872,	611,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[1][MJ_START_GAME].button},
		{"Cancel",							387,	941,	602,	UI_XSTR_COLOR_PINK, -1, &Multi_join_buttons[1][MJ_CANCEL].button},
		{"Help",								928,	782,	699,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[1][MJ_HELP].button},
		{"Options",							1036,	782,	736,	UI_XSTR_COLOR_GREEN, -1, &Multi_join_buttons[1][MJ_OPTIONS].button},
		{"Join",								1303,	937,	668,	UI_XSTR_COLOR_PINK, -1, &Multi_join_buttons[1][MJ_ACCEPT].button},
		{"Status",							1304,	60,	60,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"Server",							1305,	186,	60,	UI_XSTR_COLOR_GREEN, -1, NULL},
		{"Players",							1306,	753,	60,	UI_XSTR_COLOR_GREEN,	-1, NULL},
		{"Ping",								1307,	888,	60,	UI_XSTR_COLOR_GREEN, -1, NULL}
	}
};

// constants for coordinate look ups
#define MJ_X_COORD 0
#define MJ_Y_COORD 1
#define MJ_W_COORD 2
#define MJ_H_COORD 3

#define MULTI_JOIN_SENT_WAIT		10000					// wait this long since a join was sent to allow another
int Multi_join_sent_stamp;

// game information text areas
int Mj_max_game_items[GR_NUM_RESOLUTIONS] = {
	28,			// GR_640
	46				// GR_1024
};

int Mj_list_y[GR_NUM_RESOLUTIONS] = {
	53,			// GR_640
	77				// GR_1024
};

int Mj_status_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		34, 53, 55, 287
	},
	{ // GR_1024
		53, 77, 57, 459
	}
};

int Mj_game_icon_coords[GR_NUM_RESOLUTIONS][3] = {
	{ // GR_640
		98, 53, 28
	},
	{ // GR_1024
		124, 77, 28
	}
};	

int Mj_speed_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		122, 53, 56, 261
	},
	{ // GR_1024
		152, 77, 56, 261
	}
};	

int Mj_game_name_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		186, 53, 280, 261
	},
	{ // GR_1024
		206, 77, 311, 261
	}
};	

int Mj_players_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		473, 53, 50, 261
	},
	{ // GR_1024
		748, 77, 67, 459
	}
};	

int Mj_ping_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		551, 53, 47, 261
	},
	{ // GR_1024
		880, 77, 48, 459
	}
};	

// game speed labels
#define MJ_NUM_SPEED_LABELS		5
char *Multi_join_speed_labels[MJ_NUM_SPEED_LABELS] = {
	"< 56k",
	"56k",
	"isdn",
	"cable",
	"t1/adsl+"
};
color *Multi_join_speed_colors[MJ_NUM_SPEED_LABELS] = {
	&Color_bright_red,
	&Color_bright_red,
	&Color_bright_green,
	&Color_bright_green,
	&Color_bright_green
};

int Mj_cd_coords[GR_NUM_RESOLUTIONS] = {
	20, 30
};

//XSTR:OFF
#define IP_CONFIG_FNAME				"tcp.cfg"		// name of the file which contains known TCP addresses

// extents of the entire boundable game info region
// NOTE : these numbers are completely empirical
#define MJ_PING_GREEN				160
#define MJ_PING_YELLOW				300
#define MJ_PING_RED					700
#define MJ_PING_ONE_SECOND			1000

int Mj_list_area_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		23, 53, 565, 279 
	},
	{ // GR_1024
		53, 76, 887, 461 
	}
};	

// PXO channel filter
#define MJ_PXO_FILTER_Y				0

// special chars to indicate various status modes for servers
#define MJ_CHAR_STANDALONE			"*"
#define MJ_CHAR_CAMPAIGN			"c"
//XSTR:ON

// various interface indices
int Multi_join_list_start;							// where to start displaying from
active_game *Multi_join_list_start_item;		// a pointer to the corresponding active_game
int Multi_join_list_selected;						// which item we have selected
active_game *Multi_join_selected_item;			// a pointer to the corresponding active_game

// use this macro to modify the list start
#define MJ_LIST_START_INC()			do { Multi_join_list_start++; } while(0);
#define MJ_LIST_START_DEC()			do { Multi_join_list_start--; } while(0);
#define MJ_LIST_START_SET(vl)			do { Multi_join_list_start = vl; } while(0);

// if we should be sending a join request at the end of the frame
int Multi_join_should_send = -1;

// master tracker details
int Multi_join_frame_count;						// keep a count of frames displayed
int Multi_join_mt_tried_verify;					// already tried verifying the pilot with the tracker

// data stuff for auto joining a game
#define MULTI_AUTOJOIN_JOIN_STAMP		2000
#define MULTI_AUTOJOIN_QUERY_STAMP		2000

int Multi_did_autojoin;
net_addr Multi_autojoin_addr;
int Multi_autojoin_join_stamp;
int Multi_autojoin_query_stamp;

// our join request
join_request Multi_join_request;

// LOCAL function definitions
void multi_join_check_buttons();
void multi_join_button_pressed(int n);
void multi_join_display_games();
void multi_join_blit_game_status(active_game *game, int y);
void multi_join_load_tcp_addrs();
void multi_join_do_netstuff();
void multi_join_ping_all();
void multi_join_process_select();
void multi_join_list_scroll_up();
void multi_join_list_scroll_down();
void multi_join_list_page_up();
void multi_join_list_page_down();
active_game *multi_join_get_game(int n);
void multi_join_cull_timeouts();
void multi_join_handle_item_cull(active_game *item, int item_index);
void multi_join_send_join_request(int as_observer);
void multi_join_create_game();
void multi_join_blit_top_stuff();
int multi_join_maybe_warn();
int multi_join_warn_pxo();
void multi_join_blit_protocol();

DCF(mj_make, "")
{
	active_game ag, *newitem;
	int idx;

	dc_get_arg(ARG_INT);
	for(idx=0; idx<Dc_arg_int; idx++){
		// stuff some fake info
		memset(&ag, 0, sizeof(active_game));
		sprintf(ag.name, "Game %d", idx);
		ag.version = MULTI_FS_SERVER_VERSION;
		ag.comp_version = MULTI_FS_SERVER_VERSION;
		ag.server_addr.addr[0] = (char)idx;
		ag.flags = (AG_FLAG_COOP | AG_FLAG_FORMING | AG_FLAG_STANDALONE);		

		// add the game
		newitem = multi_update_active_games(&ag);

		// timestamp it so we get random timeouts
		if(newitem != NULL){
			// newitem->heard_from_timer = timestamp((int)frand_range(500.0f, 10000.0f));
		}
	}	
}

void multi_join_notify_new_game()
{	
	// reset the # of items	
	Multi_join_slider.set_numberItems(Active_game_count > Mj_max_game_items[gr_screen.res] ? Active_game_count - Mj_max_game_items[gr_screen.res] : 0);
	Multi_join_slider.force_currentItem(Multi_join_list_start);
}

int multi_join_autojoin_do()
{
	// if we have an active game on the list, then return a positive value so that we
	// can join the game
	if ( Active_game_head && (Active_game_count > 0) ) {
		Multi_join_selected_item = Active_game_head;
		return 1;
	}

	// send out a server_query again
	if ( timestamp_elapsed(Multi_autojoin_query_stamp) ) {
		send_server_query(&Multi_autojoin_addr);
		Multi_autojoin_query_stamp = timestamp(MULTI_AUTOJOIN_QUERY_STAMP);
	}

	return -1;
}

void multi_join_game_init()
{
	int idx;

	// do the multiplayer init stuff - multi_level_init() now does all net_player zeroing.
	// setup various multiplayer things
	Assert( Game_mode & GM_MULTIPLAYER );
	Assert( Net_player != NULL );

	switch (Multi_options_g.protocol) {	
	case NET_IPX:
		ADDRESS_LENGTH = IPX_ADDRESS_LENGTH;
		PORT_LENGTH = IPX_PORT_LENGTH;
		break;

	case NET_TCP:
		ADDRESS_LENGTH = IP_ADDRESS_LENGTH;		
		PORT_LENGTH = IP_PORT_LENGTH;			
		break;

	default :
		Int3();
	} // end switch
	
	HEADER_LENGTH = 1;

	memset( &Netgame, 0, sizeof(Netgame) );

	multi_level_init();		
	Net_player->flags |= NETINFO_FLAG_DO_NETWORKING;	
	Net_player->m_player = Player;
	memcpy(&Net_player->p_info.addr,&Psnet_my_addr,sizeof(net_addr));

	// check for the existence of a CD
	multi_common_verify_cd();

	// load my local netplayer options
	multi_options_local_load(&Net_player->p_info.options, Net_player);	

	game_flush();

	// destroy any chatbox contents which previously existed (from another game)
	chatbox_clear();

	// create the interface window
	Multi_join_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_join_window.set_mask_bmap(Multi_join_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_join_bitmap = bm_load(Multi_join_bitmap_fname[gr_screen.res]);
	if(Multi_join_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}

	// intialize the endgame system
	multi_endgame_init();	

	// initialize the common notification messaging
	multi_common_notify_init();

	// initialize the common text area
	multi_common_set_text("");	

	// load and use the common interface palette
	multi_common_load_palette();
	multi_common_set_palette();

	// load the help overlay
	help_overlay_load(MULTI_JOIN_OVERLAY);
	help_overlay_set_state(MULTI_JOIN_OVERLAY,0);
	
	// try to login to the tracker
	if (MULTI_IS_TRACKER_GAME) {
		if ( !fs2netd_login() ) {
			// failed!  go back to the main hall
			gameseq_post_event(GS_EVENT_MAIN_MENU);
			return;
		}
	}

	// do TCP and VMT specific initialization
	if ( (Multi_options_g.protocol == NET_TCP) && !MULTI_IS_TRACKER_GAME ) {		
		// if this is a TCP (non tracker) game, we'll load up our default address list right now		
		multi_join_load_tcp_addrs();		
	}	

	// initialize any and all timestamps	
	Multi_join_glr_stamp = -1;
	Multi_join_ping_stamp = -1;
	Multi_join_sent_stamp = -1;

	// reset frame count
	Multi_join_frame_count = 0;

	// haven't tried to verify on the tracker yet.
	Multi_join_mt_tried_verify = 0;

	// clear our all game lists to save hassles
	multi_join_clear_game_list();

	// create the interface buttons
	for(idx=0; idx<MULTI_JOIN_NUM_BUTTONS; idx++){
		// create the object
		Multi_join_buttons[gr_screen.res][idx].button.create(&Multi_join_window,"", Multi_join_buttons[gr_screen.res][idx].x, Multi_join_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_join_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_join_buttons[gr_screen.res][idx].button.set_bmaps(Multi_join_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Multi_join_buttons[gr_screen.res][idx].button.link_hotspot(Multi_join_buttons[gr_screen.res][idx].hotspot);
	}		

	// create all xstrs
	for(idx=0; idx<MULTI_JOIN_NUM_TEXT; idx++){
		Multi_join_window.add_XSTR(&Multi_join_text[gr_screen.res][idx]);
	}

	Multi_join_should_send = -1;

	// close any previously open chatbox
	chatbox_close();

	// create the list item select button
	Multi_join_select_button.create(&Multi_join_window, "", Mj_list_area_coords[gr_screen.res][MJ_X_COORD], Mj_list_area_coords[gr_screen.res][MJ_Y_COORD], Mj_list_area_coords[gr_screen.res][MJ_W_COORD], Mj_list_area_coords[gr_screen.res][MJ_H_COORD], 0, 1);
	Multi_join_select_button.hide();

	// slider
	Multi_join_slider.create(&Multi_join_window, Mj_slider_coords[gr_screen.res][MJ_X_COORD], Mj_slider_coords[gr_screen.res][MJ_Y_COORD], Mj_slider_coords[gr_screen.res][MJ_W_COORD], Mj_slider_coords[gr_screen.res][MJ_H_COORD], 0, Mj_slider_name[gr_screen.res], &multi_join_list_scroll_up, &multi_join_list_scroll_down, NULL);

	// make sure that we turn music/sounds back on (will be disabled after playing a mission)
	main_hall_start_music();

	// if starting a network game, then go to the create game screen
	if ( Cmdline_start_netgame ) {
		multi_join_create_game();		
	} else if ( Cmdline_connect_addr != NULL ) {
		char *p;
		short port_num;
		int ip_addr;

		// joining a game.  Send a join request to the given IP address, and wait for the return.
		memset( &Multi_autojoin_addr, 0, sizeof(net_addr) );
		Multi_autojoin_addr.type = NET_TCP;

		// create the address, looking out for port number at the end
		port_num = DEFAULT_GAME_PORT;
		p = strrchr(Cmdline_connect_addr, ':');
		if ( p ) {
			*p = '\0';
			p++;
			port_num = (short)atoi(p);
		}
		ip_addr = inet_addr(Cmdline_connect_addr);
		memcpy(Multi_autojoin_addr.addr, &ip_addr, 4);
		Multi_autojoin_addr.port = port_num;

		send_server_query(&Multi_autojoin_addr);
		Multi_autojoin_query_stamp = timestamp(MULTI_AUTOJOIN_QUERY_STAMP);
		Multi_did_autojoin = 0;
	}
}

void multi_join_clear_game_list()
{
	// misc data	
	Multi_join_list_selected = -1;	
	Multi_join_selected_item = NULL;	
	MJ_LIST_START_SET(-1);
	Multi_join_list_start_item = NULL;	

	// free up the active game list
	multi_free_active_games();

	// initialize the active game list
	Active_game_head = NULL;
	Active_game_count = 0;
}

void multi_join_game_do_frame()
{
	// check the status of our reliable socket.  If not valid, popup error and return to main menu
	// I put this code here to avoid nasty gameseq issues with states.  Also, we will have nice
	// background for the popup
	if ( !psnet_rel_check() ) {
		popup(PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("Network Error.  Try exiting and restarting FreeSpace to clear the error.  Otherwise, please reboot your machine.",756));
		gameseq_post_event(GS_EVENT_MAIN_MENU);
		return;
	}	

	// return here since we will be moving to the next stage anyway -- I don't want to see the backgrounds of
	// all the screens for < 1 second for every screen we automatically move to.
	if ( Cmdline_start_netgame ) {
		return;
	}

	// when joining a network game, wait for the server query to come back, and then join the game
	if ( Cmdline_connect_addr != NULL ) {
		int rval;

		if ( !Multi_did_autojoin ) {
			rval = popup_till_condition(multi_join_autojoin_do, XSTR("&Cancel", 779), XSTR("Joining netgame", 1500) );
			if ( rval == 0 ) {
				// cancel was hit.  Send the user back to the main hall
				gameseq_post_event(GS_EVENT_MAIN_MENU);
				Cmdline_connect_addr = NULL;		// reset this value.
			}

			// when we get here, we have the data -- join the game.
			multi_join_send_join_request(0);
			Multi_autojoin_join_stamp = timestamp(MULTI_AUTOJOIN_JOIN_STAMP);
			Multi_did_autojoin = 1;
		}

		if ( timestamp_elapsed(Multi_autojoin_join_stamp) ) {
			multi_join_send_join_request(0);
			Multi_autojoin_join_stamp = timestamp(MULTI_AUTOJOIN_JOIN_STAMP);
		}
		return;

	}	

	// reset the should send var
	Multi_join_should_send = -1;

	int k = Multi_join_window.process();

	// process any keypresses
	switch(k){
	case KEY_ESC :
		if(help_overlay_active(MULTI_JOIN_OVERLAY)){
			help_overlay_set_state(MULTI_JOIN_OVERLAY,0);
		} else {		
			if (MULTI_IS_TRACKER_GAME) {
				gameseq_post_event(GS_EVENT_PXO);
			} else {
				gameseq_post_event(GS_EVENT_MAIN_MENU);
			}
			gamesnd_play_iface(SND_USER_SELECT);
		}
		break;

	// page up the game list
	case KEY_PAGEUP:
		multi_join_list_page_up();	
		Multi_join_slider.force_currentItem(Multi_join_list_start);
		break;

	case KEY_T:
		multi_pinfo_popup(Net_player);
		break;

	// page down the game list
	case KEY_PAGEDOWN:
		multi_join_list_page_down();
		Multi_join_slider.force_currentItem(Multi_join_list_start);
		break;

	// send out a ping-all
	case KEY_P :		
		multi_join_ping_all();		
		Multi_join_ping_stamp = timestamp(MULTI_JOIN_PING_TIME);
		break;	

	// shortcut to start a game	
	case KEY_S :		
		multi_join_create_game();		
		break;

	// scroll the game list up
	case KEY_UP:
		multi_join_list_scroll_up();
		Multi_join_slider.force_currentItem(Multi_join_list_start);
		break;

	// scroll the game list down
	case KEY_DOWN:
		multi_join_list_scroll_down();
		Multi_join_slider.force_currentItem(Multi_join_list_start);
		break;
	}	

	if ( mouse_down(MOUSE_LEFT_BUTTON) ) {
		help_overlay_set_state(MULTI_JOIN_OVERLAY, 0);
	}

	// do any network related stuff
	multi_join_do_netstuff(); 

	// process any button clicks
	multi_join_check_buttons();

	// process any list selection stuff
	multi_join_process_select();

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Multi_join_bitmap);
	if(Multi_join_bitmap != -1){		
		gr_set_bitmap(Multi_join_bitmap);
		gr_bitmap(0,0);
	}
	Multi_join_window.draw();

	// display the active games
	multi_join_display_games();

	// display any text in the info area
	multi_common_render_text();

	// display any pending notification messages
	multi_common_notify_do();

	// blit the CD icon and any PXO filter stuff
	multi_join_blit_top_stuff();

	// draw the help overlay
	help_overlay_maybe_blit(MULTI_JOIN_OVERLAY);
	
	// flip the buffer
	gr_flip();

	// if we are supposed to be sending a join request
	if(Multi_join_should_send != -1){		
		multi_join_send_join_request(Multi_join_should_send);
	}
	Multi_join_should_send = -1;

	// increment the frame count
	Multi_join_frame_count++;
}

void multi_join_game_close()
{
	// unload any bitmaps
	if(!bm_unload(Multi_join_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_join_bitmap_fname[gr_screen.res]));
	}

	// unload the help overlay
	help_overlay_unload(MULTI_JOIN_OVERLAY);	

	// free up the active game list
	multi_free_active_games();
	
	// destroy the UI_WINDOW
	Multi_join_window.destroy();
}

void multi_join_check_buttons()
{
	int idx;
	for(idx=0;idx<MULTI_JOIN_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Multi_join_buttons[gr_screen.res][idx].button.pressed()){
			multi_join_button_pressed(idx);
			break;
		}
	}
}

void multi_join_button_pressed(int n)
{

	switch(n){
	case MJ_CANCEL :
		// if we're player PXO, go back there
		if (MULTI_IS_TRACKER_GAME) {
			gameseq_post_event(GS_EVENT_PXO);
		} else {
			gameseq_post_event(GS_EVENT_MAIN_MENU);
		}
		gamesnd_play_iface(SND_USER_SELECT);		
		break;
	case MJ_ACCEPT :
		if(Active_game_count <= 0){
			multi_common_add_notify(XSTR("No games found!",757));
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else if(Multi_join_list_selected == -1){
			multi_common_add_notify(XSTR("No game selected!",758));
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else if((Multi_join_sent_stamp != -1) && !timestamp_elapsed(Multi_join_sent_stamp)){
			multi_common_add_notify(XSTR("Still waiting on previous join request!",759));
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {			
			// otherwise, if he's already played PXO games, warn him	
			
			if(Player->flags & PLAYER_FLAGS_HAS_PLAYED_PXO){
				if(!multi_join_warn_pxo()){
					break;
				}			
			}
			

			// send the join request here
			Assert(Multi_join_selected_item != NULL);

			// send a join request packet
			Multi_join_should_send = 0;			
			
			gamesnd_play_iface(SND_COMMIT_PRESSED);
		}
		break;

	// help overlay
	case MJ_HELP:
		if(!help_overlay_active(MULTI_JOIN_OVERLAY)){
			help_overlay_set_state(MULTI_JOIN_OVERLAY,1);
		} else {
			help_overlay_set_state(MULTI_JOIN_OVERLAY,0);
		}
		break;

	// scroll the game list up
	case MJ_SCROLL_UP:
		multi_join_list_scroll_up();
		Multi_join_slider.force_currentItem(Multi_join_list_start);
		break;

	// scroll the game list down
	case MJ_SCROLL_DOWN:
		multi_join_list_scroll_down();
		Multi_join_slider.force_currentItem(Multi_join_list_start);
		break;

	// scroll the info text box up
	case MJ_SCROLL_INFO_UP:
		multi_common_scroll_text_up();
		break;

	// scroll the info text box down
	case MJ_SCROLL_INFO_DOWN:
		multi_common_scroll_text_down();
		break;

	// go to the options screen
	case MJ_OPTIONS:
		gameseq_post_event(GS_EVENT_OPTIONS_MENU);
		break;

	// go to the start game screen	
	case MJ_START_GAME:
		multi_join_create_game();		
		break;

	// refresh the game/server list
	case MJ_REFRESH:	
		gamesnd_play_iface(SND_USER_SELECT);
		broadcast_game_query();
		break;

	// join a game as an observer
	case MJ_JOIN_OBSERVER:
		if(Active_game_count <= 0){
			multi_common_add_notify(XSTR("No games found!",757));
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else if(Multi_join_list_selected == -1){
			multi_common_add_notify(XSTR("No game selected!",758));
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else if((Multi_join_sent_stamp != -1) && !timestamp_elapsed(Multi_join_sent_stamp)){
			multi_common_add_notify(XSTR("Still waiting on previous join request!",759));
			gamesnd_play_iface(SND_GENERAL_FAIL);
		} else {			
			// send the join request here
			Assert(Multi_join_selected_item != NULL);

			Multi_join_should_send = 1;		

			gamesnd_play_iface(SND_COMMIT_PRESSED);
		}
		break;

	default :
		multi_common_add_notify(XSTR("Not implemented yet!",760));
		gamesnd_play_iface(SND_GENERAL_FAIL);
		break;
	}
}

// display all relevant info for active games
void multi_join_display_games()
{
	active_game *moveup = Multi_join_list_start_item;	
	char str[200];		
	int w,h;
	int con_type;
	int y_start = Mj_list_y[gr_screen.res];
	int count = 0;
	
	if(moveup != NULL){
		do {			
			// blit the game status (including text and type icon)
			multi_join_blit_game_status(moveup,y_start);			
			
			// get the connection type
			con_type = (moveup->flags & AG_FLAG_CONNECTION_SPEED_MASK) >> AG_FLAG_CONNECTION_BIT;
			if((con_type > 4) || (con_type < 0)){
				con_type = 0;
			}

			// display the connection speed
			str[0] = '\0';
			strcpy_s(str, Multi_join_speed_labels[con_type]);
			gr_set_color_fast(Multi_join_speed_colors[con_type]);
			gr_string(Mj_speed_coords[gr_screen.res][MJ_X_COORD], y_start, str);

			// we'll want to have different colors for highlighted items, etc.
			if(moveup == Multi_join_selected_item){
				gr_set_color_fast(&Color_text_selected);
			} else {
				gr_set_color_fast(&Color_text_normal);
			}

			// display the game name, adding appropriate status chars
			str[0] = '\0';
			if(moveup->flags & AG_FLAG_STANDALONE){
				strcat_s(str,MJ_CHAR_STANDALONE);
			}
			if(moveup->flags & AG_FLAG_CAMPAIGN){
				strcat_s(str,MJ_CHAR_CAMPAIGN);
			}

			// tack on the actual server name			
			strcat_s(str," ");
			strcat_s(str,moveup->name);
			if(moveup->mission_name[0] != '\0'){
				strcat_s(str, " / ");
				strcat_s(str,moveup->mission_name);
			} 

			// make sure the string fits in the display area and draw it
			gr_force_fit_string(str,200,Mj_game_name_coords[gr_screen.res][MJ_W_COORD]);			
			gr_string(Mj_game_name_coords[gr_screen.res][MJ_X_COORD],y_start,str);

			// display the ping time
			if(moveup->ping.ping_avg > 0){
				if(moveup->ping.ping_avg > MJ_PING_ONE_SECOND){
					gr_set_color_fast(&Color_bright_red);
					strcpy_s(str,XSTR("> 1 sec",761));
				} else {
					// set the appropriate ping time color indicator
					if(moveup->ping.ping_avg > MJ_PING_RED){
						gr_set_color_fast(&Color_bright_red);
					} else if(moveup->ping.ping_avg > MJ_PING_YELLOW){
						gr_set_color_fast(&Color_bright_yellow);
					} else {
						gr_set_color_fast(&Color_bright_green);
					}

					sprintf(str,"%d",moveup->ping.ping_avg);
					strcat_s(str,XSTR(" ms",762));  // [[ Milliseconds ]]
				}

				gr_string(Mj_ping_coords[gr_screen.res][MJ_X_COORD],y_start,str);
			}

			// display the number of players (be sure to center it)
			if(moveup == Multi_join_selected_item){
				gr_set_color_fast(&Color_text_selected);
			} else {
				gr_set_color_fast(&Color_text_normal);
			}
			sprintf(str,"%d",moveup->num_players);			
			gr_get_string_size(&w,&h,str);
			gr_string(Mj_players_coords[gr_screen.res][MJ_X_COORD] + (Mj_players_coords[gr_screen.res][MJ_W_COORD] - w)/2,y_start,str);			

			count++;
			y_start += 10;
			moveup = moveup->next;
		} while((moveup != Active_game_head) && (count < Mj_max_game_items[gr_screen.res]));
	}
	// if there are no items on the list, display this info
	else {
		gr_set_color_fast(&Color_bright);
		gr_string(Mj_game_name_coords[gr_screen.res][MJ_X_COORD] - 30,y_start,XSTR("<No game servers found>",763));
	}
}

void multi_join_blit_game_status(active_game *game, int y)
{
	int draw,str_w;
	char status_text[25];

	// blit the proper icon
	draw = 0;	
	switch( game->flags & AG_FLAG_TYPE_MASK ){
	// coop game
	case AG_FLAG_COOP:
		if(Multi_common_icons[MICON_COOP] != -1){
			gr_set_bitmap(Multi_common_icons[MICON_COOP]);		
			draw = 1;
		}
		break;	
	
	// team vs. team game
	case AG_FLAG_TEAMS:
		if(Multi_common_icons[MICON_TVT] != -1){
			gr_set_bitmap(Multi_common_icons[MICON_TVT]);
			draw = 1;
		} 
		break;	

	// dogfight game
	case AG_FLAG_DOGFIGHT:
		if(Multi_common_icons[MICON_DOGFIGHT] != -1){
			gr_set_bitmap(Multi_common_icons[MICON_DOGFIGHT]);
			draw = 1;
		} 
		break;	
	}
	// if we're supposed to draw a bitmap
	if(draw){
		gr_bitmap(Mj_game_icon_coords[gr_screen.res][MJ_X_COORD],y-1);
	}

	// blit the proper status text
	memset(status_text,0,25);

	switch( game->flags & AG_FLAG_STATE_MASK ){
	case AG_FLAG_FORMING:
		gr_set_color_fast(&Color_bright_green);
		strcpy_s(status_text,XSTR("Forming",764));
		break;
	case AG_FLAG_BRIEFING:
		gr_set_color_fast(&Color_bright_red);
		strcpy_s(status_text,XSTR("Briefing",765));
		break;
	case AG_FLAG_DEBRIEF:
		gr_set_color_fast(&Color_bright_red);
		strcpy_s(status_text,XSTR("Debrief",766));
		break;
	case AG_FLAG_PAUSE:
		gr_set_color_fast(&Color_bright_red);
		strcpy_s(status_text,XSTR("Paused",767));
		break;
	case AG_FLAG_IN_MISSION:
		gr_set_color_fast(&Color_bright_red);
		strcpy_s(status_text,XSTR("Playing",768));
		break;
	default:
		gr_set_color_fast(&Color_bright);
		strcpy_s(status_text,XSTR("Unknown",769));
		break;
	}		
	gr_get_string_size(&str_w,NULL,status_text);
	gr_string(Mj_status_coords[gr_screen.res][MJ_X_COORD] + ((Mj_status_coords[gr_screen.res][MJ_W_COORD] - str_w)/2),y,status_text);
}

void multi_join_load_tcp_addrs()
{
	char line[MAX_IP_STRING];
	net_addr addr;	
	server_item *item;
	CFILE *file = NULL;

	// attempt to open the ip list file
	file = cfopen(IP_CONFIG_FNAME,"rt",CFILE_NORMAL,CF_TYPE_DATA);	
	if(file == NULL){
		nprintf(("Network","Error loading tcp.cfg file!\n"));
		return;
	}

	// free up any existing server list
	multi_free_server_list();

	// read in all the strings in the file
	while(!cfeof(file)){
		line[0] = '\0';
		cfgets(line,MAX_IP_STRING,file);

		// strip off any newline character
		if(line[strlen(line) - 1] == '\n'){
			line[strlen(line) - 1] = '\0';
		}

		// empty lines don't get processed
		if( (line[0] == '\0') || (line[0] == '\n') ){
			continue;
		}

		if ( !psnet_is_valid_ip_string(line) ) {
			nprintf(("Network","Invalid ip string (%s)\n",line));
		} else {			 
			// copy the server ip address
			memset(&addr,0,sizeof(net_addr));
			addr.type = NET_TCP;
			psnet_string_to_addr(&addr,line);
			if ( addr.port == 0 ){
				addr.port = DEFAULT_GAME_PORT;
			}

			// create a new server item on the list
			item = multi_new_server_item();
			if(item != NULL){
				memcpy(&item->server_addr,&addr,sizeof(net_addr));
			}			
		}
	}

	cfclose(file);
}

// do stuff like pinging servers, sending out requests, etc
void multi_join_do_netstuff()
{
	// handle game query stuff
	if (Multi_join_glr_stamp == -1) {
		broadcast_game_query();

		if(Net_player->p_info.options.flags & MLO_FLAG_LOCAL_BROADCAST){
			Multi_join_glr_stamp = timestamp(MULTI_JOIN_REFRESH_TIME_LOCAL);
		} else {
			Multi_join_glr_stamp = timestamp(MULTI_JOIN_REFRESH_TIME);
		}
	} 
	// otherwise send out game query and restamp
	else if ( timestamp_elapsed(Multi_join_glr_stamp) ) {			
		broadcast_game_query();

		if(Net_player->p_info.options.flags & MLO_FLAG_LOCAL_BROADCAST){
			Multi_join_glr_stamp = timestamp(MULTI_JOIN_REFRESH_TIME_LOCAL);
		} else {
			Multi_join_glr_stamp = timestamp(MULTI_JOIN_REFRESH_TIME);
		}		
	}

	// check to see if we've been accepted.  If so, put up message saying so
	if ( Net_player->flags & (NETINFO_FLAG_ACCEPT_INGAME|NETINFO_FLAG_ACCEPT_CLIENT|NETINFO_FLAG_ACCEPT_HOST|NETINFO_FLAG_ACCEPT_OBSERVER) ) {
		multi_common_add_notify(XSTR("Accepted.  Waiting for player data.",770));
	}

	// check to see if any join packets we have sent have timed out
	if((Multi_join_sent_stamp != -1) && (timestamp_elapsed(Multi_join_sent_stamp))){
		Multi_join_sent_stamp = -1;
		multi_common_add_notify(XSTR("Join request timed out!",771));
	}

	// check to see if we should be pinging everyone
	if((Multi_join_ping_stamp == -1) || (timestamp_elapsed(Multi_join_ping_stamp))){
		multi_join_ping_all();
		Multi_join_ping_stamp = timestamp(MULTI_JOIN_PING_TIME);
	} 

	// cull timeouts
	multi_join_cull_timeouts();
}

// evaluate a returned pong.
void multi_join_eval_pong(net_addr *addr, fix pong_time)
{	
	active_game *moveup = Active_game_head;

	if(moveup != NULL){
		do {				
			if(psnet_same(&moveup->server_addr,addr)){
				multi_ping_eval_pong(&moveup->ping);
				
				break;
			} else {
				moveup = moveup->next;
			}
		} while(moveup != Active_game_head);
	}	
}

// ping all the server on the list
void multi_join_ping_all()
{
	active_game *moveup = Active_game_head;	
	
	if(moveup != NULL){
		do {
			/* 
			moveup->ping_start = timer_get_fixed_seconds();
			moveup->ping_end = -1;
			send_ping(&moveup->server_addr);
			*/

			send_server_query(&moveup->server_addr);
			multi_ping_send(&moveup->server_addr,&moveup->ping);
			
			moveup = moveup->next;
		} while(moveup != Active_game_head);
	}
}

void multi_join_process_select()
{
	// if we don't have anything selected and there are items on the list - select the first one
	if((Multi_join_list_selected == -1) && (Active_game_count > 0)){
		Multi_join_list_selected = 0;
		Multi_join_selected_item = multi_join_get_game(0);				
		MJ_LIST_START_SET(0);
		Multi_join_list_start_item = Multi_join_selected_item;

		// send a mission description request to this guy		
		send_netgame_descript_packet(&Multi_join_selected_item->server_addr,0);
		multi_common_set_text("");

		// I sure hope this doesn't happen
		Assert(Multi_join_selected_item != NULL);		
		return;
	} 
	// otherwise see if he's clicked on an item
	else if(Multi_join_select_button.pressed() && (Active_game_count > 0)){		 
		int y,item;		
		Multi_join_select_button.get_mouse_pos(NULL,&y);
		item = y / 10;
		if(item + Multi_join_list_start < Active_game_count){		
			gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);

			Multi_join_list_selected = item + Multi_join_list_start;
			Multi_join_selected_item = multi_join_get_game(Multi_join_list_selected);
			
			// I sure hope this doesn't happen
			Assert(Multi_join_selected_item != NULL);

			// send a mission description request to this guy
			send_netgame_descript_packet(&Multi_join_selected_item->server_addr,0);
			multi_common_set_text("");			
		}		
	}

	// if he's double clicked, then select it and accept		
	if(Multi_join_select_button.double_clicked()){			
		int y,item;		
		Multi_join_select_button.get_mouse_pos(NULL,&y);
		item = y / 10;
		if(item == Multi_join_list_selected){		
			multi_join_button_pressed(MJ_ACCEPT);
		}
	}
}

// return game n (0 based index)
active_game *multi_join_get_game(int n)
{
	active_game *moveup = Active_game_head;

	if(moveup != NULL){
		if(n == 0){
			return moveup;
		} else {
			int count = 1;
			moveup = moveup->next;
			while((moveup != Active_game_head) && (count != n)){
				moveup = moveup->next;
				count++;
			}
			if(moveup == Active_game_head){
				nprintf(("Network","Warning, couldn't find game item %d!\n",n));
				return NULL;
			} else {
				return moveup;
			}
		}
	} 
	return NULL;
}

// scroll through the game list
void multi_join_list_scroll_up()
{
	// if we're not at the beginning of the list, scroll up	
	if(Multi_join_list_start_item != Active_game_head){
		Multi_join_list_start_item = Multi_join_list_start_item->prev;
		
		MJ_LIST_START_DEC();		

		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// scroll through the game list
void multi_join_list_scroll_down()
{
	if((Active_game_count - Multi_join_list_start) > Mj_max_game_items[gr_screen.res]){
		Multi_join_list_start_item = Multi_join_list_start_item->next;

		MJ_LIST_START_INC();		

		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void multi_join_list_page_up()
{
	// in this case, just set us to the beginning of the list
	if((Multi_join_list_start - Mj_max_game_items[gr_screen.res]) < 0){
		Multi_join_list_start_item = Active_game_head;		

		MJ_LIST_START_SET(0);

		gamesnd_play_iface(SND_SCROLL);		
	} else {
		// otherwise page the whole thing up
		int idx;
		for(idx=0; idx<Mj_max_game_items[gr_screen.res]; idx++){
			Multi_join_list_start_item = Multi_join_list_start_item->prev;

			MJ_LIST_START_DEC();			
		}
		gamesnd_play_iface(SND_SCROLL);
	}
}

void multi_join_list_page_down()
{	
	int count = 0;

	// page the whole thing down		
	while((count < Mj_max_game_items[gr_screen.res]) && ((Active_game_count - Multi_join_list_start) > Mj_max_game_items[gr_screen.res])){
		Multi_join_list_start_item = Multi_join_list_start_item->next;
		MJ_LIST_START_INC();			

		// next 
		count++;
	}	
	gamesnd_play_iface(SND_SCROLL);	 
}

void multi_join_cull_timeouts()
{
	active_game *backup;
	int count;
	active_game *moveup = Active_game_head;

	// traverse through the entire list if any items exist	
	count = 0;
	if(moveup != NULL){
		do {
			if((moveup->heard_from_timer != -1) && (timestamp_elapsed(moveup->heard_from_timer))){
				Active_game_count--;

				// if this is the head of the list
				if(moveup == Active_game_head){					
					// if this is the _only_ item on the list
					if(moveup->next == Active_game_head){
						// handle any gui details related to deleting this item
						multi_join_handle_item_cull(Active_game_head, count);
						
						vm_free(Active_game_head);
						Active_game_head = NULL;						
						return;
					} 
					// if there are other items on the list
					else {
						// handle any gui details related to deleting this item
						multi_join_handle_item_cull(moveup, count);
						
						Active_game_head = moveup->next;
						Active_game_head->prev = moveup->prev;
						Active_game_head->prev->next = Active_game_head;
						vm_free(moveup);
						moveup = Active_game_head;											
					}
				}
				// if its somewhere else on the list
				else {
					// handle any gui details related to deleting this item
					multi_join_handle_item_cull(moveup, count);
					
					// if its the last item on the list					
					moveup->next->prev = moveup->prev;
					moveup->prev->next = moveup->next;					
					
					// if it was the last element on the list, return
					if(moveup->next == Active_game_head){
						vm_free(moveup);
						return;
					} else {
						backup = moveup->next;
						vm_free(moveup);
						moveup = backup;						
					}
				}
			} else {
				moveup = moveup->next;				
				count++;
			}
		} while(moveup != Active_game_head);
	}
}

// deep magic begins here. 
void multi_join_handle_item_cull(active_game *item, int item_index)
{	
	// if this is the only item on the list, unset everything
	if(item->next == item){
		Multi_join_list_selected = -1;
		Multi_join_selected_item = NULL;
		
		Multi_join_slider.set_numberItems(0);
		MJ_LIST_START_SET(-1);
		Multi_join_list_start_item = NULL;

		// return
		return;
	}	
	
	// see if we should be adjusting our currently selected item
	if(item_index <= Multi_join_list_selected){
		// the selected item is the head of the list
		if(Multi_join_selected_item == Active_game_head){
			// move the pointer up since this item is about to be destroyed
			Multi_join_selected_item = Multi_join_selected_item->next;
		} else {			
			// if this is the item being deleted, select the previous one
			if(item == Multi_join_selected_item){
				// previous item
				Multi_join_selected_item = Multi_join_selected_item->prev;

				// decrement the selected index by 1
				Multi_join_list_selected--;		
			}
			// now we know its a previous item, so our pointer stays the same but our index goes down by one, since there will be
			// 1 less item on the list
			else {
				// decrement the selected index by 1
				Multi_join_list_selected--;		
			}
		}
	}
	
	// see if we should be adjusting out current start position
	if(item_index <= Multi_join_list_start){
		// the start position is the head of the list
		if(Multi_join_list_start_item == Active_game_head){
			// move the pointer up since this item is about to be destroyed
			Multi_join_list_start_item = Multi_join_list_start_item->next;
		} else {
			// if this is the item being deleted, select the previous one
			if(item == Multi_join_list_start_item){
				Multi_join_list_start_item = Multi_join_list_start_item->prev;			
				
				// decrement the starting index by 1
				MJ_LIST_START_DEC();								
			} else {
				// but decrement the starting index by 1
				MJ_LIST_START_DEC();								
			}
		}
	}

	// maybe go back up a bit so that we always have a full page of items	
	if(Active_game_count > Mj_max_game_items[gr_screen.res]){
		while((Active_game_count - Multi_join_list_start) < Mj_max_game_items[gr_screen.res]){
			Multi_join_list_start_item = Multi_join_list_start_item->prev;
			MJ_LIST_START_DEC();
		}
	}	

	// set slider location
	Multi_join_slider.set_numberItems(Active_game_count > Mj_max_game_items[gr_screen.res] ? Active_game_count - Mj_max_game_items[gr_screen.res] : 0);
	Multi_join_slider.force_currentItem(Multi_join_list_start);	
}

void multi_join_send_join_request(int as_observer)
{	
	// don't do anything if we have no items selected
	if(Multi_join_selected_item == NULL){
		return;
	}

	// 5/26/98 -- for team v team games, don't allow ingame joining :-(
	if ( (Multi_join_selected_item->flags & AG_FLAG_TEAMS) && (Multi_join_selected_item->flags & (AG_FLAG_PAUSE|AG_FLAG_IN_MISSION)) ) {
		popup(0, 1, POPUP_OK, XSTR("Joining ingame is currently not allowed for team vs. team games",772));
		return;
	}

	memset(&Multi_join_request,0,sizeof(join_request));

	// if the netgame is in password mode, put up a request for the password
	if(Multi_join_selected_item->flags & AG_FLAG_PASSWD){
		if(!multi_passwd_popup(Multi_join_request.passwd)){
			return;
		}

		nprintf(("Network", "Password : %s\n", Multi_join_request.passwd));
	}	
		
	// fill out the join request struct	
	strcpy_s(Multi_join_request.callsign,Player->callsign);
	if(Player->image_filename[0] != '\0'){
		strcpy_s(Multi_join_request.image_filename, Player->image_filename);
	}	
	if(Player->squad_filename[0] != '\0'){
		strcpy_s(Multi_join_request.squad_filename, Player->squad_filename);
	}

	// tracker id (if any)
	Multi_join_request.tracker_id = Multi_tracker_id;

	// player's rank (at least, what he wants you to _believe_)
	Multi_join_request.player_rank = (ubyte)Player->stats.rank;
	
	// misc join flags
	Multi_join_request.flags = 0;
	if(as_observer){
		Multi_join_request.flags |= JOIN_FLAG_AS_OBSERVER;
	}	

	// if the player has hacked data
	if(game_hacked_data()){
		Multi_join_request.flags |= JOIN_FLAG_HAXOR;
	}
	
	// pxo squad info
	strncpy(Multi_join_request.pxo_squad_name, Multi_tracker_squad_name, LOGIN_LEN);

	// version of this server
	Multi_join_request.version = MULTI_FS_SERVER_VERSION;

	// server compatible version
	Multi_join_request.comp_version = MULTI_FS_SERVER_COMPATIBLE_VERSION;

	// his local player options
	memcpy(&Multi_join_request.player_options,&Player->m_local_options,sizeof(multi_local_options));
			
	// set the server address for the netgame
	memcpy(&Netgame.server_addr,&Multi_join_selected_item->server_addr,sizeof(net_addr));

	// send a join request to the guy
	send_join_packet(&Multi_join_selected_item->server_addr,&Multi_join_request);

   // now we wait
	Multi_join_sent_stamp = timestamp(MULTI_JOIN_SENT_WAIT);

	psnet_flush();
	multi_common_add_notify(XSTR("Sending join request...",773));
}

void multi_join_create_game()
{
	// maybe warn the player about possible crappy server conditions
	if(!multi_join_maybe_warn()){
		return;
	}

	// make sure to flag ourself as being the master
	Net_player->flags |= (NETINFO_FLAG_AM_MASTER | NETINFO_FLAG_GAME_HOST);
	Net_player->state = NETPLAYER_STATE_HOST_SETUP;	

	// if we're in PXO mode, mark it down in our player struct
	if(MULTI_IS_TRACKER_GAME){
		Player->flags |= PLAYER_FLAGS_HAS_PLAYED_PXO;
		Player->save_flags |= PLAYER_FLAGS_HAS_PLAYED_PXO;
	} 
	// otherwise, if he's already played PXO games, warn him
	else {
		
		if(Player->flags & PLAYER_FLAGS_HAS_PLAYED_PXO){
			if(!multi_join_warn_pxo()){
				return;
			}			
		}
		
	}

	gameseq_post_event(GS_EVENT_MULTI_START_GAME);
	gamesnd_play_iface(SND_USER_SELECT);								
}

void multi_join_reset_join_stamp()
{
	// unset the timestamp here so the user can immediately send another join request
	Multi_join_sent_stamp = -1;
	multi_common_add_notify("");
}

void multi_join_blit_top_stuff()
{	
	// blit the cd icon if he has one
	if(Multi_has_cd && (Multi_common_icons[MICON_CD] != -1)){
		// get bitmap width
		int cd_w;
		bm_get_info(Multi_common_icons[MICON_CD], &cd_w, NULL, NULL, NULL, NULL);

		gr_set_bitmap(Multi_common_icons[MICON_CD]);
		gr_bitmap((gr_screen.max_w / 2) - (cd_w / 2), Mj_cd_coords[gr_screen.res]);
	} 	
}

#define CW_CODE_CANCEL				0				// cancel the action
#define CW_CODE_OK					1				// continue anyway
#define CW_CODE_INFO					2				// gimme some more information

#define LOW_WARN_TEXT				XSTR("Warning - You have low object updates selected. A server with low object updates will not be able to handle more than 1 client without performance problems",775)
#define LOW_INFO_TEXT				XSTR("Low update level caps all bandwidth at ~2000 bytes/second. It is appropriate for clients with 28.8 modems, but is not reccomended for servers. In addition, any clients connecting to this server should use low object updates as well. To change your settings go to the options menu (f2), and select the Multi button",776)

#define MED_WARN_TEXT				XSTR("Warning - You have medium object updates selected. A server with medium object updates will not be able to handle more than 1 or 2 clients without performance problems",777)
#define MED_INFO_TEXT				XSTR("Medium update level caps all bandwidth at ~4000 bytes/second. It is appropriate for clients with 56.6 modems, but is not reccomended for servers. In addition, any clients connecting to this server should use low object updates as well. To change your settings go to the options menu (f2), and select the Multi button",778)

int multi_join_warn_update_low(int code)
{
	switch(code){
	case CW_CODE_OK:
		return popup(0,3,XSTR("&Cancel",779),XSTR("&Continue",780),XSTR("&More info",781),LOW_WARN_TEXT);

	case CW_CODE_INFO:
		return popup(0,3,XSTR("&Cancel",779),XSTR("&Continue",780),XSTR("&More info",781),LOW_INFO_TEXT);
	}

	return CW_CODE_CANCEL;
}

int multi_join_warn_update_medium(int code)
{
	switch(code){
	case CW_CODE_OK:
		return popup(0,3,XSTR("&Cancel",779),XSTR("&Continue",780),XSTR("&More info",781),MED_WARN_TEXT);

	case CW_CODE_INFO:
		return popup(0,3,XSTR("&Cancel",779),XSTR("&Continue",780),XSTR("&More info",781),MED_INFO_TEXT);
	}

	return CW_CODE_CANCEL;
}

int multi_join_maybe_warn()
{
	int code;

	// if the player is set for low updates
	if(Player->m_local_options.obj_update_level == OBJ_UPDATE_LOW){
		code = CW_CODE_OK;
		do {
			code = multi_join_warn_update_low(code);
		} while((code != CW_CODE_CANCEL) && (code != CW_CODE_OK));
		
		return code;
	}
	
	// if the player is set for medium updates
	else if(Player->m_local_options.obj_update_level == OBJ_UPDATE_MEDIUM){
		code = CW_CODE_OK;
		do {
			code = multi_join_warn_update_medium(code);
		} while((code != CW_CODE_CANCEL) && (code != CW_CODE_OK));
		
		return code;
	} 
	
	return 1;
}

int multi_join_warn_pxo()
{
	return popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_TITLE_BIG | PF_TITLE_RED, 2, XSTR("&Back", 995), XSTR("&Continue",780), XSTR("Warning\n\nThis pilot has played PXO games. If you continue and play a non-PXO game, your stats will not be updated", 1006)) <= 0 ? 0 : 1;
	//return 1;
}

void multi_join_blit_protocol()
{
	gr_set_color_fast(&Color_bright);

	switch(Socket_type){
	case NET_TCP:		
		// straight TCP		
		gr_string(5, 2, "TCP");		
		break;

	case NET_IPX:
		gr_string(5, 2, "IPX");
		break;
	}
}


// -------------------------------------------------------------------------------------------------
//
// MULTIPLAYER START GAME screen
//

//XSTR:OFF
// bitmap defs
#define MULTI_SG_PALETTE			"InterfacePalette"

static char *Multi_sg_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MultiStartGame",			// GR_640
	"2_MultiStartGame"			// GR_1024
};

static char *Multi_sg_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MultiStartGame-M",			// GR_640
	"2_MultiStartGame-M"			// GR_1024
};

//XSTR:ON

int Multi_sg_rank_max_display[GR_NUM_RESOLUTIONS] = {
	2,		// GR_640
	4		// GR_1024
};

// constants for coordinate look ups
#define MSG_X_COORD 0
#define MSG_Y_COORD 1
#define MSG_W_COORD 2
#define MSG_H_COORD 3

// area definitions

// input password field
int Msg_passwd_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		36, 236, 408, 20
	},
	{ // GR_1024
		58, 377, 652, 32
	}
};

// input game title field
int Msg_title_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		29, 49, 415, 23
	},
	{ // GR_1024
		46, 78, 664, 36
	}
};

// rank selected field
int Msg_rank_sel_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		242, 254, 126, 12
	},
	{ // GR_1024
		242, 254, 126, 12
	}
};

// rank list field
int Msg_rank_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		37, 297, 131, 16
	},
	{ // GR_1024
		60, 469, 652, 32
	}
};


// button defs
#define MULTI_SG_NUM_BUTTONS	10
//#define MULTI_SG_NUM_BUTTONS	12

#define MSG_OPEN_GAME			0
//#define MSG_CLOSED_GAME			1
//#define MSG_RESTRICTED_GAME		2
#define MSG_PASSWD_GAME			1
#define MSG_RANK_SET_GAME		2
#define MSG_RANK_SCROLL_UP		3
#define MSG_RANK_SCROLL_DOWN	4
#define MSG_RANK_ABOVE			5
#define MSG_RANK_BELOW			6
#define MSG_HELP				7
#define MSG_OPTIONS				8
#define MSG_ACCEPT				9

UI_WINDOW Multi_sg_window;												// the window object for the join screen
UI_BUTTON Multi_sg_rank_button;										// for selecting the rank marker
UI_INPUTBOX	Multi_sg_game_name;										// for Netgame.name
UI_INPUTBOX Multi_sg_game_passwd;									// for Netgame.passwd
int Multi_sg_bitmap;														// the background bitmap

ui_button_info Multi_sg_buttons[GR_NUM_RESOLUTIONS][MULTI_SG_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("MSG_00",	1,		184,	34,	191,	2),		// open
//******ui_button_info("MSG_01",	1,		159,	34,	166,	1),		// closed
//******ui_button_info("MSG_02",	1,		184,	34,	191,	2),		// restricted
		ui_button_info("MSG_03",	1,		209,	34,	218,	3),		// password
		ui_button_info("MSG_04",	1,		257,	34,	266,	4),		// rank set
		ui_button_info("MSG_05",	1,		282,	-1,	-1,	5),		// rank scroll up
		ui_button_info("MSG_06",	1,		307,	-1,	-1,	6),		// rank scroll down
		ui_button_info("MSG_07",	177,	282,	210,	290,	7),		// rank above
		ui_button_info("MSG_08",	177,	307,	210,	315,	8),		// rank below
		ui_button_info("MSG_09",	536,	429,	500,	440,	9),		// help
		ui_button_info("MSG_10",	536,	454,	479,	464,	10),		// options
		ui_button_info("MSG_11",	576,	432,	571,	415,	11),		// accept
	},
	{ // GR_1024
		ui_button_info("2_MSG_00",	2,		295,	51,	307,	2),		// open
//******ui_button_info("2_MSG_01",	2,		254,	51,	267,	1),		// closed
//******ui_button_info("2_MSG_02",	2,		295,	51,	307,	2),		// restricted
		ui_button_info("2_MSG_03",	2,		335,	51,	350,	3),		// password
		ui_button_info("2_MSG_04",	2,		412,	51,	426,	4),		// rank set
		ui_button_info("2_MSG_05",	2,		452,	-1,	-1,	5),		// rank scroll up
		ui_button_info("2_MSG_06",	2,		492,	-1,	-1,	6),		// rank scroll down
		ui_button_info("2_MSG_07",	284,	452,	335,	465,	7),		// rank above
		ui_button_info("2_MSG_08",	284,	492,	335,	505,	8),		// rank below
		ui_button_info("2_MSG_09",	858,	687,	817,	706,	9),		// help
		ui_button_info("2_MSG_10",	858,	728,	797,	743,	10),		// options
		ui_button_info("2_MSG_11",	921,	692,	921,	664,	11),		// accept
	},
};

//#define MULTI_SG_NUM_TEXT			13
#define MULTI_SG_NUM_TEXT			11
UI_XSTR Multi_sg_text[GR_NUM_RESOLUTIONS][MULTI_SG_NUM_TEXT] = {
	{ // GR_640
		{"Open",					1322,		34,	191,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_OPEN_GAME].button},
//******{"Closed",				1323,		34,	166,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_CLOSED_GAME].button},
//******{"Restricted",			1324,		34,	191,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_RESTRICTED_GAME].button},
		{"Password Protected",	1325,	34,	218,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_PASSWD_GAME].button},
		{"Allow Rank",			1326,		34,	266,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_RANK_SET_GAME].button},
		{"Above",				1327,		210,	290,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_RANK_ABOVE].button},
		{"Below",				1328,		210,	315,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_RANK_BELOW].button},
		{"Help",					928,		500,	440,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_HELP].button},
		{"Options",				1036,		479,	464,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[0][MSG_OPTIONS].button},
		{"Accept",				1035,		571,	415,	UI_XSTR_COLOR_PINK,	-1,	&Multi_sg_buttons[0][MSG_ACCEPT].button},
		{"Start Game",			1329,		26,	10,	UI_XSTR_COLOR_GREEN,	-1,	NULL},
		{"Title",				1330,		26,	31,	UI_XSTR_COLOR_GREEN,	-1,	NULL},
		{"Game Type",			1331,		12,	165,	UI_XSTR_COLOR_GREEN,	-1,	NULL},
	},
	{ // GR_1024
		{"Open",					1322,		51,	307,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_OPEN_GAME].button},
//******{"Closed",				1323,		51,	267,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_CLOSED_GAME].button},
//******{"Restricted",			1324,		51,	307,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_RESTRICTED_GAME].button},
		{"Password Protected",	1325,	51,	350,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_PASSWD_GAME].button},
		{"Allow Rank",			1326,		51,	426,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_RANK_SET_GAME].button},
		{"Above",				1327,		335,	465,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_RANK_ABOVE].button},
		{"Below",				1328,		335,	505,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_RANK_BELOW].button},
		{"Help",					928,		817,	706,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_HELP].button},
		{"Options",				1036,		797,	743,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_sg_buttons[1][MSG_OPTIONS].button},
		{"Accept",				1035,		921,	664,	UI_XSTR_COLOR_PINK,	-1,	&Multi_sg_buttons[1][MSG_ACCEPT].button},
		{"Start Game",			1329,		42,	22,	UI_XSTR_COLOR_GREEN,	-1,	NULL},
		{"Title",				1330,		42,	50,	UI_XSTR_COLOR_GREEN,	-1,	NULL},
		{"Game Type",			1331,		20,	264,	UI_XSTR_COLOR_GREEN,	-1,	NULL},
	}
};

// starting index for displaying ranks
int Multi_sg_rank_start;
int Multi_sg_rank_select;

// netgame pointer to indirect through
netgame_info *Multi_sg_netgame;

// hold temporary values in this structure when on a standalone server
netgame_info Multi_sg_netgame_temp;

// forward declarations
void multi_sg_check_buttons();
void multi_sg_button_pressed(int n);
void multi_sg_init_gamenet();
void multi_sg_draw_radio_buttons();
void multi_sg_rank_scroll_up();
void multi_sg_rank_scroll_down();
void multi_sg_rank_display_stuff();
void multi_sg_rank_process_select();
void multi_sg_rank_build_name(char *in,char *out);
void multi_sg_check_passwd();
void multi_sg_check_name();
void multi_sg_release_passwd();
int multi_sg_rank_select_valid(int rank);
void multi_sg_select_rank_default();

// function which takes a rank name and returns the index.  Useful for commandline options
// for above and below rank.  We return the index of the rank in the Ranks[] array.  If
// the rank isn't found, we return -1
int multi_start_game_rank_from_name( char *rank ) {
	int i;

	for ( i = 0; i <= MAX_FREESPACE2_RANK; i++ ) {
		if ( !stricmp(Ranks[i].name, rank) ) {
			return i;
		}
	}

	return -1;
}

void multi_start_game_init()
{
	int idx;

	// initialize the gamenet
	multi_sg_init_gamenet();
	
	// create the interface window
	Multi_sg_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_sg_window.set_mask_bmap(Multi_sg_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_sg_bitmap = bm_load(Multi_sg_bitmap_fname[gr_screen.res]);
	if(Multi_sg_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}
	
	// initialize the common notification messaging
	multi_common_notify_init();

	// initialize the common text area
	multi_common_set_text("");

	// use the common interface palette
	multi_common_set_palette();
	
	// create the interface buttons
	for(idx=0; idx<MULTI_SG_NUM_BUTTONS; idx++){
		// create the object
		Multi_sg_buttons[gr_screen.res][idx].button.create(&Multi_sg_window, "", Multi_sg_buttons[gr_screen.res][idx].x, Multi_sg_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_sg_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_sg_buttons[gr_screen.res][idx].button.set_bmaps(Multi_sg_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Multi_sg_buttons[gr_screen.res][idx].button.link_hotspot(Multi_sg_buttons[gr_screen.res][idx].hotspot);
	}	

	// add all xstrs
	for(idx=0; idx<MULTI_SG_NUM_TEXT; idx++){
		Multi_sg_window.add_XSTR(&Multi_sg_text[gr_screen.res][idx]);
	}

	// load the help overlay
	help_overlay_load(MULTI_START_OVERLAY);
	help_overlay_set_state(MULTI_START_OVERLAY,0);

	// intiialize the rank selection items	
	multi_sg_select_rank_default();	
	Multi_sg_rank_start = Multi_sg_rank_select;

	// create the rank select button
	Multi_sg_rank_button.create(&Multi_sg_window,"",Msg_rank_list_coords[gr_screen.res][MSG_X_COORD],Msg_rank_list_coords[gr_screen.res][MSG_Y_COORD],Msg_rank_list_coords[gr_screen.res][MSG_W_COORD],Msg_rank_list_coords[gr_screen.res][MSG_H_COORD],0,1);	
	Multi_sg_rank_button.hide();		

	// create the netgame name input box
	Multi_sg_game_name.create(&Multi_sg_window,Msg_title_coords[gr_screen.res][MSG_X_COORD],Msg_title_coords[gr_screen.res][MSG_Y_COORD],Msg_title_coords[gr_screen.res][MSG_W_COORD],MAX_GAMENAME_LEN,"",UI_INPUTBOX_FLAG_ESC_CLR | UI_INPUTBOX_FLAG_INVIS,-1,&Color_normal);

	// create the netgame password input box, and disable it by default
	Multi_sg_game_passwd.create(&Multi_sg_window,Msg_passwd_coords[gr_screen.res][MSG_X_COORD],Msg_passwd_coords[gr_screen.res][MSG_Y_COORD],Msg_passwd_coords[gr_screen.res][MSG_W_COORD],16,"",UI_INPUTBOX_FLAG_ESC_CLR | UI_INPUTBOX_FLAG_PASSWD | UI_INPUTBOX_FLAG_INVIS,-1,&Color_normal);
	Multi_sg_game_passwd.hide();
	Multi_sg_game_passwd.disable();

	// set the netgame text to this gadget and make it have focus
	Multi_sg_game_name.set_text(Multi_sg_netgame->name);
	Multi_sg_game_name.set_focus();	

	// if starting a netgame, set the name of the game and any other options that are appropriate
	if ( Cmdline_start_netgame ) {
		if ( Cmdline_game_name != NULL ) {
			strcpy_s( Multi_sg_netgame->name, Cmdline_game_name );
			Multi_sg_game_name.set_text(Multi_sg_netgame->name);
		}

		// deal with the different game types -- only one should even be active, so we will just go down
		// the line.  Last one wins.
		if ( Cmdline_closed_game ) {
			Multi_sg_netgame->mode = NG_MODE_CLOSED;
		} else if ( Cmdline_restricted_game ) {
			Multi_sg_netgame->mode = NG_MODE_RESTRICTED;
		} else if ( Cmdline_game_password != NULL ) {
			Multi_sg_netgame->mode = NG_MODE_PASSWORD;
			strcpy_s(Multi_sg_netgame->passwd, Cmdline_game_password);
			Multi_sg_game_passwd.set_text(Multi_sg_netgame->passwd);
		}

		// deal with rank above and rank below
		if ( (Cmdline_rank_above != NULL) || (Cmdline_rank_below != NULL) ) {
			int rank;
			char *rank_str;

			if ( Cmdline_rank_above != NULL ) {
				rank_str = Cmdline_rank_above;
			} else {
				rank_str = Cmdline_rank_below;
			}

			// try and get the rank index from the name -- if found, then set the rank base
			// and the game type.  apparently we only support either above or below, not both
			// together, so I make a random choice
			rank = multi_start_game_rank_from_name( rank_str );
			if ( rank != -1 ) {
				Multi_sg_netgame->rank_base = Multi_sg_rank_select;

				// now an arbitrary decision
				if ( Cmdline_rank_above != NULL ) {
					Multi_sg_netgame->mode = NG_MODE_RANK_ABOVE;
				} else {
					Multi_sg_netgame->mode = NG_MODE_RANK_BELOW;
				}
			}
		}

		gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
	}
}

void multi_start_game_do()
{
	// return here since we will be moving to the next stage anyway -- I don't want to see the backgrounds of
	// all the screens for < 1 second for every screen we automatically move to.
	if ( Cmdline_start_netgame ) {
		Cmdline_start_netgame = 0; // DTP no quit fix; by using -startgame Quit was sort of out of function 
		return;
	}

	int k = Multi_sg_window.process();

	// process any keypresses
	switch(k){
	case KEY_ESC :		
		if(help_overlay_active(MULTI_START_OVERLAY)){
			help_overlay_set_state(MULTI_START_OVERLAY,0);
		} else {
			gamesnd_play_iface(SND_USER_SELECT);
			multi_quit_game(PROMPT_NONE);
		}
		break;
	
	// same as ACCEPT
	case KEY_LCTRL + KEY_ENTER :
	case KEY_RCTRL + KEY_ENTER :		
		gamesnd_play_iface(SND_COMMIT_PRESSED);
		gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
		break;
	}	

	if ( mouse_down(MOUSE_LEFT_BUTTON) ) {
		help_overlay_set_state(MULTI_START_OVERLAY, 0);
	}

	// check to see if the user has selected a different rank
	multi_sg_rank_process_select();

	// check any button presses
	multi_sg_check_buttons();

	// check to see if any of the input boxes have changed, and update the appropriate Netgame fields if necessary
	multi_sg_check_passwd();
	multi_sg_check_name();

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Multi_sg_bitmap);
	if(Multi_sg_bitmap != -1){
		gr_set_bitmap(Multi_sg_bitmap);
		gr_bitmap(0,0);
	}
	Multi_sg_window.draw();
	
	// display rank stuff
	multi_sg_rank_display_stuff();

	// display any pending notification messages
	multi_common_notify_do();

	// draw all radio button
	multi_sg_draw_radio_buttons();

	// draw the help overlay
	help_overlay_maybe_blit(MULTI_START_OVERLAY);
	
	// flip the buffer
	gr_flip();
}

void multi_start_game_close()
{
	// if i'm the host on a standalone server, send him my start game options (passwd, mode, etc)
	if((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		multi_options_update_start_game(Multi_sg_netgame);
	}
	
	// unload any bitmaps
	if(!bm_unload(Multi_sg_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_sg_bitmap_fname[gr_screen.res]));
	}

	// unload the help overlay
	help_overlay_unload(MULTI_START_OVERLAY);
	
	// destroy the UI_WINDOW
	Multi_sg_window.destroy();	
}

void multi_sg_check_buttons()
{
	int idx;
	for(idx=0;idx<MULTI_SG_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Multi_sg_buttons[gr_screen.res][idx].button.pressed()){
			multi_sg_button_pressed(idx);
			break;
		}
	}
}

void multi_sg_button_pressed(int n)
{
	switch(n){		
	// go to the options screen
	case MSG_OPTIONS:
		gameseq_post_event(GS_EVENT_OPTIONS_MENU);
		break;	

	// help overlay	
	case MSG_HELP:
		if(!help_overlay_active(MULTI_START_OVERLAY)){
			help_overlay_set_state(MULTI_START_OVERLAY,1);
		} else {
			help_overlay_set_state(MULTI_START_OVERLAY,0);
		}
		break;

	// the open button was pressed
	case MSG_OPEN_GAME:		
		// if the closed option is selected
		if(Multi_sg_netgame->mode != NG_MODE_OPEN){
			Multi_sg_netgame->mode = NG_MODE_OPEN;

			gamesnd_play_iface(SND_USER_SELECT);
			
			// release the password control if necessary
			multi_sg_release_passwd();
		}
		// if its already selected
		else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

/*
	// the open button was pressed
	case MSG_CLOSED_GAME:		
		// if the closed option is selected
		if(Multi_sg_netgame->mode != NG_MODE_CLOSED){
			Multi_sg_netgame->mode = NG_MODE_CLOSED;

			gamesnd_play_iface(SND_USER_SELECT);
			
			// release the password control if necessary
			multi_sg_release_passwd();
		}
		// if its already selected
		else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;
*/
	// toggle password protection
	case MSG_PASSWD_GAME:		
		// if we selected it
		if(Multi_sg_netgame->mode != NG_MODE_PASSWORD){
			gamesnd_play_iface(SND_USER_SELECT);		

			Multi_sg_game_passwd.enable();			
			Multi_sg_game_passwd.unhide();
			Multi_sg_game_passwd.set_focus();

			Multi_sg_netgame->mode = NG_MODE_PASSWORD;			

			// copy in the current network password
			Multi_sg_game_passwd.set_text(Multi_sg_netgame->passwd);
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}			
		break;

/*
	// toggle "restricted" on or off
	case MSG_RESTRICTED_GAME:
		if(Multi_sg_netgame->mode != NG_MODE_RESTRICTED){
			gamesnd_play_iface(SND_USER_SELECT);
			Multi_sg_netgame->mode = NG_MODE_RESTRICTED;

			// release the password control if necessary
			multi_sg_release_passwd();
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;
*/
	// turn off all rank requirements
	case MSG_RANK_SET_GAME:		
		// if either is set, then turn then both off
		if((Multi_sg_netgame->mode != NG_MODE_RANK_BELOW) && (Multi_sg_netgame->mode != NG_MODE_RANK_ABOVE)){
			gamesnd_play_iface(SND_USER_SELECT);

			// set it to the default case if we're turning it off
			multi_sg_select_rank_default();
			Multi_sg_rank_start = Multi_sg_rank_select;			

			Multi_sg_netgame->mode = NG_MODE_RANK_ABOVE;

			// release the password control if necessary
			multi_sg_release_passwd();
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}		
		break;

	// rank above was pressed
	case MSG_RANK_ABOVE :
		if((Multi_sg_netgame->mode == NG_MODE_RANK_ABOVE) || (Multi_sg_netgame->mode == NG_MODE_RANK_BELOW)){
			Multi_sg_netgame->mode = NG_MODE_RANK_ABOVE;

			// select the first item
			multi_sg_select_rank_default();
			Multi_sg_rank_start = Multi_sg_rank_select;

			// play a sound
			gamesnd_play_iface(SND_USER_SELECT);			
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	// rank below was pressed
	case MSG_RANK_BELOW :
		if((Multi_sg_netgame->mode == NG_MODE_RANK_ABOVE) || (Multi_sg_netgame->mode == NG_MODE_RANK_BELOW)){
			Multi_sg_netgame->mode = NG_MODE_RANK_BELOW;
			
			// select the first item
			multi_sg_select_rank_default();
			Multi_sg_rank_start = Multi_sg_rank_select;			

			// play a sound
			gamesnd_play_iface(SND_USER_SELECT);			
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}	
		break;

	// scroll the rank list up
	case MSG_RANK_SCROLL_UP:
		multi_sg_rank_scroll_up();
		break;

	// scroll the rank list down
	case MSG_RANK_SCROLL_DOWN:
		multi_sg_rank_scroll_down();
		break;

	// move to the create game screen
	case MSG_ACCEPT:
		gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
		gamesnd_play_iface(SND_COMMIT_PRESSED);
		break;

	default :
		gamesnd_play_iface(SND_GENERAL_FAIL);
		multi_common_add_notify(XSTR("Not implemented yet!",760));		
		break;
	}
}

// NOTE : this is where all Netgame initialization should take place on the host
void multi_sg_init_gamenet()
{
	char buf[128],out_name[128];		
	net_addr save;
	net_player *server_save;	

	// back this data up in case we are already connected to a standalone
	memcpy(&save,&Netgame.server_addr,sizeof(net_addr));
	server_save = Netgame.server;

	// remove campaign flags
	Game_mode &= ~(GM_CAMPAIGN_MODE);

	// clear out the Netgame structure and start filling in the values
	memset( &Netgame, 0, sizeof(Netgame) );	
	memset( &Multi_sg_netgame_temp, 0, sizeof(netgame_info) );
	
	// if we're on the standalone, we're not the server, so we don't care about setting the netgame state
	if(Net_player->state != NETPLAYER_STATE_STD_HOST_SETUP){		
		Netgame.game_state = NETGAME_STATE_HOST_SETUP;
		Multi_sg_netgame = &Netgame;	

		// NETLOG
		ml_string(NOX("Starting netgame as Host/Server"));		
	} else {
		Multi_sg_netgame = &Multi_sg_netgame_temp;

		// NETLOG
		in_addr temp_addr;
		memcpy(&temp_addr.s_addr, &Netgame.server_addr, 4);
		char *server_addr = inet_ntoa(temp_addr);				
		ml_printf(NOX("Starting netgame as Host on Standalone server : %s"), (server_addr == NULL) ? NOX("Unknown") : server_addr);
	}
	
	Net_player->tracker_player_id = Multi_tracker_id;

	Multi_sg_netgame->security = (rand() % 32766) + 1;			// get some random security number	
	Multi_sg_netgame->mode = NG_MODE_OPEN;
	Multi_sg_netgame->rank_base = RANK_ENSIGN;
	if(Multi_sg_netgame->security < 16){
		Multi_sg_netgame->security += 16;
	}

	// set the version_info field
	Multi_sg_netgame->version_info = NG_VERSION_ID;
	
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
		Netgame.host = Net_player;
	}
	
	// set the default netgame flags
	Multi_sg_netgame->flags = 0;

	// intialize endgame stuff
	multi_endgame_init();

	// load in my netgame options
	multi_options_netgame_load(&Netgame.options);		

	// load my local netplayer options
	multi_options_local_load(&Net_player->p_info.options, Net_player);	
	
	// setup the default game name, taking care of string length and player callsigns
	memset(out_name,0,128);
	memset(buf,0,128);
	pilot_format_callsign_personal(Player->callsign,out_name);
	sprintf(buf, XSTR("%s game",782), out_name);  // [[ %s will be a pilot's name ]]
	if ( strlen(buf) > MAX_GAMENAME_LEN ){
		strcpy_s(buf, XSTR("Temporary name",783));
	}
	strcpy_s(Multi_sg_netgame->name, buf);

	// set the default qos and duration
	multi_voice_maybe_update_vars(Netgame.options.voice_qos,Netgame.options.voice_record_time);

	// make sure to set the server correctly (me or the standalone)
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		memcpy(&Netgame.server_addr, &Psnet_my_addr,sizeof(net_addr));
		Netgame.server = Net_player;
		Net_player->player_id = multi_get_new_id();

		// setup debug flags
		Netgame.debug_flags = 0;
	} else {
		memcpy(&Netgame.server_addr,&save,sizeof(net_addr));
		Netgame.server = server_save;
	}

	// if I have a cd or not
	if(Multi_has_cd){
		Net_player->flags |= NETINFO_FLAG_HAS_CD;
	}	

	// if I have hacked data
	if(game_hacked_data()){
		Net_player->flags |= NETINFO_FLAG_HAXOR;
	}

	// assign my player struct and other data	
	Net_player->flags |= (NETINFO_FLAG_CONNECTED | NETINFO_FLAG_DO_NETWORKING);
	Net_player->s_info.voice_token_timestamp = -1;	

	// if we're supposed to flush our cache directory, do so now
	if(Net_player->p_info.options.flags & MLO_FLAG_FLUSH_CACHE){
		multi_flush_multidata_cache();

		// NETLOG
		ml_string(NOX("Flushing multi-data cache"));
	}
			
	game_flush();
}

void multi_sg_draw_radio_buttons()
{
	// draw the appropriate radio button
	switch(Multi_sg_netgame->mode){
	case NG_MODE_OPEN:
		Multi_sg_buttons[gr_screen.res][MSG_OPEN_GAME].button.draw_forced(2);
		break;
/*
	case NG_MODE_CLOSED:
		Multi_sg_buttons[gr_screen.res][MSG_CLOSED_GAME].button.draw_forced(2);
		break;
*/	
	case NG_MODE_PASSWORD:
		Multi_sg_buttons[gr_screen.res][MSG_PASSWD_GAME].button.draw_forced(2);
		break;
/*
	case NG_MODE_RESTRICTED:
		Multi_sg_buttons[gr_screen.res][MSG_RESTRICTED_GAME].button.draw_forced(2);
		break;
*/
	case NG_MODE_RANK_ABOVE:
		Multi_sg_buttons[gr_screen.res][MSG_RANK_SET_GAME].button.draw_forced(2);
		Multi_sg_buttons[gr_screen.res][MSG_RANK_ABOVE].button.draw_forced(2);
		break;
	case NG_MODE_RANK_BELOW:
		Multi_sg_buttons[gr_screen.res][MSG_RANK_SET_GAME].button.draw_forced(2);
		Multi_sg_buttons[gr_screen.res][MSG_RANK_BELOW].button.draw_forced(2);
		break;
	}
}

void multi_sg_rank_scroll_up()
{	
	// if he doesn't have either of the rank flags set, then ignore this
	if((Multi_sg_netgame->mode != NG_MODE_RANK_ABOVE) && (Multi_sg_netgame->mode != NG_MODE_RANK_BELOW)){
		return;
	}

	if(Multi_sg_rank_start > 0){
		Multi_sg_rank_start--;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void multi_sg_rank_scroll_down()
{
	// if he doesn't have either of the rank flags set, then ignore this
	if((Multi_sg_netgame->mode != NG_MODE_RANK_ABOVE) && (Multi_sg_netgame->mode != NG_MODE_RANK_BELOW)){
		return;
	}
	
	if((NUM_RANKS - Multi_sg_rank_start) > Multi_sg_rank_max_display[gr_screen.res]){
		Multi_sg_rank_start++;
		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}	
}

void multi_sg_rank_display_stuff()
{
	int y,idx,count;
	char rank_name[40];

	// if he doesn't have either of the rank flags set, then ignore this
	if((Multi_sg_netgame->mode != NG_MODE_RANK_ABOVE) && (Multi_sg_netgame->mode != NG_MODE_RANK_BELOW)){
		return;
	}
		
	// display the list of ranks
	y = Msg_rank_list_coords[gr_screen.res][MSG_Y_COORD];
	idx = Multi_sg_rank_start;
	count = 0;
	while((count < NUM_RANKS) && (count < Multi_sg_rank_max_display[gr_screen.res]) && (idx < NUM_RANKS)){	
		// if its the selected item, then color it differently
		if(idx == Multi_sg_rank_select){
			gr_set_color_fast(&Color_text_selected);
		} else {
			gr_set_color_fast(&Color_text_normal);
		}

		// print the text
		multi_sg_rank_build_name(Ranks[idx].name,rank_name);
		gr_string(Msg_rank_list_coords[gr_screen.res][MSG_X_COORD],y,rank_name);

		// increment stuff
		y+=10;
		idx++;
		count++;
	}

	// display the selected rank
	
	gr_set_color_fast(&Color_bright);
	multi_sg_rank_build_name(Ranks[Multi_sg_netgame->rank_base].name,rank_name);
	gr_string(Msg_rank_sel_coords[gr_screen.res][MSG_X_COORD],Msg_rank_sel_coords[gr_screen.res][MSG_Y_COORD],rank_name);
	
}

void multi_sg_rank_process_select()
{
	char string[255];
	
	// if he doesn't have either of the rank flags set, then ignore this
	if((Multi_sg_netgame->mode != NG_MODE_RANK_ABOVE) && (Multi_sg_netgame->mode != NG_MODE_RANK_BELOW)){
		return;
	}
	
	// see if he's clicked on an item on the rank list
	if(Multi_sg_rank_button.pressed()){		 
		int y,item;		
		Multi_sg_rank_button.get_mouse_pos(NULL,&y);
		item = y / 10;
		
		if(item + Multi_sg_rank_start < NUM_RANKS){		
			// evaluate whether this rank is valid for the guy to pick		
			if(multi_sg_rank_select_valid(item + Multi_sg_rank_start)){
				gamesnd_play_iface(SND_USER_SELECT);

				Multi_sg_rank_select = item + Multi_sg_rank_start;						

				// set the Netgame rank
				Multi_sg_netgame->rank_base = Multi_sg_rank_select;
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);

				memset(string,0,255);
				sprintf(string,XSTR("Illegal value for a host of your rank (%s)\n",784),Ranks[Net_player->m_player->stats.rank].name);
				multi_common_add_notify(string);
			}
		}		
	}
}

void multi_sg_rank_build_name(char *in,char *out)
{
	char use[100];
	char *first;

	strcpy_s(use,in);
	first = strtok(use," ");

	// just copy the string
	if(first == NULL){
		strcpy(out,in);
	}
	
	// if the first part of the string is lieutenant, then abbreivate it and tack on the rest of the string	
	if (stricmp(first,XSTR("lieutenant",785)) == 0) {
		first = strtok(NULL, NOX("\n"));

		// if he's not just a plain lieutenant
		if(first != NULL){
			strcpy(out,XSTR("Lt. ",786));  // [[ lieutenant ]]
			strcat(out,first);
		}
		// if he _is_ just a plain lieutenant
		else {
			strcpy(out,in);
		}
	} else {
		strcpy(out,in);
	}
}

void multi_sg_check_passwd()
{
	// check to see if the password input box has been pressed
	if(Multi_sg_game_passwd.changed()){
		Multi_sg_game_passwd.get_text(Multi_sg_netgame->passwd);
	}
}

void multi_sg_check_name()
{
	// check to see if the game name input box has been pressed
	if(Multi_sg_game_name.changed()){
		Multi_sg_game_name.get_text(Multi_sg_netgame->name);
	}
}

void multi_sg_release_passwd()
{
	// hide and disable the password input box
	Multi_sg_game_passwd.hide();
	Multi_sg_game_passwd.disable();

	// set the focus back to the name input box
	Multi_sg_game_name.set_focus();
}

int multi_sg_rank_select_valid(int rank)
{
	// rank above mode
	if(Multi_sg_netgame->mode == NG_MODE_RANK_ABOVE){
		if(Net_player->m_player->stats.rank >= rank){
			return 1;
		}
	}
	// rank below mode
	else {
		if(Net_player->m_player->stats.rank <= rank){
			return 1;
		}
	}
	
	return 0;
}

void multi_sg_select_rank_default()
{
	// pick our rank for now
	Multi_sg_rank_select = Net_player->m_player->stats.rank;

	// set the Netgame rank
	Multi_sg_netgame->rank_base = Multi_sg_rank_select;
}
		
// -------------------------------------------------------------------------------------------------
//
// MULTIPLAYER CREATE GAME screen
//

//XSTR:OFF
// bitmaps defs
char *Multi_create_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MultiCreate",			// GR_640
	"2_MultiCreate"		// GR_1024
};

char *Multi_create_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MultiCreate-M",		// GR_640
	"2_MultiCreate-M"		// GR_1024
};

char *Multi_create_loading_fname[GR_NUM_RESOLUTIONS] = {
	"PleaseWait",			// GR_640
	"2_PleaseWait"			// GR_1024
};
//XSTR:ON

#define MULTI_CREATE_NUM_BUTTONS	23

// button defs
#define MC_SHOW_ALL					0
#define MC_SHOW_COOP					1
#define MC_SHOW_TEAM					2
#define MC_SHOW_DOGFIGHT			3
#define MC_PXO_REFRESH				4
#define MC_PILOT_INFO				5
#define MC_SCROLL_LIST_UP			6 
#define MC_SCROLL_LIST_DOWN		7
#define MC_SCROLL_PLAYERS_UP		8
#define MC_SCROLL_PLAYERS_DOWN	9
#define MC_MISSION_FILTER			10
#define MC_CAMPAIGN_FILTER			11
#define MC_CANCEL						12
#define MC_TEAM0						13
#define MC_TEAM1						14
#define MC_KICK						15
#define MC_CLOSE						16
#define MC_SCROLL_INFO_UP			17
#define MC_SCROLL_INFO_DOWN		18
#define MC_HOST_OPTIONS				19
#define MC_HELP						20
#define MC_OPTIONS					21
#define MC_ACCEPT						22


UI_WINDOW Multi_create_window;										// the window object for the create screen
UI_BUTTON Multi_create_player_select_button;						// for selecting players
UI_BUTTON Multi_create_list_select_button;						// for selecting missions/campaigns
int Multi_create_bitmap = -1;												// the background bitmap
UI_SLIDER2 Multi_create_slider;										// for create list

// constants for coordinate look ups
#define MC_X_COORD 0
#define MC_Y_COORD 1
#define MC_W_COORD 2
#define MC_H_COORD 3

ui_button_info Multi_create_buttons[GR_NUM_RESOLUTIONS][MULTI_CREATE_NUM_BUTTONS] = {	
	{ // GR_640
		ui_button_info("MC_00", 32,	129,	36,	158,	0),		// show all missions
		ui_button_info("MC_01", 76,	129,	71,	158,	1),		// show coop missions
		ui_button_info("MC_02", 121,	129,	119,	158,	2),		// show team missions
		ui_button_info("MC_03", 164,	129,	166,	158,	3),		// show dogfight missions
		ui_button_info("MC_04", 399,	129,	229,	130,	4),		// pxo mission refresh
		ui_button_info("MC_05", 567,	123,	467,	132,	5),		// pilot info
		ui_button_info("MC_06", 1,		161,	-1,	-1,	6),		// scroll mission info up
		ui_button_info("MC_08", 1,		304,	-1,	-1,	8),		// scroll mission info down
		ui_button_info("MC_09", 613,	160,	-1,	-1,	9),		// scroll players up
		ui_button_info("MC_10", 613,	202,	-1,	-1,	10),		// scroll players down
		ui_button_info("MC_11", 22,	346,	27,	376,	11),		// mission filter
		ui_button_info("MC_12", 104,	346,	110,	376,	12),		// campaign filter
		ui_button_info("MC_13", 392,	341,	328,	364,	13),		// cancel
		ui_button_info("MC_14", 472,	352,	482,	381,	14),		// team 0	
		ui_button_info("MC_15", 506,	352,	514,	381,	15),		// team 1
		ui_button_info("MC_16", 539,	346,	539,	381,	16),		// kick
		ui_button_info("MC_17", 589,	346,	582,	381,	17),		// close
		ui_button_info("MC_18", 1,		406,	-1,	-1,	18),		// scroll list up
		ui_button_info("MC_19", 1,		447,	-1,	-1,	19),		// scroll list down
		ui_button_info("MC_20", 499,	434,	436,	423,	20),		// host options
		ui_button_info("MC_21", 534,	426,	-1,	-1,	21),		// help
		ui_button_info("MC_22", 534,	452,	-1,	-1,	22),		// options
		ui_button_info("MC_23", 571,	426,	572,	413,	23),		// commit
	}, 	
	{ // GR_1024
		ui_button_info("2_MC_00", 51,		207,	61,	253,	0),		// show all missions
		ui_button_info("2_MC_01", 122,	207,	124,	253,	1),		// show coop missions
		ui_button_info("2_MC_02", 193,	207,	194,	253,	2),		// show team missions
		ui_button_info("2_MC_03", 263,	207,	261,	253,	3),		// show dogfight missions
		ui_button_info("2_MC_04", 639,	207,	479,	218,	4),		// pxo mission refresh
		ui_button_info("2_MC_05", 907,	197,	748,	216,	5),		// pilot info
		ui_button_info("2_MC_06", 1,		258,	-1,	-1,	6),		// scroll mission info up
		ui_button_info("2_MC_08", 1,		487,	-1,	-1,	8),		// scroll mission info down
		ui_button_info("2_MC_09", 981,	256,	-1,	-1,	9),		// scroll players up
		ui_button_info("2_MC_10", 981,	323,	-1,	-1,	10),		// scroll players down
		ui_button_info("2_MC_11",  35,	554,	46,	601,	11),		// mission filter
		ui_button_info("2_MC_12", 166,	554,	174,	601,	12),		// campaign filter
		ui_button_info("2_MC_13", 628,	545,	559,	582,	13),		// cancel
		ui_button_info("2_MC_14", 756,	564,	772,	610,	14),		// team 0	
		ui_button_info("2_MC_15", 810,	564,	826,	610,	15),		// team 1
		ui_button_info("2_MC_16", 862,	554,	872,	610,	16),		// kick
		ui_button_info("2_MC_17", 943,	554,	949,	610,	17),		// close
		ui_button_info("2_MC_18", 1,		649,	-1,	-1,	18),		// scroll list up
		ui_button_info("2_MC_19", 1,		716,	-1,	-1,	19),		// scroll list down
		ui_button_info("2_MC_20", 798,	695,	726,	667,	20),		// host options
		ui_button_info("2_MC_21", 854,	681,	-1,	-1,	21),		// help
		ui_button_info("2_MC_22", 854,	724,	-1,	-1,	22),		// options
		ui_button_info("2_MC_23", 914,	681,	932,	667,	23),		// commit
	}, 	
};

#define MULTI_CREATE_NUM_TEXT				15
UI_XSTR Multi_create_text[GR_NUM_RESOLUTIONS][MULTI_CREATE_NUM_BUTTONS] = {
	{ // GR_640
		{"All",					1256,		36,	158,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_SHOW_ALL].button},
		{"Coop",					1257,		71,	158,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_SHOW_COOP].button},
		{"Team",					1258,		119,	158,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_SHOW_TEAM].button},
		{"Dogfight",			1259,		166,	158,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_SHOW_DOGFIGHT].button},
		{"Refresh Missions",	1260,		229,	130,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_PXO_REFRESH].button},
		{"Pilot Info",			1261,		467,	132,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_PILOT_INFO].button},	
		{"Missions",			1262,		27,	376,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_MISSION_FILTER].button},
		{"Campaigns",			1263,		110,	376,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_CAMPAIGN_FILTER].button},
		{"Cancel",				387,		328,	364,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[0][MC_CANCEL].button},
		{"1",						1264,		482,	381,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_TEAM0].button},
		{"2",						1265,		514,	381,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_TEAM1].button},
		{"Kick",					1266,		539,	381,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[0][MC_KICK].button},
		{"Close",				1508,		582,	381,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[0][MC_CLOSE].button},	
		{"Host Options",		1267,		436,	423,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[0][MC_HOST_OPTIONS].button},		
		{"Commit",				1062,		572,	413,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[0][MC_ACCEPT].button}
	},		
	{ // GR_1024		
		{"All",					1256,		61,	253,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_SHOW_ALL].button},
		{"Coop",					1257,		124,	253,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_SHOW_COOP].button},
		{"Team",					1258,		194,	253,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_SHOW_TEAM].button},
		{"Dogfight",			1259,		261,	253,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_SHOW_DOGFIGHT].button},
		{"Refresh Missions",	1260,		501,	218,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_PXO_REFRESH].button},
		{"Pilot Info",			1261,		814,	216,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_PILOT_INFO].button},	
		{"Missions",			1262,		46,	601,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_MISSION_FILTER].button},
		{"Campaigns",			1263,		174,	601,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_CAMPAIGN_FILTER].button},
		{"Cancel",				387,		559,	582,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[1][MC_CANCEL].button},
		{"1",						1264,		772,	610,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_TEAM0].button},
		{"2",						1265,		826,	610,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_TEAM1].button},
		{"Kick",					1266,		872,	610,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[1][MC_KICK].button},
		{"Close",				1508,		949,	610,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[1][MC_CLOSE].button},	
		{"Host Options",		1267,		755,	683,	UI_XSTR_COLOR_GREEN,	-1,	&Multi_create_buttons[1][MC_HOST_OPTIONS].button},		
		{"Commit",				1062,		932,	667,	UI_XSTR_COLOR_PINK,	-1,	&Multi_create_buttons[1][MC_ACCEPT].button}
	},
};

// squad war checkbox
UI_CHECKBOX	Multi_create_sw_checkbox;
char *Multi_create_sw_checkbox_fname[GR_NUM_RESOLUTIONS] = {
	"MC_SW_00",
	"MC_SW_00",
};
int Multi_create_sw_checkbox_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		6, 75
	},
	{ // GR_1024
		18, 135
	}
};
int Multi_create_sw_checkbox_text[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		6, 95
	},
	{ // GR_640
		18, 155
	},
};

// game information text areas
int Mc_list_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		105, 173, 311, 152
	},
	{ // GR_1024
		62, 275, 600, 262
	}
};

int Mc_players_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		463, 164, 144, 180
	},
	{ // GR_1024
		741, 262, 144, 180
	}
};

int Mc_info_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		47, 405, 363, 59
	},
	{ // GR_1024
		75, 648, 363, 59
	}
};

// mission icon stuff
int Mc_icon_type_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		38, -2		// y is an offset
	},
	{ // GR_1024
		61, -2		// y is an offset
	}
};

int Mc_icon_volition_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		61, -1		// y is an offset
	},
	{ // GR_1024
		98, 1		// y is an offset
	}
};

int Mc_icon_silent_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		72, 0		// y is an offset
	},
	{ // GR_1024
		115, 0		// y is an offset
	}
};

int Mc_icon_valid_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		91, 0		// y is an offset
	},
	{ // GR_1024
		146, 0		// y is an offset
	}
};

// mission/campaign list column areas
int Mc_column1_w[GR_NUM_RESOLUTIONS] = {
	194,		// GR_640
	310		// GR_1024
};

int Mc_column2_w[GR_NUM_RESOLUTIONS] = {
	38,		// GR_640
	61			// GR_1024
};

int Mc_column3_w[GR_NUM_RESOLUTIONS] = {
	77,		// GR_640
	123		// GR_1024
};

int Mc_mission_name_x[GR_NUM_RESOLUTIONS] = {
	105,		// GR_640
	168		// GR_1024
};

int Mc_mission_count_x[GR_NUM_RESOLUTIONS] = {
	314,		// GR_640
	502		// GR_1024
};

int Mc_mission_fname_x[GR_NUM_RESOLUTIONS] = {
	337,		// GR_640
	539		// GR_1024
};

int Mc_create_game_text[GR_NUM_RESOLUTIONS][2] = {
	{13, 116},	// GR_640
	{21, 186}	// GR_1024
};

int Mc_players_text[GR_NUM_RESOLUTIONS][2] = {
	{467, 150},	// GR_640
	{747, 240}	// GR_1024
};

int Mc_team_text[GR_NUM_RESOLUTIONS][2] = {
	{484, 342},	// GR_640
	{774, 547}	// GR_1024
};

int Mc_slider_coords[GR_NUM_RESOLUTIONS][4] = {
	{
		3, 197, 13, 105	// GR_640
	},
	{
		5, 316, 20, 168	// GR_1024
	}
};

char *Mc_slider_bitmap[GR_NUM_RESOLUTIONS] = {
	"slider",
	"2_slider"
};

// player list control thingie defs
#define MULTI_CREATE_PLIST_MAX_DISPLAY		20
int Multi_create_plist_select_flag;									// flag indicating if we have a play selected
short Multi_create_plist_select_id;							// the net address of the currently selected player (for lookup)

// master tracker details
int Multi_create_frame_count;											// framecount
int Multi_create_mt_tried_login;										// attempted to login this server on the MT

// mission filter settings										
int Multi_create_filter;												// what mode we're in

// game/campaign list control defs
int Multi_create_list_max_display[GR_NUM_RESOLUTIONS] = {
	15,		// GR_640
	26			// GR_1024
};


int Multi_create_list_count;
int Multi_create_list_mode;											// 0 == mission mode, 1 == campaign mode
int Multi_create_list_start;											// where to start displaying from
int Multi_create_list_select;											// which item is currently highlighted
int Multi_create_files_loaded;

SCP_vector<multi_create_info> Multi_create_mission_list;
SCP_vector<multi_create_info> Multi_create_campaign_list;

// LOCAL function definitions
void multi_create_check_buttons();
void multi_create_button_pressed(int n);
void multi_create_init_as_server();
void multi_create_init_as_client();
void multi_create_do_netstuff();
void multi_create_plist_scroll_up();
void multi_create_plist_scroll_down();
void multi_create_plist_process();
void multi_create_plist_blit_normal();
void multi_create_plist_blit_team();
void multi_create_list_scroll_up();
void multi_create_list_scroll_down();
void multi_create_list_do();
void multi_create_list_select_item(int n);
void multi_create_list_blit_icons(int list_index, int y_start);
void multi_create_accept_hit();
void multi_create_draw_filter_buttons();
void multi_create_set_selected_team(int team);
short multi_create_get_mouse_id();
int multi_create_ok_to_commit();
int multi_create_verify_cds();
void multi_create_refresh_pxo();
void multi_create_sw_clicked();

// since we can selectively filter out mission/campaign types we always need to map a selected index (which is relative 
// to the displayed list), to an absolute index (which is relative to the total file list - some of which may filtered out)
void multi_create_select_to_filename(int select_index,char *filename);
int multi_create_select_to_index(int select_index);

int Multi_create_should_show_popup = 0;

bool Multi_create_sort_mode = false;		// default to mission name sorting, "true" is mission filename sorting


// sorting function to sort mission lists.. Basic sorting on mission name
bool multi_create_sort_func(const multi_create_info &m1, const multi_create_info &m2)
{
	if (Multi_create_filter != MISSION_TYPE_MULTI) {
		if ( (m1.flags & Multi_create_filter) && !(m2.flags & Multi_create_filter) ) {
			return false;
		} else if ( (m2.flags & Multi_create_filter) && !(m1.flags & Multi_create_filter) ) {
			return true;
		}
	}

	int test = 0;

	if (Multi_create_sort_mode) {
		test = stricmp(m1.filename, m2.filename);
	} else {
		test = stricmp(m1.name, m2.name);
	}

	if (test < 0) {
		return true;
	} else if (test > 0) {
		return false;
	} else {
		return true;
	}
}

void multi_create_list_sort(int mode)
{
	bool sort_missions = false;
	bool sort_campaigns = false;
	char selected_name[255];
	int new_index = -1;

	if (Multi_create_list_count < 0) {
		return;
	}

	switch (mode) {
		case MULTI_CREATE_SHOW_MISSIONS:
			sort_missions = true;
			break;

		case MULTI_CREATE_SHOW_CAMPAIGNS:
			sort_campaigns = true;
			break;

		default:
			sort_missions = true;
			sort_campaigns = true;
			break;
	}

	// save our current selected item so that we can restore that one after sorting
	multi_create_select_to_filename(Multi_create_list_select, selected_name);

	if (sort_missions) {
		std::sort( Multi_create_mission_list.begin(), Multi_create_mission_list.end(), multi_create_sort_func);

		for (size_t idx = 0; idx < Multi_create_mission_list.size(); idx++) {
			if ( !strcmp(selected_name, Multi_create_mission_list[idx].filename) ) {
				new_index = (int)idx;
				break;
			}
		}
	}

	if (sort_campaigns) {
		std::sort( Multi_create_campaign_list.begin(), Multi_create_campaign_list.end(), multi_create_sort_func);

		for (size_t idx = 0; idx < Multi_create_campaign_list.size(); idx++) {
			if ( !strcmp(selected_name, Multi_create_campaign_list[idx].filename) ) {
				new_index = (int)idx;
				break;
			}
		}
	}

	multi_create_list_select_item(new_index);
}

void multi_create_setup_list_data(int mode)
{	
	int idx,should_sort,switched_modes;
	
	// set the current mode
	should_sort = 0;
	switched_modes = 0;
	if ( (Multi_create_list_mode != mode) && (mode != -1) ) {
		Multi_create_list_mode = mode;	
		switched_modes = 1;
	} else if (mode == -1) {
		switched_modes = 1;
	}

	// get the mission count based upon the filter selected
	if(Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS){		
		switch (Multi_create_filter) {
			case MISSION_TYPE_MULTI:
				Multi_create_list_count = (int)Multi_create_mission_list.size();

				// if we switched modes and we have more than 0 items, sort them
				if (switched_modes && (Multi_create_list_count > 0)){
					should_sort = 1;
				}
				break;

			default : 
				Multi_create_list_count = 0;

				// find all missions which match 
				for (idx = 0; idx < (int)Multi_create_mission_list.size(); idx++) {
					if(Multi_create_mission_list[idx].flags & Multi_create_filter){
						Multi_create_list_count++;
					}
				}

				// if we switched modes and we have more than 0 items, sort them
				if (switched_modes && (Multi_create_list_count > 0)){
					should_sort = 1;
				}

				break;
		}
	} else if(Multi_create_list_mode == MULTI_CREATE_SHOW_CAMPAIGNS){
		switch (Multi_create_filter) {
			case MISSION_TYPE_MULTI:
				Multi_create_list_count = (int)Multi_create_campaign_list.size();

				// if we switched modes and we have more than 0 items, sort them
				if (switched_modes && (Multi_create_list_count > 0)){
					should_sort = 1;
				}
				break;

			default :
				Multi_create_list_count = 0;

				// find all missions which match 
				for (idx = 0; idx < (int)Multi_create_campaign_list.size(); idx++) {
					if(Multi_create_campaign_list[idx].flags & Multi_create_filter){
						Multi_create_list_count++;
					}
				}

				// if we switched modes and we have more than 0 items, sort them
				if(switched_modes && (Multi_create_list_count > 0)){
					should_sort = 1;
				}
				break;
			}
	}

	// reset the list start and selected indices
	Multi_create_list_start = 0;
	Multi_create_list_select = -1;
	multi_create_list_select_item(Multi_create_list_start);	

	// sort the list of missions if necessary
	if (should_sort) {
		multi_create_list_sort(mode);
	}

	// reset the slider
	Multi_create_slider.set_numberItems(Multi_create_list_count > Multi_create_list_max_display[gr_screen.res] ? Multi_create_list_count-Multi_create_list_max_display[gr_screen.res] : 0);
}

void multi_create_game_init()
{
	int idx;
	ui_button_info *b;
	
	// now make sure to initialze various netgame stuff based upon whether we're on a standalone or not
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_create_init_as_server();
	} else {
		multi_create_init_as_client();
	}

	// initialize the player list data		
	Multi_create_plist_select_flag = 0;
	Multi_create_plist_select_id = -1;	

	// create the interface window
	Multi_create_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_create_window.set_mask_bmap(Multi_create_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_create_bitmap = bm_load(Multi_create_bitmap_fname[gr_screen.res]);
	if(Multi_create_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}

	// close any previous existing instances of the chatbox and create a new one
	chatbox_close();
	chatbox_create();

	// load the help overlay 
	help_overlay_load(MULTI_CREATE_OVERLAY);
	help_overlay_set_state(MULTI_CREATE_OVERLAY, 0);

	// initialize the common notification messaging
	multi_common_notify_init();		

	// use the common interface palette
	multi_common_set_palette();

	// create the interface buttons
	for(idx=0; idx<MULTI_CREATE_NUM_BUTTONS; idx++){
		b = &Multi_create_buttons[gr_screen.res][idx];
	
		// create the object		
		b->button.create(&Multi_create_window, "", b->x, b->y, 1, 1, ((idx == MC_SCROLL_LIST_UP) || (idx == MC_SCROLL_LIST_DOWN)), 1);

		// set the sound to play when highlighted
		b->button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		b->button.set_bmaps(b->filename);

		// set the hotspot
		b->button.link_hotspot(b->hotspot);		

		// some special case stuff for the pxo refresh button
		if(idx == MC_PXO_REFRESH){			
			// if not a PXO game, or if I'm not a server disable and hide the button
			if(!MULTI_IS_TRACKER_GAME || !MULTIPLAYER_MASTER){
				b->button.hide();
				b->button.disable();
			}			
		}
	}	

	// create xstrs
	for(idx=0; idx<MULTI_CREATE_NUM_TEXT; idx++){
		Multi_create_window.add_XSTR(&Multi_create_text[gr_screen.res][idx]);
	}

	// if this is a PXO game, enable the squadwar checkbox	
	Multi_create_sw_checkbox.create(&Multi_create_window, "", Multi_create_sw_checkbox_coords[gr_screen.res][0], Multi_create_sw_checkbox_coords[gr_screen.res][1], 0);
	Multi_create_sw_checkbox.set_bmaps(Multi_create_sw_checkbox_fname[gr_screen.res], 6, 0);
	if(!MULTI_IS_TRACKER_GAME){
		Multi_create_sw_checkbox.hide();
		Multi_create_sw_checkbox.disable();
	}

	// initialize the mission type filtering mode
	Multi_create_filter = MISSION_TYPE_MULTI;

	// initialize the list mode, and load in a list
	Multi_create_mission_list.clear();
	Multi_create_campaign_list.clear();

	Multi_create_list_mode = MULTI_CREATE_SHOW_MISSIONS;
	Multi_create_list_start = -1;
	Multi_create_list_select = -1;
	Multi_create_list_count = 0;

	Multi_create_slider.create(&Multi_create_window, Mc_slider_coords[gr_screen.res][MC_X_COORD], Mc_slider_coords[gr_screen.res][MC_Y_COORD], Mc_slider_coords[gr_screen.res][MC_W_COORD],Mc_slider_coords[gr_screen.res][MC_H_COORD], -1, Mc_slider_bitmap[gr_screen.res], &multi_create_list_scroll_up, &multi_create_list_scroll_down, NULL);

	// create the player list select button
	Multi_create_player_select_button.create(&Multi_create_window, "", Mc_players_coords[gr_screen.res][MC_X_COORD], Mc_players_coords[gr_screen.res][MC_Y_COORD], Mc_players_coords[gr_screen.res][MC_W_COORD], Mc_players_coords[gr_screen.res][MC_H_COORD], 0, 1);
	Multi_create_player_select_button.hide();		
	
	// create the mission/campaign list select button
	Multi_create_list_select_button.create(&Multi_create_window, "", Mc_list_coords[gr_screen.res][MC_X_COORD], Mc_list_coords[gr_screen.res][MC_Y_COORD], Mc_list_coords[gr_screen.res][MC_W_COORD], Mc_list_coords[gr_screen.res][MC_H_COORD], 0, 1);
	Multi_create_list_select_button.hide();	

	// set hotkeys for a couple of things.
	Multi_create_buttons[gr_screen.res][MC_ACCEPT].button.set_hotkey(KEY_CTRLED+KEY_ENTER);	

	// init some master tracker stuff
	Multi_create_frame_count = 0;
	Multi_create_mt_tried_login = 0;

	// remove campaign flags
	Game_mode &= ~(GM_CAMPAIGN_MODE);
	
	// send any pilots as appropriate
	multi_data_send_my_junk();

	Multi_create_files_loaded = 0;
}

void multi_create_game_do()
{
	//DTP CHECK ALMISSION FLAG HERE AND SKIP THE BITMAP LOADING PROGRESS 
	//SINCE WE ALREADY HAVE A MISSION SELECTED IF THIS MISSION IS A VALID MULTIPLAYER MISSION
	//IF NOT A VALID MULTIPLAYER MISSION CONTINUE LOADING, MAYBE CALL POPUP.
	if ((Cmdline_almission) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)) {	//
		multi_create_list_do(); //uhm here because off, hehe, my mind is failing right now

		// DTP Var section for the is mission multi player Check.

		char mission_name[NAME_LENGTH+1];
		int flags;
		char *filename; 
		filename = cf_add_ext( Cmdline_almission, FS_MISSION_FILE_EXT ); //DTP ADD EXTENSION needed next line
		flags = mission_parse_is_multi(filename, mission_name); //DTP flags will set if mission is multi

		if (flags) { //only continue if mission is multiplayer mission
			netgame_info *ng; 
			ng = &Netgame;

			char almissionname[256]; // needed, for the strncpy below
			strncpy(almissionname, Cmdline_almission,MAX_FILENAME_LEN); //DTP; copying name from cmd_almission line

			Netgame.options.respawn = 99; //override anything //for debugging, i often forget this.
			ng->respawn = Netgame.options.respawn;

			Netgame.campaign_mode = MP_SINGLE_MISSION; //multiplayer single mission. meaning Single mission, not single player

			strncpy(Game_current_mission_filename,almissionname,MAX_FILENAME_LEN ); // copying almissionname to Game_current_mission_filename
			strncpy(Netgame.mission_name,almissionname,MAX_FILENAME_LEN);// copying almission name to netgame.mission_name

			Multi_sync_mode = MULTI_SYNC_PRE_BRIEFING; //DTP must be set before a call to gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC) is done as it is below.
			gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);//DTP STart game

			Cmdline_almission = NULL; // we don't want to autoload anymore do we, we will be able to quit. halleluja. Startgame has already been disabled, so no need to turn of "Cmdline_start_netgame"
			return;	// we don't need to check or set regarding ships/weapons anything as we are already progressing into mission so return
		}
		else {
			popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR(" Not a multi player-mission",9999)); //DTP startgame popup pilot error
			gamesnd_play_iface(SND_GENERAL_FAIL);
			Cmdline_almission = NULL; //DTP make sure this gets nullified.
		
		}
	}
	
	int player_index;
	char *loading_str = XSTR("Loading", 1336);
	int str_w, str_h;

	// set this if we want to show the pilot info popup
	Multi_create_should_show_popup = 0;

	// first thing is to load the files
	if ( !Multi_create_files_loaded ) {
		// if I am a client, send a list request to the server for the missions
		if ( MULTIPLAYER_CLIENT ) {
			send_mission_list_request( MISSION_LIST_REQUEST );
		} else {
			int loading_bitmap;

			loading_bitmap = bm_load(Multi_create_loading_fname[gr_screen.res]);

			// draw the background, etc
			gr_reset_clip();
			GR_MAYBE_CLEAR_RES(Multi_create_bitmap);
			if(Multi_create_bitmap != -1){
				gr_set_bitmap(Multi_create_bitmap);
				gr_bitmap(0, 0);
			}
			chatbox_render();
			if ( loading_bitmap > -1 ){
				gr_set_bitmap(loading_bitmap);
			}
			gr_bitmap( Please_wait_coords[gr_screen.res][MC_X_COORD], Please_wait_coords[gr_screen.res][MC_Y_COORD] );

			// draw "Loading" on it
			gr_set_color_fast(&Color_normal);
			gr_set_font(FONT2);
			gr_get_string_size(&str_w, &str_h, loading_str);
			gr_string((gr_screen.max_w - str_w) / 2, (gr_screen.max_h - str_h) / 2, loading_str, false);
			gr_set_font(FONT1);

			gr_flip();

			multi_create_list_load_missions();
			multi_create_list_load_campaigns();

			// if this is a tracker game, validate missions
			if (MULTI_IS_TRACKER_GAME) {
				multi_update_valid_missions();
				fs2netd_update_ban_list();
			}

			// update the file list
			multi_create_setup_list_data(MULTI_CREATE_SHOW_MISSIONS);
			// the above function doesn't sort initially, so we need this to take care of it
			multi_create_list_sort(MULTI_CREATE_SHOW_MISSIONS);
			// the sort function probably changed our selection, but here we always need it to zero
			multi_create_list_select_item(0);
		}

		// don't bother setting netgame state if ont the server
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			Netgame.game_state = NETGAME_STATE_FORMING;
			send_netgame_update_packet();

			// tell FS2NetD about our game as well
			if (MULTI_IS_TRACKER_GAME) {
				fs2netd_gameserver_start();
			}
		}	

		// if we're on the standalone we have to tell him that we're now in the host setup screen	
		Net_player->state = NETPLAYER_STATE_HOST_SETUP;	
		send_netplayer_update_packet();

		Multi_create_files_loaded = 1;
	}

	int k = chatbox_process();
	k = Multi_create_window.process(k,0);

	switch(k){	
		// same as the cancel button
		case KEY_ESC: {
			if ( help_overlay_active(MULTI_CREATE_OVERLAY) ) {
				help_overlay_set_state(MULTI_CREATE_OVERLAY, 0);
			} else {		
				gamesnd_play_iface(SND_USER_SELECT);		
				multi_quit_game(PROMPT_HOST);		
			}

			break;
		}

		case KEY_CTRLED | KEY_SHIFTED | KEY_S:
			Multi_create_sort_mode = !Multi_create_sort_mode;
			multi_create_list_sort(Multi_create_list_mode);
			break;
	}	

	if ( mouse_down(MOUSE_LEFT_BUTTON) ) {
		help_overlay_set_state(MULTI_CREATE_OVERLAY, 0);
	}

	// process any button clicks
	multi_create_check_buttons();

	// do any network related stuff
	multi_create_do_netstuff(); 

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Multi_create_bitmap);
	if(Multi_create_bitmap != -1){
		gr_set_bitmap(Multi_create_bitmap);
		gr_bitmap(0,0);
	}

	// if we're not in team vs. team mode, don't draw the team buttons
	if(!(Netgame.type_flags & NG_TYPE_TEAM)){
		Multi_create_buttons[gr_screen.res][MC_TEAM0].button.hide();
		Multi_create_buttons[gr_screen.res][MC_TEAM1].button.hide();
		Multi_create_buttons[gr_screen.res][MC_TEAM0].button.disable();
		Multi_create_buttons[gr_screen.res][MC_TEAM1].button.disable();
	} else {
		Multi_create_buttons[gr_screen.res][MC_TEAM0].button.enable();
		Multi_create_buttons[gr_screen.res][MC_TEAM1].button.enable();
		Multi_create_buttons[gr_screen.res][MC_TEAM0].button.unhide();
		Multi_create_buttons[gr_screen.res][MC_TEAM1].button.unhide();				
	}		

	// draw the window itself
	Multi_create_window.draw();

	gr_set_color_fast(&Color_normal);

	// draw Create Game text
	gr_string(Mc_create_game_text[gr_screen.res][MC_X_COORD], Mc_create_game_text[gr_screen.res][MC_Y_COORD], XSTR("Create Game", 1268));

	// draw players text
	gr_string(Mc_players_text[gr_screen.res][MC_X_COORD], Mc_players_text[gr_screen.res][MC_Y_COORD], XSTR("Players", 1269));

	// draw players text
	gr_string(Mc_team_text[gr_screen.res][MC_X_COORD], Mc_team_text[gr_screen.res][MC_Y_COORD], XSTR("Team", 1258));

	// process and display the player list	
	// NOTE : this must be done before the buttons are checked to insure that a player hasn't dropped 
	multi_create_plist_process();
	if(Netgame.type_flags & NG_TYPE_TEAM){
		multi_create_plist_blit_team();
	} else {
		multi_create_plist_blit_normal();
	}

	// process and display the game/campaign list
	multi_create_list_do();
	
	// draw the correct mission filter button
	multi_create_draw_filter_buttons();

	// display any text in the info area
	multi_common_render_text();

	// display any pending notification messages
	multi_common_notify_do();	
	
	// force the correct mission/campaign button to light up
	if( Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS ){
		Multi_create_buttons[gr_screen.res][MC_MISSION_FILTER].button.draw_forced(2);
	} else {
		Multi_create_buttons[gr_screen.res][MC_CAMPAIGN_FILTER].button.draw_forced(2);
	}

	// force draw the closed button if it is toggled on
	if(Netgame.flags & NG_FLAG_TEMP_CLOSED){
		Multi_create_buttons[gr_screen.res][MC_CLOSE].button.draw_forced(2);
	}

	// process and show the chatbox thingie	
	chatbox_render();

	// draw tooltips
	Multi_create_window.draw_tooltip();

	// display the voice status indicator
	multi_common_voice_display_status();

	// blit the help overlay if necessary
	help_overlay_maybe_blit(MULTI_CREATE_OVERLAY);

	// test code
	if(MULTI_IS_TRACKER_GAME){
		if(Netgame.type_flags & NG_TYPE_SW){
			gr_set_color_fast(&Color_bright);
		} else {
			gr_set_color_fast(&Color_normal);
		}
		gr_string(Multi_create_sw_checkbox_text[gr_screen.res][0], Multi_create_sw_checkbox_text[gr_screen.res][1], "SquadWar");
	}

	// flip the buffer
	gr_flip();		
		
	// if we're supposed to show the pilot info popup, do it now
	if(Multi_create_should_show_popup){		
		// get the player index and address of the player item the mouse is currently over
		if(Multi_create_plist_select_flag){		
			player_index = find_player_id(Multi_create_plist_select_id);
			if(player_index != -1){			
				multi_pinfo_popup(&Net_players[player_index]);
			}
		}
	}

	// increment the frame count
	Multi_create_frame_count++;	
}

void multi_create_game_close()
{
	// unload any bitmaps
	if(!bm_unload(Multi_create_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_create_bitmap_fname[gr_screen.res]));
	}		

	// unload the help overlay
	help_overlay_unload(MULTI_CREATE_OVERLAY);
	
	// destroy the chatbox
	// chatbox_close();
	
	// destroy the UI_WINDOW
	Multi_create_window.destroy();
}

void multi_create_check_buttons()
{
	int idx;
	for(idx=0;idx<MULTI_CREATE_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Multi_create_buttons[gr_screen.res][idx].button.pressed()){
			multi_create_button_pressed(idx);
			break;
		}
	}

	// if the squad war checkbox was clicked
	if(Multi_create_sw_checkbox.changed()){
		multi_create_sw_clicked();
	}
}

void multi_create_button_pressed(int n)
{
	int idx;
	
	switch(n){
	case MC_CANCEL :
		gamesnd_play_iface(SND_USER_SELECT);		
		multi_quit_game(PROMPT_HOST);		
		break;
	case MC_ACCEPT :	
		// if valid commit conditions have not been met
		if(!multi_create_ok_to_commit()){
			break;
		}

		// commit
		multi_create_accept_hit();		
		break;

	// help button
	case MC_HELP :
		if(!help_overlay_active(MULTI_CREATE_OVERLAY)){
			help_overlay_set_state(MULTI_CREATE_OVERLAY,1);
		} else {
			help_overlay_set_state(MULTI_CREATE_OVERLAY,0);
		}
		break;

	// scroll the info text box up
	case MC_SCROLL_INFO_UP:
		multi_common_scroll_text_up();
		break;

	// scroll the info text box down
	case MC_SCROLL_INFO_DOWN:
		multi_common_scroll_text_down();
		break;

	// scroll the player list up
	case MC_SCROLL_PLAYERS_UP:
		multi_create_plist_scroll_up();
		break;

	// scroll the player list down
	case MC_SCROLL_PLAYERS_DOWN:
		multi_create_plist_scroll_down();
		break;

	// scroll the game/campaign list up
	case MC_SCROLL_LIST_UP:
		multi_create_list_scroll_up();
		Multi_create_slider.forceUp();	// move slider up
		break;

	// scroll the game/campaign list down
	case MC_SCROLL_LIST_DOWN:
		multi_create_list_scroll_down();
		Multi_create_slider.forceDown();	// move slider down
		break;

	// go to the options screen
	case MC_OPTIONS:		
		gamesnd_play_iface(SND_USER_SELECT);
		gameseq_post_event(GS_EVENT_OPTIONS_MENU);
		break;	

	// show all missions
	case MC_SHOW_ALL:
		if(Multi_create_filter != MISSION_TYPE_MULTI){
			gamesnd_play_iface(SND_USER_SELECT);
			Multi_create_filter = MISSION_TYPE_MULTI;
			multi_create_setup_list_data(Multi_create_list_mode);						// update the file list
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	// show cooperative missions
	case MC_SHOW_COOP:
		if(Multi_create_filter != MISSION_TYPE_MULTI_COOP){
			gamesnd_play_iface(SND_USER_SELECT);
			Multi_create_filter = MISSION_TYPE_MULTI_COOP;			
			multi_create_setup_list_data(Multi_create_list_mode);						// update the file list
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	// show team vs. team missions
	case MC_SHOW_TEAM:
		if(Multi_create_filter != MISSION_TYPE_MULTI_TEAMS){
			gamesnd_play_iface(SND_USER_SELECT);
			Multi_create_filter = MISSION_TYPE_MULTI_TEAMS;	
			multi_create_setup_list_data(Multi_create_list_mode);						// update the file list
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;	

	// show dogfight missions
	case MC_SHOW_DOGFIGHT:
		if (Multi_create_filter != MISSION_TYPE_MULTI_DOGFIGHT){
			gamesnd_play_iface(SND_USER_SELECT);
			Multi_create_filter = MISSION_TYPE_MULTI_DOGFIGHT;
			multi_create_setup_list_data(Multi_create_list_mode);						// update the file list
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	// toggle temporary netgame closed on/off
	case MC_CLOSE:
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			Netgame.flags ^= NG_FLAG_TEMP_CLOSED;
		} else {
			Netgame.options.flags |= MLO_FLAG_TEMP_CLOSED;
			multi_options_update_netgame();
		}
		gamesnd_play_iface(SND_USER_SELECT);
		break;

	// kick the currently selected player (if possible)
	case MC_KICK:
		// lookup the player at the specified index		
		if(Multi_create_plist_select_flag){		 
			idx = find_player_id(Multi_create_plist_select_id);
			// kick him - but don't ban him
			if(idx != -1){			
				multi_kick_player(idx,0);				
			}
		}
		break;
			
	// switch to individual mission mode and load in a list
	case MC_MISSION_FILTER:
		if(Multi_create_list_mode != MULTI_CREATE_SHOW_MISSIONS){
			Netgame.campaign_mode = MP_SINGLE_MISSION;

			gamesnd_play_iface(SND_USER_SELECT);												
			
			// update the file list
			multi_create_setup_list_data(MULTI_CREATE_SHOW_MISSIONS);						
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	// switch to campaign mode and load in a list
	case MC_CAMPAIGN_FILTER:		
		// switch off squad war
		Multi_create_sw_checkbox.set_state(0);
		Netgame.type_flags = NG_TYPE_COOP;

		if(Multi_create_list_mode != MULTI_CREATE_SHOW_CAMPAIGNS){
			Netgame.campaign_mode = MP_CAMPAIGN;

			gamesnd_play_iface(SND_USER_SELECT);			
			
			// update the file list
			multi_create_setup_list_data(MULTI_CREATE_SHOW_CAMPAIGNS);						
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	// attempt to set the selected player's team
	case MC_TEAM0:
		multi_create_set_selected_team(0);
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			multi_team_send_update();
		}
		break;

	// attempt to set the selected player's team
	case MC_TEAM1:
		multi_create_set_selected_team(1);
		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			multi_team_send_update();
		}
		break;	

	// popup the pilot info dialog for the currently selected pilot (will occur at the end of the frame)
	case MC_PILOT_INFO:
		Multi_create_should_show_popup = 1;
		break;

	// go to the host options screen
	case MC_HOST_OPTIONS:
		gamesnd_play_iface(SND_USER_SELECT);
		gameseq_post_event(GS_EVENT_MULTI_HOST_OPTIONS);
		break;

	// refresh PXO file list
	case MC_PXO_REFRESH:		
		if(!MULTI_IS_TRACKER_GAME){
			break;
		}
		multi_create_refresh_pxo();		
		break;

	default :
		gamesnd_play_iface(SND_GENERAL_FAIL);
		multi_common_add_notify(XSTR("Not implemented yet!",760));		
		break;
	}
}

// do stuff like pinging servers, sending out requests, etc
// # Kazan # -- This apparently DOES NOT Apply to Standalones
void multi_create_do_netstuff()
{

}

// if not on a standalone
void multi_create_init_as_server()
{
	// set me up as the host and master
	Net_player->flags |= (NETINFO_FLAG_AM_MASTER | NETINFO_FLAG_GAME_HOST);		
}

// if on a standalone
void multi_create_init_as_client()
{
	Net_player->flags |= NETINFO_FLAG_GAME_HOST;	
}

// scroll up through the player list
void multi_create_plist_scroll_up()
{	
	gamesnd_play_iface(SND_GENERAL_FAIL);
}

// scroll down through the player list
void multi_create_plist_scroll_down()
{	
	gamesnd_play_iface(SND_GENERAL_FAIL);
}

void multi_create_plist_process()
{
	int test_count,idx,player_index;
	
	// first determine if there are 0 players in the game. This should never happen since the host is _always_ in the game
	test_count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			test_count++;
		}
	}
	if(test_count <= 0){
		return;
	}
	
	// if we had a selected item but that player has left, select myself instead
	if(Multi_create_plist_select_flag){
		player_index = find_player_id(Multi_create_plist_select_id);
		if(player_index == -1){
			Multi_create_plist_select_id = Net_player->player_id;
		}
	} else {
		Multi_create_plist_select_flag = 1;
		Multi_create_plist_select_id = Net_player->player_id;		
	}	
		
	// if the player has clicked somewhere in the player list area
	if(Multi_create_player_select_button.pressed()){				
		short player_id;

		// get the player index and address of the player item the mouse is currently over
		player_id = multi_create_get_mouse_id();
		player_index = find_player_id(player_id);
		if(player_index != -1){
			Multi_create_plist_select_flag = 1;
			Multi_create_plist_select_id = player_id;			
		} 
	}		
}

void multi_create_plist_blit_normal()
{
	int idx;		
	char str[CALLSIGN_LEN+5];
	int y_start = Mc_players_coords[gr_screen.res][MC_Y_COORD];	
	int total_offset;

	// display all the players	
	for(idx=0;idx<MAX_PLAYERS;idx++){		
		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			// total x offset
			total_offset = 0;

			// highlight him if he's the host			
			if(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST){
				if(Multi_create_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active_hi);
				} else {
					gr_set_color_fast(&Color_bright);
				}
			} else {
				if(Multi_create_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active);
				} else {
					gr_set_color_fast(&Color_text_normal);
				}
			}
			
			// optionally draw his CD status
			if((Net_players[idx].flags & NETINFO_FLAG_HAS_CD) && (Multi_common_icons[MICON_CD] != -1)){
				gr_set_bitmap(Multi_common_icons[MICON_CD]);
				gr_bitmap(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset,y_start - 1);

				total_offset += Multi_common_icon_dims[MICON_CD][0] + 1;
			}			
			
			// make sure the string will fit, then display it
			strcpy_s(str,Net_players[idx].m_player->callsign);
			if(Net_players[idx].flags & NETINFO_FLAG_OBSERVER){
				strcat_s(str,XSTR("(O)",787));  // [[ Observer ]]
			}
			gr_force_fit_string(str,CALLSIGN_LEN,Mc_players_coords[gr_screen.res][MC_W_COORD] - total_offset);
			gr_string(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset,y_start,str);

			y_start += 10;			
		}
	}		
}

void multi_create_plist_blit_team()
{
	int idx;		
	char str[CALLSIGN_LEN+1];
	int y_start = Mc_players_coords[gr_screen.res][MC_Y_COORD];	
	int total_offset;

	// display all the red players first
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 0)){
			// reset total offset
			total_offset = 0;

			// highlight him if he's the host			
			if(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST){
				if(Multi_create_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active_hi);

					// be sure to blit the correct team button 
					Multi_create_buttons[gr_screen.res][MC_TEAM0].button.draw_forced(2);
				} else {
					gr_set_color_fast(&Color_bright);
				}
			} else {
				if(Multi_create_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active);

					// be sure to blit the correct team button 
					Multi_create_buttons[gr_screen.res][MC_TEAM0].button.draw_forced(2);
				} else {
					gr_set_color_fast(&Color_text_normal);
				}
			}

			// optionally draw his CD status
			if((Net_players[idx].flags & NETINFO_FLAG_HAS_CD) && (Multi_common_icons[MICON_CD] != -1)){
				gr_set_bitmap(Multi_common_icons[MICON_CD]);				
				gr_bitmap(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset,y_start - 1);

				total_offset += Multi_common_icon_dims[MICON_CD][0] + 1;
			}			

			// blit the red team indicator			
			if(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN){
				if(Multi_common_icons[MICON_TEAM0_SELECT] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM0_SELECT]);
					gr_bitmap(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset, y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM0_SELECT][0] + 1;			
				}
			} else {
				if(Multi_common_icons[MICON_TEAM0] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM0]);
					gr_bitmap(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset, y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM0][0] + 1;			
				}				
			}						

			// make sure the string will fit
			strcpy_s(str,Net_players[idx].m_player->callsign);
			if(Net_players[idx].flags & NETINFO_FLAG_OBSERVER){
				strcat_s(str,XSTR("(O)",787));
			}
			gr_force_fit_string(str,CALLSIGN_LEN,Mc_players_coords[gr_screen.res][MC_W_COORD] - total_offset);

			// display him in the correct half of the list depending on his team
			gr_string(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset,y_start,str);
			y_start += 10;
		}
	}	
	
	// display all the green players next
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 1)){
			// reset total offset
			total_offset = 0;

			// highlight him if he's the host			
			if(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST){
				if(Multi_create_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active_hi);

					// be sure to blit the correct team button 
					Multi_create_buttons[gr_screen.res][MC_TEAM1].button.draw_forced(2);
				} else {
					gr_set_color_fast(&Color_bright);
				}
			} else {
				if(Multi_create_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active);

					// be sure to blit the correct team button 
					Multi_create_buttons[gr_screen.res][MC_TEAM1].button.draw_forced(2);
				} else {
					gr_set_color_fast(&Color_text_normal);
				}
			}

			// optionally draw his CD status
			if((Net_players[idx].flags & NETINFO_FLAG_HAS_CD) && (Multi_common_icons[MICON_CD] != -1)){
				gr_set_bitmap(Multi_common_icons[MICON_CD]);
				gr_bitmap(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset,y_start - 1);

				total_offset += Multi_common_icon_dims[MICON_CD][0] + 1;
			}			

			// blit the red team indicator			
			if(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN){
				if(Multi_common_icons[MICON_TEAM1_SELECT] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM1_SELECT]);
					gr_bitmap(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset, y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM1_SELECT][0] + 1;
				}				
			} else {
				if(Multi_common_icons[MICON_TEAM1] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM1]);
					gr_bitmap(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset, y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM1][0] + 1;
				}
			}

			// make sure the string will fit
			strcpy_s(str,Net_players[idx].m_player->callsign);
			if(Net_players[idx].flags & NETINFO_FLAG_OBSERVER){
				strcat_s(str,XSTR("(O)",787));
			}
			gr_force_fit_string(str,CALLSIGN_LEN,Mc_players_coords[gr_screen.res][MC_W_COORD] - total_offset);

			// display him in the correct half of the list depending on his team
			gr_string(Mc_players_coords[gr_screen.res][MC_X_COORD] + total_offset,y_start,str);
			y_start += 10;
		}
	}			
}

void multi_create_list_scroll_up()
{
	if(Multi_create_list_start > 0){
		Multi_create_list_start--;		

		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

void multi_create_list_scroll_down()
{
	if((Multi_create_list_count - Multi_create_list_start) > Multi_create_list_max_display[gr_screen.res]){
		Multi_create_list_start++;		

		gamesnd_play_iface(SND_SCROLL);
	} else {
		gamesnd_play_iface(SND_GENERAL_FAIL);
	}
}

// gets a list of multiplayer misisons
void multi_create_list_load_missions()
{
	char *fname, mission_name[NAME_LENGTH+1];
	char wild_card[10];
	int file_count, idx;
	char **file_list = NULL;

	Multi_create_mission_list.clear();

	memset( wild_card, 0, sizeof(wild_card) );
	snprintf(wild_card, sizeof(wild_card) - 1, "*%s", FS_MISSION_FILE_EXT);

	file_list = (char**) vm_malloc( sizeof(char*) * 1024 );

	if (file_list == NULL) {
		return;
	}

	file_count = cf_get_file_list(1024, file_list, CF_TYPE_MISSIONS, wild_card);

	// maybe create a standalone dialog
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_create_gen_dialog("Loading missions");
		std_gen_set_text("Mission:", 1);
	}

	for (idx = 0; idx < file_count; idx++) {
		int flags,max_players;
		char *filename;
		uint m_respawn;

		fname = file_list[idx];
		
		// tack on any necessary file extension
		filename = cf_add_ext( fname, FS_MISSION_FILE_EXT );

		if (Game_mode & GM_STANDALONE_SERVER) {			
			std_gen_set_text(filename, 2);
		}

		flags = mission_parse_is_multi(filename, mission_name);		

		// if the mission is a multiplayer mission, then add it to the mission list
		if ( flags ) {
			max_players = mission_parse_get_multi_mission_info( filename );				
			m_respawn = The_mission.num_respawns;

			multi_create_info mcip;

			strcpy_s(mcip.filename, filename );
			strcpy_s(mcip.name, mission_name );
			mcip.flags = flags;
			mcip.respawn = m_respawn;
			mcip.max_players = (ubyte)max_players;

			// get any additional information for possibly builtin missions
			fs_builtin_mission *fb = game_find_builtin_mission(filename);
			if(fb != NULL){					
			}

			Multi_create_mission_list.push_back( mcip );
		}
	}

	if (file_list) {
		for (idx = 0; idx < file_count; idx++) {
			if (file_list[idx]) {
				vm_free(file_list[idx]);
				file_list[idx] = NULL;
			}
		}

		vm_free(file_list);
		file_list = NULL;
	}

	Multi_create_slider.set_numberItems(int(Multi_create_mission_list.size()) > Multi_create_list_max_display[gr_screen.res] ? int(Multi_create_mission_list.size())-Multi_create_list_max_display[gr_screen.res] : 0);

	// maybe create a standalone dialog
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_destroy_gen_dialog();		
	}
}

void multi_create_list_load_campaigns()
{	
	char *fname;
	int idx, file_count;
	int campaign_type,max_players;
	char title[255];
	char wild_card[10];
	char **file_list = NULL;

	Multi_create_campaign_list.clear();

	memset( wild_card, 0, sizeof(wild_card) );
	snprintf(wild_card, sizeof(wild_card) - 1, "*%s", FS_CAMPAIGN_FILE_EXT);

	file_list = (char**) vm_malloc( sizeof(char*) * 1024 );

	if (file_list == NULL) {
		return;
	}

	file_count = cf_get_file_list(1024, file_list, CF_TYPE_MISSIONS, wild_card);

	// maybe create a standalone dialog
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_create_gen_dialog("Loading campaigns");
		std_gen_set_text("Campaign:", 1);
	}

	for (idx = 0; idx < file_count; idx++) {
		int flags;
		char *filename, name[NAME_LENGTH];

		fname = file_list[idx];
		
		// tack on any necessary file extension
		filename = cf_add_ext( fname, FS_CAMPAIGN_FILE_EXT );

		if (Game_mode & GM_STANDALONE_SERVER) {			
			std_gen_set_text(filename, 2);
		}

		// if the campaign is a multiplayer campaign, then add the data to the campaign list items
		flags = mission_campaign_parse_is_multi( filename, name );
		if( flags != CAMPAIGN_TYPE_SINGLE && mission_campaign_get_info(filename,title,&campaign_type,&max_players)) {
			multi_create_info mcip;

			strcpy_s(mcip.filename, filename );
			strcpy_s(mcip.name, name );
			
			// setup various flags
			if ( flags == CAMPAIGN_TYPE_MULTI_COOP ){
				mcip.flags = MISSION_TYPE_MULTI_COOP | MISSION_TYPE_MULTI;
			} else if ( flags == CAMPAIGN_TYPE_MULTI_TEAMS ) {
				mcip.flags = MISSION_TYPE_MULTI_TEAMS | MISSION_TYPE_MULTI;
			} else {
				Int3();			// bogus campaign multi type -- find allender
			}							

			// 0 respawns for campaign files (should be contained within the mission files themselves)
			mcip.respawn = 0;

			// 0 max players for campaign files
			mcip.max_players = (unsigned char)max_players;

			// get any additional information for possibly builtin missions
			fs_builtin_mission *fb = game_find_builtin_mission(filename);
			if(fb != NULL){					
			}

			Multi_create_campaign_list.push_back( mcip );
		}
	}

	if (file_list) {
		for (idx = 0; idx < file_count; idx++) {
			if (file_list[idx]) {
				vm_free(file_list[idx]);
				file_list[idx] = NULL;
			}
		}

		vm_free(file_list);
		file_list = NULL;
	}

	// maybe create a standalone dialog
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_destroy_gen_dialog();		
	}
}

void multi_create_list_do()
{
	int idx;
	int start_index,stop_index;
	char selected_name[255];

	// bail early if there aren't any selectable items
	if(Multi_create_list_count == 0){
		return;
	}
	
	// first check to see if the user has clicked on an item
	if(Multi_create_list_select_button.pressed()){		 
		int y,item;				
		Multi_create_list_select_button.get_mouse_pos(NULL,&y);
		item = (y / 10);

		// make sure we are selectedin valid indices
		if((item < Multi_create_list_max_display[gr_screen.res]) && (item >= 0)){					
			item += Multi_create_list_start;		

			if(item < Multi_create_list_count){		
				multi_create_list_select_item(item);
				gamesnd_play_iface(SND_IFACE_MOUSE_CLICK);
			}		
		}
	}	

	// bail early if we don't have a start position
	if(Multi_create_list_start == -1){
		return;
	}

	// display the list of individual campaigns/missions
	int count = 0;
	int y_start = Mc_list_coords[gr_screen.res][MC_Y_COORD];		

	start_index = multi_create_select_to_index(Multi_create_list_start);
	stop_index = (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) ? Multi_create_mission_list.size() : Multi_create_campaign_list.size();

	for (idx = start_index; idx < stop_index; idx++) {
		multi_create_info *mcip;

		if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
			mcip = &Multi_create_mission_list[idx];
		} else {
			mcip = &Multi_create_campaign_list[idx];
		}
		
		// see if we should drop out
		if(count == Multi_create_list_max_display[gr_screen.res]){
			break;
		}

		// see if we should filter out this mission
		if ( !(mcip->flags & Multi_create_filter) ) {
			continue;
		}
		
		// highlight the selected item
		multi_create_select_to_filename(Multi_create_list_select, selected_name);

		if ( !strcmp(selected_name, mcip->filename) ) {		
			gr_set_color_fast(&Color_text_selected);
		} else {
			gr_set_color_fast(&Color_text_normal);
		}		

		// draw the type icon		
		multi_create_list_blit_icons(idx, y_start);		
		
		// force fit the mission name string
		strcpy_s(selected_name, mcip->name);
		gr_force_fit_string(selected_name, 255, Mc_column1_w[gr_screen.res]);
		gr_string(Mc_mission_name_x[gr_screen.res], y_start, selected_name);

		// draw the max players if in mission mode		
		sprintf(selected_name, "%d", (int)mcip->max_players);
		gr_string(Mc_mission_count_x[gr_screen.res], y_start, selected_name);		

		// force fit the mission filename string
		strcpy_s(selected_name, mcip->filename);
		gr_force_fit_string(selected_name, 255, Mc_column3_w[gr_screen.res]);
		gr_string(Mc_mission_fname_x[gr_screen.res], y_start, selected_name);

		y_start += 10;
		count++;
	}
}

// takes care of stuff like changing indices around and setting up the netgame structure
void multi_create_list_select_item(int n)
{
	int abs_index,campaign_type,max_players;
	char title[NAME_LENGTH+1];
	netgame_info ng_temp;
	netgame_info *ng;
	multi_create_info *mcip = NULL;

	char *campaign_desc;

	// if not on the standalone server
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		ng = &Netgame;	
	}
	// on the standalone
	else {
		memset(&ng_temp,0,sizeof(netgame_info));
		ng = &ng_temp;
	}

	if ( n != Multi_create_list_select ) {
		// check to see if this is a valid index, and bail if it is not
		abs_index = multi_create_select_to_index(n);
		if(abs_index == -1){
			return;
		}

		Multi_create_list_select = n;

		// set the mission name
		if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
			multi_create_select_to_filename(n, ng->mission_name);
		} else {
			multi_create_select_to_filename(n, ng->campaign_name);
		}

		// make sure the netgame type is properly set
		int old_type = Netgame.type_flags;
		abs_index = multi_create_select_to_index(n);

		if (abs_index != -1) {
			if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
				mcip = &Multi_create_mission_list[abs_index];
			} else {
				mcip = &Multi_create_campaign_list[abs_index];
			}

			if (mcip->flags & MISSION_TYPE_MULTI_TEAMS) {
				// if we're in squad war mode, leave it as squad war
				if(old_type & NG_TYPE_SW){
					ng->type_flags = NG_TYPE_SW;
				} else {
					ng->type_flags = NG_TYPE_TVT;
				}
			} else if (mcip->flags & MISSION_TYPE_MULTI_COOP) {
				ng->type_flags = NG_TYPE_COOP;
			} else if (mcip->flags & MISSION_TYPE_MULTI_DOGFIGHT) {
				ng->type_flags = NG_TYPE_DOGFIGHT;
			}
		}

		// if we're no longer in a TvT game, just uncheck the squadwar checkbox
		if(!(ng->type_flags & NG_TYPE_TEAM)){
			Multi_create_sw_checkbox.set_state(0);
		}

		// if we switched from something else to team vs. team mode, do some special processing
		if((ng->type_flags & NG_TYPE_TEAM) && (ng->type_flags != old_type) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
			multi_team_reset();
		}

		switch(Multi_create_list_mode){
		case MULTI_CREATE_SHOW_MISSIONS:		
			// don't forget to update the info box window thingie
			if(Net_player->flags & NETINFO_FLAG_AM_MASTER){			
				ship_level_init();		// mwa -- 10/15/97.  Call this function to reset number of ships in mission
				ng->max_players = mission_parse_get_multi_mission_info( ng->mission_name );				
				
				Assert(ng->max_players > 0);
				strcpy_s(ng->title,The_mission.name);								

				// set the information area text
				multi_common_set_text(The_mission.mission_desc);
			}
			// if we're on the standalone, send a request for the description
			else {
				send_netgame_descript_packet(&Netgame.server_addr,0);
				multi_common_set_text("");
			}

			// set the respawns as appropriate
			if (mcip) {
				if(Netgame.options.respawn <= mcip->respawn){
					ng->respawn = Netgame.options.respawn;
					nprintf(("Network", "Using netgame options for respawn count (%d %d)\n", Netgame.options.respawn, mcip->respawn));
				} else {
					ng->respawn = mcip->respawn;
					nprintf(("Network", "Using mission settings for respawn count (%d %d)\n", Netgame.options.respawn, mcip->respawn));
				}
			}
			break;
		case MULTI_CREATE_SHOW_CAMPAIGNS:
			// if not on the standalone server
			if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
				// get the campaign info				
				memset(title,0,NAME_LENGTH+1);
				if(!mission_campaign_get_info(ng->campaign_name,title,&campaign_type,&max_players, &campaign_desc)) {
					memset(ng->campaign_name,0,NAME_LENGTH+1);
					ng->max_players = 0;
				}
				// if we successfully got the # of players
				else {
					memset(ng->title,0,NAME_LENGTH+1);
					strcpy_s(ng->title,title);
					ng->max_players = max_players;					
				}

				nprintf(("Network","MC MAX PLAYERS : %d\n",ng->max_players));

				// set the information area text
				// multi_common_set_text(ng->title);
				if (campaign_desc != NULL)
				{
					multi_common_set_text(campaign_desc);
				}
				else 
				{
					multi_common_set_text("");
				}
			}
			// if on the standalone server, send a request for the description
			else {
				// no descriptions currently kept for campaigns
			}

			// netgame respawns are always 0 for campaigns (until the first mission is loaded)
			ng->respawn = 0;
			break;
		}

		if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
			// update players 
			send_netgame_update_packet();			

			// update all machines about stuff like respawns, etc.
			multi_options_update_netgame();

			// send update to FS2NetD as well
			if (MULTI_IS_TRACKER_GAME) {
				fs2netd_gameserver_update(true);
			}
		} else {
			multi_options_update_mission(ng, Multi_create_list_mode == MULTI_CREATE_SHOW_CAMPAIGNS ? 1 : 0);
		}
	}
}

void multi_create_list_blit_icons(int list_index, int y_start)
{
	multi_create_info *mcip;
	fs_builtin_mission *fb;	
	int max_index;

	// get a pointer to the list item
	max_index = (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) ? Multi_create_mission_list.size() - 1 : Multi_create_campaign_list.size() - 1;

	if ( (list_index < 0) || (list_index > max_index) ) {
		return;
	}

	if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
		mcip = &Multi_create_mission_list[list_index];
	} else {
		mcip = &Multi_create_campaign_list[list_index];
	}

	// blit the multiplayer type icons
	if(mcip->flags & MISSION_TYPE_MULTI_COOP){
		if(Multi_common_icons[MICON_COOP] >= 0){
			gr_set_bitmap(Multi_common_icons[MICON_COOP]);
			gr_bitmap(Mc_icon_type_coords[gr_screen.res][MC_X_COORD],y_start + Mc_icon_type_coords[gr_screen.res][MC_Y_COORD]);
		}
	} else if(mcip->flags & MISSION_TYPE_MULTI_TEAMS){
		if(Multi_common_icons[MICON_TVT] >= 0){
			gr_set_bitmap(Multi_common_icons[MICON_TVT]);
			gr_bitmap(Mc_icon_type_coords[gr_screen.res][MC_X_COORD],y_start + Mc_icon_type_coords[gr_screen.res][MC_Y_COORD]);
		}
	} else if(mcip->flags & MISSION_TYPE_MULTI_DOGFIGHT){
		if(Multi_common_icons[MICON_DOGFIGHT] >= 0){
			gr_set_bitmap(Multi_common_icons[MICON_DOGFIGHT]);
			gr_bitmap(Mc_icon_type_coords[gr_screen.res][MC_X_COORD],y_start + Mc_icon_type_coords[gr_screen.res][MC_Y_COORD]);
		}
	} 

	// if its a valid mission, blit the valid mission icon
	if(MULTI_IS_TRACKER_GAME && (mcip->valid_status == MVALID_STATUS_VALID)){
		if(Multi_common_icons[MICON_VALID] >= 0){
			gr_set_bitmap(Multi_common_icons[MICON_VALID]);
			gr_bitmap(Mc_icon_valid_coords[gr_screen.res][MC_X_COORD],y_start + Mc_icon_valid_coords[gr_screen.res][MC_Y_COORD]);
		}
	}

	// now see if its a builtin mission
	fb = game_find_builtin_mission(mcip->filename);	
	// if the mission is from volition, blit the volition icon
	if((fb != NULL) && (fb->flags & FSB_FROM_VOLITION)){
		if(Multi_common_icons[MICON_VOLITION] >= 0){
			gr_set_bitmap(Multi_common_icons[MICON_VOLITION]);
			gr_bitmap(Mc_icon_volition_coords[gr_screen.res][MC_X_COORD],y_start + Mc_icon_volition_coords[gr_screen.res][MC_Y_COORD]);
		}
	}	
}

void multi_create_accept_hit()
{
	char selected_name[255];
	int start_campaign = 0;
    int popup_choice = 0;

	// make sure all players have finished joining
	if(!multi_netplayer_state_check(NETPLAYER_STATE_JOINED,1)){
		popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON,1,POPUP_OK,XSTR("Please wait until all clients have finished joining",788));
		gamesnd_play_iface(SND_GENERAL_FAIL);
		return;
	} else {
		gamesnd_play_iface(SND_COMMIT_PRESSED);
	}	
	
	// do single mission stuff
	switch(Multi_create_list_mode){
	case MULTI_CREATE_SHOW_MISSIONS:	
		if(Multi_create_list_select != -1){
			// set the netgame mode
			Netgame.campaign_mode = MP_SINGLE_MISSION;

			// setup various filenames and mission names
			multi_create_select_to_filename(Multi_create_list_select,selected_name);
			strncpy( Game_current_mission_filename, selected_name, MAX_FILENAME_LEN );
			strncpy(Netgame.mission_name,selected_name,MAX_FILENAME_LEN);			

			// NETLOG
			ml_printf(NOX("Starting single mission %s, with %d players"), Game_current_mission_filename, multi_num_players());
		} else {
			multi_common_add_notify(XSTR("No mission selected!",789));
			return ;
		}
		break;

	case MULTI_CREATE_SHOW_CAMPAIGNS:
		// do campaign related stuff	
		if(Multi_create_list_select != -1){
			// set the netgame mode
			Netgame.campaign_mode = MP_CAMPAIGN;

			// start a campaign instead of a single mission
			multi_create_select_to_filename(Multi_create_list_select,selected_name);
			multi_campaign_start(selected_name);			
			start_campaign = 1;

			// NETLOG
			ml_printf(NOX("Starting campaign %s, with %d players"), selected_name, multi_num_players());
		} else {
			multi_common_add_notify(XSTR("No campaign selected!",790));
			return ;
		}
		break;
	}

	// if this is a team vs team situation, lock the players send a final team update
	if((Netgame.type_flags & NG_TYPE_TEAM) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		multi_team_host_lock_all();
		multi_team_send_update();
	}

	// coop game option validation checks
	if ( (Netgame.type_flags & NG_TYPE_COOP) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) ) {
		// check for time limit
		if (Netgame.options.mission_time_limit != i2f(-1)) {
			popup_choice = popup(0, 3, POPUP_CANCEL, POPUP_YES, XSTR( " &No", 506 ),
								XSTR("A time limit is being used in a co-op game.\r\n"
									"  Select \'Cancel\' to go back to the mission select screen.\r\n"
									"  Select \'Yes\' to continue with this time limit.\r\n"
									"  Select \'No\' to continue without this time limit.", -1));

			if (popup_choice == 0) {
				return;
			} else if (popup_choice == 2) {
				Netgame.options.mission_time_limit = i2f(-1);
			}
		}

		// check kill limit (NOTE: <= 0 is considered no limit)
		if ( (Netgame.options.kill_limit > 0) && (Netgame.options.kill_limit != 9999) ) {
			popup_choice = popup(0, 3, POPUP_CANCEL, POPUP_YES, XSTR( " &No", 506 ),
								XSTR("A kill limit is being used in a co-op game.\r\n"
									"  Select \'Cancel\' to go back to the mission select screen.\r\n"
									"  Select \'Yes\' to continue with this kill limit.\r\n"
									"  Select \'No\' to continue without this kill limit.", -1));

			if (popup_choice == 0) {
				return;
			} else if (popup_choice == 2) {
				Netgame.options.kill_limit = 9999;
			}
		}
    }

	// if not on the standalone, move to the mission sync state which will take care of everything
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		Multi_sync_mode = MULTI_SYNC_PRE_BRIEFING;
		gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);				
	} 
	// otherwise tell the standalone to do so
	else {
		// when the standalone receives this, he'll do the mission syncing himself
		send_mission_sync_packet(MULTI_SYNC_PRE_BRIEFING,start_campaign);
	}				
}

void multi_create_draw_filter_buttons()
{
	// highlight the correct filter button
	if ( Multi_create_filter == MISSION_TYPE_MULTI ){
		Multi_create_buttons[gr_screen.res][MC_SHOW_ALL].button.draw_forced(2);
	} else if ( Multi_create_filter == MISSION_TYPE_MULTI_COOP ) {
		Multi_create_buttons[gr_screen.res][MC_SHOW_ALL + 1].button.draw_forced(2);
	} else if ( Multi_create_filter == MISSION_TYPE_MULTI_TEAMS ) {
		Multi_create_buttons[gr_screen.res][MC_SHOW_ALL + 2].button.draw_forced(2);
	} else if ( Multi_create_filter == MISSION_TYPE_MULTI_DOGFIGHT ){
		Multi_create_buttons[gr_screen.res][MC_SHOW_ALL + 3].button.draw_forced(2);
	} else {
		Int3();
	}
}

void multi_create_set_selected_team(int team)
{	
	int player_index;
	
	// if we don't currently have a player selected, don't do anything
	if(!Multi_create_plist_select_flag){
		gamesnd_play_iface(SND_GENERAL_FAIL);
		return;
	}
	gamesnd_play_iface(SND_USER_SELECT);

	// otherwise attempt to set the team for this guy	
	player_index = find_player_id(Multi_create_plist_select_id);
	if(player_index != -1){	
		multi_team_set_team(&Net_players[player_index],team);		
	}
}

void multi_create_handle_join(net_player *pl)
{
	// for now just play a bloop sound
	gamesnd_play_iface(SND_ICON_DROP_ON_WING);
}

// fill in net address of player the mouse is over, return player index (or -1 if none)
short multi_create_get_mouse_id()
{
	// determine where he clicked (y pixel value)
	int y,nth,idx;		
	Multi_create_player_select_button.get_mouse_pos(NULL,&y);

	// select things a little differently if we're in team vs. team or non-team vs. team mode			
	nth = (y / 10);			
	if(Netgame.type_flags & NG_TYPE_TEAM){
		int player_index = -1;

		// look through all of team red first
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 0)){
				nth--;

				// if this is the _nth_ guy 
				if(nth < 0){
					player_index = idx;						
					break;
				}
			}
		}
			
		// if we still haven't found him yet, look through the green team
		if(player_index == -1){
			for(idx=0;idx<MAX_PLAYERS;idx++){					
				if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 1)){				
					nth--;
					// if this is the _nth_ guy 
					if(nth < 0){
						player_index = idx;						
						break;
					}
				}
			}
		}

		if(player_index != -1){
			return Net_players[player_index].player_id;
		}
	} else {
		// select the nth active player if possible, disregarding the standalone server
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
				nth--;

				// if this is the _nth_ guy 
				if(nth < 0){
					return Net_players[idx].player_id;					
				}				
			}
		}
	}				
	
	return -1;
}

void multi_create_select_to_filename(int select_index,char *filename)
{
	uint idx;

	// look through the mission list
	if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
		for (idx = 0; idx < Multi_create_mission_list.size(); idx++) {
			if (Multi_create_mission_list[idx].flags & Multi_create_filter) {
				select_index--;
			}

			// if we found the item
			if (select_index < 0) {
				strcpy(filename, Multi_create_mission_list[idx].filename);
				return;
			}
		}
	}
	// look through the campaign list
	else if (Multi_create_list_mode == MULTI_CREATE_SHOW_CAMPAIGNS) {
		for (idx = 0; idx < Multi_create_campaign_list.size(); idx++) {
			select_index--;

			// if we found the item
			if (select_index < 0) {
				strcpy(filename, Multi_create_campaign_list[idx].filename);
				return;
			}		
		}
	}

	strcpy(filename,"");
}

int multi_create_select_to_index(int select_index)
{
	uint idx;
	int lookup_index = 0;

	// look through the mission list
	if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
		for (idx = 0; idx < Multi_create_mission_list.size(); idx++) {
			if (Multi_create_mission_list[idx].flags & Multi_create_filter) {
				lookup_index++;
			} 

			// if we found the item
			if (select_index < lookup_index) {				
				return idx;
			}
		}
	}
	// look through the campaign list
	else if (Multi_create_list_mode == MULTI_CREATE_SHOW_CAMPAIGNS) {
		for (idx = 0; idx < Multi_create_campaign_list.size(); idx++) {
			select_index--;

			// if we found the item
			if (select_index < 0) {				
				return idx;
			}		
		}
	}

	return -1;
}

int multi_create_ok_to_commit()
{
	int player_count, observer_count, idx;
	int notify_of_hacked_ships_tbl = 0;
	int notify_of_hacked_weapons_tbl = 0;
	char err_string[255];
	int abs_index;
	int found_hack;

	// make sure we have a valid mission selected
	if(Multi_create_list_select < 0){
		return 0;
	}	

	// if this is not a valid mission, let the player know
	abs_index = multi_create_select_to_index(Multi_create_list_select);		
	if(abs_index < 0){
		return 0;
	}

	// if we're playing with a hacked ships.tbl (on PXO)
	notify_of_hacked_ships_tbl = 0;
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		if(!Game_ships_tbl_valid){
			notify_of_hacked_ships_tbl = 1;
		}
	} else {
		if(Netgame.flags & NG_FLAG_HACKED_SHIPS_TBL){
			notify_of_hacked_ships_tbl = 1;
		}
	}
	if(!MULTI_IS_TRACKER_GAME){
		notify_of_hacked_ships_tbl = 0;
	}
	if(notify_of_hacked_ships_tbl){
		if(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, XSTR("&Back", 995), XSTR("&Continue", 780), XSTR("You or the server you are playing on has a hacked ships.tbl. Your stats will not be updated on PXO", 1051)) <= 0){
			return 0;
		}
	}

	// if we're playing with a hacked weapons.tbl (on PXO)
	notify_of_hacked_weapons_tbl = 0;
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		if(!Game_weapons_tbl_valid){
			notify_of_hacked_weapons_tbl = 1;
		}
	} else {
		if(Netgame.flags & NG_FLAG_HACKED_WEAPONS_TBL){
			notify_of_hacked_weapons_tbl = 1;
		}
	}
	if(!MULTI_IS_TRACKER_GAME){
		notify_of_hacked_weapons_tbl = 0;
	}
	if(notify_of_hacked_weapons_tbl){
		if(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, XSTR("&Back", 995), XSTR("&Continue", 780), XSTR("You or the server you are playing on has a hacked weapons.tbl. Your stats will not be updated on PXO", 1052)) <= 0){
			return 0;
		}
	}

	// if any of the players have hacked data
	found_hack = 0;
	for(idx=0; idx<MAX_PLAYERS; idx++){
		// look for hacked players
		if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_HAXOR)){
			// we found a hack
			found_hack = 1;

			// message everyone - haha
			if(Net_players[idx].m_player != NULL){
				sprintf(err_string, "%s %s", Net_players[idx].m_player->callsign, XSTR("has hacked tables/data", 1271)); 
			} else {
				sprintf(err_string, "somebody %s", XSTR("has hacked tables/data", 1271)); 
			}
			send_game_chat_packet(Net_player, err_string, MULTI_MSG_ALL, NULL, NULL, 1);
		}
	}
	// if we found a hacked set of data
	if(found_hack){
		// if we're on PXO
		if(MULTI_IS_TRACKER_GAME){
			// don't allow squad war matches to continue
			if(Netgame.type_flags & NG_TYPE_SW){
				// if this is squad war, don't allow it to continue			
				popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("One or more players has hacked data files. You cannot play a SquadWar match unless all clients have legal data", 1272));

				return 0;
			}
			// otherwise, warn the players that stats will not saved
			else {
				// if this is squad war, don't allow it to continue			
				if(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, XSTR("&Back", 995), XSTR("&Continue", 780), XSTR("One or more players has hacked data files. If you continue, stats will not be stored at the end of the mission", 1273)) <= 0){
					return 0;
				}
			}
		}
		// non-pxo, just give a notice
		else {
			if(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, XSTR("&Back", 995), XSTR("&Continue", 780), XSTR("One or more players has hacked data files", 1274)) <= 0){
				return 0;
			}
		}
	}

	// check to see that we don't have too many observers
	observer_count = multi_num_observers();
	if(observer_count > Netgame.options.max_observers){
		// print up the error string
		sprintf(err_string,XSTR("There are too many observers in the game\n\nMax : %d\nCurrently %d\n\nPlease dump a few",791),Netgame.options.max_observers,observer_count);

		popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, err_string);
		return 0;
	}

	// check to see that we have a valid # of players for the the # of ships in the game		
	player_count = multi_num_players();
	if(player_count > Netgame.max_players){
		// print up the error string
		sprintf(err_string,XSTR("There are too many players in the game\n\nMax : %d\nCurrently %d\n\nPlease dump a few", 792), Netgame.max_players,player_count);

		popup(PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, err_string);
		return 0;
	}
	
	// check to see if teams are assigned properly in a team vs. team situation
	if(Netgame.type_flags & NG_TYPE_TEAM){
		if(!multi_team_ok_to_commit()){
			gamesnd_play_iface(SND_GENERAL_FAIL);
			popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("Teams and/or team captains are not assigned properly", 793));			
			return 0;
		}
	}

	// verify cd's	
	if(!multi_create_verify_cds()){
		gamesnd_play_iface(SND_GENERAL_FAIL);

		popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR("You need 1 CD for every 4 players!", 794));			

		return 0;
	}	
	
	// if we're playing on the tracker
	if(MULTI_IS_TRACKER_GAME){
//#ifdef PXO_CHECK_VALID_MISSIONS		
		if ( (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) && (Multi_create_mission_list[abs_index].valid_status != MVALID_STATUS_VALID) ) {
			if(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, XSTR("&Back", 995), XSTR("&Continue", 780), XSTR("You have selected a mission which is either invalid or unknown to PXO. Your stats will not be saved if you continue",996)) <= 0){
				return 0;
			}
		}		
//#endif

		// non-squad war
		if(!(Netgame.type_flags & NG_TYPE_SW)){
			// if he is playing by himself, tell him stats will not be accepted
			if(multi_num_players() == 1){
				if(popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_TITLE_RED | PF_TITLE_BIG, 2, XSTR("&Back", 995), XSTR("&Continue", 780), XSTR("Warning\n\nIf you start a PXO mission by yourself, your stats will not be updated", 997)) <= 0){
					return 0;
				}
			}
		}
		// squad war
		else {			
		}
	}	
		
	return 1;
}

int multi_create_verify_cds()
{
	int player_count = multi_num_players();
	int multi_cd_count;
	int idx;

	// count how many cds we have
	multi_cd_count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].flags & NETINFO_FLAG_HAS_CD)){
			multi_cd_count++;
		}
	}

	// determine if we have enough
	float ratio = (float)player_count / (float)multi_cd_count;
	// greater than a 4 to 1 ratio
	if(ratio > 4.0f){
		return 0;
	} 

	// we meet the conditions
	return 1;
}

// returns an index into Multi_create_mission_list
int multi_create_lookup_mission(char *fname)
{
	uint idx;

	for (idx = 0; idx < Multi_create_mission_list.size(); idx++) {
		if ( !stricmp(fname, Multi_create_mission_list[idx].filename) ) {
			return idx;
		}
	}

	// couldn't find the mission
	return -1;
}

// returns an index into Multi_create_campaign_list
int multi_create_lookup_campaign(char *fname)
{
	uint idx;

	for (idx = 0; idx < Multi_create_campaign_list.size(); idx++) {
		if ( !stricmp(fname, Multi_create_campaign_list[idx].filename) ) {
			return idx;
		}
	}

	// couldn't find the campaign
	return -1;
}


void multi_create_refresh_pxo()
{
	// delete mvalid.cfg if it exists
	cf_delete(MULTI_VALID_MISSION_FILE, CF_TYPE_DATA);

	// refresh missions from the tracker
	multi_update_valid_missions();
}

void multi_create_sw_clicked()
{
	netgame_info ng_temp;
	netgame_info *ng;
	multi_create_info *mcip;

	int file_index = multi_create_select_to_index(Multi_create_list_select);

	if (file_index < 0) {
		Int3();

		if ( Multi_create_sw_checkbox.checked() ) {
			Multi_create_sw_checkbox.set_state(0);
		}

		return;
	}

	if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
		mcip = &Multi_create_mission_list[file_index];
	} else {
		mcip = &Multi_create_campaign_list[file_index];
	}

	// either a temporary netgame or the real one
	if(MULTIPLAYER_MASTER){
		ng = &Netgame;
	} else {
		ng_temp = Netgame;
		ng = &ng_temp;
	}

	// maybe switch squad war off
	if(!Multi_create_sw_checkbox.checked()){
		// if the mission selected is a coop mission, go back to coop mode

		if (mcip->flags & MISSION_TYPE_MULTI_TEAMS) {
			ng->type_flags = NG_TYPE_TVT;
		} else if (mcip->flags & MISSION_TYPE_MULTI_DOGFIGHT) {
			ng->type_flags = NG_TYPE_DOGFIGHT;
		} else {
			ng->type_flags = NG_TYPE_COOP;
		}
	}
	// switch squad war on
	else {
		// at this point we know its safe to switch squad war on
		ng->type_flags = NG_TYPE_SW;
	}

	// update 
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){		
		// update players 
		send_netgame_update_packet();			

		// update all machines about stuff like respawns, etc.
		multi_options_update_netgame();
	}
	// on the standalone
	else {
		// standalone will take care of polling the usertracker
		multi_options_update_mission(ng, Multi_create_list_mode == MULTI_CREATE_SHOW_CAMPAIGNS ? 1 : 0);
	}
}


// -------------------------------------------------------------------------------------------------------------
// 
// MULTIPLAYER HOST OPTIONS SCREEN
//

#define MULTI_HO_NUM_BUTTONS				12
#define MULTI_HO_NUM_RADIO_BUTTONS		10

//XSTR:OFF
// bitmaps defs
#define MULTI_HO_PALETTE				"InterfacePalette"

static char *Multi_ho_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MultiHost",			// GR_640
	"2_MultiHost"			// GR_1024
};

static char *Multi_ho_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MultiHost-M",			// GR_640
	"2_MultiHost-M"		// GR_1024
};

//XSTR:ON
UI_WINDOW Multi_ho_window;												// the window object for the join screen
UI_INPUTBOX Multi_ho_respawns;										// the # of respawns allowed in the game
UI_INPUTBOX Multi_ho_time_limit;										// mission time limit
UI_INPUTBOX Multi_ho_voice_wait;										// wait time between tokens
UI_INPUTBOX Multi_ho_kill_limit;										// kill limit in a furball mission
UI_INPUTBOX Multi_ho_obs;												// # of observers we'll allow
int Multi_ho_bitmap;														// the background bitmap

// constants for coordinate lookup
#define MULTI_HO_X_COORD			0
#define MULTI_HO_Y_COORD			1
#define MULTI_HO_W_COORD			2
#define MULTI_HO_H_COORD			3
#define MULTI_HO_TEXT_X_COORD		4
#define MULTI_HO_TEXT_Y_COORD		5

// button defs
#define MULTI_HO_MSG_RANK				0		// highest ranking players can do messaging
#define MULTI_HO_MSG_LEADER			1		// wing/team leaders can do messaging
#define MULTI_HO_MSG_ANY				2		// any player can do messaging
#define MULTI_HO_MSG_HOST				3		// only the host can do messaging
#define MULTI_HO_END_RANK				4		// highest rank can and host can end mission
#define MULTI_HO_END_LEADER			5		// wing/team leaders and host can end the mission
#define MULTI_HO_END_ANY				6		// any player can end the mission
#define MULTI_HO_END_HOST				7		// only host can end the mission
#define MULTI_HO_VOICE_ON				8		// voice toggled on
#define MULTI_HO_VOICE_OFF				9		// voice toggled off
#define MULTI_HO_HOST_MODIFIES		10		// only the host or team captains can modify ships/weapons in briefing
#define MULTI_HO_ACCEPT					11		// accept button

ui_button_info Multi_ho_buttons[GR_NUM_RESOLUTIONS][MULTI_HO_NUM_BUTTONS] = {	
	{ // GR_640
		// who is allowed to message
		ui_button_info("MH_00",	3,	160,	46,	166,	0),		// highest rank
		ui_button_info("MH_01",	3,	179,	46,	185,	1),		// team/wing leader
		ui_button_info("MH_02",	3,	196,	46,	203,	2),		// any
		ui_button_info("MH_03",	3,	214,	46,	220,	3),		// host
		
		// who is allowed to end the mission
		ui_button_info("MH_04",	3,	257,	46,	265,	4),		// highest rank
		ui_button_info("MH_05",	3,	276,	46,	283,	5),		// team/wing leader
		ui_button_info("MH_06",	3,	294,	46,	300,	6),		// any
		ui_button_info("MH_07",	3,	311,	46,	317,	7),		// host		

		// voice on/off button
		ui_button_info("MH_09",	542,	158,	545,	185,	9),	
		ui_button_info("MH_10",	598,	158,	604,	185,	10),	

		// host modifies ships
		ui_button_info("MH_13",	542,	377,	437,	363,	13),	

		// exit
		ui_button_info("MH_14",	572,	428,	580,	414,	14),	
	},
	{ // GR_1024
		// who is allowed to message
		ui_button_info("2_MH_00",	5,	256,	73,	269,	0),		// highest rank
		ui_button_info("2_MH_01",	5,	286,	73,	297,	1),		// team/wing leader
		ui_button_info("2_MH_02",	5,	314,	73,	325,	2),		// any
		ui_button_info("2_MH_03",	5,	341,	73,	352,	3),		// host
		
		// who is allowed to end the mission
		ui_button_info("2_MH_04",	5,	412,	73,	425,	4),		// highest rank
		ui_button_info("2_MH_05",	5,	442,	73,	452,	5),		// team/wing leader
		ui_button_info("2_MH_06",	5,	470,	73,	480,	6),		// any
		ui_button_info("2_MH_07",	5,	497,	73,	508,	7),		// host		

		// voice on/off button
		ui_button_info("2_MH_09",	867,	253,	872,	296,	9),	
		ui_button_info("2_MH_10",	957,	253,	966,	296,	10),	

		// host modifies ships
		ui_button_info("2_MH_13",	867,	603,	784,	581,	13),	

		// exit
		ui_button_info("2_MH_14",	916,	685,	925,	665,	14),	
	},
};
UI_XSTR Multi_ho_text[GR_NUM_RESOLUTIONS][MULTI_HO_NUM_BUTTONS] = {
	{ // GR_640
		{"Highest rank",					1280,	46,	166,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_MSG_RANK].button},
		{"Team / wing-leader",			1281, 46,	185,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_MSG_LEADER].button},
		{"Any",								1282, 46,	203,	UI_XSTR_COLOR_GREEN,	-1, &Multi_ho_buttons[0][MULTI_HO_MSG_ANY].button},
		{"Host",								1283,	46,	220,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_MSG_HOST].button},
		{"Highest rank",					1280,	46,	265,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_END_RANK].button},
		{"Team / wing-leader",			1281,	46,	283,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_END_LEADER].button},
		{"Any",								1282, 46,	300,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_END_ANY].button},
		{"Host",								1283,	46,	317,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_END_HOST].button},		
		{"On",								1285,	545,	185,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_VOICE_ON].button},
		{"Off",								1286,	604,	185,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_VOICE_OFF].button},
		{"Host modifies ships",			1287,	437,	363,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[0][MULTI_HO_HOST_MODIFIES].button},
		{"Exit",								1417,	572,	418,	UI_XSTR_COLOR_PINK,	-1, &Multi_ho_buttons[0][MULTI_HO_ACCEPT].button},
	},
	{ // GR_1024
		{"Highest rank",					1280,	62,	269,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_MSG_RANK].button},
		{"Team / wing-leader",			1281, 62,	297,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_MSG_LEADER].button},
		{"Any",								1282, 62,	325,	UI_XSTR_COLOR_GREEN,	-1, &Multi_ho_buttons[1][MULTI_HO_MSG_ANY].button},
		{"Host",								1283,	62,	352,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_MSG_HOST].button},
		{"Highest rank",					1280,	62,	425,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_END_RANK].button},
		{"Team / wing-leader",			1281,	62,	452,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_END_LEADER].button},
		{"Any",								1282, 62,	480,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_END_ANY].button},
		{"Host",								1283,	62,	508,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_END_HOST].button},		
		{"On",								1285,	877,	294,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_VOICE_ON].button},
		{"Off",								1286,	967,	293,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_VOICE_OFF].button},
		{"Host modifies ships",			1287,	869,	589,	UI_XSTR_COLOR_GREEN, -1, &Multi_ho_buttons[1][MULTI_HO_HOST_MODIFIES].button},
		{"Exit",								1417,	953,	672,	UI_XSTR_COLOR_PINK,	-1, &Multi_ho_buttons[1][MULTI_HO_ACCEPT].button},
	}
};

// radio button controls
#define MULTI_HO_NUM_RADIO_GROUPS		3
#define MULTI_HO_MSG_GROUP					0							// group dealing with squadmate messaging
#define MULTI_HO_END_GROUP					1							// group dealing with ending the mission
#define MULTI_HO_VOICE_GROUP				2							// group dealing with voice stuff
int Multi_ho_radio_groups[MULTI_HO_NUM_RADIO_GROUPS] = {		// currently selected button in the radio button group
	0,0,0
};
int Multi_ho_radio_info[MULTI_HO_NUM_RADIO_BUTTONS][3] = {	// info related to each of the radio buttons themselves
	// { group #, value, button id# }
	{0, 0, 0},				// highest ranking players can do messaging
	{0, 1, 1},				// wing/team leaders can do messaging
	{0, 2, 2},				// any player can do messaging
	{0, 3, 3},				// only host can do messaging	
	{1, 0, 4},				// highest rank and host can end the mission
	{1, 1, 5},				// team/wing leader can end the mission
	{1, 2, 6},				// any player can end the mission
	{1, 3, 7},				// only the host can end the mission
	{2, 0, 8},				// voice toggled on
	{2, 1, 9}				// voice toggled off
};

// slider controls
#define MULTI_HO_NUM_SLIDERS					3
#define MULTI_HO_SLIDER_VOICE_QOS			0						// voice quality of sound 
#define MULTI_HO_SLIDER_VOICE_DUR			1						// max duration of voice recording
#define MULTI_HO_SLIDER_SKILL					2						// skill level
struct ho_sliders {
	char *filename;
	int x, y, xt, yt;
	int hotspot;
	int dot_w;
	int dots;
	UI_DOT_SLIDER_NEW slider;  // because we have a class inside this struct, we need the constructor below..

	ho_sliders(char *name, int x1, int y1, int xt1, int yt1, int h, int _dot_w, int _dots) : filename(name), x(x1), y(y1), xt(xt1), yt(yt1), hotspot(h), dot_w(_dot_w), dots(_dots){}
};
ho_sliders Multi_ho_sliders[GR_NUM_RESOLUTIONS][MULTI_HO_NUM_SLIDERS] = {
	{ // GR_640
		ho_sliders("MH_11",	428,	214,	437,	199,	11,	19,	10),			// voice qos
		ho_sliders("MH_12",	428,	261,	437,	246,	12,	19,	10),			// voice duration
		ho_sliders("MH_08",	237,	454,	230,	411,	8,		36,	5),			// skill level
	},
	{ // GR_1024		
		ho_sliders("2_MH_11",	684,	343,	690,	323,	11,	32,	10),			// voice qos
		ho_sliders("2_MH_12",	685,	418,	837,	468,	12,	32,	10),			// voice duration
		ho_sliders("2_MH_08",	379,	727,	369,	663,	8,		60,	5),			// skill level
	}
};

int Multi_ho_mission_respawn;

int Multi_ho_host_modifies;

// whether or not any of the inputboxes on this screen had focus last frame
int Multi_ho_lastframe_input = 0;

// game information text areas

// ho titles
#define MULTI_HO_NUM_TITLES					14
UI_XSTR Multi_ho_titles[GR_NUM_RESOLUTIONS][MULTI_HO_NUM_TITLES] = {
	{ // GR_640
		{ "AI Orders",				1289,		32,	144, UI_XSTR_COLOR_GREEN, -1, NULL },		
		{ "End Mission",			1290,		32,	242, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Time Limit",			1291,		32,	347, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Min",						1292,		74,	362, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Respawn Limit",		1288,		32,	378, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Kill Limit",			1293,		32,	409, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Observers",				1294,		32,	441, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Skill Level",			1284,		230,	411, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Voice Transmission",	1295,		437,	144, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Voice Quality",		1296,		437,	199, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Message Duration",	1297,		437,	246, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "sec",						1522,		523,	292, UI_XSTR_COLOR_GREEN, -1, NULL },		
		{ "sec",						1523,		523,	332, UI_XSTR_COLOR_GREEN, -1, NULL },		
		{ "Voice Wait",			1298,		437,	313, UI_XSTR_COLOR_GREEN, -1, NULL },
	},
	{ // GR_1024		
		{ "AI Orders",				1289,		48,	238, UI_XSTR_COLOR_GREEN, -1, NULL },		
		{ "End Mission",			1290,		48,	394, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Time Limit",			1291,		50,	568, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Min",						1292,		119,	581, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Respawn Limit",		1288,		50,	618, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Kill Limit",			1293,		50,	668, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Observers",				1294,		50,	718, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Skill Level",			1284,		398,	670, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Voice Transmission",	1295,		869,	239, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Voice Quality",		1296,		690,	331, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Message Duration",	1297,		690,	405, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "sec",						1522,		837,	467, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "sec",						1523,		837,	534, UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Voice Wait",			1298,		742,	510, UI_XSTR_COLOR_GREEN, -1, NULL },
	}
};

// mission time limit input box
int Ho_time_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		36, 362, 36, 17
	},
	{ // GR_1024
		58, 581, 57, 27
	}
};

// furball kill limit input box
int Ho_kill_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		36, 425, 45, 17
	},
	{ // GR_1024
		58, 684, 72, 27
	}
};

// voice recording duration text display area
int Ho_vd_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		467, 292, 55, 15
	},
	{ // GR_1024
		750, 467, 85, 28
	}
};

// voice token wait input box
int Ho_vw_coords[GR_NUM_RESOLUTIONS][6] = {
	{ // GR_640
		467, 332, 55, 15
	},
	{ // GR_1024
		750, 534, 85, 28
	}
};

// observer count input box
int Ho_obs_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		36, 457, 45, 17
	},
	{ // GR_1024
		58, 733, 72, 27
	}
};

// skill text description area
int Ho_st_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		249, 435, 172, 10
	},
	{ // GR_1024
		403, 699, 172, 10
	}
};

// respawn input box
int Ho_rsp_coords[GR_NUM_RESOLUTIONS][6] = {
	{ // GR_640
		36, 394, 45, 17, 
	},
	{ // GR_1024
		58, 632, 72, 27
	}
};

// respawn max text area
int Ho_max_rsp_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		150, 378
	},
	{ // GR_1024
		190, 618
	}
};

// maximum values for various input boxes (to notify user of overruns)
#define MULTI_HO_MAX_TIME_LIMIT				500
#define MULTI_HO_MAX_TOKEN_WAIT				5
#define MULTI_HO_MAX_KILL_LIMIT				9999
#define MULTI_HO_MAX_OBS						4

// LOCAL function definitions
void multi_ho_check_buttons();
void multi_ho_button_pressed(int n);
void multi_ho_draw_radio_groups();
void multi_ho_accept_hit();
void multi_ho_get_options();
void multi_ho_apply_options();
void multi_ho_display_record_time();
int multi_ho_check_values();
void multi_ho_check_focus();
void multi_ho_blit_max_respawns();
void multi_ho_display_skill_level();

void multi_host_options_init()
{
	int idx;

	// create the interface window
	Multi_ho_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_ho_window.set_mask_bmap(Multi_ho_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_ho_bitmap = bm_load(Multi_ho_bitmap_fname[gr_screen.res]);
	if(Multi_ho_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}		

	// initialize the common notification messaging
	multi_common_notify_init();	

	// use the common interface palette
	multi_common_set_palette();	

	// create the interface buttons
	for(idx=0;idx<MULTI_HO_NUM_BUTTONS;idx++){
		// create the object
		Multi_ho_buttons[gr_screen.res][idx].button.create(&Multi_ho_window, "", Multi_ho_buttons[gr_screen.res][idx].x, Multi_ho_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_ho_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_ho_buttons[gr_screen.res][idx].button.set_bmaps(Multi_ho_buttons[gr_screen.res][idx].filename);

		// set the hotspot, ignoring the skill level button		
		Multi_ho_buttons[gr_screen.res][idx].button.link_hotspot(Multi_ho_buttons[gr_screen.res][idx].hotspot);

		// add xstr text
		Multi_ho_window.add_XSTR(&Multi_ho_text[gr_screen.res][idx]);
	}		

	// create misc text
	for(idx=0; idx<MULTI_HO_NUM_TITLES; idx++){
		Multi_ho_window.add_XSTR(&Multi_ho_titles[gr_screen.res][idx]);
	}

	// create the interface sliders
	for(idx=0; idx<MULTI_HO_NUM_SLIDERS; idx++){
		// create the object
		Multi_ho_sliders[gr_screen.res][idx].slider.create(&Multi_ho_window, Multi_ho_sliders[gr_screen.res][idx].x, Multi_ho_sliders[gr_screen.res][idx].y, Multi_ho_sliders[gr_screen.res][idx].dots, Multi_ho_sliders[gr_screen.res][idx].filename, Multi_ho_sliders[gr_screen.res][idx].hotspot, NULL, -1, -1, -1, NULL, -1, -1, -1, Multi_ho_sliders[gr_screen.res][idx].dot_w);
	}

	// create the respawn count input box
	Multi_ho_respawns.create(&Multi_ho_window,Ho_rsp_coords[gr_screen.res][MULTI_HO_X_COORD],Ho_rsp_coords[gr_screen.res][MULTI_HO_Y_COORD],Ho_rsp_coords[gr_screen.res][MULTI_HO_W_COORD],6,"",UI_INPUTBOX_FLAG_ESC_FOC | UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_NO_LETTERS,-1,&Color_bright);		
	// if we're in campaign mode, disable it
	if(Netgame.campaign_mode == MP_CAMPAIGN){
		Multi_ho_respawns.set_text(XSTR("NA",795));  // [[ Not applicable ]]
		Multi_ho_respawns.disable();
	} else {
		Multi_ho_respawns.set_valid_chars("0123456789");
	}

	// create the time limit input box
	Multi_ho_time_limit.create(&Multi_ho_window, Ho_time_coords[gr_screen.res][MULTI_HO_X_COORD], Ho_time_coords[gr_screen.res][MULTI_HO_Y_COORD], Ho_time_coords[gr_screen.res][MULTI_HO_W_COORD], 3, "", UI_INPUTBOX_FLAG_ESC_FOC | UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_NO_LETTERS, -1, &Color_bright);
	Multi_ho_time_limit.set_valid_chars("-0123456789");

	// create the voice token wait input box
	Multi_ho_voice_wait.create(&Multi_ho_window, Ho_vw_coords[gr_screen.res][MULTI_HO_X_COORD], Ho_vw_coords[gr_screen.res][MULTI_HO_Y_COORD], Ho_vw_coords[gr_screen.res][MULTI_HO_W_COORD], 1, "", UI_INPUTBOX_FLAG_ESC_FOC | UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_NO_LETTERS, -1, &Color_bright);
	Multi_ho_voice_wait.set_valid_chars("01243456789");

	// create the furball kill limit input box
	Multi_ho_kill_limit.create(&Multi_ho_window, Ho_kill_coords[gr_screen.res][MULTI_HO_X_COORD], Ho_kill_coords[gr_screen.res][MULTI_HO_Y_COORD], Ho_kill_coords[gr_screen.res][MULTI_HO_W_COORD], 4, "", UI_INPUTBOX_FLAG_ESC_FOC | UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_NO_LETTERS, -1, &Color_bright);
	Multi_ho_kill_limit.set_valid_chars("0123456789");

	// create the observer limit input box
	Multi_ho_obs.create(&Multi_ho_window, Ho_obs_coords[gr_screen.res][MULTI_HO_X_COORD], Ho_obs_coords[gr_screen.res][MULTI_HO_Y_COORD], Ho_obs_coords[gr_screen.res][MULTI_HO_W_COORD], 1, "", UI_INPUTBOX_FLAG_ESC_FOC | UI_INPUTBOX_FLAG_INVIS | UI_INPUTBOX_FLAG_NO_LETTERS, -1, &Color_bright);
	Multi_ho_obs.set_valid_chars("01234");	
	
	// load in the current netgame defaults
	multi_ho_get_options();

	// whether or not any of the inputboxes on this screen had focus last frame
	Multi_ho_lastframe_input = 0;	

	// get the # of respawns for the currently selected mission (if any)
	if(Multi_create_list_select != -1){
		int abs_index = multi_create_select_to_index(Multi_create_list_select);

		// if he has a valid mission selected
		if(abs_index >= 0){
			if (Multi_create_list_mode == MULTI_CREATE_SHOW_MISSIONS) {
				Multi_ho_mission_respawn = (int)Multi_create_mission_list[abs_index].respawn;
			} else {
				Multi_ho_mission_respawn = (int)Multi_create_campaign_list[abs_index].respawn;
			}
		} else {
			Multi_ho_mission_respawn = -1;
		}
	} else {
		Multi_ho_mission_respawn = -1;
	}
}

void multi_ho_update_sliders()
{
	// game skill slider
	if (Game_skill_level != Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_SKILL].slider.pos) {
		if ( !(Netgame.type_flags & NG_TYPE_TEAM) ){		
			Game_skill_level = Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_SKILL].slider.pos;
			gamesnd_play_iface(SND_USER_SELECT);
		} else {	
			Game_skill_level = NUM_SKILL_LEVELS / 2;
		}
	}

	// get the voice qos options
	if (Netgame.options.voice_qos != (ubyte)(Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_QOS].slider.pos + 1)) {
		Netgame.options.voice_qos = (ubyte)(Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_QOS].slider.pos + 1);	
		gamesnd_play_iface(SND_USER_SELECT);
	}

	// get the voice duration options
	if (Netgame.options.voice_record_time != (int)(0.5f * (float)(Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_DUR].slider.pos + 1) * 1000.0f)) {
		Netgame.options.voice_record_time = (int)(0.5f * (float)(Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_DUR].slider.pos + 1) * 1000.0f);
		gamesnd_play_iface(SND_USER_SELECT);
	}

}

void multi_host_options_do()
{
	int k;	
	
	// process stuff
	k = Multi_ho_window.process();		
	chatbox_process(k);	

	// process any keypresses
	switch(k){
	case KEY_ESC :
		gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
		break;
	// same as ACCEPT
	case KEY_CTRLED + KEY_ENTER :	
		gamesnd_play_iface(SND_COMMIT_PRESSED);
		multi_ho_accept_hit();
		break;
	}

	// process any button clicks
	multi_ho_check_buttons();	

	// update the sliders
	multi_ho_update_sliders();

	// make sure that the chatbox inputbox and any inputbox on this screen are mutually exclusive in terms of focus
	multi_ho_check_focus();

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Multi_ho_bitmap);
	if(Multi_ho_bitmap != -1){
		gr_set_bitmap(Multi_ho_bitmap);
		gr_bitmap(0,0);
	}
	Multi_ho_window.draw();
	
	// draw all the radio buttons properly
	multi_ho_draw_radio_groups();
			
	// display any pending notification messages
	multi_common_notify_do();		

	// display the voice record time settings
	multi_ho_display_record_time();	

	// maybe display the max # of respawns next to the respawn input box
	multi_ho_blit_max_respawns();

	// blit the proper skill level	
	multi_ho_display_skill_level();

	// blit the "host modifies button"
	if(Multi_ho_host_modifies){
		Multi_ho_buttons[gr_screen.res][MULTI_HO_HOST_MODIFIES].button.draw_forced(2);
	}

	// process and show the chatbox thingie	
	chatbox_render();

	// draw tooltips
	Multi_ho_window.draw_tooltip();	

	// display the voice status indicator
	multi_common_voice_display_status();

	// flip the buffer
	gr_flip();
}

void multi_host_options_close()
{
	// unload any bitmaps
	if(!bm_unload(Multi_ho_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_ho_bitmap_fname[gr_screen.res]));
	}	
	
	// destroy the UI_WINDOW
	Multi_ho_window.destroy();
}

void multi_ho_check_buttons()
{
	int idx;
	for(idx=0;idx<MULTI_HO_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Multi_ho_buttons[gr_screen.res][idx].button.pressed()){
			multi_ho_button_pressed(idx);
			break;
		}
	}
}

void multi_ho_button_pressed(int n)
{
	int radio_index,idx;
	int x_pixel,y_pixel;

	// get the pixel position of the click
	Multi_ho_buttons[gr_screen.res][n].button.get_mouse_pos(&x_pixel,&y_pixel);
		
	switch(n){		
	// clicked on the accept button
	case MULTI_HO_ACCEPT:
		gamesnd_play_iface(SND_COMMIT_PRESSED);
		multi_ho_accept_hit();
		return;	
	
	// clicked on the host/captains only modify button
	case MULTI_HO_HOST_MODIFIES:
		// toggle it on or off
		Multi_ho_host_modifies = !Multi_ho_host_modifies;
		gamesnd_play_iface(SND_USER_SELECT);
		return;
	}

	// look through the radio buttons and see which one this corresponds to
	radio_index = -1;
	for(idx=0;idx<MULTI_HO_NUM_RADIO_BUTTONS;idx++){
		if(Multi_ho_radio_info[idx][2] == n){
			radio_index = idx;
			break;
		}
	}
	Assert(radio_index != -1);

	// check to see if a radio button was pressed
	if(radio_index < MULTI_HO_NUM_RADIO_BUTTONS){
		// see if this value is already picked for this radio group
		if(Multi_ho_radio_groups[Multi_ho_radio_info[radio_index][0]] != Multi_ho_radio_info[radio_index][1]){
			gamesnd_play_iface(SND_USER_SELECT);
			Multi_ho_radio_groups[Multi_ho_radio_info[radio_index][0]] = Multi_ho_radio_info[radio_index][1];
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
	}
}

void multi_ho_draw_radio_groups()
{
	int idx;
	
	// go through each item and draw it if it is the selected button in its respective group
	for(idx=0;idx<MULTI_HO_NUM_RADIO_BUTTONS;idx++){
		/// if this button is the currently selected one in its group
		if(Multi_ho_radio_info[idx][1] == Multi_ho_radio_groups[Multi_ho_radio_info[idx][0]]){
			Multi_ho_buttons[gr_screen.res][Multi_ho_radio_info[idx][2]].button.draw_forced(2);
		}
	}
}

void multi_ho_accept_hit()
{
	char resp_str[10];

	// check the values in the input boxes
	if(!multi_ho_check_values()){
		return;
	}
	
	// zero out the netgame flags
	Netgame.flags = 0;
	
	// set default options
	Netgame.options.flags = (MSO_FLAG_INGAME_XFER | MSO_FLAG_ACCEPT_PIX);

	// set the squadmate messaging flags
	switch(Multi_ho_radio_groups[MULTI_HO_MSG_GROUP]){
	case 0 : 
		Netgame.options.squad_set = MSO_SQUAD_RANK;
		break;
	case 1 :
		Netgame.options.squad_set = MSO_SQUAD_LEADER;		
		break;
	case 2 :
		Netgame.options.squad_set = MSO_SQUAD_ANY;		
		break;
	case 3 :
		Netgame.options.squad_set = MSO_SQUAD_HOST;
		break;
	default : 
		Int3(); 
	}

	// set the end mission flags
	switch(Multi_ho_radio_groups[MULTI_HO_END_GROUP]){
	case 0 : 
		Netgame.options.endgame_set = MSO_END_RANK;		
		break;
	case 1 :
		Netgame.options.endgame_set = MSO_END_LEADER;		
		break;			
	case 2 : 
		Netgame.options.endgame_set = MSO_END_ANY;		
		break;
	case 3 :
		Netgame.options.endgame_set = MSO_END_HOST;		
		break;	
	default : 
		Int3(); 
	}
	
	// set the voice toggle
	switch(Multi_ho_radio_groups[MULTI_HO_VOICE_GROUP]){
	case 0 :
		Netgame.options.flags &= ~(MSO_FLAG_NO_VOICE);
		break;
	case 1 :
		Netgame.options.flags |= MSO_FLAG_NO_VOICE;
		break;
	default : 
		Int3();
	}	

	// get the voice qos options
	Netgame.options.voice_qos = (ubyte)(Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_QOS].slider.pos + 1);	

	// get the voice duration options
	Netgame.options.voice_record_time = (int)(0.5f * (float)(Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_DUR].slider.pos + 1) * 1000.0f);
	
	// set the skill level.  If in team vs. team mode, preserve the old setting before saving
	// the pilot file.  I'll bet that this doesn't work though because the pilot file gets
	// written in a bunch of locations....sigh.
	if ( !(Netgame.type_flags & NG_TYPE_TEAM) ){		
		Game_skill_level = Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_SKILL].slider.pos;
	} else {	
		Game_skill_level = NUM_SKILL_LEVELS / 2;
	}

	// set the netgame respawn count
	// maybe warn the user that respawns will not be used for a campaign mission
	if(Netgame.campaign_mode == MP_SINGLE_MISSION){
		Multi_ho_respawns.get_text(resp_str);
		uint temp_respawn = (uint)atoi(resp_str);
		// if he currently has no mission selected, let the user set any # of respawns
		if((int)temp_respawn > Multi_ho_mission_respawn){
			if(Multi_ho_mission_respawn == -1){	
				Netgame.respawn = temp_respawn;		
				Netgame.options.respawn = temp_respawn;
			}
			// this should have been taken care of by the interface code
			else {
				Int3();
			}
		} else {	
			Netgame.options.respawn = temp_respawn;
			Netgame.respawn = temp_respawn;
		}
	}

	// get the mission time limit
	Multi_ho_time_limit.get_text(resp_str);
	int temp_time = atoi(resp_str);
	if(temp_time <= 0){
		Netgame.options.mission_time_limit = fl2f(-1.0f);
	} else if(temp_time > MULTI_HO_MAX_TIME_LIMIT){
		Int3();
	} else {
		Netgame.options.mission_time_limit = fl2f(60.0f * (float)temp_time);	
	}

	// get observer count options
	Multi_ho_obs.get_text(resp_str);
	int temp_obs = atoi(resp_str);
	if(temp_obs > MULTI_HO_MAX_OBS){
		Int3();
	} 
	Netgame.options.max_observers = (ubyte)temp_obs;	

	// get the furball kill limit
	Multi_ho_kill_limit.get_text(resp_str);
	int temp_kills = atoi(resp_str);
	if(temp_kills > MULTI_HO_MAX_KILL_LIMIT){
		Int3();
	}
	Netgame.options.kill_limit = temp_kills;

	// get the token wait limit
	Multi_ho_voice_wait.get_text(resp_str);
	int temp_wait = atoi(resp_str);
	if(temp_wait > MULTI_HO_MAX_TOKEN_WAIT){
		Int3();
	} 
	Netgame.options.voice_token_wait = (temp_wait * 1000);		

	// set the netgame option
	Netgame.options.skill_level = (ubyte)Game_skill_level;

	// get whether we're in host/captains only modify mode
	Netgame.options.flags &= ~(MSO_FLAG_SS_LEADERS);
	if(Multi_ho_host_modifies){
		Netgame.options.flags |= MSO_FLAG_SS_LEADERS;
	}	

	// store these values locally
	memcpy(&Player->m_local_options,&Net_player->p_info.options,sizeof(multi_local_options));
	memcpy(&Player->m_server_options,&Netgame.options,sizeof(multi_server_options));
	write_pilot_file(Player);	

	// apply any changes in settings (notify everyone of voice qos changes, etc)
	multi_ho_apply_options();

	// move back to the create game screen
	gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
}

void multi_ho_get_options()
{	
	char resp_str[10];
	
	// set the squadmate messaging buttons	
	switch(Netgame.options.squad_set){
	case MSO_SQUAD_RANK :		
		Multi_ho_radio_groups[MULTI_HO_MSG_GROUP] = 0;
		break;
	case MSO_SQUAD_LEADER:				
		Multi_ho_radio_groups[MULTI_HO_MSG_GROUP] = 1;
		break;
	case MSO_SQUAD_ANY:			
		Multi_ho_radio_groups[MULTI_HO_MSG_GROUP] = 2;
		break;
	case MSO_SQUAD_HOST:		
		Multi_ho_radio_groups[MULTI_HO_MSG_GROUP] = 3;
		break;
	default : 
		Int3();		
	}
	
	// set the mission end buttons	
	switch(Netgame.options.endgame_set){
	case MSO_END_RANK:			
		Multi_ho_radio_groups[MULTI_HO_END_GROUP] = 0;
		break;
	case MSO_END_LEADER:			
		Multi_ho_radio_groups[MULTI_HO_END_GROUP] = 1;
		break;
	case MSO_END_ANY:
		Multi_ho_radio_groups[MULTI_HO_END_GROUP] = 2;
		break;
	case MSO_END_HOST:
		Multi_ho_radio_groups[MULTI_HO_END_GROUP] = 3;
		break;
	default : 
		Int3();
	}			

	// set the voice toggle buttons
	if(Netgame.options.flags & MSO_FLAG_NO_VOICE){
		Multi_ho_radio_groups[MULTI_HO_VOICE_GROUP] = 1;
	} else {
		Multi_ho_radio_groups[MULTI_HO_VOICE_GROUP] = 0;
	}	

	// get the voice qos options
	Assert((Netgame.options.voice_qos >= 1) && (Netgame.options.voice_qos <= 10));
	Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_QOS].slider.pos = (Netgame.options.voice_qos - 1);

	// get the voice duration options
	Assert((Netgame.options.voice_record_time > 0) && (Netgame.options.voice_record_time <= MULTI_VOICE_MAX_TIME));
	Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_DUR].slider.pos = ((int)((float)Netgame.options.voice_record_time / 500.0f)) - 1;	

	// get the current skill level
	Assert((Game_skill_level >= 0) && (Game_skill_level < NUM_SKILL_LEVELS));
	Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_SKILL].slider.pos = Game_skill_level;	

	// get the # of observers
	memset(resp_str,0,10);
	sprintf(resp_str,"%d",Netgame.options.max_observers);
	Multi_ho_obs.set_text(resp_str);

	// set the respawn count
	if(Netgame.campaign_mode == MP_SINGLE_MISSION){
		memset(resp_str,0,10);
		sprintf(resp_str,"%u",Netgame.respawn);
		Multi_ho_respawns.set_text(resp_str);	
	}

	// set the mission time limit
	memset(resp_str,0,10);
	float tl = f2fl(Netgame.options.mission_time_limit);
	sprintf(resp_str,"%d",(int)(tl / 60.0f));
	Multi_ho_time_limit.set_text(resp_str);

	// set the furball kill limit
	memset(resp_str,0,10);
	sprintf(resp_str,"%d",Netgame.options.kill_limit);
	Multi_ho_kill_limit.set_text(resp_str);

	// set the token wait time
	memset(resp_str,0,10);
	sprintf(resp_str,"%d",Netgame.options.voice_token_wait / 1000);
	Multi_ho_voice_wait.set_text(resp_str);	

	// get whether we're in host/captains only modify mode
	if(Netgame.options.flags & MSO_FLAG_SS_LEADERS){
		Multi_ho_host_modifies = 1;
	} else {
		Multi_ho_host_modifies = 0;
	}
}

void multi_ho_apply_options()
{
	// if the voice qos or duration has changed, apply the change
	multi_voice_maybe_update_vars(Netgame.options.voice_qos,Netgame.options.voice_record_time);		

	// send an options update
	multi_options_update_netgame();	
}

// display the voice record time settings
void multi_ho_display_record_time()
{
	char time_str[30];
	int full_seconds, half_seconds;

	// clear the string
	memset(time_str,0,30);

	// get the seconds
	full_seconds = (((Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_DUR].slider.pos + 1) * 500) / 1000);
	
	// get the half-seconds
	half_seconds = ((((Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_VOICE_DUR].slider.pos + 1) * 500) % 1000) / 500) * 5;

	// format the string
	sprintf(time_str,"%d.%d",full_seconds,half_seconds);
	gr_set_color_fast(&Color_bright);
	gr_string(Ho_vd_coords[gr_screen.res][MULTI_HO_X_COORD],Ho_vd_coords[gr_screen.res][MULTI_HO_Y_COORD],time_str);
}

int multi_ho_check_values()
{
	char val_txt[255];

	memset(val_txt,0,255);

	// check against respawn settings	
	if(Multi_ho_mission_respawn != -1){
		Multi_ho_respawns.get_text(val_txt);
		// if the value is invalid, let the user know
		if(atoi(val_txt) > Multi_ho_mission_respawn){
			memset(val_txt,0,255);
			sprintf(val_txt,XSTR("Warning\nRespawn count in greater than mission specified max (%d)",796),Multi_ho_mission_respawn);			
			popup(PF_USE_AFFIRMATIVE_ICON | PF_TITLE_RED | PF_TITLE_BIG,1,POPUP_OK,val_txt);
			return 0;
		}
	}

	// check against mission time limit max
	Multi_ho_time_limit.get_text(val_txt);
	// if the value is invalid, force it to be valid
	if(atoi(val_txt) > MULTI_HO_MAX_TIME_LIMIT){
		memset(val_txt,0,255);
		sprintf(val_txt,XSTR("Warning\nMission time limit is greater than max allowed (%d)",797),MULTI_HO_MAX_TIME_LIMIT);		
		popup(PF_USE_AFFIRMATIVE_ICON | PF_TITLE_RED | PF_TITLE_BIG,1,POPUP_OK,val_txt);
		return 0;
	}

	// check against max observer limit	
	Multi_ho_obs.get_text(val_txt);
	// if the value is invalid, force it to be valid
	if(atoi(val_txt) > MULTI_HO_MAX_OBS){
		memset(val_txt,0,255);
		sprintf(val_txt,XSTR("Warning\nObserver count is greater than max allowed (%d)",798),MULTI_HO_MAX_OBS);		
		popup(PF_USE_AFFIRMATIVE_ICON | PF_TITLE_RED | PF_TITLE_BIG,1,POPUP_OK,val_txt);
		return 0;
	}

	// check against furball kill limit	
	Multi_ho_kill_limit.get_text(val_txt);
	// if the value is invalid, force it to be valid
	if(atoi(val_txt) > MULTI_HO_MAX_KILL_LIMIT){
		memset(val_txt,0,255);
		sprintf(val_txt,XSTR("Warning\nMission kill limit is greater than max allowed (%d)",799),MULTI_HO_MAX_KILL_LIMIT);		
		popup(PF_USE_AFFIRMATIVE_ICON | PF_TITLE_RED | PF_TITLE_BIG,1,POPUP_OK,val_txt);
		return 0;
	}

	// check against the token wait limit	
	Multi_ho_voice_wait.get_text(val_txt);
	if(atoi(val_txt) > MULTI_HO_MAX_TOKEN_WAIT){
		memset(val_txt,0,255);
		sprintf(val_txt,XSTR("Warning\nvoice wait time is greater than max allowed (%d)",800),MULTI_HO_MAX_TOKEN_WAIT);		
		popup(PF_USE_AFFIRMATIVE_ICON | PF_TITLE_RED | PF_TITLE_BIG,1,POPUP_OK,val_txt);
		return 0;
	}

	// all values are valid
	return 1;
}

void multi_ho_check_focus()
{
	// if an inputbox has been pressed (hit enter), lose its focus
	if (Multi_ho_respawns.pressed() || Multi_ho_time_limit.pressed() || Multi_ho_voice_wait.pressed() || Multi_ho_kill_limit.pressed() || Multi_ho_obs.pressed()) {
		Multi_ho_respawns.clear_focus();
		Multi_ho_time_limit.clear_focus();	
		Multi_ho_voice_wait.clear_focus();			
		Multi_ho_kill_limit.clear_focus();
		Multi_ho_obs.clear_focus();
		gamesnd_play_iface(SND_COMMIT_PRESSED);
		chatbox_set_focus();
		Multi_ho_lastframe_input = 0;

	} else if(!Multi_ho_lastframe_input) {
		// if we didn't have focus last frame
		if(Multi_ho_respawns.has_focus() || Multi_ho_time_limit.has_focus() || Multi_ho_kill_limit.has_focus() || Multi_ho_voice_wait.has_focus() ){
			chatbox_lose_focus();

			Multi_ho_lastframe_input = 1;
		}	
	} 
	// if we _did_ have focus last frame
	else {
		// if we no longer have focus on any of the input boxes, set the focus on the chatbox
		if(!Multi_ho_respawns.has_focus() && !Multi_ho_time_limit.has_focus() && !Multi_ho_kill_limit.has_focus() && !Multi_ho_voice_wait.has_focus() && !chatbox_has_focus()){
			chatbox_set_focus();
		}			
		// if the chatbox now has focus, clear all focus from our inputboxes
		else if (chatbox_has_focus()) {
			Multi_ho_respawns.clear_focus();
			Multi_ho_time_limit.clear_focus();
			Multi_ho_kill_limit.clear_focus();
			Multi_ho_voice_wait.clear_focus();

			Multi_ho_lastframe_input = 0;
		}
	}
}

void multi_ho_blit_max_respawns()
{
	char string[50];
	
	// if we're in campaign mode, do nothing
	if(Netgame.campaign_mode == MP_CAMPAIGN){
		return;
	}
	
	// otherwise blit the max as specified by the current mission file	
	sprintf(string,"(%d)",Multi_ho_mission_respawn);	
	gr_set_color_fast(&Color_normal);
	gr_string(Ho_max_rsp_coords[gr_screen.res][MULTI_HO_X_COORD], Ho_max_rsp_coords[gr_screen.res][MULTI_HO_Y_COORD], string);
}

void multi_ho_display_skill_level()
{
	int skill_level = Multi_ho_sliders[gr_screen.res][MULTI_HO_SLIDER_SKILL].slider.pos;

	// sanity
	Assert((skill_level >= 0) && (skill_level < NUM_SKILL_LEVELS));
	if((skill_level < 0) || (skill_level >= NUM_SKILL_LEVELS)){
		skill_level = 0;
	}

	gr_set_color_fast(&Color_bright);
	gr_string(Ho_st_coords[gr_screen.res][0], Ho_st_coords[gr_screen.res][1], Skill_level_names(skill_level, 1));
}

// -------------------------------------------------------------------------------------------------------------
// 
// MULTIPLAYER JOIN SCREEN 
//

#define MULTI_JW_NUM_BUTTONS	8

//XSTR:OFF
// bitmaps defs
#define MULTI_JW_PALETTE				"InterfacePalette"

static char *Multi_jw_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MultiJoinWait",		// GR_640
	"2_MultiJoinWait"		// GR_1024
};

static char *Multi_jw_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MultiJoinWait-M",		// GR_640
	"2_MultiJoinWait-M"		// GR_1024
};

//XSTR:ON

// button defs
#define MJW_SCROLL_PLAYERS_UP		0
#define MJW_SCROLL_PLAYERS_DOWN	1
#define MJW_TEAM0						2
#define MJW_TEAM1						3
#define MJW_PILOT_INFO				4
#define MJW_SCROLL_INFO_UP			5
#define MJW_SCROLL_INFO_DOWN		6
#define MJW_CANCEL					7

UI_WINDOW Multi_jw_window;												// the window object for the join screen
int Multi_jw_bitmap;														// the background bitmap

// constants for coordinate lookup
#define MJW_X_COORD 0
#define MJW_Y_COORD 1
#define MJW_W_COORD 2
#define MJW_H_COORD 3

ui_button_info Multi_jw_buttons[GR_NUM_RESOLUTIONS][MULTI_JW_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("MJW_00",	1,		24,	-1,	-1,	0),
		ui_button_info("MJW_01",	1,		66,	-1,	-1,	1),
		ui_button_info("MJW_02",	30,	244,	20,	272,	2),
		ui_button_info("MJW_03",	84,	244,	73,	272,	3),
		ui_button_info("MJW_04",	139,	242,	134,	272,	4),
		ui_button_info("MJW_05",	1,		406,	-1,	-1,	5),
		ui_button_info("MJW_06",	1,		447,	-1,	-1,	6),
		ui_button_info("MJW_07",	577,	428,	570,	414,	7),
	},
	{ // GR_1024
		ui_button_info("2_MJW_00",	2,		38,	-1,	-1,	0),
		ui_button_info("2_MJW_01",	2,		106,	-1,	-1,	1),
		ui_button_info("2_MJW_02",	48,	390,	47,	435,	2),
		ui_button_info("2_MJW_03",	134,	390,	133,	435,	3),
		ui_button_info("2_MJW_04",	223,	388,	225,	435,	4),
		ui_button_info("2_MJW_05",	2,		649,	-1,	-1,	5),
		ui_button_info("2_MJW_06",	2,		715,	-1,	-1,	6),
		ui_button_info("2_MJW_07",	923,	685,	931,	667,	7),
	}
};

#define MULTI_JW_NUM_TEXT			7

UI_XSTR Multi_jw_text[GR_NUM_RESOLUTIONS][MULTI_JW_NUM_TEXT] = {
	{ // GR_640
		{ "Team 1",				1308,		20,	272,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[0][MJW_TEAM0].button },
		{ "Team 2",				1309,		73,	272,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[0][MJW_TEAM1].button },
		{ "Pilot",				1310,		134,	272,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[0][MJW_PILOT_INFO].button },
		{ "Info",				1311,		134,	283,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[0][MJW_PILOT_INFO].button },
		{ "Cancel",				387,		570,	414,	UI_XSTR_COLOR_PINK, -1, &Multi_jw_buttons[0][MJW_CANCEL].button },
		{ "Players",			1269,		38,	8,		UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Choose Team",		1312,		27,	231,	UI_XSTR_COLOR_GREEN, -1, NULL },
	},
	{ // GR_1024
		{ "Team 1",				1308,		47,	435,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[1][MJW_TEAM0].button },
		{ "Team 2",				1309,		133,	435,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[1][MJW_TEAM1].button },
		{ "Pilot",				1310,		225,	435,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[1][MJW_PILOT_INFO].button },
		{ "Info",				1311,		225,	446,	UI_XSTR_COLOR_GREEN, -1, &Multi_jw_buttons[1][MJW_PILOT_INFO].button },
		{ "Cancel",				387,		931,	667,	UI_XSTR_COLOR_PINK, -1, &Multi_jw_buttons[1][MJW_CANCEL].button },
		{ "Players",			1269,		165,	12,	UI_XSTR_COLOR_GREEN, -1, NULL },
		{ "Choose Team",		1312,		45,	373,	UI_XSTR_COLOR_GREEN, -1, NULL },
	}
};

int Mjw_players_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		29, 18, 153, 210
	},
	{ // GR_1024
		46, 29, 254, 336
	}
};

int Mjw_mission_name_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		5, 380
	},
	{ // GR_1024
		47, 618
	}
};

// squad war checkbox
UI_CHECKBOX	Multi_jw_sw_checkbox;
char *Multi_jw_sw_checkbox_fname[GR_NUM_RESOLUTIONS] = {
	"MC_SW_00",
	"MC_SW_00",
};
int Multi_jw_sw_checkbox_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		6, 285
	},
	{ // GR_1024
		18, 450
	}
};
int Multi_jw_sw_checkbox_text[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		6, 305
	},
	{ // GR_640
		18, 470
	},
};


// player list control thingie defs
#define MULTI_JW_PLIST_MAX_DISPLAY		19
int Multi_jw_plist_select_flag;					// indicates whether we currently have a selected player
short Multi_jw_plist_select_id;				// id of the current selected player
UI_BUTTON Multi_jw_plist_select_button;		// for selecting a player

int Multi_jw_should_show_popup = 0;

// LOCAL function definitions
void multi_jw_check_buttons();
void multi_jw_button_pressed(int n);
void multi_jw_do_netstuff();
void multi_jw_scroll_players_up();
void multi_jw_scroll_players_down();
void multi_jw_plist_process();
void multi_jw_plist_blit_normal();
void multi_jw_plist_blit_team();
short multi_jw_get_mouse_id();

void multi_game_client_setup_init()
{
	int idx;

	// create the interface window
	Multi_jw_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_jw_window.set_mask_bmap(Multi_jw_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_jw_bitmap = bm_load(Multi_jw_bitmap_fname[gr_screen.res]);
	if(Multi_jw_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}

	// initialize the player list data	
	Multi_jw_plist_select_flag = 0;
	Multi_jw_plist_select_id = -1;	
	
	// kill any old instances of the chatbox and create a new one
	chatbox_close();
	chatbox_create(CHATBOX_FLAG_BIG | CHATBOX_FLAG_DRAW_BOX | CHATBOX_FLAG_BUTTONS);

	// initialize the common notification messaging
	multi_common_notify_init();

	// initialize the common mission info display area.
	multi_common_set_text("");	

	// use the common interface palette
	multi_common_set_palette();	

	// create the interface buttons
	for(idx=0; idx<MULTI_JW_NUM_BUTTONS; idx++){
		// create the object
		Multi_jw_buttons[gr_screen.res][idx].button.create(&Multi_jw_window, "", Multi_jw_buttons[gr_screen.res][idx].x, Multi_jw_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_jw_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_jw_buttons[gr_screen.res][idx].button.set_bmaps(Multi_jw_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Multi_jw_buttons[gr_screen.res][idx].button.link_hotspot(Multi_jw_buttons[gr_screen.res][idx].hotspot);
	}		

	// if this is a PXO game, enable the squadwar checkbox	
	Multi_jw_sw_checkbox.create(&Multi_jw_window, "", Multi_jw_sw_checkbox_coords[gr_screen.res][0], Multi_jw_sw_checkbox_coords[gr_screen.res][1], 0);
	Multi_jw_sw_checkbox.set_bmaps(Multi_jw_sw_checkbox_fname[gr_screen.res], 6, 0);
	Multi_jw_sw_checkbox.disable();
	if(!MULTI_IS_TRACKER_GAME){
		Multi_jw_sw_checkbox.hide();		
	}

	// create all xstrs
	for(idx=0; idx<MULTI_JW_NUM_TEXT; idx++){
		Multi_jw_window.add_XSTR(&Multi_jw_text[gr_screen.res][idx]);
	}
	
	// create the player select list button and hide it
	Multi_jw_plist_select_button.create(&Multi_jw_window, "", Mjw_players_coords[gr_screen.res][MJW_X_COORD], Mjw_players_coords[gr_screen.res][MJW_Y_COORD], Mjw_players_coords[gr_screen.res][MJW_W_COORD], Mjw_players_coords[gr_screen.res][MJW_H_COORD], 0, 1);
	Multi_jw_plist_select_button.hide();

	// set hotkeys
	Multi_jw_buttons[gr_screen.res][MJW_CANCEL].button.set_hotkey(KEY_ESC);	

	// remove campaign flags
	Game_mode &= ~(GM_CAMPAIGN_MODE);

	// tell the server we have finished joining
	Net_player->state = NETPLAYER_STATE_JOINED;
	send_netplayer_update_packet();	

	// NETLOG
	ml_printf(NOX("Joined netgame %s"), Netgame.name);

	// send any appropriate files
	multi_data_send_my_junk();	
}

void multi_game_client_setup_do_frame()
{
	int player_index;
	int k = chatbox_process();
	char mission_text[255];
	k = Multi_jw_window.process(k,0);	

	Multi_jw_should_show_popup = 0;

	// process any button clicks
	multi_jw_check_buttons();

	// do any network related stuff
	multi_jw_do_netstuff(); 		

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Multi_jw_bitmap);
	if(Multi_jw_bitmap != -1){		
		gr_set_bitmap(Multi_jw_bitmap);
		gr_bitmap(0,0);
	}

	// if we're not in team vs. team mode, don't draw the team buttons
	if(!(Netgame.type_flags & NG_TYPE_TEAM)){
		Multi_jw_buttons[gr_screen.res][MJW_TEAM0].button.hide();
		Multi_jw_buttons[gr_screen.res][MJW_TEAM1].button.hide();
		Multi_jw_buttons[gr_screen.res][MJW_TEAM0].button.disable();
		Multi_jw_buttons[gr_screen.res][MJW_TEAM1].button.disable();
	} else {
		Multi_jw_buttons[gr_screen.res][MJW_TEAM0].button.enable();
		Multi_jw_buttons[gr_screen.res][MJW_TEAM1].button.enable();
		Multi_jw_buttons[gr_screen.res][MJW_TEAM0].button.unhide();
		Multi_jw_buttons[gr_screen.res][MJW_TEAM1].button.unhide();		
	}

	if(MULTI_IS_TRACKER_GAME){
		// maybe check the squadwar button
		if(Netgame.type_flags & NG_TYPE_SW){
			Multi_jw_sw_checkbox.set_state(1);
			gr_set_color_fast(&Color_bright);
		} else {
			Multi_jw_sw_checkbox.set_state(0);
			gr_set_color_fast(&Color_normal);
		}
				
		gr_string(Multi_jw_sw_checkbox_text[gr_screen.res][0], Multi_jw_sw_checkbox_text[gr_screen.res][1], "SquadWar");
	}	

	// draw the UI window
	Multi_jw_window.draw();	

	// process and display the player list	
	// NOTE : this must be done before the buttons are checked to insure that a player hasn't dropped 
	multi_jw_plist_process();
	if(Netgame.type_flags & NG_TYPE_TEAM){
		multi_jw_plist_blit_team();
	} else {
		multi_jw_plist_blit_normal();
	}
		
	// display any text in the info area
	multi_common_render_text();

	// display any pending notification messages
	multi_common_notify_do();

	// blit the mission filename if possible
	if(Netgame.campaign_mode){
		if(Netgame.campaign_name[0] != '\0'){			
			strcpy_s(mission_text,Netgame.campaign_name);
			
			if(Netgame.title[0] != '\0'){
				strcat_s(mission_text,", ");
				strcat_s(mission_text,Netgame.title);
			}

			gr_set_color_fast(&Color_bright_white);
			gr_string(Mjw_mission_name_coords[gr_screen.res][MJW_X_COORD],Mjw_mission_name_coords[gr_screen.res][MJW_Y_COORD],mission_text);
		}								
	} else {
		if(Netgame.mission_name[0] != '\0'){			
			strcpy_s(mission_text,Netgame.mission_name);

			if(Netgame.title[0] != '\0'){
				strcat_s(mission_text,", ");
				strcat_s(mission_text,Netgame.title);
			}			

			gr_set_color_fast(&Color_bright_white);
			gr_string(Mjw_mission_name_coords[gr_screen.res][MJW_X_COORD],Mjw_mission_name_coords[gr_screen.res][MJW_Y_COORD],mission_text);
		}
	}	

	// process and show the chatbox thingie	
	chatbox_render();

	// draw tooltips
	Multi_jw_window.draw_tooltip();

	// display the voice status indicator
	multi_common_voice_display_status();
	
	// flip the buffer
	gr_flip();	

	// if we're supposed to be displaying a pilot info popup
	if(Multi_jw_should_show_popup){
		player_index = find_player_id(Multi_jw_plist_select_id);
		if(player_index != -1){			
			multi_pinfo_popup(&Net_players[player_index]);
		}		
	}
}

void multi_game_client_setup_close()
{
	// unload any bitmaps
	if(!bm_unload(Multi_jw_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_jw_bitmap_fname[gr_screen.res]));
	}		

	// destroy the chatbox
	// chatbox_close();
	
	// destroy the UI_WINDOW
	Multi_jw_window.destroy();

	// play a sound.
	if(Netgame.game_state == NETGAME_STATE_MISSION_SYNC){
		gamesnd_play_iface(SND_COMMIT_PRESSED);
	}
}


void multi_jw_check_buttons()
{
	int idx;
	for(idx=0;idx<MULTI_JW_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Multi_jw_buttons[gr_screen.res][idx].button.pressed()){
			multi_jw_button_pressed(idx);
			break;
		}
	}
}

void multi_jw_button_pressed(int n)
{
	switch(n){	
	case MJW_CANCEL:
		gamesnd_play_iface(SND_USER_SELECT);		
		multi_quit_game(PROMPT_CLIENT);		
		break;	
	case MJW_SCROLL_PLAYERS_UP:
		multi_jw_scroll_players_up();
		break;
	case MJW_SCROLL_PLAYERS_DOWN:
		multi_jw_scroll_players_down();
		break;	
	case MJW_SCROLL_INFO_UP:
		multi_common_scroll_text_up();
		break;
	case MJW_SCROLL_INFO_DOWN:
		multi_common_scroll_text_down();
		break;
	
	// request to set myself to team 0
	case MJW_TEAM0:
		gamesnd_play_iface(SND_USER_SELECT);
		multi_team_set_team(Net_player,0);
		break;

	// request to set myself to team 1
	case MJW_TEAM1:
		gamesnd_play_iface(SND_USER_SELECT);
		multi_team_set_team(Net_player,1);
		break;

	// pilot info popup
	case MJW_PILOT_INFO:
		Multi_jw_should_show_popup = 1;
		break;

	default :
		multi_common_add_notify(XSTR("Not implemented yet!",760));
		break;
	}
}

// do stuff like pinging servers, sending out requests, etc
void multi_jw_do_netstuff()
{
}

void multi_jw_scroll_players_up()
{
	gamesnd_play_iface(SND_GENERAL_FAIL);
}

// scroll down through the player list
void multi_jw_scroll_players_down()
{	
	gamesnd_play_iface(SND_GENERAL_FAIL);
}

void multi_jw_plist_process()
{
	int test_count,player_index,idx;
	
	// first determine if there are 0 players in the game. This should never happen since the host is _always_ in the game
	test_count = 0;
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			test_count++;
		}
	}
	if(test_count <= 0){
		return;
	}
	
	// if we had a selected item but that player has left, select myself instead
	if(Multi_jw_plist_select_flag){
		player_index = find_player_id(Multi_jw_plist_select_id);
		if(player_index == -1){
			Multi_jw_plist_select_id = Net_player->player_id;						
		}
	} else {
		Multi_jw_plist_select_flag = 1;
		Multi_jw_plist_select_id = Net_player->player_id;		
	}
		
	// if the player has clicked somewhere in the player list area
	if(Multi_jw_plist_select_button.pressed()){
		short player_id;
	
		player_id = multi_jw_get_mouse_id();
		player_index = find_player_id(player_id);
		if(player_index != -1){
			Multi_jw_plist_select_id = player_id;
			Multi_jw_plist_select_flag = 1;
		}
	}
}

void multi_jw_plist_blit_normal()
{
	int idx;		
	char str[CALLSIGN_LEN+1];
	int y_start = Mjw_players_coords[gr_screen.res][MJW_Y_COORD];	
	int total_offset;

	// display all the players	
	for(idx=0;idx<MAX_PLAYERS;idx++){		
		// reset total offset
		total_offset = 0;

		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			// highlight him if he's the host			
			if(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST){
				if(Multi_jw_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active_hi);
				} else {
					gr_set_color_fast(&Color_bright);
				}
			} else {
				if(Multi_jw_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active);
				} else {
					gr_set_color_fast(&Color_text_normal);
				}
			}

			// optionally draw his CD status
			if((Net_players[idx].flags & NETINFO_FLAG_HAS_CD) && (Multi_common_icons[MICON_CD] != -1)){
				gr_set_bitmap(Multi_common_icons[MICON_CD]);
				gr_bitmap(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start - 1);

				total_offset += Multi_common_icon_dims[MICON_CD][0] + 1;
			}			
			
			// make sure the string will fit, then display it
			strcpy_s(str,Net_players[idx].m_player->callsign);
			if(Net_players[idx].flags & NETINFO_FLAG_OBSERVER){
				strcat_s(str,"(0)");
			}
			gr_force_fit_string(str,CALLSIGN_LEN,Mjw_players_coords[gr_screen.res][MJW_W_COORD] - total_offset);
			gr_string(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start,str);

			y_start += 10;			
		}
	}		
}

void multi_jw_plist_blit_team()
{
	int idx;		
	char str[CALLSIGN_LEN+1];
	int y_start = Mjw_players_coords[gr_screen.res][MJW_Y_COORD];	
	int total_offset;

	// always blit the proper team button based on _my_ team status
	if(Net_player->p_info.team == 0){
		Multi_jw_buttons[gr_screen.res][MJW_TEAM0].button.draw_forced(2);
	} else {
		Multi_jw_buttons[gr_screen.res][MJW_TEAM1].button.draw_forced(2);
	}

	// display all the red players first
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// reset total offset
		total_offset = 0;

		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 0)){
			// highlight him if he's the host			
			if(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST){
				if(Multi_jw_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active_hi);					
				} else {
					gr_set_color_fast(&Color_bright);
				}
			} else {
				if(Multi_jw_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active);					
				} else {
					gr_set_color_fast(&Color_text_normal);
				}
			}

			// optionally draw his CD status
			if((Net_players[idx].flags & NETINFO_FLAG_HAS_CD) && (Multi_common_icons[MICON_CD] != -1)){
				gr_set_bitmap(Multi_common_icons[MICON_CD]);
				gr_bitmap(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start - 1);

				total_offset += Multi_common_icon_dims[MICON_CD][0] + 1;
			}			

			// blit the red team indicator
			if(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN){
				if(Multi_common_icons[MICON_TEAM0_SELECT] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM0_SELECT]);
					gr_bitmap(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM0_SELECT][0] + 1;
				}				
			} else {
				if(Multi_common_icons[MICON_TEAM0] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM0]);
					gr_bitmap(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM0][0] + 1;
				}
			}

			// make sure the string will fit
			strcpy_s(str,Net_players[idx].m_player->callsign);
			gr_force_fit_string(str,CALLSIGN_LEN,Mjw_players_coords[gr_screen.res][MJW_W_COORD] - total_offset);

			// display him in the correct half of the list depending on his team
			gr_string(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start,str);
			y_start += 10;
		}
	}	
	
	// display all the green players next
	for(idx=0;idx<MAX_PLAYERS;idx++){
		// reset total offset
		total_offset = 0;

		// count anyone except the standalone server (if applicable)
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 1)){
			// highlight him if he's the host			
			if(Net_players[idx].flags & NETINFO_FLAG_GAME_HOST){
				if(Multi_jw_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active_hi);				
				} else {
					gr_set_color_fast(&Color_bright);
				}
			} else {
				if(Multi_jw_plist_select_id == Net_players[idx].player_id){
					gr_set_color_fast(&Color_text_active);					
				} else {
					gr_set_color_fast(&Color_text_normal);
				}
			}

			// optionally draw his CD status
			if((Net_players[idx].flags & NETINFO_FLAG_HAS_CD) && (Multi_common_icons[MICON_CD] != -1)){
				gr_set_bitmap(Multi_common_icons[MICON_CD]);
				gr_bitmap(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start - 1);

				total_offset += Multi_common_icon_dims[MICON_CD][0] + 1;
			}			

			// blit the red team indicator
			if(Net_players[idx].flags & NETINFO_FLAG_TEAM_CAPTAIN){
				if(Multi_common_icons[MICON_TEAM1_SELECT] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM1_SELECT]);
					gr_bitmap(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM1_SELECT][0] + 1;
				}
			} else {
				if(Multi_common_icons[MICON_TEAM1] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM1]);
					gr_bitmap(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start-2);

					total_offset += Multi_common_icon_dims[MICON_TEAM1][0] + 1;
				}
			}

			// make sure the string will fit
			strcpy_s(str,Net_players[idx].m_player->callsign);
			if(Net_players[idx].flags & NETINFO_FLAG_OBSERVER){
				strcat_s(str,"(0)");
			}
			gr_force_fit_string(str,CALLSIGN_LEN,Mjw_players_coords[gr_screen.res][MJW_W_COORD] - total_offset);

			// display him in the correct half of the list depending on his team
			gr_string(Mjw_players_coords[gr_screen.res][MJW_X_COORD] + total_offset,y_start,str);
			y_start += 10;
		}
	}			
}

void multi_jw_handle_join(net_player *pl)
{
	// for now just play a bloop sound
	gamesnd_play_iface(SND_ICON_DROP_ON_WING);
}

short multi_jw_get_mouse_id()
{
	// determine where he clicked (y pixel value)
	int y,nth,idx;		
	Multi_jw_plist_select_button.get_mouse_pos(NULL,&y);

	// select things a little differently if we're in team vs. team or non-team vs. team mode			
	nth = (y / 10);			
	if(Netgame.type_flags & NG_TYPE_TEAM){
		int player_index = -1;

		// look through all of team red first
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 0)){				
				nth--;

				// if this is the _nth_ guy 
				if(nth < 0){
					player_index = idx;						
					break;
				}
			}
		}
			
		// if we still haven't found him yet, look through the green team
		if(player_index == -1){
			for(idx=0;idx<MAX_PLAYERS;idx++){
				if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_players[idx].p_info.team == 1)){					
					nth--;
					// if this is the _nth_ guy 
					if(nth < 0){
						player_index = idx;						
						break;
					}
				}
			}
		}
		if(player_index != -1){
			return Net_players[idx].player_id;			
		}		
	} else {
		// select the nth active player if possible, disregarding the standalone server
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
				nth--;

				// if this is the _nth_ guy 
				if(nth < 0){
					return Net_players[idx].player_id;					
				}
			}
		}
		return -1;
	}				

	return -1;
}


// -------------------------------------------------------------------------------------------------------------
// 
// MULTIPLAYER WAIT/SYNCH SCREEN 
//

#define MULTI_SYNC_HOST_COUNT				4		// host uses 4 buttons (and sometimes 5)
#define MULTI_SYNC_CLIENT_COUNT			3		// client only uses 3 buttons

char *Multi_sync_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"MultiSynch",		// GR_640
	"2_MultiSynch"		// GR_1024
};

char *Multi_sync_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"MultiSynch-M",		// GR_640
	"2_MultiSynch-M"			// GR_1024
};

//XSTR:ON

// constants for coordinate lookup
#define MS_X_COORD 0
#define MS_Y_COORD 1
#define MS_W_COORD 2
#define MS_H_COORD 3

UI_WINDOW Multi_sync_window;												// the window object for the join screen
int Multi_sync_button_count;												// the # of buttons to use for this instance of mission sync
int Multi_sync_bitmap;														// the background bitmap

// button defs
#define MULTI_SYNC_NUM_BUTTONS			5
#define MS_SCROLL_INFO_UP					0
#define MS_SCROLL_INFO_DOWN				1
#define MS_CANCEL								2
#define MS_KICK								3
#define MS_LAUNCH								4
ui_button_info Multi_sync_buttons[GR_NUM_RESOLUTIONS][MULTI_SYNC_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("MS_00",		1,		404,	-1,	-1,	0),
		ui_button_info("MS_01",		1,		446,	-1,	-1,	1),
		ui_button_info("MS_03",		518,	426,	519,	416,	3),
		ui_button_info("MS_02",		469,	426,	479,	416,	2),		
		ui_button_info("MS_04",		571,	420,	577,	416,	4),
	},
	{ // GR_1024
		ui_button_info("2_MS_00",		2,		647,	-1,	-1,	0),
		ui_button_info("2_MS_01",		2,		713,	-1,	-1,	1),
		ui_button_info("2_MS_03",		829,	682,	831,	667,	3),
		ui_button_info("2_MS_02",		751,	682,	766,	667,	2),		
		ui_button_info("2_MS_04",		914,	672,	924,	667,	4),
	}
};

// text
#define MULTI_SYNC_NUM_TEXT				5
#define MST_KICK								0
#define MST_LAUNCH							2
UI_XSTR Multi_sync_text[GR_NUM_RESOLUTIONS][MULTI_SYNC_NUM_TEXT] = {
	{ // GR_640
		{ "Kick",		1266,		479,	416,	UI_XSTR_COLOR_PINK,	-1, &Multi_sync_buttons[0][MS_KICK].button },
		{ "Cancel",		387,		519,	416,	UI_XSTR_COLOR_PINK,	-1, &Multi_sync_buttons[0][MS_CANCEL].button },
		{ "Launch",		801,		577,	416,	UI_XSTR_COLOR_PINK,	-1, &Multi_sync_buttons[0][MS_LAUNCH].button },
		{ "Players",	1269,		23,	133,	UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "Status",		1304,		228,	133,	UI_XSTR_COLOR_GREEN,	-1, NULL }
	},
	{ // GR_1024
		{ "Kick",		1266,		766,	667,	UI_XSTR_COLOR_PINK,	-1, &Multi_sync_buttons[1][MS_KICK].button },
		{ "Cancel",		387,		831,	667,	UI_XSTR_COLOR_PINK,	-1, &Multi_sync_buttons[1][MS_CANCEL].button },
		{ "Launch",		801,		924,	667,	UI_XSTR_COLOR_PINK,	-1, &Multi_sync_buttons[1][MS_LAUNCH].button },
		{ "Players",	1269,		38,	214,	UI_XSTR_COLOR_GREEN,	-1, NULL },
		{ "Status",		1304,		366,	214,	UI_XSTR_COLOR_GREEN,	-1, NULL }
	}
};

// player name
int Ms_status_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		38, 150, 581, 220
	},
	{ // GR_1024
		38, 228, 958, 367
	}
};

// player status coords
int Ms_status2_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		228, 150, 391, 220
	},
	{ // GR_1024
		370, 228, 626, 367
	}
};

int Ms_cd_icon_offset[GR_NUM_RESOLUTIONS] = {
	10,		// GR_640
	10			// GR_1024
};

int Ms_team_icon_offset[GR_NUM_RESOLUTIONS] = {
	38,		// GR_640
	38			// GR_1024
};

// player currently selected, index into Net_players[]
int Multi_sync_player_select = -1;

// player list control thingie defs
#define MULTI_SYNC_PLIST_MAX_DISPLAY	15
int Multi_sync_plist_start;		// where to start displaying from
int Multi_sync_plist_count;		// how many we have

// list select button
UI_BUTTON Multi_sync_plist_button;

int Multi_sync_mode = -1;

#define MULTI_SYNC_COUNTDOWN_TIME			5				// in seconds
float Multi_sync_countdown_timer;
int Multi_sync_countdown = -1;

int Multi_launch_button_created;

//XSTR:OFF
// countdown animation timer
char* Multi_sync_countdown_fname[GR_NUM_RESOLUTIONS] = {
	"Count",		// GR_640
	"2_Count"		// GR_1024
};

int Multi_sync_countdown_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		0, 0		// GR_640
	},		
	{
		0, 0		// GR_1024
	}
};

//XSTR:ON

static generic_anim Multi_sync_countdown_anim;


// PREBRIEFING STUFF
// syncing flags used by the server
int Mission_sync_flags = 0;
#define MS_FLAG_SENT_FILESIG			(1<<0)	// sent filesig requests
#define MS_FLAG_SENT_LOAD				(1<<1)	// sent load packets
#define MS_FLAG_PUSHED_BRIEFING		(1<<2)	// pushed everyone else into the briefing
#define MS_FLAG_POST_DATA				(1<<3)	// sent the post data block
#define MS_FLAG_WSS_SLOTS				(1<<4)	// all players have received wss slot data
#define MS_FLAG_PSETTINGS				(1<<5)	// send the player settings packet
#define MS_FLAG_MT_STATS_START		(1<<6)	// server has started getting player stats from the tracker
#define MS_FLAG_MT_STATS_DONE			(1<<7)	// server has finished getting player stats from the tracker (success or fail)
#define MS_FLAG_TS_SLOTS				(1<<8)	// team/ship slots have been sent
#define MS_FLAG_DATA_DONE				(1<<9)	// done transferring all necessary data
#define MS_FLAG_CAMP_DONE				(1<<10)	// send campaign pool/goal/event stuff

// POSTBRIEFING STUFF
int Multi_state_timestamp;
int Multi_sync_launch_pressed;

// LOCAL function definitions
void multi_sync_check_buttons();
void multi_sync_button_pressed(int n);
void multi_sync_scroll_info_up();
void multi_sync_scroll_info_down();
void multi_sync_display_name(char *name,int index,int np_index);		// display info on the left hand portion of the status window thingie
void multi_sync_display_status(char *status,int index);					// display info on the right hand portion of the status window thingie
void multi_sync_force_start_pre();
void multi_sync_force_start_post();
void multi_sync_launch();
void multi_sync_create_launch_button();
void multi_sync_blit_screen_all();
void multi_sync_handle_plist();

void multi_sync_common_init();
void multi_sync_common_do();
void multi_sync_common_close();

void multi_sync_pre_init();
void multi_sync_pre_do();
void multi_sync_pre_close();

void multi_sync_post_init();
void multi_sync_post_do();
void multi_sync_post_close();

int Sync_test = 1;


// perform the correct init functions
void multi_sync_init()
{	
	Multi_sync_countdown = -1;

	Sync_test = 1;

	// reset all timestamp
	multi_reset_timestamps();

	if(!(Game_mode & GM_STANDALONE_SERVER)){
		multi_sync_common_init();
	}
	
	switch(Multi_sync_mode){
	case MULTI_SYNC_PRE_BRIEFING:
		multi_sync_pre_init();
		break;
	case MULTI_SYNC_POST_BRIEFING:
		multi_sync_post_init();
		break;
	case MULTI_SYNC_INGAME:
		multi_ingame_sync_init();
		break;
	}
}

// perform the correct do frame functions
void multi_sync_do()
{
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		multi_sync_common_do();
	}

	// if the netgame is ending, don't do any sync processing
	if(multi_endgame_ending()){
		return;
	}

	// process appropriateliy
	switch(Multi_sync_mode){
	case MULTI_SYNC_PRE_BRIEFING:		
		multi_sync_pre_do();		
		break;
	case MULTI_SYNC_POST_BRIEFING:
		multi_sync_post_do();
		break;
	case MULTI_SYNC_INGAME:
		multi_ingame_sync_do();

		gr_reset_clip();		
		GR_MAYBE_CLEAR_RES(Multi_sync_bitmap);
		if(Multi_sync_bitmap != -1){
			gr_set_bitmap(Multi_sync_bitmap);
			gr_bitmap(0,0);
		}
		Multi_sync_window.draw();

		multi_sync_blit_screen_all();

		gr_flip();
		break;
	}	
}

// perform the correct close functions
void multi_sync_close()
{
	switch(Multi_sync_mode){
	case MULTI_SYNC_PRE_BRIEFING:
		multi_sync_pre_close();
		break;
	case MULTI_SYNC_POST_BRIEFING:
		multi_sync_post_close();
		break;
	case MULTI_SYNC_INGAME:
		multi_ingame_sync_close();
		break;
	}
	
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		multi_sync_common_close();
	}
}

char *multi_sync_tooltip_handler(char *str)
{
	if (!stricmp(str, NOX("@launch"))) {
		if (Multi_launch_button_created){
			return XSTR("Launch",801);
		}
	}

	return NULL;
}

void multi_sync_common_init()
{
	int idx;

	// create the interface window
	Multi_sync_window.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);
	Multi_sync_window.set_mask_bmap(Multi_sync_bitmap_mask_fname[gr_screen.res]);
	Multi_sync_window.tooltip_handler = multi_sync_tooltip_handler;

	// load the background bitmap
	Multi_sync_bitmap = bm_load(Multi_sync_bitmap_fname[gr_screen.res]);
	if (Multi_sync_bitmap < 0) {
		// we failed to load the bitmap - this is very bad
		Int3();
	}

	// initialize the player list data
	Multi_sync_plist_start = 0;
	Multi_sync_plist_count = 1;			// we can pretty safely assume that there's one player in the game - me.	

	Multi_launch_button_created = 0;	

	// create the chatbox thingie	(shouldn't be necesary to do this, but we'll put it in for good measure)
	chatbox_create();

	// force the chatbox to be small
	chatbox_force_small();

	// initialize the common notification messaging
	multi_common_notify_init();

	// initialize the common mission info display area.
	multi_common_set_text("");	

	// use the common interface palette
	multi_common_set_palette();

	// don't select any player yet.
	Multi_sync_player_select = -1;
	
	// determine how many of the 5 buttons to create
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
		Multi_sync_button_count = MULTI_SYNC_HOST_COUNT;		
	} else {
		Multi_sync_button_count = MULTI_SYNC_CLIENT_COUNT;
	}
	// create the interface buttons	
	for(idx=0; idx<Multi_sync_button_count; idx++){
		// create the object
		Multi_sync_buttons[gr_screen.res][idx].button.create(&Multi_sync_window, "", Multi_sync_buttons[gr_screen.res][idx].x, Multi_sync_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_sync_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		// this wierdness is necessary because cancel and kick buttons aren't drawn on the background bitmap,
		//   so we have to load in frame 0, too (the file should exist)
		if ((idx == MS_CANCEL) || (idx == MS_KICK) || (idx == MS_LAUNCH)) {
			Multi_sync_buttons[gr_screen.res][idx].button.set_bmaps(Multi_sync_buttons[gr_screen.res][idx].filename, 3, 0);
		} else {
			Multi_sync_buttons[gr_screen.res][idx].button.set_bmaps(Multi_sync_buttons[gr_screen.res][idx].filename);
		}

		// set the hotspot
		Multi_sync_buttons[gr_screen.res][idx].button.link_hotspot(Multi_sync_buttons[gr_screen.res][idx].hotspot);
	}		

	// add xstrs
	for(idx=0; idx<MULTI_SYNC_NUM_TEXT; idx++) {
		// don't create the "launch" button text just yet
		if(idx == MST_LAUNCH) {
			continue;
		}
		// multiplayer clients should ignore the kick button
		if(!MULTIPLAYER_MASTER && !MULTIPLAYER_HOST && (idx == MST_KICK)) {
			continue;
		}

		Multi_sync_window.add_XSTR(&Multi_sync_text[gr_screen.res][idx]);
	}

	// create the player list select button and hide it
	Multi_sync_plist_button.create(&Multi_sync_window, "", Ms_status_coords[gr_screen.res][MS_X_COORD], Ms_status_coords[gr_screen.res][MS_Y_COORD], Ms_status_coords[gr_screen.res][MS_W_COORD], Ms_status_coords[gr_screen.res][MS_H_COORD], 0, 1);
	Multi_sync_plist_button.hide();

	// set up hotkeys for certain common functions
	Multi_sync_buttons[gr_screen.res][MS_CANCEL].button.set_hotkey(KEY_ESC);
}

void multi_sync_common_do()
{	
	int k = chatbox_process();
	k = Multi_sync_window.process(k);	

	// process the player list
	multi_sync_handle_plist();

	// process any button clicks
	multi_sync_check_buttons();	

	// process any keypresses
	switch(k){
	case KEY_ESC :
		// Sync_test = 1;
		gamesnd_play_iface(SND_USER_SELECT);
		multi_quit_game(PROMPT_ALL);		
		break;	
	}				
}

void multi_sync_common_close()
{
	// unload any bitmaps
	if(!bm_unload(Multi_sync_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",Multi_sync_bitmap_fname[gr_screen.res]));
	}	
	
	// destroy the UI_WINDOW
	Multi_sync_window.destroy();
}

void multi_sync_blit_screen_all()
{
	int count,idx;	
	int state;
	float pct_complete;
	char txt[255];
	
	// display any text in the info area
	multi_common_render_text();

	// display any pending notification messages
	multi_common_notify_do();
	
	// display any info about visible players
	count = 0;	
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
			// display his name and status
			multi_sync_display_name(Net_players[idx].m_player->callsign,count,idx);
	
			// get the player state
			state = Net_players[idx].state;

			// if we're ingame joining, show all other players except myself as "playing"
			if((Net_player != NULL) && (&Net_players[idx] != Net_player) && ((Multi_sync_mode == MULTI_SYNC_INGAME) || (Net_player->flags & NETINFO_FLAG_INGAME_JOIN)) ){
				state = NETPLAYER_STATE_IN_MISSION;
			}

			switch(state){				
			case NETPLAYER_STATE_MISSION_LOADING:
				multi_sync_display_status(XSTR("Mission Loading",802),count);
				break;
			case NETPLAYER_STATE_INGAME_SHIP_SELECT:									// I don't think its possible to see this state, but...
				multi_sync_display_status(XSTR("Ingame Ship Select",803),count);
				break;
			case NETPLAYER_STATE_DEBRIEF:
				multi_sync_display_status(XSTR("Debriefing",804),count);
				break;
			case NETPLAYER_STATE_MISSION_SYNC:
				multi_sync_display_status(XSTR("Mission Sync",805),count);
				break;
			case NETPLAYER_STATE_JOINING:								
				multi_sync_display_status(XSTR("Joining",806),count);
				break;
			case NETPLAYER_STATE_JOINED:				
				multi_sync_display_status(XSTR("Joined",807),count);
				break;
			case NETPLAYER_STATE_SLOT_ACK :				
				multi_sync_display_status(XSTR("Slot Ack",808),count);
				break;			
			case NETPLAYER_STATE_BRIEFING:				
				multi_sync_display_status(XSTR("Briefing",765),count);
				break;
			case NETPLAYER_STATE_SHIP_SELECT:				
				multi_sync_display_status(XSTR("Ship Select",809),count);
				break;
			case NETPLAYER_STATE_WEAPON_SELECT:				
				multi_sync_display_status(XSTR("Weapon Select",810),count);
				break;
			case NETPLAYER_STATE_WAITING:				
				multi_sync_display_status(XSTR("Waiting",811),count);
				break;
			case NETPLAYER_STATE_IN_MISSION:				
				multi_sync_display_status(XSTR("In Mission",812),count);
				break;			
			case NETPLAYER_STATE_MISSION_LOADED:			
				multi_sync_display_status(XSTR("Mission Loaded",813),count);
				break;			
			case NETPLAYER_STATE_DATA_LOAD:
				multi_sync_display_status(XSTR("Data loading",814),count);
				break;
			case NETPLAYER_STATE_SETTINGS_ACK:
				multi_sync_display_status(XSTR("Ready To Enter Mission",815),count);
				break;					
			case NETPLAYER_STATE_INGAME_SHIPS:
				multi_sync_display_status(XSTR("Ingame Ships Packet Ack",816),count);
				break;
			case NETPLAYER_STATE_INGAME_WINGS:
				multi_sync_display_status(XSTR("Ingame Wings Packet Ack",817),count);
				break;		
			case NETPLAYER_STATE_INGAME_RPTS:
				multi_sync_display_status(XSTR("Ingame Respawn Points Ack",818),count);
				break;
			case NETPLAYER_STATE_SLOTS_ACK:
				multi_sync_display_status(XSTR("Ingame Weapon Slots Ack",819),count);
				break;			
			case NETPLAYER_STATE_POST_DATA_ACK:
				multi_sync_display_status(XSTR("Post Briefing Data Block Ack",820),count);
				break;
			case NETPLAYER_STATE_FLAG_ACK :
				multi_sync_display_status(XSTR("Flags Ack",821),count);
				break;
			case NETPLAYER_STATE_MT_STATS :
				multi_sync_display_status(XSTR("Parallax Online Stats Updating",822),count);
				break;
			case NETPLAYER_STATE_WSS_ACK :
				multi_sync_display_status(XSTR("Weapon Slots Ack",823),count);
				break;
			case NETPLAYER_STATE_HOST_SETUP :
				multi_sync_display_status(XSTR("Host setup",824),count);
				break;
			case NETPLAYER_STATE_DEBRIEF_ACCEPT:
				multi_sync_display_status(XSTR("Debrief accept",825),count);
				break;
			case NETPLAYER_STATE_DEBRIEF_REPLAY:
				multi_sync_display_status(XSTR("Debrief replay",826),count);
				break;
			case NETPLAYER_STATE_CPOOL_ACK:
				multi_sync_display_status(XSTR("Campaign ship/weapon ack",827),count);
				break;				
			case NETPLAYER_STATE_MISSION_XFER :				
				memset(txt,0,255);
				// server should display the pct completion of all clients				
				if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
					if(Net_players[idx].s_info.xfer_handle != -1){					
						pct_complete = multi_xfer_pct_complete(Net_players[idx].s_info.xfer_handle);

						// if we've got a valid xfer handle
						if((pct_complete >= 0.0) && (pct_complete <= 1.0)){						
							sprintf(txt,XSTR("Mission file xfer %d%%",828),(int)(pct_complete * 100.0f));
						}
						// otherwise
						else {
							strcpy_s(txt,XSTR("Mission file xfer",829));
						}					
					} else {
						strcpy_s(txt,XSTR("Mission file xfer",829));
					}
				}
				// clients should display only for themselves (which is the only thing they know)
				else {
					// if we've got a valid file xfer handle
					if((&Net_players[idx] == Net_player) && (Net_player->s_info.xfer_handle != -1)){
						pct_complete = multi_xfer_pct_complete(Net_player->s_info.xfer_handle);

						// if we've got a valid xfer handle
						if((pct_complete >= 0.0) && (pct_complete <= 1.0)){						
							sprintf(txt,XSTR("Mission file xfer %d%%",828),(int)(pct_complete * 100.0f));
						}
						// otherwise
						else {
							strcpy_s(txt,XSTR("Mission file xfer",829));
						}
					}
					// otherwise
					else {
						strcpy_s(txt,XSTR("Mission file xfer",829));
					}
				}

				// display the text
				multi_sync_display_status(txt,count);
				break;
			default :
				nprintf(("Network","Unhandled player state : %d !\n",Net_players[idx].state));
				break;
			}
			count++;
		}
	}	

	// display the mission start countdown timer (if any)
	//anim_render_all(GS_STATE_MULTI_MISSION_SYNC,flFrametime);
	if((gameseq_get_state() == GS_STATE_MULTI_MISSION_SYNC && Multi_sync_countdown_timer > -1.0f) || (Multi_sync_countdown != -1))
		generic_anim_render(&Multi_sync_countdown_anim, flFrametime, Multi_sync_countdown_coords[gr_screen.res][MS_X_COORD], Multi_sync_countdown_coords[gr_screen.res][MS_Y_COORD]);


	// process and show the chatbox thingie	
	chatbox_render();

	// draw tooltips
	Multi_sync_window.draw_tooltip();

	// display the voice status indicator
	multi_common_voice_display_status();
}

void multi_sync_check_buttons()
{
	int idx;
	for(idx=0;idx<Multi_sync_button_count;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Multi_sync_buttons[gr_screen.res][idx].button.pressed()){
			multi_sync_button_pressed(idx);
			break;
		}
	}
}

void multi_sync_button_pressed(int n)
{
	switch(n){	
	// exit the game
	case MS_CANCEL:
		gamesnd_play_iface(SND_USER_SELECT);		
		multi_quit_game(PROMPT_ALL);		
		break;	
	
	// scroll the info box up
	case MS_SCROLL_INFO_UP:
		multi_common_scroll_text_up();		
		break;
	
	// scroll the info box down
	case MS_SCROLL_INFO_DOWN:
		multi_common_scroll_text_down();
		break;	

	// KICK (host only)
	case MS_KICK:
		// if we have a currently selected player, kick him
		if(Multi_sync_player_select >= 0){
			multi_kick_player(Multi_sync_player_select);
		}
		break;
	
	// start the final launch countdown (post-sync only)
	case MS_LAUNCH:
		multi_sync_start_countdown();
		break;
	
	// doesn't do anything
	default :
		Int3();		
	}
}

void multi_sync_pre_init()
{
	int idx;

	Netgame.game_state = NETGAME_STATE_MISSION_SYNC;

	/* 	if we're in teamplay mode, always force skill level to be medium
	Karajorma - disabling cause it's just plain idiotic! if people want to play on insane let them

	if((Netgame.type_flags & NG_TYPE_TEAM) && (Net_player->flags & NETINFO_FLAG_GAME_HOST)){
		Netgame.options.skill_level = NUM_SKILL_LEVELS / 2;
		Game_skill_level = NUM_SKILL_LEVELS / 2;
		multi_options_update_netgame();
	}
	*/

	// notify everyone of when we get here
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		Net_player->state = NETPLAYER_STATE_MISSION_SYNC;
		send_netplayer_update_packet();
	}
	
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){	
		// NETLOG
		ml_string(NOX("Server performing pre-briefing data sync"));

		if(!(Game_mode & GM_STANDALONE_SERVER)){
			multi_common_set_text(XSTR("Server performing sync\n",830),1);
		}
		
		// maybe initialize tvt and squad war stuff
		if(Netgame.type_flags & NG_TYPE_TEAM){
			multi_team_level_init();
		}	

		// force everyone into this state		
		send_netgame_update_packet();		

		if(!(Game_mode & GM_STANDALONE_SERVER)){
			multi_common_add_text(XSTR("Send update packet\n",831),1);
		}

		// setup some of my own data
		Net_player->flags |= NETINFO_FLAG_MISSION_OK;		

		// do any output stuff
		if (Game_mode & GM_STANDALONE_SERVER) {
			std_debug_set_standalone_state_string("Mission Sync");			
		}		

		// do this here to insure we have the most up to date file checksum info
		multi_get_mission_checksum(Game_current_mission_filename);
		// parse_get_file_signature(Game_current_mission_filename);
		
		if(!(Game_mode & GM_STANDALONE_SERVER)){
			multi_common_add_text(XSTR("Got file signatures\n",832),1);
		}
	} else {
		if(!(Game_mode & GM_STANDALONE_SERVER)){
			multi_common_add_text(XSTR("Sending update state packet\n",833),1);
		}
	}

	// if we're not in team vs. team mode - set all player teams to be 0, and unset all captaincy bits
	if(!(Netgame.type_flags & NG_TYPE_TEAM)){
		for(idx=0;idx<MAX_PLAYERS;idx++){
			Net_players[idx].p_info.team = 0;
			Net_players[idx].flags &= ~(NETINFO_FLAG_TEAM_CAPTAIN);
		}
	}

	// we aren't necessarily xferring the mission file yet	
	Assert(Net_player->s_info.xfer_handle == -1);

	// always call this for good measure
	multi_campaign_flush_data();

	Mission_sync_flags = 0;
	Multi_mission_loaded = 0;
}

void multi_sync_pre_do()
{		
	int idx;
	
	// If I'm the server, wait for everyone to arrive in this state, then begin transferring data, etc.
	// all servers (standalone or no, go through this)
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		// wait for everyone to arrive, then request filesig from all of them
		if(multi_netplayer_state_check(NETPLAYER_STATE_MISSION_SYNC) && !(Mission_sync_flags & MS_FLAG_SENT_FILESIG) && !multi_endgame_ending()){
			send_file_sig_request(Netgame.mission_name);
			Mission_sync_flags |= MS_FLAG_SENT_FILESIG;

			if(!(Game_mode & GM_STANDALONE_SERVER)){
				multi_common_add_text(XSTR("Sent filesig request\n",834),1);
			}
		}		

		// if we're waiting for players to receive files, then check on their status
		if((Mission_sync_flags & MS_FLAG_SENT_FILESIG) && !multi_netplayer_flag_check(NETINFO_FLAG_MISSION_OK) && !multi_endgame_ending()){
			for(idx=0;idx<MAX_PLAYERS;idx++){
				// if this player is in the process of xferring a file
				if(MULTI_CONNECTED(Net_players[idx]) && (Net_players[idx].s_info.xfer_handle != -1)){
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
			}
		}

		// NOTE : this is now obsolete
		// once everyone is verified, do any data transfer necessary
		if(multi_netplayer_flag_check(NETINFO_FLAG_MISSION_OK) && !(Mission_sync_flags & MS_FLAG_DATA_DONE) && !multi_endgame_ending()){
			// do nothing for now
			Mission_sync_flags |= MS_FLAG_DATA_DONE;						
			
			// send campaign pool data
			multi_campaign_send_pool_status();
		}

		// wait for everyone to ack on campaign pool data (even in non-campaign situations)
		if((Mission_sync_flags & MS_FLAG_DATA_DONE) && !(Mission_sync_flags & MS_FLAG_CAMP_DONE) && !multi_endgame_ending()){
			// check to see if everyone has acked the campaign pool data
			if(multi_netplayer_state_check(NETPLAYER_STATE_CPOOL_ACK)){
				Mission_sync_flags |= MS_FLAG_CAMP_DONE;
			}
		}
				
		// once everyone is verified, tell them to load the mission
		// also make sure to load the mission myself _AFTER_ telling everyone to do so. This makes the whole process
		// move along faster
		if((Mission_sync_flags & MS_FLAG_CAMP_DONE) && !(Mission_sync_flags & MS_FLAG_SENT_LOAD) && !multi_endgame_ending()){		
			send_netplayer_load_packet(NULL);			
			Mission_sync_flags |= MS_FLAG_SENT_LOAD;

			if(!(Game_mode & GM_STANDALONE_SERVER)){
				multi_common_add_text(XSTR("Sent load packet\n",835),1);
			}

			// load the mission myself, as soon as possible
			if(!Multi_mission_loaded){
				nprintf(("Network","Server loading mission..."));
	
				// update everyone about my status
				Net_player->state = NETPLAYER_STATE_MISSION_LOADING;
				send_netplayer_update_packet();

				game_start_mission();
				psnet_flush();
				nprintf(("Network","Done\n"));
				Multi_mission_loaded = 1;				

				// update everyone about my status
				Net_player->state = NETPLAYER_STATE_MISSION_LOADED;
				send_netplayer_update_packet();

				if(!(Game_mode & GM_STANDALONE_SERVER)){
					multi_common_add_text(XSTR("Loaded mission locally\n",836),1);
				}
			}
		}
		
		// if everyone has loaded the mission, randomly assign players to ships 
		if(multi_netplayer_state_check(NETPLAYER_STATE_MISSION_LOADED) && !(Mission_sync_flags & MS_FLAG_TS_SLOTS) && !multi_endgame_ending()){
			// call the team select function to assign players to their ships, wings, etc
			multi_ts_assign_players_all();
			send_netplayer_slot_packet();										

			// mark this flag
			Mission_sync_flags |= MS_FLAG_TS_SLOTS;
		}

		// if everyone has loaded the mission, move to the team select stage
		if(Sync_test && multi_netplayer_state_check(NETPLAYER_STATE_SLOT_ACK) && !(Mission_sync_flags & MS_FLAG_PUSHED_BRIEFING) && !multi_endgame_ending()){
			Netgame.game_state = NETGAME_STATE_BRIEFING;
			send_netgame_update_packet();   // this will push everyone into the next state

			// the standalone moves to his own wait state, whereas in the normal game mode, the server/host moves in to the
			// team select state
			if(Game_mode & GM_STANDALONE_SERVER){
				gameseq_post_event(GS_EVENT_MULTI_STD_WAIT);
			} else {
				gameseq_post_event(GS_EVENT_START_GAME);					
			}

			Mission_sync_flags |= MS_FLAG_PUSHED_BRIEFING;

			if(!(Game_mode & GM_STANDALONE_SERVER)){
				multi_common_add_text(XSTR("Moving to team select\n",837),1);
			}
		}		
	} else {
		// clients should detect here if they are doing a file xfer and do error processing
		if((Net_player->state == NETPLAYER_STATE_MISSION_XFER) && (Net_player->s_info.xfer_handle != -1) && !multi_endgame_ending()){
			switch(multi_xfer_get_status(Net_player->s_info.xfer_handle)){
			// if it has successfully completed, set his ok flag
			case MULTI_XFER_SUCCESS :	
				// release my xfer handle
				multi_xfer_release_handle(Net_player->s_info.xfer_handle);
				Net_player->s_info.xfer_handle = -1;				
				break;
				
			// if it has failed or timed-out, kick the player
			case MULTI_XFER_TIMEDOUT:
			case MULTI_XFER_FAIL:
				// release my xfer handle
				multi_xfer_release_handle(Net_player->s_info.xfer_handle);
				Net_player->s_info.xfer_handle = -1;

				// leave the game qith an error code
				multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_XFER_FAIL);
				break;
			}
		}
	}

	// blit stuff
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		gr_reset_clip();
		GR_MAYBE_CLEAR_RES(Multi_sync_bitmap);
		if(Multi_sync_bitmap != -1){
			gr_set_bitmap(Multi_sync_bitmap);
			gr_bitmap(0,0);
		}
		Multi_sync_window.draw();

		multi_sync_blit_screen_all();

		gr_flip();
	}
}

void multi_sync_pre_close()
{
	// at this point, we should shut down any file xfers...
	if(Net_player->s_info.xfer_handle != -1){
		nprintf(("Network","WARNING - killing file xfer while leaving mission sync state!!!\n"));

		multi_xfer_abort(Net_player->s_info.xfer_handle);
		Net_player->s_info.xfer_handle = -1;
	}
}

void multi_sync_post_init()
{   	
	multi_reset_timestamps();

	Multi_state_timestamp = timestamp(0);

	// NETLOG
	ml_string(NOX("Performing post-briefing data sync"));
	
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_common_add_text(XSTR("Server performing sync\n",830),1);
	} else {
		multi_common_add_text(XSTR("Client performing sync\n",838),1);
	}

	// everyone should re-initialize these 
	init_multiplayer_stats();

	// reset all sequencing info
	multi_oo_reset_sequencing();

	// if I am not the master of the game, then send the firing information for my ship
	// to the host
	if ( !(Net_player->flags & NETINFO_FLAG_AM_MASTER) ){
		send_firing_info_packet();
	}	

	// if I'm not a standalone server, load up the countdown stuff
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		generic_anim_init(&Multi_sync_countdown_anim, Multi_sync_countdown_fname[gr_screen.res]);
		Multi_sync_countdown_anim.ani.bg_type = bm_get_type(Multi_sync_bitmap);
		generic_anim_stream(&Multi_sync_countdown_anim);
		if(Multi_sync_countdown_anim.num_frames < 1){
			nprintf(("General","WARNING!, Could not load countdown animation %s!\n",Multi_sync_countdown_fname[gr_screen.res]));
		}
	}

	// create objects for all permanent observers
	multi_obs_level_init();

	// clear the game start countdown timer
	Multi_sync_countdown_timer = -1.0f;	
	Multi_sync_countdown = -1;

	// if this is a team vs. team mission, mark all ship teams appropriately
	if(Netgame.type_flags & NG_TYPE_TEAM){
		multi_team_mark_all_ships();
	}

	Mission_sync_flags = 0;
	Multi_sync_launch_pressed = 0;
}

#define MULTI_POST_TIMESTAMP			7000

extern int create_wings();

void multi_sync_post_do()
{	
	int idx;
	
	// only if the host is also the master should he be doing this (non-standalone situation)
	if ( Net_player->flags & NETINFO_FLAG_AM_MASTER ) {

		// once everyone gets to this screen, send them the ship classes of all ships.
		if(multi_netplayer_state_check(NETPLAYER_STATE_WAITING) && !(Mission_sync_flags & MS_FLAG_POST_DATA)) {			
			// only the host should ever do this
			if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
				// at this point we want to delete all necessary ships, change all necessary ship classes, and set all weapons up
				multi_ts_create_wings();

				// update player ets settings
				for(idx=0;idx<MAX_PLAYERS;idx++){
					if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_PERM_OBSERVER(Net_players[idx]) && (Net_players[idx].m_player->objnum != -1)){
						multi_server_update_player_weapons(&Net_players[idx],&Ships[Objects[Net_players[idx].m_player->objnum].instance]);
					}
				}			
			}			

			// note that this is done a little differently for standalones and nonstandalones
			send_post_sync_data_packet();
			
			multi_common_add_text(XSTR("Sending post briefing block information\n",839),1);			

			Mission_sync_flags |= MS_FLAG_POST_DATA;
		}

		// send weapon slots data
		if(multi_netplayer_state_check(NETPLAYER_STATE_POST_DATA_ACK) && !(Mission_sync_flags & MS_FLAG_WSS_SLOTS)) {			
			// note that this is done a little differently for standalones and nonstandalones
			if(Netgame.type_flags & NG_TYPE_TEAM){
				send_wss_slots_data_packet(0,0);
				send_wss_slots_data_packet(1,1);
			} else {
				send_wss_slots_data_packet(0,1);
			}
			
			multi_common_add_text(XSTR("Sending weapon slots information\n",840),1);			

			Mission_sync_flags |= MS_FLAG_WSS_SLOTS;
		}
			
		// once weapon information is received, send player settings info
		if ( multi_netplayer_state_check(NETPLAYER_STATE_WSS_ACK) && !(Mission_sync_flags & MS_FLAG_PSETTINGS)) {									
			send_player_settings_packet();
			
			// server (specifically, the standalone), should set this here
			Net_player->state = NETPLAYER_STATE_SETTINGS_ACK;
			send_netplayer_update_packet();
			
			multi_common_add_text(XSTR("Sending player settings packets\n",841),1);				

			Mission_sync_flags |= MS_FLAG_PSETTINGS;			
		}					

		// check to see if the countdown timer has started and act appropriately
		if( Multi_sync_countdown_timer > -1.0f ) {

			// increment by frametime.
			Multi_sync_countdown_timer += flFrametime;

			// if the next second has expired
			if( Multi_sync_countdown_timer >= 1.0f ) {

				Multi_sync_countdown--;
				Multi_sync_countdown_timer = 0.0f;

				// if the countdown has reached 0, launch the mission
				if(Multi_sync_countdown == 0){
					Multi_sync_countdown_timer = -1.0f;

					Multi_sync_launch_pressed = 0;
					multi_sync_launch();
				}
				// otherwise send a countdown packet
				else {
					send_countdown_packet(Multi_sync_countdown);
				}
			}
		}
			
		// jump into the mission myself
		if((Multi_sync_countdown == 0) && multi_netplayer_state_check(NETPLAYER_STATE_IN_MISSION)){
			if(!((Net_player->flags & NETINFO_FLAG_GAME_HOST) && !Multi_sync_launch_pressed)){							
				Net_player->state = NETPLAYER_STATE_IN_MISSION;
				Netgame.game_state = NETGAME_STATE_IN_MISSION;				
				gameseq_post_event(GS_EVENT_ENTER_GAME);
			
				multi_common_add_text(XSTR("Moving into game\n",842),1);			
			}
		}
	}

	// host - specific stuff
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
		// create the launch button so the host can click
		if( Sync_test && multi_netplayer_state_check(NETPLAYER_STATE_SETTINGS_ACK) ){
			multi_sync_create_launch_button();
		}
	}

	// blit stuff
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		gr_reset_clip();	
		GR_MAYBE_CLEAR_RES(Multi_sync_bitmap);
		if(Multi_sync_bitmap != -1){
			gr_set_bitmap(Multi_sync_bitmap);
			gr_bitmap(0,0);
		}
		Multi_sync_window.draw();

		multi_sync_blit_screen_all();

		gr_flip();
	}
}

void multi_sync_post_close()
{
	int idx;

	if(Multi_sync_countdown_anim.num_frames > 0)
		generic_anim_unload(&Multi_sync_countdown_anim);
	
	// all players should reset sequencing
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(Net_player->flags & NETINFO_FLAG_CONNECTED){
			Net_players[idx].client_cinfo_seq = 0;
			Net_players[idx].client_server_seq = 0;	
		}
	}

	// multiplayer dogfight
	multi_df_level_pre_enter();				

	// clients should clear obj_pair array and add pair for themselves
	/*
	if ( MULTIPLAYER_CLIENT ) {
		obj_reset_pairs();
		obj_add_pairs( OBJ_INDEX(Player_obj) );	
	}
	*/
}

void multi_sync_display_name(char *name,int index,int np_index)
{
	char fit[CALLSIGN_LEN];	
	
	// make sure the string actually fits
	strcpy_s(fit,name);

	// if we're in team vs. team mode
	if(Netgame.type_flags & NG_TYPE_TEAM){
		gr_force_fit_string(fit,CALLSIGN_LEN, Ms_status2_coords[gr_screen.res][MS_X_COORD] - Ms_status_coords[gr_screen.res][MS_X_COORD] - 20 - Ms_cd_icon_offset[gr_screen.res] - Ms_team_icon_offset[gr_screen.res]);			

		// if this is the currently selected player, draw him highlighted
		if(np_index == Multi_sync_player_select){
			gr_set_color_fast(&Color_text_selected);
		} else {
			gr_set_color_fast(&Color_text_normal);
		}

		// blit the string
		gr_string(Ms_status_coords[gr_screen.res][0] + Ms_cd_icon_offset[gr_screen.res] + Ms_team_icon_offset[gr_screen.res], Ms_status_coords[gr_screen.res][MS_Y_COORD] + (index * 10),fit);

		// blit his team icon 
		// team 0		
		if(Net_players[np_index].p_info.team == 0){
			// blit the team captain icon
			if(Net_players[np_index].flags & NETINFO_FLAG_TEAM_CAPTAIN){				
				if(Multi_common_icons[MICON_TEAM0_SELECT] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM0_SELECT]);
					gr_bitmap(Ms_status_coords[gr_screen.res][MS_X_COORD] + Ms_cd_icon_offset[gr_screen.res], Ms_status_coords[gr_screen.res][MS_Y_COORD] + (index * 10) - 2);
				} 
			}
			// normal team member icon
			else {
				if(Multi_common_icons[MICON_TEAM0] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM0]);
					gr_bitmap(Ms_status_coords[gr_screen.res][MS_X_COORD] + Ms_cd_icon_offset[gr_screen.res], Ms_status_coords[gr_screen.res][MS_Y_COORD] + (index * 10) - 2);
				}
			}
		}
		// team 1
		else if(Net_players[np_index].p_info.team == 1){			
			// blit the team captain icon
			if(Net_players[np_index].flags & NETINFO_FLAG_TEAM_CAPTAIN){
				if(Multi_common_icons[MICON_TEAM1_SELECT] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM1_SELECT]);
					gr_bitmap(Ms_status_coords[gr_screen.res][MS_X_COORD] + Ms_cd_icon_offset[gr_screen.res], Ms_status_coords[gr_screen.res][MS_Y_COORD] + (index * 10) - 2);
				}
			}
			// normal team member icon
			else {
				if(Multi_common_icons[MICON_TEAM1] != -1){
					gr_set_bitmap(Multi_common_icons[MICON_TEAM1]);
					gr_bitmap(Ms_status_coords[gr_screen.res][MS_X_COORD] + Ms_cd_icon_offset[gr_screen.res], Ms_status_coords[gr_screen.res][MS_Y_COORD] + (index * 10) - 2);
				}
			}
		}		
	} else {
		gr_force_fit_string(fit, CALLSIGN_LEN, Ms_status2_coords[gr_screen.res][MS_X_COORD] - Ms_status_coords[gr_screen.res][MS_X_COORD] - 20 - Ms_cd_icon_offset[gr_screen.res]);

		// if this is the currently selected player, draw him highlighted
		if(np_index == Multi_sync_player_select){
			gr_set_color_fast(&Color_text_selected);
		} else {
			gr_set_color_fast(&Color_text_normal);
		}

		// blit the string
		gr_string(Ms_status_coords[gr_screen.res][MS_X_COORD] + Ms_cd_icon_offset[gr_screen.res], Ms_status_coords[gr_screen.res][MS_Y_COORD] + (index * 10),fit);
	}

	// maybe blit his CD status icon
	if((Net_players[np_index].flags & NETINFO_FLAG_HAS_CD) && (Multi_common_icons[MICON_CD] != -1)){
		gr_set_bitmap(Multi_common_icons[MICON_CD]);
		gr_bitmap(Ms_status_coords[gr_screen.res][MS_X_COORD], Ms_status_coords[gr_screen.res][MS_Y_COORD] + (index * 10));
	}
}

void multi_sync_display_status(char *status,int index)
{
	char fit[250];

	// make sure the string actually fits
	strcpy_s(fit, status);
	gr_force_fit_string(fit, 250, Ms_status2_coords[gr_screen.res][MS_W_COORD] - 20);
	gr_set_color_fast(&Color_bright);	
	gr_string(Ms_status2_coords[gr_screen.res][MS_X_COORD], Ms_status2_coords[gr_screen.res][MS_Y_COORD] + (index * 10), fit);		
}

void multi_sync_force_start_pre()
{
	int idx;
	int want_state = NETPLAYER_STATE_SLOT_ACK;	// kick any players who are still in this state

	// go through the player list and boot anyone who isn't in the right state
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_player != &Net_players[idx]) && (Net_players[idx].state == want_state)){		
			multi_kick_player(idx,0);				
		}
	}
}

void multi_sync_force_start_post()
{
	int idx,idx2;
	int kill_state[3];	
	int num_kill_states;
	
	// determine the state we want all players in so that we can find those who are not in the state	
	kill_state[0] = NETPLAYER_STATE_BRIEFING;
	kill_state[1] = NETPLAYER_STATE_SHIP_SELECT;
	kill_state[2] = NETPLAYER_STATE_WEAPON_SELECT;
	num_kill_states = 3;	

	// go through the player list and boot anyone who isn't in the right state
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && (Net_player != &Net_players[idx])){
			// check against all kill state
			for(idx2 = 0;idx2<num_kill_states;idx2++){
				if(Net_players[idx].state == kill_state[idx2]){
					multi_kick_player(idx,0);
					break;
				}
			}
		}
	}
}

void multi_sync_start_countdown()
{
	// don't allow repeat button presses
	if(Multi_sync_launch_pressed){
		return;
	}
	
	Multi_sync_launch_pressed = 1;

	// if I'm the server, begin the countdown
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		gamesnd_play_iface(SND_COMMIT_PRESSED);
		Multi_sync_countdown_timer = 0.0f;
		Multi_sync_countdown = MULTI_SYNC_COUNTDOWN_TIME;

		// send an initial countdown value
		send_countdown_packet(Multi_sync_countdown);
	}
	// otherwise send the "start countdown" packet to the standalone
	else {
		Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);
		send_countdown_packet(-1);
	}
}

void multi_sync_launch()
{	
	// don't allow repeat button presses
	if(Multi_sync_launch_pressed){
		return;
	}

	Multi_sync_launch_pressed = 1;

	// NETLOG
	ml_printf(NOX("Entering mission %s"), Game_current_mission_filename);

	// tell everyone to jump into the mission
	send_jump_into_mission_packet();
	Multi_state_timestamp = timestamp(MULTI_POST_TIMESTAMP);

	// set the # of players at the start of the mission
	Multi_num_players_at_start = multi_num_players();
	nprintf(("Network","# of players at start of mission : %d\n", Multi_num_players_at_start));
	
	// initialize datarate limiting for all clients
	multi_oo_rate_init_all();	
				
	multi_common_add_text(XSTR("Sending mission start packet\n",843),1);				
}

void multi_sync_create_launch_button()
{
	if (!Multi_launch_button_created) {		
		// create the object
		Multi_sync_buttons[gr_screen.res][MS_LAUNCH].button.create(&Multi_sync_window, "", Multi_sync_buttons[gr_screen.res][MS_LAUNCH].x, Multi_sync_buttons[gr_screen.res][MS_LAUNCH].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_sync_buttons[gr_screen.res][MS_LAUNCH].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_sync_buttons[gr_screen.res][MS_LAUNCH].button.set_bmaps(Multi_sync_buttons[gr_screen.res][MS_LAUNCH].filename, 3, 0);

		// set the hotspot
		Multi_sync_buttons[gr_screen.res][MS_LAUNCH].button.link_hotspot(Multi_sync_buttons[gr_screen.res][MS_LAUNCH].hotspot);

		// hotkey
		Multi_sync_buttons[gr_screen.res][MS_LAUNCH].button.set_hotkey(KEY_CTRLED+KEY_ENTER);

		// create the text for the button
		Multi_sync_window.add_XSTR(&Multi_sync_text[gr_screen.res][MST_LAUNCH]);

		// increment the button count so we start checking this one
		Multi_sync_button_count++;

		Multi_launch_button_created = 1;
	}
}

void multi_sync_handle_plist()
{
	int idx;
	int my;
	int select_index;
	
	// if we don't have a currently selected player, select one
	if((Multi_sync_player_select < 0) || !MULTI_CONNECTED(Net_players[Multi_sync_player_select])){
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
				Multi_sync_player_select = idx;
				break;
			}
		}
	}
	
	// check for button list presses
	if(Multi_sync_plist_button.pressed()){
		// get the y mouse coords
		Multi_sync_plist_button.get_mouse_pos(NULL,&my);

		// get the index of the item selected
		select_index = my / 10;

		// if the index is greater than the current # connections, do nothing
		if(select_index > (multi_num_connections() - 1)){
			return;
		}

		// translate into an absolute Net_players[] index (get the Nth net player)
		Multi_sync_player_select = -1;
		for(idx=0;idx<MAX_PLAYERS;idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
				select_index--;
			}

			// if we've found the item we're looking for
			if(select_index < 0){
				Multi_sync_player_select = idx;
				break;
			}
		}

		// if for some bizarre reason, this is an invalid player, unselect him and wait for the next interation
		// of the loop
		if((Multi_sync_player_select >= 0) && (!MULTI_CONNECTED(Net_players[Multi_sync_player_select]) || MULTI_STANDALONE(Net_players[Multi_sync_player_select])) ){
			Multi_sync_player_select = -1;
		}
	}
}


// -------------------------------------------------------------------------------------------------------------
// 
// MULTIPLAYER DEBRIEF SCREEN 
//

// other relevant data
int Multi_debrief_accept_hit;
int Multi_debrief_replay_hit;

// set if the server has left the game
int Multi_debrief_server_left = 0;

// if we've reported on TvT status all players are in the debrief
int Multi_debrief_reported_tvt = 0;

// whether stats are being accepted
// -1 == no decision yet
// 1 == accepted
// 0 == tossed

// kazan - for internal purposes stats are always accepted by default
int Multi_debrief_stats_accept_code = 1;

int Multi_debrief_server_framecount = 0;

float Multi_debrief_time = 0.0f;
float Multi_debrief_resend_time = 10.0f;

void multi_debrief_init()
{			
	int idx;

	Multi_debrief_time = 0.0f;
	Multi_debrief_resend_time = 10.0f;
	
	// do this to notify the standalone or the normal server that we're in the debrief state and ready to receive packets
	if (!(Net_player->flags & NETINFO_FLAG_AM_MASTER)) {
		Net_player->state = NETPLAYER_STATE_DEBRIEF;
		send_netplayer_update_packet();
	}	

	// unflag some stuff
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx])){
			Net_players[idx].flags &= ~(NETINFO_FLAG_RESPAWNING | NETINFO_FLAG_LIMBO | NETINFO_FLAG_WARPING_OUT);
		}
	}
	
	// if text input mode is active, clear it
	multi_msg_text_flush();	
	
	// the server has not left yet
	Multi_debrief_server_left = 0;

	// have not hit accept or replay yet
	Multi_debrief_accept_hit = 0;
	Multi_debrief_replay_hit = 0;

	// stats have not been accepted yet
	Multi_debrief_stats_accept_code = -1;

	// mark stats as not being store yet
	Netgame.flags &= ~(NG_FLAG_STORED_MT_STATS);

	// no report on TvT yet
	Multi_debrief_reported_tvt = 0;

	Multi_debrief_server_framecount = 0;
}

void multi_debrief_do_frame()
{
	Multi_debrief_time += flFrametime;

	// set the netgame state to be debriefing when appropriate
	if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Netgame.game_state != NETGAME_STATE_DEBRIEF) && multi_netplayer_state_check3(NETPLAYER_STATE_DEBRIEF, NETPLAYER_STATE_DEBRIEF_ACCEPT, NETPLAYER_STATE_DEBRIEF_REPLAY)){
		Netgame.game_state = NETGAME_STATE_DEBRIEF;
		send_netgame_update_packet();
	}
	
	// evaluate all server stuff
	if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
		multi_debrief_server_process();
	}			
}

void multi_debrief_close()
{	
	if ( MULTIPLAYER_CLIENT && (Netgame.game_state == NETGAME_STATE_MISSION_SYNC) ){
		gamesnd_play_iface( SND_COMMIT_PRESSED );
	}
}

// handle optional mission loop
void multi_maybe_set_mission_loop()
{
	int cur = Campaign.current_mission;
	if (Campaign.missions[cur].flags & CMISSION_FLAG_HAS_LOOP) {
		Assert(Campaign.loop_mission != CAMPAIGN_LOOP_MISSION_UNINITIALIZED);
	}
	bool require_repeat_mission = (Campaign.current_mission == Campaign.next_mission);

	// check for (1) mission loop available, (2) don't have to repeat last mission
	if ( (Campaign.missions[cur].flags & CMISSION_FLAG_HAS_LOOP) && (Campaign.loop_mission != -1) && !require_repeat_mission ) {

		char buffer[512];
		debrief_assemble_optional_mission_popup_text(buffer, Campaign.missions[cur].mission_branch_desc);

		int choice = popup(0 , 2, POPUP_NO, POPUP_YES, buffer);
		if (choice == 1) {
			Campaign.loop_enabled = 1;
			Campaign.next_mission = Campaign.loop_mission;
		}
	}
}

// handle all cases for when the accept key is hit in a multiplayer debriefing
void multi_debrief_accept_hit()
{
	// if we already accepted, do nothing
	// but he may need to hit accept again after the server has left the game, so allow this
	if(Multi_debrief_accept_hit){
		return;
	}
	
	// mark this so that we don't hit it again
	Multi_debrief_accept_hit = 1;

	gamesnd_play_iface(SND_COMMIT_PRESSED);

	if (MULTI_IS_TRACKER_GAME) {
		int res = popup(PF_TITLE | PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_IGNORE_ESC, 3, XSTR("&Cancel", 779), XSTR("&Accept", 844), XSTR("&Toss", 845), XSTR("(Continue Netgame)\nDo you wish to accept these stats?", 846));

		// evaluate the result
		switch (res) {
			// undo the accept
			case -1:
			case 0:
				Multi_debrief_accept_hit = 0;
				return;

			// toss the stats
			case 2 :
				break;
		
			// accept the stats
			case 1 :
				fs2netd_store_stats();
				break;
		}
	}

	// if the server has left the game, always just end the game. 
	if(Multi_debrief_server_left){
		if(!multi_quit_game(PROMPT_ALL)){
			Multi_debrief_server_left = 1;
			Multi_debrief_accept_hit = 0;
		} else {
			Multi_debrief_server_left = 0;
		}			
	} else {		
		// query the host and see if he wants to accept stats
		if(Net_player->flags & NETINFO_FLAG_GAME_HOST){
			// if we're on a tracker game, he gets no choice for storing stats
			if (MULTI_IS_TRACKER_GAME) {
				multi_maybe_set_mission_loop();
			} else {
				int res = popup(PF_TITLE | PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_IGNORE_ESC,3,XSTR("&Cancel",779),XSTR("&Accept",844),XSTR("&Toss",845),XSTR("(Continue Netgame)\nDo you wish to accept these stats?",846));
		
				// evaluate the result
				switch(res){
				// undo the accept
				case -1:
				case 0:
					Multi_debrief_accept_hit = 0;
					return;

				// set the accept code to be "not accepting"
				case 2 :
					multi_debrief_stats_toss();
					multi_maybe_set_mission_loop();
					break;
				
				// accept the stats and continue
				case 1 :
					multi_debrief_stats_accept();
					multi_maybe_set_mission_loop();
					break;
				}
			}
		}

		// set my netplayer state to be "debrief_accept", and be done with it
		Net_player->state = NETPLAYER_STATE_DEBRIEF_ACCEPT;
		send_netplayer_update_packet();
	}
}

// handle all cases for when the escape key is hit in a multiplayer debriefing
void multi_debrief_esc_hit()
{
	int res;

	if (MULTI_IS_TRACKER_GAME) {
		res = popup(PF_TITLE | PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_IGNORE_ESC, 3, XSTR("&Cancel", 779), XSTR("&Accept", 844), XSTR("&Toss", 845), XSTR("(Exit Netgame)\nDo you wish to accept these stats?", 847));

		// evaluate the result
		switch (res) {
			// undo the accept
			case -1:
			case 0:
				break;

			// toss the stats
			case 2 :
				break;
		
			// accept the stats
			case 1 :
				fs2netd_store_stats();
				break;
		}
	}

	// if the server has left
	if(Multi_debrief_server_left){
		multi_quit_game(PROMPT_ALL);
		return;
	}
	
	// display a popup
	if(Net_player->flags & NETINFO_FLAG_GAME_HOST){		
		// if the stats have already been accepted
		if((Multi_debrief_stats_accept_code != -1) || (MULTI_IS_TRACKER_GAME)){
			multi_quit_game(PROMPT_HOST);
		} else {
			res = popup(PF_TITLE | PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_IGNORE_ESC,3,XSTR("&Cancel",779),XSTR("&Accept",844),XSTR("&Toss",845),XSTR("(Exit Netgame)\nDo you wish to accept these stats?",847));
		
			// evaluate the result
			switch(res){			
			// undo the accept
			case -1:
			case 0:				
				break;

			// set the accept code to be "not accepting"
			case 2 :
				multi_debrief_stats_toss();
				multi_quit_game(PROMPT_NONE);
				break;
				
			// accept the stats and continue
			case 1 :
				multi_debrief_stats_accept();
				multi_quit_game(PROMPT_NONE);
				break;						
			}		
		}
	} else {		
		// if the stats haven't been accepted yet, or this is a tracker game
		if((Multi_debrief_stats_accept_code == -1) && !(MULTI_IS_TRACKER_GAME)){
			res = popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON,2,XSTR("&Cancel",779),XSTR("&Leave",848),XSTR("Are you sure you want to leave the netgame before stats are stored?",849));

			// evaluate the result
			if(res == 1){
				multi_quit_game(PROMPT_NONE);
			}
		}
		// otherwise go through the normal endgame channels
		else {
			multi_quit_game(PROMPT_ALL);
		}		
	}
}

void multi_debrief_replay_hit()
{
	// only the host should ever get here
	Assert(Net_player->flags & NETINFO_FLAG_GAME_HOST);

	// if the button was already pressed, do nothing
	if(Multi_debrief_accept_hit){
		return;
	}
	
	// same as hittin the except button except no stats are kept
	Multi_debrief_accept_hit = 1;

	// mark myself as being in the replay state so we know what to do next
	Net_player->state = NETPLAYER_STATE_DEBRIEF_REPLAY;
	send_netplayer_update_packet();
}

// call this when the server has left and we would otherwise be saying "contact lost with server
void multi_debrief_server_left()
{
	// the server left
	Multi_debrief_server_left = 1;

	// undo any "accept" hit so that clients can hit accept again to leave
	Multi_debrief_accept_hit = 0;
}

void multi_debrief_stats_accept()
{
	// don't do anything if we've already accepted
	if(Multi_debrief_stats_accept_code != -1){
		return;
	}
	
	Multi_debrief_stats_accept_code = 1;

	// if we're the host, and we're on a standalone, tell the standalone to begin the stats storing process
	if((Net_player->flags & NETINFO_FLAG_GAME_HOST) || (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		// send a packet to the players telling them to store their stats
		send_store_stats_packet(1);
	} 

	// add a chat line saying "stats have been accepted"
	multi_display_chat_msg(XSTR("<stats have been accepted>",850),0,0);

	// NETLOG
	ml_string(NOX("Stats stored"));
}

void multi_debrief_stats_toss()
{
	// don't do anything if we've already accepted
	if(Multi_debrief_stats_accept_code != -1){
		return;
	}
	
	Multi_debrief_stats_accept_code = 0;

	// if we're the host, and we're on a standalone, tell everyone to "toss" stats
	// Kazan - or how about we don't - we tell them we stored them
	if((Net_player->flags & NETINFO_FLAG_GAME_HOST) || (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		// send a packet to the players telling them to store their stats
		send_store_stats_packet(1);
	} 

	// add a chat line saying "stats have been accepted"
	//multi_display_chat_msg(XSTR("<stats have been tossed>",851),0,0);

	// NETLOG
	ml_string(NOX("Stats tossed"));
}

int multi_debrief_stats_accept_code()
{
	return Multi_debrief_stats_accept_code;
}

void multi_debrief_server_process()
{	
	int idx;
	int player_status,other_status;

	Multi_debrief_server_framecount++;

	// if we're > 10 seconds into the debrief and not everyone is here, try warping everyone out again
	if((Multi_debrief_time >= Multi_debrief_resend_time) && !multi_netplayer_state_check3(NETPLAYER_STATE_DEBRIEF, NETPLAYER_STATE_DEBRIEF_ACCEPT, NETPLAYER_STATE_DEBRIEF_REPLAY, 1)){
		// find all players who are not in the debrief state and hit them with the endgame packet
		for(idx=0; idx<MAX_PLAYERS; idx++){
			if( MULTI_CONNECTED(Net_players[idx]) && (Net_player != &Net_players[idx]) && 
				( ( Net_players[idx].state != NETPLAYER_STATE_DEBRIEF ) && 
				  ( Net_players[idx].state != NETPLAYER_STATE_DEBRIEF_ACCEPT ) && 
				  ( Net_players[idx].state != NETPLAYER_STATE_DEBRIEF_REPLAY ) ) ){
				send_endgame_packet(&Net_players[idx]);
			}
		}

		// next check time
		Multi_debrief_resend_time += 7.0f;
	}

	// evaluate the status of all players in the game (0 == not ready, 1 == ready to continue, 2 == ready to replay)
	other_status = 1;

	// check all players
	for(idx=0;idx<MAX_PLAYERS;idx++){
		if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx]) && !MULTI_HOST(Net_players[idx])){					
			if(Net_players[idx].state != NETPLAYER_STATE_DEBRIEF_ACCEPT){
				other_status = 0;
				break;
			}
		}
	}

	// if we haven't already reported TvT results
	if(multi_netplayer_state_check3(NETPLAYER_STATE_DEBRIEF, NETPLAYER_STATE_DEBRIEF_ACCEPT, NETPLAYER_STATE_DEBRIEF_REPLAY) && (Netgame.type_flags & NG_TYPE_TEAM) && !Multi_debrief_reported_tvt && (Multi_debrief_server_framecount > 4)){
		multi_team_report();
		Multi_debrief_reported_tvt = 1;
	}	

	// if all other players are good to go, check the host
	if(other_status){
		// if he is ready to continue
		if(Netgame.host->state == NETPLAYER_STATE_DEBRIEF_ACCEPT){					
			player_status = 1;			
		} 
		// if he wants to replay the mission
		else if(Netgame.host->state == NETPLAYER_STATE_DEBRIEF_REPLAY){
			player_status = 2;			
		} 
		// if he is not ready
		else {
			player_status = 0;
		}
	}
	// if all players are _not_ good to go
	else {
		player_status = 0;
	}
		
	// if we're in the debriefing state in a campaign mode, process accordingly
	if(Netgame.campaign_mode == MP_CAMPAIGN){
		multi_campaign_do_debrief(player_status);
	}
	// otherwise process as normal (looking for all players to be ready to go to the next mission
	else {
		if(player_status == 1){
			multi_flush_mission_stuff();

			// set the netgame state to be forming and continue
			Netgame.game_state = NETGAME_STATE_FORMING;
			send_netgame_update_packet();

			// move to the proper state
			if(Game_mode & GM_STANDALONE_SERVER){
				gameseq_post_event(GS_EVENT_STANDALONE_MAIN);
			} else {
				gameseq_post_event(GS_EVENT_MULTI_HOST_SETUP);
			}

			multi_reset_timestamps();
		} else if(player_status == 2){
			multi_flush_mission_stuff();

			// tell everyone to move into the pre-briefing sync state
			Netgame.game_state = NETGAME_STATE_MISSION_SYNC;
			send_netgame_update_packet();

			// move back to the mission sync screen for the same mission again
			Multi_sync_mode = MULTI_SYNC_PRE_BRIEFING;
			gameseq_post_event(GS_EVENT_MULTI_MISSION_SYNC);

			multi_reset_timestamps();
		}
	}	
}


// -------------------------------------------------------------------------------------------------------------
// 
// MULTIPLAYER PASSWORD POPUP
//

//XSTR:OFF
// bitmaps defs
static char *Multi_pwd_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"Password",			// GR_640
	"2_Password"		// GR_1024
};

static char *Multi_pwd_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"Password-M",		// GR_640
	"2_Password-M"		// GR_1024
};

//XSTR:ON

// constants for coordinate lookup
#define MPWD_X_COORD 0
#define MPWD_Y_COORD 1
#define MPWD_W_COORD 2
#define MPWD_H_COORD 3

// button defs
#define MULTI_PWD_NUM_BUTTONS			2
#define MPWD_CANCEL			0
#define MPWD_COMMIT			1

// password area defs
int Mpwd_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		134, 112, 367, 15
	},
	{ // GR_1024
		215, 190, 587, 24
	}
};

UI_WINDOW Multi_pwd_window;												// the window object for the join screen
UI_INPUTBOX	Multi_pwd_passwd;												// for Netgame.passwd
int Multi_pwd_bitmap;														// the background bitmap
int Multi_passwd_background = -1;
int Multi_passwd_done = -1;
int Multi_passwd_running = 0;

// password buttons
ui_button_info Multi_pwd_buttons[GR_NUM_RESOLUTIONS][MULTI_PWD_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("PWB_00",	411,	151,	405,	141,	0),
		ui_button_info("PWB_01",	460,	151,	465,	141,	1),
	}, 
	{ // GR_1024
		ui_button_info("2_PWB_00",	659,	242,	649,	225,	0),
		ui_button_info("2_PWB_01",	737,	242,	736,	225,	1),
	}, 
};

// text
#define MULTI_PWD_NUM_TEXT				3
UI_XSTR Multi_pwd_text[GR_NUM_RESOLUTIONS][MULTI_PWD_NUM_TEXT] = {
	{ // GR_640
		{ "Cancel",			387,	400,	141,	UI_XSTR_COLOR_GREEN, -1,	&Multi_pwd_buttons[0][MPWD_CANCEL].button},
		{ "Commit",			1062,	455,	141,	UI_XSTR_COLOR_GREEN, -1,	&Multi_pwd_buttons[0][MPWD_COMMIT].button},
		{ "Enter Password",	1332,	149,	92,	UI_XSTR_COLOR_GREEN, -1,	NULL},
	},
	{ // GR_1024
		{ "Cancel",			387,	649,	225,	UI_XSTR_COLOR_GREEN, -1,	&Multi_pwd_buttons[1][MPWD_CANCEL].button},
		{ "Commit",			1062,	736,	225,	UI_XSTR_COLOR_GREEN, -1,	&Multi_pwd_buttons[1][MPWD_COMMIT].button},
		{ "Enter Password",	1332,	239,	148,	UI_XSTR_COLOR_GREEN, -1,	NULL},
	}
};

// initialize all graphics, etc
void multi_passwd_init()
{
	int idx;

	// store the background as it currently is
	Multi_passwd_background = gr_save_screen();	
	
	// create the interface window
	Multi_pwd_window.create(0,0,gr_screen.max_w_unscaled,gr_screen.max_h_unscaled,0);
	Multi_pwd_window.set_mask_bmap(Multi_pwd_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Multi_pwd_bitmap = bm_load(Multi_pwd_bitmap_fname[gr_screen.res]);
	if(Multi_pwd_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}
			
	// create the interface buttons
	for(idx=0; idx<MULTI_PWD_NUM_BUTTONS; idx++){
		// create the object
		Multi_pwd_buttons[gr_screen.res][idx].button.create(&Multi_pwd_window, "", Multi_pwd_buttons[gr_screen.res][idx].x, Multi_pwd_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_pwd_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_pwd_buttons[gr_screen.res][idx].button.set_bmaps(Multi_pwd_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Multi_pwd_buttons[gr_screen.res][idx].button.link_hotspot(Multi_pwd_buttons[gr_screen.res][idx].hotspot);
	}	

	// add all xstrs
	for(idx=0; idx<MULTI_PWD_NUM_TEXT; idx++){
		Multi_pwd_window.add_XSTR(&Multi_pwd_text[gr_screen.res][idx]);
	}
	
	// create the password input box
	Multi_pwd_passwd.create(&Multi_pwd_window, Mpwd_coords[gr_screen.res][MPWD_X_COORD], Mpwd_coords[gr_screen.res][MPWD_Y_COORD],Mpwd_coords[gr_screen.res][MPWD_W_COORD], MAX_PASSWD_LEN, "", UI_INPUTBOX_FLAG_ESC_CLR | UI_INPUTBOX_FLAG_INVIS, -1, &Color_normal);
	Multi_pwd_passwd.set_focus();
	
	// link the enter key to ACCEPT
	Multi_pwd_buttons[gr_screen.res][MPWD_COMMIT].button.set_hotkey(KEY_ENTER);

	Multi_passwd_done = -1;
	Multi_passwd_running = 1;
}

// close down all graphics, etc
void multi_passwd_close()
{
	// unload any bitmaps
	bm_release(Multi_pwd_bitmap);		
		
	// destroy the UI_WINDOW
	Multi_pwd_window.destroy();

	// free up the saved background screen
	if(Multi_passwd_background >= 0){
		gr_free_screen(Multi_passwd_background);	
		Multi_passwd_background = -1;
	}

	Multi_passwd_running = 0;
}

// process any button pressed
void multi_passwd_process_buttons()
{
	// if the accept button was pressed
	if(Multi_pwd_buttons[gr_screen.res][MPWD_COMMIT].button.pressed()){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_passwd_done = 1;
	}

	// if the cancel button was pressed
	if(Multi_pwd_buttons[gr_screen.res][MPWD_CANCEL].button.pressed()){
		gamesnd_play_iface(SND_USER_SELECT);
		Multi_passwd_done = 0;
	}
}

// run the passwd popup
void multi_passwd_do(char *passwd)
{
	int k;

	while(Multi_passwd_done == -1){	
		// set frametime and run background stuff
		game_set_frametime(-1);
		game_do_state_common(gameseq_get_state());

		k = Multi_pwd_window.process();

		// process any keypresses
		switch(k){
		case KEY_ESC :							
			// set this to indicate the user has cancelled for one reason or another
			Multi_passwd_done = 0;
			break;		
		}	

		// if the input box text has changed
		if(Multi_pwd_passwd.changed()){
			strcpy(passwd,"");
			Multi_pwd_passwd.get_text(passwd);
		}

		// process any button pressed
		multi_passwd_process_buttons();
	
		// draw the background, etc
		gr_reset_clip();
		gr_clear();
		if(Multi_passwd_background >= 0){
			gr_restore_screen(Multi_passwd_background);		
		}
		gr_set_bitmap(Multi_pwd_bitmap);
		gr_bitmap(0,0);
		Multi_pwd_window.draw();
			
		// flip the buffer
		gr_flip();
	}
}

// bring up the password string popup, fill in passwd (return 1 if accept was pressed, 0 if cancel was pressed)
int multi_passwd_popup(char *passwd)
{
	// if the popup is already running for some reason, don't do anything
	if(Multi_passwd_running){
		return 0;
	}

	// initialize all graphics
	multi_passwd_init();

	// run the popup
	multi_passwd_do(passwd);

	// shut everything down
	multi_passwd_close();

	return Multi_passwd_done;
}
