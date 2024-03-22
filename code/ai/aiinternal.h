/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _AIINTERNAL_H
#define _AIINTERNAL_H

#include "ship/ship.h"
#include "weapon/weapon.h"

//Returns true if the specified object is a stealth ship, false if not
bool is_object_stealth_ship(const object* objp);

//Number of live turrets on the object attacking
int num_turrets_attacking(const object *turret_parent, int target_objnum);

//Determine whether an object is targetable within a nebula (checks for stealth)
bool object_is_targetable(const object *target, const ship *viewer);

//Returns the number of enemy fighters within threshold of pos.
int num_nearby_fighters(int enemy_team_mask, const vec3d *pos, float threshold);

//Returns true if OK for *aip to fire its current weapon at its current target.
bool check_ok_to_fire(int objnum, int target_objnum, const weapon_info *wip, int secondary_bank, const vec3d *firing_pos_global);

//Returns true if *aip has a line of sight to its current target.
bool check_los(int objnum, int target_objnum, float threshold, int primary_bank, int secondary_bank, const vec3d *firing_pos_global);

//Does all the stuff needed to aim and fire a turret.
void ai_turret_execute_behavior(const ship *shipp, ship_subsys *ss);

#endif
