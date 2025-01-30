#pragma once

#include "particle/ParticleVolume.h"
#include "particle/ParticleEffect.h"

namespace particle {
	class SpheroidVolume : public ParticleVolume {
		float m_bias;
		float m_stretch;
		float m_radius;

		enum class VolumeModularCurveOutput : uint8_t {BIAS, STRETCH, RADIUS, NUM_VALUES};
		constexpr static auto modular_curve_definition = ParticleEffect::modular_curves_definition.derive_modular_curves_output_only_subset<VolumeModularCurveOutput>(
			std::array {
				std::pair { "Bias Mult", VolumeModularCurveOutput::BIAS },
				std::pair { "Stretch Mult", VolumeModularCurveOutput::STRETCH },
				std::pair { "Radius Mult", VolumeModularCurveOutput::RADIUS }
			});
		MODULAR_CURVE_SET(m_modular_curves, modular_curve_definition);
		modular_curves_entry_instance m_modular_curve_instance;
	public:
		explicit SpheroidVolume();
		explicit SpheroidVolume(float bias, float stretch, float radius);

		vec3d sampleRandomPoint(const matrix &orientation, const std::tuple<const ParticleSource&, const size_t&>& source) override;
		void parse() override;
	};
}