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

typedef struct eval_nearest_objnum {
	int	objnum;
	object *trial_objp;
	int	enemy_team_mask;
	int	enemy_wing;
	float	range;
	int	max_attackers;
	int	nearest_objnum;
	float	nearest_dist;
	int	check_danger_weapon_objnum;
} eval_nearest_objnum;

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

// AI Code Table Wrapper
struct aicode_call_table
{
	void	(*ai_set_rearm_status) (int team, int time);
	int	(*ai_good_time_to_rearm) (object *objp);
	void	(*ai_good_secondary_time) (int team, int weapon_index, int max_fire_count, char *shipname);
	int	(*is_preferred_weapon) (int weapon_num, object *firer_objp, object *target_objp);
	void	(*ai_init_secondary_info) ();
	void	(*free_ai_stuff) ();
	void	(*parse_ai_class) ();
	void	(*reset_ai_class_names) ();
	void	(*parse_aitbl) ();
	void	(*ai_init) ();
	void	(*ai_level_init) ();
	int	(*ai_is_stealth_visible) (object *viewer_objp, object *stealth_objp);
	void	(*update_ai_stealth_info_with_error) (ai_info *aip);
	void	(*ai_update_danger_weapon) (int attacked_objnum, int weapon_objnum);
	void	(*ai_turn_towards_vector) (vec3d *dest, object *objp, float frametime, float turn_time, vec3d *slide_vec, vec3d *rel_pos, float bank_override, int flags, vec3d *rvec, int sexp_flags);
	void	(*init_ship_info) ();
	int	(*set_target_objnum) (ai_info *aip, int objnum);
	ship_subsys*	(*set_targeted_subsys) (ai_info *aip, ship_subsys *new_subsys, int parent_objnum);
	void	(*ai_object_init) (object *obj, int ai_index);
	void	(*adjust_accel_for_docking) (ai_info *aip);
	void	(*accelerate_ship) (ai_info *aip, float accel);
	void	(*change_acceleration) (ai_info *aip, float delta_accel);
	void	(*set_accel_for_target_speed) (object *objp, float tspeed);
	void	(*turn_towards_point) (object *objp, vec3d *point, vec3d *slide_vec, float bank_override);
	void	(*turn_away_from_point) (object *objp, vec3d *point, float bank_override);
	float	(*turn_towards_tangent) (object *objp, vec3d *point, float radius);
	float	(*turn_toward_tangent_with_axis) (object *objp, object *center_objp, float radius);
	int	(*object_is_targetable) (object *target, ship *viewer);
	int	(*num_enemies_attacking) (int objnum);
	float	(*get_wing_lowest_max_speed) (object *objp);
	int	(*is_ignore_object_sub) (int *ignore_objnum, int *ignore_signature, int objnum);
	int	(*find_ignore_new_object_index) (ai_info *aip, int objnum);
	int	(*is_ignore_object) (ai_info *aip, int objnum, int just_the_original=0);
	int	(*get_nearest_bbox_point) (object *ship_obj, vec3d *start, vec3d *box_pt);
	void	(*evaluate_object_as_nearest_objnum) (eval_nearest_objnum *eno);
	int	(*get_nearest_objnum) (int objnum, int enemy_team_mask, int enemy_wing, float range, int max_attackers);
	int	(*find_nearby_threat) (int objnum, int enemy_team_mask, float range, int *count);
	int	(*num_turrets_attacking) (object *turret_parent, int target_objnum);
	int	(*get_enemy_timestamp) ();
	int	(*find_enemy) (int objnum, float range, int max_attackers);
	void	(*ai_set_goal_maybe_abort_dock) (object *objp, ai_info *aip);
	void	(*force_avoid_player_check) (object *objp, ai_info *aip);
	void	(*ai_attack_object) (object *attacker, object *attacked, int priority, ship_subsys *ssp);
	void	(*ai_attack_wing) (object *attacker, int wingnum, int priority);
	void	(*ai_evade_object) (object *evader, object *evaded, int priority);
	int	(*compact_ignore_new_objects) (ai_info *aip, int force=0);
	void	(*ai_ignore_object) (object *ignorer, object *ignored, int priority, int ignore_new);
	void	(*ai_ignore_wing) (object *ignorer, int wingnum, int priority);
	void	(*create_model_path) (object *pl_objp, object *mobjp, int path_num, int subsys_path);
	void	(*create_model_exit_path) (object *pl_objp, object *mobjp, int path_num, int count);
	void	(*ai_find_path) (object *pl_objp, int objnum, int path_num, int exit_flag, int subsys_path);
	int	(*maybe_avoid_player) (object *objp, vec3d *goal_pos);
	void	(*ai_stay_still) (object *still_objp, vec3d *view_pos);
	void	(*ai_do_objects_docked_stuff) (object *docker, int docker_point, object *dockee, int dockee_point);
	void	(*ai_do_objects_undocked_stuff) (object *docker, object *dockee);
	void	(*ai_dock_with_object) (object *docker, int docker_index, object *dockee, int dockee_index, int priority, int dock_type);
	void	(*ai_start_fly_to_ship) (object *objp, int shipnum);
	void	(*ai_start_waypoints) (object *objp, int waypoint_list_index, int wp_flags);
	void	(*ai_do_stay_near) (object *objp, object *other_objp, float dist);
	void	(*ai_do_safety) (object *objp);
	void	(*ai_form_on_wing) (object *objp, object *goal_objp);
	int	(*ai_formation_object_get_slotnum) (int objnum, object *objp);
	float	(*compute_time_to_enemy) (float dist_to_enemy, object *pobjp, object *eobjp);
	void	(*ai_set_positions) (object *pl_objp, object *en_objp, ai_info *aip, vec3d *player_pos, vec3d *enemy_pos);
	float	(*maybe_recreate_path) (object *objp, ai_info *aip, int force_recreate_flag, int override_hash=0);
	void	(*set_accel_for_docking) (object *objp, ai_info *aip, float dot, float dot_to_next, float dist_to_next, float dist_to_goal, ship_info *sip);
	float	(*ai_path) ();
	void	(*ai_safety_pick_spot) (object *objp);
	float	(*ai_safety_goto_spot) (object *objp);
	void	(*ai_safety_circle_spot) (object *objp);
	void	(*ai_safety) ();
	void	(*ai_fly_to_ship) ();
	void	(*ai_waypoints) ();
	void	(*avoid_ship) ();
	int	(*maybe_resume_previous_mode) (object *objp, ai_info *aip);
	int	(*ai_maybe_fire_afterburner) (object *objp, ai_info *aip);
	void	(*maybe_afterburner_after_ship_hit) (object *objp, ai_info *aip, object *en_objp);
	int	(*is_instructor) (object *objp);
	void	(*evade_weapon) ();
	void	(*slide_face_ship) ();
	void	(*evade_ship) ();
	void	(*ai_evade) ();
	int	(*ai_select_primary_weapon_OLD) (object *objp, object *other_objp, int flags);
	int	(*ai_select_primary_weapon) (object *objp, object *other_objp, int flags);
	void	(*set_primary_weapon_linkage) (object *objp);
	int	(*ai_fire_primary_weapon) (object *objp);
	void	(*ai_select_secondary_weapon) (object *objp, ship_weapon *swp, int priority1=-1, int priority2=-1);
	void	(*ai_maybe_announce_shockwave_weapon) (object *firing_objp, int weapon_index);
	int	(*check_ok_to_fire) (int objnum, int target_objnum, weapon_info *wip);
	int	(*ai_fire_secondary_weapon) (object *objp, int priority1, int priority2);
	void	(*set_predicted_enemy_pos_turret) (vec3d *predicted_enemy_pos, vec3d *gun_pos, object *pobjp, vec3d *enemy_pos, vec3d *enemy_vel, float weapon_speed, float time_enemy_in_range);
	void	(*set_predicted_enemy_pos) (vec3d *predicted_enemy_pos, object *pobjp, object *eobjp, ai_info *aip);
	void	(*ai_chase_ct) ();
	void	(*ai_chase_eb) (ai_info *aip, ship_info *sip, vec3d *predicted_enemy_pos, float dist_to_enemy);
	float	(*ai_endangered_time) (object *ship_objp, object *weapon_objp);
	float	(*ai_endangered_by_weapon) (ai_info *aip);
	int	(*ai_near_full_strength) (object *objp);
	void	(*attack_set_accel) (ai_info *aip, float dist_to_enemy, float dot_to_enemy, float dot_from_enemy);
	void	(*get_behind_ship) (ai_info *aip, ship_info *sip, float dist_to_enemy);
	int	(*avoid_player) (object *objp, vec3d *goal_pos);
	int	(*maybe_avoid_big_ship) (object *objp, object *ignore_objp, ai_info *aip, vec3d *goal_point, float delta_time);
	void	(*ai_stealth_find) ();
	void	(*ai_stealth_sweep) ();
	void	(*ai_chase_attack) (ai_info *aip, ship_info *sip, vec3d *predicted_enemy_pos, float dist_to_enemy, int modelnum);
	void	(*ai_chase_es) (ai_info *aip, ship_info *sip);
	void	(*ai_chase_ga) (ai_info *aip, ship_info *sip);
	int	(*ai_set_attack_subsystem) (object *objp, int subnum);
	void	(*ai_set_guard_vec) (object *objp, object *guard_objp);
	void	(*ai_set_guard_wing) (object *objp, int wingnum);
	void	(*ai_set_evade_object) (object *objp, object *other_objp);
	void	(*ai_set_guard_object) (object *objp, object *other_objp);
	void	(*update_aspect_lock_information) (ai_info *aip, vec3d *vec_to_enemy, float dist_to_enemy, float enemy_radius);
	void	(*ai_chase_fly_away) (object *objp, ai_info *aip);
	int	(*has_preferred_secondary) (object *objp, object *en_objp, ship_weapon *swp);
	void	(*ai_choose_secondary_weapon) (object *objp, ai_info *aip, object *en_objp);
	float	(*set_secondary_fire_delay) (ai_info *aip, ship *shipp, weapon_info *swip);
	void	(*ai_chase_big_approach_set_goal) (vec3d *goal_pos, object *attack_objp, object *target_objp, float *accel);
	void	(*ai_chase_big_circle_set_goal) (vec3d *goal_pos, object *attack_objp, object *target_objp, float *accel);
	void	(*ai_chase_big_get_separations) (object *attack_objp, object *target_objp, vec3d *horz_vec_to_target, float *desired_separation, float *cur_separation);
	void	(*ai_chase_big_parallel_set_goal) (vec3d *goal_pos, object *attack_objp, object *target_objp, float *accel);
	void	(*ai_cruiser_chase_set_goal_pos) (vec3d *goal_pos, object *pl_objp, object *en_objp);
	int	(*maybe_hack_cruiser_chase_abort) ();
	void	(*ai_cruiser_chase) ();
	void	(*ai_chase) ();
	int	(*num_ships_attacking) (int target_objnum);
	void	(*remove_farthest_attacker) (int objnum);
	int	(*ai_maybe_limit_attackers) (int attacked_objnum);
	void	(*guard_object_was_hit) (object *guard_objp, object *hitter_objp);
	void	(*maybe_update_guard_object) (object *hit_objp, object *hitter_objp);
	int	(*ai_guard_find_nearby_bomb) (object *guarding_objp, object *guarded_objp);
	void	(*ai_guard_find_nearby_ship) (object *guarding_objp, object *guarded_objp);
	void	(*ai_guard_find_nearby_asteroid) (object *guarding_objp, object *guarded_objp);
	void	(*ai_guard_find_nearby_object) ();
	void	(*ai_big_guard) ();
	void	(*ai_guard) ();
	void	(*ai_do_objects_repairing_stuff) (object *repaired_objp, object *repair_objp, int how);
	void	(*ai_cleanup_dock_mode_subjective) (object *objp);
	void	(*ai_cleanup_dock_mode_objective) (object *objp);
	void	(*ai_cleanup_rearm_mode) (object *objp);
	void	(*ai_still) ();
	void	(*ai_stay_near) ();
	int	(*maybe_dock_obstructed) (object *cur_objp, object *goal_objp, int big_only_flag);
	void	(*ai_dock) ();
	void	(*process_subobjects) (int objnum);
	void	(*ai_fly_in_formation) (int wingnum);
	void	(*ai_disband_formation) (int wingnum);
	int	(*formation_is_leader_chaotic) (object *objp);
	void	(*ai_most_massive_object_of_its_wing_of_all_docked_objects_helper) (object *objp, dock_function_info *infop);
	int	(*ai_formation) ();
	void	(*ai_do_repair_frame) (object *objp, ai_info *aip, float frametime);
	void	(*call_doa) (object *child, object *parent);
	void	(*ai_maybe_launch_cmeasure) (object *objp, ai_info *aip);
	void	(*ai_preprocess_ignore_objnum) (object *objp, ai_info *aip);
	void	(*ai_chase_circle) (object *objp);
	void	(*ai_transfer_shield) (object *objp, int quadrant_num);
	void	(*ai_balance_shield) (object *objp);
	void	(*ai_manage_shield) (object *objp, ai_info *aip);
	void	(*ai_maybe_evade_locked_missile) (object *objp, ai_info *aip);
	void	(*maybe_evade_dumbfire_weapon) (ai_info *aip);
	int	(*ai_acquire_emerge_path) (object *pl_objp, int parent_objnum, int allowed_path_mask, vec3d *pos, vec3d *fvec);
	void	(*ai_emerge_bay_path_cleanup) (ai_info *aip);
	void	(*ai_bay_emerge) ();
	int	(*ai_find_closest_depart_path) (ai_info *aip, polymodel *pm, int allowed_path_mask);
	int	(*ai_acquire_depart_path) (object *pl_objp, int parent_objnum, int allowed_path_mask);
	void	(*ai_bay_depart) ();
	void	(*ai_sentrygun) ();
	void	(*ai_execute_behavior) (ai_info *aip);
	int	(*maybe_request_support) (object *objp);
	void	(*ai_set_mode_warp_out) (object *objp, ai_info *aip);
	void	(*ai_maybe_depart) (object *objp);
	void	(*ai_warp_out) (object *objp);
	void	(*ai_announce_ship_dying) (object *dying_objp);
	int	(*aas_1) (object *objp, ai_info *aip, vec3d *safe_pos);
	int	(*ai_avoid_shockwave) (object *objp, ai_info *aip);
	int	(*ai_await_repair_frame) (object *objp, ai_info *aip);
	void	(*ai_maybe_self_destruct) (object *objp, ai_info *aip);
	int	(*ai_need_new_target) (object *pl_objp, int target_objnum);
	int	(*maybe_big_ship_collide_recover_frame) (object *objp, ai_info *aip);
	void	(*validate_mode_submode) (ai_info *aip);
	void	(*ai_frame) (int objnum);
	void	(*ai_process) (object *obj, int ai_index, float frametime);
	void	(*init_ai_object) (int objnum);
	void	(*init_ai_objects) ();
	void	(*init_ai_system) ();
	void	(*ai_set_default_behavior) (object *obj, int classnum);
	void	(*ai_do_default_behavior) (object *obj);
	void	(*process_friendly_hit_message) (int message, object *objp);
	void	(*maybe_process_friendly_hit) (object *objp_hitter, object *objp_hit, object *objp_weapon);
	void	(*maybe_set_dynamic_chase) (ai_info *aip, int hitter_objnum);
	void	(*big_ship_collide_recover_start) (object *objp, object *big_objp, vec3d *collide_pos, vec3d *collision_normal);
	void	(*ai_update_lethality) (object *ship_obj, object *other_obj, float damage);
	void	(*ai_ship_hit) (object *objp_ship, object *hit_objp, vec3d *hitpos, int shield_quadrant, vec3d *hit_normal);
	void	(*ai_ship_destroy) (int shipnum, int method);
	void	(*ai_deathroll_start) (object *dying_objp);
	int	(*ai_abort_rearm_request) (object *requester_objp);
	void	(*ai_add_rearm_goal) (object *requester_objp, object *support_objp);
	int	(*ai_issue_rearm_request) (object *requester_objp);
	void	(*ai_rearm_repair) (object *objp, int docker_index, object *goal_objp, int dockee_index, int priority);
	void	(*cheat_fire_synaptic) (object *objp, ship *shipp, ai_info *aip);
	void	(*maybe_cheat_fire_synaptic) (object *objp, ai_info *aip);
};


// Set new pointer to override ai_code* function calls
// possibly copying the old function calls first

extern struct aicode_call_table *aicode_table;

#endif
