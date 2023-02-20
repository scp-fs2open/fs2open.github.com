//
//

#include "debris.h"
#include "object.h"
#include "shipclass.h"
#include "debris/debris.h"
#include "globalincs/linklist.h"
#include "ship/ship.h"

namespace scripting {
namespace api {


//**********HANDLE: Debris
ADE_OBJ_DERIV(l_Debris, object_h, "debris", "Debris handle", l_Object);

ADE_VIRTVAR(IsHull, l_Debris, "boolean", "Whether or not debris is a piece of hull", "boolean", "Whether debris is a hull fragment, or false if handle is invalid")
{
	object_h *oh;
	bool b=false;
	if(!ade_get_args(L, "o|b", l_Debris.GetPtr(&oh), &b))
		return ade_set_error(L, "b", false);

	if(!oh->IsValid())
		return ade_set_error(L, "b", false);

	debris *db = &Debris[oh->objp->instance];

	if(ADE_SETTING_VAR) {
		db->is_hull = b;
	}

	return ade_set_args(L, "b", db->is_hull);

}

ADE_VIRTVAR(OriginClass, l_Debris, "shipclass", "The shipclass of the ship this debris originates from", "shipclass", "The shipclass of the ship that created this debris")
{
	object_h *oh;
	int shipIdx = -1;
	if(!ade_get_args(L, "o|o", l_Debris.GetPtr(&oh), &shipIdx))
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	debris *db = &Debris[oh->objp->instance];

	if(ADE_SETTING_VAR) {
		if (shipIdx >= 0 && shipIdx < ship_info_size())
			db->ship_info_index = shipIdx;
	}

	return ade_set_args(L, "o", l_Shipclass.Set(db->ship_info_index));
}

ADE_VIRTVAR(DoNotExpire, l_Debris, "boolean", "Whether the debris should expire.  Normally, debris does not expire if it is from ships destroyed before mission or from ships that are more than 50 meters in radius.", "boolean", "True if flag is set, false if flag is not set and nil on error")
{
	object_h *objh = nullptr;
	bool set = false;

	if (!ade_get_args(L, "o|b", l_Debris.GetPtr(&objh), &set))
		return ADE_RETURN_NIL;

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	debris *db = &Debris[objh->objp->instance];

	if (ADE_SETTING_VAR)
	{
		db->flags.set(Debris_Flags::DoNotExpire, set);

		// we need to be careful to manage the hull list here
		// per comments in debris.cpp: "pieces that ... have the DoNotExpire flag should not be on it"
		if (db->is_hull)
		{
			if (set)
				debris_remove_from_hull_list(db);
			else
				debris_add_to_hull_list(db);
		}
	}

	if (db->flags[Debris_Flags::DoNotExpire])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(LifeLeft, l_Debris, "number", "The time this debris piece will last.  When this is 0 (and DoNotExpire is false) the debris will explode.", "number", "The amount of time, in seconds, the debris will last")
{
	object_h *objh = nullptr;
	float lifeleft = 0.0f;

	if (!ade_get_args(L, "o|f", l_Debris.GetPtr(&objh), &lifeleft))
		return ADE_RETURN_NIL;

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	debris *db = &Debris[objh->objp->instance];

	if (ADE_SETTING_VAR)
		db->lifeleft = lifeleft;

	return ade_set_args(L, "f", lifeleft);
}

ADE_FUNC(getDebrisRadius, l_Debris, NULL, "The radius of this debris piece", "number", "The radius of this debris piece or -1 if invalid")
{
	object_h *oh;
	if(!ade_get_args(L, "o", l_Debris.GetPtr(&oh)))
		return ade_set_error(L, "f", -1.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	debris *db = &Debris[oh->objp->instance];

	polymodel *pm = model_get(db->model_num);

	if (pm == NULL)
		return ade_set_error(L, "f", -1.0f);

	if (db->submodel_num < 0 || pm->n_models <= db->submodel_num)
		return ade_set_error(L, "f", -1.0f);

	return ade_set_error(L, "f", pm->submodel[db->submodel_num].rad);
}

ADE_FUNC(isValid, l_Debris, NULL, "Return if this debris handle is valid", "boolean", "true if valid false otherwise")
{
	object_h *oh;
	if(!ade_get_args(L, "o", l_Debris.GetPtr(&oh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", oh != NULL && oh->IsValid());
}

ADE_FUNC(isGeneric, l_Debris, nullptr, "Return if this debris is the generic debris model, not a model subobject", "boolean", "true if Debris_model")
{
	object_h *oh;
	if (!ade_get_args(L, "o", l_Debris.GetPtr(&oh)))
		return ADE_RETURN_FALSE;

	if (!oh->IsValid())
		return ADE_RETURN_FALSE;

	debris *db = &Debris[oh->objp->instance];

	return ade_set_args(L, "b", debris_is_generic(db));
}

ADE_FUNC(isVaporized, l_Debris, nullptr, "Return if this debris is the vaporized debris model, not a model subobject", "boolean", "true if Debris_vaporize_model")
{
	object_h *oh;
	if (!ade_get_args(L, "o", l_Debris.GetPtr(&oh)))
		return ADE_RETURN_FALSE;

	if (!oh->IsValid())
		return ADE_RETURN_FALSE;

	debris *db = &Debris[oh->objp->instance];

	return ade_set_args(L, "b", debris_is_vaporized(db));
}

ADE_FUNC(vanish, l_Debris, nullptr, "Vanishes this piece of debris from the mission.", "boolean", "True if the deletion was successful, false otherwise.")
{

	object_h* oh = nullptr;
	if (!ade_get_args(L, "o", l_Debris.GetPtr(&oh)))
		return ade_set_error(L, "b", false);

	if (!oh->IsValid())
		return ade_set_error(L, "b", false);

	//This skips all the fancy deathroll stuff, and just cleans it from the mission
	oh->objp->flags.set(Object::Object_Flags::Should_be_dead);

	return ade_set_args(L, "b", true);
}

}
}
