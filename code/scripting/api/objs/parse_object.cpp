
#include "parse_object.h"

#include "ship.h"
#include "shipclass.h"
#include "team.h"
#include "vecmath.h"
#include "weaponclass.h"
#include "wing.h"
//#include "globalincs/alphacolors.h" //Needed for team colors

#include "mission/missionparse.h"

#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"

extern bool sexp_check_flag_arrays(const char *flag_name, Object::Object_Flags &object_flag, Ship::Ship_Flags &ship_flags, Mission::Parse_Object_Flags &parse_obj_flag, AI::AI_Flags &ai_flag);
extern void sexp_alter_ship_flag_helper(object_ship_wing_point_team &oswpt, bool future_ships, Object::Object_Flags object_flag, Ship::Ship_Flags ship_flag, Mission::Parse_Object_Flags parse_obj_flag, AI::AI_Flags ai_flag, bool set_flag);

namespace scripting {
namespace api {

parse_object_h::parse_object_h(p_object* obj) : _obj(obj) {}
p_object* parse_object_h::getObject() const { return _obj; }
bool parse_object_h::isValid() const { return _obj != nullptr; }

void parse_object_h::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	parse_object_h pobj(nullptr);
	value.getValue(l_ParseObject.Get(&pobj));
	const ushort& netsig = pobj.isValid() ? pobj._obj->net_signature : 0;
	ADD_USHORT(netsig);
}

void parse_object_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	ushort net_signature;
	GET_USHORT(net_signature);
	new(data_ptr) parse_object_h(mission_parse_get_arrival_ship(net_signature));
}


//**********HANDLE: parse_object
ADE_OBJ(l_ParseObject, parse_object_h, "parse_object", "Handle to a parsed ship");

ADE_VIRTVAR(Name, l_ParseObject, "string",
            "The name of the parsed ship. If possible, don't set the name but set the display name instead.", "string",
            "The name or empty string on error")
{
	parse_object_h* poh = nullptr;
	const char* newName = nullptr;
	if (!ade_get_args(L, "o|s", l_ParseObject.GetPtr(&poh), &newName))
		return ade_set_error(L, "s", "");

	if (poh == nullptr)
		return ade_set_error(L, "s", "");

	if (!poh->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR) {
		strcpy_s(poh->getObject()->name, newName);
	}

	return ade_set_args(L, "s", poh->getObject()->name);
}

ADE_VIRTVAR(
    DisplayName, l_ParseObject, "string",
    "The display name of the parsed ship. If the name should be shown to the user, use this since it can be translated.",
    "string", "The display name or empty string on error")
{
	parse_object_h* poh = nullptr;
	const char* newName = nullptr;
	if (!ade_get_args(L, "o|s", l_ParseObject.GetPtr(&poh), &newName))
		return ade_set_error(L, "s", "");

	if (poh == nullptr)
		return ade_set_error(L, "s", "");

	if (!poh->isValid())
		return ade_set_error(L, "s", "");

	if (ADE_SETTING_VAR) {
		poh->getObject()->display_name = newName;

		// for compatibility reasons, if we are setting this to the empty string, clear the flag
		poh->getObject()->flags.set(Mission::Parse_Object_Flags::SF_Has_display_name, newName[0] != 0);
	}

	return ade_set_args(L, "s", poh->getObject()->get_display_name());
}

ADE_FUNC(isValid, l_ParseObject, nullptr, "Detect whether the parsed ship handle is valid", "boolean", "true if valid false otherwise")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", poh->isValid());
}

ADE_FUNC(getBreedName, l_ParseObject, nullptr, "Gets the FreeSpace type name", "string", "'Parse Object', or empty string if handle is invalid")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ade_set_error(L, "s", "");

	if (!poh->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", "Parse Object");
}

