#pragma once

#include "particle/ParticleVolume.h"
#include "particle/ParticleEffect.h"

namespace particle {
	class LegacyAACuboidVolume : public ParticleVolume {
		float m_normalVariance;
		float m_size;
		bool m_normalize;

	public:
		enum class VolumeModularCurveOutput : uint8_t {VARIANCE, NUM_VALUES};
		constexpr static auto modular_curve_definition = ParticleEffect::modular_curves_definition.derive_modular_curves_output_only_subset<VolumeModularCurveOutput>(
			std::array {
				std::pair { "Variance", VolumeModularCurveOutput::VARIANCE }
			});
		MODULAR_CURVE_SET(m_modular_curves, modular_curve_definition);

	  	explicit LegacyAACuboidVolume(float normalVariance, float size, bool normalize);

		vec3d sampleRandomPoint(const matrix &orientation, const ParticleSource& source) override;
		void parse() override {
			UNREACHABLE("Cannot parse Legacy Particle Volume!");
		};

	};
}