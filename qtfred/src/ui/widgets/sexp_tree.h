/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#pragma once

#include "parse/sexp.h"
#include "parse/parselo.h"

#include "mission/Editor.h"

#include <QTreeView>
#include <QTreeWidgetItem>

namespace fso {
namespace fred {

// Goober5000 - it's dynamic now
//#define MAX_SEXP_TREE_SIZE 500
//#define MAX_SEXP_TREE_SIZE 1050
//#define MAX_SEXP_TREE_SIZE ((MAX_SEXP_NODES)*2/3)

// tree_node type
#define SEXPT_UNUSED    0x0000
#define SEXPT_UNINIT    0x0001
#define SEXPT_UNKNOWN    0x0002

#define SEXPT_VALID        0x1000
#define SEXPT_TYPE_MASK    0x00ff
#define SEXPT_TYPE(X)    (SEXPT_TYPE_MASK & X)

#define SEXPT_OPERATOR    0x0010
#define SEXPT_NUMBER    0x0020
#define SEXPT_STRING    0x0040
#define SEXPT_VARIABLE    0x0080

// tree_node flag
#define NOT_EDITABLE    0x00
#define OPERAND            0x01
#define EDITABLE        0x02
#define COMBINED        0x04

// various tree operations notification codes (to be handled by derived class)
#define ROOT_DELETED    1
#define ROOT_RENAMED    2

#define SEXP_ITEM_F_DUP    (1<<0)

// tree behavior modes (or tree subtype)
FLAG_LIST(TreeFlags) {
	LabeledRoot = 0,
	RootDeletable,
	RootEditable,

	NUM_VALUES
};

enum class NodeImage {
	OPERATOR = 0,
	DATA,
	VARIABLE,
	ROOT,
	ROOT_DIRECTIVE,
	CHAIN,
	CHAIN_DIRECTIVE,
	GREEN_DOT,
	BLACK_DOT,
	DATA_00,
	DATA_05,
	DATA_10,
	DATA_15,
	DATA_20,
	DATA_25,
	DATA_30,
	DATA_35,
	DATA_40,
	DATA_45,
	DATA_50,
	DATA_55,
	DATA_60,
	DATA_65,
	DATA_70,
	DATA_75,
	DATA_80,
	DATA_85,
	DATA_90,
	DATA_95,
	COMMENT,
};

/**
 * @brief Generic interface for operations that may depend on the context of the SEXP tree
 */
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

/*
 * Notes: An sexp_tree_item is basically a node in a tree.  The sexp_tree is an array of
 * these node items.
 */

class sexp_tree_item {
 public:
	sexp_tree_item() : type(SEXPT_UNUSED) {
	}

	int type;
	int parent;    // pointer to parent of this item
	int child;    // pointer to first child of this item
	int next;    // pointer to next sibling
	int flags;
	char text[2 * TOKEN_LENGTH + 2];
	QTreeWidgetItem* handle;
};

class sexp_list_item {
 public:
	int type;
	int op;
	SCP_string text;
	int flags;
	sexp_list_item* next;

	sexp_list_item() : flags(0), next(NULL) {
	}

	void set_op(int op_num);
	void set_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	void set_data_dup(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));

	void add_op(int op_num);
	void add_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_data_dup(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_list(sexp_list_item* list);

	void destroy();
};

class sexp_tree: public QTreeWidget {

 Q_OBJECT
 public:
	static const int FormulaDataRole = Qt::UserRole;

 	static QIcon convertNodeImageToIcon(NodeImage image);

	explicit sexp_tree(QWidget* parent = nullptr);
	~sexp_tree() override;

