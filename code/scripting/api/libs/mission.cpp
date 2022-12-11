//
//

#include "mission.h"

#include "globalincs/linklist.h"

#include "freespace.h"

#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "executor/GameStateExecutionContext.h"
#include "executor/global_executors.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"
#include "hud/hudescort.h"
#include "hud/hudmessage.h"
#include "iff_defs/iff_defs.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionload.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "mission/missiontraining.h"
#include "missionui/missionbrief.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/redalert.h"
#include "nebula/neb.h"
#include "object/objcollide.h"
#include "parse/parselo.h"
#include "parse/sexp.h"
#include "parse/sexp/DynamicSEXP.h"
#include "parse/sexp/LuaSEXP.h"
#include "parse/sexp/LuaAISEXP.h"
#include "parse/sexp/sexp_lookup.h"
#include "playerman/player.h"
#include "scripting/api/LuaPromise.h"
#include "scripting/api/objs/LuaSEXP.h"
#include "scripting/api/objs/luaaisexp.h"
#include "scripting/api/objs/asteroid.h"
#include "scripting/api/objs/animation_handle.h"
#include "scripting/api/objs/background_element.h"
#include "scripting/api/objs/beam.h"
#include "scripting/api/objs/debris.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/event.h"
#include "scripting/api/objs/fireball.h"
#include "scripting/api/objs/fireballclass.h"
#include "scripting/api/objs/message.h"
#include "scripting/api/objs/model.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/parse_object.h"
#include "scripting/api/objs/promise.h"
#include "scripting/api/objs/sexpvar.h"
#include "scripting/api/objs/ship_registry_entry.h"
#include "scripting/api/objs/ship.h"
#include "scripting/api/objs/shipclass.h"
#include "scripting/api/objs/sound.h"
#include "scripting/api/objs/team.h"
#include "scripting/api/objs/vecmath.h"
#include "scripting/api/objs/waypoint.h"
#include "scripting/api/objs/weapon.h"
#include "scripting/api/objs/weaponclass.h"
#include "scripting/api/objs/wing.h"
#include "scripting/lua/LuaConvert.h"
#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaTable.h"
#include "scripting/scripting.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "starfield/starfield.h"
#include "weapon/beam.h"
#include "weapon/weapon.h"

#include <utility>

extern bool Ships_inited;

extern bool Game_shudder_perpetual;
extern bool Game_shudder_everywhere;
extern TIMESTAMP Game_shudder_time;
extern int Game_shudder_total;
extern float Game_shudder_intensity;

namespace scripting {
namespace api {


//**********LIBRARY: Mission
ADE_LIB(l_Mission, "Mission", "mn", "Mission library");

// for use in creating faster metadata systems, use in conjunction with getSignature()
ADE_FUNC(getObjectFromSignature, l_Mission, "number Signature", "Gets a handle of an object from its signature", "object", "Handle of object with signaure, invalid handle if signature is not in use")
{
	int sig = -1;
	int objnum;
	if(!ade_get_args(L, "i", &sig))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (sig == -1) {
		return ade_set_error(L, "o", l_Object.Set(object_h()));
	}

	objnum = obj_get_by_signature(sig);

	return ade_set_object_with_breed(L, objnum);
}

ADE_FUNC(evaluateSEXP, l_Mission, "string", "Runs the defined SEXP script, and returns the result as a boolean", "boolean", "true if the SEXP returned SEXP_TRUE or SEXP_KNOWN_TRUE; false if the SEXP returned anything else (even a number)")
{
	const char* s;
	int r_val;

	if(!ade_get_args(L, "s", &s))
		return ADE_RETURN_FALSE;

	r_val = run_sexp(s);

	if (r_val == SEXP_TRUE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(evaluateNumericSEXP, l_Mission, "string", "Runs the defined SEXP script, and returns the result as a number", "number", "the value of the SEXP result (or NaN if the SEXP returned SEXP_NAN or SEXP_NAN_FOREVER)")
{
	const char* s;
	int r_val;
	bool got_nan;

	if (!ade_get_args(L, "s", &s))
		return ade_set_args(L, "i", 0);

	r_val = run_sexp(s, true, &got_nan);

	if (got_nan)
		return ade_set_args(L, "f", std::numeric_limits<float>::quiet_NaN());
	else
		return ade_set_args(L, "i", r_val);
}

ADE_FUNC(runSEXP, l_Mission, "string", "Runs the defined SEXP script within a `when` operator", "boolean", "if the operation was successful")
{
	const char* s;
	int r_val;
	char buf[8192];

	if(!ade_get_args(L, "s", &s))
		return ADE_RETURN_FALSE;

	while (is_white_space(*s))
		s++;
	if (*s != '(')
	{
		static bool Warned_about_runSEXP_parentheses = false;
		if (!Warned_about_runSEXP_parentheses)
		{
			Warned_about_runSEXP_parentheses = true;
			Warning(LOCATION, "Invalid SEXP syntax: SEXPs must be surrounded by parentheses.  For backwards compatibility, the string has been enclosed in parentheses.  This may not be correct in all use cases.");
		}
		// this is the old sexp handling method, which is incorrect
		snprintf(buf, 8191, "( when ( true ) ( %s ) )", s);
	}
	else
	{
		// this is correct usage
		snprintf(buf, 8191, "( when ( true ) %s )", s);
	}

	r_val = run_sexp(buf);

	if (r_val == SEXP_TRUE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

//****SUBLIBRARY: Mission/Asteroids
ADE_LIB_DERIV(l_Mission_Asteroids, "Asteroids", NULL, "Asteroids in the mission", l_Mission);

ADE_INDEXER(l_Mission_Asteroids, "number Index", "Gets asteroid", "asteroid", "Asteroid handle, or invalid handle if invalid index specified")
{
	int idx = -1;
	if( !ade_get_args(L, "*i", &idx) ) {
		return ade_set_error( L, "o", l_Asteroid.Set( object_h() ) );
	}
	if( idx > -1 && idx < asteroid_count() ) {
		idx--; //Convert from Lua to C, as lua indices start from 1, not 0
		return ade_set_args(L, "o", l_Asteroid.Set(object_h(&Objects[Asteroids[idx].objnum])));
	}

	return ade_set_error(L, "o", l_Asteroid.Set( object_h() ) );
}

ADE_FUNC(__len, l_Mission_Asteroids, NULL,
		 "Number of asteroids in mission. Note that the value returned is only good until an asteroid is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Number of asteroids in the mission, or 0 if asteroids are not enabled")
{
	if(Asteroids_enabled) {
		return ade_set_args(L, "i", asteroid_count());
	}
	return ade_set_args(L, "i", 0);
}

//****SUBLIBRARY: Mission/Debris
ADE_LIB_DERIV(l_Mission_Debris, "Debris", NULL, "debris in the mission", l_Mission);

ADE_INDEXER(l_Mission_Debris, "number Index", "Array of debris in the current mission", "debris", "Debris handle, or invalid debris handle if index wasn't valid")
{
	int idx = -1;
	if( !ade_get_args( L, "*i", &idx ) ) {
		return ade_set_error(L, "o", l_Debris.Set(object_h()));
	}

	idx--; // Lua -> C

	if( idx >= 0 && idx < (int)Debris.size() ) {
		if (Debris[idx].objnum == -1) //Somehow accessed an invalid debris piece
			return ade_set_error(L, "o", l_Debris.Set(object_h()));
		return ade_set_args(L, "o", l_Debris.Set(object_h(&Objects[Debris[idx].objnum])));
	}

	return ade_set_error(L, "o", l_Debris.Set(object_h()));
}

ADE_FUNC(__len, l_Mission_Debris, NULL,
		 "Number of debris pieces in the mission. "
			 "Note that the value returned is only good until a piece of debris is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Current number of debris particles")
{
	return ade_set_args(L, "i", (int)Debris.size());
}

//****SUBLIBRARY: Mission/EscortShips
ADE_LIB_DERIV(l_Mission_EscortShips, "EscortShips", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_EscortShips, "number Index", "Gets escort ship at specified index on escort list", "ship", "Specified ship, or invalid ship handle if invalid index")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	if(idx < 1 || idx > hud_escort_num_ships_on_list())
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	//Lua->FS2
	idx--;

	idx = hud_escort_return_objnum(idx);

	if(idx < 0)
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[idx])));
}

ADE_FUNC(__len, l_Mission_EscortShips, NULL, "Current number of escort ships", "number", "Current number of escort ships")
{
	return ade_set_args(L, "i", hud_escort_num_ships_on_list());
}

//****SUBLIBRARY: Mission/Events
ADE_LIB_DERIV(l_Mission_Events, "Events", NULL, "Events", l_Mission);

ADE_INDEXER(l_Mission_Events, "number/string IndexOrName", "Indexes events list", "event", "Event handle, or invalid event handle if index was invalid")
{
	const char* s;
	if(!ade_get_args(L, "*s", &s))
		return ade_set_error(L, "o", l_Event.Set(-1));

	int i;
	for(i = 0; i < (int)Mission_events.size(); i++)
	{
		if(!stricmp(Mission_events[i].name.c_str(), s))
			return ade_set_args(L, "o", l_Event.Set(i));
	}

	//Now try as a number
	i = atoi(s);
	//Lua-->FS2
	i--;

	if(i < 0 || i >= (int)Mission_events.size())
		return ade_set_error(L, "o", l_Event.Set(-1));


	return ade_set_args(L, "o", l_Event.Set(i));
}

ADE_FUNC(__len, l_Mission_Events, NULL, "Number of events in mission", "number", "Number of events in mission")
{
	return ade_set_args(L, "i", (int)Mission_events.size());
}

//****SUBLIBRARY: Mission/SEXPVariables
ADE_LIB_DERIV(l_Mission_SEXPVariables, "SEXPVariables", NULL, "SEXP Variables", l_Mission);

ADE_INDEXER(l_Mission_SEXPVariables, "number/string IndexOrName", "Array of SEXP variables. Note that you can set a sexp variable using the array, eg \'SEXPVariables[\"newvariable\"] = \"newvalue\"\'", "sexpvariable", "Handle to SEXP variable, or invalid sexpvariable handle if index was invalid")
{
	const char* name   = nullptr;
	const char* newval = nullptr;
	if(!ade_get_args(L, "*s|s", &name, &newval))
		return ade_set_error(L, "o", l_SEXPVariable.Set(sexpvar_h()));

	int idx = get_index_sexp_variable_name(name);
	if(idx < 0)
	{
		idx = atoi(name);

		//Lua-->FS2
		idx--;
	}

	if(idx < 0 || idx >= MAX_SEXP_VARIABLES)
	{
		if(ADE_SETTING_VAR && newval != NULL)
		{
			idx = sexp_add_variable(newval, name, lua_type(L, 2) == LUA_TNUMBER ? SEXP_VARIABLE_NUMBER : SEXP_VARIABLE_STRING);
		}

		//We have failed.
		if(idx < 0)
		{
			return ade_set_error(L, "o", l_SEXPVariable.Set(sexpvar_h()));
		}
	}
	else
	{
		if(ADE_SETTING_VAR && newval != NULL)
		{
			sexp_modify_variable(newval, idx, false);
		}
	}

	return ade_set_args(L, "o", l_SEXPVariable.Set(sexpvar_h(idx)));
}

ADE_FUNC(__len, l_Mission_SEXPVariables, NULL, "Current number of SEXP variables", "number", "Counts number of loaded SEXP Variables. May be slow.")
{
	return ade_set_args(L, "i", sexp_variable_count());
}

//****SUBLIBRARY: Mission/ShipRegistry
ADE_LIB_DERIV(l_Mission_ShipRegistry, "ShipRegistry", nullptr, "The mission's ship registry: all ships parsed, created, or exited that the mission knows about", l_Mission);

ADE_INDEXER(l_Mission_ShipRegistry, "number/string IndexOrName", "Gets ship registry entry", "ship_registry_entry", "Ship registry entry handle, or invalid handle if index was invalid")
{
	const char* name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_ShipRegistryEntry.Set(-1));

