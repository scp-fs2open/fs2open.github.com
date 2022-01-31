//
//

#include "enums.h"
#include "mc_info.h"
#include "object.h"
#include "physics_info.h"
#include "shields.h"
#include "sound.h"
#include "subsystem.h"
#include "vecmath.h"

#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "object/objectshield.h"
#include "object/objectsnd.h"
#include "scripting/api/LuaEventCallback.h"
#include "scripting/lua/LuaFunction.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

namespace scripting {
namespace api {

//**********HANDLE: Object
ADE_OBJ(l_Object, object_h, "object", "Object handle");

//Helper function
//Returns 1 if object sig stored in idx exists, and stores Objects[] index in idx
//Returns 0 if object sig does not exist, and does not change idx

ADE_FUNC(__eq, l_Object, "object, object", "Checks whether two object handles are for the same object", "boolean", "True if equal, false if not or a handle is invalid")
{
	object_h *o1, *o2;
	if(!ade_get_args(L, "oo", l_Object.GetPtr(&o1), l_Object.GetPtr(&o2)))
		return ADE_RETURN_FALSE;

	if(!o1->IsValid() || !o2->IsValid())
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", o1->sig == o2->sig);
}

ADE_FUNC(__tostring, l_Object, NULL, "Returns name of object (if any)", "string", "Object name, or empty string if handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Object.GetPtr(&objh)))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	char buf[512];

	switch(objh->objp->type)
	{
		case OBJ_SHIP:
			sprintf(buf, "%s", Ships[objh->objp->instance].ship_name);
			break;
		case OBJ_WEAPON:
			sprintf(buf, "%s projectile", Weapon_info[Weapons[objh->objp->instance].weapon_info_index].get_display_name());
			break;
		default:
			sprintf(buf, "Object %d [%d]", OBJ_INDEX(objh->objp), objh->sig);
	}

	return ade_set_args(L, "s", buf);
}

ADE_VIRTVAR(Parent, l_Object, "object", "Parent of the object. Value may also be a deriviative of the 'object' class, such as 'ship'.", "object", "Parent handle, or invalid handle if object is invalid")
{
	object_h *objh;
	object_h *newparenth = NULL;
	if(!ade_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Object.GetPtr(&newparenth)))
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Object.Set(object_h()));

	if(ADE_SETTING_VAR)
	{
		if(newparenth != NULL && newparenth->IsValid())
		{
			objh->objp->parent = OBJ_INDEX(newparenth->objp);
			objh->objp->parent_sig = newparenth->sig;
			objh->objp->parent_type = newparenth->objp->type;
		}
		else
		{
			objh->objp->parent = -1;
			objh->objp->parent_sig = 0;
			objh->objp->parent_type = OBJ_NONE;
		}
	}

	if(objh->objp->parent > -1)
		return ade_set_object_with_breed(L, objh->objp->parent);
	else
		return ade_set_args(L, "o", l_Object.Set(object_h()));
}

ADE_VIRTVAR(Position, l_Object, "vector", "Object world position (World vector)", "vector", "World position, or null vector if handle is invalid")
{
	object_h *objh;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		objh->objp->pos = *v3;
		if (objh->objp->type == OBJ_WAYPOINT) {
			waypoint *wpt = find_waypoint_with_objnum(OBJ_INDEX(objh->objp));
			wpt->set_pos(v3);
		}
	}

	return ade_set_args(L, "o", l_Vector.Set(objh->objp->pos));
}

ADE_VIRTVAR(LastPosition, l_Object, "vector", "Object world position as of last frame (World vector)", "vector", "World position, or null vector if handle is invalid")
{
	object_h *objh;
	vec3d *v3=NULL;
	if(!ade_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(ADE_SETTING_VAR && v3 != NULL) {
		objh->objp->last_pos = *v3;
	}

	return ade_set_args(L, "o", l_Vector.Set(objh->objp->last_pos));
}

ADE_VIRTVAR(Orientation, l_Object, "orientation", "Object world orientation (World orientation)", "orientation", "Orientation, or null orientation if handle is invalid")
{
	object_h *objh;
	matrix_h *mh=NULL;
	if(!ade_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h(&vmd_identity_matrix)));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h(&vmd_identity_matrix)));

	if(ADE_SETTING_VAR && mh != NULL) {
		objh->objp->orient = *mh->GetMatrix();
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&objh->objp->orient)));
}

