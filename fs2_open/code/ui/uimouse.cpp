/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/UI/UIMOUSE.cpp $
 * $Revision: 2.4 $
 * $Date: 2005-07-02 19:45:02 $
 * $Author: taylor $
 *
 * Code for dealing with the mouse
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/07/26 20:47:55  Kazan
 * remove MCD complete
 *
 * Revision 2.2  2004/07/12 16:33:08  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
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
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 10    4/12/98 5:31p Lawrance
 * use timer_get_milliseconds() instead of gettime()
 * 
 * 9     4/02/98 5:27p John
 * Removed the mouse_count stuff that Hoffoss added.
 * Still some problems with mose_up_count being used twice, but that is
 * easily fixed.
 * 
 * 8     4/02/98 5:05p Hoffoss
 * Fixed a timestamp() I missed.
 * 
 * 7     4/02/98 10:04a Adam
 * Fixed UI mouse problems with buttons not registering correctly.
 * 
 * 6     4/01/98 5:07p Hoffoss
 * Changed mouse button handling for UI in order to track fast events
 * (like button going down and released all between frames).
 * 
 * 5     2/06/98 3:36p Hoffoss
 * Made disabled buttons play failed sound if clicked on.  This is now
 * standard behavior for all UI buttons everywhere.
 * 
 * 4     1/14/98 6:44p Hoffoss
 * Massive changes to UI code.  A lot cleaner and better now.  Did all
 * this to get the new UI_DOT_SLIDER to work properly, which the old code
 * wasn't flexible enough to handle.
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

#include "ui/uidefs.h"
#include "ui/ui.h"
#include "io/timer.h"


UI_MOUSE ui_mouse;

int ui_mouse_inited = 0;

void ui_mouse_process()
{
	int buttons;

	if (!ui_mouse_inited) {
		ui_mouse_inited = 1;
		ui_mouse.x = 0;
		ui_mouse.y = 0;
		ui_mouse.dx = 0;
		ui_mouse.dy = 0;
		ui_mouse.b1_status = 0;
		ui_mouse.b1_last_status = 0;
		ui_mouse.b1_time_lastpressed=0;
		ui_mouse.b2_status = 0;
		ui_mouse.b2_last_status = 0;
		ui_mouse.b2_time_lastpressed = 0;
		ui_mouse.timestamp = timer_get_milliseconds();
	}

	buttons = mouse_get_pos( &ui_mouse.x, &ui_mouse.y );

	gr_unsize_screen_pos( &ui_mouse.x, &ui_mouse.y );

	// check if mouse pressed
	if (buttons & MOUSE_LEFT_BUTTON)
		ui_mouse.b1_status = BUTTON_PRESSED;
	else
		ui_mouse.b1_status = BUTTON_RELEASED;

	if (buttons & MOUSE_RIGHT_BUTTON)
		ui_mouse.b2_status = BUTTON_PRESSED;
	else
		ui_mouse.b2_status = BUTTON_RELEASED;

	// now check if we missed something between checks, just in case
	if (mouse_down_count(MOUSE_LEFT_BUTTON))
		ui_mouse.b1_status = BUTTON_PRESSED;

	if (mouse_up_count(MOUSE_LEFT_BUTTON))
		ui_mouse.b1_status = BUTTON_RELEASED;

	if (mouse_down_count(MOUSE_RIGHT_BUTTON))
		ui_mouse.b2_status = BUTTON_PRESSED;

	if (mouse_up_count(MOUSE_RIGHT_BUTTON))
		ui_mouse.b2_status = BUTTON_RELEASED;

	// check for double clicks
	if ((ui_mouse.b1_status & BUTTON_PRESSED) && (ui_mouse.b1_last_status & BUTTON_RELEASED) ) {
		if ( timer_get_milliseconds() <= ui_mouse.b1_time_lastpressed + 250 )  //&& (ui_mouse.moved==0)
			ui_mouse.b1_status |= BUTTON_DOUBLE_CLICKED;

		ui_mouse.b1_time_lastpressed = timer_get_milliseconds();
		ui_mouse.b1_status |= BUTTON_JUST_PRESSED;

	} else if ((ui_mouse.b1_status & BUTTON_RELEASED) && (ui_mouse.b1_last_status & BUTTON_PRESSED) )
		ui_mouse.b1_status |= BUTTON_JUST_RELEASED;

	if ((ui_mouse.b2_status & BUTTON_PRESSED) && (ui_mouse.b2_last_status & BUTTON_RELEASED) ) {
		if ( timer_get_milliseconds() <= ui_mouse.b2_time_lastpressed + 250 )  //&& (ui_mouse.moved==0)
			ui_mouse.b2_status |= BUTTON_DOUBLE_CLICKED;

		ui_mouse.b2_time_lastpressed = timer_get_milliseconds();
		ui_mouse.b2_status |= BUTTON_JUST_PRESSED;

	} else if ((ui_mouse.b2_status & BUTTON_RELEASED) && (ui_mouse.b2_last_status & BUTTON_PRESSED) )
		ui_mouse.b2_status |= BUTTON_JUST_RELEASED;

	ui_mouse.b1_last_status = ui_mouse.b1_status;
	ui_mouse.b2_last_status = ui_mouse.b2_status;
}
