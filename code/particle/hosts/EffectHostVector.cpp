#include "EffectHostVector.h"

#include "math/vecmat.h"

EffectHostVector::EffectHostVector(vec3d position, matrix orientation, vec3d velocity, matrix orientationOverride, bool orientationOverrideRelative) :
	EffectHost(orientationOverride, orientationOverrideRelative), m_position(position), m_orientation(orientation), m_velocity(velocity) {}

//Vector hosts can never have a parent, so it'll always return global space
std::pair<vec3d, matrix> EffectHostVector::getPositionAndOrientation(bool /*relativeToParent*/, float /*interp*/, const tl::optional<vec3d>& tabled_offset) {
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

vec3d EffectHostVector::getVelocity() {
	return m_velocity;
}
