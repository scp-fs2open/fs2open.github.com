
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

ADE_VIRTVAR(Pitch, l_AI_Helper, "number", "The pitch thrust rate for the ship this frame, -1 to 1", "number", "The pitch rate, or 0 if the handle is invalid")
{
	return aici_getset_helper(L, &control_info::pitch);
}

ADE_VIRTVAR(Bank, l_AI_Helper, "number", "The bank thrust rate for the ship this frame, -1 to 1", "number", "The bank rate, or 0 if the handle is invalid")
{
	return aici_getset_helper(L, &control_info::bank);
}

ADE_VIRTVAR(Heading, l_AI_Helper, "number", "The heading thrust rate for the ship this frame, -1 to 1", "number", "The heading rate, or 0 if the handle is invalid")
{
	return aici_getset_helper(L, &control_info::heading);
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
	"vector target, [boolean respectDifficulty = true, vector turnrateModifier /* 100% of tabled values in all rotation axes by default */, number bank /* native bank-on-heading by default */ ]",
	"turns the ship towards the specified point during this frame",
	nullptr,
	nullptr)
{
	object_h ship;
	vec3d* target;
	bool diffTurn = true;
	vec3d* modifier = nullptr;
	float bank = 0.0f;

	int argnum = ade_get_args(L, "oo|bof", l_AI_Helper.Get(&ship), l_Vector.GetPtr(&target), &diffTurn, l_Vector.GetPtr(&modifier), &bank);
	if (argnum == 0) {
		return ADE_RETURN_NIL;
	}

	ai_turn_towards_vector(target, ship.objp, nullptr, nullptr, bank, (diffTurn ? 0 : AITTV_FAST) | (argnum >= 5 ? AITTV_FORCE_DELTA_BANK : 0), nullptr, modifier);
	return ADE_RETURN_NIL;
}

ADE_FUNC(turnTowardsOrientation,
	l_AI_Helper,
	"orientation target, [boolean respectDifficulty = true, vector turnrateModifier /* 100% of tabled values in all rotation axes by default */]",
	"turns the ship towards the specified orientation during this frame",
	nullptr,
	nullptr)
{
	object_h ship;
	matrix_h* target;
	bool diffTurn = true;
	vec3d* modifier = nullptr;

	int argnum = ade_get_args(L, "oo|bo", l_AI_Helper.Get(&ship), l_Matrix.GetPtr(&target), &diffTurn, l_Vector.GetPtr(&modifier));
	if (argnum == 0) {
		return ADE_RETURN_NIL;
	}

	matrix* mat = target->GetMatrix();
	vec3d targetVec = mat->vec.fvec + ship.objp->pos;

	ai_turn_towards_vector(&targetVec, ship.objp, nullptr, nullptr, 0.0f, (diffTurn ? 0 : AITTV_FAST), &mat->vec.rvec, modifier);
	return ADE_RETURN_NIL;
}

}
}
