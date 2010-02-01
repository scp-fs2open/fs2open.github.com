/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "weapon/corkscrew.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "io/timer.h"
#include "freespace2/freespace.h"	// for Missiontime
#include "object/object.h"



// corkscrew structure flags
#define CS_FLAG_USED						(1<<0)		// this structure is in use
#define CS_FLAG_COUNTER					(1<<1)		// counterrotate this guy

// corkscrew settings
int Corkscrew_missile_delay			= 30;			// delay between missile firings
int Corkscrew_num_missiles_fired		= 4;			// # of missiles fire in one shot
float Corkscrew_radius					= 1.25f;		// radius of the corkscrew itself
float Corkscrew_twist					= 5.0f;		// in degrees/second
int Corkscrew_helix						= 1;			// attempt to point the missile in the right direction
int Corkscrew_counterrotate			= 1;			// counterrotate every other missile
int Corkscrew_shrink						= 0;			// shrink the radius of every successive missile
float Corkscrew_shrink_val				= 0.3f;		// rate at which the radius shrinks
int Corkscrew_down_first				= 1;			// have the corkscrew go "down" first

// current counterrotation and radius shrink values
float Corkscrew_radius_cur = Corkscrew_radius;

typedef struct cscrew_info {		
	int flags;												// flags for the missile
	
	// info about the corkscrew effect for this missile
	vec3d cen_p;											// vector pointing to the "center" of the corkscrew	
	float radius;											// radius of the corkscrew
	matrix real_orient;									// the orientation used when calling physics (bashed before rendering)
	vec3d last_corkscrew_pos;							// last position along the corkscrew
} cscrew_info;

#define MAX_CORKSCREW_MISSILES	200
cscrew_info	Corkscrew_missiles[MAX_CORKSCREW_MISSILES];

// ------------------------------------------------------------------
// cscrew_level_init()
//
// Called at the start of each new mission
//
void cscrew_level_init()
{
	memset(Corkscrew_missiles, 0, sizeof(cscrew_info) * MAX_CORKSCREW_MISSILES);
}

// ------------------------------------------------------------------
// cscrew_maybe_fire_missile()
//
// Check if there are any swarm missiles to fire, and if enough time
// has elapsed since last one fired, go ahead and fire it.
//
// This is called once per ship frame in ship_move()
//
void cscrew_maybe_fire_missile(int shipnum)
{
	ship			*sp;
	ship_weapon *swp;
	int			weapon_info_index;

	Assert(shipnum >= 0 && shipnum < MAX_SHIPS );
	sp = &Ships[shipnum];

	// make sure we're supposed to be firing some missiles
	if ( sp->num_corkscrew_to_fire <= 0 ){
		return;
	}

	// make sure we have a valid weapon band
	swp = &sp->weapons;
	if ( swp->current_secondary_bank == -1 ) {
		sp->num_corkscrew_to_fire = 0;
		return;
	}

	weapon_info_index = swp->secondary_bank_weapons[swp->current_secondary_bank];
	Assert( weapon_info_index >= 0 && weapon_info_index < MAX_WEAPON_TYPES );

	// if current secondary bank is not a corkscrew missile, return
	if ( !(Weapon_info[weapon_info_index].wi_flags & WIF_CORKSCREW) ) {
		sp->num_corkscrew_to_fire = 0;
		return;
	}

	if ( timestamp_elapsed(sp->next_corkscrew_fire) ) {
		sp->next_corkscrew_fire = timestamp(Weapon_info[weapon_info_index].cs_delay);
		ship_fire_secondary( &Objects[sp->objnum], 1 );
		sp->num_corkscrew_to_fire--;
	}
}

