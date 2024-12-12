#include "ConeVolume.h"

namespace particle {
	ConeVolume::ConeVolume(::util::ParsedRandomFloatRange deviation, float length) : m_deviation(std::move(deviation)), m_length(length), m_modular_curve_instance(m_modular_curves.create_instance()) { }

	vec3d ConeVolume::sampleRandomPoint(const matrix &orientation, const std::tuple<const ParticleSource&, const size_t&>& source) {
		//It is surely possible to do this more efficiently.
		angles angs;

		angs.b = 0.0f;

		float deviationMult = m_modular_curves.get_output(VolumeModularCurveOutput::DEVIATION, source, &m_modular_curve_instance);
		angs.h = m_deviation.next() * deviationMult;
		angs.p = m_deviation.next() * deviationMult;

		matrix m;

		vm_angles_2_matrix(&m, &angs);

		matrix rotatedVel;
		vm_matrix_x_matrix(&rotatedVel, &orientation, &m);

		return rotatedVel.vec.fvec * (m_length * m_modular_curves.get_output(VolumeModularCurveOutput::LENGTH, source, &m_modular_curve_instance));
	}
}