#ifndef VOLUME_EFFECT_H
#define VOLUME_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/util/ParticleProperties.h"
#include "particle/util/EffectTiming.h"

namespace particle {
	namespace effects {
		/**
		 * @ingroup particleEffects
		 */
		class VolumeEffect : public ParticleEffect {
		private:
			util::ParticleProperties m_particleProperties;

			float m_radius = 10.0f;
			float m_bias = 1.0f;
			float m_stretch = 1.0f;
			util::EffectTiming m_timing;
			::util::UniformUIntRange m_particleNum;

			::util::UniformFloatRange m_velocity;

		public:
			explicit VolumeEffect(const SCP_string& name);

			bool processSource(ParticleSource* source) override;

			void parseValues(bool nocreate) override;

			void pageIn() override;

			void initializeSource(ParticleSource& source) override;

			EffectType getType() const override { return EffectType::Volume; }

			util::ParticleProperties& getProperties() { return m_particleProperties; }
		};
	}
}

#endif // VOLUME_EFFECT_H
