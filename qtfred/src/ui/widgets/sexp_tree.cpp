/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "sexp_tree.h"
#include "mission/util.h"
#include "mission/Editor.h"
#include "mission/object.h"

#include "parse/sexp.h"
#include "globalincs/linklist.h"
#include "ai/aigoals.h"
#include "mission/missionmessage.h"
#include "mission/missioncampaign.h"
#include "hud/hudsquadmsg.h"
#include "stats/medals.h"
#include "controlconfig/controlsconfig.h"
#include "hud/hudgauges.h"
#include "starfield/starfield.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "jumpnode/jumpnode.h"
#include "gamesnd/eventmusic.h"    // for change-soundtrack
#include "menuui/techmenu.h"    // for intel stuff
#include "weapon/emp.h"
#include "gamesnd/gamesnd.h"
#include "weapon/weapon.h"
#include "hud/hudartillery.h"
#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"
#include "sound/ds.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "mission/missiongoals.h"
#include "ship/ship.h"

#include <ui/util/menu.h>
#include <ui/util/SignalBlockers.h>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QDebug>

#define TREE_NODE_INCREMENT    100

#define MAX_OP_MENUS    30
#define MAX_SUBMENUS    (MAX_OP_MENUS * MAX_OP_MENUS)

extern SCP_vector<game_snd> Snds;

//********************sexp_tree********************

namespace fso {
namespace fred {

namespace {
QString node_image_to_resource_name(NodeImage image) {
	switch (image) {
	case NodeImage::OPERATOR:
		return ":/images/bitmap1.png";
	case NodeImage::DATA:
		return ":/images/data.png";
	case NodeImage::VARIABLE:
		return ":/images/variable.png";
	case NodeImage::ROOT:
		return ":/images/root.png";
	case NodeImage::ROOT_DIRECTIVE:
		return ":/images/root_directive.png";
	case NodeImage::CHAIN:
		return ":/images/chained.png";
	case NodeImage::CHAIN_DIRECTIVE:
		return ":/images/chained_directive.png";
	case NodeImage::GREEN_DOT:
		return ":/images/green_do.png";
	case NodeImage::BLACK_DOT:
		return ":/images/black_do.png";
	case NodeImage::DATA_00:
		return ":/images/data00.png";
	case NodeImage::DATA_05:
		return ":/images/data05.png";
	case NodeImage::DATA_10:
		return ":/images/data10.png";
	case NodeImage::DATA_15:
		return ":/images/data15.png";
	case NodeImage::DATA_20:
		return ":/images/data20.png";
	case NodeImage::DATA_25:
		return ":/images/data25.png";
	case NodeImage::DATA_30:
		return ":/images/data30.png";
	case NodeImage::DATA_35:
		return ":/images/data35.png";
	case NodeImage::DATA_40:
		return ":/images/data40.png";
	case NodeImage::DATA_45:
		return ":/images/data45.png";
	case NodeImage::DATA_50:
		return ":/images/data50.png";
	case NodeImage::DATA_55:
		return ":/images/data55.png";
	case NodeImage::DATA_60:
		return ":/images/data60.png";
	case NodeImage::DATA_65:
		return ":/images/data65.png";
	case NodeImage::DATA_70:
		return ":/images/data70.png";
	case NodeImage::DATA_75:
		return ":/images/data75.png";
	case NodeImage::DATA_80:
		return ":/images/data80.png";
	case NodeImage::DATA_85:
		return ":/images/data85.png";
	case NodeImage::DATA_90:
		return ":/images/data90.png";
	case NodeImage::DATA_95:
		return ":/images/data95.png";
	case NodeImage::COMMENT:
		return ":/images/comment.png";
	case NodeImage::CONTAINER_NAME:
		return ":/images/container_name.png";
	case NodeImage::CONTAINER_DATA:
		return ":/images/container_data.png";
	}
	return ":/images/bitmap1.png";
}
}

SexpTreeEditorInterface::SexpTreeEditorInterface() :
	SexpTreeEditorInterface(flagset<TreeFlags>{ TreeFlags::LabeledRoot, TreeFlags::RootDeletable }) {
}
SexpTreeEditorInterface::SexpTreeEditorInterface(const flagset<TreeFlags>& flags) : _flags(flags) {

}
bool SexpTreeEditorInterface::hasDefaultMessageParamter() {
	return Num_messages > Num_builtin_messages;
}
SCP_vector<SCP_string> SexpTreeEditorInterface::getMessages() {
	SCP_vector<SCP_string> list;

	for (auto i = Num_builtin_messages; i < Num_messages; i++) {
		list.emplace_back(Messages[i].name);
	}

	return list;
}
SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionGoals(const SCP_string&  /*reference_name*/) {
	SCP_vector<SCP_string> list;

	for (const auto &goal: Mission_goals) {
		auto temp_name = goal.name;
		SCP_truncate(temp_name, NAME_LENGTH);
		list.emplace_back(temp_name);
	}

	return list;
}
SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionEvents(const SCP_string&  /*reference_name*/) {
	SCP_vector<SCP_string> list;

	for (const auto &event: Mission_events) {
		auto temp_name = event.name;
		SCP_truncate(temp_name, NAME_LENGTH);
		list.emplace_back(temp_name);
	}

	return list;
}
SCP_vector<SCP_string> SexpTreeEditorInterface::getMissionNames() {
	return { SCP_string(Mission_filename) };
}
bool SexpTreeEditorInterface::hasDefaultMissionName() {
	return *Mission_filename != '\0';
}
bool SexpTreeEditorInterface::hasDefaultGoal(int operator_value) {
	return (operator_value == OP_PREVIOUS_GOAL_TRUE) || (operator_value == OP_PREVIOUS_GOAL_FALSE)
		|| (operator_value == OP_PREVIOUS_GOAL_INCOMPLETE) || !Mission_goals.empty();
}
bool SexpTreeEditorInterface::hasDefaultEvent(int operator_value) {
	return (operator_value == OP_PREVIOUS_EVENT_TRUE) || (operator_value == OP_PREVIOUS_EVENT_FALSE)
		|| (operator_value == OP_PREVIOUS_EVENT_INCOMPLETE) || !Mission_events.empty();

}
const flagset<TreeFlags>& SexpTreeEditorInterface::getFlags() const {
	return _flags;
}
int SexpTreeEditorInterface::getRootReturnType() const {
	return OPR_BOOL;
}
bool SexpTreeEditorInterface::requireCampaignOperators() const {
	return false;
}
SexpTreeEditorInterface::~SexpTreeEditorInterface() = default;

QIcon sexp_tree::convertNodeImageToIcon(NodeImage image) {
	return QIcon(node_image_to_resource_name(image));
}

// constructor
sexp_tree::sexp_tree(QWidget* parent) : QTreeWidget(parent) {
	setSelectionMode(QTreeWidget::SingleSelection);
	setSelectionBehavior(QTreeWidget::SelectItems);

	setContextMenuPolicy(Qt::CustomContextMenu);

	setHeaderHidden(true);

	select_sexp_node = -1;
	root_item = -1;
	clear_tree();

	connect(this, &QWidget::customContextMenuRequested, this, &sexp_tree::customMenuHandler);
	connect(this, &QTreeWidget::itemChanged, this, &sexp_tree::handleItemChange);
	connect(this, &QTreeWidget::itemSelectionChanged, this, &sexp_tree::handleNewItemSelected);
}

sexp_tree::~sexp_tree() = default;

// clears out the tree, so all the nodes are unused.
void sexp_tree::clear_tree(const char* op) {
	mprintf(("Resetting dynamic tree node limit from "
				SIZE_T_ARG
				" to %d...\n", tree_nodes.size(), 0));

	total_nodes = flag = 0;
	tree_nodes.clear();

	if (op) {
		clear();
		if (strlen(op)) {
			set_node(allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
			build_tree();
		}
	}
}

void sexp_tree::reset_handles() {
	uint i;

	for (i = 0; i < tree_nodes.size(); i++) {
		tree_nodes[i].handle = NULL;
	}
}

// initializes and creates a tree from a given sexp startpoint.
void sexp_tree::load_tree(int index, const char* deflt) {
	int cur;

	clear_tree();
	root_item = 0;
	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), deflt);  // setup a default tree if none
		build_tree();
		return;
	}

	if (Sexp_nodes[index].subtype == SEXP_ATOM_NUMBER) {  // handle numbers allender likes to use so much..
		cur = allocate_node(-1);
		if (atoi(Sexp_nodes[index].text)) {
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "true");
		} else {
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "false");
		}

		build_tree();
		return;
	}

	// assumption: first token is an operator.  I require this because it would cause problems
	// with child/parent relations otherwise, and it should be this way anyway, since the
	// return type of the whole sexp is boolean, and only operators can satisfy this.
	Assert(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR);
	load_branch(index, -1);
	build_tree();
}

void get_combined_variable_name(char* combined_name, const char* sexp_var_name) {
	int sexp_var_index = get_index_sexp_variable_name(sexp_var_name);

	if (sexp_var_index >= 0)
		sprintf(combined_name, "%s(%s)", Sexp_variables[sexp_var_index].variable_name, Sexp_variables[sexp_var_index].text);
	else
		sprintf(combined_name, "%s(undefined)", sexp_var_name);
}

// creates a tree from a given Sexp_nodes[] point under a given parent.  Recursive.
// Returns the allocated current node.
int sexp_tree::load_branch(int index, int parent) {
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
			load_branch(Sexp_nodes[index].first, parent);  // do the sublist and continue

		} else if (Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR) {
			cur = allocate_node(parent);
			if ((index == select_sexp_node) && !flag) {  // translate sexp node to our node
				select_sexp_node = cur;
				flag = 1;
			}

			set_node(cur, (SEXPT_OPERATOR | additional_flags), Sexp_nodes[index].text);
			load_branch(Sexp_nodes[index].rest, cur);  // operator is new parent now
			return cur;  // 'rest' was just used, so nothing left to use.

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
			}  else {
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
				"Attempt to load unknown container %s into SEXP tree. Please report!",
				Sexp_nodes[index].text);
			set_node(cur, (SEXPT_CONTAINER_DATA | SEXPT_STRING | additional_flags), Sexp_nodes[index].text);
			load_branch(Sexp_nodes[index].first, cur);  // container is new parent now

		} else
			Assert(0);  // unknown and/or invalid sexp type

		if ((index == select_sexp_node) && !flag) {  // translate sexp node to our node
			select_sexp_node = cur;
			flag = 1;
		}

		index = Sexp_nodes[index].rest;
		if (index == -1) {
			return cur;
		}
	}

	return cur;
}

int sexp_tree::query_false(int node) {
	if (node < 0) {
		node = root_item;
	}

	Assert(node >= 0);
	Assert(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID));
	Assert(tree_nodes[node].next == -1);  // must make this assumption or else it will confuse code!
	if (get_operator_const(tree_nodes[node].text) == OP_FALSE) {
		return TRUE;
	}

	return FALSE;
}

// builds an sexp of the tree and returns the index of it.  This allocates sexp nodes.
int sexp_tree::save_tree(int node) {
	if (node < 0) {
		node = root_item;
	}

	Assert(node >= 0);
	Assert(tree_nodes[node].type == (SEXPT_OPERATOR | SEXPT_VALID));
	Assert(tree_nodes[node].next == -1);  // must make this assumption or else it will confuse code!
	return save_branch(node);
}

// get variable name from sexp_tree node .text
void var_name_from_sexp_tree_text(char* var_name, const char* text) {
	auto var_name_length = strcspn(text, "(");
	Assert(var_name_length < TOKEN_LENGTH - 1);

	strncpy(var_name, text, var_name_length);
	var_name[var_name_length] = '\0';
}

