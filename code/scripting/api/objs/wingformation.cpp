//
//

#include "wingformation.h"

#include "ship/ship.h"

namespace scripting {
namespace api {

// NOTE: the wing formation index runs 1 through Wing_formations.size(), offset by 1 from the internal contents.  Index 0 is Default.
// This is necessary because the first formation is Default, but Default isn't actually stored internally as a formation.

//**********HANDLE: WingFormation
ADE_OBJ(l_WingFormation, int, "wingformation", "Wing formation handle");

ADE_FUNC(__tostring, l_WingFormation, nullptr, "Wing formation name", "string", "Wing formation name, or an empty string if handle is invalid")
{
	int formation_id;
	if (!ade_get_args(L, "o", l_WingFormation.Get(&formation_id)))
		return ade_set_error(L, "s", "");

	if (formation_id == 0)
		return ade_set_args(L, "s", "Default");

	formation_id--;	// offset from Default

	if (formation_id < 0 || formation_id >= (int)Wing_formations.size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Wing_formations[formation_id].name);
}

ADE_FUNC(__eq, l_WingFormation, "wingformation, wingformation", "Checks if the two formations are equal", "boolean", "true if equal, false otherwise")
{
	int idx1, idx2;
	if (!ade_get_args(L, "oo", l_WingFormation.Get(&idx1), l_WingFormation.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if (idx1 < 0 || idx1 >= (int)Wing_formations.size() + 1)
		return ade_set_error(L, "b", false);

	if (idx2 < 0 || idx2 >= (int)Wing_formations.size() + 1)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}

ADE_VIRTVAR(Name, l_WingFormation, "string", "Wing formation name", "string", "Wing formation name, or empty string if handle is invalid")
{
	int formation_id;
	if (!ade_get_args(L, "o", l_WingFormation.Get(&formation_id)))
		return ade_set_error(L, "s", "");

	if (formation_id == 0)
		return ade_set_args(L, "s", "Default");

	formation_id--;	// offset from Default

	if (formation_id < 0 || formation_id >= (int)Wing_formations.size())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR)
		LuaError(L, "This property is read-only");

	return ade_set_args(L, "s", Wing_formations[formation_id].name);
}

ADE_FUNC(isValid, l_WingFormation, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int formation_id;
	if (!ade_get_args(L, "o", l_WingFormation.Get(&formation_id)))
		return ADE_RETURN_NIL;

	if (formation_id < 0 || formation_id >= (int)Wing_formations.size() + 1)
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

}
}
