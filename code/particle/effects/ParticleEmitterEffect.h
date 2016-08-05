#ifndef PARTICLE_EMITTER_EFFECT_H
#define PARTICLE_EMITTER_EFFECT_H
#pragma once

#include "particle/ParticleEffect.h"
#include "particle/particle.h"

namespace particle {
namespace effects {
/**
 * @ingroup particleEffects
 */
class ParticleEmitterEffect: public ParticleEffect {
 private:
	particle_emitter m_emitter;
	int m_particleBitmap;
	float m_range;

 public:
	ParticleEmitterEffect() : ParticleEffect("") {}

	virtual bool processSource(const ParticleSource* source) override;

	virtual void parseValues(bool nocreate) override;

	virtual void pageIn() override;

	void setValues(const particle_emitter& emitter, int bitmap, float range);
};
}
}

#endif // PARTICLE_EMITTER_EFFECT_H
