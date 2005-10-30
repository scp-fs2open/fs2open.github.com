/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Flak.cpp $
 * $Revision: 2.6 $
 * $Date: 2005-10-30 06:44:59 $
 * $Author: wmcoolmon $
 *
 * flak functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.4  2004/07/26 20:47:56  Kazan
 * remove MCD complete
 *
 * Revision 2.3  2004/07/12 16:33:09  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.2  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 9     7/31/99 2:57p Dave
 * Scaled flak aim and jitter by weapon subsystem strength.
 * 
 * 8     5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * $NoKeywords: $
 */

#include "weapon/flak.h"
#include "weapon/weapon.h"
#include "weapon/muzzleflash.h"
#include "object/object.h"



// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK DEFINES/VARS
//

// temporary - max distance from target that a jittered flak aim direction can point at
#define FLAK_MAX_ERROR											60.0f							// aim at _most_ this far off of the predicted target position
float Flak_error = FLAK_MAX_ERROR;

// muzzle flash animation
#define MUZZLE_FLASH_FILE										"flk2"
#define MUZZLE_FLASH_RADIUS									15.0f
float Flak_muzzle_radius = MUZZLE_FLASH_RADIUS;
int Flak_muzzle_flash_ani = -1;

// muzzle flash limiting
#define FLAK_MUZZLE_MOD		3
int Flak_muzzle_mod = 0;

// flak ranging info
#define FLAK_RANGE_DEFAULT										65.0f							// spherical radius around the predicted target position
float Flak_range = FLAK_RANGE_DEFAULT;

// flak info
#define MAX_FLAK_INFO											350
typedef struct flak_info {	
	vec3d start_pos;							// initial pos
	float range;								// range at which we'll detonate (-1 if unused);
} flak_info;
flak_info Flak[MAX_FLAK_INFO];


// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK FUNCTIONS
//

// initialize flak stuff for the level
void flak_level_init()
{
	int num_frames;
	int fps;
	int idx;

	// if the muzzle flash ani is not loaded, do so
	if(Flak_muzzle_flash_ani == -1){
		Flak_muzzle_flash_ani = bm_load_animation(MUZZLE_FLASH_FILE, &num_frames, &fps, 1);
	}

	// zero out flak info
	memset(Flak, 0, sizeof(flak_info) * MAX_FLAK_INFO);
	for(idx=0; idx<MAX_FLAK_INFO; idx++){
		Flak[idx].range = -1.0f;
	}
}

// close down flak stuff
void flak_level_close()
{
	// zero out the ani (bitmap manager will take care of releasing it I think)
	if(Flak_muzzle_flash_ani != -1){
		Flak_muzzle_flash_ani = -1;
	}
}

// given a newly created weapon, turn it into a flak weapon
void flak_create(weapon *wp)
{
	int idx;
	int found;
	
	// make sure this is a valid flak weapon object
	Assert(wp->objnum >= 0);
	Assert(Objects[wp->objnum].type == OBJ_WEAPON);
	Assert(wp->weapon_info_index >= 0);
	Assert(Weapon_info[wp->weapon_info_index].wi_flags & WIF_FLAK);

	// switch off rendering for the object
	obj_set_flags(&Objects[wp->objnum], Objects[wp->objnum].flags & ~(OF_RENDERS));

	// try and find a free flak index
	found = 0;
	for(idx=0; idx<MAX_FLAK_INFO; idx++){
		if(Flak[idx].range < 0.0f){
			found = 1;
			break;
		}
	}

	// if we found a free slot
	if(found){
		Flak[idx].range = 0.0f;
		wp->flak_index = (short)idx;
	} else {
		nprintf(("General", "Out of FLAK slots!\n"));
	}
}

// free up a flak object
void flak_delete(int flak_index)
{
	Assert((flak_index >= 0) && (flak_index < MAX_FLAK_INFO));
	memset(&Flak[flak_index], 0, sizeof(flak_info));
	Flak[flak_index].range = -1;
}

// given a just fired flak shell, pick a detonating distance for it
void flak_pick_range(object *objp, vec3d *predicted_target_pos, float weapon_subsys_strength)
{
	float final_range;
	vec3d temp;
	
	// make sure this flak object is valid
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);
	Assert(Weapons[objp->instance].weapon_info_index >= 0);
	Assert(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_FLAK);	
	
	// if the flak index is invalid, do nothing - if this fails the flak simply becomes a non-rendering bullet
	if(Weapons[objp->instance].flak_index < 0){
		return;
	}

	// get the range to the target
	vm_vec_sub(&temp, &objp->pos, predicted_target_pos);
	final_range = vm_vec_mag(&temp);

	// add in some randomness
	final_range += (Flak_range + (Flak_range * 0.65f * (1.0f - weapon_subsys_strength))) * frand_range(-1.0f, 1.0f);	

	// make sure we're firing at least 10 meters away
	if(final_range < 10.0f){
		final_range = 10.0f;
	}

	// set the range
	flak_set_range(objp, &objp->pos, final_range);
}

