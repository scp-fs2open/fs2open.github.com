#ifndef SPOUT_EFFECT_H
#define SPOUT_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/util/ParticleProperties.h"
#include "utils/RandomRange.h"

namespace particle {
namespace effects {

/**
	* @brief A particle effect with a randomly walking 'spout' in a customizable shape
	*
	* @ingroup particleEffects
	*/
template<typename TShape>
class SpoutEffect : public ParticleEffect {
private:
	util::ParticleProperties m_particleProperties;

	ConeDirection m_direction = ConeDirection::Incoming;
	::util::UniformFloatRange m_velocity;
	::util::UniformUIntRange m_particleNum;
	::util::UniformFloatRange m_spoutSpeed;

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
	explicit SpoutEffect(const SCP_string& name) : ParticleEffect(name) {
	}

	bool processSource(ParticleSource* source) override {
		if (!m_timing.continueProcessing(source)) {
			return false;
		}

		// initialize all the spouts we might need
		if (source->m_effectData == nullptr) {
			auto spouts = std::make_shared<SCP_vector<vec3d>>();

			spouts->reserve(m_particleNum.max());
			for (uint i = 0; i < m_particleNum.max(); i++) 
				// the spouts are in their local frame
				spouts->push_back(vmd_z_vector);

			source->m_effectData = spouts;
		}

		SCP_vector<vec3d>& spouts = *std::static_pointer_cast<SCP_vector<vec3d>>(source->m_effectData);

		util::EffectTiming::TimingState time_state;
		while (m_timing.shouldCreateEffect(source, time_state)) {
			vec3d dir = getNewDirection(source);
			matrix dirMatrix;
			vm_vector_2_matrix(&dirMatrix, &dir, nullptr, nullptr);

			for (vec3d &spout : spouts) {
				// get the random step
				vec3d rand;
				vm_vec_rand_vec(&rand);
				rand *= m_spoutSpeed.next();
				
				// add it to the spout
				spout += rand;

				// nudge it back where it belongs so it doesn't get too far off course
				m_shape.restoreForce(&spout, m_spoutSpeed.max());

				// unrotate because spouts are in their local frame
				vec3d rotated_spout;
				vm_vec_unrotate(&rotated_spout, &spout, &dirMatrix);

				particle_info info;

				source->getOrigin()->applyToParticleInfo(info);

				info.vel = rotated_spout;
				if (TShape::scale_velocity_deviation()) {
					// Scale the vector with a random velocity sample and also multiply that with cos(angle between
					// info.vel and sourceDir) That should produce good looking directions where the maximum velocity is
					// only achieved when the particle travels directly on the normal/reflect vector
					vm_vec_scale(&info.vel, vm_vec_dot(&info.vel, &dir));
				}
				vm_vec_scale(&info.vel, m_velocity.next());

				if (m_particleTrail.isValid()) {
					auto part = m_particleProperties.createPersistentParticle(info);

					auto trailSource = ParticleManager::get()->createSource(m_particleTrail);
					trailSource.moveToParticle(part);

					trailSource.finish();
				}
				else {
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

		if (internal::required_string_if_new("+Spout Speed:", nocreate)) {
			m_spoutSpeed = ::util::parseUniformRange<float>(0.001f,1.0f);
		}

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

		m_timing = util::EffectTiming::parseTiming();

		if (optional_string("+Trail effect:")) {
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

#endif // SPOUT_EFFECT_H
