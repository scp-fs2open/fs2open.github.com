#pragma once

#include "particle/EffectHost.h"

#include "weapon/beam.h"

class EffectHostBeam : public EffectHost {

	int m_objnum, m_objsig;

	WeaponState m_weaponState;
public:
	EffectHostBeam(object* objp, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const override;

	vec3d getVelocity() const override;

	float getParticleMultiplier() const override;

	float getHostRadius() const override;

	bool isValid() const override;
};