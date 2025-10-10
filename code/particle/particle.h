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

	class ParticleEffect;
	struct particle_effect_tag {
	};
	using ParticleEffectHandle = ::util::ID<particle_effect_tag, ptrdiff_t, -1>;

	struct ParticleSubeffectHandle {
		ParticleEffectHandle handle;
		size_t subeffect = 0;

		const ParticleEffect& getParticleEffect() const;
	};

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

	size_t get_particle_count();


	//============================================================================
	//=============== LOW-LEVEL SINGLE PARTICLE CREATION CODE ====================
	//============================================================================

	extern int Anim_bitmap_id_fire;
	extern int Anim_num_frames_fire;

	extern int Anim_bitmap_id_smoke;
	extern int Anim_num_frames_smoke;

	extern int Anim_bitmap_id_smoke2;
	extern int Anim_num_frames_smoke2;

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

		ParticleSubeffectHandle parent_effect;
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
	void create(particle&& new_particle);

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
	WeakParticlePtr createPersistent(particle&& new_particle);
}

#endif // _PARTICLE_H

