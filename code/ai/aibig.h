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

struct object;
struct ai_info;
struct vec3d;
struct ship_subsys;

void	ai_big_ship(object *objp);
void	ai_big_chase();
void	ai_big_subsys_path_cleanup(ai_info *aip);

// strafe functions
void	ai_big_strafe();
int	ai_big_maybe_enter_strafe_mode(object *objp, int weapon_objnum, int consider_target_only=0);
void	ai_big_strafe_maybe_attack_turret(object *ship_objp, object *weapon_objp);
void ai_big_pick_attack_point(object *objp, object *attacker_objp, vec3d *attack_point, float fov=1.0f);
void ai_big_pick_attack_point_turret(object *objp, ship_subsys *ssp, vec3d *gpos, vec3d *gvec, vec3d *attack_point, float weapon_travel_dist, float fov=1.0f);


#endif

