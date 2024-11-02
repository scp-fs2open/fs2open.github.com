#pragma once

#include "particle/EffectHost.h"

#include "particle/particle.h"

class EffectHostParticle : public EffectHost {

	particle::WeakParticlePtr m_particle;
public:
	EffectHostParticle(particle::WeakParticlePtr particle, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) override;

	vec3d getVelocity() override;

	float getLifetime() override;

	float getScale() override;

	bool isValid() override;
};