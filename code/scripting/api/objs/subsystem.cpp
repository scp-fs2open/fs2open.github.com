//
//

#include "subsystem.h"
#include "model_path.h"
#include "object.h"
#include "ship.h"
#include "ship_bank.h"
#include "vecmath.h"
#include "hud/hudtarget.h"
#include "ship/shiphit.h"

bool turret_fire_weapon(int weapon_num, ship_subsys *turret, int parent_objnum, vec3d *turret_pos, vec3d *firing_vec, vec3d *predicted_pos = nullptr, float flak_range_override = 100.0f);

namespace scripting {
namespace api {

ship_subsys_h::ship_subsys_h() : object_h() {
	ss = NULL;
}
ship_subsys_h::ship_subsys_h(object* objp_in, ship_subsys* sub) : object_h(objp_in) {
	ss = sub;
}
bool ship_subsys_h::isSubsystemValid() const { return object_h::IsValid() && objp->type == OBJ_SHIP && ss != nullptr; }

//**********HANDLE: Subsystem
ADE_OBJ(l_Subsystem, ship_subsys_h, "subsystem", "Ship subsystem handle");


ADE_FUNC(__tostring, l_Subsystem, NULL, "Returns name of subsystem", "string", "Subsystem name, or empty string if handle is invalid")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "s", "");

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", ship_subsys_get_name(sso->ss));
}

ADE_VIRTVAR(ArmorClass, l_Subsystem, "string", "Current Armor class", "string", "Armor class name, or empty string if none is set")
{
	ship_subsys_h *sso;
	const char* s    = nullptr;
	const char *name = nullptr;

	if(!ade_get_args(L, "o|s", l_Subsystem.GetPtr(&sso), &s))
		return ade_set_error(L, "s", "");

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "s", "");

	ship_subsys *ssys = sso->ss;

	int atindex;
	if (ADE_SETTING_VAR && s != nullptr) {
		atindex = armor_type_get_idx(s);
		ssys->armor_type_idx = atindex;
	} else {
		atindex = ssys->armor_type_idx;
	}

	if (atindex != -1)
		name = Armor_types[atindex].GetNamePtr();
	else
		name = "";

	return ade_set_args(L, "s", name);
}

ADE_VIRTVAR(AWACSIntensity, l_Subsystem, "number", "Subsystem AWACS intensity", "number", "AWACS intensity, or 0 if handle is invalid")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!ade_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return ade_set_error(L, "f", 0.0f);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR && f >= 0.0f)
		sso->ss->awacs_intensity = f;

	return ade_set_args(L, "f", sso->ss->awacs_intensity);
}

ADE_VIRTVAR(AWACSRadius, l_Subsystem, "number", "Subsystem AWACS radius", "number", "AWACS radius, or 0 if handle is invalid")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!ade_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return ade_set_error(L, "f", 0.0f);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR && f >= 0.0f)
		sso->ss->awacs_radius = f;

	return ade_set_args(L, "f", sso->ss->awacs_radius);
}

ADE_VIRTVAR(Orientation, l_Subsystem, "orientation", "Orientation of subobject or turret base", "orientation", "Subsystem orientation, or null orientation if handle is invalid")
{
	ship_subsys_h *sso;
	matrix_h *mh = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	auto smi = sso->ss->submodel_instance_1;
	if (smi == nullptr)
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if(ADE_SETTING_VAR && mh)
	{
		smi->canonical_prev_orient = smi->canonical_orient;
		smi->canonical_orient = *mh->GetMatrix();
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&smi->canonical_orient)));
}

ADE_VIRTVAR(GunOrientation, l_Subsystem, "orientation", "Orientation of turret gun", "orientation", "Gun orientation, or null orientation if handle is invalid")
{
	ship_subsys_h *sso;
	matrix_h *mh = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	auto smi = sso->ss->submodel_instance_2;
	if (smi == nullptr)
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if(ADE_SETTING_VAR && mh)
	{
		smi->canonical_prev_orient = smi->canonical_orient;
		smi->canonical_orient = *mh->GetMatrix();
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&smi->canonical_orient)));
}