ADE_FUNC(isPlayer, l_ParseObject, nullptr, "Checks whether the parsed ship is a player ship", "boolean", "Whether the parsed ship is a player ship")
{
	parse_object_h *poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ade_set_error(L, "b", false);

	if (!poh->isValid())
		return ade_set_error(L, "b", false);

	// singleplayer
	if (!(Game_mode & GM_MULTIPLAYER))
	{
		if (poh->getObject()->flags[Mission::Parse_Object_Flags::OF_Player_start])
			return ADE_RETURN_TRUE;
		else
			return ADE_RETURN_FALSE;
	}
	// multiplayer
	else
	{
		// try and find the player
		int np_index = multi_find_player_by_parse_object(poh->getObject());
		if ((np_index >= 0) && (np_index < MAX_PLAYERS))
			return ADE_RETURN_TRUE;
		else
			return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(setFlag, l_ParseObject, "boolean set_it, string flag_name", "Sets or clears one or more flags - this function can accept an arbitrary number of flag arguments.  The flag names can be any string that the alter-ship-flag SEXP operator supports.", nullptr, "Returns nothing")
{
	parse_object_h *poh = nullptr;
	bool set_it;
	const char *flag_name;

	if (!ade_get_args(L, "obs", l_ParseObject.GetPtr(&poh), &set_it, &flag_name))
		return ADE_RETURN_NIL;
	int skip_args = 2;	// not 3 because there will be one more below

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	auto pobjp = poh->getObject();
	object_ship_wing_point_team oswpt(pobjp);

	do {
		auto object_flag = Object::Object_Flags::NUM_VALUES;
		auto ship_flag = Ship::Ship_Flags::NUM_VALUES;
		auto parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
		auto ai_flag = AI::AI_Flags::NUM_VALUES;

		sexp_check_flag_arrays(flag_name, object_flag, ship_flag, parse_obj_flag, ai_flag);

		if (parse_obj_flag == Mission::Parse_Object_Flags::NUM_VALUES)
		{
			Warning(LOCATION, "Parsed ship flag '%s' not found!", flag_name);
			return ADE_RETURN_NIL;
		}

		sexp_alter_ship_flag_helper(oswpt, true, object_flag, ship_flag, parse_obj_flag, ai_flag, set_it);

	// read the next flag
	internal::Ade_get_args_skip = ++skip_args;
	} while (ade_get_args(L, "|s", &flag_name) > 0);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getFlag, l_ParseObject, "string flag_name", "Checks whether one or more flags are set - this function can accept an arbitrary number of flag arguments.  The flag names can be any string that the alter-ship-flag SEXP operator supports.", "boolean", "Returns whether all flags are set, or nil if the parsed ship is not valid")
{
	parse_object_h *poh = nullptr;
	const char *flag_name;

	if (!ade_get_args(L, "os", l_ParseObject.GetPtr(&poh), &flag_name))
		return ADE_RETURN_NIL;
	int skip_args = 1;	// not 2 because there will be one more below

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	auto pobjp = poh->getObject();

	do {
		auto object_flag = Object::Object_Flags::NUM_VALUES;
		auto ship_flag = Ship::Ship_Flags::NUM_VALUES;
		auto parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
		auto ai_flag = AI::AI_Flags::NUM_VALUES;

		sexp_check_flag_arrays(flag_name, object_flag, ship_flag, parse_obj_flag, ai_flag);

		if (parse_obj_flag == Mission::Parse_Object_Flags::NUM_VALUES)
		{
			Warning(LOCATION, "Parsed ship flag '%s' not found!", flag_name);
			return ADE_RETURN_FALSE;
		}

		// we only check parse flags, unless this is the one object flag that is the same thing in reverse
		if (object_flag == Object::Object_Flags::Collides)
		{
			if (pobjp->flags[Mission::Parse_Object_Flags::OF_No_collide])
				return ADE_RETURN_FALSE;
		}

		if (parse_obj_flag != Mission::Parse_Object_Flags::NUM_VALUES)
		{
			if (!pobjp->flags[parse_obj_flag])
				return ADE_RETURN_FALSE;
		}

	// read the next flag
	internal::Ade_get_args_skip = ++skip_args;
	} while (ade_get_args(L, "|s", &flag_name) > 0);

	// if we're still here, all the flags we were looking for were present
	return ADE_RETURN_TRUE;
}

static int parse_object_getset_helper(lua_State* L, int p_object::* field, bool canSet = false, bool canBeNegative = false)
{
	parse_object_h* poh;
	int value;
	if (!ade_get_args(L, "o|i", l_ParseObject.GetPtr(&poh), &value))
		return ADE_RETURN_NIL;

	if (!poh || !poh->isValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR)
	{
		if (canSet)
		{
			if (canBeNegative || value >= 0)
				poh->getObject()->*field = value;
		}
		else
			LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", poh->getObject()->*field);
}

ADE_VIRTVAR(Position, l_ParseObject, "vector", "The position at which the parsed ship will arrive.", "vector",
            "The position of the parsed ship.")
{
	parse_object_h* poh = nullptr;
	vec3d* newPos       = nullptr;
	if (!ade_get_args(L, "o|o", l_ParseObject.GetPtr(&poh), l_Vector.GetPtr(&newPos)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (poh == nullptr)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!poh->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (ADE_SETTING_VAR) {
		poh->getObject()->pos = *newPos;
	}

	return ade_set_args(L, "o", l_Vector.Set(poh->getObject()->pos));
}

ADE_VIRTVAR(Orientation, l_ParseObject, "orientation", "The orientation of the parsed ship.", "orientation", "The orientation")
{
	parse_object_h* poh = nullptr;
	matrix_h* newMat    = nullptr;
	if (!ade_get_args(L, "o|o", l_ParseObject.GetPtr(&poh), l_Matrix.GetPtr(&newMat)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (poh == nullptr)
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (!poh->isValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (ADE_SETTING_VAR) {
		poh->getObject()->orient = *newMat->GetMatrix();
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&poh->getObject()->orient)));
}

ADE_VIRTVAR(ShipClass, l_ParseObject, "shipclass", "The ship class of the parsed ship.", "shipclass", "The ship class")
{
	parse_object_h* poh = nullptr;
	int newClass        = -1;
	if (!ade_get_args(L, "o|o", l_ParseObject.GetPtr(&poh), l_Shipclass.Get(&newClass)))
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	if (poh == nullptr)
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	if (!poh->isValid())
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	if (ADE_SETTING_VAR && newClass >= 0) {
		poh->getObject()->ship_class = newClass;
	}

	return ade_set_args(L, "o", l_Shipclass.Set(poh->getObject()->ship_class));
}

ADE_VIRTVAR(Team, l_ParseObject, "team", "The team of the parsed ship.", "team", "The team")
{
	parse_object_h* poh = nullptr;
	int newTeam        = -1;
	if (!ade_get_args(L, "o|o", l_ParseObject.GetPtr(&poh), l_Team.Get(&newTeam)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if (poh == nullptr)
		return ade_set_error(L, "o", l_Team.Set(-1));

	if (!poh->isValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	if (ADE_SETTING_VAR && newTeam >= 0) {
		poh->getObject()->team = newTeam;
	}

	return ade_set_args(L, "o", l_Team.Set(poh->getObject()->team));
}

ADE_VIRTVAR(TeamColor, l_ParseObject, "string", "The team color", "string", "The name of the team color or empty if not set or invalid.")
{
	parse_object_h* poh = nullptr;
	const char* team_color = nullptr;
	if (!ade_get_args(L, "o|s", l_ParseObject.GetPtr(&poh), &team_color))
		return ade_set_error(L, "s", "");

	if (!poh->isValid())
		return ade_set_error(L, "s", "");

	//Set team color
	if (ADE_SETTING_VAR && team_color != nullptr) {

		// Verify
		/*if (Team_Colors.find(team_color) == Team_Colors.end()) {
			mprintf(("Invalid team color specified in mission file for ship %s. Not setting!\n", poh->getObject()->name));
		} else {
			poh->getObject()->team_color_setting = team_color;
		}*/

		LuaError(L, "Setting team colors is not yet supported!");
	
	}	

	return ade_set_args(L, "s", poh->getObject()->team_color_setting);
}

ADE_VIRTVAR(InitialHull, l_ParseObject, "number", "The initial hull percentage of this parsed ship.", "number",
            "The initial hull")
{
	parse_object_h* poh = nullptr;
	int newInitialHull  = -1;
	if (!ade_get_args(L, "o|i", l_ParseObject.GetPtr(&poh), &newInitialHull))
		return ade_set_error(L, "i", -1);

	if (poh == nullptr)
		return ade_set_error(L, "i", -1);

	if (!poh->isValid())
		return ade_set_error(L, "i", -1);

	if (ADE_SETTING_VAR && newInitialHull >= 0 && newInitialHull <= 100) {
		poh->getObject()->initial_hull = newInitialHull;
	}

	return ade_set_args(L, "i", poh->getObject()->initial_hull);
}

ADE_VIRTVAR(InitialShields, l_ParseObject, "number", "The initial shields percentage of this parsed ship.", "number",
            "The initial shields")
{
	parse_object_h* poh   = nullptr;
	int newInitialShields = -1;
	if (!ade_get_args(L, "o|i", l_ParseObject.GetPtr(&poh), &newInitialShields))
		return ade_set_error(L, "i", -1);

	if (poh == nullptr)
		return ade_set_error(L, "i", -1);

	if (!poh->isValid())
		return ade_set_error(L, "i", -1);

	if (ADE_SETTING_VAR && newInitialShields >= 0 && newInitialShields <= 100) {
		poh->getObject()->initial_shields = newInitialShields;
	}

	return ade_set_args(L, "i", poh->getObject()->initial_shields);
}

ADE_VIRTVAR(MainStatus, l_ParseObject, nullptr,
            "Gets the \"subsystem\" status of the ship itself. This is a special subsystem that represents the primary "
            "and secondary weapons and the AI class.",
            "parse_subsystem", "The subsystem handle or invalid handle if there were no changes to the main status")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ade_set_error(L, "o", l_ParseSubsystem.Set(parse_subsys_h()));

	if (poh == nullptr)
		return ade_set_error(L, "o", l_ParseSubsystem.Set(parse_subsys_h()));

	if (!poh->isValid())
		return ade_set_error(L, "o", l_ParseSubsystem.Set(parse_subsys_h()));

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	auto po = poh->getObject();
	for (int i = 0; i < po->subsys_count; ++i) {
		if (!stricmp(Subsys_status[po->subsys_index + i].name, "Pilot")) {
			return ade_set_args(L, "o", l_ParseSubsystem.Set(parse_subsys_h(po, i)));
		}
	}

	return ade_set_args(L, "o", l_ParseSubsystem.Set(parse_subsys_h()));
}

ADE_VIRTVAR(Subsystems, l_ParseObject, nullptr, "Get the list of subsystems of this parsed ship",
            "parse_subsystem[]", "An array of the parse subsystems of this parsed ship")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ADE_RETURN_NIL;

	if (poh == nullptr)
		return ADE_RETURN_NIL;

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	luacpp::LuaTable tbl = luacpp::LuaTable::create(L);
	auto po              = poh->getObject();
	int luaIdx           = 1;
	for (int i = 0; i < po->subsys_count; ++i) {
		// Only include the "non-main" subsystems
		if (stricmp(Subsys_status[po->subsys_index + i].name, "Pilot")) {
			tbl.addValue(luaIdx, l_ParseSubsystem.Set(parse_subsys_h(poh->getObject(), i)));
			++luaIdx;
		}
	}

	return ade_set_args(L, "t", tbl);
}

template <typename LOC>
static int parse_object_getset_location_helper(lua_State* L, LOC p_object::* field, const char* location_type, const char** location_names, size_t location_names_size)
{
	parse_object_h* poh;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_ParseObject.GetPtr(&poh), &s))
		return ADE_RETURN_NIL;

	if (!poh || !poh->isValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR && s != nullptr)
	{
		int location = string_lookup(s, location_names, location_names_size);
		if (location < 0)
		{
			Warning(LOCATION, "%s location '%s' not found.", location_type, s);
			return ADE_RETURN_NIL;
		}
		poh->getObject()->*field = static_cast<LOC>(location);
	}

	return ade_set_args(L, "s", location_names[static_cast<int>(poh->getObject()->*field)]);
}

ADE_VIRTVAR(ArrivalLocation, l_ParseObject, "string", "The ship's arrival location", "string", "Arrival location, or nil if handle is invalid")
{
	return parse_object_getset_location_helper(L, &p_object::arrival_location, "Arrival", Arrival_location_names, MAX_ARRIVAL_NAMES);
}

ADE_VIRTVAR(DepartureLocation, l_ParseObject, "string", "The ship's departure location", "string", "Departure location, or nil if handle is invalid")
{
	return parse_object_getset_location_helper(L, &p_object::departure_location, "Departure", Departure_location_names, MAX_DEPARTURE_NAMES);
}

static int parse_object_getset_anchor_helper(lua_State* L, int p_object::* field)
{
	parse_object_h* poh;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_ParseObject.GetPtr(&poh), &s))
		return ADE_RETURN_NIL;

	if (!poh || !poh->isValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR && s != nullptr)
	{
		poh->getObject()->*field = (stricmp(s, "<no anchor>") == 0) ? -1 : get_parse_name_index(s);
	}

	return ade_set_args(L, "s", (poh->getObject()->*field >= 0) ? Parse_names[poh->getObject()->*field].c_str() : "<no anchor>");
}

ADE_VIRTVAR(ArrivalAnchor, l_ParseObject, "string", "The ship's arrival anchor", "string", "Arrival anchor, or nil if handle is invalid")
{
	return parse_object_getset_anchor_helper(L, &p_object::arrival_anchor);
}

ADE_VIRTVAR(DepartureAnchor, l_ParseObject, "string", "The ship's departure anchor", "string", "Departure anchor, or nil if handle is invalid")
{
	return parse_object_getset_anchor_helper(L, &p_object::departure_anchor);
}

ADE_VIRTVAR(ArrivalPathMask, l_ParseObject, "number", "The ship's arrival path mask", "number", "Arrival path mask, or nil if handle is invalid")
{
	return parse_object_getset_helper(L, &p_object::arrival_path_mask, true);
}

ADE_VIRTVAR(DeparturePathMask, l_ParseObject, "number", "The ship's departure path mask", "number", "Departure path mask, or nil if handle is invalid")
{
	return parse_object_getset_helper(L, &p_object::departure_path_mask, true);
}

ADE_VIRTVAR(ArrivalDelay, l_ParseObject, "number", "The ship's arrival delay", "number", "Arrival delay, or nil if handle is invalid")
{
	return parse_object_getset_helper(L, &p_object::arrival_delay, true);
}

ADE_VIRTVAR(DepartureDelay, l_ParseObject, "number", "The ship's departure delay", "number", "Departure delay, or nil if handle is invalid")
{
	return parse_object_getset_helper(L, &p_object::departure_delay, true);
}

ADE_VIRTVAR(ArrivalDistance, l_ParseObject, "number", "The ship's arrival distance", "number", "Arrival distance, or nil if handle is invalid")
{
	return parse_object_getset_helper(L, &p_object::arrival_distance, true);
}

ADE_FUNC(isPlayerStart, l_ParseObject, nullptr, "Determines if this parsed ship is a player start.", "boolean",
         "true if player start, false if not or if invalid")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ade_set_error(L, "b", false);

	if (poh == nullptr)
		return ade_set_error(L, "b", false);

	if (!poh->isValid())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", poh->getObject()->flags[Mission::Parse_Object_Flags::OF_Player_start]);
}

ADE_FUNC(getShip, l_ParseObject, nullptr, "Returns the ship that was created from this parsed ship, if it is present in the mission.  Note that parse objects are reused when a wing has multiple waves, so this will always return a ship from the most recently created wave.", "ship", "The created ship, an invalid handle if no ship exists, or nil if the current handle is invalid")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ADE_RETURN_NIL;

	if (!poh || !poh->isValid())
		return ADE_RETURN_NIL;

	auto objp = poh->getObject()->created_object;
	if (!objp)
		return ade_set_args(L, "o", l_Ship.Set(-1));

	return ade_set_object_with_breed(L, OBJ_INDEX(objp));
}

ADE_FUNC(getWing, l_ParseObject, nullptr, "Returns the wing that this parsed ship belongs to, if any", "wing", "The parsed ship's wing, an invalid wing handle if no wing exists, or nil if the handle is invalid")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ADE_RETURN_NIL;

	if (poh == nullptr)
		return ADE_RETURN_NIL;

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "o", l_Wing.Set(poh->getObject()->wingnum));
}

