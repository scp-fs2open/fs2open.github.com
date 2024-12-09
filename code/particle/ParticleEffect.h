#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleVolume.h"
#include "utils/RandomRange.h"
#include "utils/id.h"

#include <tl/optional.hpp>

class EffectHost;

//Due to parsing shenanigans in weapons, this needs a forward-declare here
int parse_weapon(int subtype, bool replace, const char *filename);

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
		ONETIME, //!< The effect is active exactly once
		RANGE, //!< The effect is active within a specific time range
		ALWAYS //!< The effect is always active
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

	enum class RotationType : uint8_t {
		DEFAULT,
		RANDOM,
		SCREEN_ALIGNED
	};

 private:
	friend struct ParticleParse;

	friend int ::parse_weapon(int subtype, bool replace, const char *filename);

	SCP_string m_name; //!< The name of this effect

	Duration m_duration;
	RotationType m_rotation_type;
	ShapeDirection m_direction;
	VelocityScaling m_velocity_directional_scaling;

	bool m_affectedByDetail; //Kinda deprecated. Only used by the oldest of legacy effects.
	bool m_parentLifetime;
	bool m_parentScale;
	bool m_hasLifetime;
	bool m_parent_local;
	bool m_keep_anim_length_if_available;
	bool m_vel_inherit_absolute;
	bool m_vel_inherit_from_position_absolute;

	SCP_vector<int> m_bitmap_list;
	::util::UniformRange<size_t> m_bitmap_range;

	::util::ParsedRandomFloatRange m_delayRange;
	::util::ParsedRandomFloatRange m_durationRange;
	::util::ParsedRandomFloatRange m_particlesPerSecond;
	::util::ParsedRandomFloatRange m_particleNum;
	::util::ParsedRandomFloatRange m_radius;
	::util::ParsedRandomFloatRange m_lifetime;
	::util::ParsedRandomFloatRange m_length;
	::util::ParsedRandomFloatRange m_vel_inherit;
	::util::ParsedRandomFloatRange m_velocity_scaling;

	tl::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_orientation;
	tl::optional<::util::ParsedRandomFloatRange> m_vel_inherit_from_position;

	std::shared_ptr<::particle::ParticleVolume> m_velocityVolume;
	std::shared_ptr<::particle::ParticleVolume> m_spawnVolume;

	tl::optional<vec3d> m_manual_offset;

	ParticleEffectHandle m_particleTrail;

	int m_size_lifetime_curve; //TODO replace with curve set
	int m_vel_lifetime_curve; //TODO replace with curve set

	float m_particleChance; //Deprecated. Use particle num random ranges instead.
	float m_distanceCulled; //Kinda deprecated. Only used by the oldest of legacy effects.

	matrix getNewDirection(const matrix& hostOrientation, const tl::optional<vec3d>& normal) const;
 public:
	/**
	 * @brief Initializes the base ParticleEffect
	 * @param name The name this effect should have
	 */
	explicit ParticleEffect(SCP_string name);

	// Use this to recreate deprecated legacy effects from in-engine code.
	// All cases like this should already be handled. You should thus never add new uses of this constructor.
	// This constructor will initialize certain non-parsed values differently to handle legacy effects correctly.
	// Parsing the deprecated -part.tbm effects uses the simple constructor + parseLegacy() instead!
	explicit ParticleEffect(SCP_string name,
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

	void pageIn();

	const SCP_string& getName() const { return m_name; }

	std::pair<TIMESTAMP, TIMESTAMP> getEffectDuration() const;

	float getNextSpawnDelay() const;

	bool isOnetime() const { return m_duration == Duration::ONETIME; }
};

}


#endif // PARTICLE_EFFECT_H