#define NO_PREVIOUS_NODE -9
// called recursively to save a tree branch and everything under it
// SEXPT_CONTAINER_NAME and SEXPT_MODIFIER require no special handling here
int sexp_tree::save_branch(int cur, int at_root) {
	int start, node = -1, last = NO_PREVIOUS_NODE;
	char var_name_text[TOKEN_LENGTH];

	start = -1;
	while (cur != -1) {
		if (tree_nodes[cur].type & SEXPT_OPERATOR) {
			node =
				alloc_sexp(tree_nodes[cur].text, SEXP_ATOM, SEXP_ATOM_OPERATOR, -1, save_branch(tree_nodes[cur].child));

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

// find the next free tree node and return its index.
int sexp_tree::find_free_node() {
	int i;

	for (i = 0; i < (int) tree_nodes.size(); i++) {
		if (tree_nodes[i].type == SEXPT_UNUSED) {
			return i;
		}
	}

	return -1;
}

// allocate a node.  Remains used until freed.
int sexp_tree::allocate_node() {
	int node = find_free_node();

	// need more tree nodes?
	if (node < 0) {
		int old_size = (int) tree_nodes.size();

		Assert(TREE_NODE_INCREMENT > 0);

		// allocate in blocks of TREE_NODE_INCREMENT
		tree_nodes.resize(tree_nodes.size() + TREE_NODE_INCREMENT);

		mprintf(("Bumping dynamic tree node limit from %d to "
					SIZE_T_ARG
					"...\n", old_size, tree_nodes.size()));

#ifndef NDEBUG
		for (int i = old_size; i < (int) tree_nodes.size(); i++) {
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
	tree_nodes[node].handle = NULL;

	total_nodes++;
	return node;
}

// allocate a child node under 'parent'.  Appends to end of list.
int sexp_tree::allocate_node(int parent, int after) {
	int i, index = allocate_node();

	if (parent != -1) {
		i = tree_nodes[parent].child;
		if (i == -1) {
			tree_nodes[parent].child = index;

		} else {
			while ((i != after) && (tree_nodes[i].next != -1)) {
				i = tree_nodes[i].next;
			}

			tree_nodes[index].next = tree_nodes[i].next;
			tree_nodes[i].next = index;
		}
	}

	tree_nodes[index].parent = parent;
	return index;
}

// free a node and all its children.  Also clears pointers to it, if any.
//   node = node chain to free
//   cascade =  0: free just this node and children under it. (default)
//             !0: free this node and all siblings after it.
//
void sexp_tree::free_node(int node, int cascade) {
	int i;

	// clear the pointer to node
	i = tree_nodes[node].parent;
	Assert(i != -1);
	if (tree_nodes[i].child == node) {
		tree_nodes[i].child = tree_nodes[node].next;
	} else {
		i = tree_nodes[i].child;
		while (tree_nodes[i].next != -1) {
			if (tree_nodes[i].next == node) {
				tree_nodes[i].next = tree_nodes[node].next;
				break;
			}

			i = tree_nodes[i].next;
		}
	}

	if (!cascade) {
		tree_nodes[node].next = -1;
	}

	// now free up the node and its children
	free_node2(node);
}

// more simple node freer, which works recursively.  It frees the given node and all siblings
// that come after it, as well as all children of these.  Doesn't clear any links to any of
// these freed nodes, so make sure all links are broken first. (i.e. use free_node() if you can)
//
void sexp_tree::free_node2(int node) {
	Assert(node != -1);
	Assert(tree_nodes[node].type != SEXPT_UNUSED);
	Assert(total_nodes > 0);
	modified();
	tree_nodes[node].type = SEXPT_UNUSED;
	total_nodes--;
	if (tree_nodes[node].child != -1) {
		free_node2(tree_nodes[node].child);
	}

	if (tree_nodes[node].next != -1) {
		free_node2(tree_nodes[node].next);
	}
}

// initialize the data for a node.  Should be called right after a new node is allocated.
void sexp_tree::set_node(int node, int type, const char* text) {
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

void sexp_tree::post_load() {
	if (!flag) {
		select_sexp_node = -1;
	}
}

// build or rebuild a CTreeCtrl object with the current tree data
void sexp_tree::build_tree() {
	if (!flag) {
		select_sexp_node = -1;
	}

	clear();
	add_sub_tree(0, nullptr);
}

// Create the CTreeCtrl tree from the tree data.  The tree data should already be setup by
// this point.
void sexp_tree::add_sub_tree(int node, QTreeWidgetItem* root) {
//	char str[80];
	int node2;

	Assert(node >= 0 && node < (int) tree_nodes.size());
	node2 = tree_nodes[node].child;

	// check for single argument operator case (prints as one line)
/*	if (node2 != -1 && tree_nodes[node2].child == -1 && tree_nodes[node2].next == -1) {
		sprintf(str, "%s %s", tree_nodes[node].text, tree_nodes[node2].text);
		tree_nodes[node].handle = insert(str, root);
		tree_nodes[node].flags = OPERAND | EDITABLE;
		tree_nodes[node2].flags = COMBINED;
		return;
	}*/

	// bitmap to draw in tree
	NodeImage bitmap;

	if (tree_nodes[node].type & SEXPT_OPERATOR) {
		tree_nodes[node].flags = OPERAND;
		bitmap = NodeImage::OPERATOR;
	} else {
		if (tree_nodes[node].type & SEXPT_VARIABLE) {
			tree_nodes[node].flags = NOT_EDITABLE;
			bitmap = NodeImage::VARIABLE;
		} else if (tree_nodes[node].type & SEXPT_CONTAINER_NAME) {
			tree_nodes[node].flags = NOT_EDITABLE;
			bitmap = NodeImage::CONTAINER_NAME;
		} else if (tree_nodes[node].type & SEXPT_CONTAINER_DATA) {
			tree_nodes[node].flags = NOT_EDITABLE;
			bitmap = NodeImage::CONTAINER_DATA;
		} else {
			tree_nodes[node].flags = EDITABLE;
			bitmap = get_data_image(node);
		}
	}

	root = tree_nodes[node].handle = insert(tree_nodes[node].text, bitmap, root);

	tree_nodes[node].handle->setFlags(
		tree_nodes[node].handle->flags().setFlag(Qt::ItemIsEditable, (tree_nodes[node].flags & EDITABLE)));

	node = node2;
	while (node != -1) {
		Assert(node >= 0 && node < (int) tree_nodes.size());
		Assert(tree_nodes[node].type & SEXPT_VALID);
		if (tree_nodes[node].type & (SEXPT_OPERATOR | SEXPT_CONTAINER_DATA)) {
			add_sub_tree(node, root);

		} else {
			Assert(tree_nodes[node].child == -1);
			if (tree_nodes[node].type & SEXPT_VARIABLE) {
				tree_nodes[node].handle = insert(tree_nodes[node].text, NodeImage::VARIABLE, root);
				tree_nodes[node].flags = NOT_EDITABLE;
			} else if (tree_nodes[node].type & SEXPT_CONTAINER_NAME) {
				tree_nodes[node].handle = insert(tree_nodes[node].text, NodeImage::CONTAINER_NAME, root);
				tree_nodes[node].flags = NOT_EDITABLE;
			// SEXPT_MODIFIER doesn't require special treatment here
			} else {
				auto bmap = get_data_image(node);
				tree_nodes[node].handle = insert(tree_nodes[node].text, bmap, root);
				tree_nodes[node].flags = EDITABLE;
			}

			tree_nodes[node].handle->setFlags(
				tree_nodes[node].handle->flags().setFlag(Qt::ItemIsEditable, (tree_nodes[node].flags & EDITABLE)));
		}

		node = tree_nodes[node].next;
	}
}

// construct tree nodes for an sexp, adding them to the list and returning first node
int sexp_tree::load_sub_tree(int index, bool valid, const char* text) {
	int cur;

	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | (valid ? SEXPT_VALID : 0)), text);  // setup a default tree if none
		return cur;
	}

	// assumption: first token is an operator.  I require this because it would cause problems
	// with child/parent relations otherwise, and it should be this way anyway, since the
	// return type of the whole sexp is boolean, and only operators can satisfy this.
	Assert(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR);
	cur = load_branch(index, -1);
	return cur;
}

// counts the number of arguments an operator has.  Call this with the node of the first
// argument of the operator
int sexp_tree::count_args(int node) {
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
int sexp_tree::identify_arg_type(int node) {
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
int sexp_tree::query_node_argument_type(int node) const {
	int parent_node = tree_nodes[node].parent;
	Assert(parent_node >= 0);
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

//
// See https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++
//
template<typename T>
typename T::size_type GeneralizedLevensteinDistance(const T &source,
	const T &target,
	typename T::size_type insert_cost = 1,
	typename T::size_type delete_cost = 1,
	typename T::size_type replace_cost = 1) {
	if (source.size() > target.size()) {
		return GeneralizedLevensteinDistance(target, source, delete_cost, insert_cost, replace_cost);
	}

	using TSizeType = typename T::size_type;
	const TSizeType min_size = source.size(), max_size = target.size();
	std::vector<TSizeType> lev_dist(min_size + 1);

	lev_dist[0] = 0;
	for (TSizeType i = 1; i <= min_size; ++i) {
		lev_dist[i] = lev_dist[i - 1] + delete_cost;
	}

	for (TSizeType j = 1; j <= max_size; ++j) {
		TSizeType previous_diagonal = lev_dist[0], previous_diagonal_save;
		lev_dist[0] += insert_cost;

		for (TSizeType i = 1; i <= min_size; ++i) {
			previous_diagonal_save = lev_dist[i];
			if (source[i - 1] == target[j - 1]) {
				lev_dist[i] = previous_diagonal;
			}
			else {
				lev_dist[i] = std::min(std::min(lev_dist[i - 1] + delete_cost, lev_dist[i] + insert_cost), previous_diagonal + replace_cost);
			}
			previous_diagonal = previous_diagonal_save;
		}
	}

	return lev_dist[min_size];
}

// Look for the valid operator that is the closest match for 'str' and return the operator
// number of it.  What operators are valid is determined by 'node', and an operator is valid
// if it is allowed to fit at position 'node'
//
SCP_string sexp_tree::match_closest_operator(const SCP_string &str, int node) {
	int z, i, op, arg_num, opf, opr;
	int min = -1, best = -1;

	z = tree_nodes[node].parent;
	if (z < 0) {
		return str;
	}

	op = get_operator_index(tree_nodes[z].text);
	if (op < 0) {
		return str;
	}

	// determine which argument we are of the parent
	arg_num = find_argument_number(z, node);
	opf = query_operator_argument_type(op, arg_num);	// check argument type at this position

	for (i = 0; i < (int) Operators.size(); i++) {
		opr = query_operator_return_type(i);			// figure out which type this operator returns

		if (sexp_query_type_match(opf, opr)) {
			int dist = (int)GeneralizedLevensteinDistance(str, Operators[i].text, 2, 2, 3);
			if (min < 0 || dist < min) {
				min = dist;
				best = i;
			}
		}
	}

	Assert(best >= 0);  // we better have some valid operator at this point.
	return Operators[best].text;
}

// adds to or replaces (based on passed in flag) the current operator
void sexp_tree::add_or_replace_operator(int op, int replace_flag) {
	int i, op2;

	if (replace_flag) {
		if (tree_nodes[item_index].flags & OPERAND) {  // are both operators?
			op2 = get_operator_index(tree_nodes[item_index].text);
			Assert(op2 >= 0);
			i = count_args(tree_nodes[item_index].child);
			if ((i >= Operators[op].min) && (i <= Operators[op].max)) {  // are old num args valid?
				while (i--) {
					if (query_operator_argument_type(op2, i)
						!= query_operator_argument_type(op, i)) {  // does each arg match expected type?
						break;
					}
				}

				if (i < 0) {  // everything is ok, so we can keep old arguments with new operator
					set_node(item_index, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
					tree_nodes[item_index].handle->setText(0, QString::fromStdString(Operators[op].text));
					tree_nodes[item_index].flags = OPERAND;
					return;
				}
			}
		}

		replace_operator(Operators[op].text.c_str());

	} else {
		add_operator(Operators[op].text.c_str());
	}

	// fill in all the required (minimum) arguments with default values
	for (i = 0; i < Operators[op].min; i++) {
		add_default_operator(op, i);
	}
}

// initialize node, type operator
//
void sexp_list_item::set_op(int op_num) {
	int i;

	if (op_num >= FIRST_OP) {  // do we have an op value instead of an op number (index)?
		for (i = 0; i < (int) Operators.size(); i++) {
			if (op_num == Operators[i].value) {
				op_num = i;
			}
		}  // convert op value to op number
	}

	op = op_num;
	text = Operators[op].text;
	type = (SEXPT_OPERATOR | SEXPT_VALID);
}

// initialize node, type data
// Defaults: t = SEXPT_STRING
//
void sexp_list_item::set_data(const char* str, int t) {
	op = -1;
	text = str;
	type = t;
}

// add a node to end of list
//
void sexp_list_item::add_op(int op_num) {
	sexp_list_item* item, * ptr;

	item = new sexp_list_item;
	ptr = this;
	while (ptr->next) {
		ptr = ptr->next;
	}

	ptr->next = item;
	item->set_op(op_num);
}

// add a node to end of list
// Defaults: t = SEXPT_STRING
//
void sexp_list_item::add_data(const char* str, int t) {
	sexp_list_item* item, * ptr;

	item = new sexp_list_item;
	ptr = this;
	while (ptr->next) {
		ptr = ptr->next;
	}

	ptr->next = item;
	item->set_data(str, t);
}

// add an sexp list to end of another list (join lists)
//
void sexp_list_item::add_list(sexp_list_item* list) {
	sexp_list_item* ptr;

	ptr = this;
	while (ptr->next) {
		ptr = ptr->next;
	}

	ptr->next = list;
}

// free all nodes of list
//
void sexp_list_item::destroy() {
	sexp_list_item* ptr, * ptr2;

	ptr = this;
	while (ptr) {
		ptr2 = ptr->next;

		delete ptr;
		ptr = ptr2;
	}
}

int sexp_tree::add_default_operator(int op_index, int argnum) {
	char buf[256];
	int index;
	sexp_list_item item;

	index = item_index;
	if (get_default_value(&item, buf, op_index, argnum)) {
		return -1;
	}

	if (item.type & SEXPT_OPERATOR) {
		Assert((item.op >= 0) && (item.op < (int) Operators.size()));
		add_or_replace_operator(item.op);
		item_index = index;
	} else {
		// special case for sexps that take variables
		const int op_type = query_operator_argument_type(op_index, argnum);
		if (op_type == OPF_VARIABLE_NAME) {
			int sexp_var_index = get_index_sexp_variable_name(item.text);
			Assert(sexp_var_index != -1);
			int type = SEXPT_VALID | SEXPT_VARIABLE;
			if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
				type |= SEXPT_STRING;
			} else if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
				type |= SEXPT_NUMBER;
			} else {
				Int3();
			}

			char node_text[2 * TOKEN_LENGTH + 2];
			sprintf(node_text, "%s(%s)", item.text.c_str(), Sexp_variables[sexp_var_index].text);
			add_variable_data(node_text, type);
		}
		else if (item.type & SEXPT_CONTAINER_NAME) {
			Assertion(is_container_name_opf_type(op_type) || op_type == OPF_DATA_OR_STR_CONTAINER,
				"Attempt to add default container name for a node of non-container type (%d). Please report!",
				op_type);
			add_container_name(item.text.c_str());
		}
			// modify-variable data type depends on type of variable being modified
			// (we know this block is handling the second argument since it's not OPF_VARIABLE_NAME)
		else if (Operators[op_index].value == OP_MODIFY_VARIABLE) {
			// the the variable name
			char buf2[256];
			Assert(argnum == 1);
			sexp_list_item temp_item;
			get_default_value(&temp_item, buf2, op_index, 0);
			int sexp_var_index = get_index_sexp_variable_name(temp_item.text);
			Assert(sexp_var_index != -1);

			// from name get type
			int temp_type = Sexp_variables[sexp_var_index].type;
			int type = 0;
			if (temp_type & SEXP_VARIABLE_NUMBER) {
				type = SEXPT_VALID | SEXPT_NUMBER;
			} else if (temp_type & SEXP_VARIABLE_STRING) {
				type = SEXPT_VALID | SEXPT_STRING;
			} else {
				Int3();
			}
			add_data(item.text.c_str(), type);
		}
			// all other sexps and parameters
		else {
			add_data(item.text.c_str(), item.type);
		}
	}

	return 0;
}

int sexp_tree::get_default_value(sexp_list_item* item, char* text_buf, int op, int i) {
	const char* str = NULL;
	int type, index;
	sexp_list_item* list;

	index = item_index;
	type = query_operator_argument_type(op, i);
	switch (type) {
	case OPF_NULL:
		item->set_op(OP_NOP);
		return 0;

	case OPF_BOOL:
		item->set_op(OP_TRUE);
		return 0;

	case OPF_ANYTHING:
		if (Operators[op].value == OP_INVALIDATE_ARGUMENT || Operators[op].value == OP_VALIDATE_ARGUMENT)
			item->set_data(SEXP_ARGUMENT_STRING);	// this is almost always what you want for these sexps
		else
			item->set_data("<any data>");
		return 0;

	case OPF_DATA_OR_STR_CONTAINER:
		item->set_data("<any data or string container>");
		return 0;

	case OPF_NUMBER:
	case OPF_POSITIVE:
	case OPF_AMBIGUOUS:
		// if the top level operators is an AI goal, and we are adding the last number required,
		// assume that this number is a priority and make it 89 instead of 1.
		if ((query_operator_return_type(op) == OPR_AI_GOAL) && (i == (Operators[op].min - 1))) {
			item->set_data("89", (SEXPT_NUMBER | SEXPT_VALID));
		} else if (((Operators[op].value == OP_HAS_DOCKED_DELAY) || (Operators[op].value == OP_HAS_UNDOCKED_DELAY)
			|| (Operators[op].value == OP_TIME_DOCKED) || (Operators[op].value == OP_TIME_UNDOCKED)) && (i == 2)) {
			item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
		} else if ((Operators[op].value == OP_SHIP_TYPE_DESTROYED) || (Operators[op].value == OP_GOOD_SECONDARY_TIME)) {
			item->set_data("100", (SEXPT_NUMBER | SEXPT_VALID));
		} else if (Operators[op].value == OP_SET_SUPPORT_SHIP) {
			item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
		} else if (((Operators[op].value == OP_SHIP_TAG) && (i == 1))
			|| ((Operators[op].value == OP_TRIGGER_SUBMODEL_ANIMATION) && (i == 3))) {
			item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
		} else if (Operators[op].value == OP_EXPLOSION_EFFECT) {
			int temp;
			char sexp_str_token[TOKEN_LENGTH];

			switch (i) {
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
				temp = (int) EMP_DEFAULT_INTENSITY;
				break;
			case 12:
				temp = (int) EMP_DEFAULT_TIME;
				break;
			default:
				temp = 0;
				break;
			}

			sprintf(sexp_str_token, "%d", temp);
			item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
		} else if (Operators[op].value == OP_WARP_EFFECT) {
			int temp;
			char sexp_str_token[TOKEN_LENGTH];

			switch (i) {
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
		} else if (Operators[op].value == OP_CHANGE_BACKGROUND) {
			item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
		} else if (Operators[op].value == OP_ADD_BACKGROUND_BITMAP || Operators[op].value == OP_ADD_BACKGROUND_BITMAP_NEW) {
			int temp = 0;
			char sexp_str_token[TOKEN_LENGTH];

			switch (i) {
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
		} else if (Operators[op].value == OP_ADD_SUN_BITMAP || Operators[op].value == OP_ADD_SUN_BITMAP_NEW) {
			int temp = 0;
			char sexp_str_token[TOKEN_LENGTH];

			if (i == 4) {
				temp = 100;
			}

			sprintf(sexp_str_token, "%d", temp);
			item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
		} else if (Operators[op].value == OP_MISSION_SET_NEBULA) {
			if (i == 0) {
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			} else {
				item->set_data("3000", (SEXPT_NUMBER | SEXPT_VALID));
			}
		} else if (Operators[op].value == OP_MODIFY_VARIABLE) {
			if (get_modify_variable_type(index) == OPF_NUMBER) {
				item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
			} else {
				item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
			}
		} else if (Operators[op].value == OP_MODIFY_VARIABLE_XSTR) {
			if (i == 1) {
				item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
			} else {
				item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
		} else if (Operators[op].value == OP_SET_VARIABLE_BY_INDEX) {
			if (i == 0) {
				item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
			} else {
				item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
			}
		} else if (Operators[op].value == OP_JETTISON_CARGO_NEW) {
			item->set_data("25", (SEXPT_NUMBER | SEXPT_VALID));
		} else if (Operators[op].value == OP_TECH_ADD_INTEL_XSTR || Operators[op].value == OP_TECH_REMOVE_INTEL_XSTR) {
			item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
		} else {
			item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
		}

		return 0;

		// Goober5000 - special cases that used to be numbers but are now hybrids
		case OPF_GAME_SND:
		{
			gamesnd_id sound_index;

			if (Operators[op].value == OP_EXPLOSION_EFFECT)
			{
				sound_index = GameSounds::SHIP_EXPLODE_1;
			}
			else if (Operators[op].value == OP_WARP_EFFECT)
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
				char *unique_id = Fireball_info[fireball_index].unique_id;
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
	if (list && list->text == SEXP_ARGUMENT_STRING) {
		sexp_list_item* first_ptr;

		first_ptr = list;
		list = list->next;

		delete first_ptr;
	}

	if (list) {
		// copy the information from the list to the passed-in item
		*item = *list;

		// but use the provided text buffer
		strcpy(text_buf, list->text.c_str());
		item->text = text_buf;

		// get rid of the list, since we're done with it
		list->destroy();
		item->next = NULL;

		return 0;
	}

	// catch anything that doesn't have a default value.  Just describe what should be here instead
	switch (type) {
	case OPF_SHIP:
	case OPF_SHIP_NOT_PLAYER:
	case OPF_SHIP_POINT:
	case OPF_SHIP_WING:
	case OPF_SHIP_WING_WHOLETEAM:
	case OPF_SHIP_WING_SHIPONTEAM_POINT:
	case OPF_SHIP_WING_POINT:
		str = "<name of ship here>";
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
		//str = "<any allied>";
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
		str = const_cast<char*>(font::FontManager::getFont(0)->getName().c_str());
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

	default:
		str = "<new default required!>";
		break;
	}

	item->set_data(str, (SEXPT_STRING | SEXPT_VALID));
	return 0;
}

int sexp_tree::query_default_argument_available(int op) {
	int i;

	Assert(op >= 0);
	for (i = 0; i < Operators[op].min; i++) {
		if (!query_default_argument_available(op, i)) {
			return 0;
		}
	}

	return 1;
}

int sexp_tree::query_default_argument_available(int op, int i) {
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
		return 1;

	case OPF_SHIP:
	case OPF_SHIP_WING:
	case OPF_SHIP_POINT:
	case OPF_SHIP_WING_POINT:
	case OPF_SHIP_WING_WHOLETEAM:
	case OPF_SHIP_WING_SHIPONTEAM_POINT:
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
				return 1;
			}

			ptr = GET_NEXT(ptr);
		}

		return 0;

	case OPF_SHIP_NOT_PLAYER:
	case OPF_ORDER_RECIPIENT:
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_SHIP) {
				return 1;
			}

			ptr = GET_NEXT(ptr);
		}

		return 0;

	case OPF_WING:
		for (j = 0; j < MAX_WINGS; j++) {
			if (Wings[j].wave_count) {
				return 1;
			}
		}

		return 0;

	case OPF_PERSONA:
		return (Num_personas > 0) ? 1 : 0;

	case OPF_POINT:
	case OPF_WAYPOINT_PATH:
		return Waypoint_lists.empty() ? 0 : 1;

	case OPF_MISSION_NAME:
		return _interface->hasDefaultMissionName() ? 1 : 0;

		// The following code is kept for when the campaign editor is implemented
		/*
		if (m_mode != MODE_CAMPAIGN) {
			if (!(*Mission_filename))
				return 0;

			return 1;
		}

		if (Campaign.num_missions > 0)
			return 1;

		return 0;
		 */

	case OPF_GOAL_NAME: {
		return _interface->hasDefaultGoal(Operators[op].value) ? 1 : 0;

		// The original code is kept until the campaign editor is implemented
		/*
		int value;

		value = Operators[op].value;

		if (m_mode == MODE_CAMPAIGN) {
			return 1;

			// need to be sure that previous-goal functions are available.  (i.e. we are providing a default argument for them)
		} else if ((value == OP_PREVIOUS_GOAL_TRUE) || (value == OP_PREVIOUS_GOAL_FALSE)
			|| (value == OP_PREVIOUS_GOAL_INCOMPLETE) || !Mission_goals.empty()) {
				return 1;
		}

		return 0;
		 */
	}

	case OPF_EVENT_NAME: {
		return _interface->hasDefaultEvent(Operators[op].value) ? 1 : 0;

		// The original code is kept until the campaign editor is implemented
		/*
		int value;

		value = Operators[op].value;
		if (m_mode == MODE_CAMPAIGN) {
			return 1;

			// need to be sure that previous-event functions are available.  (i.e. we are providing a default argument for them)
		} else if ((value == OP_PREVIOUS_EVENT_TRUE) || (value == OP_PREVIOUS_EVENT_FALSE)
			|| (value == OP_PREVIOUS_EVENT_INCOMPLETE) || !Mission_events.empty()) {
			return 1;
		}

		return 0;
		 */
	}

	case OPF_MESSAGE:
		return _interface->hasDefaultMessageParamter() ? 1 : 0;

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

// expand a combined line (one with an operator and its one argument on the same line) into
// 2 lines.
void sexp_tree::expand_operator(int node) {
	if (tree_nodes[node].flags & COMBINED) {
		node = tree_nodes[node].parent;
		Assert((tree_nodes[node].flags & OPERAND) && (tree_nodes[node].flags & EDITABLE));
	}

	if ((tree_nodes[node].flags & OPERAND) && (tree_nodes[node].flags & EDITABLE)) {  // expandable?
		Assert(tree_nodes[node].type & SEXPT_OPERATOR);
		auto h = tree_nodes[node].handle;
		auto child_data = tree_nodes[node].child;
		Assert(child_data != -1 && tree_nodes[child_data].next == -1 && tree_nodes[child_data].child == -1);

		h->setText(0, QString::fromUtf8(tree_nodes[node].text));
		tree_nodes[node].flags = OPERAND;
		auto bmap = get_data_image(child_data);
		tree_nodes[child_data].handle = insert(tree_nodes[child_data].text, bmap, h);
		tree_nodes[child_data].flags = EDITABLE;
		h->setExpanded(true);
	}
}

// expand a CTreeCtrl branch and all of its children
void sexp_tree::expand_branch(QTreeWidgetItem* h) {
	h->setExpanded(true);
	for (auto i = 0; i < h->childCount(); ++i) {
		expand_branch(h->child(i));
	}
}

void sexp_tree::merge_operator(int  /*node*/) {
/*	char buf[256];
	int child;

	if (tree_nodes[node].flags == EDITABLE)  // data
		node = tree_nodes[node].parent;

	if (node != -1) {
		child = tree_nodes[node].child;
		if (child != -1 && tree_nodes[child].next == -1 && tree_nodes[child].child == -1) {
			sprintf(buf, "%s %s", tree_nodes[node].text, tree_nodes[child].text);
			SetItemText(tree_nodes[node].handle, buf);
			tree_nodes[node].flags = OPERAND | EDITABLE;
			tree_nodes[child].flags = COMBINED;
			DeleteItem(tree_nodes[child].handle);
			tree_nodes[child].handle = NULL;
			return;
		}
	}*/
}

// add a data node under operator pointed to by item_index
int sexp_tree::add_data(const char* new_data, int type) {
	int node;

	expand_operator(item_index);
	node = allocate_node(item_index);
	set_node(node, type, new_data);
	auto bmap = get_data_image(node);
	tree_nodes[node].handle = insert(new_data, bmap, tree_nodes[item_index].handle);
	tree_nodes[node].flags = EDITABLE;
	modified();
	return node;
}

// add a (variable) data node under operator pointed to by item_index
int sexp_tree::add_variable_data(const char* new_data, int type) {
	int node;

	Assert(type & SEXPT_VARIABLE);

	expand_operator(item_index);
	node = allocate_node(item_index);
	set_node(node, type, new_data);
	tree_nodes[node].handle = insert(new_data, NodeImage::VARIABLE, tree_nodes[item_index].handle);
	tree_nodes[node].handle->setFlags(tree_nodes[node].handle->flags().setFlag(Qt::ItemIsEditable, false));
	tree_nodes[node].flags = NOT_EDITABLE;
	modified();
	return node;
}

// add a container name node under operator pointed to by item_index
int sexp_tree::add_container_name(const char* container_name)
{
	Assertion(container_name != nullptr, "Attempt to add null container name. Please report!");
	Assertion(get_sexp_container(container_name) != nullptr,
		"Attempt to add unknown container name %s. Please report!",
		container_name);

	expand_operator(item_index);
	int node = allocate_node(item_index);
	set_node(node, (SEXPT_VALID | SEXPT_CONTAINER_NAME | SEXPT_STRING), container_name);
	tree_nodes[node].handle = insert(container_name, NodeImage::CONTAINER_NAME, tree_nodes[item_index].handle);
	tree_nodes[node].handle->setFlags(tree_nodes[node].handle->flags().setFlag(Qt::ItemIsEditable, false));
	tree_nodes[node].flags = NOT_EDITABLE;
	modified();
	return node;
}

// add a (container) data node under operator pointed to by item_index
void sexp_tree::add_container_data(const char* container_name)
{
	Assertion(container_name != nullptr, "Attempt to add null container. Please report!");
	Assertion(get_sexp_container(container_name) != nullptr,
		"Attempt to add unknown container %s. Please report!",
		container_name);
	const int node = allocate_node(item_index);
	set_node(node, (SEXPT_VALID | SEXPT_CONTAINER_DATA | SEXPT_STRING), container_name);
	tree_nodes[node].handle = insert(container_name, NodeImage::CONTAINER_DATA, tree_nodes[item_index].handle);
	tree_nodes[node].handle->setFlags(tree_nodes[node].handle->flags().setFlag(Qt::ItemIsEditable, false));
	tree_nodes[node].flags = NOT_EDITABLE;
	item_index = node;
	modified();
}

// add an operator under operator pointed to by item_index.  Updates item_index to point
// to this new operator.
int sexp_tree::add_operator(const char* op, QTreeWidgetItem* h) {
	int node;

	if (item_index == -1) {
		node = allocate_node(-1);
		set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), op);
		tree_nodes[node].handle = insert(op, NodeImage::OPERATOR, h);
	} else {
		expand_operator(item_index);
		node = allocate_node(item_index);
		set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), op);
		tree_nodes[node].handle = insert(op, NodeImage::OPERATOR, tree_nodes[item_index].handle);
	}

	tree_nodes[node].flags = OPERAND;
	setCurrentItemIndex(node);
	modified();

	return node;
}

// add an operator with one argument under operator pointed to by item_index.  This function
// exists because the one arg case is a special case.  The operator and argument is
// displayed on the same line.
/*void sexp_tree::add_one_arg_operator(char *op, char *data, int type)
{
	char str[80];
	int node1, node2;

	expand_operator(item_index);
	node1 = allocate_node(item_index);
	node2 = allocate_node(node1);
	set_node(node1, SEXPT_OPERATOR, op);
	set_node(node2, type, data);
	sprintf(str, "%s %s", op, data);
	tree_nodes[node1].handle = insert(str, tree_nodes[item_index].handle);
	tree_nodes[node1].flags = OPERAND | EDITABLE;
	tree_nodes[node2].flags = COMBINED;
	*modified = 1;
}*/

/*
int sexp_tree::verify_tree(int *bypass)
{
	return verify_tree(0, bypass);
}

// check the sexp tree for errors.  Return -1 if error, or 0 if no errors.  If an error
// is found, item_index = node of error.
int sexp_tree::verify_tree(int node, int *bypass)
{
	int i, type, count, op, type2, op2, argnum = 0;

	if (!total_nodes)
		return 0;  // nothing to check

	Assert(node >= 0 && node < tree_nodes.size());
	Assert(tree_nodes[node].type == SEXPT_OPERATOR);

	op = get_operator_index(tree_nodes[node].text);
	if (op == -1)
		return node_error(node, "Unknown operator", bypass);

	count = count_args(tree_nodes[node].child);
	if (count < Operators[op].min)
		return node_error(node, "Too few arguments for operator", bypass);
	if (count > Operators[op].max)
		return node_error(node, "Too many arguments for operator", bypass);

	node = tree_nodes[node].child;  // get first argument
	while (node != -1) {
		type = query_operator_argument_type(op, argnum);
		Assert(tree_nodes[node].type & SEXPT_VALID);
		if (tree_nodes[node].type == SEXPT_OPERATOR) {
			if (verify_tree(node) == -1)
				return -1;

			op2 = get_operator_index(tree_nodes[node].text);  // no error checking, because it was done in the call above.
			type2 = query_operator_return_type(op2);

		} else if (tree_nodes[node].type == SEXPT_NUMBER) {
			char *ptr;

			type2 = OPR_NUMBER;
			ptr = tree_nodes[node].text;
			while (*ptr)
				if (!isdigit(*ptr++))
					return node_error(node, "Number is invalid", bypass);

		} else if (tree_nodes[node].type == SEXPT_STRING) {
			type2 = SEXP_ATOM_STRING;

		} else
			Assert(0);  // unknown and invalid sexp node type.

		switch (type) {
			case OPF_NUMBER:
				if (type2 != OPR_NUMBER)
					return node_error(node, "Number or number return type expected here", bypass);

				break;

			case OPF_SHIP:
				if (type2 == SEXP_ATOM_STRING)
					if (ship_name_lookup(tree_nodes[node].text, 1) == -1)
						type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Ship name expected here", bypass);

				break;

			case OPF_WING:
				if (type2 == SEXP_ATOM_STRING)
					if (wing_name_lookup(tree_nodes[node].text) == -1)
						type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Wing name expected here", bypass);

				break;

			case OPF_SHIP_WING:
				if (type2 == SEXP_ATOM_STRING)
					if (ship_name_lookup(tree_nodes[node].text, 1) == -1)
						if (wing_name_lookup(tree_nodes[node].text) == -1)
							type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Ship or wing name expected here", bypass);

				break;

			case OPF_BOOL:
				if (type2 != OPR_BOOL)
					return node_error(node, "Boolean return type expected here", bypass);

				break;

			case OPF_NULL:
				if (type2 != OPR_NULL)
					return node_error(node, "No return type operator expected here", bypass);

				break;

			case OPF_POINT:
				if (type2 != SEXP_ATOM_STRING || verify_vector(tree_nodes[node].text))
					return node_error(node, "3d coordinate expected here", bypass);

				break;

			case OPF_SUBSYSTEM:
			case OPF_AWACS_SUBSYSTEM:
			case OPF_ROTATING_SUBSYSTEM:
				if (type2 == SEXP_ATOM_STRING)
					if (ai_get_subsystem_type(tree_nodes[node].text) == SUBSYSTEM_UNKNOWN)
						type2 = 0;

				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Subsystem name expected here", bypass);

				break;

			case OPF_IFF:
				if (type2 == SEXP_ATOM_STRING) {
					for (i=0; i<Num_iffs; i++)
						if (!stricmp(Team_names[i], tree_nodes[node].text))
							break;
				}

				if (i == Num_iffs)
					return node_error(node, "Iff team type expected here", bypass);

				break;

			case OPF_AI_GOAL:
				if (type2 != OPR_AI_GOAL)
					return node_error(node, "Ai goal return type expected here", bypass);

				break;

			case OPF_FLEXIBLE_ARGUMENT:
				if (type2 != OPR_FLEXIBLE_ARGUMENT)
					return node_error(node, "Flexible argument return type expected here", bypass);

				break;

			case OPF_ANYTHING:
				break;

			case OPF_DOCKER_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Docker docking point name expected here", bypass);

				break;

			case OPF_DOCKEE_POINT:
				if (type2 != SEXP_ATOM_STRING)
					return node_error(node, "Dockee docking point name expected here", bypass);

				break;
		}

		node = tree_nodes[node].next;
		argnum++;
	}

	return 0;
}
*/

// display an error message and position to point of error (a node)
int sexp_tree::node_error(int node, const char* msg, int* bypass) {
	if (bypass) {
		*bypass = 1;
	}

	item_index = node;
	auto item_handle = tree_nodes[node].handle;
	if (tree_nodes[node].flags & COMBINED) {
		item_handle = tree_nodes[tree_nodes[node].parent].handle;
	}

	ensure_visible(node);
	item_handle->setSelected(true);

	auto text = QString("%1\n\nContinue checking for more errors?").arg(msg);

	if (QMessageBox::critical(this, "Sexp error", text, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
		return -1;
	} else {
		return 0;
	}
}

void sexp_tree::hilite_item(int node) {

	ensure_visible(node);
	clearSelection();
	setCurrentItem(tree_nodes[node].handle);
	scrollToItem(tree_nodes[node].handle);
}

// because the MFC function EnsureVisible() doesn't do what it says it does, I wrote this.
void sexp_tree::ensure_visible(int node) {
	auto handle = tree_nodes[node].handle->parent();

	while (handle != nullptr) {
		handle->setExpanded(true);
		handle = handle->parent();
	}
}

void get_variable_default_text_from_variable_text(char* text, char* default_text) {
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

void get_variable_name_from_sexp_tree_node_text(const char* text, char* var_name) {
	auto length = strcspn(text, "(");

	strncpy(var_name, text, length);
	var_name[length] = '\0';
}

int sexp_tree::get_modify_variable_type(int parent) {
	int sexp_var_index = -1;

	Assert(parent >= 0);
	int op_const = get_operator_const(tree_nodes[parent].text);

	Assert(tree_nodes[parent].child >= 0);
	char* node_text = tree_nodes[tree_nodes[parent].child].text;

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

	if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_BLOCK
		|| Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NOT_USED) {
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


void sexp_tree::verify_and_fix_arguments(int node) {
	int op_index, arg_num, type, tmp;
	static int here_count = 0;
	sexp_list_item* list, * ptr;
	bool is_variable_arg = false;

	if (here_count) {
		return;
	}

	here_count++;
	op_index = get_operator_index(tree_nodes[node].text);
	if (op_index < 0) {
		return;
	}

	tmp = item_index;

	arg_num = 0;
	setCurrentItemIndex(tree_nodes[node].child);
	while (item_index >= 0) {
		// get listing of valid argument values for node item_index
		type = query_operator_argument_type(op_index, arg_num);
		// special case for modify-variable
		if (type == OPF_AMBIGUOUS) {
			is_variable_arg = true;
			type = get_modify_variable_type(node);
		}
		if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
			// we don't care if the data matches
			// TODO: revisit if/when strictly typed data becomes supported
			item_index = tree_nodes[item_index].next;
			arg_num++;
			continue;
		}
		if (query_restricted_opf_range(type)) {
			list = get_listing_opf(type, node, arg_num);
			if (!list && (arg_num >= Operators[op_index].min)) {
				free_node(item_index, 1);
				setCurrentItemIndex(tmp);
				here_count--;
				return;
			}

			if (list) {
				// get a pointer to tree_nodes[item_index].text for normal value
				// or default variable value if variable
				char* text_ptr;
				char default_variable_text[TOKEN_LENGTH];
				if (tree_nodes[item_index].type & SEXPT_VARIABLE) {
					// special case for SEXPs which can modify a variable
					if (type == OPF_VARIABLE_NAME) {
						// make text_ptr to start - before '('
						get_variable_name_from_sexp_tree_node_text(tree_nodes[item_index].text, default_variable_text);
						text_ptr = default_variable_text;
					} else {
						// only the type needs checking for variables. It's up the to the FREDder to ensure the value is valid
						get_variable_name_from_sexp_tree_node_text(tree_nodes[item_index].text, default_variable_text);
						int sexp_var_index = get_index_sexp_variable_name(default_variable_text);
						bool types_match = false;
						Assert(sexp_var_index != -1);

						switch (type) {
						case OPF_NUMBER:
						case OPF_POSITIVE:
							if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
								types_match = true;
							}
							break;

						default:
							if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
								types_match = true;
							}
						}

						if (types_match) {
							// on to the next argument
							setCurrentItemIndex(tree_nodes[item_index].next);
							arg_num++;
							continue;
						} else {
							// shouldn't really be getting here unless someone has been hacking the mission in a text editor
							get_variable_default_text_from_variable_text(tree_nodes[item_index].text,
																		 default_variable_text);
							text_ptr = default_variable_text;
						}
					}
				} else {
					text_ptr = tree_nodes[item_index].text;
				}

				ptr = list;
				while (ptr) {
					// make sure text is not NULL
					// check that proposed text is valid for operator
					if (!stricmp(ptr->text.c_str(), text_ptr)) {
						break;
					}

					ptr = ptr->next;
				}

				if (!ptr) {  // argument isn't in list of valid choices,
					if (list->op >= 0) {
						replace_operator(list->text.c_str());
					} else {
						replace_data(list->text.c_str(), list->type);
					}
				}

			} else {
				bool invalid = false;
				if (type == OPF_AMBIGUOUS) {
					if (SEXPT_TYPE(tree_nodes[item_index].type) == SEXPT_OPERATOR) {
						invalid = true;
					}
				} else {
					if (SEXPT_TYPE(tree_nodes[item_index].type) != SEXPT_OPERATOR) {
						invalid = true;
					}
				}

				if (invalid) {
					replace_data("<Invalid>", (SEXPT_STRING | SEXPT_VALID));
				}
			}

			if (tree_nodes[item_index].type & SEXPT_OPERATOR) {
				verify_and_fix_arguments(item_index);
			}

		}

		//fix the node if it is the argument for modify-variable
		if (is_variable_arg //&&
			//	!(tree_nodes[item_index].type & SEXPT_OPERATOR || tree_nodes[item_index].type & SEXPT_VARIABLE )
			) {
			switch (type) {
			case OPF_AMBIGUOUS:
				tree_nodes[item_index].type |= SEXPT_STRING;
				tree_nodes[item_index].type &= ~SEXPT_NUMBER;
				break;

			case OPF_NUMBER:
				tree_nodes[item_index].type |= SEXPT_NUMBER;
				tree_nodes[item_index].type &= ~SEXPT_STRING;
				break;

			default:
				Int3();
			}
		}

		setCurrentItemIndex(tree_nodes[item_index].next);
		arg_num++;
	}

	setCurrentItemIndex(tmp);
	here_count--;
}

void sexp_tree::replace_data(const char* new_data, int type) {
	auto node = tree_nodes[item_index].child;
	if (node != -1) {
		free_node2(node);
	}

	tree_nodes[item_index].child = -1;
	auto h = tree_nodes[item_index].handle;
	while (h->childCount() > 0) {
		h->removeChild(h->child(0));
	}

	set_node(item_index, type, new_data);
	h->setText(0, new_data);
	auto bmap = get_data_image(item_index);
	h->setIcon(0, convertNodeImageToIcon(bmap));
	h->setFlags(h->flags().setFlag(Qt::ItemIsEditable, true));
	tree_nodes[item_index].flags = EDITABLE;

	// check remaining data beyond replaced data for validity (in case any of it is dependent on data just replaced)
	verify_and_fix_arguments(tree_nodes[item_index].parent);

	modified();
	update_help(currentItem());
}


// Replaces data with sexp_variable type data
void sexp_tree::replace_variable_data(int var_idx, int type) {
	char buf[128];

	Assert(type & SEXPT_VARIABLE);

	auto node = tree_nodes[item_index].child;
	if (node != -1) {
		free_node2(node);
	}

	tree_nodes[item_index].child = -1;
	auto h = tree_nodes[item_index].handle;
	while (h->childCount() > 0) {
		h->removeChild(h->child(0));
	}

	// Assemble name
	sprintf(buf, "%s(%s)", Sexp_variables[var_idx].variable_name, Sexp_variables[var_idx].text);

	set_node(item_index, type, buf);
	h->setText(0, QString::fromUtf8(buf));
	h->setIcon(0, convertNodeImageToIcon(NodeImage::VARIABLE));
	h->setFlags(h->flags().setFlag(Qt::ItemIsEditable, false));
	tree_nodes[item_index].flags = NOT_EDITABLE;

	// check remaining data beyond replaced data for validity (in case any of it is dependent on data just replaced)
	verify_and_fix_arguments(tree_nodes[item_index].parent);

	modified();
	update_help(currentItem());
}

void sexp_tree::replace_container_name(const sexp_container &container)
{
	// clean up any child nodes
	int node = tree_nodes[item_index].child;
	if (node != -1) {
		free_node2(node);
	}
	tree_nodes[item_index].child = -1;
	auto *h = tree_nodes[item_index].handle;
	while (h->childCount() > 0) {
		h->removeChild(h->child(0));
	}

	set_node(item_index, (SEXPT_VALID | SEXPT_STRING | SEXPT_CONTAINER_NAME), container.container_name.c_str());
	h->setText(0, QString::fromStdString(container.container_name));
	h->setIcon(0, convertNodeImageToIcon(NodeImage::CONTAINER_NAME));
	h->setFlags(h->flags().setFlag(Qt::ItemIsEditable, false));
	tree_nodes[item_index].flags = NOT_EDITABLE;

	modified();
	update_help(currentItem());
}

void sexp_tree::replace_container_data(const sexp_container &container,
	int type,
	bool test_child_nodes,
	bool delete_child_nodes,
	bool set_default_modifier)
{
	auto *h = tree_nodes[item_index].handle;

	// if this is already a container of the right type, don't alter the child nodes
	if (test_child_nodes && (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA)) {
		if (container.is_list()) {
			const auto *p_old_container = get_sexp_container(tree_nodes[item_index].text);

			Assertion(p_old_container != nullptr,
				"Attempt to Replace Container Data of unknown previous container %s. Please report!",
				tree_nodes[item_index].text);

			if (p_old_container->is_list()) {
				// TODO: check for strictly typed data here

				if (container.opf_type == p_old_container->opf_type) {
					delete_child_nodes = false;
					set_default_modifier = false;
				}
			}
		}
	}

	if (delete_child_nodes) {
		int node = tree_nodes[item_index].child;
		if (node != -1) {
			free_node2(node);
		}

		tree_nodes[item_index].child = -1;
		while (h->childCount() > 0) {
			h->removeChild(h->child(0));
		}
	}

	set_node(item_index, type, container.container_name.c_str());
	h->setText(0, QString::fromStdString(container.container_name));
	h->setIcon(0, convertNodeImageToIcon(NodeImage::CONTAINER_DATA));
	h->setFlags(h->flags().setFlag(Qt::ItemIsEditable, false));
	tree_nodes[item_index].flags = NOT_EDITABLE;

	if (set_default_modifier) {
		add_default_modifier(container);
	}

	modified();
	update_help(currentItem());
}


void sexp_tree::add_default_modifier(const sexp_container &container)
{
	sexp_list_item item;

	int type_to_use = (SEXPT_VALID | SEXPT_MODIFIER);

	if (container.is_map()) {
		if (any(container.type & ContainerType::STRING_KEYS)) {
			item.set_data("<any string>");
			type_to_use |= SEXPT_STRING;
		} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
			item.set_data("0");
			type_to_use |= SEXPT_NUMBER;
		} else {
			UNREACHABLE("Unknown map container key type %d", (int)container.type);
		}
	} else if (container.is_list()) {
		item.set_data(get_all_list_modifiers()[0].name);
		type_to_use |= SEXPT_STRING;
	} else {
		UNREACHABLE("Unknown container type %d", (int)container.type);
	}

	item.type = type_to_use;
	add_data(item.text.c_str(), item.type);
}

void sexp_tree::replace_operator(const char* op) {
	auto node = tree_nodes[item_index].child;
	if (node != -1) {
		free_node2(node);
	}

	tree_nodes[item_index].child = -1;
	auto h = tree_nodes[item_index].handle;
	while (h->childCount() > 0) {
		h->removeChild(h->child(0));
	}

	set_node(item_index, (SEXPT_OPERATOR | SEXPT_VALID), op);
	h->setText(0, op);
	tree_nodes[item_index].flags = OPERAND;
	modified();
	update_help(currentItem());

	// hack added at Allender's request.  If changing ship in an ai-dock operator, re-default
	// docking point.
}

/*void sexp_tree::replace_one_arg_operator(char *op, char *data, int type)
{
	char str[80];
	int node;
	HTREEITEM h;

	node = tree_nodes[item_index].child;
	if (node != -1)
		free_node2(node);

	tree_nodes[item_index].child = -1;
	h = tree_nodes[item_index].handle;
	while (ItemHasChildren(h))
		DeleteItem(GetChildItem(h));

	node = allocate_node(item_index);
	set_node(item_index, SEXPT_OPERATOR, op);
	set_node(node, type, data);
	sprintf(str, "%s %s", op, data);
	SetItemText(h, str);
	tree_nodes[item_index].flags = OPERAND | EDITABLE;
	tree_nodes[node].flags = COMBINED;
	*modified = 1;
	update_help(GetSelectedItem());
}*/

// moves a whole sexp tree branch to a new position under 'parent' and after 'after'.
// The expansion state is preserved, and node handles are updated.
void sexp_tree::move_branch(int source, int parent) {
	int node;

	// if no source, skip everything
	if (source != -1) {
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

		tree_nodes[source].parent = parent;
		tree_nodes[source].next = -1;
		if (parent) {
			if (tree_nodes[parent].child == -1) {
				tree_nodes[parent].child = source;
			} else {
				node = tree_nodes[parent].child;
				while (tree_nodes[node].next != -1) {
					node = tree_nodes[node].next;
				}

				tree_nodes[node].next = source;
			}

			move_branch(tree_nodes[source].handle, tree_nodes[parent].handle);

		} else {
			move_branch(tree_nodes[source].handle);
		}
	}
}

QTreeWidgetItem* sexp_tree::move_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent, QTreeWidgetItem* after) {
	QTreeWidgetItem* h = nullptr;
	if (source) {
		uint i;

		for (i = 0; i < tree_nodes.size(); i++) {
			if (tree_nodes[i].handle == source) {
				break;
			}
		}

		if (i < tree_nodes.size()) {
			auto icon = source->icon(0);
			h = insertWithIcon(source->text(0), icon, parent, after);
			tree_nodes[i].handle = h;
		} else {
			auto icon = source->icon(0);
			h = insertWithIcon(source->text(0), icon, parent, after);
		}

		h->setData(0, FormulaDataRole, source->data(0, FormulaDataRole));
		for (auto childIdx = 0; childIdx < source->childCount(); ++i) {
			auto child = source->child(childIdx);

			move_branch(child, h);
		}

		h->setExpanded(source->isExpanded());

		source->parent()->removeChild(source);
	}

	return h;
}

void sexp_tree::copy_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent, QTreeWidgetItem* after) {
	QTreeWidgetItem* h = nullptr;
	if (source) {
		uint i;

		for (i = 0; i < tree_nodes.size(); i++) {
			if (tree_nodes[i].handle == source) {
				break;
			}
		}

		if (i < tree_nodes.size()) {
			auto icon = source->icon(0);
			h = insertWithIcon(source->text(0), icon, parent, after);
			tree_nodes[i].handle = h;
		} else {
			auto icon = source->icon(0);
			h = insertWithIcon(source->text(0), icon, parent, after);
		}

		h->setData(0, FormulaDataRole, source->data(0, FormulaDataRole));
		for (auto childIdx = 0; childIdx < source->childCount(); ++i) {
			auto child = source->child(childIdx);

			move_branch(child, h);
		}

		h->setExpanded(source->isExpanded());
	}
}

