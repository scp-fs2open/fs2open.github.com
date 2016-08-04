
#ifndef BEAM_PIERCING_EFFECT_H
#define BEAM_PIERCING_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"

namespace particle {
namespace effects {
/**
 * @ingroup particleEffects
 */
class BeamPiercingEffect: public ParticleEffect {
 private:
	float m_radius;
	float m_velocity;
	float m_backVelocity;
	float m_variance;

	int m_effectBitmap;

 public:
	BeamPiercingEffect() : ParticleEffect("") {}

	virtual bool processSource(const ParticleSource* source) override;

	virtual void parseValues(bool nocreate) override;

	virtual void pageIn() override;

	void setValues(int bitmapIndex, float radius, float velocity, float back_velocity, float variance);
};
}
}

#endif // BEAM_PIERCING_EFFECT_H
