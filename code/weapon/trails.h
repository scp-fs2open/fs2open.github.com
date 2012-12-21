/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _TRAILS_H
#define _TRAILS_H

#include "globalincs/pstypes.h"
#include "graphics/generic.h"

#define NUM_TRAIL_SECTIONS 128

// contrail info - similar to that for missile trails
// place this inside of info structures instead of explicit structs (eg. ship_info instead of ship, or weapon_info instead of weapon)
typedef struct trail_info {
	vec3d pt;				// offset from the object's center
	float w_start;			// starting width
	float w_end;			// ending width
	float a_start;			// starting alpha
	float a_end;			// ending alpha
	float max_life;		// max_life for a section
	int stamp;				// spew timestamp
	generic_bitmap texture;	// texture to use for trail
	int n_fade_out_sections;// number of initial sections used for fading out start 'edge' of the effect
} trail_info;

typedef struct trail {
	int		head, tail;						// pointers into the queue for the trail points
	vec3d	pos[NUM_TRAIL_SECTIONS];	// positions of trail points
	float	val[NUM_TRAIL_SECTIONS];	// for each point, a value that tells how much to fade out	
	bool	object_died;					// set to zero as long as object	
	int		trail_stamp;					// trail timestamp	

	// trail info
	trail_info info;							// this is passed when creating a trail

	struct	trail * next;

} trail;

// Call at the start of freespace to init trails

// Call at start of level to reinit all missilie trail stuff
void trail_level_init();

void trail_level_close();

// Needs to be called from somewhere to move the trails each frame
void trail_move_all(float frametime);

// Needs to be called from somewhere to render the trails each frame
void trail_render_all();

// The following functions are what the weapon code calls
// to deal with trails:

// Returns -1 if failed
trail *trail_create(trail_info *info);
void trail_add_segment( trail *trailp, vec3d *pos );
void trail_set_segment( trail *trailp, vec3d *pos );
void trail_object_died( trail *trailp );
int trail_stamp_elapsed( trail *trailp );
void trail_set_stamp( trail *trailp );

#endif //_TRAILS_H
