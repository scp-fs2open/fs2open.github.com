
#include "ai_helper.h"

#include "object.h"
#include "ship.h"
#include "vecmath.h"

#include "ai/ai.h"
#include "mission/missionparse.h"
#include "ship/ship.h"

namespace scripting {
namespace api {

//**********HANDLE: AI Helper
ADE_OBJ(l_AI_Helper, object_h, "ai_helper", "A helper object to access functions for ship manipulation during AI phase");

ADE_VIRTVAR(Ship, l_AI_Helper, nullptr, "The ship this AI runs for", "ship", "The ship, or invalid ship if the handle is invalid")
{
	object_h ship;
	if (!ade_get_args(L, "o", l_AI_Helper.Get(&ship)))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	return ade_set_args(L, "o", l_Ship.Set(ship));
}

inline int aici_getset_helper(lua_State* L, float control_info::* value) {
	//Use with care! This works only if, as expected, the l_AI_Helper object is the one supplied to the luaai scripts this very frame
	object_h ship;
	float f = 0.0f;
	if (!ade_get_args(L, "o|f", l_AI_Helper.Get(&ship), &f))
		return ade_set_error(L, "f", 0.0f);

	if (!ship.IsValid())
		return ade_set_error(L, "f", 0.0f);

	if (ADE_SETTING_VAR) {
		AI_ci.*value = f;
	}

	return ade_set_args(L, "f", AI_ci.*value);
}

inline int pi_rotation_getset_helper(lua_State* L, int axis) {
	//Use with care! This works only if, as expected, the l_AI_Helper object is the one supplied to the luaai scripts this very frame
	object_h ship;
	float f = 0.0f;
	if (!ade_get_args(L, "o|f", l_AI_Helper.Get(&ship), &f))
		return ade_set_error(L, "f", 0.0f);

	if (!ship.IsValid())
		return ade_set_error(L, "f", 0.0f);

	vec3d vel_limit;
	// get the turn rate if we have Use_axial_turnrate_differences
	if (The_mission.ai_profile->flags[AI::Profile_Flags::Use_axial_turnrate_differences]) {
		vel_limit = Ship_info[Ships[ship.objp->instance].ship_info_index].max_rotvel;
	}
	else { // else get the turn time
		float turn_time = Ship_info[Ships[ship.objp->instance].ship_info_index].srotation_time;

		if (turn_time > 0.0f)
		{
			vel_limit.xyz.x = PI2 / turn_time;
			vel_limit.xyz.y = PI2 / turn_time;
			vel_limit.xyz.z = PI2 / turn_time;
		}
	}
	vec3d acc_limit = ai_get_acc_limit(&vel_limit, ship.objp);

	float currentThrustVel = ((ship.objp->phys_info.desired_rotvel.a1d[axis] - ship.objp->phys_info.rotvel.a1d[axis]) / AI_frametime) / acc_limit.a1d[axis];

	if (ADE_SETTING_VAR) {
		currentThrustVel = f;
		float targetVel = currentThrustVel * acc_limit.a1d[axis] * AI_frametime + ship.objp->phys_info.rotvel.a1d[axis];
		CLAMP(targetVel, -vel_limit.a1d[axis], vel_limit.a1d[axis]);
		ship.objp->phys_info.desired_rotvel.a1d[axis] = targetVel;

		vec3d rotstep = ship.objp->phys_info.desired_rotvel * AI_frametime;

		matrix	rotmat;
		angles rotangles{ rotstep.xyz.x, rotstep.xyz.z, rotstep.xyz.y };
		vm_angles_2_matrix(&rotmat, &rotangles);
		vm_matrix_x_matrix(&ship.objp->phys_info.ai_desired_orient, &ship.objp->orient, &rotmat);
	}

	return ade_set_args(L, "f", currentThrustVel);
}

ADE_VIRTVAR(Pitch, l_AI_Helper, "number", "The pitch thrust rate for the ship this frame, -1 to 1", "number", "The pitch rate, or 0 if the handle is invalid")
{
	return pi_rotation_getset_helper(L, 0);
}

ADE_VIRTVAR(Bank, l_AI_Helper, "number", "The bank thrust rate for the ship this frame, -1 to 1", "number", "The bank rate, or 0 if the handle is invalid")
{
	return pi_rotation_getset_helper(L, 2);
}

ADE_VIRTVAR(Heading, l_AI_Helper, "number", "The heading thrust rate for the ship this frame, -1 to 1", "number", "The heading rate, or 0 if the handle is invalid")
{
	return pi_rotation_getset_helper(L, 1);
}

ADE_VIRTVAR(ForwardThrust, l_AI_Helper, "number", "The forward thrust rate for the ship this frame, -1 to 1", "number", "The forward thrust rate, or 0 if the handle is invalid")
{
	return aici_getset_helper(L, &control_info::forward);
}

ADE_VIRTVAR(VerticalThrust, l_AI_Helper, "number", "The vertical thrust rate for the ship this frame, -1 to 1", "number", "The vertical thrust rate, or 0 if the handle is invalid")
{
	return aici_getset_helper(L, &control_info::vertical);
}

ADE_VIRTVAR(SidewaysThrust, l_AI_Helper, "number", "The sideways thrust rate for the ship this frame, -1 to 1", "number", "The sideways thrust rate, or 0 if the handle is invalid")
{
	return aici_getset_helper(L, &control_info::sideways);
}

ADE_FUNC(turnTowardsPoint,
	l_AI_Helper,
	"vector target, [boolean respectDifficulty = true, vector turnrateModifier /* 100% of tabled values in all rotation axes by default */]",
	"turns the ship towards the specified point during this frame",
	nullptr,
	nullptr)
{
	object_h ship;
	vec3d* target;
	bool diffTurn = true;
	vec3d* modifier = nullptr;
	if (!ade_get_args(L, "oo|bo", l_AI_Helper.Get(&ship), l_Vector.GetPtr(&target), &diffTurn, l_Vector.GetPtr(&modifier))) {
		return ADE_RETURN_NIL;
	}

	ai_turn_towards_vector(target, ship.objp, nullptr, nullptr, 0.0f, diffTurn ? 0 : AITTV_FAST, nullptr, modifier);
	return ADE_RETURN_NIL;
}

}
}