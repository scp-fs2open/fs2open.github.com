//
//

#include "iff_defs/iff_defs.h"
#include "weapon/beam.h"

#include "beam.h"
#include "mc_info.h"
#include "object.h"
#include "subsystem.h"
#include "team.h"
#include "vecmath.h"
#include "weaponclass.h"

namespace scripting {
namespace api {


//**********HANDLE: Beam
ADE_OBJ_DERIV(l_Beam, object_h, "beam", "Beam handle", l_Object);

ADE_VIRTVAR(Class, l_Beam, "weaponclass", "Weapon's class", "weaponclass", "Weapon class, or invalid weaponclass handle if beam handle is invalid")
{
	object_h *oh=NULL;
	int nc=-1;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&oh), l_Weaponclass.Get(&nc)))
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Weaponclass.Set(-1));

	beam *bp = &Beams[oh->objp->instance];

	if(ADE_SETTING_VAR && nc > -1) {
		bp->weapon_info_index = nc;
	}

	return ade_set_args(L, "o", l_Weaponclass.Set(bp->weapon_info_index));
}

ADE_VIRTVAR(LastShot, l_Beam, "vector", "End point of the beam", "vector", "vector or null vector if beam handle is not valid")
{
	object_h *oh=NULL;
	vec3d *vec3 = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&oh), l_Vector.GetPtr(&vec3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = &Beams[oh->objp->instance];

	if(ADE_SETTING_VAR && vec3) {
		bp->last_shot = *vec3;
	}

	return ade_set_args(L, "o", l_Vector.Set(bp->last_shot));
}

ADE_VIRTVAR(LastStart, l_Beam, "vector", "Start point of the beam", "vector", "vector or null vector if beam handle is not valid")
{
	object_h *oh=NULL;
	vec3d *v3 = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&oh), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!oh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = &Beams[oh->objp->instance];

	if(ADE_SETTING_VAR && v3) {
		bp->last_start = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(bp->last_start));
}

ADE_VIRTVAR(Target, l_Beam, "object", "Target of beam. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Beam target, or invalid object handle if beam handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(bp->target_sig != newh->sig)
			{
				bp->target = newh->objp;
				bp->target_sig = newh->sig;
			}
		}
		else
		{
			bp->target = NULL;
			bp->target_sig = 0;
		}
	}

	return ade_set_object_with_breed(L, OBJ_INDEX(bp->target));
}

ADE_VIRTVAR(TargetSubsystem, l_Beam, "subsystem", "Subsystem that beam is targeting.", "subsystem", "Target subsystem, or invalid subsystem handle if beam handle is invalid")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isSubsystemValid())
		{
			if(bp->target_sig != newh->sig)
			{
				bp->target = newh->objp;
				bp->target_subsys = newh->ss;
				bp->target_sig = newh->sig;
			}
		}
		else
		{
			bp->target = NULL;
			bp->target_subsys = NULL;
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(bp->target, bp->target_subsys)));
}

ADE_VIRTVAR(ParentShip, l_Beam, "object", "Parent of the beam.", "object", "Beam parent, or invalid object handle if beam handle is invalid")
{
	object_h *objh;
	object_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->IsValid())
		{
			if(bp->sig != newh->sig)
			{
				bp->objp = newh->objp;
				bp->sig = newh->sig;
			}
		}
		else
		{
			bp->objp = NULL;
			bp->sig = 0;
		}
	}

	return ade_set_object_with_breed(L, OBJ_INDEX(bp->objp));
}

ADE_VIRTVAR(ParentSubsystem, l_Beam, "subsystem", "Subsystem that beam is fired from.", "subsystem", "Parent subsystem, or invalid subsystem handle if beam handle is invalid")
{
	object_h *objh;
	ship_subsys_h *newh = nullptr;
	if(!ade_get_args(L, "o|o", l_Beam.GetPtr(&objh), l_Subsystem.GetPtr(&newh)))
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Subsystem.Set(ship_subsys_h()));

	if(ADE_SETTING_VAR)
	{
		if(newh && newh->isSubsystemValid())
		{
			if(bp->sig != newh->sig)
			{
				bp->objp = newh->objp;
				bp->subsys = newh->ss;
			}
		}
		else
		{
			bp->objp = NULL;
			bp->subsys = NULL;
		}
	}

	return ade_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(bp->objp, bp->subsys)));
}

