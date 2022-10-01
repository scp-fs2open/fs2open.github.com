//
//

#include "shipclass.h"
#include "model.h"
#include "cockpit_display.h"
#include "species.h"
#include "shiptype.h"
#include "vecmath.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "missionui/missionweaponchoice.h"
#include "graphics/matrix.h"
#include "missionui/missionscreencommon.h"
#include "scripting/api/objs/weaponclass.h"

namespace scripting {
namespace api {


//**********HANDLE: default primary
ADE_OBJ(l_Default_Primary, int, "default_primary", "weapon index");

ADE_INDEXER(l_Default_Primary,
	"number idx",
	"Array of ship default primaries for each bank. Returns the Weapon Class or "
	"nil if the bank is invalid for the ship class.",
	"weaponclass",
	"The weapon index")
{
	int current;
	int idx = -1;
	if (!ade_get_args(L, "oi", l_Default_Primary.Get(&current), &idx))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	ship_info* sip = &Ship_info[current];

	if (idx < 1 || idx > sip->num_primary_banks) {
		return ADE_RETURN_NIL;
	};
	idx--; // Convert to Lua's 1 based index system

	return ade_set_args(L, "o", l_Weaponclass.Set(sip->primary_bank_weapons[idx]));
}

ADE_FUNC(__len,
	l_Default_Primary,
	nullptr,
	"The number of primary banks with defaults",
	"number",
	"The number of primary banks.")
{
	int current;
	ade_get_args(L, "o", l_Default_Primary.Get(&current));
	ship_info* sip = &Ship_info[current];
	return ade_set_args(L, "i", sip->num_primary_banks);
}

//**********HANDLE: default secondary
ADE_OBJ(l_Default_Secondary, int, "default_secondary", "weapon index");

ADE_INDEXER(l_Default_Secondary,
	"number idx",
	"Array of ship default secondaries for each bank. Returns the Weapon Class or "
	"nil if the bank is invalid for the ship class.",
	"weaponclass",
	"The weapon index")
{
	int current;
	int idx = -1;
	if (!ade_get_args(L, "oi", l_Default_Secondary.Get(&current), &idx))
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	ship_info* sip = &Ship_info[current];

	if (idx < 1 || idx > sip->num_secondary_banks) {
		return ADE_RETURN_NIL;
	};
	idx--; // Convert to Lua's 1 based index system

	return ade_set_args(L, "o", l_Weaponclass.Set(sip->secondary_bank_weapons[idx]));
}

ADE_FUNC(__len,
	l_Default_Secondary,
	nullptr,
	"The number of secondary banks with defaults",
	"number",
	"The number of secondary banks.")
{
	int current;
	ade_get_args(L, "o", l_Default_Secondary.Get(&current));
	ship_info* sip = &Ship_info[current];
	return ade_set_args(L, "i", sip->num_secondary_banks);
}
	
//**********HANDLE: Shipclass
ADE_OBJ(l_Shipclass, int, "shipclass", "Ship class handle");

ADE_FUNC(__tostring, l_Shipclass, NULL, "Ship class name", "string", "Ship class name, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Ship_info[idx].name);
}

ADE_FUNC(__eq, l_Shipclass, "shipclass, shipclass", "Checks if the two classes are equal", "boolean", "true if equal, false otherwise")
{
	int idx1,idx2;
	if(!ade_get_args(L, "oo", l_Shipclass.Get(&idx1), l_Shipclass.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if(idx1 < 0 || idx1 >= ship_info_size())
		return ade_set_error(L, "b", false);

	if(idx2 < 0 || idx2 >= ship_info_size())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}


ADE_VIRTVAR(Name, l_Shipclass, "string", "Ship class name", "string", "Ship class name, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != NULL) {
		auto len = sizeof(Ship_info[idx].name);
		strncpy(Ship_info[idx].name, s, len);
		Ship_info[idx].name[len - 1] = 0;
	}

	return ade_set_args(L, "s", Ship_info[idx].name);
}

ADE_VIRTVAR(ShortName, l_Shipclass, "string", "Ship class short name", "string", "Ship short name, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != NULL) {
		auto len = sizeof(Ship_info[idx].short_name);
		strncpy(Ship_info[idx].short_name, s, len);
		Ship_info[idx].short_name[len - 1] = 0;
	}

	return ade_set_args(L, "s", Ship_info[idx].short_name);
}

ADE_VIRTVAR(TypeString, l_Shipclass, "string", "Ship class type string", "string", "Type string, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(sip->type_str);
		if(s != NULL) {
			sip->type_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->type_str, s);
		} else {
			sip->type_str = NULL;
		}
	}

