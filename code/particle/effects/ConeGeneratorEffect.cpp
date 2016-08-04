#include <math.h>

#include <globalincs/pstypes.h>

#include "particle/ParticleSource.h"
#include "particle/effects/ConeGeneratorEffect.h"

#include "bmpman/bmpman.h"
#include "math/vecmat.h"

namespace particle {
namespace effects {
ConeGeneratorEffect::ConeGeneratorEffect(const SCP_string& name)
	: ParticleEffect(name) {}

vec3d ConeGeneratorEffect::getNewDirection(const ParticleSource* source) const {
	switch (m_direction) {
		case ConeDirection::Incoming:
			return source->getOrientation()->getDirectionVector();
		case ConeDirection::Normal: {
			vec3d normal;
			if (!source->getOrientation()->getNormal(&normal)) {
				mprintf(("Effect '%s' tried to use normal direction for source without a normal!\n", m_name.c_str()));
				return source->getOrientation()->getDirectionVector();
			}

			return normal;
		}
		case ConeDirection::Reflected: {
			vec3d out = source->getOrientation()->getDirectionVector();
			vec3d normal;
			if (!source->getOrientation()->getNormal(&normal)) {
				mprintf(("Effect '%s' tried to use normal direction for source without a normal!\n", m_name.c_str()));
				return out;
			}

			// Compute reflect vector as R = V - 2*(V dot N)*N where N
			// is the normal and V is the incoming direction

			auto dot = 2 * vm_vec_dot(&out, &normal);

			vm_vec_scale(&normal, dot);
			vm_vec_sub(&out, &out, &normal);

			return out;
		}
		case ConeDirection::Reverse: {
			vec3d out = source->getOrientation()->getDirectionVector();
			vm_vec_scale(&out, -1.0f);
			return out;
		}
		default:
			Error(LOCATION, "Unhandled direction value!");
			return vmd_zero_vector;
	}
}

matrix ConeGeneratorEffect::getDisplacementMatrix() {
	angles angs;

	angs.b = 0.0f;

	angs.h = m_normalDeviation.next();
	angs.p = m_normalDeviation.next();

	matrix m;

	vm_angles_2_matrix(&m, &angs);

	return m;
}

bool ConeGeneratorEffect::processSource(const ParticleSource* source) {
	auto num = m_particleNum.next();

	for (uint i = 0; i < num; ++i) {
		vec3d dir = getNewDirection(source);

		matrix dirMatrix;
		vm_vector_2_matrix(&dirMatrix, &dir, nullptr, nullptr);

		matrix velRotation = getDisplacementMatrix();

		matrix rotatedVel;
		vm_matrix_x_matrix(&rotatedVel, &dirMatrix, &velRotation);

		particle_info info;

		memset(&info, 0, sizeof(info));
		source->getOrigin()->applyToParticleInfo(info);

		info.vel = rotatedVel.vec.fvec;
		// Scale the vector with a random velocity sample and also multiply that with cos(angle between info.vel and sourceDir)
		// That should produce good looking directions where the maximum velocity is only achieved when the particle travels directly
		// on the normal/reflect vector
		vm_vec_scale(&info.vel, m_velocity.next() * vm_vec_dot(&info.vel, &dir));

		auto part = m_particleProperties.createParticle(info);

		if (m_particleTrail >= 0) {
			auto trailSource = ParticleManager::get()->createSource(m_particleTrail);
			trailSource.moveToParticle(part);

			trailSource.finish();
		}
	}

	return false;
}

void ConeGeneratorEffect::parseValues(bool nocreate) {
	m_particleProperties.parse(nocreate);

	if (internal::required_string_if_new("+Deviation:", nocreate)) {
		float deviation;
		stuff_float(&deviation);

		m_normalDeviation = util::NormalFloatRange(0.0, fl_radians(deviation));
	}

	if (internal::required_string_if_new("+Velocity:", nocreate)) {
		m_velocity = util::parseUniformRange<float>();
	}

	if (internal::required_string_if_new("+Number:", nocreate)) {
		m_particleNum = util::parseUniformRange<uint>();
	}

	if (optional_string("+Direction:")) {
		SCP_string dirStr;
		stuff_string(dirStr, F_NAME);

		if (!stricmp(dirStr.c_str(), "Incoming")) {
			m_direction = ConeDirection::Incoming;
		}
		else if (!stricmp(dirStr.c_str(), "Normal")) {
			m_direction = ConeDirection::Normal;
		}
		else if (!stricmp(dirStr.c_str(), "Reflected")) {
			m_direction = ConeDirection::Reflected;
		}
		else if (!stricmp(dirStr.c_str(), "Reverse")) {
			m_direction = ConeDirection::Reverse;
		}
		else {
			error_display(0, "Unknown direction name '%s'!", dirStr.c_str());
		}
	}

	if (optional_string("+Trail effect:")) {
		m_particleTrail = internal::parseEffectElement();
	}
}

void ConeGeneratorEffect::pageIn() {
	m_particleProperties.pageIn();
}
}
}
