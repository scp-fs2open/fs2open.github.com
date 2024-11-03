#ifndef FS2_OPEN_EFFECTTIMING_H
#define FS2_OPEN_EFFECTTIMING_H
#pragma once

#include "utils/RandomRange.h"

namespace particle {
class ParticleSource; //TODO remove

namespace util {

//TODO Fold this file into the OmniEffect

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
   friend class ::particle::ParticleSource; //TODO remove when tidying up
	Duration m_duration;
	::util::ParsedRandomFloatRange m_delayRange;
	::util::ParsedRandomFloatRange m_durationRange;

	::util::ParsedRandomFloatRange m_particlesPerSecond = ::util::UniformFloatRange(-1.f);
 public:
	struct TimingState {
		bool initial = true;
	};

	EffectTiming();

	/**
     * @brief Parses an effect timing class
     * @return The parsed effect timing
     */
	static EffectTiming parseTiming();
};
}
}


#endif //FS2_OPEN_EFFECTTIMING_H
