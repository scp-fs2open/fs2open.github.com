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
#include "debugconsole/console.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudmessage.h"
#include "hud/hudshield.h"
#include "io/joy_ff.h"
#include "io/timer.h"
#include "network/multi.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "object/objectshield.h"
#include "scripting/scripting.h"
#include "scripting/api/objs/vecmath.h"
#include "playerman/player.h"
#include "render/3d.h"			// needed for View_position, which is used when playing 3d sound
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"


#define COLLIDE_DEBUG
#undef  COLLIDE_DEBUG

// GENERAL COLLISIONS FUNCTIONS
// calculates the inverse moment of inertia matrix in world coordinates
static void get_I_inv (matrix* I_inv, matrix* I_inv_body, matrix* orient);

// calculate the physics of extended two body collisions
void calculate_ship_ship_collision_physics(collision_info_struct *ship_ship_hit_info);

#ifndef NDEBUG
static int Collide_friendly = 1;
DCF_BOOL( collide_friendly, Collide_friendly )
#endif

static sound_handle Player_collide_sound, AI_collide_sound;
static sound_handle Player_collide_shield_sound, AI_collide_shield_sound;

/**
 * Return true if two ships are docking or if one of the two is indirectly docking to the other.
 */
static bool check_for_docking_collision(object *objp1, object *objp2)
{
	ai_info	*aip1, *aip2;
	ship		*shipp1, *shipp2;

	shipp1 = &Ships[objp1->instance];
	shipp2 = &Ships[objp2->instance];

	aip1 = &Ai_info[shipp1->ai_index];
	aip2 = &Ai_info[shipp2->ai_index];

	if (dock_check_find_docked_object(objp1, objp2)) {
		return true;
	}


	if (aip1->mode == AIM_DOCK && aip1->goal_objnum >= 0 && aip1->goal_objnum < MAX_OBJECTS) {
		if (dock_check_find_docked_object(&Objects[aip1->goal_objnum], objp2))
			return true;
	}

	if (aip2->mode == AIM_DOCK && aip2->goal_objnum >= 0 && aip2->goal_objnum < MAX_OBJECTS) {
		if (dock_check_find_docked_object(&Objects[aip2->goal_objnum], objp1))
			return true;
	}

	return false;

}

/**
 * If light_obj emerging from or departing to dock bay in heavy_obj, no collision detection.
 */
static int bay_emerge_or_depart(object *heavy_objp, object *light_objp)
{
	if (light_objp->type != OBJ_SHIP)
		return 0;

	ai_info	*aip = &Ai_info[Ships[light_objp->instance].ai_index];

	// the player shouldn't be allowed to fly through the ship just cause the rest of their wing can
	if ((Player_obj == light_objp) && !Player_use_ai){
		return 0;
	}

	if ((aip->mode == AIM_BAY_EMERGE) || (aip->mode == AIM_BAY_DEPART)) {
		if (aip->goal_objnum == OBJ_INDEX(heavy_objp))
			return 1;
	}

	return 0;
}

