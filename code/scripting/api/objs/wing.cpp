//
//

#include "wing.h"
#include "ship.h"
#include "object/object.h"
#include "ship/ship.h"

namespace scripting {
namespace api {

//**********HANDLE: Wing
ADE_OBJ(l_Wing, int, "wing", "Wing handle");

ADE_INDEXER(l_Wing, "number Index", "Array of ships in the wing", "ship", "Ship handle, or invalid ship handle if index is invalid or wing handle is invalid")
{
	int wdx;
	int sdx;
	object_h *ndx=NULL;
	if(!ade_get_args(L, "oi|o", l_Wing.Get(&wdx), &sdx, l_Ship.GetPtr(&ndx)))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	if(wdx < 0 || wdx >= Num_wings || sdx < 1 || sdx > Wings[wdx].current_count) {
		return ade_set_error(L, "o", l_Ship.Set(object_h()));
	}

	//Lua-->FS2
	sdx--;

	if(ADE_SETTING_VAR && ndx != NULL && ndx->IsValid()) {
		Wings[wdx].ship_index[sdx] = ndx->objp->instance;
	}

	return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[Wings[wdx].ship_index[sdx]].objnum])));
}

ADE_FUNC(__len, l_Wing, NULL, "Gets the number of ships in the wing", "number", "Number of ships in wing, or 0 if invalid handle")
{
	int wdx;
	if(!ade_get_args(L, "o", l_Wing.Get(&wdx)) || wdx < 0 || wdx >= Num_wings)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", Wings[wdx].current_count);
}

ADE_VIRTVAR(Name, l_Wing, "string", "Name of Wing", "string", "Wing name, or empty string if handle is invalid")
{
	int wdx;
	const char* s = nullptr;
	if ( !ade_get_args(L, "o|s", l_Wing.Get(&wdx), &s) || wdx < 0 || wdx >= Num_wings )
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != NULL) {
		auto size = sizeof(Wings[wdx].name);
		strncpy_s(Wings[wdx].name, s, size-1);
	}

	return ade_set_args(L, "s", Wings[wdx].name);
}

ADE_FUNC(isValid, l_Wing, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Wing.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= Num_wings)
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}


}
}
