//
//


#include "animation_handle.h"
#include "cockpit_display.h"
#include "enums.h"
#include "message.h"
#include "modelinstance.h"
#include "object.h"
#include "order.h"
#include "parse_object.h"
#include "ship.h"
#include "ship_bank.h"
#include "shipclass.h"
#include "subsystem.h"
#include "team.h"
#include "texture.h"
#include "vecmath.h"
#include "weaponclass.h"
#include "wing.h"

#include "ai/aigoals.h"
#include "globalincs/utility.h"
#include "hud/hudets.h"
#include "hud/hudshield.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "model/model.h"
#include "network/multiutil.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "parse/parselo.h"
#include "playerman/player.h"
#include "scripting/api/objs/message.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "waypoint.h"

#include "scripting/lua/LuaException.h"
#include "scripting/lua/LuaUtil.h"

extern void ship_reset_disabled_physics(object *objp, int ship_class);
extern bool sexp_check_flag_arrays(const char *flag_name, Object::Object_Flags &object_flag, Ship::Ship_Flags &ship_flags, Mission::Parse_Object_Flags &parse_obj_flag, AI::AI_Flags &ai_flag);
extern void sexp_alter_ship_flag_helper(object_ship_wing_point_team &oswpt, bool future_ships, Object::Object_Flags object_flag, Ship::Ship_Flags ship_flag, Mission::Parse_Object_Flags parse_obj_flag, AI::AI_Flags ai_flag, bool set_flag);
extern void interp_generate_arc_segment(SCP_vector<vec3d> &arc_segment_points, const vec3d *v1, const vec3d *v2, ubyte depth_limit, ubyte depth);

namespace scripting {
namespace api {

//**********HANDLE: Ship
ADE_OBJ_DERIV(l_Ship, object_h, "ship", "Ship handle", l_Object);

ADE_INDEXER(l_Ship, "string/number NameOrIndex", "Array of ship subsystems", "subsystem", "Subsystem handle, or invalid subsystem handle if index or ship handle is invalid")
{
	object_h *objh;
	const char* s      = nullptr;
	ship_subsys_h *sub = nullptr;
	if(!ade_get_args(L, "o|so", l_Ship.GetPtr(&objh), &s, l_Subsystem.GetPtr(&sub)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ship *shipp = &Ships[objh->objp()->instance];
	ship_subsys *ss = ship_get_subsys(shipp, s);

	if(ss == NULL)
	{
		int idx = atoi(s);
		if(idx > 0 && idx <= ship_get_num_subsys(shipp))
		{
			idx--; //Lua->FS2
			ss = ship_get_indexed_subsys(shipp, idx);
		}
	}

	if(ss == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(objh->objp(), ss)));
}

ADE_FUNC(__len, l_Ship, NULL, "Number of subsystems on ship", "number", "Subsystem number, or 0 if handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->isValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", ship_get_num_subsys(&Ships[objh->objp()->instance]));
}

ADE_FUNC(getSubsystemList,
	l_Ship,
	nullptr,
	"Get the list of subsystems on this ship",
	"iterator<subsystem>",
	"An iterator across all subsystems on the ship. Can be used in a for .. in loop. Is not valid for more than one frame.")
{
	object_h* objh;
	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	ship* shipp = &Ships[objh->objp()->instance];
	ship_subsys* ss = &shipp->subsys_list;

	return ade_set_args(L, "u", luacpp::LuaFunction::createFromStdFunction(L, [shipp, ss](lua_State* LInner, const luacpp::LuaValueList& /*params*/) mutable -> luacpp::LuaValueList {
		//Since the first element of a list is the next element from the head, and we start this function with the captured "ss" object being the head, this GET_NEXT will return the first element on first call of this lambda.
		//Similarly, an empty list is defined by the head's next element being itself, hence an empty list will immediately return nil just fine
		ss = GET_NEXT(ss);

		if (ss == END_OF_LIST(&shipp->subsys_list) || ss == nullptr) {
			return luacpp::LuaValueList{ luacpp::LuaValue::createNil(LInner) };
		}

		return luacpp::LuaValueList{ luacpp::LuaValue::createValue(LInner, l_Subsystem.Set(ship_subsys_h(&Objects[shipp->objnum], ss))) };
	}));
}

ADE_FUNC(setFlag, l_Ship, "boolean set_it, string flag_name", "Sets or clears one or more flags - this function can accept an arbitrary number of flag arguments.  The flag names can be any string that the alter-ship-flag SEXP operator supports.", nullptr, "Returns nothing")
{
	object_h *objh;
	bool set_it;
	const char *flag_name;

	if (!ade_get_args(L, "obs", l_Ship.GetPtr(&objh), &set_it, &flag_name))
		return ADE_RETURN_NIL;
	int skip_args = 2;	// not 3 because there will be one more below

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto shipp = &Ships[objh->objp()->instance];
	object_ship_wing_point_team oswpt(shipp);

	do {
		auto object_flag = Object::Object_Flags::NUM_VALUES;
		auto ship_flag = Ship::Ship_Flags::NUM_VALUES;
		auto parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
		auto ai_flag = AI::AI_Flags::NUM_VALUES;

		sexp_check_flag_arrays(flag_name, object_flag, ship_flag, parse_obj_flag, ai_flag);

		if (object_flag == Object::Object_Flags::NUM_VALUES && ship_flag == Ship::Ship_Flags::NUM_VALUES && ai_flag == AI::AI_Flags::NUM_VALUES)
		{
			Warning(LOCATION, "Ship/object/ai flag '%s' not found!", flag_name);
			return ADE_RETURN_NIL;
		}

		sexp_alter_ship_flag_helper(oswpt, true, object_flag, ship_flag, parse_obj_flag, ai_flag, set_it);

	// read the next flag
	internal::Ade_get_args_skip = ++skip_args;
	} while (ade_get_args(L, "|s", &flag_name) > 0);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getFlag, l_Ship, "string flag_name", "Checks whether one or more flags are set - this function can accept an arbitrary number of flag arguments.  The flag names can be any string that the alter-ship-flag SEXP operator supports.", "boolean", "Returns whether all flags are set, or nil if the ship is not valid")
{
	object_h *objh;
	const char *flag_name;

	if (!ade_get_args(L, "os", l_Ship.GetPtr(&objh), &flag_name))
		return ADE_RETURN_NIL;
	int skip_args = 1;	// not 2 because there will be one more below

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto objp = objh->objp();
	auto shipp = &Ships[objp->instance];
	auto aip = &Ai_info[shipp->ai_index];

	do {
		auto object_flag = Object::Object_Flags::NUM_VALUES;
		auto ship_flag = Ship::Ship_Flags::NUM_VALUES;
		auto parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
		auto ai_flag = AI::AI_Flags::NUM_VALUES;

		sexp_check_flag_arrays(flag_name, object_flag, ship_flag, parse_obj_flag, ai_flag);

		if (object_flag == Object::Object_Flags::NUM_VALUES && ship_flag == Ship::Ship_Flags::NUM_VALUES && ai_flag == AI::AI_Flags::NUM_VALUES)
		{
			Warning(LOCATION, "Ship/object/ai flag '%s' not found!", flag_name);
			return ADE_RETURN_FALSE;
		}

		// now check the flags
		if (object_flag != Object::Object_Flags::NUM_VALUES)
		{
			if (!(objp->flags[object_flag]))
				return ADE_RETURN_FALSE;
		}

		if (ship_flag != Ship::Ship_Flags::NUM_VALUES)
		{
			if (!(shipp->flags[ship_flag]))
				return ADE_RETURN_FALSE;
		}

		// we don't check parse flags, except for one that can be an object flag in reverse
		if (parse_obj_flag == Mission::Parse_Object_Flags::OF_No_collide)
		{
			if (objp->flags[Object::Object_Flags::Collides])
				return ADE_RETURN_FALSE;
		}

		if (ai_flag != AI::AI_Flags::NUM_VALUES)
		{
			if (!(aip->ai_flags[ai_flag]))
				return ADE_RETURN_FALSE;
		}

	// read the next flag
	internal::Ade_get_args_skip = ++skip_args;
	} while (ade_get_args(L, "|s", &flag_name) > 0);

	// if we're still here, all the flags we were looking for were present
	return ADE_RETURN_TRUE;
}

static int ship_getset_helper(lua_State* L, int ship::* field, bool canSet = false, bool canBeNegative = false)
{
	object_h* objh;
	int value;
	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &value))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	ship* shipp = &Ships[objh->objp()->instance];

	if (ADE_SETTING_VAR)
	{
		if (canSet)
		{
			if (canBeNegative || value >= 0)
				shipp->*field = value;
		}
		else
			LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "i", shipp->*field);
}

ADE_VIRTVAR(ShieldArmorClass, l_Ship, "string", "Current Armor class of the ships' shield", "string", "Armor class name, or empty string if none is set")
{
	object_h *objh;
	const char* s    = nullptr;
	const char *name = nullptr;

	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp()->instance];
	int atindex;
	if (ADE_SETTING_VAR && s != nullptr) {
		atindex = armor_type_get_idx(s);
		shipp->shield_armor_type_idx = atindex;
	} else {
		atindex = shipp->shield_armor_type_idx;
	}

	if (atindex != -1)
		name = Armor_types[atindex].GetNamePtr();
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(ImpactDamageClass, l_Ship, "string", "Current Impact Damage class", "string", "Impact Damage class name, or empty string if none is set")
{
	object_h *objh;
	const char* s    = nullptr;
	const char *name = nullptr;

	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp()->instance];
	int damage_index;

	if (ADE_SETTING_VAR && s != nullptr) {
		damage_index = find_item_with_string(Damage_types, &DamageTypeStruct::name, s);
		shipp->collision_damage_type_idx = damage_index;
	} else {
		damage_index = shipp->collision_damage_type_idx;
	}

	if (damage_index != -1)
		name = Damage_types[damage_index].name;
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(ArmorClass, l_Ship, "string", "Current Armor class", "string", "Armor class name, or empty string if none is set")
{
	object_h *objh;
	const char* s    = nullptr;
	const char *name = nullptr;

	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp()->instance];
	int atindex;
	if (ADE_SETTING_VAR && s != nullptr) {
		atindex = armor_type_get_idx(s);
		shipp->armor_type_idx = atindex;
	} else {
		atindex = shipp->armor_type_idx;
	}

	if (atindex != -1)
		name = Armor_types[atindex].GetNamePtr();
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(Name, l_Ship, "string", "Ship name. This is the actual name of the ship. Use <i>getDisplayString</i> to get the string which should be displayed to the player.", "string", "Ship name, or empty string if handle is invalid")
{
	object_h *objh;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(shipp->ship_name);
		strncpy(shipp->ship_name, s, len);
		shipp->ship_name[len - 1] = 0;
	}

	return ade_set_args(L, "s", shipp->ship_name);
}

ADE_VIRTVAR(DisplayName, l_Ship, "string", "Ship display name", "string", "The display name of the ship or empty if there is no display string")
{
	object_h *objh;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ade_set_error(L, "s", "");

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && s != nullptr) {
		shipp->display_name = s;

		// for compatibility reasons, if we are setting this to the empty string, clear the flag
		shipp->flags.set(Ship::Ship_Flags::Has_display_name, s[0] != 0);
	}

	return ade_set_args(L, "s", shipp->display_name.c_str());
}

ADE_FUNC(isPlayer, l_Ship, nullptr, "Checks whether the ship is a player ship", "boolean", "Whether the ship is a player ship")
{
	object_h* objh;
	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "b", false);

	if (!objh->isValid())
		return ade_set_error(L, "b", false);

	// singleplayer
	if (!(Game_mode & GM_MULTIPLAYER))
	{
		if (Player_obj == objh->objp())
			return ADE_RETURN_TRUE;
		else
			return ADE_RETURN_FALSE;
	}
	// multiplayer
	else
	{
		// try and find the player
		int np_index = multi_find_player_by_object(objh->objp());
		if ((np_index >= 0) && (np_index < MAX_PLAYERS))
			return ADE_RETURN_TRUE;
		else
			return ADE_RETURN_FALSE;
	}
}

