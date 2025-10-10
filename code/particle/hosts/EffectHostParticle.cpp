#include "EffectHostParticle.h"

#include <utility>

#include "math/vecmat.h"
#include "math/curve.h"

#include "freespace.h"

#include "particle/ParticleEffect.h"

EffectHostParticle::EffectHostParticle(particle::WeakParticlePtr particle, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_particle(std::move(particle)) {}

//Particle hosts can never have a parent, so it'll always return global space
std::pair<vec3d, matrix> EffectHostParticle::getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const {
	const auto& particle = m_particle.lock();

	relativeToParent &= particle->attached_objnum >= 0;

	vec3d pos;
	if (interp != 0.0f) {
		float vel_scalar = particle->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::VELOCITY_MULT, std::forward_as_tuple(*particle, vm_vec_mag_quick(&particle->velocity)));
		vec3d pos_last = particle->pos - (particle->velocity * vel_scalar * flFrametime);
		vm_vec_linear_interpolate(&pos, &particle->pos, &pos_last, interp);
	} else {
		pos = particle->pos;
	}

	//We might need to convert the position to global space if the parent particle has a parent
	if (particle->attached_objnum >= 0) {
		vec3d global_pos;
		vm_vec_linear_interpolate(&global_pos, &Objects[particle->attached_objnum].pos, &Objects[particle->attached_objnum].last_pos, interp);

		vm_vec_unrotate(&pos, &pos, &Objects[particle->attached_objnum].orient);
		pos += global_pos;
	}

	// find the particle direction (normalized vector)
	// note: this can't be computed for particles with 0 velocity, so use the safe version
	vec3d particle_dir;
	vm_vec_copy_normalize_safe(&particle_dir, &particle->velocity);

	if (tabled_offset)
		pos += particle_dir * tabled_offset->xyz.z;

	matrix orientation;

	if (!relativeToParent) {
		//As there's no sensible uvec in this particle orientation, relative override orientation is not that sensible. Nonetheless, allow it for compatibility, or future orientation-aware particles
		orientation = m_orientationOverrideRelative ? m_orientationOverride * *vm_vector_2_matrix_norm(&orientation, &particle_dir) : m_orientationOverride;
	}
	else {
		//The position data here is in world space
		//Since we're operating in local space, we can take the orientation override at face value if it's relative, but we need to convert it from global to local otherwise.
		matrix global_orient_transpose;
		orientation = m_orientationOverrideRelative ? m_orientationOverride : m_orientationOverride * *vm_copy_transpose(&global_orient_transpose, &Objects[particle->attached_objnum].orient);

		vm_vec_sub2(&pos, &Objects[particle->attached_objnum].pos);
		vm_vec_rotate(&pos, &pos, &Objects[particle->attached_objnum].orient);
	}

	return { pos, orientation };
}

vec3d EffectHostParticle::getVelocity() const {
	return m_particle.lock()->velocity;
}

std::pair<int, int> EffectHostParticle::getParentObjAndSig() const {
	const auto& particle = m_particle.lock();
	return {particle->attached_objnum, particle->attached_sig};
}

float EffectHostParticle::getLifetime() const {
	const auto& particle = m_particle.lock();
	return particle->max_life - particle->age;
}

float EffectHostParticle::getScale() const {
	const auto& particle = m_particle.lock();
	//For anything apart from the velocity curve, "Post-Curves Velocity" is well defined. This is needed to facilitate complex but common particle scaling and appearance curves.
	const auto& curve_input = std::forward_as_tuple(*particle,
		vm_vec_mag_quick(&particle->velocity) * particle->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::ANIM_STATE, std::forward_as_tuple(*particle, vm_vec_mag_quick(&particle->velocity))));

	return particle->radius * particle->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::RADIUS_MULT, curve_input);
}

bool EffectHostParticle::isValid() const {
	return !m_particle.expired();
}