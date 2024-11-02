#pragma once

#include "particle/EffectHost.h"
#include "object/object.h"

class EffectHostTurret : public EffectHost {
	int m_objnum, m_objsig, m_submodel, m_fire_pnt;

public:
	EffectHostTurret(object* objp, int submodel, int fire_pnt, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) override;

	vec3d getVelocity() override;

	std::pair<int, int> getParentObjAndSig() override;

	bool isValid() override;
};