	int idx = ship_registry_get_index(name);
	if (idx < 0)
	{
		idx = atoi(name);	// will return 0 if not parseable

		//Lua-->FS2
		idx--;
	}

	return ade_set_args(L, "o", l_ShipRegistryEntry.Set(idx));
}

ADE_FUNC(__len, l_Mission_ShipRegistry, nullptr,
		 "Number of ship registry entries in the mission.  The value returned is generally stable but will change if a ship is created using ship-create or if additional wing waves arrive.",
		 "number",
		 "Number of ship registry entries in the mission")
{
	return ade_set_args(L, "i", (int)Ship_registry.size());
}

//****SUBLIBRARY: Mission/Ships
ADE_LIB_DERIV(l_Mission_Ships, "Ships", NULL, "Ships in the mission", l_Mission);

ADE_INDEXER(l_Mission_Ships, "number/string IndexOrName", "Gets ship", "ship", "Ship handle, or invalid ship handle if index was invalid")
{
	const char* name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Ship.Set(object_h()));

	int idx = ship_name_lookup(name);

	if(idx > -1)
	{
		return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[idx].objnum])));
	}
	else
	{
		idx = atoi(name);
		if(idx > 0)
		{
			int count=1;

			for(int i = 0; i < MAX_SHIPS; i++)
			{
				if (Ships[i].objnum < 0 || Objects[Ships[i].objnum].type != OBJ_SHIP)
					continue;

				if(count == idx) {
					return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[i].objnum])));
				}

				count++;
			}
		}
	}

	return ade_set_error(L, "o", l_Ship.Set(object_h()));
}

ADE_FUNC(__len, l_Mission_Ships, NULL,
		 "Number of ships in the mission. "
			 "This function is somewhat slow, and should be set to a variable for use in looping situations. "
			 "Note that the value returned is only good until a ship is destroyed, and so cannot be relied on for more than one frame.",
		 "number",
		 "Number of ships in the mission, or 0 if ships haven't been initialized yet")
{
	if(Ships_inited)
		return ade_set_args(L, "i", ship_get_num_ships());
	else
		return ade_set_args(L, "i", 0);
}

//****SUBLIBRARY: Mission/ParsedShips
ADE_LIB_DERIV(l_Mission_ParsedShips, "ParsedShips", nullptr, "Parsed ships (aka parse objects) in the mission", l_Mission);

ADE_INDEXER(l_Mission_ParsedShips, "number/string IndexOrName", "Gets parsed ship", "parse_object", "Parsed ship handle, or invalid handle if index was invalid")
{
	const char *name;
	if (!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_ParseObject.Set(parse_object_h(nullptr)));

	auto pobjp = mission_parse_get_parse_object(name);

	if (pobjp)
	{
		return ade_set_args(L, "o", l_ParseObject.Set(parse_object_h(pobjp)));
	}
	else
	{
		auto idx = atoi(name);
		if (idx > 0)
		{
			idx--;	// Lua -> C++

			if (idx < static_cast<int>(Parse_objects.size()))
			{
				return ade_set_args(L, "o", l_ParseObject.Set(parse_object_h(&Parse_objects[idx])));
			}
		}
	}

	return ade_set_error(L, "o", l_ParseObject.Set(parse_object_h(nullptr)));
}

ADE_FUNC(__len, l_Mission_ParsedShips, NULL,
		 "Number of parsed ships in the mission. This function is quick and the value returned can be relied on to be stable for the entire mission.",
		 "number",
		 "Number of parsed ships in the most recently loaded mission, or 0 if no mission has been parsed yet")
{
	return ade_set_args(L, "i", static_cast<int>(Parse_objects.size()));
}

//****SUBLIBRARY: Mission/Waypoints
ADE_LIB_DERIV(l_Mission_Waypoints, "Waypoints", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Waypoints, "number Index", "Array of waypoints in the current mission", "waypoint", "Waypoint handle, or invalid waypoint handle if index was invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Waypoint.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=0;

	object *ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		if (ptr->type == OBJ_WAYPOINT)
			count++;

		if(count == idx) {
			return ade_set_args(L, "o", l_Waypoint.Set(object_h(ptr)));
		}

		ptr = GET_NEXT(ptr);
	}

	return ade_set_error(L, "o", l_Waypoint.Set(object_h()));
}

ADE_FUNC(__len, l_Mission_Waypoints, NULL, "Gets number of waypoints in mission. Note that this is only accurate for one frame.", "number", "Number of waypoints in the mission")
{
	uint count=0;
	for(uint i = 0; i < MAX_OBJECTS; i++)
	{
		if (Objects[i].type == OBJ_WAYPOINT)
			count++;
	}

	return ade_set_args(L, "i", count);
}

//****SUBLIBRARY: Mission/WaypointLists
ADE_LIB_DERIV(l_Mission_WaypointLists, "WaypointLists", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_WaypointLists, "number/string IndexOrWaypointListName", "Array of waypoint lists", "waypointlist", "Gets waypointlist handle")
{
	waypointlist_h wpl;
	const char* name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));

	wpl = waypointlist_h(name);

	if (!wpl.IsValid()) {
		char* end_ptr;
		auto idx = (int)strtol(name, &end_ptr, 10);
		if (end_ptr != name && idx >= 1) {
			// The string is a valid number and the number has a valid value
			wpl = waypointlist_h(find_waypoint_list_at_index(idx - 1));
		}
	}

	if (wpl.IsValid()) {
		return ade_set_args(L, "o", l_WaypointList.Set(wpl));
	}

	return ade_set_error(L, "o", l_WaypointList.Set(waypointlist_h()));
}

ADE_FUNC(__len, l_Mission_WaypointLists, NULL, "Number of waypoint lists in mission. Note that this is only accurate for one frame.", "number", "Number of waypoint lists in the mission")
{
	return ade_set_args(L, "i", Waypoint_lists.size());
}

//****SUBLIBRARY: Mission/Weapons
ADE_LIB_DERIV(l_Mission_Weapons, "Weapons", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Weapons, "number Index", "Gets handle to a weapon object in the mission.", "weapon", "Weapon handle, or invalid weapon handle if index is invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Weapon.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=1;

	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		if (Weapons[i].weapon_info_index < 0 || Weapons[i].objnum < 0 || Objects[Weapons[i].objnum].type != OBJ_WEAPON)
			continue;

		if(count == idx) {
			return ade_set_args(L, "o", l_Weapon.Set(object_h(&Objects[Weapons[i].objnum])));
		}

		count++;
	}

	return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}
ADE_FUNC(__len, l_Mission_Weapons, NULL, "Number of weapon objects in mission. Note that this is only accurate for one frame.", "number", "Number of weapon objects in mission")
{
	return ade_set_args(L, "i", Num_weapons);
}

//****SUBLIBRARY: Mission/Beams
ADE_LIB_DERIV(l_Mission_Beams, "Beams", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Beams, "number Index", "Gets handle to a beam object in the mission.", "beam", "Beam handle, or invalid beam handle if index is invalid")
{
	int idx;
	if(!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Beam.Set(object_h()));

	//Remember, Lua indices start at 0.
	int count=1;

	for(int i = 0; i < MAX_BEAMS; i++)
	{
		if (Beams[i].weapon_info_index < 0 || Beams[i].objnum < 0 || Objects[Beams[i].objnum].type != OBJ_BEAM)
			continue;

		if(count == idx) {
			return ade_set_args(L, "o", l_Beam.Set(object_h(&Objects[Beams[i].objnum])));
		}

		count++;
	}

	return ade_set_error(L, "o", l_Beam.Set(object_h()));
}
ADE_FUNC(__len, l_Mission_Beams, NULL, "Number of beam objects in mission. Note that this is only accurate for one frame.", "number", "Number of beam objects in mission")
{
	return ade_set_args(L, "i", Beam_count);
}

//****SUBLIBRARY: Mission/Wings
ADE_LIB_DERIV(l_Mission_Wings, "Wings", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Wings, "number/string IndexOrWingName", "Wings in the mission", "wing", "Wing handle, or invalid wing handle if index or name was invalid")
{
	const char* name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Wing.Set(-1));

	//MageKing17 - Make the count-ignoring version of the lookup and leave checking if the wing has any ships to the scripter
	int idx = wing_lookup(name);

	if(idx < 0)
	{
		idx = atoi(name);
		if(idx < 1 || idx > Num_wings)
			return ade_set_error(L, "o", l_Wing.Set(-1));

		idx--;	//Lua->FS2
	}

	return ade_set_args(L, "o", l_Wing.Set(idx));
}

ADE_FUNC(__len, l_Mission_Wings, NULL, "Number of wings in mission", "number", "Number of wings in mission")
{
	return ade_set_args(L, "i", Num_wings);
}

//****SUBLIBRARY: Mission/Teams
ADE_LIB_DERIV(l_Mission_Teams, "Teams", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Teams, "number/string IndexOrTeamName", "Teams in the mission", "team", "Team handle or invalid team handle if the requested team could not be found")
{
	const char* name;
	if(!ade_get_args(L, "*s", &name))
		return ade_set_error(L, "o", l_Team.Set(-1));

	int idx = iff_lookup(name);

	if(idx < 0)
	{
		idx = atoi(name);

		idx--;	//Lua->FS2
	}

	if(idx < 0 || idx >= (int)Iff_info.size())
		return ade_set_error(L, "o", l_Team.Set(-1));

	return ade_set_args(L, "o", l_Team.Set(idx));
}

ADE_FUNC(__len, l_Mission_Teams, NULL, "Number of teams in mission", "number", "Number of teams in mission")
{
	return ade_set_args(L, "i", Iff_info.size());
}

