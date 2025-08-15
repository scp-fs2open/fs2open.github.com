#pragma once

#include "particle/ParticleVolume.h"
#include "particle/ParticleEffect.h"
#include "utils/RandomRange.h"

namespace particle {
	class ConeVolume : public ParticleVolume {
		friend int ::parse_weapon(int, bool, const char*);

		::util::ParsedRandomFloatRange m_deviation;
		::util::ParsedRandomFloatRange m_length;

		enum class VolumeModularCurveOutput : uint8_t {DEVIATION, LENGTH, OFFSET_ROT, POINT_TO_ROT, NUM_VALUES};
		constexpr static auto modular_curve_definition = ParticleEffect::modular_curves_definition.derive_modular_curves_subset<float, VolumeModularCurveOutput>(
			std::array {
				std::pair { "Deviation Mult", VolumeModularCurveOutput::DEVIATION },
				std::pair { "Length Mult", VolumeModularCurveOutput::LENGTH },
				std::pair { "Offset Rotate Around Fvec", VolumeModularCurveOutput::OFFSET_ROT },
				std::pair { "Point To Rotate Around Fvec", VolumeModularCurveOutput::POINT_TO_ROT }
			},
			std::pair { "Fraction Particles Spawned", modular_curves_self_input{}});
		MODULAR_CURVE_SET(m_modular_curves, modular_curve_definition);
		modular_curves_entry_instance m_modular_curve_instance;
	public:
		explicit ConeVolume();
		explicit ConeVolume(::util::ParsedRandomFloatRange deviation, float length);
		explicit ConeVolume(::util::ParsedRandomFloatRange deviation, ::util::ParsedRandomFloatRange length);

		vec3d sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) override;
		void parse() override;
	};
}