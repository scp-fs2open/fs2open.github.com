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
	int m_particleBitmap = -1;
	float m_range = -1;

 public:
	ParticleEmitterEffect();

	bool processSource(const ParticleSource* source) override;

	void parseValues(bool nocreate) override;

	void pageIn() override;

	void setValues(const particle_emitter& emitter, int bitmap, float range);
};
}
}

#endif // PARTICLE_EMITTER_EFFECT_H
