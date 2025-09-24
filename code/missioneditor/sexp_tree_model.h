/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#pragma once

// Shared sexp tree model — UI-independent data structures and logic.
// Used by both FRED2 (MFC) and QtFRED (Qt) sexp tree implementations.

#include "globalincs/globals.h"
#include "globalincs/flagset.h"
#include "globalincs/vmallocator.h"  // SCP_string, SCP_vector

// -----------------------------------------------------------------------
// SEXPT_* node type/status constants
// -----------------------------------------------------------------------

#define SEXPT_UNUSED          0x0000
#define SEXPT_UNINIT          0x0001
#define SEXPT_UNKNOWN         0x0002

#define SEXPT_VALID           0x1000
#define SEXPT_TYPE_MASK       0x07ff
#define SEXPT_TYPE(X)         (SEXPT_TYPE_MASK & (X))

#define SEXPT_OPERATOR        0x0010
#define SEXPT_NUMBER          0x0020
#define SEXPT_STRING          0x0040
#define SEXPT_VARIABLE        0x0080
#define SEXPT_CONTAINER_NAME  0x0100
#define SEXPT_CONTAINER_DATA  0x0200
#define SEXPT_MODIFIER        0x0400

// -----------------------------------------------------------------------
// Node flags (editability)
// -----------------------------------------------------------------------

#define NOT_EDITABLE  0x00
#define OPERAND       0x01
#define EDITABLE      0x02
#define COMBINED      0x04

// -----------------------------------------------------------------------
// NodeImage — unified icon enum for tree nodes
// -----------------------------------------------------------------------
// Replaces FRED2's BITMAP_* #defines and QtFRED's NodeImage enum class.
// Numeric values match the original BITMAP_* constants for compatibility.

enum class NodeImage : int {
	OPERATOR = 0,
	DATA = 1,
	VARIABLE = 2,
	ROOT = 3,
	ROOT_DIRECTIVE = 4,
	CHAIN = 5,
	CHAIN_DIRECTIVE = 6,
	GREEN_DOT = 7,
	BLACK_DOT = 8,
	// Numbered data icons (DATA_00 through DATA_95, stepping by 5)
	DATA_00 = 9,
	DATA_05 = 10,
	DATA_10 = 11,
	DATA_15 = 12,
	DATA_20 = 13,
	DATA_25 = 14,
	DATA_30 = 15,
	DATA_35 = 16,
	DATA_40 = 17,
	DATA_45 = 18,
	DATA_50 = 19,
	DATA_55 = 20,
	DATA_60 = 21,
	DATA_65 = 22,
	DATA_70 = 23,
	DATA_75 = 24,
	DATA_80 = 25,
	DATA_85 = 26,
	DATA_90 = 27,
	DATA_95 = 28,
	COMMENT = 29,
	CONTAINER_NAME = 30,
	CONTAINER_DATA = 31,
};

// -----------------------------------------------------------------------
// Tree behavior mode flags and modes
// -----------------------------------------------------------------------

FLAG_LIST(TreeFlags) {
	LabeledRoot = 0,
	RootDeletable,
	RootEditable,
	AnnotationsAllowed,

	NUM_VALUES
};

// Legacy ST_* / MODE_* constants (FRED2 compatibility)
#define ST_LABELED_ROOT   0x10000
#define ST_ROOT_DELETABLE 0x20000
#define ST_ROOT_EDITABLE  0x40000

#define MODE_GOALS     (1 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)
#define MODE_EVENTS    (2 | ST_LABELED_ROOT | ST_ROOT_DELETABLE | ST_ROOT_EDITABLE)
#define MODE_CAMPAIGN  (3 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)
#define MODE_CUTSCENES (4 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)

// -----------------------------------------------------------------------
// sexp_tree_item — a single node in the sexp tree
// -----------------------------------------------------------------------
// The handle member is a void* that each UI layer casts to its native type:
//   FRED2:  static_cast<HTREEITEM>(handle)
//   QtFRED: static_cast<QTreeWidgetItem*>(handle)

