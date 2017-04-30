//
//

#include "subsystem.h"
#include "object.h"
#include "vecmath.h"
#include "ship_bank.h"
#include "ship.h"
#include "ship/shiphit.h"

bool turret_fire_weapon(int weapon_num, ship_subsys *turret, int parent_objnum, vec3d *turret_pos, vec3d *turret_fvec, vec3d *predicted_pos = NULL, float flak_range_override = 100.0f);

namespace scripting {
namespace api {

ship_subsys_h::ship_subsys_h() : object_h() {
	ss = NULL;
}
ship_subsys_h::ship_subsys_h(object* objp_in, ship_subsys* sub) : object_h(objp_in) {
	ss = sub;
}
bool ship_subsys_h::IsValid() {
	return object_h::IsValid() && objp->type == OBJ_SHIP && ss != NULL;
}

//**********HANDLE: Subsystem
ADE_OBJ(l_Subsystem, ship_subsys_h, "subsystem", "Ship subsystem handle");


ADE_FUNC(__tostring, l_Subsystem, NULL, "Returns name of subsystem", "string", "Subsystem name, or empty string if handle is invalid")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "s", "");

	if(!sso->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", ship_subsys_get_name(sso->ss));
}

ADE_VIRTVAR(ArmorClass, l_Subsystem, "string", "Current Armor class", "string", "Armor class name, or empty string if none is set")
{
	ship_subsys_h *sso;
	char *s = NULL;
	const char *name = NULL;

	if(!ade_get_args(L, "o|s", l_Subsystem.GetPtr(&sso), &s))
		return ade_set_error(L, "s", "");

	if(!sso->IsValid())
		return ade_set_error(L, "s", "");

	ship_subsys *ssys = sso->ss;

	int atindex = -1;
	if (ADE_SETTING_VAR && s != NULL) {
		atindex = armor_type_get_idx(s);
		ssys->armor_type_idx = atindex;
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

	if(!sso->IsValid())
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

	if(!sso->IsValid())
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

	if(!sso->IsValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if(ADE_SETTING_VAR && mh)
	{
		sso->ss->submodel_info_1.angs = *mh->GetAngles();
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&sso->ss->submodel_info_1.angs)));
}

ADE_VIRTVAR(GunOrientation, l_Subsystem, "orientation", "Orientation of turret gun", "orientation", "Gun orientation, or null orientation if handle is invalid")
{
	ship_subsys_h *sso;
	matrix_h *mh = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if(!sso->IsValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));

	if(ADE_SETTING_VAR && mh)
	{
		sso->ss->submodel_info_2.angs = *mh->GetAngles();
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&sso->ss->submodel_info_2.angs)));
}

ADE_VIRTVAR(HitpointsLeft, l_Subsystem, "number", "Subsystem hitpoints left", "number", "Hitpoints left, or 0 if handle is invalid. Setting a value of 0 will disable it - set a value of -1 or lower to actually blow it up.")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!ade_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!sso->IsValid())
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

	if(!sso->IsValid())
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

	if(!sso->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v != NULL)
	{
		sso->ss->system_info->pnt = *v;
	}

	return ade_set_args(L, "o", l_Vector.Set(sso->ss->system_info->pnt));
}

ADE_VIRTVAR(GunPosition, l_Subsystem, "vector", "Subsystem gun position with regards to main ship (Local vector)", "vector", "Gun position, or null vector if subsystem handle is invalid")
{
	ship_subsys_h *sso;
	vec3d *v = NULL;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!sso->IsValid())
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
	char *s = NULL;
	if(!ade_get_args(L, "o|s", l_Subsystem.GetPtr(&sso), &s))
		return ade_set_error(L, "s", "");

	if(!sso->IsValid())
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

	if(!sso->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", sso->ss->system_info->subobj_name);
}

