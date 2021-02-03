//
//

#include "weaponclass.h"
#include "model.h"
#include "weapon/weapon.h"

namespace scripting {
namespace api {


//**********HANDLE: Weaponclass
ADE_OBJ(l_Weaponclass, int, "weaponclass", "Weapon class handle");

ADE_FUNC(__tostring, l_Weaponclass, NULL, "Weapon class name", "string", "Weapon class name, or an empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Weapon_info[idx].get_display_name());
}

ADE_FUNC(__eq, l_Weaponclass, "weaponclass, weaponclass", "Checks if the two classes are equal", "boolean", "true if equal, false otherwise")
{
	int idx1,idx2;
	if(!ade_get_args(L, "oo", l_Weaponclass.Get(&idx1), l_Weaponclass.Get(&idx2)))
		return ade_set_error(L, "b", false);

	if(idx1 < 0 || idx1 >= weapon_info_size())
		return ade_set_error(L, "b", false);

	if(idx2 < 0 || idx2 >= weapon_info_size())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", idx1 == idx2);
}

ADE_VIRTVAR(Name, l_Weaponclass, "string", "Weapon class name. This is the possibly untranslated name. Use tostring(class) to get the string which should be shown to the user.", "string", "Weapon class name, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(Weapon_info[idx].name);
		strncpy(Weapon_info[idx].name, s, len);
		Weapon_info[idx].name[len - 1] = 0;
	}

	return ade_set_args(L, "s", Weapon_info[idx].name);
}

ADE_VIRTVAR(AltName, l_Weaponclass, "string", "The alternate weapon class name.", "string", "Alternate weapon class name, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(Weapon_info[idx].display_name);
		strncpy(Weapon_info[idx].display_name, s, len);
		Weapon_info[idx].display_name[len - 1] = 0;
	}

	return ade_set_args(L, "s", Weapon_info[idx].display_name);
}

ADE_VIRTVAR(Title, l_Weaponclass, "string", "Weapon class title", "string", "Weapon class title, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(Weapon_info[idx].title);
		strncpy(Weapon_info[idx].title, s, len);
		Weapon_info[idx].title[len - 1] = 0;
	}

	return ade_set_args(L, "s", Weapon_info[idx].title);
}

ADE_VIRTVAR(Description, l_Weaponclass, "string", "Weapon class description string", "string", "Description string, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	weapon_info *wip = &Weapon_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(wip->desc);
		if(s != nullptr) {
			wip->desc = (char*)vm_malloc(strlen(s)+1);
			strcpy(wip->desc, s);
		} else {
			wip->desc = nullptr;
		}
	}

	if(wip->desc != nullptr)
		return ade_set_args(L, "s", wip->desc);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(TechTitle, l_Weaponclass, "string", "Weapon class tech title", "string", "Tech title, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(Weapon_info[idx].tech_title);
		strncpy(Weapon_info[idx].tech_title, s, len);
		Weapon_info[idx].tech_title[len - 1] = 0;
	}

	return ade_set_args(L, "s", Weapon_info[idx].tech_title);
}

ADE_VIRTVAR(TechAnimationFilename, l_Weaponclass, "string", "Weapon class animation filename", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != nullptr) {
		auto len = sizeof(Weapon_info[idx].tech_anim_filename);
		strncpy(Weapon_info[idx].tech_anim_filename, s, len);
		Weapon_info[idx].tech_anim_filename[len - 1] = 0;
	}

	return ade_set_args(L, "s", Weapon_info[idx].tech_anim_filename);
}

ADE_VIRTVAR(TechDescription, l_Weaponclass, "string", "Weapon class tech description string", "string", "Description string, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	weapon_info *wip = &Weapon_info[idx];

	if(ADE_SETTING_VAR) {
		vm_free(wip->tech_desc);
		if(s != nullptr) {
			wip->tech_desc = (char*)vm_malloc(strlen(s)+1);
			strcpy(wip->tech_desc, s);
		} else {
			wip->tech_desc = nullptr;
		}
	}

	if(wip->tech_desc != nullptr)
		return ade_set_args(L, "s", wip->tech_desc);
	else
		return ade_set_args(L, "s", "");
}

