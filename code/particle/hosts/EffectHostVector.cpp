#include "EffectHostVector.h"

#include "math/vecmat.h"

EffectHostVector::EffectHostVector(vec3d position, matrix orientation, vec3d velocity, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_position(position), m_orientation(orientation), m_velocity(velocity), m_velocityMagnitudeOverride(std::nullopt) {}

//Vector hosts can never have a parent, so it'll always return global space
std::pair<vec3d, matrix> EffectHostVector::getPositionAndOrientation(bool /*relativeToParent*/, float /*interp*/, const std::optional<vec3d>& tabled_offset) const {
	vec3d pos = m_position;
	if (tabled_offset) {
		vec3d offset;
		vm_vec_unrotate(&offset, &(*tabled_offset), &m_orientation);
		pos += offset;
	}

	//In relative mode, add the override orientation, otherwise just override
	matrix orientation = m_orientationOverrideRelative ? m_orientationOverride * m_orientation : m_orientationOverride;

	return { pos, orientation };
}

vec3d EffectHostVector::getVelocity() const {
	return m_velocity;
}

float EffectHostVector::getVelocityMagnitude() const {
	if (m_velocityMagnitudeOverride)
		return *m_velocityMagnitudeOverride;
	else
		return EffectHost::getVelocityMagnitude();
}

float EffectHostVector::getHostRadius() const {
	if (m_radius)
		return *m_radius;
	else
		return EffectHost::getHostRadius();
};

void EffectHostVector::setVelocityMagnitudeOverride(float velocityMagnitudeOverride) {
	m_velocityMagnitudeOverride.emplace(velocityMagnitudeOverride);
}

void EffectHostVector::setRadius(float radius) {
	m_radius.emplace(radius);
}
