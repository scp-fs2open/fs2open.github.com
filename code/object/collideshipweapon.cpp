/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "hud/hudshield.h"
#include "hud/hudwingmanstatus.h"
#include "io/timer.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "scripting/global_hooks.h"
#include "scripting/scripting.h"
#include "scripting/api/objs/vecmath.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "weapon/weapon.h"


extern float ai_endangered_time(object *ship_objp, object *weapon_objp);
static int check_inside_radius_for_big_ships( object *ship, object *weapon_obj, obj_pair *pair );
extern float flFrametime;


/**
 * If weapon_obj is likely to hit ship_obj sooner than current aip->danger_weapon_objnum,
 * then update danger_weapon_objnum.
 */
static void update_danger_weapon(object *pship_obj, object *weapon_obj)
{
	ai_info	*aip;

	Assert(pship_obj->type == OBJ_SHIP);

	aip = &Ai_info[Ships[pship_obj->instance].ai_index];

	if (aip->danger_weapon_objnum == -1) {
		aip->danger_weapon_objnum = OBJ_INDEX(weapon_obj);
		aip->danger_weapon_signature = weapon_obj->signature;
	} else if (aip->danger_weapon_signature == Objects[aip->danger_weapon_objnum].signature) {
		float	danger_old_time, danger_new_time;

		danger_old_time = ai_endangered_time(pship_obj, &Objects[aip->danger_weapon_objnum]);
		danger_new_time = ai_endangered_time(pship_obj, weapon_obj);

		if (danger_new_time < danger_old_time) {
			aip->danger_weapon_objnum = OBJ_INDEX(weapon_obj);
			aip->danger_weapon_signature = weapon_obj->signature;
		}
	}
}

/** 
 * Deal with weapon-ship hit stuff.  
 * Separated from check_collision routine below because of multiplayer reasons.
 */
static void ship_weapon_do_hit_stuff(object *pship_obj, object *weapon_obj, vec3d *world_hitpos, vec3d *hitpos, int quadrant_num, int submodel_num, vec3d /*not a pointer intentionaly*/ hit_dir)
{
	weapon	*wp = &Weapons[weapon_obj->instance];
	weapon_info *wip = &Weapon_info[wp->weapon_info_index];
	ship *shipp = &Ships[pship_obj->instance];
	auto pmi = model_get_instance(shipp->model_instance_num);
	auto pm = model_get(pmi->model_num);
	float damage;
	vec3d force;

	vec3d worldNormal;
	model_instance_local_to_global_dir(&worldNormal, &hit_dir, pm, pmi, submodel_num, &pship_obj->orient);

	// Apply hit & damage & stuff to weapon
	weapon_hit(weapon_obj, pship_obj,  world_hitpos, quadrant_num, &worldNormal);

	if (wip->damage_time >= 0.0f && wp->lifeleft <= wip->damage_time) {
		if (wip->atten_damage >= 0.0f) {
			damage = (((wip->damage - wip->atten_damage) * (wp->lifeleft / wip->damage_time)) + wip->atten_damage);
		} else {
			damage = wip->damage * (wp->lifeleft / wip->damage_time);
		}
	} else {
		damage = wip->damage;
	}

	if (wip->damage_incidence_max != 1.0f || wip->damage_incidence_min != 1.0f) {
		float dot = -vm_vec_dot(&weapon_obj->orient.vec.fvec, &worldNormal);

		if (dot < 0.0f)
			dot = 0.0f;

		damage *= wip->damage_incidence_min + ((wip->damage_incidence_max - wip->damage_incidence_min) * dot);
	}

	// deterine whack whack
	float		blast = wip->mass;
	vm_vec_copy_scale(&force, &weapon_obj->phys_info.vel, blast );	

	// send player pain packet
	if ( (MULTIPLAYER_MASTER) && !(shipp->flags[Ship::Ship_Flags::Dying]) ){
		int np_index = multi_find_player_by_object(pship_obj);

		// if this is a player ship
		if((np_index >= 0) && (np_index != MY_NET_PLAYER_NUM) && (wip->subtype == WP_LASER)){
			send_player_pain_packet(&Net_players[np_index], wp->weapon_info_index, wip->damage * weapon_get_damage_scale(wip, weapon_obj, pship_obj), &force, world_hitpos, quadrant_num);
		}
	}	

	ship_apply_local_damage(pship_obj, weapon_obj, world_hitpos, damage, wip->damage_type_idx, quadrant_num, CREATE_SPARKS, submodel_num);

	// let the hud shield gauge know when Player or Player target is hit
	hud_shield_quadrant_hit(pship_obj, quadrant_num);

	// Let wingman status gauge know a wingman ship was hit
	if ( (Ships[pship_obj->instance].wing_status_wing_index >= 0) && ((Ships[pship_obj->instance].wing_status_wing_pos >= 0)) ) {
		hud_wingman_status_start_flash(shipp->wing_status_wing_index, shipp->wing_status_wing_pos);
	}

	// Apply a wack.  This used to be inside of ship_hit... duh! Ship_hit
	// is to apply damage, not physics, so I moved it here.
	// don't apply whack for multiplayer_client from laser - will occur with pain packet
	if (!((wip->subtype == WP_LASER) && MULTIPLAYER_CLIENT) ) {		
		// apply a whack		
		ship_apply_whack( &force, world_hitpos, pship_obj );
	}

	// Add impact decal
	if (quadrant_num == -1 && wip->impact_decal.definition_handle >= 0) {
		// Use the weapon orientation to augment the orientation computation.
		vec3d weapon_up;
		model_instance_global_to_local_dir(&weapon_up, &weapon_obj->orient.vec.uvec, pm, pmi, submodel_num, &pship_obj->orient);

		matrix decal_orient;
		vm_vector_2_matrix_norm(&decal_orient, &hit_dir, &weapon_up); // hit_dir is already normalized so we can use the more efficient function

		decals::addDecal(wip->impact_decal, pship_obj, submodel_num, *hitpos, decal_orient);
	}
}