ADE_VIRTVAR(AfterburnerFuelLeft, l_Ship, "number", "Afterburner fuel left", "number", "Afterburner fuel left, or 0 if handle is invalid")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && fuel >= 0.0f)
		shipp->afterburner_fuel = fuel;

	return ade_set_args(L, "f", shipp->afterburner_fuel);
}

ADE_VIRTVAR(AfterburnerFuelMax, l_Ship, "number", "Afterburner fuel capacity", "number", "Afterburner fuel capacity, or 0 if handle is invalid")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship_info *sip = &Ship_info[Ships[objh->objp()->instance].ship_info_index];

	if(ADE_SETTING_VAR && fuel >= 0.0f)
		sip->afterburner_fuel_capacity = fuel;

	return ade_set_args(L, "f", sip->afterburner_fuel_capacity);
}

ADE_VIRTVAR(Class, l_Ship, "shipclass", "Ship class", "shipclass", "Ship class, or invalid shipclass handle if ship handle is invalid")
{
	object_h *objh;
	int idx=-1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Shipclass.Get(&idx)))
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && idx > -1) {
		change_ship_type(objh->objp()->instance, idx, 1);
		if (shipp == Player_ship) {
			// update HUD and RTT cockpit gauges if applicable
			set_current_hud();
			ship_close_cockpit_displays(Player_ship);
			ship_init_cockpit_displays(Player_ship);
		}
	}

	if(shipp->ship_info_index < 0)
		return ade_set_error(L, "o", l_Shipclass.Set(-1));

	return ade_set_args(L, "o", l_Shipclass.Set(shipp->ship_info_index));
}

ADE_VIRTVAR(CountermeasuresLeft, l_Ship, "number", "Number of countermeasures left", "number", "Countermeasures left, or 0 if ship handle is invalid")
{
	object_h *objh;
	int newcm = -1;
	if(!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &newcm))
		return ade_set_error(L, "i", 0);

	if(!objh->isValid())
		return ade_set_error(L, "i", 0);

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && newcm > -1)
		shipp->cmeasure_count = newcm;

	return ade_set_args(L, "i", shipp->cmeasure_count);
}

ADE_VIRTVAR(CockpitDisplays, l_Ship, "displays", "An array of the cockpit displays on this ship.<br>NOTE: Only the ship of the player has these", "displays", "displays handle or invalid handle on error")
{
	object_h *objh;
	cockpit_displays_h *cdh = NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_CockpitDisplays.GetPtr(&cdh)))
		return ade_set_error(L, "o", l_CockpitDisplays.Set(cockpit_displays_h()));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_CockpitDisplays.Set(cockpit_displays_h()));

	if(ADE_SETTING_VAR)
	{
		LuaError(L, "Attempted to use incomplete feature: Cockpit displays copy");
	}

	return ade_set_args(L, "o", l_CockpitDisplays.Set(cockpit_displays_h(objh->objnum)));
}

ADE_VIRTVAR(CountermeasureClass, l_Ship, "weaponclass", "Weapon class mounted on this ship's countermeasure point", "weaponclass", "Countermeasure hardpoint weapon class, or invalid weaponclass handle if no countermeasure class or ship handle is invalid")
{
	object_h *objh;
	int newcm = -1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Weaponclass.Get(&newcm)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;

	if(!objh->isValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR) {
		shipp->current_cmeasure = newcm;
	}

	if(shipp->current_cmeasure > -1)
		return ade_set_args(L, "o", l_Weaponclass.Set(shipp->current_cmeasure));
	else
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));;
}

ADE_VIRTVAR(HitpointsMax, l_Ship, "number", "Total hitpoints", "number", "Ship maximum hitpoints, or 0 if handle is invalid")
{
	object_h *objh;
	float newhits = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &newhits))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && newhits > -1)
		shipp->ship_max_hull_strength = newhits;

	return ade_set_args(L, "f", shipp->ship_max_hull_strength);
}

ADE_VIRTVAR(ShieldRegenRate, l_Ship, "number", "Maximum percentage/100 of shield energy regenerated per second. For example, 0.02 = 2% recharge per second.", "number", "Ship maximum shield regeneration rate, or 0 if handle is invalid")
{
	object_h *objh;
	float new_shield_regen = -1;
	if (!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &new_shield_regen))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp()->instance];

	if (ADE_SETTING_VAR && new_shield_regen > -1)
		shipp->max_shield_regen_per_second = new_shield_regen;

	return ade_set_args(L, "f", shipp->max_shield_regen_per_second);
}

ADE_VIRTVAR(WeaponRegenRate, l_Ship, "number", "Maximum percentage/100 of weapon energy regenerated per second. For example, 0.02 = 2% recharge per second.", "number", "Ship maximum weapon regeneration rate, or 0 if handle is invalid")
{
	object_h *objh;
	float new_weapon_regen = -1;
	if (!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &new_weapon_regen))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp()->instance];

	if (ADE_SETTING_VAR && new_weapon_regen > -1)
		shipp->max_weapon_regen_per_second = new_weapon_regen;

	return ade_set_args(L, "f", shipp->max_weapon_regen_per_second);
}

ADE_VIRTVAR(WeaponEnergyLeft, l_Ship, "number", "Current weapon energy reserves", "number", "Ship current weapon energy reserve level, or 0 if invalid")
{
	object_h *objh;
	float neweng = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &neweng))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && neweng > -1)
		shipp->weapon_energy = neweng;

	return ade_set_args(L, "f", shipp->weapon_energy);
}

ADE_VIRTVAR(WeaponEnergyMax, l_Ship, "number", "Maximum weapon energy", "number", "Ship maximum weapon energy reserve level, or 0 if invalid")
{
	object_h *objh;
	float neweng = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &neweng))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship_info *sip = &Ship_info[Ships[objh->objp()->instance].ship_info_index];

	if(ADE_SETTING_VAR && neweng > -1)
		sip->max_weapon_reserve = neweng;

	return ade_set_args(L, "f", sip->max_weapon_reserve);
}

ADE_VIRTVAR(AutoaimFOV, l_Ship, "number", "FOV of ship's autoaim, if any", "number", "FOV (in degrees), or 0 if ship uses no autoaim or if handle is invalid")
{
	object_h *objh;
	float fov = -1;
	if(!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fov))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && fov >= 0.0f) {
		if (fov > 180.0)
			fov = 180.0;

		shipp->autoaim_fov = fov * PI / 180.0f;
	}

	return ade_set_args(L, "f", shipp->autoaim_fov * 180.0f / PI);
}

ADE_VIRTVAR(PrimaryTriggerDown, l_Ship, "boolean", "Determines if primary trigger is pressed or not", "boolean", "True if pressed, false if not, nil if ship handle is invalid")
{
	object_h *objh;
	bool trig = false;
	if(!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &trig))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR)
	{
		if(trig)
			shipp->flags.set(Ship::Ship_Flags::Trigger_down);
		else
			shipp->flags.remove(Ship::Ship_Flags::Trigger_down);
	}

	if (shipp->flags[Ship::Ship_Flags::Trigger_down])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(PrimaryBanks, l_Ship, "weaponbanktype", "Array of primary weapon banks", "weaponbanktype", "Primary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp()->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->isValid()) {
		ship_weapon *src = &Ships[swh->objh.objp()->instance].weapons;

		dst->current_primary_bank = src->current_primary_bank;
		dst->num_primary_banks = src->num_primary_banks;

		memcpy(dst->next_primary_fire_stamp, src->next_primary_fire_stamp, sizeof(dst->next_primary_fire_stamp));
		memcpy(dst->primary_animation_done_time, src->primary_animation_done_time, sizeof(dst->primary_animation_done_time));
		memcpy(dst->primary_animation_position, src->primary_animation_position, sizeof(dst->primary_animation_position));
		memcpy(dst->primary_bank_ammo, src->primary_bank_ammo, sizeof(dst->primary_bank_ammo));
		memcpy(dst->primary_bank_capacity, src->primary_bank_capacity, sizeof(dst->primary_bank_capacity));
		memcpy(dst->primary_bank_rearm_time, src->primary_bank_rearm_time, sizeof(dst->primary_bank_rearm_time));
		memcpy(dst->primary_bank_start_ammo, src->primary_bank_start_ammo, sizeof(dst->primary_bank_start_ammo));
		memcpy(dst->primary_bank_weapons, src->primary_bank_weapons, sizeof(dst->primary_bank_weapons));
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp(), dst, SWH_PRIMARY)));
}

ADE_VIRTVAR(SecondaryBanks, l_Ship, "weaponbanktype", "Array of secondary weapon banks", "weaponbanktype", "Secondary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp()->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->isValid()) {
		ship_weapon *src = &Ships[swh->objh.objp()->instance].weapons;

		dst->current_secondary_bank = src->current_secondary_bank;
		dst->num_secondary_banks = src->num_secondary_banks;

		memcpy(dst->next_secondary_fire_stamp, src->next_secondary_fire_stamp, sizeof(dst->next_secondary_fire_stamp));
		memcpy(dst->secondary_animation_done_time, src->secondary_animation_done_time, sizeof(dst->secondary_animation_done_time));
		memcpy(dst->secondary_animation_position, src->secondary_animation_position, sizeof(dst->secondary_animation_position));
		memcpy(dst->secondary_bank_ammo, src->secondary_bank_ammo, sizeof(dst->secondary_bank_ammo));
		memcpy(dst->secondary_bank_capacity, src->secondary_bank_capacity, sizeof(dst->secondary_bank_capacity));
		memcpy(dst->secondary_bank_rearm_time, src->secondary_bank_rearm_time, sizeof(dst->secondary_bank_rearm_time));
		memcpy(dst->secondary_bank_start_ammo, src->secondary_bank_start_ammo, sizeof(dst->secondary_bank_start_ammo));
		memcpy(dst->secondary_bank_weapons, src->secondary_bank_weapons, sizeof(dst->secondary_bank_weapons));
		memcpy(dst->secondary_next_slot, src->secondary_next_slot, sizeof(dst->secondary_next_slot));
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp(), dst, SWH_SECONDARY)));
}

ADE_VIRTVAR(TertiaryBanks, l_Ship, "weaponbanktype", "Array of tertiary weapon banks", "weaponbanktype", "Tertiary weapon banks, or invalid weaponbanktype handle if ship handle is invalid")
{
	object_h *objh;
	ship_banktype_h *swh = nullptr;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &Ships[objh->objp()->instance].weapons;

	if(ADE_SETTING_VAR && swh && swh->isValid()) {
		ship_weapon *src = &Ships[swh->objh.objp()->instance].weapons;

		dst->current_tertiary_bank = src->current_tertiary_bank;
		dst->num_tertiary_banks = src->num_tertiary_banks;

		dst->next_tertiary_fire_stamp = src->next_tertiary_fire_stamp;
		dst->tertiary_bank_ammo = src->tertiary_bank_ammo;
		dst->tertiary_bank_capacity = src->tertiary_bank_capacity;
		dst->tertiary_bank_rearm_time = src->tertiary_bank_rearm_time;
		dst->tertiary_bank_start_ammo = src->tertiary_bank_start_ammo;
	}

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp(), dst, SWH_TERTIARY)));
}

