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

struct object;

#define TBOX_FLASH_DURATION	1400
#define TBOX_FLASH_INTERVAL	200

#define NUM_TBOX_FLASH_TIMERS		14
#define TBOX_FLASH_NAME				0
#define TBOX_FLASH_CARGO			1
#define TBOX_FLASH_HULL				2
#define TBOX_FLASH_STATUS			3
#define TBOX_FLASH_SUBSYS			4
#define TBOX_FLASH_DOCKED			5
#define TBOX_FLASH_SQUADMSG		6
#define TBOX_FLASH_OBJECTIVE		7
#define TBOX_FLASH_COLLISION		8
#define TBOX_FLASH_CMEASURE		9
#define TBOX_FLASH_NETLAG			10
#define TBOX_FLASH_BLAST			11
#define TBOX_FLASH_EMP				12
#define TBOX_FLASH_ENGINE_WASH	13

extern int Target_static_looping;

extern int Target_window_coords[GR_NUM_RESOLUTIONS][4];

// flag to indicate whether to show the extra information about a target 
// The HUD_config controls whether this can be shown... but the player can still toggle it on/off
// during the game.
extern int Targetbox_show_extra_info;

//used to track if the player has wireframe hud target box turned on
extern int Targetbox_wire;
extern bool Lock_targetbox_mode;

void	hud_targetbox_init();
void	hud_targetbox_init_flash();
void	hud_render_target_model();
void	hud_show_target_data(float frametime);
void	hud_get_target_strength(object *objp, float *shields, float *integrity);

// used to flash text, uses the TBOX_FLASH_ #defines above
void	hud_targetbox_start_flash(int index, int duration=TBOX_FLASH_DURATION);
int	hud_targetbox_maybe_flash(int index, int flash_fast=0);
void	hud_targetbox_end_flash(int index);
int	hud_targetbox_is_bright(int index);
int	hud_targetbox_flash_expired(int index);

// functions to manage the targetbox static that appears when sensors are severely damaged
void	hud_targetbox_static_init();
int	hud_targetbox_static_maybe_blit(float frametime);

void hud_render_target_ship(object *target_objp);
void hud_render_target_debris(object *target_objp);
void hud_render_target_weapon(object *target_objp);

void hud_update_cargo_scan_sound();
void hud_cargo_scan_update(object *targetp, float frametime);

void hud_targetbox_truncate_subsys_name(char *outstr);

//swich through the valid targetbox modes
void hud_targetbox_switch_wireframe_mode();

#endif /* __FREESPACE_HUDTARGETBOX_H__ */
