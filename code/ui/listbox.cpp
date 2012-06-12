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
#include "io/key.h"
#include "globalincs/alphacolors.h"


#define KEY_BUFFER_TIMEOUT		1000		// time to clear buffer in milliseconds

#define DEFAULT_LISTBOX_ITEM_LENGTH 128

// --------------------------------------------------------------------
// UI_LISTBOX::link_hotspot
//
//
void UI_LISTBOX::link_hotspot(int up_button_num, int down_button_num)
{
	scrollbar.link_hotspot(up_button_num,down_button_num);
}

// --------------------------------------------------------------------
// UI_LISTBOX::set_bmaps
//
// Call the UI_SCROLLBAR::set_bmaps() function for the scroll bar gadget.
//
// returns:		-1 ==> error
//					 0 ==> success
//
int UI_LISTBOX::set_bmaps(char *lbox_fname, char *b_up_fname, char *b_down_fname, char *sb_fname)
{
	if (has_scrollbar) {
		scrollbar.set_bmaps(b_up_fname, b_down_fname, sb_fname);
	}
	
	// set the bitmaps for the list box rectangle
	UI_GADGET::set_bmaps(lbox_fname);
	uses_bmaps = 1;
	return 0;
}

void UI_LISTBOX::create(UI_WINDOW *wnd, int _x, int _y, int _w, int _h, int _numitems, char **_list, char *_check_list, int _max_items)
{

	int tw, th, nrows;
	int real_h;

	gr_set_font(wnd->f_id);
	gr_get_string_size(&tw, &th, "*");

	nrows = _h / th;
	real_h = nrows * th;

	base_create( wnd, UI_KIND_LISTBOX, _x, _y, _w, real_h );

   max_items = _max_items;	
   list = _list;
	num_items = _numitems;
	check_list = _check_list;	
	num_items_displayed = nrows;

	first_item = 0;
	current_item = -1;
	toggled_item = -1;
	last_scrolled = 0;
	textheight = th;
	dragging = 0;
	selected_item = -1;
	key_buffer_count = 0;
	last_typed = timer_get_milliseconds();

	if (_numitems > nrows) {
		scrollbar.create( wnd, _x+_w+3, _y, real_h, 0, _numitems-nrows, 0, nrows );
		scrollbar.set_parent(this);
		has_scrollbar = 1;

	} else {
		has_scrollbar = 0;
	}

	// kazan
	draw_frame = 1;
}


void UI_LISTBOX::set_drawframe(int mode)
{
	draw_frame = mode;
}

void UI_LISTBOX::draw()
{
	int i, x1, y1, stop;
	int w1, h1;

	UI_GADGET::draw();
	gr_set_font(my_wnd->f_id);

	if (uses_bmaps) {
		if (disabled_flag) {
			if ( bmap_ids[LBOX_DISABLED] >= 0 ) {
				gr_set_bitmap(bmap_ids[LBOX_DISABLED]);
				gr_bitmap(x, y);
			}

		} else {
			if ( bmap_ids[LBOX_NORMAL] >= 0 ) {
				gr_set_bitmap(bmap_ids[LBOX_NORMAL]);
				gr_bitmap(x, y);
			}
		}

	} else {
		gr_set_color_fast(&CBLACK);
		gr_set_clip( x, y, w, h );
		ui_rect( 0, 0, w-1, h-1 );
		gr_reset_clip();	
		if (draw_frame)
		{
			if (has_scrollbar) {
				ui_draw_sunken_border( x-2, y-2, x+w+scrollbar.w+4, y+h+1 );

			} else {
				ui_draw_sunken_border( x-2, y-2, x+w+4, y+h+1 );
			}
		}
	}

	stop = first_item+num_items_displayed;
	if (stop>num_items) stop = num_items;

	x1 = y1 = 0;
	gr_set_clip( x, y, w, h );

	for ( i=first_item; i<stop; i++ ) {
		gr_get_string_size( &w1, &h1,list[i] );

		if (check_list)
			w1 += 18;

		if (i !=current_item) {
/*
			if ((current_item == -1) && (my_wnd->selected_gadget == this ) && (i == first_item)  )	{
				if ( !uses_bmaps ) {
					gr_set_color_fast( &CBLACK );
					gr_rect( x1, y1, w1+2, h1 );
				}
				current_item = first_item;
				gr_set_color_fast( &CBRIGHT_GREEN );
			} else {
				if ( !uses_bmaps ) {
					gr_set_color_fast( &CBLACK );
					gr_rect( x1, y1, w1+2, h1 );
				}
				gr_set_color_fast( &CWHITE );
			}
*/
			if (!uses_bmaps) {
				gr_set_color_fast( &CBLACK );
				gr_rect( x1, y1, w1+2, h1 );
			}

			gr_set_color_fast(&CWHITE);

		} else {
			if (my_wnd->selected_gadget == this) {
				gr_set_color_fast( &CGRAY );
				gr_rect( x1, y1, w1+2, h1 );
				gr_set_color_fast( &CBRIGHT_GREEN );

			} else {
				gr_set_color_fast( &CGRAY );
				gr_rect( x1, y1, w1+2, h1 );
				gr_set_color_fast( &CBLACK );
			}
		}

		if ( check_list )	{
			if ( check_list[i] )	{
				gr_string( x1+2, y1, "X" );
			}

			gr_string( x1+16, y1, list[i] );

		} else
			gr_string( x1+2, y1, list[i] );

		if (i==current_item)
			gr_set_color_fast( &CGRAY );
		else
			gr_set_color_fast( &CBLACK );

		if ( !uses_bmaps ) {
			ui_rect( x1+w1+2, y1, w-1, y1+h1-1 );
			ui_rect( x1, y1, x1+1, y1+h1-1 );
		}

		y1 += h1;
	}

	if (stop < num_items_displayed-1 && !uses_bmaps) {
		gr_set_color_fast(&CBLACK);
		ui_rect( x1, y1, w-1, h-1 );
	}
}

