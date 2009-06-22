/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _HUDCONFIG_H
#define _HUDCONFIG_H

#include "hud/hud.h"

struct player;
struct ship;
struct ai_info;

#define HUD_COLOR_GREEN		0
#define HUD_COLOR_BLUE		1
#define HUD_COLOR_AMBER		2

// specify the max distance that the radar should detect objects
// Index in Radar_ranges[] array to get values

#define RR_MAX_RANGES		3				// keep up to date
#define RR_SHORT			0
#define RR_LONG				1	
#define RR_INFINITY			2
extern float Radar_ranges[RR_MAX_RANGES];
extern char *Radar_range_text(int range_num);

#define RP_SHOW_DEBRIS						(1<<0)
#define RP_SHOW_FRIENDLY_MISSILES		(1<<1)
#define RP_SHOW_HOSTILE_MISSILES			(1<<2)

#define RP_DEFAULT ( RP_SHOW_DEBRIS | RP_SHOW_FRIENDLY_MISSILES | RP_SHOW_HOSTILE_MISSILES )

extern int HUD_observer_default_flags;
extern int HUD_observer_default_flags2;
extern int HUD_default_popup_mask;
extern int HUD_default_popup_mask2;
extern int HUD_config_default_flags;
extern int HUD_config_default_flags2;

typedef struct HUD_CONFIG_TYPE {		
	int show_flags;				// whether to show gauge
	int show_flags2;				// whether to show gauge
	int popup_flags;				// whether gauge is popup 	
	int popup_flags2;				// whether gauge is popup 		
	int num_msg_window_lines;	
	int rp_flags;					// see RP_ flags above
	int rp_dist;					// one of RR_ #defines above
	int is_observer;				// 1 or 0, observer mode or not, respectively
	int main_color;				// the main color

	// colors for all the gauges
	color clr[NUM_HUD_GAUGES];
} HUD_CONFIG_TYPE;

extern HUD_CONFIG_TYPE HUD_config;

void hud_config_init();
void hud_config_do_frame(float frametime);
void hud_config_close();

void hud_set_default_hud_config(player *p);
void hud_config_set_gauge_flags(int gauge_index, int on_flag, int popup_flag);

void hud_config_restore();
void hud_config_backup();
void hud_config_as_observer(ship *shipp,ai_info *aif);


void hud_config_as_observer();
void hud_config_as_player();
void hud_config_display_text(char* gauge_text, int x, int y);
void hud_set_display_gauge_cbox();

// leave hud config without accepting changes
void hud_config_cancel();

// leave hud config with accepting changes
void hud_config_commit();

// flag access/manipulation routines
int	hud_config_show_flag_is_set(int i);
void	hud_config_show_flag_set(int i);
void	hud_config_show_flag_clear(int i);
int	hud_config_popup_flag_is_set(int i);
void	hud_config_popup_flag_set(int i);
void	hud_config_popup_flag_clear(int i);

void hud_config_record_color(int color);

// load up the given hcf file
void hud_config_color_load(char *name);

#endif

