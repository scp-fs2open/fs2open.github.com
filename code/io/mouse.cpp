/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */



#include "io/mouse.h"

#include "cmdline/cmdline.h"
#include "gamesequence/gamesequence.h"
#include "graphics/2d.h"
#include "parse/scripting.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif


int mouse_inited = 0;

int Mouse_sensitivity = 4;
int Use_mouse_to_fly = 0;

SDL_mutex* mouse_lock;


// Mouse axes
static int Mouse_x;
static int Mouse_y;

static int Mouse_dx = 0;
static int Mouse_dy = 0;

static int Last_mouse_dx = 0;
static int Last_mouse_dy = 0;

// Mousewheel axes
static int Mouse_wheel_x;
static int Mouse_wheel_y;

// Mouse button states
static int mouse_flags;
static int mouse_left_pressed = 0;
static int mouse_right_pressed = 0;
static int mouse_middle_pressed = 0;
static int mouse_x1_pressed = 0;
static int mouse_x2_pressed = 0;
static int mouse_left_up = 0;
static int mouse_right_up = 0;
static int mouse_middle_up = 0;
static int mouse_x1_up = 0;
static int mouse_x2_up = 0;

static bool Mouse_in_focus = true;


/**
 * @brief Handler called by OS on shutdown
 */
void mouse_close();

/**
* @brief Forces the mouse position to the given coordinates, if we have focus
*
* @param[in] x The new X position
* @param[in] y The new Y position
*/
void mouse_force_pos(int x, int y);

/**
 * @brief Decays the mousewheel position back to 0 and clears the appropriate flags when centered
 *
 * @param[in] btn The button (wheel direction) to check against
 */
static void mousewheel_decay(int btn);


static void mousewheel_decay(int btn) {
	switch (btn) {
	case MOUSE_WHEEL_UP:
		Mouse_wheel_y -= 1;
		break;
	case MOUSE_WHEEL_DOWN:
		Mouse_wheel_y += 1;
		break;
	case MOUSE_WHEEL_LEFT:
		Mouse_wheel_x -= 1;
		break;
	case MOUSE_WHEEL_RIGHT:
		Mouse_wheel_x += 1;
		break;
	}

	if (Mouse_wheel_x == 0) {
		mouse_flags &= ~(MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN);
	}
	if (Mouse_wheel_y == 0) {
		mouse_flags &= ~(MOUSE_WHEEL_RIGHT | MOUSE_WHEEL_LEFT);
	}
}