ADE_VIRTVAR(HitpointsLeft, l_Subsystem, "number", "Subsystem hitpoints left", "number", "Hitpoints left, or 0 if handle is invalid. Setting a value of 0 will disable it - set a value of -1 or lower to actually blow it up.")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!ade_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return ade_set_error(L, "f", 0.0f);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR)
	{
		//Only go down to 0 hits
		sso->ss->current_hits = MAX(0.0f, f);

		ship *shipp = &Ships[sso->objp->instance];
		if (f <= -1.0f && sso->ss->current_hits <= 0.0f) {
			do_subobj_destroyed_stuff(shipp, sso->ss, NULL);
		}
		ship_recalc_subsys_strength(shipp);
	}

	return ade_set_args(L, "f", sso->ss->current_hits);
}

ADE_VIRTVAR(HitpointsMax, l_Subsystem, "number", "Subsystem hitpoints max", "number", "Max hitpoints, or 0 if handle is invalid")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!ade_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return ade_set_error(L, "f", 0.0f);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR)
	{
		sso->ss->max_hits = MIN(0.0f, f);

		ship_recalc_subsys_strength(&Ships[sso->objp->instance]);
	}

	return ade_set_args(L, "f", sso->ss->max_hits);
}

ADE_VIRTVAR(Position, l_Subsystem, "vector", "Subsystem position with regards to main ship (Local Vector)", "vector", "Subsystem position, or null vector if subsystem handle is invalid")
{
	ship_subsys_h *sso;
	vec3d *v = NULL;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v != NULL)
	{
		sso->ss->system_info->pnt = *v;
	}

	return ade_set_args(L, "o", l_Vector.Set(sso->ss->system_info->pnt));
}

ADE_VIRTVAR(WorldPosition, l_Subsystem, "vector",
            "Subsystem position in world space. This handles subsystem attached to a rotating submodel properly.",
            "vector", "Subsystem position, or null vector if subsystem handle is invalid")
{
	ship_subsys_h* sso;
	vec3d* v = nullptr;
	if (!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	vec3d world_pos;
	get_subsystem_world_pos(sso->objp, sso->ss, &world_pos);

	return ade_set_args(L, "o", l_Vector.Set(world_pos));
}

ADE_VIRTVAR(GunPosition, l_Subsystem, "vector", "Subsystem gun position with regards to main ship (Local vector)", "vector", "Gun position, or null vector if subsystem handle is invalid")
{
	ship_subsys_h *sso;
	vec3d *v = NULL;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	polymodel *pm = model_get(Ship_info[Ships[sso->objp->instance].ship_info_index].model_num);
	Assert(pm != NULL);

	if(sso->ss->system_info->turret_gun_sobj < 0)
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	bsp_info *sm = &pm->submodel[sso->ss->system_info->turret_gun_sobj];

	if(ADE_SETTING_VAR && v != NULL)
		sm->offset = *v;

	return ade_set_args(L, "o", l_Vector.Set(sm->offset));
}

ADE_VIRTVAR(Name, l_Subsystem, "string", "Subsystem name", "string", "Subsystem name, or an empty string if handle is invalid")
{
	ship_subsys_h *sso;
	const char* s = nullptr;
	if(!ade_get_args(L, "o|s", l_Subsystem.GetPtr(&sso), &s))
		return ade_set_error(L, "s", "");

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "s", "");

	if(ADE_SETTING_VAR && s != NULL && strlen(s))
	{
		ship_subsys_set_name(sso->ss, s);
	}

	return ade_set_args(L, "s", ship_subsys_get_name(sso->ss));
}

ADE_FUNC(getModelName, l_Subsystem, NULL, "Returns the original name of the subsystem in the model file", "string", "name or empty string on error")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "s", "");

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", sso->ss->system_info->subobj_name);
}

