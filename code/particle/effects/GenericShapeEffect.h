#ifndef GENERIC_SHAPE_EFFECT_H
#define GENERIC_SHAPE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/util/ParticleProperties.h"
#include "utils/RandomRange.h"

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
	::util::UniformFloatRange m_velocity;
	::util::UniformUIntRange m_particleNum;

	ParticleEffectHandle m_particleTrail = ParticleEffectHandle::invalid();

	util::EffectTiming m_timing;

	TShape m_shape;

	vec3d getNewDirection(const ParticleSource* source) const {
		switch (m_direction) {
			case ConeDirection::Incoming:
				return source->getOrientation()->getDirectionVector(source->getOrigin());
			case ConeDirection::Normal: {
				vec3d normal;
				if (!source->getOrientation()->getNormal(&normal)) {
					mprintf(("Effect '%s' tried to use normal direction for source without a normal!\n", m_name.c_str()));
					return source->getOrientation()->getDirectionVector(source->getOrigin());
				}

				return normal;
			}
			case ConeDirection::Reflected: {
				vec3d out = source->getOrientation()->getDirectionVector(source->getOrigin());
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
				vec3d out = source->getOrientation()->getDirectionVector(source->getOrigin());
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

	bool processSource(ParticleSource* source) override {
		if (!m_timing.continueProcessing(source)) {
			return false;
		}

		// This uses the internal features of the timing class for determining if and how many effects should be
		// triggered this frame
		util::EffectTiming::TimingState time_state;
		while (m_timing.shouldCreateEffect(source, time_state)) {
			auto num = m_particleNum.next();

			vec3d dir = getNewDirection(source);
			matrix dirMatrix;
			vm_vector_2_matrix(&dirMatrix, &dir, nullptr, nullptr);
			for (uint i = 0; i < num; ++i) {
				matrix velRotation = m_shape.getDisplacementMatrix();

				matrix rotatedVel;
				vm_matrix_x_matrix(&rotatedVel, &dirMatrix, &velRotation);

				particle_info info;

				source->getOrigin()->applyToParticleInfo(info);

				info.vel = rotatedVel.vec.fvec;
				if (TShape::scale_velocity_deviation()) {
					// Scale the vector with a random velocity sample and also multiply that with cos(angle between
					// info.vel and sourceDir) That should produce good looking directions where the maximum velocity is
					// only achieved when the particle travels directly on the normal/reflect vector
					vm_vec_scale(&info.vel, vm_vec_dot(&info.vel, &dir));
				}
				vm_vec_scale(&info.vel, m_velocity.next());

				if (m_particleTrail.isValid()) {
					auto part = m_particleProperties.createPersistentParticle(info);

					// There are some possibilities where we can get a null pointer back. Those are very rare but we
					// still shouldn't crash in those circumstances.
					if (!part.expired()) {
						auto trailSource = ParticleManager::get()->createSource(m_particleTrail);
						trailSource.moveToParticle(part);

						trailSource.finish();
					}
				} else {
					// We don't have a trail so we don't need a persistent particle
					m_particleProperties.createParticle(info);
				}
			}
		}

		return true;
	}

	void parseValues(bool nocreate) override {
		m_particleProperties.parse(nocreate);

		m_shape.parse(nocreate);

		if (internal::required_string_if_new("+Velocity:", nocreate)) {
			m_velocity = ::util::parseUniformRange<float>();
		}

		if (internal::required_string_if_new("+Number:", nocreate)) {
			m_particleNum = ::util::parseUniformRange<uint>();
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

		bool saw_deprecated_effect_location = false;
		if (optional_string("+Trail effect:")) {
			// This is the deprecated location since this introduces ambiguities in the parsing process
			m_particleTrail = internal::parseEffectElement();
			saw_deprecated_effect_location = true;
		}

		m_timing = util::EffectTiming::parseTiming();

		if (optional_string("+Trail effect:")) {
			// This is the new and correct location. This might create duplicate effects but the warning should be clear
			// enough to avoid that
			if (saw_deprecated_effect_location) {
				error_display(0, "Found two trail effect options! Specifying '+Trail effect:' before '+Duration:' is "
				                 "deprecated since that can cause issues with conflicting effect options.");
			}
			m_particleTrail = internal::parseEffectElement();
		}
	}

	void initializeSource(ParticleSource& source) override {
		m_timing.applyToSource(&source);
	}

	EffectType getType() const override { return m_shape.getType(); }

	void pageIn() override {
		m_particleProperties.pageIn();
	}

};

}
}

#endif // GENERIC_SHAPE_EFFECT_H
