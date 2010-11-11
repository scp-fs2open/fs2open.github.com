/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUDRETICLE_H
#define _HUDRETICLE_H

#include "graphics/2d.h"
#include "hud/hud.h"

struct player;
struct weapon_info;

extern int Outer_circle_radius[GR_NUM_RESOLUTIONS];
extern int Hud_reticle_center[GR_NUM_RESOLUTIONS][2];

#define NUM_WEAPON_LINK_MODES	5
#define LINK_ONE_PRIMARY		0
#define LINK_TWO_PRIMARY		1
#define LINK_ONE_SECONDARY		2
#define LINK_TWO_SECONDARY		3
#define LINK_THREE_SECONDARY	4

struct firepoint {
	vec2d xy;
	int active;
};

class HudGaugeReticle: public HudGauge
{
protected:
	hud_frames crosshair; 
	bool firepoint_display;
	SCP_vector<firepoint> fp;

	int firepoint_size;
	int firepoint_scale_x;
	int firepoint_scale_y;
public:
	HudGaugeReticle();
	void render(float frametime);
	void initBitmaps(char *fname);
	void pageIn();
	void initFirepointDisplay(bool firepoint, int scaleX, int scaleY, int size);
	void getFirepointStatus();
};

class HudGaugeThrottle: public HudGauge
{
protected:
	hud_frames throttle_frames; 

	int Bottom_offset_y;			// Hud_throttle_bottom_y[Hud_reticle_style][gr_screen.res]
	int throttle_h;			// Hud_throttle_h[gr_screen.res]
	int throttle_w;			// Hud_throttle_frame_w[gr_screen.res]
	int throttle_aburn_h;	// Hud_throttle_aburn_h[gr_screen.res]
	int Max_speed_offsets[2];		// Max_speed_coords[gr_screen.res][0] Max_speed_coords[gr_screen.res][1]
	int Zero_speed_offsets[2];		// Zero_speed_coords[gr_screen.res][0] Zero_speed_coords[gr_screen.res][1]
	int Orbit_center_offsets[2];	// Hud_reticle_center[gr_screen.res][0] Hud_reticle_center[gr_screen.res][1]
	int orbit_radius;		// Outer_circle_radius[gr_screen.res]

	bool Show_background;
public:
	HudGaugeThrottle();
	void initThrottleStartY(int y);
	void initThrottleSizes(int w, int h);	// throttle_w will be implicitly figured out using bm_get_info
	void initAburnHeight(int h);
	void initMaxSpeedOffsets(int x, int y);
	void initZeroSpeedOffsets(int x, int y);
	void initOrbitCenterOffsets(int x, int y);
	void initOrbitRadius(int radius);
	void showBackground(bool show);
	void initBitmaps(char *fname);

	void render(float frametime);
	void renderThrottleSpeed(float current_speed, int y_end);
	void renderThrottleLine(int y);
	void renderThrottleForeground(int y_end);
	void renderThrottleBackground(int y_end);

	void pageIn();
};

class HudGaugeThreatIndicator: public HudGauge
{
protected:
	hud_frames threat_arc; // The right arc if FS2 style reticle. Top arc if FS1 style reticle

	hud_frames laser_warn;
	int Laser_warn_offsets[2];

	int laser_warn_timer;
	int laser_warn_frame;

	hud_frames lock_warn;
	int Lock_warn_offsets[2];

	int lock_warn_timer;
	int lock_warn_frame;
public:
	HudGaugeThreatIndicator();
	void initBitmaps(char *fname_arc, char *fname_laser, char *fname_lock);
	void initLaserWarnOffsets(int x, int y);
	void initLockWarnOffsets(int x, int y);
	void render(float frametime);
	void initialize();
	void pageIn();
	void renderLaserThreat();
	void renderLockThreat();
};

class HudGaugeWeaponLinking: public HudGauge
{
protected:
	hud_frames arc;

	hud_frames weapon_linking_modes[NUM_WEAPON_LINK_MODES];
	int Weapon_link_offsets[NUM_WEAPON_LINK_MODES][2];
public:
	HudGaugeWeaponLinking();
	void init1PrimaryOffsets(int x, int y);
	void init2PrimaryOffsets(int x, int y);
	void init1SecondaryOffsets(int x, int y);
	void init2SecondaryOffsets(int x, int y);
	void init3SecondaryOffsets(int x, int y);
	void initBitmaps(char *fname_arc, 
		char *fname_primary_link_1, 
		char *fname_primary_link_2, 
		char *fname_secondary_link_1, 
		char *fname_secondary_link_2, 
		char *fname_secondary_link_3);
	void render(float frametime);
	void pageIn();
};

void hud_init_reticle();
void hud_update_reticle( player *pp );

void hud_draw_outer_reticle();
void hud_draw_center_reticle();
void hud_draw_throttle_gauge();
void hud_draw_target_throttle_gauge();



#endif
