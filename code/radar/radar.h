/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _RADAR_H
#define _RADAR_H

#include "radar/radarsetup.h"

extern int Radar_static_looping;

class object;
struct blip;
struct color;

extern void radar_init_std();
extern void radar_plot_object_std( object *objp );
extern void radar_frame_init_std();
extern void radar_mission_init_std();
extern void radar_frame_render_std(float frametime);

// observer hud rendering code uses this function
void radar_draw_blips_sorted_std(int distort);
void radar_draw_range_std();
void radar_blit_gauge_std();
void radar_stuff_blip_info_std(object *objp, int is_bright, color **blip_color, int *blip_type);
void radar_null_nblips_std();
void radar_draw_circle_std( int x, int y, int rad );
void radar_blip_draw_distorted_std(blip *b);
void radar_blip_draw_flicker_std(blip *b);
void radar_draw_image_std( int x, int y, int rad, int idx, int size);

class HudGaugeRadarStd: public HudGaugeRadar
{
	hud_frames Radar_gauge;

	int current_target_x, current_target_y;
	color radar_crosshairs;

	// formerly parts of Current_radar_global
	float Radar_center_offsets[2];
public:
	HudGaugeRadarStd();
	void initCenterOffsets(float x, float y);
	void initBitmaps(char *fname);

	void blipDrawDistorted(blip *b, int x, int y);
	void blipDrawFlicker(blip *b, int x, int y);
	void blitGauge();
	void drawBlips(int blip_type, int bright, int distort);
	void drawBlipsSorted(int distort);
	void drawContactCircle( int x, int y, int rad );
	void drawContactImage( int x, int y, int rad, int idx, int clr_idx, int size );
	void drawCrosshairs(int x, int y);
	void render(float frametime);
	void pageIn();
	void plotBlip(blip *b, int *x, int *y);
	void plotObject( object *objp );
};

#endif

