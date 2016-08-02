#ifndef CONE_GENERATOR_EFFECT_H
#define CONE_GENERATOR_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "particle/util/RandomRange.h"
#include "particle/util/ParticleProperties.h"

namespace particle {
namespace effects {
enum class ConeDirection {
	Incoming,
	Normal,
	Reflected,
	Reverse
};

/**
 * @ingroup particleEffects
 */
class ConeGeneratorEffect: public ParticleEffect {
 private:
	util::ParticleProperties m_particleProperties;

	ConeDirection m_direction = ConeDirection::Incoming;
	util::NormalFloatRange m_normalDeviation;
	util::UniformFloatRange m_velocity;
	util::UniformUIntRange m_particleNum;

	ParticleEffectIndex m_particleTrail;

	vec3d getNewDirection(const ParticleSource* source) const;

	matrix getDisplacementMatrix();

 public:
	explicit ConeGeneratorEffect(const SCP_string& name);

	virtual bool processSource(const ParticleSource* source) override;

	virtual void parseValues(bool nocreate) override;

	virtual EffectType getType() const override { return EffectType::Cone; }

	virtual void pageIn() override;
};
}
}

#endif // CONE_GENERATOR_EFFECT_H
