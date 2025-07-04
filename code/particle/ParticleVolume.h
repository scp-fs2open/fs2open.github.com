#pragma once

#include "globalincs/pstypes.h"
#include "parse/parselo.h"

#include <optional>

namespace particle {
	class ParticleSource;
	class ParticleVolume {
	public:
		virtual vec3d sampleRandomPoint(const matrix &orientation, const std::tuple<const ParticleSource&, const size_t&, const vec3d&>& source, float particlesFraction) = 0;

		virtual void parse() = 0;

		virtual ~ParticleVolume() = default;

		std::optional<vec3d> posOffset;
		std::optional<vec3d> rotOffset;
	protected:
		void parseCommon() {
			if (optional_string("+Volume Position Offset:")) {
				stuff_vec3d(&posOffset.emplace());
			}
			if (optional_string("+Volume Point Towards:")) {
				stuff_vec3d(&rotOffset.emplace());
			}
		}

		vec3d pointCompensateForOffsetAndRotOffset(const vec3d& point, const matrix& orientation, float posOffsetRot, float rotOffsetRot) const {
			vec3d outpnt = point;

			if (rotOffset.has_value()) {
				vec3d rot = *rotOffset;
				vm_rot_point_around_line(&rot, &rot, rotOffsetRot, &vmd_zero_vector, &vmd_z_vector);
				matrix orientUse;
				vm_vector_2_matrix(&orientUse, &rot);
				vm_vec_unrotate(&outpnt, &outpnt, &orientUse);
			}
			if (posOffset.has_value()) {
				vec3d pos = *posOffset;
				vm_rot_point_around_line(&pos, &pos, posOffsetRot, &vmd_zero_vector, &vmd_z_vector);
				vm_vec_unrotate(&pos, &pos, &orientation);
				outpnt += pos;
			}

			return outpnt;
		}
	};
}