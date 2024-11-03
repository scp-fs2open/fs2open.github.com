#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleVolume.h"
#include "particle/util/ParticleProperties.h"
#include "utils/RandomRange.h"
#include "utils/id.h"

#include <tl/optional.hpp>

class EffectHost;

namespace particle {

struct particle_effect_tag {
};
using ParticleEffectHandle = ::util::ID<particle_effect_tag, ptrdiff_t, -1>;

class ParticleSource;

/**
 * @brief Defines a particle effect
 *
 * A particle effect contains all data necessary for spawning the particles from a particle source. A source can't
 * store effect specific data so a particle effect has to be state-less. All additions to particles should be done
 * within this class. This allows all features to be available for all particles.
 *
 * @ingroup particleSystems
 */
class ParticleEffect {
public:
	enum class Duration : uint8_t {
		Onetime, //!< The effect is active exactly once
		Range, //!< The effect is active within a specific time range
		Always //!< The effect is always active
	};

	enum class ShapeDirection : uint8_t {
		ALIGNED,
		HIT_NORMAL,
		REFLECTED,
		REVERSE
	};

	enum class VelocityScaling : uint8_t {
		NONE,
		DOT,
		DOT_INVERSE
	};

 private:
	//TODO reorder fields to minimize padding
	SCP_string m_name; //!< The name of this effect

	friend struct ParticleParse;

	Duration m_duration;
	::util::ParsedRandomFloatRange m_delayRange;
	::util::ParsedRandomFloatRange m_durationRange;
	::util::ParsedRandomFloatRange m_particlesPerSecond;

	util::ParticleProperties m_particleProperties;

	::util::ParsedRandomFloatRange m_particleNum;

	ShapeDirection m_direction;

	::util::ParsedRandomFloatRange m_vel_inherit;

	bool m_vel_inherit_absolute;

	std::shared_ptr<::particle::ParticleVolume> m_velocityVolume;

	::util::ParsedRandomFloatRange m_velocity_scaling;

	VelocityScaling m_velocity_directional_scaling;

	tl::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_orientation;

	tl::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_position;

	bool m_vel_inherit_from_position_absolute;

	std::shared_ptr<::particle::ParticleVolume> m_spawnVolume;

	ParticleEffectHandle m_particleTrail;

	//Bad legacy flags. Get rid off, or at least don't expose in new table.
	float m_particleChance;

	bool m_affectedByDetail;

	float m_distanceCulled;

	matrix getNewDirection(const matrix& hostOrientation, const tl::optional<vec3d>& normal) const;
 public:
	/**
	 * @brief Initializes the base ParticleEffect
	 * @param name The name this effect should have
	 */
	explicit ParticleEffect(const SCP_string& name);

	// Use this to recreate deprecated legacy effects from in-engine code.
	// Parsing the deprecated -part.tbm effects uses the simple constructor + parseLegacy() instead!
	explicit ParticleEffect(const SCP_string& name,
							::util::ParsedRandomFloatRange particleNum,
							ShapeDirection direction,
							::util::ParsedRandomFloatRange vel_inherit,
							bool vel_inherit_absolute,
							std::shared_ptr<::particle::ParticleVolume> velocityVolume,
							::util::ParsedRandomFloatRange velocity_scaling,
							VelocityScaling velocity_directional_scaling,
							tl::optional<::util::ParsedRandomFloatRange> vel_inherit_from_orientation,
							tl::optional<::util::ParsedRandomFloatRange> vel_inherit_from_position,
							std::shared_ptr<::particle::ParticleVolume> spawnVolume,
							ParticleEffectHandle particleTrail,
							float particleChance,
							bool affectedByDetail,
							float distanceCulled,
							bool disregardAnimationLength,
							::util::ParsedRandomFloatRange lifetime,
							::util::ParsedRandomFloatRange radius,
							int bitmap
	);

	void processSource(float interp, const std::unique_ptr<EffectHost>& host, const tl::optional<vec3d>& normal, const vec3d& vel, int parent, int parent_sig, float lifetime, float radius, float particle_percent) const;

	void parseValues(bool nocreate);

	void pageIn();

	const SCP_string& getName() const { return m_name; }

	std::pair<TIMESTAMP, TIMESTAMP> getEffectDuration() const;

	float getNextSpawnDelay() const;

	bool isOnetime() const { return m_duration == Duration::Onetime; }
};

}


#endif // PARTICLE_EFFECT_H