//****SUBLIBRARY: Mission/Messages
ADE_LIB_DERIV(l_Mission_Messages, "Messages", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Messages, "number/string IndexOrMessageName", "Messages of the mission", "message", "Message handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Message.Set(-1));

		idx--; // Lua --> FS2

		idx += Num_builtin_messages;
	}
	else
	{
		const char* name = nullptr;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Message.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Message.Set(-1));

		for (int i = Num_builtin_messages; i < (int) Messages.size(); i++)
		{
			if (!stricmp(Messages[i].name, name))
			{
				idx = i;
				break;
			}
		}
	}

	if (idx < Num_builtin_messages || idx >= (int) Messages.size())
		return ade_set_args(L, "o", l_Message.Set(-1));
	else
		return ade_set_args(L, "o", l_Message.Set(idx));
}

ADE_FUNC(__len, l_Mission_Messages, NULL, "Number of messages in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", (int) Messages.size() - Num_builtin_messages);
}

//****SUBLIBRARY: Mission/BuiltinMessages
ADE_LIB_DERIV(l_Mission_BuiltinMessages, "BuiltinMessages", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_BuiltinMessages, "number/string IndexOrMessageName", "Built-in messages of the mission", "message", "Message handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Message.Set(-1));

		idx--; // Lua --> FS2
	}
	else
	{
		const char* name = nullptr;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Message.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Message.Set(-1));

		for (int i = 0; i < Num_builtin_messages; i++)
		{
			if (!stricmp(Messages[i].name, name))
			{
				idx = i;
				break;
			}
		}
	}

	if (idx < 0 || idx >= Num_builtin_messages)
		return ade_set_args(L, "o", l_Message.Set(-1));
	else
		return ade_set_args(L, "o", l_Message.Set(idx));
}

ADE_FUNC(__len, l_Mission_BuiltinMessages, NULL, "Number of built-in messages in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", Num_builtin_messages);
}

//****SUBLIBRARY: Mission/Personas
ADE_LIB_DERIV(l_Mission_Personas, "Personas", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Personas, "number/string IndexOrName", "Personas of the mission", "persona", "Persona handle or invalid handle on error")
{
	int idx = -1;

	if (lua_isnumber(L, 2))
	{
		if (!ade_get_args(L, "*i", &idx))
			return ade_set_args(L, "o", l_Persona.Set(-1));

		idx--; // Lua --> FS2
	}
	else
	{
		const char* name = nullptr;

		if (!ade_get_args(L, "*s", &name))
			return ade_set_args(L, "o", l_Persona.Set(-1));

		if (name == NULL)
			return ade_set_args(L, "o", l_Persona.Set(-1));

		idx = message_persona_name_lookup(name);
	}

	if (idx < 0 || idx >= Num_personas)
		return ade_set_args(L, "o", l_Persona.Set(-1));
	else
		return ade_set_args(L, "o", l_Persona.Set(idx));
}

ADE_FUNC(__len, l_Mission_Personas, NULL, "Number of personas in the mission", "number", "Number of messages in mission")
{
	return ade_set_args(L, "i", Num_personas);
}

//****SUBLIBRARY: Mission/Fireballs
ADE_LIB_DERIV(l_Mission_Fireballs, "Fireballs", NULL, NULL, l_Mission);

ADE_INDEXER(l_Mission_Fireballs, "number Index", "Gets handle to a fireball object in the mission.", "fireball", "Fireball handle, or invalid fireball handle if index is invalid")
{
	int idx;
	if (!ade_get_args(L, "*i", &idx))
		return ade_set_error(L, "o", l_Fireball.Set(object_h()));

	//Remember, Lua indices start at 1.
	int count = 1;

	for (auto& current_fireball : Fireballs) {
		if (current_fireball.fireball_info_index < 0 || current_fireball.objnum < 0 || Objects[current_fireball.objnum].type != OBJ_FIREBALL)
			continue;

		if (count == idx) {
			return ade_set_args(L, "o", l_Fireball.Set(object_h(&Objects[current_fireball.objnum])));
		}

		count++;

	}

	return ade_set_error(L, "o", l_Fireball.Set(object_h()));
}
ADE_FUNC(__len, l_Mission_Fireballs, NULL, "Number of fireball objects in mission. Note that this is only accurate for one frame.", "number", "Number of fireball objects in mission")
{
	int count = fireball_get_count();
	return ade_set_args(L, "i", count);
}

ADE_FUNC(addMessage, l_Mission, "string name, string text, [persona persona]", "Adds a message", "message", "The new message or invalid handle on error")
{
	const char* name = nullptr;
	const char* text = nullptr;
	int personaIdx = -1;

	if (!ade_get_args(L, "ss|o", &name, &text, l_Persona.Get(&personaIdx)))
		return ade_set_error(L, "o", l_Message.Set(-1));

	if (name == NULL || text == NULL)
		return ade_set_error(L, "o", l_Message.Set(-1));

	if (personaIdx < 0 || personaIdx >= Num_personas)
		personaIdx = -1;

	add_message(name, text, personaIdx, 0);

	return ade_set_error(L, "o", l_Message.Set((int) Messages.size() - 1));
}

int sendMessage_sub(lua_State* L, const void* sender, int messageSource, int messageIdx, float delay, enum_h* ehp)
{
	if (messageIdx < 0 || messageIdx >= (int)Messages.size())
		return ADE_RETURN_FALSE;

	if (messageIdx < Num_builtin_messages)
	{
		LuaError(L, "Cannot send built-in messages!");
		return ADE_RETURN_FALSE;
	}

	if (delay < 0.0f)
	{
		LuaError(L, "Invalid negative delay of %f!", delay);
		return ADE_RETURN_FALSE;
	}

	int priority = MESSAGE_PRIORITY_NORMAL;

	if (ehp != nullptr)
	{
		switch (ehp->index)
		{
			case LE_MESSAGE_PRIORITY_HIGH:
				priority = MESSAGE_PRIORITY_HIGH;
				break;
			case LE_MESSAGE_PRIORITY_NORMAL:
				priority = MESSAGE_PRIORITY_NORMAL;
				break;
			case LE_MESSAGE_PRIORITY_LOW:
				priority = MESSAGE_PRIORITY_LOW;
				break;
			default:
				LuaError(L, "Invalid enumeration used! Must be one of MESSAGE_PRIORITY_*.");
				return ADE_RETURN_FALSE;
		}
	}

	message_send_unique_to_player(Messages[messageIdx].name, sender, messageSource, priority, 0, fl2i(delay * MILLISECONDS_PER_SECOND));

	return ADE_RETURN_TRUE;
}

ADE_FUNC(sendMessage,
	l_Mission,
	"string|ship sender, message message, [number delay=0.0, enumeration priority = MESSAGE_PRIORITY_NORMAL, boolean "
	"fromCommand = false]",
	"Sends a message from the given source or ship with the given priority, or optionally sends it from the "
	"mission's command source.<br>"
	"If delay is specified, the message will be delayed by the specified time in seconds.<br>"
	"If sender is <i>nil</i> the message will not have a sender.  If sender is a ship object the message will be sent from the ship; "
	"if sender is a string the message will have a non-ship source even if the string is a ship name.",
	"boolean",
	"true if successful, false otherwise")
{
	const void* sender = nullptr;
	int messageIdx = -1;
	bool fromCommand = false;
	int messageSource = MESSAGE_SOURCE_SPECIAL;
	float delay = 0.0f;

	enum_h* ehp = nullptr;

	// if first is nil then use no source
	if (lua_isnil(L, 1))
	{
		if (!ade_get_args(L, "*o|fob", l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp), &fromCommand))
			return ADE_RETURN_FALSE;

		messageSource = MESSAGE_SOURCE_NONE;
	}
	// if first is a string then treat it as such
	else if (lua_isstring(L, 1))
	{
		const char* sender_string = nullptr;

		if (!ade_get_args(L, "so|fob", &sender_string, l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp), &fromCommand))
			return ADE_RETURN_FALSE;

		if (sender_string == nullptr)
			return ADE_RETURN_FALSE;

		sender = sender_string;
	}
	// assume it's a ship
	else
	{
		object_h* ship_h = nullptr;

		if (!ade_get_args(L, "oo|fob", l_Ship.GetPtr(&ship_h), l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp), &fromCommand))
			return ADE_RETURN_FALSE;

		if (ship_h == nullptr || !ship_h->IsValid())
			return ADE_RETURN_FALSE;

		sender = &Ships[ship_h->objp->instance];
		messageSource = MESSAGE_SOURCE_SHIP;
	}

	if (fromCommand)
		messageSource = MESSAGE_SOURCE_COMMAND;

	return sendMessage_sub(L, sender, messageSource, messageIdx, delay, ehp);
}

