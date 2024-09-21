#pragma once

#include "particle/ParticleVolume.h"

namespace particle {
	class LegacyAACuboidVolume : public ParticleVolume {
		float m_normalVariance;
		float m_size;
		bool m_normalize;

	public:
	  	explicit LegacyAACuboidVolume(float normalVariance, float size, bool normalize);

		vec3d sampleRandomPoint(const matrix &orientation) override;
		void parse() override {
			UNREACHABLE("Cannot parse Legacy Particle Volume!");
		};
	};
}