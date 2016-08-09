#ifndef PARTICLE_PARTICLESOURCEWRAPPER_H
#define PARTICLE_PARTICLESOURCEWRAPPER_H
#pragma once

#include "globalincs/pstypes.h"
#include "ParticleSource.h"

enum class WeaponState : uint32_t;

namespace particle
{
	class ParticleSource;

	/**
	 * @brief A wrapper around multiple particle sources
	 *
	 * This class contains multiple sources which are grouped together. This is needed because effects may create
	 * multiple sources and this class provides transparent handling of that case.
	 *
	 * Once initialization of the sources is done you must call #finish() in order to mark the sources as properly
	 * initialized.
	 *
	 * @ingroup particleSystems
	 */
	class ParticleSourceWrapper
	{
	private:
		SCP_vector<ParticleSource*> m_sources;

		bool m_finished;

	public:
		ParticleSourceWrapper(const ParticleSourceWrapper&) SCP_DELETED_FUNCTION;

		ParticleSourceWrapper& operator=(const ParticleSourceWrapper&) SCP_DELETED_FUNCTION;

		ParticleSourceWrapper() {}
		explicit ParticleSourceWrapper(SCP_vector<ParticleSource*>&& sources);
		explicit ParticleSourceWrapper(ParticleSource* source);

		~ParticleSourceWrapper();

		ParticleSourceWrapper(ParticleSourceWrapper&& other);

		ParticleSourceWrapper& operator=(ParticleSourceWrapper&& other);

		void finish();

		void setCreationTimestamp(int timestamp);

		void moveToParticle(WeakParticlePtr ptr);

		void moveToObject(object* obj, vec3d* localPos);

		void moveTo(vec3d* pos);

		void setOrientationFromNormalizedVec(vec3d* normalizedDir);

		void setOrientationFromVec(vec3d* dir);

		void setOrientationMatrix(matrix* mtx);

		void setOrientationNormal(vec3d* normal);

		void setWeaponState(WeaponState state);
	};
}


#endif //PARTICLE_PARTICLESOURCEWRAPPER_H
