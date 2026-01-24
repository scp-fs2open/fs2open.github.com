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
#include "parse/sexp_container.h"
#include "mission/missiongoals.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "mission/missioncampaign.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "ship/ship.h"
#include "ai/ai.h"
#include "ai/ailua.h"
#include "localization/localize.h"
#include "asteroid/asteroid.h"
#include "fireball/fireballs.h"
#include "gamesnd/gamesnd.h"
#include "graphics/software/FontManager.h"
#include "hud/hudartillery.h"
#include "model/model.h"
#include "nebula/neblightning.h"
#include "starfield/starfield.h"
#include "stats/scoring.h"
#include "weapon/emp.h"

#define TREE_NODE_INCREMENT  100

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
	: SexpTreeEditorInterface(flagset<TreeFlags>())
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
	: total_nodes(0), item_index(-1),
	  root_item(-1), select_sexp_node(-1), flag(0),
	  _interface(nullptr), modified(nullptr)
{
}

SexpTreeModel::~SexpTreeModel() = default;

// -----------------------------------------------------------------------
// Tree node management
// -----------------------------------------------------------------------

int SexpTreeModel::find_free_node() const
{
	for (int i = 0; i < (int)tree_nodes.size(); i++) {
		if (tree_nodes[i].type == SEXPT_UNUSED)
			return i;
	}

	return -1;
}

// allocate a node.  Remains used until freed.
int SexpTreeModel::allocate_node()
{
	int node = find_free_node();

	// need more tree nodes?
	if (node < 0) {
		int old_size = (int)tree_nodes.size();

		Assert(TREE_NODE_INCREMENT > 0);

		// allocate in blocks of TREE_NODE_INCREMENT
		tree_nodes.resize(tree_nodes.size() + TREE_NODE_INCREMENT);

		mprintf(("Bumping dynamic tree node limit from %d to %d...\n", old_size, (int)tree_nodes.size()));

#ifndef NDEBUG
		for (int i = old_size; i < (int)tree_nodes.size(); i++) {
			sexp_tree_item* item = &tree_nodes[i];
			Assert(item->type == SEXPT_UNUSED);
		}
#endif

		// our new sexp is the first out of the ones we just created
		node = old_size;
	}

	// reset the new node
	tree_nodes[node].type = SEXPT_UNINIT;
	tree_nodes[node].parent = -1;
	tree_nodes[node].child = -1;
	tree_nodes[node].next = -1;
	tree_nodes[node].flags = 0;
	strcpy_s(tree_nodes[node].text, "<uninitialized tree node>");
	tree_nodes[node].handle = nullptr;

	total_nodes++;
	return node;
}

// allocate a child node under 'parent'.  Appends to end of list.
int SexpTreeModel::allocate_node(int parent, int after)
{
	int i, index = allocate_node();

	if (parent != -1) {
		i = tree_nodes[parent].child;
		if (i == -1) {
			tree_nodes[parent].child = index;

		} else {
			while ((i != after) && (tree_nodes[i].next != -1))
				i = tree_nodes[i].next;

			tree_nodes[index].next = tree_nodes[i].next;
			tree_nodes[i].next = index;
		}
	}

	tree_nodes[index].parent = parent;
	return index;
}

// initialize the data for a node.  Should be called right after a new node is allocated.
void SexpTreeModel::set_node(int node, int type, const char* text)
{
	Assert(type != SEXPT_UNUSED);
	Assert(tree_nodes[node].type != SEXPT_UNUSED);
	tree_nodes[node].type = type;
	size_t max_length;
	if (type & SEXPT_VARIABLE) {
		max_length = 2 * TOKEN_LENGTH + 2;
	} else if (type & (SEXPT_CONTAINER_NAME | SEXPT_CONTAINER_DATA)) {
		max_length = sexp_container::NAME_MAX_LENGTH + 1;
	} else {
		max_length = TOKEN_LENGTH;
	}
	Assert(strlen(text) < max_length);
	strcpy_s(tree_nodes[node].text, text);
}

// free a node and all its children.  Also clears pointers to it, if any.
//   node = node chain to free
//   cascade =  0: free just this node and children under it. (default)
//             !0: free this node and all siblings after it.
void SexpTreeModel::free_node(int node, int cascade)
{
	int i;

	// clear the pointer to node
	i = tree_nodes[node].parent;
	Assert(i != -1);
	if (tree_nodes[i].child == node)
		tree_nodes[i].child = tree_nodes[node].next;

	else {
		i = tree_nodes[i].child;
		while (tree_nodes[i].next != -1) {
			if (tree_nodes[i].next == node) {
				tree_nodes[i].next = tree_nodes[node].next;
				break;
			}

			i = tree_nodes[i].next;
		}
	}

	if (!cascade)
		tree_nodes[node].next = -1;

	// now free up the node and its children
	free_node2(node);
}

// more simple node freer, which works recursively.  It frees the given node and all siblings
// that come after it, as well as all children of these.  Doesn't clear any links to any of
// these freed nodes, so make sure all links are broken first. (i.e. use free_node() if you can)
void SexpTreeModel::free_node2(int node)
{
	Assert(node != -1);
	Assert(tree_nodes[node].type != SEXPT_UNUSED);
	Assert(total_nodes > 0);
	if (modified)
		*modified = 1;
	tree_nodes[node].type = SEXPT_UNUSED;
	total_nodes--;
	if (tree_nodes[node].child != -1)
		free_node2(tree_nodes[node].child);

	if (tree_nodes[node].next != -1)
		free_node2(tree_nodes[node].next);
}

// -----------------------------------------------------------------------
// Tree loading — populate tree_nodes from global Sexp_nodes
// -----------------------------------------------------------------------

// Build "varname(value)" combined text for variable display in tree
void get_combined_variable_name(char* combined_name, const char* sexp_var_name)
{
	int sexp_var_index = get_index_sexp_variable_name(sexp_var_name);

	if (sexp_var_index >= 0)
		sprintf(combined_name, "%s(%s)", Sexp_variables[sexp_var_index].variable_name, Sexp_variables[sexp_var_index].text);
	else
		sprintf(combined_name, "%s(undefined)", sexp_var_name);
}

void SexpTreeModel::clear_tree_data(const char* op)
{
	mprintf(("Resetting dynamic tree node limit from " SIZE_T_ARG " to %d...\n", tree_nodes.size(), 0));

	total_nodes = flag = 0;
	tree_nodes.clear();

	if (op && strlen(op)) {
		set_node(allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
	}
}

void SexpTreeModel::post_load()
{
	if (!flag)
		select_sexp_node = -1;
}

void SexpTreeModel::load_tree_data(int index, const char* deflt)
{
	int cur;

	clear_tree_data();
	root_item = 0;

	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), deflt);
		return;
	}

	if (Sexp_nodes[index].subtype == SEXP_ATOM_NUMBER) {
		cur = allocate_node(-1);
		if (atoi(Sexp_nodes[index].text))
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "true");
		else
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "false");
		return;
	}

	Assert(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR);
	load_branch(index, -1);
}

int SexpTreeModel::load_branch(int index, int parent)
{
	int cur = -1;
	char combined_var_name[2 * TOKEN_LENGTH + 2];

	while (index != -1) {
		int additional_flags = SEXPT_VALID;

		// special check for container modifiers
		if ((parent != -1) && (tree_nodes[parent].type & SEXPT_CONTAINER_DATA)) {
			additional_flags |= SEXPT_MODIFIER;
		}

		Assert(Sexp_nodes[index].type != SEXP_NOT_USED);
		if (Sexp_nodes[index].subtype == SEXP_ATOM_LIST) {
			load_branch(Sexp_nodes[index].first, parent);

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR) {
			cur = allocate_node(parent);
			if ((index == select_sexp_node) && !flag) {
				select_sexp_node = cur;
				flag = 1;
			}

			set_node(cur, (SEXPT_OPERATOR | additional_flags), Sexp_nodes[index].text);
			load_branch(Sexp_nodes[index].rest, cur);
			return cur;

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_NUMBER) {
			cur = allocate_node(parent);
			if (Sexp_nodes[index].type & SEXP_FLAG_VARIABLE) {
				get_combined_variable_name(combined_var_name, Sexp_nodes[index].text);
				set_node(cur, (SEXPT_VARIABLE | SEXPT_NUMBER | additional_flags), combined_var_name);
			} else {
				set_node(cur, (SEXPT_NUMBER | additional_flags), Sexp_nodes[index].text);
			}

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_STRING) {
			cur = allocate_node(parent);
			if (Sexp_nodes[index].type & SEXP_FLAG_VARIABLE) {
				get_combined_variable_name(combined_var_name, Sexp_nodes[index].text);
				set_node(cur, (SEXPT_VARIABLE | SEXPT_STRING | additional_flags), combined_var_name);
			} else {
				set_node(cur, (SEXPT_STRING | additional_flags), Sexp_nodes[index].text);
			}

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_CONTAINER_NAME) {
			Assertion(!(additional_flags & SEXPT_MODIFIER),
				"Found a container name node %s that is also a container modifier. Please report!",
				Sexp_nodes[index].text);
			Assertion(get_sexp_container(Sexp_nodes[index].text) != nullptr,
				"Attempt to load unknown container data %s into SEXP tree. Please report!",
				Sexp_nodes[index].text);
			cur = allocate_node(parent);
			set_node(cur, (SEXPT_CONTAINER_NAME | SEXPT_STRING | additional_flags), Sexp_nodes[index].text);

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_CONTAINER_DATA) {
			cur = allocate_node(parent);
			Assertion(get_sexp_container(Sexp_nodes[index].text) != nullptr,
				"Attempt to load unknown container data %s into SEXP tree. Please report!",
				Sexp_nodes[index].text);
			set_node(cur, (SEXPT_CONTAINER_DATA | SEXPT_STRING | additional_flags), Sexp_nodes[index].text);
			load_branch(Sexp_nodes[index].first, cur);

		} else
			Assert(0);

		if ((index == select_sexp_node) && !flag) {
			select_sexp_node = cur;
			flag = 1;
		}

		index = Sexp_nodes[index].rest;
		if (index == -1)
			return cur;
	}

	return cur;
}

