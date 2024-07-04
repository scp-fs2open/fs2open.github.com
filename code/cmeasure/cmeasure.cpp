/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "cmeasure/cmeasure.h"
#include "gamesnd/gamesnd.h"
#include "hud/hud.h"
#include "math/staticrand.h"
#include "mission/missionparse.h"
#include "network/multimsgs.h"
#include "object/object.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

int	Cmeasures_homing_check = 0;
int	Countermeasures_enabled = 1;			//	Debug, set to 0 means no one can fire countermeasures.
const float CMEASURE_DETONATE_DISTANCE = 40.0f;

//Used to set a countermeasure velocity after being launched from a ship as a countermeasure
//ie not as a primary or secondary.
void cmeasure_set_ship_launch_vel(object *objp, object *parent_objp, int arand)
{
	vec3d vel, rand_vec;

	//Get cmeasure rear velocity in world
	vm_vec_scale_add(&vel, &parent_objp->phys_info.vel, &parent_objp->orient.vec.fvec, -25.0f);

	//Get random velocity vector
	static_randvec(arand+1, &rand_vec);

	//Add it to the rear velocity
	vm_vec_scale_add2(&vel, &rand_vec, 2.0f);

	objp->phys_info.vel = vel;

	//Zero out this stuff so it isn't moving
	vm_vec_zero(&objp->phys_info.rotvel);
	vm_vec_zero(&objp->phys_info.max_vel);
	vm_vec_zero(&objp->phys_info.max_rotvel);
	
	// blow out his reverse thrusters. Or drag, same thing.
	objp->phys_info.rotdamp = 10000.0f;
	objp->phys_info.side_slip_time_const = 10000.0f;

	objp->phys_info.max_vel.xyz.z = -25.0f;
	vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z );

	// if this cmeasure has a single segment trail, let the trail know since we just changed the velocity
	// yeah this is hacky but that's what this function gets for CHANGING the velocity on objects with a ""CONSTANT VELOCITY""
	weapon* wp = &Weapons[objp->instance];
	if (wp->trail_ptr && wp->trail_ptr->single_segment) {
		wp->trail_ptr->vel[0] = objp->phys_info.vel;
		wp->trail_ptr->vel[1] = objp->phys_info.vel;
	}
}

/** 
 * @brief If this is a player countermeasure, let the player know they evaded a missile.
 * @param objp [description]
 *
 * During single player games, this function notifies the player that evasion has occurred.
 * Multiplayer games ensure that #send_countermeasure_success_packet() is called to notify the other player.
 */

void cmeasure_maybe_alert_success(object *objp)
{
	//Is this a countermeasure, and does it have a parent
	if ( objp->type != OBJ_WEAPON || objp->parent < 0) {
		return;
	}

	Assert(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags[Weapon::Info_Flags::Cmeasure]);

	if ( objp->parent == OBJ_INDEX(Player_obj) ) {
		hud_start_text_flash(XSTR("Evaded", 1430), 800);
		snd_play(gamesnd_get_game_sound(ship_get_sound(Player_obj, GameSounds::MISSILE_EVADED_POPUP)));
	} else if ( Objects[objp->parent].flags[Object::Object_Flags::Player_ship] ) {
		send_countermeasure_success_packet( objp->parent );
	}
}