ADE_FUNC(sendTrainingMessage, l_Mission, "message message, number time, [number delay=0.0]",
		 "Sends a training message to the player. <i>time</i> is the amount in seconds to display the message, only whole seconds are used!",
		 "boolean", "true if successfull, false otherwise")
{
	int messageIdx = -1;
	float delay = 0.0f;
	int time = -1;

	if (!ade_get_args(L, "oi|f", l_Message.Get(&messageIdx), &time, &delay))
		return ADE_RETURN_FALSE;

	if (messageIdx < 0 || messageIdx >= (int) Messages.size())
		return ADE_RETURN_FALSE;

	if (delay < 0.0f)
	{
		LuaError(L, "Got invalid delay of %f seconds!", delay);
		return ADE_RETURN_FALSE;
	}

	if (time < 0) {
		LuaError(L, "Got invalid time of %d seconds!", time);
		return ADE_RETURN_FALSE;
	}

	message_training_queue(Messages[messageIdx].name, _timestamp(fl2i(delay * MILLISECONDS_PER_SECOND)), time);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(sendPlainMessage,
	l_Mission,
	"string message",
	"Sends a plain text message without it being present in the mission message list",
	"boolean",
	"true if successful false otherwise")
{
	const char* message = nullptr;

	if (!ade_get_args(L, "s", &message))
		return ADE_RETURN_FALSE;

	if (message == nullptr)
		return ADE_RETURN_FALSE;

	HUD_sourced_printf(HUD_SOURCE_HIDDEN, "%s", message);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(createShip,
	l_Mission,
	"[string Name, shipclass Class /* First ship class by default */, orientation Orientation=null, vector Position /* "
	"null vector by default */, team Team, boolean ShowInMissionLog /* true by default */]",
	"Creates a ship and returns a handle to it using the specified name, class, world orientation, and world position; and logs it in the mission log unless specified otherwise",
	"ship",
	"Ship handle, or invalid ship handle if ship couldn't be created")
{
	const char* name = nullptr;
	int sclass       = -1;
	matrix_h* orient = nullptr;
	vec3d pos        = vmd_zero_vector;
	int team         = -1;
	bool show_in_log = true;
	ade_get_args(L, "|soooob", &name, l_Shipclass.Get(&sclass), l_Matrix.GetPtr(&orient), l_Vector.Get(&pos), l_Team.Get(&team), &show_in_log);

	matrix *real_orient = &vmd_identity_matrix;
	if(orient != NULL)
	{
		real_orient = orient->GetMatrix();
	}

	if (sclass == -1) {
		return ade_set_error(L, "o", l_Ship.Set(object_h()));
	}

	int obj_idx = ship_create(real_orient, &pos, sclass, name);

	if(obj_idx >= 0) {
		auto shipp = &Ships[Objects[obj_idx].instance];

		if (team >= 0) {
			shipp->team = team;
		}

		ship_info* sip = &Ship_info[sclass];

		model_page_in_textures(sip->model_num, sclass);

		ship_set_warp_effects(&Objects[obj_idx]);

		// if this name has a hash, create a default display name
		if (get_pointer_to_first_hash_symbol(shipp->ship_name)) {
			shipp->display_name = shipp->ship_name;
			end_string_at_first_hash_symbol(shipp->display_name);
			shipp->flags.set(Ship::Ship_Flags::Has_display_name);
		}

		if (sip->flags[Ship::Info_Flags::Intrinsic_no_shields]) {
			Objects[obj_idx].flags.set(Object::Object_Flags::No_shields);
		}

		mission_log_add_entry(LOG_SHIP_ARRIVED, shipp->ship_name, nullptr, -1, show_in_log ? 0 : MLF_HIDDEN);

		if (scripting::hooks::OnShipArrive->isActive()) {
			scripting::hooks::OnShipArrive->run(scripting::hooks::ShipSpawnConditions{ shipp, ARRIVE_AT_LOCATION, nullptr },
				scripting::hook_param_list(
					scripting::hook_param("Ship", 'o', &Objects[obj_idx])
				));
		}

		return ade_set_args(L, "o", l_Ship.Set(object_h(&Objects[obj_idx])));
	} else
		return ade_set_error(L, "o", l_Ship.Set(object_h()));
}

ADE_FUNC(createDebris,
	l_Mission,
	"[ship | shipclass | model | submodel | nil source, string | nil submodel_index_or_name, vector position, orientation, enumeration create_flags, "
	"number hitpoints, number spark_timeout_seconds, team, vector explosion_center, number explosion_force_multiplier]",
	"Creates a chunk or shard of debris with the specified parameters.  Vectors are in world coordinates.  Any parameter can be nil or negative to specify defaults.  "
	"A nil source will create generic or vaporized debris; submodel_index_or_name will be disregarded if source is submodel and can be nil to spawn random generic or "
	"vaporized debris; position defaults to 0,0,0; orientation defaults to the source orientation or a random orientation for non-ship sources or for generic/vaporized "
	"debris; create_flags can be any combination of DC_IS_HULL, DC_VAPORIZE, DC_SET_VELOCITY, or DC_FIRE_HOOK; hitpoints defaults to 1/8 source ship hitpoints or 10 "
	"hitpoints if there is no source ship; explosion_center and explosion_force_multiplier are only applicable for DC_SET_VELOCITY",
	"debris",
	"Debris handle, or invalid handle if the debris couldn't be created")
{
	model_h *mh = nullptr;
	submodel_h *smh = nullptr;
	vec3d *pos = nullptr, *explosion_center = nullptr;
	matrix_h *orient_h = nullptr;
	enum_h *create_flags = nullptr;
	float hitpoints = -1.0f, spark_timeout = -1.0f, explosion_force_multiplier = 1.0f;
	int team = -1, source_class = -1;
	object_h *source_ship = nullptr;
	ship *source_shipp = nullptr;
	bool is_hull = false;
	bool vaporize = false;
	bool set_velocity = false;
	bool fire_hook = false;

	int source_objnum = -1, model_num = -1, submodel_num = -1;

	// get first variant parameter
	if (luacpp::convert::ade_odata_is_userdata_type(L, 1, l_Ship))
	{
		ade_get_args(L, "|o", l_Ship.GetPtr(&source_ship));

		if (source_ship == nullptr || !source_ship->IsValid())
			return ade_set_args(L, "o", l_Debris.Set(object_h()));

		source_shipp = &Ships[source_ship->objp->instance];
		source_objnum = source_shipp->objnum;
		source_class = source_shipp->ship_info_index;
		model_num = Ship_info[source_class].model_num;
	}
	else if (luacpp::convert::ade_odata_is_userdata_type(L, 1, l_Shipclass))
	{
		ade_get_args(L, "|o", l_Shipclass.Get(&source_class));

		if (source_class < 0 || source_class >= ship_info_size())
			return ade_set_args(L, "o", l_Debris.Set(object_h()));

		model_num = Ship_info[source_class].model_num;
	}
	else if (luacpp::convert::ade_odata_is_userdata_type(L, 1, l_Model))
	{
		ade_get_args(L, "|o", l_Model.GetPtr(&mh));

		if (mh == nullptr || !mh->IsValid())
			return ade_set_args(L, "o", l_Debris.Set(object_h()));

		model_num = mh->GetID();
	}
	else if (luacpp::convert::ade_odata_is_userdata_type(L, 1, l_Submodel))
	{
		ade_get_args(L, "|o", l_Submodel.GetPtr(&smh));

		if (smh == nullptr || !smh->IsValid())
			return ade_set_args(L, "o", l_Debris.Set(object_h()));

		model_num = smh->GetModelID();
		submodel_num = smh->GetSubmodelIndex();
	}

	// get second variant parameter
	if (submodel_num < 0 && model_num >= 0)
	{
		if (lua_isnumber(L, 2))
		{
			ade_get_args(L, "|*i", &submodel_num);
			submodel_num--; // Lua --> C/C++
		}
		else if (lua_isstring(L, 2))
		{
			const char *name = nullptr;
			ade_get_args(L, "|*s", &name);
			submodel_num = model_find_submodel_index(model_num, name);
		}
	}

	ade_get_args(L, "|**oooffoof",
		l_Vector.GetPtr(&pos),
		l_Matrix.GetPtr(&orient_h),
		l_Enum.GetPtr(&create_flags),
		&hitpoints,
		&spark_timeout,
		l_Team.Get(&team),
		l_Vector.GetPtr(&explosion_center),
		&explosion_force_multiplier);

	// validate some arguments

	matrix *orient = orient_h == nullptr ? nullptr : orient_h->GetMatrix();

	if (create_flags != nullptr)
	{
		if (!create_flags->IsValid() || !create_flags->value)
			return ade_set_args(L, "o", l_Debris.Set(object_h()));

		is_hull = (*create_flags->value & LE_DC_IS_HULL);
		vaporize = (*create_flags->value & LE_DC_VAPORIZE);
		set_velocity = (*create_flags->value & LE_DC_SET_VELOCITY);
		fire_hook = (*create_flags->value & LE_DC_FIRE_HOOK);
	}

	int spark_timeout_millis = spark_timeout < 0.0f ? -1 : fl2i(spark_timeout * MILLISECONDS_PER_SECOND);

	// a submodel and a ship class imply hull debris
	if (submodel_num >= 0 && source_class >= 0)
		is_hull = true;

	// now call the relevant functions

	auto obj = debris_create_only(source_objnum, source_class, -1, team, hitpoints, spark_timeout_millis, model_num, submodel_num, pos, orient, is_hull, vaporize, -1);
	if (obj == nullptr)
		return ade_set_args(L, "o", l_Debris.Set(object_h()));

	if (set_velocity)
		debris_create_set_velocity(&Debris[obj->instance], source_shipp, explosion_center, explosion_force_multiplier);

	if (fire_hook)
		debris_create_fire_hook(obj, source_objnum < 0 ? nullptr : &Objects[source_objnum]);

	return ade_set_args(L, "o", l_Debris.Set(object_h(obj)));
}

ADE_FUNC(createWaypoint, l_Mission, "[vector Position, waypointlist List]",
		 "Creates a waypoint",
		 "waypoint",
		 "Waypoint handle, or invalid waypoint handle if waypoint couldn't be created")
{
	vec3d *v3 = NULL;
	waypointlist_h *wlh = NULL;
	if(!ade_get_args(L, "|oo", l_Vector.GetPtr(&v3), l_WaypointList.GetPtr(&wlh)))
		return ade_set_error(L, "o", l_Waypoint.Set(object_h()));

	// determine where we need to create it - it looks like we were given a waypoint list but not a waypoint itself
	int waypoint_instance = -1;
	if (wlh && wlh->IsValid())
	{
		int wp_list_index = find_index_of_waypoint_list(wlh->wlp);
		int wp_index = (int) wlh->wlp->get_waypoints().size() - 1;
		waypoint_instance = calc_waypoint_instance(wp_list_index, wp_index);
	}
	int obj_idx = waypoint_add(v3 != NULL ? v3 : &vmd_zero_vector, waypoint_instance);

	if(obj_idx >= 0)
		return ade_set_args(L, "o", l_Waypoint.Set(object_h(&Objects[obj_idx])));
	else
		return ade_set_args(L, "o", l_Waypoint.Set(object_h()));
}

ADE_FUNC(createWeapon,
	l_Mission,
	"[weaponclass Class /* first weapon in table by default*/, orientation Orientation=identity, vector "
	"WorldPosition/* null vector by default */, object Parent = "
	"nil, number Group = -1]",
	"Creates a weapon and returns a handle to it. 'Group' is used for lighting grouping purposes;"
	" for example, quad lasers would only need to act as one light source.",
	"weapon",
	"Weapon handle, or invalid weapon handle if weapon couldn't be created.")
{
	int wclass = -1;
	object_h *parent = NULL;
	int group = -1;
	matrix_h *orient = NULL;
	vec3d pos = vmd_zero_vector;
	ade_get_args(L, "|ooooi", l_Weaponclass.Get(&wclass), l_Matrix.GetPtr(&orient), l_Vector.Get(&pos), l_Object.GetPtr(&parent), &group);

	matrix *real_orient = &vmd_identity_matrix;
	if(orient != NULL)
	{
		real_orient = orient->GetMatrix();
	}

	int parent_idx = (parent && parent->IsValid()) ? OBJ_INDEX(parent->objp) : -1;

	int obj_idx = weapon_create(&pos, real_orient, wclass, parent_idx, group);

	if(obj_idx > -1)
		return ade_set_args(L, "o", l_Weapon.Set(object_h(&Objects[obj_idx])));
	else
		return ade_set_error(L, "o", l_Weapon.Set(object_h()));
}

ADE_FUNC(createWarpeffect,
	l_Mission,
	"vector WorldPosition, vector PointTo, number radius, number duration /* Must be >= 4*/, fireballclass Class, "
	"soundentry WarpOpenSound, soundentry WarpCloseSound, "
	"[number WarpOpenDuration = -1, number WarpCloseDuration = -1, vector Velocity /* null vector by default*/, "
	"boolean Use3DModel = false]",
	"Creates a warp-effect fireball and returns a handle to it.",
	"fireball",
	"Fireball handle, or invalid fireball handle if fireball couldn't be created.")
{
	vec3d pos = vmd_zero_vector;
	vec3d point_to = vmd_zero_vector;
	float radius = 0.0f;
	float duration = 4.0f;
	int fireballclass = -1;
	sound_entry_h *opensound = NULL;
	sound_entry_h *closesound = NULL;

	float opentime = -1.0f;
	float closetime = -1.0f;
	vec3d velocity = vmd_zero_vector;
	bool warp3d = false;

	if (!ade_get_args(L, "ooffooo|ffob", l_Vector.Get(&pos), l_Vector.Get(&point_to), &radius, &duration, l_Fireballclass.Get(&fireballclass), l_SoundEntry.GetPtr(&opensound), l_SoundEntry.GetPtr(&closesound), &opentime, &closetime, l_Vector.Get(&velocity), &warp3d)) {
		return ade_set_error(L, "o", l_Fireball.Set(object_h()));
	}

	int flags = warp3d ? FBF_WARP_VIA_SEXP | FBF_WARP_3D : FBF_WARP_VIA_SEXP;

	if (duration < 4.0f) {
		duration = 4.0f;
		LuaError(L, "The duration of the warp effect must be at least 4 seconds");
	}

	// sanity check, if these were specified
	if (duration < opentime + closetime)
	{
		//Both warp opening and warp closing must occur within the duration of the warp effect.
		opentime = closetime = duration / 2.0f;
		LuaError(L, "The duration of the warp effect must be higher than the sum of the opening and close durations");
	}

	// calculate orientation matrix ----------------

	vec3d v_orient;
	matrix m_orient;

	vm_vec_sub(&v_orient, &point_to, &pos);

	if (IS_VEC_NULL_SQ_SAFE(&v_orient))
	{
		//error in warp-effect: warp can't point to itself
		LuaError(L, "The warp effect cannot be pointing at itself");
		return ade_set_error(L, "o", l_Fireball.Set(object_h()));
	}

	vm_vector_2_matrix(&m_orient, &v_orient, nullptr, nullptr);

	int obj_idx = fireball_create(&pos, fireballclass, FIREBALL_WARP_EFFECT, -1, radius, false, &velocity, duration, -1, &m_orient, 0, flags, opensound->idx, closesound->idx, opentime, closetime);

	if (obj_idx > -1)
		return ade_set_args(L, "o", l_Fireball.Set(object_h(&Objects[obj_idx])));
	else
		return ade_set_error(L, "o", l_Fireball.Set(object_h()));
}

ADE_FUNC(createExplosion,
	l_Mission,
	"vector WorldPosition, number radius, fireballclass Class, "
	"[boolean LargeExplosion = false, vector Velocity /* null vector by default*/, object parent = nil]",
	"Creates an explosion-effect fireball and returns a handle to it.",
	"fireball",
	"Fireball handle, or invalid fireball handle if fireball couldn't be created.")
{
	vec3d pos = vmd_zero_vector;
	float radius = 0.0f;
	int fireballclass = -1;
	bool big = false;

	vec3d velocity = vmd_zero_vector;
	object_h* parent = NULL;

	if (!ade_get_args(L, "ofo|boo", l_Vector.Get(&pos), &radius, l_Fireballclass.Get(&fireballclass), &big, l_Vector.Get(&velocity), l_Object.GetPtr(&parent))) {
		return ade_set_error(L, "o", l_Fireball.Set(object_h()));
	}

	int type = big ? FIREBALL_LARGE_EXPLOSION : FIREBALL_MEDIUM_EXPLOSION;

	int parent_idx = (parent && parent->IsValid()) ? OBJ_INDEX(parent->objp) : -1;

	int obj_idx = fireball_create(&pos, fireballclass, type, parent_idx, radius, false, &velocity);

	if (obj_idx > -1)
		return ade_set_args(L, "o", l_Fireball.Set(object_h(&Objects[obj_idx])));
	else
		return ade_set_error(L, "o", l_Fireball.Set(object_h()));
}

ADE_FUNC(getMissionFilename, l_Mission, NULL, "Gets mission filename", "string", "Mission filename, or empty string if game is not in a mission")
{
	char temp[MAX_FILENAME_LEN];

	// per Axem, mn.getMissionFilename() sometimes returns the filename with the .fs2 extension, and sometimes not
	// depending if you are in a campaign or tech room
	strcpy_s(temp, Game_current_mission_filename);
	drop_extension(temp);

	return ade_set_args(L, "s", temp);
}

ADE_FUNC(startMission,
	l_Mission,
	"string|enumeration mission /* Filename or MISSION_* enumeration */, [boolean Briefing = true]",
	"Starts the defined mission",
	"boolean",
	"True, or false if the function fails")
{
	bool b = true;
	char s[MAX_FILENAME_LEN];
	const char* str = s;

	if(lua_isstring(L, 1))
	{
		if (!ade_get_args(L, "s|b", &str, &b))
			return ade_set_args(L, "b", false);

	} else {
		enum_h *e = NULL;

		if (!ade_get_args(L, "o|b", l_Enum.GetPtr(&e), &b))
			return ade_set_args(L, "b", false);

		if (e->index == LE_MISSION_REPEAT) {
			if (Num_recent_missions > 0)  {
				strcpy_s( s, Recent_missions[0] );
			} else {
				return ade_set_args(L, "b", false);
			}
		} else {
			return ade_set_args(L, "b", false);
		}
	}

	// no filename... bail
	if (str == NULL)
		return ade_set_args(L, "b", false);

	char name_copy[MAX_FILENAME_LEN];
	strcpy_s(name_copy, str);

	// if mission name has extension... it needs to be removed...
	auto file_ext = strrchr(name_copy, '.');
	if (file_ext)
		*file_ext = 0;

	// game is in MP mode... or if the file does not exist... bail
	if ((Game_mode & GM_MULTIPLAYER) || (cf_exists_full(name_copy, CF_TYPE_MISSIONS) != 0))
		return ade_set_args(L, "b", false);

	// mission is already running...
	if (Game_mode & GM_IN_MISSION) {
		// TO DO... All the things needed if this function is called in any state of the game while mission is running.
		//    most likely all require 'stricmp(str, Game_current_mission_filename)' to make sure missions arent mixed
		//    but after that it might be possible to imprement method for jumping directly into already running
		//    missions.
		return ade_set_args(L, "b", false);
		// if mission is not running
	} else {
		// due safety checks of the game_start_mission() function allow only main menu for now.
		if ((gameseq_get_state(gameseq_get_depth()) == GS_STATE_MAIN_MENU) || (gameseq_get_state(gameseq_get_depth()) == GS_STATE_SIMULATOR_ROOM)) {
			strcpy_s(Game_current_mission_filename, name_copy);
			if (b == true) {
				// start mission - go via briefing screen
				gameseq_post_event(GS_EVENT_START_GAME);
			} else {
				// start mission - enter the game directly
				gameseq_post_event(GS_EVENT_START_GAME_QUICK);
			}
			return ade_set_args(L, "b", true);
		}
	}
	return ade_set_args(L, "b", false);
}

ADE_FUNC(getMissionTime, l_Mission, nullptr, "Game time in seconds since the mission was started; is affected by time compression", "number", "Mission time (seconds) of the current or most recently played mission.")
{
	return ade_set_args(L, "x", Missiontime);
}

//WMC - These are in freespace.cpp
ADE_FUNC(loadMission, l_Mission, "string missionName", "Loads a mission", "boolean", "True if mission was loaded, otherwise false")
{
	const char* s;
	if(!ade_get_args(L, "s", &s))
		return ade_set_error(L, "b", false);

	// clear post processing settings
	gr_post_process_set_defaults();

	//NOW do the loading stuff
	get_mission_info(s, &The_mission, false);
	game_level_init();

	if(!mission_load(s))
		return ADE_RETURN_FALSE;

	game_post_level_init();

	Game_mode |= GM_IN_MISSION;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(unloadMission, l_Mission, NULL, "Stops the current mission and unloads it", NULL, NULL)
{
	SCP_UNUSED(L); // unused parameter

	if(Game_mode & GM_IN_MISSION)
	{
		game_level_close();
		Game_mode &= ~GM_IN_MISSION;
		strcpy_s(Game_current_mission_filename, "");
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(simulateFrame, l_Mission, NULL, "Simulates mission frame", NULL, NULL)
{
	game_update_missiontime();
	game_simulation_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(renderFrame, l_Mission, NULL, "Renders mission frame, but does not move anything", NULL, NULL)
{
	camid cid = game_render_frame_setup();
	game_render_frame( cid );
	game_render_post_frame();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(applyShudder, l_Mission, "number time, number intensity, [boolean perpetual, boolean everywhere]", "Applies a shudder effect to the camera. Time is in seconds. Intensity specifies the shudder effect strength; the Maxim has a value of 1440. If perpetual is true, the shudder does not decay. If everywhere is true, the shudder is applied regardless of view.", "boolean", "true if successful, false otherwise")
{
	float time = -1.0f;
	float intensity = -1.0f;
	bool perpetual = false;
	bool everywhere = false;

	if (!ade_get_args(L, "ff|bb", &time, &intensity, &perpetual, &everywhere))
		return ADE_RETURN_FALSE;

	if (time < 0.0f || intensity < 0.0f)
	{
		LuaError(L, "Illegal shudder values given. Must be bigger than zero; got time of %f and intensity of %f.", time, intensity);
		return ADE_RETURN_FALSE;
	}

	int int_time = fl2i(time * 1000.0f);

	game_shudder_apply(int_time, intensity * 0.01f, perpetual, everywhere);

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(ShudderPerpetual, l_Mission, "boolean", "Gets or sets whether the shudder is perpetual, i.e. with a constant intensity that does not decay.", "boolean", "the shudder perpetual flag")
{
	bool perpetual = Game_shudder_perpetual;

	if (ADE_SETTING_VAR && ade_get_args(L, "*b", &perpetual))
	{
		Game_shudder_perpetual = perpetual;
	}

	return ade_set_args(L, "b", perpetual);
}

ADE_VIRTVAR(ShudderEverywhere, l_Mission, "boolean", "Gets or sets whether the shudder is applied everywhere regardless of camera view.", "boolean", "the shudder everywhere flag")
{
	bool everywhere = Game_shudder_everywhere;

	if (ADE_SETTING_VAR && ade_get_args(L, "*b", &everywhere))
	{
		Game_shudder_everywhere = everywhere;
	}

	return ade_set_args(L, "b", everywhere);
}

ADE_VIRTVAR(ShudderTimeLeft, l_Mission, "number", "Gets or sets the number of seconds until the shudder stops.  This is independent of the decay time.", "number", "the shudder time left variable")
{
	float time_left = Game_shudder_time.isValid()
		? timestamp_until(Game_shudder_time) / static_cast<float>(MILLISECONDS_PER_SECOND)
		: 0.0f;

	if (ADE_SETTING_VAR && ade_get_args(L, "*f", &time_left))
	{
		if (time_left < 0.0f)
		{
			LuaError(L, "Illegal shudder time left.  Must be bigger than zero; got %f.", time_left);
			time_left = 0.0f;
		}

		Game_shudder_time = _timestamp(static_cast<int>(time_left * MILLISECONDS_PER_SECOND));
	}

	return ade_set_args(L, "f", time_left);
}

ADE_VIRTVAR(ShudderDecayTime, l_Mission, "number", "Gets or sets the shudder decay time in seconds.  This can be zero in which case the shudder will not decay.", "number", "the shudder decay time variable")
{
	float total = Game_shudder_total / static_cast<float>(MILLISECONDS_PER_SECOND);

	if (ADE_SETTING_VAR && ade_get_args(L, "*f", &total))
	{
		if (total < 0.0f)
		{
			LuaError(L, "Illegal shudder decay time.  Must be bigger than zero; got %f.", total);
			total = 0.0f;
		}

		Game_shudder_total = static_cast<int>(total * MILLISECONDS_PER_SECOND);
	}

	return ade_set_args(L, "f", total);
}

ADE_VIRTVAR(ShudderIntensity, l_Mission, "number", "Gets or sets the shudder intensity variable.  For comparison, the Maxim has a value of 1440.", "number", "the shudder intensity variable")
{
	float intensity = Game_shudder_intensity * 100.0f;

	if (ADE_SETTING_VAR && ade_get_args(L, "*f", &intensity))
	{
		if (intensity < 0.0f)
		{
			LuaError(L, "Illegal shudder intensity.  Must be bigger than zero; got %f.", intensity);
			intensity = 0.0f;
		}

		Game_shudder_intensity = intensity * 0.01f;
	}

	return ade_set_args(L, "f", intensity);
}

ADE_VIRTVAR(Gravity, l_Mission, "vector", "Gravity acceleration vector in meters / second^2", "vector", "gravity vector")
{
	vec3d* gravity_vec = &The_mission.gravity;

	if (ADE_SETTING_VAR && ade_get_args(L, "*o", l_Vector.GetPtr(&gravity_vec)))
	{
		The_mission.gravity = *gravity_vec;
	}

	return ade_set_args(L, "o", l_Vector.Set(The_mission.gravity));
}

ADE_FUNC(isInMission, l_Mission, nullptr, "get whether or not a mission is currently being played", "boolean", "true if in mission, false otherwise")
{
	return ade_set_args(L, "b", (Game_mode & GM_IN_MISSION) != 0);
}

ADE_FUNC(isInCampaign, l_Mission, NULL, "Get whether or not the current mission being played in a campaign (as opposed to the tech room's simulator)", "boolean", "true if in campaign, false if not")
{
	bool b = false;

	if (Game_mode & GM_CAMPAIGN_MODE) {
		b = true;
	}

	return ade_set_args(L, "b", b);
}

ADE_FUNC(isInCampaignLoop, l_Mission, nullptr, "Get whether or not the current mission being played is a loop mission in the context of a campaign", "boolean", "true if in loop and campaign, false if not")
{
	return ade_set_args(L, "b", (Campaign.loop_enabled) && (Game_mode & GM_CAMPAIGN_MODE));
}

ADE_FUNC(isTraining, l_Mission, nullptr, "Get whether or not the current mission being played is a training mission", "boolean", "true if in training, false if not")
{
	return ade_set_args(L, "b", (The_mission.game_type & MISSION_TYPE_TRAINING) != 0);
}

ADE_FUNC(isScramble, l_Mission, nullptr, "Get whether or not the current mission being played is a scramble mission", "boolean", "true if scramble, false if not")
{
	return ade_set_args(L, "b", (brief_only_allow_briefing() == 1));
}

ADE_FUNC(isMissionSkipAllowed, l_Mission, nullptr, "Get whether or not the player has reached the failure limit", "boolean", "true if limit reached, false if not")
{
	return ade_set_args(L, "b", (Player->failures_this_session >= PLAYER_MISSION_FAILURE_LIMIT));
}

ADE_FUNC(hasNoBriefing, l_Mission, nullptr, "Get whether or not the mission is set to skip the briefing", "boolean", "true if it should be skipped, false if not")
{
	return ade_set_args(L, "b", (The_mission.flags[Mission::Mission_Flags::No_briefing]));
}

ADE_FUNC(isNebula, l_Mission, nullptr, "Get whether or not the current mission being played is set in a nebula", "boolean", "true if in nebula, false if not")
{
	return ade_set_args(L, "b", The_mission.flags[Mission::Mission_Flags::Fullneb]);
}

ADE_VIRTVAR(NebulaSensorRange, l_Mission, "number", "Gets or sets the Neb2_awacs variable.  This is multiplied by a species-specific factor to get the \"scan range\".  Within the scan range, a ship is at least partially targetable (fuzzy blip); within half the scan range, a ship is fully targetable.  Beyond the scan range, a ship is not targetable.", "number", "the Neb2_awacs variable")
{
	float range = -1.0f;

	if (ADE_SETTING_VAR && ade_get_args(L, "*f", &range))
		Neb2_awacs = range;

	return ade_set_args(L, "f", Neb2_awacs);
}

ADE_FUNC(isSubspace, l_Mission, nullptr, "Get whether or not the current mission being played is set in subspace", "boolean", "true if in subspace, false if not")
{
	return ade_set_args(L, "b", The_mission.flags[Mission::Mission_Flags::Subspace]);
}

ADE_FUNC(getMissionTitle, l_Mission, NULL, "Get the title of the current mission", "string", "The mission title or an empty string if currently not in mission") {
	return ade_set_args(L, "s", The_mission.name);
}

ADE_FUNC(getMissionModifiedDate, l_Mission, NULL, "Get the modified date of the current mission", "string", "The mission modified date or an empty string if currently not in mission") {
	return ade_set_args(L, "s", The_mission.modified);
}

static int addBackgroundBitmap_sub(bool uses_correct_angles, lua_State* L)
{
	const char* filename = nullptr;
	float scale_x        = 1.0f;
	float scale_y        = 1.0f;
	int div_x            = 1;
	int div_y            = 1;
	matrix_h orient(&vmd_identity_matrix);

	if (!ade_get_args(L, "s|offii", &filename, l_Matrix.Get(&orient), &scale_x, &scale_y, &div_x, &div_y)) {
		return ade_set_error(L, "o", l_BackgroundElement.Set(background_el_h()));
	}

	starfield_list_entry sle;
	strcpy_s(sle.filename, filename);

	// sanity checking
	if (stars_find_bitmap(sle.filename) < 0) {
		LuaError(L, "Background bitmap %s not found!", sle.filename);
		return ade_set_error(L, "o", l_BackgroundElement.Set(background_el_h()));
	}

	sle.ang     = *orient.GetAngles();
	if (!uses_correct_angles)
		stars_correct_background_bitmap_angles(&sle.ang);

	sle.scale_x = scale_x;
	sle.scale_y = scale_y;
	sle.div_x   = div_x;
	sle.div_y   = div_y;

	// restrict parameters (same code as used by FRED)
	if (sle.scale_x > 18)
		sle.scale_x = 18;
	if (sle.scale_x < 0.1f)
		sle.scale_x = 0.1f;
	if (sle.scale_y > 18)
		sle.scale_y = 18;
	if (sle.scale_y < 0.1f)
		sle.scale_y = 0.1f;
	if (sle.div_x > 5)
		sle.div_x = 5;
	if (sle.div_x < 1)
		sle.div_x = 1;
	if (sle.div_y > 5)
		sle.div_y = 5;
	if (sle.div_y < 1)
		sle.div_y = 1;

	auto idx = stars_add_bitmap_entry(&sle);
	return ade_set_args(L, "o", l_BackgroundElement.Set(background_el_h(BackgroundType::Bitmap, idx)));
}

ADE_FUNC_DEPRECATED(addBackgroundBitmap,
	l_Mission,
	"string name, orientation orientation = identity, number scaleX = 1.0, number scale_y = 1.0, number div_x = 1.0, "
	"number div_y = 1.0",
	"Adds a background bitmap to the mission with the specified parameters, but using the old incorrectly-calculated angle math.",
	"background_element",
	"A handle to the background element, or invalid handle if the function failed.",
	gameversion::version(22, 2),
	"addBackgroundBitmap uses the old incorrectly-calculated angle math; use addBackgroundBitmapNew instead")
{
	return addBackgroundBitmap_sub(false, L);
}

ADE_FUNC(addBackgroundBitmapNew,
	l_Mission,
	"string name, orientation orientation = identity, number scaleX = 1.0, number scale_y = 1.0, number div_x = 1.0, "
	"number div_y = 1.0",
	"Adds a background bitmap to the mission with the specified parameters, treating the angles as correctly calculated.",
	"background_element",
	"A handle to the background element, or invalid handle if the function failed.")
{
	return addBackgroundBitmap_sub(true, L);
}

static int addSunBitmap_sub(bool uses_correct_angles, lua_State* L)
{
	const char* filename = nullptr;
	float scale_x        = 1.0f;
	float scale_y        = 1.0f;
	matrix_h orient(&vmd_identity_matrix);

	if (!ade_get_args(L, "s|off", &filename, l_Matrix.Get(&orient), &scale_x, &scale_y)) {
		return ade_set_error(L, "o", l_BackgroundElement.Set(background_el_h()));
	}

	starfield_list_entry sle;
	strcpy_s(sle.filename, filename);

	// sanity checking
	if (stars_find_sun(sle.filename) < 0) {
		LuaError(L, "Background bitmap %s not found!", sle.filename);
		return ade_set_error(L, "o", l_BackgroundElement.Set(background_el_h()));
	}

	sle.ang     = *orient.GetAngles();
	if (!uses_correct_angles)
		stars_correct_background_sun_angles(&sle.ang);

	sle.scale_x = scale_x;
	sle.scale_y = scale_y;
	sle.div_x   = 1;
	sle.div_y   = 1;

	// restrict parameters (same code as used by FRED)
	if (sle.scale_x > 18)
		sle.scale_x = 18;
	if (sle.scale_x < 0.1f)
		sle.scale_x = 0.1f;
	if (sle.scale_y > 18)
		sle.scale_y = 18;
	if (sle.scale_y < 0.1f)
		sle.scale_y = 0.1f;

	auto idx = stars_add_sun_entry(&sle);
	return ade_set_args(L, "o", l_BackgroundElement.Set(background_el_h(BackgroundType::Sun, idx)));
}

ADE_FUNC_DEPRECATED(addSunBitmap,
	l_Mission,
	"string name, orientation orientation = identity, number scaleX = 1.0, number scale_y = 1.0",
	"Adds a sun bitmap to the mission with the specified parameters, but using the old incorrectly-calculated angle math.",
	"background_element",
	"A handle to the background element, or invalid handle if the function failed.",
	gameversion::version(22, 2),
	"addSunBitmap uses the old incorrectly-calculated angle math; use addSunBitmapNew instead")
{
	return addSunBitmap_sub(false, L);
}

ADE_FUNC(addSunBitmapNew,
	l_Mission,
	"string name, orientation orientation = identity, number scaleX = 1.0, number scale_y = 1.0",
	"Adds a sun bitmap to the mission with the specified parameters, treating the angles as correctly calculated.",
	"background_element",
	"A handle to the background element, or invalid handle if the function failed.")
{
	return addSunBitmap_sub(true, L);
}

ADE_FUNC(removeBackgroundElement, l_Mission, "background_element el",
         "Removes the background element specified by the handle. The handle must have been returned by either "
         "addBackgroundBitmap or addBackgroundSun. This handle will be invalidated by this function.",
         "boolean", "true if successful")
{
	background_el_h* el = nullptr;
	if (!ade_get_args(L, "o", l_BackgroundElement.GetPtr(&el))) {
		return ADE_RETURN_FALSE;
	}

	if (!el->isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (el->type == BackgroundType::Bitmap) {
		int instances = stars_get_num_bitmaps();
		if (instances > el->id) {
			stars_mark_bitmap_unused(el->id);
		} else {
			LuaError(L, "Background slot %d does not exist. Slot must be less than %d.", el->id, instances);
			return ADE_RETURN_FALSE;
		}
		return ADE_RETURN_TRUE;
	} else if (el->type == BackgroundType::Sun) {
		int instances = stars_get_num_suns();
		if (instances > el->id) {
			stars_mark_sun_unused(el->id);
		} else {
			LuaError(L, "Background slot %d does not exist. Slot must be less than %d.", el->id, instances);
			return ADE_RETURN_FALSE;
		}
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(isRedAlertMission,
	l_Mission,
	nullptr,
	"Determines if the current mission is a red alert mission",
	"boolean",
	"true if red alert mission, false otherwise.")
{
	return ade_set_args(L, "b", red_alert_mission() != 0);
}

ADE_FUNC(hasCommandBriefing,
	l_Mission,
	nullptr,
	"Determines if the current mission has a command briefing",
	"boolean",
	"true if command briefing, false otherwise.")
{
	return ade_set_args(L, "b", mission_has_cmd_brief() != 0);
}

ADE_FUNC(hasGoalsStage,
	l_Mission,
	nullptr,
	"Determines if the current mission will show a Goals briefing stage",
	"boolean",
	"true if stage is active, false otherwise.")
{
	bool goals = The_mission.flags[Mission::Mission_Flags::Toggle_showing_goals] == !!(The_mission.game_type & MISSION_TYPE_TRAINING);
	
	return ade_set_args(L, "b", goals);
}

ADE_FUNC(hasDebriefing,
	l_Mission,
	nullptr,
	"Determines if the current mission has a debriefing",
	"boolean",
	"true if debriefing, false otherwise.")
{
	return ade_set_args(L, "b", !(The_mission.flags[Mission::Mission_Flags::Toggle_debriefing]));
}

ADE_FUNC(getMusicScore, l_Mission, "enumeration score", "Returns the music.tbl entry name for the specified mission music score", "string", "The name, or nil if the score is invalid")
{
	enum_h score;
	if (!ade_get_args(L, "o", l_Enum.Get(&score)))
		return ADE_RETURN_NIL;

	if (!score.IsValid() || score.index < LE_SCORE_BRIEFING || score.index > LE_SCORE_FICTION_VIEWER)
	{
		Warning(LOCATION, "Invalid music score index %d", score.index);
		return ADE_RETURN_NIL;
	}

	int spooled_index = Mission_music[score.index - LE_SCORE_BRIEFING];

	const char *name = nullptr;
	if (spooled_index >= 0 && (size_t)spooled_index < Spooled_music.size())
		name = Spooled_music[spooled_index].name;

	return ade_set_args(L, "s", name);
}

ADE_FUNC(setMusicScore, l_Mission, "enumeration score, string name", "Sets the music.tbl entry for the specified mission music score", nullptr, nullptr)
{
	enum_h score;
	const char *name;
	if (!ade_get_args(L, "os", l_Enum.Get(&score), &name))
		return ADE_RETURN_NIL;

	if (!score.IsValid() || score.index < LE_SCORE_BRIEFING || score.index > LE_SCORE_FICTION_VIEWER)
	{
		Warning(LOCATION, "Invalid music score index %d", score.index);
		return ADE_RETURN_NIL;
	}

	event_music_set_score(score.index - LE_SCORE_BRIEFING, name);

	return ADE_RETURN_TRUE;
}

int testLineOfSight_internal(lua_State* L, bool returnDist_and_Obj) {
	vec3d from, to;
	luacpp::LuaTable excludedObjects;
	bool testForShields = false, testForHull = true;
	float threshold = 10.0f;

	float distStore = 0.0f;
	float* dist = returnDist_and_Obj ? &distStore : nullptr;
	object* intersecting_obj = nullptr;

	if (!ade_get_args(L, "oo|tbbf", l_Vector.Get(&from), l_Vector.Get(&to), &excludedObjects, &testForShields, &testForHull, &threshold)) {
		return ADE_RETURN_FALSE;
	}
	if (!(testForHull || testForShields)) {
		LuaError(L, "Cannot test line of sight if neither hull nor shields are set to be tested for.");
		//Though it's technically a "line of sight", so return true
		return ADE_RETURN_TRUE;
	}

	std::unordered_set<const object*> excludedObjectIDs;

	if (excludedObjects.isValid()) {
		for (const auto& object : excludedObjects) {
			if (object.second.is(luacpp::ValueType::USERDATA)) {
				// This'll lua-error internally if it's not fed only objects. Additionally, catch the lua exception and then carry on
				try {
					object_h obj;
					object.second.getValue(l_Object.Get(&obj));
					excludedObjectIDs.emplace(obj.objp);
				}
				catch (const luacpp::LuaException& /*e*/) {
					// We were likely fed a userdata that was not an object. 
					// Since we can't actually tell whether that's the case before we try to get the value, and the attempt to get the value is printing a LuaError itself, just eat the exception here and return
					return ADE_RETURN_FALSE;
				}
			}
			else {
				//This happens on a non-userdata value, i.e. a number
				LuaError(L, "Table with objects to be excluded contained non-userdata values! Aborting...");
				return ADE_RETURN_FALSE;
			}
		}
	}

	bool hasLoS = test_line_of_sight(&from, &to, std::move(excludedObjectIDs), threshold, testForShields, testForHull, dist, &intersecting_obj);

	if (returnDist_and_Obj)
		return ade_set_args(L, "bfo", hasLoS, *dist, l_Object.Set(object_h(intersecting_obj)));
	else
		return ade_set_args(L, "b", hasLoS);
}

ADE_FUNC(hasLineOfSight, l_Mission, "vector from, vector to, [table excludedObjects /* expects list of objects, empty by default */, boolean testForShields = false, boolean testForHull = true, number threshold = 10.0]", "Checks whether the to-position is in line of sight from the from-position, disregarding specific excluded objects and objects with a radius of less then threshold.", "boolean", "true if there is line of sight, false otherwise.")
{
	return testLineOfSight_internal(L, false);
}

ADE_FUNC(getLineOfSightFirstIntersect, l_Mission, "vector from, vector to, [table excludedObjects /* expects list of objects, empty by default */, boolean testForShields = false, boolean testForHull = true, number threshold = 10.0]", "Checks whether the to-position is in line of sight from the from-position and returns the distance and intersecting object to the first interruption of the line of sight, disregarding specific excluded objects and objects with a radius of less then threshold.", "boolean, number, object", "true and zero and nil if there is line of sight, false and the distance and intersecting object otherwise.")
{
	return testLineOfSight_internal(L, true);
}

ADE_FUNC(getSpecialSubmodelAnimation, l_Mission, "string target, string type, string triggeredBy",
	"Gets an animation handle. Target is the object that should be animated (one of \"cockpit\", \"skybox\"), type is the string name of the animation type, "
	"triggeredBy is a closer specification which animation should trigger. See *-anim.tbm specifications. ",
	"animation_handle",
	"The animation handle for the specified animation, nil if invalid arguments.")
{
	const char* target = nullptr;
	const char* type = nullptr;
	const char* trigger = nullptr;

	if (!ade_get_args(L, "sss", &target, &type, &trigger))
		return ADE_RETURN_NIL;

	polymodel_instance* pmi;
	animation::ModelAnimationSet* set;

	if (stricmp(target, "cockpit") == 0) {
		if (Player_ship == nullptr || Player_ship->cockpit_model_instance < 0)
			return ade_set_args(L, "o", l_AnimationHandle.Set(animation::ModelAnimationSet::AnimationList{}));
		pmi = model_get_instance(Player_ship->cockpit_model_instance);
		set = &Ship_info[Player_ship->ship_info_index].cockpit_animations;
	}
	else if (stricmp(target, "skybox") == 0) {
		if(Nmodel_instance_num < 0)
			return ade_set_args(L, "o", l_AnimationHandle.Set(animation::ModelAnimationSet::AnimationList{}));
		pmi = model_get_instance(Nmodel_instance_num);
		set = &The_mission.skybox_model_animations;
	}
	else {
		return ADE_RETURN_NIL;
	}

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ade_set_args(L, "o", l_AnimationHandle.Set(animation::ModelAnimationSet::AnimationList{}));

	return ade_set_args(L, "o", l_AnimationHandle.Set(set->parseScripted(pmi, animtype, trigger)));
}

ADE_FUNC(updateSpecialSubmodelMoveable, l_Mission, "string target, string name, table values",
	"Updates a moveable animation. Name is the name of the moveable. For what values needs to contain, please refer to the table below, depending on the type of the moveable:"
	"Orientation:\r\n"
	"\tThree numbers, x, y, z rotation respectively, in degrees\r\n"
	"Rotation:\r\n"
	"\tThree numbers, x, y, z rotation respectively, in degrees\r\n"
	"Axis Rotation:\r\n"
	"\tOne number, rotation angle in degrees\r\n"
	"Inverse Kinematics:\r\n"
	"\tThree required numbers: x, y, z position target relative to base, in 1/100th meters\r\n"
	"\tThree optional numbers: x, y, z rotation target relative to base, in degrees\r\n",
	"boolean",
	"True if successful, false or nil otherwise")
{
	const char* target = nullptr;
	const char* name = nullptr;
	luacpp::LuaTable values;

	if (!ade_get_args(L, "sst", &target, &name, &values))
		return ADE_RETURN_NIL;

	polymodel_instance* pmi;
	animation::ModelAnimationSet* set;

	if (stricmp(target, "cockpit") == 0) {
		if (Player_ship == nullptr || Player_ship->cockpit_model_instance < 0)
			return ADE_RETURN_NIL;
		pmi = model_get_instance(Player_ship->cockpit_model_instance);
		set = &Ship_info[Player_ship->ship_info_index].cockpit_animations;
	}
	else if (stricmp(target, "skybox") == 0) {
		if (Nmodel_instance_num < 0)
			return ADE_RETURN_NIL;
		pmi = model_get_instance(Nmodel_instance_num);
		set = &The_mission.skybox_model_animations;
	}
	else {
		return ADE_RETURN_NIL;
	}

	SCP_vector<linb::any> valuesMoveable;

	if (values.isValid()) {
		for (const auto& object : values) {
			if (object.second.is(luacpp::ValueType::NUMBER)) {
				// This'll lua-error internally if it's not fed only objects. Additionally, catch the lua exception and then carry on
				try {
					valuesMoveable.emplace_back(object.second.getValue<int>());
				}
				catch (const luacpp::LuaException& /*e*/) {
					// We were likely fed a float. 
					// Since we can't actually tell whether that's the case before we try to get the value, and the attempt to get the value is printing a LuaError itself, just eat the exception here and return
					return ADE_RETURN_FALSE;
				}
			}
			else {
				//This happens on a non-userdata value, i.e. a number
				LuaError(L, "Value table contained non-numbers! Aborting...");
				return ADE_RETURN_FALSE;
			}
		}
	}

	return set->updateMoveable(pmi, name, valuesMoveable) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
}

ADE_LIB_DERIV(l_Mission_LuaSEXPs, "LuaSEXPs", nullptr, "Lua SEXPs", l_Mission);

ADE_INDEXER(l_Mission_LuaSEXPs, "string Name", "Gets a handle of a Lua SEXP", "LuaSEXP", "Lua SEXP handle or invalid handle on error")
{
	const char* name = nullptr;
	if( !ade_get_args(L, "*s", &name) ) {
		return ade_set_error( L, "o", l_LuaSEXP.Set( lua_sexp_h() ) );
	}

	if (name == nullptr) {
		return ade_set_error(L, "o", l_LuaSEXP.Set(lua_sexp_h()));
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting of Lua SEXPs is not supported!");
	}

	auto op = get_operator_const(name);

	if (op == 0) {
		LuaError(L, "SEXP '%s' is not known to the SEXP system!", name);
		return ade_set_args(L, "o", l_LuaSEXP.Set(lua_sexp_h()));
	}

	auto dynamicSEXP = sexp::get_dynamic_sexp(op);

	if (dynamicSEXP == nullptr) {
		return ade_set_args(L, "o", l_LuaSEXP.Set(lua_sexp_h()));
	}

	if (typeid(*dynamicSEXP) != typeid(sexp::LuaSEXP)) {
		LuaError(L, "Specified dynamic SEXP name does not refer to a Lua SEXP!");
		return ade_set_error(L, "o", l_LuaSEXP.Set(lua_sexp_h()));
	}

	return ade_set_args(L, "o", l_LuaSEXP.Set(lua_sexp_h(static_cast<sexp::LuaSEXP*>(dynamicSEXP))));
}

ADE_LIB_DERIV(l_Mission_LuaAISEXPs, "LuaAISEXPs", nullptr, "Lua AI SEXPs", l_Mission);

ADE_INDEXER(l_Mission_LuaAISEXPs, "string Name", "Gets a handle of a Lua SEXP", "LuaAISEXP", "Lua AI SEXP handle or invalid handle on error")
{
	const char* name = nullptr;
	if (!ade_get_args(L, "*s", &name)) {
		return ade_set_error(L, "o", l_LuaAISEXP.Set(lua_ai_sexp_h()));
	}

	if (name == nullptr) {
		return ade_set_error(L, "o", l_LuaAISEXP.Set(lua_ai_sexp_h()));
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting of Lua AI SEXPs is not supported!");
	}

	auto op = get_operator_const(name);

	if (op == 0) {
		LuaError(L, "SEXP '%s' is not known to the SEXP system!", name);
		return ade_set_args(L, "o", l_LuaAISEXP.Set(lua_ai_sexp_h()));
	}

	auto dynamicSEXP = sexp::get_dynamic_sexp(op);

	if (dynamicSEXP == nullptr) {
		return ade_set_args(L, "o", l_LuaAISEXP.Set(lua_ai_sexp_h()));
	}

	if (typeid(*dynamicSEXP) != typeid(sexp::LuaAISEXP)) {
		LuaError(L, "Specified dynamic SEXP name does not refer to a Lua SEXP!");
		return ade_set_error(L, "o", l_LuaAISEXP.Set(lua_ai_sexp_h()));
	}

	return ade_set_args(L, "o", l_LuaAISEXP.Set(lua_ai_sexp_h(static_cast<sexp::LuaAISEXP*>(dynamicSEXP))));
}

static int arrivalListIter(lua_State* L)
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "*o", l_ParseObject.GetPtr(&poh))) {
		return ADE_RETURN_NIL;
	}

	const auto next = GET_NEXT(poh->getObject());

	if (next == END_OF_LIST(&Ship_arrival_list) || next == nullptr) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_ParseObject.Set(parse_object_h(next)));
}

ADE_FUNC(getArrivalList,
	l_Mission,
	nullptr,
	"Get the list of yet to arrive ships for this mission",
	"iterator<parse_object>",
	"An iterator across all the yet to arrive ships. Can be used in a for .. in loop. Is not valid for more than one frame.")
{
	return ade_set_args(L, "u*o", luacpp::LuaFunction::createFromCFunction(L, arrivalListIter),
	                    l_ParseObject.Set(parse_object_h(&Ship_arrival_list)));
}

ADE_FUNC(getShipList,
	l_Mission,
	nullptr,
	"Get an iterator to the list of ships in this mission",
	"iterator<ship>",
	"An iterator across all ships in the mission. Can be used in a for .. in loop. Is not valid for more than one frame.")
{
	ship_obj* so = &Ship_obj_list;

	return ade_set_args(L, "u", luacpp::LuaFunction::createFromStdFunction(L, [so](lua_State* LInner, const luacpp::LuaValueList& /*params*/) mutable -> luacpp::LuaValueList {
		//Since the first element of a list is the next element from the head, and we start this function with the the captured "so" object being the head, this GET_NEXT will return the first element on first call of this lambda.
		//Similarly, an empty list is defined by the head's next element being itself, hence an empty list will immediately return nil just fine
		so = GET_NEXT(so);

		if (so == END_OF_LIST(&Ship_obj_list) || so == nullptr) {
			return luacpp::LuaValueList{ luacpp::LuaValue::createNil(LInner) };
		}

		return luacpp::LuaValueList{ luacpp::LuaValue::createValue(LInner, l_Ship.Set(object_h(&Objects[so->objnum]))) };
	}));
}

ADE_FUNC(getMissileList,
	l_Mission,
	nullptr,
	"Get an iterator to the list of missiles in this mission",
	"iterator<weapon>",
	"An iterator across all missiles in the mission. Can be used in a for .. in loop. Is not valid for more than one frame.")
{
	missile_obj* mo = &Missile_obj_list;

	return ade_set_args(L, "u", luacpp::LuaFunction::createFromStdFunction(L, [mo](lua_State* LInner, const luacpp::LuaValueList& /*params*/) mutable -> luacpp::LuaValueList {
		//Since the first element of a list is the next element from the head, and we start this function with the the captured "mo" object being the head, this GET_NEXT will return the first element on first call of this lambda.
		//Similarly, an empty list is defined by the head's next element being itself, hence an empty list will immediately return nil just fine
		mo = GET_NEXT(mo);

		if (mo == END_OF_LIST(&Missile_obj_list) || mo == nullptr) {
			return luacpp::LuaValueList{ luacpp::LuaValue::createNil(LInner) };
		}

		return luacpp::LuaValueList{ luacpp::LuaValue::createValue(LInner, l_Weapon.Set(object_h(&Objects[mo->objnum]))) };
	}));
}

ADE_FUNC(waitAsync,
	l_Mission,
	"number seconds",
	"Performs an asynchronous wait until the specified amount of mission time has passed.",
	"promise",
	"A promise with no return value that resolves when the specified time has passed")
{
	float time = -1.0f;
	if (!ade_get_args(L, "f", &time)) {
		return ADE_RETURN_NIL;
	}

	if (time <= 0.0f) {
		LuaError(L, "Invalid wait time %f specified. Must be greater than zero.", time);
		return ADE_RETURN_NIL;
	}

	class time_resolve_context : public resolve_context, public std::enable_shared_from_this<time_resolve_context> {
	  public:
		time_resolve_context(int timestamp) : m_timestamp(timestamp) {
			static int unique_id_counter = 0;
			m_unique_id = unique_id_counter++;
			nprintf(("scripting", "waitAsync: Creating asynchronous context %d.\n", m_unique_id));
		}
		void setResolver(Resolver resolver) override
		{
			// Keep checking the time until the timestamp is elapsed
			auto self = shared_from_this();
			auto cb = [this, self, resolver](
						  executor::IExecutionContext::State contextState) {
				if (contextState == executor::IExecutionContext::State::Invalid) {
					mprintf(("waitAsync: Context is invalid, possibly due to a game state change (current state is %s).  Aborting asynchronous context %d.\n", GS_state_text[gameseq_get_state()], m_unique_id));
					resolver(true, luacpp::LuaValueList());
					return executor::Executor::CallbackResult::Done;
				}

				if (timestamp_elapsed(m_timestamp)) {
					nprintf(("scripting", "waitAsync: Timestamp has elapsed for asynchronous context %d.\n", m_unique_id));
					resolver(false, luacpp::LuaValueList());
					return executor::Executor::CallbackResult::Done;
				}

				return executor::Executor::CallbackResult::Reschedule;
			};

			// Use an game state execution context here to clean up references to this as soon as possible
			executor::OnSimulationExecutor->post(
				executor::runInContext(executor::GameStateExecutionContext::captureContext(), std::move(cb)));
		}

	  private:
		int m_timestamp = -1;
		int m_unique_id = -1;
	};
	return ade_set_args(L,
		"o",
		l_Promise.Set(LuaPromise(std::make_shared<time_resolve_context>(timestamp(fl2i(time * 1000.f))))));
}

//****LIBRARY: Campaign
ADE_LIB(l_Campaign, "Campaign", "ca", "Campaign Library");

ADE_FUNC(getNextMissionFilename, l_Campaign, NULL, "Gets next mission filename", "string", "Next mission filename, or nil if the next mission is invalid")
{
	if (Campaign.next_mission < 0 || Campaign.next_mission >= MAX_CAMPAIGN_MISSIONS) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", Campaign.missions[Campaign.next_mission].name);
}

ADE_FUNC(getPrevMissionFilename, l_Campaign, NULL, "Gets previous mission filename", "string", "Previous mission filename, or nil if the previous mission is invalid")
{
	if (Campaign.prev_mission < 0 || Campaign.prev_mission >= MAX_CAMPAIGN_MISSIONS) {
		return ADE_RETURN_NIL;
	}
	return ade_set_args(L, "s", Campaign.missions[Campaign.prev_mission].name);
}

// DahBlount - This jumps to a mission, the reason it accepts a boolean value is so that players can return to campaign maps
ADE_FUNC(jumpToMission, l_Campaign, "string filename, [boolean hub]", "Jumps to a mission based on the filename. Optionally, the player can be sent to a hub mission without setting missions to skipped.", "boolean", "Jumps to a mission, or returns nil.")
{
	const char* filename = nullptr;
	bool hub = false;
	if (!ade_get_args(L, "s|b", &filename, &hub))
		return ADE_RETURN_NIL;

	mission_campaign_jump_to_mission(filename, hub);

	return ADE_RETURN_TRUE;
}

// TODO: add a proper indexer type that returns a handle
// something like ca.Mission[filename/index]



}
}
