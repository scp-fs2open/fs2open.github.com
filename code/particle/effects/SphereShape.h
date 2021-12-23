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

		vm_vec_sphere_point(&vec, m_sphereRange.next(), m_sphereRange.next());

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
