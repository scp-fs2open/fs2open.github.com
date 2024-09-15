#include "ConeVolume.h"

namespace particle {
	vec3d ConeVolume::sampleRandomPoint(const matrix &orientation) {
		//It is surely possible to do this more efficiently.
		angles angs;

		angs.b = 0.0f;

		angs.h = m_deviation.next();
		angs.p = m_deviation.next();

		matrix m;

		vm_angles_2_matrix(&m, &angs);

		matrix rotatedVel;
		vm_matrix_x_matrix(&rotatedVel, &orientation, &m);

		return m.vec.fvec * m_length;
	}
}