ADE_VIRTVAR(LastOrientation, l_Object, "orientation", "Object world orientation as of last frame (World orientation)", "orientation", "Orientation, or null orientation if handle is invalid")
{
	object_h *objh;
	matrix_h *mh=NULL;
	if(!ade_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h(&vmd_identity_matrix)));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h(&vmd_identity_matrix)));

	if(ADE_SETTING_VAR && mh != NULL) {
		objh->objp->last_orient = *mh->GetMatrix();
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&objh->objp->last_orient)));
}

ADE_VIRTVAR(Physics, l_Object, "physics", "Physics data used to move ship between frames", "physics", "Physics data, or invalid physics handle if object handle is invalid")
{
	object_h *objh;
	physics_info_h *pih = nullptr;
	if(!ade_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Physics.GetPtr(&pih)))
		return ade_set_error(L, "o", l_Physics.Set(physics_info_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Physics.Set(physics_info_h()));

	if(ADE_SETTING_VAR && pih && pih->IsValid()) {
		objh->objp->phys_info = *pih->pi;
	}

	return ade_set_args(L, "o", l_Physics.Set(physics_info_h(objh->objp)));
}

ADE_VIRTVAR(HitpointsLeft, l_Object, "number", "Hitpoints an object has left", "number", "Hitpoints left, or 0 if handle is invalid")
{
	object_h *objh = nullptr;
	float f = -1.0f;
	if(!ade_get_args(L, "o|f", l_Object.GetPtr(&objh), &f))
		return ade_set_error(L, "f", 0.0f);

	if(!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	//Set hull strength.
	if(ADE_SETTING_VAR) {
		objh->objp->hull_strength = f;
	}

	return ade_set_args(L, "f", objh->objp->hull_strength);
}

ADE_VIRTVAR(SimHitpointsLeft, l_Object, "number", "Simulated hitpoints an object has left", "number", "Simulated hitpoints left, or 0 if handle is invalid")
{
	object_h *objh = nullptr;
	float f = -1.0f;
	if (!ade_get_args(L, "o|f", l_Object.GetPtr(&objh), &f))
		return ade_set_error(L, "f", 0.0f);

	if (!objh->IsValid())
		return ade_set_error(L, "f", 0.0f);

	//Set sim hull strength.
	if (ADE_SETTING_VAR) {
		objh->objp->sim_hull_strength = f;
	}

	return ade_set_args(L, "f", objh->objp->sim_hull_strength);
}

ADE_VIRTVAR(Shields, l_Object, "shields", "Shields", "shields", "Shields handle, or invalid shields handle if object handle is invalid")
{
	object_h *objh;
	object_h *sobjh = nullptr;
	if(!ade_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Shields.GetPtr(&sobjh)))
		return ade_set_error(L, "o", l_Shields.Set(object_h()));

	if(!objh->IsValid())
		return ade_set_error(L, "o", l_Shields.Set(object_h()));

	//WMC - copy shields
	if(ADE_SETTING_VAR && sobjh && sobjh->IsValid())
	{
		for(int i = 0; i < objh->objp->n_quadrants; i++)
			shield_set_quad(objh->objp, i, shield_get_quad(sobjh->objp, i));
	}

	return ade_set_args(L, "o", l_Shields.Set(object_h(objh->objp)));
}

ADE_FUNC(getSignature, l_Object, NULL, "Gets the object's unique signature", "number", "Returns the object's unique numeric signature, or -1 if invalid.  Useful for creating a metadata system")
{
	object_h *oh;
	if(!ade_get_args(L, "o", l_Object.GetPtr(&oh)))
		return ade_set_error(L, "i", -1);

	if(!oh->IsValid())
		return ade_set_error(L, "i", -1);

	return ade_set_args(L, "i", oh->sig);
}

