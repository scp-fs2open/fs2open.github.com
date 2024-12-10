//
//

#include "team.h"
#include "iff_defs/iff_defs.h"
#include "scripting/api/objs/color.h"

namespace scripting {
namespace api {


//**********HANDLE: Team
ADE_OBJ(l_Team, int, "team", "Team handle");

ADE_FUNC(__eq, l_Team, "team, team", "Checks whether two teams are the same team", "boolean", "true if equal, false otherwise")
{
	int t1, t2;
	if(!ade_get_args(L, "oo", l_Team.Get(&t1), l_Team.Get(&t2)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", (t1 == t2));
}

ADE_VIRTVAR(Name, l_Team, "string", "Team name", "string", "Team name, or empty string if handle is invalid")
{
	int tdx=-1;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Team.Get(&tdx), &s))
		return ade_set_error(L, "s", "");

	if(!SCP_vector_inbounds(Iff_info, tdx))
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(Iff_info[tdx].iff_name);
		strncpy(Iff_info[tdx].iff_name, s, len);
		Iff_info[tdx].iff_name[len - 1] = 0;
	}

	return ade_set_args(L, "s", Iff_info[tdx].iff_name);
}

ADE_FUNC(getColor,
	l_Team,
	"boolean ReturnType",
	"Gets the IFF color of the specified Team. False to return raw rgb, true to return color object. Defaults to false.",
	"number, number, number, number | color",
	"rgb color for the specified team or nil if invalid")
{
	int idx;
	bool rc = false;
	if(!ade_get_args(L, "o|b", l_Team.Get(&idx), &rc))
		return ADE_RETURN_NIL;

	if(!SCP_vector_inbounds(Iff_info, idx))
		return ADE_RETURN_NIL;

	color* cur = iff_get_color_by_team(idx, 0, 0);

	if (!rc) {
		return ade_set_args(L, "iiii", (int)cur->red, (int)cur->green, (int)cur->blue, (int)cur->alpha);
	} else {
		return ade_set_args(L, "o", l_Color.Set(*cur));
	}
}

ADE_FUNC(isValid, l_Team, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Team.Get(&idx)))
		return ADE_RETURN_NIL;

	if(!SCP_vector_inbounds(Iff_info, idx))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getBreedName, l_Team, nullptr, "Gets the FreeSpace type name", "string", "'Team', or empty string if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_Team.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (!SCP_vector_inbounds(Iff_info, idx))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", "Team");
}

ADE_FUNC(attacks, l_Team, "team", "Checks the IFF status of another team", "boolean", "True if this team attacks the specified team")
{
	int x, y;
	ade_get_args(L, "oo", l_Team.Get(&x), l_Team.Get(&y));
	if (iff_x_attacks_y(x, y))
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}


}
}