void UI_LISTBOX::process(int focus)
{
	int OnMe, mitem, kf = 0;
	int i, j;

	selected_item = -1;
	toggled_item = -1;

	if (disabled_flag)
		return;

	if (my_wnd->selected_gadget == this)
		focus = 1;

	if (has_scrollbar) {
		scrollbar.process(0);
		if (my_wnd->selected_gadget == &scrollbar) {
			set_focus();
			focus = 1;
		}
	}

	if (num_items < 1) {
		current_item = -1;
		first_item = 0;
		old_current_item = current_item;
		old_first_item = first_item;
		
//		if (my_wnd->selected_gadget == this) {
//			my_wnd->selected_gadget == get_next();
//		}

		return;
	}

	old_current_item = current_item;
	old_first_item = first_item;

	OnMe = is_mouse_on();

	if (has_scrollbar) {
		if (scrollbar.moved) {
			first_item = scrollbar.position;
			Assert(first_item >= 0);

			if (current_item<first_item)
				current_item = first_item;

			if (current_item > first_item + num_items_displayed - 1)
				current_item = first_item + num_items_displayed - 1;
		}
	}

	if (!B1_PRESSED)
		dragging = 0;

	if (B1_PRESSED && OnMe) {
		set_focus();
		dragging = 1;
	}

	if ( key_buffer_count && (timer_get_milliseconds() > last_typed + KEY_BUFFER_TIMEOUT) )
		key_buffer_count = 0;

	if (focus) {
		if (my_wnd->keypress) {
			kf = 0;

			switch (my_wnd->keypress) {
				case KEY_ENTER:
					selected_item = current_item;
					break;

				case KEY_SPACEBAR:
					toggled_item = current_item;
					break;

				case KEY_UP:
					current_item--;
					kf = 1;
					break;

				case KEY_DOWN:
					current_item++;
					kf = 1;
					break;

				case KEY_HOME:
					current_item = 0;
					kf = 1;
					break;

				case KEY_END:
					current_item=num_items - 1;
					kf = 1;
					break;

				case KEY_PAGEUP:
					current_item -= num_items_displayed;
					kf = 1;
					break;

				case KEY_PAGEDOWN:
					current_item += num_items_displayed;
					kf = 1;
					break;

				default:		// enter the key in the key buffer
					if (my_wnd->keypress == KEY_BACKSP) {
						key_buffer_count = 0;

					} else if (key_buffer_count < MAX_KEY_BUFFER) {
						key_buffer[key_buffer_count++] = (char) my_wnd->keypress;
						last_typed = timer_get_milliseconds();
					}

					if (!key_buffer_count)
						break;

					for (i=0; i<num_items; i++) {
						char *current_text;
						
						current_text = get_string(i);
						for (j=0; j<key_buffer_count; j++)
							if ( (current_text[j] != ascii_table[key_buffer[j]]) && (current_text[j] != shifted_ascii_table[key_buffer[j]]) )
								break;

						if (j == key_buffer_count) {
							set_first_item(i - num_items_displayed / 2);
							set_current(i);
							break;
						}
					}
			}
		}

		if (kf == 1) {
			if (current_item < 0)
				current_item = 0;

			if (current_item >= num_items)
				current_item = num_items - 1;

			if (current_item < first_item)
				first_item = current_item;

			if (current_item >= first_item + num_items_displayed)
				first_item = current_item - num_items_displayed + 1;

			if (num_items <= num_items_displayed ) {
				first_item = 0;

			} else {
				if (has_scrollbar) {
					scrollbar.position = first_item;

					scrollbar.bar_position = scrollbar.position - scrollbar.start;
					scrollbar.bar_position *= scrollbar.h - scrollbar.bar_size;
					scrollbar.bar_position /= scrollbar.stop - scrollbar.start;

					if (scrollbar.bar_position < 0) {
						scrollbar.bar_position = 0;
					}
		
					if (scrollbar.bar_position > scrollbar.h - scrollbar.bar_size) {
						scrollbar.bar_position = scrollbar.h - scrollbar.bar_size;
					}
				}	
	
			}
		}
	}

	if (focus) {
		if (B1_PRESSED && dragging) {
			if (ui_mouse.y < y )
				mitem = -1;
			else
				mitem = (ui_mouse.y - y)/textheight;

			if ( (mitem < 0) && (timer_get_milliseconds() > last_scrolled + 1000 / 18) ) {
				current_item--;
				last_scrolled = timer_get_milliseconds();
			}

			if ( (mitem >= num_items_displayed) && (timer_get_milliseconds() > last_scrolled + 1000 / 18) ) {
				current_item++;
				last_scrolled = timer_get_milliseconds();
			}

			if ((mitem >= 0) && (mitem<num_items_displayed)) {
				current_item = mitem + first_item;
			}

			if (current_item < 0)
				current_item = 0;

			if (current_item >= num_items)
				current_item = num_items - 1;

			if (current_item < first_item)
				first_item = current_item;

			if (current_item >= first_item + num_items_displayed)
				first_item = current_item - num_items_displayed + 1;

			if (num_items <= num_items_displayed) {
				first_item = 0;

			} else if (has_scrollbar) {
				scrollbar.position = first_item;

				scrollbar.bar_position = scrollbar.position - scrollbar.start;
				scrollbar.bar_position *= scrollbar.h - scrollbar.bar_size;
				scrollbar.bar_position /= scrollbar.stop - scrollbar.start;

				if (scrollbar.bar_position < 0) {
					scrollbar.bar_position = 0;
				}

				if (scrollbar.bar_position > scrollbar.h - scrollbar.bar_size) {
					scrollbar.bar_position = scrollbar.h - scrollbar.bar_size;
				}
			}
		}

		if (check_list) {
			if (B1_JUST_RELEASED)
				toggled_item = current_item;
		}

		if (B1_DOUBLE_CLICKED) {
			selected_item = current_item;
		}
	}
}