int ship_ship_check_collision(collision_info_struct *ship_ship_hit_info)
{
	object *heavy_obj	= ship_ship_hit_info->heavy;
	object *light_obj = ship_ship_hit_info->light;
	int	player_involved;	// flag to indicate that A or B is the Player_obj

	Assert( heavy_obj->type == OBJ_SHIP );
	Assert( light_obj->type == OBJ_SHIP );

	ship *heavy_shipp = &Ships[heavy_obj->instance];
	ship *light_shipp = &Ships[light_obj->instance];

	ship_info *heavy_sip = &Ship_info[heavy_shipp->ship_info_index];
    ship_info *light_sip = &Ship_info[light_shipp->ship_info_index];

	// AL 12-4-97: we use the player_involved flag to ensure collisions are always
	//             done with the player, regardless of team.
	if ( heavy_obj == Player_obj || light_obj == Player_obj ) {
		player_involved = 1;
	} else {
		player_involved = 0;
	}

	// Make ships that are warping in not get collision detection done
	if ( heavy_shipp->is_arriving(ship::warpstage::STAGE1, false) ||
		 light_shipp->is_arriving(ship::warpstage::STAGE1, false)) {
		return 0;
	}

	// Don't do collision detection for docking ships, since they will always collide while trying to dock
	if (check_for_docking_collision(heavy_obj, light_obj) ) {
		return 0;
	}

	//	If light_obj emerging from or departing to dock bay in heavy_obj, no collision detection.
	if (bay_emerge_or_depart(heavy_obj, light_obj)) {
		return 0;
	}

	//	Ships which are dying should not do collision detection.
	//	Also, this is the only clean way I could figure to get ships to not do damage to each other for one frame
	//	when they are docked and departing.  Due to sequencing, they would not show up as docked, yet they
	//	would still come through here, so they would harm each other, if on opposing teams. -- MK, 2/2/98
	if ((heavy_obj->flags[Object::Object_Flags::Should_be_dead]) || (light_obj->flags[Object::Object_Flags::Should_be_dead])) {
		return 0;
	}

#ifndef NDEBUG
	//	Don't do collision detection on a pair of ships on the same team.
	//	Change this someday, but for now, it's a problem.
	if ( !Collide_friendly ) {		// Collide_friendly is a global value changed via debug console
		if ( (!player_involved) && (heavy_shipp->team == light_shipp->team) ) {
			return 0;
		}
	}
#endif

	//	Apparently we're doing same team collisions.
	//	But, if both are offscreen, ignore the collision
	if (heavy_shipp->team == light_shipp->team) {
		if ( (!(heavy_obj->flags[Object::Object_Flags::Was_rendered]) && !(light_obj->flags[Object::Object_Flags::Was_rendered])) ) {
			return 0;
		}
	}

	// Set up model_collide info
	mc_info mc;
	mc_info_init(&mc);

	// Do in heavy object RF
	mc.model_num = heavy_sip->model_num;	// Fill in the model to check
	mc.model_instance_num = heavy_shipp->model_instance_num;
	mc.orient = &heavy_obj->orient;		// The object's orient

	vec3d zero, p0, p1;
	vm_vec_zero(&zero);		// we need the physical vector and can not set its value to zero
	vm_vec_sub(&p0, &light_obj->last_pos, &heavy_obj->last_pos);
	vm_vec_sub(&p1, &light_obj->pos, &heavy_obj->pos);

	// find the light object's position in the heavy object's reference frame at last frame and also in this frame.
	vec3d p0_temp, p0_rotated;
	
	// Collision detection from rotation enabled if at max rotaional velocity and 5fps, rotation is less than PI/2
	// This should account for all ships
	if ( (vm_vec_mag_squared( &heavy_obj->phys_info.max_rotvel ) * .04) < (PI*PI/4) ) {
		// collide_rotate calculate (1) start position and (2) relative velocity
		ship_ship_hit_info->collide_rotate = 1;
		vm_vec_rotate(&p0_temp, &p0, &heavy_obj->last_orient);
		vm_vec_unrotate(&p0_rotated, &p0_temp, &heavy_obj->orient);
		mc.p0 = &p0_rotated;				// Point 1 of ray to check
		vm_vec_sub(&ship_ship_hit_info->light_rel_vel, &p1, &p0_rotated);
		vm_vec_scale(&ship_ship_hit_info->light_rel_vel, 1/flFrametime);
	}
	// should be no ships that can rotate this fast
	else {
#ifndef NDEBUG
		static bool Warned_about_fast_rotational_collisions = false;
		if (!Warned_about_fast_rotational_collisions) {
			Warning(LOCATION, "Ship '%s' rotates too quickly!  Rotational collision detection has been skipped.", heavy_sip->name);
			Warned_about_fast_rotational_collisions = true;
		}
#endif
		ship_ship_hit_info->collide_rotate = 0;
		mc.p0 = &p0;							// Point 1 of ray to check
		vm_vec_sub(&ship_ship_hit_info->light_rel_vel, &light_obj->phys_info.vel, &heavy_obj->phys_info.vel);
	}
	
	// Set up collision info
	mc.pos = &zero;						// The object's position
	mc.p1 = &p1;							// Point 2 of ray to check
	mc.radius = model_get_core_radius(light_sip->model_num);
	mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);			// flags

	//	Only check invisible face polygons for ship:ship of different teams.
	if ( !(heavy_shipp->flags[Ship::Ship_Flags::Dont_collide_invis]) ) {
		if ((heavy_obj->flags[Object::Object_Flags::Player_ship]) || (light_obj->flags[Object::Object_Flags::Player_ship]) || (heavy_shipp->team != light_shipp->team) ) {
			mc.flags |= MC_CHECK_INVISIBLE_FACES;
		}
	}
	
	// copy important data
	int copy_flags = mc.flags;  // make a copy of start end positions of sphere in  big ship RF
	vec3d copy_p0, copy_p1;
	copy_p0 = *mc.p0;
	copy_p1 = *mc.p1;

	// first test against the sphere - if this fails then don't do any submodel tests
	mc.flags = MC_ONLY_SPHERE | MC_CHECK_SPHERELINE;

	int valid_hit_occured = 0;
	polymodel *pm_light;
		
	pm_light = model_get(Ship_info[light_shipp->ship_info_index].model_num);

	if(pm_light->submodel[pm_light->detail[0]].no_collisions) {
		return 0;
	}

	if (model_collide(&mc)) {

		// Set earliest hit time
		ship_ship_hit_info->hit_time = FLT_MAX;

		auto pmi = model_get_instance(heavy_shipp->model_instance_num);
		auto pm = model_get(pmi->model_num);

		// Do collision the cool new way
		if ( ship_ship_hit_info->collide_rotate ) {
			// We collide with the sphere, find the list of rotating submodels and test one at a time
			SCP_vector<int> submodel_vector;
			model_get_rotating_submodel_list(&submodel_vector, heavy_obj);

			// turn off all rotating submodels, collide against only 1 at a time.
			// turn off collision detection for all rotating submodels
			for (auto submodel : submodel_vector) {
				pmi->submodel[submodel].collision_checked = true;
			}

			// reset flags to check MC_CHECK_MODEL | MC_CHECK_SPHERELINE and maybe MC_CHECK_INVISIBLE_FACES and MC_SUBMODEL_INSTANCE
			mc.flags = copy_flags | MC_SUBMODEL_INSTANCE;

			if (heavy_sip->collision_lod > -1) {
				mc.lod = heavy_sip->collision_lod;
			}

			// check each submodel in turn
			for (auto submodel : submodel_vector) {
				auto smi = &pmi->submodel[submodel];

				// turn on just one submodel for collision test
				smi->collision_checked = false;

				if (smi->blown_off)
				{
					smi->collision_checked = true;
					continue;
				}

				// set angles for last frame
				matrix copy_matrix = smi->canonical_orient;

				// find the start and end positions of the sphere in submodel RF
				smi->canonical_orient = smi->canonical_prev_orient;
				world_find_model_instance_point(&p0, &light_obj->last_pos, pm, pmi, submodel, &heavy_obj->last_orient, &heavy_obj->last_pos);

				smi->canonical_orient = copy_matrix;
				world_find_model_instance_point(&p1, &light_obj->pos, pm, pmi, submodel, &heavy_obj->orient, &heavy_obj->pos);

				mc.p0 = &p0;
				mc.p1 = &p1;

				mc.orient = &vmd_identity_matrix;
				mc.submodel_num = submodel;

				if ( model_collide(&mc) ) {
					if (mc.hit_dist < ship_ship_hit_info->hit_time ) {
						valid_hit_occured = 1;

						// set up ship_ship_hit_info common
						set_hit_struct_info(ship_ship_hit_info, &mc, SUBMODEL_ROT_HIT);
						model_instance_find_world_point(&ship_ship_hit_info->hit_pos, &mc.hit_point, pm, pmi, mc.hit_submodel, &heavy_obj->orient, &zero);

						// set up ship_ship_hit_info for rotating submodel
						if (ship_ship_hit_info->edge_hit == 0) {
							model_instance_find_world_dir(&ship_ship_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
						}

						// find position in submodel RF of light object at collison
						vec3d int_light_pos, diff;
						vm_vec_sub(&diff, mc.p1, mc.p0);
						vm_vec_scale_add(&int_light_pos, mc.p0, &diff, mc.hit_dist);
						model_instance_find_world_point(&ship_ship_hit_info->light_collision_cm_pos, &int_light_pos, pm, pmi, mc.hit_submodel, &heavy_obj->orient, &zero);
					}
				}
			}

		}

		// Now complete base model collision checks that do not take into account rotating submodels.
		mc.flags = copy_flags;
		*mc.p0 = copy_p0;
		*mc.p1 = copy_p1;
		mc.orient = &heavy_obj->orient;

		// usual ship_ship collision test
		if ( model_collide(&mc) )	{
			// check if this is the earliest hit
			if (mc.hit_dist < ship_ship_hit_info->hit_time) {
				valid_hit_occured = 1;

				set_hit_struct_info(ship_ship_hit_info, &mc, SUBMODEL_NO_ROT_HIT);

				// get collision normal if not edge hit
				if (ship_ship_hit_info->edge_hit == 0) {
					model_instance_find_world_dir(&ship_ship_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
				}

				// find position in submodel RF of light object at collison
				vec3d diff;
				vm_vec_sub(&diff, mc.p1, mc.p0);
				vm_vec_scale_add(&ship_ship_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);
			}
		}
	}
	
	// check if the hit point is beyond the clip plane if one of the ships is warping in or out.
	if (valid_hit_occured) {
		WarpEffect* warp_effect = nullptr;

		// this is extremely confusing but mc.hit_point_world isn't actually in world coords
		// everything above was calculated relative to the heavy's position
		vec3d actual_world_hit_pos = mc.hit_point_world + heavy_obj->pos;

		if (heavy_shipp->flags[Ship::Ship_Flags::Depart_warp] && heavy_shipp->warpout_effect != nullptr)
			warp_effect = heavy_shipp->warpout_effect;
		else if (heavy_shipp->flags[Ship::Ship_Flags::Arriving_stage_2] && heavy_shipp->warpin_effect != nullptr)
			warp_effect = heavy_shipp->warpin_effect;

		bool heavy_warp_no_collide = false;
		if (warp_effect != nullptr)
			heavy_warp_no_collide = point_is_clipped_by_warp(&actual_world_hit_pos, warp_effect);

		warp_effect = nullptr;
		if (light_shipp->flags[Ship::Ship_Flags::Depart_warp] && light_shipp->warpout_effect != nullptr)
			warp_effect = light_shipp->warpout_effect;
		else if (light_shipp->flags[Ship::Ship_Flags::Arriving_stage_2] && light_shipp->warpin_effect != nullptr)
			warp_effect = light_shipp->warpin_effect;

		bool light_warp_no_collide = false;
		if (warp_effect != nullptr)
			light_warp_no_collide = point_is_clipped_by_warp(&actual_world_hit_pos, warp_effect);

		
		if (heavy_warp_no_collide || light_warp_no_collide)
			valid_hit_occured = 0;
	}

	if (valid_hit_occured) {

		// Collision debug stuff
#ifndef NDEBUG
		object *collide_obj = NULL;
		if (heavy_obj == Player_obj) {
			collide_obj = light_obj;
		} else if (light_obj == Player_obj) {
			collide_obj = heavy_obj;
		}
		if ((collide_obj != NULL) && (Ship_info[Ships[collide_obj->instance].ship_info_index].is_fighter_bomber())) {
			const char	*submode_string = "";
			ai_info	*aip;

			extern const char *Mode_text[];
			aip = &Ai_info[Ships[collide_obj->instance].ai_index];

			if (aip->mode == AIM_CHASE)
				submode_string = Submode_text[aip->submode];

			nprintf(("AI", "Player collided with ship %s, AI mode = %s, submode = %s\n", Ships[collide_obj->instance].ship_name, Mode_text[aip->mode], submode_string));
		}
#endif
	}


	return valid_hit_occured;
}

/**
 * Gets modified mass of cruiser in cruiser/asteroid collision so cruisers don't get bumped so hard.
 * modified mass is 10x, 4x, or 2x larger than asteroid mass
 * @return 1 if modified mass is larger than given mass, 0 otherwise 
 */
static int check_special_cruiser_asteroid_collision(object *heavy, object *lighter, float *cruiser_mass, int *cruiser_light)
{
	int asteroid_type;

	if (heavy->type == OBJ_ASTEROID) {
		Assert(lighter->type == OBJ_SHIP);
		if (Ship_info[Ships[lighter->instance].ship_info_index].is_big_or_huge()) {

			asteroid_type = Asteroids[heavy->instance].asteroid_type;
			if (asteroid_type == 0) {
				*cruiser_mass = 10.0f * heavy->phys_info.mass;
			} else if (asteroid_type == 1) {
				*cruiser_mass = 4.0f * heavy->phys_info.mass;
			} else {
				*cruiser_mass = 2.0f * heavy->phys_info.mass;
			}

			if (*cruiser_mass > lighter->phys_info.mass) {
				*cruiser_light = 1;
				return 1;
			}
		}
	} else if (lighter->type == OBJ_ASTEROID) {
		Assert(heavy->type == OBJ_SHIP);
		if (Ship_info[Ships[heavy->instance].ship_info_index].is_big_or_huge()) {

			asteroid_type = Asteroids[lighter->instance].asteroid_type;
			if (asteroid_type == 0) {
				*cruiser_mass = 10.0f * lighter->phys_info.mass;
			} else if (asteroid_type == 1) {
				*cruiser_mass = 4.0f * lighter->phys_info.mass;
			} else {
				*cruiser_mass = 2.0f * lighter->phys_info.mass;
			}

			if (*cruiser_mass > heavy->phys_info.mass) {
				*cruiser_light = 0;
				return 1;
			}
		}
	}
	return 0;
}

/**
 * Find the subobject corresponding to the submodel hit
 */
static bool check_subsystem_landing_allowed(ship_info *heavy_sip, collision_info_struct *ship_ship_hit_info) {
	if (!(heavy_sip->flags[Ship::Info_Flags::Allow_landings]))
		return false;

	for (int i = 0; i < heavy_sip->n_subsystems; i++) {
		if (heavy_sip->subsystems[i].flags[Model::Subsystem_Flags::Allow_landing] &&
			heavy_sip->subsystems[i].subobj_num == ship_ship_hit_info->submodel_num) 
		{
			return true;
		}
	}
	return false;
}

// ------------------------------------------------------------------------------------------------
//		input:		ship_ship_hit		=>		structure containing ship_ship hit info
//		(includes)	A, B					=>		objects colliding
//						r_A, r_B				=>		position to collision from center of mass
//						collision_normal	=>		collision_normal (outward from B)			
//
//		output:	velocity, angular velocity, impulse
//
// ------------------------------------------------------------------------------------------------
//
// calculates correct physics response to collision between two objects given
//		masses, moments of inertia, velocities, angular velocities, 
//		relative collision positions, and the impulse direction
//
void calculate_ship_ship_collision_physics(collision_info_struct *ship_ship_hit_info)
{
	// important parameters passed thru ship_ship_or_debris_hit
	// calculate the whack applied to each ship from collision

	// make local copies of hit_struct parameters
	object *heavy = ship_ship_hit_info->heavy;
	object *lighter = ship_ship_hit_info->light;

	// gurgh... this includes asteroids and debris too
	Assert(heavy->type == OBJ_SHIP || heavy->type == OBJ_ASTEROID || heavy->type == OBJ_DEBRIS);
	Assert(lighter->type == OBJ_SHIP || lighter->type == OBJ_ASTEROID || lighter->type == OBJ_DEBRIS);

	ship_info *light_sip = (lighter->type == OBJ_SHIP) ? &Ship_info[Ships[lighter->instance].ship_info_index] : NULL;
	ship_info *heavy_sip = (heavy->type == OBJ_SHIP) ? &Ship_info[Ships[heavy->instance].ship_info_index] : NULL;

	// make cruiser/asteroid collision softer on cruisers.
	int special_cruiser_asteroid_collision;
	int cruiser_light = 0;
	float cruiser_mass = 0.0f, copy_mass = 0.0f;
	special_cruiser_asteroid_collision = check_special_cruiser_asteroid_collision(heavy, lighter, &cruiser_mass, &cruiser_light);

	if (special_cruiser_asteroid_collision) {
		if (cruiser_light) {
			Assert(lighter->phys_info.mass < cruiser_mass);
			copy_mass = lighter->phys_info.mass;
			lighter->phys_info.mass = cruiser_mass;
		} else {
			Assert(heavy->phys_info.mass < cruiser_mass);
			copy_mass = heavy->phys_info.mass;
			heavy->phys_info.mass = cruiser_mass;
		}
	}

	float		coeff_restitution;	// parameter controls amount of bounce
	float		v_rel_normal_m;		// relative collision velocity in the direction of the collision normal
	vec3d	v_rel_parallel_m;		// normalized v_rel (Va-Vb) projected onto collision surface
	vec3d	world_rotvel_heavy_m, world_rotvel_light_m, vel_from_rotvel_heavy_m, vel_from_rotvel_light_m, v_rel_m, vel_heavy_m, vel_light_m;

	coeff_restitution = 0.1f;		// relative velocity wrt normal is zero after the collision ( range 0-1 )

	// find velocity of each obj at collision point

	// heavy object is in cm reference frame so we don't get a v_heavy term.
	if ( ship_ship_hit_info->collide_rotate ) {
		// if we have collisions from rotation, the effective velocity from rotation of the large body is alreay taken account
		vm_vec_zero( &vel_heavy_m );
	} else {
		// take account the effective velocity from rotation
		vm_vec_unrotate(&world_rotvel_heavy_m, &heavy->phys_info.rotvel, &heavy->orient);	// heavy's world rotvel before collision
		vm_vec_cross(&vel_from_rotvel_heavy_m, &world_rotvel_heavy_m, &ship_ship_hit_info->r_heavy);	// heavy's velocity from rotvel before collision
		vel_heavy_m = vel_from_rotvel_heavy_m;
	}

	// if collision from rotating submodel of heavy obj, add in vel from rotvel of submodel
	vec3d local_vel_from_submodel;

	if (ship_ship_hit_info->submodel_rot_hit == 1) {
		polymodel *pm;
		polymodel_instance *pmi = NULL;
		int model_instance_num = -1;

		if (heavy->type == OBJ_SHIP) {
			pm = model_get(heavy_sip->model_num);
			model_instance_num = Ships[heavy->instance].model_instance_num;
			pmi = model_get_instance(model_instance_num);
		} else if (heavy->type == OBJ_ASTEROID) {
			pm = Asteroid_info[Asteroids[heavy->instance].asteroid_type].modelp[Asteroids[heavy->instance].asteroid_subtype];
			model_instance_num = Asteroids[heavy->instance].model_instance_num;
			pmi = model_get_instance(model_instance_num);
		} else if (heavy->type == OBJ_DEBRIS) {
			pm = model_get(Debris[heavy->instance].model_num);
		} else {
			// we should have caught this already
			Int3();
			pm = NULL;
		}
		
		if ( pmi != nullptr ) {
			auto smi = &pmi->submodel[ship_ship_hit_info->submodel_num];

			// set point on axis of rotating submodel if not already set.
			if ( !smi->axis_set ) {
				model_init_submodel_axis_pt(pm, pmi, ship_ship_hit_info->submodel_num);
			}

			vec3d omega, r_rot;

			// get world rotational velocity of rotating submodel
			model_instance_find_world_dir(&omega, &pm->submodel[ship_ship_hit_info->submodel_num].movement_axis, pm, pmi, ship_ship_hit_info->submodel_num, &heavy->orient);

			vm_vec_scale(&omega, smi->current_turn_rate);

			// world coords for r_rot
			vec3d temp;
			vm_vec_unrotate(&temp, &smi->point_on_axis, &heavy->orient);
			vm_vec_sub(&r_rot, &ship_ship_hit_info->hit_pos, &temp);

			vm_vec_cross(&local_vel_from_submodel, &omega, &r_rot);
		} else {
			vm_vec_zero(&local_vel_from_submodel);
		}
	} else {
		// didn't collide with submodel
		vm_vec_zero(&local_vel_from_submodel);
	}

	vm_vec_unrotate(&world_rotvel_light_m, &lighter->phys_info.rotvel, &lighter->orient);		// light's world rotvel before collision
	vm_vec_cross(&vel_from_rotvel_light_m, &world_rotvel_light_m, &ship_ship_hit_info->r_light);	// light's velocity from rotvel before collision
	vm_vec_add(&vel_light_m, &vel_from_rotvel_light_m, &ship_ship_hit_info->light_rel_vel);
	vm_vec_sub(&v_rel_m, &vel_light_m, &vel_heavy_m);

	// Add in effect of rotating submodel
	vm_vec_sub2(&v_rel_m, &local_vel_from_submodel);

	v_rel_normal_m = vm_vec_dot(&v_rel_m, &ship_ship_hit_info->collision_normal);// if less than zero, colliding contact taking place
																									// (v_slow - v_fast) dot (n_fast)

	if (v_rel_normal_m > 0) {
	//	This can happen in 2 situations.
	// (1) The rotational velocity is large enough to cause ships to miss.  In this case, there would most likely
	// have been a collision, but at a later time, so reset v_rel_normal_m 

	//	(2) We could also have just gotten a slightly incorrect hitpos, where r dot v_rel is nearly zero.  
	//	In this case, we know there was a collision, but slight collision and the normal is correct, so reset v_rel_normal_m
	//	need a normal direction.  We can just take the -v_light normalized.		v_rel_normal_m = -v_rel_normal_m;
		
		nprintf(("Physics", "Frame %i reset v_rel_normal_m %f Edge %i\n", Framecount, v_rel_normal_m, ship_ship_hit_info->edge_hit));
		v_rel_normal_m = -v_rel_normal_m;
	}

	//Maybe treat the current collision as a landing
	//Init values just to be safe
	vec3d light_local_vel(ship_ship_hit_info->light_rel_vel);
	float light_uvec_dot_norm = 0.0f;
	float light_fvec_dot_norm = 0.0f;
	float light_rvec_dot_norm = 0.0f;
	bool subsys_landing_allowed = lighter->type == OBJ_SHIP && heavy->type == OBJ_SHIP && check_subsystem_landing_allowed(heavy_sip, ship_ship_hit_info);
	if (subsys_landing_allowed) {
		vm_vec_rotate(&light_local_vel, &ship_ship_hit_info->light_rel_vel, &lighter->orient);
		light_uvec_dot_norm = vm_vec_dot(&ship_ship_hit_info->collision_normal, &lighter->orient.vec.uvec);
		light_fvec_dot_norm = vm_vec_dot(&ship_ship_hit_info->collision_normal, &lighter->orient.vec.fvec);
		light_rvec_dot_norm = vm_vec_dot(&ship_ship_hit_info->collision_normal, &lighter->orient.vec.rvec);
	}
	if (subsys_landing_allowed &&	
		light_local_vel.xyz.z < light_sip->collision_physics.landing_max_z &&
		light_local_vel.xyz.z > light_sip->collision_physics.landing_min_z &&
		light_local_vel.xyz.y > light_sip->collision_physics.landing_min_y &&
		fl_abs(light_local_vel.xyz.x) < light_sip->collision_physics.landing_max_x &&
		light_uvec_dot_norm > 0 &&
		light_fvec_dot_norm < light_sip->collision_physics.landing_max_angle &&
		light_fvec_dot_norm > light_sip->collision_physics.landing_min_angle &&
		fl_abs(light_rvec_dot_norm) < light_sip->collision_physics.landing_max_rot_angle)
	{
		ship_ship_hit_info->is_landing = true;
	}

	vec3d	rotational_impulse_heavy, rotational_impulse_light, delta_rotvel_heavy, delta_rotvel_light;
	vec3d	delta_vel_from_delta_rotvel_heavy, delta_vel_from_delta_rotvel_light, impulse;
	float		impulse_mag, heavy_denom, light_denom;
	matrix	heavy_I_inv, light_I_inv;

	// include a frictional collision impulse F parallel to the collision plane
	// F = I * sin (collision_normal, normalized v_rel_m)  [sin is ratio of v_rel_parallel_m to v_rel_m]
	// note:  (-) sign is needed to account for the direction of the v_rel_parallel_m
	if (IS_VEC_NULL(&v_rel_m)) {
		// If the relative velocity is zero then the compuatation below would cause NaN errors
		vm_vec_zero(&impulse);
	} else {
		float collision_speed_parallel;
		float parallel_mag;
		impulse = ship_ship_hit_info->collision_normal;
		vm_vec_projection_onto_plane(&v_rel_parallel_m, &v_rel_m, &ship_ship_hit_info->collision_normal);
		collision_speed_parallel = vm_vec_normalize_safe(&v_rel_parallel_m);
		float friction = (lighter->type == OBJ_SHIP) ? light_sip->collision_physics.friction : COLLISION_FRICTION_FACTOR;
		parallel_mag = float(-friction) * collision_speed_parallel / vm_vec_mag(&v_rel_m);
		vm_vec_scale_add2(&impulse, &v_rel_parallel_m, parallel_mag);
	}
	
	// calculate the effect on the velocity of the collison point per unit impulse
	// first find the effect thru change in rotvel
	// then find the change in the cm vel
	if (heavy == Player_obj) {
		vm_vec_zero( &delta_rotvel_heavy );
		heavy_denom = 1.0f / heavy->phys_info.mass;
	} else {
		vm_vec_cross(&rotational_impulse_heavy, &ship_ship_hit_info->r_heavy, &impulse);
		get_I_inv(&heavy_I_inv, &heavy->phys_info.I_body_inv, &heavy->orient);
		vm_vec_rotate(&delta_rotvel_heavy, &rotational_impulse_heavy, &heavy_I_inv);
		float rotation_factor = (heavy->type == OBJ_SHIP) ? heavy_sip->collision_physics.rotation_factor : COLLISION_ROTATION_FACTOR;
		vm_vec_scale(&delta_rotvel_heavy, rotation_factor);		// hack decrease rotation (delta_rotvel)
		vm_vec_cross(&delta_vel_from_delta_rotvel_heavy, &delta_rotvel_heavy , &ship_ship_hit_info->r_heavy);
		heavy_denom = vm_vec_dot(&delta_vel_from_delta_rotvel_heavy, &ship_ship_hit_info->collision_normal);
		if (heavy_denom < 0) {
			// sanity check
			heavy_denom = 0.0f;
		}
		heavy_denom += 1.0f / heavy->phys_info.mass;
	} 

	// calculate the effect on the velocity of the collison point per unit impulse
	// first find the effect thru change in rotvel
	// then find the change in the cm vel
	// SUSHI: If on a landing surface, use the same shortcut the player gets
	// This is a bit of a hack, but gets around some nasty unpredictable collision behavior
	// when trying to do AI landings for certain ships
	if (lighter == Player_obj || subsys_landing_allowed) {
		vm_vec_zero( &delta_rotvel_light );
		light_denom = 1.0f / lighter->phys_info.mass;
	} else {
		vm_vec_cross(&rotational_impulse_light, &ship_ship_hit_info->r_light, &impulse);
		get_I_inv(&light_I_inv, &lighter->phys_info.I_body_inv, &lighter->orient);
		vm_vec_rotate(&delta_rotvel_light, &rotational_impulse_light, &light_I_inv);
		float rotation_factor = (lighter->type == OBJ_SHIP) ? light_sip->collision_physics.rotation_factor : COLLISION_ROTATION_FACTOR;
		vm_vec_scale(&delta_rotvel_light, rotation_factor);		// hack decrease rotation (delta_rotvel)
		vm_vec_cross(&delta_vel_from_delta_rotvel_light, &delta_rotvel_light, &ship_ship_hit_info->r_light);
		light_denom = vm_vec_dot(&delta_vel_from_delta_rotvel_light, &ship_ship_hit_info->collision_normal);
		if (light_denom < 0) {
			// sanity check
			light_denom = 0.0f;
		}
		light_denom += 1.0f / lighter->phys_info.mass;
	} 

	// calculate the necessary impulse to achieved the desired relative velocity after the collision
	// update damage info in mc
	impulse_mag = -(1.0f + coeff_restitution)*v_rel_normal_m / (heavy_denom + light_denom);
	ship_ship_hit_info->impulse = impulse_mag;
	if (impulse_mag < 0) {
		nprintf(("Physics", "negative impulse mag -- Get Dave A if not Descent Physics\n"));
		impulse_mag = -impulse_mag;
	}

	// update the physics info structs for heavy and light objects
	// since we have already calculated delta rotvel for heavy and light in world coords
	// physics should not have to recalculate this, just change into body coords (done in collide_whack)
	vm_vec_scale(&impulse, impulse_mag);
	vm_vec_scale(&delta_rotvel_light, impulse_mag);	
	physics_collide_whack(&impulse, &delta_rotvel_light, &lighter->phys_info, &lighter->orient, ship_ship_hit_info->is_landing);
	vm_vec_negate(&impulse);
	vm_vec_scale(&delta_rotvel_heavy, -impulse_mag);
	physics_collide_whack(&impulse, &delta_rotvel_heavy, &heavy->phys_info, &heavy->orient, true);

	// If within certain bounds, we want to add some more rotation towards the "resting orientation" of the ship
	// These bounds are defined separately from normal "landing" bounds so that they can be more generous: 
	// we can have crash landings that still re-orient the ship.
	if (subsys_landing_allowed &&
		light_local_vel.xyz.z < light_sip->collision_physics.reorient_max_z  &&
		light_local_vel.xyz.z > light_sip->collision_physics.reorient_min_z &&
		light_local_vel.xyz.y > light_sip->collision_physics.reorient_min_y &&
		fl_abs(light_local_vel.xyz.x) < light_sip->collision_physics.reorient_max_x &&
		light_uvec_dot_norm > 0 &&
		light_fvec_dot_norm < light_sip->collision_physics.reorient_max_angle &&
		light_fvec_dot_norm > light_sip->collision_physics.reorient_min_angle &&
		fl_abs(light_rvec_dot_norm) < light_sip->collision_physics.reorient_max_rot_angle) 
	{
		vec3d landing_delta_rotvel;
		landing_delta_rotvel.xyz.x = (light_fvec_dot_norm * light_sip->collision_physics.reorient_mult) 
			- light_sip->collision_physics.landing_rest_angle;	
		// For yaw, use the dot product between vel vector (normalized) and orientation on the xz plane
		// This reduces to the following math
		// We also clamp to reduce huge nose swings at low speeds
		float xzVelMag = sqrt(light_local_vel.xyz.x * light_local_vel.xyz.x + light_local_vel.xyz.z * light_local_vel.xyz.z);
		float xzVelDotOrient = MIN(MAX((xzVelMag > 0 ? (light_local_vel.xyz.x / xzVelMag) : 0), -0.5f), 0.5f);
		landing_delta_rotvel.xyz.y = (xzVelMag > 2) ? (xzVelDotOrient * light_sip->collision_physics.reorient_mult) : 0;
		landing_delta_rotvel.xyz.z = light_rvec_dot_norm * light_sip->collision_physics.reorient_mult * -1;
		vm_vec_add2( &lighter->phys_info.rotvel, &landing_delta_rotvel );
	}

	// Find final positions
	// We will try not to worry about the left over time in the frame
	// heavy's position unchanged by collision
	// light's position is heavy's position plus relative position from heavy
	vm_vec_add(&lighter->pos, &heavy->pos, &ship_ship_hit_info->light_collision_cm_pos);

	// Try to move each body back to its position just before collision occured to prevent interpenetration
	// Move away in direction of light and away in direction of normal
	vec3d direction_light;	// direction light is moving relative to heavy
	vm_vec_sub(&direction_light, &ship_ship_hit_info->light_rel_vel, &local_vel_from_submodel);
	vm_vec_normalize_safe(&direction_light);

	Assert( !vm_is_vec_nan(&direction_light) );
	vm_vec_scale_add2(&heavy->pos, &direction_light,  0.2f * lighter->phys_info.mass / (heavy->phys_info.mass + lighter->phys_info.mass));
	vm_vec_scale_add2(&heavy->pos, &ship_ship_hit_info->collision_normal, -0.1f * lighter->phys_info.mass / (heavy->phys_info.mass + lighter->phys_info.mass));
	//For landings, we want minimal movement on the light ship (just enough to keep the collision detection honest)
	if (ship_ship_hit_info->is_landing) {
		vm_vec_scale_add2(&lighter->pos, &ship_ship_hit_info->collision_normal, LANDING_POS_OFFSET);
	}
	else {
		vm_vec_scale_add2(&lighter->pos, &direction_light, -0.2f * heavy->phys_info.mass / (heavy->phys_info.mass + lighter->phys_info.mass));
		vm_vec_scale_add2(&lighter->pos, &ship_ship_hit_info->collision_normal,  0.1f * heavy->phys_info.mass / (heavy->phys_info.mass + lighter->phys_info.mass));
	}

	// restore mass in case of special cruiser / asteroid collision
	if (special_cruiser_asteroid_collision) {
		if (cruiser_light) {
			lighter->phys_info.mass = copy_mass;
		} else {
			heavy->phys_info.mass = copy_mass;
		}
	}
}


// ------------------------------------------------------------------------------------------------
//	get_I_inv()
//
//		input:	I_inv_body	=>		inverse moment of inertia matrix in body coordinates
//					orient		=>		orientation matrix
//
//		output:	I_inv			=>		inverse moment of inertia matrix in world coordinates
// ------------------------------------------------------------------------------------------------
//
// calculates the inverse moment of inertia matrix from the body matrix and oreint matrix
//
static void get_I_inv (matrix* I_inv, matrix* I_inv_body, matrix* orient)
{
	matrix Mtemp1, Mtemp2;
	// I_inv = (Rt)(I_inv_body)(R)
	// This is opposite to what is commonly seen in books since we are rotating coordianates axes 
	// which is equivalent to rotating in the opposite direction (or transpose)

	vm_matrix_x_matrix(&Mtemp1, I_inv_body, orient);
	vm_copy_transpose(&Mtemp2, orient);
	vm_matrix_x_matrix(I_inv, &Mtemp2, &Mtemp1);
}

#define	PLANET_DAMAGE_SCALE	4.0f
#define	PLANET_DAMAGE_RANGE	3		//	If within this factor of radius, apply damage.

fix	Last_planet_damage_time = 0;
extern void hud_start_text_flash(char *txt, int t, int interval);

/**
 * Procss player_ship:planet damage.
 *	If within range of planet, apply damage to ship.
 */
static void mcp_1(object *player_objp, object *planet_objp)
{
	float	planet_radius;
	float	dist;

	planet_radius = planet_objp->radius;
	dist = vm_vec_dist_quick(&player_objp->pos, &planet_objp->pos);

	if (dist > planet_radius*PLANET_DAMAGE_RANGE)
		return;

	ship_apply_global_damage( player_objp, planet_objp, NULL, PLANET_DAMAGE_SCALE * flFrametime * (float)pow((planet_radius*PLANET_DAMAGE_RANGE)/dist, 3.0f), -1 );

	if ((Missiontime - Last_planet_damage_time > F1_0) || (Missiontime < Last_planet_damage_time)) {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, "%s", XSTR( "Too close to planet.  Taking damage!", 465));
		Last_planet_damage_time = Missiontime;
		snd_play_3d( gamesnd_get_game_sound(ship_get_sound(player_objp, GameSounds::ABURN_ENGAGE)), &player_objp->pos, &View_position );
	}

}

