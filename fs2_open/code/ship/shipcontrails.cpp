/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "ship/shipcontrails.h"
#include "ship/ship.h"
#include "mission/missionparse.h"



// ----------------------------------------------------------------------------------------------
// CONTRAIL DEFINES/VARS
//

// ----------------------------------------------------------------------------------------------
// CONTRAIL FORWARD DECLARATIONS
//

// if the object is below the limit for contrails
int ct_below_limit(object *objp);

// Goober0500
bool ct_display_contrails();

// if an object has active contrails
int ct_has_contrails(ship *shipp);

// update active contrails - moving existing ones, adding new ones where necessary
void ct_update_contrails(ship *shipp);

// create new contrails
void ct_create_contrails(ship *shipp);


// determine if the ship has AB trails
int ct_has_ABtrails(ship *shipp);

// update active ABtrails - moving existing ones, adding new ones where necessary
void ct_update_ABtrails(ship *shipp);

// create new ABtrails
void ct_create_ABtrails(ship *shipp);

void ct_ship_process_ABtrails(ship *shipp);

// ----------------------------------------------------------------------------------------------
// CONTRAIL FUNCTIONS
//

// call during level initialization
void ct_level_init()
{	
}

// call during level shutdown
void ct_level_close()
{	
}

// call when a ship is created to initialize its contrail stuff
void ct_ship_create(ship *shipp)
{
	int idx;
	Assert(shipp != NULL);

	// null out the ct indices for this guy
	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		shipp->trail_ptr[idx] = NULL;
	}

	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		shipp->ABtrail_ptr[idx] = NULL;
	}


}

// call when a ship is deleted to free up its contrail stuff
void ct_ship_delete(ship *shipp)
{
	int idx;		

	Assert(shipp != NULL);
	// free up any contrails this guy may have had
	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->trail_ptr[idx] != NULL){
			trail_object_died(shipp->trail_ptr[idx]);
			shipp->trail_ptr[idx] = NULL;
		}
		if(shipp->ABtrail_ptr[idx] != NULL){
			trail_object_died(shipp->ABtrail_ptr[idx]);
			shipp->ABtrail_ptr[idx] = NULL;
		}
	}
}

// call each frame for processing a ship's contrails
void ct_ship_process(ship *shipp)
{
	// seems like as good a place as any -Bobboau
	if (shipp->ab_info->texture.bitmap_id != -1)
		ct_ship_process_ABtrails(shipp);

#ifdef MULTIPLAYER_BETA_BUILD
	return;
#else

	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	
	// if trails aren't enabled, return
	if (!ct_display_contrails()) {
		return;
	}

	int idx;		
	object *objp;
	objp = &Objects[shipp->objnum];

	// if this is not a ship, we don't care
	if((objp->type != OBJ_SHIP) || (Ship_info[Ships[objp->instance].ship_info_index].ct_count <= 0)){
		return;
	}

	Assert(objp->instance >= 0);
	shipp = &Ships[objp->instance];

	// if the object is below the critical limit
	if(ct_below_limit(objp)){
		// kill any active trails he has
		for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
			if(shipp->trail_ptr[idx] != NULL){
				trail_object_died(shipp->trail_ptr[idx]);
				shipp->trail_ptr[idx] = NULL;
			}
		}
	}


	// if the object already has contrails
	if(ct_has_contrails(shipp)){
		ct_update_contrails(shipp);
	}
	// otherwise add new ones
	else {
		ct_create_contrails(shipp);
	}

#endif
}

// ----------------------------------------------------------------------------------------------
// CONTRAIL FORWARD DEFINITIONS - test stuff
//

// if the object is below the limit for contrails
int ct_below_limit(object *objp)
{
	return objp->phys_info.fspeed < (float) The_mission.contrail_threshold;
}

