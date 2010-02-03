/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _PARTICLE_H
#define _PARTICLE_H

#include "globalincs/pstypes.h"

//#define MAX_PARTICLES	2000	//	Reduced from 2000 to 800 by MK on 4/1/98.  Most I ever saw was 400 and the system recovers
											//	gracefully from running out of slots.
											// AP: Put it to 1500 on 4/15/98.  Primary hit sparks weren't finding open slots.  
											// Made todo item for John to force oldest smoke particles to give up their slots.
											// 06/05/24 - taylor - made it dynamic

//============================================================================
//==================== PARTICLE SYSTEM GAME SEQUENCING CODE ==================
//============================================================================

// Resets particle system.  Call between levels.
void particle_init();

// called at game exit to cleanup any used resources
void particle_close();

// Moves the particles for each frame
void particle_move_all(float frametime);

// Renders all the particles
void particle_render_all();

// kill all active particles
void particle_kill_all();


//============================================================================
//=============== LOW-LEVEL SINGLE PARTICLE CREATION CODE ====================
//============================================================================

// The different types of particles...
#define PARTICLE_DEBUG		0			// A red sphere, no optional data required
#define PARTICLE_BITMAP		1			// A bitmap, optional data is the bitmap number.  If bitmap is an animation,
												// lifetime is calculated by the number of frames and fps.
#define PARTICLE_FIRE		2			// The vclip used for explosions, optional means nothing
#define PARTICLE_SMOKE		3			// The vclip used for smoke, optional means nothing
#define PARTICLE_SMOKE2		4			// The vclip used for smoke, optional means nothing
#define PARTICLE_BITMAP_PERSISTENT		5		// A bitmap, optional data is the bitmap number.  If bitmap is an animation,
															// lifetime is calculated by the number of frames and fps.

#define NUM_PARTICLE_TYPES	6

// particle creation stuff
typedef struct particle_info {
	// old-style particle info
	vec3d pos;
	vec3d vel;
	float lifetime;
	float rad;
	int type;
	int optional_data;

	// new-style particle info
	float tracer_length;
	int attached_objnum;			// if these are set, the pos is relative to the pos of the origin of the attached object
	int	attached_sig;				// to make sure the object hasn't changed or died. velocity is ignored in this case
	ubyte	reverse;						// play any animations in reverse
} particle_info;

// Creates a single particle. See the PARTICLE_?? defines for types.
void particle_create( particle_info *pinfo );
void particle_create( vec3d *pos, vec3d *vel, float lifetime, float rad, int type, int optional_data = -1, float tracer_length=-1.0f, struct object *objp=NULL, bool reverse=false );


//============================================================================
//============== HIGH-LEVEL PARTICLE SYSTEM CREATION CODE ====================
//============================================================================

// Use a structure rather than pass a ton of parameters to particle_emit
typedef struct particle_emitter {
	int		num_low;			// Lowest number of particles to create
	int		num_high;			// Highest number of particles to create
	vec3d	pos;				// Where the particles emit from
	vec3d	vel;				// Initial velocity of all the particles
	float	min_life;			// How long the particles live
	float	max_life;			// How long the particles live
	vec3d	normal;				// What normal the particle emit arond
	float	normal_variance;	// How close they stick to that normal 0=good, 1=360 degree
	float	min_vel;			// How fast the slowest particle can move
	float	max_vel;			// How fast the fastest particle can move
	float	min_rad;			// Min radius
	float	max_rad;			// Max radius
} particle_emitter;

// Creates a bunch of particles. You pass a structure
// rather than a bunch of parameters.
void particle_emit( particle_emitter *pe, int type, int optional_data, float range=1.0 );

#endif // _PARTICLE_H

