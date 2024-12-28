#ifndef GENERIC_SHAPE_EFFECT_H
#define GENERIC_SHAPE_EFFECT_H
#include "osapi/dialogs.h"
#include "parse/parselo.h"
#pragma once

#include "freespace.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/util/ParticleProperties.h"
#include "particle/util/EffectTiming.h"
#include "utils/RandomRange.h"
#include "weapon/beam.h"

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
	::util::ParsedRandomFloatRange m_velocity;
	::util::ParsedRandomUintRange m_particleNum;
	float m_particleChance = 1.0f;
	::util::ParsedRandomFloatRange m_particleRoll;
	ParticleEffectHandle m_particleTrail = ParticleEffectHandle::invalid();

	util::EffectTiming m_timing;

	::util::ParsedRandomFloatRange m_vel_inherit;

	TShape m_shape;

	vec3d getNewDirection(const ParticleSource* source) const {
		switch (m_direction) {
			case ConeDirection::Incoming:
				return source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
			case ConeDirection::Normal: {
				vec3d normal;
				if (!source->getOrientation()->getNormal(&normal)) {
					return source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				}

				return normal;
			}
			case ConeDirection::Reflected: {
				vec3d out = source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				vec3d normal;
				if (!source->getOrientation()->getNormal(&normal)) {
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
				vec3d out = source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
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
		for (int time_since_creation = m_timing.shouldCreateEffect(source, time_state); time_since_creation >= 0; time_since_creation = m_timing.shouldCreateEffect(source, time_state)) {
			float interp = static_cast<float>(time_since_creation)/(flFrametime * 1000.0f);

			auto num = m_particleNum.next();

			if (source->getOrigin()->getType() == SourceOriginType::BEAM) {
				// beam particle numbers are per km
				object* b_obj = source->getOrigin()->getObjectHost();
				float dist = vm_vec_dist(&Beams[b_obj->instance].last_start, &Beams[b_obj->instance].last_shot) / 1000.0f;
				float km;
				float remainder = modff(dist, &km);
				uint old_num = num;
				num = (uint)(old_num * km); // multiply by the number of kilometers
				num += (uint)(old_num * remainder); // try to add any remainders if we have more than 1 per kilometer
				// if we still have nothing let's give it one last shot
				if (num < 1 && frand() < remainder * old_num)
					num += 1;
			}

			vec3d dir = getNewDirection(source);
			matrix dirMatrix;
			vm_vector_2_matrix(&dirMatrix, &dir, nullptr, nullptr);
			
			for (uint i = 0; i < num; ++i) {
				if (m_particleChance < 1.0f) {
					auto roll = m_particleRoll.next();
					if (roll <= 0.0f)
						continue;
				}

				matrix velRotation = m_shape.getDisplacementMatrix();

				matrix rotatedVel;
				vm_matrix_x_matrix(&rotatedVel, &dirMatrix, &velRotation);

				particle_info info;

				source->getOrigin()->applyToParticleInfo(info, m_particleProperties.m_parent_local, interp, m_particleProperties.m_manual_offset);

				vec3d velocity = rotatedVel.vec.fvec;
				if (TShape::scale_velocity_deviation()) {
					// Scale the vector with a random velocity sample and also multiply that with cos(angle between
					// info.vel and sourceDir) That should produce good looking directions where the maximum velocity is
					// only achieved when the particle travels directly on the normal/reflect vector
					vm_vec_scale(&velocity, vm_vec_dot(&velocity, &dir));
				}
				vm_vec_scale(&velocity, m_velocity.next());

				info.vel *= m_vel_inherit.next();
				info.vel += velocity;

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
			m_velocity = ::util::ParsedRandomFloatRange::parseRandomRange();
		}

		if (internal::required_string_if_new("+Number:", nocreate)) {
			m_particleNum = ::util::ParsedRandomUintRange::parseRandomRange();
		}
		if (!nocreate) {
			m_particleChance = 1.0f;
		}
		if (optional_string("+Chance:")) {
			float chance;
			stuff_float(&chance);
			if (chance <= 0.0f) {
				error_display(0,
					"Particle %s tried to set +Chance: %f\nChances below 0 would result in no particles.",
					m_name.c_str(), chance);
			} else if (chance > 1.0f) {
				error_display(0,
					"Particle %s tried to set +Chance: %f\nChances above 1 are ignored, please use +Number: (min,max) "
					"to spawn multiple particles.", m_name.c_str(), chance);
				chance = 1.0f;
			}
			m_particleChance = chance;
		}
		m_particleRoll = ::util::UniformFloatRange(m_particleChance - 1.0f, m_particleChance);

		if (optional_string("+Direction:")) {
			char dirStr[NAME_LENGTH];
			stuff_string(dirStr, F_NAME, NAME_LENGTH);

			if (!stricmp(dirStr, "Incoming")) {
				m_direction = ConeDirection::Incoming;
			}
			else if (!stricmp(dirStr, "Normal")) {
				m_direction = ConeDirection::Normal;
			}
			else if (!stricmp(dirStr, "Reflected")) {
				m_direction = ConeDirection::Reflected;
			}
			else if (!stricmp(dirStr, "Reverse")) {
				m_direction = ConeDirection::Reverse;
			}
			else {
				error_display(0, "Unknown direction name '%s'!", dirStr);
			}
		}

		bool saw_deprecated_effect_location = false;
		if (optional_string("+Trail effect:")) {
			// This is the deprecated location since this introduces ambiguities in the parsing process
			m_particleTrail = internal::parseEffectElement();
			saw_deprecated_effect_location = true;
		}

		if (optional_string("+Parent Velocity Factor:")) {
			m_vel_inherit = ::util::ParsedRandomFloatRange::parseRandomRange();
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