int SexpTreeModel::load_sub_tree(int index, bool valid, const char* text)
{
	int cur;

	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | (valid ? SEXPT_VALID : 0)), text);
		return cur;
	}

	Assert(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR);
	cur = load_branch(index, -1);
	return cur;
}

void SexpTreeModel::move_branch_data(int source, int parent)
{
	int node;

	if (source == -1)
		return;

	// unlink source from its current parent
	node = tree_nodes[source].parent;
	if (node != -1) {
		if (tree_nodes[node].child == source) {
			tree_nodes[node].child = tree_nodes[source].next;
		} else {
			node = tree_nodes[node].child;
			while (tree_nodes[node].next != source) {
				node = tree_nodes[node].next;
				Assert(node != -1);
			}
			tree_nodes[node].next = tree_nodes[source].next;
		}
	}

	// link source as child of new parent
	tree_nodes[source].parent = parent;
	tree_nodes[source].next = -1;
	if (parent != -1 && parent != 0) {
		if (tree_nodes[parent].child == -1) {
			tree_nodes[parent].child = source;
		} else {
			node = tree_nodes[parent].child;
			while (tree_nodes[node].next != -1)
				node = tree_nodes[node].next;
			tree_nodes[node].next = source;
		}
	}
}

// -----------------------------------------------------------------------
// Tree serialization
// -----------------------------------------------------------------------

// get variable name from sexp_tree node .text
static void var_name_from_sexp_tree_text(char* var_name, const char* text)
{
	auto var_name_length = strcspn(text, "(");
	Assert(var_name_length < TOKEN_LENGTH - 1);

	strncpy(var_name, text, var_name_length);
	var_name[var_name_length] = '\0';
}

// builds an sexp of the tree and returns the index of it.  This allocates sexp nodes.
int SexpTreeModel::save_tree(int node) const
{
	if (node < 0) node = root_item;
	Assert(node >= 0);
	Assert(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID));
	Assert(tree_nodes[node].next == -1);  // must make this assumption or else it will confuse code!
	return save_branch(node);
}

#define NO_PREVIOUS_NODE -9
// called recursively to save a tree branch and everything under it
// SEXPT_CONTAINER_NAME and SEXPT_MODIFIER require no special handling here
int SexpTreeModel::save_branch(int cur, int at_root) const
{
	int start, node = -1, last = NO_PREVIOUS_NODE;
	char var_name_text[TOKEN_LENGTH];

	start = -1;
	while (cur != -1) {
		if (tree_nodes[cur].type & SEXPT_OPERATOR) {
			node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, save_branch(tree_nodes[cur].child));

			if ((tree_nodes[cur].parent >= 0) && !at_root) {
				node = alloc_sexp("", SEXP_LIST, SEXP_ATOM_LIST, node, -1);
			}
		} else if (tree_nodes[cur].type & SEXPT_CONTAINER_NAME) {
			Assertion(get_sexp_container(tree_nodes[cur].text) != nullptr,
				"Attempt to save unknown container %s from SEXP tree. Please report!",
				tree_nodes[cur].text);
			node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_CONTAINER_NAME, -1, -1);
		} else if (tree_nodes[cur].type & SEXPT_CONTAINER_DATA) {
			Assertion(get_sexp_container(tree_nodes[cur].text) != nullptr,
				"Attempt to save unknown container %s from SEXP tree. Please report!",
				tree_nodes[cur].text);
			node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_CONTAINER_DATA, save_branch(tree_nodes[cur].child), -1);
		} else if (tree_nodes[cur].type & SEXPT_NUMBER) {
			// allocate number, maybe variable
			if (tree_nodes[cur].type & SEXPT_VARIABLE) {
				var_name_from_sexp_tree_text(var_name_text, tree_nodes[cur].text);
				node = alloc_sexp(var_name_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_NUMBER, -1, -1);
			} else {
				node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_NUMBER, -1, -1);
			}
		} else if (tree_nodes[cur].type & SEXPT_STRING) {
			// allocate string, maybe variable
			if (tree_nodes[cur].type & SEXPT_VARIABLE) {
				var_name_from_sexp_tree_text(var_name_text, tree_nodes[cur].text);
				node = alloc_sexp(var_name_text, (SEXP_ATOM | SEXP_FLAG_VARIABLE), SEXP_ATOM_STRING, -1, -1);
			} else {
				node = alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_STRING, -1, -1);
			}
		} else {
			Assert(0); // unknown and/or invalid type
		}

		if (last == NO_PREVIOUS_NODE) {
			start = node;
		} else if (last >= 0) {
			Sexp_nodes[last].rest = node;
		}

		last = node;
		Assert(last != NO_PREVIOUS_NODE);  // should be impossible
		cur = tree_nodes[cur].next;
		if (at_root) {
			return start;
		}
	}

	return start;
}

// -----------------------------------------------------------------------
// Default argument availability
// -----------------------------------------------------------------------

int SexpTreeModel::query_default_argument_available(int op) const
{
	int i;

	Assert(op >= 0);
	for (i = 0; i < Operators[op].min; i++)
		if (!query_default_argument_available(op, i))
			return 0;

	return 1;
}

int SexpTreeModel::query_default_argument_available(int op, int i) const
{
	int j, type;
	object* ptr;

	type = query_operator_argument_type(op, i);
	switch (type) {
		case OPF_NONE:
		case OPF_NULL:
		case OPF_BOOL:
		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_IFF:
		case OPF_AI_CLASS:
		case OPF_WHO_FROM:
		case OPF_PRIORITY:
		case OPF_SHIP_TYPE:
		case OPF_SUBSYSTEM:
		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_TRANSLATING_SUBSYSTEM:
		case OPF_SUBSYSTEM_TYPE:
		case OPF_DOCKER_POINT:
		case OPF_DOCKEE_POINT:
		case OPF_AI_GOAL:
		case OPF_KEYPRESS:
		case OPF_AI_ORDER:
		case OPF_SKILL_LEVEL:
		case OPF_MEDAL_NAME:
		case OPF_WEAPON_NAME:
		case OPF_INTEL_NAME:
		case OPF_SHIP_CLASS_NAME:
		case OPF_PROP_CLASS_NAME:
		case OPF_HUGE_WEAPON:
		case OPF_JUMP_NODE_NAME:
		case OPF_AMBIGUOUS:
		case OPF_CARGO:
		case OPF_ARRIVAL_LOCATION:
		case OPF_DEPARTURE_LOCATION:
		case OPF_ARRIVAL_ANCHOR_ALL:
		case OPF_SUPPORT_SHIP_CLASS:
		case OPF_SHIP_WITH_BAY:
		case OPF_SOUNDTRACK_NAME:
		case OPF_STRING:
		case OPF_FLEXIBLE_ARGUMENT:
		case OPF_ANYTHING:
		case OPF_DATA_OR_STR_CONTAINER:
		case OPF_SKYBOX_MODEL_NAME:
		case OPF_SKYBOX_FLAGS:
		case OPF_SHIP_OR_NONE:
		case OPF_SUBSYSTEM_OR_NONE:
		case OPF_SHIP_WING_POINT_OR_NONE:
		case OPF_SUBSYS_OR_GENERIC:
		case OPF_BACKGROUND_BITMAP:
		case OPF_SUN_BITMAP:
		case OPF_NEBULA_STORM_TYPE:
		case OPF_NEBULA_POOF:
		case OPF_TURRET_TARGET_ORDER:
		case OPF_TURRET_TYPE:
		case OPF_POST_EFFECT:
		case OPF_TARGET_PRIORITIES:
		case OPF_ARMOR_TYPE:
		case OPF_DAMAGE_TYPE:
		case OPF_FONT:
		case OPF_HUD_ELEMENT:
		case OPF_SOUND_ENVIRONMENT:
		case OPF_SOUND_ENVIRONMENT_OPTION:
		case OPF_EXPLOSION_OPTION:
		case OPF_AUDIO_VOLUME_OPTION:
		case OPF_WEAPON_BANK_NUMBER:
		case OPF_MESSAGE_OR_STRING:
		case OPF_BUILTIN_HUD_GAUGE:
		case OPF_CUSTOM_HUD_GAUGE:
		case OPF_ANY_HUD_GAUGE:
		case OPF_SHIP_EFFECT:
		case OPF_ANIMATION_TYPE:
		case OPF_SHIP_FLAG:
		case OPF_WING_FLAG:
		case OPF_NEBULA_PATTERN:
		case OPF_NAV_POINT:
		case OPF_TEAM_COLOR:
		case OPF_GAME_SND:
		case OPF_FIREBALL:
		case OPF_SPECIES:
		case OPF_LANGUAGE:
		case OPF_FUNCTIONAL_WHEN_EVAL_TYPE:
		case OPF_ANIMATION_NAME:
		case OPF_CONTAINER_VALUE:
		case OPF_WING_FORMATION:
		case OPF_CHILD_LUA_ENUM:
		case OPF_MESSAGE_TYPE:
			return 1;

		case OPF_SHIP:
		case OPF_SHIP_WING:
		case OPF_SHIP_POINT:
		case OPF_SHIP_WING_POINT:
		case OPF_SHIP_WING_WHOLETEAM:
		case OPF_SHIP_WING_SHIPONTEAM_POINT:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START)
					return 1;

				ptr = GET_NEXT(ptr);
			}

			return 0;

		case OPF_SHIP_PROP:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START || ptr->type == OBJ_PROP)
					return 1;

				ptr = GET_NEXT(ptr);
			}

			return 0;

		case OPF_PROP:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_PROP)
					return 1;

				ptr = GET_NEXT(ptr);
			}
			return 0;

		case OPF_SHIP_NOT_PLAYER:
		case OPF_ORDER_RECIPIENT:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP)
					return 1;

				ptr = GET_NEXT(ptr);
			}

			return 0;

		case OPF_WING:
			for (j = 0; j < MAX_WINGS; j++)
				if (Wings[j].wave_count)
					return 1;

			return 0;

		case OPF_PERSONA:
			return Personas.empty() ? 0 : 1;

		case OPF_POINT:
		case OPF_WAYPOINT_PATH:
			return Waypoint_lists.empty() ? 0 : 1;

		case OPF_MISSION_NAME:
			if (!_interface || !_interface->requireCampaignOperators()) {
				if (_interface && !_interface->hasDefaultMissionName())
					return 0;

				return 1;
			}

			if (Campaign.num_missions > 0)
				return 1;

			return 0;

		case OPF_GOAL_NAME: {
			int value;

			value = Operators[op].value;

			if (_interface && _interface->requireCampaignOperators())
				return 1;

			else if (_interface && _interface->hasDefaultGoal(value))
				return 1;

			return 0;
		}

		case OPF_EVENT_NAME: {
			int value;

			value = Operators[op].value;

			if (_interface && _interface->requireCampaignOperators())
				return 1;

			else if (_interface && _interface->hasDefaultEvent(value))
				return 1;

			return 0;
		}

		case OPF_MESSAGE:
			if (_interface && _interface->hasDefaultMessageParamter())
				return 1;

			return 0;

		case OPF_VARIABLE_NAME:
			return (sexp_variable_count() > 0) ? 1 : 0;

		case OPF_SSM_CLASS:
			return Ssm_info.empty() ? 0 : 1;

		case OPF_MISSION_MOOD:
			return Builtin_moods.empty() ? 0 : 1;

		case OPF_CONTAINER_NAME:
			return get_all_sexp_containers().empty() ? 0 : 1;

		case OPF_LIST_CONTAINER_NAME:
			for (const auto& container : get_all_sexp_containers()) {
				if (container.is_list()) {
					return 1;
				}
			}
			return 0;

		case OPF_MAP_CONTAINER_NAME:
			for (const auto& container : get_all_sexp_containers()) {
				if (container.is_map()) {
					return 1;
				}
			}
			return 0;

		case OPF_ASTEROID_TYPES:
			if (!get_list_valid_asteroid_subtypes().empty()) {
				return 1;
			}
			return 0;

		case OPF_DEBRIS_TYPES:
			for (const auto& this_asteroid : Asteroid_info) {
				if (this_asteroid.type == ASTEROID_TYPE_DEBRIS) {
					return 1;
				}
			}
			return 0;

		case OPF_MOTION_DEBRIS:
			if (Motion_debris_info.size() > 0) {
				return 1;
			}
			return 0;

		case OPF_BOLT_TYPE:
			if (Bolt_types.size() > 0) {
				return 1;
			}
			return 0;

		case OPF_TRAITOR_OVERRIDE:
			return Traitor_overrides.empty() ? 0 : 1;

		case OPF_LUA_GENERAL_ORDER:
			return (ai_lua_get_num_general_orders() > 0) ? 1 : 0;

		case OPF_MISSION_CUSTOM_STRING:
			return The_mission.custom_strings.empty() ? 0 : 1;

		default:
			if (!Dynamic_enums.empty()) {
				if ((type - First_available_opf_id) < (int)Dynamic_enums.size()) {
					return 1;
				} else {
					UNREACHABLE("Unhandled SEXP argument type!");
				}
			} else {
				UNREACHABLE("Unhandled SEXP argument type!");
			}
	}

	return 0;
}

