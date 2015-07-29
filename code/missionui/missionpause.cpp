/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "missionui/missionpause.h"
#include "ui/ui.h"
#include "popup/popup.h"
#include "io/key.h"
#include "sound/audiostr.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "hud/hud.h"
#include "hud/hudmessage.h"
#include "object/object.h"
#include "graphics/font.h"
#include "globalincs/alphacolors.h"
#include "weapon/weapon.h"	
#include "controlconfig/controlsconfig.h"
#include "network/multi_pause.h"




// ----------------------------------------------------------------------------------------------------------------
// PAUSE DEFINES/VARS
//

// pause bitmap name
char *Pause_bmp_name[GR_NUM_RESOLUTIONS] = {
	"PleaseWait",
	"2_PleaseWait"
};

// pause bitmap display stuff
int Please_wait_coords[GR_NUM_RESOLUTIONS][4] = {
	{ // GR_640
		152, 217, 316, 26
	},
	{ // GR_1024
		247, 346, 510, 36
	}	
};

// pause window objects
UI_WINDOW Pause_win;
UI_CHECKBOX Pause_single_step;
UI_CHECKBOX Pause_physics;
UI_CHECKBOX Pause_ai;
UI_CHECKBOX Pause_ai_render;
UI_CHECKBOX Pause_firing;
UI_CHECKBOX Pause_external_view_mode_check;
UI_BUTTON Pause_continue;
int Pause_type = PAUSE_TYPE_NORMAL;

// if we're already paused
int Paused = 0;

// background screen (for the chatbox)
int Pause_background_bitmap = -1;

// saved background screen
int Pause_saved_screen = -1;

// if we're in external vie wmode
int Pause_external_view_mode = 0;

// externs
extern int Ai_render_debug_flag;
extern int Ai_firing_enabled;
extern int physics_paused;
extern int ai_paused;
extern int last_single_step;
extern int game_single_step;

// ----------------------------------------------------------------------------------------------------------------
// PAUSE FUNCTIONS
//

int pause_get_type()
{
	return Pause_type;
}

void pause_set_type(int type)
{
	Pause_type = type;
}

// initialize the pause screen
void pause_init()
{
	// if we're already paused. do nothing
	if ( Paused ) {
		return;
	}

	Assert( !(Game_mode & GM_MULTIPLAYER) );

	// pause all weapon sounds
	weapon_pause_sounds();

	if (Pause_type == PAUSE_TYPE_NORMAL)	{
		Pause_saved_screen = gr_save_screen();
	}

	// pause all game music
	audiostream_pause_all();

	Pause_win.create(0, 0, gr_screen.max_w_unscaled, gr_screen.max_h_unscaled, 0);	

	Pause_background_bitmap = bm_load(Pause_bmp_name[gr_screen.res]);

	Paused = 1;
}

extern int button_function_demo_valid(int n);
extern int button_function(int n);

// pause do frame - will handle running multiplayer operations if necessary
void pause_do()
{
	int k;
	const char *pause_str = XSTR("Paused", 767);
	int str_w, str_h;
	// next two are for view resetting
	static int previous_Viewer_mode = -1;
	static int previous_hud_state = -1;

	Assert( !(Game_mode & GM_MULTIPLAYER) );
	
	//	RENDER A GAME FRAME HERE AS THE BACKGROUND (if normal pause)

	if(Pause_type == PAUSE_TYPE_NORMAL)	{			
		// Fall back to viewer just incase saved screen is invalid
		if(Pause_saved_screen == -1){
			Pause_type = PAUSE_TYPE_VIEWER;
		}
		else if(Pause_type == PAUSE_TYPE_NORMAL)	{
			gr_restore_screen(Pause_saved_screen);
		}
	}

	if(Pause_type == PAUSE_TYPE_NORMAL){
		if (Pause_background_bitmap >= 0) {
			gr_set_bitmap(Pause_background_bitmap);

			// draw the bitmap
			gr_bitmap(Please_wait_coords[gr_screen.res][0], Please_wait_coords[gr_screen.res][1], GR_RESIZE_MENU);
			
			// draw "Paused" on it
			gr_set_color_fast(&Color_normal);
			gr_set_font(FONT2);
			gr_get_string_size(&str_w, &str_h, pause_str);
			gr_string((gr_screen.max_w_unscaled - str_w) / 2, (gr_screen.max_h_unscaled - str_h) / 2, pause_str, GR_RESIZE_MENU);
			gr_set_font(FONT1);
		}
	}

	if (Pause_type == PAUSE_TYPE_VIEWER) {
		if (previous_Viewer_mode < 0)
			previous_Viewer_mode = Viewer_mode;

		if (previous_hud_state < 0)
			previous_hud_state = hud_disabled();
	}

	// process the ui window here
	k = Pause_win.process() & ~KEY_DEBUGGED;
	switch (k)
	{ 
		case KEY_TAB:
			hud_toggle_draw();
			break;

		// view from outside of the ship
	   	case KEY_ENTER:
			if (Pause_type == PAUSE_TYPE_VIEWER) {
				button_function_demo_valid(VIEW_EXTERNAL);
			}
			break;

		// view from target
		case KEY_PADDIVIDE:
			if (Pause_type == PAUSE_TYPE_VIEWER) {
				button_function_demo_valid(VIEW_OTHER_SHIP);
			}
			break;

		// change target
		case KEY_PADMULTIPLY:
			if (Pause_type == PAUSE_TYPE_VIEWER) {
				button_function(TARGET_NEXT);
			}
			break;

		case KEY_ESC:
		case KEY_ALTED + KEY_PAUSE:
		case KEY_PAUSE:
			// reset previous view if we happened to be playing around with it during pause
			if (Pause_type == PAUSE_TYPE_VIEWER) {
				if (previous_Viewer_mode >= 0) {
					Viewer_mode = previous_Viewer_mode;
				}

				// NOTE remember that hud state is reversed here (0 == on, 1 == off)
				if ( (previous_hud_state >= 0) && (hud_disabled() != previous_hud_state) ) {
					hud_set_draw( !previous_hud_state );
				}
			}

			gameseq_post_event(GS_EVENT_PREVIOUS_STATE);		
			break;
	}	// end switch

	// draw the background window
	Pause_win.draw();

	// a very unique case where we shouldn't be doing the page flip because we're inside of popup code
	if(!popup_active()){
		if(Pause_type == PAUSE_TYPE_NORMAL) {
			gr_flip();
		}
	} else {
		// this should only be happening in a very unique multiplayer case
		Int3();
	}
}

