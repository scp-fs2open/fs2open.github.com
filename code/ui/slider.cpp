/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "bmpman/bmpman.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "io/timer.h"
#include "missionui/missionscreencommon.h"
#include "ui/ui.h"
#include "ui/uidefs.h"



/// DOT_SLIDER class down here
void UI_DOT_SLIDER_NEW::create(UI_WINDOW *wnd, int _x, int _y, int _num_pos, char *bm_slider, int slider_mask,
																					char *bm_left, int left_mask, int left_x, int left_y,
																					char *bm_right, int right_mask, int right_x, int right_y,
																					int _dot_width)
{
	// no end buttons yet
	has_end_buttons = 0;

	// if any of the left/right arrow information is specified, make sure its _all_ specified
	if((bm_left != NULL) || (left_mask != -1) || (bm_right != NULL) || (right_mask != -1)){
		Assert((bm_left != NULL) && (left_mask >= 0) && (bm_right != NULL) && (right_mask >= 0));
		if((bm_left == NULL) || (left_mask < 0) || (bm_right == NULL) || (right_mask < 0)){
			return;
		}

		// now we know we have end buttons
		has_end_buttons = 1;
	}

	// internal stuff
	num_pos = _num_pos;
	base_create(wnd, UI_KIND_DOT_SLIDER_NEW, _x, _y, 0, 20);
	pos = 0;		
	dot_width = _dot_width;	

	// set bitmaps for the slider itself	
	button.create( wnd, "", _x, _y, 0, 0, 0, 1 );
	button.set_parent(this);
	button.link_hotspot(slider_mask);
	button.set_bmaps(bm_slider, num_pos, 0);	
	button.hide();
		
	// maybe setup buttons for the arrows
	if ( has_end_buttons ) {
		// Second button is the up (increase) button		
		up_button.create( wnd, "", right_x, right_y, 0, 0, 1, 1 );
		up_button.set_parent(this);
		up_button.set_highlight_action(common_play_highlight_sound);
		up_button.link_hotspot(right_mask);
		up_button.set_bmaps(bm_right);		

		// Third button is the down (decrease) button		
		down_button.create( wnd, "", left_x, left_y, 0, 0, 1, 1 );
		down_button.set_parent(this);
		down_button.set_highlight_action(common_play_highlight_sound);
		down_button.link_hotspot(left_mask);
		down_button.set_bmaps(bm_left);		
	}
}

void UI_DOT_SLIDER_NEW::draw()
{
	// draw end buttons
	if ( has_end_buttons ) {
		up_button.draw();
		down_button.draw();
	}
	
	// draw the proper dot
	Assert((pos >= 0) && (pos <= num_pos));	
	
	// for position -1, we don't draw (no dots)	
	if(pos >= 0){
		button.unhide();	
		button.draw_forced(pos);
		button.hide();		
	}
}