ADE_VIRTVAR(Target, l_Ship, "object", "Target of ship. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Target object, or invalid object handle if no target or ship handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	//WMC - Maybe use two argument return capabilities of Lua to set/return subsystem?
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	ai_info *aip = NULL;
	if(Ships[objh->objp()->instance].ai_index > -1)
		aip = &Ai_info[Ships[objh->objp()->instance].ai_index];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR && !(newh && aip->target_signature == newh->sig)) {
		// we have a different target, or are clearng the target
		if(newh && newh->isValid())	{
			aip->target_objnum = newh->objnum;
			aip->target_signature = newh->sig;
			aip->target_time = 0.0f;
			set_targeted_subsys(aip, nullptr, -1);

			if (aip == Player_ai)
				hud_shield_hit_reset(newh->objp());
		} else if (lua_isnil(L, 2)) {
			aip->target_objnum = -1;
			aip->target_signature = -1;
			aip->target_time = 0.0f;
			set_targeted_subsys(aip, nullptr, -1);
		}
	}

	return ade_set_object_with_breed(L, aip->target_objnum);
}

ADE_VIRTVAR(TargetSubsystem, l_Ship, "subsystem", "Target subsystem of ship.", "subsystem", "Target subsystem, or invalid subsystem handle if no target or ship handle is invalid")
{
	object_h *oh;
	ship_subsys_h *newh = nullptr;
	//WMC - Maybe use two argument return capabilities of Lua to set/return subsystem?
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&oh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!oh->isValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	ai_info *aip = NULL;
	if(Ships[oh->objp()->instance].ai_index > -1)
		aip = &Ai_info[Ships[oh->objp()->instance].ai_index];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isValid())
		{
			if (aip == Player_ai) {
				if (aip->target_signature != newh->objh.sig)
					hud_shield_hit_reset(newh->objh.objp());

				Ships[Objects[newh->ss->parent_objnum].instance].last_targeted_subobject[Player_num] = newh->ss;
			}

			aip->target_objnum = newh->objh.objnum;
			aip->target_signature = newh->objh.sig;
			aip->target_time = 0.0f;
			set_targeted_subsys(aip, newh->ss, aip->target_objnum);
		}
		else
		{
			aip->target_objnum = -1;
			aip->target_signature = -1;
			aip->target_time = 0.0f;

			set_targeted_subsys(aip, NULL, -1);
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(aip->target_objnum >= 0 ? &Objects[aip->target_objnum] : nullptr, aip->targeted_subsys)));
}

ADE_VIRTVAR(Team, l_Ship, "team", "Ship's team", "team", "Ship team, or invalid team handle if ship handle is invalid")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&oh), l_Team.Get(&nt)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if(!oh->isValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	ship *shipp = &Ships[oh->objp()->instance];

	if(ADE_SETTING_VAR && nt > -1) {
		shipp->team = nt;
	}

	return ade_set_args(L, "o", l_Team.Set(shipp->team));
}

ADE_VIRTVAR_DEPRECATED(PersonaIndex, l_Ship, "number", "Persona index", "number", "The index of the persona from messages.tbl, 0 if no persona is set", gameversion::version(25, 0), "Deprecated in favor of Persona")
{
	object_h *objh;
	int p_index = -1;
	if(!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &p_index))
		return ade_set_error(L, "i", 0);

	if(!objh->isValid())
		return ade_set_error(L, "i", 0);

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR && p_index > 0)
		shipp->persona_index = p_index - 1;

	return ade_set_args(L, "i", shipp->persona_index + 1);
}

ADE_VIRTVAR(Persona, l_Ship, "persona", "The persona of the ship, if any", "persona", "Persona handle or invalid handle on error")
{
	object_h* objh;
	int idx = -1;
	if (!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Persona.Get(&idx)))
		return ade_set_error(L, "o", l_Persona.Set(-1));

	if (!objh->isValid())
		return ade_set_error(L, "o", l_Persona.Set(-1));

	ship* shipp = &Ships[objh->objp()->instance];

	if (ADE_SETTING_VAR && idx > -1) {
		shipp->persona_index = idx;
	}

	if (!SCP_vector_inbounds(Personas, shipp->persona_index))
		return ade_set_args(L, "o", l_Persona.Set(-1));
	else
		return ade_set_args(L, "o", l_Persona.Set(shipp->persona_index));
}

ADE_VIRTVAR(Textures, l_Ship, "modelinstancetextures", "Gets ship textures", "modelinstancetextures", "Ship textures, or invalid shiptextures handle if ship handle is invalid")
{
	object_h *sh = nullptr;
	object_h *dh;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&dh), l_Ship.GetPtr(&sh)))
		return ade_set_error(L, "o", l_ModelInstanceTextures.Set(modelinstance_h()));

	if(!dh->isValid())
		return ade_set_error(L, "o", l_ModelInstanceTextures.Set(modelinstance_h()));

	polymodel_instance *dest = model_get_instance(Ships[dh->objp()->instance].model_instance_num);

	if(ADE_SETTING_VAR && sh && sh->isValid()) {
		dest->texture_replace = model_get_instance(Ships[sh->objp()->instance].model_instance_num)->texture_replace;
		
	}

	return ade_set_args(L, "o", l_ModelInstanceTextures.Set(modelinstance_h(dest)));
}

ADE_VIRTVAR(FlagAffectedByGravity, l_Ship, "boolean", "Checks for the \"affected-by-gravity\" flag", "boolean", "True if flag is set, false if flag is not set and nil on error")
{
	object_h *objh=NULL;
	bool set = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &set))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Affected_by_gravity, set);
	}

	if (shipp->flags[Ship::Ship_Flags::Affected_by_gravity])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Disabled, l_Ship, "boolean", "The disabled state of this ship", "boolean", "true if ship is disabled, false otherwise")
{
	object_h *objh=NULL;
	bool set = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &set))
		return ADE_RETURN_FALSE;

	if (!objh->isValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Disabled, set);
		if(set)
		{
			mission_log_add_entry(LOG_SHIP_DISABLED, shipp->ship_name, NULL );
		}
		else
		{
			ship_reset_disabled_physics( &Objects[shipp->objnum], shipp->ship_info_index );
		}
	}

	if (shipp->flags[Ship::Ship_Flags::Disabled])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Stealthed, l_Ship, "boolean", "Stealth status of this ship", "boolean", "true if stealthed, false otherwise or on error")
{
	object_h *objh=NULL;
	bool stealthed = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &stealthed))
		return ADE_RETURN_FALSE;

	if (!objh->isValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Stealth, stealthed);
	}

	if (shipp->flags[Ship::Ship_Flags::Stealth])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(HiddenFromSensors, l_Ship, "boolean", "Hidden from sensors status of this ship", "boolean", "true if invisible to hidden from sensors, false otherwise or on error")
{
	object_h *objh=NULL;
	bool hidden = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &hidden))
		return ADE_RETURN_FALSE;

	if (!objh->isValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR)
	{
		shipp->flags.set(Ship::Ship_Flags::Hidden_from_sensors, hidden);
	}

	if (shipp->flags[Ship::Ship_Flags::Hidden_from_sensors])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(Gliding, l_Ship, "boolean", "Specifies whether this ship is currently gliding or not.", "boolean", "true if gliding, false otherwise or in case of error")
{
	object_h *objh=NULL;
	bool gliding = false;

	if (!ade_get_args(L, "o|b", l_Ship.GetPtr(&objh), &gliding))
		return ADE_RETURN_FALSE;

	if (!objh->isValid())
		return ADE_RETURN_FALSE;

	ship *shipp = &Ships[objh->objp()->instance];

	if(ADE_SETTING_VAR)
	{
		if (Ship_info[shipp->ship_info_index].can_glide)
		{
			object_set_gliding(&Objects[shipp->objnum], gliding, true);
		}
	}

	if (objh->objp()->phys_info.flags & PF_GLIDING || objh->objp()->phys_info.flags & PF_FORCE_GLIDE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(EtsEngineIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Engine index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->isValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Engine Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp()->instance].engine_recharge_index);
}

ADE_VIRTVAR(EtsShieldIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Shield index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->isValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Shield Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp()->instance].shield_recharge_index);
}

ADE_VIRTVAR(EtsWeaponIndex, l_Ship, "number", "(SET not implemented, see EtsSetIndexes)", "number", "Ships ETS Weapon index value, 0 to MAX_ENERGY_INDEX")
{
	object_h *objh=NULL;
	int ets_idx = 0;

	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &ets_idx))
		return ade_set_error(L, "i", 0);

	if (!objh->isValid())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR)
		LuaError(L, "Attempted to set incomplete feature: ETS Weapon Index (see EtsSetIndexes)");

	return ade_set_args(L, "i", Ships[objh->objp()->instance].weapon_recharge_index);
}

ADE_VIRTVAR(Orders, l_Ship, "shiporders", "Array of ship orders", "shiporders", "Ship orders, or invalid handle if ship handle is invalid")
{
	object_h *objh = NULL;
	object_h *newh = NULL;
	if(!ade_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_ShipOrders.GetPtr(&newh)))
		return ade_set_error(L, "o", l_ShipOrders.Set(object_h()));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_ShipOrders.Set(object_h()));;

	if(ADE_SETTING_VAR)
	{
		LuaError(L, "Attempted to use incomplete feature: Ai orders copy. Use giveOrder instead");
	}

	return ade_set_args(L, "o", l_ShipOrders.Set(object_h(objh->objp())));
}

ADE_VIRTVAR(WaypointSpeedCap, l_Ship, "number", "Waypoint speed cap", "number", "The limit on the ship's speed for traversing waypoints.  -1 indicates no speed cap.  0 will be returned if handle is invalid.")
{
	object_h* objh;
	int speed_cap = -1;
	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &speed_cap))
		return ade_set_error(L, "i", 0);

	if (!objh->isValid())
		return ade_set_error(L, "i", 0);

	ship* shipp = &Ships[objh->objp()->instance];
	ai_info* aip = &Ai_info[shipp->ai_index];

	if (ADE_SETTING_VAR)
	{
		// cap speed to range (-1, 32767) to store within int
		CLAMP(speed_cap, -1, 32767);

		aip->waypoint_speed_cap = speed_cap;
	}

	return ade_set_args(L, "i", aip->waypoint_speed_cap);
}

int waypoint_getter(lua_State* L, auto predicate(lua_State*, ai_info*)->int)
{
	object_h* objh;
	int speed_cap = -1;
	if (!ade_get_args(L, "o|i", l_Ship.GetPtr(&objh), &speed_cap))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	ship* shipp = &Ships[objh->objp()->instance];
	ai_info* aip = &Ai_info[shipp->ai_index];

	return predicate(L, aip);
}

ADE_FUNC(getWaypointList, l_Ship, nullptr, "Waypoint list", "waypointlist", "The current waypoint path the ship is following, if any; or nil if the ship handle is invalid.  To set the waypoint list, use ship orders.")
{
	return waypoint_getter(L, [](lua_State* _L, ai_info* aip)
		{
			if (aip->mode == AIM_WAYPOINTS)
				return ade_set_args(_L, "o", l_WaypointList.Set(waypointlist_h(aip->wp_list_index)));
			else
				return ade_set_args(_L, "o", l_WaypointList.Set(waypointlist_h()));
		});
}

ADE_FUNC(getWaypointIndex, l_Ship, nullptr, "Waypoint index", "number", "The current waypoint index the ship is moving towards, if any; or nil if the ship handle is invalid.  To set the waypoint index, use ship orders.")
{
	return waypoint_getter(L, [](lua_State* _L, ai_info* aip)
		{
			if (aip->mode == AIM_WAYPOINTS)
				return ade_set_args(_L, "i", aip->wp_index + 1);
			else
				return ade_set_args(_L, "i", 0);
		});
}

ADE_FUNC(getWaypointsInReverse, l_Ship, nullptr, "Whether the waypoint path is being traveled in reverse", "boolean", "Whether the current waypoint path, if any, is being traveled in reverse; or nil if the ship handle is invalid.  To set this flag, use ship orders.")
{
	return waypoint_getter(L, [](lua_State* _L, ai_info* aip)
		{
			if (aip->mode == AIM_WAYPOINTS)
				return ade_set_args(_L, "b", (aip->wp_flags & WPF_BACKTRACK) != 0);
			else
				return ADE_RETURN_NIL;
		});
}

