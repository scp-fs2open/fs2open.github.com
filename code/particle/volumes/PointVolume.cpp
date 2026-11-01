#include "PointVolume.h"

#include "math/vecmat.h"

namespace particle {
	PointVolume::PointVolume() : m_modular_curve_instance(m_modular_curves.create_instance()) { };

	vec3d PointVolume::sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction, const EffectHost& /*host*/) {
		auto curveSource = std::tuple_cat(source, std::make_tuple(particlesFraction));

		return pointCompensateForOffsetAndRotOffset(ZERO_VECTOR, orientation,
			m_modular_curves.get_output_or_default(VolumeModularCurveOutput::OFFSET_ROT, curveSource, 0.f, &m_modular_curve_instance),
			m_modular_curves.get_output_or_default(VolumeModularCurveOutput::POINT_TO_ROT, curveSource, 0.f, &m_modular_curve_instance));
	}

	void PointVolume::parse() {
		ParticleVolume::parseCommon();

		m_modular_curves.parse("$Volume Curve:");
	}
}
