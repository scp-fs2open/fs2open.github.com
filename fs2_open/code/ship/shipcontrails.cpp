/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Ship/ShipContrails.cpp $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:02 $
 * $Author: penguin $
 *
 * all sorts of cool stuff about ships
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     4/25/99 3:02p Dave
 * Build defines for the E3 build.
 * 
 * 4     4/20/99 6:39p Dave
 * Almost done with artillery targeting. Added support for downloading
 * images on the PXO screen.
 * 
 * 3     4/12/99 11:03p Dave
 * Removed contrails and muzzle flashes from MULTIPLAYER_BETA builds.
 * 
 * 2     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 1     11/14/98 3:40p Dave
 * 
 * 1     11/13/98 3:28p Dave
 * 
 * 
 * $NoKeywords: $
 */

#include "shipcontrails.h"
#include "object.h"
#include "ship.h"
#include "linklist.h"
#include "3d.h"
#include "alphacolors.h"
#include "trails.h"
#include "bmpman.h"
#include "missionparse.h"

// ----------------------------------------------------------------------------------------------
// CONTRAIL DEFINES/VARS
//

// ----------------------------------------------------------------------------------------------
// CONTRAIL FORWARD DECLARATIONS
//

// if the object is below the limit for contrails
int ct_below_limit(object *objp);

// if an object has active contrails
int ct_has_contrails(ship *shipp);

// update active contrails - moving existing ones, adding new ones where necessary
void ct_update_contrails(ship *shipp);

// create new contrails
void ct_create_contrails(ship *shipp);

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
		shipp->trail_num[idx] = (short)-1;
	}
}

// call when a ship is deleted to free up its contrail stuff
void ct_ship_delete(ship *shipp)
{
	int idx;		

	Assert(shipp != NULL);
	// free up any contrails this guy may have had
	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->trail_num[idx] >= 0){
			trail_object_died(shipp->trail_num[idx]);
			shipp->trail_num[idx] = (short)-1;
		}
	}
}

// call each frame for processing a ship's contrails
void ct_ship_process(ship *shipp)
{
#ifdef MULTIPLAYER_BETA_BUILD
	return;
#else
	int idx;		
	object *objp;

	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	objp = &Objects[shipp->objnum];

	// if not a fullneb mission - do nothing
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		return;
	}

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
			if(shipp->trail_num[idx] >= 0){
				trail_object_died(shipp->trail_num[idx]);
				shipp->trail_num[idx] = (short)-1;
			}
		}

		// were done
		return;
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
	return objp->phys_info.fspeed < 45.0f;
}

// if a ship has active contrails
int ct_has_contrails(ship *shipp)
{
	int idx;

	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->trail_num[idx] >= 0){
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
	vector v1;
	matrix m;
	int idx;
	ship_info *sip;
	object *objp;

	// if not a fullneb mission - do nothing
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		return;
	}

	// get object and ship info
	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	Assert(shipp->ship_info_index >= 0);
	objp = &Objects[shipp->objnum];
	sip = &Ship_info[shipp->ship_info_index];

	// get the inverse rotation matrix
	vm_copy_transpose_matrix(&m, &objp->orient);

	// process each contrail	
	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		// if this is a valid contrail
		if(shipp->trail_num[idx] >= 0){	
			// get the point for the contrail
			vm_vec_rotate(&v1, &sip->ct_info[idx].pt, &m);
			vm_vec_add2(&v1, &objp->pos);
		
			// if the spew stamp has elapsed
			if(trail_stamp_elapsed(shipp->trail_num[idx])){	
				trail_add_segment(shipp->trail_num[idx], &v1);
				trail_set_stamp(shipp->trail_num[idx]);
			} else {
				trail_set_segment(shipp->trail_num[idx], &v1);
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
	vector v1;
	int idx;
	matrix m;
	ship_info *sip;
	object *objp;

	// if not a fullneb mission - do nothing
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		return;
	}

	// get object and ship info
	Assert(shipp != NULL);
	Assert(shipp->objnum >= 0);
	Assert(shipp->ship_info_index >= 0);
	objp = &Objects[shipp->objnum];
	sip = &Ship_info[shipp->ship_info_index];

	// get the inverse rotation matrix
	vm_copy_transpose_matrix(&m, &objp->orient);

	for(idx=0; idx<sip->ct_count; idx++){
		shipp->trail_num[idx] = (short)trail_create(sip->ct_info[idx]);	

		// add the point		
		vm_vec_rotate(&v1, &sip->ct_info[idx].pt, &m);
		vm_vec_add2(&v1, &objp->pos);
		trail_add_segment(shipp->trail_num[idx], &v1);
		trail_add_segment(shipp->trail_num[idx], &v1);
	}
#endif
}