void UI_DOT_SLIDER_NEW::process(int focus)
{
	if (disabled_flag) {
		if (!hidden && !my_wnd->use_hack_to_get_around_stupid_problem_flag) {
			if (button.is_mouse_on() && B1_JUST_PRESSED) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			} else if (has_end_buttons && (up_button.is_mouse_on() || down_button.is_mouse_on())) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
			

			if ( (hotkey >= 0) && (my_wnd->keypress == hotkey) ){
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
		}

		return;
	}

	// check focus and derived focus with one variable
	if (my_wnd->selected_gadget == this){
		focus = 1;
	}

	// first check the dot area
	button.process(focus);
	if (button.button_down() || button.pressed() || mouse_captured()) {
		capture_mouse();  // while we are changing level, ignore all other buttons
		
		pos = (ui_mouse.x - x) / dot_width;		

		if (pos < 0){
			pos = 0;
		}

		// if we have 10 positions, 0 - 9 are valid
		if ( pos >= num_pos ) {
			pos = num_pos - 1;
		}

		return;
	}

	if ( has_end_buttons ) {
		up_button.process(focus);
		if (up_button.pressed()) {
			if (pos < num_pos-1){
				pos++;
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
		}

		down_button.process(focus);
		if (down_button.pressed()) {
			if(pos){
				pos--;
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
		}
	}
}

//
// OLD DOT SLIDER - TO BE PHASED OUT. IF YOU NEED TO USE A UI_DOT_SLIDER, use a UI_DOT_SLIDER_NEW -------------------
//

/// DOT_SLIDER class down here
void UI_DOT_SLIDER::create(UI_WINDOW *wnd, int _x, int _y, char *bm, int id, int end_buttons, int _num_pos)
{
	char	filename[MAX_PATH_LEN];
	int	bx, by, bw, hotspot;

	has_end_buttons = end_buttons;

	if ( has_end_buttons ) {
		bx = _x + 24;
		by = _y + 1;
		bw = 190;
		hotspot = id + 1;
	} else {
		bx = _x;
		by = _y;
		bw = 80;
		hotspot = id;
	}

	num_pos = _num_pos;

	sprintf(filename, "%s%.2d", bm, hotspot);
	first_frame = bm_load_animation(filename, &total_frames);
	if (first_frame < 0) {
		Error(LOCATION, "Could not load %s.ani\n", filename);
		disable();
		hide();
		return;
	}

	base_create(wnd, UI_KIND_DOT_SLIDER, bx, by, bw, 20);
	pos = 0;

	// A DOT_SLIDER has up to 3 child buttons..

	by = _y;

	// First button is the region with the dots
	button.create( wnd, "", bx, by, bw, 20, 0, 1 );
	button.set_parent(this);
	button.link_hotspot(hotspot);
	button.hide();

	if ( has_end_buttons ) {
		// Second button is the up (increase) button
		sprintf(filename, "%s%.2d", bm, id + 2);
		up_button.create( wnd, "", _x + 216, _y, 22, 24, 1, 1 );
		up_button.set_parent(this);
		up_button.set_highlight_action(common_play_highlight_sound);
		up_button.set_bmaps(filename);
		up_button.link_hotspot(id + 2);

		// Third button is the down (decrease) button
		sprintf(filename, "%s%.2d", bm, id);
		down_button.create( wnd, "", _x, _y, 22, 24, 1, 1 );
		down_button.set_parent(this);
		down_button.set_highlight_action(common_play_highlight_sound);
		down_button.set_bmaps(filename);
		down_button.link_hotspot(id);
	}
}

void UI_DOT_SLIDER::destroy()
{
	// release ani frames for the dots.
	bm_release(first_frame);

	UI_GADGET::destroy();
}

void UI_DOT_SLIDER::draw()
{
	if ( has_end_buttons ) {
		up_button.draw();
		down_button.draw();
	}
	Assert((pos >= 0) && (pos <= num_pos));
	gr_set_bitmap(first_frame + pos);  // draw the dot level
	gr_bitmap(x, y, GR_RESIZE_MENU);
}

void UI_DOT_SLIDER::process(int focus)
{
	if (disabled_flag)
		return;

	// check focus and derived focus with one variable
	if (my_wnd->selected_gadget == this)
		focus = 1;

	// first check the dot area
	button.process(focus);
	if (button.button_down() || button.pressed() || mouse_captured()) {
		capture_mouse();  // while we are changing level, ignore all other buttons

		if ( has_end_buttons ) {
			pos = (ui_mouse.x - x + 17) / 19;
		} else {
			pos = (ui_mouse.x - x) / 19;
		}

		if (pos < 0){
			pos = 0;
		}

		if ( pos > num_pos ){
			pos = num_pos;
		}

		return;
	}

	if ( has_end_buttons ) {
		up_button.process(focus);
		if (up_button.pressed()) {
			if (pos < num_pos){
				pos++;
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
		}

		down_button.process(focus);
		if (down_button.pressed()) {
			if (pos){
				pos--;
			} else {
				gamesnd_play_iface(SND_GENERAL_FAIL);
			}
		}
	}
}

