#pragma once

#include "particle/ParticleVolume.h"

namespace particle {
	class LegacyAACuboidVolume : public ParticleVolume {
		float m_normalVariance;
		float m_size;
		bool m_normalize;

	public:
		vec3d sampleRandomPoint(const matrix &orientation) override;
	};
}