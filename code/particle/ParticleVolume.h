#pragma once

#include "globalincs/pstypes.h"

namespace particle {
	class ParticleVolume {

	public:
		virtual vec3d sampleRandomPoint(const matrix &orientation) = 0;

		virtual void parse() = 0;
	};
}