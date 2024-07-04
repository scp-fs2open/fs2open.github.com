#include "medals.h"

#include "stats/medals.h"

namespace scripting {
namespace api {

medal_h::medal_h() : medal(-1) {}
medal_h::medal_h(int l_medal) : medal(l_medal) {}

medal_stuff* medal_h::getMedal() const
{
	if (!isValid())
		return nullptr;

	return &Medals[medal];
}

bool medal_h::isValid() const
{
	return SCP_vector_inbounds(Medals, medal);
}

bool medal_h::isRank() const
{
	return medal == Rank_medal_index;
}

//**********HANDLE: medal
ADE_OBJ(l_Medal, medal_h, "medal", "Medal handle");

ADE_FUNC(isValid, l_Medal, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isValid());
}

ADE_VIRTVAR(Name, l_Medal, nullptr, "The name of the medal", "string", "The name")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getMedal()->get_display_name());
}

ADE_VIRTVAR(Bitmap, l_Medal, nullptr, "The bitmap of the medal", "string", "The bitmap")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getMedal()->bitmap);
}

ADE_VIRTVAR(NumMods, l_Medal, nullptr, "The number of mods of the medal", "number", "The number of mods")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "i", 0);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getMedal()->num_versions);
}

ADE_VIRTVAR(FirstMod, l_Medal, nullptr, "The first mod of the medal. Some start at 1, some start at 0", "number", "The first mod")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "i", 0);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	int start = 0;

	if (current.getMedal()->version_starts_at_1)
		start = 1;

	return ade_set_args(L, "i", start);
}

ADE_VIRTVAR(KillsNeeded, l_Medal, nullptr, "The number of kills needed to earn this badge. If not a badge, then returns 0", "number", "The number of kills needed")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
	}
	if (!current.isValid()) {
		return ade_set_error(L, "i", 0);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getMedal()->kills_needed);
}

ADE_FUNC(isRank, l_Medal, nullptr, "Detects whether medal is the rank medal", "boolean", "true if yes, false if not, nil if a syntax/type error occurs")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", current.isRank());
}

} // namespace api
} // namespace scripting