class sexp_tree_item {
public:
	sexp_tree_item() : type(SEXPT_UNUSED), parent(-1), child(-1), next(-1), flags(0), handle(nullptr) {
		text[0] = '\0';
	}

	int type;
	int parent;    // index of parent node (-1 if none)
	int child;     // index of first child node (-1 if none)
	int next;      // index of next sibling (-1 if none)
	int flags;
	char text[2 * TOKEN_LENGTH + 2];
	void* handle;  // opaque UI handle — never dereferenced by model code
};

// -----------------------------------------------------------------------
// sexp_list_item — linked list node for building option listings
// -----------------------------------------------------------------------

class sexp_list_item {
public:
	int type;
	int op;
	SCP_string text;
	sexp_list_item* next;

	sexp_list_item() : type(0), op(-1), next(nullptr) {}

	void set_op(int op_num);
	void set_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_op(int op_num);
	void add_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_list(sexp_list_item* list);
	void shallow_copy(const sexp_list_item* src);
	void destroy();
};

// -----------------------------------------------------------------------
// SexpTreeEditorInterface — context interface implemented by editor dialogs
// -----------------------------------------------------------------------
// Both FRED2 and QtFRED dialogs can implement this to provide context-dependent
// data (messages, goals, events, mission names) to the shared tree model.
// Uses SCP types — UI layers translate at the boundary as needed.

class SexpTreeEditorInterface {
	flagset<TreeFlags> _flags;

public:
	SexpTreeEditorInterface();
	explicit SexpTreeEditorInterface(const flagset<TreeFlags>& flags);
	virtual ~SexpTreeEditorInterface();

	virtual bool hasDefaultMessageParamter();
	virtual SCP_vector<SCP_string> getMessages();

	virtual SCP_vector<SCP_string> getMissionGoals(const SCP_string& reference_name);
	virtual bool hasDefaultGoal(int operator_value);

	virtual SCP_vector<SCP_string> getMissionEvents(const SCP_string& reference_name);
	virtual bool hasDefaultEvent(int operator_value);

	virtual SCP_vector<SCP_string> getMissionNames();
	virtual bool hasDefaultMissionName();

	virtual int getRootReturnType() const;

	const flagset<TreeFlags>& getFlags() const;

	virtual bool requireCampaignOperators() const;
};

// Forward declaration for OPF function parameter
enum class ContainerType;

// -----------------------------------------------------------------------
// SexpTreeModel — shared UI-independent sexp tree model
// -----------------------------------------------------------------------
// Owns tree node data and provides all pure-logic operations.
// Both FRED2 and QtFRED sexp_tree classes delegate to this model.
// OPF listing functions are declared here and implemented in sexp_tree_opf.cpp.

class SexpTreeModel {
public:
	SexpTreeModel();
	~SexpTreeModel();

	// Tree node storage
	SCP_vector<sexp_tree_item> tree_nodes;
	int total_nodes;
	int m_mode;
	int item_index;

	// Editor context interface (set by UI layer)
	SexpTreeEditorInterface* _interface;

	// --- Tree navigation helpers (used by OPF functions) ---
	int find_argument_number(int parent_node, int child_node) const;
	int find_ancestral_argument_number(int parent_op, int child_node) const;
	bool is_node_eligible_for_special_argument(int parent_node) const;

	// --- OPF listing functions (implemented in sexp_tree_opf.cpp) ---
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
	sexp_list_item* get_container_modifiers(int con_data_node) const;
	sexp_list_item* get_list_container_modifiers() const;
	sexp_list_item* get_map_container_modifiers(int con_data_node) const;
	sexp_list_item* get_container_multidim_modifiers(int con_data_node) const;

	// Static utilities
	static bool is_container_name_opf_type(int op_type);
};
