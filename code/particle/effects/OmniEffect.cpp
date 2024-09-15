#include "particle/effects/OmniEffect.h"

#include "weapon/beam.h"
#include "render/3d.h"

namespace particle {
namespace effects {

void OmniParticleEffect::initializeSource(ParticleSource &source) {
	m_timing.applyToSource(&source);
}

//This MUST be refactored into ParticleSourceHost
	vec3d OmniParticleEffect::getNewDirection(const ParticleSource* source) const {
		switch (m_direction) {
			case ShapeDirection::Aligned:
				return source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
			case ShapeDirection::HitNormal: {
				vec3d normal;
				if (!source->getOrientation()->getNormal(&normal)) {
					return source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				}

				return normal;
			}
			case ShapeDirection::Reflected: {
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
			case ShapeDirection::Reverse: {
				vec3d out = source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				vm_vec_scale(&out, -1.0f);
				return out;
			}
			default:
				Error(LOCATION, "Unhandled direction value!");
				return vmd_zero_vector;
		}
	}

bool OmniParticleEffect::processSource(ParticleSource* source) {
	if (!m_timing.continueProcessing(source)) {
		return false;
	}

	float particle_percent = m_particleChance;

	if (m_affectedByDetail){
		if (Detail.num_particles > 0)
			particle_percent *= (0.5f + (0.25f * static_cast<float>(Detail.num_particles - 1)));
		else
			return true; //Will not emit on current detail settings, but may in the future.
	}

	vec3d pe_pos; //TODO actually calc this here and not once per particle in applytoparticleinfo...

	if (m_distanceCulled > 0.f) {
		float min_dist = 125.0f;
		float dist = 0.0f;//vm_vec_dist_quick(&pe_pos, &Eye_position) / m_distanceCulled;
		if (dist > min_dist)
			particle_percent *= min_dist / dist;
	}

	vec3d sourceDir = getNewDirection(source);
	matrix sourceDirMatrix;
	vm_vector_2_matrix(&sourceDirMatrix, &sourceDir, nullptr, nullptr);

	if (source->getOrigin()->getType() == SourceOriginType::BEAM) {
		// beam particle numbers are per km
		object* b_obj = source->getOrigin()->getObjectHost();
		float dist = vm_vec_dist(&Beams[b_obj->instance].last_start, &Beams[b_obj->instance].last_shot) / 1000.0f;
		particle_percent *= dist;
	}

	// This uses the internal features of the timing class for determining if and how many effects should be triggered
	// this frame
	util::EffectTiming::TimingState time_state;
	for (int time_since_creation = m_timing.shouldCreateEffect(source, time_state); time_since_creation >= 0; time_since_creation = m_timing.shouldCreateEffect(source, time_state)) {
		float interp = static_cast<float>(time_since_creation)/(f2fl(Frametime) * 1000.0f);

		float num = m_particleNum.next() * particle_percent;
		unsigned int num_spawn;

		if (num >= 1.f){
			num_spawn = static_cast<unsigned int>(num);
		}
		else {
			num_spawn = (num >= frand() ? 1 : 0);
		}


		for (uint i = 0; i < num_spawn; ++i) {
			particle_info info;

			source->getOrigin()->applyToParticleInfo(info, m_particleProperties.m_parent_local, interp, m_particleProperties.m_manual_offset);

			info.vel *= m_vel_inherit.next();

			vec3d velocity = ZERO_VECTOR;
			vec3d localPos = ZERO_VECTOR;

			if (m_spawnVolume != nullptr) {
				localPos += m_spawnVolume->sampleRandomPoint(sourceDirMatrix);
			}

			if (m_velocityVolume != nullptr) {
				velocity += m_velocityVolume->sampleRandomPoint(sourceDirMatrix);
			}

			if (m_vel_inherit_from_position.has_value()){
				velocity += localPos * m_vel_inherit_from_position->next();
			}

			if (m_velocity_scaling != VelocityScaling::NONE) {
				// Scale the vector with a random velocity sample and also multiply that with cos(angle between
				// info.vel and sourceDir) That should produce good looking directions where the maximum velocity is
				// only achieved when the particle travels directly on the normal/reflect vector
				float dot = vm_vec_dot(&velocity, &sourceDir);
				vm_vec_scale(&velocity, m_velocity_scaling == VelocityScaling::DOT ? dot : 1.f / std::max(0.001f, dot));
			}

			vm_vec_scale(&velocity, m_velocity.next());

			info.pos += localPos;
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

	// Continue processing this source
	return true;
}

void OmniParticleEffect::pageIn() {
	m_particleProperties.pageIn();
}

}
}