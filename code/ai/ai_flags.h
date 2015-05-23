#ifndef AI_FLAGS_H
#define AI_FLAGS_H

#include "globalincs/pstypes.h"

namespace AI {
	FLAG_LIST(AI_Flags) {
		Formation_wing,				//	Fly in formation as part of wing.
		Awaiting_repair,			//	Awaiting a repair ship.
		Being_repaired,				//	Currently docked with repair ship.
		Repairing,					//	Repairing a ship (or going to repair a ship)
		Seek_lock,					//	set if should focus on gaining aspect lock, not hitting with lasers
		Formation_object,			//	Fly in formation off a specific object.
		Temporary_ignore,			//	Means current ignore_objnum is only temporary, not an order from the player.
		Use_exit_path,				//  Used by path code, to flag path as an exit path
		Use_static_path,			//  Used by path code, use fixed path, don't try to recreate
		Target_collision,			//	Collided with aip->target_objnum last frame.  Avoid that ship for half a second or so.
		Unload_secondaries,			//	Fire secondaries as fast as possible!
		On_subsys_path,				//  Current path leads to a subsystem
		Avoid_shockwave_ship,		//	Avoid an existing shockwave from a ship.
		Avoid_shockwave_weapon,		//	Avoid an expected shockwave from a weapon.  shockwave_object field contains object index.
		Avoid_shockwave_started,	//	Already started avoiding shockwave, don't keep deciding whether to avoid.
		Attack_slowly,				//	Move slowly while attacking.
		Repair_obstructed,			//	Ship wants to be repaired, but path is obstructed.
		Kamikaze,					//	Crash into target
		No_dynamic,					//	Not allowed to get dynamic goals
		Avoiding_small_ship,		//	Avoiding a player ship.
		Avoiding_big_ship,			//	Avoiding a large ship.
		Big_ship_collide_recover_1,	//	Collided into a big ship.  Recovering by flying away.
		Big_ship_collide_recover_2,	//	Collided into a big ship.  Fly towards big ship sphere perimeter.
		Stealth_pursuit,			//  AI is trying to fight stealth ship
		Unload_primaries,			//	Fire primaries as fast as possible!
		Trying_unsuccessfully_to_warp,	// Trying to warp, but can't warp at the moment
		Free_afterburner_use,		// Use afterburners while following waypoints or flying towards objects

		NUM_VALUES
	};

	FLAG_LIST(Goal_flags) {
		Docker_index_valid,	// when set, index field for docker is valid
		Dockee_index_valid,	// when set, index field for dockee is valid
		Goal_on_hold,		// when set, this goal cannot currently be satisfied, although it could be in the future
		Subsys_needs_fixup,	// when set, the subsystem index (for a destroy subsystem goal) is invalid and must be gotten from the subsys name stored in docker.name field!!
		Goal_override,		// paired with AIG_TYPE_DYNAMIC to mean this goal overrides any other goal
		Purge,				// purge this goal next time we process
		Goals_purged,		// this goal has already caused other goals to get purged
		Depart_sound_played,// Goober5000 - replacement for AL's hack ;)
		Target_own_team,

		NUM_VALUES
	};

	FLAG_LIST(Override_flags) {
		Full,		//	Full sexp control
		Roll,		//	Sexp forced roll maneuver
		Pitch,		//	Sexp forced pitch change
		Heading,	//	Sexp forced heading change
		Full_lat,	//  full control over up/side/forward movement
		Up,			//	vertical movement
		Sideways,	//	horizontal movement
		Forward,	//	forward movement

		NUM_VALUES
	};

	FLAG_LIST(Profile_flags) {
		Smart_shield_management,
		Big_ships_can_attack_beam_turrets_on_untargeted_ships,
		Smart_primary_weapon_selection,
		Smart_secondary_weapon_selection,
		Allow_rapid_secondary_dumbfire,
		Huge_turret_weapons_ignore_bombs,
		Dont_insert_random_turret_fire_delay,
		Hack_improve_non_homing_swarm_turret_fire_accuracy,
		Shockwaves_damage_small_ship_subsystems,
		Navigation_subsys_governs_warp,
		No_min_dock_speed_cap,
		Disable_linked_fire_penalty,
		Disable_weapon_damage_scaling,
		Use_additive_weapon_velocity,
		Use_newtonian_dampening,
		Include_beams_in_stat_calcs,
		Kill_scoring_scales_with_damage,
		Assist_scoring_scales_with_damage,
		Allow_multi_event_scoring,
		Smart_afterburner_management,
		Fix_linked_primary_bug,
		Prevent_targeting_bombs_beyond_range,
		Smart_subsystem_targeting_for_turrets,
		Fix_heat_seeker_stealth_bug,
		Multi_allow_empty_primaries,
		Multi_allow_empty_secondaries,
		Allow_turrets_target_weapons_freely,
		Use_only_single_fov_for_turrets,
		Allow_vertical_dodge,
		Force_beam_turret_fov,
		Fix_ai_class_bug,
		Turrets_ignore_target_radius,
		No_special_player_avoid,
		Perform_fewer_scream_checks,
		All_ships_manage_shields,
		Advanced_turret_fov_edge_checks,
		Require_turret_to_have_target_in_fov,
		Ai_aims_from_ship_center,
		Allow_primary_link_at_start,
		Beams_damage_weapons,
		Player_weapon_scale_fix,
		No_warp_camera,
		Aspect_lock_countermeasure,
		Ai_guards_specific_ship_in_wing,
		Fix_ai_path_order_bug,
		Strict_turred_tagged_only_targeting,
		Aspect_invulnerability_fix,

		NUM_VALUES
	};
}

#endif