	if(sip->type_str != NULL)
		return ade_set_args(L, "s", sip->type_str);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(ManeuverabilityString, l_Shipclass, "string", "Ship class maneuverability string", "string", "Maneuverability string, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(sip->maneuverability_str);
		if(s != NULL) {
			sip->maneuverability_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->maneuverability_str, s);
		} else {
			sip->maneuverability_str = NULL;
		}
	}

	if(sip->maneuverability_str != NULL)
		return ade_set_args(L, "s", sip->maneuverability_str);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(ArmorString, l_Shipclass, "string", "Ship class armor string", "string", "Armor string, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(sip->armor_str);
		if(s != NULL) {
			sip->armor_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->armor_str, s);
		} else {
			sip->armor_str = NULL;
		}
	}

	if(sip->armor_str != NULL)
		return ade_set_args(L, "s", sip->armor_str);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(ManufacturerString, l_Shipclass, "string", "Ship class manufacturer", "string", "Manufacturer, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(sip->manufacturer_str);
		if(s != NULL) {
			sip->manufacturer_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->manufacturer_str, s);
		} else {
			sip->manufacturer_str = NULL;
		}
	}

	if(sip->manufacturer_str != NULL)
		return ade_set_args(L, "s", sip->manufacturer_str);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(LengthString, l_Shipclass, "string", "Ship class length", "string", "Length, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Length is not supported");
	}

	if (sip->ship_length != nullptr)
		return ade_set_args(L, "s", sip->ship_length);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(GunMountsString, l_Shipclass, "string", "Ship class gun mounts", "string", "Gun mounts, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Gun mounts is not supported");
	}

	if (sip->gun_mounts != nullptr)
		return ade_set_args(L, "s", sip->gun_mounts);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(MissileBanksString, l_Shipclass, "string", "Ship class missile banks", "string", "Missile banks, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Missile banks is not supported");
	}

	if (sip->missile_banks != nullptr)
		return ade_set_args(L, "s", sip->missile_banks);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(VelocityString, l_Shipclass, "string", "Ship class velocity", "string", "velocity, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting Missile banks is not supported");
	}

	char str[100];
	sprintf(str, XSTR("%d m/s", 743), fl2i((float)sip->max_vel.xyz.z * Hud_speed_multiplier));

	return ade_set_args(L, "s", str);
}


ADE_VIRTVAR(Description, l_Shipclass, "string", "Ship class description", "string", "Description, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(sip->desc);
		if(s != NULL) {
			sip->desc = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->desc, s);
		} else {
			sip->desc = NULL;
		}
	}

	if(sip->desc != NULL)
		return ade_set_args(L, "s", sip->desc);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(SelectIconFilename, l_Shipclass, "string", "Ship class select icon filename", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		LuaError(L, "Setting Select Icon is not supported");
	}

	return ade_set_args(L, "s", Ship_info[idx].icon_filename);
}

ADE_VIRTVAR(SelectAnimFilename, l_Shipclass, "string", "Ship class select animation filename", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		LuaError(L, "Setting Select Anim is not supported");
	}

	return ade_set_args(L, "s", Ship_info[idx].anim_filename);
}

ADE_VIRTVAR(SelectOverheadFilename, l_Shipclass, "string", "Ship class select overhead filename", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if (!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		LuaError(L, "Setting Select Overhead Image is not supported");
	}

	return ade_set_args(L, "s", Ship_info[idx].overhead_filename);
}

ADE_VIRTVAR(TechDescription, l_Shipclass, "string", "Ship class tech description", "string", "Tech description, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	ship_info *sip = &Ship_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(sip->tech_desc);
		if(s != NULL) {
			sip->tech_desc = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->tech_desc, s);
		} else {
			sip->tech_desc = NULL;
		}
	}

	if(sip->tech_desc != NULL)
		return ade_set_args(L, "s", sip->tech_desc);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(numPrimaryBanks,
	l_Shipclass,
	nullptr,
	"Number of primary banks on this ship class",
	"number",
	"number of banks or nil is ship handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= ship_info_size())
		return ADE_RETURN_NIL;

	ship_info* sip = &Ship_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting number of banks is not supported");
	}

	return ade_set_args(L, "i", sip->num_primary_banks);
}

