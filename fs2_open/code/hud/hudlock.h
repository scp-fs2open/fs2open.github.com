/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Hud/HUDlock.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:58 $
 * $Author: penguin $
 *
 * Header file for missile locking code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 12    4/08/98 8:33p Lawrance
 * Make player re-acquire lock when targeting subsystems on a locked ship.
 * 
 * 11    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 10    2/28/98 7:03p Lawrance
 * Change player missile locking to use dot product, so we can use it in
 * the external views
 * 
 * 9     1/23/98 6:25p Lawrance
 * Change player missile locking to lock on subsystem points automatically
 * 
 * 8     1/21/98 7:20p Lawrance
 * Make subsystem locking only work with line-of-sight, cleaned up locking
 * code, moved globals to player struct.
 * 
 * 7     1/19/98 10:02p Lawrance
 * Fix bug with locking on friendlies
 * 
 * 6     11/17/97 6:37p Lawrance
 * new gauges: extended target view, new lock triangles, support ship view
 * 
 * 5     4/13/97 3:53p Lawrance
 * separate out the non-rendering dependant portions of the HUD ( sounds,
 * updating lock position, changing targets, etc) and put into
 * hud_update_frame()
 * 
 * 4     3/19/97 5:53p Lawrance
 * integrating new Misc_sounds[] array (replaces old Game_sounds
 * structure)
 * 
 * 3     1/02/97 7:12p Lawrance
 * adding hooks for more sounds
 * 
 * 2     12/23/96 7:53p Lawrance
 * missile locking working in new source files
 *
 * $NoKeywords: $
 */

#ifndef _HUDLOCK_H
#define _HUDLOCK_H

void hud_init_missile_lock();
void hud_draw_lock_triangles(int center_x, int center_y, float frametime);
void hud_calculate_lock_position(float frametime);
void hud_calculate_lock_start_pos();
void hud_show_lock_indicator(float frametime);
void hud_update_lock_indicator(float frametime);
void hud_stop_looped_locking_sounds();
void hud_lock_reset(float lock_time_scale=1.0f);

#endif
