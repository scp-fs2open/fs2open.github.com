#pragma once

#include "weapon/beam.h"
#include "particle/EffectHost.h"

class EffectHostObject : public EffectHost {

	vec3d m_offset;

	int m_objnum, m_objsig;

	WeaponState m_weaponState;
public:
	EffectHostObject(const object* objp, vec3d offset, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const override;

	vec3d getVelocity() const override;

	std::pair<int, int> getParentObjAndSig() const override;

	float getHostRadius() const override;

	bool isValid() const override;

	void setupProcessing() override;
};