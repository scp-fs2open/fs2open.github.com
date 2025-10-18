/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#pragma once

// Sexp tree action logic — action execution that bridges model and UI.
// These functions modify model data AND update the UI via ISexpTreeUI callbacks.
// Both FRED2 and QtFRED create an instance alongside their SexpTreeModel.

#include "missioneditor/sexp_tree_model.h"

class sexp_container;

class SexpTreeActions {
public:
	SexpTreeActions(SexpTreeModel& model, ISexpTreeUI& ui);

	// --- Replace operations (modify current item_index node) ---
	void replace_data(const char* data, int type);
	void replace_variable_data(int var_idx, int type);
	void replace_container_name(const sexp_container& container);
	void replace_container_data(const sexp_container& container,
		int type,
		bool test_child_nodes,
		bool delete_child_nodes,
		bool set_default_modifier);
	void replace_operator(const char* op);

	// --- Expand/merge operations ---
	void expand_operator(int node);

	// --- Add operations (add child under current item_index node) ---
	int add_data(const char* data, int type);
	int add_variable_data(const char* data, int type);
	int add_container_name(const char* container_name);
	void add_container_data(const char* container_name);
	void add_operator(const char* op, void* parent_handle = nullptr);
	void add_default_modifier(const sexp_container& container);

	// --- Compound operations ---
	void add_or_replace_operator(int op, int replace_flag = 0);
	int add_default_operator(int op_index, int argnum);

	// --- Validation ---
	void verify_and_fix_arguments(int node);

	// --- Variable/container bulk operations ---
	void delete_sexp_tree_variable(const char* var_name);
	void modify_sexp_tree_variable(const char* old_name, int sexp_var_index);
	bool rename_container_nodes(const SCP_string& old_name, const SCP_string& new_name);

private:
	// Helper: delete all UI children of a node and clear model child link
	void clear_node_children(int node_index);

	SexpTreeModel& _model;
	ISexpTreeUI& _ui;
};