// ------------------------------------------------------------------
// cscrew_create()
//
//	Get a free corkscrew missile entry, and initialize the struct members
//
int cscrew_create(object *obj)
{
	int			i;
	cscrew_info	*cscrewp = NULL;	
	weapon_info *wip;

	wip = &Weapon_info[Weapons[obj->instance].weapon_info_index];
	
	for ( i = 0; i < MAX_CORKSCREW_MISSILES; i++ ) {
		cscrewp = &Corkscrew_missiles[i];
		if ( !(cscrewp->flags & CS_FLAG_USED) ) {
			break;		
		}
	}

	if ( i >= MAX_CORKSCREW_MISSILES ) {
		nprintf(("Warning","No more corkscrew missiles are available\n"));
		return -1;
	}

	// mark the guy as "used"
	cscrewp->flags = CS_FLAG_USED;

	// determine if he is counterrotating
	if(wip->cs_crotate){
		if(frand_range(0.0f, 1.0f) < 0.5f){
			cscrewp->flags |= CS_FLAG_COUNTER;
		}		
	}

	// get the "center" pointing vector
	vec3d neg;
	neg = obj->orient.vec.uvec;
	if(Corkscrew_down_first){
		vm_vec_negate(&neg);
	}
	vm_vec_scale_add2(&cscrewp->cen_p, &neg, wip->cs_radius);	

	// move the missile up so that the corkscrew point is at the muzzle of the gun
	// vm_vec_scale_add2(&obj->pos, &obj->orient.vec.uvec, Corkscrew_radius);

	// store some initial helix params
	cscrewp->real_orient = obj->orient;
	cscrewp->last_corkscrew_pos = obj->pos;
	
	return i;
}

// ------------------------------------------------------------------
// cscrew_delete()
//
//
void cscrew_delete(int i)
{	
	if ( !(Corkscrew_missiles[i].flags & CS_FLAG_USED) ) {
		Int3();	
	}

	memset(&Corkscrew_missiles[i], 0, sizeof(cscrew_info));
}

// pre process the corkscrew weapon by putting him in the "center" of his corkscrew
void cscrew_process_pre(object *objp)
{		
	cscrew_info *ci;
	
	// check stuff
	Assert(objp->type == OBJ_WEAPON);	
	Assert(Weapons[objp->instance].cscrew_index >= 0);
	Assert(Corkscrew_missiles[Weapons[objp->instance].cscrew_index].flags & CS_FLAG_USED);

	ci = &Corkscrew_missiles[Weapons[objp->instance].cscrew_index];

	// unrotate the missile itself
	if(Corkscrew_helix){
		// restore the "real" matrix now
		objp->orient = ci->real_orient;		
	}
	// move the missile back to the center of the corkscrew	
	vm_vec_add2(&objp->pos, &ci->cen_p);	
}

// post process the corkscrew weapon by putting him back to the right spot on his corkscrew
void cscrew_process_post(object *objp)
{	
	vec3d cen, neg;
	vec3d new_pt;	
	weapon *wp;
	weapon_info *wip;
	cscrew_info *ci;
	float twist_val;

	// check stuff
	Assert(objp->type == OBJ_WEAPON);	
	Assert(Weapons[objp->instance].cscrew_index >= 0);
	Assert(Corkscrew_missiles[Weapons[objp->instance].cscrew_index].flags & CS_FLAG_USED);

	// get various useful pointers
	wp = &Weapons[objp->instance];
	wip = &Weapon_info[wp->weapon_info_index];
	ci = &Corkscrew_missiles[wp->cscrew_index];

	// move to the outside of the corkscrew			
	neg = ci->cen_p;
	cen = objp->pos;	
	vm_vec_negate(&neg);		
	vm_vec_add2(&objp->pos, &neg);		

	// determine what direction (clockwise or counterclockwise) the missile will spin	
	twist_val = ci->flags & CS_FLAG_COUNTER ? -wip->cs_twist : wip->cs_twist;
	twist_val *= flFrametime;	
	
	// rotate the missile position
	vm_rot_point_around_line(&new_pt, &objp->pos, twist_val, &cen, &objp->orient.vec.fvec);	
	objp->pos = new_pt;

	// rotate the missile itself
	if(Corkscrew_helix){		
		vec3d dir;
	
		// compute a "fake" orient and store the old one for safekeeping
		ci->real_orient = objp->orient;
		vm_vec_sub(&dir, &objp->pos, &ci->last_corkscrew_pos);
		vm_vec_normalize(&dir);
		vm_vector_2_matrix(&objp->orient, &dir, NULL, NULL);	
		
		// mark down this position so we can orient nicely _next_ frame
		ci->last_corkscrew_pos = objp->pos;
	}	

	// get the new center pointing vector
	vm_vec_sub(&ci->cen_p, &cen, &objp->pos);

	// do trail stuff here
	if ( wp->trail_ptr != NULL )	{
		if (trail_stamp_elapsed(wp->trail_ptr)) {
			trail_add_segment( wp->trail_ptr, &objp->pos );
			trail_set_stamp(wp->trail_ptr);
		} else {
			trail_set_segment( wp->trail_ptr, &objp->pos );
		}
	}	
}