ADE_VIRTVAR(Model, l_Weaponclass, "model", "Model", "model", "Weapon class model, or invalid model handle if weaponclass handle is invalid")
{
	int weapon_info_idx=-1;
	model_h *mdl = nullptr;
	if(!ade_get_args(L, "o|o", l_Weaponclass.Get(&weapon_info_idx), l_Model.GetPtr(&mdl)))
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	if(weapon_info_idx < 0 || weapon_info_idx >= weapon_info_size())
		return ade_set_error(L, "o", l_Model.Set(model_h(-1)));

	weapon_info *wip = &Weapon_info[weapon_info_idx];

	int mid = (mdl ? mdl->GetID() : -1);

	if(ADE_SETTING_VAR && mid > -1) {
		wip->model_num = mid;
	}

	return ade_set_args(L, "o", l_Model.Set(model_h(wip->model_num)));
}

ADE_VIRTVAR(ArmorFactor, l_Weaponclass, "number", "Amount of weapon damage applied to ship hull (0-1.0)", "number", "Armor factor, or empty string if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].armor_factor = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].armor_factor);
}

ADE_VIRTVAR(Damage, l_Weaponclass, "number", "Amount of damage that weapon deals", "number", "Damage amount, or 0 if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].damage = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].damage);
}

ADE_VIRTVAR(FireWait, l_Weaponclass, "number", "Weapon fire wait (cooldown time) in seconds", "number", "Fire wait time, or 0 if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].fire_wait = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].fire_wait);
}

ADE_VIRTVAR(FreeFlightTime, l_Weaponclass, "number", "The time the weapon will fly before turing onto its target", "number", "Free flight time or emty string if invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].free_flight_time = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].free_flight_time);
}

ADE_VIRTVAR(LifeMax, l_Weaponclass, "number", "Life of weapon in seconds", "number", "Life of weapon, or 0 if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].lifetime = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].lifetime);
}

ADE_VIRTVAR(Range, l_Weaponclass, "number", "Range of weapon in meters", "number", "Weapon Range, or 0 if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].weapon_range = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].weapon_range);
}

ADE_VIRTVAR(Mass, l_Weaponclass, "number", "Weapon mass", "number", "Weapon mass, or 0 if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].mass = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].mass);
}

ADE_VIRTVAR(ShieldFactor, l_Weaponclass, "number", "Amount of weapon damage applied to ship shields (0-1.0)", "number", "Shield damage factor, or 0 if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].shield_factor = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].shield_factor);
}

ADE_VIRTVAR(SubsystemFactor, l_Weaponclass, "number", "Amount of weapon damage applied to ship subsystems (0-1.0)", "number", "Subsystem damage factor, or 0 if handle is invalid")
{
	int idx;
	float f = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &f))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].subsystem_factor = f;
	}

	return ade_set_args(L, "f", Weapon_info[idx].subsystem_factor);
}

ADE_VIRTVAR(TargetLOD, l_Weaponclass, "number", "LOD used for weapon model in the targeting computer", "number", "LOD number, or 0 if handle is invalid")
{
	int idx;
	int lod = 0;
	if(!ade_get_args(L, "o|i", l_Weaponclass.Get(&idx), &lod))
		return ade_set_error(L, "i", 0);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "i", 0);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].hud_target_lod = lod;
	}

	return ade_set_args(L, "i", Weapon_info[idx].hud_target_lod);
}

ADE_VIRTVAR(Speed, l_Weaponclass, "number", "Weapon max speed, aka $Velocity in weapons.tbl", "number", "Weapon speed, or 0 if handle is invalid")
{
	int idx;
	float spd = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &spd))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].max_speed = spd;
	}

	return ade_set_args(L, "f", Weapon_info[idx].max_speed);
}