void sexp_tree::move_root(QTreeWidgetItem* source, QTreeWidgetItem* dest, bool insert_before) {
	auto after = dest;

	if (insert_before) {
		Warning(LOCATION, "Inserting before a tree item is not yet implemented in qtFRED");
	}

	auto h = move_branch(source, itemFromIndex(rootIndex()), after);
	setCurrentItem(h);
	modified();
}

QTreeWidgetItem*
sexp_tree::insert(const QString& lpszItem, NodeImage image, QTreeWidgetItem* hParent, QTreeWidgetItem* hInsertAfter) {
	return insertWithIcon(lpszItem, convertNodeImageToIcon(image), hParent, hInsertAfter);
}

QTreeWidgetItem* sexp_tree::insertWithIcon(const QString& lpszItem,
										   const QIcon& image,
										   QTreeWidgetItem* hParent,
										   QTreeWidgetItem* hInsertAfter) {
	util::SignalBlockers blockers(this);

	QTreeWidgetItem* item = nullptr;
	if (hParent == nullptr) {
		if (hInsertAfter == nullptr) {
			item = new QTreeWidgetItem(this);
		} else {
			item = new QTreeWidgetItem(this, hInsertAfter);
		}
	} else {
		if (hInsertAfter == nullptr) {
			item = new QTreeWidgetItem(hParent);
		} else {
			item = new QTreeWidgetItem(hParent, hInsertAfter);
		}
	}
	item->setText(0, lpszItem);
	item->setIcon(0, image);
	item->setFlags(item->flags() | Qt::ItemIsEditable);

	return item;
}

QTreeWidgetItem* sexp_tree::handle(int node) {
	return tree_nodes[node].handle;
}

const char* sexp_tree::help(int code) {
	int i;

	i = (int) Sexp_help.size();
	while (i--) {
		if (Sexp_help[i].id == code) {
			break;
		}
	}

	if (i >= 0) {
		return Sexp_help[i].help.c_str();
	}

	return NULL;
}

// get type of item clicked on
int sexp_tree::get_type(QTreeWidgetItem* h) {
	uint i;

	// get index into sexp_tree
	for (i = 0; i < tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			break;
		}
	}

	if ((i >= tree_nodes.size())) {
		// Int3();	// This would be the root of the tree  -- ie, event name
		return -1;
	}

	return tree_nodes[i].type;
}

// get node of item clicked on
int sexp_tree::get_node(QTreeWidgetItem* h) {
	uint i;

	// get index into sexp_tree
	for (i = 0; i < tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			break;
		}
	}

	if ((i >= tree_nodes.size())) {
		// Int3();	// This would be the root of the tree  -- ie, event name
		return -1;
	}

	return i;
}

