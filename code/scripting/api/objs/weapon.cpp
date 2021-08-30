//
//

#include <weapon/weapon.h>
#include "weapon.h"
#include "object.h"
#include "weaponclass.h"
#include "subsystem.h"
#include "vecmath.h"
#include "team.h"
#include "mc_info.h"
#include "iff_defs/iff_defs.h"

namespace scripting {
namespace api {

//**********HANDLE: Weapon
ADE_OBJ_DERIV(l_Weapon, object_h, "weapon", "Weapon handle", l_Object);

ADE_VIRTVAR(Class, l_Weapon, "weaponclass", "Weapon's class", "weaponclass", "Weapon class, or invalid weaponclass handle if weapon handle is invalid")
{
	object_h *oh=NULL;
	int nc=-1;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Weaponclass.Get(&nc)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nc > -1) {
		wp->weapon_info_index = nc;
	}

	return ade_set_args(L, "o", l_Weaponclass.Set(wp->weapon_info_index));
}

ADE_VIRTVAR(DestroyedByWeapon, l_Weapon, "boolean", "Whether weapon was destroyed by another weapon", "boolean", "True if weapon was destroyed by another weapon, false if weapon was destroyed by another object or if weapon handle is invalid")
{
	object_h *oh=NULL;
	bool b = false;

	int numargs = ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &b);

	if(!numargs)
		return ade_set_error(L, "b", false);

	if(!oh->IsValid())
		return ade_set_error(L, "b", false);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && numargs > 1) {
		wp->weapon_flags.set(Weapon::Weapon_Flags::Destroyed_by_weapon, b);
	}

	return ade_set_args(L, "b", wp->weapon_flags[Weapon::Weapon_Flags::Destroyed_by_weapon]);
}

ADE_VIRTVAR(LifeLeft, l_Weapon, "number", "Weapon life left (in seconds)", "number", "Life left (seconds) or 0 if weapon handle is invalid")
{
	object_h *oh=NULL;
	float nll = -1.0f;
	if(!ade_get_args(L, "o|f", l_Weapon.GetPtr(&oh), &nll))
		return ade_set_error(L, "f", 0.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nll >= 0.0f) {
		wp->lifeleft = nll;
	}

	return ade_set_args(L, "f", wp->lifeleft);
}

ADE_VIRTVAR(FlakDetonationRange, l_Weapon, "number", "Range at which flak will detonate (meters)", "number", "Detonation range (meters) or 0 if weapon handle is invalid")
{
	object_h *oh=NULL;
	float rng = -1.0f;
	if(!ade_get_args(L, "o|f", l_Weapon.GetPtr(&oh), &rng))
		return ade_set_error(L, "f", 0.0f);

	if(!oh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && rng >= 0.0f) {
		wp->det_range = rng;
	}

	return ade_set_args(L, "f", wp->det_range);
}

ADE_VIRTVAR(Target, l_Weapon, "object", "Target of weapon. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Weapon target, or invalid object handle if weapon handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(wp->target_sig != newh->sig)
			{
				weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, OBJ_INDEX(newh->objp), 1);
			}
		}
		else
		{
			weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, -1);
		}
	}

	return ade_set_object_with_breed(L, wp->target_num);
}

ADE_VIRTVAR(ParentTurret, l_Weapon, "subsystem", "Turret which fired this weapon.", "subsystem", "Turret subsystem handle, or an invalid handle if the weapon not fired from a turret")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isSubsystemValid())
		{
			if(wp->turret_subsys != newh->ss)
			{
				wp->turret_subsys = newh->ss;
			}
		}
		else
		{
			wp->turret_subsys = NULL;
		}
	}

	if(wp->turret_subsys == NULL)
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));
	else
		return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(&Objects[wp->turret_subsys->parent_objnum], wp->turret_subsys)));
}

ADE_VIRTVAR(HomingObject, l_Weapon, "object", "Object that weapon will home in on. Value may also be a deriviative of the 'object' class, such as 'ship'", "object", "Object that weapon is homing in on, or an invalid object handle if weapon is not homing or the weapon handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if (newh && newh->IsValid())
		{
			if (wp->target_sig != newh->sig)
			{
				weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, OBJ_INDEX(newh->objp), 1);
			}
		}
		else
		{
			weapon_set_tracking_info(OBJ_INDEX(objh->objp), objh->objp->parent, -1);
		}
	}

	if(wp->homing_object == &obj_used_list)
		return ade_set_args(L, "o", l_Object.Set(object_h()));
	else
		return ade_set_object_with_breed(L, OBJ_INDEX(wp->homing_object));
}

