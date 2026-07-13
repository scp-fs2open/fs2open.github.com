#include "missioneditor/sexp_tree_model.h"
#include "missioneditor/sexp_annotation_model.h"

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

// Shared sexp tree model — UI-independent data structures and logic.
// Used by both FRED2 (MFC) and QtFRED (Qt) sexp tree implementations.

constexpr int TREE_NODE_INCREMENT = 100;

// -----------------------------------------------------------------------
// sexp_list_item implementation
// -----------------------------------------------------------------------

// Initialize this item as an operator.
// If op_num is an op value (>= FIRST_OP), it is converted to an operator index first.
void sexp_list_item::set_op(int op_num)
{
	int op_index = op_num;

	if (op_num >= FIRST_OP) {  // do we have an op value instead of an op number (index)?
		op_index = -1;
		for (int i = 0; i < static_cast<int>(Operators.size()); i++) {
			if (op_num == Operators[i].value) {
				op_index = i; // convert op value to op number
				break;
			}
		}
	}

	if (!SCP_vector_inbounds(Operators, op_index)) {
		op = -1;
		text = "<invalid operator>";
		type = SEXPT_UNINIT;
		return;
	}

	op = op_index;
	text = Operators[op].text;
	type = (SEXPT_OPERATOR | SEXPT_VALID);
}

// Initialize this item as a data element with the given display text and type flags.
void sexp_list_item::set_data(const char* str, int t)
{
	op = -1;
	text = str;
	type = t;
}

// Allocate a new item, append it to the end of this linked list, and set it as an operator.
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

// Allocate a new item, append it to the end of this linked list, and set it as data.
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

// Append another linked list to the tail of this one (transfers ownership).
void sexp_list_item::add_list(sexp_list_item* list)
{
	sexp_list_item* ptr;

	ptr = this;
	while (ptr->next)
		ptr = ptr->next;

	ptr->next = list;
}

// Delete this item and all subsequent items in the linked list.
// WARNING: 'this' is deleted — do not use the pointer after calling.
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

SexpTreeEditorInterface::SexpTreeEditorInterface(const flagset<TreeFlags>& flags)
	: _flags(flags)
{
}

SexpTreeEditorInterface::~SexpTreeEditorInterface() = default;

// Returns true if mission-specific (non-builtin) messages exist.
bool SexpTreeEditorInterface::hasDefaultMessageParameter()
{
	return Num_messages > Num_builtin_messages;
}

// Return the names of all mission-specific messages (skipping builtins).
SCP_vector<SCP_string> SexpTreeEditorInterface::getMessages()
{
	SCP_vector<SCP_string> list;

	for (auto i = Num_builtin_messages; i < Num_messages; i++) {
		list.emplace_back(Messages[i].name);
	}

	return list;
}

// Return all mission goal names. Campaign editor overrides this to filter by reference mission.
SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionGoals(const SCP_string& /*reference_name*/)
{
	SCP_vector<SCP_string> list;
	list.reserve(Mission_goals.size());

	for (const auto& goal : Mission_goals) {
		list.emplace_back(goal.name, 0, NAME_LENGTH - 1);
	}

	return list;
}

// Returns true if a goal name can be provided as a default for the given operator.
// Always true for previous-goal operators; otherwise requires goals to exist.
bool SexpTreeEditorInterface::hasDefaultGoal(int operator_value)
{
	return (operator_value == OP_PREVIOUS_GOAL_TRUE) || (operator_value == OP_PREVIOUS_GOAL_FALSE)
		|| (operator_value == OP_PREVIOUS_GOAL_INCOMPLETE) || !Mission_goals.empty();
}

// Return all mission event names. Campaign editor overrides this to filter by reference mission.
SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionEvents(const SCP_string& /*reference_name*/)
{
	SCP_vector<SCP_string> list;
	list.reserve(Mission_events.size());

	for (const auto& event : Mission_events) {
		list.emplace_back(event.name, 0, NAME_LENGTH - 1);
	}

	return list;
}

// Returns true if an event name can be provided as a default for the given operator.
bool SexpTreeEditorInterface::hasDefaultEvent(int operator_value)
{
	return (operator_value == OP_PREVIOUS_EVENT_TRUE) || (operator_value == OP_PREVIOUS_EVENT_FALSE)
		|| (operator_value == OP_PREVIOUS_EVENT_INCOMPLETE) || !Mission_events.empty();
}