ADE_VIRTVAR(Bomb, l_Weaponclass, "boolean", "Is weapon class flagged as bomb", "boolean", "New flag")
{
	int idx;
	bool newVal = false;
	if(!ade_get_args(L, "o|b", l_Weaponclass.Get(&idx), &newVal))
		return ADE_RETURN_FALSE;

	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	weapon_info *info = &Weapon_info[idx];

	if(ADE_SETTING_VAR)
	{
		info->wi_flags.set(Weapon::Info_Flags::Bomb, newVal);
	}


	if (info->wi_flags[Weapon::Info_Flags::Bomb])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(InTechDatabase, l_Weaponclass, "boolean", "Gets or sets whether this weapon class is visible in the tech room", "boolean", "True or false")
{
	int idx;
	bool new_value;
	if (!ade_get_args(L, "o|b", l_Weaponclass.Get(&idx), &new_value))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "b", false);

	if (ADE_SETTING_VAR) {
		Weapon_info[idx].wi_flags.set(Weapon::Info_Flags::In_tech_database, new_value);
	}

	return ade_set_args(L, "b", Weapon_info[idx].wi_flags[Weapon::Info_Flags::In_tech_database]);
}

ADE_VIRTVAR(CargoSize, l_Weaponclass, "number", "The cargo size of this weapon class", "number", "The new cargo size or -1 on error")
{
	int idx;
	float newVal = -1.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &newVal))
		return ade_set_args(L, "f", -1.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_args(L, "f", -1.0f);

	weapon_info *info = &Weapon_info[idx];

	if(ADE_SETTING_VAR)
	{
		if(newVal > 0)
		{
			info->cargo_size = newVal;
		}
		else
		{
			LuaError(L, "Cargo size must be bigger than zero, got %f!", newVal);
		}
	}

	return ade_set_args(L, "f", info->cargo_size);
}

ADE_FUNC(isValid, l_Weaponclass, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(getWeaponClassIndex, l_Weaponclass, NULL, "Gets the index value of the weapon class", "number", "index value of the weapon class")
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ade_set_args(L, "i", -1);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_args(L, "i", -1);

	return ade_set_args(L, "i", idx + 1);
}

ADE_FUNC(isLaser, l_Weaponclass, NULL, "Return true if the weapon is a primary weapon (this includes Beams). This function is deprecated, use isPrimary instead.", "boolean", "true if the weapon is a primary, false otherwise")
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	if (Weapon_info[idx].subtype == WP_LASER)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(isMissile, l_Weaponclass, NULL, "Return true if the weapon is a secondary weapon. This function is deprecated, use isSecondary instead.", "boolean", "true if the weapon is a secondary, false otherwise")
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	if (Weapon_info[idx].subtype == WP_MISSILE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(isPrimary, l_Weaponclass, NULL, "Return true if the weapon is a primary weapon (this includes Beams)", "boolean", "true if the weapon is a primary, false otherwise")
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	if (Weapon_info[idx].subtype == WP_LASER)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(isSecondary, l_Weaponclass, NULL, "Return true if the weapon is a secondary weapon", "boolean", "true if the weapon is a secondary, false otherwise")
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	if (Weapon_info[idx].subtype == WP_MISSILE)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(isBeam, l_Weaponclass, NULL, "Return true if the weapon is a beam", "boolean", "true if the weapon is a beam, false otherwise")
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	if (Weapon_info[idx].wi_flags[Weapon::Info_Flags::Beam] || Weapon_info[idx].subtype == WP_BEAM)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

// Checks if a weapon has been paged in (counted as used)
ADE_FUNC(isWeaponUsed, l_Weaponclass, NULL, "Return true if the weapon is paged in.", "boolean", "True if the weapon is paged in, false if otherwise")
{
	int idx = -1;
	if (!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	if (!weapon_used(idx)) {
		return ADE_RETURN_FALSE;
	} else {
		return ADE_RETURN_TRUE;
	}
}

// Pages in a weapon
ADE_FUNC(loadWeapon, l_Weaponclass, NULL, "Pages in a weapon. Returns True on success.", "boolean", "True if page in was successful, false otherwise.")
{
	int idx = -1;
	if (!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_FALSE;

	if (!weapon_page_in(idx)) {
		return ADE_RETURN_FALSE;
	} else {
		return ADE_RETURN_TRUE;
	}
}


}
}
