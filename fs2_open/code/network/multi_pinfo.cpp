/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Network/multi_pinfo.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:08 $
 * $Author: penguin $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:26  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *  
 * 
 * 15    9/10/99 9:44p Dave
 * Bumped version # up. Make server reliable connects not have such a huge
 * timeout. 
 * 
 * 14    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 13    8/22/99 5:53p Dave
 * Scoring fixes. Added self destruct key. Put callsigns in the logfile
 * instead of ship designations for multiplayer players.
 * 
 * 12    8/03/99 11:43a Dave
 * Don't allow medals button in demo.
 * 
 * 11    7/26/99 11:14a Andsager
 * Disable medals in demo multiplayer
 * 
 * 10    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 9     1/30/99 1:29a Dave
 * Fixed nebula thumbnail problem. Full support for 1024x768 choose pilot
 * screen.  Fixed beam weapon death messages.
 * 
 * 8     1/14/99 6:06p Dave
 * 100% full squad logo support for single player and multiplayer.
 * 
 * 7     1/12/99 4:07a Dave
 * Put in barracks code support for selecting squad logos. Properly
 * distribute squad logos in a multiplayer game.
 * 
 * 6     12/14/98 12:13p Dave
 * Spiffed up xfer system a bit. Put in support for squad logo file xfer.
 * Need to test now.
 * 
 * 5     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 4     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 
 * $NoKeywords: $
 */

#include "ui/ui.h"
#include "bmpman/bmpman.h"
#include "gamesnd/gamesnd.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "io/key.h"
#include "stats/medals.h"
#include "palman/palman.h"
#include "io/mouse.h"
#include "network/multi.h"
#include "network/multi_xfer.h"
#include "network/multi_pinfo.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "render/3d.h"

// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO DEFINES/VARS
//

//XSTR:OFF

#define MULTI_PINFO_NUM_BUTTONS		4

// bitmaps defs
char *Multi_pinfo_bitmap_name[GR_NUM_RESOLUTIONS] = {
	"PilotInfo",
	"2_PilotInfo"
};
char *Multi_pinfo_bitmap_mask[GR_NUM_RESOLUTIONS] = {
	"PilotInfo-M",
	"2_PilotInfo-M"
};

// button defs
#define MPI_SCROLL_STATS_UP			0
#define MPI_SCROLL_STATS_DOWN			1
#define MPI_MEDALS						2
#define MPI_EXIT							3

// pilot image area defs
int Multi_pinfo_pilot_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		22, 159, 160, 120
	},
	{ // GR_1024
		35, 254, 256, 192
	}
};
int Multi_pinfo_squad_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		22, 299, 128, 128
	},
	{ // GR_1024
		35, 479, 205, 205
	}
};

// pilot bitmaps
typedef struct np_bitmap {
	int bitmap;									// bitmap id
	char filename[MAX_FILENAME_LEN];		// filename
} np_bitmap;
np_bitmap Mp_pilot;			// pilot pic
np_bitmap Mp_squad;			// squad logo

UI_WINDOW Multi_pinfo_window;											// the window object for the join screen
UI_BUTTON Multi_pinfo_select_button;								// for selecting list items
int Multi_pinfo_bitmap;													// the background bitmap
ui_button_info Multi_pinfo_buttons[GR_NUM_RESOLUTIONS][MULTI_PINFO_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info("PIB_00",	617,	256,	-1,	-1,	0),
		ui_button_info("PIB_01",	617,	298,	-1,	-1,	1),
		ui_button_info("PIB_02",	172,	322,	-1,	-1,	2),
		ui_button_info("PIB_03",	219,	332,	217,	318,	3)
	},
	{ // GR_1024
		ui_button_info("2_PIB_00",	988,	410,	-1,	-1,	0),
		ui_button_info("2_PIB_01",	988,	477,	-1,	-1,	1),
		ui_button_info("2_PIB_02",	276,	516,	-1,	-1,	2),
		ui_button_info("2_PIB_03",	350,	532,	348,	510,	3)
	}
};

