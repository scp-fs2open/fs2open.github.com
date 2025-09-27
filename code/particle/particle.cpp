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
#include "particle/ParticleEffect.h"
#include "debugconsole/console.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "lighting/lighting.h"
#include "math/curve.h"
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

		if (Neb_affects_particles)
			nebula_handle_alpha(alpha, pos, 
				Neb2_fog_visibility_particle_const + (rad * Neb2_fog_visibility_particle_scaled_factor));

		return alpha;
	}
}

namespace particle
{
	int Anim_bitmap_id_fire = -1;
	int Anim_num_frames_fire = -1;

	int Anim_bitmap_id_smoke = -1;
	int Anim_num_frames_smoke = -1;

	int Anim_bitmap_id_smoke2 = -1;
	int Anim_num_frames_smoke2 = -1;

	// Reset everything between levels
	void init()
	{
		if (Is_standalone)
		{
			Particles_enabled = 0;
			return;
		}

		// FIRE!!!
		if (Anim_bitmap_id_fire < 0)
		{
			Anim_bitmap_id_fire = bm_load_animation("particleexp01", &Anim_num_frames_fire, nullptr, NULL, 0);
			if (Anim_bitmap_id_fire < 0)
				Warning(LOCATION, "Could not load legacy fire particle bitmap (particleexp01)!");
		}

		// Cough, cough
		if (Anim_bitmap_id_smoke < 0)
		{
			Anim_bitmap_id_smoke = bm_load_animation("particlesmoke01", &Anim_num_frames_smoke, nullptr, NULL, 0);
			if (Anim_bitmap_id_smoke < 0)
				Warning(LOCATION, "Could not load legacy smoke particle bitmap (particlesmoke01)!");
		}

		// wheeze
		if (Anim_bitmap_id_smoke2 < 0)
		{
			Anim_bitmap_id_smoke2 = bm_load_animation("particlesmoke02", &Anim_num_frames_smoke2, nullptr, NULL, 0);
			if (Anim_bitmap_id_smoke2 < 0)
				Warning(LOCATION, "Could not load legacy smoke2 particle bitmap (particlesmoke02)!");
		}
	}

	const ParticleEffect& ParticleSubeffectHandle::getParticleEffect() const {
		//TODO possibly cache this!
		return ParticleManager::get()->getEffect(handle)[subeffect];
	}

	// only call from game_shutdown()!!!
	void close()
	{
		Persistent_particles.clear();
		Particles.clear();
	}

	size_t get_particle_count() {
		return Particles.size() + Persistent_particles.size();
	}

	void page_in()
	{
		if (!Particles_enabled)
		{
			return;
		}

		bm_page_in_texture(Anim_bitmap_id_fire);
		bm_page_in_texture(Anim_bitmap_id_smoke);
		bm_page_in_texture(Anim_bitmap_id_smoke2);

		ParticleManager::get()->pageIn();
	}

	DCF_BOOL2(particles, Particles_enabled, "Turns particles on/off",
			  "Usage: particles [bool]\nTurns particle system on/off.  If nothing passed, then toggles it.\n");

	static bool maybe_cull_particle(const particle& new_particle) {
		if (!Particles_enabled)
		{
			return true;
		}

		vec3d world_pos = new_particle.pos;
		if (new_particle.attached_objnum >= 0) {
			vm_vec_unrotate(&world_pos, &world_pos, &Objects[new_particle.attached_objnum].orient);
			world_pos += Objects[new_particle.attached_objnum].pos;
		}
		// treat particles on lower detail levels as 'further away' for the purposes of culling
		float adjusted_dist = vm_vec_dist(&Eye_position, &world_pos) * powf(2.5f, (float)(static_cast<int>(DefaultDetailPreset::Num_detail_presets) - Detail.num_particles));
		// treat bigger particles as 'closer'
		adjusted_dist /= new_particle.radius;
		float cull_start_dist = 1000.f;
		if (adjusted_dist > cull_start_dist) {
			if (frand() > 1.0f / (log2(adjusted_dist / cull_start_dist) + 1.0f))
				return true;
		}

		if (new_particle.nframes >= 0 && new_particle.bitmap < 0)
				return true;

		return false;
	}

	void create(particle&& new_particle) {
		if (maybe_cull_particle(new_particle))
			return;

		Particles.push_back(new_particle);
	}

	// Creates a single particle. See the PARTICLE_?? defines for types.
	WeakParticlePtr createPersistent(particle&& new_particle)
	{
		if (maybe_cull_particle(new_particle))
			return {};

		ParticlePtr new_particle_ptr = std::make_shared<particle>(new_particle);

		Persistent_particles.push_back(new_particle_ptr);

		return {new_particle_ptr};
	}

	/**
	 * @brief Moves a single particle
	 * @param frametime The length of the current frame
	 * @param part The particle to process for movement
	 * @return @c true if the particle has expired and should be removed, @c false otherwise
	 */
	bool move_particle(float frametime, particle* part) {
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

		const auto& source_effect = part->parent_effect.getParticleEffect();

		float part_velocity =  vm_vec_mag_quick(&part->velocity);
		float vel_scalar = source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::VELOCITY_MULT, std::forward_as_tuple(*part, part_velocity) );

		// move as a regular particle
		vec3d prev_pos = part->pos;
		part->pos += (part->velocity * vel_scalar) * frametime;

		const auto& curve_input = std::forward_as_tuple(*part, part_velocity * vel_scalar);