ADE_FUNC(makeShipArrive, l_ParseObject, nullptr, "Causes this parsed ship to arrive as if its arrival cue had become true.  Note that reinforcements are only marked as available, not actually created.", "boolean", "true if created, false otherwise")
{
	parse_object_h* poh = nullptr;
	if (!ade_get_args(L, "o", l_ParseObject.GetPtr(&poh)))
		return ADE_RETURN_NIL;

	if (poh == nullptr)
		return ADE_RETURN_NIL;

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	poh->getObject()->arrival_delay = 0;
	return mission_maybe_make_ship_arrive(poh->getObject(), true) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
}

ADE_VIRTVAR(CollisionGroups, l_ParseObject, "number", "Collision group data", "number", "Current set of collision groups. NOTE: This is a bitfield, NOT a normal number.")
{
	parse_object_h* poh = nullptr;
	int id = 0;
	if (!ade_get_args(L, "o|i", l_ParseObject.GetPtr(&poh), &id))
		return ade_set_error(L, "i", 0);

	if (!poh->isValid())
		return ade_set_error(L, "i", 0);

	//Set collision group data
	if (ADE_SETTING_VAR)
		poh->getObject()->collision_group_id = id;

	return ade_set_args(L, "i", poh->getObject()->collision_group_id);
}

