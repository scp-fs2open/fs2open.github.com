#ifndef PARTICLE_EFFECT_H
#define PARTICLE_EFFECT_H
#pragma once

#include "globalincs/pstypes.h"

namespace particle {
class ParticleSource;

/**
 * @brief The type of an Effect
 * If you add a new effect type you need to add it to the enum.
 *
 * @ingroup particleSystems
 */
enum class EffectType: int64_t {
	Invalid = -1,
	Single,
	Composite,
	Cone,
	Sphere,
	Volume,

	MAX,
};

/**
 * @brief Defines a particle effect
 *
 * A particle effect contains all data necessary for spawning the particles for one particle source. A source can't
 * store effect specific data so a particle effect has to be state-less. To add your own particle effects just extend
 * this class and override the functions you need. If the effect type should be available for table creation
 * take a look at constructEffect in ParticleManager.cpp.
 *
 * @ingroup particleSystems
 */
class ParticleEffect {
 protected:
	SCP_string m_name; //!< The name if this effect

 public:
	/**
	 * @brief Initializes the base ParticleEffect
	 * @param name The name this effect should have
	 */
	explicit ParticleEffect(const SCP_string& name) : m_name(name) {}

	virtual ~ParticleEffect() {}

	const SCP_string& getName() const { return m_name; }

	/**
	 * @brief Parses the values of this effect
	 *
	 * @note If your effect should be available for usage in tables you can use this function for parsing the values. The
	 * usual parsing functions are available.
	 *
	 * @param nocreate
	 */
	virtual void parseValues(bool  /*nocreate*/) {}

	/**
	 * @brief Page in used effects
	 *
	 * @note This is called at mission start to determine which textures are used by this effect. Use #bm_page_in_texture to
	 * specify which textures will be used.
	 */
	virtual void pageIn() {}

	/**
	 * @brief Process a particle source
	 *
	 * @note This is the main function of the effect. In this function the implementation should generate new particles
	 * according to its configuration. The return value is used to determine if the source should continue to exist.
	 * Return @c true if the source should be processed in the next frame, return @c false if this is effect is done.
	 *
	 * @warning Implementations of this function must be able to handle multiple calls to this function even if a
	 * previous call returned @c false.
	 *
	 * @param source The source to process
	 * @return @c true if the effect should continue to be processed, @c false if the effect is done.
	 */
	virtual bool processSource(ParticleSource* source) = 0;

	/**
	 * @brief Initializes the source for this effect
	 *
	 * @note Implementations can use this function to apply one-time operations to a source. This could be used to set
	 * the lifetime of a source once if the effect supports it. See #SingleParticleEffect for an example of how this
	 * could be used.
	 *
	 * @param source The source to be initialized
	 */
	virtual void initializeSource(ParticleSource&  /*source*/) {}

	/**
	 * @brief Gets the type of this effect
	 *
	 * @note Implementations should override this and return their own type if possible
	 *
	 * @return The effect type.
	 */
	virtual EffectType getType() const { return EffectType::Invalid; }
};

/**
 * A particle pointer.
 */
typedef ParticleEffect* ParticleEffectPtr;
}


#endif // PARTICLE_EFFECT_H

