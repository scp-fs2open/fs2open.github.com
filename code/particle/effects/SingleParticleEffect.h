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

 public:
	explicit SingleParticleEffect(const SCP_string& name);

	virtual bool processSource(const ParticleSource* source) SCP_OVERRIDE;

	virtual void parseValues(bool nocreate) SCP_OVERRIDE;

	virtual void pageIn() SCP_OVERRIDE;

	virtual void initializeSource(ParticleSource& source) SCP_OVERRIDE;

	virtual EffectType getType() const SCP_OVERRIDE { return EffectType::Single; }

	util::ParticleProperties& getProperties() { return m_particleProperties; }

	static SingleParticleEffect* createInstance(int effectID, float minSize, float maxSize,
												float lifetime = -1.0f);
};
}
}

#endif // SINGLE_PARTICLE_EFFECT_H
