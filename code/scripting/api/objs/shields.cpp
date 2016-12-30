
#include "shields.h"
#include "enums.h"
#include "object/objectshield.h"

namespace scripting {
namespace api {

//**********HANDLE: Shields
ADE_OBJ(l_Shields, object_h, "shields", "Shields handle");

ADE_FUNC(__len, l_Shields, NULL, "Number of shield segments", "number", "Number of shield segments or 0 if handle is invalid")
{
	object_h *objh;

	if(!ade_get_args(L, "o", l_Shields.GetPtr(&objh)))
		return ade_set_error(L, "i", -1);

	if(!objh->IsValid())
		return ade_set_error(L, "i", -1);

	return ade_set_args(L, "i", objh->objp->n_quadrants);
}

ADE_INDEXER(l_Shields, "enumeration/number", "Gets or sets shield segment strength. Use \"SHIELD_*\" enumerations (for standard 4-quadrant shields) or index of a specific segment, or NONE for the entire shield", "number", "Segment/shield strength, or 0 if handle is invalid")
{
	object_h *objh;
	float nval = -1.0f;

	object *objp = NULL;
	int qdx = -1;

	if(lua_isstring(L, 2))
	{
		char *qd = NULL;
		if(!ade_get_args(L, "os|f", l_Shields.GetPtr(&objh), &qd, &nval))
			return ade_set_error(L, "f", 0.0f);

		if(!objh->IsValid())
			return ade_set_error(L, "f", 0.0f);

		objp = objh->objp;

		//Which quadrant?
		int qdi;
		if(qd == NULL)
			qdx = -1;
		else if((qdi = atoi(qd)) > 0 && qdi <= objp->n_quadrants)
			qdx = qdi-1;	//LUA->FS2
		else
			return ade_set_error(L, "f", 0.0f);
	} else {
		enum_h *qd = NULL;
		if(!ade_get_args(L, "oo|f", l_Shields.GetPtr(&objh), l_Enum.GetPtr(&qd), &nval))
			return 0;

		if(!objh->IsValid())
			return ade_set_error(L, "f", 0.0f);

		objp = objh->objp;

		switch(qd->index)
		{
			case LE_NONE:
				qdx = -1;
				break;
			case LE_SHIELD_FRONT:
				qdx = FRONT_QUAD;
				break;
			case LE_SHIELD_LEFT:
				qdx = LEFT_QUAD;
				break;
			case LE_SHIELD_RIGHT:
				qdx = RIGHT_QUAD;
				break;
			case LE_SHIELD_BACK:
				qdx = REAR_QUAD;
				break;
			default:
				return ade_set_error(L, "f", 0.0f);
		}
	}

	//Set/get all quadrants
	if(qdx == -1) {
		if(ADE_SETTING_VAR && nval >= 0.0f)
			shield_set_strength(objp, nval);

		return ade_set_args(L, "f", shield_get_strength(objp));
	}

	//Set one quadrant?
	if(ADE_SETTING_VAR && nval >= 0.0f)
		shield_set_quad(objp, qdx, nval);

	//Get one quadrant
	return ade_set_args(L, "f", shield_get_quad(objp, qdx));
}

//WMC - Not sure if I want this to be a variable. It'd make more sense
//as a function, since it modifies all quadrant variables
//WMC - Ehh, screw it.
ADE_VIRTVAR(CombinedLeft, l_Shields, "number", "Total shield hitpoints left (for all segments combined)", "number", "Combined shield strength, or 0 if handle is invalid")
{
	object_h *objh;
	float nval = -1.0f;
	if(!ade_get_args(L, "o|f", l_Shields.GetPtr(&objh), &nval))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR && nval >= 0.0f) {
		shield_set_strength(objh->objp, nval);
	}

	return ade_set_args(L, "f", shield_get_strength(objh->objp));
}

ADE_VIRTVAR(CombinedMax, l_Shields, "number", "Maximum shield hitpoints (for all segments combined)", "number", "Combined maximum shield strength, or 0 if handle is invalid")
{
	object_h *objh;
	float nval = -1.0f;
	if(!ade_get_args(L, "o|f", l_Shields.GetPtr(&objh), &nval))
		return 0;

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR && nval >= 0.0f) {
		shield_set_max_strength(objh->objp, nval);
	}

	return ade_set_args(L, "f", shield_get_max_strength(objh->objp));
}

ADE_FUNC(isValid, l_Shields, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	object_h *oh;
	if(!ade_get_args(L, "o", l_Shields.GetPtr(&oh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", oh->IsValid());
}

}
}
