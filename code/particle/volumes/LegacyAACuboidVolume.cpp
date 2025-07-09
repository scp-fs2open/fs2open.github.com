#include "LegacyAACuboidVolume.h"

#include "math/vecmat.h"

namespace particle {
	LegacyAACuboidVolume::LegacyAACuboidVolume(float normalVariance, float size, bool normalize) : m_normalVariance(normalVariance), m_size(size), m_normalize(normalize), m_modular_curve_instance(m_modular_curves.create_instance()) {	}

	vec3d LegacyAACuboidVolume::sampleRandomPoint(const matrix &orientation, decltype(ParticleEffect::modular_curves_definition)::input_type_t source, float particlesFraction) {
		auto curveSource = std::tuple_cat(source, std::make_tuple(particlesFraction));
		float variance = m_normalVariance * m_modular_curves.get_output(VolumeModularCurveOutput::VARIANCE, curveSource, &m_modular_curve_instance);

		vec3d normal;

		normal.xyz.x = orientation.vec.fvec.xyz.x + (frand() * 2.0f - 1.0f) * variance;
		normal.xyz.y = orientation.vec.fvec.xyz.y + (frand() * 2.0f - 1.0f) * variance;
		normal.xyz.z = orientation.vec.fvec.xyz.z + (frand() * 2.0f - 1.0f) * variance;

		if (m_normalize)
			vm_vec_normalize_safe(&normal);

		return normal * m_size;
	}
}