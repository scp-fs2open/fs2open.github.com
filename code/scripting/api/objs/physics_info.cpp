
#include "physics_info.h"
#include "vecmath.h"
#include "math/vecmat.h"
#include "ship/shiphit.h"

namespace scripting {
namespace api {

physics_info_h::physics_info_h() {
	objh = object_h();
	pi = NULL;
}
physics_info_h::physics_info_h(object* objp) {
	objh = object_h(objp);
	pi = &objp->phys_info;
}
physics_info_h::physics_info_h(physics_info* in_pi) {
	pi = in_pi;
}
bool physics_info_h::IsValid() {
	if (objh.objp != NULL) {
		return objh.IsValid();
	} else {
		return (pi != NULL);
	}
}


ADE_OBJ(l_Physics, physics_info_h, "physics", "Physics handle");

ADE_VIRTVAR(AfterburnerAccelerationTime, l_Physics, "number", "Afterburner acceleration time", "number", "Afterburner acceleration time, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->afterburner_forward_accel_time_const = f;
	}

	return ade_set_args(L, "f", pih->pi->afterburner_forward_accel_time_const);
}

ADE_VIRTVAR(AfterburnerVelocityMax, l_Physics, "vector", "Afterburner max velocity (Local vector)", "vector", "Afterburner max velocity, or null vector if handle is invalid")
{
	physics_info_h *pih;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!pih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		pih->pi->afterburner_max_vel = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(pih->pi->afterburner_max_vel));
}

ADE_VIRTVAR(BankingConstant, l_Physics, "number", "Banking constant", "number", "Banking constant, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->delta_bank_const = f;
	}

	return ade_set_args(L, "f", pih->pi->delta_bank_const);
}

ADE_VIRTVAR(ForwardAccelerationTime, l_Physics, "number", "Forward acceleration time", "number", "Forward acceleration time, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->forward_accel_time_const = f;
	}

	return ade_set_args(L, "f", pih->pi->forward_accel_time_const);
}

ADE_VIRTVAR(ForwardDecelerationTime, l_Physics, "number", "Forward deceleration time", "number", "Forward decleration time, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->forward_decel_time_const = f;
	}

	return ade_set_args(L, "f", pih->pi->forward_decel_time_const);
}

ADE_VIRTVAR(ForwardThrust, l_Physics, "number", "Forward thrust amount (-1 - 1), used primarily for thruster graphics and does not affect any physical behavior", "number", "Forward thrust, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->linear_thrust.xyz.z = f;
	}

	return ade_set_args(L, "f", pih->pi->linear_thrust.xyz.z);
}

ADE_VIRTVAR(Mass, l_Physics, "number", "Object mass", "number", "Object mass, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->mass = f;
	}

	return ade_set_args(L, "f", pih->pi->mass);
}

ADE_VIRTVAR(RotationalVelocity, l_Physics, "vector", "Rotational velocity (Local vector)", "vector", "Rotational velocity, or null vector if handle is invalid")
{
	physics_info_h *pih;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!pih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		pih->pi->rotvel = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(pih->pi->rotvel));
}

ADE_VIRTVAR(RotationalVelocityDamping, l_Physics, "number", "Rotational damping, ie derivative of rotational speed", "number", "Rotational damping, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->rotdamp = f;
	}

	return ade_set_args(L, "f", pih->pi->rotdamp);
}

ADE_VIRTVAR(RotationalVelocityDesired, l_Physics, "vector", "Desired rotational velocity", "vector", "Desired rotational velocity, or null vector if handle is invalid")
{
	physics_info_h *pih;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!pih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		pih->pi->desired_rotvel = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(pih->pi->desired_rotvel));
}

ADE_VIRTVAR(RotationalVelocityMax, l_Physics, "vector", "Maximum rotational velocity (Local vector)", "vector", "Maximum rotational velocity, or null vector if handle is invalid")
{
	physics_info_h *pih;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!pih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		pih->pi->max_rotvel = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(pih->pi->max_rotvel));
}

