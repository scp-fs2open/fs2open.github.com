#include "particle/ParticleSourceWrapper.h"
#include "particle/ParticleSource.h"

namespace particle
{
	ParticleSourceWrapper::ParticleSourceWrapper(SCP_vector<ParticleSource*>&& sources) : m_sources(std::move(sources))
	{
	}

	ParticleSourceWrapper::ParticleSourceWrapper(ParticleSource* source)
	{
		m_sources.push_back(source);
	}

	ParticleSourceWrapper::ParticleSourceWrapper(ParticleSourceWrapper&& other) noexcept
	{
		*this = std::move(other);
	}

	ParticleSourceWrapper::~ParticleSourceWrapper()
	{

		// Prevent empty wrapper from causing issues
		if (!m_sources.empty())
		{
			Assertion(m_finished, "Source wrapper wasn't finished! Get a coder!\n"
					"(If you hit this then a coder hasn't properly finished creating a particle source)");
		}
	}

	ParticleSourceWrapper& ParticleSourceWrapper::operator=(ParticleSourceWrapper&& other) noexcept
	{
		std::swap(other.m_sources, m_sources);
		return *this;
	}

	void ParticleSourceWrapper::finish()
	{
		// This function is currently only used for debugging but may be extended to do some final adjustments on the sources in the future
#ifndef NDEBUG
		for (auto& source : m_sources)
		{
			Assertion(source->isValid(), "Source is not valid after initializing! Get a coder!\n"
					"(If you hit this it means that a coder hasn't properly initialized a particle source)");
		}
#endif
		for (auto& source : m_sources)
		{
			source->finishCreation();
		}

		m_finished = true;
	}

	void ParticleSourceWrapper::setCreationTimestamp(int timestamp)
	{
		for (auto& source : m_sources)
		{
			source->getTiming()->setCreationTimestamp(timestamp);
		}
	}

	void ParticleSourceWrapper::moveToParticle(const WeakParticlePtr& ptr)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->moveToParticle(ptr);
		}
	}

	void ParticleSourceWrapper::moveToObject(const object* obj, const vec3d* d)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->moveToObject(obj, d);
		}
	}	

	void ParticleSourceWrapper::moveToSubobject(const object* obj, int subobject, const vec3d* d)
	{
		for (auto& source : m_sources) {
			source->getOrigin()->moveToSubobject(obj, subobject, d);
		}
	}	

	void ParticleSourceWrapper::moveToTurret(const object* obj, int subobject, int fire_pos)
	{
		for (auto& source : m_sources) {
			source->getOrigin()->moveToTurret(obj, subobject, fire_pos);
		}
	}	
	
	void ParticleSourceWrapper::moveToBeam(const object* obj)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->moveToBeam(obj);
		}
	}

	void ParticleSourceWrapper::moveTo(const vec3d* pos, const matrix* orientation)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->moveTo(pos, orientation);
		}
	}

	void ParticleSourceWrapper::setVelocity(const vec3d* vel)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->setVelocity(vel);
		}
	}

	void ParticleSourceWrapper::setOrientationFromNormalizedVec(const vec3d* normalizedDir, bool relative)
	{
		for (auto& source : m_sources)
		{
			source->getOrientation()->setFromNormalizedVector(*normalizedDir, relative);
		}
	}


	void ParticleSourceWrapper::setOrientationFromVec(const vec3d* dir, bool relative)
	{
		for (auto& source : m_sources)
		{
			source->getOrientation()->setFromVector(*dir, relative);
		}
	}

	void ParticleSourceWrapper::setOrientationMatrix(const matrix* mtx, bool relative)
	{
		for (auto& source : m_sources)
		{
			source->getOrientation()->setFromMatrix(*mtx, relative);
		}
	}

	void ParticleSourceWrapper::setOrientationNormal(const vec3d* normal)
	{
		for (auto& source : m_sources)
		{
			source->getOrientation()->setNormal(*normal);
		}
	}

	void ParticleSourceWrapper::setWeaponState(WeaponState state)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->setWeaponState(state);
		}
	}
}
