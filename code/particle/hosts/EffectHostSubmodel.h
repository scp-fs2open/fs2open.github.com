#pragma once

#include "particle/EffectHost.h"

class EffectHostSubmodel : public EffectHost {

	vec3d m_offset;

	int m_objnum, m_objsig, m_submodel;
public:

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) override;

	vec3d getVelocity() override;

	std::pair<int, int> getParentObjAndSig() override;

	bool isValid() override;
};