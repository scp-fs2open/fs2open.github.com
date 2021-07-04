/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "bmpman/bmpman.h"
#include "particle/particle.h"
#include "particle/ParticleManager.h"
#include "debugconsole/console.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "render/batching.h"
#include "tracing/tracing.h"
#include "tracing/Monitor.h"
#include "utils/Random.h"
#include "nebula/neb.h"
#include "mission/missionparse.h"
#include "mod_table/mod_table.h"

using namespace particle;

namespace
{
	SCP_vector<::particle::particle> Particles;
	SCP_vector<ParticlePtr> Persistent_particles;

	int Anim_bitmap_id_fire = -1;
	int Anim_num_frames_fire = -1;

	int Anim_bitmap_id_smoke = -1;
	int Anim_num_frames_smoke = -1;

	int Anim_bitmap_id_smoke2 = -1;
	int Anim_num_frames_smoke2 = -1;

	static int Particles_enabled = 1;

	float get_current_alpha(vec3d* pos, float rad)
	{
		float dist;
		float alpha = 0.99999f;

		const float inner_radius = MIN(30.0f, rad);
		const float magic_num = MIN(2.75f, rad / 10.0f);

		// determine what alpha to draw this bitmap with
		// higher alpha the closer the bitmap gets to the eye
		dist = vm_vec_dist_quick(&Eye_position, pos);

		// if the point is inside the inner radius, alpha is based on distance to the player's eye,
		// becoming more transparent as it gets close
		if (dist <= inner_radius)
		{
			// alpha per meter between the magic # and the inner radius
			alpha /= (inner_radius - magic_num);

			// above value times the # of meters away we are
			alpha *= (dist - magic_num);
			if (alpha < 0.05f)
				return 0.0f;
		}

		if (The_mission.flags[Mission::Mission_Flags::Fullneb] && Neb_affects_particles)
			alpha *= neb2_get_fog_visibility(pos, NEB_FOG_VISIBILITY_MULT_PARTICLE(rad));

		return alpha;
	}

	inline int get_percent(int count)
	{
		if (count == 0)
			return 0;

		// this should basically return a scale like:
		//  50, 75, 100, 125, 150, ...
		// based on value of 'count' (detail level)
		return (50 + (25 * (count - 1)));
	}
}

namespace particle
{
	// Reset everything between levels
	void init()
	{
		// FIRE!!!
		if (Anim_bitmap_id_fire == -1)
		{
			Anim_bitmap_id_fire = bm_load_animation("particleexp01", &Anim_num_frames_fire, nullptr, NULL, 0);
		}

		// Cough, cough
		if (Anim_bitmap_id_smoke == -1)
		{
			Anim_bitmap_id_smoke = bm_load_animation("particlesmoke01", &Anim_num_frames_smoke, nullptr, NULL, 0);
		}

		// wheeze
		if (Anim_bitmap_id_smoke2 == -1)
		{
			Anim_bitmap_id_smoke2 = bm_load_animation("particlesmoke02", &Anim_num_frames_smoke2, nullptr, NULL, 0);
		}
	}

	// only call from game_shutdown()!!!
	void close()
	{
		Persistent_particles.clear();
		Particles.clear();
	}

	void page_in()
	{
		bm_page_in_texture(Anim_bitmap_id_fire);
		bm_page_in_texture(Anim_bitmap_id_smoke);
		bm_page_in_texture(Anim_bitmap_id_smoke2);

		ParticleManager::get()->pageIn();
	}

	DCF_BOOL2(particles, Particles_enabled, "Turns particles on/off",
			  "Usage: particles [bool]\nTurns particle system on/off.  If nothing passed, then toggles it.\n");

