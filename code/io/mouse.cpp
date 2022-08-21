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

#include "controlconfig/controlsconfig.h"
#include "graphics/2d.h"
#include "io/mouse.h"
#include "options/Option.h"
#include "scripting/scripting.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"

#include "gamesequence/gamesequence.h"

int mouse_inited = 0;

LOCAL int Mouse_x;
LOCAL int Mouse_y;

LOCAL int Mouse_wheel_x;
LOCAL int Mouse_wheel_y;

SDL_mutex* mouse_lock;

int mouse_flags;
int mouse_left_pressed = 0;
int mouse_right_pressed = 0;
int mouse_middle_pressed = 0;
int mouse_x1_pressed = 0;
int mouse_x2_pressed = 0;
int mouse_left_up = 0;
int mouse_right_up = 0;
int mouse_middle_up = 0;
int mouse_x1_up = 0;
int mouse_x2_up = 0;

int Mouse_dx = 0;
int Mouse_dy = 0;
int Mouse_dz = 0;

int Mouse_sensitivity = 4;

static auto MouseSensitivityOption =
    options::OptionBuilder<int>("Input.MouseSensitivity", "Sensitivity", "The sentitivity of the mouse input.")
        .category("Input")
        .range(0, 9)
        .level(options::ExpertLevel::Beginner)
        .default_val(4)
        .bind_to(&Mouse_sensitivity)
        .importance(0)
        .finish();

bool Use_mouse_to_fly = false;

static SCP_string mouse_mode_display(bool mode) { return mode ? "Joy-0" : "Mouse"; }

static auto UseMouseOption = options::OptionBuilder<bool>("Input.UseMouse", "Mouse", "Use the mouse for flying")
                                 .category("Input")
				 .display(mouse_mode_display) 
                                 .level(options::ExpertLevel::Beginner)
                                 .default_val(false)
                                 .bind_to(&Use_mouse_to_fly)
                                 .importance(1)
                                 .finish();

namespace
{
	bool mouse_key_event_handler(const SDL_Event& e)
	{
		if (!os::events::isWindowEvent(e, os::getSDLMainWindow())) {
			return false;
		}

		switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				mouse_mark_button(MOUSE_LEFT_BUTTON, e.button.state);
				break;
			case SDL_BUTTON_RIGHT:
				mouse_mark_button(MOUSE_RIGHT_BUTTON, e.button.state);
				break;
			case SDL_BUTTON_MIDDLE:
				mouse_mark_button(MOUSE_MIDDLE_BUTTON, e.button.state);
				break;
			case SDL_BUTTON_X1:
				mouse_mark_button(MOUSE_X1_BUTTON, e.button.state);
				break;
			case SDL_BUTTON_X2:
				mouse_mark_button(MOUSE_X2_BUTTON, e.button.state);
				break;
			default:
				// SDL gave us an unknown button. Just log it
				mprintf(("Unknown SDL button %i\n", e.button.button));
		}

		return true;
	}

	bool mouse_motion_event_handler(const SDL_Event& e)
	{
		if (!os::events::isWindowEvent(e, os::getSDLMainWindow())) {
			return false;
		}

		mouse_event(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);

		return true;
	}

	bool mouse_wheel_event_handler(const SDL_Event& e)
	{
		if (!os::events::isWindowEvent(e, os::getSDLMainWindow())) {
			return false;
		}

#if SDL_VERSION_ATLEAST(2, 0, 4)
		mousewheel_motion(e.wheel.x, e.wheel.y, e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
#else
		mousewheel_motion(e.wheel.x, e.wheel.y, false);
#endif

		return true;
	}
}

void mouse_force_pos(int x, int y);

/**
 * @brief Decays the mousewheel position back to 0 and clears the appropriate flags when nuetral
 * @param[in] btn The button (wheel direction) to check against
 */
void mousewheel_decay(int btn);

static bool Mouse_in_focus = true;

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
	Mouse_wheel_x = 0;
	Mouse_wheel_y = 0;

	// we do want to make sure that button presses go through event polling though
	// (should be on by default already, just here as a reminder)
	SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_ENABLE );
	SDL_EventState( SDL_MOUSEBUTTONUP, SDL_ENABLE );
	SDL_EventState( SDL_MOUSEWHEEL, SDL_ENABLE );

	SDL_UnlockMutex( mouse_lock );

	os::events::addEventListener(SDL_MOUSEBUTTONDOWN, os::events::DEFAULT_LISTENER_WEIGHT, mouse_key_event_handler);
	os::events::addEventListener(SDL_MOUSEBUTTONUP, os::events::DEFAULT_LISTENER_WEIGHT, mouse_key_event_handler);

	os::events::addEventListener(SDL_MOUSEMOTION, os::events::DEFAULT_LISTENER_WEIGHT, mouse_motion_event_handler);

	os::events::addEventListener(SDL_MOUSEWHEEL, os::events::DEFAULT_LISTENER_WEIGHT, mouse_wheel_event_handler);

	atexit( mouse_close );
}


