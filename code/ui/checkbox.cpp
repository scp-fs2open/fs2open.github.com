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
#include "globalincs/alphacolors.h"

void UI_CHECKBOX::create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _state )
{
	int _w, _h;

//	gr_get_string_size( &_w, &_h, "X" );
	_w = 18;
	_h = 18;

	if ( _text )	
		text = vm_strdup( _text );
	else
		text = NULL;

	base_create( wnd, UI_KIND_CHECKBOX, _x, _y, _w, _h );

	position = 0;
	pressed_down = 0;
	flag = _state;
}

void UI_CHECKBOX::destroy()
{
	if (text)
		vm_free(text);

	UI_GADGET::destroy();
}


void UI_CHECKBOX::draw()
{
	int offset;

	if ( uses_bmaps ) {

		if ( disabled_flag ) {
			if ( flag ) {
				if ( bmap_ids[CBOX_DISABLED_MARKED] != -1 ) {
					gr_set_bitmap(bmap_ids[CBOX_DISABLED_MARKED]);
					gr_bitmap(x,y,GR_RESIZE_MENU);
				}
			}
			else {
				if ( bmap_ids[CBOX_DISABLED_CLEAR] != -1 ) {
					gr_set_bitmap(bmap_ids[CBOX_DISABLED_CLEAR]);
					gr_bitmap(x,y,GR_RESIZE_MENU);
				}
			}
		}
		else {		// not disabled
			if ( position == 0 )	{	// up
				if ( flag ) {			// marked
					if ( bmap_ids[CBOX_UP_MARKED] != -1 ) {
						gr_set_bitmap(bmap_ids[CBOX_UP_MARKED]);
						gr_bitmap(x,y,GR_RESIZE_MENU);
					}
				}
				else {					// not marked
					if ( bmap_ids[CBOX_UP_CLEAR] != -1 ) {
						gr_set_bitmap(bmap_ids[CBOX_UP_CLEAR]);
						gr_bitmap(x,y,GR_RESIZE_MENU);
					}
				}
			}
			else {						// down 
				if ( flag ) {			// marked
					if ( bmap_ids[CBOX_DOWN_MARKED] != -1 ) {
						gr_set_bitmap(bmap_ids[CBOX_DOWN_MARKED]);
						gr_bitmap(x,y,GR_RESIZE_MENU);
					}
				}
				else {					// not marked
					if ( bmap_ids[CBOX_DOWN_CLEAR] != -1 ) {
						gr_set_bitmap(bmap_ids[CBOX_DOWN_CLEAR]);
						gr_bitmap(x,y,GR_RESIZE_MENU);
					}
				}
			}
		}

	} else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip( x, y, w, h, GR_RESIZE_MENU );

		if (position == 0 )	{
			ui_draw_box_out( 0, 0, w-1, h-1 );
			offset = 0;

		} else {
			ui_draw_box_in( 0, 0, w-1, h-1 );
			offset = 1;
		}

		if (disabled_flag)
			gr_set_color_fast( &CGRAY );
		else if (my_wnd->selected_gadget == this)
			gr_set_color_fast( &CBRIGHT_GREEN );
		else 
			gr_set_color_fast( &CGREEN );

		if (flag)
			ui_string_centered( Middle(w) + offset, Middle(h) + offset, "X" );
		else
			ui_string_centered( Middle(w) + offset, Middle(h) + offset, " " );

		if (text) {
			gr_reset_clip();
			gr_string( x+w+4, y+2, text, GR_RESIZE_MENU );
		}
	}
}

void UI_CHECKBOX::process(int focus)
{
	int OnMe, oldposition;

	if (disabled_flag) {
		position = 0;
		return;
	}

	if (my_wnd->selected_gadget == this)
		focus = 1;

	OnMe = is_mouse_on();

	oldposition = position;

	if ( B1_PRESSED && OnMe ) {
		position = 1;
	} else  {
		position = 0;
	}

	if (my_wnd->keypress == hotkey )	{
		position = 2;
		my_wnd->last_keypress = 0;
	}
		
	if ( focus && ((my_wnd->keypress == KEY_SPACEBAR) || (my_wnd->keypress == KEY_ENTER)) )
		position = 2;

	if (focus)
		if ( (oldposition == 2) && (keyd_pressed[KEY_SPACEBAR] || keyd_pressed[KEY_ENTER]) )
			position = 2;

	pressed_down = 0;

	if (position == 0) {
		if ( (oldposition == 1) && OnMe ){
			pressed_down = 1;
		}
		if ( (oldposition == 2) && focus ){
			pressed_down = 1;
		}
	}

	if (pressed_down && user_function )	{
		user_function();
	}

	if (pressed_down)
		flag = !flag;
}

int UI_CHECKBOX::changed()
{
	return pressed_down;
}

int UI_CHECKBOX::checked()
{
	return flag;
}

void UI_CHECKBOX::set_state(int _state)
{
	flag = _state;
}
