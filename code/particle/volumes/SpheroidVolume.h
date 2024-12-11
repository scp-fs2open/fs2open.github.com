#pragma once

#include "particle/ParticleVolume.h"

namespace particle {
	class SpheroidVolume : public ParticleVolume {
		float m_bias;
		float m_stretch;
		float m_radius;

	public:
		explicit SpheroidVolume(float bias, float stretch, float radius);

		vec3d sampleRandomPoint(const matrix &orientation, const ParticleSource& source) override;
		void parse() override {
			//TODO
		};
	};
}