// Return the current mission filename (campaign editor overrides for multi-mission lists).
SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionNames()
{
	SCP_vector<SCP_string> list;
	if (*Mission_filename != '\0') {
		list.emplace_back(Mission_filename);
	}
	return list;
}

// Returns true if a mission filename is currently set (non-empty).
bool SexpTreeEditorInterface::hasDefaultMissionName()
{
	return *Mission_filename != '\0';
}

// Returns the expected return type for root operators. Default is OPR_BOOL.
// Debriefing overrides this to return OPR_NULL.
int SexpTreeEditorInterface::getRootReturnType() const
{
	return OPR_BOOL;
}

// Return the tree behavior flags configured for this editor interface.
const flagset<TreeFlags>& SexpTreeEditorInterface::getFlags() const
{
	return _flags;
}

// Returns false by default. Campaign editor overrides to return true,
// which restricts the operator list to campaign-usable operators only.
bool SexpTreeEditorInterface::requireCampaignOperators() const
{
	return false;
}

// -----------------------------------------------------------------------
// SexpTreeModel implementation
// -----------------------------------------------------------------------

// Initialize model with all state cleared. The UI layer must set _interface and
// modified before using the tree.
SexpTreeModel::SexpTreeModel()
	: total_nodes(0), item_index(-1),
	  root_item(-1), select_sexp_node(-1), flag(0),
	  _interface(nullptr), modified(nullptr),
	  _opf(*this)
{
}

SexpTreeModel::~SexpTreeModel() = default;

// -----------------------------------------------------------------------
// Tree node management
// -----------------------------------------------------------------------