template <typename LOC>
static int ship_getset_location_helper(lua_State* L, LOC ship::* field, const char* location_type, const char** location_names, size_t location_names_size)
{
	object_h* objh;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	ship* shipp = &Ships[objh->objp()->instance];

	if (ADE_SETTING_VAR && s != nullptr)
	{
		int location = string_lookup(s, location_names, location_names_size);
		if (location < 0)
		{
			Warning(LOCATION, "%s location '%s' not found.", location_type, s);
			return ADE_RETURN_NIL;
		}
		shipp->*field = static_cast<LOC>(location);
	}

	return ade_set_args(L, "s", location_names[static_cast<int>(shipp->*field)]);
}

ADE_VIRTVAR(ArrivalLocation, l_Ship, "string", "The ship's arrival location", "string", "Arrival location, or nil if handle is invalid")
{
	return ship_getset_location_helper(L, &ship::arrival_location, "Arrival", Arrival_location_names, MAX_ARRIVAL_NAMES);
}

ADE_VIRTVAR(DepartureLocation, l_Ship, "string", "The ship's departure location", "string", "Departure location, or nil if handle is invalid")
{
	return ship_getset_location_helper(L, &ship::departure_location, "Departure", Departure_location_names, MAX_DEPARTURE_NAMES);
}

static int ship_getset_anchor_helper(lua_State* L, int ship::* field)
{
	object_h* objh;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	ship* shipp = &Ships[objh->objp()->instance];

	if (ADE_SETTING_VAR && s != nullptr)
	{
		shipp->*field = (stricmp(s, "<no anchor>") == 0) ? -1 : get_parse_name_index(s);
	}

	return ade_set_args(L, "s", (shipp->*field >= 0) ? Parse_names[shipp->*field].c_str() : "<no anchor>");
}

ADE_VIRTVAR(ArrivalAnchor, l_Ship, "string", "The ship's arrival anchor", "string", "Arrival anchor, or nil if handle is invalid")
{
	return ship_getset_anchor_helper(L, &ship::arrival_anchor);
}

ADE_VIRTVAR(DepartureAnchor, l_Ship, "string", "The ship's departure anchor", "string", "Departure anchor, or nil if handle is invalid")
{
	return ship_getset_anchor_helper(L, &ship::departure_anchor);
}

ADE_VIRTVAR(ArrivalPathMask, l_Ship, "number", "The ship's arrival path mask", "number", "Arrival path mask, or nil if handle is invalid")
{
	return ship_getset_helper(L, &ship::arrival_path_mask, true);
}

ADE_VIRTVAR(DeparturePathMask, l_Ship, "number", "The ship's departure path mask", "number", "Departure path mask, or nil if handle is invalid")
{
	return ship_getset_helper(L, &ship::departure_path_mask, true);
}

ADE_VIRTVAR(ArrivalDelay, l_Ship, "number", "The ship's arrival delay", "number", "Arrival delay, or nil if handle is invalid")
{
	return ship_getset_helper(L, &ship::arrival_delay, true);
}

ADE_VIRTVAR(DepartureDelay, l_Ship, "number", "The ship's departure delay", "number", "Departure delay, or nil if handle is invalid")
{
	return ship_getset_helper(L, &ship::departure_delay, true);
}

ADE_VIRTVAR(ArrivalDistance, l_Ship, "number", "The ship's arrival distance", "number", "Arrival distance, or nil if handle is invalid")
{
	return ship_getset_helper(L, &ship::arrival_distance, true);
}

extern int sendMessage_sub(lua_State* L, const void* sender, int messageSource, int messageIdx, float delay, enum_h* ehp);

ADE_FUNC(sendMessage,
	l_Ship,
	"message message, [number delay=0.0, enumeration priority = MESSAGE_PRIORITY_NORMAL]",
	"Sends a message from the given ship with the given priority.<br>"
	"If delay is specified, the message will be delayed by the specified time in seconds.",
	"boolean",
	"true if successful, false otherwise")
{
	int messageIdx = -1;
	float delay = 0.0f;

	object_h* ship_h = nullptr;
	enum_h* ehp = nullptr;

	if (!ade_get_args(L, "oo|fob", l_Ship.GetPtr(&ship_h), l_Message.Get(&messageIdx), &delay, l_Enum.GetPtr(&ehp)))
		return ADE_RETURN_FALSE;

	if (ship_h == nullptr || !ship_h->isValid())
		return ADE_RETURN_FALSE;

	return sendMessage_sub(L, &Ships[ship_h->objp()->instance], MESSAGE_SOURCE_SHIP, messageIdx, delay, ehp);
}

ADE_FUNC(turnTowardsPoint,
	l_Ship,
	"vector target, [boolean respectDifficulty = true, vector turnrateModifier /* 100% of tabled values in all rotation axes by default */, number bank /* native bank-on-heading by default */ ]",
	"turns the ship towards the specified point during this frame",
	nullptr,
	nullptr)
{
	object_h shiph;
	vec3d* target;
	bool diffTurn = true;
	vec3d* modifier = nullptr;
	float bank = 0.0f;

	int argnum = ade_get_args(L, "oo|bof", l_Ship.Get(&shiph), l_Vector.GetPtr(&target), &diffTurn, l_Vector.GetPtr(&modifier), &bank);
	if (argnum == 0) {
		return ADE_RETURN_NIL;
	}

	ai_turn_towards_vector(target, shiph.objp(), nullptr, nullptr, bank, (diffTurn ? 0 : AITTV_FAST) | (argnum >= 5 ? AITTV_FORCE_DELTA_BANK : 0), nullptr, modifier);
	return ADE_RETURN_NIL;
}

ADE_FUNC(turnTowardsOrientation,
	l_Ship,
	"orientation target, [boolean respectDifficulty = true, vector turnrateModifier /* 100% of tabled values in all rotation axes by default */]",
	"turns the ship towards the specified orientation during this frame",
	nullptr,
	nullptr)
{
	object_h ship;
	matrix_h* target;
	bool diffTurn = true;
	vec3d* modifier = nullptr;

	int argnum = ade_get_args(L, "oo|bo", l_Ship.Get(&ship), l_Matrix.GetPtr(&target), &diffTurn, l_Vector.GetPtr(&modifier));
	if (argnum == 0) {
		return ADE_RETURN_NIL;
	}

	matrix* mat = target->GetMatrix();
	vec3d targetVec = mat->vec.fvec + ship.objp()->pos;

	ai_turn_towards_vector(&targetVec, ship.objp(), nullptr, nullptr, 0.0f, (diffTurn ? 0 : AITTV_FAST), &mat->vec.rvec, modifier);
	return ADE_RETURN_NIL;
}

ADE_FUNC(getCenterPosition, l_Ship, nullptr, "Returns the position of the ship's physical center, which may not be the position of the origin of the model", "vector", "World position of the center of the ship, or nil if an error occurred")
{
	object_h *shiph;
	vec3d center_pos, actual_local_center;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ADE_RETURN_NIL;

	// find local center
	ship_class_get_actual_center(&Ship_info[Ships[shiph->objp()->instance].ship_info_index], &actual_local_center);

	// find world position of the center
	vm_vec_unrotate(&center_pos, &actual_local_center, &shiph->objp()->orient);
	vm_vec_add2(&center_pos, &shiph->objp()->pos);

	return ade_set_args(L, "o", l_Vector.Set(center_pos));
}

ADE_FUNC(kill, l_Ship, "[object Killer, vector Hitpos]", "Kills the ship. Set \"Killer\" to a ship (or a weapon fired by that ship) to credit it for the kill in the mission log. Set it to the ship being killed to self-destruct. Set \"Hitpos\" to the world coordinates of the weapon impact.", "boolean", "True if successful, false or nil otherwise")
{
	object_h *victim, *killer = nullptr;
	vec3d *hitpos = nullptr;
	if(!ade_get_args(L, "o|oo", l_Ship.GetPtr(&victim), l_Object.GetPtr(&killer), l_Vector.GetPtr(&hitpos)))
		return ADE_RETURN_NIL;

	if(!victim->isValid())
		return ADE_RETURN_NIL;

	if(killer && !killer->isValid())
		return ADE_RETURN_NIL;

	// use the current hull percentage for damage-after-death purposes
	// (note that this does not actually affect scoring)
	float percent_killed = -get_hull_pct(victim->objp(), true);
	if (percent_killed > 1.0f)
		percent_killed = 1.0f;

	ship_hit_kill(victim->objp(), killer ? killer->objp() : nullptr, hitpos, percent_killed, (killer && victim->sig == killer->sig), true);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(checkVisibility,
	l_Ship,
	"[ship viewer]",
	"checks if a ship can appear on the viewer's radar. If a viewer is not provided it assumes the viewer is the "
	"player.\n",
	"number",
	"Returns 0 - not visible, 1 - partially visible, 2 - fully visible")
{
	object_h* v1 = nullptr;
	object_h* v2 = nullptr;
	if (!ade_get_args(L, "o|o", l_Ship.GetPtr(&v1), l_Ship.GetPtr(&v2)))
		return ade_set_error(L, "i", 0);
	if (!v1 || !v1->isValid())
		return ade_set_error(L, "i", 0);
	if (v2 && !v2->isValid())
		return ade_set_error(L, "i", 0);

	ship* viewer_shipp = nullptr;
	ship* viewed_shipp = nullptr;
	viewed_shipp = &Ships[v1->objp()->instance];
	if (v2)
		viewer_shipp = &Ships[v2->objp()->instance];

	return ade_set_args(L, "i", ship_check_visibility(viewed_shipp, viewer_shipp));
}

ADE_FUNC(addShipEffect, l_Ship, "string name, number durationMillis", "Activates an effect for this ship. Effect names are defined in Post_processing.tbl, and need to be implemented in the main shader. This functions analogous to the ship-effect sexp. NOTE: only one effect can be active at any time, adding new effects will override effects already in progress.\n", "boolean", "Returns true if the effect was successfully added, false otherwise") {
	object_h *shiph;
	const char* effect = nullptr;
	int duration;
	int effect_num;

	if (!ade_get_args(L, "o|si", l_Ship.GetPtr(&shiph), &effect, &duration))
		return ade_set_error(L, "b", false);

	if (!shiph->isValid())
		return ade_set_error(L, "b", false);

	effect_num = get_effect_from_name(effect);
	if (effect_num == -1)
		return ade_set_error(L, "b", false);

	ship* shipp = &Ships[shiph->objp()->instance];

	shipp->shader_effect_num = effect_num;
	shipp->shader_effect_duration = duration;
	shipp->shader_effect_timestamp = _timestamp(duration);

	return ade_set_args(L, "b", true);
}

ADE_FUNC(hasShipExploded, l_Ship, NULL, "Checks if the ship explosion event has already happened", "number", "Returns 1 if first explosion timestamp is passed, 2 if second is passed, 0 otherwise")
{
	object_h *shiph;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ade_set_error(L, "i", 0);

	if(!shiph->isValid())
		return ade_set_error(L, "i", 0);

	ship *shipp = &Ships[shiph->objp()->instance];

	if (shipp->flags[Ship::Ship_Flags::Dying]) {
		if (shipp->final_death_time == 0) {
			return ade_set_args(L, "i", 2);
		}
		if (shipp->pre_death_explosion_happened == 1) {
			return ade_set_args(L, "i", 1);
		}
		return ade_set_args(L, "i", 3);
	}

	return ade_set_args(L, "i", 0);
}

ADE_FUNC(isArrivingWarp, l_Ship, nullptr, "Checks if the ship is arriving via warp.  This includes both stage 1 (when the portal is opening) and stage 2 (when the ship is moving through the portal).", "boolean", "True if the ship is warping in, false otherwise")
{
	object_h *shiph;
	if (!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ade_set_error(L, "b", false);

	if (!shiph->isValid())
		return ade_set_error(L, "b", false);

	ship *shipp = &Ships[shiph->objp()->instance];

	return ade_set_args(L, "b", shipp->is_arriving(ship::warpstage::BOTH, false));
}

ADE_FUNC(isDepartingWarp, l_Ship, nullptr, "Checks if the ship is departing via warp", "boolean", "True if the Depart_warp flag is set, false otherwise")
{
	object_h *shiph;
	if (!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ade_set_error(L, "b", false);

	if (!shiph->isValid())
		return ade_set_error(L, "b", false);

	ship *shipp = &Ships[shiph->objp()->instance];

	return ade_set_args(L, "b", shipp->flags[Ship::Ship_Flags::Depart_warp]);
}

ADE_FUNC(isDepartingDockbay, l_Ship, nullptr, "Checks if the ship is departing via warp", "boolean", "True if the Depart_dockbay flag is set, false otherwise")
{
	object_h *shiph;
	if (!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ade_set_error(L, "b", false);

	if (!shiph->isValid())
		return ade_set_error(L, "b", false);

	ship *shipp = &Ships[shiph->objp()->instance];

	return ade_set_args(L, "b", shipp->flags[Ship::Ship_Flags::Depart_dockbay]);
}

ADE_FUNC(isDying, l_Ship, nullptr, "Checks if the ship is dying (doing its death roll or exploding)", "boolean", "True if the Dying flag is set, false otherwise")
{
	object_h *shiph;
	if (!ade_get_args(L, "o", l_Ship.GetPtr(&shiph)))
		return ade_set_error(L, "b", false);

	if (!shiph->isValid())
		return ade_set_error(L, "b", false);

	ship *shipp = &Ships[shiph->objp()->instance];

	return ade_set_args(L, "b", shipp->flags[Ship::Ship_Flags::Dying]);
}

ADE_FUNC(fireCountermeasure, l_Ship, NULL, "Launches a countermeasure from the ship", "boolean", "Whether countermeasure was launched or not")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "b", false);

	if(!objh->isValid())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", ship_launch_countermeasure(objh->objp()) != 0);
}

ADE_FUNC(firePrimary, l_Ship, NULL, "Fires ship primary bank(s)", "number", "Number of primary banks fired")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->isValid())
		return ade_set_error(L, "i", 0);

	int i = 0;
	i += ship_fire_primary(objh->objp());

	return ade_set_args(L, "i", i);
}

