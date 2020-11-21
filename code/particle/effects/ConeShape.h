#ifndef CONE_GENERATOR_EFFECT_H
#define CONE_GENERATOR_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "utils/RandomRange.h"

namespace particle {
namespace effects {

/**
 * @ingroup particleEffects
 */
class ConeShape {
	::util::NormalFloatRange m_normalDeviation;
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

	// restoreForce keeps the normalized spout vector (in local orientation) within the shape bounds
	// Cone doesnt do much unless its near the edge of the cone, where it nudges it back
	void restoreForce(vec3d* spout, float spout_speed) {
		if (m_normalDeviation.max() == 0.0f)
			*spout = vmd_z_vector;
		else {
			float deviation = vm_vec_delta_ang(&vmd_z_vector, spout, nullptr);
			vm_vec_scale_add(spout, spout, &vmd_z_vector, (spout_speed / sinf(m_normalDeviation.max()) * (deviation / m_normalDeviation.max())));
			vm_vec_normalize(spout);
		}
	}

	void parse(bool nocreate) {
		if (internal::required_string_if_new("+Deviation:", nocreate)) {
			float deviation;
			stuff_float(&deviation);

			if (deviation < 0.001f) {
				error_display(0, "A standard deviation of %f is not valid. Must be greater than 0. Defaulting to 1.",
							  deviation);
				deviation = 1.0f;
			}

			m_normalDeviation = ::util::NormalFloatRange(0.0, fl_radians(deviation));
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
