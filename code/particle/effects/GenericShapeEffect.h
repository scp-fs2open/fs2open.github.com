#ifndef GENERIC_SHAPE_EFFECT_H
#define GENERIC_SHAPE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/util/RandomRange.h"
#include "particle/util/ParticleProperties.h"

namespace particle {
namespace effects {

enum class ConeDirection {
	Incoming,
	Normal,
	Reflected,
	Reverse
};

/**
 * @brief A generic particle effect with a customizable shape
 *
 * @ingroup particleEffects
 */
template<typename TShape>
class GenericShapeEffect : public ParticleEffect {
 private:
	util::ParticleProperties m_particleProperties;

	ConeDirection m_direction = ConeDirection::Incoming;
	util::UniformFloatRange m_velocity;
	util::UniformUIntRange m_particleNum;

	ParticleEffectIndex m_particleTrail = -1;

	TShape m_shape;

	vec3d getNewDirection(const ParticleSource* source) const {
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

 public:
	explicit GenericShapeEffect(const SCP_string& name) : ParticleEffect(name) {
	}

	virtual bool processSource(const ParticleSource* source) override {
		auto num = m_particleNum.next();

		vec3d dir = getNewDirection(source);
		matrix dirMatrix;
		vm_vector_2_matrix(&dirMatrix, &dir, nullptr, nullptr);
		for (uint i = 0; i < num; ++i) {
			matrix velRotation = m_shape.getDisplacementMatrix();

			matrix rotatedVel;
			vm_matrix_x_matrix(&rotatedVel, &dirMatrix, &velRotation);

			particle_info info;

			memset(&info, 0, sizeof(info));
			source->getOrigin()->applyToParticleInfo(info);

			info.vel = rotatedVel.vec.fvec;
			if (TShape::scale_velocity_deviation()) {
				// Scale the vector with a random velocity sample and also multiply that with cos(angle between info.vel and sourceDir)
				// That should produce good looking directions where the maximum velocity is only achieved when the particle travels directly
				// on the normal/reflect vector
				vm_vec_scale(&info.vel, vm_vec_dot(&info.vel, &dir));
			}
			vm_vec_scale(&info.vel, m_velocity.next());

			auto part = m_particleProperties.createParticle(info);

			if (m_particleTrail >= 0) {
				auto trailSource = ParticleManager::get()->createSource(m_particleTrail);
				trailSource.moveToParticle(part);

				trailSource.finish();
			}
		}

		return false;
	}

	virtual void parseValues(bool nocreate) override {
		m_particleProperties.parse(nocreate);

		m_shape.parse(nocreate);

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

	virtual EffectType getType() const override { return m_shape.getType(); }

	virtual void pageIn() override {
		m_particleProperties.pageIn();
	}

};

}
}

#endif // GENERIC_SHAPE_EFFECT_H
