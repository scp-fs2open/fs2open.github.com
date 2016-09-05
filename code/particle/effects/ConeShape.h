#ifndef CONE_GENERATOR_EFFECT_H
#define CONE_GENERATOR_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/util/RandomRange.h"

namespace particle {
namespace effects {

/**
 * @ingroup particleEffects
 */
class ConeShape {
	util::NormalFloatRange m_normalDeviation;
 public:
	ConeShape() {}

	matrix getDisplacementMatrix() {
		angles angs;

		angs.b = 0.0f;

		angs.h = m_normalDeviation.next();
		angs.p = m_normalDeviation.next();

		matrix m;

		vm_angles_2_matrix(&m, &angs);

		return m;
	}

	void parse(bool nocreate) {
		if (internal::required_string_if_new("+Deviation:", nocreate)) {
			float deviation;
			stuff_float(&deviation);

			m_normalDeviation = util::NormalFloatRange(0.0, fl_radians(deviation));
		}
	}

	EffectType getType() const { return EffectType::Cone; }

	/**
	 * @brief Specifies if the velocities of the particles should be scaled with the deviation from the direction
	 * @return @c true
	 */
	static SCP_CONSTEXPR bool scale_velocity_deviation() {
		return true;
	}
};

}
}

#endif // CONE_GENERATOR_EFFECT_H
