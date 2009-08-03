/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FS2OPEN_RADARSETUP_H
#define _FS2OPEN_RADARSETUP_H

#include "hud/hudconfig.h"

//which radar type are we using
//to add another radar type, begin by adding a RADAR_MODE_* define and increment MAX_RADAR_MODES
#define RADAR_MODE_STANDARD 0
#define RADAR_MODE_ORB 1
#define MAX_RADAR_MODES 2

//structures
#define NUM_FLICKER_TIMERS	2

struct object;

typedef struct rcol {
	ubyte	r, g, b;
} rcol;

typedef struct blip	{
	blip	*prev, *next;
	int	x, y, rad;
	int	flags;	// BLIP_ flags defined above
	color *blip_color;
	vec3d position;
	int radar_image_2d;
	int radar_image_size;
	float radar_projection_size;
} blip;


#define MAX_BLIPS 150

#define	MAX_RADAR_COLORS		5
#define MAX_RADAR_LEVELS		2		// bright and dim radar dots are allowed

#define RCOL_BOMB				0
#define RCOL_NAVBUOY_CARGO		1
#define RCOL_WARPING_SHIP		2
#define RCOL_JUMP_NODE			3
#define RCOL_TAGGED				4

extern rcol Radar_color_rgb[MAX_RADAR_COLORS][MAX_RADAR_LEVELS];
extern color Radar_colors[MAX_RADAR_COLORS][MAX_RADAR_LEVELS];


#define BLIP_TYPE_JUMP_NODE			0
#define BLIP_TYPE_NAVBUOY_CARGO		1
#define BLIP_TYPE_BOMB				2
#define BLIP_TYPE_WARPING_SHIP		3
#define BLIP_TYPE_TAGGED_SHIP		4
#define BLIP_TYPE_NORMAL_SHIP		5

#define MAX_BLIP_TYPES	6

extern blip	Blip_bright_list[MAX_BLIP_TYPES];		// linked list of bright blips
extern blip	Blip_dim_list[MAX_BLIP_TYPES];			// linked list of dim blips

extern blip	Blips[MAX_BLIPS];								// blips pool
extern int	N_blips;										// next blip index to take from pool

#define BLIP_MUTATE_TIME	100

// blip flags
#define BLIP_CURRENT_TARGET	(1<<0)
#define BLIP_DRAW_DIM		(1<<1)	// object is farther than Radar_bright_range units away
#define BLIP_DRAW_DISTORTED	(1<<2)	// object is resistant to sensors, so draw distorted

struct radar_globals
{
	int Radar_radius[GR_NUM_RESOLUTIONS][2];
	float Radar_center[GR_NUM_RESOLUTIONS][2];
	int Radar_coords[GR_NUM_RESOLUTIONS][2];
	char Radar_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN];
	int Radar_blip_radius_normal[GR_NUM_RESOLUTIONS];
	int Radar_blip_radius_target[GR_NUM_RESOLUTIONS];
	int Radar_dist_coords[GR_NUM_RESOLUTIONS][RR_MAX_RANGES][2];
};

extern radar_globals Radar_globals[MAX_RADAR_MODES],*Current_radar_global;

extern float Radar_farthest_dist;
extern int Blip_mutate_id;

extern int Radar_static_playing;			// is static currently playing on the radar?
extern int Radar_static_next;				// next time to toggle static on radar
extern int Radar_avail_prev_frame;		// was radar active last frame?
extern int Radar_death_timer;				// timestamp used to play static on radar

extern hud_frames Radar_gauge;

extern float	radx, rady;
extern float	Radar_bright_range;					// range at which we start dimming the radar blips
extern int		Radar_calc_bright_dist_timer;		// timestamp at which we recalc Radar_bright_range
extern int		Radar_flicker_timer[NUM_FLICKER_TIMERS];					// timestamp used to flicker blips on and off
extern int		Radar_flicker_on[NUM_FLICKER_TIMERS];		

extern int See_all;

//function pointers to radar functions.
//i'll explain everything that needs to be done if you want to add another radar type
extern void (*radar_stuff_blip_info)(object *objp, int is_bright, color **blip_color, int *blip_type);
extern void (*radar_blip_draw_distorted)(blip *b);
extern void (*radar_blip_draw_flicker)(blip *b);
extern void (*radar_blit_gauge)();
extern void (*radar_draw_blips_sorted)(int distort);
extern void (*radar_draw_circle)( int x, int y, int rad );
extern void (*radar_draw_range)();
extern void (*radar_frame_init)();
extern void (*radar_frame_render)(float frametime);
extern void (*radar_init)();
extern void (*radar_mission_init)();
extern void (*radar_null_nblips)();
extern void (*radar_page_in)();
extern void (*radar_plot_object)( object *objp );

extern int Radar_static_looping;

//selects the radar mode so you can switch between radar modes depending on command lines/mission requirements
void select_radar_mode(int radar_mode);

#endif //_FS2OPEN_RADARSETUP_H
