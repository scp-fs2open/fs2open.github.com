/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/



#ifndef _MOUSE_H
#define _MOUSE_H

#include "controlconfig/controlsconfig.h"
#include "globalincs/pstypes.h"

extern int Mouse_sensitivity;
extern bool Use_mouse_to_fly;

// call once to init the mouse
void mouse_init();

extern void mouse_mark_button( uint flags, int set );

// Fills in xpos & ypos if not NULL.
// Returns Button states
// Always returns coordinates clipped to screen coordinates.
extern int mouse_get_pos( int *xpos, int *ypos );
// same as above but gets an unscaled reading (ie. the 1024x768/640x480 equivalent)
extern int mouse_get_pos_unscaled( int *xpos, int *ypos );

// get_real_pos could be negative.
extern void mouse_get_real_pos(int *mx, int *my);

extern void mouse_set_pos(int xpos,int ypos);

#define MOUSE_LEFT_BUTTON   (1<<0)
#define MOUSE_RIGHT_BUTTON  (1<<1)
#define MOUSE_MIDDLE_BUTTON (1<<2)
#define MOUSE_X1_BUTTON     (1<<3)
#define MOUSE_X2_BUTTON     (1<<4)
#define MOUSE_WHEEL_UP      (1<<5)  // Wheel is treated like a pair of buttons, but they don't have a down nor up count
#define MOUSE_WHEEL_DOWN    (1<<6)
#define MOUSE_WHEEL_LEFT    (1<<7)
#define MOUSE_WHEEL_RIGHT   (1<<8)

#define MOUSE_NUM_BUTTONS		9

// keep the following #defines up to date with the #defines above
// These mouse_button defines are a quick check to verify the input button is a mouse button
// Likewise, the mouse_wheel defines quickly check against the mousewheel directions
#define LOWEST_MOUSE_BUTTON     MOUSE_LEFT_BUTTON
#define HIGHEST_MOUSE_BUTTON    MOUSE_X2_BUTTON

#define LOWEST_MOUSE_WHEEL      MOUSE_WHEEL_UP
#define HIGHEST_MOUSE_WHEEL     MOUSE_WHEEL_RIGHT

/**
 * Checks how many times the given mouse button went down since last check
 *
 * @returns 0 if the CC_bind is not CID_MOUSE when Use_mouse_to_fly == false, or
 * @returns 0 if the CC_bind is not CID_JOY0 when Use_mouse_to_fly == true, or
 * @returns 0 if the button id >= NUM_MOUSE_BUTTONS, or
 * @returns 0 if the button did not go down, or
 *
 * @returns the number of times the button went down
 */
int mouse_down_count(const CC_bind &bind, int reset_count = 1);

/**
 * Checks how many times the given mouse button went down since last check
 */
int mouse_down_count(int n, int reset_count = 1);

/**
 * @brief Returns the number of times button n went from down to up since last call
 */
int mouse_up_count(const CC_bind &bind);

int mouse_up_count(int n);

/**
 * @brief Flushes the mouse's states clean. Called whenever it gains/loses focus
 */
void mouse_flush();

/**
 * Checks if the given mouse button is down
 * 
 * @returns 0 if the CC_bind is not CID_MOUSE when Use_mouse_to_fly == false, or
 * @returns 0 if the CC_bind is not CID_JOY0 when Use_mouse_to_fly == true, or
 * @returns 0 if the button id >= NUM_MOUSE_BUTTONS, or
 * @returns 0 if the button is not down, or
 * 
 * @returns 1 if the given button is down
 */
int mouse_down(const CC_bind &bind);

/**
 * Returns 1 if any of the given mouse buttons are down. 0 otherwise
 */
int mouse_down(int btn);

void mouse_reset_deltas();
void mouse_get_delta(int *dx = NULL, int *dy = NULL, int *dz = NULL);

void mouse_event(int x, int y, int dx, int dy);

/**
 * Called when there is motion on the mouse wheel(s). Supports 2 axes
 */
void mousewheel_motion(int x, int y, bool reversed);

extern void mouse_force_pos(int x, int y);

/**
 * Get the closest distance of a bit mask to 0
 * @note use this to translate mouse button masks to button id's
 */
short bit_distance(short x);

#endif
