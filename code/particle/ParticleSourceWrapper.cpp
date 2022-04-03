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

	void ParticleSourceWrapper::moveToObject(object* obj, vec3d* d)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->moveToObject(obj, d);
		}
	}

	void ParticleSourceWrapper::moveTo(vec3d* pos)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->moveTo(pos);
		}
	}

	void ParticleSourceWrapper::setVelocity(vec3d* vel)
	{
		for (auto& source : m_sources)
		{
			source->getOrigin()->setVelocity(vel);
		}
	}

	void ParticleSourceWrapper::setOrientationFromNormalizedVec(vec3d* normalizedDir, bool relative)
	{
		for (auto& source : m_sources)
		{
			source->getOrientation()->setFromNormalizedVector(*normalizedDir, relative);
		}
	}


	void ParticleSourceWrapper::setOrientationFromVec(vec3d* dir, bool relative)
	{
		for (auto& source : m_sources)
		{
			source->getOrientation()->setFromVector(*dir, relative);
		}
	}

	void ParticleSourceWrapper::setOrientationMatrix(matrix* mtx, bool relative)
	{
		for (auto& source : m_sources)
		{
			source->getOrientation()->setFromMatrix(*mtx, relative);
		}
	}

	void ParticleSourceWrapper::setOrientationNormal(vec3d* normal)
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
