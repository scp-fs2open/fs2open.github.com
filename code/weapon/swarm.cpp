/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/linklist.h"
#include "io/timer.h"
#include "object/object.h"
#include "ship/ship.h"
#include "weapon/swarm.h"
#include "weapon/weapon.h"



#define SWARM_DIST_OFFSET			2.0f		// distance swarm missile should vary from original path
#define SWARM_CONE_LENGTH			10000.0f	// used to pick a target point far in the distance
#define SWARM_CHANGE_DIR_TIME		400		// time to force change in direction of swarm missile
#define SWARM_ANGLE_CHANGE			(4*PI/180)			// in rad


// *No longer need this  -Et1
//#define SWARM_MISSILE_DELAY		150		// time delay between each swarm missile that is fired


#define SWARM_TIME_VARIANCE		100		// max time variance when deciding when to change swarm missile course

#define SWARM_FRAME_STOP_SWARMING	4			// Estimated number of zigzag frames BEFORE impact to stop zig-zagging and instead home in straight to the target

#define TURRET_SWARM_VALIDITY_CHECKTIME	5000	// number of ms between checks on turret_swam_info checks

#define SWARM_USED						(1<<0)


turret_swarm_info Turret_swarm_info[MAX_TURRET_SWARM_INFO];

int Turret_swarm_validity_next_check_time;

// ------------------------------------------------------------------
// swarm_level_init()
//
// Called at the start of each new mission
//
void swarm_level_init()
{
	int					i;
	turret_swarm_info	*tswarmp;

	for (i=0; i<MAX_TURRET_SWARM_INFO; i++) {
		tswarmp = &Turret_swarm_info[i];
		tswarmp->flags = 0;
		tswarmp->weapon_class = -1;
		tswarmp->num_to_launch = 0;
		tswarmp->parent_objnum = -1;
		tswarmp->parent_sig	  = -1;
		tswarmp->target_objnum = -1;
		tswarmp->target_sig	  = -1;
		tswarmp->turret		  = NULL;
		tswarmp->target_subsys = NULL;
		tswarmp->time_to_fire  = 0;
	}

	Turret_swarm_validity_next_check_time = timestamp(TURRET_SWARM_VALIDITY_CHECKTIME);
}

// ------------------------------------------------------------------
// swarm_maybe_fire_missile()
//
// Check if there are any swarm missiles to fire, and if enough time
// has elapsed since last one fired, go ahead and fire it.
//
// This is called once per ship frame in ship_move()
//
void swarm_maybe_fire_missile(int shipnum)
{
	ship			*sp;
	ship_weapon *swp;
	int			weapon_info_index;

	Assert(shipnum >= 0 && shipnum < MAX_SHIPS );
	sp = &Ships[shipnum];

	if ( sp->num_swarm_missiles_to_fire <= 0 ) {
		sp->swarm_missile_bank = -1;
		return;
	}

	swp = &sp->weapons;
	if ( sp->swarm_missile_bank == -1 ) {
		sp->num_swarm_missiles_to_fire = 0;
		return;
	}

	weapon_info_index = swp->secondary_bank_weapons[sp->swarm_missile_bank];
	Assert( weapon_info_index >= 0 && weapon_info_index < weapon_info_size() );

	// if swarm secondary bank is not a swarm missile, return
	if ( !(Weapon_info[weapon_info_index].wi_flags[Weapon::Info_Flags::Swarm]) ) {
		sp->num_swarm_missiles_to_fire = 0;
		sp->swarm_missile_bank = -1;
		return;
	}

	if ( timestamp_elapsed(sp->next_swarm_fire) )
    {

        // *Get timestamp from weapon info's -Et1
		sp->next_swarm_fire = timestamp(Weapon_info[weapon_info_index].SwarmWait );

		ship_fire_secondary( &Objects[sp->objnum], 1 );
		sp->num_swarm_missiles_to_fire--;
	}
}

// if it can't home on something, just set target dead ahead
void swarm_set_default_target_pos(object* objp, swarm_info* swarmp) {
	vm_vec_scale_add(&swarmp->current_target, &objp->pos, &objp->orient.vec.fvec, SWARM_CONE_LENGTH);
}