/**
 * Return true if *objp is a planet, else return false.
 *	Hack: Just checking first six letters of name.
 */
static int is_planet(object *objp)
{
	return (strnicmp(Ships[objp->instance].ship_name, NOX("planet"), 6) == 0);
}


/**
 * If exactly one of these is a planet and the other is a player ship, do something special.
 * @return true if this was a ship:planet (or planet_ship) collision and we processed it. Else return false.
 */
static int maybe_collide_planet (object *obj1, object *obj2)
{
	ship_info	*sip1, *sip2;

	sip1 = &Ship_info[Ships[obj1->instance].ship_info_index];
	sip2 = &Ship_info[Ships[obj2->instance].ship_info_index];

	if (sip1->flags[Ship::Info_Flags::Player_ship]) {
		if (is_planet(obj2)) {
			mcp_1(obj1, obj2);
			return 1;
		}
	} else if (sip2->flags[Ship::Info_Flags::Player_ship]) {
		if (is_planet(obj1)) {
			mcp_1(obj2, obj1);
			return 1;
		}
	}

	return 0;
}

/**
 * Given a global point and an object, get the quadrant number the point belongs to.
 */
int get_ship_quadrant_from_global(vec3d *global_pos, object *objp)
{
	vec3d	tpos;
	vec3d	rotpos;

	vm_vec_sub(&tpos, global_pos, &objp->pos);
	vm_vec_rotate(&rotpos, &tpos, &objp->orient);
	return get_quadrant(&rotpos, objp);
}