	bool init_particle(particle* part, particle_info* info) {
		if (!Particles_enabled)
		{
			return false;
		}

		int fps = 1;

		part->pos = info->pos;
		part->velocity = info->vel;
		part->age = 0.0f;
		part->max_life = info->lifetime;
		part->radius = info->rad;
		part->type = info->type;
		part->optional_data = info->optional_data;
		part->attached_objnum = info->attached_objnum;
		part->attached_sig = info->attached_sig;
		part->reverse = info->reverse;
		part->particle_index = (int) Persistent_particles.size();
		part->looping = false;
		part->length = info->length;

		switch (info->type)
		{
		case PARTICLE_BITMAP:
		case PARTICLE_BITMAP_PERSISTENT:
		{
			Assertion(bm_is_valid(info->optional_data), "Invalid bitmap handle passed to particle create.");

			bm_get_info(info->optional_data, NULL, NULL, NULL, &part->nframes, &fps);

			if (part->nframes > 1 && info->lifetime_from_animation)
			{
				// Recalculate max life for ani's
				part->max_life = i2fl(part->nframes) / i2fl(fps);
			}

			break;
		}

		case PARTICLE_FIRE:
		{
			if (Anim_bitmap_id_fire < 0)
			{
				return false;
			}

			part->optional_data = Anim_bitmap_id_fire;
			part->nframes = Anim_num_frames_fire;

			break;
		}

		case PARTICLE_SMOKE:
		{
			if (Anim_bitmap_id_smoke < 0)
			{
				return false;
			}

			part->optional_data = Anim_bitmap_id_smoke;
			part->nframes = Anim_num_frames_smoke;

			break;
		}

		case PARTICLE_SMOKE2:
		{
			if (Anim_bitmap_id_smoke2 < 0)
			{
				return false;
			}

			part->optional_data = Anim_bitmap_id_smoke2;
			part->nframes = Anim_num_frames_smoke2;

			break;
		}

		default:
			part->nframes = 1;
			break;
		}

		return true;
	}

	void create(particle_info* pinfo) {
		particle part;
		if (!init_particle(&part, pinfo)) {
			return;
		}

		Particles.push_back(part);
	}

	// Creates a single particle. See the PARTICLE_?? defines for types.
	WeakParticlePtr createPersistent(particle_info* pinfo)
	{
		ParticlePtr new_particle = std::make_shared<particle>();

		if (!init_particle(new_particle.get(), pinfo)) {
			return WeakParticlePtr();
		}

		Persistent_particles.push_back(new_particle);

		return WeakParticlePtr(new_particle);
	}

	void create(vec3d* pos,
				vec3d* vel,
				float lifetime,
				float rad,
				ParticleType type,
				int optional_data,
				object* objp,
				bool reverse) {
		particle_info pinfo;

		Assertion((type >= 0) && (type < NUM_PARTICLE_TYPES), "Invalid particle type %d specified!", type);

		// setup old data
		pinfo.pos = *pos;
		pinfo.vel = *vel;
		pinfo.lifetime = lifetime;
		pinfo.rad = rad;
		pinfo.type = type;
		pinfo.optional_data = optional_data;

		// setup new data
		if (objp == NULL) {
			pinfo.attached_objnum = -1;
			pinfo.attached_sig = -1;
		} else {
			pinfo.attached_objnum = OBJ_INDEX(objp);
			pinfo.attached_sig = objp->signature;
		}
		pinfo.reverse = reverse;

		// lower level function
		create(&pinfo);
	}

