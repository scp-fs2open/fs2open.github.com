#include "PointVolume.h"

#include "math/vecmat.h"

namespace particle {
	PointVolume::PointVolume() : m_pos(ZERO_VECTOR), m_modular_curve_instance(m_modular_curves.create_instance()) { };
	PointVolume::PointVolume(const vec3d& pos) : m_pos(pos), m_modular_curve_instance(m_modular_curves.create_instance()) { };

	vec3d PointVolume::sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) {
		auto curveSource = std::tuple_cat(source, std::make_tuple(particlesFraction));

		vec3d pos = m_pos;

		// maybe scale it
		if (m_modular_curves.has_curve(VolumeModularCurveOutput::DISTANCE)) {
			pos *= m_modular_curves.get_output(VolumeModularCurveOutput::DISTANCE, curveSource, &m_modular_curve_instance);
		}

		vm_vec_unrotate(&pos, &pos, &orientation);

		return pointCompensateForOffsetAndRotOffset(pos,
					m_modular_curves.get_output(VolumeModularCurveOutput::OFFSET_ROT, curveSource, &m_modular_curve_instance),
					m_modular_curves.get_output(VolumeModularCurveOutput::POINT_TO_ROT, curveSource, &m_modular_curve_instance));
	}

	void PointVolume::parse() {
		if (optional_string("+Position:")) {
			stuff_vec3d(&m_pos);
		}

		ParticleVolume::parseCommon();

		m_modular_curves.parse("$Volume Curve:");
	}
}
