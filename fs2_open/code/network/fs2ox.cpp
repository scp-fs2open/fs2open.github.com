/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
 */ 

/*
 * $Logfile$
 * $Revision: 1.3 $
 * $Date: 2004-03-05 09:02:02 $
 * $Author: Goober5000 $
 *
 * C file for implementing PXO-substitute (FS2OX -- "fs2_open exchange") screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 1.1  2002/07/29 22:24:26  penguin
 * First attempt at "fs2_open exchange" (PXO substitute) screen
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "ui/ui.h"
#include "bmpman/bmpman.h"
#include "gamesnd/gamesnd.h"
#include "io/key.h"
#include "gamesequence/gamesequence.h"
#include "network/fs2ox.h"

// LOCAL function definitions
void fs2ox_check_buttons();
void fs2ox_button_pressed(int n);

//XSTR:OFF
// bitmaps defs
static char *fs2ox_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"PXOChat",		// GR_640
	"2_PXOChat"		// GR_1024
};

static char *fs2ox_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"PXOChat-M",		// GR_640
	"2_PXOChat-M"		// GR_1024
};
//XSTR:ON

UI_WINDOW fs2ox_window;		// the window object for the FS2OX screen
int fs2ox_bitmap;													// the background bitmap
int fs2ox_frame_count;						// keep a count of frames displayed


// button defs
#define FS2OX_NUM_BUTTONS 15

#define FS2OX_USERS_SCROLL_UP			0
#define FS2OX_USERS_SCROLL_DOWN		1
#define FS2OX_WEB_RANKING				2
#define FS2OX_PILOT_INFO				3
#define FS2OX_PILOT_FIND				4
#define FS2OX_MOTD						5
#define FS2OX_CHANNELS_XXX1			6	// ?
#define FS2OX_CHANNELS_XXX2			7	// ?
#define FS2OX_CHANNELS_SCROLL_UP		8
#define FS2OX_CHANNELS_SCROLL_DOWN	9
#define FS2OX_CHATBOX_SCROLL_UP		10
#define FS2OX_CHATBOX_SCROLL_DOWN	11
#define FS2OX_EXIT						12
#define FS2OX_HELP						13
#define FS2OX_ACCEPT						14

//XSTR:OFF
ui_button_info fs2ox_buttons[GR_NUM_RESOLUTIONS][FS2OX_NUM_BUTTONS] = {
	{ // GR_640
		//					 filename   x     y      xt    yt   hotspot
		ui_button_info( "pxb_00",	1,		103,	-1,	-1,	0 ),						// users scroll up
		ui_button_info( "pxb_01",	1,		334,	-1,	-1,	1 ),						// users scroll down
		ui_button_info( "pxb_02",	17,	386,	-1,	-1,	2 ),						// web ranking
		ui_button_info( "pxb_03",	71,	385,	-1,	-1,	3 ),						// pilot info
		ui_button_info( "pxb_04",	115,	385,	-1,	-1,	4 ),						// find pilot
		ui_button_info( "pxb_05",	1,		443,	-1,	-1,	5 ),						// show_motd
		ui_button_info( "pxb_06",	330,	96,	-1,	-1,	6 ),						// channels xxx1
		ui_button_info( "pxb_07",	330,	131,	-1,	-1,	7 ),						// channels xxx2
		ui_button_info( "pxb_08",	618,	92,	-1,	-1,	8 ),						// channels scroll up
		ui_button_info( "pxb_09",	618,	128,	-1,	-1,	9 ),						// channels scroll down
		ui_button_info( "pxb_10",	615,	171,	-1,	-1,	10 ),						// chatbox scroll up
		ui_button_info( "pxb_11",	615,	355,	-1,	-1,	11 ),						// chatbox scroll down
		ui_button_info( "pxb_12",	481,	435,	-1,	-1,	12 ),						// exit
		ui_button_info( "pxb_13",	533,	433,	-1,	-1,	13 ),						// help
		ui_button_info( "pxb_14",	573,	432,	-1,	-1,	14 ),						// accept
	},
	{ // GR_1024
		// NOTE! these positions are WRONG for 1024... !FIXME!
		ui_button_info( "2_pxb_00",	1,		103,	-1,	-1,	0 ),						// users scroll up
		ui_button_info( "2_pxb_01",	1,		334,	-1,	-1,	1 ),						// users scroll down
		ui_button_info( "2_pxb_02",	17,	386,	-1,	-1,	2 ),						// web ranking
		ui_button_info( "2_pxb_03",	71,	385,	-1,	-1,	3 ),						// pilot info
		ui_button_info( "2_pxb_04",	115,	385,	-1,	-1,	4 ),						// find pilot
		ui_button_info( "2_pxb_05",	1,		443,	-1,	-1,	5 ),						// show_motd
		ui_button_info( "2_pxb_06",	330,	96,	-1,	-1,	6 ),						// channels xxx1
		ui_button_info( "2_pxb_07",	330,	131,	-1,	-1,	7 ),						// channels xxx2
		ui_button_info( "2_pxb_08",	618,	92,	-1,	-1,	8 ),						// channels scroll up
		ui_button_info( "2_pxb_09",	618,	128,	-1,	-1,	9 ),						// channels scroll down
		ui_button_info( "2_pxb_10",	615,	171,	-1,	-1,	10 ),						// chatbox scroll up
		ui_button_info( "2_pxb_11",	615,	355,	-1,	-1,	11 ),						// chatbox scroll down
		ui_button_info( "2_pxb_12",	481,	435,	-1,	-1,	12 ),						// exit
		ui_button_info( "2_pxb_13",	533,	433,	-1,	-1,	13 ),						// help
		ui_button_info( "2_pxb_14",	573,	432,	-1,	-1,	14 ),						// accept
	}
};
//XSTR:ON


// text def
#define FS2OX_NUM_TEXT			9

UI_XSTR fs2ox_text[GR_NUM_RESOLUTIONS][FS2OX_NUM_TEXT] = {
	{ // GR_640		
		// string					xstr	x		y		color						font UI_GADGET
		{"Web",						1313, 18,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_WEB_RANKING].button},
		{"Ranking",					1314, 5,		425,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_WEB_RANKING].button},
		{"Pilot",					1310,	71,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_PILOT_INFO].button},
		{"Info",						1311,	74,	425,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_PILOT_INFO].button},
		{"Find",						1315,	120,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_PILOT_FIND].button},
		{"Motd",						1316,	34,	455,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_MOTD].button},
		{"Exit",						1416,	495,	420,	UI_XSTR_COLOR_PINK,  -1, &fs2ox_buttons[0][FS2OX_EXIT].button},	
		{"Help",						928,	540,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_HELP].button},
		{"Games",					1319,	590,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[0][FS2OX_ACCEPT].button},
	},
	{ // GR_1024		
		// NOTE! these positions are WRONG for 1024... !FIXME!
		{"Web",						1313, 18,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_WEB_RANKING].button},
		{"Ranking",					1314, 5,		425,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_WEB_RANKING].button},
		{"Pilot",					1310,	71,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_PILOT_INFO].button},
		{"Info",						1311,	74,	425,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_PILOT_INFO].button},
		{"Find",						1315,	120,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_PILOT_FIND].button},
		{"Motd",						1316,	34,	455,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_MOTD].button},
		{"Exit",						1416,	495,	420,	UI_XSTR_COLOR_PINK,  -1, &fs2ox_buttons[1][FS2OX_EXIT].button},	
		{"Help",						928,	540,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_HELP].button},
		{"Games",					1319,	590,	415,	UI_XSTR_COLOR_GREEN, -1, &fs2ox_buttons[1][FS2OX_ACCEPT].button},
	}
};



/**
 * Initialize the FS2OX screen.
 */
