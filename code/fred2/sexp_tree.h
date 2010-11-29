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

#include "parse/sexp.h"
#include "parse/parselo.h"

// Goober5000 - it's dynamic now
//#define MAX_SEXP_TREE_SIZE 500
//#define MAX_SEXP_TREE_SIZE 1050
//#define MAX_SEXP_TREE_SIZE ((MAX_SEXP_NODES)*2/3)

// tree_node type
#define SEXPT_UNUSED	0x0000
#define SEXPT_UNINIT	0x0001
#define SEXPT_UNKNOWN	0x0002

#define SEXPT_VALID		0x1000
#define SEXPT_TYPE_MASK	0x00ff
#define SEXPT_TYPE(X)	(SEXPT_TYPE_MASK & X)

#define SEXPT_OPERATOR	0x0010
#define SEXPT_NUMBER	0x0020
#define SEXPT_STRING	0x0040
#define SEXPT_VARIABLE	0x0080

// tree_node flag
#define NOT_EDITABLE	0x00
#define OPERAND			0x01
#define EDITABLE		0x02
#define COMBINED		0x04

// Bitmaps
#define BITMAP_OPERATOR			0
#define BITMAP_DATA				1
#define BITMAP_VARIABLE			2
#define BITMAP_ROOT				3
#define BITMAP_ROOT_DIRECTIVE	4
#define BITMAP_CHAIN			5
#define BITMAP_CHAIN_DIRECTIVE	6
#define BITMAP_GREEN_DOT		7
#define BITMAP_BLACK_DOT		8
#define BITMAP_BLUE_DOT			BITMAP_ROOT
#define BITMAP_RED_DOT			BITMAP_ROOT_DIRECTIVE
#define BITMAP_NUMBERED_DATA		9
//Therefore NEXT DEFINE should be 9+12 or 21



// tree behavior modes (or tree subtype)
#define ST_LABELED_ROOT		0x10000
#define ST_ROOT_DELETABLE	0x20000
#define ST_ROOT_EDITABLE	0x40000

#define MODE_GOALS		(1 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)
#define MODE_EVENTS		(2 | ST_LABELED_ROOT | ST_ROOT_DELETABLE | ST_ROOT_EDITABLE)
#define MODE_CAMPAIGN	(3 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)

// various tree operations notification codes (to be handled by derived class)
#define ROOT_DELETED	1
#define ROOT_RENAMED	2

#define SEXP_ITEM_F_DUP	(1<<0)

/*
 * Notes: An sexp_tree_item is basically a node in a tree.  The sexp_tree is an array of
 * these node items.
 */

class sexp_tree_item
{
public:
	sexp_tree_item() : type(SEXPT_UNUSED) {}

	int type;
	int parent;	// pointer to parent of this item
	int child;	// pointer to first child of this item
	int next;	// pointer to next sibling
	int flags;
	char text[2 * TOKEN_LENGTH + 2];
	HTREEITEM handle;
};

class sexp_list_item
{
public:
	int type;
	int op;
	char *text;
	int flags;
	sexp_list_item *next;

	sexp_list_item() : flags(0), next(NULL) {}