// Scan for the first unused slot in the tree_nodes array. Returns -1 if none available.
int SexpTreeModel::find_free_node() const
{
	for (int i = 0; i < static_cast<int>(tree_nodes.size()); i++) {
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
		int old_size = static_cast<int>(tree_nodes.size());

		Assertion(TREE_NODE_INCREMENT > 0, "Invalid tree node increment");

		// allocate in blocks of TREE_NODE_INCREMENT
		tree_nodes.resize(tree_nodes.size() + TREE_NODE_INCREMENT);

		nprintf(("Fred", "Bumping dynamic tree node limit from %d to %d...\n", old_size, static_cast<int>(tree_nodes.size())));

#ifndef NDEBUG
		for (int i = old_size; i < static_cast<int>(tree_nodes.size()); i++) {
			sexp_tree_item* item = &tree_nodes[i];
			Assertion(item->type == SEXPT_UNUSED, "Invalid tree node type");
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

// allocate a child node under 'parent', inserting it directly after the existing
// sibling whose tree_nodes index is 'after'.  If 'after' is not found in parent's
// child list (e.g. -1, or a node belonging to a different parent), the new node is
// appended to the end of the sibling chain instead.
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
	Assertion(type != SEXPT_UNUSED, "Invalid node type");
	Assertion(tree_nodes[node].type != SEXPT_UNUSED, "Uninitialized tree node");
	tree_nodes[node].type = type;
	size_t max_length;
	if (type & SEXPT_VARIABLE) {
		max_length = SEXP_TREE_NODE_TEXT_SIZE;
	} else if (type & (SEXPT_CONTAINER_NAME | SEXPT_CONTAINER_DATA)) {
		max_length = sexp_container::NAME_MAX_LENGTH + 1;
	} else {
		max_length = TOKEN_LENGTH;
	}
	Assertion(strlen(text) < max_length, "Text exceeds maximum length");
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
	Assertion(i != -1, "Invalid parent node");
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
	Assertion(node != -1, "Invalid node index");
	Assertion(tree_nodes[node].type != SEXPT_UNUSED, "Uninitialized tree node");
	Assertion(total_nodes > 0, "Invalid total nodes count");
	if (modified)
		*modified = 1;
	tree_nodes[node].type = SEXPT_UNUSED;
	tree_nodes[node].handle = nullptr;
	total_nodes--;

	// Remove any annotation referencing this node so that if allocate_node()
	// reuses this slot, the new node won't inherit a stale annotation.  Also
	// remove any root-label annotation keyed to this node as an event formula
	// (rootKey), so that an event whose formula later lands in this slot won't
	// inherit a deleted event's annotation.  The global Event_annotations are
	// only rewritten at save, so annotations removed here still survive a Cancel.
	if (annotation_model) {
		annotation_model->removeByKey(node);
		annotation_model->removeByKey(SexpAnnotationModel::rootKey(node));
	}

	// Recursively free children and following siblings.
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
		snprintf(combined_name, SEXP_TREE_NODE_TEXT_SIZE, "%s(%s)", Sexp_variables[sexp_var_index].variable_name, Sexp_variables[sexp_var_index].text);
	else
		snprintf(combined_name, SEXP_TREE_NODE_TEXT_SIZE, "%s(undefined)", sexp_var_name);
}

// Clear all tree nodes and reset counters. If 'op' is provided and non-empty,
// create a single root operator node with that text (e.g. "true" or "false").
void SexpTreeModel::clear_tree_data(const char* op)
{
	mprintf(("Resetting dynamic tree node limit from " SIZE_T_ARG " to %d...\n", tree_nodes.size(), 0));

	total_nodes = flag = 0;
	tree_nodes.clear();

	if (op && strlen(op)) {
		set_node(allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
	}
}

// After loading, reset select_sexp_node to -1 if the target node was not found.
void SexpTreeModel::post_load()
{
	if (!flag)
		select_sexp_node = -1;
}

// Load a complete sexp formula from the global Sexp_nodes array into tree_nodes.
// If index < 0, creates a single root with the default text. If the root is a
// bare number, converts it to "true"/"false".
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

	Assertion(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR, "Invalid SEXP node subtype");
	load_branch(index, -1);
}

// Recursively load a chain of sexp nodes (following rest pointers) as children of 'parent'.
// Handles operators, numbers, strings, variables, and container references.
// Also tracks select_sexp_node for auto-selection after load.
// Returns the index of the first tree node created in this chain.
int SexpTreeModel::load_branch(int index, int parent)
{
	int cur = -1;
	char combined_var_name[SEXP_TREE_NODE_TEXT_SIZE];

	while (index != -1) {
		int additional_flags = SEXPT_VALID;

		// special check for container modifiers
		if ((parent != -1) && (tree_nodes[parent].type & SEXPT_CONTAINER_DATA)) {
			additional_flags |= SEXPT_MODIFIER;
		}

		Assertion(Sexp_nodes[index].type != SEXP_NOT_USED, "Invalid SEXP node type");
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
			Assertion(0, "Unknown SEXP node subtype");

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

// Load a sub-tree for labeled-root trees (events, goals, cutscenes).
// If index < 0, creates a single operator node with the given text.
// Returns the root node index of the loaded sub-tree.
int SexpTreeModel::load_sub_tree(int index, bool valid, const char* text)
{
	int cur;

	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | (valid ? SEXPT_VALID : 0)), text);
		return cur;
	}

	Assertion(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR, "Invalid SEXP node subtype");
	cur = load_branch(index, -1);
	return cur;
}

// Unlink a node from its current parent and re-parent it under 'parent'.
// The node (and its entire subtree) is appended as the last child of the new parent.
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
				Assertion(node != -1, "Invalid node");
			}
			tree_nodes[node].next = tree_nodes[source].next;
		}
	}

	// link source as child of new parent
	tree_nodes[source].parent = parent;
	tree_nodes[source].next = -1;
	if (parent != -1) {
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

// Extract just the variable name from "varname(value)" display format into var_name.
static void var_name_from_sexp_tree_text(char* var_name, const char* text)
{
	auto var_name_length = strcspn(text, "(");
	Assertion(var_name_length < TOKEN_LENGTH - 1, "Variable name too long");

	strncpy(var_name, text, var_name_length);
	var_name[var_name_length] = '\0';
}

// builds an sexp of the tree and returns the index of it.  This allocates sexp nodes.
int SexpTreeModel::save_tree(int node) const
{
	if (node < 0) node = root_item;
	Assertion(node >= 0, "Invalid root item");
	Assertion(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID), "Invalid root item type");
	Assertion(tree_nodes[node].next == -1, "Invalid root item next");
	return save_branch(node);
}

