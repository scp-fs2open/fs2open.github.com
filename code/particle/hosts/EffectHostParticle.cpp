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

	vec3d particle_dir = particle->attachment.local_vel_to_global(particle->velocity);
	vm_vec_normalize_safe(&particle_dir);

	matrix orientation;
	pos = particle->attachment.local_pos_to_global(pos, interp);

	if (!relativeToParent) {
		orientation = m_orientationOverrideRelative ? m_orientationOverride * *vm_vector_2_matrix(&orientation, &particle_dir) : m_orientationOverride;
	} else {
		const auto& parent = getParentAttachment();
		pos = parent.global_pos_to_local(pos);

		const auto& [parent_pos, parent_orient] = parent.get_frame(interp);
		vm_vec_rotate(&particle_dir, &particle_dir, &parent_orient);

		if (m_orientationOverrideRelative) {
			orientation = m_orientationOverride * *vm_vector_2_matrix(&orientation, &particle_dir);
		} else {
			matrix parent_orient_transpose;
			orientation = m_orientationOverride * *vm_copy_transpose(&parent_orient_transpose, &parent_orient);
		}
	}

	if (tabled_offset)
		pos += particle_dir * tabled_offset->xyz.z;

	return { pos, orientation };
}

vec3d EffectHostParticle::getVelocity() const {
	return m_particle.lock()->velocity;
}

effects::EffectAttachment EffectHostParticle::getParentAttachment() const {
	Assertion(isValid(), "Tried to query particle attachment on an invalid particle");
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