	void set_op(int op_num);
	void set_data(char *str, int t = (SEXPT_STRING | SEXPT_VALID));
	void set_data_dup(char *str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_op(int op_num);
	void add_data(char *str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_data_dup(char *str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_list(sexp_list_item *list);
	void destroy();
};

class sexp_tree : public CTreeCtrl
{
public:
	sexp_tree();

	int find_text(char *text, int *find);
	int query_restricted_opf_range(int opf);
	void verify_and_fix_arguments(int node);
	void post_load();
	void update_help(HTREEITEM h);
	char *help(int code);
	HTREEITEM insert(LPCTSTR lpszItem, int image = BITMAP_ROOT, int sel_image = BITMAP_ROOT, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM handle(int node);
	int get_type(HTREEITEM h);
	void setup(CEdit *ptr = NULL);
	int query_false(int node = -1);
	int add_default_operator(int op, int argnum);
	int get_default_value(sexp_list_item *item, int op, int i);
	int query_default_argument_available(int op);
	int query_default_argument_available(int op, int i);
	void swap_roots(HTREEITEM one, HTREEITEM two);
	void move_branch(int source, int parent = -1);
	HTREEITEM move_branch(HTREEITEM source, HTREEITEM parent = TVI_ROOT, HTREEITEM after = TVI_LAST);
	void copy_branch(HTREEITEM source, HTREEITEM parent = TVI_ROOT, HTREEITEM after = TVI_LAST);
	void setup_selected(HTREEITEM h = NULL);
	void add_or_replace_operator(int op, int replace_flag = 0);
	void replace_one_arg_operator(char *op, char *data, int type);
	void replace_operator(char *op);
	void replace_data(char *data, int type);
	void replace_variable_data(int var_idx, int type);
	void link_modified(int *ptr);
	void ensure_visible(int node);
	int node_error(int node, char *msg, int *bypass);
	void expand_branch(HTREEITEM h);
	void expand_operator(int node);
	void merge_operator(int node);
	int end_label_edit(TVITEMA &item);
	int edit_label(HTREEITEM h);
	int identify_arg_type(int node);
	int count_args(int node);
	void right_clicked(int mode = 0);
	int ctree_size;
	virtual void build_tree();
	void set_node(int index, int type, char *text);
	void free_node(int node, int cascade = 0);
	int allocate_node(int parent, int after = -1);
	int allocate_node();
	int find_free_node();
	void clear_tree(char *op = NULL);
	void reset_handles();
	int save_tree(int node = -1);
	void load_tree(int index, char *deflt = "true");
	void add_one_arg_operator(char *op, char *data, int type);
	void add_operator(char *op, HTREEITEM h = TVI_ROOT);
	int add_data(char *data, int type);
	int add_variable_data(char *data, int type);
	void add_sub_tree(int node, HTREEITEM root);
	int load_sub_tree(int index, bool valid, char *text);
	void hilite_item(int node);
	char *match_closest_operator(char *str, int node);
	void delete_sexp_tree_variable(const char *var_name);
	void modify_sexp_tree_variable(const char *old_name, int sexp_var_index);
	int get_item_index_to_var_index();
	int get_tree_name_to_sexp_variable_index(const char *tree_name);
	int get_modify_variable_type(int parent);
	int get_variable_count(const char *var_name);
	int get_loadout_variable_count(int var_index);

	// Goober5000
	int find_argument_number(int parent_node, int child_node);
	int find_ancestral_argument_number(int parent_op, int child_node);
	int query_node_argument_type(int node);

	//WMC
	int get_sibling_place(int node);
	int get_data_image(int node);


	sexp_list_item *get_listing_opf(int opf, int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_null();
	sexp_list_item *get_listing_opf_bool(int parent_node = -1);
	sexp_list_item *get_listing_opf_positive();
	sexp_list_item *get_listing_opf_number();
	sexp_list_item *get_listing_opf_ship(int parent_node = -1);
	sexp_list_item *get_listing_opf_wing();
	sexp_list_item *get_listing_opf_subsystem(int parent_node, int arg_index);
	sexp_list_item *get_listing_opf_subsystem_type(int parent_node);
	sexp_list_item *get_listing_opf_point();
	sexp_list_item *get_listing_opf_iff();
	sexp_list_item *get_listing_opf_ai_goal(int parent_node);
	sexp_list_item *get_listing_opf_docker_point(int parent_node);
	sexp_list_item *get_listing_opf_dockee_point(int parent_node);
	sexp_list_item *get_listing_opf_message();
	sexp_list_item *get_listing_opf_who_from();
	sexp_list_item *get_listing_opf_priority();
	sexp_list_item *get_listing_opf_waypoint_path();
	sexp_list_item *get_listing_opf_ship_point();
	sexp_list_item *get_listing_opf_ship_wing_point();
	sexp_list_item *get_listing_opf_ship_wing_team();
	sexp_list_item *get_listing_opf_ship_wing_point_or_none();
	sexp_list_item *get_listing_opf_mission_name();
	sexp_list_item *get_listing_opf_goal_name(int parent_node);
	sexp_list_item *get_listing_opf_ship_wing();
	sexp_list_item *get_listing_opf_order_recipient();
	sexp_list_item *get_listing_opf_ship_type();
	sexp_list_item *get_listing_opf_keypress();
	sexp_list_item *get_listing_opf_event_name(int parent_node);
	sexp_list_item *get_listing_opf_ai_order();
	sexp_list_item *get_listing_opf_skill_level();
	sexp_list_item *get_listing_opf_medal_name();
	sexp_list_item *get_listing_opf_weapon_name();
	sexp_list_item *get_listing_opf_ship_class_name();
	sexp_list_item *get_listing_opf_hud_gauge_name();
	sexp_list_item *get_listing_opf_huge_weapon();
	sexp_list_item *get_listing_opf_ship_not_player();
	sexp_list_item *get_listing_opf_jump_nodes();
	sexp_list_item *get_listing_opf_variable_names();
	sexp_list_item *get_listing_opf_variable_type();
	sexp_list_item *get_listing_opf_skybox_model();
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
	sexp_list_item *get_listing_opf_armor_types();
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
	sexp_list_item *get_listing_hud_gauge();

	int m_mode;
	int item_index;
	int select_sexp_node;  // used to select an sexp item on dialog box open.
	BOOL		m_dragging;
	HTREEITEM	m_h_drag;
	HTREEITEM	m_h_drop;
	CImageList	*m_p_image_list;
	CEdit *help_box;
	CEdit *mini_help_box;
	CPoint m_pt;

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
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	int load_branch(int index, int parent);
	int save_branch(int cur, int at_root = 0);
	void free_node2(int node);

	int flag;
	int *modified;

	SCP_vector<sexp_tree_item> tree_nodes;
	int total_nodes;

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
