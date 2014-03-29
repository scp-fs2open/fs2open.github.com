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



// ---------------------------------------------------------------------------------------
// UI_ICON::create()
//
//
void UI_ICON::create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _w, int _h)
{
	if (_text)	
		text = vm_strdup(_text);
	else
		text = NULL;

	base_create(wnd, UI_KIND_ICON, _x, _y, _w, _h);
	m_flags = 0;
}

void UI_ICON::destroy()
{
	if (text) {
		vm_free(text);
		text = NULL;
	}

	UI_GADGET::destroy();
}

void UI_ICON::draw()
{
	if (uses_bmaps) {
		gr_reset_clip();
		if (disabled_flag) {
			if (bmap_ids[ICON_DISABLED] != -1) {
				gr_set_bitmap(bmap_ids[ICON_DISABLED]);
				gr_bitmap(x,y,GR_RESIZE_MENU);
			}

		} else if (this->is_mouse_on()) {
			if (B1_PRESSED) {
				if (bmap_ids[ICON_SELECTED] != -1) {
					gr_set_bitmap(bmap_ids[ICON_SELECTED]);
					gr_bitmap(x, y, GR_RESIZE_MENU);
				}

			} else {
				if (bmap_ids[ICON_HIGHLIGHT] != -1) {
					gr_set_bitmap(bmap_ids[ICON_HIGHLIGHT]);
					gr_bitmap(x, y, GR_RESIZE_MENU);
				}
			}

		} else {
			if (bmap_ids[ICON_NORMAL] != -1) {
				gr_set_bitmap(bmap_ids[ICON_NORMAL]);
				gr_bitmap(x, y, GR_RESIZE_MENU);
			}
		}

	} else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip(x, y, w, h, GR_RESIZE_MENU);

		ui_draw_box_out(0, 0, w-1, h-1);
		if (disabled_flag)
			gr_set_color_fast(&CDARK_GRAY);
		else 
			gr_set_color_fast(&CBLACK);

		if (text)
			ui_string_centered(Middle(w), Middle(h), text);

		gr_reset_clip();
	}
}

// -----------------------------------------------------------------------
// process()
//
void UI_ICON::process(int focus)
{
	int OnMe;

	if (disabled_flag) {
		return;
	}

	OnMe = is_mouse_on();

	if (!OnMe) {
		m_flags |= ICON_NOT_HIGHLIGHTED;

	} else {
		if ( m_flags & ICON_NOT_HIGHLIGHTED ) {
			m_flags |= ICON_JUST_HIGHLIGHTED;
			// if a callback exists, call it
			m_flags &= ~ICON_NOT_HIGHLIGHTED;
		}
	}
}