ADE_FUNC(addToCollisionGroup, l_ParseObject, "number group", "Adds this parsed ship to the specified collision group.  The group must be between 0 and 31, inclusive.", nullptr, "Returns nothing")
{
	parse_object_h* poh = nullptr;
	int group;

	if (!ade_get_args(L, "oi", l_ParseObject.GetPtr(&poh), &group))
		return ADE_RETURN_NIL;

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	if (group >= 0 && group <= 31)
		poh->getObject()->collision_group_id |= (1 << group);
	else
		Warning(LOCATION, "In addToCollisionGroup, group %d must be between 0 and 31, inclusive", group);

	return ADE_RETURN_NIL;
}

ADE_FUNC(removeFromCollisionGroup, l_ParseObject, "number group", "Removes this parsed ship from the specified collision group.  The group must be between 0 and 31, inclusive.", nullptr, "Returns nothing")
{
	parse_object_h* poh = nullptr;
	int group;

	if (!ade_get_args(L, "oi", l_ParseObject.GetPtr(&poh), &group))
		return ADE_RETURN_NIL;

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	if (group >= 0 && group <= 31)
		poh->getObject()->collision_group_id &= ~(1 << group);
	else
		Warning(LOCATION, "In removeFromCollisionGroup, group %d must be between 0 and 31, inclusive", group);

	return ADE_RETURN_NIL;
}

