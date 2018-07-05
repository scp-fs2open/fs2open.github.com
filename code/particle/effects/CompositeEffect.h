#ifndef COMPOSIE_EFFECT_H
#define COMPOSIE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleManager.h"
#include "utils/RandomRange.h"

namespace particle {
namespace effects {
/**
 * @ingroup particleEffects
 */
class CompositeEffect: public ParticleEffect {
 private:
	SCP_vector<ParticleEffectPtr> m_childEffects;

 public:
	explicit CompositeEffect(const SCP_string& name);

	bool processSource(const ParticleSource* source) override;

	void parseValues(bool nocreate) override;

	void pageIn() override;

	EffectType getType() const override { return EffectType::Composite; }

	const SCP_vector<ParticleEffectPtr>& getEffects() const { return m_childEffects; }

	void addEffect(ParticleEffectPtr effect);
};
}
}

#endif // COMPOSIE_EFFECT_H
