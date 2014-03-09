/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include "io/mouse.h"
#include "graphics/2d.h"
#include "parse/scripting.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"

int mouse_inited = 0;

LOCAL int Mouse_x;
LOCAL int Mouse_y;

SDL_mutex* mouse_lock;

int mouse_flags;
int mouse_left_pressed = 0;
int mouse_right_pressed = 0;
int mouse_middle_pressed = 0;
int mouse_left_up = 0;
int mouse_right_up = 0;
int mouse_middle_up = 0;

int Mouse_dx = 0;
int Mouse_dy = 0;
int Mouse_dz = 0;

int Last_mouse_dx = 0;
int Last_mouse_dy = 0;
int Last_mouse_dz = 0;

int Mouse_sensitivity = 4;
int Use_mouse_to_fly = 0;


void mouse_force_pos(int x, int y);

static bool Mouse_in_focus = true;

void mouse_got_focus()
{
    if ( !mouse_inited ) return;

    Mouse_in_focus = true;

    mouse_flush();
}

void mouse_lost_focus()
{
    if ( !mouse_inited ) return;

    Mouse_in_focus = false;

    mouse_flush();
}

void mouse_close()
{
	if (!mouse_inited)
		return;

	mouse_inited = 0;

	SDL_DestroyMutex( mouse_lock );
}

void mouse_init()
{
	// Initialize queue
	if ( mouse_inited ) return;
	mouse_inited = 1;

	mouse_lock = SDL_CreateMutex();

	SDL_LockMutex( mouse_lock );

	mouse_flags = 0;
	Mouse_x = gr_screen.max_w / 2;
	Mouse_y = gr_screen.max_h / 2;

	// we do want to make sure that button presses go through event polling though
	// (should be on by default already, just here as a reminder)
	SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_ENABLE );
	SDL_EventState( SDL_MOUSEBUTTONUP, SDL_ENABLE );

	SDL_UnlockMutex( mouse_lock );	

	atexit( mouse_close );
}


// ----------------------------------------------------------------------------
// mouse_mark_button() is called asynchronously by the OS when a mouse button
// goes up or down.  The mouse button that is affected is passed via the 
// flags parameter.  
//
// parameters:   flags ==> mouse button pressed/released
//               set   ==> 1 - button is pressed
//                         0 - button is released

void mouse_mark_button( uint flags, int set)
{
	if ( !mouse_inited ) return;

	SDL_LockMutex( mouse_lock );

	if ( !(mouse_flags & MOUSE_LEFT_BUTTON) )	{

		if ( (flags & MOUSE_LEFT_BUTTON) && (set == 1) ) {
			mouse_left_pressed++;
		}
	}
	else {
		if ( (flags & MOUSE_LEFT_BUTTON) && (set == 0) ){
			mouse_left_up++;
		}
	}

	if ( !(mouse_flags & MOUSE_RIGHT_BUTTON) )	{

		if ( (flags & MOUSE_RIGHT_BUTTON) && (set == 1) ){
			mouse_right_pressed++;
		}
	}
	else {
		if ( (flags & MOUSE_RIGHT_BUTTON) && (set == 0) ){
			mouse_right_up++;
		}
	}

	if ( !(mouse_flags & MOUSE_MIDDLE_BUTTON) )	{

		if ( (flags & MOUSE_MIDDLE_BUTTON) && (set == 1) ){
			mouse_middle_pressed++;
		}
	}
	else {
		if ( (flags & MOUSE_MIDDLE_BUTTON) && (set == 0) ){
			mouse_middle_up++;
		}
	}

	if ( set ){
		mouse_flags |= flags;
	} else {
		mouse_flags &= ~flags;
	}

	SDL_UnlockMutex( mouse_lock );	

	Script_system.SetHookVar("MouseButton", 'i', &mouse_flags);

	//WMC - On Mouse Pressed and On Mouse Released hooks
	if(set == 1)
	{
		Script_system.RunCondition(CHA_MOUSEPRESSED);
	}
	else if(set == 0)
	{
		Script_system.RunCondition(CHA_MOUSERELEASED);
	}

	Script_system.RemHookVar("MouseButton");
}

void mouse_flush()
{
	if (!mouse_inited)
		return;

	mouse_reset_deltas();
	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	SDL_LockMutex( mouse_lock );
	mouse_left_pressed = 0;
	mouse_right_pressed = 0;
	mouse_middle_pressed = 0;
	mouse_flags = 0;
	SDL_UnlockMutex( mouse_lock );	
}