ADE_FUNC(fireSecondary, l_Ship, NULL, "Fires ship secondary bank(s)", "number", "Number of secondary banks fired")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "i", 0);

	if(!objh->isValid())
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", ship_fire_secondary(objh->objp(), 0));
}

ADE_FUNC_DEPRECATED(getAnimationDoneTime, l_Ship, "number Type, number Subtype", "Gets time that animation will be done", "number", "Time (seconds), or 0 if ship handle is invalid",
	gameversion::version(22, 0, 0, 0),
	"To account for the new animation tables, please use getSubmodelAnimationTime()")
{
	object_h *objh;
	const char* s = nullptr;
	int subtype=-1;
	if(!ade_get_args(L, "o|si", l_Ship.GetPtr(&objh), &s, &subtype))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	auto type = animation::anim_match_type(s);
	if(type == animation::ModelAnimationTriggerType::None)
		return ADE_RETURN_FALSE;

	int time_ms = Ship_info[Ships[objh->objp()->instance].ship_info_index].animations.getAll(model_get_instance(Ships[objh->objp()->instance].model_instance_num), type, subtype).getTime();
	float time_s = (float)time_ms / 1000.0f;

	return ade_set_args(L, "f", time_s);
}

ADE_FUNC(clearOrders, l_Ship, NULL, "Clears a ship's orders list", "boolean", "True if successful, otherwise false or nil")
{
	object_h *objh = NULL;
	if(!ade_get_args(L, "o", l_Object.GetPtr(&objh)))
		return ADE_RETURN_NIL;
	if(!objh->isValid())
		return ade_set_error(L, "b", false);

	//The actual clearing of the goals
	ai_clear_ship_goals( &Ai_info[Ships[objh->objp()->instance].ai_index]);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(giveOrder, l_Ship, "enumeration Order, [object Target=nil, subsystem TargetSubsystem=nil, number Priority=1.0, shipclass TargetShipclass=nil]", "Uses the goal code to execute orders", "boolean", "True if order was given, otherwise false or nil")
{
	object_h *objh = NULL;
	enum_h *eh = NULL;
	float priority = 1.0f;
	int sclass = -1;
	object_h *tgh = NULL;
	ship_subsys_h *tgsh = NULL;
	if(!ade_get_args(L, "oo|oofo", l_Object.GetPtr(&objh), l_Enum.GetPtr(&eh), l_Object.GetPtr(&tgh), l_Subsystem.GetPtr(&tgsh), &priority, l_Shipclass.Get(&sclass)))
		return ADE_RETURN_NIL;

	if(!objh->isValid() || !eh->isValid())
		return ade_set_error(L, "b", false);

	//wtf...
	if(priority < 0.0f)
		return ade_set_error(L, "b", false);

	if(priority > 2.0f)
		priority = 2.0f;

	bool tgh_valid = tgh && tgh->isValid();
	bool tgsh_valid = tgsh && tgsh->isValid();
	ai_goal_mode ai_mode = AI_GOAL_NONE;
	int ai_submode = -1234567;
	const char *ai_shipname = NULL;
	int int_data = 0;
	float float_data = 0.0f;
	switch(eh->index)
	{
		case LE_ORDER_ATTACK:
		{
			if(tgsh_valid)
			{
				ai_mode = AI_GOAL_DESTROY_SUBSYSTEM;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
				ai_submode = ship_find_subsys( &Ships[tgsh->objh.objp()->instance], tgsh->ss->system_info->subobj_name );
			}
			else if(tgh_valid && tgh->objp()->type == OBJ_WEAPON)
			{
				ai_mode = AI_GOAL_CHASE_WEAPON;
				ai_submode = tgh->objp()->instance;
			}
			else if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_CHASE;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
				ai_submode = SM_ATTACK;
			}
			break;
		}
		case LE_ORDER_DOCK:
		{
			ai_shipname = Ships[tgh->objp()->instance].ship_name;
			ai_mode = AI_GOAL_DOCK;
			ai_submode = AIS_DOCK_0;
			break;
		}
		case LE_ORDER_WAYPOINTS:
		case LE_ORDER_WAYPOINTS_ONCE:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_WAYPOINT)
			{
				ai_mode = eh->index == LE_ORDER_WAYPOINTS_ONCE ? AI_GOAL_WAYPOINTS_ONCE : AI_GOAL_WAYPOINTS;
				int wp_list_index, wp_index;
				calc_waypoint_indexes(tgh->objp()->instance, wp_list_index, wp_index);
				if (wp_list_index >= 0 && wp_index >= 0)
				{
					ai_shipname = Waypoint_lists[wp_list_index].get_name();
					int_data = wp_index;
				}
			}
			break;
		}
		case LE_ORDER_DEPART:
		{
			ai_mode = AI_GOAL_WARP;
			ai_submode = -1;
			break;
		}
		case LE_ORDER_FORM_ON_WING:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_FORM_ON_WING;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_UNDOCK:
		{
			ai_mode = AI_GOAL_UNDOCK;
			ai_submode = AIS_UNDOCK_0;

			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_GUARD:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_GUARD;
				ai_submode = AIS_GUARD_PATROL;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_DISABLE:
		case LE_ORDER_DISABLE_TACTICAL:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = (eh->index == LE_ORDER_DISABLE) ? AI_GOAL_DISABLE_SHIP : AI_GOAL_DISABLE_SHIP_TACTICAL;
				ai_submode = -SUBSYSTEM_ENGINE;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_DISARM:
		case LE_ORDER_DISARM_TACTICAL:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = (eh->index == LE_ORDER_DISARM) ? AI_GOAL_DISARM_SHIP : AI_GOAL_DISARM_SHIP_TACTICAL;
				ai_submode = -SUBSYSTEM_TURRET;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_ATTACK_ANY:
		{
			ai_mode = AI_GOAL_CHASE_ANY;
			ai_submode = SM_ATTACK;
			break;
		}
		case LE_ORDER_IGNORE:
		case LE_ORDER_IGNORE_NEW:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = (eh->index == LE_ORDER_IGNORE) ? AI_GOAL_IGNORE : AI_GOAL_IGNORE_NEW;
				ai_submode = 0;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_EVADE:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_EVADE_SHIP;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_STAY_NEAR:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_STAY_NEAR_SHIP;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
				ai_submode = -1;
			}
			break;
		}
		case LE_ORDER_KEEP_SAFE_DISTANCE:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_KEEP_SAFE_DISTANCE;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
				ai_submode = -1;
			}
			break;
		}
		case LE_ORDER_REARM:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_REARM_REPAIR;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_STAY_STILL:
		{
			ai_mode = AI_GOAL_STAY_STILL;
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
			}
			break;
		}
		case LE_ORDER_PLAY_DEAD:
		{
			ai_mode = AI_GOAL_PLAY_DEAD;
			break;
		}
		case LE_ORDER_PLAY_DEAD_PERSISTENT:
		{
			ai_mode = AI_GOAL_PLAY_DEAD_PERSISTENT;
			break;
		}
		case LE_ORDER_FLY_TO:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ai_mode = AI_GOAL_FLY_TO_SHIP;
				ai_shipname = Ships[tgh->objp()->instance].ship_name;
				ai_submode = 0;
			}
			break;
		}
		case LE_ORDER_ATTACK_WING:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ship *shipp = &Ships[tgh->objp()->instance];
				if (shipp->wingnum != -1)
				{
					ai_mode = AI_GOAL_CHASE_WING;
					ai_shipname = Wings[shipp->wingnum].name;
					ai_submode = SM_ATTACK;
				}
			}
			break;
		}
		case LE_ORDER_GUARD_WING:
		{
			if(tgh_valid && tgh->objp()->type == OBJ_SHIP)
			{
				ship *shipp = &Ships[tgh->objp()->instance];
				if (shipp->wingnum != -1)
				{
					ai_mode = AI_GOAL_GUARD_WING;
					ai_shipname = Wings[shipp->wingnum].name;
					ai_submode = AIS_GUARD_STATIC;
				}
			}
			break;
		}
		case LE_ORDER_ATTACK_SHIP_CLASS:
		{
			if(sclass >= 0)
			{
				ai_mode = AI_GOAL_CHASE_SHIP_CLASS;
				ai_shipname = Ship_info[sclass].name;
				ai_submode = SM_ATTACK;
			}
			break;
		}
		default:
			return ade_set_error(L, "b", false);
	}

	//Nothing got set!
	if(ai_mode == AI_GOAL_NONE)
		return ade_set_error(L, "b", false);

	//Fire off the goal
	ai_add_ship_goal_scripting(ai_mode, ai_submode, (int)(priority*100.0f), ai_shipname, &Ai_info[Ships[objh->objp()->instance].ai_index], int_data, float_data);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(doManeuver,
	l_Ship,
	"number Duration, number Heading, number Pitch, number Bank, boolean ApplyAllRotation, number Vertical, number "
	"Sideways, number Forward, boolean ApplyAllMovement, number ManeuverBitfield",
	"Sets ship maneuver over the defined time period",
	"boolean",
	"True if maneuver order was given, otherwise false or nil")
{
	object_h *objh;
	float heading, pitch, bank, up, sideways, forward;
	bool apply_all_rotate = false, apply_all_move = false;
	int duration, maneuver_flags = 0;
	if(!ade_get_args(L, "oifffbfffb|i", l_Ship.GetPtr(&objh), &duration, &heading, &pitch, &bank, &apply_all_rotate, &up, &sideways, &forward, &apply_all_move, &maneuver_flags))
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];
	ai_info *aip = &Ai_info[shipp->ai_index];
	control_info *cip = &aip->ai_override_ci;

	if (!(maneuver_flags & CIF_DONT_OVERRIDE_OLD_MANEUVERS)) {
		aip->ai_override_flags.reset();
	}
	
	bool applied_rot = false;
	bool applied_lat = false;
	if (apply_all_rotate) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Full_rot);
		cip->heading = heading;
		cip->pitch = pitch;
		cip->bank = bank;
		applied_rot = true;
	} else {
		if (heading != 0) {
			cip->heading = heading;
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Heading);
			applied_rot = true;
		}
		if (pitch != 0) {
			cip->pitch = pitch;
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Pitch);
			applied_rot = true;
		}
		if (bank != 0) {
			cip->bank = bank;
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Roll);
			applied_rot = true;
		}
	}
	if (apply_all_move) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Full_lat);
		cip->vertical = up;
		cip->sideways = sideways;
		cip->forward = forward;
		applied_lat = true;
	} else {
		if (up != 0) {
			cip->vertical = up;
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Up);
			applied_lat = true;
		}
		if (sideways != 0) {
			cip->sideways = sideways;
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Sideways);
			applied_lat = true;
		}
		if (forward != 0) {
			cip->forward = forward;
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Forward);
			applied_lat = true;
		}
	}

	// handle infinite timestamps
	if (duration >= 2) {
		if (applied_rot)
			aip->ai_override_rot_timestamp = timestamp(duration);
		if (applied_lat)
			aip->ai_override_lat_timestamp = timestamp(duration);
	}
	else {
		if (applied_rot) {
			aip->ai_override_rot_timestamp = timestamp(10);
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Rotational_never_expire);
		}
		if (applied_lat) {
			aip->ai_override_lat_timestamp = timestamp(10);
			aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Lateral_never_expire);
		}
	}

	if (maneuver_flags & CIF_DONT_BANK_WHEN_TURNING) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Dont_bank_when_turning);
	}
	if (maneuver_flags & CIF_DONT_CLAMP_MAX_VELOCITY) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Dont_clamp_max_velocity);
	}
	if (maneuver_flags & CIF_INSTANTANEOUS_ACCELERATION) {
		aip->ai_override_flags.set(AI::Maneuver_Override_Flags::Instantaneous_acceleration);
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC_DEPRECATED(triggerAnimation, l_Ship, "string Type, [number Subtype, boolean Forwards, boolean Instant]",
		 "Triggers an animation. Type is the string name of the animation type, "
			 "Subtype is the subtype number, such as weapon bank #, Forwards and Instant are boolean, defaulting to true & false respectively."
			 "<br><strong>IMPORTANT: Function is in testing and should not be used with official mod releases</strong>",
		 "boolean",
		 "True if successful, false or nil otherwise",
	gameversion::version(22, 0, 0, 0),
	"To account for the new animation tables, please use triggerSubmodelAnimation()")
{
	object_h *objh;
	const char* s = nullptr;
	bool b = true;
	bool instant = false;
	int subtype=-1;
	if(!ade_get_args(L, "o|sibb", l_Ship.GetPtr(&objh), &s, &subtype, &b, &instant))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	auto type = animation::anim_match_type(s);
	if(type == animation::ModelAnimationTriggerType::None)
		return ADE_RETURN_FALSE;

	Ship_info[Ships[objh->objp()->instance].ship_info_index].animations.getAll(model_get_instance(Ships[objh->objp()->instance].model_instance_num), type, subtype).start(b ? animation::ModelAnimationDirection::FWD : animation::ModelAnimationDirection::RWD, instant, instant);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(triggerSubmodelAnimation, l_Ship, "string type, string triggeredBy, [boolean forwards = true, boolean resetOnStart = false, boolean completeInstant = false, boolean pause = false]",
	"Triggers an animation. If used often with the same type / triggeredBy, consider using getSubmodelAnimation for performance reasons. "
	"Type is the string name of the animation type, triggeredBy is a closer specification which animation should trigger. See *-anim.tbm specifications. "
	"Forwards controls the direction of the animation. ResetOnStart will cause the animation to play from its initial state, as opposed to its current state. CompleteInstant will immediately complete the animation. Pause will instead stop the animation at the current state.",
	"boolean",
	"True if successful, false or nil otherwise")
{
	object_h* objh;
	const char* type = nullptr;
	const char* trigger = nullptr;
	bool forwards = true;
	bool forced = false;
	bool instant = false;
	bool pause = false;

	if (!ade_get_args(L, "oss|bbbb", l_Ship.GetPtr(&objh), &type, &trigger, &forwards, &forced, &instant, &pause))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ADE_RETURN_FALSE;

	ship* shipp = &Ships[objh->objp()->instance];

	return Ship_info[shipp->ship_info_index].animations.parseScripted(model_get_instance(shipp->model_instance_num), animtype, trigger).start(forwards ? animation::ModelAnimationDirection::FWD : animation::ModelAnimationDirection::RWD, forced || instant, instant, pause) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
}

ADE_FUNC(getSubmodelAnimation, l_Ship, "string type, string triggeredBy",
	"Gets an animation handle. Type is the string name of the animation type, triggeredBy is a closer specification which animation should trigger. See *-anim.tbm specifications. ",
	"animation_handle",
	"The animation handle for the specified animation, nil if invalid arguments.")
{
	object_h* objh;
	const char* type = nullptr;
	const char* trigger = nullptr;

	if (!ade_get_args(L, "oss", l_Ship.GetPtr(&objh), &type, &trigger))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ade_set_args(L, "o", l_AnimationHandle.Set(animation::ModelAnimationSet::AnimationList{}));

	ship* shipp = &Ships[objh->objp()->instance];

	return ade_set_args(L, "o", l_AnimationHandle.Set(Ship_info[shipp->ship_info_index].animations.parseScripted(model_get_instance(shipp->model_instance_num), animtype, trigger)));
}

ADE_FUNC(stopLoopingSubmodelAnimation, l_Ship, "string type, string triggeredBy",
	"Stops a currently looping animation after it has finished its current loop. If used often with the same type / triggeredBy, consider using getSubmodelAnimation for performance reasons. Type is the string name of the animation type, "
	"triggeredBy is a closer specification which animation was triggered. See *-anim.tbm specifications. ",
	"boolean",
	"True if successful, false or nil otherwise")
{
	object_h* objh;
	const char* type = nullptr;
	const char* trigger = nullptr;

	if (!ade_get_args(L, "oss", l_Ship.GetPtr(&objh), &type, &trigger))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ADE_RETURN_FALSE;

	ship* shipp = &Ships[objh->objp()->instance];

	Ship_info[shipp->ship_info_index].animations.parseScripted(model_get_instance(shipp->model_instance_num), animtype, trigger).setFlag(animation::Animation_Instance_Flags::Stop_after_next_loop);
	return ADE_RETURN_TRUE;
}

ADE_FUNC(setAnimationSpeed, l_Ship, "string type, string triggeredBy, [number speedMultiplier = 1.0]",
	"Sets the speed multiplier at which an animation runs. If used often with the same type / triggeredBy, consider using getSubmodelAnimation for performance reasons. Anything other than 1 will not work in multiplayer. Type is the string name of the animation type, "
	"triggeredBy is a closer specification which animation should trigger. See *-anim.tbm specifications.",
	nullptr,
	nullptr)
{
	object_h* objh;
	const char* type = nullptr;
	const char* trigger = nullptr;
	float speed = 1.0f;

	if (!ade_get_args(L, "oss|f", l_Ship.GetPtr(&objh), &type, &trigger, &speed))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ADE_RETURN_NIL;

	ship* shipp = &Ships[objh->objp()->instance];

	Ship_info[shipp->ship_info_index].animations.parseScripted(model_get_instance(shipp->model_instance_num), animtype, trigger).setSpeed(speed);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getSubmodelAnimationTime, l_Ship, "string type, string triggeredBy", "Gets time that animation will be done. If used often with the same type / triggeredBy, consider using getSubmodelAnimation for performance reasons.", "number", "Time (seconds), or 0 if ship handle is invalid")
{
	object_h* objh;
	const char* type = nullptr;
	const char* trigger = nullptr;
	if (!ade_get_args(L, "oss", l_Ship.GetPtr(&objh), &type, &trigger))
		return ade_set_error(L, "f", 0.0f);

	if (!objh->isValid())
		return ade_set_error(L, "f", 0.0f);

	auto animtype = animation::anim_match_type(type);
	if (animtype == animation::ModelAnimationTriggerType::None)
		return ade_set_error(L, "f", 0.0f);

	ship* shipp = &Ships[objh->objp()->instance];

	int time_ms = Ship_info[shipp->ship_info_index].animations.parseScripted(model_get_instance(shipp->model_instance_num), animtype, trigger).getTime();
	float time_s = (float)time_ms / 1000.0f;

	return ade_set_args(L, "f", time_s);
}

ADE_FUNC(updateSubmodelMoveable, l_Ship, "string name, table values",
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
	object_h* objh;
	const char* name = nullptr;
	luacpp::LuaTable values;

	if (!ade_get_args(L, "ost", l_Ship.GetPtr(&objh), &name, &values))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	SCP_vector<std::any> valuesMoveable;

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

	ship* shipp = &Ships[objh->objp()->instance];
	return Ship_info[shipp->ship_info_index].animations.updateMoveable(model_get_instance(shipp->model_instance_num), name, valuesMoveable) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
}

ADE_FUNC(warpIn, l_Ship, NULL, "Warps ship in", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	if (object_is_docked(objh->objp()))
	{
		// Ships that are docked need a designated dock leader to bring the entire group in.  The dock leader is, de facto, the arriving ship;
		// so by scripting a certain ship to warp in, the script author has designated it as the leader.  That being said, if the script
		// calls warpIn() multiple times on the same docked group, only set the flag on the first ship.
		dock_function_info dfi;
		dock_evaluate_all_docked_objects(objh->objp(), &dfi, dock_find_dock_leader_helper);
		if (!dfi.maintained_variables.objp_value)
			Ships[objh->objp()->instance].flags.set(Ship::Ship_Flags::Dock_leader);
	}

	shipfx_warpin_start(objh->objp());

	return ADE_RETURN_TRUE;
}

ADE_FUNC(warpOut, l_Ship, NULL, "Warps ship out", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	shipfx_warpout_start(objh->objp());

	return ADE_RETURN_TRUE;
}

ADE_FUNC(canWarp, l_Ship, nullptr, "Checks whether ship has a working subspace drive, is allowed to use it, and is not disabled or limited by subsystem strength.", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];
	if( ship_can_warp_full_check(shipp) ){
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_FUNC(canBayDepart, l_Ship, nullptr, "Checks whether ship has a bay departure location and if its mother ship is present.", "boolean", "True if successful, or nil if ship handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];
	if( ship_can_bay_depart(shipp) ){
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

// Aardwolf's function for finding if a ship should be drawn as blue on the radar/minimap
ADE_FUNC_DEPRECATED(isWarpingIn, l_Ship, NULL, "Checks if ship is in stage 1 of warping in", "boolean", "True if the ship is in stage 1 of warping in; false if not; nil for an invalid handle",
	gameversion::version(24, 2, 0, 0),
	"This function's name may imply that it tests for the entire warping sequence.  To avoid confusion, it has been deprecated in favor of isWarpingStage1.")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];
	if(shipp->is_arriving(ship::warpstage::STAGE1, false)){
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

// This has functionality identical to isWarpingIn
ADE_FUNC(isWarpingStage1, l_Ship, NULL, "Checks if ship is in stage 1 of warping in, which is the stage when the warp portal is opening but before the ship has gone through.  During this stage, the ship's radar blip is blue, while the ship itself is invisible, does not collide, and has velocity 0.", "boolean", "True if the ship is in stage 1 of warping in; false if not; nil for an invalid handle")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];
	if(shipp->is_arriving(ship::warpstage::STAGE1, false)){
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_FUNC(isWarpingStage2, l_Ship, NULL, "Checks if ship is in stage 2 of warping in, which is the stage when it is traversing the warp portal.  Stage 2 ends as soon as the ship is completely through the portal and does not include portal closing or ship deceleration.", "boolean", "True if the ship is in stage 2 of warping in; false if not; nil for an invalid handle")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	ship *shipp = &Ships[objh->objp()->instance];
	if(shipp->is_arriving(ship::warpstage::STAGE2, false)){
		return ADE_RETURN_TRUE;
	}

	return ADE_RETURN_FALSE;
}

ADE_FUNC(getEMP, l_Ship, NULL, "Returns the current emp effect strength acting on the object", "number", "Current EMP effect strength or NIL if object is invalid")
{
	object_h *objh = NULL;
	object *obj = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ADE_RETURN_NIL;
	}

	if(!objh->isValid())
		return ADE_RETURN_NIL;

	obj = objh->objp();

	ship *shipp = &Ships[obj->instance];

	return ade_set_args(L, "f", shipp->emp_intensity);
}

ADE_FUNC(getTimeUntilExplosion, l_Ship, nullptr, "Returns the time in seconds until the ship explodes (the ship's final_death_time timestamp)", "number", "Time until explosion or -1, if invalid handle or ship isn't exploding")
{
	object_h *objh = nullptr;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "f", -1.0f);
	if(!objh->isValid())
		return ade_set_error(L, "f", -1.0f);

	ship *shipp = &Ships[objh->objp()->instance];

	if (!timestamp_valid(shipp->final_death_time))
		return ade_set_args(L, "f", -1.0f);

	int time_until = timestamp_until(shipp->final_death_time);

	return ade_set_args(L, "f", (i2fl(time_until) / 1000.0f));
}

ADE_FUNC(setTimeUntilExplosion, l_Ship, "number Time", "Sets the time in seconds until the ship explodes (the ship's final_death_time timestamp).  This function will only work if the ship is in its death roll but hasn't exploded yet, which can be checked via isDying() or getTimeUntilExplosion().", "boolean", "True if successful, false if the ship is invalid or not currently exploding")
{
	object_h *objh = nullptr;
	float delta_s;

	if (!ade_get_args(L, "of", l_Ship.GetPtr(&objh), &delta_s))
		return ade_set_error(L, "b", false);
	if (!objh->isValid())
		return ade_set_error(L, "b", false);

	ship *shipp = &Ships[objh->objp()->instance];

	if (!timestamp_valid(shipp->final_death_time))
		return ade_set_args(L, "b", false);

	int delta_ms = fl2i(delta_s * 1000.0f);
	if (delta_ms < 2)
		delta_ms = 2;

	shipp->final_death_time = timestamp(delta_ms);

	return ade_set_args(L, "b", true);
}

ADE_FUNC(getCallsign, l_Ship, NULL, "Gets the callsign of the ship in the current mission", "string", "The callsign or an empty string if the ship doesn't have a callsign or an error occurs")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "s", "");
	}

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp()->instance];

	if (shipp->callsign_index < 0)
		return ade_set_args(L, "s", "");

	char temp_callsign[NAME_LENGTH];

	*temp_callsign = 0;
	strcpy(temp_callsign, mission_parse_lookup_callsign_index(shipp->callsign_index));

	if (*temp_callsign)
		return ade_set_args(L, "s", temp_callsign);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(getAltClassName, l_Ship, NULL, "Gets the alternate class name of the ship", "string", "The alternate class name or an empty string if the ship doesn't have such a thing or an error occurs")
{
	object_h *objh = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh))) {
		return ade_set_error(L, "s", "");
	}

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	ship *shipp = &Ships[objh->objp()->instance];

	if (shipp->alt_type_index < 0)
		return ade_set_args(L, "s", "");

	char temp_altname[NAME_LENGTH];

	strcpy_s(temp_altname, mission_parse_lookup_alt_index(shipp->alt_type_index));

	if (*temp_altname)
		return ade_set_args(L, "s", temp_altname);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(getMaximumSpeed, l_Ship, "[number energy = 0.333]", "Gets the maximum speed of the ship with the given energy on the engines", "number", "The maximum speed or -1 on error")
{
	object_h *objh = NULL;
	float energy = 0.333f;

	if (!ade_get_args(L, "o|f", l_Ship.GetPtr(&objh), &energy)) {
		return ade_set_error(L, "f", -1.0f);
	}

	if(!objh->isValid())
		return ade_set_error(L, "f", -1.0f);

	if (energy < 0.0f || energy > 1.0f)
	{
		LuaError(L, "Invalid energy level %f! Needs to be in [0, 1].", energy);

		return ade_set_args(L, "f", -1.0f);
	}
	else
	{
		return ade_set_args(L, "f", ets_get_max_speed(objh->objp(), energy));
	}
}

