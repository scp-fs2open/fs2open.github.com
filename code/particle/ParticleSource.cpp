#include <math/bitarray.h>
#include "freespace.h"
#include "particle/ParticleSource.h"
#include "weapon/weapon.h"
#include "ship/ship.h"

namespace particle {

ParticleSource::ParticleSource() : m_normal(std::nullopt), m_effect(ParticleEffectHandle::invalid()) {
	for (size_t i = 0; i < 64; i++) {
		m_effect_is_running[i] = true;
	}
}

bool ParticleSource::isValid() const {
	if (!m_effect_is_running.any()) {
		return false;
	}

	if (!m_effect.isValid()) {
		return false;
	}

	if (!m_host->isValid()) {
		return false;
	}

	return true;
}

void ParticleSource::finishCreation() {
	m_host->setupProcessing();

	for (const auto& effect : ParticleManager::get()->getEffect(m_effect)) {
		const auto& [begin, end] = effect.getEffectDuration();
		m_timing.emplace_back(SourceTiming{begin, end});
	}
}

const SCP_vector<ParticleEffect>& ParticleSource::getEffect() const {
	return ParticleManager::get()->getEffect(m_effect);
}

bool ParticleSource::process() {
	Assertion(m_host != nullptr, "Particle Source has no host!");
	const auto& effectList = getEffect();

	const vec3d& vel = m_host->getVelocity();
	const auto& [parent, parent_sig] = m_host->getParentObjAndSig();
	float parent_radius = m_host->getScale();
	float parent_lifetime = m_host->getLifetime();
	float particleMultiplier = m_host->getParticleMultiplier();

	bool result = false;
	for (size_t i = 0; i < effectList.size(); i++) {
		const auto& effect = effectList[i];
		auto& timing = m_timing[i];
		if (m_effect_is_running[i]) {
			bool needs_to_continue_running = true;

			while(timestamp_elapsed(timing.m_nextCreation) && needs_to_continue_running) {
				//Find "time" in last frame where particle spawned
				float interp = static_cast<float>(timestamp_since(timing.m_nextCreation)) / (f2fl(Frametime) * 1000.0f);

				// we need to clamp this to 1 because a spawn delay lower than it takes to spawn the particle in ms means we try to spawn infinite particles
				float freqMult = effect.m_modular_curves.get_output(ParticleEffect::ParticleCurvesOutput::PARTICLE_FREQ_MULT, std::pair<const ParticleSource&, size_t>(*this, i));
				auto time_diff_ms = std::max(fl2i(effect.getNextSpawnDelay() / freqMult * MILLISECONDS_PER_SECOND), 1);
				timing.m_nextCreation = timestamp_delta(timing.m_nextCreation, time_diff_ms);

				//Some of these
				effect.processSource(interp, *this, i, vel, parent, parent_sig, parent_lifetime, parent_radius, particleMultiplier);

				bool isDone = effect.isOnetime() || timestamp_compare(timing.m_endTimestamp, timing.m_nextCreation) < 0;

				m_effect_is_running[i] = !isDone;
				needs_to_continue_running = !isDone;
			}
			result |= needs_to_continue_running;
		}
	}

	return result;
}

void ParticleSource::setNormal(const vec3d& normal) {
	m_normal = normal;
}

void ParticleSource::setTriggerRadius(float radius) {
	m_triggerRadius = radius;
}

void ParticleSource::setTriggerVelocity(float velocity) {
	m_triggerVelocity = velocity;
}

void ParticleSource::setHost(std::unique_ptr<EffectHost> host) {
	m_host = std::move(host);
}

float ParticleSource::getEffectRemainingTime(const std::tuple<const ParticleSource&, const size_t&>& source) {
	return i2fl(timestamp_until(std::get<0>(source).m_timing[std::get<1>(source)].m_endTimestamp)) / i2fl(MILLISECONDS_PER_SECOND);
}
}