ADE_VIRTVAR(ShockwaveShakeAmplitude, l_Physics, "number", "How much shaking from shockwaves is applied to object", "number", "Shockwave shake amplitude, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->shockwave_shake_amp = f;
	}

	return ade_set_args(L, "f", pih->pi->shockwave_shake_amp);
}

ADE_VIRTVAR(SideThrust, l_Physics, "number", "Side thrust amount (-1 - 1), used primarily for thruster graphics and does not affect any physical behavior", "number", "Side thrust amount, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->linear_thrust.xyz.x = f;
	}

	return ade_set_args(L, "f", pih->pi->linear_thrust.xyz.x);
}

ADE_VIRTVAR(SlideAccelerationTime, l_Physics, "number", "Time to accelerate to maximum slide velocity", "number", "Sliding acceleration time, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->slide_accel_time_const = f;
	}

	return ade_set_args(L, "f", pih->pi->slide_accel_time_const);
}

ADE_VIRTVAR(SlideDecelerationTime, l_Physics, "number", "Time to decelerate from maximum slide speed", "number", "Sliding deceleration time, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->slide_decel_time_const = f;
	}

	return ade_set_args(L, "f", pih->pi->slide_decel_time_const);
}

ADE_VIRTVAR(Velocity, l_Physics, "vector", "Object world velocity (World vector)", "vector", "Object velocity, or null vector if handle is invalid")
{
	physics_info_h *pih;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!pih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		pih->pi->vel = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(pih->pi->vel));
}

ADE_VIRTVAR(VelocityDamping, l_Physics, "number", "Damping, the natural period (1 / omega) of the dampening effects on top of the acceleration model. Called 'side_slip_time_const' in code base. ", "number", "Damping, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->side_slip_time_const = f;
	}

	return ade_set_args(L, "f", pih->pi->side_slip_time_const);
}

ADE_VIRTVAR(VelocityDesired, l_Physics, "vector", "Desired velocity (World vector)", "vector", "Desired velocity, or null vector if handle is invalid")
{
	physics_info_h *pih;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!pih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		pih->pi->desired_vel = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(pih->pi->desired_vel));
}

ADE_VIRTVAR(VelocityMax, l_Physics, "vector", "Object max local velocity (Local vector)", "vector", "Maximum velocity, or null vector if handle is invalid")
{
	physics_info_h *pih;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!pih->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		pih->pi->max_vel = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(pih->pi->max_vel));
}

ADE_VIRTVAR(VerticalThrust, l_Physics, "number", "Vertical thrust amount (-1 - 1), used primarily for thruster graphics and does not affect any physical behavior", "number", "Vertical thrust amount, or 0 if handle is invalid")
{
	physics_info_h *pih;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		pih->pi->linear_thrust.xyz.y = f;
	}

	return ade_set_args(L, "f", pih->pi->linear_thrust.xyz.y);
}

ADE_VIRTVAR(AfterburnerActive, l_Physics, "boolean", "Specifies if the afterburner is active or not", "boolean", "true if afterburner is active false otherwise")
{
	physics_info_h *pih;
	bool set = false;

	if(!ade_get_args(L, "o|b", l_Physics.GetPtr(&pih), &set))
		return ade_set_error(L, "b", false);

	if(!pih->IsValid())
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR)
	{
		if(set)
			pih->pi->flags |= PF_AFTERBURNER_ON;
		else
			pih->pi->flags &= ~PF_AFTERBURNER_ON;
	}

	if (pih->pi->flags & PF_AFTERBURNER_ON)
		return ade_set_args(L, "b",  true);
	else
		return ade_set_args(L, "b",  false);
}

