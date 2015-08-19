/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "asteroid/asteroid.h"
#include "debris/debris.h"
#include "math/fvi.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "parse/scripting.h"
#include "weapon/weapon.h"



// placeholder struct for ship_debris collisions
typedef struct ship_weapon_debris_struct {
	object	*ship_object;
	object	*debris_object;
	vec3d	ship_collision_cm_pos;
	vec3d	r_ship;
	vec3d	collision_normal;
	int		shield_hit_tri;
	vec3d	shield_hit_tri_point;
	float		impulse;
} ship_weapon_debris_struct;


/**
 * Checks debris-weapon collisions.  
 * @param pair obj_pair pointer to the two objects. pair->a is debris and pair->b is weapon.
 * @return 1 if all future collisions between these can be ignored
 */
int collide_debris_weapon( obj_pair * pair )
{
	vec3d	hitpos;
	int		hit;
	object *pdebris = pair->a;
	object *weapon_obj = pair->b;

	Assert( pdebris->type == OBJ_DEBRIS );
	Assert( weapon_obj->type == OBJ_WEAPON );

	// first check the bounding spheres of the two objects.
	hit = fvi_segment_sphere(&hitpos, &weapon_obj->last_pos, &weapon_obj->pos, &pdebris->pos, pdebris->radius);
	if (hit) {
		hit = debris_check_collision(pdebris, weapon_obj, &hitpos );
		if ( !hit )
			return 0;

		Script_system.SetHookObjects(4, "Weapon", weapon_obj, "Debris", pdebris, "Self", weapon_obj, "Object", pdebris);
		bool weapon_override = Script_system.IsConditionOverride(CHA_COLLIDEDEBRIS, weapon_obj);

		Script_system.SetHookObjects(2, "Self", pdebris, "Object", weapon_obj);
		bool debris_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, pdebris);

		if(!weapon_override && !debris_override)
		{
			weapon_hit( weapon_obj, pdebris, &hitpos );
			debris_hit( pdebris, weapon_obj, &hitpos, Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].damage );
		}

		Script_system.SetHookObjects(2, "Self", weapon_obj, "Object", pdebris);
		if(!(debris_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEDEBRIS, '\0', NULL, weapon_obj);

		Script_system.SetHookObjects(2, "Self", pdebris, "Object", weapon_obj);
		if((debris_override && !weapon_override) || (!debris_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEWEAPON, '\0', NULL, pdebris, Weapons[weapon_obj->instance].weapon_info_index);

		Script_system.RemHookVars(4, "Weapon", "Debris", "Self", "Object");
		return 0;

	} else {
		return weapon_will_never_hit( weapon_obj, pdebris, pair );
	}
}				



/**
 * Checks debris-weapon collisions.  
 * @param pair obj_pair pointer to the two objects. pair->a is debris and pair->b is weapon.
 * @return 1 if all future collisions between these can be ignored
 */
int collide_asteroid_weapon( obj_pair * pair )
{
	if (!Asteroids_enabled)
		return 0;

	vec3d	hitpos;
	int		hit;
	object	*pasteroid = pair->a;
	object	*weapon_obj = pair->b;

	Assert( pasteroid->type == OBJ_ASTEROID);
	Assert( weapon_obj->type == OBJ_WEAPON );

	// first check the bounding spheres of the two objects.
	hit = fvi_segment_sphere(&hitpos, &weapon_obj->last_pos, &weapon_obj->pos, &pasteroid->pos, pasteroid->radius);
	if (hit) {
		hit = asteroid_check_collision(pasteroid, weapon_obj, &hitpos );
		if ( !hit )
			return 0;

		Script_system.SetHookObjects(4, "Weapon", weapon_obj, "Asteroid", pasteroid, "Self", weapon_obj, "Object", pasteroid);

		bool weapon_override = Script_system.IsConditionOverride(CHA_COLLIDEASTEROID, weapon_obj);
		Script_system.SetHookObjects(2, "Self",pasteroid, "Object", weapon_obj);
		bool asteroid_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, pasteroid);

		if(!weapon_override && !asteroid_override)
		{
			weapon_hit( weapon_obj, pasteroid, &hitpos );
			asteroid_hit( pasteroid, weapon_obj, &hitpos, Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].damage );
		}

		Script_system.SetHookObjects(2, "Self", weapon_obj, "Object", pasteroid);
		if(!(asteroid_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEASTEROID, '\0', NULL, weapon_obj);

		Script_system.SetHookObjects(2, "Self", pasteroid, "Object", weapon_obj);
		if((asteroid_override && !weapon_override) || (!asteroid_override && !weapon_override))
			Script_system.RunCondition(CHA_COLLIDEWEAPON, '\0', NULL, pasteroid, Weapons[weapon_obj->instance].weapon_info_index);

		Script_system.RemHookVars(4, "Weapon", "Asteroid", "Self", "Object");
		return 0;

	} else {
		return weapon_will_never_hit( weapon_obj, pasteroid, pair );
	}
}				


