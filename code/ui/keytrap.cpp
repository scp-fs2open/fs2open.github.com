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



void UI_KEYTRAP::create(UI_WINDOW *wnd, int key, void (*_user_function)(void) )
{
	base_create( wnd, UI_KIND_BUTTON, 0, 0, 0, 0 );

	pressed_down = 0;
	set_hotkey(key);
	set_callback(_user_function);

	parent = this;		// Ugly.  This keeps KEYTRAPS from getting keyboard control.
}

void UI_KEYTRAP::draw()
{
}

void UI_KEYTRAP::process(int focus)
{
	pressed_down = 0;

	if (disabled_flag) {
		return;
	}

	if (my_wnd->keypress == hotkey)	{
		pressed_down = 1;
		my_wnd->last_keypress = 0;
	}

	if (pressed_down && user_function)	{
		user_function();
	}
}

int UI_KEYTRAP::pressed()
{
	return pressed_down;
}
