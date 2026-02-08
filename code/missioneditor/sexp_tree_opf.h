/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#pragma once

/**
 * OPF (Operator Parameter Format) listing functions for the sexp tree.
 *
 * SexpTreeOPF is responsible for building the linked lists of valid values
 * for each OPF_* argument type (ships, wings, waypoints, variables, etc.).
 * These lists are used by context menus, the operator quick-search popup,
 * and validation logic to determine what options are available at each
 * argument position in the sexp tree.
 *
 * Holds a back-reference to SexpTreeModel for read-only access to tree_nodes[],
 * _interface, and helper methods like is_node_eligible_for_special_argument().
 *
 * All get_listing_opf_*() functions return a sexp_list_item* linked list.
 * Caller must call destroy() on the returned list when done.
 */

// Forward declarations — full definitions come from sexp_tree_model.h
struct sexp_list_item;
struct sexp_tree_item;
class SexpTreeModel;
enum class ContainerType;

class SexpTreeOPF {
public:
	explicit SexpTreeOPF(SexpTreeModel& model);

	// Master dispatcher — routes to the appropriate get_listing_opf_* based on opf type
	sexp_list_item* get_listing_opf(int opf, int parent_node, int arg_index);

	sexp_list_item* get_listing_opf_null();
	sexp_list_item* get_listing_opf_flexible_argument();
	sexp_list_item* get_listing_opf_bool(int parent_node = -1);
	sexp_list_item* get_listing_opf_positive();
	sexp_list_item* get_listing_opf_number();
	sexp_list_item* get_listing_opf_ship(int parent_node = -1);
	sexp_list_item* get_listing_opf_prop();
	sexp_list_item* get_listing_opf_wing();
	sexp_list_item* get_listing_opf_subsystem(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_subsystem_type(int parent_node);
	sexp_list_item* get_listing_opf_point();
	sexp_list_item* get_listing_opf_iff();
	sexp_list_item* get_listing_opf_ai_class();
	sexp_list_item* get_listing_opf_support_ship_class();
	sexp_list_item* get_listing_opf_ssm_class();
	sexp_list_item* get_listing_opf_ship_with_bay();
	sexp_list_item* get_listing_opf_soundtrack_name();
	sexp_list_item* get_listing_opf_arrival_location();
	sexp_list_item* get_listing_opf_departure_location();
	sexp_list_item* get_listing_opf_arrival_anchor_all();
	sexp_list_item* get_listing_opf_ai_goal(int parent_node);
	sexp_list_item* get_listing_opf_docker_point(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_dockee_point(int parent_node);
	sexp_list_item* get_listing_opf_message();
	sexp_list_item* get_listing_opf_persona();
	sexp_list_item* get_listing_opf_font();
	sexp_list_item* get_listing_opf_who_from();
	sexp_list_item* get_listing_opf_priority();
	sexp_list_item* get_listing_opf_sound_environment();
	sexp_list_item* get_listing_opf_sound_environment_option();
	sexp_list_item* get_listing_opf_adjust_audio_volume();
	sexp_list_item* get_listing_opf_builtin_hud_gauge();
	sexp_list_item* get_listing_opf_custom_hud_gauge();
	sexp_list_item* get_listing_opf_any_hud_gauge();
	sexp_list_item* get_listing_opf_ship_effect();
	sexp_list_item* get_listing_opf_explosion_option();
	sexp_list_item* get_listing_opf_waypoint_path();
	sexp_list_item* get_listing_opf_ship_point();
	sexp_list_item* get_listing_opf_ship_wing_wholeteam();
	sexp_list_item* get_listing_opf_ship_wing_shiponteam_point();
	sexp_list_item* get_listing_opf_ship_wing_point();
	sexp_list_item* get_listing_opf_ship_wing_point_or_none();
	sexp_list_item* get_listing_opf_mission_name();
	sexp_list_item* get_listing_opf_goal_name(int parent_node);
	sexp_list_item* get_listing_opf_ship_wing();
	sexp_list_item* get_listing_opf_ship_prop();
	sexp_list_item* get_listing_opf_order_recipient();
	sexp_list_item* get_listing_opf_ship_type();
	sexp_list_item* get_listing_opf_keypress();
	sexp_list_item* get_listing_opf_event_name(int parent_node);
	sexp_list_item* get_listing_opf_ai_order();
	sexp_list_item* get_listing_opf_skill_level();
	sexp_list_item* get_listing_opf_cargo();
	sexp_list_item* get_listing_opf_string();
	sexp_list_item* get_listing_opf_medal_name();
	sexp_list_item* get_listing_opf_weapon_name();
	sexp_list_item* get_listing_opf_intel_name();
	sexp_list_item* get_listing_opf_ship_class_name();
	sexp_list_item* get_listing_opf_prop_class_name();
	sexp_list_item* get_listing_opf_huge_weapon();
	sexp_list_item* get_listing_opf_ship_not_player();
	sexp_list_item* get_listing_opf_ship_or_none();
	sexp_list_item* get_listing_opf_subsystem_or_none(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_subsys_or_generic(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_jump_nodes();
	sexp_list_item* get_listing_opf_variable_names();
	sexp_list_item* get_listing_opf_skybox_model();
	sexp_list_item* get_listing_opf_skybox_flags();
	sexp_list_item* get_listing_opf_background_bitmap();
	sexp_list_item* get_listing_opf_sun_bitmap();
	sexp_list_item* get_listing_opf_nebula_storm_type();
	sexp_list_item* get_listing_opf_nebula_poof();
	sexp_list_item* get_listing_opf_turret_target_order();
	sexp_list_item* get_listing_opf_turret_types();
	sexp_list_item* get_listing_opf_post_effect();
	sexp_list_item* get_listing_opf_turret_target_priorities();
	sexp_list_item* get_listing_opf_armor_type();
	sexp_list_item* get_listing_opf_damage_type();
	sexp_list_item* get_listing_opf_animation_type();
	sexp_list_item* get_listing_opf_hud_elements();
	sexp_list_item* get_listing_opf_weapon_banks();
	sexp_list_item* get_listing_opf_mission_moods();
	sexp_list_item* get_listing_opf_ship_flags();
	sexp_list_item* get_listing_opf_wing_flags();
	sexp_list_item* get_listing_opf_team_colors();
	sexp_list_item* get_listing_opf_nebula_patterns();
	sexp_list_item* get_listing_opf_asteroid_types();
	sexp_list_item* get_listing_opf_debris_types();
	sexp_list_item* get_listing_opf_motion_debris();
	sexp_list_item* get_listing_opf_game_snds();
	sexp_list_item* get_listing_opf_fireball();
	sexp_list_item* get_listing_opf_species();
	sexp_list_item* get_listing_opf_language();
	sexp_list_item* get_listing_opf_functional_when_eval_type();
	sexp_list_item* get_listing_opf_animation_name(int parent_node);
	sexp_list_item* get_listing_opf_sexp_containers(ContainerType con_type);
	sexp_list_item* get_listing_opf_wing_formation();
	sexp_list_item* get_listing_opf_bolt_types();
	sexp_list_item* get_listing_opf_traitor_overrides();
	sexp_list_item* get_listing_opf_lua_general_orders();
	sexp_list_item* get_listing_opf_message_types();
	sexp_list_item* get_listing_opf_lua_enum(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_mission_custom_strings();
	sexp_list_item* check_for_dynamic_sexp_enum(int opf);

	// Container modifier helpers

	//! Returns the list of valid modifiers for a container data node (dispatches to list/map)
	sexp_list_item* get_container_modifiers(int con_data_node) const;
	//! Returns the list of valid modifiers for list containers (At, Get, etc.)
	sexp_list_item* get_list_container_modifiers() const;
	//! Returns the list of valid modifiers for map containers, based on key type
	sexp_list_item* get_map_container_modifiers(int con_data_node) const;
	//! Returns the list of valid modifiers for multidimensional container access
	sexp_list_item* get_container_multidim_modifiers(int con_data_node) const;

	//! Returns true if the given OPF type is one of the container-name types
	static bool is_container_name_opf_type(int op_type);

private:
	SexpTreeModel& _model;
};