ADE_VIRTVAR(HomingPosition, l_Weapon, "vector", "Position that weapon will home in on (World vector), setting this without a homing object in place will not have any effect!",
			"vector", "Homing point, or null vector if weapon handle is invalid")
{
	object_h *objh;
	vec3d *v3 = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR)
	{
		if(v3)
		{
			wp->homing_pos = *v3;
		}
		else
		{
			wp->homing_pos = vmd_zero_vector;
		}

		// need to update the position for multiplayer.
		if (Game_mode & GM_MULTIPLAYER) {
			wp->weapon_flags.set(Weapon::Weapon_Flags::Multi_homing_update_needed);
		}
	}

	return ade_set_args(L, "o", l_Vector.Set(wp->homing_pos));
}

ADE_VIRTVAR(HomingSubsystem, l_Weapon, "subsystem", "Subsystem that weapon will home in on.", "subsystem", "Homing subsystem, or invalid subsystem handle if weapon is not homing or weapon handle is invalid")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	weapon *wp = NULL;
	if(objh->objp->instance > -1)
		wp = &Weapons[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isSubsystemValid())
		{
			if(wp->target_sig != newh->sig)
			{
				wp->homing_object = newh->objp;
				wp->homing_subsys = newh->ss;
				get_subsystem_pos(&wp->homing_pos, wp->homing_object, wp->homing_subsys);
			}
		}
		else
		{
			wp->homing_object = &obj_used_list;
			wp->homing_pos = vmd_zero_vector;
			wp->homing_subsys = NULL;
		}

		// need to update the position for multiplayer.
		if (Game_mode & GM_MULTIPLAYER) {
			wp->weapon_flags.set(Weapon::Weapon_Flags::Multi_homing_update_needed);
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(wp->homing_object, wp->homing_subsys)));
}

ADE_VIRTVAR(Team, l_Weapon, "team", "Weapon's team", "team", "Weapon team, or invalid team handle if weapon handle is invalid")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!ade_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Team.Get(&nt)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	weapon *wp = &Weapons[oh->objp->instance];

	if(ADE_SETTING_VAR && nt > -1 && nt < Iff_info.size()) {
		wp->team = nt;
	}

	return ade_set_args(L, "o", l_Team.Set(wp->team));
}

ADE_VIRTVAR(OverrideHoming, l_Weapon, "boolean",
            "Whether homing is overridden for this weapon. When homing is overridden then the engine will not update "
            "the homing position of the weapon which means that it can be handled by scripting.",
            "boolean", "true if homing is overridden")
{
	object_h* oh = nullptr;
	bool new_val = false;
	if (!ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &new_val))
		return ade_set_error(L, "b", false);

	if (!oh->IsValid())
		return ade_set_error(L, "b", false);

	weapon* wp = &Weapons[oh->objp->instance];

	if (ADE_SETTING_VAR) {
		wp->weapon_flags.set(Weapon::Weapon_Flags::Overridden_homing, new_val);
	}

	return ade_set_args(L, "b", wp->weapon_flags[Weapon::Weapon_Flags::Overridden_homing]);
}

ADE_FUNC(isArmed, l_Weapon, "[boolean HitTarget]", "Checks if the weapon is armed.", "boolean", "boolean value of the weapon arming status")
{
	object_h *oh = NULL;
	bool hit_target = false;
	if(!ade_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &hit_target))
		return ADE_RETURN_FALSE;

	if(!oh->IsValid())
		return ADE_RETURN_FALSE;

	weapon *wp = &Weapons[oh->objp->instance];

	if(weapon_armed(wp, hit_target))
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_FUNC(getCollisionInformation, l_Weapon, nullptr, "Returns the collision information for this weapon",
         "collision_info", "The collision information or invalid handle if none")
{
	object_h *oh=NULL;
	if(!ade_get_args(L, "o", l_Weapon.GetPtr(&oh)))
		return ADE_RETURN_NIL;

	if(!oh->IsValid())
		return ADE_RETURN_NIL;

	weapon *wp = &Weapons[oh->objp->instance];

	if (wp->collisionInfo != nullptr)
		return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h(*wp->collisionInfo)));
	else
		return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h()));
}

ADE_FUNC(vanish, l_Weapon, nullptr, "Vanishes this weapon from the mission.", "boolean", "True if the deletion was successful, false otherwise.")
{

	object_h* oh = nullptr;
	if (!ade_get_args(L, "o", l_Weapon.GetPtr(&oh)))
		return ade_set_error(L, "b", false);

	if (!oh->IsValid())
		return ade_set_error(L, "b", false);

	oh->objp->flags.set(Object::Object_Flags::Should_be_dead);

	return ade_set_args(L, "b", true);
}

}
}

