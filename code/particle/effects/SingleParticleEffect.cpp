#include "particle/particle.h"
#include "particle/ParticleEffect.h"
#include "particle/ParticleSource.h"
#include "particle/effects/SingleParticleEffect.h"

#include "bmpman/bmpman.h"

/**
 * @defgroup particleEffects Particle Effects
 *
 * @ingroup particleSystems
 */

namespace particle {
namespace effects {
SingleParticleEffect::SingleParticleEffect(const SCP_string& name)
	: ParticleEffect(name) {}

bool SingleParticleEffect::processSource(const ParticleSource* source) {
	if (!m_timing.continueProcessing(source)) {
		return false;
	}

	particle_info info;
	memset(&info, 0, sizeof(info));

	source->getOrigin()->applyToParticleInfo(info);
	info.vel = vmd_zero_vector;

	m_particleProperties.createParticle(info);

	// Continue processing this source
	return true;
}

void SingleParticleEffect::parseValues(bool nocreate) {
	m_particleProperties.parse(nocreate);

	m_timing = util::EffectTiming::parseTiming();
}

void SingleParticleEffect::pageIn() {
	m_particleProperties.pageIn();
}

void SingleParticleEffect::initializeSource(ParticleSource& source) {
	m_timing.applyToSource(&source);
}

SingleParticleEffect* SingleParticleEffect::createInstance(int effectID, float minSize, float maxSize,
														   float lifetime) {
	Assertion(bm_is_valid(effectID), "Invalid effect id %d passed!", effectID);
	Assertion(minSize <= maxSize, "Maximum size %f is more than minimum size %f!", maxSize, minSize);
	Assertion(minSize >= 0.0f, "Minimum size may not be less than zero, got %f!", minSize);

	auto effectPtr = new SingleParticleEffect("");
	effectPtr->m_particleProperties.m_bitmap = effectID;
	effectPtr->m_particleProperties.m_radius = util::UniformFloatRange(minSize, maxSize);

	if (lifetime > 0.0f) {
		effectPtr->m_particleProperties.m_hasLifetime = true;
		effectPtr->m_particleProperties.m_lifetime = util::UniformFloatRange(lifetime);
	}

	return effectPtr;
}
}
}