#define MULTI_PINFO_NUM_TEXT			1
UI_XSTR Multi_pinfo_text[GR_NUM_RESOLUTIONS][MULTI_PINFO_NUM_TEXT] = {
	{ // GR_640
		{ "Close",		428,	217,	318,	UI_XSTR_COLOR_PINK, -1,	&Multi_pinfo_buttons[0][MPI_EXIT].button },		
	},
	{ // GR_1024
		{ "Close",		428,	348,	510,	UI_XSTR_COLOR_PINK, -1,	&Multi_pinfo_buttons[1][MPI_EXIT].button },		
	}
};

//XSTR:ON

// stats labels
#define MULTI_PINFO_NUM_STATS_LABELS		9
#define MPI_RANK									0
#define MPI_MISSIONS_FLOWN						1
#define MPI_FLIGHT_TIME							2
#define MPI_LAST_FLOWN							3
#define MPI_FIGHTER_KILLS						4
// #define MPI_OTHER_KILLS							5
#define MPI_PSHOTS_FIRED						5
//#define MPI_PSHOTS_HIT							6
 #define MPI_PSHOTS_PCT							6
#define MPI_SSHOTS_FIRED						7
// #define MPI_SSHOTS_HIT							10
#define MPI_SSHOTS_PCT							8

char *Multi_pinfo_stats_labels[MULTI_PINFO_NUM_STATS_LABELS]; 

#define MAX_LABEL_TEXT		50
char Multi_pinfo_stats_vals[MULTI_PINFO_NUM_STATS_LABELS][MAX_LABEL_TEXT];
int Multi_pinfo_stats_label_offsets[MULTI_PINFO_NUM_STATS_LABELS] = {
	20,10,10,20,20,10,20,10,10,
};

// stats area defs
int Multi_pinfo_stats_area_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		215, 163, 414, 155
	},
	{ // GR_1024
		335, 261, 662, 248
	}
};
int Multi_pinfo_stats_x[GR_NUM_RESOLUTIONS] = {
	460,		// GR_640
	650		// GR_1024
};

// is the popup already running
int Multi_pinfo_popup_running = 0;

// background bitmap to be blitted
int Multi_pinfo_screen_save = -1;

// flag indicating if the popup has gotten messed up somewhere and should bail
int Multi_pinfo_popup_error = 0;

// flag indicating if the popup should be done
int Multi_pinfo_popup_done = 0;

// player this popup is being used for
net_player *Multi_pinfo_popup_player = NULL;

// screen shader
extern shader Grey_shader;

// hardware textures backup
int Multi_pinfo_hardware_texture_backup;


// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO FORWARD DECLARATIONS
//

// initialize all popup details (graphics, etc)
void multi_pinfo_popup_init(net_player *pl);

// run the popup in a tight loop (no states)
void multi_pinfo_popup_do();

// close the popup
void multi_pinfo_popup_close();

// blit the pilot image
void multi_pinfo_blit_pilot_image();

// blit the pilot squadron logo
void multi_pinfo_blit_squadron_logo();

// blit the player statistics
void multi_pinfo_blit_player_stats();

// check for button presses
void multi_pinfo_popup_check_buttons();

// act on a button press
void multi_pinfo_popup_button_pressed(int n);

// display the medals screen for this player
void multi_pinfo_do_medals();

// load up and use the proper palette
void multi_pinfo_set_palette();

// build the stats value strings for this player
void multi_pinfo_build_stats();

// if the pilot's image was currently loading when we started the popup, load it up now if its finished
void multi_pinfo_maybe_reload_pic(np_bitmap *b);

// reset the player infomation for this popup
void multi_pinfo_reset_player(net_player *np);

// lookup the "previous" player in the netplayer list, return null if not found
net_player *multi_pinfo_get_prev_player(net_player *np);

