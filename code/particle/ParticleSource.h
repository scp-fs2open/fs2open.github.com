#ifndef PARTICLE_SOURCE_H
#define PARTICLE_SOURCE_H
#pragma once

#include "globalincs/pstypes.h"

#include "object/object.h"

#include "particle/ParticleManager.h"
#include "particle/particle.h"

#include "io/timer.h"

#include "particle/EffectHost.h"

#include <tl/optional.hpp>

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

	tl::optional<vec3d> m_normal;

	SCP_vector<SourceTiming> m_timing; //!< The time informations of the particle source

	ParticleEffectHandle m_effect; //!< The effect that is assigned to this source

	size_t m_processingCount; //!< The number of times this effect has been processed

	static constexpr size_t max_composite_size = 64;

	std::bitset<max_composite_size> m_effect_is_running;

	void initializeThrusterOffset(weapon* wp, weapon_info* wip);
 public:
	ParticleSource();

	inline ParticleEffectHandle getEffect() { return m_effect; }

	inline void setEffect(ParticleEffectHandle eff) {
		Assert(eff.isValid());
		m_effect = eff;
	}

	inline size_t getProcessingCount() const { return m_processingCount; }

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

	void setHost(std::unique_ptr<EffectHost> host);
};
}


#endif // PARTICLE_SOURCE_H

