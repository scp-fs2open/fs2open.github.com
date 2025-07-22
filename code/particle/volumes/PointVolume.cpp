#include "PointVolume.h"

#include "math/vecmat.h"

namespace particle {
	PointVolume::PointVolume() : m_modular_curve_instance(m_modular_curves.create_instance()) { };

	vec3d PointVolume::sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) {
		auto curveSource = std::tuple_cat(source, std::make_tuple(particlesFraction));

		return pointCompensateForOffsetAndRotOffset(ZERO_VECTOR, orientation,
					m_modular_curves.get_output(VolumeModularCurveOutput::OFFSET_ROT, curveSource, &m_modular_curve_instance),
					m_modular_curves.get_output(VolumeModularCurveOutput::POINT_TO_ROT, curveSource, &m_modular_curve_instance));
	}

	void PointVolume::parse() {
		ParticleVolume::parseCommon();

		m_modular_curves.parse("$Volume Curve:");
	}
}
