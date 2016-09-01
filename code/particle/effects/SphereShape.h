//
//

#ifndef _SPHERE_SHAPE_H
#define _SPHERE_SHAPE_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/util/RandomRange.h"

namespace particle {
namespace effects {

/**
 * @ingroup particleEffects
 */
class SphereShape {
	util::UniformFloatRange m_sphereRange;
 public:
	SphereShape() : m_sphereRange(0.00001f, 0.9999999f) {}

	matrix getDisplacementMatrix() {
		auto u = m_sphereRange.next();
		auto v = m_sphereRange.next();

		auto theta = 2 * PI * u;
		auto phi = acos(2 * v - 1);

		angles angs;

		angs.b = 0.0f;

		angs.h = theta;
		angs.p = phi;

		matrix m;

		vm_angles_2_matrix(&m, &angs);

		return m;
	}

	void parse(bool nocreate) {
	}

	EffectType getType() const { return EffectType::Sphere; }
};

}
}

#endif //_SPHERE_SHAPE_H
