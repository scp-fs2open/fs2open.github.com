//
//

#include "debris.h"
#include "object.h"
#include "shipclass.h"
#include "debris/debris.h"
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
		db->is_hull = b ? 1 : 0;
	}

	return ade_set_args(L, "b", db->is_hull ? true : false);

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
		if (shipIdx >= 0 && shipIdx < static_cast<int>(Ship_info.size()))
			db->ship_info_index = shipIdx;
	}

	return ade_set_error(L, "o", l_Shipclass.Set(db->ship_info_index));
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


}
}
