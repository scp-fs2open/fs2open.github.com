#include "particle/particle.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleSource.h"
#include "particle/effects/VolumeEffect.h"

#include "bmpman/bmpman.h"

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
			while (m_timing.shouldCreateEffect(source, time_state)) {
				auto num = m_particleNum.next();

				vec3d stretch_dir = source->getOrientation()->getDirectionVector(source->getOrigin());
				matrix stretch_matrix = vm_stretch_matrix(&stretch_dir, m_stretch);
				for (uint i = 0; i < num; ++i) {
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
					source->getOrigin()->applyToParticleInfo(info);

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
				m_velocity = ::util::parseUniformRange<float>();
			}

			if (internal::required_string_if_new("+Number:", nocreate)) {
				m_particleNum = ::util::parseUniformRange<uint>();
			}

			if (internal::required_string_if_new("+Volume radius:", nocreate)) {
				float radius;
				stuff_float(&radius);

				if (radius < 0.001f) {
					Warning(LOCATION, "A volume radius of %f is not valid. Must be greater than 0. Defaulting to 10.", radius);
					radius = 10.0f;
				}
				m_radius = radius;
			}

			if (optional_string("+Bias:")) {
				float bias;
				stuff_float(&bias);

				if (bias < 0.001f) {
					Warning(LOCATION, "A volume bias value of %f is not valid. Must be greater than 0.", bias);
					bias = 1.0f;
				}
				m_bias = bias;
			}

			if (optional_string("+Stretch:")) {
				float stretch;
				stuff_float(&stretch);

				if (stretch < 0.001f) {
					Warning(LOCATION, "A volume stretch value of %f is not valid. Must be greater than 0.", stretch);
					stretch = 1.0f;
				}
				m_stretch = stretch;
			}

			if (optional_string("+Parent Velocity Factor:")) {
				m_vel_inherit = ::util::parseUniformRange<float>();
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