// close the pause screen
void pause_close()
{
	// if we're not paused - do nothing
	if ( !Paused ) {
		return;
	}

	Assert( !(Game_mode & GM_MULTIPLAYER) );

	// unpause all weapon sounds
	weapon_unpause_sounds();

	// deinit stuff
	if(Pause_saved_screen != -1) {
		gr_free_screen(Pause_saved_screen);
		Pause_saved_screen = -1;
	}

	if (Pause_background_bitmap != -1){
		bm_release(Pause_background_bitmap);
		Pause_background_bitmap = -1;
	}

	Pause_win.destroy();		
	game_flush();

	// unpause all the music
	audiostream_unpause_all();		

	Paused = 0;
}

// debug pause init
void pause_debug_init()
{
	Pause_win.create( 100,100,400,300, WIN_DIALOG );

	Pause_physics.create( &Pause_win, NOX("Physics Pause <P>"), 200, 150, physics_paused );
	Pause_ai.create( &Pause_win, NOX("AI Pause <A>"), 200, 175, ai_paused );
	#ifndef NDEBUG
	Pause_ai_render.create( &Pause_win, NOX("AI Render Stuff <R>"), 200, 200, Ai_render_debug_flag);
	#endif
	Pause_firing.create( &Pause_win, NOX("AI firing <F>"), 200, 225, Ai_firing_enabled);
	Pause_external_view_mode_check.create( &Pause_win, NOX("External View <E>"), 200, 250, Pause_external_view_mode);
	Pause_single_step.create( &Pause_win, NOX("Single Step <S>"), 200, 290, game_single_step );
	Pause_continue.create( &Pause_win, NOX("Leave Pause"), 200, 350, 200, 40 );

	Pause_single_step.set_hotkey( KEY_S );
	Pause_physics.set_hotkey( KEY_P );
	Pause_ai.set_hotkey( KEY_A );
	Pause_ai_render.set_hotkey( KEY_R );
	Pause_firing.set_hotkey( KEY_F );
	Pause_external_view_mode_check.set_hotkey( KEY_E );
	Pause_continue.set_hotkey( KEY_ESC );

	Pause_continue.set_focus();
}

// debug pause do frame
void pause_debug_do()
{
	int key;

	key = Pause_win.process();
	if ( Pause_single_step.changed())	{
		game_single_step = Pause_single_step.checked();
	}

	if ( Pause_physics.changed())	{
		physics_paused = Pause_physics.checked();
	}

	if ( Pause_ai.changed())	{
		ai_paused = Pause_ai.checked();
		if (ai_paused){
			obj_init_all_ships_physics();
		}
	}

	if ( Pause_ai_render.changed())	{
		Ai_render_debug_flag = Pause_ai_render.checked();
	}

	if ( Pause_firing.changed())	{
		Ai_firing_enabled = Pause_firing.checked();
	}

	if ( Pause_external_view_mode_check.changed())	{
		Pause_external_view_mode = Pause_external_view_mode_check.checked();
		if (Pause_external_view_mode){
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "External view of player ship.", 182));
		} else {
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "View from inside player ship.", 183));
		}
	}

	if ( Pause_continue.pressed() || (key == KEY_PAUSE) )	{	//	Changed, MK, 11/9/97, only Pause break pause.
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	gr_clear();
	Pause_win.draw();

	gr_flip();
}

// debug pause close
void pause_debug_close()
{
	last_single_step = 0;	// Make so single step waits a frame before stepping
	Pause_win.destroy();
	game_flush();
}
