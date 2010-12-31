/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
//WMC - Made this generic weapon stuffs
/*
#define MAX_FLAK_INFO											350
typedef struct flak_info {	
	vec3d start_pos;							// initial pos
	float range;								// range at which we'll detonate (-1 if unused);
} flak_info;
flak_info Flak[MAX_FLAK_INFO];
*/


// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK FUNCTIONS
//

// initialize flak stuff for the level
void flak_level_init()
{
	int num_frames;
	int fps;
	//int idx;

	// if the muzzle flash ani is not loaded, do so
	if(Flak_muzzle_flash_ani == -1){
		Flak_muzzle_flash_ani = bm_load_animation(MUZZLE_FLASH_FILE, &num_frames, &fps, NULL, 1);
	}

	// zero out flak info
	/*
	memset(Flak, 0, sizeof(flak_info) * MAX_FLAK_INFO);
	for(idx=0; idx<MAX_FLAK_INFO; idx++){
		Flak[idx].range = -1.0f;
	}
	*/
}

// close down flak stuff
void flak_level_close()
{
	// zero out the ani (bitmap manager will take care of releasing it I think)
	//WMC - Check to make sure
	if(Flak_muzzle_flash_ani != -1)
	{
		if(bm_is_valid(Flak_muzzle_flash_ani))
			bm_unload(Flak_muzzle_flash_ani);
		else
			Flak_muzzle_flash_ani = -1;
	}
}

// given a newly created weapon, turn it into a flak weapon
/*
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
}*/

// free up a flak object
/*
void flak_delete(int flak_index)
{
	Assert((flak_index >= 0) && (flak_index < MAX_FLAK_INFO));
	memset(&Flak[flak_index], 0, sizeof(flak_info));
	Flak[flak_index].range = -1;
}
*/

// given a just fired flak shell, pick a detonating distance for it
void flak_pick_range(object *objp, vec3d *firing_pos, vec3d *predicted_target_pos, float weapon_subsys_strength)
{
	float final_range;
	float det_range;
	vec3d temp;
	
	// make sure this flak object is valid
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);
	Assert(Weapons[objp->instance].weapon_info_index >= 0);
	Assert(Weapon_info[Weapons[objp->instance].weapon_info_index].wi_flags & WIF_FLAK);	
	
	// if the flak index is invalid, do nothing - if this fails the flak simply becomes a non-rendering bullet
	/*
	if(Weapons[objp->instance].flak_index < 0){
		return;
	}
	*/

	// get the range to the target
	vm_vec_sub(&temp, &objp->pos, predicted_target_pos);
	final_range = vm_vec_mag(&temp);

	//Is it larger than det_range?
	det_range = Weapon_info[Weapons[objp->instance].weapon_info_index].det_range;
	if(det_range != 0.0f && final_range > det_range)
	{
		flak_set_range(objp, det_range);
		return;
	}

	// add in some randomness
	final_range += (Flak_range + (Flak_range * 0.65f * (1.0f - weapon_subsys_strength))) * frand_range(-1.0f, 1.0f);	

	// make sure we're firing at least 10 meters away
	if(final_range < 10.0f){
		final_range = 10.0f;
	}

	// set the range
	flak_set_range(objp, final_range);
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
	vm_rot_point_around_line(&rand_twist_post, &rand_twist_pre, fl_radians(359.0f * frand_range(0.0f, 1.0f)), &vmd_zero_vector, dir);

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

//	Assert(Weapon_info[turret_weapon_class].muzzle_flash >= 0);
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
/*
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
*/

// given a just fired flak shell, pick a detonating distance for it
void flak_set_range(object *objp, float range)
{
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);	

	// setup the flak info
	Weapons[objp->instance].det_range = range;
	//Flak[Weapons[objp->instance].flak_index].start_pos = *start_pos;
}

// get the current range for the flak object
float flak_get_range(object *objp)
{
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);	
	
	return Weapons[objp->instance].det_range;
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
