//
//

#ifndef _SPHERE_SHAPE_H
#define _SPHERE_SHAPE_H
#pragma once

#include "globalincs/pstypes.h"

#include "math/bitarray.h"
#include "particle/ParticleEffect.h"
#include "utils/RandomRange.h"

namespace particle {
namespace effects {

/**
 * @ingroup particleEffects
 */
class SphereShape {
	::util::UniformFloatRange m_sphereRange;
 public:
	SphereShape() : m_sphereRange(0.f, 1.f) {}

	matrix getDisplacementMatrix() {
		auto u = m_sphereRange.next();
		auto v = m_sphereRange.next();

		auto theta = 2 * PI * u;
		auto phi = acos(2 * v - 1);

		vec3d vec;
		vec.xyz.x = sin(theta)*cos(phi);
		vec.xyz.y = sin(theta)*sin(phi);
		vec.xyz.z = cos(theta);

		matrix m;
		vm_vector_2_matrix_norm(&m, &vec);

		return m;
	}

	void parse(bool  /*nocreate*/) {
	}

	EffectType getType() const { return EffectType::Sphere; }

	/**
	 * @brief Specifies if the velocities of the particles should be scaled with the deviation from the direction
	 * @return @c true
	 */
	static constexpr bool scale_velocity_deviation() {
		return false;
	}
};

}
}

#endif //_SPHERE_SHAPE_H
