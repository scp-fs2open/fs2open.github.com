#include "particle/particle.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleSource.h"
#include "particle/effects/VolumeEffect.h"

#include "bmpman/bmpman.h"
#include "weapon/beam.h"

/**
 * @defgroup particleEffects Particle Effects
 *
 * @ingroup particleSystems
 */

namespace particle {
	namespace effects {
		VolumeEffect::VolumeEffect(const SCP_string& name)
			: ParticleEffect(name) {}

		bool VolumeEffect::processSource(ParticleSource* source) {
			if (!m_timing.continueProcessing(source)) {
				return false;
			}

			// This uses the internal features of the timing class for determining if and how many effects should be
			// triggered this frame
			util::EffectTiming::TimingState time_state;
			for (int time_since_creation = m_timing.shouldCreateEffect(source, time_state); time_since_creation >= 0; time_since_creation = m_timing.shouldCreateEffect(source, time_state)) {
				float interp = static_cast<float>(time_since_creation)/(f2fl(Frametime) * 1000.0f);

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

				vec3d stretch_dir = source->getOrientation()->getDirectionVector(source->getOrigin(), m_particleProperties.m_parent_local);
				matrix stretch_matrix = vm_stretch_matrix(&stretch_dir, m_stretch);

				for (uint i = 0; i < num; ++i) {
					if (m_particleChance < 1.0f) {
						auto roll = m_particleRoll.next();
						if (roll <= 0.0f)
							continue;
					}

					vec3d pos;
					// get an unbiased random point in the sphere
					vm_vec_random_in_sphere(&pos, &vmd_zero_vector, 1.0f, false);

					// maybe bias it towards the center or edge
					if (m_bias != 1.0f) {
						float mag = vm_vec_mag(&pos);
						vm_vec_normalize(&pos);
						mag = powf(mag, m_bias);
						pos *= mag;
					}

					// maybe stretch it
					vec3d copy_pos = pos;
					if (m_stretch != 1.0f)
						vm_vec_rotate(&pos, &copy_pos, &stretch_matrix);

					particle_info info;
					source->getOrigin()->applyToParticleInfo(info, m_particleProperties.m_parent_local, interp, m_particleProperties.m_manual_offset);

					// make their velocity radial, and based on position, allows for some very cool effects
					vec3d velocity = pos;
					pos *= m_radius;
					info.pos += pos;

					vm_vec_scale(&velocity, m_velocity.next());

					info.vel *= m_vel_inherit.next();
					info.vel += velocity;

					m_particleProperties.createParticle(info);
				}
			}

			// Continue processing this source
			return true;
		}

		void VolumeEffect::parseValues(bool nocreate) {
			m_particleProperties.parse(nocreate);

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
						m_name.c_str(),
						chance);
				} else if (chance >= 1.0f) {
					error_display(0,
						"Particle %s tried to set +Chance: %f\nChances above 1 are ignored, please use +Number: "
						"(min,max) to spawn multiple particles.",
						m_name.c_str(),
						chance);
					chance = 1.0f;
				}
				m_particleChance = chance;
			}
			m_particleRoll = ::util::UniformFloatRange(m_particleChance - 1.0f, m_particleChance);

			if (internal::required_string_if_new("+Volume radius:", nocreate)) {
				float radius;
				stuff_float(&radius);

				if (radius < 0.001f) {
					error_display(0, "A volume radius of %f is not valid. Must be greater than 0. Defaulting to 10.", radius);
					radius = 10.0f;
				}
				m_radius = radius;
			}

			if (optional_string("+Bias:")) {
				float bias;
				stuff_float(&bias);

				if (bias < 0.001f) {
					error_display(0, "A volume bias value of %f is not valid. Must be greater than 0.", bias);
					bias = 1.0f;
				}
				m_bias = bias;
			}

			if (optional_string("+Stretch:")) {
				float stretch;
				stuff_float(&stretch);

				if (stretch < 0.001f) {
					error_display(0, "A volume stretch value of %f is not valid. Must be greater than 0.", stretch);
					stretch = 1.0f;
				}
				m_stretch = stretch;
			}

			if (optional_string("+Parent Velocity Factor:")) {
				m_vel_inherit = ::util::ParsedRandomFloatRange::parseRandomRange();
			}

			m_timing = util::EffectTiming::parseTiming();
		}

		void VolumeEffect::pageIn() {
			m_particleProperties.pageIn();
		}

		void VolumeEffect::initializeSource(ParticleSource& source) {
			m_timing.applyToSource(&source);
		}
	}
}
