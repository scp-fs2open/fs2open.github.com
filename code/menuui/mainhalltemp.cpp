/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "menuui/mainhallmenu.h"
#include "menuui/mainhalltemp.h"
#include "ui/ui.h"
#include "io/key.h"
#include "gamesnd/gamesnd.h"
#include "gamesequence/gamesequence.h"
#include "mission/missioncampaign.h"
#include "mission/missionload.h"
#include "playerman/player.h"
#include "freespace2/freespace.h"


// ------------------------------------------------------------------------------------------------------------------------
// TEMP MAIN HALL DEFINES/VARS
//

#define MHT_NUM_BUTTONS	6

char *Mht_bitmap_fname[GR_NUM_RESOLUTIONS] = {
	"mht_background",				// GR_640
	"2_mht_background"			// GR_1024
};

char *Mht_bitmap_mask_fname[GR_NUM_RESOLUTIONS] = {
	"mht_mask",			// GR_640
	"2_mht_mask"		// GR_1024
};

// button defs
#define MHT_READY_ROOM				0
#define MHT_CAMPAIGN_ROOM			1
#define MHT_OPTIONS					2
#define MHT_TECH_ROOM				3
#define MHT_BARRACKS					4
#define MHT_EXIT						5

UI_WINDOW Mht_window;													// the window object for the join screen
int Mht_bitmap;															// the background bitmap

ui_button_info Mht_buttons[GR_NUM_RESOLUTIONS][MHT_NUM_BUTTONS] = {
	{ // GR_640
		ui_button_info( "MHT_00",	15,	194,	-1,	-1,	2 ),						// ready room
		ui_button_info( "MHT_01",	15,	222,	-1,	-1,	5 ),						// campaigns		
		ui_button_info( "MHT_02",	14,	251,	-1,	-1,	4 ),						// options
		ui_button_info( "MHT_03",	14,	280,	-1,	-1,	3 ),						// tech room
		ui_button_info( "MHT_04",	14,	309,	-1,	-1,	1 ),						// barracks
		ui_button_info( "MHT_05",	16,	339,	-1,	-1,	0 ),						// exit
	},		
	{ // GR_1024
		ui_button_info( "2_MHT_00",	25,	312,	-1,	-1,	2 ),						// ready room
		ui_button_info( "2_MHT_01",	25,	357,	-1,	-1,	5 ),						// campaigns		
		ui_button_info( "2_MHT_02",	24,	403,	-1,	-1,	4 ),						// options
		ui_button_info( "2_MHT_03",	24,	450,	-1,	-1,	3 ),						// tech room
		ui_button_info( "2_MHT_04",	25,	497,	-1,	-1,	1 ),						// barracks
		ui_button_info( "2_MHT_05",	27,	544,	-1,	-1,	0 ),						// exit
	}
};


// ------------------------------------------------------------------------------------------------------------------------
// TEMP MAIN HALL FUNCTIONS
//

void mht_check_buttons();
void mht_button_pressed(int n);
void mht_exit_game();

void mht_init()
{	
	int idx;

	// create the interface window
	Mht_window.create(0, 0, gr_screen.max_w_unscaled,gr_screen.max_h_unscaled, 0);
	Mht_window.set_mask_bmap(Mht_bitmap_mask_fname[gr_screen.res]);

	// load the background bitmap
	Mht_bitmap = bm_load(Mht_bitmap_fname[gr_screen.res]);
	if(Mht_bitmap < 0){
		// we failed to load the bitmap - this is very bad
		Int3();
	}
	
	// create the interface buttons
	for(idx=0; idx<MHT_NUM_BUTTONS; idx++){
		// create the object
		Mht_buttons[gr_screen.res][idx].button.create(&Mht_window, "", Mht_buttons[gr_screen.res][idx].x, Mht_buttons[gr_screen.res][idx].y, 1, 1, 0, 1);

		// set the sound to play when highlighted
		Mht_buttons[gr_screen.res][idx].button.set_highlight_action(common_play_highlight_sound);

		// set the ani for the button
		Mht_buttons[gr_screen.res][idx].button.set_bmaps(Mht_buttons[gr_screen.res][idx].filename);

		// set the hotspot
		Mht_buttons[gr_screen.res][idx].button.link_hotspot(Mht_buttons[gr_screen.res][idx].hotspot);
	}			

	// remove any multiplayer flags from the game mode
	Game_mode &= ~(GM_MULTIPLAYER);

	// initialize the music
	main_hall_start_music();

	// set the game_mode based on the type of player
	Assert( Player != NULL );
	if ( Player->flags & PLAYER_FLAGS_IS_MULTI ){
		Game_mode = GM_MULTIPLAYER;
	} else {
		Game_mode = GM_NORMAL;
	}
}

