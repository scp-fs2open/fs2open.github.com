/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/UI/SCROLL.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:02 $
 * $Author: penguin $
 *
 * Code for vertical scrollbars.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 15    3/23/98 5:48p Hoffoss
 * Improved listbox handling.  Most notibly the scrollbar arrows work now.
 * 
 * 14    2/03/98 4:21p Hoffoss
 * Made UI controls draw white text when disabled.
 * 
 * 13    1/14/98 6:44p Hoffoss
 * Massive changes to UI code.  A lot cleaner and better now.  Did all
 * this to get the new UI_DOT_SLIDER to work properly, which the old code
 * wasn't flexible enough to handle.
 * 
 * 12    8/24/97 5:24p Lawrance
 * improve drawing of buttons 
 * 
 * 11    6/12/97 12:39p John
 * made ui use freespace colors
 * 
 * 10    6/11/97 1:13p John
 * Started fixing all the text colors in the game.
 * 
 * 9     5/26/97 10:26a Lawrance
 * get slider control working 100%
 * 
 * 8     5/22/97 5:36p Lawrance
 * allowing custom art for scrollbars
 * 
 * 7     1/28/97 4:58p Lawrance
 * allowing hidden UI components
 * 
 * 6     12/04/96 3:00p John
 * Added code to allow adjusting of HUD colors and saved it to the player
 * config file.
 * 
 * 5     12/03/96 11:29a John
 * Made scroll buttons on listbox scroll once, then delay, then repeat
 * when the buttons are held down.
 * 
 * 4     12/02/96 2:50p John
 * Made list box not scroll instantly.
 * 
 * 3     12/02/96 2:17p John
 * Made right button drag UI gadgets around and
 * Ctrl+Shift+Alt+F12 dumps out where they are.
 * 
 * 2     11/15/96 11:43a John
 * 
 * 1     11/14/96 6:55p John
 *
 * $NoKeywords: $
 */

#include "uidefs.h"
#include "ui.h"
#include "timer.h"
#include "alphacolors.h"


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
};

void UI_SCROLLBAR::draw()
{
	UI_GADGET::draw();

	if (uses_bmaps) {
		gr_reset_clip();
		if (disabled_flag) {
			if ( bmap_ids[SB_DISABLED] != -1 ) {
				gr_set_bitmap(bmap_ids[SB_DISABLED]);
				gr_bitmap(x,y);
			}

		} else {
			if ( bmap_ids[SB_NORMAL] != -1 ) {
				gr_set_bitmap(bmap_ids[SB_NORMAL]);
				gr_bitmap(x,y);
			}
		}

		gr_set_clip( x, y, w, h );
		ui_draw_box_out( 0, bar_position, w - 1, bar_position + bar_size - 1 );

	} else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip( x, y, w, h );

		if (my_wnd->selected_gadget == this)
			gr_set_color_fast(&CBRIGHT_GREEN);
		else
			gr_set_color_fast(&CGRAY);

	/*
		ui_rect( 0, 0, w-1, bar_position-1 );
		ui_rect( 0, bar_position+bar_size, w-1, h-1);
	*/
		ui_rect( 0, 0, w-1, h-1 );
		ui_draw_box_out( 0, bar_position, w - 1, bar_position + bar_size - 1 );
	}
}

void UI_SCROLLBAR::process(int focus)
{
	int OnMe, OnSlider;
	int oldpos, op;

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
	oldpos = bar_position;

	if (up_button.pressed()) {
		position--;
		if (position < start)
			position = start;

		bar_position = position - start;
		bar_position *= h - bar_size;
		bar_position /= stop - start;
		set_focus();
	}
/*
	if ( (up_button.position != 0) || (focus && keyd_pressed[KEY_UP]) ) {
		if ( (timer_get_milliseconds() > last_scrolled + 50) || up_button.just_pressed() ) {
			if ( up_button.just_pressed() ) {
				last_scrolled = timer_get_milliseconds() + 300;
			} else
				last_scrolled = timer_get_milliseconds();

			position--;
			if (position < start)
				position = start;

			bar_position = position - start;
			bar_position *= h - bar_size;
			bar_position /= stop - start;
		}
	}*/

	if (down_button.pressed()) {
		position++;
		if (position > stop)
			position = stop;

		bar_position = position - start;
		bar_position *= h - bar_size;
		bar_position /= stop - start;
		set_focus();
	}

/*	if ( down_button.position || (keyfocus && keyd_pressed[KEY_DOWN]) ) {
		if ( (timer_get_milliseconds() > last_scrolled + 50) || down_button.just_pressed() ) {
			if ( down_button.just_pressed() )
				last_scrolled = timer_get_milliseconds() + 300;
			else
				last_scrolled = timer_get_milliseconds();

			position++;
			if (position > stop )
				position = stop;

			bar_position = position-start;
			bar_position *= h-bar_size;
			bar_position /= (stop-start);
		}
	}*/

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