void mousewheel_motion(int x, int y) {

#if SDL_VERSION_ATLEAST(2, 0, 4)
	if (direction == SDL_MOUSEWHEEL_FLIPPED) {
		x = -x;
		y = -y;
	}
#endif

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

void mouse_close() {
	if (!mouse_inited)
		return;


	mouse_inited = 0;

	SDL_DestroyMutex(mouse_lock);
}

int mouse_down(int btn) {
	int tmp;
	if (!mouse_inited) return 0;

	// Bail if not a button or wheel direction
	if ((btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_WHEEL)) return 0;


	SDL_LockMutex(mouse_lock);


	if (mouse_flags & btn) {
		tmp = 1;
		if ((btn >= LOWEST_MOUSE_WHEEL) && (btn <= HIGHEST_MOUSE_WHEEL)) {
			mousewheel_decay(btn);
		}
	} else {
		tmp = 0;
	}

	SDL_UnlockMutex(mouse_lock);

	return tmp;
}

int mouse_down_count(int n, int reset_count) {
	int tmp = 0;
	if (!mouse_inited) return 0;

	if ((n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	SDL_LockMutex(mouse_lock);

	switch (n) {
	case MOUSE_LEFT_BUTTON:
		tmp = mouse_left_pressed;
		if (reset_count) {
			mouse_left_pressed = 0;
		}
		break;

	case MOUSE_RIGHT_BUTTON:
		tmp = mouse_right_pressed;
		if (reset_count) {
			mouse_right_pressed = 0;
		}
		break;

	case MOUSE_MIDDLE_BUTTON:
		tmp = mouse_middle_pressed;
		if (reset_count) {
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
	default:
		// Somebody tried checking multiple buttons. Get a Coder!
		Assert(0);
	}

	SDL_UnlockMutex(mouse_lock);

	return tmp;
}

float mouse_down_time(int btn) {
	float tmp;
	if (!mouse_inited) return 0.0f;

	// Bail if not a button or wheel direction
	if ((btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_WHEEL)) return 0.0f;

	SDL_LockMutex(mouse_lock);

	if (mouse_flags & btn) {
		tmp = 1.0f;
		if ((btn >= LOWEST_MOUSE_WHEEL) && (btn <= HIGHEST_MOUSE_WHEEL)) {
			mousewheel_decay(btn);
		}
	} else {
		tmp = 0.0f;
	}

	SDL_UnlockMutex(mouse_lock);

	return tmp;
}

void mouse_event(int x, int y, int dx, int dy) {
	Mouse_x = x;
	Mouse_y = y;

	// Add up these delta values so we don't overwrite previous events,
	// should be reset in gr_flip my mouse_reset_deltas()
	Mouse_dx += dx;
	Mouse_dy += dy;

	if (Mouse_dx != 0 || Mouse_dy != 0) {
		Script_system.RunCondition(CHA_MOUSEMOVED);
	}
}

void mouse_flush() {
	if (!mouse_inited)
		return;

	mouse_reset_deltas();
	Mouse_dx = Mouse_dy = 0;
	Mouse_wheel_x = Mouse_wheel_y = 0;
	SDL_LockMutex(mouse_lock);
	mouse_left_pressed = 0;
	mouse_right_pressed = 0;
	mouse_middle_pressed = 0;
	mouse_x1_pressed = 0;
	mouse_x2_pressed = 0;
	mouse_flags = 0;
	SDL_UnlockMutex(mouse_lock);
}

void mouse_force_pos(int x, int y) {
	if (os_foreground()) {  // only mess with windows's mouse if we are in control of it
		SDL_WarpMouseInWindow(os_get_window(), x, y);
	}
}

void mouse_get_delta(int *dx, int *dy) {
	if (dx)
		*dx = Mouse_dx;
	if (dy)
		*dy = Mouse_dy;
}

int mouse_get_pos(int *xpos, int *ypos) {
	int flags;

	if (!Mouse_in_focus) {
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

	if (Mouse_x < 0) {
		Mouse_x = 0;
	}

	if (Mouse_y < 0) {
		Mouse_y = 0;
	}

	if (Mouse_x >= gr_screen.max_w) {
		Mouse_x = gr_screen.max_w - 1;
	}

	if (Mouse_y >= gr_screen.max_h) {
		Mouse_y = gr_screen.max_h - 1;
	}

	if (xpos) {
		*xpos = Mouse_x;
	}

	if (ypos) {
		*ypos = Mouse_y;
	}

	return flags;
}

int mouse_get_pos_unscaled(int *xpos, int *ypos) {
	int flags = mouse_get_pos(xpos, ypos);

	gr_unsize_screen_pos((xpos) ? xpos : NULL, (ypos) ? ypos : NULL, NULL, NULL, GR_RESIZE_MENU);

	return flags;
}

void mouse_get_real_pos(int *mx, int *my) {
	SDL_GetMouseState(mx, my);
}

void mouse_got_focus() {
	if (!mouse_inited) return;

	Mouse_in_focus = true;

	mouse_flush();
}

void mouse_init() {
	// Initialize queue
	if (mouse_inited) return;
	mouse_inited = 1;

	mouse_lock = SDL_CreateMutex();

	SDL_LockMutex(mouse_lock);

	mouse_flags = 0;
	Mouse_x = gr_screen.max_w / 2;
	Mouse_y = gr_screen.max_h / 2;
	Mouse_wheel_x = 0;
	Mouse_wheel_y = 0;

	// we do want to make sure that button presses go through event polling though
	// (should be on by default already, just here as a reminder)
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEWHEEL, SDL_ENABLE);

	SDL_UnlockMutex(mouse_lock);

	atexit(mouse_close);
}

void mouse_lost_focus() {
	if (!mouse_inited) return;

	Mouse_in_focus = false;

	mouse_flush();
}

void mouse_mark_button(uint flags, int set) {
	if (!mouse_inited) return;

	SDL_LockMutex(mouse_lock);

	if (!(mouse_flags & MOUSE_LEFT_BUTTON)) {

		if ((flags & MOUSE_LEFT_BUTTON) && (set == 1)) {
			mouse_left_pressed++;
		}
	} else {
		if ((flags & MOUSE_LEFT_BUTTON) && (set == 0)) {
			mouse_left_up++;
		}
	}

	if (!(mouse_flags & MOUSE_RIGHT_BUTTON)) {

		if ((flags & MOUSE_RIGHT_BUTTON) && (set == 1)) {
			mouse_right_pressed++;
		}
	} else {
		if ((flags & MOUSE_RIGHT_BUTTON) && (set == 0)) {
			mouse_right_up++;
		}
	}

	if (!(mouse_flags & MOUSE_MIDDLE_BUTTON)) {

		if ((flags & MOUSE_MIDDLE_BUTTON) && (set == 1)) {
			mouse_middle_pressed++;
		}
	} else {
		if ((flags & MOUSE_MIDDLE_BUTTON) && (set == 0)) {
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

	if (set) {
		mouse_flags |= flags;
	} else {
		mouse_flags &= ~flags;
	}

	SDL_UnlockMutex(mouse_lock);

	Script_system.SetHookVar("MouseButton", 'i', &flags);

	//WMC - On Mouse Pressed and On Mouse Released hooks
	if (set == 1) {
		Script_system.RunCondition(CHA_MOUSEPRESSED);
	} else if (set == 0) {
		Script_system.RunCondition(CHA_MOUSERELEASED);
	}

	Script_system.RemHookVar("MouseButton");
}

void mouse_reset_deltas() {
	Last_mouse_dx = Mouse_dx;
	Last_mouse_dy = Mouse_dy;

	Mouse_dx = Mouse_dy = 0;
}

void mouse_set_pos(int xpos, int ypos) {
	mouse_force_pos(xpos, ypos);
}

int mouse_up_count(int n) {
	int tmp = 0;
	if (!mouse_inited) return 0;

	if ((n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	SDL_LockMutex(mouse_lock);

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
		// Somebody tried to check multiple buttons. Get a Coder!
		Assert(0);
		break;
	}

	SDL_UnlockMutex(mouse_lock);

	return tmp;
}