extern int Framecount;

static int ship_weapon_check_collision(object *ship_objp, object *weapon_objp, float time_limit = 0.0f, int *next_hit = nullptr)
{
	mc_info mc, mc_shield, mc_hull;
	ship	*shipp;
	ship_info *sip;
	weapon	*wp;
	weapon_info	*wip;

	Assert( ship_objp != nullptr );
	Assert( ship_objp->type == OBJ_SHIP );
	Assert( ship_objp->instance >= 0 );

	shipp = &Ships[ship_objp->instance];
	sip = &Ship_info[shipp->ship_info_index];

	Assert( weapon_objp != nullptr );
	Assert( weapon_objp->type == OBJ_WEAPON );
	Assert( weapon_objp->instance >= 0 );

	wp = &Weapons[weapon_objp->instance];
	wip = &Weapon_info[wp->weapon_info_index];


	Assert( shipp->objnum == OBJ_INDEX(ship_objp));

	// Make ships that are warping in not get collision detection done
	if ( shipp->is_arriving() ) return 0;
	
	//	Return information for AI to detect incoming fire.
	//	Could perhaps be done elsewhere at lower cost --MK, 11/7/97
	float	dist = vm_vec_dist_quick(&ship_objp->pos, &weapon_objp->pos);
	if (dist < weapon_objp->phys_info.speed) {
		update_danger_weapon(ship_objp, weapon_objp);
	}

	int	valid_hit_occurred = 0;				// If this is set, then hitpos is set
	int	quadrant_num = -1;
	polymodel *pm = model_get(sip->model_num);

	//	total time is flFrametime + time_limit (time_limit used to predict collisions into the future)
	vec3d weapon_end_pos;
	vm_vec_scale_add( &weapon_end_pos, &weapon_objp->pos, &weapon_objp->phys_info.vel, time_limit );


	vec3d weapon_start_pos = weapon_objp->last_pos;
	// Maybe take into account the ship's velocity, so it won't later overstep the weapon's
	// current position (what will be its last_pos next frame)
	if (The_mission.ai_profile->flags[AI::Profile_Flags::Fixed_ship_weapon_collision])
		weapon_start_pos += ship_objp->phys_info.vel * flFrametime;

	if (!IS_VEC_NULL(&The_mission.gravity) && wip->gravity_const != 0.0f) {
		// subtle point about simulating collisions against a parabola
		// every simulated point, the positions every frame, are perfectly on the correct parabola
		// however this means collision is checking *the lines inbetween* those points, which will always be undernearth the parabola
		// the grater the frametime, the greater the discrepancy, and can cause erroneous misses
		// So just for collision purposes, we offset these points slightly in the opposite direction of gravity
		// at least to ensure the *average* position at all interpolated points is on the parabola
		weapon_start_pos -= The_mission.gravity * flFrametime * flFrametime * (1. / 12);
		weapon_end_pos -= The_mission.gravity * flFrametime * flFrametime * (1. / 12);
	}

	// Goober5000 - I tried to make collision code here much saner... here begin the (major) changes
	mc_info_init(&mc);

	// set up collision structs
	mc.model_instance_num = shipp->model_instance_num;
	mc.model_num = sip->model_num;
	mc.submodel_num = -1;
	mc.orient = &ship_objp->orient;
	mc.pos = &ship_objp->pos;
	mc.p0 = &weapon_start_pos;
	mc.p1 = &weapon_end_pos;
	mc.lod = sip->collision_lod;
	memcpy(&mc_shield, &mc, sizeof(mc_info));
	memcpy(&mc_hull, &mc, sizeof(mc_info));

	// (btw, these are leftover comments from below...)
	//
	//	Note: This code is obviously stupid. We want to add the shield point if there is shield to hit, but:
	//		1. We want the size/color of the hit effect to indicate shield damage done.  (i.e., for already-weak shield, smaller effect)
	//		2. Currently (8/9/97), shield_apply_damage() passes lefer damage to hull, which might not make sense.  If
	//			wouldn't have collided with hull, shouldn't do damage.  Once this is fixed, the code below needs to cast the
	//			vector through to the hull if there is leftover damage.
	//
	// WIF2_PIERCE_SHIELDS pierces shields
	// AL 1-14-97: "Puncture" doesn't mean penetrate shield anymore, it means that it punctures
	//					hull to inflict maximum subsystem damage
	//
	// _argv[-1], 16 Jan 2005: Surface shields.
	// Surface shields allow for shields on a ship without a shield mesh.  Good for putting real shields
	// on the Lucifer.  This also fixes the strange bug where shots will occasionally go through the
	// shield mesh when they shouldn't.  I don't know what causes this, but this fixes that -- shields
	// will absorb it when it hits the hull instead.  This has no fancy graphical effect, though.
	// Someone should make one.

	// check both kinds of collisions
	int shield_collision = 0;
	int hull_collision = 0;

	// check shields for impact
	if (!(ship_objp->flags[Object::Object_Flags::No_shields])) {
		if (sip->flags[Ship::Info_Flags::Auto_spread_shields]) {
			// The weapon is not allowed to impact the shield before it reaches this point
			vec3d shield_ignored_until = weapon_objp->last_pos;

			float weapon_flown_for = vm_vec_dist(&wp->start_pos, &weapon_objp->last_pos);
			float min_weapon_span;

			if (sip->auto_shield_spread_min_span >= 0.0f) {
				min_weapon_span = sip->auto_shield_spread_min_span;
			} else {
				min_weapon_span = sip->auto_shield_spread;
			}

			// If weapon hasn't yet flown a distance greater than the maximum ignore
			// range, then some part of the currently checked range needs to be
			// ignored
			if (weapon_flown_for < min_weapon_span) {
				vm_vec_sub(&shield_ignored_until, &weapon_end_pos, &wp->start_pos);
				vm_vec_normalize(&shield_ignored_until);
				vm_vec_scale(&shield_ignored_until, min_weapon_span);
				vm_vec_add2(&shield_ignored_until, &wp->start_pos);
			}

			float this_range = vm_vec_dist(&weapon_objp->last_pos, &weapon_end_pos);

			// The range during which the weapon is not allowed to collide with the
			// shield, except if it actually hits the hull
			float ignored_range;

			// If the weapon has not yet surpassed the ignore range, calculate the
			// remaining ignore range
			if (vm_vec_dist(&wp->start_pos, &shield_ignored_until) > weapon_flown_for)
				ignored_range = vm_vec_dist(&weapon_objp->last_pos, &shield_ignored_until);
			else
				ignored_range = 0.0f;

			// The range during which the weapon may impact the shield
			float active_range = this_range - ignored_range;

			// During the ignored range, we only check for a ray collision with
			// the model
			if (ignored_range > 0.0f) {
				mc_shield.flags = MC_CHECK_MODEL;
				mc_shield.p1 = &shield_ignored_until;

				shield_collision = model_collide(&mc_shield);

				mc_shield.p1 = &weapon_end_pos;
				mc_shield.hit_dist = mc_shield.hit_dist * (ignored_range / this_range);
			}

			// If no collision with the model found in the ignore range, only
			// then do we check for sphereline collisions with the model during the
			// non-ignored range
			if (!shield_collision && weapon_flown_for + this_range > min_weapon_span) {
				mc_shield.p0 = &shield_ignored_until;

				mc_shield.p1 = &weapon_end_pos;

				mc_shield.radius = sip->auto_shield_spread;

				if (sip->auto_shield_spread_from_lod > -1) {
					mc_shield.lod = sip->auto_shield_spread_from_lod;
				}

				mc_shield.flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE;

				shield_collision = model_collide(&mc_shield);

				mc_shield.lod = sip->collision_lod;
				mc_shield.submodel_num = -1;

				// Because we manipulated p0 and p1 above, hit_dist will be
				// relative to the values we used, not the values the rest of
				// the code expects; this fixes that
				mc_shield.p0 = &weapon_objp->last_pos;
				mc_shield.p1 = &weapon_end_pos;
				mc_shield.hit_dist = (ignored_range + (active_range * mc_shield.hit_dist)) / this_range;
			}

			if (shield_collision) {
				// If we used a sphereline check, then the collision point will lie
				// somewhere on the ship's hull; this re-positions it to lie on the
				// correct point along the weapon's path
				if (mc_shield.flags & MC_CHECK_SPHERELINE) {
					vec3d tempv;
					vm_vec_sub(&tempv, mc_shield.p1, mc_shield.p0);
					vm_vec_scale(&tempv, mc_shield.hit_dist);
					vm_vec_add2(&tempv, mc_shield.p0);
					mc_shield.hit_point_world = tempv;
				}

				// Re-calculate hit_point because it's likely pointing to the wrong
				// place
				vec3d tempv;
				vm_vec_sub(&tempv, &mc_shield.hit_point_world, &ship_objp->pos);
				vm_vec_rotate(&mc_shield.hit_point, &tempv, &ship_objp->orient);
			}
		} else if (sip->flags[Ship::Info_Flags::Surface_shields]) {
			if (pm->shield.ntris > 0) {
				// If there is a shield mesh, we need to check that first
				mc_shield.flags = MC_CHECK_SHIELD;
				shield_collision = model_collide(&mc_shield);
			}

			if (!shield_collision) {
				// But if no shield mesh or it was missed, check for a hull collision
				mc_shield.flags = MC_CHECK_MODEL;
				shield_collision = model_collide(&mc_shield);

				// Because we used MC_CHECK_MODEL, the returned hit position might be
				// in a submodel's frame of reference, so we need to ensure we end up
				// in the ship's frame of reference
				vec3d local_pos;
				vm_vec_sub(&local_pos, &mc_shield.hit_point_world, &ship_objp->pos);
				vm_vec_rotate(&mc_shield.hit_point, &local_pos, &ship_objp->orient);
			}
		} else {
			// Normal collision check against a shield mesh
			mc_shield.flags = MC_CHECK_SHIELD;
			shield_collision = (pm->shield.ntris > 0) ? model_collide(&mc_shield) : 0;
		}
	}

	// If we found a shield collision but were only checking for a simple model
	// collision, we can re-use the same collision info for the hull as well
	if (shield_collision && mc_shield.flags == MC_CHECK_MODEL) {
		memcpy(&mc_hull, &mc_shield, sizeof(mc_info));
		hull_collision = shield_collision;

		// The weapon has impacted on the hull, so if it should therefore bypass
		// the shields altogether, we do it here
		if (sip->auto_shield_spread_bypass) {
			shield_collision = 0;
		}
	} else {
		mc_hull.flags = MC_CHECK_MODEL;
		hull_collision = model_collide(&mc_hull);
	}

	// check if the hit point is beyond the clip plane when warping out.
	if (hull_collision || shield_collision) {
		WarpEffect* warp_effect = nullptr;

		if (shipp->flags[Ship::Ship_Flags::Depart_warp] && shipp->warpout_effect != nullptr) 
			warp_effect = shipp->warpout_effect;
		else if (shipp->flags[Ship::Ship_Flags::Arriving_stage_2] && shipp->warpin_effect != nullptr)
			warp_effect = shipp->warpin_effect;

		bool hull_no_collide, shield_no_collide;
		hull_no_collide = shield_no_collide = false;
		if (warp_effect != nullptr) {
			hull_no_collide = point_is_clipped_by_warp(&mc_hull.hit_point_world, warp_effect);
			shield_no_collide = point_is_clipped_by_warp(&mc_shield.hit_point_world, warp_effect);
		}

		if (hull_no_collide)
			hull_collision = 0;
		if (shield_no_collide)
			shield_collision = 0;
	}

	if (shield_collision) {
		// pick out the shield quadrant
		quadrant_num = get_quadrant(&mc_shield.hit_point, ship_objp);

		// make sure that the shield is active in that quadrant
		if (shipp->flags[Ship::Ship_Flags::Dying] || !ship_is_shield_up(ship_objp, quadrant_num)) {
			quadrant_num = -1;
			shield_collision = 0;
		}

		// see if we hit the shield
		if (quadrant_num >= 0) {
			// do the hit effect
			if ( mc_shield.shield_hit_tri != -1 && (mc_shield.hit_dist*(flFrametime + time_limit) - flFrametime) < 0.0f ) {
				add_shield_point(OBJ_INDEX(ship_objp), mc_shield.shield_hit_tri, &mc_shield.hit_point);
			}

			// if this weapon pierces the shield, then do the hit effect, but act like a shield collision never occurred;
			// otherwise, we have a valid hit on this shield
			if (wip->wi_flags[Weapon::Info_Flags::Pierce_shields]) {
				quadrant_num = -1;
				shield_collision = 0;
			}
		}
	}

	// see which impact we use
	if (shield_collision)
	{
		memcpy(&mc, &mc_shield, sizeof(mc_info));
		Assert(quadrant_num >= 0);
		valid_hit_occurred = 1;
	}
	else if (hull_collision)
	{
		memcpy(&mc, &mc_hull, sizeof(mc_info));
		valid_hit_occurred = 1;
	}

	// deal with predictive collisions.  Find their actual hit time and see if they occured in current frame
	if (next_hit && valid_hit_occurred) {
		// find hit time
		*next_hit = (int) (1000.0f * (mc.hit_dist*(flFrametime + time_limit) - flFrametime) );
		if (*next_hit > 0)
			// if hit occurs outside of this frame, do not do damage 
			return 1;
	}

	if ( valid_hit_occurred )
	{
		wp->collisionInfo = new mc_info;	// The weapon will free this memory later
		memcpy(wp->collisionInfo, &mc, sizeof(mc_info));

		bool ship_override = false, weapon_override = false;

		if (Script_system.IsActiveAction(CHA_COLLIDEWEAPON)) {
			Script_system.SetHookObjects(4, "Self", ship_objp, "Object", weapon_objp, "Ship", ship_objp, "Weapon", weapon_objp);
			Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(mc.hit_point_world));
			ship_override = Script_system.IsConditionOverride(CHA_COLLIDEWEAPON, ship_objp, weapon_objp);
			Script_system.RemHookVars({ "Self", "Object", "Ship", "Weapon", "Hitpos" });
		}

		if (scripting::hooks::OnShipCollision->isActive()) {
			weapon_override = scripting::hooks::OnShipCollision->isOverride(
				scripting::hook_param_list(scripting::hook_param("Self", 'o', weapon_objp),
					scripting::hook_param("Object", 'o', ship_objp),
					scripting::hook_param("Ship", 'o', ship_objp),
					scripting::hook_param("Weapon", 'o', weapon_objp),
					scripting::hook_param("Hitpos", 'o', mc.hit_point_world)),
				weapon_objp, ship_objp);
		}

		if(!ship_override && !weapon_override) {
			if (shield_collision && quadrant_num >= 0) {
				if ((sip->shield_impact_explosion_anim > -1) && (wip->shield_impact_explosion_radius > 0)) {
					shield_impact_explosion(&mc.hit_point, ship_objp, wip->shield_impact_explosion_radius, sip->shield_impact_explosion_anim);
				}
			}
			ship_weapon_do_hit_stuff(ship_objp, weapon_objp, &mc.hit_point_world, &mc.hit_point, quadrant_num, mc.hit_submodel, mc.hit_normal);
		}

		if (Script_system.IsActiveAction(CHA_COLLIDEWEAPON) && !(weapon_override && !ship_override))
		{
			Script_system.SetHookObjects(4, "Self", ship_objp, "Object", weapon_objp, "Ship", ship_objp, "Weapon", weapon_objp);
			Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(mc.hit_point_world));
			Script_system.RunCondition(CHA_COLLIDEWEAPON, ship_objp, weapon_objp);
			Script_system.RemHookVars({ "Self", "Object", "Ship", "Weapon", "Hitpos" });
		}

		if (scripting::hooks::OnShipCollision->isActive() && ((weapon_override && !ship_override) || (!weapon_override && !ship_override)))
		{
			scripting::hooks::OnShipCollision->run(
				scripting::hook_param_list(scripting::hook_param("Self", 'o', weapon_objp),
					scripting::hook_param("Object", 'o', ship_objp),
					scripting::hook_param("Ship", 'o', ship_objp),
					scripting::hook_param("Weapon", 'o', weapon_objp),
					scripting::hook_param("Hitpos", 'o', mc.hit_point_world)),
				weapon_objp, ship_objp);
		}
	}
	else if ((Missiontime - wp->creation_time > F1_0/2) && (wip->is_homing()) && (wp->homing_object == ship_objp)) {
		if (dist < wip->shockwave.inner_rad) {
			vec3d	vec_to_ship;

			vm_vec_normalized_dir(&vec_to_ship, &ship_objp->pos, &weapon_objp->pos);

			if (vm_vec_dot(&vec_to_ship, &weapon_objp->orient.vec.fvec) < 0.0f) {
				// check if we're colliding against "invisible" ship
				if (!(shipp->flags[Ship::Ship_Flags::Dont_collide_invis])) {
					wp->lifeleft = 0.001f;
					wp->weapon_flags.set(Weapon::Weapon_Flags::Begun_detonation);

					if (ship_objp == Player_obj)
						nprintf(("Jim", "Frame %i: Weapon %d set to detonate, dist = %7.3f.\n", Framecount, OBJ_INDEX(weapon_objp), dist));
					valid_hit_occurred = 1;
				}
			}

		}
	}

	return valid_hit_occurred;
}


