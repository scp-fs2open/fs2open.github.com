/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDbrackets.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Header file for drawing target brackets on the HUD
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     12/21/98 5:02p Dave
 * Modified all hud elements to be multi-resolution friendly.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 14    5/03/98 1:07a Mike
 * Show + for ships attacking your target, whether hostile or friendly.
 * 
 * 13    3/19/98 5:36p Lawrance
 * Let subsystem brackets grow to screen size
 * 
 * 12    3/02/98 11:32p Lawrance
 * Allow asteroids about to impact ships to be bracketed
 * 
 * 11    11/27/97 4:24p Lawrance
 * change appearance of subsystem targeting brackets
 * 
 * 10    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 9     4/09/97 3:30p Lawrance
 * let target brackets grow to bracket ship entirely
 * 
 * 8     4/08/97 1:28p Lawrance
 * get brackets for targeting and messaging drawing right
 * 
 * 7     4/08/97 9:58a Lawrance
 * center bracket on target center.  Give min and max dimensions to
 * subsystem target brackets.
 * 
 * 6     4/07/97 3:50p Allender
 * ability to assign > 1 ship to a hotkey.  Enabled use of hotkeys in
 * squadmate messaging
 * 
 * 5     3/27/97 5:44p Lawrance
 * drawing dashed lines for sub-object targeting box that is not in line
 * of sight
 * 
 * 4     3/27/97 3:59p Lawrance
 * made brackets draw even if center of target is offscreen
 * 
 * 3     3/27/97 9:29a Lawrance
 * If reach maximum bounding box size, use radius targeting box method
 * 
 * 2     12/24/96 4:30p Lawrance
 * Target bracket drawing code moved to separate files
 *
 * $NoKeywords: $
 */


#ifndef HUD_BRACKETS
#define HUD_BRACKETS

#include "2d.h"

void hud_init_brackets();
void draw_bounding_brackets(int x1, int y1, int x2, int y2, int w_correction, int h_correction, float distance=0.0f, int target_objnum=-1);
void draw_bounding_brackets_subobject();
void draw_brackets_square(int x1, int y1, int x2, int y2);
void draw_brackets_diamond(int x1, int y1, int x2, int y2);
void draw_brackets_square_quick(int x1, int y1, int x2, int y2, int thick=0);
void draw_brackets_diamond_quick(int x1, int y1, int x2, int y2, int thick=0);
void draw_brackets_dashed_square_quick(int x1, int y1, int x2, int y2);
int hud_brackets_get_iff_color(int team);

#endif
