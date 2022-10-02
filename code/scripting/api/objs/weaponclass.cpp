//
//

#include "weaponclass.h"
#include "model.h"
#include "weapon/weapon.h"
#include "graphics/matrix.h"
#include "vecmath.h"
#include "missionui/missionscreencommon.h"

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

ADE_VIRTVAR(SelectIconFilename, l_Weaponclass, "string", "Weapon class select icon filename", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		LuaError(L, "Setting Select Icon is not supported");
	}

	return ade_set_args(L, "s", Weapon_info[idx].icon_filename);
}

ADE_VIRTVAR(SelectAnimFilename, l_Weaponclass, "string", "Weapon class select animation filename", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return ade_set_error(L, "s", "");

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR) {
		LuaError(L, "Setting Select Anim is not supported");
	}

	return ade_set_args(L, "s", Weapon_info[idx].anim_filename);
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


ADE_VIRTVAR(InnerRadius, l_Weaponclass, "number", "Radius at which the full explosion damage is done. Marks the line where damage attenuation begins. Same as $Inner Radius in weapons.tbl", "number", "Inner Radius, or 0 if handle is invalid")
{
	int idx;
	float irad = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &irad))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].shockwave.inner_rad = irad;
	}

	return ade_set_args(L, "f", Weapon_info[idx].shockwave.inner_rad);
}


ADE_VIRTVAR(OuterRadius, l_Weaponclass, "number", "Maximum Radius at which any damage is done with this weapon. Same as $Outer Radius in weapons.tbl", "number", "Outer Radius, or 0 if handle is invalid")
{
	int idx;
	float orad = 0.0f;
	if(!ade_get_args(L, "o|f", l_Weaponclass.Get(&idx), &orad))
		return ade_set_error(L, "f", 0.0f);

	if(idx < 0 || idx >= weapon_info_size())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR) {
		Weapon_info[idx].shockwave.outer_rad = orad;
	}

	return ade_set_args(L, "f", Weapon_info[idx].shockwave.outer_rad);
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

ADE_VIRTVAR(CustomData, l_Weaponclass, nullptr, "Gets the custom data table for this weapon class", "table", "The weapon class's custom data table") 
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;
	
	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_NIL;
		
	auto table = luacpp::LuaTable::create(L);
	
	weapon_info *wip = &Weapon_info[idx];

	for (const auto& pair : wip->custom_data)
	{
		table.addValue(pair.first, pair.second);
	}

	return ade_set_args(L, "t", &table);	
}

