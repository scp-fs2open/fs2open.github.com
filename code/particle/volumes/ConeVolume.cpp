#include "ConeVolume.h"

namespace particle {
	ConeVolume::ConeVolume(::util::ParsedRandomFloatRange deviation, float length) : m_deviation(std::move(deviation)), m_length(length) { }

	vec3d ConeVolume::sampleRandomPoint(const matrix &orientation, const ParticleSource& source) {
		//It is surely possible to do this more efficiently.
		angles angs;

		angs.b = 0.0f;

		angs.h = m_deviation.next();
		angs.p = m_deviation.next();

		matrix m;

		vm_angles_2_matrix(&m, &angs);

		matrix rotatedVel;
		vm_matrix_x_matrix(&rotatedVel, &orientation, &m);

		return rotatedVel.vec.fvec * m_length;
	}
}