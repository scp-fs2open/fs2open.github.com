//
//

#include "wing.h"
#include "ship.h"
#include "object/object.h"
#include "ship/ship.h"

extern bool sexp_check_flag_array(const char *flag_name, Ship::Wing_Flags &wing_flag);

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
		auto len = sizeof(Wings[wdx].name);
		strncpy(Wings[wdx].name, s, len);
		Wings[wdx].name[len - 1] = 0;
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

ADE_FUNC(setFlag, l_Wing, "boolean set_it, string flag_name", "Sets or clears one or more flags - this function can accept an arbitrary number of flag arguments.  The flag names are currently limited to the arrival and departure parseable flags.", nullptr, "Returns nothing")
{
	int wingnum;
	bool set_it;
	const char *flag_name;

	if (!ade_get_args(L, "obs", l_Wing.Get(&wingnum), &set_it, &flag_name))
		return ADE_RETURN_NIL;
	int skip_args = 2;	// not 3 because there will be one more below

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	auto wingp = &Wings[wingnum];

	do {
		auto wing_flag = Ship::Wing_Flags::NUM_VALUES;

		sexp_check_flag_array(flag_name, wing_flag);

		if (wing_flag == Ship::Wing_Flags::NUM_VALUES)
		{
			Warning(LOCATION, "Wing flag '%s' not found!", flag_name);
			return ADE_RETURN_NIL;
		}

		wingp->flags.set(wing_flag, set_it);

	// read the next flag
	internal::Ade_get_args_skip = ++skip_args;
	} while (ade_get_args(L, "|s", &flag_name) > 0);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getFlag, l_Wing, "string flag_name", "Checks whether one or more flags are set - this function can accept an arbitrary number of flag arguments.  The flag names are currently limited to the arrival and departure parseable flags.", "boolean", "Returns whether all flags are set, or nil if the wing is not valid")
{
	int wingnum;
	const char *flag_name;

	if (!ade_get_args(L, "os", l_Wing.Get(&wingnum), &flag_name))
		return ADE_RETURN_NIL;
	int skip_args = 1;	// not 2 because there will be one more below

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	auto wingp = &Wings[wingnum];

	do {
		auto wing_flag = Ship::Wing_Flags::NUM_VALUES;

		sexp_check_flag_array(flag_name, wing_flag);

		if (wing_flag == Ship::Wing_Flags::NUM_VALUES)
		{
			Warning(LOCATION, "Wing flag '%s' not found!", flag_name);
			return ADE_RETURN_FALSE;
		}

		if (!wingp->flags[wing_flag])
			return ADE_RETURN_FALSE;

	// read the next flag
	internal::Ade_get_args_skip = ++skip_args;
	} while (ade_get_args(L, "|s", &flag_name) > 0);

	// if we're still here, all the flags we were looking for were present
	return ADE_RETURN_TRUE;
}

ADE_FUNC(makeWingArrive, l_Wing, nullptr, "Causes this wing to arrive as if its arrival cue had become true.  Note that reinforcements are only marked as available, not actually created.", "boolean", "true if created, false otherwise")
{
	int wingnum = -1;
	if (!ade_get_args(L, "o", l_Wing.Get(&wingnum)))
		return ADE_RETURN_NIL;

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	Wings[wingnum].arrival_delay = 0;
	return mission_maybe_make_wing_arrive(wingnum, true) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
}


}
}
