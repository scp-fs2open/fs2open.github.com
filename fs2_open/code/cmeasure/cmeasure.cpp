/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "cmeasure/cmeasure.h"
//#include "freespace2/freespace.h"
//#include "model/model.h"
#include "ship/ship.h"
#include "math/staticrand.h"
#include "object/object.h"
#include "weapon/weapon.h"
#include "hud/hud.h"
#include "gamesnd/gamesnd.h"
#include "network/multimsgs.h"
#include "mission/missionparse.h"


//cmeasure_info Cmeasure_info[MAX_CMEASURE_TYPES];
//cmeasure Cmeasures[MAX_CMEASURES];

//int	Num_cmeasure_types = 0;
//int	Num_cmeasures = 0;
//int	Cmeasure_inited = 0;
int	Cmeasures_homing_check = 0;
int	Countermeasures_enabled = 1;			//	Debug, set to 0 means no one can fire countermeasures.

// This will get called at the start of each level.
//Not anymore - WMC
/*
void cmeasure_init()
{
	int i;

	if ( !Cmeasure_inited )
		Cmeasure_inited = 1;

	// Reset everything between levels
	Num_cmeasures = 0;

	for (i=0; i<MAX_CMEASURES; i++ )	{
		Cmeasures[i].subtype = CMEASURE_UNUSED;
	}
		
}*/
/*
void cmeasure_render(object * objp)
{
	// JAS TODO: Replace with proper fireball
	cmeasure			*cmp;
	cmeasure_info	*cmip;
	
	cmp = &Cmeasures[objp->instance];
	cmip = &Cmeasure_info[cmp->subtype];

	if ( cmp->subtype == CMEASURE_UNUSED )	{
		Int3();	//	Hey, what are we doing in here?
		return;
	}
	
	if ( cmip->model_num > -1 )	{
		model_clear_instance(cmip->model_num);
		model_render(cmip->model_num, &objp->orient, &objp->pos, MR_NO_LIGHTING );
	} else {
		mprintf(( "Not rendering countermeasure because model_num is negative\n" ));
	}
}*/
/*
void cmeasure_delete( object * objp )
{
	int num;

	num = objp->instance;

//	Assert( Cmeasures[num].objnum == OBJ_INDEX(objp));

	Cmeasures[num].subtype = CMEASURE_UNUSED;
	Num_cmeasures--;
	Assert( Num_cmeasures >= 0 );
}*/

// broke cmeasure_move into two functions -- process_pre and process_post (as was done with
// all *_move functions).  Nothing to do for process_pre
/*
void cmeasure_process_pre( object *objp, float frame_time)
{
}*/
/*
void cmeasure_process_post(object * objp, float frame_time)
{
	int num;
	num = objp->instance;
	
//	Assert( Cmeasures[num].objnum == objnum );
	cmeasure *cmp = &Cmeasures[num];

	if ( cmp->lifeleft >= 0.0f) {
		cmp->lifeleft -= frame_time;
		if ( cmp->lifeleft < 0.0f )	{
			objp->flags |= OF_SHOULD_BE_DEAD;
//			demo_do_flag_dead(OBJ_INDEX(objp));
		}
	}

}*/