ADE_FUNC(EtsSetIndexes, l_Ship, "number EngineIndex, number ShieldIndex, number WeaponIndex",
		 "Sets ships ETS systems to specified values",
		 "boolean",
		 "True if successful, false if target ships ETS was missing, or only has one system")
{
	object_h *objh=NULL;
	int ets_idx[num_retail_ets_gauges] = {0};

	if (!ade_get_args(L, "oiii", l_Ship.GetPtr(&objh), &ets_idx[ENGINES], &ets_idx[SHIELDS], &ets_idx[WEAPONS]))
		return ADE_RETURN_FALSE;

	if (!objh->isValid())
		return ADE_RETURN_FALSE;

	sanity_check_ets_inputs(ets_idx);

	int sindex = objh->objp()->instance;
	if (validate_ship_ets_indxes(sindex, ets_idx)) {
		set_recharge_rates(objh->objp(), ets_idx[SHIELDS], ets_idx[WEAPONS], ets_idx[ENGINES]);
		return ADE_RETURN_TRUE;
	} else {
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(getParsedShip, l_Ship, nullptr, "Returns the parsed ship that was used to create this ship, if any", "parse_object", "The parsed ship, an invalid handle if no parsed ship exists, or nil if the current handle is invalid")
{
	object_h *objh = nullptr;
	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "o", l_ParseObject.Set(parse_object_h(nullptr)));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_ParseObject.Set(parse_object_h(nullptr)));

	auto shipp = &Ships[objh->objp()->instance];
	auto ship_entry = ship_registry_get(shipp->ship_name);
	if (!ship_entry || !ship_entry->has_p_objp())
		return ade_set_args(L, "o", l_ParseObject.Set(parse_object_h(nullptr)));

	return ade_set_args(L, "o", l_ParseObject.Set(parse_object_h(ship_entry->p_objp())));
}

