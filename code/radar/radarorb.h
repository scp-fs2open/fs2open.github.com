/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _RADARORB_H
#define _RADARORB_H

#include "radar/radarsetup.h"

extern int Radar_static_looping;

class object;
struct blip;
struct color;
struct vec3d;

#define NUM_ORB_RING_SLICES 16

class HudGaugeRadarOrb: public HudGaugeRadar
{
	char Radar_fname[MAX_FILENAME_LEN];
	hud_frames Radar_gauge;

	vec3d target_position;

	vec3d orb_ring_yz[NUM_ORB_RING_SLICES];
	vec3d orb_ring_xy[NUM_ORB_RING_SLICES];
	vec3d orb_ring_xz[NUM_ORB_RING_SLICES];

	color Orb_color_orange;
	color Orb_color_teal;
	color Orb_color_purple;
	color Orb_crosshairs;

	float Radar_center_offsets[2];
public:
	HudGaugeRadarOrb();
	void initBitmaps(char *fname);
	void initCenterOffsets(float x, float y);

	void loadDefaultPositions();
	void blipDrawDistorted(blip *b, vec3d *pos);
	void blipDrawFlicker(blip *b, vec3d *pos);
	void blitGauge();
	void drawBlips(int blip_type, int bright, int distort);
	void drawBlipsSorted(int distort);
	void drawContact(vec3d *pnt, int rad);
	void drawContactHtl(vec3d *pnt, int rad);
	void drawContactImage(vec3d *pnt, int rad, int idx, int clr_idx, float mult);
	void drawCrosshairs( vec3d pnt );
	void doneDrawing();
	void doneDrawingHtl();
	void drawOutlines();
	void drawOutlinesHtl();
	void setupView();
	void setupViewHtl();
	int calcAlpha(vec3d* pt);
	void render(float frametime);
	void pageIn();
	void plotBlip(blip *b, vec3d *scaled_pos);
};

#endif

