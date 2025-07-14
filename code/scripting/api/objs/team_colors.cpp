//
//

#include "team_colors.h"
#include "globalincs/alphacolors.h"
#include "scripting/api/objs/color.h"

namespace scripting::api {

//**********HANDLE: TeamColor
ADE_OBJ(l_TeamColor, int, "teamcolor", "Team color handle");

ADE_FUNC(__tostring, l_TeamColor, nullptr, "Team color name", "string", "Team color name, or an empty string if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_TeamColor.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (!SCP_vector_inbounds(Team_Names, idx))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Team_Names[idx].c_str());
}

ADE_FUNC(__eq, l_TeamColor, "teamcolor, teamcolor", "Checks if the two team colors are equal", "boolean", "true if equal, false otherwise")
{
	int idx1, idx2;
	if (!ade_get_args(L, "oo", l_TeamColor.Get(&idx1), l_TeamColor.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if (!SCP_vector_inbounds(Team_Names, idx1))
		return ade_set_error(L, "b", false);

	if (!SCP_vector_inbounds(Team_Names, idx2))
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}

ADE_VIRTVAR(Name, l_TeamColor, nullptr, "The team color name", "string", "Team color name, or empty string if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_TeamColor.Get(&idx)))
		return ade_set_error(L, "s", "");

	if (!SCP_vector_inbounds(Team_Names, idx))
		return ade_set_error(L, "s", "");

	const auto& it = Team_Colors.find(Team_Names[idx]);
	if (it == Team_Colors.end()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Team Color Name is not supported");
	}

	return ade_set_args(L, "s", Team_Names[idx].c_str());
}

ADE_VIRTVAR(BaseColor, l_TeamColor, nullptr, "Team color base color", "color", "Team color base color, or nil if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_TeamColor.Get(&idx)))
		return ADE_RETURN_NIL;

	if (!SCP_vector_inbounds(Team_Names, idx))
		return ADE_RETURN_NIL;

	const auto& it = Team_Colors.find(Team_Names[idx]);
	if (it == Team_Colors.end()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Team Color Base is not supported");
	}

	const auto& color_values = it->second.base;

	color cur;

	gr_init_alphacolor(&cur, static_cast<int>(color_values.r), static_cast<int>(color_values.g), static_cast<int>(color_values.b), 255);

	return ade_set_args(L, "o", l_Color.Set(cur));
}

ADE_VIRTVAR(StripeColor, l_TeamColor, nullptr, "Team color stripe color", "color", "Team color stripe color, or nil if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_TeamColor.Get(&idx)))
		return ADE_RETURN_NIL;

	if (!SCP_vector_inbounds(Team_Names, idx))
		return ADE_RETURN_NIL;

	const auto& it = Team_Colors.find(Team_Names[idx]);
	if (it == Team_Colors.end()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Team Color Stripe is not supported");
	}

	const auto& color_values = it->second.stripe;

	color cur;

	gr_init_alphacolor(&cur, static_cast<int>(color_values.r), static_cast<int>(color_values.g), static_cast<int>(color_values.b), 255);

	return ade_set_args(L, "o", l_Color.Set(cur));
}

ADE_FUNC(isValid, l_TeamColor, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if (!ade_get_args(L, "o", l_TeamColor.Get(&idx)))
		return ADE_RETURN_NIL;

	if (!SCP_vector_inbounds(Team_Names, idx))
		return ADE_RETURN_FALSE;

	const auto& it = Team_Colors.find(Team_Names[idx]);
	if (it == Team_Colors.end()) {
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

}
