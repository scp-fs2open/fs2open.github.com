#include "SpheroidVolume.h"

#include "math/vecmat.h"

namespace particle {
	SpheroidVolume::SpheroidVolume() : m_bias(1.f), m_stretch(1.f), m_radius(1.f), m_modular_curve_instance(m_modular_curves.create_instance()) { };
	SpheroidVolume::SpheroidVolume(float bias, float stretch, float radius) : m_bias(bias), m_stretch(stretch), m_radius(radius), m_modular_curve_instance(m_modular_curves.create_instance()) { };

	vec3d SpheroidVolume::sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) {
		auto curveSource = std::tuple_cat(source, std::make_tuple(particlesFraction));
		vec3d pos;
		// get an unbiased random point in the sphere
		vm_vec_random_in_sphere(&pos, &vmd_zero_vector, 1.0f, false);

		// maybe bias it towards the center or edge
		float bias = m_bias * m_modular_curves.get_output(VolumeModularCurveOutput::BIAS, curveSource, &m_modular_curve_instance);
		if (!fl_equal(bias, 1.f)) {
			float mag = vm_vec_mag(&pos);

			if (fl_near_zero(mag)) {
				if (fl_near_zero(bias)) {
					//Mag and bias are zero, the point needs to be put on some point on the sphere's surface. Technically, this could be random, but as its exceedingly rare, don't bother
					pos = vec3d{{{1.f, 0.f, 0.f}}};
				}
				//else: Mag is zero, but bias is not. The point should stay at 0,0,0, so no change is necessary
			}
			else {
				pos *= powf(mag, bias) / mag;
			}
		}

		// maybe stretch it
		float stretch = m_stretch * m_modular_curves.get_output(VolumeModularCurveOutput::STRETCH, curveSource, &m_modular_curve_instance);
		if (!fl_equal(stretch, 1.f)) {
			matrix stretch_matrix = vm_stretch_matrix(&orientation.vec.fvec, stretch);
			vm_vec_rotate(&pos, &pos, &stretch_matrix);
		}

		// maybe scale it
		float radius = m_radius * m_modular_curves.get_output(VolumeModularCurveOutput::RADIUS, curveSource, &m_modular_curve_instance);
		if (!fl_equal(radius, 1.f)) {
			pos *= radius;
		}

		return pointCompensateForOffsetAndRotOffset(pos, orientation,
					m_modular_curves.get_output(VolumeModularCurveOutput::OFFSET_ROT, curveSource, &m_modular_curve_instance),
					m_modular_curves.get_output(VolumeModularCurveOutput::POINT_TO_ROT, curveSource, &m_modular_curve_instance));
	}

	void SpheroidVolume::parse() {
		if (optional_string("+Radius:")) {
			stuff_float(&m_radius);
		}
		if (optional_string("+Bias:")) {
			stuff_float(&m_bias);
		}
		if (optional_string("+Stretch:")) {
			stuff_float(&m_stretch);
		}

		ParticleVolume::parseCommon();

		m_modular_curves.parse("$Volume Curve:");
	}
}
