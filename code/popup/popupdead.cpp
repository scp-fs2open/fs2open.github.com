/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/





#define POPUPDEAD_NUM_CHOICES				3		// normal
#define POPUPDEAD_NUM_CHOICES_RA			4		// red alert
#define POPUPDEAD_NUM_CHOICES_SKIP		3		// skip mission menu

#define POPUPDEAD_NUM_CHOICES_MAX		4

#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "hud/hudmessage.h"
#include "io/key.h"
#include "io/timer.h"
#include "mission/missioncampaign.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "popup/popupdead.h"
#include "ui/ui.h"


UI_WINDOW	Popupdead_window;
UI_BUTTON	Popupdead_buttons[POPUPDEAD_NUM_CHOICES_MAX];				// actual lit buttons
UI_BUTTON	Popupdead_button_regions[POPUPDEAD_NUM_CHOICES_MAX];	// fake buttons used for mouse detection over text

int Popupdead_region_coords[GR_NUM_RESOLUTIONS][POPUPDEAD_NUM_CHOICES_MAX][4] =
{	
	{	// GR_640
		{464, 389, 497, 403},		// upper right pixel of text, lower right pixel of button (for tiny popup)
		{464, 413, 497, 427},
		{464, 435, 497, 446},		
		{464, 457, 497, 466},
	}, 
	{	// GR_1024
		{745, 627, 809, 664},
		{745, 663, 809, 700},		// upper right pixel of text, lower right pixel of button (for tiny popup)
		{745, 699, 809, 736},		
		{745, 735, 809, 772},
	}, 
};

int Popupdead_button_coords[GR_NUM_RESOLUTIONS][POPUPDEAD_NUM_CHOICES_MAX][2] =
{
	{	// GR_640
		{478, 387},						// upper left pixel (tiny popup)
		{478, 410},
		{478, 432},
		{478, 455},
	},
	{	// GR_1024
		{760, 620},						// upper left pixel (tiny popup)
		{760, 656},
		{760, 692},
		{760, 728},
	}
};

char *Popupdead_background_filename[GR_NUM_RESOLUTIONS] = {
	"PopDeath",		// GR_640
	"2_PopDeath"	// GR-1024
};

int Popupdead_background_coords[GR_NUM_RESOLUTIONS][2] = 
{
	{ // GR_640
		131, 363
	},
	{ // GR_1024
		205, 581
	}
};

char *Popupdead_button_filenames[GR_NUM_RESOLUTIONS][POPUPDEAD_NUM_CHOICES_MAX] = 
{
	{	// GR_640
		"PopD_00",				// first choice
		"PopD_01",				// second choice
		"PopD_02",				// third choice
		"PopD_03",				// fourth choice
	},
	{	// GR_1024
		"2_PopD_00",			// first choice
		"2_PopD_01",			// second choice
		"2_PopD_02",			// third choice
		"2_PopD_03",			// fourth choice
	}
};

int Popupdead_skip_message_y[GR_NUM_RESOLUTIONS] = { 
		96,	// GR_640
		160
};


static const char *Popupdead_button_text[POPUPDEAD_NUM_CHOICES_MAX];

// multiplayer specifics to help with return values since they can vary
#define POPUPDEAD_OBS_ONLY			1
#define POPUPDEAD_OBS_QUIT			2
#define POPUPDEAD_RESPAWN_ONLY	3
#define POPUPDEAD_RESPAWN_QUIT	4

int Popupdead_default_choice;		// What the default choice is (ie activated when Enter pressed)
int Popupdead_active	=	0;			// A dead popup is active
int Popupdead_choice;				// Index for choice picked (-1 if none picked)
int Popupdead_num_choices;			// number of buttons
int Popupdead_multi_type;			// what kind of popup is active for muliplayer
int Popupdead_skip_active = 0;	// The skip-misison popup is active
int Popupdead_skip_already_shown = 0;

int Popupdead_timer;