// ------------------------------------------------------------------
// swarm_create()
//
//	Set up all the basic info for a swarm missile
//
void swarm_create(object* objp, swarm_info* swarmp)
{
	swarmp->change_timestamp = 1;
	swarmp->homing_objnum = -1;
	swarmp->zig_direction = golden_ratio_rand() * PI2;

	vm_vec_zero(&swarmp->offset);

	swarm_set_default_target_pos(objp, swarmp);
}

// ------------------------------------------------------------------
// swarm_update_direction()
//
//	Check if we want to update the direction of a swarm missile.
//
void swarm_update_direction(object *objp, swarm_info* swarmp)
{
	weapon_info	*wip;
	weapon		*wp;
	object		*hobjp;
	vec3d		obj_to_target;	// Vector pointing from the swarm missile to its target
	float			vel, target_dist, radius;
	physics_info	*pi;

	Assert(objp->instance >= 0 && objp->instance < MAX_WEAPONS);

	wp = &Weapons[objp->instance];

	wip = &Weapon_info[wp->weapon_info_index];
	hobjp = wp->homing_object;
	pi = &Objects[wp->objnum].phys_info;

	// check if homing is lost.. if it is then set the target point dead ahead
	if ( swarmp->homing_objnum != -1 && hobjp == &obj_used_list ) {
		swarmp->change_timestamp = 1;
		swarmp->homing_objnum = -1;
		swarm_set_default_target_pos(objp, swarmp);
	}

	if ( hobjp != &obj_used_list ) {
		swarmp->homing_objnum = OBJ_INDEX(hobjp);
	}

	if ( timestamp_elapsed(swarmp->change_timestamp) ) {
		
		// Time to (maybe) zig-zag!
		int zig_zag_time = fl2i(SWARM_CHANGE_DIR_TIME + SWARM_TIME_VARIANCE * (frand() - 0.5f) * 2);
		swarmp->change_timestamp = timestamp(zig_zag_time);

		float missile_age = f2fl(Missiontime - wp->creation_time);

		if (hobjp != &obj_used_list && missile_age > 0.5f && missile_age > wip->free_flight_time)
		{
			// This is a copy of the relevant bits near the end of weapon_home() to make missiles lead their targets
			// note that if the swarm missile has a target_lead_scaler of 0 this is equivalent to retail behavior (no leading)
			if (Swarmers_lead_targets) {
				vec3d target_pos = wp->homing_pos;
				vec3d vec_to_goal;
				float dist_to_target = vm_vec_normalized_dir(&vec_to_goal, &target_pos, &objp->pos);
				float time_to_target = dist_to_target / wip->max_speed;

				vec3d tvec;
				tvec = objp->phys_info.vel;
				vm_vec_normalize(&tvec);

				float old_dot = vm_vec_dot(&tvec, &vec_to_goal);

				if ((old_dot > 0.1f) && (time_to_target > 0.1f)) {
					if (wip->wi_flags[Weapon::Info_Flags::Variable_lead_homing]) {
						target_pos += hobjp->phys_info.vel * (0.33f * wip->target_lead_scaler * MIN(time_to_target, 6.0f));
					}
					else if (wip->is_locked_homing()) {
						target_pos += hobjp->phys_info.vel * MIN(time_to_target, 2.0f);
					}
				}
				swarmp->current_target = target_pos;
			} else { // Else, retail behavior (usually simply the target's position)
				swarmp->current_target = wp->homing_pos;
			}
		}

		obj_to_target = swarmp->current_target - objp->pos;
		target_dist = vm_vec_mag_quick(&obj_to_target);

		// If homing swarm missile is close to target, let missile home in on original target
		if ((target_dist / pi->speed) <= ((zig_zag_time / 1000.0f) * SWARM_FRAME_STOP_SWARMING)) {
			vm_vec_zero(&swarmp->offset);
		} else {
			
			// calculate a radius around our target such that it would be the same angular offset
			// as SWARM_DIST_OFFSET (2 meters) at our next zig distance
			
			float missile_dist;     // straight-line distance the missile will travel between now and next check
			float missile_speed;    // current speed of the missile

			missile_speed = pi->speed;
			missile_dist = missile_speed * zig_zag_time / 1000.0f;
			if (missile_dist < SWARM_DIST_OFFSET)
				missile_dist = SWARM_DIST_OFFSET;
			float angle_offset = asinf_safe(SWARM_DIST_OFFSET / missile_dist);
			Assert(!fl_is_nan(angle_offset));

			// Radius around the target pos. Shortens as the missiles get closer to the target.
			radius = tanf(angle_offset) * target_dist;

			// maybe zig zag to a different angle
			int zigs_zagged = (int)(missile_age * (1 / (SWARM_CHANGE_DIR_TIME / 1000.0f)));
			int mod_signature = (Game_mode & GM_MULTIPLAYER ? objp->net_signature : objp->signature) % 3;
			// depending on the signature, switch every 3, 4, or 5 zig zags
			if (zigs_zagged % (mod_signature + 3)) { 
				// angle change!
				swarmp->zig_direction = golden_ratio_rand() * PI2;
			} else // otherwise go 180 degrees off of what you were before
				swarmp->zig_direction += PI;

			// then rotate by it 
			swarmp->offset = objp->orient.vec.uvec * radius;
			vm_rot_point_around_line(&swarmp->offset, &swarmp->offset, swarmp->zig_direction, &vmd_zero_vector, &objp->orient.vec.fvec);
		}
	}

	vec3d actual_target = swarmp->current_target + swarmp->offset;
	ai_turn_towards_vector(&actual_target, objp, nullptr, nullptr, 0.0f, 0);
	vel = vm_vec_mag(&objp->phys_info.desired_vel);
	vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, vel);
}

