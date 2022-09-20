
#include "parse_object.h"

#include "shipclass.h"
#include "vecmath.h"
#include "weaponclass.h"

extern bool sexp_check_flag_arrays(const char *flag_name, Object::Object_Flags &object_flag, Ship::Ship_Flags &ship_flags, Mission::Parse_Object_Flags &parse_obj_flag, AI::AI_Flags &ai_flag);
extern void sexp_alter_ship_flag_helper(object_ship_wing_point_team &oswpt, bool future_ships, Object::Object_Flags object_flag, Ship::Ship_Flags ship_flag, Mission::Parse_Object_Flags parse_obj_flag, AI::AI_Flags ai_flag, bool set_flag);

namespace scripting {
namespace api {

parse_object_h::parse_object_h(p_object* obj) : _obj(obj) {}
p_object* parse_object_h::getObject() const { return _obj; }
bool parse_object_h::isValid() const { return _obj != nullptr; }

//**********HANDLE: parse_object
ADE_OBJ(l_ParseObject, parse_object_h, "parse_object", "Handle to a parse object");

ADE_VIRTVAR(Name, l_ParseObject, "string",
            "The name of the object. If possible, don't set the name but set the display name instead.", "string",
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
    "The display name of the object. If the name should be shown to the user, use this since it can be translated.",
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

ADE_FUNC(setFlag, l_ParseObject, "boolean set_it, string flag_name1, [string flag_name2, string flag_name3, string flag_name4, string flag_name5]", "Sets or clears a flag or series of flags.  The flag name can be any string that the alter-ship-flag SEXP operator accepts.", nullptr, "Returns nothing")
{
	parse_object_h *poh = nullptr;
	bool set_it;
	const char *flag_name[5];

	int num_args = ade_get_args(L, "obs|ssss", l_ParseObject.GetPtr(&poh), &set_it, &flag_name[0], &flag_name[1], &flag_name[2], &flag_name[3], &flag_name[4]);
	if (num_args < 3)
		return ADE_RETURN_NIL;

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	auto pobjp = poh->getObject();
	object_ship_wing_point_team oswpt(pobjp);

	for (int i = 0; i < num_args - 2; i++)
	{
		auto object_flag = Object::Object_Flags::NUM_VALUES;
		auto ship_flag = Ship::Ship_Flags::NUM_VALUES;
		auto parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
		auto ai_flag = AI::AI_Flags::NUM_VALUES;

		sexp_check_flag_arrays(flag_name[i], object_flag, ship_flag, parse_obj_flag, ai_flag);
		sexp_alter_ship_flag_helper(oswpt, true, object_flag, ship_flag, parse_obj_flag, ai_flag, set_it);
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(getFlag, l_ParseObject, "string flag_name1, [string flag_name2, string flag_name3, string flag_name4, string flag_name5]", "Checks whether one or more specified flags are set.  The flag name can be any string that the alter-ship-flag SEXP operator accepts.", "boolean", "Returns whether all flags are set, or nil if the ship is not valid")
{
	parse_object_h *poh = nullptr;
	const char *flag_name[5];

	int num_args = ade_get_args(L, "os|ssss", l_ParseObject.GetPtr(&poh), &flag_name[0], &flag_name[1], &flag_name[2], &flag_name[3], &flag_name[4]);
	if (num_args < 2)
		return ADE_RETURN_NIL;

	if (!poh->isValid())
		return ADE_RETURN_NIL;

	auto pobjp = poh->getObject();

	for (int i = 0; i < num_args - 1; i++)
	{
		auto object_flag = Object::Object_Flags::NUM_VALUES;
		auto ship_flag = Ship::Ship_Flags::NUM_VALUES;
		auto parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
		auto ai_flag = AI::AI_Flags::NUM_VALUES;

		sexp_check_flag_arrays(flag_name[i], object_flag, ship_flag, parse_obj_flag, ai_flag);

		// we only check parse flags

		if (parse_obj_flag != Mission::Parse_Object_Flags::NUM_VALUES)
		{
			if (!pobjp->flags[parse_obj_flag])
				return ADE_RETURN_FALSE;
		}
	}

	// if we're still here, all the flags we were looking for were present
	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(Position, l_ParseObject, "vector", "The position at which the object will arrive.", "vector",
            "The position of the object.")
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

ADE_VIRTVAR(Orientation, l_ParseObject, "orientation", "The orientation of the object.", "orientation", "The orientation")
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

ADE_VIRTVAR(ShipClass, l_ParseObject, "shipclass", "The ship class of the object.", "shipclass", "The ship class")
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

ADE_VIRTVAR(InitialHull, l_ParseObject, "number", "The initial hull percentage of this object.", "number",
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

ADE_VIRTVAR(InitialShields, l_ParseObject, "number", "The initial shields percentage of this object.", "number",
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

ADE_VIRTVAR(Subsystems, l_ParseObject, nullptr, "Get the list of subsystems of this parse object",
            "parse_subsystem[]", "An array of the parse subsystems of this object")
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

ADE_FUNC(isPlayerStart, l_ParseObject, nullptr, "Determines if this parse object is a player start.", "boolean",
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

parse_subsys_h::parse_subsys_h() = default;
parse_subsys_h::parse_subsys_h(p_object* obj, int subsys_offset) : _obj(obj), _subsys_offset(subsys_offset) {}
subsys_status* parse_subsys_h::getSubsys() const { return &Subsys_status[_obj->subsys_index + _subsys_offset]; }
bool parse_subsys_h::isValid() const { return _obj != nullptr && _subsys_offset < _obj->subsys_count; }

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