ADE_VIRTVAR(Team, l_Beam, "team", "Beam's team", "team", "Beam team, or invalid team handle if beam handle is invalid")
{
	object_h* oh = nullptr;
	int nt = -1;
	if (!ade_get_args(L, "o|o", l_Beam.GetPtr(&oh), l_Team.Get(&nt)))
		return ade_set_error(L, "o", l_Team.Set(-1));

	if (!oh->IsValid())
		return ade_set_error(L, "o", l_Team.Set(-1));

	beam* b = &Beams[oh->objp->instance];

	if (ADE_SETTING_VAR && nt >= 0 && nt < (int)Iff_info.size())
		b->team = (char)nt;

	return ade_set_args(L, "o", l_Team.Set(b->team));
}

ADE_FUNC(getCollisionCount, l_Beam, NULL, "Get the number of collisions in frame.", "number", "Number of beam collisions")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Beam.GetPtr(&objh)))
		return ADE_RETURN_NIL;

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", bp->f_collision_count);
}

ADE_FUNC(getCollisionPosition, l_Beam, "number", "Get the position of the defined collision.", "vector", "World vector")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	// so we have valid beam and valid indexer
	return ade_set_args(L, "o", l_Vector.Set(bp->f_collisions[idx].cinfo.hit_point_world));
}

ADE_FUNC(getCollisionInformation, l_Beam, "number", "Get the collision information of the specified collision",
         "collision_info", "handle to information or invalid handle on error")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ade_set_error(L, "o", l_ColInfo.Set(mc_info_h()));

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ade_set_error(L, "o", l_ColInfo.Set(mc_info_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_ColInfo.Set(mc_info_h()));

	// so we have valid beam and valid indexer
	return ade_set_args(L, "o", l_ColInfo.Set(mc_info_h(bp->f_collisions[idx].cinfo)));
}

ADE_FUNC(getCollisionObject, l_Beam, "number", "Get the target of the defined collision.", "object", "Object the beam collided with")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	// so we have valid beam and valid indexer
	return ade_set_object_with_breed(L, bp->f_collisions[idx].c_objnum);
}

ADE_FUNC(isExitCollision, l_Beam, "number", "Checks if the defined collision was exit collision.", "boolean", "True if the collision was exit collision, false if entry, nil otherwise")
{
	object_h *objh;
	int idx;
	if(!ade_get_args(L, "oi", l_Beam.GetPtr(&objh), &idx))
		return ADE_RETURN_NIL;

	// convert from Lua to C
	idx--;
	if ((idx >= MAX_FRAME_COLLISIONS) || (idx < 0))
		return ADE_RETURN_NIL;

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ADE_RETURN_NIL;

	// so we have valid beam and valid indexer
	if (bp->f_collisions[idx].is_exit_collision)
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_FUNC(getStartDirectionInfo, l_Beam, NULL, "Gets the start information about the direction. The vector is a normalized vector from LastStart showing the start direction of a slashing beam", "vector", "The start direction or null vector if invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Beam.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam_info inf = bp->binfo;

	return ade_set_args(L, "o", l_Vector.Set(inf.dir_a));
}

ADE_FUNC(getEndDirectionInfo, l_Beam, NULL, "Gets the end information about the direction. The vector is a normalized vector from LastStart showing the end direction of a slashing beam", "vector", "The start direction or null vector if invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Beam.GetPtr(&objh)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam *bp = NULL;
	if(objh->objp->instance > -1)
		bp = &Beams[objh->objp->instance];
	else
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	beam_info inf = bp->binfo;

	return ade_set_args(L, "o", l_Vector.Set(inf.dir_b));
}


}
}