// ------------------------------------------------------------------
// turret_swarm_create()
//
//	Get a free swarm missile entry, and initialize the struct members
//
int turret_swarm_create()
{
	int i;
	turret_swarm_info	*tswarmp = NULL;

	for (i=0; i<MAX_TURRET_SWARM_INFO; i++) {
		tswarmp = &Turret_swarm_info[i];
		if ( !(tswarmp->flags & SWARM_USED) ) 
			break;		
	}

	if ( i >= MAX_TURRET_SWARM_INFO ) {
		nprintf(("Warning","No more turret swarm info slots are available\n"));
		return -1;
	}

	tswarmp->flags = 0;
	tswarmp->weapon_class = -1;
	tswarmp->num_to_launch = 0;
	tswarmp->parent_objnum = -1;
	tswarmp->parent_sig = -1;
	tswarmp->target_objnum = -1;
	tswarmp->target_sig = -1;
	tswarmp->turret = NULL;
	tswarmp->target_subsys = NULL;
	tswarmp->time_to_fire = 0;

	tswarmp->flags |= SWARM_USED;
	return i;
}

// ------------------------------------------------------------------
// turret_swarm_delete()
//
void turret_swarm_delete(int i)
{
	turret_swarm_info		*tswarmp;

	Assert(i >= 0 && i < MAX_TURRET_SWARM_INFO);
	tswarmp = &Turret_swarm_info[i];

	if ( !(tswarmp->flags & SWARM_USED) ) {
		Int3();	// tried to delete a swarm missile that didn't exist, get DaveA
	}

	tswarmp->flags = 0;
}