ADE_FUNC(getPrimaryBankCapacity,
	l_Shipclass,
	"number index",
	"Returns the capacity of the specified primary bank",
	"number",
	"The bank capacity or nil if the index is invalid")
{
	int shipIdx;
	int idx;
	if (!ade_get_args(L, "ii", &shipIdx, &idx))
		return ADE_RETURN_NIL;

	ship_info* sip = &Ship_info[shipIdx];

	if (idx < 1 || idx > sip->num_primary_banks) {
		return ADE_RETURN_NIL;
	};

	idx--; // Convert from Lua's 1 based index system

	return ade_set_args(L, "f", sip->primary_bank_ammo_capacity[idx]);
}

ADE_VIRTVAR(numSecondaryBanks,
	l_Shipclass,
	nullptr,
	"Number of secondary banks on this ship class",
	"number",
	"number of banks or nil is ship handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= ship_info_size())
		return ADE_RETURN_NIL;

	ship_info* sip = &Ship_info[idx];

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting number of banks is not supported");
	}

	return ade_set_args(L, "i", sip->num_secondary_banks);
}

ADE_FUNC(getSecondaryBankCapacity,
	l_Shipclass,
	"number index",
	"Returns the capacity of the specified secondary bank",
	"number",
	"The bank capacity or nil if the index is invalid")
{
	int shipIdx;
	int idx;
	if (!ade_get_args(L, "ii", &shipIdx, &idx))
		return ADE_RETURN_NIL;

	ship_info* sip = &Ship_info[shipIdx];

	if (idx < 1 || idx > sip->num_secondary_banks) {
		return ADE_RETURN_NIL;
	};

	idx--; // Convert from Lua's 1 based index system

	return ade_set_args(L, "f", sip->secondary_bank_ammo_capacity[idx]);
}

ADE_VIRTVAR(defaultPrimaries,
	l_Shipclass,
	"number",
	"Array of default primary weapons",
	"default_primary",
	"The weapons array or nil if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= ship_info_size())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "o", l_Default_Primary.Set(idx));
}

ADE_VIRTVAR(defaultSecondaries,
	l_Shipclass,
	"number",
	"Array of default secondary weapons",
	"default_secondary",
	"The weapons array or nil if handle is invalid")
{
	int idx;
	if (!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= ship_info_size())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This property is read only.");
	}

	return ade_set_args(L, "o", l_Default_Secondary.Set(idx));
}

ADE_FUNC(isWeaponAllowedOnShip,
	l_Shipclass,
	"number index, [number bank]",
	"Gets whether or not a weapon is allowed on a ship class. "
	"Optionally check a specific bank. Banks are 1 to a maximum of 7 where the first banks are Primaries and rest are "
	"Secondaries. "
	"Exact numbering depends on the ship class being checked. Note also that this will consider dogfight weapons only "
	"if "
	"a dogfight mission has been loaded. Index is index into Weapon Classes.",
	"boolean",
	"True if allowed, false if not.")
{
	int idx;
	int wepidx;
	int bank = 0;
	if (!ade_get_args(L, "oi|i", l_Shipclass.Get(&idx), &wepidx, &bank))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "b", false);
	wepidx--; // Convert from Lua
	if (wepidx < 0 || wepidx >= weapon_info_size())
		return ade_set_error(L, "b", false);
	if (bank != 0) {
		if (bank < 0 || bank >= (Ship_info[idx].num_primary_banks + Ship_info[idx].num_secondary_banks))
			return ade_set_error(L, "b", false);
	};
	bank--; // Convert from Lua

	bool retv = false;
	if ((bank >= 0) && (eval_weapon_flag_for_game_type(Ship_info[idx].restricted_loadout_flag[bank]))) {
		if (eval_weapon_flag_for_game_type(Ship_info[idx].allowed_bank_restricted_weapons[bank][wepidx]))
			retv = true;
	} else if (eval_weapon_flag_for_game_type(Ship_info[idx].allowed_weapons[wepidx])) {
		retv = true;
	}

	return ade_set_args(L, "b", retv);
}