ADE_FUNC(isValid, l_Object, NULL, "Detects whether handle is valid", "boolean", "true if handle is valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	object_h *oh;
	if(!ade_get_args(L, "o", l_Object.GetPtr(&oh)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", oh->IsValid());
}

ADE_FUNC(getBreedName, l_Object, NULL, "Gets object type", "string", "Object type name, or empty string if handle is invalid")
{
	object_h *objh;
	if(!ade_get_args(L, "o", l_Object.GetPtr(&objh)))
		return ade_set_error(L, "s", "");

	if(!objh->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", Object_type_names[objh->objp->type]);
}

ADE_VIRTVAR(CollisionGroups, l_Object, "number", "Collision group data", "number", "Current collision group signature. NOTE: This is a bitfield, NOT a normal number.")
{
	object_h *objh = NULL;
	int id = 0;
	if(!ade_get_args(L, "o|i", l_Object.GetPtr(&objh), &id))
		return ade_set_error(L, "i", 0);

	if(!objh->IsValid())
		return ade_set_error(L, "i", 0);

	//Set collision group data
	if(ADE_SETTING_VAR) {
		objh->objp->collision_group_id = id;
	}

	return ade_set_args(L, "i", objh->objp->collision_group_id);
}

ADE_FUNC(addToCollisionGroup, l_Object, "number group", "Adds this object to the specified collision group.  The group must be between 0 and 31, inclusive.", nullptr, "Returns nothing")
{
	object_h *objh = nullptr;
	int group;

	if (!ade_get_args(L, "oi", l_Object.GetPtr(&objh), &group))
		return ADE_RETURN_NIL;

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	if (group >= 0 && group <= 31)
		objh->objp->collision_group_id |= (1 << group);

	return ADE_RETURN_NIL;
}

ADE_FUNC(removeFromCollisionGroup, l_Object, "number group", "Removes this object from the specified collision group.  The group must be between 0 and 31, inclusive.", nullptr, "Returns nothing")
{
	object_h *objh = nullptr;
	int group;

	if (!ade_get_args(L, "oi", l_Object.GetPtr(&objh), &group))
		return ADE_RETURN_NIL;

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	if (group >= 0 && group <= 31)
		objh->objp->collision_group_id &= ~(1 << group);

	return ADE_RETURN_NIL;
}

ADE_FUNC(getfvec, l_Object, "[boolean normalize]", "Returns the objects' current fvec.", "vector", "Objects' forward vector, or nil if invalid. If called with a true argument, vector will be normalized.")
{
	object_h *objh = NULL;
	object *obj = NULL;
	bool normalize = false;

	if (!ade_get_args(L, "o|b", l_Object.GetPtr(&objh), &normalize)) {
		return ADE_RETURN_NIL;
	}

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	obj = objh->objp;
	vec3d v1 = obj->orient.vec.fvec;
	if (normalize)
		vm_vec_normalize(&v1);

	return ade_set_args(L, "o", l_Vector.Set(v1));
}

ADE_FUNC(getuvec, l_Object, "[boolean normalize]", "Returns the objects' current uvec.", "vector", "Objects' up vector, or nil if invalid. If called with a true argument, vector will be normalized.")
{
	object_h *objh = NULL;
	object *obj = NULL;
	bool normalize = false;

	if (!ade_get_args(L, "o|b", l_Object.GetPtr(&objh), &normalize)) {
		return ADE_RETURN_NIL;
	}

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	obj = objh->objp;
	vec3d v1 = obj->orient.vec.uvec;
	if (normalize)
		vm_vec_normalize(&v1);

	return ade_set_args(L, "o", l_Vector.Set(v1));
}

ADE_FUNC(getrvec, l_Object, "[boolean normalize]", "Returns the objects' current rvec.", "vector", "Objects' rvec, or nil if invalid. If called with a true argument, vector will be normalized.")
{
	object_h *objh = NULL;
	object *obj = NULL;
	bool normalize = false;

	if (!ade_get_args(L, "o|b", l_Object.GetPtr(&objh), &normalize)) {
		return ADE_RETURN_NIL;
	}

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	obj = objh->objp;
	vec3d v1 = obj->orient.vec.rvec;
	if (normalize)
		vm_vec_normalize(&v1);

	return ade_set_args(L, "o", l_Vector.Set(v1));
}

ADE_FUNC(
    checkRayCollision, l_Object, "vector StartPoint, vector EndPoint, [boolean Local=false]",
    "Checks the collisions between the polygons of the current object and a ray. Start and end vectors are in world "
    "coordinates",
    "vector, collision_info",
    "World collision point (local if boolean is set to true) and the specific collsision info, nil if no collisions")
{
	object_h *objh = NULL;
	object *obj = NULL;
	int model_num = -1, model_instance_num = -1, temp = 0;
	vec3d *v3a, *v3b;
	bool local = false;
	if(!ade_get_args(L, "ooo|b", l_Object.GetPtr(&objh), l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b), &local))
		return ADE_RETURN_NIL;

	if(!objh->IsValid())
		return ADE_RETURN_NIL;

	obj = objh->objp;
	int flags = 0;
	int submodel = -1;

	switch(obj->type) {
		case OBJ_SHIP:
			model_num = Ship_info[Ships[obj->instance].ship_info_index].model_num;
			flags = (MC_CHECK_MODEL | MC_CHECK_RAY);
			break;
		case OBJ_WEAPON:
			model_num = Weapon_info[Weapons[obj->instance].weapon_info_index].model_num;
			flags = (MC_CHECK_MODEL | MC_CHECK_RAY);
			break;
		case OBJ_DEBRIS:
			model_num = Debris[obj->instance].model_num;
			flags = (MC_CHECK_MODEL | MC_CHECK_RAY | MC_SUBMODEL);
			submodel = Debris[obj->instance].submodel_num;
			break;
		case OBJ_ASTEROID:
			temp = Asteroids[obj->instance].asteroid_subtype;
			model_num = Asteroid_info[Asteroids[obj->instance].asteroid_type].model_num[temp];
			flags = (MC_CHECK_MODEL | MC_CHECK_RAY);
			break;
		default:
			return ADE_RETURN_NIL;
	}

	if (model_num < 0)
		return ADE_RETURN_NIL;

	if (obj->type == OBJ_SHIP) {
		model_instance_num = Ships[obj->instance].model_instance_num;
	} else if (obj->type == OBJ_WEAPON) {
		model_instance_num = Weapons[obj->instance].model_instance_num;
	} else if (obj->type == OBJ_ASTEROID) {
		model_instance_num = Asteroids[obj->instance].model_instance_num;
	}

	mc_info hull_check;
	mc_info_init(&hull_check);

	hull_check.model_num = model_num;
	hull_check.model_instance_num = model_instance_num;
	hull_check.submodel_num = submodel;
	hull_check.orient = &obj->orient;
	hull_check.pos = &obj->pos;
	hull_check.p0 = v3a;
	hull_check.p1 = v3b;
	hull_check.flags = flags;

	if ( !model_collide(&hull_check) ) {
		return ADE_RETURN_NIL;
	}

	if (local)
		return ade_set_args(L, "oo", l_Vector.Set(hull_check.hit_point), l_ColInfo.Set(mc_info_h(hull_check)));
	else
		return ade_set_args(L, "oo", l_Vector.Set(hull_check.hit_point_world),  l_ColInfo.Set(mc_info_h(hull_check)));
}

ADE_FUNC(addPreMoveHook, l_Object, "function(object object) => void callback",
         "Registers a callback on this object which is called every time <i>before</i> the physics rules are applied "
         "to the object. The callback is attached to this specific object and will not be called anymore once the "
         "object is deleted. The parameter of the function is the object that is being moved.",
         nullptr, "Returns nothing.")
{
	object_h* objh = nullptr;
	luacpp::LuaFunction callback;
	if (!ade_get_args(L, "ou", l_Object.GetPtr(&objh), &callback)) {
		return ADE_RETURN_NIL;
	}

	if (!callback.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	objh->objp->pre_move_event.add(make_lua_callback<void, object*>(callback));

	return ADE_RETURN_NIL;
}

ADE_FUNC(addPostMoveHook, l_Object, "function(object object) => void callback",
         "Registers a callback on this object which is called every time <i>after</i> the physics rules are applied "
         "to the object. The callback is attached to this specific object and will not be called anymore once the "
         "object is deleted. The parameter of the function is the object that is being moved.",
         nullptr, "Returns nothing.")
{
	object_h* objh = nullptr;
	luacpp::LuaFunction callback;
	if (!ade_get_args(L, "ou", l_Object.GetPtr(&objh), &callback)) {
		return ADE_RETURN_NIL;
	}

	if (!callback.isValid()) {
		return ADE_RETURN_NIL;
	}

	if (!objh->IsValid())
		return ADE_RETURN_NIL;

	objh->objp->post_move_event.add(make_lua_callback<void, object*>(callback));

	return ADE_RETURN_NIL;
}

ADE_FUNC(assignSound, l_Object, "soundentry GameSnd, [vector Offset=nil, enumeration Flags=0, subsystem Subsys=nil]",
	"Assigns a sound to this object, with optional offset, sound flags (OS_XXXX), and associated subsystem.",
	"number",
	"Returns the index of the sound on this object, or -1 if a sound could not be assigned.")
{
	object_h* objh = nullptr;
	sound_entry_h *seh = nullptr;
	vec3d *offset = nullptr;
	enum_h enum_flags;
	int flags = 0;
	ship_subsys_h *tgsh = nullptr;

	if (!ade_get_args(L, "oo|ooo", l_Object.GetPtr(&objh), l_SoundEntry.GetPtr(&seh), l_Vector.GetPtr(&offset), l_Enum.Get(&enum_flags), l_Subsystem.GetPtr(&tgsh)))
		return ade_set_error(L, "i", -1);

	if (!objh->IsValid() || !seh->IsValid() || (tgsh && (!tgsh->IsValid() || !tgsh->isSubsystemValid())))
		return ade_set_error(L, "i", -1);

	auto objp = objh->objp;
	auto gs_id = seh->idx;
	auto subsys = tgsh ? tgsh->ss : nullptr;
	if (!offset)
		offset = &vmd_zero_vector;
	if (enum_flags.index >= 0)
		flags = enum_flags.index;

	int snd_idx = obj_snd_assign(OBJ_INDEX(objp), gs_id, offset, flags, subsys);

	return ade_set_args(L, "i", snd_idx);
}

ADE_FUNC(removeSoundByIndex, l_Object, "number index", "Removes an assigned sound from this object.", nullptr, "Returns nothing.")
{
	object_h* objh = nullptr;
	int snd_idx;

	if (!ade_get_args(L, "oi", l_Object.GetPtr(&objh), &snd_idx))
		return ADE_RETURN_NIL;

	auto objp = objh->objp;

	if (snd_idx < 0 || snd_idx >= (int)objp->objsnd_num.size())
	{
		LuaError(L, "Sound index is out of range for object %d!", OBJ_INDEX(objp));
		return ADE_RETURN_NIL;
	}

	obj_snd_delete(objp, snd_idx);

	return ADE_RETURN_NIL;
}

ADE_FUNC(removeSound, l_Object, "soundentry GameSnd, [subsystem Subsys=nil]",
	"Removes all sounds of the given type from the object or object's subsystem",
	nullptr,
	"Returns nothing.")
{
	object_h* objh = nullptr;
	sound_entry_h *seh = nullptr;
	ship_subsys_h *tgsh = nullptr;

	if (!ade_get_args(L, "oo|o", l_Object.GetPtr(&objh), l_SoundEntry.GetPtr(&seh), l_Subsystem.GetPtr(&tgsh)))
		return ADE_RETURN_NIL;

	if (!objh->IsValid() || !seh->IsValid() || (tgsh && (!tgsh->IsValid() || !tgsh->isSubsystemValid())))
		return ADE_RETURN_NIL;

	auto objp = objh->objp;
	auto gs_id = seh->idx;
	auto subsys = tgsh ? tgsh->ss : nullptr;

	obj_snd_delete_type(OBJ_INDEX(objp), gs_id, subsys);

	return ADE_RETURN_NIL;
}


ADE_FUNC(getIFFColor, l_Object, "number, number, number", 
	"Gets the IFF color of the object",
	"number, number, number", 
	"IFF rgb color of the object or nil if object invalid")
{
	object_h* objh;

	if (!ade_get_args(L, "o", l_Object.GetPtr(&objh)))
		return ade_set_error(L, "iii", ADE_RETURN_NIL, ADE_RETURN_NIL, ADE_RETURN_NIL);

	if (!objh->IsValid())
		return ade_set_error(L, "iii", ADE_RETURN_NIL, ADE_RETURN_NIL, ADE_RETURN_NIL);

	auto objp = objh->objp;
	color* col = hud_get_iff_color(objp);

	int r = col->red;
	int g = col->green;
	int b = col->blue;

	return ade_set_args(L, "iii", r, g, b);
}

} // namespace api
} // namespace scripting
