//
//

#ifndef _SPHERE_SHAPE_H
#define _SPHERE_SHAPE_H
#pragma once

#include "globalincs/pstypes.h"

#include "math/bitarray.h"
#include "math/vecmat.h"
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

		vec3d vec;
		auto theta_scalar = m_sphereRange.next();
		auto phi_scalar = m_sphereRange.next();
		while (phi_scalar == 1.0f) {
			phi_scalar = m_sphereRange.next();
		}

		vm_vec_unit_sphere_point(&vec, theta_scalar, phi_scalar);

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
