#pragma once

#include "particle/EffectHost.h"
#include "math/vecmat.h"

class EffectHostVector : public EffectHost {

	vec3d m_position;
	matrix m_orientation;
	vec3d m_velocity;
public:
	EffectHostVector(vec3d position, matrix orientation, vec3d velocity, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const tl::optional<vec3d>& tabled_offset) override;

	vec3d getVelocity() override;

};