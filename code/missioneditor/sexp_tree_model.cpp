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
