//
//

#include "particle.h"
#include "vecmath.h"
#include "object.h"

namespace scripting {
namespace api {

particle_h::particle_h() {
}
particle_h::particle_h(const particle::WeakParticlePtr& part_p) {
	this->part = part_p;
}
particle::WeakParticlePtr particle_h::Get() {
	return this->part;
}
bool particle_h::isValid() {
	return !part.expired();
}


//**********HANDLE: Particle
ADE_OBJ(l_Particle, particle_h, "particle", "Handle to a particle");

ADE_VIRTVAR(Position, l_Particle, "vector", "The current position of the particle (world vector)", "vector", "The current position")
{
	particle_h *ph = NULL;
	vec3d newVec = vmd_zero_vector;
	if (!ade_get_args(L, "o|o", l_Particle.GetPtr(&ph), l_Vector.Get(&newVec)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ph == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!ph->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ADE_SETTING_VAR)
	{
		ph->Get().lock()->pos = newVec;
	}

	return ade_set_args(L, "o", l_Vector.Set(ph->Get().lock()->pos));
}

ADE_VIRTVAR(Velocity, l_Particle, "vector", "The current velocity of the particle (world vector)", "vector", "The current velocity")
{
	particle_h *ph = NULL;
	vec3d newVec = vmd_zero_vector;
	if (!ade_get_args(L, "o|o", l_Particle.GetPtr(&ph), l_Vector.Get(&newVec)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ph == NULL)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!ph->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ADE_SETTING_VAR)
	{
		ph->Get().lock()->velocity = newVec;
	}

	return ade_set_args(L, "o", l_Vector.Set(ph->Get().lock()->velocity));
}

ADE_VIRTVAR(Age, l_Particle, "number", "The time this particle already lives", "number", "The current age or -1 on error")
{
	particle_h *ph = NULL;
	float newAge = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newAge))
		return ade_set_error(L, "f", -1.0f);

	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (newAge >= 0)
			ph->Get().lock()->age = newAge;
	}

	return ade_set_args(L, "f", ph->Get().lock()->age);
}

ADE_VIRTVAR(MaximumLife, l_Particle, "number", "The time this particle can live", "number", "The maximal life or -1 on error")
{
	particle_h *ph = NULL;
	float newLife = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newLife))
		return ade_set_error(L, "f", -1.0f);

	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (newLife >= 0)
			ph->Get().lock()->max_life = newLife;
	}

	return ade_set_args(L, "f", ph->Get().lock()->max_life);
}

ADE_VIRTVAR(Looping, l_Particle, "boolean",
            "The looping status of the particle. If a particle loops then it will not be removed when its max_life "
            "value has been reached. Instead its animation will be reset to the start. When the particle should "
            "finally be removed then set this to false and set MaxLife to 0.",
            "boolean", "The looping status")
{
	particle_h* ph = nullptr;
	bool newloop   = false;
	if (!ade_get_args(L, "o|b", l_Particle.GetPtr(&ph), &newloop))
		return ADE_RETURN_FALSE;

	if (ph == nullptr)
		return ADE_RETURN_FALSE;

	if (!ph->isValid())
		return ADE_RETURN_FALSE;

	if (ADE_SETTING_VAR) {
		ph->Get().lock()->looping = newloop;
	}

	return ade_set_args(L, "b", ph->Get().lock()->looping);
}

ADE_VIRTVAR(Radius, l_Particle, "number", "The radius of the particle", "number", "The radius or -1 on error")
{
	particle_h *ph = NULL;
	float newRadius = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newRadius))
		return ade_set_error(L, "f", -1.0f);

	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (newRadius >= 0)
			ph->Get().lock()->radius = newRadius;
	}

	return ade_set_args(L, "f", ph->Get().lock()->radius);
}

ADE_VIRTVAR(TracerLength, l_Particle, "number", "The tracer legth of the particle", "number", "The radius or -1 on error")
{
	particle_h *ph = NULL;
	float newTracer = -1.0f;
	if (!ade_get_args(L, "o|f", l_Particle.GetPtr(&ph), &newTracer))
		return ade_set_error(L, "f", -1.0f);

	if (ph == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (!ph->isValid())
		return ade_set_error(L, "f", -1.0f);

	// tracer_length has been deprecated
	return ade_set_args(L, "f", -1.0f);
}

ADE_VIRTVAR(AttachedObject, l_Particle, "object", "The object this particle is attached to. If valid the position will be relative to this object and the velocity will be ignored.", "object", "Attached object or invalid object handle on error")
{
	particle_h *ph = NULL;
	object_h *newObj = nullptr;
	if (!ade_get_args(L, "o|o", l_Particle.GetPtr(&ph), l_Object.GetPtr(&newObj)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (ph == NULL)
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (!ph->isValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (ADE_SETTING_VAR)
	{
		if (newObj != nullptr && newObj->IsValid())
			ph->Get().lock()->attached_objnum = newObj->objp->signature;
	}

	return ade_set_object_with_breed(L, ph->Get().lock()->attached_objnum);
}

ADE_FUNC(isValid, l_Particle, NULL, "Detects whether this handle is valid", "boolean", "true if valid false if not")
{
	particle_h *ph = NULL;
	if (!ade_get_args(L, "o", l_Particle.GetPtr(&ph)))
		return ADE_RETURN_FALSE;

	if (ph == NULL)
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", ph->isValid());
}


}
}

