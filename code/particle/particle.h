/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Particle/Particle.h $
 * $Revision: 2.4 $
 * $Date: 2005-04-05 05:53:23 $
 * $Author: taylor $
 *
 * Includes for particle system
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/08/11 05:06:32  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:02:09  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2004/01/24 14:31:27  randomtiger
 * Added the D3D particle code, its not bugfree but works perfectly on my card and helps with the framerate.
 * Its optional and off by default, use -d3d_particle to activiate.
 * Also bumped up D3D ambient light setting, it was way too dark.
 * Its now set to something similar to the original game.
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 6     1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 5     1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 4     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 3     1/21/99 2:06p Dave
 * Final checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 6     5/13/98 3:25p John
 * Added code to make explosion impacts not get removed by other
 * particles.
 * 
 * 5     5/11/98 10:06a John
 * Added new particle for Adam
 * 
 * 4     4/30/98 11:31a Andsager
 * Added particles to big ship explosions.  Modified particle_emit() to
 * take optional range to increase range at which pariticles are created.
 * 
 * 3     1/29/98 11:48a John
 * Added new counter measure rendering as model code.   Made weapons be
 * able to have impact explosion.
 * 
 * 2     1/02/98 5:04p John
 * Several explosion related changes.  Made fireballs not be used as
 * ani's.  Made ship spark system expell particles.  Took away impact
 * explosion for weapon hitting ship... this needs to get added to weapon
 * info and makes shield hit more obvious.  Only make sparks when hit
 * hull, not shields.
 * 
 * 1     12/23/97 8:26a John
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _PARTICLE_H
#define _PARTICLE_H

#include "globalincs/pstypes.h"

#define MAX_PARTICLES	2000	//	Reduced from 2000 to 800 by MK on 4/1/98.  Most I ever saw was 400 and the system recovers
											//	gracefully from running out of slots.
											// AP: Put it to 1500 on 4/15/98.  Primary hit sparks weren't finding open slots.  
											// Made todo item for John to force oldest smoke particles to give up their slots.

//============================================================================
//==================== PARTICLE SYSTEM GAME SEQUENCING CODE ==================
//============================================================================

// Resets particle system.  Call between levels.
void particle_init();

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

// particle creation stuff
typedef struct particle_info {
	// old-style particle info
	vec3d pos;
	vec3d vel;
	float lifetime;
	float rad;
	int type;
	uint optional_data;	

	// new-style particle info
	float tracer_length;
	short attached_objnum;			// if these are set, the pos is relative to the pos of the origin of the attached object
	int	attached_sig;				// to make sure the object hasn't changed or died. velocity is ignored in this case
	ubyte	reverse;						// play any animations in reverse
} particle_info;

// Creates a single particle. See the PARTICLE_?? defines for types.
void particle_create( particle_info *pinfo );
void particle_create( vec3d *pos, vec3d *vel, float lifetime, float rad, int type, uint optional_data = 0 );


//============================================================================
//============== HIGH-LEVEL PARTICLE SYSTEM CREATION CODE ====================
//============================================================================

// Use a structure rather than pass a ton of parameters to particle_emit
typedef struct particle_emitter {
	int		num_low;				// Lowest number of particles to create
	int		num_high;			// Highest number of particles to create
	vec3d	pos;					// Where the particles emit from
	vec3d	vel;					// Initial velocity of all the particles
	float		min_life;			// How long the particles live
	float		max_life;			// How long the particles live
	vec3d	normal;				// What normal the particle emit arond
	float		normal_variance;	//	How close they stick to that normal 0=good, 1=360 degree
	float		min_vel;				// How fast the slowest particle can move
	float		max_vel;				// How fast the fastest particle can move
	float		min_rad;				// Min radius
	float		max_rad;				// Max radius
} particle_emitter;

// Creates a bunch of particles. You pass a structure
// rather than a bunch of parameters.
void particle_emit( particle_emitter *pe, int type, uint optional_data, float range=1.0 );

#endif // _PARTICLE_H