void fs2ox_init()
{
	int idx;

	// TODO - init local variable

	// reset frame count
	fs2ox_frame_count = 0;

	// create the interface window
	fs2ox_window.create(0,0,gr_screen.max_w,gr_screen.max_h,0);
	fs2ox_window.set_mask_bmap(fs2ox_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	fs2ox_bitmap = bm_load(fs2ox_bitmap_fname[gr_screen.res]);
	if(fs2ox_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}


	// create the interface buttons
	for(idx=0; idx < FS2OX_NUM_BUTTONS; idx++){
		// create the object
		fs2ox_buttons[gr_screen.res][idx].button.create(&fs2ox_window,"", fs2ox_buttons[gr_screen.res][idx].x, fs2ox_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		fs2ox_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		fs2ox_buttons[gr_screen.res][idx].button.set_bmaps(fs2ox_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		fs2ox_buttons[gr_screen.res][idx].button.link_hotspot(fs2ox_buttons[gr_screen.res][idx].hotspot);
	}		

	// create all xstrs
	for(idx=0; idx < FS2OX_NUM_TEXT; idx++){
		fs2ox_window.add_XSTR(&fs2ox_text[gr_screen.res][idx]);
	}


	// TODO - load the help overlay
//	help_overlay_load(FS2OX_OVERLAY);
//	help_overlay_set_state(FS2OX_OVERLAY,0);

}


/**
 * Close the FS2OX screen.
 */
void fs2ox_close()
{
	// unload any bitmaps
	if(!bm_unload(fs2ox_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n",fs2ox_bitmap_fname[gr_screen.res]));
	}

	// unload the help overlay
//	help_overlay_unload(FS2OX_OVERLAY);	
	
	// destroy the UI_WINDOW
	fs2ox_window.destroy();
}


/**
 * Frame processing for FS2OX screen.
 */
void fs2ox_do_frame()
{

	int k = fs2ox_window.process();

	// process any keypresses
	switch(k){
	case KEY_ESC :
//  		if(help_overlay_active(FS2OX_OVERLAY)){
//  			help_overlay_set_state(FS2OX_OVERLAY,0);
//  		} else {		
			gameseq_post_event(GS_EVENT_MAIN_MENU);			
			gamesnd_play_iface(SND_USER_SELECT);
//  		}
		break;
	}

	// process any button clicks
	fs2ox_check_buttons();

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(fs2ox_bitmap);
	if(fs2ox_bitmap != -1){		
		gr_set_bitmap(fs2ox_bitmap);
		gr_bitmap(0,0);
	}
	fs2ox_window.draw();

	// flip the buffer
	gr_flip();

	// increment the frame count
	fs2ox_frame_count++;
}



void fs2ox_check_buttons()
{
	int idx;
	for(idx=0;idx<FS2OX_NUM_BUTTONS;idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(fs2ox_buttons[gr_screen.res][idx].button.pressed()){
			fs2ox_button_pressed(idx);
			break;
		}
	}
}



void fs2ox_button_pressed(int n)
{
	switch(n){
	// help overlay
//  	case FS2OX_HELP:
//  		if(!help_overlay_active(FS2OX_OVERLAY)){
//  			help_overlay_set_state(FS2OX_OVERLAY,1);
//  		} else {
//  			help_overlay_set_state(FS2OX_OVERLAY,0);
//  		}
//  		break;

	// go to the games list
	case FS2OX_ACCEPT:
		gameseq_post_event(GS_EVENT_MULTI_JOIN_GAME);
		break;

	default :
//  		multi_common_add_notify(XSTR("Not implemented yet!",760));
		gamesnd_play_iface(SND_GENERAL_FAIL);
		break;
	}
}