ADE_FUNC(getWing, l_Ship, NULL, "Returns the ship's wing", "wing", "Wing handle, or invalid wing handle if ship is not part of a wing")
{
	object_h *objh = NULL;
	ship *shipp = NULL;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Wing.Set(-1));

	if(!objh->isValid())
		return ade_set_error(L, "o", l_Wing.Set(-1));

	shipp = &Ships[objh->objp()->instance];
	return ade_set_args(L, "o", l_Wing.Set(shipp->wingnum));
}

ADE_FUNC(getDisplayString, l_Ship, nullptr, "Returns the string which should be used when displaying the name of the ship to the player", "string", "The display string or empty if handle is invalid")
{
	object_h *objh = nullptr;
	ship *shipp = nullptr;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "s", "");

	if(!objh->isValid())
		return ade_set_error(L, "s", "");

	shipp = &Ships[objh->objp()->instance];
	return ade_set_args(L, "s", shipp->get_display_name());
}

ADE_FUNC(vanish, l_Ship, nullptr, "Vanishes this ship from the mission. Works in Singleplayer only and will cause the ship exit to not be logged.", "boolean", "True if the deletion was successful, false otherwise.")
{
	object_h* objh = nullptr;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return ade_set_error(L, "b", false);

	if (!objh->isValid())
		return ade_set_error(L, "b", false);

	ship_actually_depart(objh->objp()->instance, SHIP_VANISHED);

	return ade_set_args(L, "b", true);
}

ADE_FUNC(setGlowPointBankActive, l_Ship, "boolean active, [number bank]", "Activates or deactivates one or more of a ship's glow point banks - this function can accept an arbitrary number of bank arguments.  Omit the bank number or specify -1 to activate or deactivate all banks.", nullptr, "Returns nothing")
{
	object_h* objh = nullptr;
	bool active, at_least_one = false, do_all = false;
	int bank_num;

	if (!ade_get_args(L, "ob", l_Ship.GetPtr(&objh), &active))
		return ADE_RETURN_NIL;
	int skip_args = 1;	// not 2 because there will be one more before we read the first number

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto shipp = &Ships[objh->objp()->instance];

	// read as many bank numbers as we have
	while (internal::Ade_get_args_skip = ++skip_args, ade_get_args(L, "|i", &bank_num) > 0)
	{
		if (bank_num < 0)
		{
			do_all = true;
			break;
		}
		at_least_one = true;

		if (static_cast<size_t>(bank_num) < shipp->glow_point_bank_active.size())
			shipp->glow_point_bank_active[bank_num] = active;
	}

	// set all banks
	if (!at_least_one || do_all)
	{
		for (size_t i = 0; i < shipp->glow_point_bank_active.size(); ++i)
			shipp->glow_point_bank_active[i] = active;
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(numDocked, l_Ship, nullptr, "Returns the number of ships this ship is directly docked with", "number", "The number of ships")
{
	object_h *docker_objh = nullptr;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&docker_objh)))
		return ADE_RETURN_NIL;

	if (docker_objh == nullptr || !docker_objh->isValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", dock_count_direct_docked_objects(docker_objh->objp()));
}

ADE_FUNC(isDocked, l_Ship, "[ship... dockee_ships]", "Returns whether this ship is docked to all of the specified dockee ships, or is docked at all if no ships are specified", "boolean", "Returns whether the ship is docked")
{
	bool found_arg = false;
	int skip_args = 1;
	object_h *docker_objh = nullptr, *dockee_objh = nullptr;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&docker_objh)))
		return ADE_RETURN_NIL;

	if (docker_objh == nullptr || !docker_objh->isValid())
		return ADE_RETURN_NIL;

	while (true)
	{
		// read the next ship
		internal::Ade_get_args_skip = skip_args++;
		if (ade_get_args(L, "|o", l_Ship.GetPtr(&dockee_objh)) == 0)
			break;
		found_arg = true;

		// make sure ship exists
		if (dockee_objh == nullptr || !dockee_objh->isValid())
			continue;

		// see if we are docked to it
		if (!dock_check_find_direct_docked_object(docker_objh->objp(), dockee_objh->objp()))
			return ADE_RETURN_FALSE;
	}

	// all ships passed
	if (found_arg)
		return ADE_RETURN_TRUE;

	// if we didn't find any specified ships, then see if we are docked at all
	return object_is_docked(docker_objh->objp()) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
}

