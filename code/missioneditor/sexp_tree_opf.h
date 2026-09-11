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

	// Non-copyable and non-movable: permanently bound to its owning model
	SexpTreeOPF(const SexpTreeOPF&) = delete;
	SexpTreeOPF& operator=(const SexpTreeOPF&) = delete;
	SexpTreeOPF(SexpTreeOPF&&) = delete;
	SexpTreeOPF& operator=(SexpTreeOPF&&) = delete;

	// Master dispatcher — routes to the appropriate get_listing_opf_* based on opf type
	sexp_list_item* get_listing_opf(int opf, int parent_node, int arg_index) const;

	static sexp_list_item* get_listing_opf_null();
	static sexp_list_item* get_listing_opf_flexible_argument();
	sexp_list_item* get_listing_opf_bool(int parent_node = -1) const;
	static sexp_list_item* get_listing_opf_positive();
	static sexp_list_item* get_listing_opf_number();
	sexp_list_item* get_listing_opf_ship(int parent_node = -1) const;
	static sexp_list_item* get_listing_opf_prop();
	static sexp_list_item* get_listing_opf_wing();
	sexp_list_item* get_listing_opf_subsystem(int parent_node, int arg_index) const;
	sexp_list_item* get_listing_opf_subsystem_type(int parent_node) const;
	static sexp_list_item* get_listing_opf_point();
	static sexp_list_item* get_listing_opf_iff();
	static sexp_list_item* get_listing_opf_ai_class();
	static sexp_list_item* get_listing_opf_support_ship_class();
	static sexp_list_item* get_listing_opf_ssm_class();
	static sexp_list_item* get_listing_opf_ship_with_bay();
	static sexp_list_item* get_listing_opf_soundtrack_name();
	static sexp_list_item* get_listing_opf_arrival_location();
	static sexp_list_item* get_listing_opf_departure_location();
	static sexp_list_item* get_listing_opf_arrival_anchor_all();
	sexp_list_item* get_listing_opf_ai_goal(int parent_node) const;
	sexp_list_item* get_listing_opf_docker_point(int parent_node, int arg_index) const;
	sexp_list_item* get_listing_opf_dockee_point(int parent_node) const;
	sexp_list_item* get_listing_opf_message() const;
	static sexp_list_item* get_listing_opf_persona();
	static sexp_list_item* get_listing_opf_font();
	static sexp_list_item* get_listing_opf_who_from();
	static sexp_list_item* get_listing_opf_priority();
	static sexp_list_item* get_listing_opf_sound_environment();
	static sexp_list_item* get_listing_opf_sound_environment_option();
	static sexp_list_item* get_listing_opf_adjust_audio_volume();
	static sexp_list_item* get_listing_opf_builtin_hud_gauge();
	static sexp_list_item* get_listing_opf_custom_hud_gauge();
	static sexp_list_item* get_listing_opf_any_hud_gauge();
	static sexp_list_item* get_listing_opf_ship_effect();
	static sexp_list_item* get_listing_opf_explosion_option();
	static sexp_list_item* get_listing_opf_waypoint_path();
	sexp_list_item* get_listing_opf_ship_point() const;
	sexp_list_item* get_listing_opf_ship_wing_wholeteam() const;
	sexp_list_item* get_listing_opf_ship_wing_shiponteam_point() const;
	sexp_list_item* get_listing_opf_ship_wing_point() const;
	sexp_list_item* get_listing_opf_ship_wing_point_or_none() const;
	sexp_list_item* get_listing_opf_mission_name() const;
	sexp_list_item* get_listing_opf_goal_name(int parent_node) const;
	sexp_list_item* get_listing_opf_ship_wing() const;
	sexp_list_item* get_listing_opf_ship_prop() const;
	sexp_list_item* get_listing_opf_order_recipient() const;
	static sexp_list_item* get_listing_opf_ship_type();
	static sexp_list_item* get_listing_opf_keypress();
	sexp_list_item* get_listing_opf_event_name(int parent_node) const;
	static sexp_list_item* get_listing_opf_ai_order();
	static sexp_list_item* get_listing_opf_skill_level();
	static sexp_list_item* get_listing_opf_cargo();
	static sexp_list_item* get_listing_opf_string();
	static sexp_list_item* get_listing_opf_medal_name();
	static sexp_list_item* get_listing_opf_weapon_name();
	static sexp_list_item* get_listing_opf_intel_name();
	static sexp_list_item* get_listing_opf_ship_class_name();
	static sexp_list_item* get_listing_opf_prop_class_name();
	static sexp_list_item* get_listing_opf_huge_weapon();
	static sexp_list_item* get_listing_opf_ship_not_player();
	sexp_list_item* get_listing_opf_ship_or_none() const;
	sexp_list_item* get_listing_opf_subsystem_or_none(int parent_node, int arg_index) const;
	sexp_list_item* get_listing_opf_subsys_or_generic(int parent_node, int arg_index) const;
	static sexp_list_item* get_listing_opf_jump_nodes();
	static sexp_list_item* get_listing_opf_variable_names();
	static sexp_list_item* get_listing_opf_skybox_model();
	static sexp_list_item* get_listing_opf_skybox_flags();
	static sexp_list_item* get_listing_opf_background_bitmap();
	static sexp_list_item* get_listing_opf_sun_bitmap();
	static sexp_list_item* get_listing_opf_nebula_storm_type();
	static sexp_list_item* get_listing_opf_nebula_poof();
	static sexp_list_item* get_listing_opf_turret_target_order();
	static sexp_list_item* get_listing_opf_turret_types();
	static sexp_list_item* get_listing_opf_post_effect();
	static sexp_list_item* get_listing_opf_turret_target_priorities();
	static sexp_list_item* get_listing_opf_armor_type();
	static sexp_list_item* get_listing_opf_damage_type();
	static sexp_list_item* get_listing_opf_animation_type();
	static sexp_list_item* get_listing_opf_hud_elements();
	static sexp_list_item* get_listing_opf_weapon_banks();
	static sexp_list_item* get_listing_opf_mission_moods();
	static sexp_list_item* get_listing_opf_ship_flags();
	static sexp_list_item* get_listing_opf_wing_flags();
	static sexp_list_item* get_listing_opf_team_colors();
	static sexp_list_item* get_listing_opf_nebula_patterns();
	static sexp_list_item* get_listing_opf_asteroid_types();
	static sexp_list_item* get_listing_opf_debris_types();
	static sexp_list_item* get_listing_opf_motion_debris();
	static sexp_list_item* get_listing_opf_game_snds();
	static sexp_list_item* get_listing_opf_fireball();
	static sexp_list_item* get_listing_opf_species();
	static sexp_list_item* get_listing_opf_language();
	static sexp_list_item* get_listing_opf_functional_when_eval_type();
	sexp_list_item* get_listing_opf_animation_name(int parent_node) const;
	static sexp_list_item* get_listing_opf_sexp_containers(ContainerType con_type);
	static sexp_list_item* get_listing_opf_wing_formation();
	static sexp_list_item* get_listing_opf_bolt_types();
	static sexp_list_item* get_listing_opf_traitor_overrides();
	static sexp_list_item* get_listing_opf_lua_general_orders();
	static sexp_list_item* get_listing_opf_message_types();
	sexp_list_item* get_listing_opf_lua_enum(int parent_node, int arg_index) const;
	static sexp_list_item* get_listing_opf_mission_custom_strings();
	static sexp_list_item* check_for_dynamic_sexp_enum(int opf);

	// Container modifier helpers

	//! Returns the list of valid modifiers for a container data node (dispatches to list/map)
	sexp_list_item* get_container_modifiers(int con_data_node) const;
	//! Returns the list of valid modifiers for list containers (At, Get, etc.)
	static sexp_list_item* get_list_container_modifiers();
	//! Returns the list of valid modifiers for map containers, based on key type
	sexp_list_item* get_map_container_modifiers(int con_data_node) const;
	//! Returns the list of valid modifiers for multidimensional container access
	sexp_list_item* get_container_multidim_modifiers(int con_data_node) const;

	//! Returns true if the given OPF type is one of the container-name types
	static bool is_container_name_opf_type(int op_type);

	// --- Default argument availability and values ---

	//! Returns non-zero if all minimum required arguments of operator op have defaults
	int query_default_argument_available(int op) const;
	//! Returns non-zero if argument position i of operator op has a default value available
	int query_default_argument_available(int op, int i) const;
	//! Determine and populate the default value for argument position i of operator op.
	//! Returns 0 on success, -1 if no default available.
	int get_default_value(sexp_list_item* item, int op, int i) const;

private:
	SexpTreeModel& _model;
};
