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

#include "missioneditor/sexp_tree_opf.h"
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

	// Initialize this item as an operator (converts op value to index if needed)
	void set_op(int op_num);
	// Initialize this item as a data element with the given text and type flags
	void set_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	// Append a new operator item to the end of this linked list
	void add_op(int op_num);
	// Append a new data item to the end of this linked list
	void add_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	// Append another linked list to the end of this one
	void add_list(sexp_list_item* list);
	// Copy fields from src without transferring ownership of the next pointer
	void shallow_copy(const sexp_list_item* src);
	// Delete this item and all subsequent items in the linked list
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
	// Default constructor — uses empty flags (no labeled roots, no root deletion)
	SexpTreeEditorInterface();
	// Construct with explicit tree behavior flags
	explicit SexpTreeEditorInterface(const flagset<TreeFlags>& flags);
	virtual ~SexpTreeEditorInterface();

	// Returns true if there are mission-specific (non-builtin) messages available
	virtual bool hasDefaultMessageParamter();
	// Returns the list of mission-specific message names
	virtual SCP_vector<SCP_string> getMessages();

	// Returns the list of mission goal names (reference_name used by campaign overrides)
	virtual SCP_vector<SCP_string> getMissionGoals(const SCP_string& reference_name);
	// Returns true if a default goal is available for the given operator value
	virtual bool hasDefaultGoal(int operator_value);

	// Returns the list of mission event names (reference_name used by campaign overrides)
	virtual SCP_vector<SCP_string> getMissionEvents(const SCP_string& reference_name);
	// Returns true if a default event is available for the given operator value
	virtual bool hasDefaultEvent(int operator_value);

	// Returns available mission filenames for the mission-name OPF type
	virtual SCP_vector<SCP_string> getMissionNames();
	// Returns true if a mission filename is currently set
	virtual bool hasDefaultMissionName();

	// Returns the expected return type for root-level operators (default: OPR_BOOL)
	virtual int getRootReturnType() const;

	// Returns the tree behavior flags for this editor interface
	const flagset<TreeFlags>& getFlags() const;

	// Returns true if this editor operates in campaign mode (affects operator filtering)
	virtual bool requireCampaignOperators() const;

	// Callbacks for labeled-root tree operations (overridden by dialog classes)
	// Called when a labeled root node is deleted; returns the formula node to select next
	virtual int onRootDeleted(int formula_node) { return formula_node; }
	// Called when a labeled root node's text is renamed
	virtual void onRootRenamed(int formula_node, const char* new_name) { (void)formula_node; (void)new_name; }
	// Called when a new formula is inserted under a labeled root
	virtual void onRootInserted(int old_formula, int new_formula) { (void)old_formula; (void)new_formula; }
	// Called when labeled root nodes are reordered via drag-and-drop
	virtual void onRootMoved(int node1, int node2, bool insert_before) { (void)node1; (void)node2; (void)insert_before; }
};

// -----------------------------------------------------------------------
// Shared free-function utilities for sexp tree variable text
// -----------------------------------------------------------------------

// Extract variable name from "varname(value)" format
void get_variable_name_from_sexp_tree_node_text(const char* text, char* var_name);
// Extract default value from "varname(value)" format
void get_variable_default_text_from_variable_text(char* text, char* default_text);
// Build "varname(value)" combined text for variable display in tree
void get_combined_variable_name(char* combined_name, const char* sexp_var_name);

// -----------------------------------------------------------------------
// ISexpTreeUI — callback interface for UI operations
// -----------------------------------------------------------------------
// Action code in SexpTreeActions calls these to update the UI widget.
// FRED2 implements with MFC CTreeCtrl calls; QtFRED with QTreeWidget calls.
// Handles are opaque void* — each UI layer casts to its native type.

class ISexpTreeUI {
public:
	virtual ~ISexpTreeUI() = default;

	// Tree widget manipulation

