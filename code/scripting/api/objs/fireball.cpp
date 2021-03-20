//
//

#include "fireball/fireballs.h"
#include "fireball.h"
#include "object.h"
#include "fireballclass.h"
#include "enums.h"

namespace scripting {
namespace api {

//**********HANDLE: Fireball
ADE_OBJ_DERIV(l_Fireball, object_h, "fireball", "Fireball handle", l_Object);

ADE_VIRTVAR(Class, l_Fireball, "fireballclass", "Fireball's class", "fireballclass", "Fireball class, or invalid fireballclass handle if fireball handle is invalid")
{
	object_h *oh=NULL;
	int nc=-1;
	if(!ade_get_args(L, "o|o", l_Fireball.GetPtr(&oh), l_Fireballclass.Get(&nc)))
		return ade_set_error(L, "o", l_Fireballclass.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Fireballclass.Set(-1));

	if (oh->objp->instance < 0 || oh->objp->instance >= static_cast<int>(Fireballs.size()))
		return ade_set_error(L, "o", l_Fireballclass.Set(-1));

	fireball *fb = &Fireballs[oh->objp->instance];

	if(ADE_SETTING_VAR && nc > -1) {
		fb->fireball_info_index = nc;
	}

	return ade_set_args(L, "o", l_Fireballclass.Set(fb->fireball_info_index));
}

ADE_VIRTVAR(RenderType, l_Fireball, "enumeration", "Fireball's render type", "enumeration", "Fireball rendertype, or handle to invalid enum if fireball handle is invalid or a bad enum was given")
{
	object_h* oh = NULL;
	enum_h type;
	if (!ade_get_args(L, "o|o", l_Fireball.GetPtr(&oh), l_Enum.Get(&type)))
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	if (!oh->IsValid())
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	if (oh->objp->instance < 0 || oh->objp->instance >= static_cast<int>(Fireballs.size()))
		return ade_set_error(L, "o", l_Enum.Set(enum_h()));

	fireball* fb = &Fireballs[oh->objp->instance];

	if (ADE_SETTING_VAR && type.IsValid()) {
		int nt = -1;
		switch (type.index) {
		case LE_FIREBALL_MEDIUM_EXPLOSION:
			nt = FIREBALL_MEDIUM_EXPLOSION;
			break;
		case LE_FIREBALL_LARGE_EXPLOSION:
			nt = FIREBALL_LARGE_EXPLOSION;
			break;
		case LE_FIREBALL_WARP_EFFECT:
			nt = FIREBALL_WARP_EFFECT;
			break;
		default:
			return ade_set_error(L, "o", l_Enum.Set(enum_h()));
			break;
		}

		fb->fireball_render_type = nt;
	}

	switch (fb->fireball_render_type) {
	case FIREBALL_MEDIUM_EXPLOSION:
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_FIREBALL_MEDIUM_EXPLOSION)));
	case FIREBALL_LARGE_EXPLOSION:
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_FIREBALL_LARGE_EXPLOSION)));
	case FIREBALL_WARP_EFFECT:
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_FIREBALL_WARP_EFFECT)));
	}

	return ade_set_error(L, "o", l_Enum.Set(enum_h()));
	
}

ADE_VIRTVAR(TimeElapsed, l_Fireball, NULL, "Time this fireball exists in seconds", "number", "Time this fireball exists or 0 if fireball handle is invalid")
{
	object_h *oh=NULL;
	float nll = -1.0f;
	if(!ade_get_args(L, "o", l_Fireball.GetPtr(&oh), &nll))
		return ade_set_error(L, "f", 0.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if (oh->objp->instance < 0 || oh->objp->instance <= static_cast<int>(Fireballs.size())) 
		return ade_set_error(L, "f", 0.0f);

	fireball *fb = &Fireballs[oh->objp->instance];

	return ade_set_args(L, "f", fb->time_elapsed);
}

ADE_VIRTVAR(TotalTime, l_Fireball, NULL, "Total lifetime of the fireball's animation in seconds", "number", "Total lifetime of the fireball's animation or 0 if fireball handle is invalid")
{
	object_h* oh = NULL;
	float nll = -1.0f;
	if (!ade_get_args(L, "o", l_Fireball.GetPtr(&oh), &nll))
		return ade_set_error(L, "f", 0.0f);

	if (!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if (oh->objp->instance < 0 || oh->objp->instance >= static_cast<int>(Fireballs.size()))
		return ade_set_error(L, "f", 0.0f);

	fireball* fb = &Fireballs[oh->objp->instance];

	return ade_set_args(L, "f", fb->total_time);
}

ADE_FUNC(isWarp, l_Fireball, NULL, "Checks if the fireball is a warp effect.", "boolean", "boolean value of the fireball warp status or false if the handle is invalid")
{
	object_h *oh = NULL;
	if(!ade_get_args(L, "o", l_Fireball.GetPtr(&oh)))
		return ADE_RETURN_FALSE;

	if(!oh->IsValid())
		return ADE_RETURN_FALSE;

	if(fireball_is_warp(oh->objp))
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_FUNC(vanish, l_Fireball, nullptr, "Vanishes this fireball from the mission.", "boolean", "True if the deletion was successful, false otherwise.")
{

	object_h* oh = nullptr;

	if (!ade_get_args(L, "o", l_Fireball.GetPtr(&oh)))
		return ade_set_error(L, "b", false);

	if (!oh->IsValid())
		return ade_set_error(L, "b", false);

	//Should be sufficient for Fireballs, as the fireball internal functions also call this, for example to free up a fireball if the limit is reached
	obj_delete(OBJ_INDEX(oh->objp));

	return ade_set_args(L, "b", true);
}

}
}