void sexp_tree::update_help(QTreeWidgetItem* h) {
	if (h == nullptr) {
		helpChanged("");
		miniHelpChanged("");
	}

	int i, j, z, c, code, index, sibling_place;

	for (i = 0; i < (int) Operators.size(); i++) {
		for (j = 0; j < (int) op_menu.size(); j++) {
			if (get_category(Operators[i].value) == op_menu[j].id) {
				if (!help(Operators[i].value)) {
					mprintf(("Allender!  If you add new sexp operators, add help for them too! :)\n"));
				}
			}
		}
	}

	for (i = 0; i < (int) tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			break;
		}
	}

	if ((i >= (int) tree_nodes.size()) || !tree_nodes[i].type) {
		helpChanged("");
		miniHelpChanged("");
		return;
	}

	if (SEXPT_TYPE(tree_nodes[i].type) == SEXPT_OPERATOR) {
		miniHelpChanged("");
	} else {
		z = tree_nodes[i].parent;
		if (z < 0) {
			Warning(LOCATION, "Sexp data \"%s\" has no parent!", tree_nodes[i].text);
			return;
		}

		code = get_operator_const(tree_nodes[z].text);
		index = get_operator_index(tree_nodes[z].text);
		sibling_place = get_sibling_place(i) + 1;    //We want it to start at 1

		//*****Minihelp box
		if ((SEXPT_TYPE(tree_nodes[i].type) == SEXPT_NUMBER)
			|| ((SEXPT_TYPE(tree_nodes[i].type) == SEXPT_STRING) && sibling_place > 0)) {
			char buffer[10240] = { "" };

			//Get the help for the current operator
			const char* helpstr = help(code);
			bool display_number = true;

			//If a help string exists, try to display it
			if (helpstr != NULL) {
				char searchstr[32];
				const char* loc = NULL, * loc2 = NULL;

				if (loc == NULL) {
					sprintf(searchstr, "\n%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}

				if (loc == NULL) {
					sprintf(searchstr, "\t%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if (loc == NULL) {
					sprintf(searchstr, " %d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if (loc == NULL) {
					sprintf(searchstr, "%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if (loc == NULL) {
					loc = strstr(helpstr, "Rest:");
				}
				if (loc == NULL) {
					loc = strstr(helpstr, "All:");
				}

				if (loc != NULL) {
					//Skip whitespace
					while (*loc == '\r' || *loc == '\n' || *loc == ' ' || *loc == '\t') {
						loc++;
					}

					//Find EOL
					loc2 = strpbrk(loc, "\r\n");
					if (loc2 != NULL) {
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

			//Display argument number
			if (display_number) {
				sprintf(buffer, "%d:", sibling_place);
			}

			miniHelpChanged(QString::fromUtf8(buffer));
		}

		if (index >= 0) {
			c = 0;
			j = tree_nodes[z].child;
			while ((j >= 0) && (j != i)) {
				j = tree_nodes[j].next;
				c++;
			}

			Assert(j >= 0);
			if (query_operator_argument_type(index, c) == OPF_MESSAGE) {
				for (j = 0; j < Num_messages; j++) {
					if (!stricmp(Messages[j].name, tree_nodes[i].text)) {
						auto text = QString("Message Text:\n%1").arg(Messages[j].message);
						helpChanged(text);
						return;
					}
				}
			}
		}

		i = z;
	}

	code = get_operator_const(tree_nodes[i].text);
	auto str = help(code);
	if (!str) {
		str = "No help available";
	}

	helpChanged(QString::fromUtf8(str));
}

// find list of sexp_tree nodes with text
// stuff node indices into find[]
int sexp_tree::find_text(const char* text, int* find, int max_depth) {
	int find_count;

	// initialize find
	for (int i = 0; i < max_depth; i++) {
		find[i] = -1;
	}

	find_count = 0;

	for (size_t i = 0; i < tree_nodes.size(); i++) {
		// only look at used and editable nodes
		if ((tree_nodes[i].flags & EDITABLE && (tree_nodes[i].type != SEXPT_UNUSED))) {
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


// Determine if a given opf code has a restricted argument range (i.e. has a specific, limited
// set of argument values, or has virtually unlimited possibilities.  For example, boolean values
// only have true or false, so it is restricted, but a number could be anything, so it's not.
//
int sexp_tree::query_restricted_opf_range(int opf) {
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

// generate listing of valid argument values.
// opf = operator format to generate list for
// parent_node = the parent node we are generating list for
// arg_index = argument number of parent this argument will go at
//
// Goober5000 - add the listing from get_listing_opf_sub to the end of a new list containing
// the special argument item, but only if it's a child of a when-argument (or similar) sexp.
// Also only do this if the list has at least one item, because otherwise the argument code
// would have nothing to select from.
sexp_list_item* sexp_tree::get_listing_opf(int opf, int parent_node, int arg_index) {
	sexp_list_item head;
	sexp_list_item* list = NULL;

	switch (opf) {
	case OPF_NONE:
		list = NULL;
		break;

	case OPF_NULL:
		list = get_listing_opf_null();
		break;

	case OPF_BOOL:
		list = get_listing_opf_bool(parent_node);
		break;

	case OPF_NUMBER:
		list = get_listing_opf_number();
		break;

	case OPF_SHIP:
		list = get_listing_opf_ship(parent_node);
		break;

	case OPF_WING:
		list = get_listing_opf_wing();
		break;

	case OPF_AWACS_SUBSYSTEM:
	case OPF_ROTATING_SUBSYSTEM:
	case OPF_SUBSYSTEM:
		list = get_listing_opf_subsystem(parent_node, arg_index);
		break;

	case OPF_SUBSYSTEM_TYPE:
		list = get_listing_opf_subsystem_type(parent_node);
		break;

	case OPF_POINT:
		list = get_listing_opf_point();
		break;

	case OPF_IFF:
		list = get_listing_opf_iff();
		break;

	case OPF_AI_CLASS:
		list = get_listing_opf_ai_class();
		break;

	case OPF_SUPPORT_SHIP_CLASS:
		list = get_listing_opf_support_ship_class();
		break;

	case OPF_SSM_CLASS:
		list = get_listing_opf_ssm_class();
		break;

	case OPF_ARRIVAL_LOCATION:
		list = get_listing_opf_arrival_location();
		break;

	case OPF_DEPARTURE_LOCATION:
		list = get_listing_opf_departure_location();
		break;

	case OPF_ARRIVAL_ANCHOR_ALL:
		list = get_listing_opf_arrival_anchor_all();
		break;

	case OPF_SHIP_WITH_BAY:
		list = get_listing_opf_ship_with_bay();
		break;

	case OPF_SOUNDTRACK_NAME:
		list = get_listing_opf_soundtrack_name();
		break;

	case OPF_AI_GOAL:
		list = get_listing_opf_ai_goal(parent_node);
		break;

	case OPF_FLEXIBLE_ARGUMENT:
		list = get_listing_opf_flexible_argument();
		break;

	case OPF_DOCKER_POINT:
		list = get_listing_opf_docker_point(parent_node, arg_index);
		break;

	case OPF_DOCKEE_POINT:
		list = get_listing_opf_dockee_point(parent_node);
		break;

	case OPF_MESSAGE:
		list = get_listing_opf_message();
		break;

	case OPF_WHO_FROM:
		list = get_listing_opf_who_from();
		break;

	case OPF_PRIORITY:
		list = get_listing_opf_priority();
		break;

	case OPF_WAYPOINT_PATH:
		list = get_listing_opf_waypoint_path();
		break;

	case OPF_POSITIVE:
		list = get_listing_opf_positive();
		break;

	case OPF_MISSION_NAME:
		list = get_listing_opf_mission_name();
		break;

	case OPF_SHIP_POINT:
		list = get_listing_opf_ship_point();
		break;

	case OPF_GOAL_NAME:
		list = get_listing_opf_goal_name(parent_node);
		break;

	case OPF_SHIP_WING:
		list = get_listing_opf_ship_wing();
		break;

	case OPF_SHIP_WING_WHOLETEAM:
		list = get_listing_opf_ship_wing_wholeteam();
		break;

	case OPF_SHIP_WING_SHIPONTEAM_POINT:
		list = get_listing_opf_ship_wing_shiponteam_point();
		break;

	case OPF_SHIP_WING_POINT:
		list = get_listing_opf_ship_wing_point();
		break;

	case OPF_SHIP_WING_POINT_OR_NONE:
		list = get_listing_opf_ship_wing_point_or_none();
		break;

	case OPF_ORDER_RECIPIENT:
		list = get_listing_opf_order_recipient();
		break;

	case OPF_SHIP_TYPE:
		list = get_listing_opf_ship_type();
		break;

	case OPF_KEYPRESS:
		list = get_listing_opf_keypress();
		break;

	case OPF_EVENT_NAME:
		list = get_listing_opf_event_name(parent_node);
		break;

	case OPF_AI_ORDER:
		list = get_listing_opf_ai_order();
		break;

	case OPF_SKILL_LEVEL:
		list = get_listing_opf_skill_level();
		break;

	case OPF_CARGO:
		list = get_listing_opf_cargo();
		break;

	case OPF_STRING:
		list = get_listing_opf_string();
		break;

	case OPF_MEDAL_NAME:
		list = get_listing_opf_medal_name();
		break;

	case OPF_WEAPON_NAME:
		list = get_listing_opf_weapon_name();
		break;

	case OPF_INTEL_NAME:
		list = get_listing_opf_intel_name();
		break;

	case OPF_SHIP_CLASS_NAME:
		list = get_listing_opf_ship_class_name();
		break;

	case OPF_HUGE_WEAPON:
		list = get_listing_opf_huge_weapon();
		break;

	case OPF_SHIP_NOT_PLAYER:
		list = get_listing_opf_ship_not_player();
		break;

	case OPF_SHIP_OR_NONE:
		list = get_listing_opf_ship_or_none();
		break;

	case OPF_SUBSYSTEM_OR_NONE:
		list = get_listing_opf_subsystem_or_none(parent_node, arg_index);
		break;

	case OPF_SUBSYS_OR_GENERIC:
		list = get_listing_opf_subsys_or_generic(parent_node, arg_index);
		break;

	case OPF_JUMP_NODE_NAME:
		list = get_listing_opf_jump_nodes();
		break;

	case OPF_VARIABLE_NAME:
		list = get_listing_opf_variable_names();
		break;

	case OPF_AMBIGUOUS:
		list = NULL;
		break;

	case OPF_ANYTHING:
		list = NULL;
		break;

	case OPF_SKYBOX_MODEL_NAME:
		list = get_listing_opf_skybox_model();
		break;

	case OPF_SKYBOX_FLAGS:
		list = get_listing_opf_skybox_flags();
		break;

	case OPF_BACKGROUND_BITMAP:
		list = get_listing_opf_background_bitmap();
		break;

	case OPF_SUN_BITMAP:
		list = get_listing_opf_sun_bitmap();
		break;

	case OPF_NEBULA_STORM_TYPE:
		list = get_listing_opf_nebula_storm_type();
		break;

	case OPF_NEBULA_POOF:
		list = get_listing_opf_nebula_poof();
		break;

	case OPF_TURRET_TARGET_ORDER:
		list = get_listing_opf_turret_target_order();
		break;

	case OPF_TARGET_PRIORITIES:
		list = get_listing_opf_turret_target_priorities();
		break;

	case OPF_ARMOR_TYPE:
		list = get_listing_opf_armor_type();
		break;

	case OPF_DAMAGE_TYPE:
		list = get_listing_opf_damage_type();
		break;

	case OPF_ANIMATION_TYPE:
		list = get_listing_opf_animation_type();
		break;

	case OPF_PERSONA:
		list = get_listing_opf_persona();
		break;

	case OPF_POST_EFFECT:
		list = get_listing_opf_post_effect();
		break;

	case OPF_FONT:
		list = get_listing_opf_font();
		break;

	case OPF_HUD_ELEMENT:
		list = get_listing_opf_hud_elements();
		break;

	case OPF_SOUND_ENVIRONMENT:
		list = get_listing_opf_sound_environment();
		break;

	case OPF_SOUND_ENVIRONMENT_OPTION:
		list = get_listing_opf_sound_environment_option();
		break;

	case OPF_AUDIO_VOLUME_OPTION:
		list = get_listing_opf_adjust_audio_volume();
		break;

	case OPF_EXPLOSION_OPTION:
		list = get_listing_opf_explosion_option();
		break;

	case OPF_WEAPON_BANK_NUMBER:
		list = get_listing_opf_weapon_banks();
		break;

	case OPF_MESSAGE_OR_STRING:
		list = get_listing_opf_message();
		break;

	case OPF_BUILTIN_HUD_GAUGE:
		list = get_listing_opf_builtin_hud_gauge();
		break;

	case OPF_CUSTOM_HUD_GAUGE:
		list = get_listing_opf_custom_hud_gauge();
		break;

	case OPF_ANY_HUD_GAUGE:
		list = get_listing_opf_any_hud_gauge();
		break;

	case OPF_SHIP_EFFECT:
		list = get_listing_opf_ship_effect();
		break;

	case OPF_MISSION_MOOD:
		list = get_listing_opf_mission_moods();
		break;

	case OPF_SHIP_FLAG:
		list = get_listing_opf_ship_flags();
		break;

	case OPF_WING_FLAG:
		list = get_listing_opf_wing_flags();
		break;

	case OPF_TEAM_COLOR:
		list = get_listing_opf_team_colors();
		break;

	case OPF_NEBULA_PATTERN:
		list = get_listing_opf_nebula_patterns();
		break;

	case OPF_GAME_SND:
		list = get_listing_opf_game_snds();
		break;

	case OPF_FIREBALL:
		list = get_listing_opf_fireball();
		break;

	case OPF_SPECIES:
		list = get_listing_opf_species();
		break;

	case OPF_LANGUAGE:
		list = get_listing_opf_language();
		break;

	case OPF_FUNCTIONAL_WHEN_EVAL_TYPE:
		list = get_listing_opf_functional_when_eval_type();
		break;

	case OPF_ANIMATION_NAME:
		list = get_listing_opf_animation_name(parent_node);
		break;	

	case OPF_CONTAINER_NAME:
		list = get_listing_opf_sexp_containers(ContainerType::LIST | ContainerType::MAP);
		break;

	case OPF_LIST_CONTAINER_NAME:
		list = get_listing_opf_sexp_containers(ContainerType::LIST);
		break;

	case OPF_MAP_CONTAINER_NAME:
		list = get_listing_opf_sexp_containers(ContainerType::MAP);
		break;

	case OPF_CONTAINER_VALUE:
		list = nullptr;
		break;

	case OPF_DATA_OR_STR_CONTAINER:
		list = nullptr;
		break;

	case OPF_WING_FORMATION:
		list = get_listing_opf_wing_formation();
		break;

	default:
		// We're at the end of the list so check for any dynamic enums
		list = check_for_dynamic_sexp_enum(opf);
		break;
	}


	// skip OPF_NONE, also skip for OPF_NULL, because it takes no data (though it can take plenty of operators)
	if (opf == OPF_NULL || opf == OPF_NONE) {
		return list;
	}

	// skip the special argument if we aren't at the right spot in when-argument or
	// every-time-argument
	if (!is_node_eligible_for_special_argument(parent_node)) {
		return list;
	}

	// the special item is a string and should not be added for numeric lists
	if (opf != OPF_NUMBER && opf != OPF_POSITIVE) {
		head.add_data(SEXP_ARGUMENT_STRING);
	}

	if (list != NULL) {
		// append other list
		head.add_list(list);
	}

	// return listing
	return head.next;
}

// Goober5000
int sexp_tree::find_argument_number(int parent_node, int child_node) const {
	int arg_num, current_node;

	// code moved/adapted from match_closest_operator
	arg_num = 0;
	current_node = tree_nodes[parent_node].child;
	while (current_node >= 0) {
		// found?
		if (current_node == child_node) {
			return arg_num;
		}

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
int sexp_tree::find_ancestral_argument_number(int parent_op, int child_node) const {
	if (child_node == -1) {
		return -1;
	}

	int parent_node;
	int current_node;

	current_node = child_node;
	parent_node = tree_nodes[current_node].parent;

	while (parent_node >= 0) {
		// check if the parent operator is the one we're looking for
		if (get_operator_const(tree_nodes[parent_node].text) == parent_op) {
			return find_argument_number(parent_node, current_node);
		}

		// continue iterating up the tree
		current_node = parent_node;
		parent_node = tree_nodes[current_node].parent;
	}

	// not found
	return -1;
}

/**
* Gets the proper data image for the tree item's place
* in its parent hierarchy.
*/
NodeImage sexp_tree::get_data_image(int node) {
	int count = get_sibling_place(node) + 1;

	if (count <= 0) {
		return NodeImage::DATA;
	}

	if (count % 5 != 0) {
		return NodeImage::DATA;
	}

	int idx = (count % 100) / 5;

	return static_cast<NodeImage>(static_cast<int>(NodeImage::DATA_00) + idx);
}

int sexp_tree::get_sibling_place(int node) {
	if (tree_nodes[node].parent > (int) tree_nodes.size()) {
		return -1;
	}
	if (tree_nodes[node].parent < 0) {
		return -1;
	}

	sexp_tree_item* myparent = &tree_nodes[tree_nodes[node].parent];

	if (myparent->child == -1) {
		return -1;
	}

	sexp_tree_item* mysibling = &tree_nodes[myparent->child];

	int count = 0;
	while (true) {
		if (mysibling == &tree_nodes[node]) {
			break;
		}

		if (mysibling->next == -1) {
			break;
		}

		count++;
		mysibling = &tree_nodes[mysibling->next];
	}

	return count;
}


sexp_list_item* sexp_tree::get_listing_opf_null() {
	int i;
	sexp_list_item head;

	for (i = 0; i < (int) Operators.size(); i++) {
		if (query_operator_return_type(i) == OPR_NULL) {
			head.add_op(i);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_flexible_argument() {
	int i;
	sexp_list_item head;

	for (i = 0; i < (int) Operators.size(); i++) {
		if (query_operator_return_type(i) == OPR_FLEXIBLE_ARGUMENT) {
			head.add_op(i);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_bool(int parent_node) {
	int i, only_basic;
	sexp_list_item head;

	// search for the previous goal/event operators.  If found, only add the true/false
	// sexpressions to the list
	only_basic = 0;
	if (parent_node != -1) {
		int op;

		op = get_operator_const(tree_nodes[parent_node].text);
		if ((op == OP_PREVIOUS_GOAL_TRUE) || (op == OP_PREVIOUS_GOAL_FALSE) || (op == OP_PREVIOUS_EVENT_TRUE)
			|| (op == OP_PREVIOUS_EVENT_FALSE)) {
			only_basic = 1;
		}

	}

	for (i = 0; i < (int) Operators.size(); i++) {
		if (query_operator_return_type(i) == OPR_BOOL) {
			if (!only_basic || (only_basic && ((Operators[i].value == OP_TRUE) || (Operators[i].value == OP_FALSE)))) {
				head.add_op(i);
			}
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_positive() {
	int i, z;
	sexp_list_item head;

	for (i = 0; i < (int) Operators.size(); i++) {
		z = query_operator_return_type(i);
		// Goober5000's number hack
		if ((z == OPR_NUMBER) || (z == OPR_POSITIVE)) {
			head.add_op(i);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_number() {
	int i, z;
	sexp_list_item head;

	for (i = 0; i < (int) Operators.size(); i++) {
		z = query_operator_return_type(i);
		if ((z == OPR_NUMBER) || (z == OPR_POSITIVE)) {
			head.add_op(i);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship(int parent_node) {
	object* ptr;
	sexp_list_item head;
	int op = 0, dock_ship = -1, require_cap_ship = 0;

	// look at the parent node and get the operator.  Some ship lists should be filtered based
	// on what the parent operator is
	if (parent_node >= 0) {
		op = get_operator_const(tree_nodes[parent_node].text);

		// get the dock_ship number of if this goal is an ai dock goal.  used to prune out unwanted ships out
		// of the generated ship list
		dock_ship = -1;
		if (op == OP_AI_DOCK) {
			int z;

			z = tree_nodes[parent_node].parent;
			Assert(z >= 0);
			Assert(!stricmp(tree_nodes[z].text, "add-ship-goal") || !stricmp(tree_nodes[z].text, "add-wing-goal")
					   || !stricmp(tree_nodes[z].text, "add-goal"));

			z = tree_nodes[z].child;
			Assert(z >= 0);

			dock_ship = ship_name_lookup(tree_nodes[z].text, 1);
			Assert(dock_ship != -1);
		}
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if (op == OP_AI_DOCK) {
				// only include those ships in the list which the given ship can dock with.
				if ((dock_ship != ptr->instance) && ship_docking_valid(dock_ship, ptr->instance)) {
					head.add_data(Ships[ptr->instance].ship_name);
				}

			} else if (op == OP_CAP_SUBSYS_CARGO_KNOWN_DELAY) {
				if (((Ship_info[Ships[ptr->instance].ship_info_index].is_huge_ship()) &&    // big ship
					!(Ships[ptr->instance].flags[Ship::Ship_Flags::Toggle_subsystem_scanning]))
					||                // which is not flagged OR
						((!(Ship_info[Ships[ptr->instance].ship_info_index].is_huge_ship())) &&  // small ship
							(Ships[ptr->instance].flags[Ship::Ship_Flags::Toggle_subsystem_scanning]))) {                // which is flagged

					head.add_data(Ships[ptr->instance].ship_name);
				}
			} else {
				if (!require_cap_ship || Ship_info[Ships[ptr->instance].ship_info_index].is_huge_ship()) {
					head.add_data(Ships[ptr->instance].ship_name);
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_wing() {
	int i;
	sexp_list_item head;

	for (i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count) {
			head.add_data(Wings[i].name);
		}
	}

	return head.next;
}

// specific types of subsystems we're looking for
#define OPS_CAP_CARGO        1
#define OPS_STRENGTH            2
#define OPS_BEAM_TURRET        3
#define OPS_AWACS                4
#define OPS_ROTATE            5
#define OPS_ARMOR            6
sexp_list_item* sexp_tree::get_listing_opf_subsystem(int parent_node, int arg_index) {
	int op, child, sh;
	int special_subsys = 0;
	sexp_list_item head;
	ship_subsys* subsys;

	// determine if the parent is one of the set subsystem strength items.  If so,
	// we want to append the "Hull" name onto the end of the menu
	Assert(parent_node >= 0);

	// get the operator type of the node
	op = get_operator_const(tree_nodes[parent_node].text);

	// first child node
	child = tree_nodes[parent_node].child;
	Assert(child >= 0);

	switch (op) {
		// where we care about hull strength
	case OP_REPAIR_SUBSYSTEM:
	case OP_SABOTAGE_SUBSYSTEM:
	case OP_SET_SUBSYSTEM_STRNGTH:
		special_subsys = OPS_STRENGTH;
		break;

		// Armor types need Hull and Shields but not Simulated Hull
	case OP_SET_ARMOR_TYPE:
		special_subsys = OPS_ARMOR;
		break;

		// awacs subsystems
	case OP_AWACS_SET_RADIUS:
		special_subsys = OPS_AWACS;
		break;

		// rotating
	case OP_LOCK_ROTATING_SUBSYSTEM:
	case OP_FREE_ROTATING_SUBSYSTEM:
	case OP_REVERSE_ROTATING_SUBSYSTEM:
	case OP_ROTATING_SUBSYS_SET_TURN_TIME:
		special_subsys = OPS_ROTATE;
		break;

		// where we care about capital ship subsystem cargo
	case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
		special_subsys = OPS_CAP_CARGO;

		// get the next sibling
		child = tree_nodes[child].next;
		break;

		// where we care about turrets carrying beam weapons
	case OP_BEAM_FIRE:
		special_subsys = OPS_BEAM_TURRET;

		// if this is arg index 3 (targeted ship)
		if (arg_index == 3) {
			special_subsys = OPS_STRENGTH;

			// iterate to the next field two times
			child = tree_nodes[child].next;
			Assert(child >= 0);
			child = tree_nodes[child].next;
		} else {
			Assert(arg_index == 1);
		}
		break;

	case OP_BEAM_FIRE_COORDS:
		special_subsys = OPS_BEAM_TURRET;
		break;

		// these sexps check the subsystem of the *second entry* on the list, not the first
	case OP_DISTANCE_CENTER_SUBSYSTEM:
	case OP_DISTANCE_BBOX_SUBSYSTEM:
	case OP_SET_CARGO:
	case OP_IS_CARGO:
	case OP_CHANGE_AI_CLASS:
	case OP_IS_AI_CLASS:
	case OP_MISSILE_LOCKED:
	case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
	case OP_IS_IN_TURRET_FOV:
		// iterate to the next field
		child = tree_nodes[child].next;
		break;

		// this sexp checks the subsystem of the *fourth entry* on the list
	case OP_QUERY_ORDERS:
		// iterate to the next field three times
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		break;

		// this sexp checks the subsystem of the *seventh entry* on the list
	case OP_BEAM_FLOATING_FIRE:
		// iterate to the next field six times
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		break;

		// this sexp checks the subsystem of the *ninth entry* on the list
	case OP_WEAPON_CREATE:
		// iterate to the next field eight times
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		Assert(child >= 0);
		child = tree_nodes[child].next;
		break;

	default:
		if (op < First_available_operator_id) {
			break;
		} else {
			int this_index = get_dynamic_parameter_index(tree_nodes[parent_node].text, arg_index);

			if (this_index >= 0) {
				for (int count = 0; count < this_index; count++) {
					child = tree_nodes[child].next;
				}
			} else {
				error_display(1,
					"Expected to find a dynamic lua parent parameter for node %i in operator %s but found nothing!",
					arg_index,
					tree_nodes[parent_node].text);
			}
		}
	}

	// now find the ship and add all relevant subsystems
	Assert(child >= 0);
	sh = ship_name_lookup(tree_nodes[child].text, 1);
	if (sh >= 0) {
		subsys = GET_FIRST(&Ships[sh].subsys_list);
		while (subsys != END_OF_LIST(&Ships[sh].subsys_list)) {
			// add stuff
			switch (special_subsys) {
				// subsystem cargo
			case OPS_CAP_CARGO:
				head.add_data(subsys->system_info->subobj_name);
				break;

				// beam fire
			case OPS_BEAM_TURRET:
				head.add_data(subsys->system_info->subobj_name);
				break;

				// awacs level
			case OPS_AWACS:
				if (subsys->system_info->flags[Model::Subsystem_Flags::Awacs]) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

				// rotating
			case OPS_ROTATE:
				if (subsys->system_info->flags[Model::Subsystem_Flags::Rotates]) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

				// everything else
			default:
				head.add_data(subsys->system_info->subobj_name);
				break;
			}

			// next subsystem
			subsys = GET_NEXT(subsys);
		}
	}

	// if one of the subsystem strength operators, append the Hull string and the Simulated Hull string
	if (special_subsys == OPS_STRENGTH) {
		head.add_data(SEXP_HULL_STRING);
		head.add_data(SEXP_SIM_HULL_STRING);
	}
	// if setting armor type we only need Hull and Shields
	if (special_subsys == OPS_ARMOR) {
		head.add_data(SEXP_HULL_STRING);
		head.add_data(SEXP_SHIELD_STRING);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_subsystem_type(int parent_node) {
	int i, child, shipnum, num_added = 0;
	sexp_list_item head;
	ship_subsys* subsys;

	// first child node
	child = tree_nodes[parent_node].child;
	Assert(child >= 0);

	// now find the ship
	shipnum = ship_name_lookup(tree_nodes[child].text, 1);
	if (shipnum < 0) {
		return head.next;
	}

	// add all relevant subsystem types
	for (i = 0; i < SUBSYSTEM_MAX; i++) {
		// don't allow these two
		if (i == SUBSYSTEM_NONE || i == SUBSYSTEM_UNKNOWN) {
			continue;
		}

		// loop through all ship subsystems
		subsys = GET_FIRST(&Ships[shipnum].subsys_list);
		while (subsys != END_OF_LIST(&Ships[shipnum].subsys_list)) {
			// check if this subsystem is of this type
			if (i == subsys->system_info->type) {
				// subsystem type is applicable, so add it
				head.add_data(Subsystem_types[i]);
				num_added++;
				break;
			}

			// next subsystem
			subsys = GET_NEXT(subsys);
		}
	}

	// if no subsystem types, go ahead and add NONE (even though it won't be checked)
	if (num_added == 0) {
		head.add_data(Subsystem_types[SUBSYSTEM_NONE]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_point() {
	char buf[NAME_LENGTH + 8];
	SCP_list<waypoint_list>::iterator ii;
	int j;
	sexp_list_item head;

	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii) {
		for (j = 0; (uint) j < ii->get_waypoints().size(); ++j) {
			sprintf(buf, "%s:%d", ii->get_name(), j + 1);
			head.add_data(buf);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_iff() {
	int i;
	sexp_list_item head;

	for (i = 0; i < (int)Iff_info.size(); i++) {
		head.add_data(Iff_info[i].iff_name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ai_class() {
	int i;
	sexp_list_item head;

	for (i = 0; i < Num_ai_classes; i++) {
		head.add_data(Ai_class_names[i]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_support_ship_class() {
	sexp_list_item head;

	head.add_data("<species support ship class>");

	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		if (it->flags[Ship::Info_Flags::Support]) {
			head.add_data(it->name);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ssm_class() {
	sexp_list_item head;

	for (auto it = Ssm_info.cbegin(); it != Ssm_info.cend(); ++it) {
		head.add_data(it->name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_with_bay() {
	object* objp;
	sexp_list_item head;

	head.add_data("<no anchor>");

	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
			// determine if this ship has a docking bay
			if (ship_has_dock_bay(objp->instance)) {
				head.add_data(Ships[objp->instance].ship_name);
			}
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_soundtrack_name() {
	sexp_list_item head;

	head.add_data("<No Music>");

	for (auto &st: Soundtracks) {
		head.add_data(st.name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_arrival_location() {
	int i;
	sexp_list_item head;

	for (i = 0; i < MAX_ARRIVAL_NAMES; i++) {
		head.add_data(Arrival_location_names[i]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_departure_location() {
	int i;
	sexp_list_item head;

	for (i = 0; i < MAX_DEPARTURE_NAMES; i++) {
		head.add_data(Departure_location_names[i]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_arrival_anchor_all() {
	int i, restrict_to_players;
	object* objp;
	sexp_list_item head;

	for (restrict_to_players = 0; restrict_to_players < 2; restrict_to_players++) {
		for (i = 0; i < (int)Iff_info.size(); i++) {
			char tmp[NAME_LENGTH + 15];
			stuff_special_arrival_anchor_name(tmp, i, restrict_to_players, 0);

			head.add_data(tmp);
		}
	}

	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
			head.add_data(Ships[objp->instance].ship_name);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ai_goal(int parent_node) {
	int i, n, w, z, child;
	sexp_list_item head;

	Assert(parent_node >= 0);
	child = tree_nodes[parent_node].child;
	Assert(child >= 0);
	n = ship_name_lookup(tree_nodes[child].text, 1);
	if (n >= 0) {
		// add operators if it's an ai-goal and ai-goal is allowed for that ship
		for (i = 0; i < (int) Operators.size(); i++) {
			if ((query_operator_return_type(i) == OPR_AI_GOAL) && query_sexp_ai_goal_valid(Operators[i].value, n)) {
				head.add_op(i);
			}
		}

	} else {
		z = wing_name_lookup(tree_nodes[child].text);
		if (z >= 0) {
			for (w = 0; w < Wings[z].wave_count; w++) {
				n = Wings[z].ship_index[w];
				// add operators if it's an ai-goal and ai-goal is allowed for that ship
				for (i = 0; i < (int) Operators.size(); i++) {
					if ((query_operator_return_type(i) == OPR_AI_GOAL)
						&& query_sexp_ai_goal_valid(Operators[i].value, n)) {
						head.add_op(i);
					}
				}
			}
			// when dealing with the special argument add them all. It's up to the FREDder to ensure invalid orders aren't given
		} else if (!strcmp(tree_nodes[child].text, SEXP_ARGUMENT_STRING)) {
			for (i = 0; i < (int) Operators.size(); i++) {
				if (query_operator_return_type(i) == OPR_AI_GOAL) {
					head.add_op(i);
				}
			}
		} else {
			return NULL;
		}  // no valid ship or wing to check against, make nothing available
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_docker_point(int parent_node, int argnum) {
	int z;
	sexp_list_item head;
	int sh = -1;

	Assert(parent_node >= 0);
	Assert(!stricmp(tree_nodes[parent_node].text, "ai-dock") || !stricmp(tree_nodes[parent_node].text, "set-docked") ||
		   get_operator_const(tree_nodes[parent_node].text) >= (int)First_available_operator_id);

	if (!stricmp(tree_nodes[parent_node].text, "ai-dock")) {
		z = tree_nodes[parent_node].parent;
		Assert(z >= 0);
		Assert(!stricmp(tree_nodes[z].text, "add-ship-goal") || !stricmp(tree_nodes[z].text, "add-wing-goal")
				   || !stricmp(tree_nodes[z].text, "add-goal"));

		z = tree_nodes[z].child;
		Assert(z >= 0);

		sh = ship_name_lookup(tree_nodes[z].text, 1);
	} else if (!stricmp(tree_nodes[parent_node].text, "set-docked")) {
		//Docker ship should be the first child node
		z = tree_nodes[parent_node].child;
		sh = ship_name_lookup(tree_nodes[z].text, 1);
	}
	// for Lua sexps
	else if (get_operator_const(tree_nodes[parent_node].text) >= (int)First_available_operator_id) {
		int this_index = get_dynamic_parameter_index(tree_nodes[parent_node].text, arg_num);

		if (this_index >= 0) {
			z = tree_nodes[parent_node].child;

			for (int j = 0; j < this_index; j++) {
				z = tree_nodes[z].next;
			}

			sh = ship_name_lookup(tree_nodes[z].text, 1);
		} else {
			error_display(1,
				"Expected to find a dynamic lua parent parameter for node %i in operator %s but found nothing!",
				arg_num,
				tree_nodes[parent_node].text);
		}
	}

	if (sh >= 0) {
		auto list = _editor->get_docking_list(Ship_info[Ships[sh].ship_info_index].model_num);
		for (auto& dock : list) {
			head.add_data(dock.c_str());
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_dockee_point(int parent_node) {
	int z;
	sexp_list_item head;
	int sh = -1;

	Assert(parent_node >= 0);
	Assert(!stricmp(tree_nodes[parent_node].text, "ai-dock") || !stricmp(tree_nodes[parent_node].text, "set-docked"));

	if (!stricmp(tree_nodes[parent_node].text, "ai-dock")) {
		z = tree_nodes[parent_node].child;
		Assert(z >= 0);

		sh = ship_name_lookup(tree_nodes[z].text, 1);
	} else if (!stricmp(tree_nodes[parent_node].text, "set-docked")) {
		//Dockee ship should be the third child node
		z = tree_nodes[parent_node].child;    // 1
		z = tree_nodes[z].next;                // 2
		z = tree_nodes[z].next;                // 3

		sh = ship_name_lookup(tree_nodes[z].text, 1);
	}

	if (sh >= 0) {
		auto list = _editor->get_docking_list(Ship_info[Ships[sh].ship_info_index].model_num);
		for (auto& dock : list) {
			head.add_data(dock.c_str());
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_message() {
	sexp_list_item head;

	for (auto& msg : _interface->getMessages()) {
		head.add_data(msg.c_str());
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_persona() {
	int i;
	sexp_list_item head;

	for (i = 0; i < Num_personas; i++) {
		if (Personas[i].flags & PERSONA_FLAG_WINGMAN) {
			head.add_data(Personas[i].name);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_font() {
	int i;
	sexp_list_item head;

	for (i = 0; i < font::FontManager::numberOfFonts(); i++) {
		head.add_data(const_cast<char*>(font::FontManager::getFont(i)->getName().c_str()));
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_who_from() {
	object* ptr;
	sexp_list_item head;

	//head.add_data("<any allied>");
	head.add_data("#Command");
	head.add_data("<any wingman>");
	head.add_data("<none>");

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if (Ship_info[Ships[get_ship_from_obj(ptr)].ship_info_index].is_flyable()) {
				head.add_data(Ships[ptr->instance].ship_name);
			}
		}

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_priority() {
	sexp_list_item head;

	head.add_data("High");
	head.add_data("Normal");
	head.add_data("Low");
	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_sound_environment() {
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	for (int i = 0; i < (int) EFX_presets.size(); i++) {
		head.add_data(EFX_presets[i].name.c_str());
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_sound_environment_option() {
	sexp_list_item head;

	for (int i = 0; i < Num_sound_environment_options; i++) {
		head.add_data(Sound_environment_option[i]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_adjust_audio_volume() {
	sexp_list_item head;

	for (int i = 0; i < Num_adjust_audio_options; i++) {
		head.add_data(Adjust_audio_options[i]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_builtin_hud_gauge() {
	sexp_list_item head;

	for (int i = 0; i < Num_hud_gauge_types; i++) {
		head.add_data(Hud_gauge_types[i].name);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_custom_hud_gauge()
{
	sexp_list_item head;
	SCP_unordered_set<SCP_string> all_gauges;

	for (auto &gauge : default_hud_gauges)
	{
		all_gauges.insert(gauge->getCustomGaugeName());
		head.add_data(gauge->getCustomGaugeName());
	}

	for (auto &si : Ship_info)
	{
		for (auto &gauge : si.hud_gauges)
		{
			// avoid duplicating any HUD gauges
			if (all_gauges.count(gauge->getCustomGaugeName()) == 0)
			{
				all_gauges.insert(gauge->getCustomGaugeName());
				head.add_data(gauge->getCustomGaugeName());
			}
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_any_hud_gauge()
{
	sexp_list_item head;

	head.add_list(get_listing_opf_builtin_hud_gauge());
	head.add_list(get_listing_opf_custom_hud_gauge());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_effect() {
	sexp_list_item head;

	for (SCP_vector<ship_effect>::iterator sei = Ship_effects.begin(); sei != Ship_effects.end(); ++sei) {
		head.add_data(sei->name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_explosion_option() {
	sexp_list_item head;

	for (int i = 0; i < Num_explosion_options; i++) {
		head.add_data(Explosion_option[i]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_waypoint_path() {
	SCP_list<waypoint_list>::iterator ii;
	sexp_list_item head;

	for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii) {
		head.add_data(ii->get_name());
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_point() {
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_point());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_wholeteam() {
	int i;
	sexp_list_item head;

	for (i = 0; i < (int)Iff_info.size(); i++) {
		head.add_data(Iff_info[i].iff_name);
	}

	head.add_list(get_listing_opf_ship_wing());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_shiponteam_point() {
	int i;
	sexp_list_item head;

	for (i = 0; i < (int)Iff_info.size(); i++) {
		SCP_string tmp;
		sprintf(tmp, "<any %s>", Iff_info[i].iff_name);
		std::transform(begin(tmp), end(tmp), begin(tmp), [](char c) { return (char)::tolower(c); });
		head.add_data(tmp.c_str());
	}

	head.add_list(get_listing_opf_ship_wing_point());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_point() {
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());
	head.add_list(get_listing_opf_point());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_point_or_none() {
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_ship_wing_point());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_mission_name() {
	sexp_list_item head;

	for (auto& mission : _interface->getMissionNames()) {
		head.add_data(mission.c_str());
	}

	// This code is kept until the campaign editor is added
	/*
	if ((m_mode == MODE_CAMPAIGN) && (Cur_campaign_mission >= 0)) {
		for (i = 0; i < Campaign.num_missions; i++)
			if ((i == Cur_campaign_mission)
				|| (Campaign.missions[i].level < Campaign.missions[Cur_campaign_mission].level))
				head.add_data(Campaign.missions[i].name);

	} else
		head.add_data(Mission_filename);
	 */

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_goal_name(int parent_node) {
	sexp_list_item head;

	Assert(parent_node >= 0);
	auto child = tree_nodes[parent_node].child;

	// This is used by the campaign editor to show the entries for specific missions
	SCP_string reference;
	if (child >= 0) {
		reference = tree_nodes[child].text;
	}

	for (auto& entry : _interface->getMissionGoals(reference)) {
		head.add_data(entry.c_str());
	}

	// This code is kept for reference purposes until the campaign editor is added
	/*
	if (m_mode == MODE_CAMPAIGN) {
		int child;

		Assert(parent_node >= 0);
		child = tree_nodes[parent_node].child;
		Assert(child >= 0);

		for (m = 0; m < Campaign.num_missions; m++)
			if (!stricmp(Campaign.missions[m].name, tree_nodes[child].text))
				break;

		if (m < Campaign.num_missions) {
			if (Campaign.missions[m].flags & CMISSION_FLAG_FRED_LOAD_PENDING)  // haven't loaded goal names yet.
			{
				read_mission_goal_list(m);
				Campaign.missions[m].flags &= ~CMISSION_FLAG_FRED_LOAD_PENDING;
			}

			for (i = 0; i < (int)Campaign.missions[m].goals.size(); i++)
				head.add_data(Campaign.missions[m].goals[i].name);
		}
	} else {
		for (i = 0; i < (int)Mission_goals.size(); i++)
			head.add_data(Mission_goals[i].name.c_str());
	}
	 */

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing() {
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_order_recipient() {
	sexp_list_item head;

	head.add_data("<all fighters>");

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());
	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_type() {
	unsigned int i;
	sexp_list_item head;

	for (i = 0; i < Ship_types.size(); i++) {
		head.add_data(Ship_types[i].name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_keypress() {
	sexp_list_item head;
	const auto& Default_config = Control_config_presets[0].bindings;

	for (size_t i = 0; i < Control_config.size(); ++i) {
		auto btn = Default_config[i].get_btn(CID_KEYBOARD);

		if ((btn >= -1) && !Control_config[i].disabled) {
			head.add_data(textify_scancode(btn));
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_event_name(int parent_node) {
	sexp_list_item head;

	Assert(parent_node >= 0);
	auto child = tree_nodes[parent_node].child;

	// This is used by the campaign editor to show the entries for specific missions
	SCP_string reference;
	if (child >= 0) {
		reference = tree_nodes[child].text;
	}

	for (auto& entry : _interface->getMissionEvents(reference)) {
		head.add_data(entry.c_str());
	}

	// This code is kept until the campaign editor is added
	/*
	if (m_mode == MODE_CAMPAIGN) {
		Assert(parent_node >= 0);
		auto child = tree_nodes[parent_node].child;
		Assert(child >= 0);

		for (m = 0; m < Campaign.num_missions; m++)
			if (!stricmp(Campaign.missions[m].name, tree_nodes[child].text))
				break;

		if (m < Campaign.num_missions) {
			if (Campaign.missions[m].flags & CMISSION_FLAG_FRED_LOAD_PENDING)  // haven't loaded goal names yet.
			{
				read_mission_goal_list(m);
				Campaign.missions[m].flags &= ~CMISSION_FLAG_FRED_LOAD_PENDING;
			}

			for (i = 0; i < (int)Campaign.missions[m].events.size(); i++)
				head.add_data(Campaign.missions[m].events[i].name);
		}
	} else {
		for (i = 0; i < (int)Mission_events.size(); i++)
			head.add_data(Mission_events[i].name.c_str());
	}
	 */

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ai_order() {
	sexp_list_item head;

	for (const auto& order : Player_orders) {
		head.add_data(order.hud_name.c_str());
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_skill_level() {
	int i;
	sexp_list_item head;

	for (i = 0; i < NUM_SKILL_LEVELS; i++) {
		head.add_data(Skill_level_names(i, 0));
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_cargo() {
	sexp_list_item head;

	head.add_data("Nothing");
	for (int i = 0; i < Num_cargo; i++) {
		if (stricmp(Cargo_names[i], "nothing") != 0) {
			head.add_data(Cargo_names[i]);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_string() {
	sexp_list_item head;

	head.add_data(SEXP_ANY_STRING);

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_medal_name() {
	int i;
	sexp_list_item head;

	for (i = 0; i < (int)Medals.size(); i++) {
		// don't add Rank or the Ace badges
		if ((i == Rank_medal_index) || (Medals[i].kills_needed > 0)) {
			continue;
		}
		head.add_data(Medals[i].name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_weapon_name() {
	sexp_list_item head;

	for (auto &wi : Weapon_info) {
		head.add_data(wi.name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_intel_name() {
	sexp_list_item head;

	for (auto &ii : Intel_info) {
		head.add_data(ii.name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_class_name() {
	sexp_list_item head;

	for (auto &si : Ship_info) {
		head.add_data(si.name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_huge_weapon() {
	sexp_list_item head;

	for (auto &wi : Weapon_info) {
		if (wi.wi_flags[Weapon::Info_Flags::Huge]) {
			head.add_data(wi.name);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_not_player() {
	object* ptr;
	sexp_list_item head;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_SHIP) {
			head.add_data(Ships[ptr->instance].ship_name);
		}

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_ship_or_none() {
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_ship());

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_subsystem_or_none(int parent_node, int arg_index) {
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_subsystem(parent_node, arg_index));

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_subsys_or_generic(int parent_node, int arg_index) {
	sexp_list_item head;

	head.add_data(SEXP_ALL_ENGINES_STRING);
	head.add_data(SEXP_ALL_TURRETS_STRING);
	head.add_list(get_listing_opf_subsystem(parent_node, arg_index));

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_jump_nodes() {
	sexp_list_item head;

	SCP_list<CJumpNode>::iterator jnp;
	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
		head.add_data(jnp->GetName());
	}

	return head.next;
}

// creates list of Sexp_variables
sexp_list_item* sexp_tree::get_listing_opf_variable_names() {
	int i;
	sexp_list_item head;

	for (i = 0; i < MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			head.add_data(Sexp_variables[i].variable_name);
		}
	}

	return head.next;
}

// get default skybox model name
sexp_list_item* sexp_tree::get_listing_opf_skybox_model() {

	sexp_list_item head;
	head.add_data("default");
	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_skybox_flags() {
	sexp_list_item head;
	int i;

	for (i = 0; i < Num_skybox_flags; ++i) {
		head.add_data(Skybox_flags[i]);
	}
	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_background_bitmap() {
	sexp_list_item head;
	int i;

	for (i = 0; i < stars_get_num_entries(false, true); i++) {
		head.add_data(stars_get_name_FRED(i, false));
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_sun_bitmap() {
	sexp_list_item head;
	int i;

	for (i = 0; i < stars_get_num_entries(true, true); i++) {
		head.add_data(stars_get_name_FRED(i, true));
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_nebula_storm_type() {
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (size_t i = 0; i < Storm_types.size(); i++) {
		head.add_data(Storm_types[i].name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_nebula_poof() {
	sexp_list_item head;

	for (poof_info &pf : Poof_info) {
		head.add_data(pf.name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_turret_target_order() {
	int i;
	sexp_list_item head;

	for (i = 0; i < NUM_TURRET_ORDER_TYPES; i++) {
		head.add_data(Turret_target_order_names[i]);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_post_effect() {
	unsigned int i;
	sexp_list_item head;

	SCP_vector<SCP_string> ppe_names;
	gr_get_post_process_effect_names(ppe_names);
	for (i = 0; i < ppe_names.size(); i++) {
		head.add_data(ppe_names[i].c_str());
	}
	head.add_data("lightshafts");

	return head.next;
}


sexp_list_item* sexp_tree::get_listing_opf_turret_target_priorities() {
	size_t t;
	sexp_list_item head;

	for (t = 0; t < Ai_tp_list.size(); t++) {
		head.add_data(Ai_tp_list[t].name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_armor_type() {
	size_t t;
	sexp_list_item head;
	head.add_data(SEXP_NONE_STRING);
	for (t = 0; t < Armor_types.size(); t++) {
		head.add_data(Armor_types[t].GetNamePtr());
	}
	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_damage_type() {
	size_t t;
	sexp_list_item head;
	head.add_data(SEXP_NONE_STRING);
	for (t = 0; t < Damage_types.size(); t++) {
		head.add_data(Damage_types[t].name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_animation_type() {
	sexp_list_item head;

	for (const auto &animation_type_name: animation::Animation_types) {
		head.add_data(animation_type_name.second.first);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_hud_elements() {
	sexp_list_item head;
	head.add_data("warpout");

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_weapon_banks() {
	sexp_list_item head;
	head.add_data(SEXP_ALL_BANKS_STRING);

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_mission_moods() {
	sexp_list_item head;
	for (SCP_vector<SCP_string>::iterator iter = Builtin_moods.begin(); iter != Builtin_moods.end(); ++iter) {
		head.add_data(iter->c_str());
	}

	return head.next;
}

template <typename M, typename T, typename PTM>
static void add_flag_name_helper(M& flag_name_map, sexp_list_item& head, T flag_name_array[], PTM T::* member, size_t flag_name_count)
{
	for (size_t i = 0; i < flag_name_count; i++)
	{
		auto name = flag_name_array[i].*member;
		if (flag_name_map.count(name) == 0)
		{
			head.add_data(name);
			flag_name_map.insert(name);
		}
	}
}

sexp_list_item *sexp_tree::get_listing_opf_ship_flags()
{
	sexp_list_item head;
	// prevent duplicate names, comparing case-insensitively
	SCP_unordered_set<SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to> all_flags;

	add_flag_name_helper(all_flags, head, Object_flag_names, &obj_flag_name::flag_name, (size_t)Num_object_flag_names);
	add_flag_name_helper(all_flags, head, Ship_flag_names, &ship_flag_name::flag_name, Num_ship_flag_names);
	add_flag_name_helper(all_flags, head, Parse_object_flags, &flag_def_list_new<Mission::Parse_Object_Flags>::name, Num_parse_object_flags);
	add_flag_name_helper(all_flags, head, Ai_flag_names, &ai_flag_name::flag_name, (size_t)Num_ai_flag_names);

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_wing_flags() {
	size_t i;
	sexp_list_item head;
	// wing flags
	for (i = 0; i < Num_wing_flag_names; i++) {
		head.add_data(Wing_flag_names[i].flag_name);
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_team_colors() {
	sexp_list_item head;
	head.add_data("None");
	for (SCP_map<SCP_string, team_color>::iterator tcolor = Team_Colors.begin(); tcolor != Team_Colors.end();
		 ++tcolor) {
		head.add_data(tcolor->first.c_str());
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_nebula_patterns() {
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (int i = 0; i < MAX_NEB2_BITMAPS; i++) {
		if (strlen(Neb2_bitmap_filenames[i]) > 0) {
			head.add_data(Neb2_bitmap_filenames[i]);
		}
	}

	return head.next;
}

sexp_list_item* sexp_tree::get_listing_opf_game_snds() {
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (SCP_vector<game_snd>::iterator iter = Snds.begin(); iter != Snds.end(); ++iter) {
		if (!can_construe_as_integer(iter->name.c_str())) {
			head.add_data(iter->name.c_str());
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_fireball()
{
	sexp_list_item head;

	for (int i = 0; i < Num_fireball_types; ++i)
	{
		char *unique_id = Fireball_info[i].unique_id;

		if (strlen(unique_id) > 0)
			head.add_data(unique_id);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_species()	// NOLINT
{
	sexp_list_item head;

	for (auto &species : Species_info)
		head.add_data(species.species_name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_language()	// NOLINT
{
	sexp_list_item head;

	for (auto &lang: Lcl_languages)
		head.add_data(lang.lang_name);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_functional_when_eval_type()	// NOLINT
{
	sexp_list_item head;

	for (int i = 0; i < Num_functional_when_eval_types; i++)
		head.add_data(Functional_when_eval_type[i]);

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_animation_name(int parent_node)
{
	int op, child, sh;
	sexp_list_item head;

	Assert(parent_node >= 0);

	// get the operator type of the node
	op = get_operator_const(tree_nodes[parent_node].text);

	// first child node
	child = tree_nodes[parent_node].child;
	Assert(child >= 0);
	sh = ship_name_lookup(tree_nodes[child].text, 1);

	switch(op) {
		case OP_TRIGGER_ANIMATION_NEW:
		case OP_STOP_LOOPING_ANIMATION: {
			child = tree_nodes[child].next;
			auto triggerType = animation::anim_match_type(tree_nodes[child].text);

			for(const auto& animation : Ship_info[Ships[sh].ship_info_index].animations.getRegisteredTriggers()){
				if(animation.type != triggerType)
					continue;

				if(animation.subtype != animation::ModelAnimationSet::SUBTYPE_DEFAULT) {
					int animationSubtype = animation.subtype;

					if(animation.type == animation::ModelAnimationTriggerType::DockBayDoor){
						//Because of the old system, this is this weird exception. Don't explicitly suggest the NOT doors, as they cannot be explicitly targeted anyways
						if(animation.subtype < 0)
							continue;
						
						animationSubtype--;
					}

					head.add_data(std::to_string(animationSubtype).c_str());
				}
				else
					head.add_data(animation.name.c_str());
			}
			
			break;
		}

		case OP_UPDATE_MOVEABLE:
			for(const auto& moveable : Ship_info[Ships[sh].ship_info_index].animations.getRegisteredMoveables())
				head.add_data(moveable.c_str());

			break;
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_sexp_containers(ContainerType con_type)
{
	sexp_list_item head;

	for (const auto &container : get_all_sexp_containers()) {
		if (any(container.type & con_type)) {
			head.add_data(container.container_name.c_str(), (SEXPT_CONTAINER_NAME | SEXPT_STRING | SEXPT_VALID));
		}
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_listing_opf_wing_formation()	// NOLINT
{
	sexp_list_item head;

	head.add_data("Default");
	for (const auto &formation : Wing_formations)
		head.add_data(formation.name);

	return head.next;
}

sexp_list_item *sexp_tree::get_container_modifiers(int con_data_node) const
{
	Assertion(con_data_node != -1, "Attempt to get modifiers for invalid container node. Please report!");
	Assertion(tree_nodes[con_data_node].type & SEXPT_CONTAINER_DATA,
		"Attempt to get modifiers for non-container data node %s. Please report!",
		tree_nodes[con_data_node].text);

	const auto *p_container = get_sexp_container(tree_nodes[con_data_node].text);
	Assertion(p_container,
		"Attempt to get modifiers for unknown container %s. Please report!",
		tree_nodes[con_data_node].text);
	const auto &container = *p_container;

	sexp_list_item head;
	sexp_list_item *list = nullptr;

	if (container.is_list()) {
		list = get_list_container_modifiers();
	} else if (container.is_map()) {
		// start the list with "<argument>" if relevant
		if (is_node_eligible_for_special_argument(con_data_node) &&
			any(container.type & ContainerType::STRING_KEYS)) {
			head.add_data(SEXP_ARGUMENT_STRING, (SEXPT_VALID | SEXPT_STRING | SEXPT_MODIFIER));
		}

		list = get_map_container_modifiers(con_data_node);
	} else {
		UNREACHABLE("Unknown container type %d", (int)p_container->type);
	}

	if (list) {
		head.add_list(list);
	}

	return head.next;
}

sexp_list_item *sexp_tree::get_list_container_modifiers()
{
	sexp_list_item head;

	for (const auto& modifier : get_all_list_modifiers()) {
		head.add_data(modifier.name, SEXPT_VALID | SEXPT_MODIFIER | SEXPT_STRING);
	}

	return head.next;
}

// FIXME TODO: if you use this function with remove-from-map SEXP, don't use SEXPT_MODIFIER
sexp_list_item *sexp_tree::get_map_container_modifiers(int con_data_node) const
{
	sexp_list_item head;

	Assertion(tree_nodes[con_data_node].type & SEXPT_CONTAINER_DATA,
		"Found map modifier for non-container data node %s. Please report!",
		tree_nodes[con_data_node].text);

	const auto *p_container = get_sexp_container(tree_nodes[con_data_node].text);
	Assertion(p_container != nullptr,
		"Found map modifier for unknown container %s. Please report!",
		tree_nodes[con_data_node].text);

	const auto &container = *p_container;
	Assertion(container.is_map(),
		"Found map modifier for non-map container %s with type %d. Please report!",
		tree_nodes[con_data_node].text,
		(int)container.type);

	int type = SEXPT_VALID | SEXPT_MODIFIER;
	if (any(container.type & ContainerType::STRING_KEYS)) {
		type |= SEXPT_STRING;
	} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
		type |= SEXPT_NUMBER;
	} else {
		UNREACHABLE("Unknown map container key type %d", (int)container.type);
	}

	for (const auto &kv_pair : container.map_data) {
		head.add_data(kv_pair.first.c_str(), type);
	}

	return head.next;
}

// get potential options for container multidimensional modifiers
// the value could be either string or number, checked in-mission
sexp_list_item *sexp_tree::get_container_multidim_modifiers(int con_data_node) const
{
	Assertion(con_data_node != -1,
		"Attempt to get multidimensional modifiers for invalid container node. Please report!");
	Assertion(tree_nodes[con_data_node].type & SEXPT_CONTAINER_DATA,
		"Attempt to get multidimensional modifiers for non-container data node %s. Please report!",
		tree_nodes[con_data_node].text);

	sexp_list_item head;

	if (is_node_eligible_for_special_argument(con_data_node)) {
		head.add_data(SEXP_ARGUMENT_STRING, (SEXPT_VALID | SEXPT_STRING | SEXPT_MODIFIER));
	}

	// the FREDder might want to use a list modifier
	sexp_list_item *list = get_list_container_modifiers();

	head.add_list(list);

	return head.next;
}

sexp_list_item* sexp_tree::check_for_dynamic_sexp_enum(int opf)
{
	sexp_list_item head;

	int item = opf - First_available_opf_id;

	if (item < (int)Dynamic_enums.size()) {

		for (const SCP_string& enum_item : Dynamic_enums[item].list) {
			head.add_data(enum_item.c_str());
		}
		return head.next;
	} else {
		// else if opf is invalid do this
		UNREACHABLE("Unhandled SEXP argument type!"); // unknown OPF code
		return nullptr;
	}
}

// given a node's parent, check if node is eligible for being used with the special argument
bool sexp_tree::is_node_eligible_for_special_argument(int parent_node) const
{
	Assertion(parent_node != -1,
		"Attempt to access invalid parent node for special arg eligibility check. Please report!");

	const int w_arg = find_ancestral_argument_number(OP_WHEN_ARGUMENT, parent_node);
	const int e_arg = find_ancestral_argument_number(OP_EVERY_TIME_ARGUMENT, parent_node);
	return w_arg >= 1 || e_arg >= 1; /* || the same for any future _ARGUMENT sexps */
}

// Deletes sexp_variable from sexp_tree.
// resets tree to not include given variable, and resets text and type
void sexp_tree::delete_sexp_tree_variable(const char* var_name) {
	char search_str[64];
	char replace_text[TOKEN_LENGTH];

	sprintf(search_str, "%s(", var_name);

	// store old item index
	int old_item_index = item_index;

	for (uint idx = 0; idx < tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(tree_nodes[idx].text, search_str) != NULL) {

				// check type is number or string
				Assert((tree_nodes[idx].type & SEXPT_NUMBER) || (tree_nodes[idx].type & SEXPT_STRING));

				// reset type as not variable
				int type = tree_nodes[idx].type &= ~SEXPT_VARIABLE;

				// reset text
				if (tree_nodes[idx].type & SEXPT_NUMBER) {
					strcpy_s(replace_text, "number");
				} else {
					strcpy_s(replace_text, "string");
				}

				// set item_index and replace data
				setCurrentItemIndex(idx);
				replace_data(replace_text, type);
			}
		}
	}

	// restore item_index
	setCurrentItemIndex(old_item_index);
}


// Modify sexp_tree for a change in sexp_variable (name, type, or default value)
void sexp_tree::modify_sexp_tree_variable(const char* old_name, int sexp_var_index) {
	char search_str[64];
	int type;

	Assert(Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_SET);
	Assert((Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER)
			   || (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING));

	// Get type for sexp_tree node
	if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
		type = (SEXPT_NUMBER | SEXPT_VALID);
	} else {
		type = (SEXPT_STRING | SEXPT_VALID);
	}

	// store item index;
	int old_item_index = item_index;

	// Search string in sexp_tree nodes
	sprintf(search_str, "%s(", old_name);

	for (uint idx = 0; idx < tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(tree_nodes[idx].text, search_str) != NULL) {
				// temp set item_index
				item_index = idx;

				// replace variable data
				replace_variable_data(sexp_var_index, (type | SEXPT_VARIABLE));
			}
		}
	}

	// restore item_index
	item_index = old_item_index;
}


// convert from item_index to sexp_variable index, -1 if not
int sexp_tree::get_item_index_to_var_index() {
	// check valid item index and node is a variable
	if ((item_index > 0) && (tree_nodes[item_index].type & SEXPT_VARIABLE)) {

		return get_tree_name_to_sexp_variable_index(tree_nodes[item_index].text);
	} else {
		return -1;
	}
}

int sexp_tree::get_tree_name_to_sexp_variable_index(const char* tree_name) {
	char var_name[TOKEN_LENGTH];

	auto chars_to_copy = strcspn(tree_name, "(");
	Assert(chars_to_copy < TOKEN_LENGTH - 1);

	// Copy up to '(' and add null termination
	strncpy(var_name, tree_name, chars_to_copy);
	var_name[chars_to_copy] = '\0';

	// Look up index
	return get_index_sexp_variable_name(var_name);
}

int sexp_tree::get_variable_count(const char* var_name) {
	uint idx;
	int count = 0;
	char compare_name[64];

	// get name to compare
	strcpy_s(compare_name, var_name);
	strcat_s(compare_name, "(");

	// look for compare name
	for (idx = 0; idx < tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(tree_nodes[idx].text, compare_name)) {
				count++;
			}
		}
	}

	return count;
}

// Returns the number of times a variable with this name has been used by player loadout
int sexp_tree::get_loadout_variable_count(int var_index) {
	// we shouldn't be being passed the index of variables that do not exist
	Assert (var_index >= 0 && var_index < MAX_SEXP_VARIABLES);

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

bool sexp_tree::is_container_name_opf_type(const int op_type)
{
	return (op_type == OPF_CONTAINER_NAME) ||
		(op_type == OPF_LIST_CONTAINER_NAME) ||
		(op_type == OPF_MAP_CONTAINER_NAME);
}

void sexp_tree::initializeEditor(::fso::fred::Editor* edit, SexpTreeEditorInterface* editorInterface) {
	if (editorInterface == nullptr) {
		// If there is no special interface then we supply the default implementation
		_owned_interface.reset(new SexpTreeEditorInterface());
		editorInterface = _owned_interface.get();
	}

	_editor = edit;
	_interface = editorInterface;
}
void sexp_tree::customMenuHandler(const QPoint& pos) {
	QTreeWidgetItem* h = this->itemAt(pos);

	if (h == nullptr) {
		return;
	}



	auto menu = buildContextMenu(h);

	menu->exec(mapToGlobal(pos));
}
std::unique_ptr<QMenu> sexp_tree::buildContextMenu(QTreeWidgetItem* h) {
	int i, j, z, count, op, add_type, replace_type, type, subcategory_id;
	sexp_list_item* list;

	add_instance = replace_instance = -1;
	Assert((int) op_menu.size() < MAX_OP_MENUS);
	Assert((int) op_submenu.size() < MAX_SUBMENUS);

	std::unique_ptr<QMenu> popup_menu(new QMenu(tr("Edit SEXP tree")));

	auto delete_act =
		popup_menu->addAction(tr("&Delete Item"), this, [this]() { deleteActionHandler(); }, QKeySequence::Delete);
	auto edit_data_act = popup_menu->addAction(tr("&Edit Data"), this, [this]() { editDataActionHandler(); });
	popup_menu->addAction(tr("Expand All"), this, [this]() { expand_branch(currentItem()); });

	popup_menu->addSection(tr("Copy operations"));
	auto cut_act = popup_menu->addAction(tr("Cut"), this, [this]() { cutActionHandler(); }, QKeySequence::Cut);
	cut_act->setEnabled(false);
	auto copy_act = popup_menu->addAction(tr("Copy"), this, [this]() { copyActionHandler(); }, QKeySequence::Copy);
	auto paste_act = popup_menu->addAction(tr("Paste"), this, [this]() { pasteActionHandler(); }, QKeySequence::Paste);
	paste_act->setEnabled(false);

	popup_menu->addSection(tr("Add"));
	auto add_op_menu = popup_menu->addMenu(tr("Add Operator"));

	auto add_data_menu = popup_menu->addMenu(tr("Add Data"));
	auto add_number_act = add_data_menu->addAction(tr("Number"), this, [this]() { addNumberDataHandler(); });
	add_number_act->setEnabled(false);
	auto add_string_act = add_data_menu->addAction(tr("String"), this, [this]() { addStringDataHandler(); });
	add_string_act->setEnabled(false);
	add_data_menu->addSeparator();

	popup_menu->addSeparator();
	auto add_paste_act = popup_menu->addAction(tr("Add Paste"), this, [this]() { addPasteActionHandler(); });
	add_paste_act->setEnabled(false);
	popup_menu->addSeparator();

	auto insert_op_menu = popup_menu->addMenu(tr("Insert Operator"));
	popup_menu->addSeparator();

	auto replace_op_menu = popup_menu->addMenu(tr("Replace Operator"));

	auto replace_data_menu = popup_menu->addMenu(tr("Replace Data"));
	auto replace_number_act =
		replace_data_menu->addAction(tr("Number"), this, [this]() { replaceNumberDataHandler(); });
	replace_number_act->setEnabled(false);
	auto replace_string_act =
		replace_data_menu->addAction(tr("String"), this, [this]() { replaceStringDataHandler(); });
	replace_string_act->setEnabled(false);
	replace_data_menu->addSeparator();

	popup_menu->addSection("Variables");

	popup_menu->addAction(tr("Add Variable"), this, []() {});
	auto modify_variable_act = popup_menu->addAction(tr("Modify Variable"), this, []() {});

	auto replace_variable_menu = popup_menu->addMenu(tr("Replace Variable"));

	popup_menu->addSeparator();

	popup_menu->addSection("Containers");

	auto add_modify_container_act = popup_menu->addAction(tr("Add/Modify Container"), this, []() {});
	add_modify_container_act->setEnabled(false);
	auto replace_container_name_menu = popup_menu->addMenu(tr("Replace Container Name"));
	auto replace_container_data_menu = popup_menu->addMenu(tr("Replace Container Data"));

	update_help(h);
	//SelectDropTarget(h);  // WTF: Why was this here???

	// get item_index
	item_index = -1;
	for (i = 0; i < (int) tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			setCurrentItemIndex(i);
			break;
		}
	}

	// check not root (-1)
	if (item_index >= 0) {
		// get type of sexp_tree item clicked on
		type = get_type(h);

		int parent = tree_nodes[item_index].parent;
		if (parent >= 0) {
			op = get_operator_index(tree_nodes[parent].text);
			Assertion(op >= 0 || tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
				"Encountered unknown SEXP operator %s. Please report!",
				tree_nodes[parent].text);
			int first_arg = tree_nodes[parent].child;

			// get arg count of item to replace
			Replace_count = 0;
			int temp = first_arg;
			while (temp != item_index) {
				Replace_count++;
				temp = tree_nodes[temp].next;

				// DB - added 3/4/99
				if (temp == -1) {
					break;
				}
			}

			int op_type = 0;

			if (op >= 0) {
				op_type =
					query_operator_argument_type(op, Replace_count); // check argument type at this position
			} else {
				Assertion(tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
					"Unknown SEXP operator %s. Please report!",
					tree_nodes[parent].text);
				const auto *p_container = get_sexp_container(tree_nodes[parent].text);
				Assertion(p_container != nullptr,
					"Found modifier for unknown container %s. Please report!",
					tree_nodes[parent].text);
				op_type = p_container->opf_type;
			}
			Assertion(op_type > 0,
				"Could not find valid operand type for node %s with type %d (op %d). Please report!",
				tree_nodes[parent].text,
				tree_nodes[parent].type,
				op);

			// special case don't allow replace data for variable names
			// Goober5000 - why?  the only place this happens is when replacing the ambiguous argument in
			// modify-variable with a variable, which seems legal enough.
			//if (op_type != OPF_AMBIGUOUS) {

			// Goober5000 - given the above, we have to figure out what type this stands for
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

			// jg18 - container values (container data/map keys) can be anything
			// the type is checked in check_sexp_syntax()
			if (op_type == OPF_CONTAINER_VALUE)
			{
				type = SEXPT_NUMBER | SEXPT_STRING;
			}

			if ((type & SEXPT_STRING) || (type & SEXPT_NUMBER)) {

				int max_sexp_vars = MAX_SEXP_VARIABLES;
				// prevent collisions in id numbers: ID_VARIABLE_MENU + 512 = ID_ADD_MENU
				Assert(max_sexp_vars < 512);

				for (int idx = 0; idx < max_sexp_vars; idx++) {
					if (Sexp_variables[idx].type & SEXP_VARIABLE_SET) {
						// skip block variables
						if (Sexp_variables[idx].type & SEXP_VARIABLE_BLOCK) {
							continue;
						}

						auto disabled = true;
						// maybe gray flag MF_GRAYED

						// get type -- gray "string" or number accordingly
						if (type & SEXPT_STRING) {
							if (Sexp_variables[idx].type & SEXP_VARIABLE_STRING) {
								disabled = false;
							}
						}
						if (type & SEXPT_NUMBER) {
							if (Sexp_variables[idx].type & SEXP_VARIABLE_NUMBER) {
								disabled = false;
							}
						}

						// if modify-variable and changing variable, enable all variables
						if (op_type == OPF_VARIABLE_NAME) {
							Modify_variable = 1;
							disabled = false;
						} else {
							Modify_variable = 0;
						}

						// enable navsystem always
						if (op_type == OPF_NAV_POINT) {
							disabled = false;
						}

						// enable all for container multidimensionality
						if ((type & SEXPT_MODIFIER) && Replace_count > 0) {
							disabled = false;
						}

						char buf[128];
						// append list of variable names and values
						// set id as ID_VARIABLE_MENU + idx
						sprintf(buf, "%s (%s)", Sexp_variables[idx].variable_name, Sexp_variables[idx].text);

						auto action = replace_variable_menu->addAction(QString::fromUtf8(buf),
																	   this,
																	   [this, idx]() { handleReplaceVariableAction(idx); });
						action->setEnabled(!disabled);
					}
				}

				// Replace Container Name submenu
				if (is_container_name_opf_type(op_type) || (op_type == OPF_DATA_OR_STR_CONTAINER)) {
					const auto &containers = get_all_sexp_containers();
					for (int idx = 0; idx < (int)containers.size(); ++idx) {
						const auto &container = containers[idx];

						auto disabled = true;
						// maybe gray flag MF_GRAYED

						if (op_type == OPF_CONTAINER_NAME) {
							// allow all containers
							disabled = false;
						} else if ((op_type == OPF_LIST_CONTAINER_NAME) && container.is_list()) {
							disabled = false;
						} else if ((op_type == OPF_MAP_CONTAINER_NAME) && container.is_map()) {
							disabled = false;
						} else if (op_type == OPF_DATA_OR_STR_CONTAINER && container.is_of_string_type()) {
							disabled = false;
						}

						auto action =
							replace_container_name_menu->addAction(QString::fromStdString(container.container_name),
								this,
								[this, idx]() { handleReplaceContainerNameAction(idx); });
						action->setEnabled(!disabled);
					}
				}

				// Replace Container Data submenu
				// disallowed on variable-type SEXP args, to prevent FSO/FRED crashes
				// also disallowed for special argument options (not supported for now)
				if (op_type != OPF_VARIABLE_NAME && op_type != OPF_ANYTHING && op_type != OPF_DATA_OR_STR_CONTAINER) {
					const auto &containers = get_all_sexp_containers();
					for (int idx = 0; idx < (int)containers.size(); ++idx) {
						const auto &container = containers[idx];
						auto disabled = true;
						// maybe gray flag MF_GRAYED

						if ((type & SEXPT_STRING) && any(container.type & ContainerType::STRING_DATA)) {
							disabled = false;
						}

						if ((type & SEXPT_NUMBER) && any(container.type & ContainerType::NUMBER_DATA)) {
							disabled = false;
						}

						// enable all for container multidimensionality
						if ((tree_nodes[item_index].type & SEXPT_MODIFIER) && Replace_count > 0) {
							disabled = false;
						}

						auto action =
							replace_container_data_menu->addAction(QString::fromStdString(container.container_name),
								this,
								[this, idx]() { handleReplaceContainerDataAction(idx); });
						action->setEnabled(!disabled);
					}
				}
			}
			//}
		}
	}

	// can't modify if no variables
	modify_variable_act->setEnabled(sexp_variable_count() > 0);

	// add popup menus for all the operator categories
	QMenu* add_op_submenu[MAX_OP_MENUS];
	QMenu* replace_op_submenu[MAX_OP_MENUS];
	QMenu* insert_op_submenu[MAX_OP_MENUS];
	for (i = 0; i < (int) op_menu.size(); i++) {
		add_op_submenu[i] = add_op_menu->addMenu(QString::fromStdString(op_menu[i].name.c_str()));
		replace_op_submenu[i] = replace_op_menu->addMenu(QString::fromStdString(op_menu[i].name.c_str()));
		insert_op_submenu[i] = insert_op_menu->addMenu(QString::fromStdString(op_menu[i].name));
	}

	// add all the submenu items first
	QMenu* add_op_subcategory_menu[MAX_SUBMENUS];
	QMenu* replace_op_subcategory_menu[MAX_SUBMENUS];
	QMenu* insert_op_subcategory_menu[MAX_SUBMENUS];
	for (i = 0; i < (int) op_submenu.size(); i++) {
		for (j = 0; j < (int) op_menu.size(); j++) {
			if (op_menu[j].id == category_of_subcategory(op_submenu[i].id)) {
				add_op_subcategory_menu[i] = add_op_submenu[j]->addMenu(QString::fromStdString(op_submenu[i].name));
				replace_op_subcategory_menu[i] =
					replace_op_submenu[j]->addMenu(QString::fromStdString(op_submenu[i].name));
				insert_op_subcategory_menu[i] =
					insert_op_submenu[j]->addMenu(QString::fromStdString(op_submenu[i].name));
				break;    // only 1 category valid
			}
		}
	}

	// The MFC code could use some internal WinAPI IDs for keeping this mapping but that is not available in Qt so we
	// need to do this ourself. This could be improved by applying the actions for the operator actions when the action
	// is added.
	std::unordered_map<int, QAction*> operator_action_mapping;

	// add operator menu items to the various CATEGORY submenus they belong in
	for (i = 0; i < (int) Operators.size(); i++) {
		// add only if it is not in a subcategory
		subcategory_id = get_subcategory(Operators[i].value);
		if (subcategory_id == OP_SUBCATEGORY_NONE) {
			// put it in the appropriate menu
			for (j = 0; j < (int) op_menu.size(); j++) {
				if (op_menu[j].id == get_category(Operators[i].value)) {
					switch (Operators[i].value) {
// Commented out by Goober5000 to allow these operators to be selectable
/*#ifdef NDEBUG
						// various campaign operators
						case OP_WAS_PROMOTION_GRANTED:
						case OP_WAS_MEDAL_GRANTED:
						case OP_GRANT_PROMOTION:
						case OP_GRANT_MEDAL:
						case OP_TECH_ADD_SHIP:
						case OP_TECH_ADD_WEAPON:
						case OP_TECH_ADD_INTEL_XSTR:
						case OP_TECH_REMOVE_INTEL_XSTR:
						case OP_TECH_RESET_TO_DEFAULT:
#endif*/
						// unlike the above operators, these are deprecated
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
						j = (int) op_menu.size();    // don't allow these operators to be visible
						break;
					}

					if (j < (int) op_menu.size()) {
						auto add_act =
							add_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text), this, [this, i]() {
								add_or_replace_operator(i, 0);
							});
						add_act->setEnabled(false);
						operator_action_mapping.insert(std::make_pair(Operators[i].value, add_act));

						auto replace_act = replace_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
																			this,
																			[this, i]() {
																				add_or_replace_operator(i, 1);
																			});
						replace_act->setEnabled(false);
						operator_action_mapping.insert(std::make_pair(Operators[i].value | OP_REPLACE_FLAG,
																	  replace_act));

						auto insert_act = insert_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
																		  this,
																		  [this, i]() {
																			  insertOperatorAction(i);
																		  });
						insert_act->setEnabled(true);
						operator_action_mapping.insert(std::make_pair(Operators[i].value | OP_INSERT_FLAG, insert_act));
					}

					break;    // only 1 category valid
				}
			}
		}
			// if it is in a subcategory, handle it
		else {
			// put it in the appropriate submenu
			for (j = 0; j < (int) op_submenu.size(); j++) {
				if (op_submenu[j].id == subcategory_id) {
					switch (Operators[i].value) {
// Commented out by Goober5000 to allow these operators to be selectable
/*#ifdef NDEBUG
						// various campaign operators
						case OP_WAS_PROMOTION_GRANTED:
						case OP_WAS_MEDAL_GRANTED:
						case OP_GRANT_PROMOTION:
						case OP_GRANT_MEDAL:
						case OP_TECH_ADD_SHIP:
						case OP_TECH_ADD_WEAPON:
						case OP_TECH_ADD_INTEL_XSTR:
						case OP_TECH_REMOVE_INTEL_XSTR:
						case OP_TECH_RESET_TO_DEFAULT:
#endif*/
						// unlike the above operators, these are deprecated
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
						j = (int) op_submenu.size();    // don't allow these operators to be visible
						break;
					}

					if (j < (int) op_submenu.size()) {
						auto add_act = add_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
																			 this,
																			 [this, i]() {
																				 add_or_replace_operator(i, 0);
																			 });
						add_act->setEnabled(false);
						operator_action_mapping.insert(std::make_pair(Operators[i].value, add_act));

						auto replace_act =
							replace_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
																	  this,
																	  [this, i]() {
																		  add_or_replace_operator(i, 1);
																	  });
						replace_act->setEnabled(false);
						operator_action_mapping.insert(std::make_pair(Operators[i].value | OP_REPLACE_FLAG,
																	  replace_act));

						auto insert_act =
							insert_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
																	 this,
																	 [this, i]() {
																		 insertOperatorAction(i);
																	 });
						insert_act->setEnabled(true);
						operator_action_mapping.insert(std::make_pair(Operators[i].value | OP_INSERT_FLAG, insert_act));
					}

					break;    // only 1 subcategory valid
				}
			}
		}
	}

	// find local index (i) of current item (from its handle)
	for (i = 0; i < (int) tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			break;
		}
	}

	// special case: item is a ROOT node, and a label that can be edited (not an item in the sexp tree)
	if ((item_index == -1) && _interface->getFlags()[TreeFlags::LabeledRoot]) {
		edit_data_act->setEnabled(_interface->getFlags()[TreeFlags::RootEditable]);

		// disable copy, insert op
		copy_act->setEnabled(false);
		insert_op_menu->setEnabled(false);
		/*
		for (j = 0; j < (int) Operators.size(); j++) {
			menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
		}
		*/

		util::propagate_disabled_status(popup_menu.get());
		return popup_menu;
	}

	Assert(item_index != -1);  // handle not found, which should be impossible.
	if (!(tree_nodes[item_index].flags & EDITABLE)) {
		edit_data_act->setEnabled(false);
	}

	if (tree_nodes[item_index].parent == -1) {  // root node
		delete_act->setEnabled(false); // can't delete the root item.
	}

/*		if ((tree_nodes[item_index].flags & OPERAND) && (tree_nodes[item_index].flags & EDITABLE))  // expandable?
		menu.EnableMenuItem(ID_SPLIT_LINE, MF_ENABLED);

	z = tree_nodes[item_index].child;
	if (z != -1 && tree_nodes[z].next == -1 && tree_nodes[z].child == -1)
		menu.EnableMenuItem(ID_SPLIT_LINE, MF_ENABLED);

	z = tree_nodes[tree_nodes[item_index].parent].child;
	if (z != -1 && tree_nodes[z].next == -1 && tree_nodes[z].child == -1)
		menu.EnableMenuItem(ID_SPLIT_LINE, MF_ENABLED);*/

	// change enabled status of 'add' type menu options.
	add_type = 0;

	// container multidimensionality
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		// using local var for add count to avoid breaking implicit assumptions about Add_count
		const int modifier_node = tree_nodes[item_index].child;
		Assertion(modifier_node != -1,
			"No modifier found for container data node %s. Please report!",
			tree_nodes[item_index].text);
		const int modifier_add_count = count_args(modifier_node);

		const auto *p_container = get_sexp_container(tree_nodes[item_index].text);
		Assertion(p_container,
			"Found modifier for unknown container %s. Please report!",
			tree_nodes[item_index].text);

		if (modifier_add_count == 1 && p_container->is_list() &&
			get_list_modifier(tree_nodes[modifier_node].text) == ListModifier::AT_INDEX) {
			// only valid value is a list index
			add_type = OPR_NUMBER;
			add_number_act->setEnabled(true);
		} else {
			// container multidimensionality
			add_type = OPR_STRING;

			// the next thing we want to add could literally be any legal key for any map or the legal entries for a list container
			// so give the FREDder a hand and offer the list modifiers, but only the FREDder can know if they're relevant
			list = get_container_multidim_modifiers(item_index);

			if (list) {
				sexp_list_item *ptr = nullptr;

				int data_idx = 0;
				ptr = list;
				while (ptr) {
					if (ptr->op >= 0) {
						// enable operators with correct return type
						auto iter = operator_action_mapping.find(Operators[ptr->op].value);
						if (iter != operator_action_mapping.end()) {
							iter->second->setEnabled(true);
						}
					} else {
						// add data
						add_data_menu->addAction(QString::fromStdString(ptr->text),
							this,
							[this, data_idx]() { addReplaceTypedDataHandler(data_idx, false); });
					}

					data_idx++;
					ptr = ptr->next;
				}
			}

			add_number_act->setEnabled(true);
			add_string_act->setEnabled(true);
		}
	} else if (tree_nodes[item_index].flags & OPERAND) {
		add_type = OPR_STRING;
		int child = tree_nodes[item_index].child;
		Add_count = count_args(child);
		op = get_operator_index(tree_nodes[item_index].text);
		Assert(op >= 0);

		// get listing of valid argument values and add to menus
		type = query_operator_argument_type(op, Add_count);
		list = get_listing_opf(type, item_index, Add_count);
		if (list) {
			sexp_list_item* ptr;

			int data_idx = 0;
			ptr = list;
			while (ptr) {
				if (ptr->op >= 0) {
					// enable operators with correct return type
					auto iter = operator_action_mapping.find(Operators[ptr->op].value);
					if (iter != operator_action_mapping.end()) {
						iter->second->setEnabled(true);
					}
				} else {
					// add data
					add_data_menu->addAction(QString::fromStdString(ptr->text),
											 this,
											 [this, data_idx]() { addReplaceTypedDataHandler(data_idx, false); });
				}

				data_idx++;
				ptr = ptr->next;
			}
		}

		// special handling for the non-string formats
		if (type == OPF_NONE) {  // an argument can't be added
			add_type = 0;

		} else if (type == OPF_NULL) {  // arguments with no return values
			add_type = OPR_NULL;

			// Goober5000
		} else if (type == OPF_FLEXIBLE_ARGUMENT) {
			add_type = OPR_FLEXIBLE_ARGUMENT;

		} else if (type == OPF_NUMBER) {  // takes numbers
			add_type = OPR_NUMBER;
			add_number_act->setEnabled(true);
		} else if (type == OPF_POSITIVE) {  // takes non-negative numbers
			add_type = OPR_POSITIVE;
			add_number_act->setEnabled(true);
		} else if (type == OPF_BOOL) {  // takes true/false bool values
			add_type = OPR_BOOL;

		} else if (type == OPF_AI_GOAL) {
			add_type = OPR_AI_GOAL;
		} else if (type == OPF_CONTAINER_VALUE) {
			// allow both strings and numbers
			// types are checked in check_sepx_syntax()
			add_number_act->setEnabled(true);
		}

		// add_type unchanged from above
		if (add_type == OPR_STRING && !is_container_name_opf_type(type)) {
			add_string_act->setEnabled(true);
		}

		list->destroy();
	}

	// disable operators that do not have arguments available
	for (j = 0; j < (int) Operators.size(); j++) {
		if (!query_default_argument_available(j)) {
			auto iter = operator_action_mapping.find(Operators[j].value);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
		}
	}


	// change enabled status of 'replace' type menu options.
	replace_type = 0;
	int parent = tree_nodes[item_index].parent;
	if (parent >= 0) {
		replace_type = OPR_STRING;
		op = get_operator_index(tree_nodes[parent].text);
		Assertion(op >= 0 || tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
			"Encountered unknown SEXP operator %s. Please report!",
			tree_nodes[parent].text);
		int first_arg = tree_nodes[parent].child;
		count = count_args(tree_nodes[parent].child);

		if (op >= 0) {
			// already at minimum number of arguments?
			if (count <= Operators[op].min) {
				delete_act->setEnabled(false);
			}
		} else if ((tree_nodes[parent].type & SEXPT_CONTAINER_DATA) && (item_index == first_arg)) {
			// a container data node's initial modifier can't be deleted
			Assertion(tree_nodes[item_index].type & SEXPT_MODIFIER,
				"Container data %s node's first modifier %s is not a modifier. Please report!",
				tree_nodes[parent].text,
				tree_nodes[item_index].text);
			delete_act->setEnabled(false);
		}


		// get arg count of item to replace
		Replace_count = 0;
		int temp = first_arg;
		while (temp != item_index) {
			Replace_count++;
			temp = tree_nodes[temp].next;

			// DB - added 3/4/99
			if (temp == -1) {
				break;
			}
		}

		if (op >= 0) {
			// maybe gray delete
			for (i = Replace_count + 1; i < count; i++) {
				if (query_operator_argument_type(op, i - 1) != query_operator_argument_type(op, i)) {
					delete_act->setEnabled(false);
					break;
				}
			}

			type = query_operator_argument_type(op, Replace_count); // check argument type at this position
		} else {
			Assertion(tree_nodes[parent].type& SEXPT_CONTAINER_DATA,
				"Unknown SEXP operator %s. Please report!",
				tree_nodes[parent].text);
			const auto *p_container = get_sexp_container(tree_nodes[parent].text);
			Assertion(p_container != nullptr,
				"Found modifier for unknown container %s. Please report!",
				tree_nodes[parent].text);
			type = p_container->opf_type;
		}

		// special case reset type for ambiguous
		if (type == OPF_AMBIGUOUS) {
			type = get_modify_variable_type(parent);
		}

		// Container modifiers use their own list of possible arguments
		if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
			const auto *p_container = get_sexp_container(tree_nodes[parent].text);
			Assertion(p_container != nullptr,
				"Found modifier for unknown container %s. Please report!",
				tree_nodes[parent].text);
			const int first_modifier = tree_nodes[parent].child;
			if (Replace_count == 1 && p_container->is_list() &&
				get_list_modifier(tree_nodes[first_modifier].text) == ListModifier::AT_INDEX) {
				// only valid value is a list index (number)
				list = nullptr;
				replace_type = OPR_NUMBER;
			} else {
				list = get_container_modifiers(parent);
			}
		} else {
			list = get_listing_opf(type, parent, Replace_count);
		}

		// special case don't allow replace data for variable or container names
		if ((type != OPF_VARIABLE_NAME) && !is_container_name_opf_type(type) && list) {
			sexp_list_item* ptr;

			int data_idx = 0;
			ptr = list;
			while (ptr) {
				if (ptr->op >= 0) {
					auto iter = operator_action_mapping.find(Operators[ptr->op].value | OP_REPLACE_FLAG);
					if (iter != operator_action_mapping.end()) {
						iter->second->setEnabled(true);
					}
				} else {
					replace_data_menu->addAction(QString::fromStdString(ptr->text),
											 this,
											 [this, data_idx]() { addReplaceTypedDataHandler(data_idx, true); });
				}

				data_idx++;
				ptr = ptr->next;
			}
		}

		if (type == OPF_NONE) {  // takes no arguments
			replace_type = 0;

		} else if (type == OPF_NUMBER) {  // takes numbers
			replace_type = OPR_NUMBER;
			replace_number_act->setEnabled(true);
		} else if (type == OPF_POSITIVE) {  // takes non-negative numbers
			replace_type = OPR_POSITIVE;
			replace_number_act->setEnabled(true);
		} else if (type == OPF_BOOL) {  // takes true/false bool values
			replace_type = OPR_BOOL;

		} else if (type == OPF_NULL) {  // takes operator that doesn't return a value
			replace_type = OPR_NULL;
		} else if (type == OPF_AI_GOAL) {
			replace_type = OPR_AI_GOAL;
		}

			// Goober5000
		else if (type == OPF_FLEXIBLE_ARGUMENT) {
			replace_type = OPR_FLEXIBLE_ARGUMENT;
		}
			// Goober5000
		else if (type == OPF_GAME_SND || type == OPF_FIREBALL || type == OPF_WEAPON_BANK_NUMBER) {
			// even though these default to strings, we allow replacing them with index values
			replace_type = OPR_POSITIVE;
			replace_number_act->setEnabled(true);

		} else if (type == OPF_CONTAINER_VALUE) {
			// allow strings and numbers
			// type is checked in check_sexp_syntax()
			add_number_act->setEnabled(true);
		}

		// default to string, except for container names
		if (replace_type == OPR_STRING && !is_container_name_opf_type(type)) {
			replace_string_act->setEnabled(true);
		}

		if (op >= 0) { // skip when handling "replace container data"
			// modify string or number if (modify_variable)
			if (Operators[op].value == OP_MODIFY_VARIABLE) {
				int modify_type = get_modify_variable_type(parent);

				if (modify_type == OPF_NUMBER) {
					replace_number_act->setEnabled(true);
					replace_string_act->setEnabled(false);
				}
				// no change for string type
			} else if (Operators[op].value == OP_SET_VARIABLE_BY_INDEX) {
				// it depends on which argument we are modifying
				// first argument is always a number
				if (Replace_count == 0) {
					replace_number_act->setEnabled(true);
					replace_string_act->setEnabled(false);
				}
				// second argument could be anything
				else {
					int modify_type = get_modify_variable_type(parent);

					if (modify_type == OPF_NUMBER) {
						replace_number_act->setEnabled(true);
						replace_string_act->setEnabled(false);
					}
					// no change for string type
				}
			}
		}

		if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
			Assertion(tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
				"Attempt to check modifier of non-container node %s. Please report!",
				tree_nodes[parent].text);
			const int first_modifier_node = tree_nodes[parent].child;
			Assertion(first_modifier_node != -1,
				"Could not find first modifier of container data node %s. Please report!",
				tree_nodes[parent].text);
			const auto *p_container = get_sexp_container(tree_nodes[parent].text);
			Assertion(p_container,
				"Attempt to get first modifier for unknown container %s. Please report!",
				tree_nodes[parent].text);
			const auto &container = *p_container;

			if (Replace_count == 0) {
				if (container.is_list()) {
					// the only valid values are either the list modifiers or Replace Variable/Cotnainer Data with string data
					replace_number_act->setEnabled(false);
					replace_string_act->setEnabled(false);
					edit_data_act->setEnabled(false);
				} else if (container.is_map()) {
					if (any(container.type & ContainerType::STRING_KEYS)) {
						replace_number_act->setEnabled(false);
						replace_string_act->setEnabled(true);
					} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
						replace_number_act->setEnabled(true);
						replace_string_act->setEnabled(false);
					} else {
						UNREACHABLE("Map container with type %d has unknown key type", (int)container.type);
					}
				} else {
					UNREACHABLE("Unknown container type %d", (int)container.type);
				}
			} else if (Replace_count == 1 && container.is_list() &&
				get_list_modifier(tree_nodes[first_modifier_node].text) ==
				ListModifier::AT_INDEX) {
				// only valid value is a list index
				replace_number_act->setEnabled(true);
				replace_string_act->setEnabled(false);
			} else {
				// multidimensional modifiers can be anything, including possibly a list modifier
				// the value can be validated only at runtime (i.e., in-mission)
				replace_number_act->setEnabled(true);
				replace_string_act->setEnabled(true);
			}
		}

		list->destroy();

	} else {  // top node, so should be a Boolean type.
		replace_type = _interface->getRootReturnType();
		for (j = 0; j < (int) Operators.size(); j++) {
			if (query_operator_return_type(j) == replace_type) {
				auto iter = operator_action_mapping.find(Operators[j].value | OP_REPLACE_FLAG);
				if (iter != operator_action_mapping.end()) {
					iter->second->setEnabled(true);
				}
			}
		}
	}

	// disable operators that do not have arguments available
	for (j = 0; j < (int) Operators.size(); j++) {
		if (!query_default_argument_available(j)) {
			auto iter = operator_action_mapping.find(Operators[j].value | OP_REPLACE_FLAG);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
		}
	}


	// change enabled status of 'insert' type menu options.
	z = tree_nodes[item_index].parent;
	Assert(z >= -1);
	if (z != -1) {
		op = get_operator_index(tree_nodes[z].text);
		Assertion(op != -1 || tree_nodes[z].type & SEXPT_CONTAINER_DATA,
			"Encountered unknown SEXP operator %s. Please report!",
			tree_nodes[z].text);
		j = tree_nodes[z].child;
		count = 0;
		while (j != item_index) {
			count++;
			j = tree_nodes[j].next;
		}

		if (op >= 0) {
			type = query_operator_argument_type(op, count); // check argument type at this position
		} else {
			Assertion(tree_nodes[z].type & SEXPT_CONTAINER_DATA,
				"Unknown SEXP operator %s. Please report!",
				tree_nodes[z].text);
			const auto *p_container = get_sexp_container(tree_nodes[z].text);
			Assertion(p_container != nullptr,
				"Found modifier for unknown container %s. Please report!",
				tree_nodes[z].text);
			type = p_container->opf_type;
		}

	} else {
		type = _interface->getRootReturnType();
	}

	for (j = 0; j < (int) Operators.size(); j++) {
		z = query_operator_return_type(j);
		if (!sexp_query_type_match(type, z) || (Operators[j].min < 1)) {
			auto iter = operator_action_mapping.find(Operators[j].value | OP_INSERT_FLAG);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
		}

		z = query_operator_argument_type(j, 0);
		if ((type == OPF_NUMBER) && (z == OPF_POSITIVE)) {
			z = OPF_NUMBER;
		}

		// Goober5000's number hack
		if ((type == OPF_POSITIVE) && (z == OPF_NUMBER)) {
			z = OPF_POSITIVE;
		}

		if (z != type) {
			auto iter = operator_action_mapping.find(Operators[j].value | OP_INSERT_FLAG);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
		}
	}

	// disable operators that do not have arguments available
	for (j = 0; j < (int) Operators.size(); j++) {
		if (!query_default_argument_available(j)) {
			auto iter = operator_action_mapping.find(Operators[j].value | OP_INSERT_FLAG);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
		}
	}


	// disable non campaign operators if in campaign mode
	for (j = 0; j < (int) Operators.size(); j++) {
		z = 0;
		if (_interface->requireCampaignOperators()) {
			if (!usable_in_campaign(Operators[j].value))
				z = 1;
		}

		if (z) {
			auto iter = operator_action_mapping.find(Operators[j].value);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
			iter = operator_action_mapping.find(Operators[j].value | OP_REPLACE_FLAG);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
			iter = operator_action_mapping.find(Operators[j].value | OP_INSERT_FLAG);
			if (iter != operator_action_mapping.end()) {
				iter->second->setEnabled(false);
			}
		}
	}

	if ((Sexp_clipboard > -1) && (Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED)) {
		Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);
		Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
			"Attempt to use container name %s from SEXP clipboard. Please report!",
			Sexp_nodes[Sexp_clipboard].text);

		if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
			j = get_operator_const(CTEXT(Sexp_clipboard));
			Assert(j);
			z = query_operator_return_type(j);

			if ((z == OPR_POSITIVE) && (replace_type == OPR_NUMBER)) {
				z = OPR_NUMBER;
			}

			// Goober5000's number hack
			if ((z == OPR_NUMBER) && (replace_type == OPR_POSITIVE)) {
				z = OPR_POSITIVE;
			}

			if (replace_type == z) {
				paste_act->setEnabled(true);
			}

			z = query_operator_return_type(j);
			if ((z == OPR_POSITIVE) && (add_type == OPR_NUMBER)) {
				z = OPR_NUMBER;
			}

			if (add_type == z) {
				add_paste_act->setEnabled(true);
			}

		} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_CONTAINER_DATA) {
			// TODO: check for strictly typed container keys/data
			const auto *p_container = get_sexp_container(Sexp_nodes[Sexp_clipboard].text);
			// if-check in case the container was renamed/deleted after the container data was cut/copied
			if (p_container != nullptr) {
				const auto &container = *p_container;
				if (any(container.type & ContainerType::NUMBER_DATA)) {
					// there's no way to check for OPR_POSITIVE, since the value
					// is known only in-mission, so we'll handle OPR_NUMBER only
					if (replace_type == OPR_NUMBER)
						paste_act->setEnabled(true);
					if (add_type == OPR_NUMBER)
						add_paste_act->setEnabled(true);
				} else if (any(container.type & ContainerType::STRING_DATA)) {
					if (replace_type == OPR_STRING && !is_container_name_opf_type(type))
						paste_act->setEnabled(true);
					if (add_type == OPR_STRING && !is_container_name_opf_type(type))
						add_paste_act->setEnabled(true);
				} else {
					UNREACHABLE("Unknown container data type %d", (int)container.type);
				}
			}

		} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
			if ((replace_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1)) {
				edit_data_act->setEnabled(true);
			} else if (replace_type == OPR_NUMBER) {
				edit_data_act->setEnabled(true);
			}

			if ((add_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1)) {
				add_paste_act->setEnabled(true);
			} else if (add_type == OPR_NUMBER) {
				add_paste_act->setEnabled(true);
			}

		} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
			if (replace_type == OPR_STRING && !is_container_name_opf_type(type)) {
				edit_data_act->setEnabled(true);
			}

			if (add_type == OPR_STRING && !is_container_name_opf_type(type)) {
				add_paste_act->setEnabled(true);
			}

		} else
			Int3();  // unknown and/or invalid sexp type
	}

	if (delete_act->isEnabled()) {
		cut_act->setEnabled(true);
	}

	// all of the following restrictions may be revisited in the future
	if (tree_nodes[item_index].type & (SEXPT_MODIFIER | SEXPT_CONTAINER_NAME)) {
		// modifiers and container names don't support cut/copy/paste
		cut_act->setEnabled(false);
		copy_act->setEnabled(false);
		paste_act->setEnabled(false);
	}
	// can't use else-if here, because container data is a valid modifier
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		// container data nodes don't support add-pasting modifiers
		add_paste_act->setEnabled(false);
	}

	util::propagate_disabled_status(popup_menu.get());
	return popup_menu;
}
void sexp_tree::cutActionHandler() {
	if (Sexp_clipboard != -1) {
		sexp_unmark_persistent(Sexp_clipboard);
		free_sexp2(Sexp_clipboard);
	}

	Sexp_clipboard = save_branch(item_index, 1);
	sexp_mark_persistent(Sexp_clipboard);

	// fall through to ID_DELETE case.
	deleteActionHandler();
}
void sexp_tree::deleteActionHandler() {
	if (currentItem() == nullptr) {
		return;
	}

	if (_interface->getFlags()[TreeFlags::RootDeletable] && (item_index == -1)) {
		auto item = currentItem();
		item_index = item->data(0, FormulaDataRole).toInt();

		rootNodeDeleted(item_index);

		free_node2(item_index);
		delete item;
		modified();
		return;
	}

	Assert(item_index >= 0);
	auto h_parent = currentItem()->parent();
	auto parent = tree_nodes[item_index].parent;

	Assert(parent != -1 && tree_nodes[parent].handle == h_parent);
	free_node(item_index);
	delete currentItem();

	modified();
}
void sexp_tree::editDataActionHandler() {
	beginItemEdit(currentItem());
}
void sexp_tree::handleItemChange(QTreeWidgetItem* item, int  /*column*/) {
	if (!_currently_editing) {
		return;
	}
	_currently_editing = false;

	auto str = item->text(0);
	bool update_node = true;
	uint node;

	if (str.isEmpty()) {
		return;
	}

	for (node = 0; node < tree_nodes.size(); node++) {
		if (tree_nodes[node].handle == item) {
			break;
		}
	}

	if (node == tree_nodes.size()) {
		setCurrentItemIndex(qvariant_cast<int>(item->data(0, FormulaDataRole)));

		rootNodeRenamed(item_index);

		return;
	}

	Assert(node < tree_nodes.size());
	if (tree_nodes[node].type & SEXPT_OPERATOR) {
		auto op = match_closest_operator(str.toStdString(), node);
		if (op.empty()) {
			return;
		}    // Goober5000 - avoids crashing

		// use the text of the operator we found
		str = QString::fromStdString(op);
		item->setText(0, str);

		setCurrentItemIndex(node);
		int op_num = get_operator_index(op.c_str());
		if (op_num >= 0) {
			add_or_replace_operator(op_num, 1);
		} else {
			update_node = false;
		}
	}
	// gotta sidestep Goober5000's number hack and check entries are actually positive.
	else if (tree_nodes[node].type & SEXPT_NUMBER) {
		if (query_node_argument_type(node) == OPF_POSITIVE) {
			int val = str.toInt();
			if (val < 0) {
				QMessageBox::critical(this, "Invalid Number", "Can not enter a negative value");
				update_node = false;
			}
		}
	}

	// Error checking would not hurt here
	auto len = str.size();
	if (len >= TOKEN_LENGTH) {
		len = TOKEN_LENGTH - 1;
	}

	if (update_node) {
		modified();
		auto strBytes = str.toUtf8(); // avoid using dangling ptr
		strncpy(tree_nodes[node].text, strBytes.constData(), len);
		tree_nodes[node].text[len] = 0;

		// let's make sure we aren't introducing any invalid characters, per Mantis #2893
		lcl_fred_replace_stuff(tree_nodes[node].text, TOKEN_LENGTH - 1);
	} else {
		item->setText(0, QString::fromUtf8(tree_nodes[node].text, len));
	}
}
void sexp_tree::copyActionHandler() {
	// If a clipboard already exist, unmark it as persistent and free old clipboard
	if (Sexp_clipboard != -1) {
		sexp_unmark_persistent(Sexp_clipboard);
		free_sexp2(Sexp_clipboard);
	}

	// Allocate new clipboard and mark persistent
	Sexp_clipboard = save_branch(item_index, 1);
	sexp_mark_persistent(Sexp_clipboard);
}
void sexp_tree::pasteActionHandler() {
	// the following assumptions are made..
	Assert((Sexp_clipboard > -1) && (Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED));
	Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
		"Attempt to use container name %s from SEXP clipboard. Please report!",
		Sexp_nodes[Sexp_clipboard].text);

	if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
		expand_operator(item_index);
		replace_operator(CTEXT(Sexp_clipboard));
		if (Sexp_nodes[Sexp_clipboard].rest != -1) {
			load_branch(Sexp_nodes[Sexp_clipboard].rest, item_index);
			auto i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_nodes[item_index].handle);
				i = tree_nodes[i].next;
			}
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_CONTAINER_DATA) {
		expand_operator(item_index);
		const auto *p_container = get_sexp_container(Sexp_nodes[Sexp_clipboard].text);
		Assertion(p_container,
			"Attempt to paste unknown container %s. Please report!",
			Sexp_nodes[Sexp_clipboard].text);
		const auto &container = *p_container;
		// this should always be true, but just in case
		const bool has_modifiers = (Sexp_nodes[Sexp_clipboard].first != -1);
		int new_type = tree_nodes[item_index].type & (~(SEXPT_VARIABLE | SEXPT_CONTAINER_NAME) | SEXPT_CONTAINER_DATA);
		replace_container_data(container, new_type, false, true, !has_modifiers);
		if (has_modifiers) {
			load_branch(Sexp_nodes[Sexp_clipboard].first, item_index);
			int i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_nodes[item_index].handle);
				i = tree_nodes[i].next;
			}
		} else {
			add_default_modifier(container);
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
		Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
		if (Sexp_nodes[Sexp_clipboard].type & SEXP_FLAG_VARIABLE) {
			int var_idx = get_index_sexp_variable_name(Sexp_nodes[Sexp_clipboard].text);
			Assert(var_idx > -1);
			replace_variable_data(var_idx, (SEXPT_VARIABLE | SEXPT_NUMBER | SEXPT_VALID));
		} else {
			expand_operator(item_index);
			replace_data(CTEXT(Sexp_clipboard), (SEXPT_NUMBER | SEXPT_VALID));
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
		Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
		if (Sexp_nodes[Sexp_clipboard].type & SEXP_FLAG_VARIABLE) {
			int var_idx = get_index_sexp_variable_name(Sexp_nodes[Sexp_clipboard].text);
			Assert(var_idx > -1);
			replace_variable_data(var_idx, (SEXPT_VARIABLE | SEXPT_STRING | SEXPT_VALID));
		} else {
			expand_operator(item_index);
			replace_data(CTEXT(Sexp_clipboard), (SEXPT_STRING | SEXPT_VALID));
		}

	} else
		Assert(0);  // unknown and/or invalid sexp type

	expand_branch(currentItem());

}
void sexp_tree::insertOperatorAction(int op) {
	int flags;

	auto z = tree_nodes[item_index].parent;
	flags = tree_nodes[item_index].flags;
	auto node = allocate_node(z, item_index);
	set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
	tree_nodes[node].flags = flags;
	QTreeWidgetItem* h;
	if (z >= 0) {
		h = tree_nodes[z].handle;
	} else {
		h = tree_nodes[item_index].handle->parent();
		if (!_interface->getFlags()[TreeFlags::LabeledRoot]) {
			h = nullptr;
			root_item = node;
		} else {
			rootNodeFormulaChanged(item_index, node);
			h->setData(0, FormulaDataRole, node);
		}
	}

	auto item_handle = tree_nodes[node].handle =
						   insert(Operators[op].text.c_str(), NodeImage::OPERATOR, h, tree_nodes[item_index].handle);
	move_branch(item_index, node);

	setCurrentItemIndex(node);
	for (auto i = 1; i < Operators[op].min; i++) {
		add_default_operator(op, i);
	}

	item_handle->setExpanded(true);
	modified();
}
void sexp_tree::addNumberDataHandler() {
	int theType = SEXPT_NUMBER | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		theType |= SEXPT_MODIFIER;
	}

	int theNode = add_data("number", theType);
	beginItemEdit(tree_nodes[theNode].handle);
}
void sexp_tree::addStringDataHandler() {
	int theType = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		theType |= SEXPT_MODIFIER;
	}

	int theNode = add_data("string", theType);
	beginItemEdit(tree_nodes[theNode].handle);
}
void sexp_tree::replaceNumberDataHandler() {
	expand_operator(item_index);
	int type = SEXPT_NUMBER | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	replace_data("number", type);
	beginItemEdit(tree_nodes[item_index].handle);
}
void sexp_tree::replaceStringDataHandler() {
	expand_operator(item_index);
	int type = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	replace_data("string", type);
	beginItemEdit(tree_nodes[item_index].handle);
}
void sexp_tree::beginItemEdit(QTreeWidgetItem* item) {
	_currently_editing = true;
	editItem(item);
}
void sexp_tree::addReplaceTypedDataHandler(int data_idx, bool replace) {
	Assert(item_index >= 0);
	const int op_node = replace ? tree_nodes[item_index].parent : item_index;

	sexp_list_item *list = nullptr;
	if (tree_nodes[op_node].type & SEXPT_CONTAINER_DATA) {
		// container data modifier
		if (replace && Replace_count == 0) {
			list = get_container_modifiers(op_node);
		} else {
			list = get_container_multidim_modifiers(op_node);
		}
	} else {
		int op = get_operator_index(tree_nodes[op_node].text);
		Assert(op >= 0);
		auto argcount = replace ? Replace_count : Add_count;
		auto type = query_operator_argument_type(op, argcount);
		list = get_listing_opf(type, item_index, argcount);
	}
	Assert(list);

	auto ptr = list;
	while (data_idx) {
		data_idx--;
		ptr = ptr->next;
		Assert(ptr);
	}

	Assert((SEXPT_TYPE(ptr->type) != SEXPT_OPERATOR) && (ptr->op < 0));
	expand_operator(item_index);
	if (replace) {
		replace_data(ptr->text.c_str(), ptr->type);
	} else {
		add_data(ptr->text.c_str(), ptr->type);
	}
	list->destroy();
}
void sexp_tree::addPasteActionHandler() {
	// add paste, instead of replace.
	// the following assumptions are made..
	Assert((Sexp_clipboard > -1) && (Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED));
	Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
		"Attempt to use container name %s from SEXP clipboard. Please report!",
		Sexp_nodes[Sexp_clipboard].text);

	if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
		expand_operator(item_index);
		add_operator(CTEXT(Sexp_clipboard));
		if (Sexp_nodes[Sexp_clipboard].rest != -1) {
			load_branch(Sexp_nodes[Sexp_clipboard].rest, item_index);
			auto i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_nodes[item_index].handle);
				i = tree_nodes[i].next;
			}
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_CONTAINER_DATA) {
		expand_operator(item_index);
		add_container_data(Sexp_nodes[Sexp_clipboard].text);
		const int modifier_node = Sexp_nodes[Sexp_clipboard].first;
		if (modifier_node != -1) {
			load_branch(modifier_node, item_index);
			int i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_nodes[item_index].handle);
				i = tree_nodes[i].next;
			}
		} else {
			// this shouldn't happen, but just in case
			const auto *p_container = get_sexp_container(Sexp_nodes[Sexp_clipboard].text);
			Assertion(p_container,
				"Attempt to add-paste unknown container %s. Please report!",
				Sexp_nodes[Sexp_clipboard].text);
			add_default_modifier(*p_container);
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
		Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
		expand_operator(item_index);
		add_data(CTEXT(Sexp_clipboard), (SEXPT_NUMBER | SEXPT_VALID));

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
		Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
		expand_operator(item_index);
		add_data(CTEXT(Sexp_clipboard), (SEXPT_STRING | SEXPT_VALID));

	} else
		Assert(0);  // unknown and/or invalid sexp type

	expand_branch(currentItem());
}
void sexp_tree::setCurrentItemIndex(int node) {
	item_index = node;
	if (node < 0) {
		setCurrentItem(nullptr);
	} else {
		setCurrentItem(tree_nodes[node].handle);
	}
}
void sexp_tree::handleReplaceVariableAction(int id) {
	Assert(item_index >= 0);

	// get index into list of type valid variables
	Assert( (id >= 0) && (id < MAX_SEXP_VARIABLES) );

	int type = get_type(currentItem());
	Assert( (type & SEXPT_NUMBER) || (type & SEXPT_STRING) );

	// don't do type check for modify-variable or OPF_CONTAINER_VALUE (can be either type)
	if (Modify_variable || query_node_argument_type(item_index) == OPF_CONTAINER_VALUE) {
		if (Sexp_variables[id].type & SEXP_VARIABLE_NUMBER) {
			type = SEXPT_NUMBER;
		} else if (Sexp_variables[id].type & SEXP_VARIABLE_STRING) {
			type = SEXPT_STRING;
		} else {
			Int3();	// unknown type
		}

	} else {
		// verify type in tree is same as type in Sexp_variables array
		if (type & SEXPT_NUMBER) {
			Assert(Sexp_variables[id].type & SEXP_VARIABLE_NUMBER);
		}

		if (type & SEXPT_STRING) {
			Assert( (Sexp_variables[id].type & SEXP_VARIABLE_STRING) );
		}
	}

	// Replace data
	replace_variable_data(id, (type | SEXPT_VARIABLE));

}
void sexp_tree::handleReplaceContainerNameAction(int idx) {
	Assertion(item_index >= 0, "Attempt to Replace Container Name with no node selected. Please report!");

	const auto &containers = get_all_sexp_containers();
	Assertion((idx >= 0) && (idx < (int)containers.size()), "Unknown Container Index %d. Please report!", idx);

	const int type = get_type(currentItem());
	Assertion(type & SEXPT_STRING,
		"Attempt to replace container name on non-string node %s with type %d. Please report!",
		tree_nodes[item_index].text,
		type);

	replace_container_name(containers[idx]);
}
void sexp_tree::handleReplaceContainerDataAction(int idx) {
	Assertion(item_index >= 0, "Attempt to Replace Container Data with no node selected. Please report!");

	const auto &containers = get_all_sexp_containers();
	Assertion((idx >= 0) && (idx < (int)containers.size()),
		"Unknown Container index %d. Please report!", idx);

	int type = get_type(currentItem());
	Assertion((type & SEXPT_NUMBER) || (type & SEXPT_STRING),
		"Attempt to use Replace Container Data on a non-data node. Please report!");

	// variable/container name don't mix with container data
	// DISCUSSME: what about variable name as SEXP arg type?
	type &= ~(SEXPT_VARIABLE | SEXPT_CONTAINER_NAME);
	replace_container_data(containers[idx], (type | SEXPT_CONTAINER_DATA), true, true, true);

	auto *handle = tree_nodes[item_index].handle;
	expand_branch(handle);
}
void sexp_tree::handleNewItemSelected() {
	auto selectedItem = currentItem();

	update_help(selectedItem);

	if (selectedItem == nullptr) {
		selectedRootChanged(-1);
		setCurrentItemIndex(-1);
		return;
	}

	// Set the item index so that it is always up to date
	item_index = get_node(selectedItem);

	auto item = selectedItem;
	while (item->parent() != nullptr) {
		item = item->parent();
	}

	selectedRootChanged(item->data(0, FormulaDataRole).toInt());
}
void sexp_tree::deleteCurrentItem() {
	deleteActionHandler();
}
int sexp_tree::getCurrentItemIndex() const {
	return item_index;
}

}
}
