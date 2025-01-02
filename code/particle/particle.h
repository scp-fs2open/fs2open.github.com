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

extern bool Randomize_particle_rotation;

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

	extern int Anim_bitmap_id_fire;
	extern int Anim_num_frames_fire;

	extern int Anim_bitmap_id_smoke;
	extern int Anim_num_frames_smoke;

	extern int Anim_bitmap_id_smoke2;
	extern int Anim_num_frames_smoke2;

	// particle creation stuff
	typedef struct particle_info {
		// old-style particle info
		vec3d pos = vmd_zero_vector;
		vec3d vel = vmd_zero_vector;
		float lifetime = -1.0f;
		float rad = -1.0f;
		int bitmap = -1;
		int nframes = -1;

		// new-style particle info
		int attached_objnum = -1;				// if these are set, the pos is relative to the pos of the origin of the attached object
		int attached_sig = -1;					// to make sure the object hasn't changed or died. velocity is ignored in this case
		bool reverse = false;					// play any animations in reverse
		bool lifetime_from_animation = true;	// if the particle plays an animation then use the anim length for the particle life
		float length = 0.f;						// if set, makes the particle render like a laser, oriented along its path
		bool use_angle = Randomize_particle_rotation;	// whether particles created from this will use angles (i.e. can be rotated)
														// (the field is initialized to the game_settings variable here because not all particle creation goes through ParticleProperties::createParticle)
		int size_lifetime_curve = -1;			// a curve idx for size over lifetime, if applicable
		int vel_lifetime_curve = -1;			// a curve idx for velocity over lifetime, if applicable
	} particle_info;

	typedef struct particle {
		// old style data
		vec3d	pos;				// position
		vec3d	velocity;			// velocity
		float	age;				// How long it's been alive
		float	max_life;			// How much life we had
		bool    looping;            // If the particle will loop its animation at the end of its life instead of expiring
		float	radius;				// radius
		int		bitmap;		// depends on type
		int		nframes;			// If an ani, how many frames?	

		// new style data
		int		attached_objnum;	// if this is set, pos is relative to the attached object. velocity is ignored
		int		attached_sig;		// to check for dead/nonexistent objects
		bool	reverse;			// play any animations in reverse
		float   length;				// the length of the particle for laser-style rendering
		float	angle;
		bool	use_angle;			// whether this particle can be rotated
		int		size_lifetime_curve;// a curve idx for size over lifetime, if applicable
		int		vel_lifetime_curve;		// a curve idx for velocity over lifetime, if applicable
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
	void create(const vec3d* pos,
				const vec3d* vel,
				float lifetime,
				float rad,
				int bitmap = -1,
				const object* objp = nullptr,
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
}

#endif // _PARTICLE_H

