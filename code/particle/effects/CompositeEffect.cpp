#include "particle/particle.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleSource.h"
#include "particle/effects/CompositeEffect.h"

namespace particle {
namespace effects {
CompositeEffect::CompositeEffect(const SCP_string& name) : ParticleEffect(name) {}

bool CompositeEffect::processSource(const ParticleSource*) {
	Assertion(false,
			  "Processing a composite source is not supported! This was caused by a coding error, get a coder!");
	return false;
}

void CompositeEffect::parseValues(bool) {
	while (optional_string("+Child effect:")) {
		auto effectId = internal::parseEffectElement();
		if (effectId >= 0) {
			ParticleEffectPtr effect = ParticleManager::get()->getEffect(effectId);

			if (effect->getType() == EffectType::Composite) {
				error_display(0,
							  "A composite effect cannot contain more composite effects! The effect as not been added.");
			}
			else {
				addEffect(effect);
			}
		}
	}
}

void CompositeEffect::pageIn() {
	for (auto& effect : m_childEffects) {
		effect->pageIn();
	}
}

void CompositeEffect::addEffect(ParticleEffectPtr effect) {
	m_childEffects.push_back(effect);
}
}
}