// lookup the "next" player in the netplayer list, return null if not found
net_player *multi_pinfo_get_next_player(net_player *np);


// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO FUNCTIONS
//

// fire up the player info popup, select first available pilot if np == NULL
void multi_pinfo_popup(net_player *np)
{
	// if the popup is already running, don't do anything
	if(Multi_pinfo_popup_running){
		return;
	}

	// set the player for informational purposes
	Assert(np != NULL);	

	// play the popup appear sound
	gamesnd_play_iface(SND_POPUP_APPEAR);

	// initialize the popup
	multi_pinfo_popup_init(np);

	// mark the popup as running
	Multi_pinfo_popup_running = 1;

	// run the popup
	multi_pinfo_popup_do();

	// close the popup
	multi_pinfo_popup_close();

	// play the popup disappear sound
	gamesnd_play_iface(SND_POPUP_DISAPPEAR);
}

// notify the popup that a player has left
void multi_pinfo_notify_drop(net_player *np)
{
	net_player *reset;

	// if we're no active, bail
	if(!Multi_pinfo_popup_running){
		return;
	}

	// if this is not the player we're currently displaying, bail
	if(np != Multi_pinfo_popup_player){
		return;
	}

	// otherwise we need to switch to someone else
	reset = multi_pinfo_get_prev_player(np);
	if(reset != NULL){
		multi_pinfo_reset_player(reset);
		return;
	}
	reset = multi_pinfo_get_next_player(np);
	if(reset != NULL){
		multi_pinfo_reset_player(reset);
		return;
	}

	// bail, since there's no one else
	Int3();
	Multi_pinfo_popup_done = 1;
}


// ---------------------------------------------------------------------------------------
// MULTI PLAYER INFO FORWARD DEFINITIONS
//

