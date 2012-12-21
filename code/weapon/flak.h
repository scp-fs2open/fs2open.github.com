/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FLAK_WEAPONS_HEADER_FILE
#define _FLAK_WEAPONS_HEADER_FILE

#include "physics/physics.h"
#include "weapon/weapon.h"

// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK DEFINES/VARS
//

struct weapon;
struct object;
struct vec3d;

// --------------------------------------------------------------------------------------------------------------------------------------
// FLAK FUNCTIONS
//

// initialize flak stuff for the level
void flak_level_init();

// close down flak stuff
void flak_level_close();

// given a just fired flak shell, pick a detonating distance for it
void flak_pick_range(object *objp, vec3d *firing_pos, vec3d *predicted_target_pos, float weapon_subsys_strength);

// add some jitter to a flak gun's aiming direction, take into account range to target so that we're never _too_ far off
// assumes dir is normalized
void flak_jitter_aim(vec3d *dir, float dist_to_target, float weapon_subsys_strength, weapon_info* wip);

// create a muzzle flash from a flak gun based upon firing position and weapon type
void flak_muzzle_flash(vec3d *pos, vec3d *dir, physics_info *pip, int turret_weapon_class);

// set the range on a flak object
void flak_set_range(object *objp, float range);

// get the current range for the flak object
float flak_get_range(object *objp);

#endif
