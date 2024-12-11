#pragma once

#include "globalincs/pstypes.h"

namespace particle {
	class ParticleSource;
	class ParticleVolume {

	public:
		virtual vec3d sampleRandomPoint(const matrix &orientation, const ParticleSource& source) = 0;

		virtual void parse() = 0;

		virtual ~ParticleVolume() = default;
	};
}