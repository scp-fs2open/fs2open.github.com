#pragma once

#include "particle/EffectHost.h"
#include "math/vecmat.h"

#include <optional>

class EffectHostVector : public EffectHost {

	vec3d m_position;
	matrix m_orientation;
	vec3d m_velocity;
  	std::optional<float> m_velocityMagnitudeOverride;
	std::optional<float> m_radius;
public:
	EffectHostVector(vec3d position, matrix orientation, vec3d velocity, matrix orientationOverride = vmd_identity_matrix, bool orientationOverrideRelative = true);

	std::pair<vec3d, matrix> getPositionAndOrientation(bool relativeToParent, float interp, const std::optional<vec3d>& tabled_offset) const override;

	vec3d getVelocity() const override;

	float getVelocityMagnitude() const override;

	float getHostRadius() const override;

	void setVelocityMagnitudeOverride(float velocityMagnitudeOverride);

	void setRadius(float radius);
};