ADE_VIRTVAR(AfterburnerFuelMax, l_Shipclass, "number", "Afterburner fuel capacity", "number", "Afterburner capacity, or 0 if handle is invalid")
{
	int idx;
	float fuel = -1.0f;
	if(!ade_get_args(L, "o|f", l_Shipclass.Get(&idx), &fuel))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR && fuel >= 0.0f)
		Ship_info[idx].afterburner_fuel_capacity = fuel;

	return ade_set_args(L, "f", Ship_info[idx].afterburner_fuel_capacity);
}

ADE_VIRTVAR(ScanTime, l_Shipclass, nullptr, "Ship scan time", "number", "Time required to scan, or 0 if handle is invalid. This propery is read-only")
{
	int idx;
	if (!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ade_set_error(L, "i", 0);

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "i", 0);

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting ScanTime is not supported");
	}

	return ade_set_args(L, "i", Ship_info[idx].scan_time);
}

ADE_VIRTVAR(CountermeasuresMax, l_Shipclass, "number", "Maximum number of countermeasures the ship can carry", "number", "Countermeasure capacity, or 0 if handle is invalid")
{
	int idx;
	int i = -1;
	if(!ade_get_args(L, "o|i", l_Shipclass.Get(&idx), &i))
		return ade_set_error(L, "i", 0);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR && i > -1) {
		Ship_info[idx].cmeasure_max = i;
	}

	return ade_set_args(L, "i", Ship_info[idx].cmeasure_max);
}

