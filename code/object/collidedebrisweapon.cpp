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
#include "scripting/scripting.h"
#include "scripting/global_hooks.h"
#include "scripting/api/objs/vecmath.h"
#include "weapon/weapon.h"



/**
 * Checks debris-weapon collisions.  
 * @param pair obj_pair pointer to the two objects. pair->a is debris and pair->b is weapon.
 * @return 1 if all future collisions between these can be ignored
 */
int collide_debris_weapon( obj_pair * pair )
{
	vec3d	hitpos, hitnormal;
	object *pdebris = pair->a;
	object *weapon_obj = pair->b;

	Assert( pdebris->type == OBJ_DEBRIS );
	Assert( weapon_obj->type == OBJ_WEAPON );

	if (reject_due_collision_groups(pdebris, weapon_obj))
		return 0;

	// first check the bounding spheres of the two objects.
	int hit = fvi_segment_sphere(&hitpos, &weapon_obj->last_pos, &weapon_obj->pos, &pdebris->pos, pdebris->radius);
	if (hit) {
		hit = debris_check_collision(pdebris, weapon_obj, &hitpos, nullptr, &hitnormal );

		if ( !hit )
			return 0;

		bool weapon_override = false, debris_override = false;

		if (scripting::hooks::OnDebrisCollision->isActive()) {
			weapon_override = scripting::hooks::OnDebrisCollision->isOverride(scripting::hooks::CollisionConditions{ {weapon_obj, pdebris} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', weapon_obj),
					scripting::hook_param("Object", 'o', pdebris),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Debris", 'o', pdebris),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}
		if (scripting::hooks::OnWeaponCollision->isActive()) {
			debris_override = scripting::hooks::OnWeaponCollision->isOverride(scripting::hooks::CollisionConditions{ {weapon_obj, pdebris} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', pdebris),
					scripting::hook_param("Object", 'o', weapon_obj),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Debris", 'o', pdebris),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}

		if(!weapon_override && !debris_override)
		{
			weapon_hit( weapon_obj, pdebris, &hitpos, -1, &hitnormal );
			debris_hit( pdebris, weapon_obj, &hitpos, Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].damage );
		}

		if (scripting::hooks::OnDebrisCollision->isActive() && !(debris_override && !weapon_override))
		{
			scripting::hooks::OnDebrisCollision->run(scripting::hooks::CollisionConditions{ {weapon_obj, pdebris} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', weapon_obj),
					scripting::hook_param("Object", 'o', pdebris),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Debris", 'o', pdebris),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}

		if (scripting::hooks::OnWeaponCollision->isActive() && ((debris_override && !weapon_override) || (!debris_override && !weapon_override)))
		{
			scripting::hooks::OnWeaponCollision->run(scripting::hooks::CollisionConditions{ {weapon_obj, pdebris} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', pdebris),
					scripting::hook_param("Object", 'o', weapon_obj),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Debris", 'o', pdebris),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}

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

	vec3d	hitpos, hitnormal;
	object	*pasteroid = pair->a;
	object	*weapon_obj = pair->b;

	Assert( pasteroid->type == OBJ_ASTEROID);
	Assert( weapon_obj->type == OBJ_WEAPON );

	// first check the bounding spheres of the two objects.
	int hit = fvi_segment_sphere(&hitpos, &weapon_obj->last_pos, &weapon_obj->pos, &pasteroid->pos, pasteroid->radius);
	if (hit) {
		hit = asteroid_check_collision(pasteroid, weapon_obj, &hitpos, nullptr, &hitnormal);
		if ( !hit )
			return 0;

		bool weapon_override = false, asteroid_override = false;

		if (scripting::hooks::OnAsteroidCollision->isActive()) {
			weapon_override = scripting::hooks::OnAsteroidCollision->isOverride(scripting::hooks::CollisionConditions{ {weapon_obj, pasteroid} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', weapon_obj),
					scripting::hook_param("Object", 'o', pasteroid),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Asteroid", 'o', pasteroid),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}
		if (scripting::hooks::OnWeaponCollision->isActive()) {
			asteroid_override = scripting::hooks::OnWeaponCollision->isOverride(scripting::hooks::CollisionConditions{ {weapon_obj, pasteroid} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', pasteroid),
					scripting::hook_param("Object", 'o', weapon_obj),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Asteroid", 'o', pasteroid),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}

		if(!weapon_override && !asteroid_override)
		{
			weapon_hit( weapon_obj, pasteroid, &hitpos, -1, &hitnormal);
			asteroid_hit( pasteroid, weapon_obj, &hitpos, Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].damage );
		}

		if (scripting::hooks::OnAsteroidCollision->isActive() && !(asteroid_override && !weapon_override))
		{
			scripting::hooks::OnAsteroidCollision->run(scripting::hooks::CollisionConditions{ {weapon_obj, pasteroid} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', weapon_obj),
					scripting::hook_param("Object", 'o', pasteroid),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Asteroid", 'o', pasteroid),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}

		if (scripting::hooks::OnWeaponCollision->isActive() && ((asteroid_override && !weapon_override) || (!asteroid_override && !weapon_override)))
		{
			scripting::hooks::OnWeaponCollision->run(scripting::hooks::CollisionConditions{ {weapon_obj, pasteroid} },
				scripting::hook_param_list(scripting::hook_param("Self", 'o', pasteroid),
					scripting::hook_param("Object", 'o', weapon_obj),
					scripting::hook_param("Weapon", 'o', weapon_obj),
					scripting::hook_param("Asteroid", 'o', pasteroid),
					scripting::hook_param("Hitpos", 'o', hitpos)));
		}

		return 0;

	} else {
		return weapon_will_never_hit( weapon_obj, pasteroid, pair );
	}
}				