constexpr int NO_PREVIOUS_NODE = -9;
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
			Assertion(0, "Unknown and/or invalid type");
		}

		if (last == NO_PREVIOUS_NODE) {
			start = node;
		} else if (last >= 0) {
			Sexp_nodes[last].rest = node;
		}

		last = node;
		Assertion(last != NO_PREVIOUS_NODE, "Invalid last node");
		cur = tree_nodes[cur].next;
		if (at_root) {
			return start;
		}
	}

	return start;
}

// -----------------------------------------------------------------------
// Tree navigation helpers
// -----------------------------------------------------------------------

// Return the 0-based argument position of child_node among parent_node's children.
// Returns -1 if child_node is not a direct child of parent_node.
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

// Walk up the tree from child_node until we find an ancestor whose operator constant
// matches parent_op, then return which argument of that ancestor we came through.
// Returns -1 if no matching ancestor is found.
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

// Check if the given node is inside a when-argument or every-time-argument action list,
// which makes the special <argument> string a valid value at this position.
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

// Count the number of nodes in the sibling chain starting from 'node'.
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
		Assertion(tree_nodes[node].type & SEXPT_VALID, "Invalid tree node");
		switch (SEXPT_TYPE(tree_nodes[node].type)) {
			case SEXPT_OPERATOR:
				type = get_operator_const(tree_nodes[node].text);
				Assertion(type, "Invalid operator");
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
int SexpTreeModel::query_restricted_opf_range(int opf)
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

// Return the 0-based position of 'node' among its parent's children.
// Returns -1 if the node has no valid parent.
int SexpTreeModel::get_sibling_place(int node) const
{
	if (!SCP_vector_inbounds(tree_nodes, node))
		return -1;
	
	if (!SCP_vector_inbounds(tree_nodes, tree_nodes[node].parent))
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

// Return a numbered data icon based on sibling position (every 5th argument gets a
// numbered icon for visual grouping). Falls back to plain DATA icon for most positions.
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
	if (idx >= 20) {
		return NodeImage::DATA;
	}

	return static_cast<NodeImage>(static_cast<int>(NodeImage::DATA_00) + idx);
}

// Return true if the root-level operator is the "false" sexp operator.
bool SexpTreeModel::query_false(int node) const
{
	if (node < 0) node = root_item;
	Assertion(node >= 0, "Invalid node");
	Assertion(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID), "Invalid node type");
	Assertion(tree_nodes[node].next == -1, "Invalid node next");
	return get_operator_const(tree_nodes[node].text) == OP_FALSE;
}

// Look for the valid operator that is the closest match for 'str' and return the operator
// number of it.  What operators are valid is determined by 'node', and an operator is valid
// if it is allowed to fit at position 'node'
SCP_string SexpTreeModel::match_closest_operator(const SCP_string& str, int node) const
{
	int z, op, arg_num, opf;

	z = tree_nodes[node].parent;
	if (z < 0) {
		return str;
	}

	op = get_operator_index(tree_nodes[z].text);
	if (op < 0)
		return str;

	// determine which argument slot 'node' occupies in its parent, and look up
	// the OPF type the parent operator expects at that position
	arg_num = find_argument_number(z, node);
	opf = query_operator_argument_type(op, arg_num);

	// find the best operator
	int best = sexp_match_closest_operator(str, opf);
	if (best < 0) {
		Warning(LOCATION, "Unable to find an operator match for string '%s' and argument type %d", str.c_str(), opf);
		return str;
	}
	return Operators[best].text;
}

// Look up the help text string for the given sexp operator code from the Sexp_help table.
// Returns nullptr if no help text exists for this operator.
const char* SexpTreeModel::help(int code)
{
	int i;

	i = static_cast<int>(Sexp_help.size());
	while (i--) {
		if (Sexp_help[i].id == code)
			break;
	}

	if (i >= 0)
		return Sexp_help[i].help.c_str();

	return nullptr;
}

// Search all editable, used tree nodes for exact case-insensitive text matches.
// Populates 'find' array with matching node indices (up to max_depth entries).
// Returns the number of matches found.
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

