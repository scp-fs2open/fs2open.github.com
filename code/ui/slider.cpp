/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include "ui/uidefs.h"
#include "ui/ui.h"
#include "io/timer.h"

#include "missionui/missionscreencommon.h"
#include "bmpman/bmpman.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"



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

/*
// --------------------------------------------------------------------
// UI_SLIDER::link_hotspot
//
//
void UI_SLIDER::link_hotspot(int left_button_num, int right_button_num)
{
	left_button.link_hotspot(left_button_num);
	right_button.link_hotspot(right_button_num);
}

// --------------------------------------------------------------------
// UI_SLIDER::set_bmaps
//
// Call the UI_GADGET::set_bmaps() function for the child components
// of a scroll bar (the up and down button).  Set up the bmaps for the
// line itself.
//
// We also need to get the dimensions of the bitmap button so we can update
// the dimensions of the scrollbar.
//
// returns:		-1 ==> error
//					 0 ==> success
//
int UI_SLIDER::set_bmaps(char *left_button_fname, char *right_button_fname, char *bar_fname, char *marker_fname)
{
	int m_w,m_h;

	left_button.set_bmaps(left_button_fname);
	right_button.set_bmaps(right_button_fname);
	
	// set the bitmaps for the rectangle that is the scrollbar itself
	((UI_GADGET*)this)->set_bmaps(bar_fname);
	((UI_GADGET*)this)->set_bmaps(marker_fname,2);	// skip the first two bitmaps 

	bm_get_info( bmap_ids[SLIDER_MARKER_NORMAL], &m_w, &m_h, NULL );
	// force the slider dimensions based on size of marker bitmap
	w = n_positions * m_w;
	marker_w = m_w;
	marker_h = m_h;
	pixel_range = w-marker_w;
	increment = pixel_range / n_positions;

	right_button.update_dimensions(x+w, y, -1, -1);

	uses_bmaps = 1;

	return 0;
}

void UI_SLIDER::hide()
{
	hidden = 1;
	left_button.hide();
	right_button.hide();
}

void UI_SLIDER::unhide()
{
	hidden = 0;
	left_button.unhide();
	right_button.unhide();
}

int UI_SLIDER::get_hidden()
{
	return hidden;
}

void UI_SLIDER::create(UI_WINDOW *wnd, int _x, int _y, int _w, int _h, float _start, float _stop, float _current, int _n_positions )
{
	char *up = "<";
	char *down = ">";
	int bw, bh, real_w;
	bw=bh=_h;
	real_w = _n_positions*bw;
	base_create( wnd, UI_KIND_SLIDER, _x, _y, real_w, _h);

	left_button.create( wnd, up, _x-bw, _y, _h, _h );
	left_button.set_parent(this);

	right_button.create( wnd, down, _x+real_w, _y, bh, bh );
	right_button.set_parent(this);

	horz = 0;
	start = _start;
	stop = _stop;
	current = _current;

	Assert( _current >= _start );
	Assert( _current <= _stop );
	Assert( stop >= 0 );

	n_positions = _n_positions;

	dragging = 0;
	last_scrolled = 0;
	moved = 1;

	marker_w = _h;
	marker_h = _h;

	pixel_range = w-marker_w;
	marker_x = x + fl2i( ( (current - start)/(stop-start) * pixel_range ) );
	increment = pixel_range / n_positions;
	Assert(increment >= 1);
	mouse_locked = 0;
};

void UI_SLIDER::draw()
{
	if ( uses_bmaps ) {
		gr_reset_clip();
		if ( disabled_flag ) {
			if ( bmap_ids[SLIDER_BAR_DISABLED] != -1 ) {
				gr_set_bitmap(bmap_ids[SLIDER_BAR_DISABLED]);
				gr_bitmap(x,y,GR_RESIZE_MENU);
			}

			if ( bmap_ids[SLIDER_MARKER_DISABLED] != -1 ) {
				gr_set_bitmap(bmap_ids[SLIDER_MARKER_DISABLED]);
				gr_bitmap(marker_x,marker_y,GR_RESIZE_MENU);
			}

		}
		else {
			if ( bmap_ids[SLIDER_BAR_NORMAL] != -1 ) {
				gr_set_bitmap(bmap_ids[SLIDER_BAR_NORMAL]);
				gr_bitmap(x,y,GR_RESIZE_MENU);
			}

			if ( bmap_ids[SLIDER_MARKER_NORMAL] != -1 ) {
				gr_set_bitmap(bmap_ids[SLIDER_MARKER_NORMAL]);
				gr_bitmap(marker_x,marker_y,GR_RESIZE_MENU);
			}
		}
	}
	else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip( x, y, w, h, GR_RESIZE_MENU );

		if (my_wnd->selected_gadget == this)
			gr_set_color_fast( &CBRIGHT_GREEN );
		else
			gr_set_color_fast( &CGRAY );

		ui_rect( 0, 0, w-1, h-1 );

		gr_set_clip( marker_x, marker_y, w, h, GR_RESIZE_MENU );
		ui_draw_box_out(0, 0, marker_w, marker_h);
	}
}

void UI_SLIDER::process(int focus)
{
	int OnMe, OnMarker, keyfocus;
	int oldpos, op;
	float percent;
	moved = 0;

	if (disabled_flag) {
		return;
	}

	if (my_wnd->selected_gadget == this)
		keyfocus = 1;

	left_button.process(focus);
	right_button.process(focus);

	marker_y = y;
	keyfocus = 0;

	if (start == stop) {
		marker_x = x;
		return;
	}

	op = marker_x;
	oldpos = fake_position;

	OnMarker = 0;
	OnMe = is_mouse_on();
	if ( OnMe ) {
		if ( ui_mouse.x >= (marker_x ) && ui_mouse.x <= (marker_x+marker_w) ) {
			OnMarker = 1;
			if ( B1_PRESSED )
				mouse_locked = 1;
		}
	}

	if ( !B1_PRESSED) {
		mouse_locked = 0;
	}

	if ( (left_button.position!=0) || (keyfocus && keyd_pressed[KEY_LEFT]) || ( OnMe && B1_PRESSED && ui_mouse.x < marker_x) || (mouse_locked && ui_mouse.x < marker_x ) )	{
		if ( (timer_get_milliseconds() > last_scrolled+50) || left_button.just_pressed() || B1_JUST_PRESSED || mouse_locked || my_wnd->keypress == KEY_LEFT)	{
			if ( left_button.just_pressed() || B1_JUST_PRESSED || mouse_locked || my_wnd->keypress == KEY_LEFT )	{
				last_scrolled = timer_get_milliseconds() + 300;
			} else
				last_scrolled = timer_get_milliseconds();
			marker_x -= increment;
			if (marker_x < x )
				marker_x = x;
		}
	}

	if ( (right_button.position!=0) || (keyfocus && keyd_pressed[KEY_RIGHT]) || ( OnMe && B1_PRESSED && ui_mouse.x > (marker_x+marker_w)) || (mouse_locked && ui_mouse.x > marker_x+marker_w) ) {
		if ( (timer_get_milliseconds() > last_scrolled+50) || right_button.just_pressed() || B1_JUST_PRESSED || mouse_locked || my_wnd->keypress == KEY_RIGHT)	{
			if ( right_button.just_pressed() || B1_JUST_PRESSED || mouse_locked || my_wnd->keypress == KEY_RIGHT)	
				last_scrolled = timer_get_milliseconds() + 300;
			else
				last_scrolled = timer_get_milliseconds();
			marker_x += increment;
			if (marker_x > (x+n_positions*increment) )
				marker_x = x+n_positions*increment;
		}
	}

	percent = i2fl(marker_x - x)/i2fl(pixel_range);
	current = percent * (stop - start);
}

int UI_SLIDER::getpos()
{
	return marker_x;
}

float UI_SLIDER::getcurrent()
{
	return current;
}

int UI_SLIDER::changed()
{
	return moved;
}
*/