#define	MIN_REL_SPEED_FOR_LOUD_COLLISION		50		// relative speed of two colliding objects at which we play the "loud" collide sound

void collide_ship_ship_sounds_init()
{
	Player_collide_sound        = sound_handle::invalid();
	AI_collide_sound            = sound_handle::invalid();
	Player_collide_shield_sound = sound_handle::invalid();
	AI_collide_shield_sound     = sound_handle::invalid();
}

gamesnd_id choose_collision_sound(gamesnd_id default_snd, object *A, object *B)
{
	gamesnd_id a_snd, b_snd;

	if (default_snd == gamesnd_id(GameSounds::SHIP_SHIP_HEAVY))
	{
		if (A->type == OBJ_SHIP)
			a_snd = Ship_info[Ships[A->instance].ship_info_index].collision_physics.collision_sound_heavy_idx;
		else if (A->type == OBJ_DEBRIS)
			a_snd = Ship_info[Debris[A->instance].ship_info_index].debris_collision_sound_heavy;

		if (B->type == OBJ_SHIP)
			b_snd = Ship_info[Ships[B->instance].ship_info_index].collision_physics.collision_sound_heavy_idx;
		else if (B->type == OBJ_DEBRIS)
			b_snd = Ship_info[Debris[B->instance].ship_info_index].debris_collision_sound_heavy;
	}
	else if (default_snd == gamesnd_id(GameSounds::SHIP_SHIP_LIGHT))
	{
		if (A->type == OBJ_SHIP)
			a_snd = Ship_info[Ships[A->instance].ship_info_index].collision_physics.collision_sound_light_idx;
		else if (A->type == OBJ_DEBRIS)
			a_snd = Ship_info[Debris[A->instance].ship_info_index].debris_collision_sound_light;

		if (B->type == OBJ_SHIP)
			b_snd = Ship_info[Ships[B->instance].ship_info_index].collision_physics.collision_sound_light_idx;
		else if (B->type == OBJ_DEBRIS)
			b_snd = Ship_info[Debris[B->instance].ship_info_index].debris_collision_sound_light;
	}
	else if (default_snd == gamesnd_id(GameSounds::SHIP_SHIP_SHIELD))
	{
		if (A->type == OBJ_SHIP)
			a_snd = Ship_info[Ships[A->instance].ship_info_index].collision_physics.collision_sound_shielded_idx;
		if (B->type == OBJ_SHIP)
			b_snd = Ship_info[Ships[B->instance].ship_info_index].collision_physics.collision_sound_shielded_idx;
	}

	// if both A *and* B have a sound, arbitrarily choose A's sound
	if (a_snd.isValid())
		return a_snd;
	else if (b_snd.isValid())
		return b_snd;
	else
		return default_snd;
}

