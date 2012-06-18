/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "weapon/flak.h"
#include "weapon/muzzleflash.h"
#include "object/object.h"

// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK FUNCTIONS
//

/**
 * Initialize flak stuff for the level
 */
void flak_level_init()
{
}

/**
 * Close down flak stuff
 */
void flak_level_close()
{
}

/**
 * Given a just fired flak shell, pick a detonating distance for it
 */
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

	weapon_info* wip = &Weapon_info[Weapons[objp->instance].weapon_info_index];

	// get the range to the target
	vm_vec_sub(&temp, &objp->pos, predicted_target_pos);
	final_range = vm_vec_mag(&temp);

	//Is it larger than det_range?
	det_range = wip->det_range;
	if(det_range != 0.0f && final_range > det_range)
	{
		flak_set_range(objp, det_range);
		return;
	}

	// add in some randomness
	float random_range = ((wip->flak_detonation_accuracy + (wip->flak_detonation_accuracy * 0.65f * (1.0f - weapon_subsys_strength))) * frand_range(-1.0f, 1.0f));
	final_range += random_range;	

	// make sure we're firing at least 10 meters away, or the weapons' arm distance if one was defined.
	if (wip->arm_dist > 0.0f) {
		if(final_range < wip->arm_dist){
			final_range = wip->arm_dist;
		} 
	} else {
		if (final_range < 10.0f) {
			final_range = 10.0f;
		}
	}

	// set the range
	flak_set_range(objp, final_range);
}

/**
 * Add some jitter to a flak gun's aiming direction, take into account range to target so that we're never _too_ far off
 * assumes dir is normalized
 */
void flak_jitter_aim(vec3d *dir, float dist_to_target, float weapon_subsys_strength, weapon_info* wip)
{			
	vec3d rand_twist_pre, rand_twist_post;		
	matrix temp;
	vec3d final_aim;
	float error_val;
	
	// get the matrix needed to rotate the base direction to the actual direction		
	vm_vector_2_matrix(&temp, dir, NULL, NULL);

	// error value	
	error_val = wip->flak_targeting_accuracy + (wip->flak_targeting_accuracy * 0.65f * (1.0f - weapon_subsys_strength));
	
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

/**
 * Create a muzzle flash from a flak gun based upon firing position and weapon type
 */
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

	if(Weapon_info[turret_weapon_class].muzzle_flash < 0){
		return;
	}

	mflash_create(pos, dir, pip, Weapon_info[turret_weapon_class].muzzle_flash);
}

/**
 * Given a just fired flak shell, pick a detonating distance for it
 */
void flak_set_range(object *objp, float range)
{
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);	

	// setup the flak info
	Weapons[objp->instance].det_range = range;
}

/**
 * Get the current range for the flak object
 */
float flak_get_range(object *objp)
{
	Assert(objp->type == OBJ_WEAPON);
	Assert(objp->instance >= 0);	
	
	return Weapons[objp->instance].det_range;
}