ADE_VIRTVAR(PrimaryBanks, l_Subsystem, "weaponbanktype", "Array of primary weapon banks", "weaponbanktype", "Primary banks, or invalid weaponbanktype handle if subsystem handle is invalid")
{
	ship_subsys_h *sso, *sso2 = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Subsystem.GetPtr(&sso2)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &sso->ss->weapons;

	if (ADE_SETTING_VAR && sso2 && sso2->isSubsystemValid()) {
		ship_weapon *src = &sso2->ss->weapons;

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

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(sso->objp, dst, SWH_PRIMARY)));
}

ADE_VIRTVAR(SecondaryBanks, l_Subsystem, "weaponbanktype", "Array of secondary weapon banks", "weaponbanktype", "Secondary banks, or invalid weaponbanktype handle if subsystem handle is invalid")
{
	ship_subsys_h *sso, *sso2 = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Subsystem.GetPtr(&sso2)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &sso->ss->weapons;

	if (ADE_SETTING_VAR && sso2 && sso2->isSubsystemValid()) {
		ship_weapon *src = &sso2->ss->weapons;

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

	return ade_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(sso->objp, dst, SWH_SECONDARY)));
}


ADE_VIRTVAR(Target, l_Subsystem, "object", "Object targeted by this subsystem. If used to set a new target, AI targeting will be switched off.", "object", "Targeted object, or invalid object handle if subsystem handle is invalid")
{
	ship_subsys_h *sso;
	object_h *objh = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Object.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	ship_subsys *ss = sso->ss;

	if(ADE_SETTING_VAR && objh && objh->IsValid())
	{
		ss->turret_enemy_objnum = OBJ_INDEX(objh->objp);
		ss->turret_enemy_sig = objh->sig;
		ss->targeted_subsys = NULL;
		ss->scripting_target_override = true;
	}

	return ade_set_object_with_breed(L, ss->turret_enemy_objnum);
}

ADE_VIRTVAR(TurretResets, l_Subsystem, "boolean", "Specifies wether this turrets resets after a certain time of inactivity", "boolean", "true if turret resets, false otherwise")
{
	ship_subsys_h *sso;
	bool newVal = false;
	if (!ade_get_args(L, "o|b", l_Subsystem.GetPtr(&sso), &newVal))
		return ADE_RETURN_FALSE;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_FALSE;

	if(ADE_SETTING_VAR)
	{
		sso->ss->system_info->flags.set(Model::Subsystem_Flags::Turret_reset_idle, newVal);
	}

	if (sso->ss->system_info->flags[Model::Subsystem_Flags::Turret_reset_idle])
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_VIRTVAR(TurretResetDelay, l_Subsystem, "number", "The time (in milliseconds) after that the turret resets itself", "number", "Reset delay")
{
	ship_subsys_h *sso;
	int newVal = -1;
	if (!ade_get_args(L, "o|i", l_Subsystem.GetPtr(&sso), &newVal))
		return ade_set_error(L, "i", -1);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "i", -1);

	if (!(sso->ss->system_info->flags[Model::Subsystem_Flags::Turret_reset_idle]))
		return ade_set_error(L, "i", -1);

	if(ADE_SETTING_VAR)
	{
		if ((sso->ss->system_info->flags[Model::Subsystem_Flags::Turret_reset_idle]))
			sso->ss->system_info->turret_reset_delay = newVal;
	}

	return ade_set_args(L, "i", sso->ss->system_info->turret_reset_delay);
}

ADE_VIRTVAR(TurnRate, l_Subsystem, "number", "The turn rate", "number", "Turnrate or -1 on error")
{
	ship_subsys_h *sso;
	float newVal = -1.0f;
	if (!ade_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &newVal))
		return ade_set_error(L, "f", -1.0f);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "f", -1.0f);

	if(ADE_SETTING_VAR)
	{
		sso->ss->system_info->turret_turning_rate = newVal;
	}

	return ade_set_args(L, "f", sso->ss->system_info->turret_turning_rate);
}