	// Insert a new tree item with the given text/icon under parent_handle, after insert_after.
	// Returns the newly created opaque handle.
	virtual void* ui_insert_item(const char* text, NodeImage image,
								  void* parent_handle, void* insert_after) = 0;
	// Remove a tree item from the widget
	virtual void ui_delete_item(void* handle) = 0;
	// Update the displayed text of a tree item
	virtual void ui_set_item_text(void* handle, const char* text) = 0;
	// Update the icon of a tree item
	virtual void ui_set_item_image(void* handle, NodeImage image) = 0;
	// Return the first child handle of the given tree item, or nullptr if none
	virtual void* ui_get_child_item(void* handle) = 0;
	// Return true if the tree item has any children
	virtual bool ui_has_children(void* handle) = 0;
	// Expand a single tree item to show its children
	virtual void ui_expand_item(void* handle) = 0;
	// Set the tree's current selection to this item
	virtual void ui_select_item(void* handle) = 0;
	// Scroll the tree view so that this item is visible
	virtual void ui_ensure_visible(void* handle) = 0;

	// Subtree operations

	// Recursively add child tree items for all children of the given model node
	virtual void ui_add_children_visual(int parent_node_index) = 0;
	// Recursively expand the subtree rooted at the given handle
	virtual void ui_expand_branch(void* handle) = 0;

	// Notifications

	// Notify the UI that model data has been modified (sets dirty flag)
	virtual void ui_notify_modified() = 0;
	// Recalculate and display help text for the given tree item
	virtual void ui_update_help(void* handle) = 0;
};

// -----------------------------------------------------------------------
// Context menu state — computed by model, consumed by UI to build menus
// -----------------------------------------------------------------------

struct SexpContextMenuState {
	// Special labeled root case (item_index == -1 with labeled root mode)
	bool is_labeled_root = false;
	bool is_root_editable = false;

	// Simple boolean flags
	bool can_edit_text = false;
	bool can_edit_comment = false;
	bool can_edit_bg_color = false;
	bool can_add_variable = true;
	bool can_modify_variable = false;
	bool can_copy = true;
	bool can_cut = false;
	bool can_paste = false;
	bool can_paste_add = false;
	bool can_delete = true;
	bool can_replace_number = false;
	bool can_replace_string = false;
	bool can_add_number = false;
	bool can_add_string = false;

	// Types for operator enabling in menus
	int add_type = 0;       // OPR_* return type expected for add context
	int replace_type = 0;   // OPR_* return type expected for replace context
	int insert_opf_type = 0; // OPF_* type for insert context

	// State needed by command handlers
	int add_count = 0;
	int replace_count = 0;
	int modify_variable = 0;

	// Campaign mode filtering
	bool campaign_mode = false;

	// Data lists for add/replace data submenus (caller must call destroy())
	// Entries with op >= 0 are operators to enable; op < 0 are data items for the submenu
	sexp_list_item* add_data_list = nullptr;
	int add_data_opf_type = 0;  // OPF_* type, needed for OPF_VARIABLE_NAME display
	sexp_list_item* replace_data_list = nullptr;

	// Operators enabled from data lists (indices into Operators[])
	SCP_vector<int> add_enabled_op_indices;
	SCP_vector<int> replace_enabled_op_indices;

	// Per-operator enabled state (indexed by operator index, sized to Operators.size())
	// These incorporate all enable/disable logic: data list matches, default argument
	// availability, return type matching, insert type matching, and campaign mode.
	SCP_vector<bool> op_add_enabled;
	SCP_vector<bool> op_replace_enabled;
	SCP_vector<bool> op_insert_enabled;

	// Variable replacement menu entries
	struct VariableEntry {
		int var_index;
		bool enabled;
	};
	SCP_vector<VariableEntry> replace_variables;

	// Container replacement menu entries (index = position in get_all_sexp_containers())
	struct ContainerEntry {
		bool enabled;
	};
	bool show_container_names = false;
	SCP_vector<ContainerEntry> replace_container_names;
	bool show_container_data = false;
	SCP_vector<ContainerEntry> replace_container_data;

	// Free the dynamically allocated data lists (must be called when done with this state)
	void cleanup() {
		if (add_data_list) { add_data_list->destroy(); add_data_list = nullptr; }
		if (replace_data_list) { replace_data_list->destroy(); replace_data_list = nullptr; }
	}
};

// -----------------------------------------------------------------------
// SexpTreeModel — shared UI-independent sexp tree model
// -----------------------------------------------------------------------
// Owns tree node data and provides all pure-logic operations.
// Both FRED2 and QtFRED sexp_tree classes delegate to this model.
// OPF listing functions are in the owned SexpTreeOPF _opf member (see sexp_tree_opf.h).