/**
 * Checks ship-weapon collisions.  
 * @param pair obj_pair pointer to the two objects. pair->a is ship and pair->b is weapon.
 * @return 1 if all future collisions between these can be ignored
 */
int collide_ship_weapon( obj_pair * pair )
{
	int		did_hit;
	object *ship = pair->a;
	object *weapon_obj = pair->b;
	
	Assert( ship->type == OBJ_SHIP );
	Assert( weapon_obj->type == OBJ_WEAPON );

	ship_info *sip = &Ship_info[Ships[ship->instance].ship_info_index];

	// Cyborg17 - no ship-ship collisions when doing multiplayer rollback
	if ( (Game_mode & GM_MULTIPLAYER) && multi_ship_record_get_rollback_wep_mode() && (weapon_obj->parent_sig == OBJ_INDEX(ship)) ) {
		return 0;
	}

	// Don't check collisions for player if past first warpout stage.
	if ( Player->control_mode > PCM_WARPOUT_STAGE1)	{
		if ( ship == Player_obj )
			return 0;
	}

	if (reject_due_collision_groups(ship, weapon_obj))
		return 0;

	// Cull lasers within big ship spheres by casting a vector forward for (1) exit sphere or (2) lifetime of laser
	// If it does hit, don't check the pair until about 200 ms before collision.  
	// If it does not hit and is within error tolerance, cull the pair.

	if ( (sip->is_big_or_huge()) && (weapon_obj->phys_info.flags & PF_CONST_VEL) ) {
		// Check when within ~1.1 radii.  
		// This allows good transition between sphere checking (leaving the laser about 200 ms from radius) and checking
		// within the sphere with little time between.  There may be some time for "small" big ships
		// Note: culling ships with auto spread shields seems to waste more performance than it saves,
		// so we're not doing that here
		if ( !(sip->flags[Ship::Info_Flags::Auto_spread_shields]) && vm_vec_dist_squared(&ship->pos, &weapon_obj->pos) < (1.2f*ship->radius*ship->radius) ) {
			return check_inside_radius_for_big_ships( ship, weapon_obj, pair );
		}
	}

	did_hit = ship_weapon_check_collision( ship, weapon_obj );

	if ( !did_hit )	{
		// Since we didn't hit, check to see if we can disable all future collisions
		// between these two.
		return weapon_will_never_hit( weapon_obj, ship, pair );
	}

	return 0;
}

