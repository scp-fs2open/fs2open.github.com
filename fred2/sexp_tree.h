/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _SEXP_TREE_H
#define _SEXP_TREE_H

// 4786 is identifier truncated to 255 characters (happens all the time in Microsoft #includes) -- Goober5000
#pragma warning(disable: 4786)

#include "OperatorComboBox.h"
#include "missioneditor/sexp_tree_model.h"
#include "missioneditor/sexp_tree_actions.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "parse/parselo.h"

// Goober5000 - it's dynamic now
//#define MAX_SEXP_TREE_SIZE 500
//#define MAX_SEXP_TREE_SIZE 1050
//#define MAX_SEXP_TREE_SIZE ((MAX_SEXP_NODES)*2/3)

// FRED2 BITMAP_* compatibility aliases (map to shared NodeImage enum values)
#define BITMAP_OPERATOR         static_cast<int>(NodeImage::OPERATOR)
#define BITMAP_DATA             static_cast<int>(NodeImage::DATA)
#define BITMAP_VARIABLE         static_cast<int>(NodeImage::VARIABLE)
#define BITMAP_ROOT             static_cast<int>(NodeImage::ROOT)
#define BITMAP_ROOT_DIRECTIVE   static_cast<int>(NodeImage::ROOT_DIRECTIVE)
#define BITMAP_CHAIN            static_cast<int>(NodeImage::CHAIN)
#define BITMAP_CHAIN_DIRECTIVE  static_cast<int>(NodeImage::CHAIN_DIRECTIVE)
#define BITMAP_GREEN_DOT        static_cast<int>(NodeImage::GREEN_DOT)
#define BITMAP_BLACK_DOT        static_cast<int>(NodeImage::BLACK_DOT)
#define BITMAP_BLUE_DOT         BITMAP_ROOT
#define BITMAP_RED_DOT          BITMAP_ROOT_DIRECTIVE
#define BITMAP_NUMBERED_DATA    static_cast<int>(NodeImage::DATA_00)
// There are 20 number bitmaps, 9 to 28, counting by 5s from 0 to 95
#define BITMAP_COMMENT          static_cast<int>(NodeImage::COMMENT)
#define BITMAP_CONTAINER_NAME   static_cast<int>(NodeImage::CONTAINER_NAME)
#define BITMAP_CONTAINER_DATA   static_cast<int>(NodeImage::CONTAINER_DATA)

// various tree operations notification codes (to be handled by derived class)
#define ROOT_DELETED	1
#define ROOT_RENAMED	2

// Typed handle accessors for FRED2 (MFC HTREEITEM)
inline HTREEITEM tree_item_handle(const sexp_tree_item& item) {
	return static_cast<HTREEITEM>(item.handle);
}

class sexp_tree : public CTreeCtrl, public ISexpTreeUI
{
public:
	// Shared model and action layer (must be declared before reference aliases below)
	SexpTreeModel _model;
	SexpTreeActions _actions;

	sexp_tree();