/******************************************************************************
 * @brief Marks a mouse button as up or down
 *
 * @param[in] flags Which button(s) are pressed/released
 * @param[in] set   Direction of the button(s). 1 = pressed, 0 = released
 *
 * @note This function is extern'ed by osapi.cpp and freespace.cpp
 */
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

	if (!(mouse_flags & MOUSE_X1_BUTTON)) {

		if ((flags & MOUSE_X1_BUTTON) && (set == 1)) {
			mouse_x1_pressed++;
		}
	} else {
		if ((flags & MOUSE_X1_BUTTON) && (set == 0)) {
			mouse_x1_up++;
		}
	}

	if (!(mouse_flags & MOUSE_X2_BUTTON)) {

		if ((flags & MOUSE_X2_BUTTON) && (set == 1)) {
			mouse_x2_pressed++;
		}
	} else {
		if ((flags & MOUSE_X2_BUTTON) && (set == 0)) {
			mouse_x2_up++;
		}
	}

	if ( set ){
		mouse_flags |= flags;
	} else {
		mouse_flags &= ~flags;
	}

	SDL_UnlockMutex( mouse_lock );

	Script_system.SetHookVar("MouseButton", 'i', flags);

	if (Script_system.IsActiveAction(CHA_MOUSEPRESSED) || Script_system.IsActiveAction(CHA_MOUSERELEASED)) {
		//WMC - On Mouse Pressed and On Mouse Released hooks
		if (set == 1) {
			Script_system.RunCondition(CHA_MOUSEPRESSED);
		} else if (set == 0) {
			Script_system.RunCondition(CHA_MOUSERELEASED);
		}
	}

	Script_system.RemHookVar("MouseButton");
}

void mouse_flush()
{
	if (!mouse_inited)
		return;

	mouse_reset_deltas();
	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	Mouse_wheel_x = Mouse_wheel_y = 0;
	SDL_LockMutex( mouse_lock );
	mouse_left_pressed = 0;
	mouse_right_pressed = 0;
	mouse_middle_pressed = 0;
	mouse_x1_pressed = 0;
	mouse_x2_pressed = 0;
	mouse_flags = 0;
	SDL_UnlockMutex( mouse_lock );
}

int mouse_down_count(const CC_bind &bind, int reset_count)
{
	// Bail if the incoming bind is not the right CID according to mouse-fly mode
	auto CID = bind.get_cid();
	if (Use_mouse_to_fly) {
		// Mouse is Joy0 in this mode
		if (CID != CID_JOY0) {
			return 0;
		}
	} else {
		// Mouse is Mouse in this mode
		if (CID != CID_MOUSE) {
			return 0;
		}
	}

	int btn = bind.get_btn();

	if (btn > MOUSE_NUM_BUTTONS) {
		return 0;
	}

	btn = 1 << btn;

	return mouse_down_count(btn, reset_count);
}