// add some jitter to a flak gun's aiming direction, take into account range to target so that we're never _too_ far off
// assumes dir is normalized
void flak_jitter_aim(vec3d *dir, float dist_to_target, float weapon_subsys_strength)
{			
	vec3d rand_twist_pre, rand_twist_post;		
	matrix temp;
	vec3d final_aim;
	float error_val;
	
	// get the matrix needed to rotate the base direction to the actual direction		
	vm_vector_2_matrix(&temp, dir, NULL, NULL);

	// error value	
	error_val = Flak_error + (Flak_error * 0.65f * (1.0f - weapon_subsys_strength));
	
	// scale the rvec by some random value and make it the "pre-twist" value
	float rand_dist = frand_range(0.0f, error_val);
	// no jitter - so do nothing
	if(rand_dist <= 0.0f){
		return;
	}
	vm_vec_copy_scale(&rand_twist_pre, &temp.vec.rvec, rand_dist);

	// now rotate the twist vector around the x axis (the base aim axis) at a random angle
	vm_rot_point_around_line(&rand_twist_post, &rand_twist_pre, fl_radian(359.0f * frand_range(0.0f, 1.0f)), &vmd_zero_vector, dir);

	// add the resulting vector to the base aim vector and normalize
	final_aim = *dir;
	vm_vec_scale(&final_aim, dist_to_target);
	vm_vec_add(dir, &final_aim, &rand_twist_post);
	vm_vec_normalize(dir);
}

// create a muzzle flash from a flak gun based upon firing position and weapon type
void flak_muzzle_flash(vec3d *pos, vec3d *dir, physics_info *pip, int turret_weapon_class)
{
	// sanity
	Assert((turret_weapon_class >= 0) && (turret_weapon_class < Num_weapon_types));
	if((turret_weapon_class < 0) || (turret_weapon_class >= Num_weapon_types)){
		return;
	}
	Assert(Weapon_info[turret_weapon_class].wi_flags & WIF_FLAK);
	if(!(Weapon_info[turret_weapon_class].wi_flags & WIF_FLAK)){
		return;
	}
	Assert(Weapon_info[turret_weapon_class].wi_flags & WIF_MFLASH);
	if(!(Weapon_info[turret_weapon_class].wi_flags & WIF_MFLASH)){
		return;
	}
	Assert(Weapon_info[turret_weapon_class].muzzle_flash >= 0);
	if(Weapon_info[turret_weapon_class].muzzle_flash < 0){
		return;
	}

	// maybe skip this flash
	if(!(Flak_muzzle_mod % FLAK_MUZZLE_MOD)){
		// call the muzzle flash code
		mflash_create(pos, dir, pip, Weapon_info[turret_weapon_class].muzzle_flash);
	}

	Flak_muzzle_mod++;
	if(Flak_muzzle_mod >= 10000){
		Flak_muzzle_mod = 0;
	}	
}

// maybe detonate a flak shell early/late (call from weapon_process_pre(...))
void flak_maybe_detonate(object *objp)
{			
	vec3d temp;	

	// multiplayer clients should never detonate flak early
	// if(MULTIPLAYER_CLIENT){
		// return;
	// }

	// if the shell has gone past its range, blow it up
	vm_vec_sub(&temp, &objp->pos, &Flak[Weapons[objp->instance].flak_index].start_pos);
	if(vm_vec_mag(&temp) >= Flak[Weapons[objp->instance].flak_index].range){
		weapon_detonate(objp);		
	}
}

// given a just fired flak shell, pick a detonating distance for it
void flak_set_range(object *objp, vec3d *start_pos, float range)
{
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);	
	Assert(Weapons[objp->instance].flak_index >= 0);

	// setup the flak info
	Flak[Weapons[objp->instance].flak_index].range = range;
	Flak[Weapons[objp->instance].flak_index].start_pos = *start_pos;
}

// get the current range for the flak object
float flak_get_range(object *objp)
{
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);	
	Assert(Weapons[objp->instance].flak_index >= 0);
	
	return Flak[Weapons[objp->instance].flak_index].range;
}

DCF(flak, "show flak dcf commands")
{
	dc_printf("flak_err <float>      : set the radius of error for flak targeting\n");	
	dc_printf("flak_range <float>		: set the radius of error for detonation of a flak shell\n");
	dc_printf("flak_rad <float>      : set the radius for the muzzle flash on a flak gun\n");
}

DCF(flak_err, "set the radius of error for flak targeting")
{
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){		 
		Flak_error = Dc_arg_float;
	}
}

DCF(flak_range, "set the radius of error for detonation of a flak shell")
{
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){		 
		Flak_range = Dc_arg_float;
	}
}

DCF(flak_rad, "set the radius of flak gun muzzle flash")
{
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){		 
		Flak_muzzle_radius = Dc_arg_float;
	}
}