/**
 * Upper limit estimate ship speed at end of time
 */
static float estimate_ship_speed_upper_limit( object *ship, float time ) 
{
	float exponent;
	float delta_v;
	float factor;

	delta_v = Ship_info[Ships[ship->instance].ship_info_index].max_vel.xyz.z - ship->phys_info.speed;
	
	if (ship->phys_info.forward_accel_time_const == 0) {
		return ship->phys_info.speed;
	}
	
	exponent = time / ship->phys_info.forward_accel_time_const;

	factor = 1.0f - (float)exp( -exponent );
	return ship->phys_info.speed + factor*delta_v;
}

// maximum error allowed in detecting collisions between laser and big ship inside the radius
// this is set for a 150 m radius ship.  For ships with larger radius, the error scales according
// to the ration of radii, but is never less than 2 m
#define ERROR_STD	2	

/**
 * When inside radius of big ship, check if we can cull collision pair determine the time when pair should next be checked
 * @return 1 if pair can be culled
 * @return 0 if pair can not be culled
 */
static int check_inside_radius_for_big_ships( object *ship, object *weapon_obj, obj_pair *pair )
{
	vec3d error_vel;		// vel perpendicular to laser
	float error_vel_mag;	// magnitude of error_vel
	float time_to_max_error, time_to_exit_sphere;
	float ship_speed_at_exit_sphere, error_at_exit_sphere;	
	float max_error = (float) ERROR_STD / 150.0f * ship->radius;
	if (max_error < 2)
		max_error = 2.0f;

	float weapon_max_vel = weapon_obj->phys_info.max_vel.xyz.z;
	Assertion(IS_VEC_NULL(&The_mission.gravity) || weapon_obj->phys_info.gravity_const == 0.0f, "check_inside_radius_for_big_ships being used for a ballistic weapon");

	time_to_exit_sphere = (ship->radius + vm_vec_dist(&ship->pos, &weapon_obj->pos)) / (weapon_max_vel - ship->phys_info.max_vel.xyz.z);
	ship_speed_at_exit_sphere = estimate_ship_speed_upper_limit( ship, time_to_exit_sphere );
	// update estimated time to exit sphere
	time_to_exit_sphere = (ship->radius + vm_vec_dist(&ship->pos, &weapon_obj->pos)) / (weapon_max_vel - ship_speed_at_exit_sphere);
	vm_vec_scale_add( &error_vel, &ship->phys_info.vel, &weapon_obj->orient.vec.fvec, -vm_vec_dot(&ship->phys_info.vel, &weapon_obj->orient.vec.fvec) );
	error_vel_mag = vm_vec_mag_quick( &error_vel );
	error_vel_mag += 0.5f * (ship->phys_info.max_vel.xyz.z - error_vel_mag)*(time_to_exit_sphere/ship->phys_info.forward_accel_time_const);
	// error_vel_mag is now average velocity over period
	error_at_exit_sphere = error_vel_mag * time_to_exit_sphere;
	time_to_max_error = max_error / error_at_exit_sphere * time_to_exit_sphere;

	// find the minimum time we can safely check into the future.
	// limited by (1) time to exit sphere (2) time to weapon expires
	// if ship_weapon_check_collision comes back with a hit_time > error limit, ok
	// if ship_weapon_check_collision comes finds no collision, next check time based on error time
	float limit_time;		// furthest time to check (either lifetime or exit sphere)
	if ( time_to_exit_sphere < Weapons[weapon_obj->instance].lifeleft ) {
		limit_time = time_to_exit_sphere;
	} else {
		limit_time = Weapons[weapon_obj->instance].lifeleft;
	}

	// Note:  when estimated hit time is less than 200 ms, look at every frame
	int hit_time;	// estimated time of hit in ms

	// modify ship_weapon_check_collision to do damage if hit_time is negative (ie, hit occurs in this frame)
	if ( ship_weapon_check_collision( ship, weapon_obj, limit_time, &hit_time ) ) {
		// hit occured in while in sphere
		if (hit_time < 0) {
			// hit occured in the frame
			return 1;
		} else if (hit_time > 200) {
			pair->next_check_time = timestamp(hit_time - 200);
			return 0;
			// set next check time to time - 200
		} else {
			// set next check time to next frame
			pair->next_check_time = 1;
			return 0;
		}
	} else {
		if (limit_time > time_to_max_error) {
		// no hit, but beyond error tolerance
			if (1000*time_to_max_error > 200) {
				pair->next_check_time = timestamp( (int)(1000*time_to_max_error) - 200 );
			} else {
				pair->next_check_time = 1;
			}
			return 0;
		} else {
			// no hit and within error tolerance
			return 1;
		}
	}
}
