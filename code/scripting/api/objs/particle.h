#pragma once

#include "scripting/ade_api.h"
#include "particle/particle.h"
#include "particle/ParticleSource.h"

namespace scripting {
namespace api {

class particle_h
{
 protected:
	particle::WeakParticlePtr part;
 public:
	particle_h();

	explicit particle_h(const particle::WeakParticlePtr& part_p);

	particle::WeakParticlePtr Get() const;

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_Particle, particle_h);

DECLARE_ADE_OBJ(l_ParticleEffect, particle::ParticleEffectHandle);

class particle_source_h
{
  protected:
	particle::ParticleSource* source = nullptr;
	uint32_t sourceValidityCounter = 0;
  public:
	particle_source_h() = default;

	explicit particle_source_h(particle::ParticleSource* source_p, uint32_t sourceValidityCounter_p);

	particle::ParticleSource* Get() const;

	bool isValid() const;
};

DECLARE_ADE_OBJ(l_ParticleSource, particle_source_h);

}
}

