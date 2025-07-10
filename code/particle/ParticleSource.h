#ifndef PARTICLE_SOURCE_H
#define PARTICLE_SOURCE_H
#pragma once

#include "globalincs/pstypes.h"
#include "object/object.h"
#include "particle/particle.h"
#include "io/timer.h"
#include "particle/EffectHost.h"

#include <optional>

struct weapon;

struct weapon_info;

// Forward declaration so we don't need weapons.h
enum class WeaponState: uint32_t;

namespace particle {
/**
 * The origin type
 */
enum class SourceOriginType {
	NONE, //!< Invalid origin
	VECTOR, //!< World-space offset
	BEAM, //!< A beam
	OBJECT, //!< An object
	SUBOBJECT, //!< A subobject
	TURRET, //!< A turret
	PARTICLE //!< A particle
};

class ParticleEffect;
struct particle_effect_tag {
};
using ParticleEffectHandle = ::util::ID<particle_effect_tag, ptrdiff_t, -1>;

/**
 * @brief The orientation of a particle source
 *
 * Currently only the forward direction vector is useful because the other vectors of the matrix are chosen pretty
 * arbitrarily. This also contains a normal vector if it was specified when creating the source.
 * 
 * A source's SourceOrientation is distinct from its host orientation. The host orientation is either defined in
 * SourceOrigin or gathered from the parent object, depending on host type. Host orientation is applied before position offset.
 * SourceOrientation is applied after position offset.
 * 
 * An orientation can be either relative or global. In relative mode all transforms should be relative to the host
 * object and its orientation. In global mode, all directions are in world-space,
 * and host orientation is overridden completely (though it will still affect the orientation of position offsets).
 * 
 * Normals are always in world-space.
 * 
 */
struct SourceTiming {
	TIMESTAMP m_startTimestamp;
	TIMESTAMP m_nextCreation;
	TIMESTAMP m_endTimestamp;
};

/**
 * @brief A particle source
 *
 * A particle source contains the information about where and for how long particles are created. A particle effect uses
 * this information to create new particles. A particle source has not effect-specific information which means that an
 * effect can only use the information contained in this object.
 *
 * @ingroup particleSystems
 */
class ParticleSource {
 private:
	std::unique_ptr<EffectHost> m_host;

	std::optional<vec3d> m_normal;

	std::optional<float> m_triggerRadius;
	std::optional<float> m_triggerVelocity;

	SCP_vector<SourceTiming> m_timing; //!< The time informations of the particle source

	ParticleEffectHandle m_effect; //!< The effect that is assigned to this source

	static constexpr size_t max_composite_size = 64;

	std::bitset<max_composite_size> m_effect_is_running;

	friend class ParticleEffect;

	static float getEffectRemainingTime(const std::tuple<const ParticleSource&, const size_t&>& source);

	static float getEffectRunningTime(const std::tuple<const ParticleSource&, const size_t&>& source);

	static float getEffectVisualSize(const std::tuple<const ParticleSource&, const size_t&, const vec3d&>& source);
 public:
	ParticleSource();

	inline ParticleEffectHandle getEffectHandle() const { return m_effect; }

	const SCP_vector<ParticleEffect>& getEffect() const;

	inline void setEffect(ParticleEffectHandle eff) {
		Assert(eff.isValid());
		m_effect = eff;
	}

	/**
	 * @brief Finishes the creation of a particle source
	 *
	 * This is used to initialize some status that is only available after everything has been set up.
	 */
	void finishCreation();

	/**
	 * @brief Does one processing step for this source
	 * @return @c true if the source should continue to be processed
	 */
	bool process();

	/**
	 * @brief Determines if the source is valid
	 * @return @c true if the source is valid, @c false otherwise.
	 */
	bool isValid() const;

	void setNormal(const vec3d& normal);

	void setTriggerRadius(float radius);
	void setTriggerVelocity(float velocity);

	void setHost(std::unique_ptr<EffectHost> host);
};
}


#endif // PARTICLE_SOURCE_H