		if (source_effect.m_light_source) {
			const auto& light_source = *source_effect.m_light_source;

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
			
			float light_radius = light_source.light_radius * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_RADIUS_MULT, curve_input);
			float source_radius = light_source.source_radius * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_SOURCE_RADIUS_MULT, curve_input);
			float intensity = light_source.intensity * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_INTENSITY_MULT, curve_input);
			float r = light_source.r * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_R_MULT, curve_input);
			float g = light_source.g * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_G_MULT, curve_input);
			float b = light_source.b * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_B_MULT, curve_input);

			switch (light_source.light_source_mode) {
			case ParticleEffect::LightInformation::LightSourceMode::POINT:
				light_add_point(&p_pos, light_radius, light_radius, intensity, r, g, b, source_radius);
				break;
			case ParticleEffect::LightInformation::LightSourceMode::TO_LAST_POS: {
				vec3d p_prev_pos;
				if (part->attached_objnum >= 0)
				{
					vm_vec_unrotate(&p_prev_pos, &prev_pos, &Objects[part->attached_objnum].last_orient);
					vm_vec_add2(&p_prev_pos, &Objects[part->attached_objnum].last_pos);
				}
				else
				{
					p_prev_pos = prev_pos;
				}
				light_add_tube(&p_prev_pos, &p_pos, light_radius, light_radius, intensity, r, g, b, source_radius);
			}
			break;
			case ParticleEffect::LightInformation::LightSourceMode::AS_PARTICLE:
				if (part->length != 0.0f) {
					vec3d p1;
					vm_vec_copy_normalize_safe(&p1, &part->velocity);
					if (part->attached_objnum >= 0) {
						vm_vec_unrotate(&p1, &p1, &Objects[part->attached_objnum].orient);
					}
					p1 *= part->length * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LENGTH_MULT, curve_input);
					p1 += p_pos;
					light_add_tube(&p_pos, &p1, light_radius, light_radius, intensity, r, g, b, source_radius);
				}
				else {
					light_add_point(&p_pos, light_radius, light_radius, intensity, r, g, b, source_radius);
				}
				break;
			case ParticleEffect::LightInformation::LightSourceMode::CONE: {
				float cone_angle = light_source.cone_angle * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_CONE_ANGLE_MULT, curve_input);
				float cone_inner_angle = light_source.cone_inner_angle * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LIGHT_CONE_INNER_ANGLE_MULT, curve_input);
				vec3d p1;
				vm_vec_copy_normalize_safe(&p1, &part->velocity);
				if (part->attached_objnum >= 0) {
					vm_vec_unrotate(&p1, &p1, &Objects[part->attached_objnum].orient);
				}

				light_add_cone(&p_pos, &p1, cone_angle, cone_inner_angle, false, light_radius, light_radius, intensity, r, g, b, source_radius);
			}
			break;
			}
		}

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

		const auto& source_effect = part->parent_effect.getParticleEffect();

		//For anything apart from the velocity curve, "Post-Curves Velocity" is well defined. This is needed to facilitate complex but common particle scaling and appearance curves.
		const auto& curve_input = std::forward_as_tuple(*part,
			vm_vec_mag_quick(&part->velocity) * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::ANIM_STATE, std::forward_as_tuple(*part, vm_vec_mag_quick(&part->velocity))));

		// figure out which frame we should be using
		int framenum;
		int cur_frame;
		if (part->nframes > 1) {
			if (source_effect.m_lifetime_curves.has_curve(ParticleEffect::ParticleLifetimeCurvesOutput::ANIM_STATE)) {
				cur_frame = fl2i(i2fl(part->nframes - 1) * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::ANIM_STATE, curve_input));
			}
			else {
				framenum = bm_get_anim_frame(part->bitmap, part->age, part->max_life, part->looping);
				cur_frame = part->reverse ? (part->nframes - framenum - 1) : framenum;
			}
		}
		else
		{
			cur_frame = 0;
		}

		framenum = part->bitmap;

		Assert( (cur_frame < part->nframes) || (part->nframes == 0 && cur_frame == 0) );

		float radius = part->radius * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::RADIUS_MULT, curve_input);

		if (part->length != 0.0f) {
			vec3d p0 = p_pos;

			vec3d p1;
			vm_vec_copy_normalize_safe(&p1, &part->velocity);
			if (part->attached_objnum >= 0) {
				vm_vec_unrotate(&p1, &p1, &Objects[part->attached_objnum].orient);
			}
			p1 *= part->length * source_effect.m_lifetime_curves.get_output(ParticleEffect::ParticleLifetimeCurvesOutput::LENGTH_MULT, curve_input);
			p1 += p_pos;

			batching_add_laser(framenum + cur_frame, &p0, radius, &p1, radius);
		}
		else {
			// it will subtract Physics_viewer_bank, so without the flag we counter that and make it screen-aligned again
			batching_add_volume_bitmap_rotated(framenum + cur_frame, &pos, part->use_angle ? part->angle : Physics_viewer_bank, radius, alpha);
		}

		return true;
	}

	void render_all()
	{
		GR_DEBUG_SCOPE("Render Particles");
		TRACE_SCOPE(tracing::ParticlesRenderAll);

		if (!Particles_enabled)
			return;

		if (Persistent_particles.empty() && Particles.empty())
			return;

		for (auto& part : Persistent_particles) {
			render_particle(part.get());
		}

		for (auto& part : Particles) {
			render_particle(&part);
		}

	}
}