extern int Cmdline_mpnoreturn;
// Initialize the dead popup data
void popupdead_start()
{
	int			i;
	UI_BUTTON	*b;

	if ( Popupdead_active ) {
		return;
	}

	// increment number of deaths
	Player->failures_this_session++;


	// create base window
	Popupdead_window.create(Popupdead_background_coords[gr_screen.res][0], Popupdead_background_coords[gr_screen.res][1], 1, 1, 0);
	Popupdead_window.set_foreground_bmap(Popupdead_background_filename[gr_screen.res]);

	Popupdead_num_choices = 0;
	Popupdead_multi_type = -1;

	if ((The_mission.max_respawn_delay >= 0) && ( Game_mode & GM_MULTIPLAYER )) {
		Popupdead_timer = timestamp(The_mission.max_respawn_delay * 1000); 
		if (Game_mode & GM_MULTIPLAYER) {
			if(!(Net_player->flags & NETINFO_FLAG_LIMBO)){
				if (The_mission.max_respawn_delay) {
					HUD_printf("Player will automatically respawn in %d seconds", The_mission.max_respawn_delay);
				}
				else {
					HUD_printf("Player will automatically respawn now"); 
				}
			}
		}
	}

	if ( Game_mode & GM_NORMAL ) {
		// also do a campaign check here?
		if (0) { //((Player->show_skip_popup) && (!Popupdead_skip_already_shown) && (Game_mode & GM_CAMPAIGN_MODE) && (Game_mode & GM_NORMAL) && (Player->failures_this_session >= PLAYER_MISSION_FAILURE_LIMIT)) {
			// init the special preliminary death popup that gives the skip option
			Popupdead_button_text[0] = XSTR( "Do Not Skip This Mission", 1473);
			Popupdead_button_text[1] = XSTR( "Advance To The Next Mission", 1474);
			Popupdead_button_text[2] = XSTR( "Don't Show Me This Again", 1475);
			Popupdead_num_choices = POPUPDEAD_NUM_CHOICES_SKIP;
			Popupdead_skip_active = 1;
		} else if(The_mission.flags & MISSION_FLAG_RED_ALERT) {
			// We can't staticly declare these because they are externalized
			Popupdead_button_text[0] = XSTR( "Quick Start Mission", 105);
			Popupdead_button_text[1] = XSTR( "Return To Flight Deck", 106);
			Popupdead_button_text[2] = XSTR( "Return To Briefing", 107);
			Popupdead_button_text[3] = XSTR( "Replay previous mission", 1432);
			Popupdead_num_choices = POPUPDEAD_NUM_CHOICES_RA;
		} else {
			Popupdead_button_text[0] = XSTR( "Quick Start Mission", 105);
			Popupdead_button_text[1] = XSTR( "Return To Flight Deck", 106);
			Popupdead_button_text[2] = XSTR( "Return To Briefing", 107);
			Popupdead_num_choices = POPUPDEAD_NUM_CHOICES;
		}
	} else {
		// in multiplayer, we have different choices depending on respawn mode, etc.

		// if the player has run out of respawns and must either quit and become an observer
		if(Net_player->flags & NETINFO_FLAG_LIMBO){

			// the master should not be able to quit the game
			if( ((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (multi_num_players() > 1)) || (Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN) ) {
				Popupdead_button_text[0] = XSTR( "Observer Mode", 108);
				Popupdead_num_choices = 1;
				Popupdead_multi_type = POPUPDEAD_OBS_ONLY;
			} else {
				Popupdead_button_text[0] = XSTR( "Observer Mode", 108);
				Popupdead_button_text[1] = XSTR( "Return To Flight Deck", 106);
				Popupdead_num_choices = 2;
				Popupdead_multi_type = POPUPDEAD_OBS_QUIT;
			}
		} else {
			// the master of the game should not be allowed to quit
			if ( ((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (multi_num_players() > 1)) || (Net_player->flags & NETINFO_FLAG_TEAM_CAPTAIN) ) {
				Popupdead_button_text[0] = XSTR( "Respawn", 109);
				Popupdead_num_choices = 1;
				Popupdead_multi_type = POPUPDEAD_RESPAWN_ONLY;
			} else {
				Popupdead_button_text[0] = XSTR( "Respawn", 109);
				if(!Cmdline_mpnoreturn)
				{
					Popupdead_button_text[1] = XSTR( "Return To Flight Deck", 106);
					Popupdead_num_choices = 2;
				}
				else
				{
					Popupdead_num_choices = 1;
				}
				Popupdead_multi_type = POPUPDEAD_RESPAWN_QUIT;
			}
		}
	}

	// create buttons
	for (i=0; i < Popupdead_num_choices; i++) {
		b = &Popupdead_buttons[i];
		b->create(&Popupdead_window, "", Popupdead_button_coords[gr_screen.res][i][0], Popupdead_button_coords[gr_screen.res][i][1], 30, 20, 0, 1);
		b->set_bmaps(Popupdead_button_filenames[gr_screen.res][i], 3, 0);
		b->set_highlight_action(common_play_highlight_sound);

		// create invisible buttons to detect mouse presses... can't use mask since button region is dynamically sized
		int lx, w, h;
		gr_get_string_size(&w, &h, Popupdead_button_text[i]);
		lx = Popupdead_region_coords[gr_screen.res][i][0] - w;
		b = &Popupdead_button_regions[i];	
		b->create(&Popupdead_window, "", lx, Popupdead_region_coords[gr_screen.res][i][1], Popupdead_region_coords[gr_screen.res][i][2]-lx, Popupdead_region_coords[gr_screen.res][i][3]-Popupdead_region_coords[gr_screen.res][i][1], 0, 1);
		b->hide();
	}
	
	Popupdead_default_choice = 0;
	Popupdead_choice = -1;
	Popupdead_active = 1;
}

// maybe play a sound when key up/down is pressed to switch default choice
void popupdead_play_default_change_sound()
{
	int i, mouse_over=0;
	UI_BUTTON *br, *b;

	// only play if mouse not currently highlighting a choice
	for ( i = 0; i < Popupdead_num_choices; i++ ) {
		br = &Popupdead_button_regions[i];
		b = &Popupdead_buttons[i];
		if ( br->button_down() ) {
			mouse_over=1;
			break;
		}

		if ( br->button_hilighted() && !b->button_down() ) {
			mouse_over=1;
			break;
		}

		if ( b->button_hilighted() ) {
			mouse_over=1;
		}
	}

	if (!mouse_over) {
		gamesnd_play_iface(SND_USER_SELECT);
	}
}

// do any key processing here
// exit:	-1		=>	nothing was done
//			>=0	=> a choice was selected
int popupdead_process_keys(int k)
{
	int masked_k;

	if ( k <= 0 ) {
		return -1;
	}
	
	switch(k) {

	case KEY_ENTER:
		return Popupdead_default_choice;	// select the current default choice
		break;

	case KEY_ESC:
		if (Popupdead_skip_active) {
			return 0;								// 0 mimics a "do not skip"
		} else {
			return 1;								// do nothing here for now - 1 mimics a "return to flight deck"
		}
		break;

	case KEY_DOWN:
	case KEY_PAD2:
	case KEY_TAB:
		popupdead_play_default_change_sound();
		Popupdead_default_choice++;
		if ( Popupdead_default_choice >= Popupdead_num_choices ) {
			Popupdead_default_choice=0;
		}
		break;

	case KEY_UP:
	case KEY_PAD8:
	case KEY_SHIFTED+KEY_TAB:
		popupdead_play_default_change_sound();
		Popupdead_default_choice--;
		if ( Popupdead_default_choice < 0 ) {
			Popupdead_default_choice=Popupdead_num_choices-1;
		}
		break;

	case KEY_PAUSE:
		game_process_pause_key();
		break;

	default:
		break;
	} // end switch

	// read the dead key set
	masked_k = k & ~KEY_CTRLED;	// take out CTRL modifier only
	process_set_of_keys(masked_k, Dead_key_set_size, Dead_key_set);
	button_info_do(&Player->bi);	// call functions based on status of button_info bit vectors

	return -1;
}


// see if any popup buttons have been pressed
// exit: -1						=> no buttons pressed
//			>=0					=>	button index that was pressed
int popupdead_check_buttons()
{
	int			i;
	UI_BUTTON	*b;

	for ( i = 0; i < Popupdead_num_choices; i++ ) {
		b = &Popupdead_button_regions[i];
		if ( b->pressed() ) {
			return i;
		}

		b = &Popupdead_buttons[i];
		if ( b->pressed() ) {
			return i;
		}
	}

	return -1;
}

// See if any of the button should change appearance based on mouse position
void popupdead_force_draw_buttons()
{
	int i,mouse_is_highlighting=0;
	UI_BUTTON *br, *b;

	for ( i = 0; i < Popupdead_num_choices; i++ ) {
		br = &Popupdead_button_regions[i];
		b = &Popupdead_buttons[i];
		if ( br->button_down() ) {
			b->draw_forced(2);
			mouse_is_highlighting=1;
			continue;
		}

		if ( (b->button_hilighted()) || (br->button_hilighted() && !b->button_down()) ) {
			Popupdead_default_choice=i;
			mouse_is_highlighting=1;
			b->draw_forced(1);
		}
	}

	// Only if mouse is not highlighting an option, let the default choice be drawn highlighted
	if ( (!mouse_is_highlighting) && (Popupdead_num_choices>1) ) {
		for ( i = 0; i < Popupdead_num_choices; i++ ) {
			b = &Popupdead_buttons[i];
			// highlight the default choice
			if ( i == Popupdead_default_choice ) {
				b->draw_forced(1);
			}
		}
	}
}

// Draw the button text nicely formatted in the popup
void popupdead_draw_button_text()
{
	int w,h,i,sx,sy;

	gr_set_color_fast(&Color_bright_blue);

	for ( i=0; i < Popupdead_num_choices; i++ ) {
		gr_get_string_size(&w, &h, Popupdead_button_text[i]);
		sx = Popupdead_region_coords[gr_screen.res][i][0]-w;
		sy = Popupdead_region_coords[gr_screen.res][i][1]+4;
		gr_string(sx, sy, Popupdead_button_text[i], GR_RESIZE_MENU);
	}
}

// Called once per frame to run the dead popup
int popupdead_do_frame(float frametime)
{
	int k, choice;

	if ( !Popupdead_active ) {
		return -1;
	}

	// maybe show skip mission popup
	if ((!Popupdead_skip_already_shown) && (Player->show_skip_popup) && (Game_mode & GM_NORMAL) && (Game_mode & GM_CAMPAIGN_MODE) && (Player->failures_this_session >= PLAYER_MISSION_FAILURE_LIMIT)) {
		int popup_choice = popup(0, 3, XSTR("Do Not Skip This Mission", 1473),
												 XSTR("Advance To The Next Mission", 1474),
												 XSTR("Don't Show Me This Again", 1475),
												 XSTR("You have failed this mission five times.  If you like, you may advance to the next mission.", 1472) );
		switch (popup_choice) {
		case 0:
			// stay on this mission, so proceed to normal death popup
			// in other words, do nothing.
			break;
		case 1:
			// skip this mission
			Popupdead_active = 0;
			mission_campaign_skip_to_next();
			gameseq_post_event(GS_EVENT_START_GAME);
			return -1;
		case 2:
			// don't show this again
			Player->show_skip_popup = 0;
			break;
		}

		Popupdead_skip_already_shown = 1;
	}

	
	k = Popupdead_window.process();

	choice = popupdead_process_keys(k);
	if ( choice >= 0 ) {
		// do something different for single/multiplayer
		if ( Game_mode & GM_NORMAL ) {
			Popupdead_choice=choice;
		} else {
			Assert( Popupdead_multi_type != -1 );
			switch ( Popupdead_multi_type ) {
				
			case POPUPDEAD_OBS_ONLY:
			case POPUPDEAD_OBS_QUIT:
				Popupdead_choice = POPUPDEAD_DO_OBSERVER;
				if ( (Popupdead_multi_type == POPUPDEAD_OBS_QUIT) && (choice == 1) )
					Popupdead_choice = POPUPDEAD_DO_MAIN_HALL;
				break;

			case POPUPDEAD_RESPAWN_ONLY:
			case POPUPDEAD_RESPAWN_QUIT:
				Popupdead_choice = POPUPDEAD_DO_RESPAWN;
				if ( (Popupdead_multi_type == POPUPDEAD_RESPAWN_QUIT) && (choice == 1) )
					Popupdead_choice = POPUPDEAD_DO_MAIN_HALL;
				break;

			default:
				Int3();
				break;
			}
		}
	}

	choice = popupdead_check_buttons();
	if ( choice >= 0 ) {
		// do something different for single/multiplayer
		if ( Game_mode & GM_NORMAL ) {
			Popupdead_choice=choice;
		} else {
			Assert( Popupdead_multi_type != -1 );
			switch ( Popupdead_multi_type ) {
				
			case POPUPDEAD_OBS_ONLY:
			case POPUPDEAD_OBS_QUIT:
				Popupdead_choice = POPUPDEAD_DO_OBSERVER;
				if ( (Popupdead_multi_type == POPUPDEAD_OBS_QUIT) && (choice == 1) )
					Popupdead_choice = POPUPDEAD_DO_MAIN_HALL;
				break;

			case POPUPDEAD_RESPAWN_ONLY:
			case POPUPDEAD_RESPAWN_QUIT:
				Popupdead_choice = POPUPDEAD_DO_RESPAWN;
				if ( (Popupdead_multi_type == POPUPDEAD_RESPAWN_QUIT) && (choice == 1) )
					Popupdead_choice = POPUPDEAD_DO_MAIN_HALL;
				break;

			default:
				Int3();
				break;
			}
		}
	}

	Popupdead_window.draw();
	popupdead_force_draw_buttons();
	popupdead_draw_button_text();

	// maybe force the player to respawn if they've taken too long to choose
	if (( Game_mode & GM_MULTIPLAYER ) && (The_mission.max_respawn_delay >= 0) && (timestamp_elapsed(Popupdead_timer)) && (choice < 0)) {
		if (( Popupdead_multi_type == POPUPDEAD_RESPAWN_ONLY) || ( Popupdead_multi_type == POPUPDEAD_RESPAWN_QUIT)) {
			Popupdead_choice = POPUPDEAD_DO_RESPAWN; 
		}
	}

	return Popupdead_choice;
}

// Close down the dead popup
void popupdead_close()
{
	if ( !Popupdead_active ) {
		return;
	}

	gamesnd_play_iface(SND_POPUP_DISAPPEAR);
	Popupdead_window.destroy();
	game_flush();

	Popupdead_active = 0;
	Popupdead_skip_active = 0;
	Popupdead_skip_already_shown = 0;
}

// Is there a dead popup active?
int popupdead_is_active()
{
	return Popupdead_active;
} 