parse_subsys_h::parse_subsys_h() = default;
parse_subsys_h::parse_subsys_h(p_object* obj, int subsys_offset) : _obj(obj), _subsys_offset(subsys_offset) {}
subsys_status* parse_subsys_h::getSubsys() const { return &Subsys_status[_obj->subsys_index + _subsys_offset]; }
bool parse_subsys_h::isValid() const { return _obj != nullptr && _subsys_offset < _obj->subsys_count; }

void parse_subsys_h::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	parse_subsys_h pobj;
	value.getValue(l_ParseSubsystem.Get(&pobj));
	const ushort& netsig = pobj.isValid() ? pobj._obj->net_signature : 0;
	ADD_USHORT(netsig);
	ADD_INT(pobj._subsys_offset);
}

void parse_subsys_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	ushort net_signature;
	int ss;
	GET_USHORT(net_signature);
	GET_INT(ss);
	new(data_ptr) parse_subsys_h(mission_parse_get_arrival_ship(net_signature), ss);
}

//**********HANDLE: parse_object
ADE_OBJ(l_ParseSubsystem, parse_subsys_h, "parse_subsystem", "Handle to a parse subsystem");

ADE_VIRTVAR(Name, l_ParseSubsystem, "string",
            "The name of the subsystem. If possible, don't set the name but set the display name instead.", "string",
            "The name or empty string on error")
{
	parse_subsys_h* poh = nullptr;
	const char* newName = nullptr;
	if (!ade_get_args(L, "o|s", l_ParseSubsystem.GetPtr(&poh), &newName)) {
		return ade_set_error(L, "s", "");
	}

	if (poh == nullptr) {
		return ade_set_error(L, "s", "");
	}

	if (!poh->isValid()) {
		return ade_set_error(L, "s", "");
	}

	if (ADE_SETTING_VAR) {
		strcpy_s(poh->getSubsys()->name, newName);
	}

	return ade_set_args(L, "s", poh->getSubsys()->name);
}

