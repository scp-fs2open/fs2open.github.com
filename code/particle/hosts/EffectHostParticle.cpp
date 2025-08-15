#include "EffectHostParticle.h"

#include <utility>

#include "math/vecmat.h"
#include "math/curve.h"

#include "freespace.h"

EffectHostParticle::EffectHostParticle(particle::WeakParticlePtr particle, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_particle(std::move(particle)) {}

//Particle hosts can never have a parent, so it'll always return global space
std::pair<vec3d, matrix> EffectHostParticle::getPositionAndOrientation(bool /*relativeToParent*/, float interp, const std::optional<vec3d>& tabled_offset) const {
	const auto& particle = m_particle.lock();

	vec3d pos;
	if (interp != 0.0f) {
		float vel_scalar = 1.0f;
		if (particle->vel_lifetime_curve >= 0) {
			vel_scalar = Curves[particle->vel_lifetime_curve].GetValue(particle->age / particle->max_life);
		}
		vec3d pos_last = particle->pos - (particle->velocity * vel_scalar * flFrametime);
		vm_vec_linear_interpolate(&pos, &particle->pos, &pos_last, interp);
	} else {
		pos = particle->pos;
	}

	// find the particle direction (normalized vector)
	// note: this can't be computed for particles with 0 velocity, so use the safe version
	vec3d particle_dir;
	vm_vec_copy_normalize_safe(&particle_dir, &particle->velocity);

	if (tabled_offset)
		pos += particle_dir * tabled_offset->xyz.z;

	matrix orientation;
	//As there's no sensible uvec in this particle orientation, relative override orientation is not that sensible. Nonetheless, allow it for compatibility, or future orientation-aware particles
	orientation = m_orientationOverrideRelative ? m_orientationOverride * *vm_vector_2_matrix_norm(&orientation, &particle_dir) : m_orientationOverride;

	return { pos, orientation };
}

vec3d EffectHostParticle::getVelocity() const {
	return m_particle.lock()->velocity;
}

float EffectHostParticle::getLifetime() const {
	const auto& particle = m_particle.lock();
	return particle->max_life - particle->age;
}

float EffectHostParticle::getScale() const {
	const auto& particle = m_particle.lock();
	int idx = particle->size_lifetime_curve;
	if (idx >= 0)
		return particle->radius * Curves[idx].GetValue(particle->age / particle->max_life);
	else
		return particle->radius;
}

bool EffectHostParticle::isValid() const {
	return !m_particle.expired();
}