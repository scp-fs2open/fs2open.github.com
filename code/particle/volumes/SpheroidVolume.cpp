#include "SpheroidVolume.h"

#include "math/vecmat.h"

namespace particle {
	SpheroidVolume::SpheroidVolume(float bias, float stretch, float radius) : m_bias(bias), m_stretch(stretch), m_radius(radius), m_modular_curve_instance(m_modular_curves.create_instance()) { };

	vec3d SpheroidVolume::sampleRandomPoint(const matrix &orientation, const std::tuple<const ParticleSource&, const size_t&>& source) {
		vec3d pos;
		// get an unbiased random point in the sphere
		vm_vec_random_in_sphere(&pos, &vmd_zero_vector, 1.0f, false);

		// maybe bias it towards the center or edge
		float bias = m_bias * m_modular_curves.get_output(VolumeModularCurveOutput::BIAS, source, &m_modular_curve_instance);
		if (!fl_equal(bias, 1.f)) {
			float mag = vm_vec_mag(&pos);
			pos *= powf(mag, bias) / mag;
		}

		// maybe stretch it
		float stretch = m_stretch * m_modular_curves.get_output(VolumeModularCurveOutput::STRETCH, source, &m_modular_curve_instance);
		if (!fl_equal(stretch, 1.f)) {
			matrix stretch_matrix = vm_stretch_matrix(&orientation.vec.fvec, stretch);
			vm_vec_rotate(&pos, &pos, &stretch_matrix);
		}

		// maybe scale it
		float radius = m_radius * m_modular_curves.get_output(VolumeModularCurveOutput::RADIUS, source, &m_modular_curve_instance);
		if (!fl_equal(radius, 1.f)) {
			pos *= radius;
		}

		return pos;
	}
}
