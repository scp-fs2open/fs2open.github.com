#include "ConeVolume.h"

namespace particle {
	ConeVolume::ConeVolume() : m_deviation(::util::UniformFloatRange(0.f)), m_length(::util::UniformFloatRange(1.f)), m_modular_curve_instance(m_modular_curves.create_instance()) { }
	ConeVolume::ConeVolume(::util::ParsedRandomFloatRange deviation, float length) : m_deviation(deviation), m_length(::util::UniformFloatRange(length)), m_modular_curve_instance(m_modular_curves.create_instance()) { }

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

		return rotatedVel.vec.fvec * (m_length.next() * m_modular_curves.get_output(VolumeModularCurveOutput::LENGTH, source, &m_modular_curve_instance));
	}

	void ConeVolume::parse() {
		int deviation_type = required_string_one_of(2, "+Deviation:", "+Deviation Profile:");
		if (deviation_type == 0) {
			required_string("+Deviation:");
			float deviation;
			stuff_float(&deviation);

			if (deviation < 0.001f) {
				error_display(0, "A standard deviation of %f is not valid. Must be greater than 0. Defaulting to 1.", deviation);
				deviation = 1.0f;
			}

			m_deviation = ::util::BoundedNormalFloatRange(::util::BoundedNormalDistribution::param_type{ std::normal_distribution<float>::param_type(0.f, fl_radians(deviation)), -PI, PI });
		}
		else if (deviation_type == 1) {
			required_string("+Deviation Profile:");
			//Note, this is in radians NOT degrees. But given this is a rare and advanced option, it should be fine
			m_deviation = ::util::ParsedRandomFloatRange::parseRandomRange(-PI, PI);
		}

		if (optional_string("+Length:")) {
			m_length = ::util::ParsedRandomFloatRange::parseRandomRange(0);
		}

		m_modular_curves.parse("$Volume Curve:");
	}
}