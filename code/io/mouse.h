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

#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <windows.h>
#endif

extern int Mouse_sensitivity;
extern int Use_mouse_to_fly;
extern int Mouse_hidden;
extern int Keep_mouse_centered;

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

#define MOUSE_LEFT_BUTTON		(1<<0)
#define MOUSE_RIGHT_BUTTON		(1<<1)
#define MOUSE_MIDDLE_BUTTON	(1<<2)

#define MOUSE_NUM_BUTTONS		3

// keep the following two #defines up to date with the #defines above
#define LOWEST_MOUSE_BUTTON	(1<<0)
#define HIGHEST_MOUSE_BUTTON	(1<<2)

// Returns the number of times button n went from up to down since last call
int mouse_down_count(int n, int reset_count = 1);
// Returns the number of times button n went from down to up since last call
int mouse_up_count(int n);

extern void mouse_flush();

int mouse_down(int btn);			// returns 1 if mouse button btn is down, 0 otherwise
float mouse_down_time(int btn);	// returns the fraction of time btn has been down since last call
int mouse_is_visible();				// returns 1 if mouse is visible, 0 otherwise

void mouse_lost_focus();
void mouse_got_focus();

void mouse_eval_deltas();
void mouse_get_delta(int *dx = NULL, int *dy = NULL, int *dz = NULL);

// portable routines to get and set the mouse position, relative
// to current window
void getWindowMousePos(POINT * pt);
void setWindowMousePos(POINT * pt);

extern void mouse_force_pos(int x, int y);

#endif