ADE_VIRTVAR(Targetable, l_Subsystem, "boolean", "Targetability of this subsystem", "boolean", "true if targetable, false otherwise or on error")
{
	ship_subsys_h *sso;
	bool newVal = false;
	if (!ade_get_args(L, "o|b", l_Subsystem.GetPtr(&sso), &newVal))
		return ade_set_error(L, "b", false);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "b", false);

	if(ADE_SETTING_VAR)
	{
		sso->ss->flags.set(Ship::Subsystem_Flags::Untargetable, newVal);
	}

	return ade_set_args(L, "b", !(sso->ss->flags[Ship::Subsystem_Flags::Untargetable]));
}

ADE_VIRTVAR(Radius, l_Subsystem, "number", "The radius of this subsystem", "number", "The radius or 0 on error")
{
	ship_subsys_h *sso;
	if (!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "f", 0.0f);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR)
	{
		LuaError(L, "Setting radius for subsystems is not allowed!");
	}

	return ade_set_args(L, "f", sso->ss->system_info->radius);
}

ADE_VIRTVAR(TurretLocked, l_Subsystem, "boolean", "Whether the turret is locked. Setting to true locks the turret, setting to false frees it.", "boolean", "True if turret is locked, false otherwise")
{
	ship_subsys_h *sso;
	bool newVal = false;
	if (!ade_get_args(L, "o|b", l_Subsystem.GetPtr(&sso), &newVal))
		return ade_set_error(L, "b", false);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "b", false);

	if(ADE_SETTING_VAR)
	{
		sso->ss->weapons.flags.set(Ship::Weapon_Flags::Turret_Lock, newVal);
	}

	return ade_set_args(L, "b", (sso->ss->weapons.flags[Ship::Weapon_Flags::Turret_Lock]));
}

ADE_VIRTVAR(NextFireTimestamp, l_Subsystem, "number", "The next time the turret may attempt to fire", "number", "Mission time (seconds) or -1 on error")
{
	ship_subsys_h *sso;
	float newVal = -1.0f;
	if (!ade_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &newVal))
		return ade_set_error(L, "f", -1.0f);

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "f", -1.0f);

	if(ADE_SETTING_VAR)
	{
		sso->ss->turret_next_fire_stamp = (int)(newVal * 1000);
	}

	return ade_set_args(L, "f", sso->ss->turret_next_fire_stamp / 1000.0f);
}

ADE_VIRTVAR(ModelPath, l_Subsystem, "modelpath", "The model path points belonging to this subsystem", "modelpath",
            "The model path of this subsystem")
{
	ship_subsys_h* sso;
	if (!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "o", l_ModelPath.Set(model_path_h()));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_ModelPath.Set(model_path_h()));

	if (ADE_SETTING_VAR) {
		LuaError(L, "Setting ModelPath is not supported yet!");
	}

	auto model    = model_get(sso->ss->system_info->model_num);
	auto path_idx = sso->ss->system_info->path_num;

	if (path_idx < 0) {
		// No model path for this subsystem
		return ade_set_args(L, "o", l_ModelPath.Set(model_path_h()));
	}

	Assertion(path_idx >= 0 && path_idx < model->n_paths, "Path index %d is invalid!", path_idx);

	return ade_set_args(L, "o", l_ModelPath.Set(model_path_h(*sso, model->paths[path_idx])));
}

ADE_FUNC(targetingOverride, l_Subsystem, "boolean", "If set to true, AI targeting for this turret is switched off. If set to false, the AI will take over again.", "boolean", "Returns true if successful, false otherwise")
{
	bool targetOverride = false;
	ship_subsys_h *sso;
	if(!ade_get_args(L, "ob", l_Subsystem.GetPtr(&sso), &targetOverride))
		return ADE_RETURN_FALSE;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_FALSE;

	ship_subsys *ss = sso->ss;

	ss->scripting_target_override = targetOverride;
	return ADE_RETURN_TRUE;
}

