#ifndef FS2_OPEN_EFFECTTIMING_H
#define FS2_OPEN_EFFECTTIMING_H
#pragma once

#include "particle/ParticleSource.h"
#include "utils/RandomRange.h"

namespace particle {
namespace util {

/**
 * @defgroup particleUtils Particle Effect utilities
 *
 * @ingroup particleSystems
 */

/**
 * @brief The possible duration modes
 *
 * @ingroup particleUtils
 */
enum class Duration {
	Onetime, //!< The effect is active exactly once
	Range, //!< The effect is active within a specific time range
	Always //!< The effect is always active
};

/**
 * @brief Class for managing the timing of an effect
 *
 * This allows to use a random time range for the duration of an effect
 *
 * @ingroup particleUtils
 */
class EffectTiming {
 private:
	Duration m_duration;
	::util::UniformFloatRange m_delayRange;
	::util::UniformFloatRange m_durationRange;

	float m_particlesPerSecond = -1.f;
 public:
	struct TimingState {
		bool initial = true;
	};

	EffectTiming();

	/**
     * @brief Applies the timing information to a specific source
     * @param source The source to be modified
     */
	void applyToSource(ParticleSource* source) const;

	/**
     * @brief Determines if processing should continue
     * @param source The source which should be checked
     * @return @c true if processing should contine, @c false otherwise
     */
	bool continueProcessing(const ParticleSource* source) const;

	/**
	 * @brief Given the properties of this timing structure, determine if a new effect should be created for a source.
	 * @param source The source to check.
	 * @param localState Internal state used by this function that is needed multiple times. Default construct
	 * TimingState and then pass it to this function.
	 * @return @c true if a new effect should be created, @c false. This might return @c true multiple times per frame.
	 */
	bool shouldCreateEffect(ParticleSource* source, TimingState& localState) const;

	/**
     * @brief Parses an effect timing class
     * @return The parsed effect timing
     */
	static EffectTiming parseTiming();
};
}
}


#endif //FS2_OPEN_EFFECTTIMING_H
