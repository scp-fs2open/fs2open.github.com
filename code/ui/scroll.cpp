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
#include "globalincs/alphacolors.h"



// --------------------------------------------------------------------
// UI_SCROLLBAR::link_hotspot
//
//
void UI_SCROLLBAR::link_hotspot(int up_button_num, int down_button_num)
{
	up_button.link_hotspot(up_button_num);
	down_button.link_hotspot(down_button_num);
}

// --------------------------------------------------------------------
// UI_SCROLLBAR::set_bmaps
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
int UI_SCROLLBAR::set_bmaps(char *up_button_fname, char *down_button_fname, char *line_fname)
{
	int bx, by, bh, bw;

	up_button.set_bmaps(up_button_fname);
	down_button.set_bmaps(down_button_fname);
	up_button.get_dimensions(&bx, &by, &bw, &bh);
	
	// set the bitmaps for the rectangle that is the scrollbar itself
	((UI_GADGET*)this)->set_bmaps(line_fname);
	uses_bmaps = 1;

	update_dimensions(x,y+bw,bw,h-bw*2);

	return 0;
}

void UI_SCROLLBAR::hide()
{
	hidden = 1;
	up_button.hide();
	down_button.hide();
}

void UI_SCROLLBAR::unhide()
{
	hidden = 0;
	up_button.unhide();
	down_button.unhide();
}

int UI_SCROLLBAR::get_hidden()
{
	return hidden;
}

void UI_SCROLLBAR::create(UI_WINDOW *wnd, int _x, int _y, int _h, int _start, int _stop, int _position, int _window_size)
{
	char *up = "^";
	char *down = "v";
	int bw = 20;

	base_create( wnd, UI_KIND_SCROLLBAR, _x, _y + bw, bw, _h - bw * 2 );

	up_button.create( wnd, up, _x, _y, bw, bw, 1 );
	up_button.set_parent(this);
	up_button.set_hotkey_if_focus(KEY_UP);

	down_button.create( wnd, down, _x, _y + _h - bw, bw, bw, 1 );
	down_button.set_parent(this);
	down_button.set_hotkey_if_focus(KEY_DOWN);

	horz = 0;
	start = _start;
	stop = _stop;
	position = _position;
	window_size = _window_size;
	bar_length = h;
	bar_position =  0;

	Assert( stop >= 0 );

	if (stop != start)
		bar_size = (window_size * h) / (stop - start + window_size + 1);
	else
		bar_size = h;

	if (bar_size < 7)
		bar_size = 7;

	bar_position = position - start;
	bar_position *= h - bar_size;
	bar_position /= stop - start;

	dragging = 0;
	last_scrolled = 0;
	moved = 1;
}

void UI_SCROLLBAR::draw()
{
	UI_GADGET::draw();

	if (uses_bmaps) {
		gr_reset_clip();
		if (disabled_flag) {
			if ( bmap_ids[SB_DISABLED] != -1 ) {
				gr_set_bitmap(bmap_ids[SB_DISABLED]);
				gr_bitmap(x,y,GR_RESIZE_MENU);
			}

		} else {
			if ( bmap_ids[SB_NORMAL] != -1 ) {
				gr_set_bitmap(bmap_ids[SB_NORMAL]);
				gr_bitmap(x,y,GR_RESIZE_MENU);
			}
		}

		gr_set_clip( x, y, w, h, GR_RESIZE_MENU );
		ui_draw_box_out( 0, bar_position, w - 1, bar_position + bar_size - 1 );

	} else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip( x, y, w, h, GR_RESIZE_MENU );

		if (my_wnd->selected_gadget == this)
			gr_set_color_fast(&CBRIGHT_GREEN);
		else
			gr_set_color_fast(&CGRAY);

		ui_rect( 0, 0, w-1, h-1 );
		ui_draw_box_out( 0, bar_position, w - 1, bar_position + bar_size - 1 );
	}
}

void UI_SCROLLBAR::process(int focus)
{
	int OnMe, OnSlider;
	int op;

	moved = 0;
	if (disabled_flag) {
		return;
	}

	if (my_wnd->selected_gadget == this)
		focus = 1;

	up_button.process(focus);
	down_button.process(focus);

	if (start == stop) {
		position = 0;
		bar_position = 0;
		return;
	}

	op = position;

	if (up_button.pressed()) {
		position--;
		if (position < start)
			position = start;

		bar_position = position - start;
		bar_position *= h - bar_size;
		bar_position /= stop - start;
		set_focus();
	}

	if (down_button.pressed()) {
		position++;
		if (position > stop)
			position = stop;

		bar_position = position - start;
		bar_position *= h - bar_size;
		bar_position /= stop - start;
		set_focus();
	}

	OnMe = is_mouse_on();

	if (!B1_PRESSED)
		dragging = 0;

	OnSlider = 0;
	if ( (ui_mouse.y >= bar_position + y) && (ui_mouse.y < bar_position + y + bar_size) && OnMe )
		OnSlider = 1;

	if (B1_JUST_PRESSED && OnSlider) {
		dragging = 1;
		drag_x = ui_mouse.x;
		drag_y = ui_mouse.y;
		drag_starting = bar_position;
		set_focus();
	}

	if ( B1_PRESSED && OnMe && !OnSlider && (timer_get_milliseconds() > last_scrolled + 1000 / (18*4)) ) {
		last_scrolled = timer_get_milliseconds();

		if ( ui_mouse.y < bar_position+y )	{
			// Page Up
			position -= window_size;
			if (position < start)
				position = start;

		} else {
			// Page Down
			position += window_size;
			if (position > stop)
				position = stop;
		}

		bar_position = position - start;
		bar_position *= h - bar_size;
		bar_position /= stop - start;
		set_focus();
	}

	if (B1_PRESSED && dragging) {
		bar_position = drag_starting + ui_mouse.y - drag_y;

		if (bar_position < 0) {
			bar_position = 0;
		}

		if (bar_position > h - bar_size) {
			bar_position = h - bar_size;
		}

		position = bar_position;
		position *= stop - start;
		position /= h - bar_size;
		position += start;

		if (position > stop)
			position = stop;

		if (position < start)
			position = start;

		set_focus();
	}

	if (op != position)
		moved = 1;
	else
		moved = 0;
}

int UI_SCROLLBAR::getpos()
{
	return position;
}

int UI_SCROLLBAR::changed()
{
	return moved;
}
