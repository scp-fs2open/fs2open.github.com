#include "RingVolume.h"

#include "math/vecmat.h"

namespace particle {
	RingVolume::RingVolume() : m_radius(1.f), m_onEdge(false), m_modular_curve_instance(m_modular_curves.create_instance()) { };
	RingVolume::RingVolume(float radius, bool onEdge) : m_radius(radius), m_onEdge(onEdge), m_modular_curve_instance(m_modular_curves.create_instance()) { };

	vec3d RingVolume::sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) {
		auto curveSource = std::tuple_cat(source, std::make_tuple(particlesFraction));
		vec3d pos;
		// get an unbiased random point in the sphere
		vm_vec_random_in_circle(&pos, &vmd_zero_vector, &orientation, m_radius * m_modular_curves.get_output(VolumeModularCurveOutput::RADIUS, curveSource, &m_modular_curve_instance), false);

		return pointCompensateForOffsetAndRotOffset(pos, orientation,
					m_modular_curves.get_output(VolumeModularCurveOutput::OFFSET_ROT, curveSource, &m_modular_curve_instance),
					m_modular_curves.get_output(VolumeModularCurveOutput::POINT_TO_ROT, curveSource, &m_modular_curve_instance));
	}

	void RingVolume::parse() {
		if (optional_string("+Radius:")) {
			stuff_float(&m_radius);
		}

		if (optional_string("+On Edge:")) {
			stuff_boolean(&m_onEdge);
		}

		ParticleVolume::parseCommon();

		m_modular_curves.parse("$Volume Curve:");
	}
}