	int find_text(const char* text, int* find, int max_depth);
	int query_restricted_opf_range(int opf);
	void verify_and_fix_arguments(int node);
	void post_load();
	void update_help(QTreeWidgetItem* h);
	const char* help(int code);
	QTreeWidgetItem* insert(const QString& lpszItem,
							NodeImage image = NodeImage::ROOT,
							QTreeWidgetItem* hParent = nullptr,
							QTreeWidgetItem* hInsertAfter = nullptr);
	QTreeWidgetItem* handle(int node);
	int get_type(QTreeWidgetItem* h);
	int get_node(QTreeWidgetItem* h);
	int query_false(int node = -1);
	int add_default_operator(int op, int argnum);
	int get_default_value(sexp_list_item* item, char* text_buf, int op, int i);
	int query_default_argument_available(int op);
	int query_default_argument_available(int op, int i);
	void swap_roots(QTreeWidgetItem* one, QTreeWidgetItem* two);
	void move_branch(int source, int parent = -1);
	QTreeWidgetItem*
	move_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent = nullptr, QTreeWidgetItem* after = nullptr);
	void copy_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent = nullptr, QTreeWidgetItem* after = nullptr);
	void add_or_replace_operator(int op, int replace_flag = 0);
//	void replace_one_arg_operator(const char *op, const char *data, int type);
	void replace_operator(const char* op);
	void replace_data(const char* new_data, int type);
	void replace_variable_data(int var_idx, int type);
	void ensure_visible(int node);
	int node_error(int node, const char* msg, int* bypass);
	void expand_branch(QTreeWidgetItem* h);
	void expand_operator(int node);
	void merge_operator(int node);
	int identify_arg_type(int node);
	int count_args(int node);
	int ctree_size;
	virtual void build_tree();
	void set_node(int index, int type, const char* text);
	void free_node(int node, int cascade = 0);
	int allocate_node(int parent, int after = -1);
	int allocate_node();
	int find_free_node();
	void clear_tree(const char* op = NULL);
	void reset_handles();
	int save_tree(int node = -1);
	void load_tree(int index, const char* deflt = "true");
	int add_operator(const char* op, QTreeWidgetItem* h = nullptr);
	int add_data(const char* new_data, int type);
	int add_variable_data(const char* new_data, int type);
	void add_sub_tree(int node, QTreeWidgetItem* root);
	int load_sub_tree(int index, bool valid, const char* text);
	void hilite_item(int node);
	SCP_string match_closest_operator(const SCP_string &str, int node);
	void delete_sexp_tree_variable(const char* var_name);
	void modify_sexp_tree_variable(const char* old_name, int sexp_var_index);
	int get_item_index_to_var_index();
	int get_tree_name_to_sexp_variable_index(const char* tree_name);
	int get_modify_variable_type(int parent);
	int get_variable_count(const char* var_name);
	int get_loadout_variable_count(int var_index);

	// Goober5000
	int find_argument_number(int parent_node, int child_node);
	int find_ancestral_argument_number(int parent_op, int child_node);
	int query_node_argument_type(int node);

	//WMC
	int get_sibling_place(int node);
	NodeImage get_data_image(int node);