ADE_VIRTVAR(Damage, l_ParseSubsystem, "number", "The percentage to what the subsystem is damage", "number",
            "The percentage or negative on error")
{
	parse_subsys_h* poh = nullptr;
	float newPercentage = -1.0f;
	if (!ade_get_args(L, "o|f", l_ParseSubsystem.GetPtr(&poh), &newPercentage)) {
		return ade_set_error(L, "f", -1.0f);
	}

	if (poh == nullptr) {
		return ade_set_error(L, "f", -1.0f);
	}

	if (!poh->isValid()) {
		return ade_set_error(L, "f", -1.0f);
	}

	if (ADE_SETTING_VAR && newPercentage >= 0.0f && newPercentage <= 100.0f) {
		poh->getSubsys()->percent = newPercentage;
	}

	return ade_set_args(L, "f", poh->getSubsys()->percent);
}

ADE_VIRTVAR(PrimaryBanks, l_ParseSubsystem, nullptr, "The overridden primary banks", "weaponclass[]",
            "The primary bank weapons or nil if not changed from default")
{
	parse_subsys_h* poh = nullptr;
	float newPercentage = -1.0f;
	if (!ade_get_args(L, "o|f", l_ParseSubsystem.GetPtr(&poh), &newPercentage)) {
		return ADE_RETURN_NIL;
	}

	if (poh == nullptr) {
		return ADE_RETURN_NIL;
	}

	if (!poh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (poh->getSubsys()->primary_banks[0] == SUBSYS_STATUS_NO_CHANGE) {
		return ADE_RETURN_NIL;
	}

	luacpp::LuaTable tbl = luacpp::LuaTable::create(L);
	auto po              = poh->getSubsys();
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; ++i) {
		if (po->primary_banks[i] == -1) {
			break;
		}
		tbl.addValue(i + 1, l_Weaponclass.Set(po->primary_banks[i]));
	}

	return ade_set_args(L, "t", tbl);
}