void mht_do()
{	
	int k = Mht_window.process();

	// need to ensure ambient is playing, since it may be stopped by a playing movie
	main_hall_start_ambient();

	// process any keypresses
	switch(k){
	case KEY_ESC :		
		mht_exit_game();
		break;

	case KEY_B:
		gameseq_post_event( GS_EVENT_BARRACKS_MENU );
		break;	

	case KEY_G:
		if(Player->flags & PLAYER_FLAGS_IS_MULTI){
			break;
		}

		if (Num_recent_missions > 0)	{
			strcpy_s( Game_current_mission_filename, Recent_missions[0] );
		} else {
			mission_load_up_campaign();
			strcpy_s( Game_current_mission_filename, Campaign.missions[0].name );
		}

		Campaign.current_mission = -1;
		gameseq_post_event(GS_EVENT_START_GAME_QUICK);
		break;

	case KEY_L:
		gameseq_post_event( GS_EVENT_LOAD_MISSION_MENU );
		break;

	case KEY_F2:
		gameseq_post_event(GS_EVENT_OPTIONS_MENU);
		break;

	case KEY_M:
		if (Player->flags & PLAYER_FLAGS_IS_MULTI){
			main_hall_do_multi_ready();
		}
		break;
	}	

	// process button presses
	mht_check_buttons();

	// draw the background, etc
	gr_reset_clip();
	GR_MAYBE_CLEAR_RES(Mht_bitmap);
	if(Mht_bitmap != -1){		
		gr_set_bitmap(Mht_bitmap);
		gr_bitmap(0,0);
	}
	Mht_window.draw();	
	
	// flip the buffer
	gr_flip();	
}

void mht_close()
{
	// unload any bitmaps
	if(!bm_unload(Mht_bitmap)){
		nprintf(("General","WARNING : could not unload background bitmap %s\n", Mht_bitmap_fname[gr_screen.res]));
	}	
	
	// destroy the UI_WINDOW
	Mht_window.destroy();
}

void mht_check_buttons()
{
	int idx;
	for(idx=0; idx<MHT_NUM_BUTTONS; idx++){
		// we only really need to check for one button pressed at a time, so we can break after 
		// finding one.
		if(Mht_buttons[gr_screen.res][idx].button.pressed()){
			mht_button_pressed(idx);
			break;
		}
	}
}

void mht_button_pressed(int n)
{
	switch(n){		
	case MHT_READY_ROOM:
		if (Player->flags & PLAYER_FLAGS_IS_MULTI){
			main_hall_do_multi_ready();
		} else {			
			gameseq_post_event(GS_EVENT_NEW_CAMPAIGN);			

			gamesnd_play_iface(SND_USER_SELECT);
		}
		break;

	case MHT_CAMPAIGN_ROOM:
		gameseq_post_event(GS_EVENT_CAMPAIGN_ROOM);			
		gamesnd_play_iface(SND_USER_SELECT);
		break;
		
	case MHT_OPTIONS:
		gameseq_post_event(GS_EVENT_OPTIONS_MENU);
		gamesnd_play_iface(SND_USER_SELECT);
		break;

	case MHT_TECH_ROOM:
		gameseq_post_event( GS_EVENT_TECH_MENU );
		gamesnd_play_iface(SND_USER_SELECT);
		break;
	
	case MHT_BARRACKS:
		gameseq_post_event( GS_EVENT_BARRACKS_MENU );
		gamesnd_play_iface(SND_USER_SELECT);
		break;

	case MHT_EXIT:
		mht_exit_game();
		gamesnd_play_iface(SND_USER_SELECT);
		break;
	}							
}

void mht_exit_game()
{
	// stop music first
	main_hall_stop_music(true);
	main_hall_stop_ambient();
	gameseq_post_event(GS_EVENT_QUIT_GAME);
}
