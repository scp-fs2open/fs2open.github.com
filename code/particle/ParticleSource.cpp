#include <math/bitarray.h>
#include <math/curve.h>
#include "freespace.h"
#include "particle/ParticleSource.h"
#include "weapon/weapon.h"
#include "ParticleSource.h"
#include "ship/ship.h"

namespace particle {

void SourceOrigin::setVelocity(const vec3d* vel) {
	Assertion(vel, "Invalid vector pointer passed!");
	if (!vel)
		return;

	m_velocity = *vel;
}

void SourceOrigin::setWeaponState(WeaponState state) {
	m_weaponState = state;
}

void SourceOrigin::moveTo(const vec3d* pos, const matrix* orientation) {
	Assertion(pos, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::VECTOR;
	m_origin.m_host_orientation = *orientation;
	m_origin.m_pos = *pos;
}

void SourceOrigin::moveToBeam(const object* objp) {
	Assertion(objp, "Invalid object pointer passed!");

	m_originType = SourceOriginType::BEAM;
	m_origin.m_object = object_h(OBJ_INDEX(objp));
}

void SourceOrigin::moveToObject(const object* objp, const vec3d* offset) {
	Assertion(objp, "Invalid object pointer passed!");
	Assertion(offset, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::OBJECT;
	m_origin.m_object = object_h(OBJ_INDEX(objp));

	m_offset = *offset;
}

void SourceOrigin::moveToSubobject(const object* objp, int subobject, const vec3d* offset)
{
	Assertion(objp, "Invalid object pointer passed!");
	Assertion(offset, "Invalid vector pointer passed!");

	m_originType = SourceOriginType::SUBOBJECT;
	m_origin.m_object = object_h(OBJ_INDEX(objp));
	m_origin.m_subobject = subobject;

	m_offset = *offset;
}

void SourceOrigin::moveToTurret(const object* objp, int subobject, int fire_pos)
{
	Assertion(objp, "Invalid object pointer passed!");

	m_originType = SourceOriginType::TURRET;
	m_origin.m_object = object_h(OBJ_INDEX(objp));
	m_origin.m_subobject = subobject;
	m_origin.m_fire_pos = fire_pos;

	m_offset = vmd_zero_vector;
}

void SourceOrigin::moveToParticle(const WeakParticlePtr& weakParticlePtr) {
	m_originType = SourceOriginType::PARTICLE;
	m_origin.m_particle = weakParticlePtr;
}

ParticleSource::ParticleSource() : m_normal(tl::nullopt), m_effect(ParticleEffectHandle::invalid()), m_processingCount(0) {
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
		TIMESTAMP begin = _timestamp(effect.m_timing.m_delayRange.next() * 1000.0f);
		TIMESTAMP end;
		if (effect.m_timing.m_duration == util::Duration::Always)
			end = TIMESTAMP::never();
		else
			end = timestamp_delta(begin, effect.m_timing.m_durationRange.next() * 1000.0f);
		m_timing.emplace_back(SourceTiming{begin, end});
	}
}

bool ParticleSource::process() {
	//TODO make more efficient / get rid of the horror that is the singleton pattern
	const auto& effectList = ParticleManager::get()->getEffect(m_effect);

	const vec3d& vel = m_host->getVelocity();
	const auto& [parent, parent_sig] = m_host->getParentObjAndSig();
	float radius = m_host->getScale();
	float lifetime = m_host->getLifetime();
	float particleMultiplier = m_host->getParticleMultiplier();

	bool result = false;
	for (size_t i = 0; i < effectList.size(); i++) {
		const auto& effect = effectList[i];
		auto& timing = m_timing[i];
		if (m_effect_is_running[i]) {
			bool needs_to_continue_running = true;

			while(timestamp_elapsed(timing.m_nextCreation)) {
				//Find "time" in last frame where particle spawned
				float interp = static_cast<float>(timestamp_since(timing.m_nextCreation)) / (f2fl(Frametime) * 1000.0f);

				// Calculate next spawn
				auto secondsPerParticle = 1.0f / effect.m_timing.m_particlesPerSecond.next();
				// we need to clamp this to 1 because a spawn delay lower than it takes to spawn the particle in ms means we try to spawn infinite particles
				auto time_diff_ms = std::max(fl2i(secondsPerParticle * MILLISECONDS_PER_SECOND), 1);
				timing.m_nextCreation = timestamp_delta(timing.m_nextCreation, time_diff_ms);

				effect.processSource(interp, m_host, m_normal, vel, parent, parent_sig, lifetime, radius, particleMultiplier);

				bool isDone = effect.m_timing.m_duration == util::Duration::Onetime || timestamp_compare(timing.m_endTimestamp, timing.m_nextCreation) < 0;

				m_effect_is_running[i] = !isDone;
				needs_to_continue_running = !isDone;
			}
			result |= needs_to_continue_running;
		}
	}

	++m_processingCount;
	return result;
}

void ParticleSource::setNormal(const vec3d& normal) {
	m_normal = normal;
}
}
