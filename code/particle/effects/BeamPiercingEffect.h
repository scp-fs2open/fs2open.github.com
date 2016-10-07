
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
	float m_radius = -1.f;
	float m_velocity = -1.f;
	float m_backVelocity = -1.f;
	float m_variance = -1.f;

	int m_effectBitmap = -1;

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
