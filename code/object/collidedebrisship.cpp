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
#include "hud/hud.h"
#include "io/timer.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "scripting/scripting.h"
#include "scripting/api/objs/vecmath.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "ship/shiphit.h"


void calculate_ship_ship_collision_physics(collision_info_struct *ship_ship_hit_info);

/**
 * Checks debris-ship collisions.  
 * @param pair obj_pair pointer to the two objects. pair->a is debris and pair->b is ship.
 * @return 1 if all future collisions between these can be ignored
 */
int collide_debris_ship( obj_pair * pair )
{
	float dist;
	object *debris_objp = pair->a;
	object *ship_objp = pair->b;

	// Don't check collisions for warping out player
	if ( Player->control_mode != PCM_NORMAL )	{
		if ( ship_objp == Player_obj )
			return 0;
	}

	Assert( debris_objp->type == OBJ_DEBRIS );
	Assert( ship_objp->type == OBJ_SHIP );

	if (reject_due_collision_groups(debris_objp, ship_objp))
		return 0;

	ship* shipp = &Ships[ship_objp->instance];
	// don't check collision if it's our own debris and we are dying
	if ( (debris_objp->parent == OBJ_INDEX(ship_objp)) && (shipp->flags[Ship::Ship_Flags::Dying]) )
		return 0;

	dist = vm_vec_dist( &debris_objp->pos, &ship_objp->pos );
	if ( dist < debris_objp->radius + ship_objp->radius )	{
		int hit;
		vec3d	hitpos;
		// create and initialize ship_ship_hit_info struct
		collision_info_struct debris_hit_info;
		init_collision_info_struct(&debris_hit_info);

		if ( debris_objp->phys_info.mass > ship_objp->phys_info.mass ) {
			debris_hit_info.heavy = debris_objp;
			debris_hit_info.light = ship_objp;
		} else {
			debris_hit_info.heavy = ship_objp;
			debris_hit_info.light = debris_objp;
		}

		hit = debris_check_collision(debris_objp, ship_objp, &hitpos, &debris_hit_info );
		if ( hit )
		{
			bool ship_override = false, debris_override = false;

			if (Script_system.IsActiveAction(CHA_COLLIDEDEBRIS)) {
				Script_system.SetHookObjects(4, "Self", ship_objp, "Object", debris_objp, "Ship", ship_objp, "Debris", debris_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				ship_override = Script_system.IsConditionOverride(CHA_COLLIDEDEBRIS, ship_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Debris", "Hitpos" });
			}

			if (Script_system.IsActiveAction(CHA_COLLIDESHIP)) {
				Script_system.SetHookObjects(4, "Self", debris_objp, "Object", ship_objp, "Ship", ship_objp, "Debris", debris_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				debris_override = Script_system.IsConditionOverride(CHA_COLLIDESHIP, debris_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Debris", "Hitpos" });
			}

			if(!ship_override && !debris_override)
			{
				float		ship_damage;	
				float		debris_damage;

				// do collision physics
				calculate_ship_ship_collision_physics( &debris_hit_info );

				if ( debris_hit_info.impulse < 0.5f )
					return 0;

				// calculate ship damage
				ship_damage = 0.005f * debris_hit_info.impulse;	//	Cut collision-based damage in half.
				//	Decrease heavy damage by 2x.
				if (ship_damage > 5.0f)
					ship_damage = 5.0f + (ship_damage - 5.0f)/2.0f;

				// calculate debris damage and set debris damage to greater or debris and ship
				// debris damage is needed since we can really whack some small debris with afterburner and not do
				// significant damage to ship but the debris goes off faster than afterburner speed.
				debris_damage = debris_hit_info.impulse/debris_objp->phys_info.mass;	// ie, delta velocity of debris
				debris_damage = (debris_damage > ship_damage) ? debris_damage : ship_damage;

				// modify ship damage by debris damage multiplier
				ship_damage *= Debris[debris_objp->instance].damage_mult;

				// supercaps cap damage at 10-20% max hull ship damage
				if (Ship_info[shipp->ship_info_index].flags[Ship::Info_Flags::Supercap]) {
					float cap_percent_damage = frand_range(0.1f, 0.2f);
					ship_damage = MIN(ship_damage, cap_percent_damage * shipp->ship_max_hull_strength);
				}

				// apply damage to debris
				debris_hit( debris_objp, ship_objp, &hitpos, debris_damage);		// speed => damage
				int apply_ship_damage;

				// apply damage to ship unless 1) debris is from ship
				apply_ship_damage = (ship_objp->signature != debris_objp->parent_sig);

				if ( debris_hit_info.heavy == ship_objp) {
					int quadrant_num = get_ship_quadrant_from_global(&hitpos, ship_objp);
					if (The_mission.ai_profile->flags[AI::Profile_Flags::No_shield_damage_from_ship_collisions] || 
						(ship_objp->flags[Object::Object_Flags::No_shields]) || !ship_is_shield_up(ship_objp, quadrant_num) ) {
						quadrant_num = -1;
					}
					if (apply_ship_damage) {
						ship_apply_local_damage(debris_hit_info.heavy, debris_hit_info.light, &hitpos, ship_damage, Debris[debris_objp->instance].damage_type_idx, quadrant_num, CREATE_SPARKS, debris_hit_info.submodel_num);
					}
				} else {
					// don't draw sparks using sphere hit position
					if (apply_ship_damage) {
						ship_apply_local_damage(debris_hit_info.light, debris_hit_info.heavy, &hitpos, ship_damage, Debris[debris_objp->instance].damage_type_idx, MISS_SHIELDS, NO_SPARKS);
					}
				}

				// maybe print Collision on HUD
				if ( ship_objp == Player_obj ) {					
					hud_start_text_flash(XSTR("Collision", 1431), 2000);
				}

				collide_ship_ship_do_sound(&hitpos, ship_objp, debris_objp, ship_objp==Player_obj);
			}

			if (Script_system.IsActiveAction(CHA_COLLIDEDEBRIS) && !(debris_override && !ship_override))
			{
				Script_system.SetHookObjects(4, "Self", ship_objp, "Object", debris_objp, "Ship", ship_objp, "Debris", debris_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				Script_system.RunCondition(CHA_COLLIDEDEBRIS, ship_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Debris", "Hitpos" });
			}

			if (Script_system.IsActiveAction(CHA_COLLIDESHIP) && ((debris_override && !ship_override) || (!debris_override && !ship_override)))
			{
				Script_system.SetHookObjects(4, "Self", debris_objp, "Object", ship_objp, "Ship", ship_objp, "Debris", debris_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				Script_system.RunCondition(CHA_COLLIDESHIP, debris_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Debris", "Hitpos" });
			}

			return 0;
		}
	} else {	//	Bounding spheres don't intersect, set timestamp for next collision check.
		float	ship_max_speed, debris_speed;
		float	time;

		if (ship_is_beginning_warpout_speedup(ship_objp)) {
			ship_max_speed = MAX(ship_get_max_speed(shipp), ship_get_warpout_speed(ship_objp));
		} else {
			ship_max_speed = ship_get_max_speed(shipp);
		}
		ship_max_speed = MAX(ship_max_speed, 10.0f);
		ship_max_speed = MAX(ship_max_speed, ship_objp->phys_info.vel.xyz.z);

		debris_speed = debris_objp->phys_info.speed;

		time = 1000.0f * (dist - ship_objp->radius - debris_objp->radius - 10.0f) / (ship_max_speed + debris_speed);		// 10.0f is a safety factor
		time -= 200.0f;		// allow one frame slow frame at ~5 fps

		if (time > 100) {
			pair->next_check_time = timestamp( fl2i(time) );
		} else {
			pair->next_check_time = timestamp(0);	// check next time
		}
	}

	return 0;
}

/**
 * Checks asteroid-ship collisions.  
 * @param pair obj_pair pointer to the two objects. pair->a is asteroid and pair->b is ship.
 * @return 1 if all future collisions between these can be ignored
 */
int collide_asteroid_ship( obj_pair * pair )
{
	if (!Asteroids_enabled)
		return 0;

	float		dist;
	object	*asteroid_objp = pair->a;
	object	*ship_objp = pair->b;

	// Don't check collisions for warping out player
	if ( Player->control_mode != PCM_NORMAL )	{
		if ( ship_objp == Player_obj ) return 0;
	}

	if (asteroid_objp->hull_strength < 0.0f)
		return 0;

	Assert( asteroid_objp->type == OBJ_ASTEROID );
	Assert( ship_objp->type == OBJ_SHIP );

	dist = vm_vec_dist( &asteroid_objp->pos, &ship_objp->pos );

	ship* shipp = &Ships[ship_objp->instance];

	if ( dist < asteroid_objp->radius + ship_objp->radius )	{
		int hit;
		vec3d	hitpos;
		// create and initialize ship_ship_hit_info struct
		collision_info_struct asteroid_hit_info;
		init_collision_info_struct(&asteroid_hit_info);

		if ( asteroid_objp->phys_info.mass > ship_objp->phys_info.mass ) {
			asteroid_hit_info.heavy = asteroid_objp;
			asteroid_hit_info.light = ship_objp;
		} else {
			asteroid_hit_info.heavy = ship_objp;
			asteroid_hit_info.light = asteroid_objp;
		}

		hit = asteroid_check_collision(asteroid_objp, ship_objp, &hitpos, &asteroid_hit_info );
		if ( hit )
		{
			bool ship_override = false, asteroid_override = false;

			//Scripting support (WMC)
			if (Script_system.IsActiveAction(CHA_COLLIDEASTEROID)) {
				Script_system.SetHookObjects(4, "Self", ship_objp, "Object", asteroid_objp, "Ship", ship_objp, "Asteroid", asteroid_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				ship_override = Script_system.IsConditionOverride(CHA_COLLIDEASTEROID, ship_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Asteroid", "Hitpos" });
			}

			if (Script_system.IsActiveAction(CHA_COLLIDESHIP)) {
				Script_system.SetHookObjects(4, "Self", asteroid_objp, "Object", ship_objp, "Ship", ship_objp, "Asteroid", asteroid_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				asteroid_override = Script_system.IsConditionOverride(CHA_COLLIDESHIP, asteroid_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Asteroid", "Hitpos" });
			}

			if(!ship_override && !asteroid_override)
			{
				float		ship_damage;	
				float		asteroid_damage;

				vec3d asteroid_vel = asteroid_objp->phys_info.vel;

				// do collision physics
				calculate_ship_ship_collision_physics( &asteroid_hit_info );

				if ( asteroid_hit_info.impulse < 0.5f )
					return 0;

				// limit damage from impulse by making max impulse (for damage) 2*m*v_max_relative
				float max_ship_impulse = (2.0f*ship_objp->phys_info.max_vel.xyz.z+vm_vec_mag_quick(&asteroid_vel)) * 
					(ship_objp->phys_info.mass*asteroid_objp->phys_info.mass) / (ship_objp->phys_info.mass + asteroid_objp->phys_info.mass);

				if (asteroid_hit_info.impulse > max_ship_impulse) {
					ship_damage = 0.001f * max_ship_impulse;
				} else {
					ship_damage = 0.001f * asteroid_hit_info.impulse;	//	Cut collision-based damage in half.
				}

				//	Decrease heavy damage by 2x.
				if (ship_damage > 5.0f)
					ship_damage = 5.0f + (ship_damage - 5.0f)/2.0f;

				if ((ship_damage > 500.0f) && (ship_damage > shipp->ship_max_hull_strength/8.0f)) {
					ship_damage = shipp->ship_max_hull_strength/8.0f;
					nprintf(("AI", "Pinning damage to %s from asteroid at %7.3f (%7.3f percent)\n", shipp->ship_name, ship_damage, 100.0f * ship_damage/ shipp->ship_max_hull_strength));
				}

				//	Decrease damage during warp out because it's annoying when your escoree dies during warp out.
				if (Ai_info[shipp->ai_index].mode == AIM_WARP_OUT)
					ship_damage /= 3.0f;

				// calculate asteroid damage and set asteroid damage to greater or asteroid and ship
				// asteroid damage is needed since we can really whack some small asteroid with afterburner and not do
				// significant damage to ship but the asteroid goes off faster than afterburner speed.
				asteroid_damage = asteroid_hit_info.impulse/asteroid_objp->phys_info.mass;	// ie, delta velocity of asteroid
				asteroid_damage = (asteroid_damage > ship_damage) ? asteroid_damage : ship_damage;

				// apply damage to asteroid
				asteroid_hit( asteroid_objp, ship_objp, &hitpos, asteroid_damage);		// speed => damage

				int ast_damage_type = Asteroid_info[Asteroids[asteroid_objp->instance].asteroid_type].damage_type_idx;

				if ( asteroid_hit_info.heavy == ship_objp) {
					int quadrant_num = get_ship_quadrant_from_global(&hitpos, ship_objp);
					if (The_mission.ai_profile->flags[AI::Profile_Flags::No_shield_damage_from_ship_collisions] || 
						(ship_objp->flags[Object::Object_Flags::No_shields]) || !ship_is_shield_up(ship_objp, quadrant_num) ) {
						quadrant_num = -1;
					}
					ship_apply_local_damage(asteroid_hit_info.heavy, asteroid_hit_info.light, &hitpos, ship_damage, ast_damage_type, quadrant_num, CREATE_SPARKS, asteroid_hit_info.submodel_num);
				} else {
					// don't draw sparks (using sphere hitpos)
					ship_apply_local_damage(asteroid_hit_info.light, asteroid_hit_info.heavy, &hitpos, ship_damage, ast_damage_type, MISS_SHIELDS, NO_SPARKS);
				}

				// maybe print Collision on HUD
				if ( ship_objp == Player_obj ) {					
					hud_start_text_flash(XSTR("Collision", 1431), 2000);
				}

				collide_ship_ship_do_sound(&hitpos, ship_objp, asteroid_objp, ship_objp==Player_obj);
			}

			if (Script_system.IsActiveAction(CHA_COLLIDEASTEROID) && !(asteroid_override && !ship_override))
			{
				Script_system.SetHookObjects(4, "Self", ship_objp, "Object", asteroid_objp, "Ship", ship_objp, "Asteroid", asteroid_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				Script_system.RunCondition(CHA_COLLIDEASTEROID, ship_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Asteroid", "Hitpos" });
			}

			if (Script_system.IsActiveAction(CHA_COLLIDESHIP) && ((asteroid_override && !ship_override) || (!asteroid_override && !ship_override)))
			{
				Script_system.SetHookObjects(4, "Self", asteroid_objp, "Object", ship_objp, "Ship", ship_objp, "Asteroid", asteroid_objp);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(hitpos));
				Script_system.RunCondition(CHA_COLLIDESHIP, asteroid_objp);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "Asteroid", "Hitpos" });
			}

			return 0;
		}

		return 0;
	} else {
		// estimate earliest time at which pair can hit
		float asteroid_max_speed, ship_max_speed, time;

		asteroid_max_speed = vm_vec_mag(&asteroid_objp->phys_info.vel);		// Asteroid... vel gets reset, not max vel.z
		asteroid_max_speed = MAX(asteroid_max_speed, 10.0f);

		if (ship_is_beginning_warpout_speedup(ship_objp)) {
			ship_max_speed = MAX(ship_get_max_speed(shipp), ship_get_warpout_speed(ship_objp));
		} else {
			ship_max_speed = ship_get_max_speed(shipp);
		}
		ship_max_speed = MAX(ship_max_speed, 10.0f);
		ship_max_speed = MAX(ship_max_speed, ship_objp->phys_info.vel.xyz.z);


		time = 1000.0f * (dist - ship_objp->radius - asteroid_objp->radius - 10.0f) / (asteroid_max_speed + ship_max_speed);		// 10.0f is a safety factor
		time -= 200.0f;		// allow one frame slow frame at ~5 fps

		if (time > 100) {
			pair->next_check_time = timestamp( fl2i(time) );
		} else {
			pair->next_check_time = timestamp(0);	// check next time
		}
		return 0;
	}
}