	/**
	 * @brief Moves a single particle
	 * @param frametime The length of the current frame
	 * @param part The particle to process for movement
	 * @return @c true if the particle has expired and should be removed, @c false otherwise
	 */
	static bool move_particle(float frametime, particle* part) {
		if (part->age == 0.0f)
		{
			part->age = 0.00001f;
		}
		else
		{
			part->age += frametime;
		}

		bool remove_particle = false;

		// if its time expired, remove it. If the particle is looping then it will never be removed due to age
		if (part->age > part->max_life && !part->looping)
		{
			// special case, if max_life is 0 then we want it to render at least once
			if ((part->age > frametime) || (part->max_life > 0.0f))
			{
				remove_particle = true;
			}
		}

		// if the particle is attached to an object which has become invalid, kill it
		if (part->attached_objnum >= 0)
		{
			// if the signature has changed, or it's bogus, kill it
			if ((part->attached_objnum >= MAX_OBJECTS) ||
				(part->attached_sig != Objects[part->attached_objnum].signature))
			{
				remove_particle = true;
			}
		}

		if (remove_particle)
		{
			return true;
		}

		// move as a regular particle
		vm_vec_scale_add2(&part->pos, &part->velocity, frametime);

		return false;
	}

	void move_all(float frametime)
	{
		TRACE_SCOPE(tracing::ParticlesMoveAll);

		if (!Particles_enabled)
			return;

		if (Persistent_particles.empty() && Particles.empty())
			return;

		for (auto p = Persistent_particles.begin(); p != Persistent_particles.end();)
		{
			ParticlePtr part = *p;
			if (move_particle(frametime, part.get()))
			{
				// if we're sitting on the very last particle, popping-back will invalidate the iterator!
				if (p + 1 == Persistent_particles.end())
				{
					Persistent_particles.pop_back();
					break;
				}

				*p = Persistent_particles.back();
				Persistent_particles.pop_back();
				continue;
			}

			// next particle
			++p;
		}

		for (auto p = Particles.begin(); p != Particles.end();)
		{
			if (move_particle(frametime, &(*p)))
			{
				// if we're sitting on the very last particle, popping-back will invalidate the iterator!
				if (p + 1 == Particles.end())
				{
					Particles.pop_back();
					break;
				}

				*p = Particles.back();
				Particles.pop_back();
				continue;
			}

			// next particle
			++p;
		}
	}

	// kill all active particles
	void kill_all()
	{
		// kill all active particles
		Particles.clear();
		Persistent_particles.clear();
	}

	/**
	 * @brief Renders a single particle
	 * @param part The particle to render
	 * @return @c true if the particle has been added to the rendering batch, @c false otherwise
	 */
	static bool render_particle(particle* part) {
		// skip back-facing particles (ripped from fullneb code)
		// Wanderer - add support for attached particles
		vec3d p_pos;
		if (part->attached_objnum >= 0)
		{
			vm_vec_unrotate(&p_pos, &part->pos, &Objects[part->attached_objnum].orient);
			vm_vec_add2(&p_pos, &Objects[part->attached_objnum].pos);
		}
		else
		{
			p_pos = part->pos;
		}

		if (vm_vec_dot_to_point(&Eye_matrix.vec.fvec, &Eye_position, &p_pos) <= 0.0f)
		{
			return false;
		}

		// calculate the alpha to draw at
		auto alpha = get_current_alpha(&p_pos, part->radius);

		// if it's transparent then just skip it
		if (alpha <= 0.0f)
		{
			return false;
		}

		vertex pos;
		auto flags = g3_rotate_vertex(&pos, &p_pos);

		if (flags)
		{
			return false;
		}

		g3_transfer_vertex(&pos, &p_pos);

		// figure out which frame we should be using
		int framenum;
		int cur_frame;
		if (part->nframes > 1) {
			framenum = bm_get_anim_frame(part->optional_data, part->age, part->max_life, part->looping);
			cur_frame = part->reverse ? (part->nframes - framenum - 1) : framenum;
		}
		else
		{
			cur_frame = 0;
		}

		if (part->type == PARTICLE_DEBUG)
		{
			gr_set_color(255, 0, 0);
			g3_draw_sphere_ez(&p_pos, part->radius);
		}
		else
		{
			framenum = part->optional_data;

			Assert( cur_frame < part->nframes );

			if (part->length != 0.0f) {
				vec3d p0 = part->pos;

				vec3d p1;
				vm_vec_copy_normalize(&p1, &part->velocity);
				p1 *= part->length;
				p1 += part->pos;

				batching_add_laser(framenum + cur_frame, &p0, part->radius, &p1, part->radius);
			}
			else {
				batching_add_volume_bitmap(framenum + cur_frame, &pos, part->particle_index % 8, part->radius, alpha);
			}


			return true;
		}

		return false;
	}