ADE_VIRTVAR(PrimaryAmmo, l_ParseSubsystem, nullptr, "The overridden primary ammunition, as a percentage of the default",
            "weaponclass[]", "The primary bank ammunition percantage or nil if not changed from default")
{
	parse_subsys_h* poh = nullptr;
	float newPercentage = -1.0f;
	if (!ade_get_args(L, "o|f", l_ParseSubsystem.GetPtr(&poh), &newPercentage)) {
		return ADE_RETURN_NIL;
	}

	if (poh == nullptr) {
		return ADE_RETURN_NIL;
	}

	if (!poh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	// Check the primary banks to see if changes were applied
	if (poh->getSubsys()->primary_banks[0] == SUBSYS_STATUS_NO_CHANGE) {
		return ADE_RETURN_NIL;
	}

	luacpp::LuaTable tbl = luacpp::LuaTable::create(L);
	auto po              = poh->getSubsys();
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; ++i) {
		if (po->primary_banks[i] == -1) {
			break;
		}
		tbl.addValue(i + 1, po->primary_ammo[i]);
	}

	return ade_set_args(L, "t", tbl);
}

ADE_VIRTVAR(SecondaryBanks, l_ParseSubsystem, nullptr, "The overridden secondary banks", "weaponclass[]",
            "The secondary bank weapons or nil if not changed from default")
{
	parse_subsys_h* poh = nullptr;
	float newPercentage = -1.0f;
	if (!ade_get_args(L, "o|f", l_ParseSubsystem.GetPtr(&poh), &newPercentage)) {
		return ADE_RETURN_NIL;
	}

	if (poh == nullptr) {
		return ADE_RETURN_NIL;
	}

	if (!poh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	if (poh->getSubsys()->secondary_banks[0] == SUBSYS_STATUS_NO_CHANGE) {
		return ADE_RETURN_NIL;
	}

	luacpp::LuaTable tbl = luacpp::LuaTable::create(L);
	auto po              = poh->getSubsys();
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; ++i) {
		if (po->secondary_banks[i] == -1) {
			break;
		}
		tbl.addValue(i + 1, l_Weaponclass.Set(po->secondary_banks[i]));
	}

	return ade_set_args(L, "t", tbl);
}