/**
 * Determine what sound to play when two ships collide
 */
void collide_ship_ship_do_sound(vec3d *world_hit_pos, object *A, object *B, int player_involved)
{
	vec3d	rel_vel;
	float		rel_speed;
			
	vm_vec_sub(&rel_vel, &A->phys_info.desired_vel, &B->phys_info.desired_vel);
	rel_speed = vm_vec_mag_quick(&rel_vel);

	if ( rel_speed > MIN_REL_SPEED_FOR_LOUD_COLLISION ) {
		auto snd_id = choose_collision_sound(GameSounds::SHIP_SHIP_HEAVY, A, B);
		snd_play_3d( gamesnd_get_game_sound(snd_id), world_hit_pos, &View_position );
	} else {
		auto snd_id = choose_collision_sound(GameSounds::SHIP_SHIP_LIGHT, A, B);
		if ( player_involved ) {
			if ( !snd_is_playing(Player_collide_sound) ) {
				Player_collide_sound = snd_play_3d( gamesnd_get_game_sound(snd_id), world_hit_pos, &View_position );
			}
		} else {
			if ( !snd_is_playing(AI_collide_sound) ) {
				AI_collide_sound = snd_play_3d( gamesnd_get_game_sound(snd_id), world_hit_pos, &View_position );
			}
		}
	}

	// maybe play a "shield" collision sound overlay if appropriate
	if ( (shield_get_strength(A) > 5) || (shield_get_strength(B) > 5) ) {
		auto snd_id = choose_collision_sound(GameSounds::SHIP_SHIP_SHIELD, A, B);
		if ( player_involved ) {
			if ( !snd_is_playing(Player_collide_sound) ) {
				Player_collide_shield_sound = snd_play_3d( gamesnd_get_game_sound(snd_id), world_hit_pos, &View_position );
			}
		} else {
			if ( !snd_is_playing(Player_collide_sound) ) {
				AI_collide_shield_sound = snd_play_3d( gamesnd_get_game_sound(snd_id), world_hit_pos, &View_position );
			}
		}
	}
}