ADE_VIRTVAR(GravityConst, l_Physics, "number", "Multiplier for the effect of gravity on this object", "number", "Multiplier, or 0 if handle is invalid")
{
	physics_info_h* pih;
	float grav_const = 0.0f;
	if (!ade_get_args(L, "o|f", l_Physics.GetPtr(&pih), &grav_const))
		return ade_set_error(L, "f", 0.0f);

	if (!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if (ADE_SETTING_VAR) {
		pih->pi->gravity_const = grav_const;
	}

	return ade_set_args(L, "f", pih->pi->gravity_const);
}

ADE_FUNC(isValid, l_Physics, NULL, "True if valid, false or nil if not", "boolean", "Detects whether handle is valid")
{
	physics_info_h *pih;
	if(!ade_get_args(L, "o", l_Physics.GetPtr(&pih)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", pih->IsValid());
}

ADE_FUNC(getSpeed, l_Physics, NULL, "Gets total speed as of last frame", "number", "Total speed, or 0 if handle is invalid")
{
	physics_info_h *pih;
	if(!ade_get_args(L, "o", l_Physics.GetPtr(&pih)))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", pih->pi->speed);
}

ADE_FUNC(getForwardSpeed, l_Physics, NULL, "Gets total speed in the ship's 'forward' direction as of last frame", "number", "Total forward speed, or 0 if handle is invalid")
{
	physics_info_h *pih;
	if(!ade_get_args(L, "o", l_Physics.GetPtr(&pih)))
		return ade_set_error(L, "f", 0.0f);

	if(!pih->IsValid())
		return ade_set_error(L, "f", 0.0f);

	return ade_set_args(L, "f", pih->pi->fspeed);
}

// Nuke's afterburner function
ADE_FUNC(isAfterburnerActive, l_Physics, NULL, "True if Afterburners are on, false or nil if not", "boolean", "Detects whether afterburner is active")
{
	physics_info_h *pih;
	if(!ade_get_args(L, "o", l_Physics.GetPtr(&pih)))
		return ADE_RETURN_NIL;

	if(!pih->IsValid())
		return ade_set_error(L, "b", false);

	if (pih->pi->flags & PF_AFTERBURNER_ON)
		return ade_set_args(L, "b",  true);
	else
		return ade_set_args(L, "b",  false);
}

//nukes glide function
ADE_FUNC(isGliding, l_Physics, NULL, "True if glide mode is on, false or nil if not", "boolean", "Detects if ship is gliding")
{
	physics_info_h *pih;
	if(!ade_get_args(L, "o", l_Physics.GetPtr(&pih)))
		return ADE_RETURN_NIL;

	if(!pih->IsValid())
		return ade_set_error(L, "b", false);

	if (pih->pi->flags & (PF_GLIDING|PF_FORCE_GLIDE))
		return ade_set_args(L, "b",  true);
	else
		return ade_set_args(L, "b",  false);
}

ADE_FUNC(applyWhack, l_Physics, "vector Impulse, [ vector Position]", "Applies a whack to an object based on an impulse vector, indicating the direction and strength of whack and optionally at a position relative to the ship in world orientation, the ship's center being default.", "boolean", "true if it succeeded, false otherwise")
{
	object_h objh;
	physics_info_h *pih;
	vec3d *impulse;
	vec3d *offset = nullptr;

	if (!ade_get_args(L, "oo|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&impulse), l_Vector.GetPtr(&offset)))
		return ADE_RETURN_NIL;

	objh = pih->objh;
	if (offset == nullptr)
		offset = &objh.objp->pos;
	else
		vm_vec_add2(offset, &objh.objp->pos);

	ship_apply_whack(impulse, offset, objh.objp);

	return ADE_RETURN_TRUE;

}

ADE_FUNC(applyWhackWorld, l_Physics, "vector Impulse, [ vector Position]", "Applies a whack to an object based on an impulse vector, indicating the direction and strength of whack and optionally at a world position, the ship's center being default.", "boolean", "true if it succeeded, false otherwise")
{
	object_h objh;
	physics_info_h* pih;
	vec3d* impulse;
	vec3d* world_pos = nullptr;

	if (!ade_get_args(L, "oo|o", l_Physics.GetPtr(&pih), l_Vector.GetPtr(&impulse), l_Vector.GetPtr(&world_pos)))
		return ADE_RETURN_NIL;

	objh = pih->objh;
	if (!world_pos) {
		world_pos = &objh.objp->pos;
	}

	ship_apply_whack(impulse, world_pos, objh.objp);

	return ADE_RETURN_TRUE;

}

}
}