// Determine and return the default value for operator argument position i.
// Returns 0 on success, -1 if no default available.
int SexpTreeModel::get_default_value(sexp_list_item* item, char* text_buf, int op, int i)
{
	const char* str = nullptr;
	int type, index;
	sexp_list_item* list;

	index = item_index;
	type = query_operator_argument_type(op, i);
	switch (type)
	{
		case OPF_NULL:
			item->set_op(OP_NOP);
			return 0;

		case OPF_BOOL:
			item->set_op(OP_TRUE);
			return 0;

		case OPF_ANYTHING:
			if (Operators[op].value == OP_INVALIDATE_ARGUMENT || Operators[op].value == OP_VALIDATE_ARGUMENT)
				item->set_data(SEXP_ARGUMENT_STRING);
			else
				item->set_data("<any data>");
			return 0;

		case OPF_DATA_OR_STR_CONTAINER:
			item->set_data("<any data or string container>");
			return 0;

		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_AMBIGUOUS:
			// if the top level operator is an AI goal, and we are adding the last number required,
			// assume that this number is a priority and make it 89 instead of 1.
			if ((query_operator_return_type(op) == OPR_AI_GOAL) && (i == (Operators[op].min - 1)))
			{
				item->set_data("89", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (((Operators[op].value == OP_HAS_DOCKED_DELAY) || (Operators[op].value == OP_HAS_UNDOCKED_DELAY) || (Operators[op].value == OP_TIME_DOCKED) || (Operators[op].value == OP_TIME_UNDOCKED)) && (i == 2))
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ((Operators[op].value == OP_SHIP_TYPE_DESTROYED) || (Operators[op].value == OP_GOOD_SECONDARY_TIME))
			{
				item->set_data("100", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_SET_SUPPORT_SHIP)
			{
				item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ((Operators[op].value == OP_SHIP_TAG) && (i == 1) || (Operators[op].value == OP_TRIGGER_SUBMODEL_ANIMATION) && (i == 3))
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_EXPLOSION_EFFECT)
			{
				int temp;
				char sexp_str_token[TOKEN_LENGTH];

				switch (i)
				{
					case 3:
						temp = 10;
						break;
					case 4:
						temp = 10;
						break;
					case 5:
						temp = 100;
						break;
					case 6:
						temp = 10;
						break;
					case 7:
						temp = 100;
						break;
					case 11:
						temp = (int)EMP_DEFAULT_INTENSITY;
						break;
					case 12:
						temp = (int)EMP_DEFAULT_TIME;
						break;
					default:
						temp = 0;
						break;
				}

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_WARP_EFFECT)
			{
				int temp;
				char sexp_str_token[TOKEN_LENGTH];

				switch (i)
				{
					case 6:
						temp = 100;
						break;
					case 7:
						temp = 10;
						break;
					default:
						temp = 0;
						break;
				}

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_CHANGE_BACKGROUND)
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_ADD_BACKGROUND_BITMAP || Operators[op].value == OP_ADD_BACKGROUND_BITMAP_NEW)
			{
				int temp = 0;
				char sexp_str_token[TOKEN_LENGTH];

				switch (i)
				{
					case 4:
					case 5:
						temp = 100;
						break;

					case 6:
					case 7:
						temp = 1;
						break;
				}

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_ADD_SUN_BITMAP || Operators[op].value == OP_ADD_SUN_BITMAP_NEW)
			{
				int temp = 0;
				char sexp_str_token[TOKEN_LENGTH];

				if (i == 4)
					temp = 100;

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_MISSION_SET_NEBULA)
			{
				if (i == 0)
					item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data("3000", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_MODIFY_VARIABLE)
			{
				if (get_modify_variable_type(index) == OPF_NUMBER)
					item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_MODIFY_VARIABLE_XSTR)
			{
				if (i == 1)
					item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
				else
					item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_SET_VARIABLE_BY_INDEX)
			{
				if (i == 0)
					item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_JETTISON_CARGO_NEW)
			{
				item->set_data("25", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_TECH_ADD_INTEL_XSTR || Operators[op].value == OP_TECH_REMOVE_INTEL_XSTR)
			{
				item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else
			{
				item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
			}

			return 0;

		// Goober5000 - special cases that used to be numbers but are now hybrids
		case OPF_GAME_SND:
		{
			gamesnd_id sound_index;

			if ((Operators[op].value == OP_EXPLOSION_EFFECT))
			{
				sound_index = GameSounds::SHIP_EXPLODE_1;
			}
			else if ((Operators[op].value == OP_WARP_EFFECT))
			{
				sound_index = (i == 8) ? GameSounds::CAPITAL_WARP_IN : GameSounds::CAPITAL_WARP_OUT;
			}

			if (sound_index.isValid())
			{
				game_snd* snd = gamesnd_get_game_sound(sound_index);
				if (can_construe_as_integer(snd->name.c_str()))
					item->set_data(snd->name.c_str(), (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data(snd->name.c_str(), (SEXPT_STRING | SEXPT_VALID));
				return 0;
			}

			// if no hardcoded default, just use the listing default
			break;
		}

		// Goober5000 - ditto
		case OPF_FIREBALL:
		{
			int fireball_index = -1;

			if (Operators[op].value == OP_EXPLOSION_EFFECT)
			{
				fireball_index = FIREBALL_MEDIUM_EXPLOSION;
			}
			else if (Operators[op].value == OP_WARP_EFFECT)
			{
				fireball_index = FIREBALL_WARP;
			}

			if (fireball_index >= 0)
			{
				char* unique_id = Fireball_info[fireball_index].unique_id;
				if (strlen(unique_id) > 0)
					item->set_data(unique_id, (SEXPT_STRING | SEXPT_VALID));
				else
				{
					char num_str[NAME_LENGTH];
					sprintf(num_str, "%d", fireball_index);
					item->set_data(num_str, (SEXPT_NUMBER | SEXPT_VALID));
				}
				return 0;
			}

			// if no hardcoded default, just use the listing default
			break;
		}

		// new default value
		case OPF_PRIORITY:
			item->set_data("Normal", (SEXPT_STRING | SEXPT_VALID));
			return 0;
	}

	list = get_listing_opf(type, index, i);

	// Goober5000 - the way this is done is really stupid, so stupid hacks are needed to deal with it
	// this particular hack is necessary because the argument string should never be a default
	if (list && list->text == SEXP_ARGUMENT_STRING)
	{
		sexp_list_item* first_ptr;

		first_ptr = list;
		list = list->next;

		delete first_ptr;
	}

	if (list)
	{
		// copy the information from the list to the passed-in item
		*item = *list;

		// but use the provided text buffer
		strcpy(text_buf, list->text.c_str());
		item->text = text_buf;

		// get rid of the list, since we're done with it
		list->destroy();
		item->next = nullptr;

		return 0;
	}

	// catch anything that doesn't have a default value.  Just describe what should be here instead
	switch (type)
	{
		case OPF_SHIP:
		case OPF_SHIP_NOT_PLAYER:
		case OPF_SHIP_POINT:
		case OPF_SHIP_WING:
		case OPF_SHIP_PROP:
		case OPF_SHIP_WING_WHOLETEAM:
		case OPF_SHIP_WING_SHIPONTEAM_POINT:
		case OPF_SHIP_WING_POINT:
			str = "<name of ship here>";
			break;

		case OPF_PROP:
			str = "<name of prop here>";
			break;

		case OPF_ORDER_RECIPIENT:
			str = "<all fighters>";
			break;

		case OPF_SHIP_OR_NONE:
		case OPF_SUBSYSTEM_OR_NONE:
		case OPF_SHIP_WING_POINT_OR_NONE:
			str = SEXP_NONE_STRING;
			break;

		case OPF_WING:
			str = "<name of wing here>";
			break;

		case OPF_DOCKER_POINT:
			str = "<docker point>";
			break;

		case OPF_DOCKEE_POINT:
			str = "<dockee point>";
			break;

		case OPF_SUBSYSTEM:
		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_TRANSLATING_SUBSYSTEM:
		case OPF_SUBSYS_OR_GENERIC:
			str = "<name of subsystem>";
			break;

		case OPF_SUBSYSTEM_TYPE:
			str = Subsystem_types[SUBSYSTEM_NONE];
			break;

		case OPF_POINT:
			str = "<waypoint>";
			break;

		case OPF_MESSAGE:
			str = "<Message>";
			break;

		case OPF_WHO_FROM:
			str = "<any wingman>";
			break;

		case OPF_WAYPOINT_PATH:
			str = "<waypoint path>";
			break;

		case OPF_MISSION_NAME:
			str = "<mission name>";
			break;

		case OPF_GOAL_NAME:
			str = "<goal name>";
			break;

		case OPF_SHIP_TYPE:
			str = "<ship type here>";
			break;

		case OPF_EVENT_NAME:
			str = "<event name>";
			break;

		case OPF_HUGE_WEAPON:
			str = "<huge weapon type>";
			break;

		case OPF_JUMP_NODE_NAME:
			str = "<Jump node name>";
			break;

		case OPF_NAV_POINT:
			str = "<Nav 1>";
			break;

		case OPF_ANYTHING:
			str = "<any data>";
			break;

		case OPF_DATA_OR_STR_CONTAINER:
			str = "<any data or string container>";
			break;

		case OPF_PERSONA:
			str = "<persona name>";
			break;

		case OPF_FONT:
			str = font::FontManager::getFont(0)->getName().c_str();
			break;

		case OPF_AUDIO_VOLUME_OPTION:
			str = "Music";
			break;

		case OPF_POST_EFFECT:
			str = "<Effect Name>";
			break;

		case OPF_CUSTOM_HUD_GAUGE:
			str = "<Custom hud gauge>";
			break;

		case OPF_ANY_HUD_GAUGE:
			str = "<Custom or builtin hud gauge>";
			break;

		case OPF_ANIMATION_NAME:
			str = "<Animation trigger name>";
			break;

		case OPF_CONTAINER_VALUE:
			str = "<container value>";
			break;

		case OPF_MESSAGE_TYPE:
			str = Builtin_messages[0].name;
			break;

		case OPF_VARIABLE_NAME:
			str = "<variable name>";
			break;

		case OPF_CONTAINER_NAME:
			str = "<container name>";
			break;

		case OPF_LIST_CONTAINER_NAME:
			str = "<list container name>";
			break;

		case OPF_MAP_CONTAINER_NAME:
			str = "<map container name>";
			break;

		default:
			str = "<new default required!>";
			break;
	}

	item->set_data(str, (SEXPT_STRING | SEXPT_VALID));
	return 0;
}

// -----------------------------------------------------------------------
// Tree navigation helpers
// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------
// Query / analysis functions
// -----------------------------------------------------------------------

int SexpTreeModel::count_args(int node) const
{
	int count = 0;

	while (node != -1) {
		count++;
		node = tree_nodes[node].next;
	}

	return count;
}

// identify what type of argument this is.  You call it with the node of the first argument
// of an operator.  It will search through enough of the arguments to determine what type of
// data they are.
int SexpTreeModel::identify_arg_type(int node) const
{
	int type = -1;

	while (node != -1) {
		Assert(tree_nodes[node].type & SEXPT_VALID);
		switch (SEXPT_TYPE(tree_nodes[node].type)) {
			case SEXPT_OPERATOR:
				type = get_operator_const(tree_nodes[node].text);
				Assert(type);
				return query_operator_return_type(type);

			case SEXPT_NUMBER:
				return OPR_NUMBER;

			case SEXPT_STRING:  // either a ship or a wing
				type = SEXP_ATOM_STRING;
				break;  // don't return, because maybe we can narrow selection down more.
		}

		node = tree_nodes[node].next;
	}

	return type;
}

// given a tree node, returns the argument type it should be.
// OPF_NULL means no value (or a "void" value) is returned.  OPF_NONE means there shouldn't
// be any argument at this position at all.
int SexpTreeModel::query_node_argument_type(int node) const
{
	int parent_node = tree_nodes[node].parent;
	if (parent_node < 0) {		// parent nodes are -1 for a top-level operator like 'when'
		return OPF_NULL;
	}

	int argnum = find_argument_number(parent_node, node);
	if (argnum < 0) {
		return OPF_NONE;
	}

	int op_num = get_operator_index(tree_nodes[parent_node].text);
	if (op_num < 0) {
		return OPF_NONE;
	}

	return query_operator_argument_type(op_num, argnum);
}

// Determine if a given opf code has a restricted argument range (i.e. has a specific, limited
// set of argument values, or has virtually unlimited possibilities.  For example, boolean values
// only have true or false, so it is restricted, but a number could be anything, so it's not.
int SexpTreeModel::query_restricted_opf_range(int opf) const
{
	switch (opf) {
		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_WHO_FROM:

		// Goober5000 - these are needed too (otherwise the arguments revert to their defaults)
		case OPF_STRING:
		case OPF_ANYTHING:
		case OPF_CONTAINER_VALUE: // jg18
		case OPF_DATA_OR_STR_CONTAINER: // jg18
			return 0;
	}

	return 1;
}

int SexpTreeModel::get_sibling_place(int node) const
{
	if (tree_nodes[node].parent < 0 || tree_nodes[node].parent > (int)tree_nodes.size())
		return -1;

	const sexp_tree_item* myparent = &tree_nodes[tree_nodes[node].parent];

	if (myparent->child == -1)
		return -1;

	const sexp_tree_item* mysibling = &tree_nodes[myparent->child];

	int count = 0;
	while (true) {
		if (mysibling == &tree_nodes[node])
			break;

		if (mysibling->next == -1)
			break;

		count++;
		mysibling = &tree_nodes[mysibling->next];
	}

	return count;
}

NodeImage SexpTreeModel::get_data_image(int node) const
{
	int count = get_sibling_place(node) + 1;

	if (count <= 0) {
		return NodeImage::DATA;
	}

	if (count % 5 != 0) {
		return NodeImage::DATA;
	}

	int idx = (count % 100) / 5;

	// There are 20 numbered data icons (DATA_00 through DATA_95)
	if (idx > 20) {
		return NodeImage::DATA;
	}

	return static_cast<NodeImage>(static_cast<int>(NodeImage::DATA_00) + idx);
}

int SexpTreeModel::query_false(int node) const
{
	if (node < 0) node = root_item;
	Assert(node >= 0);
	Assert(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID));
	Assert(tree_nodes[node].next == -1);  // must make this assumption or else it will confuse code!
	if (get_operator_const(tree_nodes[node].text) == OP_FALSE) {
		return TRUE;
	}

	return FALSE;
}

// Look for the valid operator that is the closest match for 'str' and return the operator
// number of it.  What operators are valid is determined by 'node', and an operator is valid
// if it is allowed to fit at position 'node'
const SCP_string& SexpTreeModel::match_closest_operator(const SCP_string& str, int node) const
{
	int z, op, arg_num, opf;

	z = tree_nodes[node].parent;
	if (z < 0) {
		return str;
	}

	op = get_operator_index(tree_nodes[z].text);
	if (op < 0)
		return str;

	// determine which argument we are of the parent
	arg_num = find_argument_number(z, node);
	opf = query_operator_argument_type(op, arg_num);	// check argument type at this position

	// find the best operator
	int best = sexp_match_closest_operator(str, opf);
	if (best < 0) {
		Warning(LOCATION, "Unable to find an operator match for string '%s' and argument type %d", str.c_str(), opf);
		return str;
	}
	return Operators[best].text;
}

const char* SexpTreeModel::help(int code)
{
	int i;

	i = (int)Sexp_help.size();
	while (i--) {
		if (Sexp_help[i].id == code)
			break;
	}

	if (i >= 0)
		return Sexp_help[i].help.c_str();

	return nullptr;
}

int SexpTreeModel::find_text(const char* text, int* find, int max_depth) const
{
	int find_count;

	// initialize find
	for (int i = 0; i < max_depth; i++) {
		find[i] = -1;
	}

	find_count = 0;

	for (size_t i = 0; i < tree_nodes.size(); i++) {
		// only look at used and editable nodes
		if ((tree_nodes[i].flags & EDITABLE) && (tree_nodes[i].type != SEXPT_UNUSED)) {
			// find the text
			if (!stricmp(tree_nodes[i].text, text)) {
				find[find_count++] = static_cast<int>(i);

				// don't exceed max count - array bounds
				if (find_count == max_depth) {
					break;
				}
			}
		}
	}

	return find_count;
}

// -----------------------------------------------------------------------
// Free-function utilities for sexp tree variable text
// -----------------------------------------------------------------------

void get_variable_name_from_sexp_tree_node_text(const char* text, char* var_name)
{
	auto length = strcspn(text, "(");

	strncpy(var_name, text, length);
	var_name[length] = '\0';
}

void get_variable_default_text_from_variable_text(char* text, char* default_text)
{
	char* start;

	// find '('
	start = strstr(text, "(");
	Assert(start);
	start++;

	// get length and copy all but last char ")"
	auto len = strlen(start);
	strncpy(default_text, start, len - 1);

	// add null termination
	default_text[len - 1] = '\0';
}

// -----------------------------------------------------------------------
// Variable / container utilities
// -----------------------------------------------------------------------

int SexpTreeModel::get_item_index_to_var_index() const
{
	// check valid item index and node is a variable
	if ((item_index > 0) && (tree_nodes[item_index].type & SEXPT_VARIABLE)) {
		return get_tree_name_to_sexp_variable_index(tree_nodes[item_index].text);
	} else {
		return -1;
	}
}

int SexpTreeModel::get_tree_name_to_sexp_variable_index(const char* tree_name)
{
	char var_name[TOKEN_LENGTH];

	auto chars_to_copy = strcspn(tree_name, "(");
	Assert(chars_to_copy < TOKEN_LENGTH - 1);

	// Copy up to '(' and add null termination
	strncpy(var_name, tree_name, chars_to_copy);
	var_name[chars_to_copy] = '\0';

	// Look up index
	return get_index_sexp_variable_name(var_name);
}

int SexpTreeModel::get_modify_variable_type(int parent) const
{
	int sexp_var_index = -1;

	Assert(parent >= 0);
	int op_const = get_operator_const(tree_nodes[parent].text);

	Assert(tree_nodes[parent].child >= 0);
	const char* node_text = tree_nodes[tree_nodes[parent].child].text;

	if (op_const == OP_MODIFY_VARIABLE) {
		sexp_var_index = get_tree_name_to_sexp_variable_index(node_text);
	} else if (op_const == OP_SET_VARIABLE_BY_INDEX) {
		if (can_construe_as_integer(node_text)) {
			sexp_var_index = atoi(node_text);
		} else if (strchr(node_text, '(') && strchr(node_text, ')')) {
			// the variable index is itself a variable!
			return OPF_AMBIGUOUS;
		}
	} else {
		Int3();  // should not be called otherwise
	}

	// if we don't have a valid variable, allow replacement with anything
	if (sexp_var_index < 0)
		return OPF_AMBIGUOUS;

	if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_BLOCK || Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NOT_USED) {
		// assume number so that we can allow tree display of number operators
		return OPF_NUMBER;
	} else if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
		return OPF_NUMBER;
	} else if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
		return OPF_AMBIGUOUS;
	} else {
		Int3();
		return 0;
	}
}

int SexpTreeModel::get_variable_count(const char* var_name) const
{
	int count = 0;
	char compare_name[64];

	// get name to compare
	strcpy_s(compare_name, var_name);
	strcat_s(compare_name, "(");

	// look for compare name
	for (size_t idx = 0; idx < tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(tree_nodes[idx].text, compare_name)) {
				count++;
			}
		}
	}

	return count;
}

// Returns the number of times a variable with this name has been used by player loadout
int SexpTreeModel::get_loadout_variable_count(int var_index) const
{
	// we shouldn't be being passed the index of variables that do not exist
	Assert(var_index >= 0 && var_index < MAX_SEXP_VARIABLES);

	int idx;
	int count = 0;

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		for (idx = 0; idx < Team_data[i].num_ship_choices; idx++) {
			if (!strcmp(Team_data[i].ship_list_variables[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}

			if (!strcmp(Team_data[i].ship_count_variables[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}
		}

		for (idx = 0; idx < Team_data[i].num_weapon_choices; idx++) {
			if (!strcmp(Team_data[i].weaponry_pool_variable[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}
			if (!strcmp(Team_data[i].weaponry_amount_variable[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}
		}
	}

	return count;
}

int SexpTreeModel::get_container_usage_count(const SCP_string& container_name) const
{
	int count = 0;

	for (int node_idx = 0; node_idx < (int)tree_nodes.size(); node_idx++) {
		if (is_matching_container_node(node_idx, container_name)) {
			count++;
		}
	}

	return count;
}

bool SexpTreeModel::is_matching_container_node(int node, const SCP_string& container_name) const
{
	return (tree_nodes[node].type & SEXPT_VALID) &&
		   (tree_nodes[node].type & (SEXPT_CONTAINER_NAME | SEXPT_CONTAINER_DATA)) &&
		   !stricmp(tree_nodes[node].text, container_name.c_str());
}

bool SexpTreeModel::is_container_name_argument(int node) const
{
	Assertion(node >= 0 && node < (int)tree_nodes.size(),
		"Attempt to check if out-of-range node %d is a container name argument. Please report!",
		node);

	if (tree_nodes[node].parent == -1) {
		return false;
	}

	const int arg_opf_type = query_node_argument_type(node);
	return is_container_name_opf_type(arg_opf_type);
}

// -----------------------------------------------------------------------
// Context menu state computation
// -----------------------------------------------------------------------

bool SexpTreeModel::is_operator_hidden(int op_value)
{
	switch (op_value) {
		// hidden per GitHub issue #6400
		case OP_GET_VARIABLE_BY_INDEX:
		case OP_SET_VARIABLE_BY_INDEX:
		case OP_COPY_VARIABLE_FROM_INDEX:
		case OP_COPY_VARIABLE_BETWEEN_INDEXES:
		// deprecated operators
		case OP_HITS_LEFT_SUBSYSTEM:
		case OP_CUTSCENES_SHOW_SUBTITLE:
		case OP_ORDER:
		case OP_TECH_ADD_INTEL:
		case OP_TECH_REMOVE_INTEL:
		case OP_HUD_GAUGE_SET_ACTIVE:
		case OP_HUD_ACTIVATE_GAUGE_TYPE:
		case OP_JETTISON_CARGO_DELAY:
		case OP_STRING_CONCATENATE:
		case OP_SET_OBJECT_SPEED_X:
		case OP_SET_OBJECT_SPEED_Y:
		case OP_SET_OBJECT_SPEED_Z:
		case OP_DISTANCE:
		case OP_SCRIPT_EVAL:
		case OP_TRIGGER_SUBMODEL_ANIMATION:
		case OP_ADD_BACKGROUND_BITMAP:
		case OP_ADD_SUN_BITMAP:
		case OP_JUMP_NODE_SET_JUMPNODE_NAME:
		case OP_KEY_RESET:
		case OP_SET_ASTEROID_FIELD:
		case OP_SET_DEBRIS_FIELD:
		case OP_NEBULA_TOGGLE_POOF:
		case OP_NEBULA_FADE_POOF:
			return true;
		default:
			return false;
	}
}

SexpContextMenuState SexpTreeModel::compute_context_menu_state()
{
	SexpContextMenuState state;

	state.campaign_mode = _interface && _interface->requireCampaignOperators();

	// --- Annotations ---
	if (_interface && _interface->getFlags()[TreeFlags::AnnotationsAllowed]) {
		state.can_edit_comment = true;
		state.can_edit_bg_color = true;
	}

	// --- Variables always allowed ---
	state.can_add_variable = true;
	state.can_modify_variable = (sexp_variable_count() > 0);

	// --- Handle labeled root special case ---
	if (item_index == -1) {
		if (_interface && _interface->getFlags()[TreeFlags::LabeledRoot]) {
			state.is_labeled_root = true;
			state.is_root_editable = _interface && _interface->getFlags()[TreeFlags::RootEditable];
			state.can_edit_text = state.is_root_editable;
			state.can_copy = false;
			// Initialize per-operator vectors to all-false
			int num_ops = (int)Operators.size();
			state.op_add_enabled.assign(num_ops, false);
			state.op_replace_enabled.assign(num_ops, false);
			state.op_insert_enabled.assign(num_ops, false);
			return state;
		}
	}

	Assert(item_index != -1);

	// --- Text edit ---
	state.can_edit_text = (tree_nodes[item_index].flags & EDITABLE) != 0;

	// --- Delete ---
	if (tree_nodes[item_index].parent == -1) {
		state.can_delete = false;  // can't delete root
	}

	// --- Variable/container menu building (from node context) ---
	if (item_index >= 0) {
		int type = tree_nodes[item_index].type;

		int parent = tree_nodes[item_index].parent;
		if (parent >= 0) {
			int op = get_operator_index(tree_nodes[parent].text);
			Assertion(op >= 0 || tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
				"Encountered unknown SEXP operator %s. Please report!",
				tree_nodes[parent].text);
			int first_arg = tree_nodes[parent].child;

			// get arg count of item to replace (for variable context)
			int var_replace_count = 0;
			int temp = first_arg;
			while (temp != item_index) {
				var_replace_count++;
				temp = tree_nodes[temp].next;
				if (temp == -1) break;
			}

			int op_type = 0;
			if (op >= 0) {
				op_type = query_operator_argument_type(op, var_replace_count);
			} else {
				Assertion(tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
					"Unknown SEXP operator %s. Please report!",
					tree_nodes[parent].text);
				const auto* p_container = get_sexp_container(tree_nodes[parent].text);
				Assertion(p_container != nullptr,
					"Found modifier for unknown container %s. Please report!",
					tree_nodes[parent].text);
				op_type = p_container->opf_type;
			}
			Assertion(op_type > 0,
				"Found invalid operator type %d for node with text %s. Please report!",
				op_type, tree_nodes[parent].text);

			// Goober5000 - handle ambiguous type
			if (op_type == OPF_AMBIGUOUS) {
				int modify_type = get_modify_variable_type(parent);
				if (modify_type == OPF_NUMBER) {
					type = SEXPT_NUMBER;
				} else if (modify_type == OPF_AMBIGUOUS) {
					type = SEXPT_STRING;
				} else {
					Int3();
					type = tree_nodes[first_arg].type;
				}
			}

			// Goober5000 - certain types accept both integers and a list of strings
			if (op_type == OPF_GAME_SND || op_type == OPF_FIREBALL || op_type == OPF_WEAPON_BANK_NUMBER) {
				type = SEXPT_NUMBER | SEXPT_STRING;
			}

			// jg18 - container values can be anything
			if (op_type == OPF_CONTAINER_VALUE) {
				type = SEXPT_NUMBER | SEXPT_STRING;
			}

			if ((type & SEXPT_STRING) || (type & SEXPT_NUMBER)) {
				// Build variable replacement entries
				int max_sexp_vars = MAX_SEXP_VARIABLES;
				Assert(max_sexp_vars < 512);

				for (int idx = 0; idx < max_sexp_vars; idx++) {
					if (Sexp_variables[idx].type & SEXP_VARIABLE_SET) {
						if (Sexp_variables[idx].type & SEXP_VARIABLE_BLOCK) {
							continue;
						}

						bool enabled = false;
						if ((type & SEXPT_STRING) && (Sexp_variables[idx].type & SEXP_VARIABLE_STRING)) {
							enabled = true;
						}
						if ((type & SEXPT_NUMBER) && (Sexp_variables[idx].type & SEXP_VARIABLE_NUMBER)) {
							enabled = true;
						}
						if (op_type == OPF_VARIABLE_NAME) {
							state.modify_variable = 1;
							enabled = true;
						} else {
							state.modify_variable = 0;
						}
						if (op_type == OPF_NAV_POINT) {
							enabled = true;
						}
						if ((type & SEXPT_MODIFIER) && var_replace_count > 0) {
							enabled = true;
						}

						SexpContextMenuState::VariableEntry entry;
						entry.var_index = idx;
						entry.enabled = enabled;
						state.replace_variables.push_back(entry);
					}
				}

				// Replace Container Name submenu
				if (is_container_name_opf_type(op_type) || op_type == OPF_DATA_OR_STR_CONTAINER) {
					state.show_container_names = true;
					for (const auto& container : get_all_sexp_containers()) {
						bool enabled = false;
						if (op_type == OPF_CONTAINER_NAME) {
							enabled = true;
						} else if ((op_type == OPF_LIST_CONTAINER_NAME) && container.is_list()) {
							enabled = true;
						} else if ((op_type == OPF_MAP_CONTAINER_NAME) && container.is_map()) {
							enabled = true;
						} else if ((op_type == OPF_DATA_OR_STR_CONTAINER) && container.is_of_string_type()) {
							enabled = true;
						}
						state.replace_container_names.push_back({enabled});
					}
				}

				// Replace Container Data submenu
				if (op_type != OPF_VARIABLE_NAME && (op < 0 || !is_argument_provider_op(Operators[op].value))) {
					state.show_container_data = true;
					for (const auto& container : get_all_sexp_containers()) {
						bool enabled = false;
						if ((type & SEXPT_STRING) && any(container.type & ContainerType::STRING_DATA)) {
							enabled = true;
						}
						if ((type & SEXPT_NUMBER) && any(container.type & ContainerType::NUMBER_DATA)) {
							enabled = true;
						}
						if ((tree_nodes[item_index].type & SEXPT_MODIFIER) && var_replace_count > 0) {
							enabled = true;
						}
						state.replace_container_data.push_back({enabled});
					}
				}
			}
		}
	}

	// --- Add type computation ---
	state.add_type = 0;

	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		const int modifier_node = tree_nodes[item_index].child;
		Assertion(modifier_node != -1,
			"No modifier found for container data node %s. Please report!",
			tree_nodes[item_index].text);
		const int modifier_add_count = count_args(modifier_node);

		const auto* p_container = get_sexp_container(tree_nodes[item_index].text);
		Assertion(p_container,
			"Found modifier for unknown container %s. Please report!",
			tree_nodes[item_index].text);

		if (modifier_add_count == 1 && p_container->is_list() &&
			get_list_modifier(tree_nodes[modifier_node].text) == ListModifier::AT_INDEX) {
			state.add_type = OPR_NUMBER;
			state.can_add_number = true;
		} else {
			state.add_type = OPR_STRING;
			state.add_data_list = get_container_multidim_modifiers(item_index);
			if (state.add_data_list) {
				sexp_list_item* ptr = state.add_data_list;
				while (ptr) {
					if (ptr->op >= 0) {
						state.add_enabled_op_indices.push_back(ptr->op);
					}
					ptr = ptr->next;
				}
			}
			state.can_add_number = true;
			state.can_add_string = true;
		}
	} else if (tree_nodes[item_index].flags & OPERAND) {
		state.add_type = OPR_STRING;
		int child = tree_nodes[item_index].child;
		state.add_count = count_args(child);
		int op = get_operator_index(tree_nodes[item_index].text);
		Assert(op >= 0);

		int type = query_operator_argument_type(op, state.add_count);
		state.add_data_opf_type = type;
		state.add_data_list = get_listing_opf(type, item_index, state.add_count);
		if (state.add_data_list) {
			sexp_list_item* ptr = state.add_data_list;
			while (ptr) {
				if (ptr->op >= 0) {
					state.add_enabled_op_indices.push_back(ptr->op);
				}
				ptr = ptr->next;
			}
		}

		if (type == OPF_NONE) {
			state.add_type = 0;
		} else if (type == OPF_NULL) {
			state.add_type = OPR_NULL;
		} else if (type == OPF_FLEXIBLE_ARGUMENT) {
			state.add_type = OPR_FLEXIBLE_ARGUMENT;
		} else if (type == OPF_NUMBER) {
			state.add_type = OPR_NUMBER;
			state.can_add_number = true;
		} else if (type == OPF_POSITIVE) {
			state.add_type = OPR_POSITIVE;
			state.can_add_number = true;
		} else if (type == OPF_BOOL) {
			state.add_type = OPR_BOOL;
		} else if (type == OPF_AI_GOAL) {
			state.add_type = OPR_AI_GOAL;
		} else if (type == OPF_CONTAINER_VALUE) {
			state.can_add_number = true;
		}

		if (state.add_type == OPR_STRING && !is_container_name_opf_type(type)) {
			state.can_add_string = true;
		}
	}

	// --- Replace type computation ---
	state.replace_type = 0;
	int parent = tree_nodes[item_index].parent;
	int replace_opf_type = 0;  // will be set below for clipboard validation
	if (parent >= 0) {
		state.replace_type = OPR_STRING;
		int op = get_operator_index(tree_nodes[parent].text);
		Assertion(op >= 0 || tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
			"Encountered unknown SEXP operator %s. Please report!",
			tree_nodes[parent].text);
		int first_arg = tree_nodes[parent].child;
		int count = count_args(tree_nodes[parent].child);

		if (op >= 0) {
			if (count <= Operators[op].min) {
				state.can_delete = false;
			}
		} else if ((tree_nodes[parent].type & SEXPT_CONTAINER_DATA) && (item_index == first_arg)) {
			Assertion(tree_nodes[item_index].type & SEXPT_MODIFIER,
				"Container data %s node's first modifier %s is not a modifier. Please report!",
				tree_nodes[parent].text, tree_nodes[item_index].text);
			state.can_delete = false;
		}

		// get arg count of item to replace
		state.replace_count = 0;
		int temp = first_arg;
		while (temp != item_index) {
			state.replace_count++;
			temp = tree_nodes[temp].next;
			if (temp == -1) break;
		}

		int type;
		if (op >= 0) {
			// maybe gray delete
			for (int i = state.replace_count + 1; i < count; i++) {
				if (query_operator_argument_type(op, i - 1) != query_operator_argument_type(op, i)) {
					state.can_delete = false;
					break;
				}
			}
			type = query_operator_argument_type(op, state.replace_count);
		} else {
			Assertion(tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
				"Unknown SEXP operator %s. Please report!",
				tree_nodes[parent].text);
			const auto* p_container = get_sexp_container(tree_nodes[parent].text);
			Assertion(p_container != nullptr,
				"Found modifier for unknown container %s. Please report!",
				tree_nodes[parent].text);
			type = p_container->opf_type;
		}

		replace_opf_type = type;

		// special case reset type for ambiguous
		if (type == OPF_AMBIGUOUS) {
			type = get_modify_variable_type(parent);
		}

		// Container modifiers use their own list of possible arguments
		sexp_list_item* replace_list;
		if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
			const auto* p_container = get_sexp_container(tree_nodes[parent].text);
			Assertion(p_container != nullptr,
				"Found modifier for unknown container %s. Please report!",
				tree_nodes[parent].text);
			const int first_modifier = tree_nodes[parent].child;
			if (state.replace_count == 1 && p_container->is_list() &&
				get_list_modifier(tree_nodes[first_modifier].text) == ListModifier::AT_INDEX) {
				replace_list = nullptr;
				state.replace_type = OPR_NUMBER;
			} else {
				replace_list = get_container_modifiers(parent);
			}
		} else {
			replace_list = get_listing_opf(type, parent, state.replace_count);
		}

		// special case: don't allow replace data for variable or container names
		if ((type != OPF_VARIABLE_NAME) && !is_container_name_opf_type(type) && replace_list) {
			state.replace_data_list = replace_list;
			sexp_list_item* ptr = replace_list;
			while (ptr) {
				if (ptr->op >= 0) {
					state.replace_enabled_op_indices.push_back(ptr->op);
				}
				ptr = ptr->next;
			}
		} else if (replace_list) {
			replace_list->destroy();
		}

		if (type == OPF_NONE) {
			state.replace_type = 0;
		} else if (type == OPF_NUMBER) {
			state.replace_type = OPR_NUMBER;
			state.can_replace_number = true;
		} else if (type == OPF_POSITIVE) {
			state.replace_type = OPR_POSITIVE;
			state.can_replace_number = true;
		} else if (type == OPF_BOOL) {
			state.replace_type = OPR_BOOL;
		} else if (type == OPF_NULL) {
			state.replace_type = OPR_NULL;
		} else if (type == OPF_AI_GOAL) {
			state.replace_type = OPR_AI_GOAL;
		} else if (type == OPF_FLEXIBLE_ARGUMENT) {
			state.replace_type = OPR_FLEXIBLE_ARGUMENT;
		} else if (type == OPF_GAME_SND || type == OPF_FIREBALL || type == OPF_WEAPON_BANK_NUMBER) {
			state.replace_type = OPR_POSITIVE;
			state.can_replace_number = true;
		} else if (type == OPF_CONTAINER_VALUE) {
			state.can_replace_number = true;
		}

		if (state.replace_type == OPR_STRING && !is_container_name_opf_type(type)) {
			state.can_replace_string = true;
		}

		if (op >= 0) {
			if (Operators[op].value == OP_MODIFY_VARIABLE) {
				int modify_type = get_modify_variable_type(parent);
				if (modify_type == OPF_NUMBER) {
					state.can_replace_number = true;
					state.can_replace_string = false;
				}
			} else if (Operators[op].value == OP_SET_VARIABLE_BY_INDEX) {
				if (state.replace_count == 0) {
					state.can_replace_number = true;
					state.can_replace_string = false;
				} else {
					int modify_type = get_modify_variable_type(parent);
					if (modify_type == OPF_NUMBER) {
						state.can_replace_number = true;
						state.can_replace_string = false;
					}
				}
			}
		}

		// Container modifier special cases for replace number/string
		if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
			Assertion(tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
				"Container modifier found whose parent %s is not a container. Please report!",
				tree_nodes[parent].text);
			const int first_modifier_node = tree_nodes[parent].child;
			Assertion(first_modifier_node != -1,
				"Container data node named %s has no modifier. Please report!",
				tree_nodes[parent].text);
			const auto* p_container = get_sexp_container(tree_nodes[parent].text);
			Assertion(p_container,
				"Attempt to get first modifier for unknown container %s. Please report!",
				tree_nodes[parent].text);
			const auto& container = *p_container;

			if (state.replace_count == 0) {
				if (container.is_list()) {
					state.can_replace_number = false;
					state.can_replace_string = false;
					state.can_edit_text = false;
				} else if (container.is_map()) {
					if (any(container.type & ContainerType::STRING_KEYS)) {
						state.can_replace_number = false;
						state.can_replace_string = true;
					} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
						state.can_replace_number = true;
						state.can_replace_string = false;
					} else {
						UNREACHABLE("Map container with type %d has unknown key type", (int)container.type);
					}
				} else {
					UNREACHABLE("Unknown container type %d", (int)container.type);
				}
			} else if (state.replace_count == 1 && container.is_list() &&
					   get_list_modifier(tree_nodes[first_modifier_node].text) == ListModifier::AT_INDEX) {
				state.can_replace_number = true;
				state.can_replace_string = false;
			} else {
				state.can_replace_number = true;
				state.can_replace_string = true;
			}
		}

	} else {
		// top node - should be Boolean or Null type
		state.replace_type = _interface ? _interface->getRootReturnType() : OPR_BOOL;
		// Operators matching the replace_type will be enabled by the UI
		// (they iterate operators and check query_operator_return_type)
	}

	// --- Insert operator context type ---
	int z = tree_nodes[item_index].parent;
	Assert(z >= -1);
	if (z != -1) {
		int op = get_operator_index(tree_nodes[z].text);
		Assertion(op != -1 || tree_nodes[z].type & SEXPT_CONTAINER_DATA,
			"Encountered unknown SEXP operator %s. Please report!",
			tree_nodes[z].text);
		int j = tree_nodes[z].child;
		int insert_count = 0;
		while (j != item_index) {
			insert_count++;
			j = tree_nodes[j].next;
		}

		if (op >= 0) {
			state.insert_opf_type = query_operator_argument_type(op, insert_count);
		} else {
			Assertion(tree_nodes[z].type & SEXPT_CONTAINER_DATA,
				"Unknown SEXP operator %s. Please report!",
				tree_nodes[z].text);
			const auto* p_container = get_sexp_container(tree_nodes[z].text);
			Assertion(p_container != nullptr,
				"Found modifier for unknown container %s. Please report!",
				tree_nodes[z].text);
			state.insert_opf_type = p_container->opf_type;
		}
	} else {
		state.insert_opf_type = (state.replace_type == OPR_NULL) ? OPF_NULL : OPF_BOOL;
	}

	// --- Per-operator enabled state ---
	// Pre-compute which operators are enabled for add/replace/insert so the UI
	// layers don't need to duplicate this logic.
	{
		int num_ops = (int)Operators.size();
		state.op_add_enabled.assign(num_ops, false);
		state.op_replace_enabled.assign(num_ops, false);
		state.op_insert_enabled.assign(num_ops, false);

		// Seed from data list matches
		for (int op_idx : state.add_enabled_op_indices) {
			state.op_add_enabled[op_idx] = true;
		}
		for (int op_idx : state.replace_enabled_op_indices) {
			state.op_replace_enabled[op_idx] = true;
		}

		// Enable replace operators for top-level nodes based on return type
		if (parent < 0) {
			for (int j = 0; j < num_ops; j++) {
				if (query_operator_return_type(j) == state.replace_type)
					state.op_replace_enabled[j] = true;
			}
		}

		// Insert: enable based on return type matching and first-arg type matching
		for (int j = 0; j < num_ops; j++) {
			int ret = query_operator_return_type(j);
			int arg0 = query_operator_argument_type(j, 0);

			// Number/positive equivalence hacks
			if ((state.insert_opf_type == OPF_NUMBER) && (arg0 == OPF_POSITIVE)) arg0 = OPF_NUMBER;
			if ((state.insert_opf_type == OPF_POSITIVE) && (arg0 == OPF_NUMBER)) arg0 = OPF_POSITIVE;

			if (sexp_query_type_match(state.insert_opf_type, ret) && (Operators[j].min >= 1) && (arg0 == state.insert_opf_type)) {
				state.op_insert_enabled[j] = true;
			}
		}

		// Disable operators that don't have default arguments available
		for (int j = 0; j < num_ops; j++) {
			if (!query_default_argument_available(j)) {
				state.op_add_enabled[j] = false;
				state.op_replace_enabled[j] = false;
				state.op_insert_enabled[j] = false;
			}
		}

		// Disable non-campaign operators in campaign mode
		if (state.campaign_mode) {
			for (int j = 0; j < num_ops; j++) {
				if (!usable_in_campaign(Operators[j].value)) {
					state.op_add_enabled[j] = false;
					state.op_replace_enabled[j] = false;
					state.op_insert_enabled[j] = false;
				}
			}
		}
	}

	// --- Clipboard paste validation ---
	if ((Sexp_clipboard > -1) && (Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED)) {
		Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);
		Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
			"Attempt to use container name %s from SEXP clipboard. Please report!",
			Sexp_nodes[Sexp_clipboard].text);

		if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
			int j = get_operator_const(CTEXT(Sexp_clipboard));
			Assert(j);
			int ret = query_operator_return_type(j);

			if ((ret == OPR_POSITIVE) && (state.replace_type == OPR_NUMBER))
				ret = OPR_NUMBER;
			if ((ret == OPR_NUMBER) && (state.replace_type == OPR_POSITIVE))
				ret = OPR_POSITIVE;
			if (state.replace_type == ret)
				state.can_paste = true;

			ret = query_operator_return_type(j);
			if ((ret == OPR_POSITIVE) && (state.add_type == OPR_NUMBER))
				ret = OPR_NUMBER;
			if (state.add_type == ret)
				state.can_paste_add = true;

		} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_CONTAINER_DATA) {
			const auto* p_container = get_sexp_container(Sexp_nodes[Sexp_clipboard].text);
			if (p_container != nullptr) {
				const auto& container = *p_container;
				if (any(container.type & ContainerType::NUMBER_DATA)) {
					if (state.replace_type == OPR_NUMBER)
						state.can_paste = true;
					if (state.add_type == OPR_NUMBER)
						state.can_paste_add = true;
				} else if (any(container.type & ContainerType::STRING_DATA)) {
					if (state.replace_type == OPR_STRING && !is_container_name_opf_type(replace_opf_type))
						state.can_paste = true;
					if (state.add_type == OPR_STRING && !is_container_name_opf_type(replace_opf_type))
						state.can_paste_add = true;
				} else {
					UNREACHABLE("Unknown container data type %d", (int)container.type);
				}
			}

		} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
			if ((state.replace_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1))
				state.can_paste = true;
			else if (state.replace_type == OPR_NUMBER)
				state.can_paste = true;

			if ((state.add_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1))
				state.can_paste_add = true;
			else if (state.add_type == OPR_NUMBER)
				state.can_paste_add = true;

		} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
			if (state.replace_type == OPR_STRING && !is_container_name_opf_type(replace_opf_type))
				state.can_paste = true;
			if (state.add_type == OPR_STRING && !is_container_name_opf_type(replace_opf_type))
				state.can_paste_add = true;

		} else {
			Int3();  // unknown sexp type
		}
	}

	// --- Cut requires delete to be enabled ---
	state.can_cut = state.can_delete;

	// --- Modifier/container restrictions ---
	if (tree_nodes[item_index].type & (SEXPT_MODIFIER | SEXPT_CONTAINER_NAME)) {
		state.can_cut = false;
		state.can_copy = false;
		state.can_paste = false;
	}
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		state.can_paste_add = false;
	}

	return state;
}

