/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FREESPACE_HUDTARGETBOX_H__
#define __FREESPACE_HUDTARGETBOX_H__

#include "graphics/2d.h"
#include "radar/radarsetup.h"
#include "ship/ship.h"

struct object;

#define TBOX_FLASH_DURATION	1400
#define TBOX_FLASH_INTERVAL	200

// Flash timers that don't actually affect the targetbox have been moved to respective gauge classes (Swifty)
#define NUM_TBOX_FLASH_TIMERS		5 
#define TBOX_FLASH_NAME				0
#define TBOX_FLASH_CARGO			1
#define TBOX_FLASH_HULL				2
#define TBOX_FLASH_STATUS			3
#define TBOX_FLASH_SUBSYS			4

extern int Target_static_looping;
extern int Target_display_cargo;
extern char Cargo_string[256];

extern int Target_window_coords[GR_NUM_RESOLUTIONS][4];

//used to track if the player has wireframe hud target box turned on
extern int Targetbox_wire;
extern int Targetbox_shader_effect;
extern bool Lock_targetbox_mode;

class HudGaugeTargetBox: public HudGauge // HUD_TARGET_MONITOR
{
	hud_frames Monitor_frame;
	hud_frames Integrity_bar;
	int Monitor_mask;

	char static_fname[MAX_FILENAME_LEN];
	hud_anim Monitor_static;

	int Viewport_w;
	int Viewport_h;
	int Viewport_offsets[2];

	int Integrity_bar_offsets[2];
	int integrity_bar_h;
	int Status_offsets[2];
	
	int Name_offsets[2];
	int Class_offsets[2];
	int Dist_offsets[2];
	int Speed_offsets[2];
	int Cargo_string_offsets[2];

	// remember, these coords describe the rightmost position of this element, not the leftmost like it usually does.
	int Hull_offsets[2];

	int Cargo_scan_start_offsets[2];
	int Cargo_scan_h;
	int Cargo_scan_w;

	int Subsys_name_offsets[2];
	bool Use_subsys_name_offsets;

	int Subsys_integrity_offsets[2];
	bool Use_subsys_integrity_offsets;

	int Disabled_status_offsets[2];
	bool Use_disabled_status_offsets;

	bool Desaturated;

	// first element is time flashing expires, second element is time of next flash
	int Next_flash_timers[NUM_TBOX_FLASH_TIMERS];

	int Last_ts;
	int flash_flags;
public:
	HudGaugeTargetBox();

	void initBitmaps(char *fname_monitor, char *fname_monitor_mask, char *fname_integrity, char *fname_static);
	void initViewportOffsets(int x, int y);
	void initViewportSize(int w, int h);
	void initIntegrityOffsets(int x, int y);
	void initIntegrityHeight(int h);
	void initStatusOffsets(int x, int y);
	void initNameOffsets(int x, int y);
	void initClassOffsets(int x, int y);
	void initDistOffsets(int x, int y);
	void initSpeedOffsets(int x, int y);
	void initCargoStringOffsets(int x, int y);
	void initHullOffsets(int x, int y);
	void initCargoScanStartOffsets(int x, int y);
	void initCargoScanSize(int w, int h);
	void initSubsysNameOffsets(int x, int y, bool activate);
	void initSubsysIntegrityOffsets(int x, int y, bool activate);
	void initDisabledStatusOffsets(int x, int y, bool activate);
	void initDesaturate(bool desaturate);

	void initialize();
	void pageIn();
	void render(float frametime);
	void renderTargetShip(object *target_objp);
	void renderTargetWeapon(object *target_objp);
	void renderTargetDebris(object *target_objp);
	void renderTargetAsteroid(object *target_objp);
	void renderTargetJumpNode(object *target_objp);
	void renderTargetSetup(vec3d *camera_eye, matrix *camera_orient, float zoom);
	void renderTargetClose();
	void renderTargetForeground();
	void renderTargetIntegrity(int disabled, int force_obj_num = -1);
	int maybeFlashElement(int index, int flash_fast=0);
	void renderTargetShipInfo(object *target_objp);
	void maybeRenderCargoScan(ship_info *target_sip);
	void initFlashTimer(int index);
	void showTargetData(float frametime);
};

class HudGaugeExtraTargetData: public HudGauge // HUD_TARGET_MONITOR_EXTRA_DATA
{
	hud_frames bracket; 

	int flash_timer[2];
	bool flash_flags;

	int bracket_offsets[2]; // Targetbox_coords[gr_screen.res][TBOX_EXTRA]
	int dock_offsets[2]; // Targetbox_coords[gr_screen.res][TBOX_EXTRA_DOCK]
	int time_offsets[2]; // Targetbox_coords[gr_screen.res][TBOX_EXTRA_TIME]
	int order_offsets[2]; // Targetbox_coords[gr_screen.res][TBOX_EXTRA_ORDERS]
public:
	HudGaugeExtraTargetData();
	void initBitmaps(char *fname);
	void initBracketOffsets(int x, int y);
	void initDockOffsets(int x, int y);
	void initTimeOffsets(int x, int y);
	void initOrderOffsets(int x, int y);
	void updateFrame();
	void render(float frametime);
	void initialize();
	void initDockFlashTimer();
	void startFlashDock(int duration=TBOX_FLASH_DURATION);
	int maybeFlashDock(int flash_fast=0);
	void endFlashDock();
	void pageIn();
};

void	hud_targetbox_init();
void	hud_targetbox_init_flash();
void	hud_get_target_strength(object *objp, float *shields, float *integrity);

// used to flash text, uses the TBOX_FLASH_ #defines above
void	hud_targetbox_start_flash(int index, int duration=TBOX_FLASH_DURATION);
void	hud_targetbox_end_flash(int index);

// functions to manage the targetbox static that appears when sensors are severely damaged
void	hud_init_target_static();
void	hud_update_target_static();

void	hud_update_cargo_scan_sound();
void	hud_cargo_scan_update(object *targetp, float frametime);

void	hud_update_ship_status(object *targetp);

int		hud_targetbox_subsystem_in_view(object *target_objp, int *sx, int *sy);
void hud_targetbox_truncate_subsys_name(char *outstr);

//swich through the valid targetbox modes
void	hud_targetbox_switch_wireframe_mode();

#endif /* __FREESPACE_HUDTARGETBOX_H__ */