ADE_FUNC(hasCustomData, l_Weaponclass, nullptr, "Detects whether the weapon class has any custom data", "boolean", "true if the weaponclass's custom_data is not empty, false otherwise") 
{
	int idx;
	if(!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;
	
	if(idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_NIL;

	weapon_info *wip = &Weapon_info[idx];

	bool result = !wip->custom_data.empty();
	return ade_set_args(L, "b", result);
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

ADE_FUNC(renderTechModel,
	l_Weaponclass,
	"number X1, number Y1, number X2, number Y2, [number RotationPercent =0, number PitchPercent =0, number "
	"BankPercent=40, number Zoom=1.3]",
	"Draws weapon tech model",
	"boolean",
	"Whether weapon was rendered")
{
	int x1, y1, x2, y2;
	angles rot_angles = {0.0f, 0.0f, 40.0f};
	int idx;
	float zoom = 1.3f;
	if (!ade_get_args(L,
			"oiiii|ffff",
			l_Weaponclass.Get(&idx),
			&x1,
			&y1,
			&x2,
			&y2,
			&rot_angles.h,
			&rot_angles.p,
			&rot_angles.b,
			&zoom))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= weapon_info_size())
		return ade_set_args(L, "b", false);

	if (x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	CLAMP(rot_angles.p, 0.0f, 100.0f);
	CLAMP(rot_angles.b, 0.0f, 100.0f);
	CLAMP(rot_angles.h, 0.0f, 100.0f);

	weapon_info* wip = &Weapon_info[idx];
	model_render_params render_info;

	//Load the model if it exists or exit early
	if (VALID_FNAME(wip->tech_model)) {
		wip->model_num = model_load(wip->tech_model, 0, NULL, 0);
	} else {
		return ade_set_args(L, "b", false);
	}

	if (wip->model_num < 0)
		return ade_set_args(L, "b", false);

	// Handle angles
	matrix orient = vmd_identity_matrix;
	angles view_angles = {-0.6f, 0.0f, 0.0f};
	vm_angles_2_matrix(&orient, &view_angles);

	rot_angles.p = (rot_angles.p * 0.01f) * PI2;
	rot_angles.b = (rot_angles.b * 0.01f) * PI2;
	rot_angles.h = (rot_angles.h * 0.01f) * PI2;
	vm_rotate_matrix_by_angles(&orient, &rot_angles);

	// Clip
	gr_set_clip(x1, y1, x2 - x1, y2 - y1, GR_RESIZE_NONE);

	// Handle 3D init stuff
	g3_start_frame(1);
	g3_set_view_matrix(&wip->closeup_pos, &vmd_identity_matrix, wip->closeup_zoom * zoom);

	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	// Handle light
	light_reset();
	vec3d light_dir = vmd_zero_vector;
	light_dir.xyz.y = 1.0f;
	light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
	light_rotate_all();

	// Draw the ship!!
	model_clear_instance(wip->model_num);
	render_info.set_detail_level_lock(0);

	uint render_flags = MR_AUTOCENTER | MR_NO_FOGGING;

	if (wip->wi_flags[Weapon::Info_Flags::Mr_no_lighting])
		render_flags |= MR_NO_LIGHTING;

	render_info.set_flags(render_flags);

	model_render_immediate(&render_info, wip->model_num, &orient, &vmd_zero_vector);

	// OK we're done
	gr_end_view_matrix();
	gr_end_proj_matrix();

	// Bye!!
	g3_end_frame();
	gr_reset_clip();

	return ade_set_args(L, "b", true);
}

// Nuke's alternate tech model rendering function
ADE_FUNC(renderTechModel2,
	l_Weaponclass,
	"number X1, number Y1, number X2, number Y2, [orientation Orientation=nil, number Zoom=1.3]",
	"Draws weapon tech model",
	"boolean",
	"Whether weapon was rendered")
{
	int x1, y1, x2, y2;
	int idx;
	float zoom = 1.3f;
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "oiiiio|f", l_Weaponclass.Get(&idx), &x1, &y1, &x2, &y2, l_Matrix.GetPtr(&mh), &zoom))
		return ade_set_error(L, "b", false);

	if (idx < 0 || idx >= weapon_info_size())
		return ade_set_args(L, "b", false);

	if (x2 < x1 || y2 < y1)
		return ade_set_args(L, "b", false);

	weapon_info* wip = &Weapon_info[idx];
	model_render_params render_info;

	// Load the model if it exists or exit early
	if (VALID_FNAME(wip->tech_model)) {
		wip->model_num = model_load(wip->tech_model, 0, NULL, 0);
	} else {
		return ade_set_args(L, "b", false);
	}

	if (wip->model_num < 0)
		return ade_set_args(L, "b", false);

	// Handle angles
	matrix* orient = mh->GetMatrix();

	// Clip
	gr_set_clip(x1, y1, x2 - x1, y2 - y1, GR_RESIZE_NONE);

	// Handle 3D init stuff
	g3_start_frame(1);
	g3_set_view_matrix(&wip->closeup_pos, &vmd_identity_matrix, wip->closeup_zoom * zoom);

	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	// Handle light
	light_reset();
	vec3d light_dir = vmd_zero_vector;
	light_dir.xyz.y = 1.0f;
	light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
	light_rotate_all();

	// Draw the ship!!
	model_clear_instance(wip->model_num);
	render_info.set_detail_level_lock(0);

	uint render_flags = MR_AUTOCENTER | MR_NO_FOGGING;

	if (wip->wi_flags[Weapon::Info_Flags::Mr_no_lighting])
		render_flags |= MR_NO_LIGHTING;

	render_info.set_flags(render_flags);

	model_render_immediate(&render_info, wip->model_num, orient, &vmd_zero_vector);

	// OK we're done
	gr_end_view_matrix();
	gr_end_proj_matrix();

	// Bye!!
	g3_end_frame();
	gr_reset_clip();

	return ade_set_args(L, "b", true);
}

ADE_FUNC(renderSelectModel,
	l_Weaponclass,
	"number x, number y, [number width = 629, number height = 355, number = currentEffectSetting]",
	"Draws the 3D select weapon model with the chosen effect at the specified coordinates. Restart should "
	"be true on the first frame this is called and false on subsequent frames. Note that primary weapons "
	"will not render anything if they do not have a valid pof model defined! Valid selection effects are 1 (fs1) or 2 (fs2), "
	"defaults to the mod setting or the model's setting.",
	"boolean",
	"true if rendered, false if error")
{
	int idx;
	bool restart;
	int x1;
	int y1;
	int x2 = 629;
	int y2 = 355;
	int effect = -1;
	if (!ade_get_args(L, "obii|iii", l_Weaponclass.Get(&idx), &restart, &x1, &y1, &x2, &y2, &effect))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= weapon_info_size())
		return ade_set_args(L, "b", false);

	if (effect == 0 || effect > 2) {
		LuaError(L, "Valid effect values are 1 or 2, got %i", effect);
		return ade_set_args(L, "b", false);
	}

	if (restart) {
		anim_timer_start = timer_get_milliseconds();
	}

	weapon_info* wip = &Weapon_info[idx];

	if (effect == -1) {
		effect = wip->selection_effect;
	}

	int modelNum;
	if (VALID_FNAME(wip->tech_model)) {
		modelNum = model_load(wip->tech_model, 0, nullptr, 0);
	} else if (wip->render_type != WRT_LASER) {
		modelNum = model_load(wip->pofbitmap_name, 0, nullptr);
	} else {
		return ade_set_args(L, "b", false);
	}

	static float WepRot = 0.0f;

	model_render_params render_info;

	draw_model_rotating(&render_info,
		modelNum,
		x1,
		y1,
		x2,
		y2,
		&WepRot,
		&wip->closeup_pos,
		wip->closeup_zoom * 0.65f,
		REVOLUTION_RATE,
		MR_IS_MISSILE | MR_AUTOCENTER | MR_NO_FOGGING,
		GR_RESIZE_NONE,
		effect);

	return ade_set_args(L, "b", true);
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

ADE_FUNC(isWeaponRequired,
	l_Weaponclass,
	nullptr,
	"Checks if a weapon is required for the currently loaded mission",
	"boolean",
	"true if required, false if otherwise. Nil if the weapon class is invalid or a mission has not been loaded")
{
	int idx = -1;
	if (!ade_get_args(L, "o", l_Weaponclass.Get(&idx)))
		return ADE_RETURN_NIL;

	if (idx < 0 || idx >= weapon_info_size())
		return ADE_RETURN_NIL;

	//This could be requested before Common_team has been initialized, so let's check.
	if (Common_select_inited) {
		return ade_set_args(L, "b", Team_data[Common_team].weapon_required[idx]);
	} else {
		return ADE_RETURN_NIL;
	}
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