int mouse_down_count(int n, int reset_count) {
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

		case MOUSE_X1_BUTTON:
			tmp = mouse_x1_pressed;
			if (reset_count) {
				mouse_x1_pressed = 0;
			}
			break;

		case MOUSE_X2_BUTTON:
			tmp = mouse_x2_pressed;
			if (reset_count) {
				mouse_x2_pressed = 0;
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
int mouse_up_count(const CC_bind &bind)
{
	if (bind.get_cid() != CID_MOUSE) {
		return 0;
	}
	int n = 1 << bind.get_btn();
	return mouse_up_count(n);
}

int mouse_up_count(int n) {
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

		case MOUSE_X1_BUTTON:
			tmp = mouse_x1_up;
			mouse_x1_up = 0;
			break;

		case MOUSE_X2_BUTTON:
			tmp = mouse_x2_up;
			mouse_x2_up = 0;
			break;

		default:
			Assert(0);	// can't happen
			break;
	} // end switch

	SDL_UnlockMutex( mouse_lock );

	return tmp;
}

// returns 1 if mouse button btn is down, 0 otherwise

int mouse_down(const CC_bind &bind)
{
	// Bail if the incoming bind is not the right CID according to mouse-fly mode
	auto CID = bind.get_cid();
	if (Use_mouse_to_fly) {
		// Mouse is Joy0 in this mode
		if (CID != CID_JOY0) {
			return 0;
		}
	} else {
		// Mouse is Mouse in this mode
		if (CID != CID_MOUSE) {
			return 0;
		}
	}

	int btn = bind.get_btn();

	if (btn >= MOUSE_NUM_BUTTONS) {
		return 0;
	}

	btn = 1 << btn;

	return mouse_down(btn);
}

int mouse_down(int btn) {
	int tmp;
	if ( !mouse_inited ) return 0;

	// Bail if not a button or wheel direction
	if ((btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_WHEEL)) return 0;


	SDL_LockMutex( mouse_lock );


	if (mouse_flags & btn) {
		tmp = 1;
		if ((btn >= LOWEST_MOUSE_WHEEL) && (btn <= HIGHEST_MOUSE_WHEEL)) {
			mousewheel_decay(btn);
		}
	} else {
		tmp = 0;
	}

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
		SDL_WarpMouseInWindow(os::getSDLMainWindow(), x, y);
	}
}

// Reset deltas so we don't have duplicate mouse deltas
void mouse_reset_deltas()
{
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

	if (Script_system.IsActiveAction(CHA_MOUSEMOVED) && (Mouse_dx != 0 || Mouse_dy != 0))
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

	gr_unsize_screen_pos( (xpos) ? xpos : NULL, (ypos) ? ypos : NULL, NULL, NULL, GR_RESIZE_MENU );

	return flags;
}

void mouse_get_real_pos(int *mx, int *my)
{
	SDL_GetMouseState(mx, my);
}

void mouse_set_pos(int xpos, int ypos)
{
	mouse_force_pos(xpos, ypos);
}

void mousewheel_motion(int x, int y, bool reversed) {
	if (reversed) {
		x = -x;
		y = -y;
	}

	Mouse_wheel_x += x;
	Mouse_wheel_y += y;

	// These nested if's should take care of all edge cases.
	// Since x and y's magnitudes can be larger than 1, it is possible to ignore the idle state
	if (Mouse_wheel_y > 0) {
		// UP
		mouse_flags |= MOUSE_WHEEL_UP;
		mouse_flags &= ~MOUSE_WHEEL_DOWN;
	} else if (Mouse_wheel_y < 0) {
		// DOWN
		mouse_flags |= MOUSE_WHEEL_DOWN;
		mouse_flags &= ~MOUSE_WHEEL_UP;
	} else {
		mouse_flags &= ~(MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN);
	}

	if (Mouse_wheel_x > 0) {
		// RIGHT
		mouse_flags |= MOUSE_WHEEL_RIGHT;
		mouse_flags &= ~MOUSE_WHEEL_LEFT;
	} else if (Mouse_wheel_x < 0) {
		// LEFT
		mouse_flags |= MOUSE_WHEEL_LEFT;
		mouse_flags &= ~MOUSE_WHEEL_RIGHT;
	} else {
		mouse_flags &= ~(MOUSE_WHEEL_RIGHT | MOUSE_WHEEL_LEFT);
	}
}

void mousewheel_decay(int btn) {
	if (btn & MOUSE_WHEEL_UP) {
		Mouse_wheel_y -= 1;
	}
	if (btn & MOUSE_WHEEL_DOWN) {
		Mouse_wheel_y += 1;
	}
	if (btn & MOUSE_WHEEL_LEFT) {
		Mouse_wheel_x -= 1;
	}
	if (btn & MOUSE_WHEEL_RIGHT) {
		Mouse_wheel_x += 1;
	}

	if (Mouse_wheel_x == 0) {
		mouse_flags &= ~(MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN);
	}
	if (Mouse_wheel_y == 0) {
		mouse_flags &= ~(MOUSE_WHEEL_RIGHT | MOUSE_WHEEL_LEFT);
	}
}

short bit_distance(short x) {
	short i;
	const short max_dist = sizeof(short) * 8;
	
	for (i = 0; i < max_dist; ++i, x >>= 1) {
		if (x & 0x01) {
			break;
		}
	}

	if (i >= max_dist) {
		return -1;
	}

	return i;
}
