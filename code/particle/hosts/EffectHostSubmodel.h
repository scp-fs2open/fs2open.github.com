#pragma once

#include "particle/EffectHost.h"
#include "object/object.h"

class EffectHostSubmodel : public EffectHost {

	vec3d m_offset;

	int m_objnum, m_objsig, m_submodel;
public:
	EffectHostSubmodel(object* objp, int submodel, vec3d offset, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) override;

	vec3d getVelocity() override;

	std::pair<int, int> getParentObjAndSig() override;

	bool isValid() override;
};