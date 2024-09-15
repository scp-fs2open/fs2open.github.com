#pragma once

#include "particle/ParticleVolume.h"

namespace particle {
	class SpheroidVolume : public ParticleVolume {
		float m_bias;
		float m_stretch;
		float m_radius;

	public:
		vec3d sampleRandomPoint(const matrix &orientation) override;
	};
}