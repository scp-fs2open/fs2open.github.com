#pragma once

#include "particle/EffectHost.h"

#include "particle/particle.h"

class EffectHostParticle : public EffectHost {

	particle::WeakParticlePtr m_particle;
public:
	EffectHostParticle(particle::WeakParticlePtr particle, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const override;

	vec3d getVelocity() const override;

	float getLifetime() const override;

	float getScale() const override;

	float getHostRadius() const override { return getScale(); };

	bool isValid() const override;
};