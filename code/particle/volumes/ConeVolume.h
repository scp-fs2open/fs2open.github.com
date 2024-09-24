#pragma once

#include "particle/ParticleVolume.h"

#include "utils/RandomRange.h"

namespace particle {
	class ConeVolume : public ParticleVolume {
		::util::ParsedRandomFloatRange m_deviation;
		float m_length;

	public:
		explicit ConeVolume(::util::ParsedRandomFloatRange deviation, float length);

		vec3d sampleRandomPoint(const matrix &orientation) override;
		void parse() override {
			//TODO
		};
	};
}