// Set up turret swarm info struct
void turret_swarm_set_up_info(int parent_objnum, ship_subsys *turret, weapon_info *wip, int weapon_num)
{
	turret_swarm_info	*tsi;
	object *parent_obj, *target_obj;
	ship *shipp;
	int tsi_index;

	// weapon info pointer
	//Removed check in the interests of speed -WMC
	/*
	Assert((turret_weapon_class >= 0) && (turret_weapon_class < weapon_info_size()));
	if((turret_weapon_class < 0) || (turret_weapon_class >= weapon_info_size())){
		return;
	}
	*/

	// get ship pointer	
	Assert((parent_objnum >= 0) && (parent_objnum < MAX_OBJECTS));
	if((parent_objnum < 0) || (parent_objnum >= MAX_OBJECTS)){
		return;
	}
	parent_obj = &Objects[parent_objnum];
	Assert(parent_obj->type == OBJ_SHIP);
	shipp = &Ships[parent_obj->instance];
	Assert((turret->turret_enemy_objnum >= 0) && (turret->turret_enemy_objnum < MAX_OBJECTS));
	if((turret->turret_enemy_objnum < 0) || (turret->turret_enemy_objnum >= MAX_OBJECTS)){
		return;
	}
	target_obj = &Objects[turret->turret_enemy_objnum];

	// valid swarm weapon
	Assert(((wip->wi_flags[Weapon::Info_Flags::Swarm]) && (wip->swarm_count > 0)) || ((wip->wi_flags[Weapon::Info_Flags::Corkscrew]) && (wip->cs_num_fired > 0)));

	if(!((wip->wi_flags[Weapon::Info_Flags::Swarm]) || (wip->wi_flags[Weapon::Info_Flags::Corkscrew])) || ((wip->wi_flags[Weapon::Info_Flags::Swarm]) && (wip->swarm_count <= 0)) || ((wip->wi_flags[Weapon::Info_Flags::Corkscrew]) && (wip->cs_num_fired <= 0)))
		return;

	// get turret_swarm_info
	tsi_index = turret_swarm_create();
	if (tsi_index == -1) {
		return;
	}	

	// set turret to point to tsi
	tsi = &Turret_swarm_info[tsi_index];

	if (turret->turret_swarm_num == MAX_TFP)
	{
		mprintf(("Overlapping turret swarm firing intervals\n"));
		turret_swarm_delete(turret->turret_swarm_info_index[0]);
		int old_s;
		for (old_s = 0; old_s < (MAX_TFP - 1); old_s++)
		{
			turret->turret_swarm_info_index[old_s] = turret->turret_swarm_info_index[old_s + 1];
		}
		turret->turret_swarm_info_index[MAX_TFP - 1] = -1;
		turret->turret_swarm_num--;
		shipp->num_turret_swarm_info--;
	}
	turret->turret_swarm_info_index[turret->turret_swarm_num] = tsi_index;
	turret->turret_swarm_num++;

	// increment ship tsi counter
	shipp->num_turret_swarm_info++;

    // *Unnecessary check, now done on startup   -Et1
    /*

	// make sure time is sufficient to launch all the missiles before next volley
#ifndef NDEBUG	
	Assert(wip->swarm_count * SWARM_MISSILE_DELAY < wip->fire_wait * 1000.0f);
#endif

    */

	ship_weapon *swp = &turret->weapons;
	int bank_fired = swp->current_secondary_bank;

	// initialize tsi
	tsi->weapon_class = weapon_info_get_index(wip);
	if (wip->wi_flags[Weapon::Info_Flags::Swarm]) {
		tsi->num_to_launch = wip->swarm_count;
	} else {
		tsi->num_to_launch = wip->cs_num_fired;
	}
	if (turret->system_info->flags[Model::Subsystem_Flags::Turret_use_ammo]) {
		swp->secondary_bank_ammo[bank_fired] -= tsi->num_to_launch;
	}
	tsi->parent_objnum = parent_objnum;
	tsi->parent_sig    = parent_obj->signature;
	tsi->target_objnum = turret->turret_enemy_objnum;
	tsi->target_sig    = target_obj->signature;
	tsi->turret = turret;
	tsi->target_subsys = turret->targeted_subsys;
	tsi->time_to_fire = 1;	// first missile next frame
	tsi->weapon_num = weapon_num;
}

