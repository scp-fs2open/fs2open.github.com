#pragma once

#include "particle/ParticleVolume.h"
#include "particle/ParticleEffect.h"
#include "utils/RandomRange.h"

namespace particle {
	class ConeVolume : public ParticleVolume {
		::util::ParsedRandomFloatRange m_deviation;
		::util::ParsedRandomFloatRange m_length;

		enum class VolumeModularCurveOutput : uint8_t {DEVIATION, LENGTH, NUM_VALUES};
		constexpr static auto modular_curve_definition = ParticleEffect::modular_curves_definition.derive_modular_curves_output_only_subset<VolumeModularCurveOutput>(
			std::array {
				std::pair { "Deviation Mult", VolumeModularCurveOutput::DEVIATION },
				std::pair { "Length Mult", VolumeModularCurveOutput::LENGTH }
			});
		MODULAR_CURVE_SET(m_modular_curves, modular_curve_definition);
		modular_curves_entry_instance m_modular_curve_instance;
	public:
		explicit ConeVolume();
		explicit ConeVolume(::util::ParsedRandomFloatRange deviation, float length);

		vec3d sampleRandomPoint(const matrix &orientation, const std::tuple<const ParticleSource&, const size_t&>& source) override;
		void parse() override;
	};
}