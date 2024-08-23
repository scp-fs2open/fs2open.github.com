#pragma once

#include "particle/particle.h"
#include "utils/RandomRange.h"

namespace particle {
namespace util {

enum class RotationType { DEFAULT, RANDOM, SCREEN_ALIGNED };

/**
 * @brief The creation properties of a particle
 *
 * This can be used by effects to store range values for particle creation
 *
 * @ingroup particleUtils
 */
class ParticleProperties {
private:
	/**
	 * @brief Choose particle from bitmap list
	 */
	int chooseBitmap();

 public:

	SCP_vector<int> m_bitmap_list;
	::util::UniformRange<size_t> m_bitmap_range;
	::util::ParsedRandomFloatRange m_radius;
	bool m_parentLifetime = false;
	bool m_parentScale = false;
	bool m_hasLifetime = false;
	::util::ParsedRandomFloatRange m_lifetime;
	::util::ParsedRandomFloatRange m_length;
	int m_size_lifetime_curve;
	int m_vel_lifetime_curve;
	RotationType m_rotation_type;
	vec3d m_manual_offset;
	bool m_parent_local = false;

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
	void createParticle(particle_info& info);

	/**
	 * @brief Creates a particle with the stored values
	 * @param info The base values of the particle. Some values will be overwritten by this function
	 * @return The created particle
	 */
	WeakParticlePtr createPersistentParticle(particle_info& info);

	void pageIn();
};
}
}
