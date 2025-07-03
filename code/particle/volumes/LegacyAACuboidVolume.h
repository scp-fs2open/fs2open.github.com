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

	private:
		constexpr static auto modular_curve_definition = ParticleEffect::modular_curves_definition.derive_modular_curves_subset<float, VolumeModularCurveOutput>(
			std::array {
				std::pair { "Variance", VolumeModularCurveOutput::VARIANCE }
			},
			std::pair { "Fraction Particles Spawned", modular_curves_self_input{}});

	public:
		MODULAR_CURVE_SET(m_modular_curves, modular_curve_definition);

	private:
		modular_curves_entry_instance m_modular_curve_instance;

	public:
	  	explicit LegacyAACuboidVolume(float normalVariance, float size, bool normalize);

		vec3d sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) override;
		void parse() override {
			UNREACHABLE("Cannot parse Legacy Particle Volume!");
		};

	};
}