	void render_all()
	{
		GR_DEBUG_SCOPE("Render Particles");
		TRACE_SCOPE(tracing::ParticlesRenderAll);

		bool render_batch = false;

		if (!Particles_enabled)
			return;

		if (Persistent_particles.empty() && Particles.empty())
			return;

		for (auto& part : Persistent_particles) {
			if (render_particle(part.get())) {
				render_batch = true;
			}
		}

		for (auto& part : Particles) {
			if (render_particle(&part)) {
				render_batch = true;
			}
		}

		if (render_batch)
		{
			batching_render_all();
		}
	}

	//============================================================================
	//============== HIGH-LEVEL PARTICLE SYSTEM CREATION CODE ====================
	//============================================================================

	// Use a structure rather than pass a ton of parameters to particle_emit
	/*
    typedef struct particle_emitter {
    int		num_low;			// Lowest number of particles to create
    int		num_high;			// Highest number of particles to create
    vec3d	pos;				// Where the particles emit from
    vec3d	vel;				// Initial velocity of all the particles
    float	lifetime;			// How long the particles live
    vec3d	normal;				// What normal the particle emit arond
    float	normal_variance;	// How close they stick to that normal 0=good, 1=360 degree
    float	min_vel;			// How fast the slowest particle can move
    float	max_vel;			// How fast the fastest particle can move
    float	min_rad;			// Min radius
    float	max_rad;			// Max radius
    } particle_emitter;
    */

	// Creates a bunch of particles. You pass a structure
	// rather than a bunch of parameters.
	void emit(particle_emitter* pe, ParticleType type, int optional_data, float range)
	{
		int i, n;

		if (!Particles_enabled)
			return;

		int n1, n2;

		// Account for detail
		int percent = get_percent(Detail.num_particles);

		//Particle rendering drops out too soon.  Seems to be around 150 m.  Is it detail level controllable?  I'd like it to be 500-1000
		float min_dist = 125.0f;
		float dist = vm_vec_dist_quick(&pe->pos, &Eye_position) / range;
		if (dist > min_dist)
		{
			percent = fl2i(i2fl(percent) * min_dist / dist);
			if (percent < 1)
			{
				return;
			}
		}
		//mprintf(( "Dist = %.1f, percent = %d%%\n", dist, percent ));

		n1 = (pe->num_low * percent) / 100;
		n2 = (pe->num_high * percent) / 100;

		// How many to emit?
		n = Random::next(n1, n2);

		if (n < 1) return;


		for (i = 0; i < n; i++)
		{
			// Create a particle
			vec3d tmp_vel;
			vec3d normal;                // What normal the particle emit arond

			float radius = ((pe->max_rad - pe->min_rad) * frand()) + pe->min_rad;

			float speed = ((pe->max_vel - pe->min_vel) * frand()) + pe->min_vel;

			float life = ((pe->max_life - pe->min_life) * frand()) + pe->min_life;

			normal.xyz.x = pe->normal.xyz.x + (frand() * 2.0f - 1.0f) * pe->normal_variance;
			normal.xyz.y = pe->normal.xyz.y + (frand() * 2.0f - 1.0f) * pe->normal_variance;
			normal.xyz.z = pe->normal.xyz.z + (frand() * 2.0f - 1.0f) * pe->normal_variance;
			vm_vec_normalize_safe(&normal);
			vm_vec_scale_add(&tmp_vel, &pe->vel, &normal, speed);

			create(&pe->pos, &tmp_vel, life, radius, type, optional_data);
		}
	}
}