	sexp_list_item* get_listing_opf(int opf, int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_null();
	sexp_list_item* get_listing_opf_bool(int parent_node = -1);
	sexp_list_item* get_listing_opf_positive();
	sexp_list_item* get_listing_opf_number();
	sexp_list_item* get_listing_opf_ship(int parent_node = -1);
	sexp_list_item* get_listing_opf_wing();
	sexp_list_item* get_listing_opf_subsystem(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_subsystem_type(int parent_node);
	sexp_list_item* get_listing_opf_point();
	sexp_list_item* get_listing_opf_iff();
	sexp_list_item* get_listing_opf_ai_goal(int parent_node);
	sexp_list_item* get_listing_opf_docker_point(int parent_node);
	sexp_list_item* get_listing_opf_dockee_point(int parent_node);
	sexp_list_item* get_listing_opf_message();
	sexp_list_item* get_listing_opf_who_from();
	sexp_list_item* get_listing_opf_priority();
	sexp_list_item* get_listing_opf_waypoint_path();
	sexp_list_item* get_listing_opf_ship_point();
	sexp_list_item* get_listing_opf_ship_wing();
	sexp_list_item* get_listing_opf_ship_wing_wholeteam();
	sexp_list_item* get_listing_opf_ship_wing_shiponteam_point();
	sexp_list_item* get_listing_opf_ship_wing_point();
	sexp_list_item* get_listing_opf_ship_wing_point_or_none();
	sexp_list_item* get_listing_opf_mission_name();
	sexp_list_item* get_listing_opf_goal_name(int parent_node);
	sexp_list_item* get_listing_opf_order_recipient();
	sexp_list_item* get_listing_opf_ship_type();
	sexp_list_item* get_listing_opf_keypress();
	sexp_list_item* get_listing_opf_event_name(int parent_node);
	sexp_list_item* get_listing_opf_ai_order();
	sexp_list_item* get_listing_opf_skill_level();
	sexp_list_item* get_listing_opf_medal_name();
	sexp_list_item* get_listing_opf_weapon_name();
	sexp_list_item* get_listing_opf_ship_class_name();
	sexp_list_item* get_listing_opf_huge_weapon();
	sexp_list_item* get_listing_opf_ship_not_player();
	sexp_list_item* get_listing_opf_jump_nodes();
	sexp_list_item* get_listing_opf_variable_names();
	sexp_list_item* get_listing_opf_skybox_model();
	sexp_list_item* get_listing_opf_skybox_flags();
	sexp_list_item* get_listing_opf_background_bitmap();
	sexp_list_item* get_listing_opf_sun_bitmap();
	sexp_list_item* get_listing_opf_nebula_storm_type();
	sexp_list_item* get_listing_opf_nebula_poof();
	sexp_list_item* get_listing_opf_cargo();
	sexp_list_item* get_listing_opf_ai_class();
	sexp_list_item* get_listing_opf_support_ship_class();
	sexp_list_item* get_listing_opf_arrival_location();
	sexp_list_item* get_listing_opf_arrival_anchor_all();
	sexp_list_item* get_listing_opf_departure_location();
	sexp_list_item* get_listing_opf_ship_with_bay();
	sexp_list_item* get_listing_opf_soundtrack_name();
	sexp_list_item* get_listing_opf_intel_name();
	sexp_list_item* get_listing_opf_string();
	sexp_list_item* get_listing_opf_ssm_class();
	sexp_list_item* get_listing_opf_flexible_argument();
	sexp_list_item* get_listing_opf_ship_or_none();
	sexp_list_item* get_listing_opf_subsystem_or_none(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_subsys_or_generic(int parent_node, int arg_index);
	sexp_list_item* get_listing_opf_turret_target_order();
	sexp_list_item* get_listing_opf_armor_type();
	sexp_list_item* get_listing_opf_damage_type();
	sexp_list_item* get_listing_opf_turret_target_priorities();
	sexp_list_item* get_listing_opf_persona();
	sexp_list_item* get_listing_opf_font();
	sexp_list_item* get_listing_opf_post_effect();
	sexp_list_item* get_listing_opf_hud_elements();
	sexp_list_item* get_listing_opf_sound_environment();
	sexp_list_item* get_listing_opf_sound_environment_option();
	sexp_list_item* get_listing_opf_explosion_option();
	sexp_list_item* get_listing_opf_adjust_audio_volume();
	sexp_list_item* get_listing_opf_weapon_banks();
	sexp_list_item* get_listing_opf_builtin_hud_gauge();
	sexp_list_item* get_listing_opf_custom_hud_gauge();
	sexp_list_item* get_listing_opf_ship_effect();
	sexp_list_item* get_listing_opf_animation_type();
	sexp_list_item* get_listing_opf_mission_moods();
	sexp_list_item* get_listing_opf_ship_flags();
	sexp_list_item* get_listing_opf_team_colors();
	sexp_list_item* get_listing_opf_nebula_patterns();
	sexp_list_item* get_listing_opf_game_snds();
	sexp_list_item* get_listing_opf_fireball();
	sexp_list_item *get_listing_opf_species();
	sexp_list_item *get_listing_opf_language();
	sexp_list_item *get_listing_opf_functional_when_eval_type();


	int getCurrentItemIndex() const;
	void setCurrentItemIndex(int index);
	int select_sexp_node;  // used to select an sexp item on dialog box open.

	void initializeEditor(Editor* edit, SexpTreeEditorInterface* editorInterface = nullptr);

	void deleteCurrentItem();
 signals:
	void miniHelpChanged(const QString& text);
	void helpChanged(const QString& text);
	void modified();

	void rootNodeDeleted(int node);
	void rootNodeRenamed(int node);
	void rootNodeFormulaChanged(int old, int node);

	void selectedRootChanged(int formula);

	// Generated message map functions
 protected:
	QTreeWidgetItem* insertWithIcon(const QString& lpszItem,
									const QIcon& image,
									QTreeWidgetItem* hParent = nullptr,
									QTreeWidgetItem* hInsertAfter = nullptr);

	void customMenuHandler(const QPoint& pos);

	void handleNewItemSelected();

	std::unique_ptr<QMenu> buildContextMenu(QTreeWidgetItem* h);

	int load_branch(int index, int parent);
	int save_branch(int cur, int at_root = 0);
	void free_node2(int node);

	int flag;

	SCP_vector<sexp_tree_item> tree_nodes;
	int total_nodes;

	// This flag is used for keeping track if we are currently editing a tree node
	bool _currently_editing = false;

	int root_item;
	// these 2 variables are used to help location data sources.  Sometimes looking up
	// valid data can require complex code just to get to an index that is required to
	// locate data.  These are set up in right_clicked() to try and short circuit having
	// to do the lookup again in the code that actually does the adding or replacing of
	// the data if it's selected.
	int add_instance;  // a source reference index indicator for adding data
	int replace_instance;  // a source reference index indicator for replacing data

	int item_index;

	Editor* _editor = nullptr;
	SexpTreeEditorInterface* _interface = nullptr;

	// If there is no special interface then we supply a default one which needs to be stored somewhere
	std::unique_ptr<SexpTreeEditorInterface> _owned_interface;

	int Add_count, Replace_count;
	int Modify_variable;

	void handleItemChange(QTreeWidgetItem* item, int column);

	void beginItemEdit(QTreeWidgetItem* item);

	void deleteActionHandler();
	void cutActionHandler();
	void copyActionHandler();
	void pasteActionHandler();
	void addPasteActionHandler();
	void editDataActionHandler();
	void addNumberDataHandler();
	void addStringDataHandler();
	void addReplaceTypedDataHandler(int data_idx, bool replace);
	void handleReplaceVariableAction(int idx);

	void insertOperatorAction(int op);
};

}
}
