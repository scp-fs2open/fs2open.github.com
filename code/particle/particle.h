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
#include "object/object.h"

#include <memory>

namespace particle
{
	//============================================================================
	//==================== PARTICLE SYSTEM GAME SEQUENCING CODE ==================
	//============================================================================

	// Resets particle system.  Call between levels.
	void init();

	// called at game exit to cleanup any used resources
	void close();

	// Moves the particles for each frame
	void move_all(float frametime);

	// Renders all the particles
	void render_all();

	// kill all active particles
	void kill_all();


	//============================================================================
	//=============== LOW-LEVEL SINGLE PARTICLE CREATION CODE ====================
	//============================================================================

	/**
	 * The different types of particles
	 */
	enum ParticleType
	{
		PARTICLE_DEBUG, //!< A simple sphere; optional data provides the color which defaults to red
		PARTICLE_BITMAP, //!< A bitmap, optional data is the bitmap number.  If bitmap is an animation, lifetime is calculated by the number of frames and fps.
		PARTICLE_FIRE, //!< The vclip used for explosions, optional means nothing
		PARTICLE_SMOKE, //!< The vclip used for smoke, optional means nothing
		PARTICLE_SMOKE2, //!< The vclip used for smoke, optional means nothing
		PARTICLE_BITMAP_PERSISTENT, //!< A bitmap, optional data is the bitmap number.  If bitmap is an animation, lifetime is calculated by the number of frames and fps.

		NUM_PARTICLE_TYPES,
		INVALID_TYPE
	};

	// particle creation stuff
	typedef struct particle_info {
		// old-style particle info
		vec3d pos = vmd_zero_vector;
		vec3d vel = vmd_zero_vector;
		float lifetime = -1.0f;
		float rad = -1.0f;
		ParticleType type = INVALID_TYPE;
		int optional_data = -1;

		// new-style particle info
		int attached_objnum = -1;				// if these are set, the pos is relative to the pos of the origin of the attached object
		int attached_sig = -1;					// to make sure the object hasn't changed or died. velocity is ignored in this case
		bool reverse = false;					// play any animations in reverse
		bool lifetime_from_animation = true;	// if the particle plays an animation then use the anim length for the particle life
		float length = 0.f;						// if set, makes the particle render like a laser, oriented along its path
	} particle_info;

	typedef struct particle {
		// old style data
		vec3d	pos;				// position
		vec3d	velocity;			// velocity
		float	age;				// How long it's been alive
		float	max_life;			// How much life we had
		bool    looping;            // If the particle will loop its animation at the end of its life instead of expiring
		float	radius;				// radius
		int		type;				// type										// -1 = None
		int		optional_data;		// depends on type
		int		nframes;			// If an ani, how many frames?	

		// new style data
		int		attached_objnum;	// if this is set, pos is relative to the attached object. velocity is ignored
		int		attached_sig;		// to check for dead/nonexistent objects
		bool	reverse;			// play any animations in reverse
		int		particle_index;		// used to keep particle offset in dynamic array for orient usage
		float   length;				// the length of the particle for laser-style rendering
	} particle;

	typedef std::weak_ptr<particle> WeakParticlePtr;
	typedef std::shared_ptr<particle> ParticlePtr;

	/**
	 * @brief Creates a non-persistent particle
	 *
	 * A non-persistent particle will be managed more efficiently than a persistent particle but it will not be possible
	 * to keep a reference to the particle. This should be used whenever the a particle handle is not required (which
	 * should be the case for most usages).
	 *
	 * @param pinfo A structure containg information about how the particle should be created
	 */
	void create(particle_info* pinfo);

	/**
	 * @brief Convenience function for creating a non-persistent particle without explicitly creating a particle_info
	 * structure.
	 * @return The particle handle
	 *
	 * @see particle::create(particle_info* pinfo)
	 */
	void create(vec3d* pos,
				vec3d* vel,
				float lifetime,
				float rad,
				ParticleType type,
				int optional_data = -1,
				object* objp = NULL,
				bool reverse = false);

	/**
	 * @brief Creates a persistent particle
	 *
	 * A persistent particle is handled differently from a standard particle. It is possible to hold a weak or strong
	 * reference to a persistent particle which allows to track where the particle is and also allows to change particle
	 * properties after it has been created.
	 *
	 * @param pinfo A structure containg information about how the particle should be created
	 * @return A weak reference to the particle
	 */
    WeakParticlePtr createPersistent(particle_info* pinfo);

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
	void emit(particle_emitter *pe, ParticleType type, int optional_data, float range = 1.0);
}

#endif // _PARTICLE_H

