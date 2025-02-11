#pragma once

#include "particle/EffectHost.h"
#include "object/object.h"

class EffectHostTurret : public EffectHost {
	int m_objnum, m_objsig, m_submodel, m_fire_pnt;

public:
	EffectHostTurret(object* objp, int submodel, int fire_pnt, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const override;

	vec3d getVelocity() const override;

	std::pair<int, int> getParentObjAndSig() const override;

	float getHostRadius() const override;

	bool isValid() const override;
};