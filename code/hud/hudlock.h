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

#include "hud/hud.h"

void hud_init_missile_lock();
void hud_draw_lock_triangles(int center_x, int center_y, float frametime);
void hud_calculate_lock_position(float frametime);
void hud_calculate_lock_start_pos();
void hud_show_lock_indicator(float frametime, vec3d *lock_point_pos);
void hud_do_lock_indicator(float frametime);
void hud_stop_looped_locking_sounds();
void hud_lock_reset(float lock_time_scale=1.0f);

class HudGaugeLock: public HudGauge
{
protected:
	hud_anim Lock_gauge;
	hud_anim Lock_anim;

	bool loop_locked_anim;

	int Lock_gauge_half_w;
	int Lock_gauge_half_h;
	int Lockspin_half_w;
	int Lockspin_half_h;
	float Lock_triangle_height;
	float Lock_triangle_base;
	int Lock_target_box_width;
	int Lock_target_box_height;

	int Rotate_time_id;
	int Lock_gauge_draw_stamp;
	int Lock_gauge_draw;
public:
	HudGaugeLock();
	void initBitmaps(char *lock_gauge_fname, char *lock_anim_fname);
	void initGaugeHalfSize(int w, int h);
	void initSpinHalfSize(int w, int h);
	void initTriHeight(float h);
	void initTriBase(float length);
	void initTargetBoxSize(int w, int h);
	void initLoopLockedAnim(bool loop);

	void render(float frametime);
	void renderLockTriangles(int center_x, int center_y, float frametime);
	void renderLockTrianglesOld(int center_x, int center_y, int radius);
	void pageIn();
	void initialize();
};

#endif
