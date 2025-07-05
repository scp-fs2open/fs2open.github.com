#pragma once

#include "particle/ParticleVolume.h"
#include "particle/ParticleEffect.h"

namespace particle {
	class SpheroidVolume : public ParticleVolume {
		float m_bias;
		float m_stretch;
		float m_radius;

	public:
		enum class VolumeModularCurveOutput : uint8_t {BIAS, STRETCH, RADIUS, OFFSET_ROT, POINT_TO_ROT, NUM_VALUES};

	private:
		constexpr static auto modular_curve_definition = ParticleEffect::modular_curves_definition.derive_modular_curves_subset<float, VolumeModularCurveOutput>(
			std::array {
				std::pair { "Bias Mult", VolumeModularCurveOutput::BIAS },
				std::pair { "Stretch Mult", VolumeModularCurveOutput::STRETCH },
				std::pair { "Radius Mult", VolumeModularCurveOutput::RADIUS },
				std::pair { "Offset Rotate Around Fvec", VolumeModularCurveOutput::OFFSET_ROT },
				std::pair { "Point To Rotate Around Fvec", VolumeModularCurveOutput::POINT_TO_ROT }
			},
			std::pair { "Fraction Particles Spawned", modular_curves_self_input{}});

	public:
		MODULAR_CURVE_SET(m_modular_curves, modular_curve_definition);

	private:
		modular_curves_entry_instance m_modular_curve_instance;

	public:
		explicit SpheroidVolume();
		explicit SpheroidVolume(float bias, float stretch, float radius);

		vec3d sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) override;
		void parse() override;
	};
}