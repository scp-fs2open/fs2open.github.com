#pragma once

#include "particle/particle.h"
#include "utils/RandomRange.h"

namespace particle {
namespace util {
/**
 * @brief The creation properties of a particle
 *
 * This can be used by effects to store range values for particle creation
 *
 * @ingroup particleUtils
 */
class ParticleProperties {
 public:
	int m_bitmap = -1;
	::util::UniformFloatRange m_radius;

	bool m_hasLifetime = false;
	::util::UniformFloatRange m_lifetime;

	ParticleProperties();

	/**
	 * @brief Parses the particle values
	 * @param nocreate @c true if +nocreate was found
	 */
	void parse(bool nocreate);

	/**
	 * @brief Creates a particle with the stored values
	 * @param info The base values of the particle. Some values will be overwritten by this function
	 * @return The created particle
	 */
	WeakParticlePtr createParticle(particle_info& info);

	void pageIn();
};
}
}