// debug console functionality
void cscrew_display_dcf()
{
	dc_printf("Corkscrew settings\n\n");
	dc_printf("Delay (cscrew_delay) : %d\n",Corkscrew_missile_delay);
	dc_printf("Count (cscrew_count) : %d\n",Corkscrew_num_missiles_fired);
	dc_printf("Radius (cscrew_radius) :	%f\n",Corkscrew_radius);
	dc_printf("Twist (cscrew_twist) : %f\n",Corkscrew_twist);	
	if(Corkscrew_helix){
		dc_printf("Helix (cscrew_helix): ON\n");
	} else {
		dc_printf("Helix (cscrew_helix): OFF\n");
	}
	if(Corkscrew_counterrotate){
		dc_printf("Counterrotate (cscrew_counter): ON\n");
	} else {
		dc_printf("Counterrotate (cscrew_counter): OFF\n");
	}
	if(Corkscrew_shrink){
		dc_printf("Shrink (cscrew_shrink): ON\n");
	} else {
		dc_printf("Shrink (cscrew_shrink): OFF\n");
	}
	dc_printf("Corkscrew shrink (cscrew_shrinkval): %f\n", Corkscrew_shrink_val);
	if(Corkscrew_down_first){
		dc_printf("Corkscrew down first : ON\n");
	} else {
		dc_printf("Corkscrew down first : OFF\n");
	}
}

DCF(cscrew, "Listing of corkscrew missile debug console functions")
{
	cscrew_display_dcf();
}

DCF(cscrew_delay, "Change the delay between corkscrew firing")
{	
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){
		Corkscrew_missile_delay = Dc_arg_int;		
	}

	cscrew_display_dcf();
}

DCF(cscrew_count, "Change the # of corkscrew missiles fired")
{	
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){
		Corkscrew_num_missiles_fired = Dc_arg_int;		
	}

	cscrew_display_dcf();
}

DCF(cscrew_radius, "Change the radius of corkscrew missiles")
{	
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		Corkscrew_radius = Dc_arg_float;
	}

	cscrew_display_dcf();
}

DCF(cscrew_twist, "Change the rate of the corkscrew twist")
{
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		Corkscrew_twist = Dc_arg_float;
	}

	cscrew_display_dcf();
}

DCF(cscrew_helix, "Attempt to orient missile nicely along the corkscrew")
{
	Corkscrew_helix = !Corkscrew_helix;

	cscrew_display_dcf();
}

DCF(cscrew_counter, "Counterrotate every other missile")
{
	Corkscrew_counterrotate = !Corkscrew_counterrotate;

	cscrew_display_dcf();
}

DCF(cscrew_shrink, "Shrink the radius of every other missile")
{
	Corkscrew_shrink = !Corkscrew_shrink;

	cscrew_display_dcf();
}

DCF(cscrew_shrinkval, "Change the rate at which the radii shrink")
{
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		Corkscrew_shrink_val = Dc_arg_float;
	}

	cscrew_display_dcf();
}

DCF(cscrew_down, "Cause the missile to spiral down first")
{
	Corkscrew_down_first = !Corkscrew_down_first;

	cscrew_display_dcf();
}
