//
//

#include "particle.h"
#include "vecmath.h"
#include "object.h"
#include "model.h"
#include "particle/ParticleManager.h"
#include "particle/ParticleEffect.h"

namespace scripting {
namespace api {

particle_h::particle_h() {
}
particle_h::particle_h(const particle::WeakParticlePtr& part_p) {
	this->part = part_p;
}
particle::WeakParticlePtr particle_h::Get() const {
	return this->part;
}
bool particle_h::isValid() const {
	return !part.expired();
}

particle_source_h::particle_source_h(particle::ParticleSource* source_p, uint32_t sourceValidityCounter_p) {
	this->source = source_p;
	this->sourceValidityCounter = sourceValidityCounter_p;
}

particle::ParticleSource* particle_source_h::Get() const {
	return isValid() ? this->source : nullptr;
}

bool particle_source_h::isValid() const {
	return particle::ParticleManager::get()->getSourceValidityCounter() == sourceValidityCounter;
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
		if (newObj != nullptr && newObj->isValid())
			ph->Get().lock()->attached_objnum = newObj->sig;
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

ADE_FUNC_DEPRECATED(setColor, l_Particle, "number r, number g, number b", "Sets the color for a particle.  If the particle does not support color, the function does nothing.  (Currently only debug particles support color.)", nullptr, nullptr, gameversion::version(25,0,0), "Debug particles are deprecated as of FSO 25.0.0! Use particles with a solid-color bitmap instead!")
{
	LuaError(L, "Debug particles are deprecated as of FSO 25.0.0!");
	return ADE_RETURN_NIL;
}


ADE_OBJ(l_ParticleEffect, ::particle::ParticleEffectHandle, "particle_effect", "Handle to a tabled particle effect");

ADE_FUNC(getName, l_ParticleEffect, nullptr, "Returns the name under which this effect is stored", "string", "the name of the particle effect, or an empty string for an invalid handle")
{
	::particle::ParticleEffectHandle ph;
	if (!ade_get_args(L, "o", l_ParticleEffect.Get(&ph)))
		return ade_set_error(L, "s", "");

	if (!ph.isValid())
		return ade_set_error(L, "s", "");

	const auto& particle_effect = particle::ParticleManager::get()->getEffect(ph);
	if (particle_effect.empty())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", particle_effect.front().getName().c_str());
}

ADE_FUNC(createSource, l_ParticleEffect, nullptr, "Creates a new particle source, spawning particles as per this particle effect", "particle_source", "the particle source, or nil for an invalid handle")
{
	::particle::ParticleEffectHandle ph;
	if (!ade_get_args(L, "o", l_ParticleEffect.Get(&ph)))
		return ADE_RETURN_NIL;

	if (!ph.isValid())
		return ADE_RETURN_NIL;

	auto source = particle::ParticleManager::get()->createSource(ph);
	return ade_set_args(L, "o", l_ParticleSource.Set(particle_source_h(source, particle::ParticleManager::get()->getSourceValidityCounter())));
}

ADE_OBJ(l_ParticleSource, particle_source_h, "particle_source", "Handle to a particle source. Only valid immediately after acquiring.");

ADE_FUNC(setNormal, l_ParticleSource, "vector normal", "Sets the normal vector of this particle source.", nullptr, nullptr)
{
	particle_source_h ps;
	vec3d normal;
	if (!ade_get_args(L, "oo", l_ParticleSource.Get(&ps), l_Vector.Get(&normal)))
		return ADE_RETURN_NIL;

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ADE_RETURN_NIL;

	vm_vec_normalize_safe(&normal);
	psp->setNormal(normal);

	return ADE_RETURN_NIL;
}

ADE_FUNC(setTriggerRadius, l_ParticleSource, "number triggerRadius", "Sets the trigger radius of this particle source.", nullptr, nullptr)
{
	particle_source_h ps;
	float trigger;
	if (!ade_get_args(L, "of", l_ParticleSource.Get(&ps), &trigger))
		return ADE_RETURN_NIL;

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ADE_RETURN_NIL;

	psp->setTriggerRadius(trigger);

	return ADE_RETURN_NIL;
}

ADE_FUNC(setTriggerVelocity, l_ParticleSource, "number triggerVelocity", "Sets the trigger velocity of this particle source.", nullptr, nullptr)
{
	particle_source_h ps;
	float trigger;
	if (!ade_get_args(L, "of", l_ParticleSource.Get(&ps), &trigger))
		return ADE_RETURN_NIL;

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ADE_RETURN_NIL;

	psp->setTriggerVelocity(trigger);

	return ADE_RETURN_NIL;
}

ADE_FUNC(createOnCoordinates, l_ParticleSource, "vector position, orientation orientation, [vector velocity = zero_vector, orientation orientationOverride = identity, boolean orientationOverrideIsRelative = true]", "Actually spawns this particle source at the specified position.", "boolean", "returns true if the source was successfully created, false otherwise")
{
	particle_source_h ps;
	vec3d position;
	matrix_h orientation;
	vec3d velocity = ZERO_VECTOR;
	matrix_h orientationOverride(&vmd_identity_matrix);
	bool orientationOverrideRelative = true;

	if (!ade_get_args(L, "ooo|oob", l_ParticleSource.Get(&ps), l_Vector.Get(&position), l_Matrix.Get(&orientation), l_Vector.Get(&velocity), l_Matrix.Get(&orientationOverride), &orientationOverrideRelative))
		return ade_set_args(L, "b", false);

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ade_set_args(L, "b", false);

	psp->setHost(std::make_unique<EffectHostVector>(position, *orientation.GetMatrix(), velocity, *orientationOverride.GetMatrix(), orientationOverrideRelative));
	psp->finishCreation();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(createOnObject, l_ParticleSource, "object object, vector offset, [orientation orientationOverride = identity, boolean orientationOverrideIsRelative = true]", "Actually spawns this particle source, attached to the specified object.", "boolean", "returns true if the source was successfully created, false otherwise")
{
	particle_source_h ps;
	object_h objh;
	vec3d position;
	matrix_h orientationOverride(&vmd_identity_matrix);
	bool orientationOverrideRelative = true;

	if (!ade_get_args(L, "ooo|ob", l_ParticleSource.Get(&ps), l_Object.Get(&objh), l_Vector.Get(&position), l_Matrix.Get(&orientationOverride), &orientationOverrideRelative))
		return ade_set_args(L, "b", false);

	if (!objh.isValid())
		return ade_set_args(L, "b", false);

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ade_set_args(L, "b", false);

	psp->setHost(std::make_unique<EffectHostObject>(objh.objp(), position, *orientationOverride.GetMatrix(), orientationOverrideRelative));
	psp->finishCreation();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(createOnSubmodel, l_ParticleSource, "object object, submodel submodel, vector offset, [orientation orientationOverride = identity, boolean orientationOverrideIsRelative = true]", "Actually spawns this particle source, attached to the specified submodel.", "boolean", "returns true if the source was successfully created, false otherwise")
{
	particle_source_h ps;
	object_h objh;
	submodel_h subobjh;
	vec3d position;
	matrix_h orientationOverride(&vmd_identity_matrix);
	bool orientationOverrideRelative = true;

	if (!ade_get_args(L, "oooo|ob", l_ParticleSource.Get(&ps), l_Object.Get(&objh), l_Submodel.Get(&subobjh), l_Vector.Get(&position), l_Matrix.Get(&orientationOverride), &orientationOverrideRelative))
		return ade_set_args(L, "b", false);

	if (!(subobjh.isValid() && objh.isValid()))
		return ade_set_args(L, "b", false);

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ade_set_args(L, "b", false);

	psp->setHost(std::make_unique<EffectHostSubmodel>(objh.objp(), subobjh.GetSubmodelIndex(), position, *orientationOverride.GetMatrix(), orientationOverrideRelative));
	psp->finishCreation();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(createOnTurret, l_ParticleSource, "object object, submodel submodel, number firepoint, [orientation orientationOverride = identity, boolean orientationOverrideIsRelative = true]", "Actually spawns this particle source, attached to the specified turret firepoint.", "boolean", "returns true if the source was successfully created, false otherwise")
{
	particle_source_h ps;
	object_h objh;
	submodel_h subobjh;
	int firepoint;
	matrix_h orientationOverride(&vmd_identity_matrix);
	bool orientationOverrideRelative = true;

	if (!ade_get_args(L, "oooi|ob", l_ParticleSource.Get(&ps), l_Object.Get(&objh), l_Submodel.Get(&subobjh), &firepoint, l_Matrix.Get(&orientationOverride), &orientationOverrideRelative))
		return ade_set_args(L, "b", false);

	if (!(subobjh.isValid() && objh.isValid()))
		return ade_set_args(L, "b", false);

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ade_set_args(L, "b", false);

	psp->setHost(std::make_unique<EffectHostTurret>(objh.objp(), subobjh.GetSubmodelIndex(), firepoint, *orientationOverride.GetMatrix(), orientationOverrideRelative));
	psp->finishCreation();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(createOnBeam, l_ParticleSource, "object object, submodel submodel, number firepoint, [orientation orientationOverride = identity, boolean orientationOverrideIsRelative = true]", "Actually spawns this particle source along the length of the beam.", "boolean", "returns true if the source was successfully created, false otherwise")
{
	particle_source_h ps;
	object_h objh;
	matrix_h orientationOverride(&vmd_identity_matrix);
	bool orientationOverrideRelative = true;

	if (!ade_get_args(L, "ooo|ob", l_ParticleSource.Get(&ps), l_Object.Get(&objh), l_Matrix.Get(&orientationOverride), &orientationOverrideRelative))
		return ade_set_args(L, "b", false);

	if (!objh.isValid() || objh.objp()->type != OBJ_BEAM)
		return ade_set_args(L, "b", false);

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ade_set_args(L, "b", false);

	psp->setHost(std::make_unique<EffectHostBeam>(objh.objp(), *orientationOverride.GetMatrix(), orientationOverrideRelative));
	psp->finishCreation();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(createOnParticle, l_ParticleSource, "particle particle, [orientation orientationOverride = identity, boolean orientationOverrideIsRelative = true]", "Actually spawns this particle source, attached to the specified persistent particle.", "boolean", "returns true if the source was successfully created, false otherwise")
{
	particle_source_h ps;
	particle_h particle;
	matrix_h orientationOverride(&vmd_identity_matrix);
	bool orientationOverrideRelative = true;

	if (!ade_get_args(L, "ooo|ob", l_ParticleSource.Get(&ps), l_Particle.Get(&particle), l_Matrix.Get(&orientationOverride), &orientationOverrideRelative))
		return ade_set_args(L, "b", false);

	if (!particle.isValid())
		return ade_set_args(L, "b", false);

	particle::ParticleSource* psp = ps.Get();
	if (psp == nullptr)
		return ade_set_args(L, "b", false);

	psp->setHost(std::make_unique<EffectHostParticle>(particle.Get(), *orientationOverride.GetMatrix(), orientationOverrideRelative));
	psp->finishCreation();

	return ade_set_args(L, "b", true);
}

}
}

