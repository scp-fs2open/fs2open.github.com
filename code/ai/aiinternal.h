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

//Returns true if the specified object is a stealth ship, false if not
bool is_object_stealth_ship(object* objp);

//Number of live turrets on the object attacking
int num_turrets_attacking(object *turret_parent, int target_objnum);

//Determine whether an object is targetable within a nebula (checks for stealth)
int object_is_targetable(object *target, ship *viewer);

//Returns the number of enemy fighters within threshold of pos.
int num_nearby_fighters(int enemy_team_mask, vec3d *pos, float threshold);

//Returns true if OK for *aip to fire its current weapon at its current target.
int check_ok_to_fire(int objnum, int target_objnum, weapon_info *wip);

//Does all the stuff needed to aim and fire a turret.
void ai_fire_from_turret(ship *shipp, ship_subsys *ss, int parent_objnum);

// AI Turret Table Wrapper

struct eval_enemy_obj_struct;

struct aiturret_call_table
{
	int    (*turret_select_best_weapon) (ship_subsys *turret, object *target);
	int    (*valid_turret_enemy) (object *objp, object *turret_parent);
	void   (*evaluate_obj_as_target) (object *objp, eval_enemy_obj_struct *eeo);
	int    (*get_nearest_turret_objnum) (int turret_parent_objnum, ship_subsys *turret_subsys, int enemy_team_mask, vec3d *tpos, vec3d *tvec, int current_enemy, bool big_only_flag, bool small_only_flag, bool tagged_only_flag, bool beam_flag);
	int    (*find_turret_enemy) (ship_subsys *turret_subsys, int objnum, vec3d *tpos, vec3d *tvec, int current_enemy, float fov);
	int    (*turret_should_pick_new_target) (ship_subsys *turret);
	void   (*turret_set_next_fire_timestamp) (int weapon_num, weapon_info *wip, ship_subsys *turret, ai_info *aip);
	int    (*turret_should_fire_aspect) (ship_subsys *turret, float dot, weapon_info *wip);
	bool   (*turret_fire_weapon) (int weapon_num, ship_subsys *turret, int parent_objnum, vec3d *turret_pos, vec3d *turret_fvec, vec3d *predicted_pos, float flak_range_override);
	void   (*turret_swarm_fire_from_turret) (turret_swarm_info *tsi);
	void   (*ai_fire_from_turret) (ship *shipp, ship_subsys *ss, int parent_objnum);
};

// Set new pointer to override ai_goal* function calls
// possibly copying the old function calls first

extern struct aiturret_call_table *aiturret_table;

#endif