int mouse_down_count(int n, int reset_count)
{
	int tmp = 0;
	if ( !mouse_inited ) return 0;

	if ( (n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	SDL_LockMutex( mouse_lock );

	switch (n) {
		case MOUSE_LEFT_BUTTON:
			tmp = mouse_left_pressed;
			if ( reset_count ) {
				mouse_left_pressed = 0;
			}
			break;

		case MOUSE_RIGHT_BUTTON:
			tmp = mouse_right_pressed;
			if ( reset_count ) {
				mouse_right_pressed = 0;
			}
			break;

		case MOUSE_MIDDLE_BUTTON:
			tmp = mouse_middle_pressed;
			if ( reset_count ) {
				mouse_middle_pressed = 0;
			}
			break;
	} // end switch

	SDL_UnlockMutex( mouse_lock );	

	return tmp;
}

// mouse_up_count() returns the number of times button n has gone from down to up
// since the last call
//
// parameters:  n - button of mouse (see #define's in mouse.h)
//
int mouse_up_count(int n)
{
	int tmp = 0;
	if ( !mouse_inited ) return 0;

	if ( (n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	SDL_LockMutex( mouse_lock );

	switch (n) {
		case MOUSE_LEFT_BUTTON:
			tmp = mouse_left_up;
			mouse_left_up = 0;
			break;

		case MOUSE_RIGHT_BUTTON:
			tmp = mouse_right_up;
			mouse_right_up = 0;
			break;

		case MOUSE_MIDDLE_BUTTON:
			tmp = mouse_middle_up;
			mouse_middle_up = 0;
			break;

		default:
			Assert(0);	// can't happen
			break;
	} // end switch

	SDL_UnlockMutex( mouse_lock );	

	return tmp;
}

// returns 1 if mouse button btn is down, 0 otherwise

int mouse_down(int btn)
{
	int tmp;
	if ( !mouse_inited ) return 0;

	if ( (btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_BUTTON)) return 0;


	SDL_LockMutex( mouse_lock );


	if ( mouse_flags & btn )
		tmp = 1;
	else
		tmp = 0;

	SDL_UnlockMutex( mouse_lock );	

	return tmp;
}

// returns the fraction of time btn has been down since last call 
// (currently returns 1 if buttons is down, 0 otherwise)
//
float mouse_down_time(int btn)
{
	float tmp;
	if ( !mouse_inited ) return 0.0f;

	if ( (btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_BUTTON)) return 0.0f;

	SDL_LockMutex( mouse_lock );

	if ( mouse_flags & btn )
		tmp = 1.0f;
	else
		tmp = 0.0f;

	SDL_UnlockMutex( mouse_lock );

	return tmp;
}

void mouse_get_delta(int *dx, int *dy, int *dz)
{
	if (dx)
		*dx = Mouse_dx;
	if (dy)
		*dy = Mouse_dy;
	if (dz)
		*dz = Mouse_dz;
}

// Forces the actual windows cursor to be at (x,y).  This may be independent of our tracked (x,y) mouse pos.
void mouse_force_pos(int x, int y)
{
	if (os_foreground()) {  // only mess with windows's mouse if we are in control of it
		SDL_WarpMouseInWindow(os_get_window(), x, y);
	}
}

#include "gamesequence/gamesequence.h"

// Reset deltas so we don't have duplicate mouse deltas
void mouse_reset_deltas()
{
	Last_mouse_dx = Mouse_dx;
	Last_mouse_dy = Mouse_dy;
	Last_mouse_dz = Mouse_dz;
	
	Mouse_dx = Mouse_dy = Mouse_dz = 0;
}

void mouse_event(int x, int y, int dx, int dy)
{
	Mouse_x = x;
	Mouse_y = y;

	// Add up these delta values so we don't overwrite previous events,
	// should be reset in gr_flip my mouse_reset_deltas()
	Mouse_dx += dx;
	Mouse_dy += dy;

	if(Mouse_dx != 0 || Mouse_dy != 0)
	{
		Script_system.RunCondition(CHA_MOUSEMOVED);
	}
}

int mouse_get_pos(int *xpos, int *ypos)
{
	int flags;

    if (!Mouse_in_focus)
    {
        if (xpos)
			*xpos = Mouse_x;

		if (ypos)
			*ypos = Mouse_y;

        return 0;
    }

	if (!mouse_inited) {
		*xpos = *ypos = 0;
		return 0;
	}

	flags = mouse_flags;

	if (Mouse_x < 0){
		Mouse_x = 0;
	}

	if (Mouse_y < 0){
		Mouse_y = 0;
	}

	if (Mouse_x >= gr_screen.max_w){
		Mouse_x = gr_screen.max_w - 1;
	}

	if (Mouse_y >= gr_screen.max_h){
 		Mouse_y = gr_screen.max_h - 1;
	}
	
	if (xpos){
		*xpos = Mouse_x;
	}

	if (ypos){
		*ypos = Mouse_y;
	}

	return flags;
}

int mouse_get_pos_unscaled( int *xpos, int *ypos )
{
	int flags = mouse_get_pos( xpos, ypos );

	gr_unsize_screen_pos( (xpos) ? xpos : NULL, (ypos) ? ypos : NULL );

	return flags;
}

void mouse_get_real_pos(int *mx, int *my)
{
	POINT pnt;
	getWindowMousePos(&pnt);
	
	*mx = pnt.x;
	*my = pnt.y;
}

void mouse_set_pos(int xpos, int ypos)
{
	mouse_force_pos(xpos, ypos);
}

// portable routine to get the mouse position, relative
// to current window
void getWindowMousePos(POINT * pt)
{
	Assert(pt != NULL);

	int x = 0;
	int y = 0;

	SDL_GetMouseState(&x, &y);

	pt->x = x;
	pt->y = y;
}