// initialize all popup details (graphics, etc)
void multi_pinfo_popup_init(net_player *np)
{
	int idx;
	
	// no errors to start with
	Multi_pinfo_popup_error = 0;

	// shouldn't be done
	Multi_pinfo_popup_done = 0;

	// store the background as it currently is
	Multi_pinfo_screen_save = gr_save_screen();
	if(Multi_pinfo_screen_save == -1){
		Multi_pinfo_popup_error = 1;
		return;
	}

	// create the interface window
	Multi_pinfo_window.create(0,0,gr_screen.max_w,gr_screen.max_h,0);
	Multi_pinfo_window.set_mask_bmap(Multi_pinfo_bitmap_mask[gr_screen.res]);

	// load the background bitmap
	Multi_pinfo_bitmap = bm_load(Multi_pinfo_bitmap_name[gr_screen.res]);
	if(Multi_pinfo_bitmap < 0){
		Multi_pinfo_popup_error = 1;
		return;	
	}

	// backup hardware textures setting and bash to max
	Multi_pinfo_hardware_texture_backup = Detail.hardware_textures;
	Detail.hardware_textures = MAX_DETAIL_LEVEL;

	// zero bitmap info
	Mp_pilot.bitmap = -1;
	strcpy(Mp_pilot.filename, "");
	Mp_squad.bitmap = -1;
	strcpy(Mp_squad.filename, "");

	// set the player status
	multi_pinfo_reset_player(np);	
	
	// create the interface buttons
	for(idx=0;idx<MULTI_PINFO_NUM_BUTTONS;idx++){
		// create the object
		Multi_pinfo_buttons[gr_screen.res][idx].button.create(&Multi_pinfo_window, "", Multi_pinfo_buttons[gr_screen.res][idx].x, Multi_pinfo_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Multi_pinfo_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Multi_pinfo_buttons[gr_screen.res][idx].button.set_bmaps(Multi_pinfo_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Multi_pinfo_buttons[gr_screen.res][idx].button.link_hotspot(Multi_pinfo_buttons[gr_screen.res][idx].hotspot);
	}			

	// add xstrs
	for(idx=0; idx<MULTI_PINFO_NUM_TEXT; idx++){
		Multi_pinfo_window.add_XSTR(&Multi_pinfo_text[gr_screen.res][idx]);
	}

	// disable medals button for the demo
#ifdef FS2_DEMO
	Multi_pinfo_buttons[gr_screen.res][MPI_MEDALS].button.hide();
	Multi_pinfo_buttons[gr_screen.res][MPI_MEDALS].button.disable();
#endif

	// initialize strings	
	Multi_pinfo_stats_labels[0] = strdup(XSTR("Rank", 1007));
	Multi_pinfo_stats_labels[1] = strdup(XSTR("Missions Flown", 1008));
	Multi_pinfo_stats_labels[2] = strdup(XSTR("Flight Time", 1009));
	Multi_pinfo_stats_labels[3] = strdup(XSTR("Last Flown",1010));
	Multi_pinfo_stats_labels[4] = strdup(XSTR("Total Kills", 115));
	Multi_pinfo_stats_labels[5] = strdup(XSTR("Primary Shots Fired", 1012));
	Multi_pinfo_stats_labels[6] = strdup(XSTR("Primary Hit %", 1013));
	Multi_pinfo_stats_labels[7] = strdup(XSTR("Secondary Shots Fired",	1014));
	Multi_pinfo_stats_labels[8] = strdup(XSTR("Secondary Hit %", 1015));				
}

// run the popup in a tight loop (no states)
void multi_pinfo_popup_do()
{
	int k;
	
	// if there was an error in initialization, return immediately
	if(Multi_pinfo_popup_error){
		return;
	}

	// tight loop
	while(!Multi_pinfo_popup_done){		
		multi_pinfo_maybe_reload_pic(&Mp_pilot);		
		multi_pinfo_maybe_reload_pic(&Mp_squad);		

		// process the window
		k = Multi_pinfo_window.process();
		switch(k){
		case KEY_ESC :
			Multi_pinfo_popup_done = 1;
			break;
		}

		// check button presses
		multi_pinfo_popup_check_buttons();

		// set frametime and run background stuff
		game_set_frametime(-1);
		game_do_state_common(gameseq_get_state());
		
		// draw the background bitmap and the ui window over it
		Assert(Multi_pinfo_screen_save != -1);
		gr_reset_clip();
		gr_restore_screen(Multi_pinfo_screen_save);		

		// grey the screen
		gr_set_shader(&Grey_shader);
		gr_shade(0,0,gr_screen.clip_width, gr_screen.clip_height);
		
		// draw the background bitmap
		gr_set_bitmap(Multi_pinfo_bitmap);
		gr_bitmap(0,0);		

		// blit the selected pilot image
		multi_pinfo_blit_pilot_image();

		// blit the squadron logo
		multi_pinfo_blit_squadron_logo();

		// blit the player statistics
		multi_pinfo_blit_player_stats();		

		// draw the ui window and flip
		Multi_pinfo_window.draw();		
		gr_flip();
	}
}

// close the popup
void multi_pinfo_popup_close()
{
	int idx;
	
	// unload any bitmaps
	if(Multi_pinfo_bitmap != -1){
		bm_release(Multi_pinfo_bitmap);		
	}	

	// free the background screen if possible
	if(Multi_pinfo_screen_save >= 0){
		gr_free_screen(Multi_pinfo_screen_save);	
	}

	// release the pilot/squad images
	if(Mp_pilot.bitmap != -1){
		bm_release(Mp_pilot.bitmap);
	}
	if(Mp_squad.bitmap != -1){
		bm_release(Mp_squad.bitmap);
	}

	// free up strings
	for(idx=0; idx<MULTI_PINFO_NUM_STATS_LABELS; idx++){
		if(Multi_pinfo_stats_labels[idx] != NULL){
			free(Multi_pinfo_stats_labels[idx]);
			Multi_pinfo_stats_labels[idx] = NULL;
		}
	}	

	// unset the player handle
	Multi_pinfo_popup_player = NULL;

	// mark the popup as not running
	Multi_pinfo_popup_running = 0;
	
	// destroy the UI_WINDOW
	Multi_pinfo_window.destroy();

	// restore hardware textures detail level
	Detail.hardware_textures = Multi_pinfo_hardware_texture_backup;
}

// blit the pilot image
void multi_pinfo_blit_pilot_image()
{
	char place_text[100];	
	int w;

	// if we don't have a bitmap handle, blit a placeholder
	if(Mp_pilot.bitmap == -1){
		gr_set_color_fast(&Color_normal);		

		// if there is no image
		if(strlen(Mp_pilot.filename) <= 0){
			strcpy(place_text,XSTR("No/Invalid Image", 1053));
		} 
		// if the image is xferring
		else if(multi_xfer_lookup(Mp_pilot.filename)){
			strcpy(place_text,XSTR("Image Transferring", 691));
		}
		// if we're not accepting images
		else if(!(Net_player->p_info.options.flags & MLO_FLAG_ACCEPT_PIX) || !(Netgame.options.flags & MSO_FLAG_ACCEPT_PIX)){
			strcpy(place_text,XSTR("No Image", 692));
		}
		// otherwise we wait
		else {
			strcpy(place_text,XSTR("Waiting", 690));
		}		

		// center the text
		gr_get_string_size(&w,NULL,place_text);
		gr_string(Multi_pinfo_pilot_coords[gr_screen.res][0] + ((Multi_pinfo_pilot_coords[gr_screen.res][2] - w)/2), Multi_pinfo_pilot_coords[gr_screen.res][1], place_text);
	} 
	// otherwise blit the bitmap
	else {
		gr_set_bitmap(Mp_pilot.bitmap);

		// get width and heigh
		int w, h;
		bm_get_info(Mp_pilot.bitmap, &w, &h, NULL, NULL, NULL, NULL);

		gr_bitmap(Multi_pinfo_pilot_coords[gr_screen.res][0] + ((Multi_pinfo_pilot_coords[gr_screen.res][2] - w)/2), 
					 Multi_pinfo_pilot_coords[gr_screen.res][1] + ((Multi_pinfo_pilot_coords[gr_screen.res][3] - h)/2));
		// g3_draw_2d_poly_bitmap(Multi_pinfo_pilot_coords[gr_screen.res][0], Multi_pinfo_pilot_coords[gr_screen.res][1], Multi_pinfo_pilot_coords[gr_screen.res][2], Multi_pinfo_pilot_coords[gr_screen.res][3]);
	}
}

// blit the pilot squadron logo
void multi_pinfo_blit_squadron_logo()
{
	char place_text[100];	
	int w;
	player *p = Multi_pinfo_popup_player->player;

	// if we don't have a bitmap handle, blit a placeholder
	if(Mp_squad.bitmap == -1){
		gr_set_color_fast(&Color_normal);		

		// if there is no image
		if(strlen(p->squad_filename) <= 0){
			strcpy(place_text,XSTR("No/Invalid Image", 1053));
		} 
		// if the image is xferring
		else if(multi_xfer_lookup(p->squad_filename)){
			strcpy(place_text,XSTR("Image Transferring", 691));
		}
		// if we're not accepting images
		else if(!(Net_player->p_info.options.flags & MLO_FLAG_ACCEPT_PIX) || !(Netgame.options.flags & MSO_FLAG_ACCEPT_PIX)){
			strcpy(place_text,XSTR("No Image", 692));
		}
		// otherwise we wait
		else {
			strcpy(place_text,XSTR("Waiting", 690));
		}				

		// center the text
		gr_get_string_size(&w, NULL, place_text);
		gr_string(Multi_pinfo_squad_coords[gr_screen.res][0] + ((Multi_pinfo_squad_coords[gr_screen.res][2] - w)/2), Multi_pinfo_squad_coords[gr_screen.res][1], place_text);
	} 
	// otherwise blit the bitmap
	else {
		gr_set_bitmap(Mp_squad.bitmap);
		// gr_bitmap(MPI_SQUAD_X, MPI_SQUAD_Y);

		// get width and heigh
		int w, h;
		bm_get_info(Mp_squad.bitmap, &w, &h, NULL, NULL, NULL, NULL);

		gr_bitmap(Multi_pinfo_squad_coords[gr_screen.res][0] + ((Multi_pinfo_squad_coords[gr_screen.res][2] - w)/2), 
					 Multi_pinfo_squad_coords[gr_screen.res][1] + ((Multi_pinfo_squad_coords[gr_screen.res][3] - h)/2));
		// g3_draw_2d_poly_bitmap(Multi_pinfo_squad_coords[gr_screen.res][0], Multi_pinfo_squad_coords[gr_screen.res][1], Multi_pinfo_squad_coords[gr_screen.res][2], Multi_pinfo_squad_coords[gr_screen.res][3]);
	}
}

// blit the player statistics
void multi_pinfo_blit_player_stats()
{
	int idx,y_start;	

	// blit the player's callsign and "all time stats"
	gr_set_color_fast(&Color_bright);
	gr_string(Multi_pinfo_stats_area_coords[gr_screen.res][0], Multi_pinfo_stats_area_coords[gr_screen.res][1], Multi_pinfo_popup_player->player->callsign);
	gr_string(Multi_pinfo_stats_x[gr_screen.res], Multi_pinfo_stats_area_coords[gr_screen.res][1], XSTR("All Time Stats", 128));
	
	gr_set_color_fast(&Color_normal);

	// blit all the labels
	y_start = Multi_pinfo_stats_area_coords[gr_screen.res][1] + 15;
	for(idx=0;idx<MULTI_PINFO_NUM_STATS_LABELS;idx++){
		gr_string(Multi_pinfo_stats_area_coords[gr_screen.res][0], y_start, Multi_pinfo_stats_labels[idx]);
		y_start += Multi_pinfo_stats_label_offsets[idx];
	}	

	// blit all the stats values themselves
	y_start = Multi_pinfo_stats_area_coords[gr_screen.res][1] + 15;
	for(idx=0;idx<MULTI_PINFO_NUM_STATS_LABELS;idx++){
		gr_string(Multi_pinfo_stats_x[gr_screen.res], y_start, Multi_pinfo_stats_vals[idx]);
		y_start += Multi_pinfo_stats_label_offsets[idx];
	}	
}

// check for button presses
void multi_pinfo_popup_check_buttons()
{
	int idx;

	// check for all buttons
	for(idx=0;idx<MULTI_PINFO_NUM_BUTTONS;idx++){
		if(Multi_pinfo_buttons[gr_screen.res][idx].button.pressed()){
			multi_pinfo_popup_button_pressed(idx);
			break;
		}
	}
}

// act on a button press
void multi_pinfo_popup_button_pressed(int n)
{
	net_player *swap;

	switch(n){
	case MPI_EXIT:
		Multi_pinfo_popup_done = 1;		
		break;

	case MPI_MEDALS:
		gamesnd_play_iface(SND_USER_SELECT);
		multi_pinfo_do_medals();
		break;

	case MPI_SCROLL_STATS_UP:
		swap = multi_pinfo_get_prev_player(Multi_pinfo_popup_player);
		if(swap != NULL){
			gamesnd_play_iface(SND_USER_SELECT);
			multi_pinfo_reset_player(swap);
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	case MPI_SCROLL_STATS_DOWN:
		swap = multi_pinfo_get_next_player(Multi_pinfo_popup_player);
		if(swap != NULL){
			gamesnd_play_iface(SND_USER_SELECT);
			multi_pinfo_reset_player(swap);
		} else {
			gamesnd_play_iface(SND_GENERAL_FAIL);
		}
		break;

	default :
		gamesnd_play_iface(SND_GENERAL_FAIL);
		break;
	}
}

// display the medals screen for this player
void multi_pinfo_do_medals()
{
#ifdef FS2DEMO
	game_feature_not_in_demo_popup();
#else
	int ret_code;

	// initialize the medals screen
	medal_main_init(Multi_pinfo_popup_player->player,MM_POPUP);

	// run the medals screen until it says that it should be closed
	do {
		// set frametime and run common functions
		game_set_frametime(-1);
		game_do_state_common(gameseq_get_state());

		// run the medals screen
		ret_code = medal_main_do();		
	} while(ret_code && !Multi_pinfo_popup_done);

	// close the medals screen down
	medal_main_close();

	// restore the proper palette
	multi_pinfo_set_palette();
#endif
}

// load up and use the proper palette
void multi_pinfo_set_palette()
{
#ifndef HARDWARE_ONLY
	palette_use_bm_palette(Multi_pinfo_bitmap);
#endif
}

// build the stats value strings for this player
void multi_pinfo_build_stats()
{
	// int idx;
	// int fighter_kills,other_kills;
	scoring_struct *sc = &Multi_pinfo_popup_player->player->stats;

	// build alltime fighter and non-fighter kills
	/*
	fighter_kills = 0;
	other_kills = 0;
	for(idx=0;idx<MAX_SHIP_TYPES;idx++){
		if(sc->kills[idx] > 0){
			if(Ship_info[idx].flags & SIF_FIGHTER){
				fighter_kills += sc->kills[idx];
			} else {
				other_kills += sc->kills[idx];
			}
		}
	}	
	*/
	sprintf(Multi_pinfo_stats_vals[MPI_FIGHTER_KILLS], "%d", sc->kill_count);
	
	// sprintf(Multi_pinfo_stats_vals[MPI_OTHER_KILLS],"%d",other_kills);

	// missions flown
	sprintf(Multi_pinfo_stats_vals[MPI_MISSIONS_FLOWN],"%d",(int)sc->missions_flown);

	// flight time		
	game_format_time(fl2f((float)sc->flight_time),Multi_pinfo_stats_vals[MPI_FLIGHT_TIME]);		

	// last flown	
	if(sc->last_flown == 0){
		strcpy(Multi_pinfo_stats_vals[MPI_LAST_FLOWN],XSTR("No missions flown",693));
	} else {
		tm *tmr = gmtime(&sc->last_flown);
		if(tmr != NULL){
			strftime(Multi_pinfo_stats_vals[MPI_LAST_FLOWN],MAX_LABEL_TEXT,"%m/%d/%y %H:%M",tmr);
		} else {
			strcpy(Multi_pinfo_stats_vals[MPI_LAST_FLOWN], "");			
		}
	}	

	// rank
	strcpy(Multi_pinfo_stats_vals[MPI_RANK],Ranks[sc->rank].name);

	// primary shots fired
	sprintf(Multi_pinfo_stats_vals[MPI_PSHOTS_FIRED],"%d",sc->p_shots_fired);

	// primary shots hit
	// sprintf(Multi_pinfo_stats_vals[MPI_PSHOTS_HIT],"%d",sc->p_shots_hit);
	
	// primary hit pct
	sprintf(Multi_pinfo_stats_vals[MPI_PSHOTS_PCT],"%d%%",(int)(100.0f * ((float)sc->p_shots_hit / (float)sc->p_shots_fired)));
	// primary shots fired
	sprintf(Multi_pinfo_stats_vals[MPI_SSHOTS_FIRED],"%d",sc->s_shots_fired);

	// primary shots hit
	// sprintf(Multi_pinfo_stats_vals[MPI_SSHOTS_HIT],"%d",sc->s_shots_hit);
	
	// primary hit pct
	sprintf(Multi_pinfo_stats_vals[MPI_SSHOTS_PCT],"%d%%",(int)(100.0f * ((float)sc->s_shots_hit / (float)sc->s_shots_fired)));
}

// if the pilot's image was currently loading when we started the popup, load it up now if its finished
void multi_pinfo_maybe_reload_pic(np_bitmap *b)
{	
	// if the bitmap is valid, do nothing
	if(b->bitmap >= 0){
		return;
	}	

	// if the local player is not accepting pix or the netgame is not accepting pix, bail here
	if(!(Net_player->p_info.options.flags & MLO_FLAG_ACCEPT_PIX) || !(Netgame.options.flags & MSO_FLAG_ACCEPT_PIX)){		
		return;
	}			

	// if the bitmap filename is bogus
	if(strlen(b->filename) <= 0){
		return;
	}	

	// try again
	b->bitmap = bm_load_duplicate(b->filename);	
}

// attempt to validate a bitmap (ie, return whether its displayable or not)
/*
int multi_pinfo_validate_bitmap(int bitmap)
{
	int w,h;
	
	// if the bitmap handle is invalid false
	if(bitmap == -1){
		return 0;
	}
	
	// get the bitmap info
	w = -1;
	h = -1;
	bm_get_info(bitmap,&w,&h);	

	// return fail
	if((w != MPI_IMAGE_W) || (h != MPI_IMAGE_H)){
		return 0;
	}

	// return success
	return 1;
}
*/

// is the pilot info popup currently active?
int multi_pinfo_popup_active()
{
	return Multi_pinfo_popup_running;
}

// kill the currently active popup (if any)
void multi_pinfo_popup_kill()
{
	// we're done, byatch
	Multi_pinfo_popup_done = 1;
}

// reset the player infomation for this popup
void multi_pinfo_reset_player(net_player *np)
{	
	// assign the player
	Multi_pinfo_popup_player = np;

	// unload any old image data if necessary
	strcpy(Mp_pilot.filename, "");
	if(Mp_pilot.bitmap != -1){
		bm_release(Mp_pilot.bitmap);
		Mp_pilot.bitmap = -1;
	}
	strcpy(Mp_squad.filename, "");
	if(Mp_squad.bitmap != -1){
		bm_release(Mp_squad.bitmap);
		Mp_squad.bitmap = -1;
	}	
	
	// try and load pilot pic/squad logo
	if(strlen(np->player->image_filename) >= 0){
		strcpy(Mp_pilot.filename, np->player->image_filename);
		Mp_pilot.bitmap = bm_load_duplicate(Mp_pilot.filename);
	}
	if(strlen(np->player->squad_filename) >= 0){
		strcpy(Mp_squad.filename, np->player->squad_filename);
		Mp_squad.bitmap = bm_load_duplicate(Mp_squad.filename);
	}

	// build the stats value strings for this player
	multi_pinfo_build_stats();
}

// lookup the "previous" player in the netplayer list, return null if not found
net_player *multi_pinfo_get_prev_player(net_player *np)
{
	int start_index;
	int idx;

	// get the starting index to look from
	start_index = NET_PLAYER_INDEX(np);
	if(start_index > 0){		
		// look backwards
		for(idx=start_index-1; idx>=0; idx--){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
				return &Net_players[idx];
			}
		}
	}
	
	return NULL;
}

// lookup the "next" player in the netplayer list, return null if not found
net_player *multi_pinfo_get_next_player(net_player *np)
{
	int start_index;
	int idx;

	// get the starting index to look from
	start_index = NET_PLAYER_INDEX(np);	
	if(start_index < (MAX_PLAYERS - 1)){		
		// look forwards
		for(idx=start_index+1; idx<MAX_PLAYERS; idx++){
			if(MULTI_CONNECTED(Net_players[idx]) && !MULTI_STANDALONE(Net_players[idx])){
				return &Net_players[idx];
			}
		}
	}
	
	return NULL;
}