/**
 * obj1 and obj2 collided.
 * If different teams, kamikaze bit set and other ship is large, auto-explode!
 */
static void do_kamikaze_crash(object *obj1, object *obj2)
{
	ai_info	*aip1, *aip2;
	ship		*ship1, *ship2;

	ship1 = &Ships[obj1->instance];
	ship2 = &Ships[obj2->instance];

	aip1 = &Ai_info[ship1->ai_index];
	aip2 = &Ai_info[ship2->ai_index];

	if (ship1->team != ship2->team) {
		if (aip1->ai_flags[AI::AI_Flags::Kamikaze]) {
			if (Ship_info[ship2->ship_info_index].is_big_or_huge()) {
				obj1->hull_strength = KAMIKAZE_HULL_ON_DEATH;
				shield_set_strength(obj1, 0.0f);
			}
		} if (aip2->ai_flags[AI::AI_Flags::Kamikaze]) {
            if (Ship_info[ship1->ship_info_index].is_big_or_huge()) {
				obj2->hull_strength = KAMIKAZE_HULL_ON_DEATH;
				shield_set_strength(obj2, 0.0f);
			}
		}
	}
}

/**
 * Response when hit by fast moving cap ship
 */
static void maybe_push_little_ship_from_fast_big_ship(object *big_obj, object *small_obj, float impulse, vec3d *normal)
{
	// Move player out of the way of a BIG|HUGE ship warping in or out
	int big_class = Ship_info[Ships[big_obj->instance].ship_info_index].class_type;
	int small_class = Ship_info[Ships[small_obj->instance].ship_info_index].class_type;
	if (big_class > -1 && Ship_types[big_class].flags[Ship::Type_Info_Flags::Warp_pushes]) {
		if (small_class > -1 && Ship_types[small_class].flags[Ship::Type_Info_Flags::Warp_pushable]) {
			float big_speed = vm_vec_mag_quick(&big_obj->phys_info.vel);
			if (big_speed > 3*big_obj->phys_info.max_vel.xyz.z) {
				// push player away in direction perp to forward of big ship
				// get perp vec
				vec3d temp, perp;
				vm_vec_sub(&temp, &small_obj->pos, &big_obj->pos);
				vm_vec_scale_add(&perp, &temp, &big_obj->orient.vec.fvec, -vm_vec_dot(&temp, &big_obj->orient.vec.fvec));
				vm_vec_normalize_quick(&perp);

				// don't drive into sfc we just collided with
				if (vm_vec_dot(&perp, normal) < 0) {
					vm_vec_negate(&perp);
				}

				// get magnitude of added perp vel
				float added_perp_vel_mag = impulse / small_obj->phys_info.mass;

				// add to vel and ramp vel
				vm_vec_scale_add2(&small_obj->phys_info.vel, &perp, added_perp_vel_mag);
				vm_vec_rotate(&small_obj->phys_info.prev_ramp_vel, &small_obj->phys_info.vel, &small_obj->orient);
			}
		}
	}
}

/**
 * Checks ship-ship collisions.  
 * @return 1 if all future collisions between these can be ignored because pair->a or pair->b aren't ships
 * @return Otherwise always returns 0, since two ships can always collide unless one (1) dies or (2) warps out.
 */