// Extract the variable name portion from "varname(value)" display format.
// Copies everything before '(' into var_name.
void get_variable_name_from_sexp_tree_node_text(const char* text, char* var_name)
{
	auto length = strcspn(text, "(");

	strncpy(var_name, text, length);
	var_name[length] = '\0';
}

// Extract the default value portion from "varname(value)" display format.
// Copies the text between '(' and ')' into default_text.
void get_variable_default_text_from_variable_text(char* text, char* default_text)
{
	char* start;

	// find '('
	start = strstr(text, "(");
	Assertion(start, "Invalid variable text format");
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

// If the currently selected node (item_index) is a variable node, look up and
// return its sexp variable index. Returns -1 if not a variable or invalid.
int SexpTreeModel::get_item_index_to_var_index() const
{
	// check valid item index and node is a variable
	if (SCP_vector_inbounds(tree_nodes, item_index) && (tree_nodes[item_index].type & SEXPT_VARIABLE)) {
		return get_tree_name_to_sexp_variable_index(tree_nodes[item_index].text);
	} else {
		return -1;
	}
}

// Parse a "varname(value)" formatted tree node text, extract the variable name,
// and look up its index in the global Sexp_variables array.
int SexpTreeModel::get_tree_name_to_sexp_variable_index(const char* tree_name)
{
	char var_name[TOKEN_LENGTH];

	auto chars_to_copy = strcspn(tree_name, "(");
	Assertion(chars_to_copy < TOKEN_LENGTH - 1, "Variable name too long");

	// Copy up to '(' and add null termination
	strncpy(var_name, tree_name, chars_to_copy);
	var_name[chars_to_copy] = '\0';

	// Look up index
	return get_index_sexp_variable_name(var_name);
}

// For modify-variable and set-variable-by-index operators, determine the expected
// type of the value argument. Returns OPF_NUMBER for numeric variables,
// OPF_AMBIGUOUS for string variables or when the type can't be determined.
int SexpTreeModel::get_modify_variable_type(int parent) const
{
	int sexp_var_index = -1;

	Assertion(parent >= 0, "Invalid parent node");
	int op_const = get_operator_const(tree_nodes[parent].text);

	Assertion(tree_nodes[parent].child >= 0, "Invalid child node");
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
		Assertion(false, "get_modify_variable_type called for operator %d, which is neither modify-variable nor set-variable-by-index", op_const);
	}

	// if we don't have a valid variable, allow replacement with anything
	if (sexp_var_index < 0)
		return OPF_AMBIGUOUS;

	const int var_type = Sexp_variables[sexp_var_index].type;

	// BLOCK/NOT_USED are treated as numbers so that the tree can still display
	// number operators against them.
	if (var_type & (SEXP_VARIABLE_BLOCK | SEXP_VARIABLE_NOT_USED | SEXP_VARIABLE_NUMBER)) {
		return OPF_NUMBER;
	} else if (var_type & SEXP_VARIABLE_STRING) {
		return OPF_AMBIGUOUS;
	} else {
		Assertion(false, "Sexp variable '%s' has unrecognized type flags 0x%x", Sexp_variables[sexp_var_index].variable_name, var_type);
		return 0;
	}
}

// Count how many tree nodes reference the given variable name.
// Matches by checking for "varname(" prefix in variable-type nodes.
int SexpTreeModel::get_variable_count(const char* var_name) const
{
	int count = 0;
	char compare_name[64];

	// get name to compare
	strcpy_s(compare_name, var_name);
	strcat_s(compare_name, "(");

	// look for compare name
	for (const auto& tree_node : tree_nodes) {
		if (tree_node.type & SEXPT_VARIABLE) {
			if (strstr(tree_node.text, compare_name)) {
				count++;
			}
		}
	}

	return count;
}

