#pragma once

#include "particle/ParticleVolume.h"
#include "particle/ParticleEffect.h"

namespace particle {
	class RingVolume : public ParticleVolume {
		float m_radius;
		bool m_onEdge;

		enum class VolumeModularCurveOutput : uint8_t {RADIUS, OFFSET_ROT, POINT_TO_ROT, NUM_VALUES};
		constexpr static auto modular_curve_definition = ParticleEffect::modular_curves_definition.derive_modular_curves_subset<float, VolumeModularCurveOutput>(
			std::array {
				std::pair { "Radius Mult", VolumeModularCurveOutput::RADIUS },
				std::pair { "Offset Rotate Around Fvec", VolumeModularCurveOutput::OFFSET_ROT },
				std::pair { "Point To Rotate Around Fvec", VolumeModularCurveOutput::POINT_TO_ROT }
			},
			std::pair { "Fraction Particles Spawned", modular_curves_self_input{}});
		MODULAR_CURVE_SET(m_modular_curves, modular_curve_definition);
		modular_curves_entry_instance m_modular_curve_instance;
	public:
		explicit RingVolume();
		explicit RingVolume(float radius, bool onEdge);

		vec3d sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) override;
		void parse() override;
	};
}