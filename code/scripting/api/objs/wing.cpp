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

static int wing_getset_helper(lua_State* L, int wing::* field, bool canSet = false, bool canBeNegative = false)
{
	int wingnum, value;
	if (!ade_get_args(L, "o|i", l_Wing.Get(&wingnum), &value))
		return ADE_RETURN_NIL;

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		if (canSet)
		{
			if (canBeNegative || value >= 0)
				Wings[wingnum].*field = value;
		}
		else
			LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", Wings[wingnum].*field);
}

ADE_VIRTVAR(FormationName, l_Wing, "string", "Gets or sets the formation name of the wing.", "string", "Name of wing formation, or nil if wing or formation is invalid")
{
	int wingnum;
	const char* formation_name;

	if (!ade_get_args(L, "o|s", l_Wing.Get(&wingnum), &formation_name))
		return ADE_RETURN_NIL;

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	wing* wingp = &Wings[wingnum];

	if (ADE_SETTING_VAR && ade_get_args(L, "*s", &formation_name)) {
		int formation_index = wing_formation_lookup(formation_name);
		if (formation_index < 0) {
			if (stricmp(formation_name, "Default") != 0)
				return ADE_RETURN_NIL;
		}
		wingp->formation = formation_index;
	}

	return ade_set_args(L, "s", Wing_formations[wingp->formation].name);
}

ADE_VIRTVAR(FormationScale, l_Wing, "float", "Gets or sets the scale of the current wing formation.", "float", "scale of wing formation, nil if wing or formation invalid")
{
	int wingnum;
	float formation_scale;

	if (!ade_get_args(L, "o|f", l_Wing.Get(&wingnum), &formation_scale))
		return ADE_RETURN_NIL;

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	wing* wingp = &Wings[wingnum];

	if (ADE_SETTING_VAR && ade_get_args(L, "*f", &formation_scale)) {
		wingp->formation_scale = formation_scale;
	}

	return ade_set_args(L, "f", wingp->formation_scale);
}

ADE_VIRTVAR(CurrentCount, l_Wing, nullptr, "Gets the number of ships in the wing that are currently present", "number", "Number of ships, or nil if invalid handle")
{
	return wing_getset_helper(L, &wing::current_count);
}

ADE_VIRTVAR(WaveCount, l_Wing, nullptr, "Gets the maximum number of ships in a wave for this wing", "number", "Number of ships, or nil if invalid handle")
{
	return wing_getset_helper(L, &wing::wave_count);
}

ADE_VIRTVAR(NumWaves, l_Wing, nullptr, "Gets the number of waves for this wing", "number", "Number of waves, or nil if invalid handle")
{
	return wing_getset_helper(L, &wing::num_waves);
}

ADE_VIRTVAR(CurrentWave, l_Wing, nullptr, "Gets the current wave number for this wing", "number", "Wave number, 0 if the wing has not yet arrived, or nil if invalid handle")
{
	return wing_getset_helper(L, &wing::current_wave);
}

ADE_VIRTVAR(TotalArrived, l_Wing, nullptr, "Gets the number of ships that have arrived over the course of the mission, regardless of wave", "number", "Number of ships, or nil if invalid handle")
{
	return wing_getset_helper(L, &wing::total_arrived_count);
}

ADE_VIRTVAR(TotalDestroyed, l_Wing, nullptr, "Gets the number of ships that have been destroyed over the course of the mission, regardless of wave", "number", "Number of ships, or nil if invalid handle")
{
	return wing_getset_helper(L, &wing::total_destroyed);
}

ADE_VIRTVAR(TotalDeparted, l_Wing, nullptr, "Gets the number of ships that have departed over the course of the mission, regardless of wave", "number", "Number of ships, or nil if invalid handle")
{
	return wing_getset_helper(L, &wing::total_departed);
}

ADE_VIRTVAR(TotalVanished, l_Wing, nullptr, "Gets the number of ships that have vanished over the course of the mission, regardless of wave", "number", "Number of ships, or 0 if invalid handle")
{
	return wing_getset_helper(L, &wing::total_vanished);
}

static int wing_getset_location_helper(lua_State* L, int wing::* field, const char* location_type, const char** location_names, size_t location_names_size)
{
	int wingnum;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Wing.Get(&wingnum), &s))
		return ADE_RETURN_NIL;

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR && s != nullptr)
	{
		int location = string_lookup(s, location_names, location_names_size);
		if (location < 0)
		{
			Warning(LOCATION, "%s location '%s' not found.", location_type, s);
			return ADE_RETURN_NIL;
		}
		Wings[wingnum].*field = location;
	}

	return ade_set_args(L, "s", location_names[Wings[wingnum].*field]);
}

ADE_VIRTVAR(ArrivalLocation, l_Wing, "string", "The wing's arrival location", "string", "Arrival location, or nil if handle is invalid")
{
	return wing_getset_location_helper(L, &wing::arrival_location, "Arrival", Arrival_location_names, MAX_ARRIVAL_NAMES);
}

ADE_VIRTVAR(DepartureLocation, l_Wing, "string", "The wing's departure location", "string", "Departure location, or nil if handle is invalid")
{
	return wing_getset_location_helper(L, &wing::departure_location, "Departure", Departure_location_names, MAX_DEPARTURE_NAMES);
}

static int wing_getset_anchor_helper(lua_State* L, int wing::* field)
{
	int wingnum;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Wing.Get(&wingnum), &s))
		return ADE_RETURN_NIL;

	if (wingnum < 0 || wingnum >= Num_wings)
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR && s != nullptr)
	{
		Wings[wingnum].*field = (stricmp(s, "<no anchor>") == 0) ? -1 : get_parse_name_index(s);
	}

	return ade_set_args(L, "s", (Wings[wingnum].*field >= 0) ? Parse_names[Wings[wingnum].*field] : "<no anchor>");
}

ADE_VIRTVAR(ArrivalAnchor, l_Wing, "string", "The wing's arrival anchor", "string", "Arrival anchor, or nil if handle is invalid")
{
	return wing_getset_anchor_helper(L, &wing::arrival_anchor);
}

ADE_VIRTVAR(DepartureAnchor, l_Wing, "string", "The wing's departure anchor", "string", "Departure anchor, or nil if handle is invalid")
{
	return wing_getset_anchor_helper(L, &wing::departure_anchor);
}

ADE_VIRTVAR(ArrivalPathMask, l_Wing, "number", "The wing's arrival path mask", "number", "Arrival path mask, or nil if handle is invalid")
{
	return wing_getset_helper(L, &wing::arrival_path_mask, true);
}

ADE_VIRTVAR(DeparturePathMask, l_Wing, "number", "The wing's departure path mask", "number", "Departure path mask, or nil if handle is invalid")
{
	return wing_getset_helper(L, &wing::departure_path_mask, true);
}

ADE_VIRTVAR(ArrivalDelay, l_Wing, "number", "The wing's arrival delay", "number", "Arrival delay, or nil if handle is invalid")
{
	return wing_getset_helper(L, &wing::arrival_delay, true);
}

ADE_VIRTVAR(DepartureDelay, l_Wing, "number", "The wing's departure delay", "number", "Departure delay, or nil if handle is invalid")
{
	return wing_getset_helper(L, &wing::departure_delay, true);
}

ADE_VIRTVAR(ArrivalDistance, l_Wing, "number", "The wing's arrival distance", "number", "Arrival distance, or nil if handle is invalid")
{
	return wing_getset_helper(L, &wing::arrival_distance, true);
}


}
}