	int find_text(const char *text, int *find);
	int query_restricted_opf_range(int opf);
	void verify_and_fix_arguments(int node);
	void post_load();
	void update_help(HTREEITEM h);
	static const char *help(int code);
	HTREEITEM insert(LPCTSTR lpszItem, int image = BITMAP_ROOT, int sel_image = BITMAP_ROOT, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM handle(int node);
	int get_type(HTREEITEM h);
	void setup(CEdit *ptr = NULL);
	int query_false(int node = -1);
	int add_default_operator(int op, int argnum);
	int get_default_value(sexp_list_item *item, char *text_buf, int op, int i);
	int query_default_argument_available(int op);
	int query_default_argument_available(int op, int i);
	void move_root(HTREEITEM source, HTREEITEM dest, bool insert_before);
	void move_branch(int source, int parent = -1);
	HTREEITEM move_branch(HTREEITEM source, HTREEITEM parent = TVI_ROOT, HTREEITEM after = TVI_LAST);
	void copy_branch(HTREEITEM source, HTREEITEM parent = TVI_ROOT, HTREEITEM after = TVI_LAST);
	void setup_selected(HTREEITEM h = NULL);
	void add_or_replace_operator(int op, int replace_flag = 0);
//	void replace_one_arg_operator(const char *op, const char *data, int type);
	void replace_operator(const char *op);
	void replace_data(const char *data, int type);
	void replace_variable_data(int var_idx, int type);
	void replace_container_name(const sexp_container &container);
	void replace_container_data(const sexp_container &container,
		int type,
		bool test_child_nodes,
		bool delete_child_nodes,
		bool set_default_modifier);
	void add_default_modifier(const sexp_container &container);
	void link_modified(int *ptr);
	void ensure_visible(int node);
	int node_error(int node, const char *msg, int *bypass);
	void expand_branch(HTREEITEM h);
	void expand_operator(int node);
	void merge_operator(int node);
	int end_label_edit(TVITEMA &item);
	int edit_label(HTREEITEM h, bool *is_operator = nullptr);
	virtual void edit_comment(HTREEITEM h);
	virtual void edit_bg_color(HTREEITEM h);
	int identify_arg_type(int node);
	int count_args(int node);
	void right_clicked(int mode = 0);
	int ctree_size;
	virtual void build_tree();
	void set_node(int index, int type, const char *text);
	void free_node(int node, int cascade = 0);
	int allocate_node(int parent, int after = -1);
	int allocate_node();
	int find_free_node();
	void clear_tree(const char *op = NULL);
	void reset_handles();
	int save_tree(int node = -1);
	void load_tree(int index, const char *deflt = "true");
	void add_operator(const char *op, HTREEITEM h = TVI_ROOT);
	int add_data(const char *data, int type);
	int add_variable_data(const char *data, int type);
	int add_container_name(const char *container_name);
	void add_container_data(const char *container_name);
	void add_sub_tree(int node, HTREEITEM root);
	int load_sub_tree(int index, bool valid, const char *text);
	void hilite_item(int node);
	const SCP_string &match_closest_operator(const SCP_string &str, int node);
	void delete_sexp_tree_variable(const char *var_name);
	void modify_sexp_tree_variable(const char *old_name, int sexp_var_index);
	int get_item_index_to_var_index();
	int get_tree_name_to_sexp_variable_index(const char *tree_name);
	int get_modify_variable_type(int parent);
	int get_variable_count(const char *var_name);
	int get_loadout_variable_count(int var_index);

	// Karajorma/jg18
	int get_container_usage_count(const SCP_string &container_name) const;
	bool rename_container_nodes(const SCP_string &old_name, const SCP_string &new_name);
	bool is_matching_container_node(int node, const SCP_string &container_name) const;
	bool is_container_name_argument(int node) const;
	static bool is_container_name_opf_type(int op_type);

	// Goober5000
	int find_argument_number(int parent_node, int child_node) const;
	int find_ancestral_argument_number(int parent_op, int child_node) const;
	int query_node_argument_type(int node) const;

	//WMC
	int get_sibling_place(int node);
	int get_data_image(int node);


	sexp_list_item *get_listing_opf(int opf, int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_null();
	sexp_list_item *get_listing_opf_bool(int parent_node = -1);
	sexp_list_item *get_listing_opf_positive();
	sexp_list_item *get_listing_opf_number();
	sexp_list_item *get_listing_opf_ship(int parent_node = -1);
	sexp_list_item *get_listing_opf_prop();
	sexp_list_item *get_listing_opf_wing();
	sexp_list_item *get_listing_opf_subsystem(int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_subsystem_type(int parent_node);
	sexp_list_item *get_listing_opf_point();
	sexp_list_item *get_listing_opf_iff();
	sexp_list_item *get_listing_opf_ai_goal(int parent_node);
	sexp_list_item *get_listing_opf_docker_point(int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_dockee_point(int parent_node);
	sexp_list_item *get_listing_opf_message();
	sexp_list_item *get_listing_opf_who_from();
	sexp_list_item *get_listing_opf_priority();
	sexp_list_item *get_listing_opf_waypoint_path();
	sexp_list_item *get_listing_opf_ship_point();
	sexp_list_item *get_listing_opf_ship_wing();
	sexp_list_item *get_listing_opf_ship_prop();
	sexp_list_item *get_listing_opf_ship_wing_wholeteam();
	sexp_list_item *get_listing_opf_ship_wing_shiponteam_point();
	sexp_list_item *get_listing_opf_ship_wing_point();
	sexp_list_item *get_listing_opf_ship_wing_point_or_none();
	sexp_list_item *get_listing_opf_mission_name();
	sexp_list_item *get_listing_opf_goal_name(int parent_node);
	sexp_list_item *get_listing_opf_order_recipient();
	sexp_list_item *get_listing_opf_ship_type();
	sexp_list_item *get_listing_opf_keypress();
	sexp_list_item *get_listing_opf_event_name(int parent_node);
	sexp_list_item *get_listing_opf_ai_order();
	sexp_list_item *get_listing_opf_skill_level();
	sexp_list_item *get_listing_opf_medal_name();
	sexp_list_item *get_listing_opf_weapon_name();
	sexp_list_item *get_listing_opf_ship_class_name();
	sexp_list_item *get_listing_opf_prop_class_name();
	sexp_list_item *get_listing_opf_huge_weapon();
	sexp_list_item *get_listing_opf_ship_not_player();
	sexp_list_item *get_listing_opf_jump_nodes();
	sexp_list_item *get_listing_opf_variable_names();
	sexp_list_item *get_listing_opf_skybox_model();
	sexp_list_item *get_listing_opf_skybox_flags();
	sexp_list_item *get_listing_opf_background_bitmap();
	sexp_list_item *get_listing_opf_sun_bitmap();
	sexp_list_item *get_listing_opf_nebula_storm_type();
	sexp_list_item *get_listing_opf_nebula_poof();
	sexp_list_item *get_listing_opf_cargo();
	sexp_list_item *get_listing_opf_ai_class();
	sexp_list_item *get_listing_opf_support_ship_class();
	sexp_list_item *get_listing_opf_arrival_location();
	sexp_list_item *get_listing_opf_arrival_anchor_all();
	sexp_list_item *get_listing_opf_departure_location();
	sexp_list_item *get_listing_opf_ship_with_bay();
	sexp_list_item *get_listing_opf_soundtrack_name();
	sexp_list_item *get_listing_opf_intel_name();
	sexp_list_item *get_listing_opf_string();
	sexp_list_item *get_listing_opf_ssm_class();
	sexp_list_item *get_listing_opf_flexible_argument();
	sexp_list_item *get_listing_opf_ship_or_none();
	sexp_list_item *get_listing_opf_subsystem_or_none(int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_subsys_or_generic(int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_turret_target_order();
	sexp_list_item* get_listing_opf_turret_types();
	sexp_list_item *get_listing_opf_armor_type();
	sexp_list_item *get_listing_opf_damage_type();
	sexp_list_item *get_listing_opf_turret_target_priorities();
	sexp_list_item *get_listing_opf_persona();
	sexp_list_item *get_listing_opf_font();
	sexp_list_item *get_listing_opf_post_effect();
	sexp_list_item *get_listing_opf_hud_elements();
	sexp_list_item *get_listing_opf_sound_environment();
	sexp_list_item *get_listing_opf_sound_environment_option();
	sexp_list_item *get_listing_opf_explosion_option();
	sexp_list_item *get_listing_opf_adjust_audio_volume();
	sexp_list_item *get_listing_opf_weapon_banks();
	sexp_list_item *get_listing_opf_builtin_hud_gauge();
	sexp_list_item *get_listing_opf_custom_hud_gauge();
	sexp_list_item *get_listing_opf_any_hud_gauge();
	sexp_list_item *get_listing_opf_ship_effect();
	sexp_list_item *get_listing_opf_animation_type();
	sexp_list_item *get_listing_opf_mission_moods();
	sexp_list_item *get_listing_opf_ship_flags();
	sexp_list_item *get_listing_opf_wing_flags();
	sexp_list_item *get_listing_opf_team_colors();
	sexp_list_item *get_listing_opf_nebula_patterns();
	sexp_list_item *get_listing_opf_asteroid_types();
	sexp_list_item *get_listing_opf_debris_types();
	sexp_list_item *get_listing_opf_motion_debris();
	sexp_list_item *get_listing_opf_game_snds();
	sexp_list_item *get_listing_opf_fireball();
	sexp_list_item *get_listing_opf_species();
	sexp_list_item *get_listing_opf_language();
	sexp_list_item *get_listing_opf_functional_when_eval_type();
	sexp_list_item *get_listing_opf_animation_name(int parent_node);
	sexp_list_item *get_listing_opf_sexp_containers(ContainerType con_type);
	sexp_list_item *get_listing_opf_wing_formation();
	sexp_list_item *check_for_dynamic_sexp_enum(int opf);
	sexp_list_item *get_listing_opf_bolt_types();
	sexp_list_item *get_listing_opf_traitor_overrides();
	sexp_list_item *get_listing_opf_message_types();
	sexp_list_item *get_listing_opf_lua_general_orders();
	sexp_list_item *get_listing_opf_lua_enum(int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_mission_custom_strings();

	// container modifier options for container data nodes
	sexp_list_item *get_container_modifiers(int con_data_node) const;
	sexp_list_item *get_list_container_modifiers() const;
	sexp_list_item *get_map_container_modifiers(int con_data_node) const;
	sexp_list_item *get_container_multidim_modifiers(int con_data_node) const;

	bool is_node_eligible_for_special_argument(int parent_node) const;

	int& m_mode = _model.m_mode;
	int& item_index = _model.item_index;
	int select_sexp_node;  // used to select an sexp item on dialog box open.
	BOOL		m_dragging;
	HTREEITEM	m_h_drag;
	HTREEITEM	m_h_drop;
	CImageList	*m_p_image_list;
	CEdit *help_box;
	CEdit *mini_help_box;
	CPoint m_pt;
	OperatorComboBox m_operator_box;

	void start_operator_edit(HTREEITEM h);
	void end_operator_edit(bool confirm);

	// ISexpTreeUI implementation
	void* ui_insert_item(const char* text, NodeImage image, void* parent_handle, void* insert_after) override;
	void ui_delete_item(void* handle) override;
	void ui_set_item_text(void* handle, const char* text) override;
	void ui_set_item_image(void* handle, NodeImage image) override;
	void* ui_get_child_item(void* handle) override;
	bool ui_has_children(void* handle) override;
	void ui_expand_item(void* handle) override;
	void ui_select_item(void* handle) override;
	void ui_ensure_visible(void* handle) override;
	void ui_notify_modified() override;
	void ui_update_help(void* handle) override;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(sexp_tree)
	public:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(sexp_tree)
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	virtual void NodeCut();
	virtual void NodeDelete();
	virtual void NodeCopy();
	virtual void NodeReplacePaste();
	virtual void NodeAddPaste();

	void update_item(HTREEITEM handle);

	int load_branch(int index, int parent);
	int save_branch(int cur, int at_root = 0);
	void free_node2(int node);

	int flag;
	int*& modified = _model.modified;
	bool m_operator_popup_active;
	bool m_operator_popup_created;
	int m_font_height;
	int m_font_max_width;

	SCP_vector<sexp_tree_item>& tree_nodes = _model.tree_nodes;
	int& total_nodes = _model.total_nodes;

	HTREEITEM item_handle;
	int root_item;
	// these 2 variables are used to help location data sources.  Sometimes looking up
	// valid data can require complex code just to get to an index that is required to
	// locate data.  These are set up in right_clicked() to try and short circuit having
	// to do the lookup again in the code that actually does the adding or replacing of
	// the data if it's selected.
	int add_instance;  // a source reference index indicator for adding data
	int replace_instance;  // a source reference index indicator for replacing data

	DECLARE_MESSAGE_MAP()
};

#endif
