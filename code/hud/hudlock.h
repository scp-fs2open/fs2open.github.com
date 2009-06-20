/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUDLOCK_H
#define _HUDLOCK_H

void hud_init_missile_lock();
void hud_draw_lock_triangles(int center_x, int center_y, float frametime);
void hud_calculate_lock_position(float frametime);
void hud_calculate_lock_start_pos();
void hud_show_lock_indicator(float frametime, vec3d *lock_point_pos);
void hud_do_lock_indicator(float frametime);
void hud_stop_looped_locking_sounds();
void hud_lock_reset(float lock_time_scale=1.0f);

#endif
