#pragma once

#include "scripting/ade_api.h"
#include "particle/particle.h"

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

}
}

