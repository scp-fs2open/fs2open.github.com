#ifndef PARTICLE_MANAGER_H
#define PARTICLE_MANAGER_H
#pragma once

#include "globalincs/pstypes.h"
#include "particle/ParticleEffect.h"
#include "utils/id.h"

#include "particle/hosts/EffectHostBeam.h"
#include "particle/hosts/EffectHostObject.h"
#include "particle/hosts/EffectHostParticle.h"
#include "particle/hosts/EffectHostSubmodel.h"
#include "particle/hosts/EffectHostTurret.h"
#include "particle/hosts/EffectHostVector.h"

#include "particle/ParticleSource.h"

namespace particle {

/**
 * The particle index type.
 */

class ParticleSource;

/**
 * @brief Manages high-level particle effects and sources
 *
 * @ingroup particleSystems
 *
 * This class is responsible for managing the loaded effects and processing the created particle sources. The whole effect
 * subsystem is designed to not allocate memory at runtime (except if the vector resizes but that should only happen a few times).
 */
class ParticleManager {
 private:
	SCP_vector<SCP_vector<ParticleEffect>> m_effects; //!< All parsed effects

	SCP_vector<ParticleSource> m_sources; //!< The currently active sources

	bool m_processingSources = false; //!< @c true if sources are currently being processed
	/**
	 * If the sources are currently being processed, no additional sources can be added. Instead, they are added to this
	 * vector and then added to the main vector when processing is done.
	 */
	SCP_vector<ParticleSource> m_deferredSourceAdding;

	/**
	 * The global paticle manager
	 */
	static std::unique_ptr<ParticleManager> m_manager;

	static void parseConfigFiles();

	/**
	 * @brief Creates a source and returns a pointer to it
	 *
	 * This also handles the case when a source is created when current processing the sources
	 *
	 * @warning The pointer is temporary and only valid until the #m_sources is modified!
	 * @return The source pointer
	 */
	ParticleSource* createSource();
 public:
	ParticleManager();

	/**
	 * @brief Initializes the effect system
	 *
	 * This creates the ParticleManager and parses the config files
	 */
	static void init();

	/**
	 * @brief Gets the global particle manager
	 * @return The particle manager
	 */
	static inline ParticleManager* get() {
		Assertion(m_manager != nullptr, "ParticleManager was not properly inited!");

		return m_manager.get();
	}

	/**
	 * @brief Shuts down the particle effect system
	 */
	static void shutdown();

	/**
	 * @brief Gets an effect by index
	 * @param effectID The id of the effect to retrieve
	 * @return The particle effect pointer, will not be @c nullptr
	 */
	inline const SCP_vector<ParticleEffect>& getEffect(ParticleEffectHandle effectID) const
	{
		Assertion(effectID.value() >= 0 &&
		              effectID.value() < static_cast<ParticleEffectHandle::impl_type>(m_effects.size()),
		          "Particle effect index " PTRDIFF_T_ARG " is invalid!", effectID.value());

		return m_effects[effectID.value()];
	}

	/**
	 * @brief Gets an effect by name
	 *
	 * @note If possible, only call this once and then store the index. The lookup is being done by a sequential search
	 * which means it's pretty slow.
	 *
	 * @param name The name of the effect that is being searched, may not be empty
	 * @return The index of the effect
	 */
	ParticleEffectHandle getEffectByName(const SCP_string& name);

	/**
	 * @brief Adds an effect
	 * @param effect The effect to add
	 * @return The index of the added effect
	 */
	ParticleEffectHandle addEffect(ParticleEffect&& effect);

	/**
	 * @brief Adds a composite effect
	 * @param effect The effects of the composite effect to add
	 * @return The index of the added effect
	 */
	ParticleEffectHandle addEffect(SCP_vector<ParticleEffect>&& effect);

	/**
	 * @brief Does one processing step of the particle manager
	 * @param frameTime The length of the current frame
	 */
	void doFrame(float frameTime);

	/**
	 * @brief Removes all sources
	 */
	void clearSources();

	/**
	 * @brief Pages in all used effects
	 */
	void pageIn();

	/**
	 * @brief Creates a source for the specified effect
	 *
	 * This returns a wrapper class because some effects may create multiple sources.
	 *
	 * @param index The index of the effect
	 * @return A wrapper class which allows access to the created sources
	 */
	ParticleSource* createSource(ParticleEffectHandle index);
};

namespace internal {
/**
 * @brief Utility function for required_string
 *
 * If no_create is @c false then this will act like #required_string, otherwise it will be an #optional_string
 *
 * @param token The token to check for
 * @param no_create The no_create value
 * @return @c true if the token was present, @c false otherwise
 */
bool required_string_if_new(const char* token, bool no_create);

/**
 * @brief Parses an animation or list of animations.
 *
 * Parses an animation or list of animations and returns the handle(s). If critical is @c true then a failure to load the animation will cause
 * an error. Otherwise it will be cause a warning.
 *
 * @param critical @c true if a failure is critical
 * @return The vector of animation handles
 */

SCP_vector<int> parseAnimationList(bool critical = true);
}

namespace util {
/**
 * @brief Parses an effect name
 *
 * This can be used to parse an effect from another part of the engine. This parses an effect name and returns the
 * index. If the name is not valid an error is displayed.
 *
 * @param objectName Can be optionally specified so the error message is a bit more specific
 * @return The index
 */
ParticleEffectHandle parseEffect(const SCP_string& objectName = "");
}
}


#endif // PARTICLE_MANAGER_H