ADE_FUNC(hasFired, l_Subsystem, NULL, "Determine if a subsystem has fired", "boolean", "true if if fired, false if not fired, or nil if invalid. resets fired flag when called.")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_NIL;

	if(sso->ss->flags[Ship::Subsystem_Flags::Has_fired]){
		sso->ss->flags.remove(Ship::Subsystem_Flags::Has_fired);
		return ADE_RETURN_TRUE;
	}
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(isTurret, l_Subsystem, NULL, "Determines if this subsystem is a turret", "boolean", "true if subsystem is turret, false otherwise or nil on error")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sso->ss->system_info->type == SUBSYSTEM_TURRET);
}

ADE_FUNC(isTargetInFOV, l_Subsystem, "object Target", "Determines if the object is in the turrets FOV", "boolean", "true if in FOV, false if not, nil on error or if subsystem is not a turret ")
{
	ship_subsys_h *sso;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Object.GetPtr(&newh)))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid() || !newh || !newh->IsValid() || !(sso->ss->system_info->type == SUBSYSTEM_TURRET))
		return ADE_RETURN_NIL;

	vec3d	tpos,tvec;
	ship_get_global_turret_info(sso->objp, sso->ss->system_info, &tpos, &tvec);

	int in_fov = object_in_turret_fov(newh->objp,sso->ss,&tvec,&tpos,vm_vec_dist(&newh->objp->pos,&tpos));

	if (in_fov)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(isPositionInFOV, l_Subsystem, "vector Target", "Determines if a position is in the turrets FOV", "boolean", "true if in FOV, false if not, nil on error or if subsystem is not a turret ")
{
	ship_subsys_h* sso;
	vec3d target = vmd_zero_vector;
	if (!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Vector.Get(&target)))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid() || !(sso->ss->system_info->type == SUBSYSTEM_TURRET))
		return ADE_RETURN_NIL;

	vec3d tpos, tvec;
	ship_get_global_turret_info(sso->objp, sso->ss->system_info, &tpos, &tvec);

	vec3d v2e;
	vm_vec_normalized_dir(&v2e, &target, &tpos);
	bool in_fov = turret_fov_test(sso->ss, &tvec, &v2e);

	if (in_fov)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(fireWeapon, l_Subsystem, "[number TurretWeaponIndex = 1, number FlakRange = 100]", "Fires weapon on turret", nullptr, nullptr)
{
	ship_subsys_h *sso;
	int wnum = 1;
	float flak_range = 100.0f;
	if(!ade_get_args(L, "o|if", l_Subsystem.GetPtr(&sso), &wnum, &flak_range))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_NIL;

	// no place to fire a weapon; this may not actually be a turret
	if (sso->ss->system_info->turret_num_firing_points <= 0)
	{
		return ADE_RETURN_NIL;
	}

	if (sso->ss->current_hits <= 0)
	{
		return ADE_RETURN_FALSE;
	}

	wnum--;	//Lua->FS2

	//Get default turret info
	vec3d gpos, gvec;

	ship_get_global_turret_gun_info(sso->objp, sso->ss, &gpos, &gvec, 1, NULL);

	bool rtn = turret_fire_weapon(wnum, sso->ss, OBJ_INDEX(sso->objp), &gpos, &gvec, NULL, flak_range);

	sso->ss->turret_next_fire_pos++;

	return ade_set_args(L, "b", rtn);
}