// if a ship has active contrails
int ct_has_contrails(ship *shipp)
{
	int idx;

	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->trail_ptr[idx] != NULL){
			return 1;
		}
	}

	// no contrails
	return 0;
}

// update active contrails - moving existing ones, adding new ones where necessary
void ct_update_contrails(ship *shipp)
{
#ifdef MULTIPLAYER_BETA_BUILD
	return;
#else

	// if trails aren't enabled, return
	if (!ct_display_contrails()) {
		return;
	}

	vec3d v1;
	int idx;
	ship_info *sip;
	object *objp;

	// get object and ship info
	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	Assert(shipp->ship_info_index >= 0);
	objp = &Objects[shipp->objnum];
	sip = &Ship_info[shipp->ship_info_index];

	// process each contrail	
	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		// if this is a valid contrail
			if(shipp->trail_ptr[idx] != NULL){	
				// get the point for the contrail
				vm_vec_unrotate(&v1, &sip->ct_info[idx].pt, &objp->orient);
				vm_vec_add2(&v1, &objp->pos);

				// if the spew stamp has elapsed
				if(trail_stamp_elapsed(shipp->trail_ptr[idx])){	
					trail_add_segment(shipp->trail_ptr[idx], &v1);
					trail_set_stamp(shipp->trail_ptr[idx]);
				} else {
					trail_set_segment(shipp->trail_ptr[idx], &v1);
				}			
			}
	}
#endif
}

// create new contrails
void ct_create_contrails(ship *shipp)
{
#ifdef MULTIPLAYER_BETA_BUILD
	return;
#else

	// if trails aren't enabled, return
	if (!ct_display_contrails()) {
		return;
	}

	vec3d v1;
	int idx;
	ship_info *sip;
	object *objp;

	// get object and ship info
	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	Assert(shipp->ship_info_index >= 0);
	objp = &Objects[shipp->objnum];
	sip = &Ship_info[shipp->ship_info_index];


	for(idx=0; idx<sip->ct_count; idx++)
	{
		//if (this is a neb mision and this is a neb trail) or an ABtrail -Bobboau
		shipp->trail_ptr[idx] = trail_create(&sip->ct_info[idx]);	
	
		if(shipp->trail_ptr[idx] != NULL)
		{
			// add the point		
			vm_vec_unrotate(&v1, &sip->ct_info[idx].pt, &objp->orient);
			vm_vec_add2(&v1, &objp->pos);
			trail_add_segment(shipp->trail_ptr[idx], &v1);
			trail_add_segment(shipp->trail_ptr[idx], &v1);
		}
	}

#endif
}


// call each frame for processing a ship's ABtrails
void ct_ship_process_ABtrails(ship *shipp)
{
	int idx;		
	object *objp;
	ship_info* sip;

	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	objp = &Objects[shipp->objnum];	
	sip=&Ship_info[shipp->ship_info_index];

	// if this is not a ship, we don't care
	if((objp->type != OBJ_SHIP) || (Ships[objp->instance].ab_count <= 0)){
		return;
	}

	// if the ship has no afterburner trail bitmap, don't bother with anything
	if (sip->afterburner_trail.bitmap_id < 0)
		return;


	Assert(objp->instance >= 0);
	shipp = &Ships[objp->instance];


	if(!(objp->phys_info.flags & PF_AFTERBURNER_ON)){ //if the after burners aren't on -Bobboau
		for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
			if(shipp->ABtrail_ptr[idx] != NULL)
			{
				trail_object_died(shipp->ABtrail_ptr[idx]);
				shipp->ABtrail_ptr[idx] = NULL;
			}
		}

		//No more abtrails
		return;
	}

	// if the object already has ABtrails
	if(ct_has_ABtrails(shipp)){
		ct_update_ABtrails(shipp);
	}
	// otherwise add new ones
	else {
		ct_create_ABtrails(shipp);
	}

