#ifndef SINGLE_PARTICLE_EFFECT_H
#define SINGLE_PARTICLE_EFFECT_H
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
class SingleParticleEffect: public ParticleEffect {
 private:
	util::ParticleProperties m_particleProperties;

	util::EffectTiming m_timing;

	float m_vel_inherit = 0.0f;

 public:
	explicit SingleParticleEffect(const SCP_string& name);

	bool processSource(ParticleSource* source) override;

	void parseValues(bool nocreate) override;

	void pageIn() override;

	void initializeSource(ParticleSource& source) override;

	EffectType getType() const override { return EffectType::Single; }

	util::ParticleProperties& getProperties() { return m_particleProperties; }

	static SingleParticleEffect* createInstance(int effectID, float minSize, float maxSize,
												float lifetime = -1.0f);
};
}
}

#endif // SINGLE_PARTICLE_EFFECT_H
