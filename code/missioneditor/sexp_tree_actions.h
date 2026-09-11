#pragma once

// Sexp tree action logic — action execution that bridges model and UI.
// These functions modify model data AND update the UI via ISexpTreeUI callbacks.
// Both FRED2 and QtFRED create an instance alongside their SexpTreeModel.

#include "missioneditor/sexp_tree_model.h"

struct sexp_container;

class SexpTreeActions {
public:
	// Construct with references to the shared model and the UI callback interface.
	// The model provides tree node data; the UI interface updates the visual widget.
	SexpTreeActions(SexpTreeModel& model, ISexpTreeUI& ui);

	// Non-copyable and non-movable: permanently bound to its model and UI
	SexpTreeActions(const SexpTreeActions&) = delete;
	SexpTreeActions& operator=(const SexpTreeActions&) = delete;
	SexpTreeActions(SexpTreeActions&&) = delete;
	SexpTreeActions& operator=(SexpTreeActions&&) = delete;

	// --- Replace operations (modify current item_index node) ---

	// Replace the current node with plain data (number or string).
	// Clears any children, updates the icon to a data image, sets the node as EDITABLE,
	// then verifies and fixes subsequent sibling arguments for validity.
	void replace_data(const char* data, int type);
	// Replace the current node with a variable reference.
	// Builds "varname(value)" display text, sets the VARIABLE icon, marks NOT_EDITABLE,
	// then verifies and fixes subsequent sibling arguments.
	void replace_variable_data(int var_idx, int type);
	// Replace the current node with a container name reference.
	// Clears children, sets the CONTAINER_NAME icon, marks NOT_EDITABLE.
	void replace_container_name(const sexp_container& container);
	// Replace the current node with a container data reference.
	// Optionally tests if child nodes can be preserved (same container type),
	// optionally deletes children, and optionally adds a default modifier.
	void replace_container_data(const sexp_container& container, int type, bool test_child_nodes, bool delete_child_nodes, bool set_default_modifier);
	// Replace the current node with a new operator.
	// Clears all children (arguments will be re-added by the caller).
	void replace_operator(const char* op);

	// --- Expand/merge operations ---

	// If the node has a single COMBINED child, expand it into a proper operator+child display.
	// This is used before adding children to an operator that was previously displayed
	// in condensed "operator data" form.
	void expand_operator(int node);

	// --- Add operations (add child under current item_index node) ---

	// Add a plain data child (number or string) under the current node.
	// Expands the operator if needed, returns the new node index.
	int add_data(const char* data, int type);
	// Add a variable reference child under the current node.
	// Sets the VARIABLE icon and marks NOT_EDITABLE. Returns the new node index.
	int add_variable_data(const char* data, int type);
	// Add a container name child under the current node.
	// Sets the CONTAINER_NAME icon and marks NOT_EDITABLE. Returns the new node index.
	int add_container_name(const char* container_name);
	// Add a container data child under the current node.
	// Sets the CONTAINER_DATA icon, marks NOT_EDITABLE, and updates item_index to
	// the new node (so subsequent add_default_modifier works on the right node).
	void add_container_data(const char* container_name);
	// Add an operator child under the current node (or as a new root if item_index == -1).
	// parent_handle is used for root-level insertion in labeled-root trees.
	// Updates item_index to the newly created operator node.
	void add_operator(const char* op, void* parent_handle = nullptr);
	// Add the default first modifier for a container (map key placeholder or list modifier).
	void add_default_modifier(const sexp_container& container);

	// --- Compound operations ---

	// Add or replace an operator with its full set of default arguments.
	// If replacing and the old operator has compatible argument types/count, preserves them.
	// Otherwise, clears old arguments and fills in all minimum required defaults.
	void add_or_replace_operator(int op, int replace_flag = 0);
	// Add a single default argument for position argnum of the given operator.
	// Handles special cases: variable names, container names, modify-variable types.
	// Returns 0 on success, -1 if no default value available.
	int add_default_operator(int op_index, int argnum);
	// Insert an operator above the current node and move the current node under it.
	// root_parent_handle is only used for labeled-root trees, where roots are inserted
	// under a label item instead of as true tree roots.
	int insert_operator(int op, void* root_parent_handle = nullptr);
	// Resolve and apply a typed add/replace-data menu entry by index.
	// add_count and replace_count are the cached argument positions computed by menu state.
	int add_or_replace_typed_data(int data_idx, bool replace, int add_count, int replace_count);
	// Resolve variable type validation/coercion and replace current node with the variable.
	// current_node_type should be the selected node's SEXPT_NUMBER/SEXPT_STRING flags.
	void replace_variable_with_type_validation(int var_idx, int current_node_type, bool allow_type_coercion);

	// --- Validation ---

	// Walk through all arguments of the operator at 'node' and verify each one is
	// still valid for its expected OPF type. Invalid arguments are replaced with the
	// first valid option from the listing. Extra arguments beyond the operator minimum
	// are removed if no valid listing exists. Recurses into child operators.
	void verify_and_fix_arguments(int node);

	// --- Clipboard operations ---

	// Copy the current node (and its subtree) to the global sexp clipboard.
	// Serializes via save_branch and marks as persistent to prevent garbage collection.
	void clipboard_copy();
	// Replace the current node with the contents of the sexp clipboard.
	// Handles operators (with children), container data (with modifiers),
	// numbers, strings, and variable references.
	void clipboard_paste_replace();
	// Add the contents of the sexp clipboard as a new child of the current node.
	// Similar to paste_replace but inserts instead of replacing.
	void clipboard_paste_add();

	// --- Variable/container bulk operations ---

	// Remove all references to the named variable from the tree.
	// Replaces variable nodes with plain "number" or "string" placeholder text.
	void delete_sexp_tree_variable(const char* var_name);
	// Update all references to a renamed/modified variable.
	// Finds nodes matching old_name and replaces them with updated variable data.
	void modify_sexp_tree_variable(const char* old_name, int sexp_var_index);
	// Rename all container name and container data nodes matching old_name to new_name.
	// Returns true if any nodes were renamed.
	bool rename_container_nodes(const SCP_string& old_name, const SCP_string& new_name);

private:
	// Delete all UI children of a node and free their model data.
	// Resets the model's child link to -1.
	void clear_node_children(int node_index);

	SexpTreeModel& _model;  // shared tree node data and logic
	ISexpTreeUI& _ui;       // UI callback interface for widget updates
	int _verify_arguments_reentry_guard = 0;
};