ADE_VIRTVAR(Model, l_Shipclass, "model", "Model", "model", "Ship class model, or invalid model handle if shipclass handle is invalid")
{
	int ship_info_idx=-1;
	model_h *mdl = NULL;
	if(!ade_get_args(L, "o|o", l_Shipclass.Get(&ship_info_idx), l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	if(ship_info_idx < 0 || ship_info_idx >= ship_info_size())
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	ship_info *sip = &Ship_info[ship_info_idx];

	int mid = (mdl ? mdl->GetID() : -1);

	if(ADE_SETTING_VAR && mid > -1) {
		sip->model_num = mid;
	}

	return ade_set_args(L, "o", l_Model.Set(model_h(sip->model_num)));
}

ADE_VIRTVAR(CockpitModel, l_Shipclass, "model", "Model used for first-person cockpit", "model", "Cockpit model")
{
	int ship_info_idx=-1;
	model_h *mdl = NULL;
	if(!ade_get_args(L, "o|o", l_Shipclass.Get(&ship_info_idx), l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_Model.Set(model_h()));

	if(ship_info_idx < 0 || ship_info_idx >= ship_info_size())
		return ade_set_error(L, "o", l_Model.Set(model_h()));

	ship_info *sip = &Ship_info[ship_info_idx];

	int mid = (mdl ? mdl->GetID() : -1);

	if(ADE_SETTING_VAR) {
		sip->cockpit_model_num = mid;
	}

	return ade_set_args(L, "o", l_Model.Set(model_h(sip->cockpit_model_num)));
}

ADE_VIRTVAR(CockpitDisplays, l_Shipclass, "cockpitdisplays", "Gets the cockpit display information array of this ship class", "cockpitdisplays", "Array handle containing the information or invalid handle on error")
{
	int ship_info_idx=-1;
	cockpit_displays_info_h *cdih = NULL;
	if(!ade_get_args(L, "o|o", l_Shipclass.Get(&ship_info_idx), l_CockpitDisplayInfos.GetPtr(&cdih)))
		return ade_set_error(L, "o", l_CockpitDisplayInfos.Set(cockpit_displays_info_h()));

	if(ship_info_idx < 0 || ship_info_idx >= ship_info_size())
		return ade_set_error(L, "o", l_CockpitDisplayInfos.Set(cockpit_displays_info_h()));

	if(ADE_SETTING_VAR) {
		LuaError(L, "Attempted to use incomplete feature: Cockpit display information copy");
	}

	return ade_set_args(L, "o", l_CockpitDisplayInfos.Set(cockpit_displays_info_h(ship_info_idx)));
}

ADE_VIRTVAR(HitpointsMax, l_Shipclass, "number", "Ship class hitpoints", "number", "Hitpoints, or 0 if handle is invalid")
{
	int idx;
	float f = -1.0f;
	if(!ade_get_args(L, "o|f", l_Shipclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR && f >= 0.0f) {
		Ship_info[idx].max_hull_strength = f;
	}

	return ade_set_args(L, "f", Ship_info[idx].max_hull_strength);
}

ADE_VIRTVAR(Species, l_Shipclass, "species", "Ship class species", "species", "Ship class species, or invalid species handle if shipclass handle is invalid")
{
	int idx;
	int sidx = -1;
	if(!ade_get_args(L, "o|o", l_Shipclass.Get(&idx), l_Species.Get(&sidx)))
		return ade_set_error(L, "o", l_Species.Set(-1));

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "o", l_Species.Set(-1));

	if(ADE_SETTING_VAR && sidx > -1 && sidx < (int)Species_info.size()) {
		Ship_info[idx].species = sidx;
	}

	return ade_set_args(L, "o", l_Species.Set(Ship_info[idx].species));
}

ADE_VIRTVAR(Type, l_Shipclass, "shiptype", "Ship class type", "shiptype", "Ship type, or invalid handle if shipclass handle is invalid")
{
	int idx;
	int sidx = -1;
	if(!ade_get_args(L, "o|o", l_Shipclass.Get(&idx), l_Shiptype.Get(&sidx)))
		return ade_set_error(L, "o", l_Shiptype.Set(-1));

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "o", l_Shiptype.Set(-1));

	if(ADE_SETTING_VAR && sidx > -1 && sidx < (int)Ship_types.size()) {
		Ship_info[idx].class_type = sidx;
	}

	return ade_set_args(L, "o", l_Shiptype.Set(Ship_info[idx].class_type));
}

ADE_VIRTVAR(AltName, l_Shipclass, "string", "Alternate name for ship class", "string", "Alternate string or empty string if handle is invalid")
{
	const char* newName = nullptr;
	int idx;
	if(!ade_get_args(L, "o|s", l_Shipclass.Get(&idx), &newName))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && newName != NULL) {
		if (strlen(newName) >= NAME_LENGTH)
		{
			LuaError(L, "Cannot set alternate name value to '%s' because it is too long, maximum length is %d!", newName, NAME_LENGTH - 1);
			return ade_set_error(L, "s", "");
		}

		strcpy_s(Ship_info[idx].display_name, newName);
	}

	return ade_set_args(L, "s", Ship_info[idx].display_name);
}

ADE_VIRTVAR(Score, l_Shipclass, "string", "The score of this ship class", "number", "The score or -1 on invalid ship class")
{
	int idx;
	int new_score;
	if(!ade_get_args(L, "o|i", l_Shipclass.Get(&idx), &new_score))
		return ade_set_error(L, "i", -1);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "i", -1);

	if(ADE_SETTING_VAR) {
		Ship_info[idx].score = new_score;
	}

	return ade_set_args(L, "i", Ship_info[idx].score);
}

ADE_VIRTVAR(InTechDatabase, l_Shipclass, "boolean", "Gets or sets whether this ship class is visible in the tech room", "boolean", "True or false")
{
	int idx;
	bool new_value;
	if (!ade_get_args(L, "o|b", l_Shipclass.Get(&idx), &new_value))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "b", false);

	auto flag = (Player && (Player->flags & PLAYER_FLAGS_IS_MULTI))
		? Ship::Info_Flags::In_tech_database_m
		: Ship::Info_Flags::In_tech_database;

	if (ADE_SETTING_VAR) {
		Ship_info[idx].flags.set(flag, new_value);
	}

	return ade_set_args(L, "b", Ship_info[idx].flags[flag]);
}

ADE_VIRTVAR(PowerOutput, l_Shipclass, "number", "Gets or sets a ship class' power output", "number", "The ship class' current power output")
{
	int idx;
	float new_power;
	if (!ade_get_args(L, "o|f", l_Shipclass.Get(&idx), &new_power))
		return ade_set_error(L, "f", -1.0f);

	if (idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR) {
		Ship_info[idx].power_output = new_power;
	}

	return ade_set_args(L, "f", Ship_info[idx].power_output);
}

