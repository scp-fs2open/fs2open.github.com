/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/UI/RADIO.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:10 $
 * $Author: penguin $
 *
 * Code to handle radio buttons.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
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
 * 9     3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 8     2/03/98 4:21p Hoffoss
 * Made UI controls draw white text when disabled.
 * 
 * 7     1/14/98 6:44p Hoffoss
 * Massive changes to UI code.  A lot cleaner and better now.  Did all
 * this to get the new UI_DOT_SLIDER to work properly, which the old code
 * wasn't flexible enough to handle.
 * 
 * 6     6/12/97 12:39p John
 * made ui use freespace colors
 * 
 * 5     6/11/97 1:13p John
 * Started fixing all the text colors in the game.
 * 
 * 4     5/26/97 10:26a Lawrance
 * get slider control working 100%
 * 
 * 3     1/01/97 6:46p Lawrance
 * changed text color of radio button to green from black
 * 
 * 2     11/15/96 11:43a John
 * 
 * 1     11/14/96 6:55p John
 *
 * $NoKeywords: $
 */

#include "ui/uidefs.h"
#include "ui/ui.h"
#include "globalincs/alphacolors.h"

void UI_RADIO::create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _state, int _group )
{
	int _w, _h;

//	gr_get_string_size( &_w, &_h, "X" );
	_w = 18;
	_h = 18;

	if (_text)	
		text = strdup(_text);
	else
		text = NULL;

	base_create( wnd, UI_KIND_RADIO, _x, _y, _w, _h );

	position = 0;
	pressed_down = 0;
	flag = _state;
	group = _group;
};

void UI_RADIO::destroy()
{
	if (text)
		free(text);

	UI_GADGET::destroy();
}

void UI_RADIO::draw()
{
	int offset;


	if ( uses_bmaps ) {

		if ( disabled_flag ) {
			if ( flag ) {
				if ( bmap_ids[RADIO_DISABLED_MARKED] != -1 ) {
					gr_set_bitmap(bmap_ids[RADIO_DISABLED_MARKED]);
					gr_bitmap(x,y);
				}
			}
			else {
				if ( bmap_ids[RADIO_DISABLED_CLEAR] != -1 ) {
					gr_set_bitmap(bmap_ids[RADIO_DISABLED_CLEAR]);
					gr_bitmap(x,y);
				}
			}
		}
		else {		// not disabled
			if ( position == 0 )	{	// up
				if ( flag ) {			// marked
					if ( bmap_ids[RADIO_UP_MARKED] != -1 ) {
						gr_set_bitmap(bmap_ids[RADIO_UP_MARKED]);
						gr_bitmap(x,y);
					}
				}
				else {					// not marked
					if ( bmap_ids[RADIO_UP_CLEAR] != -1 ) {
						gr_set_bitmap(bmap_ids[RADIO_UP_CLEAR]);
						gr_bitmap(x,y);
					}
				}
			}
			else {						// down 
				if ( flag ) {			// marked
					if ( bmap_ids[RADIO_DOWN_MARKED] != -1 ) {
						gr_set_bitmap(bmap_ids[RADIO_DOWN_MARKED]);
						gr_bitmap(x,y);
					}
				}
				else {					// not marked
					if ( bmap_ids[RADIO_DOWN_CLEAR] != -1 ) {
						gr_set_bitmap(bmap_ids[RADIO_DOWN_CLEAR]);
						gr_bitmap(x,y);
					}
				}
			}
		}
	}
	else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip( x, y, w, h );

		if (position == 0 )	{
			ui_draw_box_out( 0, 0, w-1, h-1 );
			offset = 0;
		} else {
			ui_draw_box_in( 0, 0, w-1, h-1 );
			offset = 1;
		}

		if (disabled_flag)
			gr_set_color_fast(&CDARK_GRAY);
		else if (my_wnd->selected_gadget == this)
			gr_set_color_fast(&CBRIGHT_GREEN);
		else 
			gr_set_color_fast(&CGREEN);

	//	if (flag)
	//		ui_string_centered(  Middle(w)+offset, Middle(h)+offset, "*" );
	//	else
	//		ui_string_centered(  Middle(w)+offset, Middle(h)+offset, "o" );
		if (flag)	{
			gr_circle( Middle(w)+offset, Middle(h)+offset, 8 );
		} else {
			gr_circle( Middle(w)+offset, Middle(h)+offset, 8 );
			gr_set_color_fast( &CWHITE );
			gr_circle( Middle(w)+offset, Middle(h)+offset, 4 );
		}

		if (disabled_flag)
			gr_set_color_fast(&CDARK_GRAY);
		else if (my_wnd->selected_gadget == this)
			gr_set_color_fast(&CBRIGHT_GREEN);
		else 
			gr_set_color_fast(&CGREEN);

		if ( text )	{
			gr_reset_clip();
			gr_string( x+w+4, y+2, text );
		}
	}
}

void UI_RADIO::process(int focus)
{
	int OnMe, oldposition;

	if (disabled_flag)	{
		position = 0;
		return;
	}

	if (my_wnd->selected_gadget == this)
		focus = 1;

	OnMe = is_mouse_on();

	oldposition = position;

	if (B1_PRESSED && OnMe) {
		position = 1;
	} else  {
		position = 0;
	}

	if (my_wnd->keypress == hotkey)	{
		position = 2;
		my_wnd->last_keypress = 0;
	}
		
	if ( focus && ((my_wnd->keypress == KEY_SPACEBAR) || (my_wnd->keypress == KEY_ENTER)) )
		position = 2;

	if (focus)
		if ( (oldposition == 2) && (keyd_pressed[KEY_SPACEBAR] || keyd_pressed[KEY_ENTER]) )
			position = 2;

	pressed_down = 0;

	if (position) {
		if ( (oldposition == 1) && OnMe )
			pressed_down = 1;
		if ( (oldposition == 2) && focus )
			pressed_down = 1;
	}

	if (pressed_down && user_function) {
		user_function();
	}

	if (pressed_down && (flag == 0)) {
		UI_GADGET *tmp = (UI_GADGET *) next;
		UI_RADIO *tmpr;

		while (tmp != this)	{
			if (tmp->kind == UI_KIND_RADIO) {
				tmpr = (UI_RADIO *) tmp;
				if ((tmpr->group == group) && tmpr->flag) {
					tmpr->flag = 0;
					tmpr->pressed_down = 0;
				}
			}

			tmp = tmp->next;
		}

		flag = 1;
	}
}

int UI_RADIO::changed()
{
	return pressed_down;
}

int UI_RADIO::checked()
{
	return flag;
}