void turret_swarm_fire_from_turret(turret_swarm_info *tsi);
// check if ship has turret ready to fire swarm type missiles
void turret_swarm_maybe_fire_missile(int shipnum)
{
	ship *shipp = &Ships[shipnum];
	ship_subsys *subsys;
	turret_swarm_info *tsi;
	object *parent_obj, *target_obj;
	int num_turret_swarm_turrets_left;
	int k, j;
	weapon_info *wip;

	// check if ship has any turrets ready to fire
	if (shipp->num_turret_swarm_info <= 0) {
		Assert(shipp->num_turret_swarm_info == 0);
		return;
	}

	// ship obj which has fired turret swarm missiles
	parent_obj = &Objects[shipp->objnum];
	num_turret_swarm_turrets_left = shipp->num_turret_swarm_info;

	// search ship subsystems for turrets with valid turret_swarm_info_index
	for (subsys = GET_FIRST(&shipp->subsys_list); subsys != END_OF_LIST(&shipp->subsys_list); subsys = GET_NEXT(subsys)) {
 		if (subsys->turret_swarm_num > 0) {

			int swarms_per_turret = subsys->turret_swarm_num;

			for (k = 0; k < swarms_per_turret; k++)
			{
				int turret_tsi = subsys->turret_swarm_info_index[k];
				num_turret_swarm_turrets_left--;
				Assert(num_turret_swarm_turrets_left >= 0);

				// get turret_swarm_info
				Assert( (turret_tsi >= 0) && (turret_tsi < MAX_TURRET_SWARM_INFO) );
				tsi = &Turret_swarm_info[turret_tsi];
				wip = &Weapon_info[tsi->weapon_class];

				// check if parent ship is valid (via signature)
				if ( tsi->parent_sig == parent_obj->signature ) {

					// make sure we have the right turret.
					Assert(tsi->turret == subsys);
	
					// check if time to fire
					if (timestamp_elapsed(tsi->time_to_fire)) {
						Assert(tsi->num_to_launch > 0);
	
						// check target still alive
						if (tsi->target_objnum > -1) {
							target_obj= &Objects[tsi->target_objnum];

							if (target_obj->signature != tsi->target_sig) {
								// poor target, it died
								tsi->target_objnum = -1;
							}
						}

						// make sure turret is still alive and fire swarmer
						if (subsys->current_hits > 0) {
							turret_swarm_fire_from_turret(tsi);
						}

	                    // *Get timestamp from weapon info's -Et1
						if (wip->wi_flags[Weapon::Info_Flags::Swarm]) {
							tsi->time_to_fire = timestamp( wip->SwarmWait );
						} else {
							tsi->time_to_fire = timestamp( wip->cs_delay );
						}

						// do book keeping
						tsi->num_to_launch--;

						if (tsi->num_to_launch == 0) {
							// we are done firing, so see about resetting any animation timestamps for reversal (closing)...
							// (I figure that a good estimate is to trigger a close after three additional swarms had fired - taylor)
							if (subsys->turret_animation_position == MA_POS_READY)
								subsys->turret_animation_done_time = timestamp( Weapon_info[tsi->weapon_class].SwarmWait * 3);

							shipp->num_turret_swarm_info--;
							subsys->turret_swarm_num--;
							turret_swarm_delete(subsys->turret_swarm_info_index[k]);
							subsys->turret_swarm_info_index[k] = -1;
						}
					}
				} else {
					Warning(LOCATION,	"Found turret swarm info on ship: %s with turret: %s, but signature does not match.", shipp->ship_name, subsys->system_info->subobj_name);
					shipp->num_turret_swarm_info--;
					subsys->turret_swarm_num--;
					turret_swarm_delete(subsys->turret_swarm_info_index[k]);
					subsys->turret_swarm_info_index[k] = -1;
				}
			}
			//swarm reset stuff
			for (k = 0; k < (MAX_TFP - 1); k++)
			{
				for (j = (k + 1); j < MAX_TFP; j++)
				{
					if ((subsys->turret_swarm_info_index[k] == -1) && (subsys->turret_swarm_info_index[j] != -1))
					{
						subsys->turret_swarm_info_index[k] = subsys->turret_swarm_info_index[j];
						subsys->turret_swarm_info_index[j] = -1;
					}
				}
			}
		}
	}
	Assert(num_turret_swarm_turrets_left == 0);
}

// check Turret_swarm_info for info that are invalid - ie, ships died while firing.
void turret_swarm_check_validity()
{
	int i;
	turret_swarm_info *tswarmp;
	object *ship_objp;

	if (timestamp_elapsed(Turret_swarm_validity_next_check_time)) {

		// reset timestamp
		Turret_swarm_validity_next_check_time = timestamp(TURRET_SWARM_VALIDITY_CHECKTIME);

		// go through all Turret_swarm_info, check obj and obj->signature
		for (i=0; i<MAX_TURRET_SWARM_INFO; i++) {
			tswarmp = &Turret_swarm_info[i];

			if (tswarmp->flags & SWARM_USED) {
				ship_objp = &Objects[tswarmp->parent_objnum];
				if (ship_objp->type == OBJ_SHIP) {
					if (ship_objp->signature == tswarmp->parent_sig) {
						continue;
					}
				}

				turret_swarm_delete(i);
			}
		}
	}
}