ADE_FUNC(rotateTurret, l_Subsystem, "vector Pos, boolean reset=false", "Rotates the turret to face Pos or resets the turret to its original state", "boolean", "true on success false otherwise")
{
	ship_subsys_h *sso;
	vec3d pos = vmd_zero_vector;
	bool reset = false;
	if (!ade_get_args(L, "oo|b", l_Subsystem.GetPtr(&sso), l_Vector.Get(&pos), &reset))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_NIL;

	if (sso->ss->submodel_instance_1 == nullptr || sso->ss->submodel_instance_2 == nullptr)
		return ADE_RETURN_NIL;

	//Get default turret info
	object *objp = sso->objp;

	auto pmi = model_get_instance(Ships[objp->instance].model_instance_num);
	auto pm = model_get(pmi->model_num);

	int ret_val = model_rotate_gun(objp, pm, pmi, sso->ss, &pos, reset);

	if (ret_val)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(getTurretHeading, l_Subsystem, NULL, "Returns the turrets forward vector", "vector", "Returns a normalized version of the forward vector or null vector on error")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	vec3d gvec;
	object *objp = sso->objp;

	auto pmi = model_get_instance(Ships[objp->instance].model_instance_num);
	auto pm = model_get(pmi->model_num);

	model_instance_local_to_global_dir(&gvec, &sso->ss->system_info->turret_norm, pm, pmi, sso->ss->system_info->turret_gun_sobj, &objp->orient);

	vec3d out;
	vm_vec_rotate(&out, &gvec, &objp->orient);

	return ade_set_args(L, "o", l_Vector.Set(out));
}

ADE_FUNC(getFOVs, l_Subsystem, nullptr, "Returns current turrets FOVs", "number, number, number",
         "Standard FOV, maximum barrel elevation, turret base fov.")
{
	ship_subsys_h *sso;
	float fov, fov_e, fov_y;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_NIL;

	model_subsystem *tp = sso->ss->system_info;

	fov = tp->turret_fov;
	fov_e = tp->turret_max_fov;
	fov_y = tp->turret_base_fov;

	return ade_set_args(L, "fff", fov, fov_e, fov_y);
}

ADE_FUNC(
    getNextFiringPosition, l_Subsystem, nullptr,
    "Retrieves the next position and firing normal this turret will fire from. This function returns a world position",
    "vector, vector", "vector or null vector on error")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Vector.Set(vmd_zero_vector));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Vector.Set(vmd_zero_vector));

	vec3d gpos, gvec;

	ship_get_global_turret_gun_info(sso->objp, sso->ss, &gpos, &gvec, 1, NULL);

	return ade_set_args(L, "oo", l_Vector.Set(gpos), l_Vector.Set(gvec));
}

ADE_FUNC(getTurretMatrix, l_Subsystem, nullptr, "Returns current subsystems turret matrix", "orientation", "Turret matrix.")
{
	ship_subsys_h *sso;
	matrix m;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_NIL;

	model_subsystem *tp = sso->ss->system_info;

	// we have to fake a turret matrix because that field is no longer part of model_subsystem
	vm_vector_2_matrix(&m, &tp->turret_norm, nullptr, nullptr);

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&m)));
}

ADE_FUNC(getParent, l_Subsystem, NULL, "The object parent of this subsystem, is of type ship", "object", "object handle or invalid handle on error")
{
	ship_subsys_h *sso = NULL;
	object_h *objhp = NULL;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Ship.GetPtr(&objhp)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if (!sso->isSubsystemValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	return ade_set_object_with_breed(L, OBJ_INDEX(sso->objp));
}

ADE_FUNC(isInViewFrom, l_Subsystem, "vector from",
         "Checks if the subsystem is in view from the specified position. This only checks for occlusion by the parent "
         "object, not by other objects in the mission.",
         "boolean", "true if in view, false otherwise")
{
	ship_subsys_h *sso = nullptr;
	vec3d *from = nullptr;
	if(!ade_get_args(L, "oo", l_Subsystem.GetPtr(&sso), l_Vector.GetPtr(&from)))
		return ADE_RETURN_FALSE;

	if (!sso->isSubsystemValid())
		return ADE_RETURN_FALSE;

	vec3d world_pos;
	get_subsystem_world_pos(sso->objp, sso->ss, &world_pos);

	// Disable facing check since the HUD code does the same
	int in_sight = ship_subsystem_in_sight(sso->objp, sso->ss, from, &world_pos, 0);

	return ade_set_args(L, "b", in_sight != 0);
}

ADE_FUNC(isValid, l_Subsystem, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sso->isSubsystemValid());
}

}
}

