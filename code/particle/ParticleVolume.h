#pragma once

#include "globalincs/pstypes.h"

#include <optional>

namespace particle {
	class ParticleSource;
	class ParticleVolume {

	public:
		virtual vec3d sampleRandomPoint(const matrix &orientation, const std::tuple<const ParticleSource&, const size_t&, const vec3d&>& source, float particlesFraction) = 0;

		virtual void parse() {
			//TODO
		};

		virtual ~ParticleVolume() = default;
	protected:

		vec3d pointCompensateForOffsetAndRotOffset(const vec3d& point, const matrix &orientation, const std::optional<vec3d>& posOffset, const std::optional<vec3d>& rotOffset, float posOffsetRot, float rotOffsetRot) {
			//TODO
			return point;
		}
	};
}