int UI_LISTBOX::toggled()
{
	if (check_list) {
		return toggled_item;
	} else {
		return -1;
	}
}

int UI_LISTBOX::selected()
{
	if (check_list) {
		return -1;
	} else {
		return selected_item;
	}
}

int UI_LISTBOX::current()
{
	return current_item;
}

void UI_LISTBOX::set_current(int _index)
{
	current_item = _index;
}

void UI_LISTBOX::set_first_item(int _index)
{
	if (_index < 0)
		_index = 0;
	else if (_index > num_items)
		_index = num_items;

	first_item = _index;
}

char *UI_LISTBOX::get_string(int _index)
{
	return list[_index];
}

void UI_LISTBOX::clear_all_items()
{
   int idx;

	for ( idx=0; idx<num_items; idx++ )
		list[idx][0] = 0;

	num_items = 0;
	first_item = 0;
}

void UI_LISTBOX::set_new_list(int _numitems, char **_list)
{
 	num_items = _numitems;
	list = _list;
	current_item = 0;
}

void UI_LISTBOX::ScrollEnd()
{
	if (num_items > num_items_displayed)
		first_item = num_items - num_items_displayed + 1;
}

void UI_LISTBOX::RemoveFirstItem()
{
	for (int i = 0; i < num_items-1; i++)
		list[i] = list[i+1];
	num_items--;
}

int UI_LISTBOX::MaxSize()
{
	return max_items;
}

int UI_LISTBOX::CurSize()
{
	return num_items;
}

int UI_LISTBOX::add_string(char *str)
{
   if (max_items < 0)  // only if we created an "empty" listbox can we add items
		return 0;

	else {
		if ( (num_items == max_items - 1) || (strlen(str) > DEFAULT_LISTBOX_ITEM_LENGTH) )
			return 0;                     // we've reached our limit

		else {
			list[num_items] = vm_strdup(str);
			num_items++;
			return 1;
		}
	}
}

int UI_LISTBOX::sel_changed()
{
	return old_current_item == current_item ? 0 : 1;
}
