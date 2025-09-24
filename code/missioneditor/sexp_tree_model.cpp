/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

// Shared sexp tree model — UI-independent data structures and logic.
// Used by both FRED2 (MFC) and QtFRED (Qt) sexp tree implementations.

#include "missioneditor/sexp_tree_model.h"

#include "parse/sexp.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"

// -----------------------------------------------------------------------
// sexp_list_item implementation
// -----------------------------------------------------------------------

void sexp_list_item::set_op(int op_num)
{
	int i;

	if (op_num >= FIRST_OP) {  // do we have an op value instead of an op number (index)?
		for (i = 0; i < (int)Operators.size(); i++)
			if (op_num == Operators[i].value)
				op_num = i;  // convert op value to op number
	}

	op = op_num;
	text = Operators[op].text;
	type = (SEXPT_OPERATOR | SEXPT_VALID);
}

void sexp_list_item::set_data(const char* str, int t)
{
	op = -1;
	text = str;
	type = t;
}

void sexp_list_item::add_op(int op_num)
{
	sexp_list_item* item;
	sexp_list_item* ptr;

	item = new sexp_list_item;
	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = item;
	item->set_op(op_num);
}

void sexp_list_item::add_data(const char* str, int t)
{
	sexp_list_item* item;
	sexp_list_item* ptr;

	item = new sexp_list_item;
	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = item;
	item->set_data(str, t);
}

void sexp_list_item::add_list(sexp_list_item* list)
{
	sexp_list_item* ptr;

	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = list;
}

void sexp_list_item::destroy()
{
	sexp_list_item* ptr;
	sexp_list_item* ptr2;

	ptr = this;
	while (ptr) {
		ptr2 = ptr->next;

		delete ptr;
		ptr = ptr2;
	}
}

// -----------------------------------------------------------------------
// SexpTreeEditorInterface implementation
// -----------------------------------------------------------------------

SexpTreeEditorInterface::SexpTreeEditorInterface()
	: SexpTreeEditorInterface(flagset<TreeFlags>{TreeFlags::LabeledRoot, TreeFlags::RootDeletable})
{
}

SexpTreeEditorInterface::SexpTreeEditorInterface(const flagset<TreeFlags>& flags)
	: _flags(flags)
{
}

SexpTreeEditorInterface::~SexpTreeEditorInterface() = default;

bool SexpTreeEditorInterface::hasDefaultMessageParamter()
{
	return Num_messages > Num_builtin_messages;
}

SCP_vector<SCP_string> SexpTreeEditorInterface::getMessages()
{
	SCP_vector<SCP_string> list;

	for (auto i = Num_builtin_messages; i < Num_messages; i++) {
		list.emplace_back(Messages[i].name);
	}

	return list;
}

SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionGoals(const SCP_string& /*reference_name*/)
{
	SCP_vector<SCP_string> list;
	list.reserve(Mission_goals.size());

	for (const auto& goal : Mission_goals) {
		list.emplace_back(goal.name, 0, NAME_LENGTH - 1);
	}

	return list;
}

bool SexpTreeEditorInterface::hasDefaultGoal(int operator_value)
{
	return (operator_value == OP_PREVIOUS_GOAL_TRUE) || (operator_value == OP_PREVIOUS_GOAL_FALSE)
		|| (operator_value == OP_PREVIOUS_GOAL_INCOMPLETE) || !Mission_goals.empty();
}

SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionEvents(const SCP_string& /*reference_name*/)
{
	SCP_vector<SCP_string> list;
	list.reserve(Mission_events.size());

	for (const auto& event : Mission_events) {
		list.emplace_back(event.name, 0, NAME_LENGTH - 1);
	}

	return list;
}

bool SexpTreeEditorInterface::hasDefaultEvent(int operator_value)
{
	return (operator_value == OP_PREVIOUS_EVENT_TRUE) || (operator_value == OP_PREVIOUS_EVENT_FALSE)
		|| (operator_value == OP_PREVIOUS_EVENT_INCOMPLETE) || !Mission_events.empty();
}

SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionNames()
{
	SCP_vector<SCP_string> list;
	if (*Mission_filename != '\0') {
		list.emplace_back(Mission_filename);
	}
	return list;
}

bool SexpTreeEditorInterface::hasDefaultMissionName()
{
	return *Mission_filename != '\0';
}

int SexpTreeEditorInterface::getRootReturnType() const
{
	return OPR_BOOL;
}

const flagset<TreeFlags>& SexpTreeEditorInterface::getFlags() const
{
	return _flags;
}

bool SexpTreeEditorInterface::requireCampaignOperators() const
{
	return false;
}

// -----------------------------------------------------------------------
// SexpTreeModel implementation
// -----------------------------------------------------------------------

SexpTreeModel::SexpTreeModel()
	: total_nodes(0), m_mode(0), item_index(-1), _interface(nullptr)
{
}

SexpTreeModel::~SexpTreeModel() = default;

// Goober5000
int SexpTreeModel::find_argument_number(int parent_node, int child_node) const
{
	int arg_num, current_node;

	// code moved/adapted from match_closest_operator
	arg_num = 0;
	current_node = tree_nodes[parent_node].child;
	while (current_node >= 0)
	{
		// found?
		if (current_node == child_node)
			return arg_num;

		// continue iterating
		arg_num++;
		current_node = tree_nodes[current_node].next;
	}

	// not found
	return -1;
}

// Goober5000
// backtrack through parents until we find the operator matching
// parent_op, then find the argument we went through
int SexpTreeModel::find_ancestral_argument_number(int parent_op, int child_node) const
{
	if (child_node == -1)
		return -1;

	int parent_node;
	int current_node;

	current_node = child_node;
	parent_node = tree_nodes[current_node].parent;

	while (parent_node >= 0)
	{
		// check if the parent operator is the one we're looking for
		if (get_operator_const(tree_nodes[parent_node].text) == parent_op)
			return find_argument_number(parent_node, current_node);

		// continue iterating up the tree
		current_node = parent_node;
		parent_node = tree_nodes[current_node].parent;
	}

	return -1;
}

bool SexpTreeModel::is_node_eligible_for_special_argument(int parent_node) const
{
	Assertion(parent_node != -1,
		"Attempt to access invalid parent node for special arg eligibility check. Please report!");

	const int w_arg = find_ancestral_argument_number(OP_WHEN_ARGUMENT, parent_node);
	const int e_arg = find_ancestral_argument_number(OP_EVERY_TIME_ARGUMENT, parent_node);
	return w_arg >= 1 || e_arg >= 1;
}
