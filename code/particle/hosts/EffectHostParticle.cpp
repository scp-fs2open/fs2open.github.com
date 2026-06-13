#include "EffectHostParticle.h"

#include <utility>

#include "math/vecmat.h"
#include "math/curve.h"

#include "freespace.h"

#include "particle/ParticleEffect.h"

EffectHostParticle::EffectHostParticle(particle::WeakParticlePtr particle, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_particle(std::move(particle)) {}

std::pair<vec3d, matrix> EffectHostParticle::getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const {
	const auto& particle = m_particle.lock();

	vec3d pos;
	if (interp != 0.0f) {
		float vel_scalar = particle->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::VELOCITY_MULT, std::forward_as_tuple(*particle, vm_vec_mag_quick(&particle->velocity)));
		vec3d pos_last = particle->pos - (particle->velocity * vel_scalar * flFrametime);
		vm_vec_linear_interpolate(&pos, &particle->pos, &pos_last, interp);
	} else {
		pos = particle->pos;
	}

	// find the particle direction (normalized vector)
	// note: this can't be computed for particles with 0 velocity, so use the safe version
	vec3d particle_dir;
	vm_vec_copy_normalize_safe(&particle_dir, &particle->velocity);

	matrix orientation;

	if (!relativeToParent) {
		particle_dir = particle->attachment.local_vel_to_global(particle->velocity);
		vm_vec_normalize_safe(&particle_dir);
		pos = particle->attachment.local_pos_to_global(pos, interp);

		//As there's no sensible uvec in this particle orientation, relative override orientation is not that sensible. Nonetheless, allow it for compatibility, or future orientation-aware particles
		orientation = m_orientationOverrideRelative ? m_orientationOverride * *vm_vector_2_matrix(&orientation, &particle_dir) : m_orientationOverride;
	}
	else {
		particle_dir = particle->attachment.global_vel_to_local(particle->velocity);
		vm_vec_normalize_safe(&particle_dir);
		pos = particle->attachment.global_to_local(pos);

		//Since we're operating in local space, we can take the orientation override at face value if it's relative, but we need to convert it from global to local otherwise.
		orientation = m_orientationOverrideRelative ? m_orientationOverride : m_orientationOverride *  *vm_vector_2_matrix(&orientation, &particle_dir);
	}

	if (tabled_offset)
		pos += particle_dir * tabled_offset->xyz.z;

	return { pos, orientation };
}

vec3d EffectHostParticle::getVelocity() const {
	return m_particle.lock()->velocity;
}

effects::EffectAttachment EffectHostParticle::getParentAttachment() const {
	return effects::EffectAttachment(effects::attachment_particle{m_particle}).resolve_true_parent();
}

float EffectHostParticle::getLifetime() const {
	const auto& particle = m_particle.lock();
	return particle->max_life - particle->age;
}

float EffectHostParticle::getScale() const {
	const auto& particle = m_particle.lock();
	//For anything apart from the velocity curve, "Post-Curves Velocity" is well defined. This is needed to facilitate complex but common particle scaling and appearance curves.
	const auto& curve_input = std::forward_as_tuple(*particle,
		vm_vec_mag_quick(&particle->velocity) * particle->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::VELOCITY_MULT, std::forward_as_tuple(*particle, vm_vec_mag_quick(&particle->velocity))));

	return particle->radius * particle->parent_effect.getParticleEffect().m_lifetime_curves.get_output(particle::ParticleEffect::ParticleLifetimeCurvesOutput::RADIUS_MULT, curve_input);
}

bool EffectHostParticle::isValid() const {
	return !m_particle.expired();
}
