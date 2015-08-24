/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/systemvars.h"
#include "object/object.h"
#include "observer/observer.h"
#include "playerman/player.h"




observer Observers[MAX_OBSERVER_OBS];

void observer_init()
{
	int idx;
	for(idx=0;idx<MAX_OBSERVER_OBS;idx++){
		Observers[idx].flags = 0;
		Observers[idx].objnum = -1;
		Observers[idx].target_objnum = -1;
	}
}

int observer_create(matrix *orient, vec3d *pos)
{
	int objnum,idx;	
	physics_info *pi;

	// try and find the first free observer slot
	for(idx=0;idx<MAX_OBSERVER_OBS;idx++){
		if(!(Observers[idx].flags & OBS_FLAG_USED))
			break;
	}
	// if we couldn't find an open slot
	if(idx == MAX_OBSERVER_OBS){
		mprintf(("Ran out of observer slots!\n"));
		return -1;
	}
	
	// attempt to create the object
	objnum = obj_create(OBJ_OBSERVER,-1,idx,orient,pos,1.0f,0);

	// fail situation
	if(objnum == -1)
		return -1;

	// give the observer Descent style physics
	Objects[objnum].flags |= OF_PHYSICS;
	physics_init(&Objects[objnum].phys_info);
	pi = &Objects[objnum].phys_info;
	pi->flags |= PF_ACCELERATES | PF_SLIDE_ENABLED;
	
	
	// setup some physics parameters
	pi->max_vel.xyz.x = OBS_MAX_VEL_X;
	pi->max_vel.xyz.y = OBS_MAX_VEL_Y;
	pi->max_vel.xyz.z = OBS_MAX_VEL_Z;	
	vm_vec_zero(&pi->prev_ramp_vel);
	vm_vec_zero(&pi->desired_vel);
	vm_vec_zero(&pi->desired_rotvel);
	vm_vec_zero(&pi->vel);
	vm_vec_zero(&pi->rotvel);
	vm_vec_zero(&pi->prev_fvec);
	vm_set_identity(&pi->last_rotmat);
	pi->forward_thrust = 0.0f;
	pi->speed = 0.0f;
	pi->fspeed = 0.0f;
	pi->heading = 0.0f;
		
	// set up the observer data
	Observers[idx].flags |= OBS_FLAG_USED;
	Observers[idx].objnum = objnum;
	Observers[idx].target_objnum = -1;

	return objnum;
}

void observer_delete(object *obj)
{
	int num;
	
	num = obj->instance;
	Assert( Observers[num].objnum == OBJ_INDEX(obj));

	Observers[num].objnum = -1;
	Observers[num].target_objnum = -1;
	Observers[num].flags = 0;           // mark it as being free
}

// get the eye position and orientation for the passed observer object
void observer_get_eye(vec3d *eye_pos, matrix *eye_orient, object *obj)
{
	// copy in the observer position and orientation
	memcpy(eye_pos,&obj->pos,sizeof(vec3d));
	memcpy(eye_orient,&obj->orient,sizeof(matrix));
}