/*	if(shipp->ab_info->stamp > timestamp( (int)(shipp->ab_info->max_life * 1000) )  ){
		for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
			if(shipp->ABtrail_num[idx] >= 0){
				trail_object_died(shipp->ABtrail_num[idx]);
				shipp->ABtrail_num[idx] = (short)-1;
			}
		}
	}
*/
}



// create new ABtrails
void ct_create_ABtrails(ship *shipp)
{
	vec3d v1;
	int idx;
	ship_info *sip;
	object *objp;

	// get object and ship info
	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	Assert(shipp->ship_info_index >= 0);
	objp = &Objects[shipp->objnum];
	sip = &Ship_info[shipp->ship_info_index];

	// if the ship has no afterburner trail bitmap, don't bother with anything
	if (sip->afterburner_trail.bitmap_id < 0)
		return;

	if(objp->phys_info.flags & PF_AFTERBURNER_ON){//AB trails-Bobboau

		for(idx=0; idx<shipp->ab_count; idx++)
		{
		
			shipp->ABtrail_ptr[idx] = trail_create(&shipp->ab_info[idx]);	
			if(shipp->ABtrail_ptr[idx] != NULL)
			{
				// get the point for the contrail
				vm_vec_unrotate(&v1, &shipp->ab_info[idx].pt, &objp->orient);
				vm_vec_add2(&v1, &objp->pos);
				// if the spew stamp has elapsed
				if(trail_stamp_elapsed(shipp->ABtrail_ptr[idx])){	
					trail_add_segment(shipp->ABtrail_ptr[idx], &v1);
					trail_set_stamp(shipp->ABtrail_ptr[idx]);
				} else {
					trail_set_segment(shipp->ABtrail_ptr[idx], &v1);
				}
			}
		}
/*		if( objp == Player_obj){
			HUD_printf("trailnum %d", shipp->ABtrail_num[0]);
		}
*/

	}
}

// update active ABtrails - moving existing ones, adding new ones where necessary
void ct_update_ABtrails(ship *shipp)
{
	vec3d v1;
	int idx;
	ship_info *sip;
	object *objp;

	// get object and ship info
	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	Assert(shipp->ship_info_index >= 0);
	objp = &Objects[shipp->objnum];
	sip = &Ship_info[shipp->ship_info_index];

	// if the ship has no afterburner trail bitmap, don't bother with anything
	if (sip->afterburner_trail.bitmap_id < 0)
		return;

	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(objp->phys_info.flags & PF_AFTERBURNER_ON){//ABtrails
			if(shipp->ABtrail_ptr[idx] != NULL){	
				// get the point for the contrail
				vm_vec_unrotate(&v1, &shipp->ab_info[idx].pt, &objp->orient);
				vm_vec_add2(&v1, &objp->pos);

				// if the spew stamp has elapsed
				if(trail_stamp_elapsed(shipp->ABtrail_ptr[idx])){	
					trail_add_segment(shipp->ABtrail_ptr[idx], &v1);
					trail_set_stamp(shipp->ABtrail_ptr[idx]);
				} else {
					trail_set_segment(shipp->ABtrail_ptr[idx], &v1);
				}			
			}
		}

	}	
}


// if a ship has active ABtrails
int ct_has_ABtrails(ship *shipp)
{
	int idx;
	ship_info* sip=&Ship_info[shipp->ship_info_index];

	// if the ship has no afterburner trail bitmap, don't bother with anything
	if (sip->afterburner_trail.bitmap_id < 0)
		return 0;

	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->ABtrail_ptr[idx] != NULL){
			return 1;
		}
	}


	// no contrails
	return 0;
}

bool ct_display_contrails()
{
	bool display;

	// normally display trails in a full nebula environment
	display = (The_mission.flags & MISSION_FLAG_FULLNEB) ? true : false;

	// toggle according to flag
	if (The_mission.flags & MISSION_FLAG_TOGGLE_SHIP_TRAILS)
		display = !display;

	return display;
}
