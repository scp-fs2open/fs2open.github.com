#include "SpheroidVolume.h"

#include "math/vecmat.h"

namespace particle {
	SpheroidVolume::SpheroidVolume(float bias, float stretch, float radius) : m_bias(bias), m_stretch(stretch), m_radius(radius) { };

	vec3d SpheroidVolume::sampleRandomPoint(const matrix &orientation) {
		vec3d pos;
		// get an unbiased random point in the sphere
		vm_vec_random_in_sphere(&pos, &vmd_zero_vector, 1.0f, false);

		// maybe bias it towards the center or edge
		if (m_bias != 1.0f) {
			float mag = vm_vec_mag(&pos);
			pos *= powf(mag, m_bias) / mag;
		}

		// maybe stretch it
		if (m_stretch != 1.0f) {
			matrix stretch_matrix = vm_stretch_matrix(&orientation.vec.fvec, m_stretch);
			vm_vec_rotate(&pos, &pos, &stretch_matrix);
		}

		// maybe scale it
		if (m_radius != 1.0f) {
			pos *= m_radius;
		}

		return pos;
	}
}
