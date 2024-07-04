/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __AIBIG_H__
#define __AIBIG_H__

class object;
struct ai_info;
struct vec3d;
class ship_subsys;

void	ai_big_chase();
void	ai_big_subsys_path_cleanup(ai_info *aip);

// strafe functions
void	ai_big_strafe();
int	ai_big_maybe_enter_strafe_mode(const object *objp, int weapon_objnum);
void	ai_big_strafe_maybe_attack_turret(const object *ship_objp, const object *weapon_objp);
void ai_big_pick_attack_point(const object *objp, const object *attacker_objp, vec3d *attack_point, float fov=1.0f);
void ai_big_pick_attack_point_turret(const object *objp, ship_subsys *ssp, const vec3d *gpos, const vec3d *gvec, vec3d *attack_point, float weapon_travel_dist, float fov=1.0f);

// default distance for following subsystem path points --wookieejedi
// the value is 5 because that was the original value specified in ai_big_maybe_follow_subsys_path()
const int Default_subsystem_path_pt_dist = 5;
const int Minimum_subsystem_path_pt_dist = 1;

#endif