ADE_VIRTVAR(ScanningTimeMultiplier, l_Shipclass, nullptr, "Time multiplier for scans performed by this ship class", "number", "Scanning time multiplier, or 0 if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		LuaError(L, "Setting ScanningTimeMultiplier is not supported");
	}

	return ade_set_args(L, "f", Ship_info[idx].scanning_time_multiplier);
}

ADE_VIRTVAR(ScanningRangeMultiplier, l_Shipclass, nullptr, "Range multiplier for scans performed by this ship class", "number", "Scanning range multiplier, or 0 if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		LuaError(L, "Setting ScanningRangeMultiplier is not supported");
	}

	return ade_set_args(L, "f", Ship_info[idx].scanning_range_multiplier);
}

ADE_VIRTVAR(CustomData, l_Shipclass, nullptr, "Gets the custom data table for this ship class", "table", "The ship class's custom data table") 
{
	int idx;
	if(!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ADE_RETURN_NIL;
	
	if(idx < 0 || idx >= ship_info_size())
		return ADE_RETURN_NIL;

	auto table = luacpp::LuaTable::create(L);
	
	ship_info *sip = &Ship_info[idx];

	for (const auto& pair : sip->custom_data)
	{
		table.addValue(pair.first, pair.second);
	}

	return ade_set_args(L, "t", &table);	
}

ADE_FUNC(hasCustomData, l_Shipclass, nullptr, "Detects whether the ship class has any custom data", "boolean", "true if the shipclass's custom_data is not empty, false otherwise") 
{
	int idx;
	if(!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ADE_RETURN_NIL;
	
	if(idx < 0 || idx >= ship_info_size())
		return ADE_RETURN_NIL;

	ship_info *sip = &Ship_info[idx];

	bool result = !sip->custom_data.empty();
	return ade_set_args(L, "b", result);
}

ADE_FUNC(isValid, l_Shipclass, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= ship_info_size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isInTechroom, l_Shipclass, NULL, "Gets whether or not the ship class is available in the techroom", "boolean", "Whether ship has been revealed in the techroom, false if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ade_set_error(L, "b", false);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_error(L, "b", false);

	bool b = false;
	if(Player != NULL && (Player->flags & PLAYER_FLAGS_IS_MULTI) && (Ship_info[idx].flags[Ship::Info_Flags::In_tech_database_m])) {
		b = true;
	} else if(Ship_info[idx].flags[Ship::Info_Flags::In_tech_database]) {
		b = true;
	}

	return ade_set_args(L, "b", b);
}

ADE_FUNC(renderTechModel,
	l_Shipclass,
	"number X1, number Y1, number X2, number Y2, [number RotationPercent =0, number PitchPercent =0, number "
	"BankPercent=40, number Zoom=1.3]",
	"Draws ship model as if in techroom",
	"boolean",
	"Whether ship was rendered")
{
	int x1,y1,x2,y2;
	angles rot_angles = {0.0f, 0.0f, 40.0f};
	int idx;
	float zoom = 1.3f;
	if(!ade_get_args(L, "oiiii|ffff", l_Shipclass.Get(&idx), &x1, &y1, &x2, &y2, &rot_angles.h, &rot_angles.p, &rot_angles.b, &zoom))
		return ade_set_error(L, "b", false);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_args(L, "b", false);

	if(x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	CLAMP(rot_angles.p, 0.0f, 100.0f);
	CLAMP(rot_angles.b, 0.0f, 100.0f);
	CLAMP(rot_angles.h, 0.0f, 100.0f);

	ship_info *sip = &Ship_info[idx];
	model_render_params render_info;

	if (sip->uses_team_colors) {
		render_info.set_team_color(sip->default_team_name, "none", 0, 0);
	}

	//Make sure model is loaded
	sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0], 0);

	if(sip->model_num < 0)
		return ade_set_args(L, "b", false);

	//Handle angles
	matrix orient = vmd_identity_matrix;
	angles view_angles = {-0.6f, 0.0f, 0.0f};
	vm_angles_2_matrix(&orient, &view_angles);

	rot_angles.p = (rot_angles.p*0.01f) * PI2;
	rot_angles.b = (rot_angles.b*0.01f) * PI2;
	rot_angles.h = (rot_angles.h*0.01f) * PI2;
	vm_rotate_matrix_by_angles(&orient, &rot_angles);

	//Clip
	gr_set_clip(x1,y1,x2-x1,y2-y1,GR_RESIZE_NONE);

	//Handle 3D init stuff
	g3_start_frame(1);
	g3_set_view_matrix(&sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * zoom);

	gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	//setup lights
	common_setup_room_lights();

	//Draw the ship!!
	model_clear_instance(sip->model_num);
	render_info.set_detail_level_lock(0);

	uint render_flags = MR_AUTOCENTER | MR_NO_FOGGING;

	if(sip->flags[Ship::Info_Flags::No_lighting])
		render_flags |= MR_NO_LIGHTING;

	render_info.set_flags(render_flags);

	model_render_immediate(&render_info, sip->model_num, &orient, &vmd_zero_vector);

	//OK we're done
	gr_end_view_matrix();
	gr_end_proj_matrix();

	//Bye!!
	g3_end_frame();
	gr_reset_clip();

	return ade_set_args(L, "b", true);
}

// Nuke's alternate tech model rendering function
ADE_FUNC(renderTechModel2, l_Shipclass, "number X1, number Y1, number X2, number Y2, [orientation Orientation=nil, number Zoom=1.3]", "Draws ship model as if in techroom", "boolean", "Whether ship was rendered")
{
	int x1,y1,x2,y2;
	int idx;
	float zoom = 1.3f;
	matrix_h *mh = NULL;
	if(!ade_get_args(L, "oiiiio|f", l_Shipclass.Get(&idx), &x1, &y1, &x2, &y2,  l_Matrix.GetPtr(&mh), &zoom))
		return ade_set_error(L, "b", false);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_args(L, "b", false);

	if(x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	ship_info *sip = &Ship_info[idx];
	model_render_params render_info;

	if (sip->uses_team_colors) {
		render_info.set_team_color(sip->default_team_name, "none", 0, 0);
	}

	//Make sure model is loaded
	sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0], 0);

	if(sip->model_num < 0)
		return ade_set_args(L, "b", false);

	//Handle angles
	matrix *orient = mh->GetMatrix();

	//Clip
	gr_set_clip(x1,y1,x2-x1,y2-y1,GR_RESIZE_NONE);

	//Handle 3D init stuff
	g3_start_frame(1);
	g3_set_view_matrix(&sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * zoom);

	gr_set_proj_matrix( Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	//setup lights
	common_setup_room_lights();

	//Draw the ship!!
	model_clear_instance(sip->model_num);
	render_info.set_detail_level_lock(0);

	uint render_flags = MR_AUTOCENTER | MR_NO_FOGGING;

	if(sip->flags[Ship::Info_Flags::No_lighting])
		render_flags |= MR_NO_LIGHTING;

	render_info.set_flags(render_flags);

	model_render_immediate(&render_info, sip->model_num, orient, &vmd_zero_vector);

	//OK we're done
	gr_end_view_matrix();
	gr_end_proj_matrix();

	//Bye!!
	g3_end_frame();
	gr_reset_clip();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(isModelLoaded, l_Shipclass, "[boolean Load = false]", "Checks if the model used for this shipclass is loaded or not and optionally loads the model, which might be a slow operation.", "boolean", "If the model is loaded or not")
{
	int idx;
	bool load_check = false;
	if(!ade_get_args(L, "o|b", l_Shipclass.Get(&idx), &load_check))
		return ADE_RETURN_FALSE;

	ship_info *sip = &Ship_info[idx];

	if (sip == NULL)
		return ADE_RETURN_FALSE;

	if(load_check){
		sip->model_num = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0]);
	}

	if (sip->model_num > -1)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(getShipClassIndex, l_Shipclass, nullptr, "Gets the index value of the ship class", "number", "index value of the ship class")
{
	int idx;
	if(!ade_get_args(L, "o", l_Shipclass.Get(&idx)))
		return ade_set_args(L, "i", -1);

	if(idx < 0 || idx >= ship_info_size())
		return ade_set_args(L, "i", -1);

	return ade_set_args(L, "i", idx + 1); // Lua is 1-based
}


}
}
