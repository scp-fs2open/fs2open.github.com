/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/UI/icon.cpp $
 * $Revision: 2.2 $
 * $Date: 2004-07-12 16:33:08 $
 * $Author: Kazan $
 *
 * C++ class implementation for icon UI element
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
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
 * 5     2/03/98 4:21p Hoffoss
 * Made UI controls draw white text when disabled.
 * 
 * 4     1/20/98 10:36a Hoffoss
 * Fixed optimized warnings.
 * 
 * 3     1/14/98 6:43p Hoffoss
 * Massive changes to UI code.  A lot cleaner and better now.  Did all
 * this to get the new UI_DOT_SLIDER to work properly, which the old code
 * wasn't flexible enough to handle.
 * 
 * 2     9/07/97 10:05p Lawrance
 * add icon class
 * 
 * 1     9/06/97 11:22p Lawrance
 *
 * $NoKeywords: $
 */

#include "ui/uidefs.h"
#include "ui/ui.h"
#include "globalincs/alphacolors.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

// ---------------------------------------------------------------------------------------
// UI_ICON::create()
//
//
void UI_ICON::create(UI_WINDOW *wnd, char *_text, int _x, int _y, int _w, int _h)
{
	if (_text)	
		text = strdup(_text);
	else
		text = NULL;

	base_create(wnd, UI_KIND_ICON, _x, _y, _w, _h);
	m_flags = 0;
}

void UI_ICON::destroy()
{
	if (text) {
		free(text);
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
				gr_bitmap(x,y);
			}

		} else if (this->is_mouse_on()) {
			if (B1_PRESSED) {
				if (bmap_ids[ICON_SELECTED] != -1) {
					gr_set_bitmap(bmap_ids[ICON_SELECTED]);
					gr_bitmap(x, y);
				}

			} else {
				if (bmap_ids[ICON_HIGHLIGHT] != -1) {
					gr_set_bitmap(bmap_ids[ICON_HIGHLIGHT]);
					gr_bitmap(x, y);
				}
			}

		} else {
			if (bmap_ids[ICON_NORMAL] != -1) {
				gr_set_bitmap(bmap_ids[ICON_NORMAL]);
				gr_bitmap(x, y);
			}
		}

	} else {
		gr_set_font(my_wnd->f_id);
		gr_set_clip(x, y, w, h);

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
