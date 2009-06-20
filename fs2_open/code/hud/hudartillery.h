/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _FS2_HUD_ARTILLERY_HEADER_FILE
#define _FS2_HUD_ARTILLERY_HEADER_FILE

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"

// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY DEFINES/VARS
//
#define MAX_SSM_TYPES			10
#define MAX_SSM_STRIKES			10
#define MAX_SSM_COUNT			10

// global ssm types
typedef struct ssm_info {
	char			name[NAME_LENGTH];				// strike name
	int			count;								// # of missiles in this type of strike
	int			weapon_info_index;				// missile type
	float			warp_radius;						// radius of associated warp effect	
	float			warp_time;							// how long the warp effect lasts
	float			radius;								// radius around the shooting ship	
	float			offset;								// offset in front of the shooting ship
} ssm_info;

// creation info for the strike (useful for multiplayer)
typedef struct ssm_firing_info {
	int     delay_stamp[MAX_SSM_COUNT];	    // timestamps
	vec3d   start_pos[MAX_SSM_COUNT];       // start positions
	
	int             ssm_index;							// index info ssm_info array
	struct object*  target;								// target for the strike
    int             ssm_team;                           // team that fired the ssm.
} ssm_firing_info;

// the strike itself
typedef struct ssm_strike {
	int			fireballs[MAX_SSM_COUNT];		// warpin effect fireballs
	int			done_flags[MAX_SSM_COUNT];		// when we've fired off the individual missiles
	
	// this is the info that controls how the strike behaves (just like for beam weapons)
	ssm_firing_info		sinfo;

	ssm_strike	*next, *prev;						// for list
} ssm_strike;


extern int Ssm_info_count;
extern ssm_info Ssm_info[MAX_SSM_TYPES];


// -----------------------------------------------------------------------------------------------------------------------
// ARTILLERY FUNCTIONS
//

// level init
void hud_init_artillery();

// update all hud artillery related stuff
void hud_artillery_update();

// render all hud artillery related stuff
void hud_artillery_render();

// start a subspace missile effect
void ssm_create(object *target, vec3d *start, int ssm_index, ssm_firing_info *override, int team);

// Goober5000
extern int ssm_info_lookup(char *name);

#endif