ADE_VIRTVAR(PrimaryBanks, l_Subsystem, "weaponbanktype", "Array of primary weapon banks", "weaponbanktype", "Primary banks, or invalid weaponbanktype handle if subsystem handle is invalid")
{
	ship_subsys_h *sso, *sso2 = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Subsystem.GetPtr(&sso2)))
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	if(!sso->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &sso->ss->weapons;

	if(ADE_SETTING_VAR && sso2 && sso2->IsValid()) {
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

	if(!sso->IsValid())
		return ade_set_error(L, "o", l_WeaponBankType.Set(ship_banktype_h()));

	ship_weapon *dst = &sso->ss->weapons;

	if(ADE_SETTING_VAR && sso2 && sso2->IsValid()) {
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

	if(!sso->IsValid())
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

	if (!sso->IsValid())
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

	if (!sso->IsValid())
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

	if (!sso->IsValid())
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

	if (!sso->IsValid())
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

	if (!sso->IsValid())
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

	if (!sso->IsValid())
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

	if (!sso->IsValid())
		return ade_set_error(L, "f", -1.0f);

	if(ADE_SETTING_VAR)
	{
		sso->ss->turret_next_fire_stamp = (int)(newVal * 1000);
	}

	return ade_set_args(L, "f", sso->ss->turret_next_fire_stamp / 1000.0f);
}

ADE_FUNC(targetingOverride, l_Subsystem, "boolean", "If set to true, AI targeting for this turret is switched off. If set to false, the AI will take over again.", "boolean", "Returns true if successful, false otherwise")
{
	bool targetOverride = false;
	ship_subsys_h *sso;
	if(!ade_get_args(L, "ob", l_Subsystem.GetPtr(&sso), &targetOverride))
		return ADE_RETURN_FALSE;

	if(!sso->IsValid())
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

	if(!sso->IsValid())
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

	if (!sso->IsValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sso->ss->system_info->type == SUBSYSTEM_TURRET);
}

ADE_FUNC(isTargetInFOV, l_Subsystem, "object Target", "Determines if the object is in the turrets FOV", "boolean", "true if in FOV, false if not, nil on error or if subsystem is not a turret ")
{
	ship_subsys_h *sso;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Object.GetPtr(&newh)))
		return ADE_RETURN_NIL;

	if (!sso->IsValid() || !newh || !newh->IsValid() || !(sso->ss->system_info->type == SUBSYSTEM_TURRET))
		return ADE_RETURN_NIL;

	vec3d	tpos,tvec;
	ship_get_global_turret_info(sso->objp, sso->ss->system_info, &tpos, &tvec);

	int in_fov = object_in_turret_fov(newh->objp,sso->ss,&tvec,&tpos,vm_vec_dist(&newh->objp->pos,&tpos));

	if (in_fov)
		return ADE_RETURN_TRUE;
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(fireWeapon, l_Subsystem, "[Turret weapon index = 1, Flak range = 100]", "Fires weapon on turret", NULL, NULL)
{
	ship_subsys_h *sso;
	int wnum = 1;
	float flak_range = 100.0f;
	if(!ade_get_args(L, "o|if", l_Subsystem.GetPtr(&sso), &wnum, &flak_range))
		return ADE_RETURN_NIL;

	if(!sso->IsValid())
		return ADE_RETURN_NIL;

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

ADE_FUNC(rotateTurret, l_Subsystem, "vector Pos[, boolean reset=false", "Rotates the turret to face Pos or resets the turret to its original state", "boolean", "true on success false otherwise")
{
	ship_subsys_h *sso;
	vec3d pos = vmd_zero_vector;
	bool reset = false;
	if (!ade_get_args(L, "oo|b", l_Subsystem.GetPtr(&sso), l_Vector.Get(&pos), &reset))
		return ADE_RETURN_NIL;

	//Get default turret info
	vec3d gpos, gvec;
	model_subsystem *tp = sso->ss->system_info;
	object *objp = sso->objp;

	//Rotate turret position with ship
	vm_vec_unrotate(&gpos, &tp->pnt, &sso->objp->orient);

	//Add turret position to appropriate world space
	vm_vec_add2(&gpos, &sso->objp->pos);

	// Find direction of turret
	model_instance_find_world_dir(&gvec, &tp->turret_norm, Ships[objp->instance].model_instance_num, tp->turret_gun_sobj, &objp->orient);

	int ret_val = model_rotate_gun(Ship_info[(&Ships[sso->objp->instance])->ship_info_index].model_num, tp, &Objects[sso->objp->instance].orient, &sso->ss->submodel_info_1.angs, &sso->ss->submodel_info_2.angs, &Objects[sso->objp->instance].pos, &pos, (&Ships[sso->objp->instance])->objnum, reset);

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

	if(!sso->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	vec3d gvec;
	object *objp = sso->objp;

	model_instance_find_world_dir(&gvec, &sso->ss->system_info->turret_norm, Ships[objp->instance].model_instance_num, sso->ss->system_info->turret_gun_sobj, &objp->orient);

	vec3d out;
	vm_vec_rotate(&out, &gvec, &sso->objp->orient);

	return ade_set_args(L, "o", l_Vector.Set(out));
}

ADE_FUNC(getFOVs, l_Subsystem, NULL, "Returns current turrets FOVs", "number, number, number", "Standard FOV, maximum barrel elevation, turret base fov.")
{
	ship_subsys_h *sso;
	float fov, fov_e, fov_y;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	if(!sso->IsValid())
		return ADE_RETURN_NIL;

	model_subsystem *tp = sso->ss->system_info;

	fov = tp->turret_fov;
	fov_e = tp->turret_max_fov;
	fov_y = tp->turret_y_fov;

	return ade_set_args(L, "fff", fov, fov_e, fov_y);
}

ADE_FUNC(getNextFiringPosition, l_Subsystem, NULL, "Retrieves the next position and firing normal this turret will fire from. This function returns a world position", "vector, vector", "vector or null vector on error")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Vector.Set(vmd_zero_vector));

	if(!sso->IsValid())
		return ade_set_error(L, "oo", l_Vector.Set(vmd_zero_vector), l_Vector.Set(vmd_zero_vector));

	vec3d gpos, gvec;

	ship_get_global_turret_gun_info(sso->objp, sso->ss, &gpos, &gvec, 1, NULL);

	return ade_set_args(L, "oo", l_Vector.Set(gpos), l_Vector.Set(gvec));
}

ADE_FUNC(getTurretMatrix, l_Subsystem, NULL, "Returns current subsystems turret matrix", "matrix", "Turret matrix.")
{
	ship_subsys_h *sso;
	matrix m;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	if(!sso->IsValid())
		return ADE_RETURN_NIL;

	model_subsystem *tp = sso->ss->system_info;

	m = tp->turret_matrix;

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&m)));
}

ADE_FUNC(getParent, l_Subsystem, NULL, "The object parent of this subsystem, is of type ship", "object", "object handle or invalid handle on error")
{
	ship_subsys_h *sso = NULL;
	object_h *objhp = NULL;
	if(!ade_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Ship.GetPtr(&objhp)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!sso->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	return ade_set_args(L, "o", l_Object.Set(object_h(sso->objp)));
}

ADE_FUNC(isValid, l_Subsystem, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	ship_subsys_h *sso;
	if(!ade_get_args(L, "o", l_Subsystem.GetPtr(&sso)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sso->IsValid());
}

}
}