// -----------------------------------------------------------------------
// compute_help_text — extract help/mini-help text for a node
// -----------------------------------------------------------------------
SexpTreeModel::HelpTextResult SexpTreeModel::compute_help_text(int node_index, const SCP_string& node_comment) const
{
	HelpTextResult result;

	if (node_index < 0 || node_index >= (int)tree_nodes.size() || !tree_nodes[node_index].type) {
		result.help_text = node_comment;
		return result;
	}

	int i = node_index;

	// Prepend empty lines if we have a comment (for non-root nodes)
	SCP_string adjusted_comment = node_comment;
	if (!adjusted_comment.empty())
		adjusted_comment.insert(0, "\r\n\r\n");

	if (SEXPT_TYPE(tree_nodes[i].type) == SEXPT_OPERATOR) {
		// For operators, mini-help stays empty
	} else {
		int z = tree_nodes[i].parent;
		if (z < 0) {
			Warning(LOCATION, "Sexp data \"%s\" has no parent!", tree_nodes[i].text);
			return result;
		}

		int code = get_operator_const(tree_nodes[z].text);
		int index = get_operator_index(tree_nodes[z].text);
		int sibling_place = get_sibling_place(i) + 1;

		// Mini-help box
		if ((SEXPT_TYPE(tree_nodes[i].type) == SEXPT_NUMBER)
			|| ((SEXPT_TYPE(tree_nodes[i].type) == SEXPT_STRING) && sibling_place > 0)) {
			char buffer[10240] = {""};

			const char* helpstr = help(code);
			bool display_number = true;

			if (helpstr != nullptr) {
				char searchstr[32];
				const char* loc = nullptr;
				const char* loc2 = nullptr;

				sprintf(searchstr, "\n%d:", sibling_place);
				loc = strstr(helpstr, searchstr);

				if (loc == nullptr) {
					sprintf(searchstr, "\t%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if (loc == nullptr) {
					sprintf(searchstr, " %d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if (loc == nullptr) {
					sprintf(searchstr, "%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if (loc == nullptr) {
					loc = strstr(helpstr, "Rest:");
				}
				if (loc == nullptr) {
					loc = strstr(helpstr, "All:");
				}

				if (loc != nullptr) {
					while (*loc == '\r' || *loc == '\n' || *loc == ' ' || *loc == '\t')
						loc++;

					loc2 = strpbrk(loc, "\r\n");
					if (loc2 != nullptr) {
						size_t size = loc2 - loc;
						strncpy(buffer, loc, size);
						if (size < sizeof(buffer)) {
							buffer[size] = '\0';
						}
						display_number = false;
					} else {
						strcpy_s(buffer, loc);
						display_number = false;
					}
				}
			}

			if (display_number) {
				sprintf(buffer, "%d:", sibling_place);
			}

			result.mini_help_text = buffer;
		}

		if (index >= 0) {
			int c = 0;
			int j = tree_nodes[z].child;
			while ((j >= 0) && (j != i)) {
				j = tree_nodes[j].next;
				c++;
			}

			Assert(j >= 0);

			// Message text display
			if (query_operator_argument_type(index, c) == OPF_MESSAGE) {
				for (j = 0; j < Num_messages; j++) {
					if (!stricmp(Messages[j].name, tree_nodes[i].text)) {
						SCP_string text;
						sprintf(text, "Message Text:\r\n%s%s", Messages[j].message, adjusted_comment.c_str());
						result.help_text = text;
						return result;
					}
				}
			}

			// Ship flag description
			if (query_operator_argument_type(index, c) == OPF_SHIP_FLAG) {
				Object::Object_Flags object_flag = Object::Object_Flags::NUM_VALUES;
				Ship::Ship_Flags ship_flag = Ship::Ship_Flags::NUM_VALUES;
				Mission::Parse_Object_Flags parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
				AI::AI_Flags ai_flag = AI::AI_Flags::NUM_VALUES;
				SCP_string desc;

				sexp_check_flag_arrays(tree_nodes[i].text, object_flag, ship_flag, parse_obj_flag, ai_flag);

				if (object_flag != Object::Object_Flags::NUM_VALUES) {
					for (size_t n = 0; n < (size_t)Num_object_flag_names; n++) {
						if (object_flag == Object_flag_descriptions[n].flag) {
							desc = Object_flag_descriptions[n].flag_desc;
							break;
						}
					}
				}

				if (ship_flag != Ship::Ship_Flags::NUM_VALUES) {
					for (size_t n = 0; n < (size_t)Num_ship_flag_names; n++) {
						if (ship_flag == Ship_flag_descriptions[n].flag) {
							desc = Ship_flag_descriptions[n].flag_desc;
							break;
						}
					}
				}

				if (ai_flag != AI::AI_Flags::NUM_VALUES) {
					for (size_t n = 0; n < (size_t)Num_ai_flag_names; n++) {
						if (ai_flag == Ai_flag_descriptions[n].flag) {
							desc = Ai_flag_descriptions[n].flag_desc;
							break;
						}
					}
				}

				if (desc.empty()) {
					if (parse_obj_flag != Mission::Parse_Object_Flags::NUM_VALUES) {
						for (size_t n = 0; n < (size_t)Num_parse_object_flags; n++) {
							if (parse_obj_flag == Parse_object_flag_descriptions[n].def) {
								desc = Parse_object_flag_descriptions[n].flag_desc;
								break;
							}
						}
					}
				}

				if (desc.empty())
					desc = "Unknown flag. Let a coder know!";

				result.help_text = desc;
				return result;
			}

			// Wing flag description
			if (query_operator_argument_type(index, c) == OPF_WING_FLAG) {
				Ship::Wing_Flags wing_flag = Ship::Wing_Flags::NUM_VALUES;
				SCP_string desc;

				sexp_check_flag_array(tree_nodes[i].text, wing_flag);

				if (wing_flag != Ship::Wing_Flags::NUM_VALUES) {
					for (size_t n = 0; n < (size_t)Num_wing_flag_names; n++) {
						if (wing_flag == Wing_flag_descriptions[n].flag) {
							desc = Wing_flag_descriptions[n].flag_desc;
							break;
						}
					}
				}

				if (desc.empty())
					desc = "Unknown flag. Let a coder know!";

				result.help_text = desc;
				return result;
			}
		}

		i = z;
	}

	int code = get_operator_const(tree_nodes[i].text);
	auto str = help(code);
	if (!str) {
		result.help_text = SCP_string("No help available") + adjusted_comment;
	} else {
		result.help_text = SCP_string(str) + adjusted_comment;
	}

	return result;
}

// -----------------------------------------------------------------------
// validate_label_edit — validate and resolve edited node text
// -----------------------------------------------------------------------
SexpTreeModel::LabelEditResult SexpTreeModel::validate_label_edit(int node_index, const SCP_string& new_text) const
{
	LabelEditResult result;
	result.resolved_text = new_text;

	if (tree_nodes[node_index].type & SEXPT_OPERATOR) {
		result.is_operator = true;
		auto op = match_closest_operator(new_text, node_index);
		if (op.empty()) {
			result.update_node = false;
			return result;
		}

		result.resolved_text = op;
		result.operator_index = get_operator_index(op.c_str());
		if (result.operator_index < 0) {
			result.update_node = false;
		}
	} else if (tree_nodes[node_index].type & SEXPT_NUMBER) {
		if (query_node_argument_type(node_index) == OPF_POSITIVE) {
			int val = atoi(new_text.c_str());
			if (val < 0) {
				result.negative_number_error = true;
				result.update_node = false;
			}
		}
	}

	// Truncate to TOKEN_LENGTH
	if (result.resolved_text.size() >= TOKEN_LENGTH) {
		result.resolved_text.resize(TOKEN_LENGTH - 1);
	}

	return result;
}

// -----------------------------------------------------------------------
// apply_label_edit — apply validated edit to node text
// -----------------------------------------------------------------------
void SexpTreeModel::apply_label_edit(int node_index, const SCP_string& resolved_text)
{
	auto len = resolved_text.size();
	if (len >= TOKEN_LENGTH)
		len = TOKEN_LENGTH - 1;

	strncpy(tree_nodes[node_index].text, resolved_text.c_str(), len);
	tree_nodes[node_index].text[len] = 0;

	// Sanitize for invalid characters (Mantis #2893)
	lcl_fred_replace_stuff(tree_nodes[node_index].text, TOKEN_LENGTH - 1);
}

// -----------------------------------------------------------------------
// compute_node_visual_info — determine flags and image for a tree node
// -----------------------------------------------------------------------
SexpTreeModel::NodeVisualInfo SexpTreeModel::compute_node_visual_info(int node_index) const
{
	NodeVisualInfo info;

	if (tree_nodes[node_index].type & SEXPT_OPERATOR) {
		info.flags = OPERAND;
		info.image = NodeImage::OPERATOR;
	} else if (tree_nodes[node_index].type & SEXPT_VARIABLE) {
		info.flags = NOT_EDITABLE;
		info.image = NodeImage::VARIABLE;
	} else if (tree_nodes[node_index].type & SEXPT_CONTAINER_NAME) {
		info.flags = NOT_EDITABLE;
		info.image = NodeImage::CONTAINER_NAME;
	} else if (tree_nodes[node_index].type & SEXPT_CONTAINER_DATA) {
		info.flags = NOT_EDITABLE;
		info.image = NodeImage::CONTAINER_DATA;
	} else {
		info.flags = EDITABLE;
		info.image = get_data_image(node_index);
	}

	return info;
}