class SexpTreeModel {
public:
	SexpTreeModel();
	~SexpTreeModel();

	// Tree node storage
	SCP_vector<sexp_tree_item> tree_nodes;
	int total_nodes;
	int item_index;   // currently selected node index, or -1 if none (or labeled root selected)

	// Tree loading state
	int root_item;
	int select_sexp_node;  // translates global sexp node index to tree node during load
	int flag;              // "found select_sexp_node" flag during load

	// Reset select_sexp_node to -1 if it was not found during load_branch
	void post_load();

	// Editor context interface (set by UI layer)
	SexpTreeEditorInterface* _interface;

	// Modification tracking — UI layer sets this to point at its dirty flag.
	// Model code sets *modified = 1 when tree data changes.
	int* modified;

	// --- Tree navigation helpers ---

	// Return the 0-based argument position of child_node among parent_node's children, or -1
	int find_argument_number(int parent_node, int child_node) const;
	// Walk up ancestors to find parent_op, then return the argument position we traversed through
	int find_ancestral_argument_number(int parent_op, int child_node) const;
	// Check if a node is inside a when-argument/every-time-argument action list (eligible for <argument>)
	bool is_node_eligible_for_special_argument(int parent_node) const;

	// --- Tree node management ---

	// Find the first unused slot in tree_nodes, or return -1 if all are used
	int find_free_node() const;
	// Allocate a standalone node (no parent linkage), growing tree_nodes if needed
	int allocate_node();
	// Allocate a node as a child of 'parent', inserted after 'after' (-1 = append to end)
	int allocate_node(int parent, int after = -1);
	// Set the type and text of an already-allocated node
	void set_node(int node, int type, const char* text);
	// Free a node and its children; if cascade!=0 also free all subsequent siblings
	void free_node(int node, int cascade = 0);
	// Internal recursive free — frees node, all its children, and all its next siblings
	void free_node2(int node);

	// --- Tree loading (populate tree_nodes from Sexp_nodes) ---

	// Clear all tree data and optionally create a single root operator node
	void clear_tree_data(const char* op = nullptr);
	// Load a complete sexp formula into tree_nodes from global Sexp_nodes array
	void load_tree_data(int index, const char* deflt = "true");
	// Recursively load a branch of the sexp tree, returning the first node created
	int load_branch(int index, int parent);
	// Load a sub-tree (used for labeled-root trees like events/goals), returns root node
	int load_sub_tree(int index, bool valid, const char* text);

	// --- Tree structure manipulation ---

	// Move a node (and its subtree) from its current parent to a new parent
	void move_branch_data(int source, int parent);

	// --- Tree serialization ---

	// Serialize the entire tree back into global Sexp_nodes, returning the root sexp index
	int save_tree(int node = -1) const;
	// Recursively serialize a branch, returning the starting sexp node index
	int save_branch(int cur, int at_root = 0) const;

	// --- Default argument values ---

	// Determine and populate the default value for argument position i of operator op.
	// Returns 0 on success, -1 if no default available.
	int get_default_value(sexp_list_item* item, char* text_buf, int op, int i);
	// Returns non-zero if all minimum required arguments of operator op have defaults
	int query_default_argument_available(int op) const;
	// Returns non-zero if argument position i of operator op has a default value available
	int query_default_argument_available(int op, int i) const;

	// --- Query / analysis functions ---

	// Count the number of sibling nodes starting from 'node' (following next pointers)
	int count_args(int node) const;
	// Determine the return type of the argument chain starting at 'node'
	int identify_arg_type(int node) const;
	// Return the OPF_* argument type expected at this node's position under its parent
	int query_node_argument_type(int node) const;
	// Returns non-zero if the given OPF type has a restricted (enumerated) set of values
	int query_restricted_opf_range(int opf) const;
	// Return the 0-based sibling position of this node among its parent's children
	int get_sibling_place(int node) const;
	// Return the appropriate numbered data icon for a node based on its sibling position
	NodeImage get_data_image(int node) const;
	// Return TRUE if the root operator is OP_FALSE
	int query_false(int node = -1) const;
	// Find the valid operator whose name most closely matches 'str' at position 'node'
	const SCP_string& match_closest_operator(const SCP_string& str, int node) const;
	// Look up the help text string for the given sexp operator code
	static const char* help(int code);
	// Search all editable nodes for matching text; populates 'find' array, returns match count
	int find_text(const char* text, int* find, int max_depth) const;

