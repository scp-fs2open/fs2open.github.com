/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "weapon/swarm.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "globalincs/linklist.h"
#include "object/object.h"



#define SWARM_DIST_OFFSET			2.0		// distance swarm missile should vary from original path
#define SWARM_CONE_LENGTH			10000.0f	// used to pick a target point far in the distance
#define SWARM_CHANGE_DIR_TIME		400		// time to force change in direction of swarm missile
#define SWARM_ANGLE_CHANGE			(4*PI/180)			// in rad


// *No longer need this  -Et1
//#define SWARM_MISSILE_DELAY		150		// time delay between each swarm missile that is fired


#define SWARM_TIME_VARIANCE		100		// max time variance when deciding when to change swarm missile course

#define SWARM_DIST_STOP_SWARMING	300

#define TURRET_SWARM_VALIDITY_CHECKTIME	5000	// number of ms between checks on turret_swam_info checks

#define SWARM_USED						(1<<0)
#define SWARM_POSITIVE_PATH			(1<<1)


swarm_info	Swarm_missiles[MAX_SWARM_MISSILES];

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
	swarm_info			*swarmp;
	turret_swarm_info	*tswarmp;

	for ( i = 0; i < MAX_SWARM_MISSILES; i++ ) {
		swarmp = &Swarm_missiles[i];
		swarmp->flags = 0;
		swarmp->change_timestamp = 1;
		swarmp->path_num = -1;
	}

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
	Assert( weapon_info_index >= 0 && weapon_info_index < MAX_WEAPON_TYPES );

	// if swarm secondary bank is not a swarm missile, return
	if ( !(Weapon_info[weapon_info_index].wi_flags & WIF_SWARM) ) {
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

// ------------------------------------------------------------------
// swarm_create()
//
//	Get a free swarm missile entry, and initialize the struct members
//
int swarm_create()
{
	int			i;
	swarm_info	*swarmp = NULL;

	for ( i = 0; i < MAX_SWARM_MISSILES; i++ ) {
		swarmp = &Swarm_missiles[i];
		if ( !(swarmp->flags & SWARM_USED) ) 
			break;		
	}

	if ( i >= MAX_SWARM_MISSILES ) {
		nprintf(("Warning","No more swarm missiles are available\n"));
		return -1;
	}

	swarmp->flags = 0;
	swarmp->change_timestamp = 1;
	swarmp->path_num = -1;
	swarmp->homing_objnum = -1;

	swarmp->flags |= SWARM_USED;
	return i;
}

// ------------------------------------------------------------------
// swarm_delete()
//
//
void swarm_delete(int i)
{
	swarm_info	*swarmp;

	Assert(i >= 0 && i < MAX_SWARM_MISSILES);
	swarmp = &Swarm_missiles[i];

	if ( !(swarmp->flags & SWARM_USED) ) {
		Int3();	// tried to delete a swarm missile that didn't exist, get Alan
	}

	swarmp->flags = 0;
}

// ------------------------------------------------------------------
// swarm_update_direction()
//
//	Check if we want to update the direction of a swarm missile.
//
void swarm_update_direction(object *objp, float frametime)
{
	weapon_info	*wip;
	weapon		*wp;
	object		*hobjp;
	swarm_info	*swarmp;
	vec3d		obj_to_target;
	float			vel, target_dist, radius, missile_speed, missile_dist;
	physics_info	*pi;

	Assert(objp->instance >= 0 && objp->instance < MAX_WEAPONS);

	wp = &Weapons[objp->instance];

	if (wp->swarm_index == -1) {
		return;
	}

	wip = &Weapon_info[wp->weapon_info_index];
	hobjp = wp->homing_object;
	pi = &Objects[wp->objnum].phys_info;
	swarmp = &Swarm_missiles[wp->swarm_index];

	// check if homing is lost.. if it is then get a new path to move swarm missile along
	if ( swarmp->homing_objnum != -1 && hobjp == &obj_used_list ) {
		swarmp->change_timestamp = 1;
		swarmp->path_num = -1;
		swarmp->homing_objnum = -1;
	}

	if ( hobjp != &obj_used_list ) {
		swarmp->homing_objnum = OBJ_INDEX(hobjp);
	}

	if ( timestamp_elapsed(swarmp->change_timestamp) ) {

		if ( swarmp->path_num == -1 ) {
			if ( Objects[objp->parent].type != OBJ_SHIP ) {
				//AL: parent ship died... so just pick some random paths
				swarmp->path_num	= myrand()%4;
			} else {
				ship *parent_shipp;
				parent_shipp = &Ships[Objects[objp->parent].instance];
				swarmp->path_num = (parent_shipp->next_swarm_path++)%4;

				if ( parent_shipp->next_swarm_path%4 == 0 ) {
					swarmp->flags ^= SWARM_POSITIVE_PATH;
				}
			}

			vm_vec_scale_add(&swarmp->original_target, &objp->pos, &objp->orient.vec.fvec, SWARM_CONE_LENGTH);
			swarmp->circle_rvec = objp->orient.vec.rvec;
			swarmp->circle_uvec = objp->orient.vec.uvec;

			swarmp->change_count = 1;
			swarmp->change_time = fl2i(SWARM_CHANGE_DIR_TIME + SWARM_TIME_VARIANCE*(frand() - 0.5f) * 2);

			vm_vec_zero(&swarmp->last_offset);

			missile_speed = pi->speed;
			missile_dist	= missile_speed * swarmp->change_time/1000.0f;
			if ( missile_dist < SWARM_DIST_OFFSET ) {
				missile_dist=i2fl(SWARM_DIST_OFFSET);
			}
			swarmp->angle_offset = (float)(asin(SWARM_DIST_OFFSET / missile_dist));
			Assert(!_isnan(swarmp->angle_offset) );
		}

		swarmp->change_timestamp = timestamp(swarmp->change_time);

		// check if swarm missile is homing, if so need to calculate a new target pos to turn towards
		if ( hobjp != &obj_used_list && f2fl(Missiontime - wp->creation_time) > 0.5f && ( f2fl(Missiontime - wp->creation_time) > wip->free_flight_time ) ) {
			swarmp->original_target = wp->homing_pos;

			// Calculate a rvec and uvec that will determine the displacement from the
			// intended target.  Use crossprod to generate a right vector, from the missile
			// up vector and the vector connecting missile to the homing object.
			swarmp->circle_uvec = objp->orient.vec.uvec;
			swarmp->circle_rvec = objp->orient.vec.rvec;

			missile_speed = pi->speed;
			missile_dist = missile_speed * swarmp->change_time/1000.0f;
			if (missile_dist == 0.0f) // Just in case of div by zero, which can happen with local SSMs
				swarmp->angle_offset = (float)asin(SWARM_DIST_OFFSET);
			else
				swarmp->angle_offset = (float)(asin(SWARM_DIST_OFFSET / missile_dist));
			Assert(!_isnan(swarmp->angle_offset) );
		}

		vm_vec_sub(&obj_to_target, &swarmp->original_target, &objp->pos);
		target_dist = vm_vec_mag_quick(&obj_to_target);
		swarmp->last_dist = target_dist;

		// If homing swarm missile is close to target, let missile home in on original target
		if ( target_dist < SWARM_DIST_STOP_SWARMING ) {
			swarmp->new_target = swarmp->original_target;
			goto swarm_new_target_calced;
		}

		radius = (float)tan(swarmp->angle_offset) * target_dist;
		vec3d rvec_component, uvec_component;

		swarmp->change_count++;
		if ( swarmp->change_count > 2 ) {
			swarmp->flags ^= SWARM_POSITIVE_PATH;
			swarmp->change_count = 0;
		}

		// pick a new path number to follow once at center
		if ( swarmp->change_count == 1 ) {
			swarmp->path_num = swarmp->path_num + myrand()%3;
			if ( swarmp->path_num > 3 ) {
				swarmp->path_num = 0;
			}
		}

		vm_vec_zero(&rvec_component);
		vm_vec_zero(&uvec_component);

		switch ( swarmp->path_num ) {
			case 0:	// straight up and down
				if ( swarmp->flags & SWARM_POSITIVE_PATH )
					vm_vec_copy_scale( &uvec_component, &swarmp->circle_uvec, radius);
				else
					vm_vec_copy_scale( &uvec_component, &swarmp->circle_uvec, -radius);
				break;

			case 1:	// left/right
				if ( swarmp->flags & SWARM_POSITIVE_PATH )
					vm_vec_copy_scale( &rvec_component, &swarmp->circle_rvec, radius);
				else
					vm_vec_copy_scale( &rvec_component, &swarmp->circle_rvec, -radius);
				break;

			case 2:	// top/right - bottom/left
				if ( swarmp->flags & SWARM_POSITIVE_PATH ) {
					vm_vec_copy_scale( &rvec_component, &swarmp->circle_rvec, radius);
					vm_vec_copy_scale( &uvec_component, &swarmp->circle_uvec, radius);
				}
				else {
					vm_vec_copy_scale( &rvec_component, &swarmp->circle_rvec, -radius);
					vm_vec_copy_scale( &uvec_component, &swarmp->circle_uvec, -radius);
				}
				break;

			case 3:	// top-left - bottom/right
				if ( swarmp->flags & SWARM_POSITIVE_PATH ) {
					vm_vec_copy_scale( &rvec_component, &swarmp->circle_rvec, -radius);
					vm_vec_copy_scale( &uvec_component, &swarmp->circle_uvec, radius);
				}
				else {
					vm_vec_copy_scale( &rvec_component, &swarmp->circle_rvec, radius);
					vm_vec_copy_scale( &uvec_component, &swarmp->circle_uvec, -radius);
				}
				break;
			default:
				Int3();
				break;
		}

		swarmp->new_target = swarmp->original_target;
		vm_vec_zero(&swarmp->last_offset);
		vm_vec_add(&swarmp->last_offset, &uvec_component, &rvec_component);
		vm_vec_add2(&swarmp->new_target, &swarmp->last_offset);
	}
	else {
		if ( hobjp != &obj_used_list && f2fl(Missiontime - wp->creation_time) > 0.5f ) {

			swarmp->new_target = swarmp->original_target;
			if ( swarmp->last_dist < SWARM_DIST_STOP_SWARMING ) {
				swarmp->new_target = wp->homing_pos;
				goto swarm_new_target_calced;
			}

			vm_vec_add2(&swarmp->new_target, &swarmp->last_offset);
		}
	}

	swarm_new_target_calced:

	ai_turn_towards_vector(&swarmp->new_target, objp, frametime, wip->turn_time, NULL, NULL, 0.0f, 0);
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
		Int3();
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
	Assert((turret_weapon_class >= 0) && (turret_weapon_class < Num_weapon_types));
	if((turret_weapon_class < 0) || (turret_weapon_class >= Num_weapon_types)){
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
	Assert(((wip->wi_flags & WIF_SWARM) && (wip->swarm_count > 0)) || ((wip->wi_flags & WIF_CORKSCREW) && (wip->cs_num_fired > 0)));

	if(!((wip->wi_flags & WIF_SWARM) || (wip->wi_flags & WIF_CORKSCREW)) || ((wip->wi_flags & WIF_SWARM) && (wip->swarm_count <= 0)) || ((wip->wi_flags & WIF_CORKSCREW) && (wip->cs_num_fired <= 0)))
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
	// initialize tsi
	tsi->weapon_class = WEAPON_INFO_INDEX(wip);
	if (wip->wi_flags & WIF_SWARM)
		tsi->num_to_launch = wip->swarm_count;
	else
		tsi->num_to_launch = wip->cs_num_fired;
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
						if (wip->wi_flags & WIF_SWARM) {
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
