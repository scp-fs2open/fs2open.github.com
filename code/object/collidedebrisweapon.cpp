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



// Everything the narrowphase produces for a debris/asteroid vs weapon hit.  The mc_info is
// carried across rather than being written straight onto the weapon, because that write has to
// happen on the main thread.
struct debris_weapon_collision_data {
	vec3d hitpos;
	vec3d hitnormal;
	mc_info mc;
};

/**
 * Applies a debris-weapon collision.  Main thread only.
 */
void collide_debris_weapon_process( obj_pair * pair, const std::any& collision_data )
{
	const auto& cd = std::any_cast<const debris_weapon_collision_data&>(collision_data);

	object *pdebris = pair->a;
	object *weapon_obj = pair->b;

	// mutable copies: the impact/hit helpers below take non-const vec3d*
	vec3d hitpos = cd.hitpos;
	vec3d hitnormal = cd.hitnormal;

	Weapons[weapon_obj->instance].collisionInfo = new mc_info(cd.mc);	// The weapon will free this memory later

	{
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
			vec3d force = weapon_obj->phys_info.vel * Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].mass;
			bool armed = weapon_hit( weapon_obj, pdebris, &hitpos, -1 );
			float damage = Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].damage;
			std::array<std::optional<ConditionData>, NumHitTypes> impact_data = {};
			impact_data[static_cast<std::underlying_type_t<HitType>>(HitType::HULL)] = ConditionData {
				SpecialImpactCondition::DEBRIS,
				HitType::HULL,
				damage,
				pdebris->hull_strength,
				Debris[pdebris->instance].max_hull,
			};
			maybe_play_conditional_impacts(impact_data, weapon_obj, pdebris, armed, -1, &hitpos, nullptr, &hitnormal);
			debris_hit( pdebris, weapon_obj, &hitpos, damage , &force);
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
	}
}

/**
 * Checks debris-weapon collisions.  Pure: safe to run on a collision worker thread.
 * @param pair obj_pair pointer to the two objects. pair->a is debris and pair->b is weapon.
 */
collision_result collide_debris_weapon_check( obj_pair * pair )
{
	object *pdebris = pair->a;
	object *weapon_obj = pair->b;

	Assert( pdebris->type == OBJ_DEBRIS );
	Assert( weapon_obj->type == OBJ_WEAPON );

	if (reject_due_collision_groups(pdebris, weapon_obj))
		return { false, std::any(), &collide_debris_weapon_process };

	debris_weapon_collision_data cd;

	// first check the bounding spheres of the two objects.
	int hit = fvi_segment_sphere(&cd.hitpos, &weapon_obj->last_pos, &weapon_obj->pos, &pdebris->pos, pdebris->radius);
	if (hit) {
		hit = debris_check_collision(pdebris, weapon_obj, &cd.hitpos, nullptr, &cd.hitnormal, &cd.mc );

		if ( !hit )
			return { false, std::any(), &collide_debris_weapon_process };

		return { false, std::any(cd), &collide_debris_weapon_process };
	} else {
		return { weapon_will_never_hit( weapon_obj, pdebris, pair ) != 0, std::any(), &collide_debris_weapon_process };
	}
}

int collide_debris_weapon( obj_pair * pair )
{
	const auto& [never_check_again, collision_data, process_fnc] = collide_debris_weapon_check(pair);

	if (collision_data.has_value()) {
		process_fnc(pair, collision_data);
	}

	return never_check_again ? 1 : 0;
}



/**
 * Applies an asteroid-weapon collision.  Main thread only.
 */
void collide_asteroid_weapon_process( obj_pair * pair, const std::any& collision_data )
{
	const auto& cd = std::any_cast<const debris_weapon_collision_data&>(collision_data);

	object	*pasteroid = pair->a;
	object	*weapon_obj = pair->b;

	// mutable copies: the impact/hit helpers below take non-const vec3d*
	vec3d hitpos = cd.hitpos;
	vec3d hitnormal = cd.hitnormal;

	{
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
			vec3d force = weapon_obj->phys_info.vel * Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].mass;
			bool armed = weapon_hit( weapon_obj, pasteroid, &hitpos, -1);
			float damage = Weapon_info[Weapons[weapon_obj->instance].weapon_info_index].damage;
			std::array<std::optional<ConditionData>, NumHitTypes> impact_data = {};
			impact_data[static_cast<std::underlying_type_t<HitType>>(HitType::HULL)] = ConditionData {
				SpecialImpactCondition::DEBRIS,
				HitType::HULL,
				damage,
				pasteroid->hull_strength,
				Asteroid_info[Asteroids[pasteroid->instance].asteroid_type].initial_asteroid_strength,
			};
			maybe_play_conditional_impacts(impact_data, weapon_obj, pasteroid, armed, -1, &hitpos, nullptr, &hitnormal);
			asteroid_hit( pasteroid, weapon_obj, &hitpos, damage, &force );
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
	}
}

/**
 * Checks asteroid-weapon collisions.  Pure: safe to run on a collision worker thread.
 * @param pair obj_pair pointer to the two objects. pair->a is asteroid and pair->b is weapon.
 */
collision_result collide_asteroid_weapon_check( obj_pair * pair )
{
	if (!Asteroids_enabled)
		return { false, std::any(), &collide_asteroid_weapon_process };

	object	*pasteroid = pair->a;
	object	*weapon_obj = pair->b;

	Assert( pasteroid->type == OBJ_ASTEROID);
	Assert( weapon_obj->type == OBJ_WEAPON );

	debris_weapon_collision_data cd;

	// first check the bounding spheres of the two objects.
	int hit = fvi_segment_sphere(&cd.hitpos, &weapon_obj->last_pos, &weapon_obj->pos, &pasteroid->pos, pasteroid->radius);
	if (hit) {
		hit = asteroid_check_collision(pasteroid, weapon_obj, &cd.hitpos, nullptr, &cd.hitnormal);
		if ( !hit )
			return { false, std::any(), &collide_asteroid_weapon_process };

		return { false, std::any(cd), &collide_asteroid_weapon_process };
	} else {
		return { weapon_will_never_hit( weapon_obj, pasteroid, pair ) != 0, std::any(), &collide_asteroid_weapon_process };
	}
}

int collide_asteroid_weapon( obj_pair * pair )
{
	const auto& [never_check_again, collision_data, process_fnc] = collide_asteroid_weapon_check(pair);

	if (collision_data.has_value()) {
		process_fnc(pair, collision_data);
	}

	return never_check_again ? 1 : 0;
}