ADE_FUNC(setDocked, l_Ship, "ship dockee_ship, [string | number docker_point, string | number dockee_point]", "Immediately docks this ship with another ship.", "boolean", "Returns whether the docking was successful, or nil if an input was invalid")
{
	object_h *docker_objh = nullptr, *dockee_objh = nullptr;
	int docker_point = -1, dockee_point = -1;

	if (!ade_get_args(L, "oo", l_Ship.GetPtr(&docker_objh), l_Ship.GetPtr(&dockee_objh)))
		return ADE_RETURN_NIL;

	if (docker_objh == nullptr || dockee_objh == nullptr || !docker_objh->isValid() || !dockee_objh->isValid())
		return ADE_RETURN_NIL;

	// cannot dock an object to itself
	if (docker_objh->objp()->instance == dockee_objh->objp()->instance)
		return ADE_RETURN_FALSE;

	auto docker_shipp = &Ships[docker_objh->objp()->instance];
	auto dockee_shipp = &Ships[dockee_objh->objp()->instance];

	auto docker_pm = model_get(Ship_info[docker_shipp->ship_info_index].model_num);
	auto dockee_pm = model_get(Ship_info[dockee_shipp->ship_info_index].model_num);

	if (lua_isnumber(L, 3))
	{
		ade_get_args(L, "**i", &docker_point);
		docker_point--;	// Lua --> C/C++
	}
	else if (lua_isstring(L, 3))
	{
		const char *name = nullptr;
		ade_get_args(L, "**s", &name);
		docker_point = model_find_dock_name_index(Ship_info[docker_shipp->ship_info_index].model_num, name);
	}
	else
	{
		docker_point = 0;
	}

	if (lua_isnumber(L, 4))
	{
		ade_get_args(L, "***i", &dockee_point);
		dockee_point--;	// Lua --> C/C++
	}
	else if (lua_isstring(L, 4))
	{
		const char* name = nullptr;
		ade_get_args(L, "***s", &name);
		dockee_point = model_find_dock_name_index(Ship_info[dockee_shipp->ship_info_index].model_num, name);
	}
	else
	{
		dockee_point = 0;
	}

	if (docker_point < 0 || docker_point >= docker_pm->n_docks)
	{
		Warning(LOCATION, "Invalid docker point specified for ship '%s'", docker_shipp->ship_name);
		return ADE_RETURN_NIL;
	}

	if (dockee_point < 0 || dockee_point >= dockee_pm->n_docks)
	{
		Warning(LOCATION, "Invalid dockee point specified for ship '%s'", dockee_shipp->ship_name);
		return ADE_RETURN_NIL;
	}

	//Make sure that the specified dockpoints are all free (if not, do nothing)
	if (dock_find_object_at_dockpoint(docker_objh->objp(), docker_point) != nullptr ||
		dock_find_object_at_dockpoint(dockee_objh->objp(), dockee_point) != nullptr)
	{
		// at least one of the specified dockpoints is occupied
		return ADE_RETURN_FALSE;
	}

	//Set docked
	dock_orient_and_approach(docker_objh->objp(), docker_point, dockee_objh->objp(), dockee_point, DOA_DOCK_STAY);
	ai_do_objects_docked_stuff(docker_objh->objp(), docker_point, dockee_objh->objp(), dockee_point, true);

	return ADE_RETURN_TRUE;
}

static int jettison_helper(lua_State *L, object_h *docker_objh, float jettison_speed, int skip_args)
{
	object_h *dockee_objh = nullptr;
	bool found_arg = false;
	int num_ships_undocked = 0;

	while (true)
	{
		// read the next ship
		internal::Ade_get_args_skip = skip_args++;
		if (ade_get_args(L, "|o", l_Ship.GetPtr(&dockee_objh)) == 0)
			break;
		found_arg = true;

		// make sure ship exists
		if (dockee_objh == nullptr || !dockee_objh->isValid())
			continue;

		// make sure we are docked to it
		if (!dock_check_find_direct_docked_object(docker_objh->objp(), dockee_objh->objp()))
			continue;

		object_jettison_cargo(docker_objh->objp(), dockee_objh->objp(), jettison_speed, true);
		num_ships_undocked++;
	}

	// if we didn't find any specified ships, then we need to jettison all of them
	if (!found_arg)
	{
		// Goober5000 - as with ai_deathroll_start, we can't simply iterate through the dock list while we're
		// undocking things.  So just repeatedly jettison the first object.
		while (object_is_docked(docker_objh->objp()))
		{
			object_jettison_cargo(docker_objh->objp(), dock_get_first_docked_object(docker_objh->objp()), jettison_speed, true);
			num_ships_undocked++;
		}
	}

	return ade_set_args(L, "i", num_ships_undocked);
}

ADE_FUNC(setUndocked, l_Ship, "[ship... dockee_ships /* All docked ships by default */]", "Immediately undocks one or more dockee ships from this ship.", "number", "Returns the number of ships undocked")
{
	object_h *docker_objh = nullptr;

	if (!ade_get_args(L, "o", l_Ship.GetPtr(&docker_objh)))
		return ADE_RETURN_NIL;

	if (docker_objh == nullptr || !docker_objh->isValid())
		return ADE_RETURN_NIL;

	return jettison_helper(L, docker_objh, 0.0f, 1);
}

ADE_FUNC(jettison, l_Ship, "number jettison_speed, [ship... dockee_ships /* All docked ships by default */]", "Jettisons one or more dockee ships from this ship at the specified speed.", "number", "Returns the number of ships jettisoned")
{
	object_h *docker_objh = nullptr;
	float jettison_speed = 0.0f;

	if (!ade_get_args(L, "of", l_Ship.GetPtr(&docker_objh), &jettison_speed))
		return ADE_RETURN_NIL;

	if (docker_objh == nullptr || !docker_objh->isValid())
		return ADE_RETURN_NIL;

	return jettison_helper(L, docker_objh, jettison_speed, 2);
}

ADE_FUNC(AddElectricArc, l_Ship, "vector firstPoint, vector secondPoint, number duration, number width, [number segment_depth, boolean persistent_points]",
	"Creates an electric arc on the ship between two points in the ship's reference frame, for the specified duration in seconds, and the specified width in meters.  Optionally, "
		"specify the segment depth (the number of times the spark is divided) and whether to generate a set of arc points that will persist from frame to frame.",
	"number",
	"The arc index if successful, 0 otherwise")
{
	object_h* objh = nullptr;
	vec3d* v1;
	vec3d* v2;
	float duration = 0.0f;
	float width = 0.0f;
	int segment_depth = 4;
	bool persistent_points = false;

	if (!ade_get_args(L, "oooff|ib", l_Ship.GetPtr(&objh), l_Vector.GetPtr(&v1), l_Vector.GetPtr(&v2), &duration, &width, &segment_depth, &persistent_points))
		return ade_set_error(L, "i", 0);

	if (!objh->isValid())
		return ade_set_error(L, "i", 0);

	auto shipp = &Ships[objh->objp()->instance];

	// spawn the arc in the first unused slot, or in a new slot if there are no unused ones
	auto arc = ship_find_or_create_electrical_arc_slot(shipp, false);
	if (arc)
	{
		arc->timestamp = _timestamp(fl2i(duration * MILLISECONDS_PER_SECOND));

		arc->endpoint_1 = *v1;
		arc->endpoint_2 = *v2;

		//Set the arc colors
		arc->primary_color_1 = Arc_color_damage_p1;
		arc->primary_color_2 = Arc_color_damage_p2;
		arc->secondary_color = Arc_color_damage_s1;

		arc->type = MARC_TYPE_SCRIPTED;

		arc->width = width;
		arc->segment_depth = static_cast<ubyte>(segment_depth);

		// we might want to generate the arc points ahead of time, rather than every frame
		if (persistent_points)
		{
			arc->persistent_arc_points.reset(new SCP_vector<vec3d>());

			// need to add the first point
			arc->persistent_arc_points->push_back(*v1);

			// this should fill in all of the middle, and the last, points
			interp_generate_arc_segment(*arc->persistent_arc_points, v1, v2, static_cast<ubyte>(segment_depth), 1);	// start at depth 1 for the benefit of Lua
		}

		return ade_set_args(L, "i", static_cast<int>(arc - shipp->electrical_arcs.data()) + 1);	// FS2 -> Lua
	}

	return ade_set_args(L, "i", 0);
}

ADE_FUNC(DeleteElectricArc, l_Ship, "number index",
	"Removes the specified electric arc from the ship.",
	nullptr,
	nullptr)
{
	object_h* objh = nullptr;
	int index;

	if (!ade_get_args(L, "oi", l_Ship.GetPtr(&objh), &index))
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto shipp = &Ships[objh->objp()->instance];

	index--;	// Lua -> FS2
	if (SCP_vector_inbounds(shipp->electrical_arcs, index))
	{
		shipp->electrical_arcs[index].timestamp = TIMESTAMP::invalid();
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(ModifyElectricArc, l_Ship, "number index, vector firstPoint, vector secondPoint, [number width, number segment_depth, boolean persistent_points]",
	"Sets the endpoints (in the ship's reference frame), width, and segment depth of the specified electric arc on the ship, plus whether the arc has persistent points.  "
		"If this arc already had a collection of persistent points and it still does after this function is called, the points will be regenerated.",
	nullptr,
	nullptr)
{
	object_h* objh = nullptr;
	int index;
	vec3d* v1;
	vec3d* v2;
	float width = 0.0f;
	int segment_depth = 4;
	bool persistent_points = false;

	int args = ade_get_args(L, "oioo|fib", l_Ship.GetPtr(&objh), &index, l_Vector.GetPtr(&v1), l_Vector.GetPtr(&v2), &width, &segment_depth, &persistent_points);
	if (args < 4)
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto shipp = &Ships[objh->objp()->instance];

	index--;	// Lua -> FS2
	if (SCP_vector_inbounds(shipp->electrical_arcs, index))
	{
		auto &arc = shipp->electrical_arcs[index];
		arc.endpoint_1 = *v1;
		arc.endpoint_2 = *v2;

		if (args >= 5)
			arc.width = width;
		if (args >= 6)
			arc.segment_depth = static_cast<ubyte>(segment_depth);
		if (args >= 7)
		{
			if (persistent_points)
			{
				if (!arc.persistent_arc_points)
					arc.persistent_arc_points.reset(new SCP_vector<vec3d>());
			}
			else
			{
				if (arc.persistent_arc_points)
					arc.persistent_arc_points.reset();
			}
		}

		// persistent points need to be regenerated when the arc is moved; they also need to be generated if we are adding them for the first time
		if (arc.persistent_arc_points)
		{
			arc.persistent_arc_points->clear();

			// need to add the first point
			arc.persistent_arc_points->push_back(*v1);

			// this should fill in all of the middle, and the last, points
			interp_generate_arc_segment(*arc.persistent_arc_points, v1, v2, static_cast<ubyte>(segment_depth), 1);	// start at depth 1 for the benefit of Lua
		}
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(ModifyElectricArcPoints, l_Ship, "number index, table points, [number width]",
	"Sets the collection of persistent points to be used by this arc, as well as optionally the arc's width.  "
		"The table of points should consist of Vectors (e.g. created with ba.createVector()), arrays with three elements each, or tables with 'x'/'X', 'y'/'Y', and 'z'/'Z' pairs.  There must be at least two points.",
	nullptr,
	nullptr)
{
	object_h* objh = nullptr;
	int index;
	luacpp::LuaTable luaPoints;
	float width = 0.0f;

	int args = ade_get_args(L, "oit|f", l_Ship.GetPtr(&objh), &index, &luaPoints, &width);
	if (args < 3)
		return ADE_RETURN_NIL;

	if (!objh->isValid())
		return ADE_RETURN_NIL;

	auto shipp = &Ships[objh->objp()->instance];

	index--;	// Lua -> FS2
	if (SCP_vector_inbounds(shipp->electrical_arcs, index) && luaPoints.isValid())
	{
		SCP_vector<vec3d> fsoPoints;

		// convert Lua points to FSO points
		for (const auto &entry : luaPoints)
		{
			try
			{
				fsoPoints.push_back(luacpp::util::valueToVec3d(entry.second));
			}
			catch (const luacpp::LuaException &e)
			{
				LuaError(L, "%s", e.what());
				return ADE_RETURN_NIL;
			}
		}

		if (fsoPoints.size() < 2)
		{
			LuaError(L, "Points table passed to ship:ModifyElectricArcPoints() has fewer than two points!");
			return ADE_RETURN_NIL;
		}

		auto &arc = shipp->electrical_arcs[index];
		arc.endpoint_1 = fsoPoints.front();
		arc.endpoint_2 = fsoPoints.back();

		// need to create the persistent point storage if it isn't set up yet
		if (!arc.persistent_arc_points)
			arc.persistent_arc_points.reset(new SCP_vector<vec3d>());

		// assign all of our new points to the persistent point storage
		arc.persistent_arc_points->operator=(std::move(fsoPoints));

		if (args >= 4)
			arc.width = width;
	}

	return ADE_RETURN_NIL;
}

}
}
