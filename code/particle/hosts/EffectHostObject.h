#pragma once

#include "weapon/beam.h"
#include "particle/EffectHost.h"

class EffectHostObject : public EffectHost {

	vec3d m_offset;

	int m_objnum, m_objsig;

	WeaponState m_weaponState;
public:
	EffectHostObject(object* objp, vec3d offset, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) override;

	vec3d getVelocity() override;

	std::pair<int, int> getParentObjAndSig() override;

	bool isValid() override;

	void setupProcessing() override;
};