	// --- Help text computation ---
	struct HelpTextResult {
		SCP_string help_text;       // full help text for the help box
		SCP_string mini_help_text;  // short argument description for the mini-help box
	};
	// Compute help and mini-help text for a tree node, including any user comment
	HelpTextResult compute_help_text(int node_index, const SCP_string& node_comment) const;

	// --- Label edit validation ---
	struct LabelEditResult {
		bool update_node = true;
		bool is_operator = false;
		SCP_string resolved_text;   // text after operator matching / validation
		int operator_index = -1;    // operator index if resolved, -1 otherwise
		bool negative_number_error = false;  // true if user entered negative for OPF_POSITIVE
	};
	// Validate user-entered text for a node; resolves operator names and checks constraints
	LabelEditResult validate_label_edit(int node_index, const SCP_string& new_text) const;
	// Apply validated text to a node, truncating to TOKEN_LENGTH and sanitizing characters
	void apply_label_edit(int node_index, const SCP_string& resolved_text);

	// --- Node visual info for tree building ---
	struct NodeVisualInfo {
		int flags;       // OPERAND, EDITABLE, or NOT_EDITABLE
		NodeImage image; // icon to display for this node
	};
	// Determine the editability flags and icon for a node based on its type
	NodeVisualInfo compute_node_visual_info(int node_index) const;

	// --- Variable / container utilities ---

	// Convert the currently selected item_index to its corresponding sexp variable index
	int get_item_index_to_var_index() const;
	// Parse a tree node's "varname(value)" text and look up the sexp variable index
	static int get_tree_name_to_sexp_variable_index(const char* tree_name);
	// For modify-variable operators, determine if the target variable expects OPF_NUMBER or OPF_AMBIGUOUS
	int get_modify_variable_type(int parent) const;
	// Count how many tree nodes reference the given variable name
	int get_variable_count(const char* var_name) const;
	// Count how many times a variable is used in player loadout data (Team_data)
	int get_loadout_variable_count(int var_index) const;
	// Count how many tree nodes reference the given container name
	int get_container_usage_count(const SCP_string& container_name) const;
	// Check if a specific node is a valid, matching container reference for the given name
	bool is_matching_container_node(int node, const SCP_string& container_name) const;
	// Check if a node's parent operator expects a container name at this argument position
	bool is_container_name_argument(int node) const;

	// --- OPF listing and container modifier queries ---
	// Owned by this model. Provides get_listing_opf(), get_container_modifiers(), etc.
	// See sexp_tree_opf.h for the full API.
	SexpTreeOPF _opf;

	// --- Context menu state computation ---

	// Analyze the current selection and compute which context menu actions are available.
	// The returned state includes operator enablement, variable/container menus, and
	// clipboard paste validation. Caller must call cleanup() on the result.
	SexpContextMenuState compute_context_menu_state();
	// Returns true if the given operator value should be hidden from menus (deprecated/hidden ops)
	static bool is_operator_hidden(int op_value);

private:
	// --- Private helpers for compute_context_menu_state() ---

	// Set campaign mode, annotation availability, and variable support
	void ctx_init(SexpContextMenuState& state) const;
	// Handle the labeled-root special case; returns true if state is complete (early return)
	bool ctx_handle_labeled_root(SexpContextMenuState& state) const;
	// Build the variable and container replacement menu entries for the selected node
	void ctx_compute_variable_menus(SexpContextMenuState& state) const;
	// Determine what can be added as a new child of the selected node
	void ctx_compute_add_type(SexpContextMenuState& state);
	// Determine what can replace the selected node; returns the OPF type for clipboard validation
	int ctx_compute_replace_type(SexpContextMenuState& state);
	// Determine the OPF type for inserting an operator before the selected node
	void ctx_compute_insert_type(SexpContextMenuState& state) const;
	// Pre-compute which operators are enabled for add/replace/insert
	void ctx_compute_operator_enablement(SexpContextMenuState& state) const;
	// Check if the clipboard contents can be pasted in the current context
	void ctx_validate_clipboard(SexpContextMenuState& state, int replace_opf_type) const;
	// Apply final cut/copy/paste restrictions based on node type
	void ctx_apply_restrictions(SexpContextMenuState& state) const;
};
