#ifndef PARTICLE_SOURCE_H
#define PARTICLE_SOURCE_H
#pragma once

#include "globalincs/pstypes.h"

#include "object/object.h"

#include "particle/particle.h"
#include "particle/ParticleEffect.h"

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

class ParticleSource;

class ParticleEffect;

/**
 * @brief A source origin
 *
 * The SourceOrigin class encapsualtes the information about where a source is located. It allows a source to be
 * relative to an object or a particle and keeps track of whether the host objects are still valid.
 */
class SourceOrigin {
 private:
	SourceOriginType m_originType;

	struct {
		vec3d m_pos;

		// this is used only for vector-type sources, and is not used when the effect's parent is an object
		// when the parent is an object, we get the relevant orientation from the object, so that it stays updated
		// when the parent is an object, we get the relevant orientation from the object, so that it stays updated
		matrix m_host_orientation;

		object_h m_object;

		int m_subobject;

		int m_fire_pos;

		WeakParticlePtr m_particle;
	} m_origin;

	WeaponState m_weaponState;

	vec3d m_offset;

	vec3d m_velocity;

 public:
	/**
	 * @brief Initializes the origin with default values
	 */
	SourceOrigin();

	/**
	 * @brief Gets the current, global position of the origin
	 * 
	 * For object sources, can interpolate between object's current position and its position last frame
	 * 
	 * Be aware for beam sources this will give *random* points along its length
	 * 
	 * @param posOut The pointer where the location will be stored
	 * 
	 * @param interp For objects, the point's position between the current-frame
	 * and last-frame positions of the object, with 0.0 being current-frame and 1.0 being last-frame
	 */
	void getGlobalPosition(vec3d* posOut, float interp = 1.0f, tl::optional<vec3d> manual_offset = tl::nullopt) const;

	void getHostOrientation(matrix* matOut, bool allow_relative) const;

	inline SourceOriginType getType() const { return m_originType; }

	inline object* getObjectHost() const { return m_origin.m_object.objp_or_null(); }

	/**
	 * @brief Determines if the origin is valid
	 *
	 * This checks if the hosting object is valid or if the hosting particle is still valid.
	 *
	 * @return @c true if the origin is valid, @c false otherwise
	 */
	bool isValid() const;

	/**
	 * @brief Applies the information to a particle info
	 *
	 * This fills the provided &info with certain data from the origin, such as objnum/signature of an object origin,
	 * the scale and remaining lifetime of a particle origin, or simply the current global position of the origin
	 *
	 * @param info The particle_info this should be applied to
	 */
	void applyToParticleInfo(particle_info& info, bool allowRelative = false, float interp = 1.0f, tl::optional<vec3d> manual_offset = tl::nullopt) const;

	/**
	 * @brief Gets the velocity of the origin host
	 * @return The velocity of the host
	 */
	vec3d getVelocity() const;

	/**
	 * @brief Gets the current scale of the origin host, always 1 for non-particles
	 * @return The scale of the host
	 */
	float getScale() const;

	/**
	 * @brief Gets the remaining lifetime of the origin host, always -1.0 for non-particles
	 * @return The lifeleft of the host
	 */
	float getLifetime() const;

	/**
	 * @brief Sets the weapon state in which this origin is valid
	 * @param state The weapon state to use
	 */
	void setWeaponState(WeaponState state);

	/**
	 * @brief Moves the source to the specified world location
	 * @param pos The world position
	 */
	void moveTo(const vec3d* pos, const matrix* orientation = &vmd_identity_matrix);

	/**
	 * @brief Moves the source to the specified beam object
	 * @param objp The hosting beam
	 */
	void moveToBeam(const object* objp);

	/**
	 * @brief Moves the source to the specified object with an offset
	 * @param objp The hosting object
	 * @param offset The position relative to this object
	 */
	void moveToObject(const object* objp, const vec3d* offset);

		/**
	 * @brief Moves the source to the specified object with an offset
	 * @param objp The hosting object
	 * @param offset The position relative to this object
	 */
	void moveToSubobject(const object* objp, int subobject, const vec3d* offset);

	/**
	 * @brief Moves the source to the specified object with an offset
	 * @param objp The hosting object
	 * @param offset The position relative to this object
	 */
	void moveToTurret(const object* objp, int subobject, int fire_pos);

	/**
	 * @brief Moves the source to the specified particle
	 * @param weakParticlePtr The hosting particle
	 */
	void moveToParticle(const WeakParticlePtr& weakParticlePtr);

	/**
	* @brief Sets the velocity of the source, will not move the source, but particles created may inherit this velocity
	* @param vel The world velocity
	*/
	void setVelocity(const vec3d* vel);

	friend class ParticleSource;
};

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
};
}


#endif // PARTICLE_SOURCE_H