// Returns the number of times a variable with this name has been used by player loadout
int SexpTreeModel::get_loadout_variable_count(int var_index)
{
	// we shouldn't be being passed the index of variables that do not exist
	Assertion(var_index >= 0 && var_index < MAX_SEXP_VARIABLES, "Invalid variable index");

	int idx;
	int count = 0;

	for (auto& team_datum : Team_data) {
		for (idx = 0; idx < team_datum.num_ship_choices; idx++) {
			if (!strcmp(team_datum.ship_list_variables[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}

			if (!strcmp(team_datum.ship_count_variables[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}
		}

		for (idx = 0; idx < team_datum.num_weapon_choices; idx++) {
			if (!strcmp(team_datum.weaponry_pool_variable[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}
			if (!strcmp(team_datum.weaponry_amount_variable[idx], Sexp_variables[var_index].variable_name)) {
				count++;
			}
		}
	}

	return count;
}

// Count how many tree nodes are container name or container data references
// matching the given container_name.
int SexpTreeModel::get_container_usage_count(const SCP_string& container_name) const
{
	int count = 0;

	for (int node_idx = 0; node_idx < static_cast<int>(tree_nodes.size()); node_idx++) {
		if (is_matching_container_node(node_idx, container_name)) {
			count++;
		}
	}

	return count;
}

// Check if a node is a valid container reference (name or data) matching the given name.
bool SexpTreeModel::is_matching_container_node(int node, const SCP_string& container_name) const
{
	return (tree_nodes[node].type & SEXPT_VALID) &&
		   (tree_nodes[node].type & (SEXPT_CONTAINER_NAME | SEXPT_CONTAINER_DATA)) &&
		   !stricmp(tree_nodes[node].text, container_name.c_str());
}

// Check if the given node's position under its parent expects a container name argument
// (OPF_CONTAINER_NAME, OPF_LIST_CONTAINER_NAME, or OPF_MAP_CONTAINER_NAME).
bool SexpTreeModel::is_container_name_argument(int node) const
{
	Assertion(SCP_vector_inbounds(tree_nodes, node),
		"Attempt to check if out-of-range node %d is a container name argument. Please report!",
		node);

	if (tree_nodes[node].parent == -1) {
		return false;
	}

	const int arg_opf_type = query_node_argument_type(node);
	return SexpTreeOPF::is_container_name_opf_type(arg_opf_type);
}

// -----------------------------------------------------------------------
// Context menu state computation
// -----------------------------------------------------------------------

// Returns true if the given operator should be hidden from context menus.
// Includes operators hidden per GitHub issue #6400 and deprecated operators
// that have been superseded by newer versions.
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

// Analyze the currently selected tree node (item_index) and compute the full
// context menu state: which actions are available (edit, copy, cut, paste, delete),
// which operators are enabled for add/replace/insert, and which variables and
// containers can be used as replacements. The UI layers use this state to build
// their context menus without needing to duplicate any of the enabling logic.
SexpContextMenuState SexpTreeModel::compute_context_menu_state() const
{
	SexpContextMenuState state;

	ctx_init(state);
	if (ctx_handle_labeled_root(state)) {
		return state;
	}

	Assertion(item_index != -1, "Invalid item index");

	state.can_edit_text = (tree_nodes[item_index].flags & EDITABLE) != 0;
	if (tree_nodes[item_index].parent == -1) {
		state.can_delete = false;  // can't delete root
	}

	ctx_compute_variable_menus(state);
	ctx_compute_add_type(state);
	int replace_opf_type = ctx_compute_replace_type(state);
	ctx_compute_insert_type(state);
	ctx_compute_operator_enablement(state);
	ctx_validate_clipboard(state, replace_opf_type);
	ctx_apply_restrictions(state);

	return state;
}

// -----------------------------------------------------------------------
// Private helpers for compute_context_menu_state()
// -----------------------------------------------------------------------

// Set initial context menu state: campaign mode flag, annotation availability,
// and whether the modify-variable option should be enabled.
void SexpTreeModel::ctx_init(SexpContextMenuState& state) const
{
	state.campaign_mode = _interface && _interface->requireCampaignOperators();

	if (_interface && _interface->getFlags()[TreeFlags::AnnotationsAllowed]) {
		state.can_edit_comment = true;
		state.can_edit_bg_color = true;
	}

	state.can_add_variable = true;
	state.can_modify_variable = (sexp_variable_count() > 0);
}

// Handle the labeled-root special case (item_index == -1 in labeled-root mode).
// Sets up a minimal state with only text editing allowed, disables all operator
// menus, and returns true if this special case applies (caller should return early).
bool SexpTreeModel::ctx_handle_labeled_root(SexpContextMenuState& state) const
{
	if (item_index != -1) {
		return false;
	}

	if (!(_interface && _interface->getFlags()[TreeFlags::LabeledRoot])) {
		return false;
	}

	state.is_labeled_root = true;
	state.is_root_editable = _interface && _interface->getFlags()[TreeFlags::RootEditable];
	state.can_edit_text = state.is_root_editable;
	state.can_copy = false;
	state.can_delete = _interface->getFlags()[TreeFlags::RootDeletable];

	int num_ops = static_cast<int>(Operators.size());
	state.op_add_enabled.assign(num_ops, false);
	state.op_replace_enabled.assign(num_ops, false);
	state.op_insert_enabled.assign(num_ops, false);

	return true;
}

// Build the variable and container replacement menu entries for the selected node.
// Determines the expected argument type at this position, then populates
// replace_variables, replace_container_names, and replace_container_data
// with per-entry enabled/disabled state based on type compatibility.
void SexpTreeModel::ctx_compute_variable_menus(SexpContextMenuState& state) const
{
	if (item_index < 0) {
		return;
	}

	int type = tree_nodes[item_index].type;
	int parent = tree_nodes[item_index].parent;
	if (parent < 0) {
		return;
	}

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

	if (!((type & SEXPT_STRING) || (type & SEXPT_NUMBER))) {
		return;
	}

	// Build variable replacement entries
	int max_sexp_vars = MAX_SEXP_VARIABLES;
	Assertion(max_sexp_vars < 512, "Invalid max SEXP variables");

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
	if (SexpTreeOPF::is_container_name_opf_type(op_type) || op_type == OPF_DATA_OR_STR_CONTAINER) {
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

// Determine what can be added as a new child of the selected node.
// Sets add_type (OPR_* return type for operator filtering), populates
// add_data_list with valid data items, and enables can_add_number/can_add_string.
// Handles both container data nodes and regular operator nodes.
void SexpTreeModel::ctx_compute_add_type(SexpContextMenuState& state) const
{
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
			state.add_data_list = _opf.get_container_multidim_modifiers(item_index);
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
		Assertion(op >= 0, "Invalid operator index %d for operator text '%s' (item_index %d)", op, tree_nodes[item_index].text, item_index);

		int type = query_operator_argument_type(op, state.add_count);
		state.add_data_opf_type = type;
		state.add_data_list = _opf.get_listing_opf(type, item_index, state.add_count);
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

		if (state.add_type == OPR_STRING && !SexpTreeOPF::is_container_name_opf_type(type)) {
			state.can_add_string = true;
		}
	}
}

// Determine what can replace the selected node. Sets replace_type, populates
// replace_data_list, enables can_replace_number/can_replace_string, and may
// disable can_delete if this argument can't be removed. Returns the OPF_* type
// for the replace position (needed by ctx_validate_clipboard).
int SexpTreeModel::ctx_compute_replace_type(SexpContextMenuState& state) const
{
	state.replace_type = 0;
	int parent = tree_nodes[item_index].parent;
	int replace_opf_type = 0;

	if (parent < 0) {
		// top node - should be Boolean or Null type
		state.replace_type = _interface ? _interface->getRootReturnType() : OPR_BOOL;
		return replace_opf_type;
	}

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
			replace_list = _opf.get_container_modifiers(parent);
		}
	} else {
		replace_list = _opf.get_listing_opf(type, parent, state.replace_count);
	}

	// special case: don't allow replace data for variable or container names
	if ((type != OPF_VARIABLE_NAME) && !SexpTreeOPF::is_container_name_opf_type(type) && replace_list) {
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

	if (state.replace_type == OPR_STRING && !SexpTreeOPF::is_container_name_opf_type(type)) {
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
					UNREACHABLE("Map container with type %d has unknown key type", static_cast<int>(container.type));
				}
			} else {
				UNREACHABLE("Unknown container type %d", static_cast<int>(container.type));
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

	return replace_opf_type;
}

// Determine the OPF_* argument type for inserting an operator before the selected node.
// The inserted operator must accept the current node's type as its first argument and
// return a type compatible with the parent's expectation at this position.
void SexpTreeModel::ctx_compute_insert_type(SexpContextMenuState& state) const
{
	int z = tree_nodes[item_index].parent;
	Assertion(z >= -1, "Invalid parent node");
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
}

// Pre-compute per-operator enabled state for add/replace/insert menus.
// Seeds from data list operator matches, then enables based on return type
// compatibility. Disables operators without default arguments available
// and filters out non-campaign operators in campaign mode.
void SexpTreeModel::ctx_compute_operator_enablement(SexpContextMenuState& state) const
{
	int parent = tree_nodes[item_index].parent;
	int num_ops = static_cast<int>(Operators.size());
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
		if (!_opf.query_default_argument_available(j)) {
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

// Check if the sexp clipboard contents can be pasted in the current context.
// Validates return type compatibility for operators, data type for numbers/strings,
// and container type for container data. Sets can_paste and can_paste_add accordingly.
void SexpTreeModel::ctx_validate_clipboard(SexpContextMenuState& state, int replace_opf_type)
{
	if (Sexp_clipboard <= -1 || Sexp_nodes[Sexp_clipboard].type == SEXP_NOT_USED) {
		return;
	}

	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST, "Invalid SEXP node subtype");
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
		"Attempt to use container name %s from SEXP clipboard. Please report!",
		Sexp_nodes[Sexp_clipboard].text);

	if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
		int j = get_operator_const(CTEXT(Sexp_clipboard));
		Assertion(j, "Invalid operator");
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
				if (state.replace_type == OPR_STRING && !SexpTreeOPF::is_container_name_opf_type(replace_opf_type))
					state.can_paste = true;
				if (state.add_type == OPR_STRING && !SexpTreeOPF::is_container_name_opf_type(replace_opf_type))
					state.can_paste_add = true;
			} else {
				UNREACHABLE("Unknown container data type %d", static_cast<int>(container.type));
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
		if (state.replace_type == OPR_STRING && !SexpTreeOPF::is_container_name_opf_type(replace_opf_type))
			state.can_paste = true;
		if (state.add_type == OPR_STRING && !SexpTreeOPF::is_container_name_opf_type(replace_opf_type))
			state.can_paste_add = true;

	} else {
		Int3();  // unknown sexp type
	}
}

// Apply final cut/copy/paste restrictions based on node type.
// Container name and modifier nodes cannot be cut or copied; container data
// nodes cannot receive paste-add operations.
void SexpTreeModel::ctx_apply_restrictions(SexpContextMenuState& state) const
{
	state.can_cut = state.can_delete;

	if (tree_nodes[item_index].type & (SEXPT_MODIFIER | SEXPT_CONTAINER_NAME)) {
		state.can_cut = false;
		state.can_copy = false;
		state.can_paste = false;
	}
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		state.can_paste_add = false;
	}
}

// -----------------------------------------------------------------------
// compute_help_text — extract help/mini-help text for a node
// -----------------------------------------------------------------------
// Generate the help box and mini-help box text for the given node.
// For operators: displays the operator's help text from the Sexp_help table.
// For data nodes: shows the relevant argument description from the parent operator's
// help text, and also handles special cases like message text preview, ship flag
// descriptions, and wing flag descriptions.
// Appends any user comment (annotation) to the end of the help text.
SexpTreeModel::HelpTextResult SexpTreeModel::compute_help_text(int node_index, const SCP_string& node_comment) const
{
	HelpTextResult result;

	if (!SCP_vector_inbounds(tree_nodes, node_index) || !tree_nodes[node_index].type) {
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

			Assertion(j >= 0, "Invalid child node");

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
// Validate user-entered text for a tree node. For operator nodes, resolves the
// closest matching valid operator name. For number nodes in OPF_POSITIVE positions,
// rejects negative values. Truncates text to TOKEN_LENGTH.
// Returns a result struct indicating whether the edit should be applied.
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
// Write the validated text into the tree node's text field, truncating to
// TOKEN_LENGTH and sanitizing any invalid characters (Mantis #2893).
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
// Determine the editability flags (OPERAND/EDITABLE/NOT_EDITABLE) and the icon
// (NodeImage) for a tree node based on its type. Operators get the OPERAND flag,
// variables and containers are NOT_EDITABLE, and plain data nodes are EDITABLE
// with a position-based numbered icon.
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