int collide_ship_ship( obj_pair * pair )
{
	int	player_involved;
	float dist;
	object *A = pair->a;
	object *B = pair->b;

	if ( A->type == OBJ_WAYPOINT ) return 1;
	if ( B->type == OBJ_WAYPOINT ) return 1;
	
	Assert( A->type == OBJ_SHIP );
	Assert( B->type == OBJ_SHIP );

	// Cyborg17 - no ship-ship collisions when doing multiplayer rollback
	if ( (Game_mode & GM_MULTIPLAYER) && multi_ship_record_get_rollback_wep_mode() ) {
		return 0;
	}

	if (reject_due_collision_groups(A,B))
		return 0;

	// If the player is one of the two colliding ships, flag this... it is used in
	// several places this function.
	if ( A == Player_obj || B == Player_obj ) {
		player_involved = 1;
	} else {
		player_involved = 0;
	}

	// Don't check collisions for warping out player if past stage 1.
	if ( player_involved && (Player->control_mode > PCM_WARPOUT_STAGE1) )	{
		return 0;
	}

	dist = vm_vec_dist( &A->pos, &B->pos );

	//	If one of these is a planet, do special stuff.
	if (maybe_collide_planet(A, B))
		return 0;

	if ( dist < A->radius + B->radius )	{
		int		hit;

		object	*HeavyOne, *LightOne;
		// if two objects have the same mass, make the one with the larger pointer address the HeavyOne.
		if ( fl_abs(A->phys_info.mass - B->phys_info.mass) < 1 ) {
			if (A > B) {
				HeavyOne = A;
				LightOne = B;
			} else {
				HeavyOne = B;
				LightOne = A;
			}
		} else {
			if (A->phys_info.mass > B->phys_info.mass) {
				HeavyOne = A;
				LightOne = B;
			} else {
				HeavyOne = B;
				LightOne = A;
			}
		}

		ship_info *light_sip = &Ship_info[Ships[LightOne->instance].ship_info_index];
		ship_info* heavy_sip = &Ship_info[Ships[HeavyOne->instance].ship_info_index];

		collision_info_struct ship_ship_hit_info;
		init_collision_info_struct(&ship_ship_hit_info);

		ship_ship_hit_info.heavy = HeavyOne;		// heavy object, generally slower moving
		ship_ship_hit_info.light = LightOne;		// light object, generally faster moving

		vec3d world_hit_pos;

		hit = ship_ship_check_collision(&ship_ship_hit_info);

		pair->next_check_time = timestamp(0);

		if ( hit )
		{
			Script_system.SetHookObjects(4, "Self", A, "Object", B, "Ship", A, "ShipB", B);
			Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(world_hit_pos));
			bool a_override = Script_system.IsConditionOverride(CHA_COLLIDESHIP, A);
			Script_system.RemHookVars({ "Self", "Object", "Ship", "ShipB", "Hitpos" });

			// Yes, this should be reversed.
			Script_system.SetHookObjects(4, "Self", B, "Object", A, "Ship", B, "ShipB", A);
			Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(world_hit_pos));
			bool b_override = Script_system.IsConditionOverride(CHA_COLLIDESHIP, B);
			Script_system.RemHookVars({ "Self", "Object", "Ship", "ShipB", "Hitpos" });

			if(!a_override && !b_override)
			{
				//
				// Start of a codeblock that was originally taken from ship_ship_check_collision
				// Moved here to properly handle ship-ship collision overrides and not process their physics when overridden by lua
				//

				ship *light_shipp = &Ships[ship_ship_hit_info.heavy->instance];
				ship *heavy_shipp = &Ships[ship_ship_hit_info.heavy->instance];

				object* heavy_obj = ship_ship_hit_info.heavy;
				object* light_obj = ship_ship_hit_info.light;
				// Update ai to deal with collisions
				if (heavy_obj - Objects == Ai_info[light_shipp->ai_index].target_objnum) {
					Ai_info[light_shipp->ai_index].ai_flags.set(AI::AI_Flags::Target_collision);
				}
				if (light_obj - Objects == Ai_info[heavy_shipp->ai_index].target_objnum) {
					Ai_info[heavy_shipp->ai_index].ai_flags.set(AI::AI_Flags::Target_collision);
				}

				// SET PHYSICS PARAMETERS
				// already have (hitpos - heavy) and light_cm_pos

				// get r_heavy and r_light
				ship_ship_hit_info.r_heavy = ship_ship_hit_info.hit_pos;
				vm_vec_sub(&ship_ship_hit_info.r_light, &ship_ship_hit_info.hit_pos, &ship_ship_hit_info.light_collision_cm_pos);

				// set normal for edge hit
				if (ship_ship_hit_info.edge_hit) {
					vm_vec_copy_normalize(&ship_ship_hit_info.collision_normal, &ship_ship_hit_info.r_light);
					vm_vec_negate(&ship_ship_hit_info.collision_normal);
				}

				// get world hitpos
				vm_vec_add(&world_hit_pos, &heavy_obj->pos, &ship_ship_hit_info.r_heavy);

				// do physics
				calculate_ship_ship_collision_physics(&ship_ship_hit_info);

				// Provide some separation for the case of same team
				if (heavy_shipp->team == light_shipp->team) {
					//	If a couple of small ships, just move them apart.

					if ((heavy_sip->is_small_ship()) && (light_sip->is_small_ship())) {
						if ((heavy_obj->flags[Object::Object_Flags::Player_ship]) || (light_obj->flags[Object::Object_Flags::Player_ship])) {
							vec3d h_to_l_vec;
							vec3d rel_vel_h;
							vec3d perp_rel_vel;

							vm_vec_sub(&h_to_l_vec, &heavy_obj->pos, &light_obj->pos);
							vm_vec_sub(&rel_vel_h, &heavy_obj->phys_info.vel, &light_obj->phys_info.vel);
							float mass_sum = light_obj->phys_info.mass + heavy_obj->phys_info.mass;

							// get comp of rel_vel perp to h_to_l_vec;
							float mag = vm_vec_dot(&h_to_l_vec, &rel_vel_h) / vm_vec_mag_squared(&h_to_l_vec);
							vm_vec_scale_add(&perp_rel_vel, &rel_vel_h, &h_to_l_vec, -mag);
							vm_vec_normalize(&perp_rel_vel);

							vm_vec_scale_add2(&heavy_obj->phys_info.vel, &perp_rel_vel,
								heavy_sip->collision_physics.both_small_bounce * light_obj->phys_info.mass / mass_sum);
							vm_vec_scale_add2(&light_obj->phys_info.vel, &perp_rel_vel,
								-(light_sip->collision_physics.both_small_bounce) * heavy_obj->phys_info.mass / mass_sum);

							vm_vec_rotate(&heavy_obj->phys_info.prev_ramp_vel, &heavy_obj->phys_info.vel, &heavy_obj->orient);
							vm_vec_rotate(&light_obj->phys_info.prev_ramp_vel, &light_obj->phys_info.vel, &light_obj->orient);
						}
					}
					else {
						// add extra velocity to separate the two objects, backing up the direction we came in.
						// TODO: add effect of velocity from rotating submodel
						float rel_vel = vm_vec_mag_quick(&ship_ship_hit_info.light_rel_vel);
						if (rel_vel < 1) {
							rel_vel = 1.0f;
						}
						float		mass_sum = heavy_obj->phys_info.mass + light_obj->phys_info.mass;
						vm_vec_scale_add2(&heavy_obj->phys_info.vel, &ship_ship_hit_info.light_rel_vel,
							heavy_sip->collision_physics.bounce * light_obj->phys_info.mass / (mass_sum * rel_vel));
						vm_vec_rotate(&heavy_obj->phys_info.prev_ramp_vel, &heavy_obj->phys_info.vel, &heavy_obj->orient);
						vm_vec_scale_add2(&light_obj->phys_info.vel, &ship_ship_hit_info.light_rel_vel,
							-(light_sip->collision_physics.bounce) * heavy_obj->phys_info.mass / (mass_sum * rel_vel));
						vm_vec_rotate(&light_obj->phys_info.prev_ramp_vel, &light_obj->phys_info.vel, &light_obj->orient);
					}
				}

				//
				// End of the codeblock that was originally taken from ship_ship_check_collision
				//

				float		damage;

				if ( player_involved && (Player->control_mode == PCM_WARPOUT_STAGE1) )	{
					gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
					HUD_printf("%s", XSTR( "Warpout sequence aborted.", 466));
				}

				damage = 0.005f * ship_ship_hit_info.impulse;	//	Cut collision-based damage in half.
				//	Decrease heavy damage by 2x.
				if (damage > 5.0f){
					damage = 5.0f + (damage - 5.0f)/2.0f;
				}

				do_kamikaze_crash(A, B);

				if (ship_ship_hit_info.impulse > 0) {
					//Only flash the "Collision" text if not landing
					if ( player_involved && !ship_ship_hit_info.is_landing) {					
						hud_start_text_flash(XSTR("Collision", 1431), 2000);
					}
				}

				//If this is a landing, play a different sound
				if (ship_ship_hit_info.is_landing) {
					if (vm_vec_mag(&ship_ship_hit_info.light_rel_vel) > MIN_LANDING_SOUND_VEL) {
						if ( player_involved ) {
							if ( !snd_is_playing(Player_collide_sound) ) {
								Player_collide_sound = snd_play_3d( gamesnd_get_game_sound(light_sip->collision_physics.landing_sound_idx), &world_hit_pos, &View_position );
							}
						} else {
							if ( !snd_is_playing(AI_collide_sound) ) {
								AI_collide_sound = snd_play_3d( gamesnd_get_game_sound(light_sip->collision_physics.landing_sound_idx), &world_hit_pos, &View_position );
							}
						}
					}
				}
				else {
					collide_ship_ship_do_sound(&world_hit_pos, A, B, player_involved);
				}

				// check if we should do force feedback stuff
				if (player_involved && (ship_ship_hit_info.impulse > 0)) {
					float scaler;
					vec3d v;

					scaler = -ship_ship_hit_info.impulse / Player_obj->phys_info.mass * 300;
					vm_vec_copy_normalize(&v, &world_hit_pos);
					joy_ff_play_vector_effect(&v, scaler);
				}

				#ifndef NDEBUG
				if ( !Collide_friendly ) {
					if ( Ships[A->instance].team == Ships[B->instance].team ) {
						vec3d	collision_vec, right_angle_vec;
						vm_vec_normalized_dir(&collision_vec, &ship_ship_hit_info.hit_pos, &A->pos);
						if (vm_vec_dot(&collision_vec, &A->orient.vec.fvec) > 0.999f){
							right_angle_vec = A->orient.vec.rvec;
						} else {
							vm_vec_cross(&right_angle_vec, &A->orient.vec.uvec, &collision_vec);
						}

						vm_vec_scale_add2( &A->phys_info.vel, &right_angle_vec, +2.0f);
						vm_vec_scale_add2( &B->phys_info.vel, &right_angle_vec, -2.0f);

						return 0;
					}
				}
				#endif

				//Only do damage if not a landing
				if (!ship_ship_hit_info.is_landing) {
					//	Scale damage based on skill level for player.
					if ((LightOne->flags[Object::Object_Flags::Player_ship]) || (HeavyOne->flags[Object::Object_Flags::Player_ship])) {

						// Cyborg17 - Pretty hackish, but it's our best option, limit the amount of times a collision can
						// happen to multiplayer clients, because otherwise the server can kill clients far too quickly.
						// So here it goes, first only do this on the master (has an intrinsic multiplayer check) 
						if (MULTIPLAYER_MASTER) {
							// check to see if both colliding ships are player ships
							bool second_player_check = false;
							if ((LightOne->flags[Object::Object_Flags::Player_ship]) && (HeavyOne->flags[Object::Object_Flags::Player_ship]))
								second_player_check = true;

							// iterate through each player
							for (net_player & current_player : Net_players) {
								// check that this player's ship is valid, and that it's not the server ship.
								if ((current_player.m_player != nullptr) && !(current_player.flags & NETINFO_FLAG_AM_MASTER) && (current_player.m_player->objnum > 0) && current_player.m_player->objnum < MAX_OBJECTS) {
									// check that one of the colliding ships is this player's ship
									if ((LightOne == &Objects[current_player.m_player->objnum]) || (HeavyOne == &Objects[current_player.m_player->objnum])) {
										// finally if the host is also a player, ignore making these adjustments for him because he is in a pure simulation.
										if (&Ships[Objects[current_player.m_player->objnum].instance] != Player_ship) {
											// temp set this as an uninterpolated ship, to make the collision look more natural until the next update comes in.
											multi_oo_set_client_simulation_mode(Objects[current_player.m_player->objnum].net_signature);

											// check to see if it has been long enough since the last collision, if not negate the damage
											if (!timestamp_elapsed(current_player.s_info.player_collision_timestamp)) {
												damage = 0.0f;
											} else {
												// make the usual adjustments
												damage *= (float)(Game_skill_level * Game_skill_level + 1) / (NUM_SKILL_LEVELS + 1);
												// if everything is good to go, set the timestamp for the next collision
												current_player.s_info.player_collision_timestamp = timestamp(PLAYER_COLLISION_TIMESTAMP);
											}
										}

										// did we find the player we were looking for?
										if (!second_player_check) {
											break;
										// if we found one of the players we were looking for, set this to false so that the next one breaks the loop
										} else {
											second_player_check = false;
										}
									}
								}
							}
						// if not in multiplayer, just do the damage adjustment.
						} else {
							damage *= (float) (Game_skill_level*Game_skill_level+1)/(NUM_SKILL_LEVELS+1);
						}
					} else if (Ships[LightOne->instance].team == Ships[HeavyOne->instance].team) {
						//	Decrease damage if non-player ships and not large.
						//	Looks dumb when fighters are taking damage from bumping into each other.
						if ((LightOne->radius < 50.0f) && (HeavyOne->radius <50.0f)) {
							damage /= 4.0f;
						}
					}					

					int	quadrant_num = -1;					
					if (!The_mission.ai_profile->flags[AI::Profile_Flags::No_shield_damage_from_ship_collisions] && !(ship_ship_hit_info.heavy->flags[Object::Object_Flags::No_shields])) {
						quadrant_num = get_ship_quadrant_from_global(&world_hit_pos, ship_ship_hit_info.heavy);
						if (!ship_is_shield_up(ship_ship_hit_info.heavy, quadrant_num))
							quadrant_num = -1;
					}

					float damage_heavy = (100.0f * damage / HeavyOne->phys_info.mass);
					ship_apply_local_damage(ship_ship_hit_info.heavy, ship_ship_hit_info.light, &world_hit_pos, damage_heavy, light_shipp->collision_damage_type_idx, 
						quadrant_num, CREATE_SPARKS, ship_ship_hit_info.submodel_num, &ship_ship_hit_info.collision_normal);

					hud_shield_quadrant_hit(ship_ship_hit_info.heavy, quadrant_num);

					// don't draw sparks (using sphere hitpos)
					float damage_light = (100.0f * damage / LightOne->phys_info.mass);
					ship_apply_local_damage(ship_ship_hit_info.light, ship_ship_hit_info.heavy, &world_hit_pos, damage_light, heavy_shipp->collision_damage_type_idx, 
						MISS_SHIELDS, NO_SPARKS, -1, &ship_ship_hit_info.collision_normal);

					hud_shield_quadrant_hit(ship_ship_hit_info.light, -1);

					maybe_push_little_ship_from_fast_big_ship(ship_ship_hit_info.heavy, ship_ship_hit_info.light, ship_ship_hit_info.impulse, &ship_ship_hit_info.collision_normal);
				}
			}

			if(!(b_override && !a_override))
			{
				Script_system.SetHookObjects(4, "Self", A, "Object", B, "Ship", A, "ShipB", B);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(world_hit_pos));
				Script_system.RunCondition(CHA_COLLIDESHIP, A);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "ShipB", "Hitpos" });
			}
			if((b_override && !a_override) || (!b_override && !a_override))
			{
				// Yes, this should be reversed.
				Script_system.SetHookObjects(4, "Self", B, "Object", A, "Ship", B, "ShipB", A);
				Script_system.SetHookVar("Hitpos", 'o', scripting::api::l_Vector.Set(world_hit_pos));
				Script_system.RunCondition(CHA_COLLIDESHIP, B);
				Script_system.RemHookVars({ "Self", "Object", "Ship", "ShipB", "Hitpos" });
			}

			return 0;
		}					
    }
    else {
        // estimate earliest time at which pair can hit

        // cap ships warping in/out can exceed ship's expected velocity
        // if ship is warping in, in stage 1, its velocity is 0, so make ship try to collide next frame

        // if ship is huge and warping in or out
        if (((Ships[A->instance].is_arriving(ship::warpstage::STAGE1, false)) && (Ship_info[Ships[A->instance].ship_info_index].is_big_or_huge()))
			|| ((Ships[B->instance].is_arriving(ship::warpstage::STAGE1, false)) && (Ship_info[Ships[B->instance].ship_info_index].is_big_or_huge())) ) {
			pair->next_check_time = timestamp(0);	// check next time
			return 0;
		}

		// get max of (1) max_vel.z, (2) 10, (3) afterburner_max_vel.z, (4) vel.z (for warping in ships exceeding expected max vel)
		float shipA_max_speed, shipB_max_speed, time;

		// get shipA max speed
		if (ship_is_beginning_warpout_speedup(A)) {
			shipA_max_speed = MAX(ship_get_max_speed(&Ships[A->instance]), ship_get_warpout_speed(A));
		} else {
			shipA_max_speed = ship_get_max_speed(&Ships[A->instance]);
		}

		// Maybe warping in or finished warping in with excessive speed
		shipA_max_speed = MAX(shipA_max_speed, vm_vec_mag(&A->phys_info.vel));
		shipA_max_speed = MAX(shipA_max_speed, 10.0f);

		// get shipB max speed
		if (ship_is_beginning_warpout_speedup(B)) {
			shipB_max_speed = MAX(ship_get_max_speed(&Ships[B->instance]), ship_get_warpout_speed(B));
		} else {
			shipB_max_speed = ship_get_max_speed(&Ships[B->instance]);
		}

		// Maybe warping in or finished warping in with excessive speed
		shipB_max_speed = MAX(shipB_max_speed, vm_vec_mag(&B->phys_info.vel));
		shipB_max_speed = MAX(shipB_max_speed, 10.0f);

		time = 1000.0f * (dist - A->radius - B->radius) / (shipA_max_speed + shipB_max_speed);
		time -= 200.0f;		// allow one frame slow frame at ~5 fps

		if (time > 0) {
			pair->next_check_time = timestamp( fl2i(time) );
		} else {
			pair->next_check_time = timestamp(0);	// check next time
		}
	}
	
	return 0;
}