// creates one countermeasure.  A ship fires 1 of these per launch.  rand_val is used
// in multiplayer.  If -1, then create a random number.  If non-negative, use this
// number for static_rand functions
/*
int cmeasure_create( object * source_obj, vec3d * pos, int cm_type, int rand_val )
{
	int		n, objnum, parent_objnum, arand;
	object	* obj;
	ship		*shipp;
	cmeasure	*cmp;
	cmeasure_info	*cmeasurep;

	Cmeasures_homing_check = 2;		//	Tell homing code to scan everything for two frames.  If only one frame, get sync problems due to objects being created at end of frame!

	parent_objnum = OBJ_INDEX(source_obj);

	Assert( source_obj->type == OBJ_SHIP );	
	Assert( source_obj->instance >= 0 && source_obj->instance < MAX_SHIPS );	
	
	shipp = &Ships[source_obj->instance];

	if ( Num_cmeasures >= MAX_CMEASURES)
		return -1;

	for (n=0; n<MAX_CMEASURES; n++ )	
		if ( Cmeasures[n].subtype == CMEASURE_UNUSED)
			break;
	if ( n == MAX_CMEASURES)
		return -1;

	nprintf(("Network", "Cmeasure created by %s\n", Ships[source_obj->instance].ship_name));

	cmp = &Cmeasures[n];
	cmeasurep = &Cmeasure_info[cm_type];

	if ( pos == NULL )
		pos = &source_obj->pos;

	objnum = obj_create( OBJ_CMEASURE, parent_objnum, n, &source_obj->orient, pos, 1.0f, OF_RENDERS | OF_PHYSICS );
	
	Assert( objnum >= 0 && objnum < MAX_OBJECTS );

	// Create Debris piece n!
	if ( rand_val == -1 )
		arand = myrand();				// use a random number to get lifeleft, and random vector for displacement from ship
	else
		arand = rand_val;

	cmp->lifeleft = static_randf(arand) * (cmeasurep->life_max - cmeasurep->life_min) / cmeasurep->life_min;
	if (source_obj->flags & OF_PLAYER_SHIP){
		cmp->lifeleft *= The_mission.ai_profile->cmeasure_life_scale[Game_skill_level];
	}
	cmp->lifeleft = cmeasurep->life_min + cmp->lifeleft * (cmeasurep->life_max - cmeasurep->life_min);

	//	cmp->objnum = objnum;
	cmp->team = shipp->team;
	cmp->subtype = cm_type;
	cmp->objnum = objnum;
	cmp->source_objnum = parent_objnum;
	cmp->source_sig = Objects[objnum].signature;

	cmp->flags = 0;

	nprintf(("Jim", "Frame %i: Launching countermeasure #%i\n", Framecount, Objects[objnum].signature));

	obj = &Objects[objnum];
	
	Num_cmeasures++;

	//Set countermeasure velocity
	vec3d vel, rand_vec;

	//Get cmeasure rear velocity in world
	vm_vec_scale_add(&vel, &source_obj->phys_info.vel, &source_obj->orient.vec.fvec, -25.0f);

	//Get random velocity vector
	static_randvec(arand+1, &rand_vec);

	//Add it to the rear velocity
	vm_vec_scale_add2(&vel, &rand_vec, 2.0f);

	obj->phys_info.vel = vel;

	vm_vec_zero(&obj->phys_info.rotvel);
	vm_vec_zero(&obj->phys_info.max_vel);
	vm_vec_zero(&obj->phys_info.max_rotvel);

	// blow out his reverse thrusters. Or drag, same thing.
	obj->phys_info.rotdamp = 10000.0f;
	obj->phys_info.side_slip_time_const = 10000.0f;

	obj->phys_info.max_vel.xyz.z = -25.0f;
	vm_vec_copy_scale(&obj->phys_info.desired_vel, &obj->orient.vec.fvec, obj->phys_info.max_vel.xyz.z );

	return arand;										// need to return this value for multiplayer purposes
}*/

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
	
	//Idunnit. -WMC
	//objp->phys_info.flags |= PF_CONST_VEL;

	//WMC - I don't think this stuff is needed with the flag I set
	//WMC - Or maybe it is.
	
	// blow out his reverse thrusters. Or drag, same thing.
	objp->phys_info.rotdamp = 10000.0f;
	objp->phys_info.side_slip_time_const = 10000.0f;

	objp->phys_info.max_vel.xyz.z = -25.0f;
	vm_vec_copy_scale(&objp->phys_info.desired_vel, &objp->orient.vec.fvec, objp->phys_info.max_vel.xyz.z );
}

void cmeasure_select_next(ship *shipp)
{
	Assert(shipp != NULL);
	int i, new_index;

	for (i = 1; i < Num_weapon_types; i++)
	{
		new_index = (shipp->current_cmeasure + i) % Num_weapon_types;

		if(Weapon_info[new_index].wi_flags & WIF_CMEASURE)
		{
			shipp->current_cmeasure = new_index;
			return;
		}
	}

	//snd_play( &Snds[SND_CMEASURE_CYCLE] );

	mprintf(("Countermeasure type set to %i in frame %i\n", shipp->current_cmeasure, Framecount));
}

// If this is a player countermeasure, let the player know he evaded a missile
void cmeasure_maybe_alert_success(object *objp)
{
	//Is this a countermeasure, and does it have a parent
	if ( objp->type != OBJ_WEAPON || objp->parent < 0) {
		return;
	}

	Assert(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_CMEASURE);

	if ( objp->parent == OBJ_INDEX(Player_obj) ) {
		hud_start_text_flash(XSTR("Evaded", 1430), 800);
		snd_play(&Snds[SND_MISSILE_EVADED_POPUP]);
	} else if ( Objects[objp->parent].flags & OF_PLAYER_SHIP ) {
		send_countermeasure_success_packet( objp->parent );
	}
}
