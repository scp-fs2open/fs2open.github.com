#include "medals.h"

#include "stats/medals.h"

namespace scripting {
namespace api {

medal_h::medal_h() : medal(-1) {}
medal_h::medal_h(int l_medal) : medal(l_medal) {}

medal_stuff* medal_h::getMedal() const
{
	return &Medals[medal];
}

//**********HANDLE: medal
ADE_OBJ(l_Medal, medal_h, "medal", "Medal handle");

ADE_VIRTVAR(Name, l_Medal, nullptr, "The name of the medal", "string", "The name")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
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

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "s", current.getMedal()->bitmap);
}

ADE_VIRTVAR(NumMods, l_Medal, nullptr, "The number of mods of the medal", "string", "The number of mods")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", current.getMedal()->num_versions);
}

ADE_VIRTVAR(FirstMod, l_Medal, nullptr, "The first mod of the medal. Some start at 1, some start at 0", "string", "The first mod")
{
	medal_h current;
	if (!ade_get_args(L, "o", l_Medal.Get(&current))) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	int start = 0;

	if (current.getMedal()->version_starts_at_1)
		start = 1;

	return ade_set_args(L, "i", start);
}

} // namespace api
} // namespace scripting