ADE_VIRTVAR(SecondaryAmmo, l_ParseSubsystem, nullptr, "The overridden secondary ammunition, as a percentage of the default",
            "weaponclass[]", "The secondary bank ammunition percantage or nil if not changed from default")
{
	parse_subsys_h* poh = nullptr;
	float newPercentage = -1.0f;
	if (!ade_get_args(L, "o|f", l_ParseSubsystem.GetPtr(&poh), &newPercentage)) {
		return ADE_RETURN_NIL;
	}

	if (poh == nullptr) {
		return ADE_RETURN_NIL;
	}

	if (!poh->isValid()) {
		return ADE_RETURN_NIL;
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	// Check the secondary banks to see if changes were applied
	if (poh->getSubsys()->secondary_banks[0] == SUBSYS_STATUS_NO_CHANGE) {
		return ADE_RETURN_NIL;
	}

	luacpp::LuaTable tbl = luacpp::LuaTable::create(L);
	auto po              = poh->getSubsys();
	for (int i = 0; i < MAX_SHIP_PRIMARY_BANKS; ++i) {
		if (po->secondary_banks[i] == -1) {
			break;
		}
		tbl.addValue(i + 1, po->secondary_ammo[i]);
	}

	return ade_set_args(L, "t", tbl);
}

} // namespace api
} // namespace scripting
