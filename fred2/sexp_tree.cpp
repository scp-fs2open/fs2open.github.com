/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "Sexp_tree.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "Management.h"
#include "parse/sexp.h"
#include "parse/sexp/sexp_lookup.h"
#include "OperatorArgTypeSelect.h"
#include "globalincs/linklist.h"
#include "EventEditor.h"
#include "MissionGoalsDlg.h"
#include "MissionCutscenesDlg.h"
#include "ai/aigoals.h"
#include "ai/ailua.h"
#include "mission/missionmessage.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"
#include "missioneditor/common.h"
#include "CampaignEditorDlg.h"
#include "hud/hudsquadmsg.h"
#include "IgnoreOrdersDlg.h"
#include "stats/medals.h"
#include "controlconfig/controlsconfig.h"
#include "hud/hudgauges.h"
#include "starfield/starfield.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "jumpnode/jumpnode.h"
#include "prop/prop.h"
#include "AddVariableDlg.h"
#include "ModifyVariableDlg.h"
#include "gamesnd/eventmusic.h"	// for change-soundtrack
#include "menuui/techmenu.h"	// for intel stuff
#include "weapon/emp.h"
#include "gamesnd/gamesnd.h"
#include "weapon/weapon.h"
#include "hud/hudartillery.h"
#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"
#include "sound/ds.h"
#include "globalincs/alphacolors.h"
#include "localization/localize.h"
#include "AddModifyContainerDlg.h"
#include "asteroid/asteroid.h"

#define TREE_NODE_INCREMENT	100

#define MAX_OP_MENUS	30
#define MAX_SUBMENUS	(MAX_OP_MENUS * MAX_OP_MENUS)

#define ID_CONTAINER_NAME_MENU	0xd600
#define ID_CONTAINER_DATA_MENU	0xd800
#define ID_VARIABLE_MENU	0xda00
#define ID_ADD_MENU			0xdc00
#define ID_REPLACE_MENU		0xde00
// note: stay below 0xe000 so we don't collide with MFC defines..

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//********************sexp_tree********************

BEGIN_MESSAGE_MAP(sexp_tree, CTreeCtrl)
	//{{AFX_MSG_MAP(sexp_tree)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeyDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static int Add_count, Replace_count;
static int Modify_variable;

// constructor
sexp_tree::sexp_tree()
	: m_operator_box(help), _actions(_model, *this)
{
	select_sexp_node = -1;
	root_item = -1;
	m_mode = 0;
	m_dragging = FALSE;
	m_p_image_list = NULL;
	help_box = NULL;
	clear_tree();

	m_operator_popup_active = false;
	m_operator_popup_created = false;
	m_font_height = 0;
	m_font_max_width = 0;
}

// --- ISexpTreeUI implementation ---

void* sexp_tree::ui_insert_item(const char* text, NodeImage image, void* parent_handle, void* insert_after)
{
	int img = static_cast<int>(image);
	HTREEITEM hParent = parent_handle ? static_cast<HTREEITEM>(parent_handle) : TVI_ROOT;
	HTREEITEM hAfter = insert_after ? static_cast<HTREEITEM>(insert_after) : TVI_LAST;
	return static_cast<void*>(InsertItem(text, img, img, hParent, hAfter));
}

void sexp_tree::ui_delete_item(void* handle)
{
	DeleteItem(static_cast<HTREEITEM>(handle));
}

void sexp_tree::ui_set_item_text(void* handle, const char* text)
{
	SetItemText(static_cast<HTREEITEM>(handle), text);
}

void sexp_tree::ui_set_item_image(void* handle, NodeImage image)
{
	int img = static_cast<int>(image);
	SetItemImage(static_cast<HTREEITEM>(handle), img, img);
}

void* sexp_tree::ui_get_child_item(void* handle)
{
	return static_cast<void*>(GetChildItem(static_cast<HTREEITEM>(handle)));
}

bool sexp_tree::ui_has_children(void* handle)
{
	return ItemHasChildren(static_cast<HTREEITEM>(handle)) != 0;
}

void sexp_tree::ui_expand_item(void* handle)
{
	Expand(static_cast<HTREEITEM>(handle), TVE_EXPAND);
}

void sexp_tree::ui_select_item(void* handle)
{
	SelectItem(static_cast<HTREEITEM>(handle));
}

void sexp_tree::ui_ensure_visible(void* handle)
{
	EnsureVisible(static_cast<HTREEITEM>(handle));
}

void sexp_tree::ui_notify_modified()
{
	if (modified)
		*modified = 1;
}

void sexp_tree::ui_update_help(void* handle)
{
	update_help(static_cast<HTREEITEM>(handle));
}

// clears out the tree, so all the nodes are unused.
void sexp_tree::clear_tree(const char *op)
{
	mprintf(("Resetting dynamic tree node limit from %d to %d...\n", tree_nodes.size(), 0));

	total_nodes = flag = 0;
	tree_nodes.clear();

	if (op) {
		DeleteAllItems();
		if (strlen(op)) {
			set_node(allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
			build_tree();
		}
	}
}

void sexp_tree::reset_handles()
{
	uint i;

	for (i=0; i<tree_nodes.size(); i++)
		tree_nodes[i].handle = NULL;
}

// initializes and creates a tree from a given sexp startpoint.
void sexp_tree::load_tree(int index, const char *deflt)
{
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
		if (atoi(Sexp_nodes[index].text))
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "true");
		else
			set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "false");

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

void get_combined_variable_name(char *combined_name, const char *sexp_var_name)
{
	int sexp_var_index = get_index_sexp_variable_name(sexp_var_name);

	if (sexp_var_index >= 0)
		sprintf(combined_name, "%s(%s)", Sexp_variables[sexp_var_index].variable_name, Sexp_variables[sexp_var_index].text);
	else
		sprintf(combined_name, "%s(undefined)", sexp_var_name);
}

// creates a tree from a given Sexp_nodes[] point under a given parent.  Recursive.
// Returns the allocated current node.
int sexp_tree::load_branch(int index, int parent)
{
	int cur = -1;
	char combined_var_name[2*TOKEN_LENGTH + 2];

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
			load_branch(Sexp_nodes[index].first, cur);  // container is new parent now

		} else
			Assert(0);  // unknown and/or invalid sexp type

		if ((index == select_sexp_node) && !flag) {  // translate sexp node to our node
			select_sexp_node = cur;
			flag = 1;
		}

		index = Sexp_nodes[index].rest;
		if (index == -1)
			return cur;
	}

	return cur;
}

int sexp_tree::query_false(int node)
{
	if (node < 0) node = root_item;
	return _model.query_false(node);
}

// builds an sexp of the tree and returns the index of it.  This allocates sexp nodes.
int sexp_tree::save_tree(int node)
{
	if (node < 0) node = root_item;
	return _model.save_tree(node);
}

// get variable name from sexp_tree node .text
void var_name_from_sexp_tree_text(char *var_name, const char *text)
{
	auto var_name_length = strcspn(text, "(");
	Assert(var_name_length < TOKEN_LENGTH - 1);

	strncpy(var_name, text, var_name_length);
	var_name[var_name_length] = '\0';
}

#define NO_PREVIOUS_NODE -9
// called recursively to save a tree branch and everything under it
// SEXPT_CONTAINER_NAME and SEXPT_MODIFIER require no special handling here
int sexp_tree::save_branch(int cur, int at_root)
{
	return _model.save_branch(cur, at_root);
}

// find the next free tree node and return its index.
int sexp_tree::find_free_node()
{
	return _model.find_free_node();
}

// allocate a node.  Remains used until freed.
int sexp_tree::allocate_node()
{
	return _model.allocate_node();
}

// allocate a child node under 'parent'.  Appends to end of list.
int sexp_tree::allocate_node(int parent, int after)
{
	return _model.allocate_node(parent, after);
}

// free a node and all its children.  Also clears pointers to it, if any.
//   node = node chain to free
//   cascade =  0: free just this node and children under it. (default)
//             !0: free this node and all siblings after it.
//
void sexp_tree::free_node(int node, int cascade)
{
	_model.free_node(node, cascade);
}

// more simple node freer, which works recursively.  It frees the given node and all siblings
// that come after it, as well as all children of these.  Doesn't clear any links to any of
// these freed nodes, so make sure all links are broken first. (i.e. use free_node() if you can)
//
void sexp_tree::free_node2(int node)
{
	_model.free_node2(node);
}

// initialize the data for a node.  Should be called right after a new node is allocated.
void sexp_tree::set_node(int node, int type, const char *text)
{
	_model.set_node(node, type, text);
}

void sexp_tree::post_load()
{
	if (!flag)
		select_sexp_node = -1;
}

// build or rebuild a CTreeCtrl object with the current tree data
void sexp_tree::build_tree()
{
	if (!flag)
		select_sexp_node = -1;

	DeleteAllItems();
	add_sub_tree(0, TVI_ROOT);
}

// Create the CTreeCtrl tree from the tree data.  The tree data should already be setup by
// this point.
void sexp_tree::add_sub_tree(int node, HTREEITEM root)
{
//	char str[80];
	int node2;

	Assert(node >= 0 && node < (int)tree_nodes.size());
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
	int bitmap;

	if (tree_nodes[node].type & SEXPT_OPERATOR) {
		tree_nodes[node].flags = OPERAND;
		bitmap = BITMAP_OPERATOR;
	} else {
		if (tree_nodes[node].type & SEXPT_VARIABLE) {
			tree_nodes[node].flags = NOT_EDITABLE;
			bitmap = BITMAP_VARIABLE;
		} else if (tree_nodes[node].type & SEXPT_CONTAINER_NAME) {
			tree_nodes[node].flags = NOT_EDITABLE;
			bitmap = BITMAP_CONTAINER_NAME;
		} else if (tree_nodes[node].type & SEXPT_CONTAINER_DATA) {
			tree_nodes[node].flags = NOT_EDITABLE;
			bitmap = BITMAP_CONTAINER_DATA;
		} else {
			tree_nodes[node].flags = EDITABLE;
			bitmap = get_data_image(node);
		}
	}

	tree_nodes[node].handle = insert(tree_nodes[node].text, bitmap, bitmap, root);
	root = tree_item_handle(tree_nodes[node]);

	node = node2;
	while (node != -1) {
		Assert(node >= 0 && node < (int)tree_nodes.size());
		Assert(tree_nodes[node].type & SEXPT_VALID);
		if (tree_nodes[node].type & (SEXPT_OPERATOR | SEXPT_CONTAINER_DATA)) {
			add_sub_tree(node, root);

		} else {
			Assert(tree_nodes[node].child == -1);
			if (tree_nodes[node].type & SEXPT_VARIABLE) {
				tree_nodes[node].handle = insert(tree_nodes[node].text, BITMAP_VARIABLE, BITMAP_VARIABLE, root);
				tree_nodes[node].flags = NOT_EDITABLE;
			} else if (tree_nodes[node].type & SEXPT_CONTAINER_NAME) {
				tree_nodes[node].handle = insert(tree_nodes[node].text, BITMAP_CONTAINER_NAME, BITMAP_CONTAINER_NAME, root);
				tree_nodes[node].flags = NOT_EDITABLE;
			// SEXPT_MODIFIER doesn't require special treatment here
			} else {
				int bmap = get_data_image(node);
				tree_nodes[node].handle = insert(tree_nodes[node].text, bmap, bmap, root);
				tree_nodes[node].flags = EDITABLE;
			}
		}

		node = tree_nodes[node].next;
	}
}

// construct tree nodes for an sexp, adding them to the list and returning first node
int sexp_tree::load_sub_tree(int index, bool valid, const char *text)
{
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

void sexp_tree::setup_selected(HTREEITEM h)
{
	if (!h)
		h = GetSelectedItem();

	update_item(h);
}

void sexp_tree::update_item(HTREEITEM h)
{
	item_handle = h;
	item_index = -1;

	for (int i = 0; i < (int)tree_nodes.size(); ++i) {
		if (tree_nodes[i].handle == h) {
			item_index = i;
			break;
		}
	}
}

// handler for right mouse button clicks.
void sexp_tree::right_clicked(int mode)
{
	int i, j, z, count, op, add_type, replace_type, type = 0, subcategory_id;
	sexp_list_item *list;
	UINT _flags;
	HTREEITEM h;
	POINT click_point, mouse;
	CMenu menu, *mptr, *popup_menu, *add_data_menu = NULL, *replace_data_menu = NULL;
	CMenu *add_op_menu, add_op_submenu[MAX_OP_MENUS];
	CMenu *replace_op_menu, replace_op_submenu[MAX_OP_MENUS];
	CMenu *insert_op_menu, insert_op_submenu[MAX_OP_MENUS];
	CMenu *replace_variable_menu = NULL;
	CMenu *replace_container_name_menu = nullptr;
	CMenu *replace_container_data_menu = nullptr;
	CMenu add_op_subcategory_menu[MAX_SUBMENUS];
	CMenu replace_op_subcategory_menu[MAX_SUBMENUS];
	CMenu insert_op_subcategory_menu[MAX_SUBMENUS];

	m_mode = mode;
	add_instance = replace_instance = -1;
	Assert((int)op_menu.size() < MAX_OP_MENUS);
	Assert((int)op_submenu.size() < MAX_SUBMENUS);

	GetCursorPos(&mouse);
	click_point = mouse;
	ScreenToClient(&click_point);
	h = HitTest(CPoint(click_point), &_flags);  // find out what they clicked on

	if (h && menu.LoadMenu(IDR_MENU_EDIT_SEXP_TREE)) {
		update_help(h);
		popup_menu = menu.GetSubMenu(0);
		ASSERT(popup_menu != NULL);
		//SelectDropTarget(h);  // WTF: Why was this here???

		add_op_menu = replace_op_menu = insert_op_menu = NULL;

		// get pointers to several key popup menus we'll need to modify
		i = popup_menu->GetMenuItemCount();
		while (i--) {
			if ( (mptr = popup_menu->GetSubMenu(i)) > 0 ) {
				char buf[256];
				popup_menu->GetMenuString(i, buf, sizeof(buf), MF_BYPOSITION);

				if (!stricmp(buf, "add operator")) {
					add_op_menu = mptr;

				} else if (!stricmp(buf, "replace operator")) {
					replace_op_menu = mptr;

				} else if (!stricmp(buf, "add data")) {
					add_data_menu = mptr;

				} else if (!stricmp(buf, "replace data")) {
					replace_data_menu = mptr;

				} else if (!stricmp(buf, "insert operator")) {
					insert_op_menu = mptr;

				} else if (!stricmp(buf, "replace variable")) {
					replace_variable_menu = mptr;

				} else if (!stricmp(buf, "replace container name")) {
					replace_container_name_menu = mptr;

				} else if (!stricmp(buf, "replace container data")) {
					replace_container_data_menu = mptr;
				}
			}
		}

		// add popup menus for all the operator categories
		for (i=0; i<(int)op_menu.size(); i++)
		{
			add_op_submenu[i].CreatePopupMenu();
			replace_op_submenu[i].CreatePopupMenu();
			insert_op_submenu[i].CreatePopupMenu();

			add_op_menu->AppendMenu(MF_POPUP, (UINT) add_op_submenu[i].m_hMenu, op_menu[i].name.c_str());
			replace_op_menu->AppendMenu(MF_POPUP, (UINT) replace_op_submenu[i].m_hMenu, op_menu[i].name.c_str());
			insert_op_menu->AppendMenu(MF_POPUP, (UINT) insert_op_submenu[i].m_hMenu, op_menu[i].name.c_str());
		}

		// get rid of the placeholders we needed to ensure popup menus stayed popup menus,
		// i.e. MSDEV will convert empty popup menus into normal menu items.
		add_op_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		replace_op_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		insert_op_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		replace_variable_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		replace_container_name_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
		replace_container_data_menu->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);

		// get item_index
		update_item(h);

		// annotations only work in the event editor
		if (m_mode == MODE_EVENTS)
		{
			menu.EnableMenuItem(ID_EDIT_COMMENT, MF_ENABLED);
			menu.EnableMenuItem(ID_EDIT_BG_COLOR, MF_ENABLED);
		}
		else
		{
			menu.EnableMenuItem(ID_EDIT_COMMENT, MF_GRAYED);
			menu.EnableMenuItem(ID_EDIT_BG_COLOR, MF_GRAYED);
		}

		/*
		Goober5000 - allow variables in all modes;
		the restriction seems unnecessary IMHO
		
		// Do SEXP_VARIABLE stuff here.
		if (m_mode != MODE_EVENTS)
		{
			// only allow variables in event mode
			menu.EnableMenuItem(ID_SEXP_TREE_ADD_VARIABLE, MF_GRAYED);
			menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_GRAYED);
		}
		else
		*/
		{
			menu.EnableMenuItem(ID_SEXP_TREE_ADD_VARIABLE, MF_ENABLED);
			menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_ENABLED);
			
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
						if(temp == -1){
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
						const auto* p_container = get_sexp_container(tree_nodes[parent].text);
						Assertion(p_container != nullptr,
							"Found modifier for unknown container %s. Please report!",
							tree_nodes[parent].text);
						op_type = p_container->opf_type;
					}
					Assertion(op_type > 0,
						"Found invalid operator type %d for node with text %s. Please report!",
						op_type,
						tree_nodes[parent].text);

					// special case don't allow replace data for variable names
					// Goober5000 - why?  the only place this happens is when replacing the ambiguous argument in
					// modify-variable with a variable, which seems legal enough.
					//if (op_type != OPF_AMBIGUOUS) {

						// Goober5000 - given the above, we have to figure out what type this stands for
						if (op_type == OPF_AMBIGUOUS)
						{
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
						if (op_type == OPF_GAME_SND || op_type == OPF_FIREBALL || op_type == OPF_WEAPON_BANK_NUMBER)
						{
							type = SEXPT_NUMBER | SEXPT_STRING;
						}

						// jg18 - container values (container data/map keys) can be anything
						// the type is checked in check_sexp_syntax()
						if (op_type == OPF_CONTAINER_VALUE)
						{
							type = SEXPT_NUMBER | SEXPT_STRING;
						}
						
						if ( (type & SEXPT_STRING) || (type & SEXPT_NUMBER) ) {

							int max_sexp_vars = MAX_SEXP_VARIABLES;
							// prevent collisions in id numbers: ID_VARIABLE_MENU + 512 = ID_ADD_MENU
							Assert(max_sexp_vars < 512);

							for (int idx=0; idx<max_sexp_vars; idx++) {
								if (Sexp_variables[idx].type & SEXP_VARIABLE_SET) {
									// skip block variables
									if (Sexp_variables[idx].type & SEXP_VARIABLE_BLOCK) {
										continue;
									}

									UINT flags = MF_STRING | MF_GRAYED;
									// maybe gray flag MF_GRAYED

									// get type -- gray "string" or number accordingly
									if ( type & SEXPT_STRING ) {
										if ( Sexp_variables[idx].type & SEXP_VARIABLE_STRING ) {
											flags &= ~MF_GRAYED;
										}
									}
									if ( type & SEXPT_NUMBER ) {
										if ( Sexp_variables[idx].type & SEXP_VARIABLE_NUMBER ) {
											flags &= ~MF_GRAYED;
										}
									}

									// if modify-variable and changing variable, enable all variables
									if (op_type == OPF_VARIABLE_NAME) {
										Modify_variable = 1;
										flags &= ~MF_GRAYED;
									} else {
										Modify_variable = 0;
									}

									// enable navsystem always
									if (op_type == OPF_NAV_POINT)
										flags &= ~MF_GRAYED;

									// enable all for container multidimensionality
									if ((type & SEXPT_MODIFIER) && Replace_count > 0)
										flags &= ~MF_GRAYED;

									if (!( (idx + 3) % 30)) {
										flags |= MF_MENUBARBREAK;
									}

									char buf[128];
									// append list of variable names and values
									// set id as ID_VARIABLE_MENU + idx
									sprintf(buf, "%s (%s)", Sexp_variables[idx].variable_name, Sexp_variables[idx].text);

									replace_variable_menu->AppendMenu(flags, (ID_VARIABLE_MENU + idx), buf);
								}
							}

							// Replace Container Name submenu
							if (is_container_name_opf_type(op_type) || op_type == OPF_DATA_OR_STR_CONTAINER) {
								int container_name_index = 0;
								for (const auto &container : get_all_sexp_containers()) {
									UINT flags = MF_STRING | MF_GRAYED;

									if (op_type == OPF_CONTAINER_NAME) {
										// allow all containers
										flags &= ~MF_GRAYED;
									} else if ((op_type == OPF_LIST_CONTAINER_NAME) && container.is_list()) {
										flags &= ~MF_GRAYED;
									} else if ((op_type == OPF_MAP_CONTAINER_NAME) && container.is_map()) {
										flags &= ~MF_GRAYED;
									} else if ((op_type == OPF_DATA_OR_STR_CONTAINER) &&
											   container.is_of_string_type()) {
										flags &= ~MF_GRAYED;
									}

									replace_container_name_menu->AppendMenu(flags,
										(ID_CONTAINER_NAME_MENU + container_name_index++),
										container.container_name.c_str());
								}
							}

							// Replace Container Data submenu
							// disallowed on variable-type SEXP args, to prevent FSO/FRED crashes
							// also disallowed for special argument options (not supported for now)
							// op < 0 means we're on a container modifier, and nested Replace Container Data is allowed
							if (op_type != OPF_VARIABLE_NAME && (op < 0 || !is_argument_provider_op(Operators[op].value))) {
								int container_data_index = 0;
								for (const auto &container : get_all_sexp_containers()) {
									UINT flags = MF_STRING | MF_GRAYED;

									if ((type & SEXPT_STRING) && any(container.type & ContainerType::STRING_DATA)) {
										flags &= ~MF_GRAYED;
									}

									if ((type & SEXPT_NUMBER) && any(container.type & ContainerType::NUMBER_DATA)) {
										flags &= ~MF_GRAYED;
									}

									// enable all for container multidimensionality
									if ((tree_nodes[item_index].type & SEXPT_MODIFIER) && Replace_count > 0)
										flags &= ~MF_GRAYED;

									replace_container_data_menu->AppendMenu(flags,
										(ID_CONTAINER_DATA_MENU + container_data_index++),
										container.container_name.c_str());
								}
							}
						}
					//}
				}
			}

			// can't modify if no variables
			if (sexp_variable_count() == 0) {
				menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_GRAYED);
			}
		}

		// add all the submenu items first
		for (i=0; i<(int)op_submenu.size(); i++)
		{
			add_op_subcategory_menu[i].CreatePopupMenu();
			replace_op_subcategory_menu[i].CreatePopupMenu();
			insert_op_subcategory_menu[i].CreatePopupMenu();
			
			for (j=0; j<(int)op_menu.size(); j++)
			{
				if (op_menu[j].id == category_of_subcategory(op_submenu[i].id))
				{
					add_op_submenu[j].AppendMenu(MF_POPUP, (UINT) add_op_subcategory_menu[i].m_hMenu, op_submenu[i].name.c_str());
					replace_op_submenu[j].AppendMenu(MF_POPUP, (UINT) replace_op_subcategory_menu[i].m_hMenu, op_submenu[i].name.c_str());
					insert_op_submenu[j].AppendMenu(MF_POPUP, (UINT) insert_op_subcategory_menu[i].m_hMenu, op_submenu[i].name.c_str());
					break;	// only 1 category valid
				}
			}
		}

		// add operator menu items to the various CATEGORY submenus they belong in
		for (i=0; i<(int)Operators.size(); i++)
		{
			// add only if it is not in a subcategory
			subcategory_id = get_subcategory(Operators[i].value);
			if (subcategory_id == OP_SUBCATEGORY_NONE)
			{
				// put it in the appropriate menu
				for (j=0; j<(int)op_menu.size(); j++)
				{
					if (op_menu[j].id == get_category(Operators[i].value))
					{
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

							// hide these operators per GitHub issue #6400
							case OP_GET_VARIABLE_BY_INDEX:
							case OP_SET_VARIABLE_BY_INDEX:
							case OP_COPY_VARIABLE_FROM_INDEX:
							case OP_COPY_VARIABLE_BETWEEN_INDEXES:

							// unlike the various campaign operators, these are deprecated
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
								j = (int)op_menu.size();	// don't allow these operators to be visible
								break;
						}

						if (j < (int)op_menu.size()) {
							add_op_submenu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value, Operators[i].text.c_str());
							replace_op_submenu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value | OP_REPLACE_FLAG, Operators[i].text.c_str());
							insert_op_submenu[j].AppendMenu(MF_STRING, Operators[i].value | OP_INSERT_FLAG, Operators[i].text.c_str());
						}

						break;	// only 1 category valid
					}
				}
			}
			// if it is in a subcategory, handle it
			else
			{
				// put it in the appropriate submenu
				for (j=0; j<(int)op_submenu.size(); j++)
				{
					if (op_submenu[j].id == subcategory_id)
					{
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

							// hide these operators per GitHub issue #6400
							case OP_GET_VARIABLE_BY_INDEX:
							case OP_SET_VARIABLE_BY_INDEX:
							case OP_COPY_VARIABLE_FROM_INDEX:
							case OP_COPY_VARIABLE_BETWEEN_INDEXES:

							// unlike the various campaign operators, these are deprecated
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
								j = (int)op_submenu.size();	// don't allow these operators to be visible
								break;
						}

						if (j < (int)op_submenu.size()) {
							add_op_subcategory_menu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value, Operators[i].text.c_str());
							replace_op_subcategory_menu[j].AppendMenu(MF_STRING | MF_GRAYED, Operators[i].value | OP_REPLACE_FLAG, Operators[i].text.c_str());
							insert_op_subcategory_menu[j].AppendMenu(MF_STRING, Operators[i].value | OP_INSERT_FLAG, Operators[i].text.c_str());
						}

						break;	// only 1 subcategory valid
					}
				}
			}
		}

		// find local index (i) of current item (from its handle)
		SelectItem(h);
		update_item(h);

		// special case: item is a ROOT node, and a label that can be edited (not an item in the sexp tree)
		if ((item_index == -1) && (m_mode & ST_LABELED_ROOT)) {
			if (m_mode & ST_ROOT_EDITABLE) {
				menu.EnableMenuItem(ID_EDIT_TEXT, MF_ENABLED);
			} else {
				menu.EnableMenuItem(ID_EDIT_TEXT, MF_GRAYED);
			}

			// disable copy, insert op
			menu.EnableMenuItem(ID_EDIT_COPY, MF_GRAYED);
			for (j=0; j<(int)Operators.size(); j++) {
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
			}

			gray_menu_tree(popup_menu);
			popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, mouse.x, mouse.y, this);
			return;
		}

		Assert(item_index != -1);  // handle not found, which should be impossible.
		if (!(tree_nodes[item_index].flags & EDITABLE)) {
			menu.EnableMenuItem(ID_EDIT_TEXT, MF_GRAYED);
		}

		if (tree_nodes[item_index].parent == -1) {  // root node
			menu.EnableMenuItem(ID_DELETE, MF_GRAYED);  // can't delete the root item.
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
				menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);
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
							menu.EnableMenuItem(Operators[ptr->op].value, MF_ENABLED);
						} else {
							// add data
							if ((data_idx + 3) % 30) {
								add_data_menu->AppendMenu(MF_STRING | MF_ENABLED, ID_ADD_MENU + data_idx, ptr->text.c_str());
							} else {
								add_data_menu->AppendMenu(MF_MENUBARBREAK | MF_STRING | MF_ENABLED, ID_ADD_MENU + data_idx, ptr->text.c_str());
							}
						}

						data_idx++;
						ptr = ptr->next;
					}
				}

				menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);
				menu.EnableMenuItem(ID_ADD_STRING, MF_ENABLED);
			}
		} else if (tree_nodes[item_index].flags & OPERAND)	{
			add_type = OPR_STRING;
			int child = tree_nodes[item_index].child;
			Add_count = count_args(child);
			op = get_operator_index(tree_nodes[item_index].text);
			Assert(op >= 0);

			// get listing of valid argument values and add to menus
			type = query_operator_argument_type(op, Add_count);
			list = get_listing_opf(type, item_index, Add_count);
			if (list) {
				sexp_list_item *ptr;

				int data_idx = 0;
				ptr = list;
				while (ptr) {
					if (ptr->op >= 0) {
						// enable operators with correct return type
						menu.EnableMenuItem(Operators[ptr->op].value, MF_ENABLED);

					} else {
						UINT flags = MF_STRING | MF_ENABLED;

						if (!((data_idx + 3) % 30)) {
							flags |= MF_MENUBARBREAK;
						}

						// add data
						if (type == OPF_VARIABLE_NAME) {
							char buf[128];
							sprintf(buf, "%s (%s)", Sexp_variables[data_idx].variable_name, Sexp_variables[data_idx].text);
							add_data_menu->AppendMenu(flags, ID_ADD_MENU + data_idx, buf);
						} else {
							add_data_menu->AppendMenu(flags, ID_ADD_MENU + data_idx, ptr->text.c_str());
						}
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
				menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);

			} else if (type == OPF_POSITIVE) {  // takes non-negative numbers
				add_type = OPR_POSITIVE;
				menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);

			} else if (type == OPF_BOOL) {  // takes true/false bool values
				add_type = OPR_BOOL;

			} else if (type == OPF_AI_GOAL) {
				add_type = OPR_AI_GOAL;

			} else if (type == OPF_CONTAINER_VALUE) {
				// allow both strings and numbers
				// types are checked in check_sepx_syntax()
				menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);
			}

			// add_type unchanged from above
			if (add_type == OPR_STRING && !is_container_name_opf_type(type)) {
				menu.EnableMenuItem(ID_ADD_STRING, MF_ENABLED);
			}

			list->destroy();
		}

		// disable operators that do not have arguments available
		for (j=0; j<(int)Operators.size(); j++) {
			if (!query_default_argument_available(j)) {
				menu.EnableMenuItem(Operators[j].value, MF_GRAYED);
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
					menu.EnableMenuItem(ID_DELETE, MF_GRAYED);
				}
			} else if ((tree_nodes[parent].type & SEXPT_CONTAINER_DATA) && (item_index == first_arg)) {
				// a container data node's initial modifier can't be deleted
				Assertion(tree_nodes[item_index].type & SEXPT_MODIFIER,
					"Container data %s node's first modifier %s is not a modifier. Please report!",
					tree_nodes[parent].text,
					tree_nodes[item_index].text);
				menu.EnableMenuItem(ID_DELETE, MF_GRAYED);
			}

			// get arg count of item to replace
			Replace_count = 0;
			int temp = first_arg;
			while (temp != item_index) {
				Replace_count++;
				temp = tree_nodes[temp].next;

				// DB - added 3/4/99
				if(temp == -1){
					break;
				}
			}

			if (op >= 0) {
				// maybe gray delete
				for (i = Replace_count + 1; i < count; i++) {
					if (query_operator_argument_type(op, i - 1) != query_operator_argument_type(op, i)) {
						menu.EnableMenuItem(ID_DELETE, MF_GRAYED);
						break;
					}
				}

				type = query_operator_argument_type(op, Replace_count); // check argument type at this position
			} else {
				Assertion(tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
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
				sexp_list_item *ptr;

				int data_idx = 0;
				ptr = list;
				while (ptr) {
					if (ptr->op >= 0) {
						menu.EnableMenuItem(Operators[ptr->op].value | OP_REPLACE_FLAG, MF_ENABLED);

					} else {
						if ( (data_idx + 3) % 30)
							replace_data_menu->AppendMenu(MF_STRING | MF_ENABLED, ID_REPLACE_MENU + data_idx, ptr->text.c_str());
						else
							replace_data_menu->AppendMenu(MF_MENUBARBREAK | MF_STRING | MF_ENABLED, ID_REPLACE_MENU + data_idx, ptr->text.c_str());
					}

					data_idx++;
					ptr = ptr->next;
				}
			}

			if (type == OPF_NONE) {  // takes no arguments
				replace_type = 0;

			} else if (type == OPF_NUMBER) {  // takes numbers
				replace_type = OPR_NUMBER;
				menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);

			} else if (type == OPF_POSITIVE) {  // takes non-negative numbers
				replace_type = OPR_POSITIVE;
				menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);

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
				menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);

			} else if (type == OPF_CONTAINER_VALUE) {
				// allow strings and numbers
				// type is checked in check_sexp_syntax()
				menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
			}

			// default to string, except for container names
			if (replace_type == OPR_STRING && !is_container_name_opf_type(type)) {
				menu.EnableMenuItem(ID_REPLACE_STRING, MF_ENABLED);
			}

			if (op >= 0) { // skip when handling "replace container data"
				// modify string or number if (modify_variable)
				if (Operators[op].value == OP_MODIFY_VARIABLE) {
					int modify_type = get_modify_variable_type(parent);

					if (modify_type == OPF_NUMBER) {
						menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
						menu.EnableMenuItem(ID_REPLACE_STRING, MF_GRAYED);
					}
					// no change for string type
				}
				else if (Operators[op].value == OP_SET_VARIABLE_BY_INDEX) {
					// it depends on which argument we are modifying
					// first argument is always a number
					if (Replace_count == 0) {
						menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
						menu.EnableMenuItem(ID_REPLACE_STRING, MF_GRAYED);
					}
					// second argument could be anything
					else {
						int modify_type = get_modify_variable_type(parent);

						if (modify_type == OPF_NUMBER) {
							menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
							menu.EnableMenuItem(ID_REPLACE_STRING, MF_GRAYED);
						}
						// no change for string type
					}
				}
			}

			if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
				Assertion(tree_nodes[parent].type & SEXPT_CONTAINER_DATA,
					"Container modifier found whose parent %s is not a container. Please report!",
					tree_nodes[parent].text);
				const int first_modifier_node = tree_nodes[parent].child;
				Assertion(first_modifier_node != -1,
					"Container data node named %s has no modifier. Please report!",
					tree_nodes[parent].text);
				const auto *p_container = get_sexp_container(tree_nodes[parent].text);
				Assertion(p_container,
					"Attempt to get first modifier for unknown container %s. Please report!",
					tree_nodes[parent].text);
				const auto &container = *p_container;

				if (Replace_count == 0) {
					if (container.is_list()) {
						// the only valid values are either the list modifiers or Replace Variable/Cotnainer Data with string data
						menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_GRAYED);
						menu.EnableMenuItem(ID_REPLACE_STRING, MF_GRAYED);
						menu.EnableMenuItem(ID_EDIT_TEXT, MF_GRAYED);
					} else if (container.is_map()) {
						if (any(container.type & ContainerType::STRING_KEYS)) {
							menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_GRAYED);
							menu.EnableMenuItem(ID_REPLACE_STRING, MF_ENABLED);
						} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
							menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
							menu.EnableMenuItem(ID_REPLACE_STRING, MF_GRAYED);
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
					menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
					menu.EnableMenuItem(ID_REPLACE_STRING, MF_GRAYED);
				} else {
					// multidimensional modifiers can be anything, including possibly a list modifier
					// the value can be validated only at runtime (i.e., in-mission)
					menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
					menu.EnableMenuItem(ID_REPLACE_STRING, MF_ENABLED);
				}
			}

			list->destroy();

		} else {  // top node, so should be a Boolean type.
			if (m_mode == MODE_EVENTS) {  // return type should be null
				replace_type = OPR_NULL;
				for (j=0; j<(int)Operators.size(); j++)
					if (query_operator_return_type(j) == OPR_NULL)
						menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_ENABLED);

			} else {
				replace_type = OPR_BOOL;
				for (j=0; j<(int)Operators.size(); j++)
					if (query_operator_return_type(j) == OPR_BOOL)
						menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_ENABLED);
			}
		}

		// disable operators that do not have arguments available
		for (j=0; j<(int)Operators.size(); j++) {
			if (!query_default_argument_available(j)) {
				menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_GRAYED);
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
				type = query_operator_argument_type(op, count);
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
			if (m_mode == MODE_EVENTS)
				type = OPF_NULL;
			else
				type = OPF_BOOL;
		}

		for (j=0; j<(int)Operators.size(); j++) {
			z = query_operator_return_type(j);
			if (!sexp_query_type_match(type, z) || (Operators[j].min < 1))
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);

			z = query_operator_argument_type(j, 0);
			if ((type == OPF_NUMBER) && (z == OPF_POSITIVE))
				z = OPF_NUMBER;

			// Goober5000's number hack
			if ((type == OPF_POSITIVE) && (z == OPF_NUMBER))
				z = OPF_POSITIVE;

			if (z != type)
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
		}

		// disable operators that do not have arguments available
		for (j=0; j<(int)Operators.size(); j++) {
			if (!query_default_argument_available(j)) {
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
			}
		}


		// disable non campaign operators if in campaign mode
		for (j=0; j<(int)Operators.size(); j++) {
			z = 0;
			if (m_mode == MODE_CAMPAIGN) {
				if (!usable_in_campaign(Operators[j].value))
					z = 1;
			}

			if (z) {
				menu.EnableMenuItem(Operators[j].value, MF_GRAYED);
				menu.EnableMenuItem(Operators[j].value | OP_REPLACE_FLAG, MF_GRAYED);
				menu.EnableMenuItem(Operators[j].value | OP_INSERT_FLAG, MF_GRAYED);
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

				if ((z == OPR_POSITIVE) && (replace_type == OPR_NUMBER))
					z = OPR_NUMBER;

				// Goober5000's number hack
				if ((z == OPR_NUMBER) && (replace_type == OPR_POSITIVE))
					z = OPR_POSITIVE;

				if (replace_type == z)
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				z = query_operator_return_type(j);
				if ((z == OPR_POSITIVE) && (add_type == OPR_NUMBER))
					z = OPR_NUMBER;

				if (add_type == z)
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

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
							menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);
						if (add_type == OPR_NUMBER)
							menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);
					} else if (any(container.type & ContainerType::STRING_DATA)) {
						if (replace_type == OPR_STRING && !is_container_name_opf_type(type))
							menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);
						if (add_type == OPR_STRING && !is_container_name_opf_type(type))
							menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);
					} else {
						UNREACHABLE("Unknown container data type %d", (int)container.type);
					}
				}

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
				if ((replace_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1))
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				else if (replace_type == OPR_NUMBER)
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				if ((add_type == OPR_POSITIVE) && (atoi(CTEXT(Sexp_clipboard)) > -1))
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

				else if (add_type == OPR_NUMBER)
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

			} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
				if (replace_type == OPR_STRING && !is_container_name_opf_type(type))
					menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);

				if (add_type == OPR_STRING && !is_container_name_opf_type(type))
					menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);

			} else
				Int3();  // unknown and/or invalid sexp type
		}

		if (!(menu.GetMenuState(ID_DELETE, MF_BYCOMMAND) & MF_GRAYED))
			menu.EnableMenuItem(ID_EDIT_CUT, MF_ENABLED);

		// all of the following restrictions may be revisited in the future
		if (tree_nodes[item_index].type & (SEXPT_MODIFIER | SEXPT_CONTAINER_NAME)) {
			// modifiers and container names don't support cut/copy/paste
			menu.EnableMenuItem(ID_EDIT_CUT, MF_GRAYED);
			menu.EnableMenuItem(ID_EDIT_COPY, MF_GRAYED);
			menu.EnableMenuItem(ID_EDIT_PASTE, MF_GRAYED);
		}
		// can't use else-if here, because container data is a valid modifier
		if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
			// container data nodes don't support add-pasting modifiers
			menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_GRAYED);
		}

		gray_menu_tree(popup_menu);
		popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, mouse.x, mouse.y, this);
	}
}

// counts the number of arguments an operator has.  Call this with the node of the first
// argument of the operator
int sexp_tree::count_args(int node)
{
	return _model.count_args(node);
}

// identify what type of argument this is.  You call it with the node of the first argument
// of an operator.  It will search through enough of the arguments to determine what type of
// data they are.
int sexp_tree::identify_arg_type(int node)
{
	return _model.identify_arg_type(node);
}

// determine if an item should be editable.  This doesn't actually edit the label.
int sexp_tree::edit_label(HTREEITEM h, bool *is_operator)
{
	uint i;

	if (is_operator != nullptr)
		*is_operator = false;

	for (i=0; i<tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			break;
		}
	}

	// Check if tree root
	if (i == tree_nodes.size()) {
		if (m_mode & ST_ROOT_EDITABLE) {
			return 1;
		}

		return 0;
	}

	// Operators are editable
	if (tree_nodes[i].type & SEXPT_OPERATOR) {
		if (is_operator != nullptr)
			*is_operator = true;
		return 1;
	}

	// Variables and containers must be edited through dialog box
	if (tree_nodes[i].type & (SEXPT_VARIABLE | SEXPT_CONTAINER_NAME | SEXPT_CONTAINER_DATA)) {
		return 0;
	}

	// Don't edit if not flaged as editable
	if (!(tree_nodes[i].flags & EDITABLE)) {
		return 0;
	}

	// Otherwise, allow editing
	return 1;

/*
	if (tree_nodes[i].flags & OPERAND) {
		data = tree_nodes[i].child;

		SetItemText(h, tree_nodes[i].text);
		tree_nodes[i].flags = OPERAND;
		tree_nodes[data].handle = insert(tree_nodes[data].text, tree_nodes[data].type, tree_nodes[data].flags, h);
		item_handle = tree_item_handle(tree_nodes[data]);
		tree_nodes[data].flags = EDITABLE;
		Expand(h, TVE_EXPAND);
		SelectItem(item_handle);
		return 2;
	}
*/
}

void sexp_tree::edit_comment(HTREEITEM h)
{
	// Not implemented in the base class
}

void sexp_tree::edit_bg_color(HTREEITEM h)
{
	// Not implemented in the base class
}

// given a tree node, returns the argument type it should be.
// OPF_NULL means no value (or a "void" value) is returned.  OPF_NONE means there shouldn't be any argument at this position at all.
int sexp_tree::query_node_argument_type(int node) const
{
	return _model.query_node_argument_type(node);
}

int sexp_tree::end_label_edit(TVITEMA &item)
{
	if (!item.pszText)
		return 0;

	HTREEITEM h = item.hItem; 
	SCP_string str(item.pszText);
	int r = 1;	
	bool update_node = true; 
	uint node;

	for (node=0; node<tree_nodes.size(); node++)
		if (tree_nodes[node].handle == h)
			break;

	if (node == tree_nodes.size()) {
		if (m_mode == MODE_EVENTS) {
			item_index = (int)GetItemData(h);
			Assert(Event_editor_dlg);
			node = Event_editor_dlg->handler(ROOT_RENAMED, item_index, str.c_str());
			return 1;

		} else
			Int3();  // root labels shouldn't have been editable!
	}

	Assert(node < tree_nodes.size());
	if (tree_nodes[node].type & SEXPT_OPERATOR) {
		auto op = match_closest_operator(str, node);
		if (op.empty()) return 0;	// Goober5000 - avoids crashing

		// use the text of the operator we found
		SetItemText(h, op.c_str());
		str = op;

		item_index = node;
		int op_num = get_operator_index(op.c_str()); 
		if (op_num >= 0 ) {
			add_or_replace_operator(op_num, 1);
		}
		else {
			update_node = false;
		}
		r = 0;
	}
	// gotta sidestep Goober5000's number hack and check entries are actually positive. 
	else if (tree_nodes[node].type & SEXPT_NUMBER) {
		if (query_node_argument_type(node) == OPF_POSITIVE) {
			int val = atoi(str.c_str()); 
			if (val < 0) {
				MessageBox("Can not enter a negative value", "Invalid Number", MB_ICONEXCLAMATION); 
				update_node = false;
			}
		}
	}

	// Error checking would not hurt here
	auto len = str.size();
	if (len >= TOKEN_LENGTH)
		len = TOKEN_LENGTH - 1;

	if (update_node) {
		*modified = 1;
		strncpy(tree_nodes[node].text, str.c_str(), len);
		tree_nodes[node].text[len] = 0;

		// let's make sure we aren't introducing any invalid characters, per Mantis #2893
		lcl_fred_replace_stuff(tree_nodes[node].text, TOKEN_LENGTH - 1);
	}
	else {
		item.pszText = tree_nodes[node].text;
		return 1;
	}

	return r;
}

// Look for the valid operator that is the closest match for 'str' and return the operator
// number of it.  What operators are valid is determined by 'node', and an operator is valid
// if it is allowed to fit at position 'node'
//
const SCP_string &sexp_tree::match_closest_operator(const SCP_string &str, int node)
{
	return _model.match_closest_operator(str, node);
}

void sexp_tree::start_operator_edit(HTREEITEM h)
{
	if (m_operator_popup_active)
		return;

	// this can get out of sync if we add an event and then try to edit an operator in a different event
	update_item(h);

	// sanity checks
	Assertion(item_handle == h, "Mismatch between item handle and the handle being edited!");
	Assertion(item_index >= 0 && item_index < (int)tree_nodes.size() && !tree_nodes.empty(), "Unknown node being edited!");
	Assertion(tree_nodes[item_index].handle == item_handle, "Mismatch between tree node and item handle!");

	// we are editing an operator, so find out which type it should be
	auto opf_type = (sexp_opf_t)query_node_argument_type(item_index);

	// do first-time setup
	if (!m_operator_popup_created)
	{
		// text metrics
		TEXTMETRIC tm;
		auto dc = GetDC();
		dc->GetTextMetrics(&tm);
		m_font_height = tm.tmHeight;
		dc->SelectObject(GetFont());
		m_font_max_width = 0;
		for (auto& op : Operators)
		{
			auto font_extent = dc->GetTextExtent(op.text.c_str());
			if (font_extent.cx > m_font_max_width)
				m_font_max_width = font_extent.cx;
		}

		// adjust for scroll bar and border edge
		m_font_max_width += GetSystemMetrics(SM_CXVSCROLL) + 2 * GetSystemMetrics(SM_CXEDGE);
	}

	// calculate position and size of the dropdown
	RECT item_rect, dropdown_rect;
	GetItemRect(h, &item_rect, TRUE);
	dropdown_rect.top = item_rect.top;
	dropdown_rect.left = item_rect.left;
	dropdown_rect.right = dropdown_rect.left + m_font_max_width;
	dropdown_rect.bottom = dropdown_rect.top + m_font_height * 10;

	// create or just position it
	if (!m_operator_popup_created)
	{
		m_operator_box.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_SIMPLE | CBS_HASSTRINGS | CBS_OWNERDRAWFIXED, dropdown_rect, this, IDC_SEXP_POPUP_LIST);
		m_operator_box.SetFont(GetFont());
		m_operator_popup_created = true;
	}
	else
	{
		m_operator_box.MoveWindow(&dropdown_rect);
	}

	m_operator_box.refresh_popup_operators(opf_type, tree_nodes[item_index].text);

	m_operator_box.ShowWindow(SW_SHOWNORMAL);
	m_operator_box.SetFocus();
	m_operator_popup_active = true;
}

void sexp_tree::end_operator_edit(bool confirm)
{
	if (!m_operator_popup_active)
		return;

	m_operator_box.cleanup(confirm);
	m_operator_box.ShowWindow(SW_HIDE);
	m_operator_popup_active = false;
}

// this really only handles messages generated by the right click popup menu
// now it also handles messages from the operator combo box
BOOL sexp_tree::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int i, z, id, data, node, op;
	sexp_list_item *list, *ptr;
	HTREEITEM h;

	if ((item_index >= 0) && (item_index < total_nodes) && !tree_nodes.empty())
		item_handle = tree_item_handle(tree_nodes[item_index]);

	id = LOWORD(wParam);
	data = HIWORD(wParam);

#ifndef NDEBUG
	// Sanity check: None of these #defines used in this function should overlap with any SEXP operators, or mysterious bugs will ensue
	// note: this won't catch operator values plus OP_REPLACE_FLAG or OP_INSERT_FLAG
	switch (id)
	{
		case ID_SEXP_TREE_ADD_VARIABLE:
		case ID_SEXP_TREE_MODIFY_VARIABLE:
		case ID_EDIT_SEXP_TREE_EDIT_CONTAINERS:
		case ID_EDIT_COPY:
		case ID_EDIT_PASTE:
		case ID_EDIT_PASTE_SPECIAL:
		case ID_SPLIT_LINE:
		case ID_EXPAND_ALL:
		case ID_EDIT_TEXT:
		case ID_EDIT_COMMENT:
		case ID_EDIT_BG_COLOR:
		case ID_REPLACE_NUMBER:
		case ID_REPLACE_STRING:
		case ID_ADD_STRING:
		case ID_ADD_NUMBER:
		case ID_EDIT_CUT:
		case ID_DELETE:
		case IDC_SEXP_POPUP_LIST:
			Assertion(id >= sexp::operator_upper_bound(), "A resource definition (%d) must not overlap with an operator value!", id);
			break;

		default:
			if ((id >= ID_VARIABLE_MENU) && (id < ID_VARIABLE_MENU + 511)
				|| (id >= ID_ADD_MENU) && (id < ID_ADD_MENU + 511)
				|| (id >= ID_REPLACE_MENU) && (id < ID_REPLACE_MENU + 511)
				|| (id >= ID_CONTAINER_NAME_MENU) && (id < ID_CONTAINER_NAME_MENU + 511)
				|| (id >= ID_CONTAINER_DATA_MENU) && (id < ID_CONTAINER_DATA_MENU + 511))
			{
				Assertion(id >= sexp::operator_upper_bound(), "A resource definition (%d) must not overlap with an operator value!", id);
			}
			break;
	}
#endif // !NDEBUG

	// Add variable
	if (id == ID_SEXP_TREE_ADD_VARIABLE) {
		CAddVariableDlg dlg;
		dlg.DoModal();

		if ( dlg.m_create ) {

			// set type
			int type;
			if ( dlg.m_type_number ) {
				type = SEXP_VARIABLE_NUMBER;
			} else {
				type = SEXP_VARIABLE_STRING;
			}

			if ( dlg.m_type_network_variable ) {
				type |= SEXP_VARIABLE_NETWORK;
			}

			if ( dlg.m_type_on_mission_progress) {
				type |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
			} else if ( dlg.m_type_on_mission_close) {
				type |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
			}

			if (dlg.m_type_eternal) {
				type |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
			}

			// add variable
			sexp_add_variable(dlg.m_default_value, dlg.m_variable_name, type);

			// sort variable
			sexp_variable_sort();
		}
		return 1;
	}

	// Modify variable
	if (id == ID_SEXP_TREE_MODIFY_VARIABLE) {
		CModifyVariableDlg dlg;

		// get sexp_variable index for item index
		dlg.m_start_index = get_item_index_to_var_index();

		// get pointer to tree
		dlg.m_p_sexp_tree = this;

		dlg.DoModal();

		Assert( !(dlg.m_deleted && dlg.m_do_modify) );

		if (dlg.m_deleted) {
			// find index in sexp_variable list
			int sexp_var_index = get_index_sexp_variable_name(dlg.m_cur_variable_name);
			Assert(sexp_var_index != -1);

			// delete from list
			sexp_variable_delete(sexp_var_index);

			// sort list
			sexp_variable_sort();

			// delete from sexp_tree, replacing with "number" or "string" as needed
			// further error checking from add_data()
			delete_sexp_tree_variable(dlg.m_cur_variable_name);

			return 1;
		}

		if (dlg.m_do_modify) {
			// check sexp_tree -- warn on type
			// find index and change either (1) name, (2) type, (3) value
			int sexp_var_index = get_index_sexp_variable_name(dlg.m_old_var_name);
			Assert(sexp_var_index != -1);

			// save old name, since name may be modified
			char old_name[TOKEN_LENGTH];
			strcpy_s(old_name, Sexp_variables[sexp_var_index].variable_name);

			// set type
			int type;
			if (dlg.m_type_number) {
				type = SEXP_VARIABLE_NUMBER;
			} else {
				type = SEXP_VARIABLE_STRING;
			}

			if ( dlg.m_type_network_variable ) {
				type |= SEXP_VARIABLE_NETWORK;
			}

			if ( dlg.m_type_on_mission_progress) {
				type |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
			} else if ( dlg.m_type_on_mission_close) {
				type |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
			}

			if (dlg.m_type_eternal) {
				type |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
			}

			// update sexp_variable
			sexp_fred_modify_variable(dlg.m_default_value, dlg.m_cur_variable_name, sexp_var_index, type);

			// modify sexp_tree
			modify_sexp_tree_variable(old_name, sexp_var_index);

			// Don't sort until after modify, since modify uses index
			if (dlg.m_modified_name) {
				sexp_variable_sort();
			}

			return 1;
		}

		// no change
		return 1;
	}

	// Add/Modify Container
	if (id == ID_EDIT_SEXP_TREE_EDIT_CONTAINERS) {
		CAddModifyContainerDlg dlg(*this);

		dlg.DoModal();

		bool renamed_anything = false;
		for (const auto &renamed_container : dlg.get_renamed_containers()) {
			const SCP_string &old_name = renamed_container.first;
			const SCP_string &new_name = renamed_container.second;
			if (rename_container_nodes(old_name, new_name)) {
				renamed_anything = true;
			}
		}

		if (renamed_anything) {
			*modified = 1;
		}

		return 1;
	}

	// check if REPLACE_VARIABLE_MENU
	if ( (id >= ID_VARIABLE_MENU) && (id < ID_VARIABLE_MENU + 511)) {

		Assert(item_index >= 0);

		// get index into list of type valid variables
		int var_idx = id - ID_VARIABLE_MENU;
		Assert( (var_idx >= 0) && (var_idx < MAX_SEXP_VARIABLES) );

		int type = get_type(item_handle);
		Assert( (type & SEXPT_NUMBER) || (type & SEXPT_STRING) );

		// don't do type check for modify-variable or OPF_CONTAINER_VALUE (can be either type)
		if (Modify_variable || query_node_argument_type(item_index) == OPF_CONTAINER_VALUE) {
			if (Sexp_variables[var_idx].type & SEXP_VARIABLE_NUMBER) {
				type = SEXPT_NUMBER;
			} else if (Sexp_variables[var_idx].type & SEXP_VARIABLE_STRING) {
				type = SEXPT_STRING;
			} else {
				Int3();	// unknown type
			}

		} else {	
			// verify type in tree is same as type in Sexp_variables array
			if (type & SEXPT_NUMBER) {
				Assert(Sexp_variables[var_idx].type & SEXP_VARIABLE_NUMBER);
			}

			if (type & SEXPT_STRING) {
				Assert( (Sexp_variables[var_idx].type & SEXP_VARIABLE_STRING) );
			}
		}

		// Replace data
		replace_variable_data(var_idx, (type | SEXPT_VARIABLE));

		return 1;
	}


	if ((id >= ID_ADD_MENU) && (id < ID_ADD_MENU + 511)) {
		auto saved_id = id;
		Assert(item_index >= 0);

		int type = 0;

		if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
			list = get_container_multidim_modifiers(item_index);
		} else {
			op = get_operator_index(tree_nodes[item_index].text);
			Assert(op >= 0);

			type = query_operator_argument_type(op, Add_count);
			list = get_listing_opf(type, item_index, Add_count);
		}
		Assert(list);

		id -= ID_ADD_MENU;
		ptr = list;
		while (id) {
			id--;
			ptr = ptr->next;
			Assert(ptr);
		}

		Assert((SEXPT_TYPE(ptr->type) != SEXPT_OPERATOR) && (ptr->op < 0));
		expand_operator(item_index);
		node = add_data(ptr->text.c_str(), ptr->type);
		list->destroy();

		// bolted-on ugly hack
		if (type == OPF_VARIABLE_NAME) {
			auto var_idx = saved_id - ID_ADD_MENU;
			auto saved_item_index = item_index;

			if (Sexp_variables[var_idx].type & SEXP_VARIABLE_NUMBER) {
				type = SEXPT_NUMBER;
			}
			else if (Sexp_variables[var_idx].type & SEXP_VARIABLE_STRING) {
				type = SEXPT_STRING;
			}
			else {
				UNREACHABLE("Unknown sexp variable type");
			}

			item_index = node;
			replace_variable_data(var_idx, (type | SEXPT_VARIABLE));
			item_index = saved_item_index;
		}

		return 1;
	}

	if ((id >= ID_REPLACE_MENU) && (id < ID_REPLACE_MENU + 511)) {
		Assert(item_index >= 0);
		Assert(tree_nodes[item_index].parent >= 0);

		if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
			list = get_container_modifiers(tree_nodes[item_index].parent);
		} else {
			op = get_operator_index(tree_nodes[tree_nodes[item_index].parent].text);
			Assert(op >= 0);

			auto type = query_operator_argument_type(op, Replace_count); // check argument type at this position
			list = get_listing_opf(type, tree_nodes[item_index].parent, Replace_count);
		}
		Assert(list);

		id -= ID_REPLACE_MENU;
		ptr = list;
		while (id) {
			id--;
			ptr = ptr->next;
			Assert(ptr);
		}

		Assert((SEXPT_TYPE(ptr->type) != SEXPT_OPERATOR) && (ptr->op < 0));
		expand_operator(item_index);
		replace_data(ptr->text.c_str(), ptr->type);
		list->destroy();
		return 1;
	}

	if ((id >= ID_CONTAINER_NAME_MENU) && (id < ID_CONTAINER_NAME_MENU + 511)) {
		Assertion(item_index >= 0, "Attempt to Replace Container Name with no node selected. Please report!");

		const auto &containers = get_all_sexp_containers();
		const int container_index = id - ID_CONTAINER_NAME_MENU;
		Assertion((container_index >= 0) && (container_index < (int)containers.size()),
			"Unknown Container Index %d. Please report!",
			container_index);

		const int type = get_type(item_handle);
		Assertion(type & SEXPT_STRING,
			"Attempt to replace container name on non-string node %s with type %d. Please report!",
			tree_nodes[item_index].text,
			type);

		replace_container_name(containers[container_index]);
	}

	if ((id >= ID_CONTAINER_DATA_MENU) && (id < ID_CONTAINER_DATA_MENU + 511)) {
		Assertion(item_index >= 0, "Attempt to Replace Container Data with no node selected. Please report!");

		const auto &containers = get_all_sexp_containers();
		const int container_index = id - ID_CONTAINER_DATA_MENU;
		Assertion((container_index >= 0) && (container_index < (int)containers.size()),
			"Unknown Container Index %d. Please report!",
			container_index);

		int type = get_type(item_handle);
		Assertion((type & SEXPT_NUMBER) || (type & SEXPT_STRING),
			"Attempt to use Replace Container Data on a non-data node. Please report!");

		// variable/container name don't mix with container data
		// DISCUSSME: what about variable name as SEXP arg type?
		type &= ~(SEXPT_VARIABLE | SEXPT_CONTAINER_NAME);
		replace_container_data(containers[container_index], (type | SEXPT_CONTAINER_DATA), true, true, true);

		HTREEITEM handle = tree_item_handle(tree_nodes[item_index]);
		expand_branch(handle);
	}

	for (op=0; op<(int)Operators.size(); op++) {
		if (id == Operators[op].value) {
			add_or_replace_operator(op);
			return 1;
		}

		if (id == (Operators[op].value | OP_REPLACE_FLAG)) {
			add_or_replace_operator(op, 1);
			expand_branch(item_handle); 
			return 1;
		}

		if (id == (Operators[op].value | OP_INSERT_FLAG)) {
			int flags;

			z = tree_nodes[item_index].parent;
			flags = tree_nodes[item_index].flags;
			node = allocate_node(z, item_index);
			set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
			tree_nodes[node].flags = flags;
			if (z >= 0)
				h = tree_item_handle(tree_nodes[z]);

			else {
				h = GetParentItem(tree_item_handle(tree_nodes[item_index]));
				if (m_mode == MODE_GOALS) {
					Assert(Goal_editor_dlg);
					Goal_editor_dlg->insert_handler(item_index, node);
					SetItemData(h, node);

				} else if (m_mode == MODE_EVENTS) {
					Assert(Event_editor_dlg);
					Event_editor_dlg->insert_handler(item_index, node);
					SetItemData(h, node);

				} else if (m_mode == MODE_CAMPAIGN) {
					Campaign_tree_formp->insert_handler(item_index, node);
					SetItemData(h, node);

				} else {
					h = TVI_ROOT;
					root_item = node;
				}
			}

			tree_nodes[node].handle = insert(Operators[op].text.c_str(), BITMAP_OPERATOR, BITMAP_OPERATOR, h, tree_item_handle(tree_nodes[item_index]));
			item_handle = tree_item_handle(tree_nodes[node]);
			move_branch(item_index, node);

			item_index = node;
			for (i=1; i<Operators[op].min; i++)
				add_default_operator(op, i);

			Expand(item_handle, TVE_EXPAND);
			*modified = 1;
			return 1;
		}
	}

	switch (id) {
		case ID_EDIT_COPY:
			NodeCopy();
			return 1;

		case ID_EDIT_PASTE:
			NodeReplacePaste();
			return 1;

		case ID_EDIT_PASTE_SPECIAL:  // add paste, instead of replace.
			NodeAddPaste();
			return 1;

/*		case ID_SPLIT_LINE:
			if ((tree_nodes[item_index].flags & OPERAND) && (tree_nodes[item_index].flags & EDITABLE))  // expandable?
				expand_operator(item_index);
			else
				merge_operator(item_index);

			return 1;*/

		case ID_EXPAND_ALL:
			expand_branch(item_handle);
			return 1;

		case ID_EDIT_TEXT:
			if (edit_label(item_handle)) {
				*modified = 1;
				EditLabel(item_handle);
			}
			return 1;

		case ID_EDIT_COMMENT:
			edit_comment(item_handle);
			return 1;

		case ID_EDIT_BG_COLOR:
			edit_bg_color(item_handle);
			return 1;

		case ID_REPLACE_NUMBER:
			expand_operator(item_index);
			if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
				replace_data("number", (SEXPT_NUMBER | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				replace_data("number", (SEXPT_NUMBER | SEXPT_VALID));
			}
			EditLabel(tree_item_handle(tree_nodes[item_index]));
			return 1;

		case ID_REPLACE_STRING:
			expand_operator(item_index);
			if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
				replace_data("string", (SEXPT_STRING | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				replace_data("string", (SEXPT_STRING | SEXPT_VALID));
			}
			EditLabel(tree_item_handle(tree_nodes[item_index]));
			return 1;

		case ID_ADD_STRING:	{
			int theNode;

			if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
				theNode = add_data("string", (SEXPT_STRING | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				theNode = add_data("string", (SEXPT_STRING | SEXPT_VALID));
			}
			EditLabel(tree_item_handle(tree_nodes[theNode]));
			return 1;
		}

		case ID_ADD_NUMBER:	{
			int theNode;

			if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
				theNode = add_data("number", (SEXPT_NUMBER | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				theNode = add_data("number", (SEXPT_NUMBER | SEXPT_VALID));
			}
			EditLabel(tree_item_handle(tree_nodes[theNode]));
			return 1;
		}

		case ID_EDIT_CUT:
			NodeCut();
			return 1;

		case ID_DELETE:
			NodeDelete();
			return 1;

		case IDC_SEXP_POPUP_LIST:
		{
			bool command_handled = false;

			switch (data)
			{
				case CBN_SELCHANGE:
				{
					int index = m_operator_box.GetCurSel();
					if (index >= 0)
					{
						if (m_operator_box.IsItemEnabled(index))
						{
							op = m_operator_box.GetOpIndex(index);

							// close the popup
							end_operator_edit(true);

							// do the operator replacement
							add_or_replace_operator(op, 1);
							expand_branch(item_handle);
						}
						// if the selected item wasn't enabled, do nothing
					}
					else
						end_operator_edit(false);

					command_handled = true;
					break;
				}

				case CBN_KILLFOCUS:
				{
					if (m_operator_popup_active && m_operator_box.PressedEnter())
					{
						op = m_operator_box.GetOpIndex(-1);

						// close the popup
						end_operator_edit(true);

						// do the operator replacement
						add_or_replace_operator(op, 1);
						expand_branch(item_handle);
					}
					else
						end_operator_edit(false);

					command_handled = true;
					break;
				}

				default:
					break;
			}

			if (command_handled)
				return TRUE;
		}
	}
	
	return CTreeCtrl::OnCommand(wParam, lParam);
}

void sexp_tree::NodeCut()
{
	if (item_index < 0)
		return;

	NodeCopy();
	NodeDelete();
}

void sexp_tree::NodeDelete()
{
	int parent, theNode;
	HTREEITEM h_parent;

	if ((m_mode & ST_ROOT_DELETABLE) && (item_index == -1)) {
		item_index = (int)GetItemData(item_handle);
		if (m_mode == MODE_GOALS) {
			Assert(Goal_editor_dlg);
			theNode = Goal_editor_dlg->handler(ROOT_DELETED, item_index);

		}else if (m_mode == MODE_CUTSCENES) {
			Assert(Cutscene_editor_dlg);
			theNode = Cutscene_editor_dlg->handler(ROOT_DELETED, item_index);

		} else if (m_mode == MODE_EVENTS) {
			Assert(Event_editor_dlg);
			theNode = Event_editor_dlg->handler(ROOT_DELETED, item_index);

		} else {
			Assert(m_mode == MODE_CAMPAIGN);
			theNode = Campaign_tree_formp->handler(ROOT_DELETED, item_index);
		}

		Assert(theNode >= 0);
		free_node2(theNode);
		DeleteItem(item_handle);
		*modified = 1;
		return;
	}

	Assert(item_index >= 0);
	h_parent = GetParentItem(item_handle);
	parent = tree_nodes[item_index].parent;

	// can't delete the root node
	if (parent < 0)
		return;

	Assert(parent != -1 && tree_nodes[parent].handle == h_parent);
	free_node(item_index);
	DeleteItem(item_handle);

	theNode = tree_nodes[parent].child;
/*			if (node != -1 && tree_nodes[node].next == -1 && tree_nodes[node].child == -1) {
		sprintf(buf, "%s %s", tree_nodes[parent].text, tree_nodes[node].text);
		SetItem(h_parent, TVIF_TEXT, buf, 0, 0, 0, 0, 0);
		tree_nodes[parent].flags = OPERAND | EDITABLE;
		tree_nodes[node].flags = COMBINED;
		DeleteItem(tree_item_handle(tree_nodes[node]));
	}*/

	*modified = 1;
}

void sexp_tree::NodeCopy()
{
	if (item_index < 0)
		return;

	// If a clipboard already exist, unmark it as persistent and free old clipboard
	if (Sexp_clipboard != -1) {
		sexp_unmark_persistent(Sexp_clipboard);
		free_sexp2(Sexp_clipboard);
	}

	// Allocate new clipboard and mark persistent
	Sexp_clipboard = save_branch(item_index, 1);
	sexp_mark_persistent(Sexp_clipboard);
}

void sexp_tree::NodeReplacePaste()
{
	if (item_index < 0 || Sexp_clipboard < 0)
		return;

	int i;

	// the following assumptions are made..
	Assert(Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED);
	Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
		"Attempt to use container name %s from SEXP clipboard. Please report!",
		Sexp_nodes[Sexp_clipboard].text);

	if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
		expand_operator(item_index);
		replace_operator(CTEXT(Sexp_clipboard));
		if (Sexp_nodes[Sexp_clipboard].rest != -1) {
			load_branch(Sexp_nodes[Sexp_clipboard].rest, item_index);
			i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_item_handle(tree_nodes[item_index]));
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
		int new_type = tree_nodes[item_index].type & ~(SEXPT_VARIABLE | SEXPT_CONTAINER_NAME) | SEXPT_CONTAINER_DATA;
		replace_container_data(container, new_type, false, true, !has_modifiers);
		if (has_modifiers) {
			load_branch(Sexp_nodes[Sexp_clipboard].first, item_index);
			i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_item_handle(tree_nodes[item_index]));
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
		}
		else {
			expand_operator(item_index);
			replace_data(CTEXT(Sexp_clipboard), (SEXPT_NUMBER | SEXPT_VALID));
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
		Assert(Sexp_nodes[Sexp_clipboard].rest == -1);
		if (Sexp_nodes[Sexp_clipboard].type & SEXP_FLAG_VARIABLE) {
			int var_idx = get_index_sexp_variable_name(Sexp_nodes[Sexp_clipboard].text);
			Assert(var_idx > -1);
			replace_variable_data(var_idx, (SEXPT_VARIABLE | SEXPT_STRING | SEXPT_VALID));
		}
		else {
			expand_operator(item_index);
			replace_data(CTEXT(Sexp_clipboard), (SEXPT_STRING | SEXPT_VALID));
		}

	} else
		Assert(0);  // unknown and/or invalid sexp type

	expand_branch(item_handle);
}

void sexp_tree::NodeAddPaste()
{
	if (item_index < 0 || Sexp_clipboard < 0)
		return;

	int i;

	// the following assumptions are made..
	Assert(Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED);
	Assert(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
		"Attempt to use container name %s from SEXP clipboard. Please report!",
		Sexp_nodes[Sexp_clipboard].text);

	if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
		expand_operator(item_index);
		add_operator(CTEXT(Sexp_clipboard));
		if (Sexp_nodes[Sexp_clipboard].rest != -1) {
			load_branch(Sexp_nodes[Sexp_clipboard].rest, item_index);
			i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_item_handle(tree_nodes[item_index]));
				i = tree_nodes[i].next;
			}
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_CONTAINER_DATA) {
		expand_operator(item_index);
		add_container_data(Sexp_nodes[Sexp_clipboard].text);
		const int modifier_node = Sexp_nodes[Sexp_clipboard].first;
		if (modifier_node != -1) {
			load_branch(modifier_node, item_index);
			i = tree_nodes[item_index].child;
			while (i != -1) {
				add_sub_tree(i, tree_item_handle(tree_nodes[item_index]));
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

	expand_branch(item_handle);
}

// adds to or replaces (based on passed in flag) the current operator
void sexp_tree::add_or_replace_operator(int op, int replace_flag)
{
	_actions.add_or_replace_operator(op, replace_flag);
}

// sexp_list_item methods are now in the shared sexp_tree_model.cpp

int sexp_tree::add_default_operator(int op_index, int argnum)
{
	return _actions.add_default_operator(op_index, argnum);
}

int sexp_tree::get_default_value(sexp_list_item *item, char *text_buf, int op, int i)
{
	return _model.get_default_value(item, text_buf, op, i);
}

int sexp_tree::query_default_argument_available(int op)
{
	return _model.query_default_argument_available(op);
}

int sexp_tree::query_default_argument_available(int op, int i)
{
	return _model.query_default_argument_available(op, i);
}

// expand a combined line (one with an operator and its one argument on the same line) into
// 2 lines.
void sexp_tree::expand_operator(int node)
{
	_actions.expand_operator(node);
}

// expand a CTreeCtrl branch and all of its children
void sexp_tree::expand_branch(HTREEITEM h)
{
	Expand(h, TVE_EXPAND);
	h = GetChildItem(h);
	while (h) {
		expand_branch(h);
		h = GetNextSiblingItem(h);
	}
}

void sexp_tree::merge_operator(int node)
{
/*	char buf[256];
	int child;

	if (tree_nodes[node].flags == EDITABLE)  // data
		node = tree_nodes[node].parent;

	if (node != -1) {
		child = tree_nodes[node].child;
		if (child != -1 && tree_nodes[child].next == -1 && tree_nodes[child].child == -1) {
			sprintf(buf, "%s %s", tree_nodes[node].text, tree_nodes[child].text);
			SetItemText(tree_item_handle(tree_nodes[node]), buf);
			tree_nodes[node].flags = OPERAND | EDITABLE;
			tree_nodes[child].flags = COMBINED;
			DeleteItem(tree_item_handle(tree_nodes[child]));
			tree_nodes[child].handle = NULL;
			return;
		}
	}*/
}

// add a data node under operator pointed to by item_index
int sexp_tree::add_data(const char *data, int type)
{
	return _actions.add_data(data, type);
}

// add a (variable) data node under operator pointed to by item_index
int sexp_tree::add_variable_data(const char *data, int type)
{
	return _actions.add_variable_data(data, type);
}

// add a container name node under operator pointed to by item_index
int sexp_tree::add_container_name(const char *container_name)
{
	return _actions.add_container_name(container_name);
}

// add a (container) data node under operator pointed to by item_index
void sexp_tree::add_container_data(const char *container_name)
{
	_actions.add_container_data(container_name);
}

// add an operator under operator pointed to by item_index.  Updates item_index to point
// to this new operator.
void sexp_tree::add_operator(const char *op, HTREEITEM h)
{
	_actions.add_operator(op, (h == TVI_ROOT) ? nullptr : static_cast<void*>(h));
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
	tree_nodes[node1].handle = insert(str, tree_item_handle(tree_nodes[item_index]));
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
			case OPF_TRANSLATING_SUBSYSTEM:
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
int sexp_tree::node_error(int node, const char *msg, int *bypass)
{
	char text[512];

	if (bypass)
		*bypass = 1;

	item_index = node;
	item_handle = tree_item_handle(tree_nodes[node]);
	if (tree_nodes[node].flags & COMBINED)
		item_handle = tree_item_handle(tree_nodes[tree_nodes[node].parent]);

	ensure_visible(node);
	SelectItem(item_handle);
	sprintf(text, "%s\n\nContinue checking for more errors?", msg);
	if (MessageBox(text, "Sexp error", MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
		return -1;
	else
		return 0;
}

void sexp_tree::hilite_item(int node)
{
	ensure_visible(node);
	SelectItem(tree_item_handle(tree_nodes[node]));
}

// because the MFC function EnsureVisible() doesn't do what it says it does, I wrote this.
void sexp_tree::ensure_visible(int node)
{
	Assert(node != -1);
	if (tree_nodes[node].parent != -1)
		ensure_visible(tree_nodes[node].parent);  // expand all parents first

	if (tree_nodes[node].child != -1)  // expandable?
		Expand(tree_item_handle(tree_nodes[node]), TVE_EXPAND);  // expand this item
}

void sexp_tree::link_modified(int *ptr)
{
	modified = ptr;
}

void get_variable_default_text_from_variable_text(char *text, char *default_text)
{
	char *start;

	// find '('
	start = strstr(text, "(");
	Assert(start);
	start++;

	// get length and copy all but last char ")"
	auto len = strlen(start);
	strncpy(default_text, start, len-1);

	// add null termination
	default_text[len-1] = '\0';
}

void get_variable_name_from_sexp_tree_node_text(const char *text, char *var_name)
{
	auto length = strcspn(text, "(");

	strncpy(var_name, text, length);
	var_name[length] = '\0';
}

int sexp_tree::get_modify_variable_type(int parent)
{
	return _model.get_modify_variable_type(parent);
}


void sexp_tree::verify_and_fix_arguments(int node)
{
	_actions.verify_and_fix_arguments(node);
}

void sexp_tree::replace_data(const char *data, int type)
{
	_actions.replace_data(data, type);
}


// Replaces data with sexp_variable type data
void sexp_tree::replace_variable_data(int var_idx, int type)
{
	_actions.replace_variable_data(var_idx, type);
}

void sexp_tree::replace_container_name(const sexp_container &container)
{
	_actions.replace_container_name(container);
}

void sexp_tree::replace_container_data(const sexp_container &container,
	int type,
	bool test_child_nodes,
	bool delete_child_nodes,
	bool set_default_modifier)
{
	_actions.replace_container_data(container, type, test_child_nodes, delete_child_nodes, set_default_modifier);
}


void sexp_tree::add_default_modifier(const sexp_container &container)
{
	_actions.add_default_modifier(container);
}

void sexp_tree::replace_operator(const char *op)
{
	_actions.replace_operator(op);
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
	h = tree_item_handle(tree_nodes[item_index]);
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
void sexp_tree::move_branch(int source, int parent)
{
	int node;

	// if no source, skip everything
	if (source != -1) {
		node = tree_nodes[source].parent;
		if (node != -1) {
			if (tree_nodes[node].child == source)
				tree_nodes[node].child = tree_nodes[source].next;
			else {
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
			if (tree_nodes[parent].child == -1)
				tree_nodes[parent].child = source;
			else {
				node = tree_nodes[parent].child;
				while (tree_nodes[node].next != -1)
					node = tree_nodes[node].next;

				tree_nodes[node].next = source;
			}

			move_branch(tree_item_handle(tree_nodes[source]), tree_item_handle(tree_nodes[parent]));

		} else
			move_branch(tree_item_handle(tree_nodes[source]));
	}
}

HTREEITEM sexp_tree::move_branch(HTREEITEM source, HTREEITEM parent, HTREEITEM after)
{
	uint i;
	int image1, image2;
	HTREEITEM h = 0, child, next;

	if (source) {
		for (i=0; i<tree_nodes.size(); i++)
			if (tree_nodes[i].handle == source)
				break;

		if (i < tree_nodes.size()) {
			GetItemImage(source, image1, image2);
			h = insert(GetItemText(source), image1, image2, parent, after);
			tree_nodes[i].handle = h;

		} else {
			GetItemImage(source, image1, image2);
  			h = insert(GetItemText(source), image1, image2, parent, after);
		}

		SetItemData(h, GetItemData(source));
		child = GetChildItem(source);
		while (child) {
			next = GetNextSiblingItem(child);
			move_branch(child, h);
			child = next;
		}

		if (GetItemState(source, TVIS_EXPANDED) & TVIS_EXPANDED)
			Expand(h, TVE_EXPAND);

		DeleteItem(source);
	}

	return h;
}

void sexp_tree::copy_branch(HTREEITEM source, HTREEITEM parent, HTREEITEM after)
{
	uint i;
	int image1, image2;
	HTREEITEM h, child;

	if (source) {
		for (i=0; i<tree_nodes.size(); i++)
			if (tree_nodes[i].handle == source)
				break;

		if (i < tree_nodes.size()) {
			GetItemImage(source, image1, image2);
			h = insert(GetItemText(source), image1, image2, parent, after);
			tree_nodes[i].handle = h;

		} else {
			GetItemImage(source, image1, image2);
  			h = insert(GetItemText(source), image1, image2, parent, after);
		}

		SetItemData(h, GetItemData(source));
		child = GetChildItem(source);
		while (child) {
			copy_branch(child, h);
			child = GetNextSiblingItem(child);
		}

		if (GetItemState(source, TVIS_EXPANDED) & TVIS_EXPANDED)
			Expand(h, TVE_EXPAND);
	}
}

void sexp_tree::move_root(HTREEITEM source, HTREEITEM dest, bool insert_before)
{
	HTREEITEM h, after = dest;

	Assert(!GetParentItem(source));
	Assert(!GetParentItem(dest));

	if (insert_before)
	{
		// since we can only insert after something, find the item previous to the destination; or indicate the first item if there is no previous item
		after = GetNextItem(dest, TVGN_PREVIOUS);
		if (after == nullptr)
			after = TVI_FIRST;
	}

	h = move_branch(source, TVI_ROOT, after);
	SelectItem(h);
	SelectItem(h);
	*modified = 1;
}

void sexp_tree::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UINT flags = 0;

//	ScreenToClient(&m_pt);
	ASSERT(!m_dragging);
	m_h_drag = HitTest(m_pt, &flags);
	m_h_drop = NULL;

	if (!m_mode || GetParentItem(m_h_drag))
		return;

	ASSERT(m_p_image_list == NULL);
	m_p_image_list = CreateDragImage(m_h_drag);  // get the image list for dragging
	if (!m_p_image_list)
		return;

	m_p_image_list->DragShowNolock(TRUE);
	m_p_image_list->SetDragCursorImage(0, CPoint(0, 0));
	m_p_image_list->BeginDrag(0, CPoint(0,0));
	m_p_image_list->DragMove(m_pt);
	m_p_image_list->DragEnter(this, m_pt);
	SetCapture();
	m_dragging = TRUE;
}

void sexp_tree::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_pt = point;
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void sexp_tree::OnMouseMove(UINT nFlags, CPoint point) 
{
	HTREEITEM hitem = NULL;
	UINT flags = 0;

	if (m_dragging) {
		ASSERT(m_p_image_list != NULL);
		m_p_image_list->DragMove(point);
		if ((hitem = HitTest(point, &flags)) != NULL)
			if (!GetParentItem(hitem)) {
				m_p_image_list->DragLeave(this);
				SelectDropTarget(hitem);
				m_h_drop = hitem;
				m_p_image_list->DragEnter(this, point);
			}
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void sexp_tree::OnLButtonUp(UINT nFlags, CPoint point) 
{
	int node1, node2;

	if (m_dragging) {
		ASSERT(m_p_image_list != NULL);
		m_p_image_list->DragLeave(this);
		m_p_image_list->EndDrag();
		delete m_p_image_list;
		m_p_image_list = NULL;

		if (m_h_drop && m_h_drag != m_h_drop) {
			Assert(m_h_drag);
			node1 = (int)GetItemData(m_h_drag);
			node2 = (int)GetItemData(m_h_drop);

			// If we're moving up, insert before the dropped item.  If we're moving down,
			// insert after the dropped item.  The idea is to always end up where we dropped.
			bool insert_before = false;
			for (auto h = m_h_drag; h != nullptr; h = GetNextItem(h, TVGN_PREVIOUS))
			{
				if (h == m_h_drop)
				{
					insert_before = true;
					break;
				}
			}

			move_root(m_h_drag, m_h_drop, insert_before);

			if (m_mode == MODE_GOALS) {
				Assert(Goal_editor_dlg);
				Goal_editor_dlg->move_handler(node1, node2, insert_before);

			} else if (m_mode == MODE_EVENTS) {
				Assert(Event_editor_dlg);
				Event_editor_dlg->move_handler(node1, node2, insert_before);

			} else if (m_mode == MODE_CAMPAIGN) {
				Assert(Campaign_tree_formp);
				Campaign_tree_formp->move_handler(node1, node2, insert_before);

			} else
				UNREACHABLE("Unhandled dialog mode!");

		} else
			MessageBeep(0);

		ReleaseCapture();
		m_dragging = FALSE;
		SelectDropTarget(NULL);
	}

	CTreeCtrl::OnLButtonUp(nFlags, point);
}

const static UINT Numbered_data_bitmaps[] = {
	IDB_DATA_00,
	IDB_DATA_05,
	IDB_DATA_10,
	IDB_DATA_15,
	IDB_DATA_20,
	IDB_DATA_25,
	IDB_DATA_30,
	IDB_DATA_35,
	IDB_DATA_40,
	IDB_DATA_45,
	IDB_DATA_50,
	IDB_DATA_55,
	IDB_DATA_60,
	IDB_DATA_65,
	IDB_DATA_70,
	IDB_DATA_75,
	IDB_DATA_80,
	IDB_DATA_85,
	IDB_DATA_90,
	IDB_DATA_95
};

void sexp_tree::setup(CEdit *ptr)
{
	CImageList *pimagelist;
	CBitmap bitmap;

	help_box = ptr;
	if (help_box) {
		int stops[2] = { 10, 30 };

		help_box -> SetTabStops(2, (LPINT) stops);
	}

	pimagelist = GetImageList(TVSIL_NORMAL);
	if (!pimagelist) {
		pimagelist = new CImageList();
		pimagelist->Create(16, 16, TRUE/*bMask*/, 2, 22);

		//*****Add generic images
		bitmap.LoadBitmap(IDB_OPERATOR);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_DATA);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_VARIABLE);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_ROOT);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_ROOT_DIRECTIVE);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_CHAINED);
		pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_CHAINED_DIRECTIVE);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_GREEN_DOT);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_BLACK_DOT);
		pimagelist->Add(&bitmap, (COLORREF) 0xFFFFFF);
		bitmap.DeleteObject();

		//*****Add numbered data entries
		int num = sizeof(Numbered_data_bitmaps)/sizeof(UINT);
		int i = 0;
		for(i = 0; i < num; i++)
		{
			bitmap.LoadBitmap(Numbered_data_bitmaps[i]);
			pimagelist->Add(&bitmap, (COLORREF) 0xFF00FF);
			bitmap.DeleteObject();
		}

		bitmap.LoadBitmap(IDB_COMMENT);
		pimagelist->Add(&bitmap, (COLORREF)0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_CONTAINER_NAME);
		pimagelist->Add(&bitmap, (COLORREF)0xFF00FF);
		bitmap.DeleteObject();

		bitmap.LoadBitmap(IDB_CONTAINER_DATA);
		pimagelist->Add(&bitmap, (COLORREF)0xFF00FF);
		bitmap.DeleteObject();

		SetImageList(pimagelist, TVSIL_NORMAL);
	}
}
//#define BITMAP_OPERATOR 0
//#define BITMAP_DATA 1
//#define BITMAP_VARIABLE 2
//#define BITMAP_ROOT 3
//#define BITMAP_ROOT_DIRECTIVE 4
//#define BITMAP_CHAIN 5
//#define BITMAP_CHAIN_DIRECTIVE 6
//#define BITMAP_GREEN_DOT 7
//#define BITMAP_BLACK_DOT 8


HTREEITEM sexp_tree::insert(LPCTSTR lpszItem, int image, int sel_image, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	return InsertItem(lpszItem, image, sel_image, hParent, hInsertAfter);

}

void sexp_tree::OnDestroy() 
{
	CImageList *pimagelist;

	pimagelist = GetImageList(TVSIL_NORMAL);
	if (pimagelist) {
		pimagelist->DeleteImageList();
		delete pimagelist;
	}

	CTreeCtrl::OnDestroy();
}

HTREEITEM sexp_tree::handle(int node)
{
	return tree_item_handle(tree_nodes[node]);
}

const char *sexp_tree::help(int code)
{
	return SexpTreeModel::help(code);
}

// get type of item clicked on
int sexp_tree::get_type(HTREEITEM h)
{
	uint i;

	// get index into sexp_tree 
	for (i=0; i<tree_nodes.size(); i++)
		if (tree_nodes[i].handle == h)
			break;

	if ( (i >= tree_nodes.size()) ) {
		// Int3();	// This would be the root of the tree  -- ie, event name
		return -1;
	}

	return tree_nodes[i].type;
}


void sexp_tree::update_help(HTREEITEM h)
{
	int i, j, z, c, code, index, sibling_place;
	CString text;

	for (i=0; i<(int)Operators.size(); i++) {
		for (j=0; j<(int)op_menu.size(); j++) {
			if (get_category(Operators[i].value) == op_menu[j].id) {
				if (!help(Operators[i].value)) {
					mprintf(("Allender!  If you add new sexp operators, add help for them too! :) Sexp %s has no help.\n", Operators[i].text.c_str()));
				}
			}
		}
	}

	help_box = (CEdit *) GetParent()->GetDlgItem(IDC_HELP_BOX);
	if (!help_box || !::IsWindow(help_box->m_hWnd))
		return;

	mini_help_box = (CEdit *) GetParent()->GetDlgItem(IDC_MINI_HELP_BOX);
	if (mini_help_box && !::IsWindow(mini_help_box->m_hWnd))
		return;

	for (i=0; i<(int)tree_nodes.size(); i++)
		if (tree_nodes[i].handle == h)
			break;

	int thisIndex = event_annotation_lookup(h);
	SCP_string nodeComment;

	if (thisIndex >= 0) {
		if (!Event_annotations[thisIndex].comment.empty()) {
			nodeComment = "Node Comments:\r\n   " + Event_annotations[thisIndex].comment;
		}
	} else {
		nodeComment = "";
	}

	if ((i >= (int)tree_nodes.size()) || !tree_nodes[i].type) {
		help_box->SetWindowText(nodeComment.c_str());
		if (mini_help_box)
			mini_help_box->SetWindowText("");
		return;
	}

	// Now that we're done with top level nodes we can add the empty lines because
	// everything else below is supposed to have help text
	if (!nodeComment.empty())
		nodeComment.insert(0, "\r\n\r\n");

	if (SEXPT_TYPE(tree_nodes[i].type) == SEXPT_OPERATOR)
	{
		if (mini_help_box)
			mini_help_box->SetWindowText("");
	}
	else
	{
		z = tree_nodes[i].parent;
		if (z < 0) {
			Warning(LOCATION, "Sexp data \"%s\" has no parent!", tree_nodes[i].text);
			return;
		}

		code = get_operator_const(tree_nodes[z].text);
		index = get_operator_index(tree_nodes[z].text);
		sibling_place = get_sibling_place(i) + 1;	//We want it to start at 1

		//*****Minihelp box
		if((SEXPT_TYPE(tree_nodes[i].type) == SEXPT_NUMBER) || (SEXPT_TYPE(tree_nodes[i].type) == SEXPT_STRING) && sibling_place > 0)
		{
			char buffer[10240] = {""};

			//Get the help for the current operator
			const char *helpstr = help(code);
			bool display_number = true;

			//If a help string exists, try to display it
			if(helpstr != NULL)
			{
				char searchstr[32];
				const char *loc=NULL, *loc2=NULL;

				if(loc == NULL)
				{
					sprintf(searchstr, "\n%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}

				if(loc == NULL)
				{
					sprintf(searchstr, "\t%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if(loc == NULL)
				{
					sprintf(searchstr, " %d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if(loc == NULL)
				{
					sprintf(searchstr, "%d:", sibling_place);
					loc = strstr(helpstr, searchstr);
				}
				if(loc == NULL)
				{
					loc = strstr(helpstr, "Rest:");
				}
				if(loc == NULL)
				{
					loc = strstr(helpstr, "All:");
				}

				if(loc != NULL)
				{
					//Skip whitespace
					while(*loc=='\r' || *loc == '\n' || *loc == ' ' || *loc == '\t') loc++;

					//Find EOL
					loc2 = strpbrk(loc, "\r\n");
					if(loc2 != NULL)
					{
						size_t size = loc2-loc;
						strncpy(buffer, loc, size);
						if(size < sizeof(buffer))
						{
							buffer[size] = '\0';
						}
						display_number = false;
					}
					else
					{
						strcpy_s(buffer, loc);
						display_number = false;
					}
				}
			}

			//Display argument number
			if(display_number)
			{
				sprintf(buffer, "%d:", sibling_place);
			}

			if (mini_help_box)
				mini_help_box->SetWindowText(buffer);
		}

		if (index >= 0) {
			c = 0;
			j = tree_nodes[z].child;
			while ((j >= 0) && (j != i)) {
				j = tree_nodes[j].next;
				c++;
			}

			Assert(j >= 0);
			// If the node is a message then display it
			if (query_operator_argument_type(index, c) == OPF_MESSAGE) {
				for (j=0; j<Num_messages; j++)
					if (!stricmp(Messages[j].name, tree_nodes[i].text)) {
						text.Format("Message Text:\r\n%s%s", Messages[j].message, nodeComment.c_str());
						help_box->SetWindowText((LPCSTR)text);
						return;
					}
			}
			
			// If the node is a ship flag, then display the flag's description
			if (query_operator_argument_type(index, c) == OPF_SHIP_FLAG) {
				Object::Object_Flags object_flag = Object::Object_Flags::NUM_VALUES;
				Ship::Ship_Flags ship_flag = Ship::Ship_Flags::NUM_VALUES;
				Mission::Parse_Object_Flags parse_obj_flag = Mission::Parse_Object_Flags::NUM_VALUES;
				AI::AI_Flags ai_flag = AI::AI_Flags::NUM_VALUES;
				SCP_string desc;

				sexp_check_flag_arrays(tree_nodes[i].text, object_flag, ship_flag, parse_obj_flag, ai_flag);

				// Ship flags are pulled from multiple categories, so we have to search them all. Ew.
				if (object_flag != Object::Object_Flags::NUM_VALUES){
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

				// Only check through parse object flags if we haven't found anything yet
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

				//If we still didn't find anything, say so!
				if (desc.empty())
					desc = "Unknown flag. Let a coder know!";

				text.Format("%s", desc.c_str());
				help_box->SetWindowText((LPCSTR)text);
				return;
			}

			// If the node is a wing flag, then display the flag's description
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

				// If we still didn't find anything, say so!
				if (desc.empty())
					desc = "Unknown flag. Let a coder know!";

				text.Format("%s", desc.c_str());
				help_box->SetWindowText((LPCSTR)text);
				return;
			}
		}

		i = z;
	}

	code = get_operator_const(tree_nodes[i].text);
	auto str = help(code);
	if (!str) {
		text.Format("No help available%s", nodeComment.c_str());
	} else {
		text.Format("%s%s", str, nodeComment.c_str());
	}

	help_box->SetWindowText((LPCSTR)text);
}

// find list of sexp_tree nodes with text
// stuff node indices into find[]
int sexp_tree::find_text(const char *text, int *find)
{
	return _model.find_text(text, find, MAX_SEARCH_MESSAGE_DEPTH);
}

// This solution was found at https://community.notepad-plus-plus.org/topic/21158/way-to-disallow-copying-text/7
//   Another interesting thing with the script's execution is that if you don't allow Notepad++ to process the Ctrl+c
//   keycombo as a WM_KEYDOWN event, you'll get an "ETX" (hex code = 0x03) character in your document at the caret
//   position. This occurs because, with Notepad++ ignoring the Ctrl+c as a command, it thinks that you want to embed
//   a control character with value "3" into the doc (it gets a WM_CHAR message to that effect, which it would normally
//   filter out on its own). So... in the case of preventing copying data, we have to remove any Ctrl+c or Ctrl+x
//   characters that might get inserted; we do this by processing the WM_CHAR message and filtering those as well.
void sexp_tree::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void sexp_tree::OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult) 
{
	int key;
	TV_KEYDOWN *pTVKeyDown = (TV_KEYDOWN *) pNMHDR;

	key = pTVKeyDown->wVKey;

	// Handle clipboard operations for the sexp_tree and its subclasses.  The *proper* way to do this
	// would be to catch the WM_CUT, WM_COPY, and WM_PASTE messages, but I wasn't able to get it to work
	// despite considerable research and effort.  So this achieves the same result by just catching
	// the shortcut keys instead of the messages they produce.  The shortcut keys are still sent to
	// the window, so the OnChar handler is necessary to catch them and prevent an error beep.
	// 
	// Only capture keys on the nodes when we're not currently editing their text
	//
	if (GetEditControl() == nullptr)
	{
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			// Currently the item_index and item_handle are only updated
			// when right-clicking, so we need to do another update
			// before processing the key press.  Ideally they should be
			// updated when the selection changes, but there are some
			// hidden side-effects to making it selection-dependent
			// that are difficult to track down.			
			if (key == 'X')
			{
				update_item(GetSelectedItem());
				NodeCut();
			}
			else if (key == 'C')
			{
				update_item(GetSelectedItem());
				NodeCopy();
			}
			else if (key == 'V')
			{
				if (GetKeyState(VK_SHIFT) & 0x8000) 
				{
					update_item(GetSelectedItem());
					NodeReplacePaste();
				}
				else 
				{
					update_item(GetSelectedItem());
					auto orig_handle = item_handle;
					NodeAddPaste();
					// when using the keyboard shortcut, stay on the original node after pasting
					SelectItem(orig_handle);
					update_item(orig_handle);
				}
			}
		}
	}

	if (key == VK_SPACE)
		EditLabel(GetSelectedItem());

	*pResult = 0;
}

// Determine if a given opf code has a restricted argument range (i.e. has a specific, limited
// set of argument values, or has virtually unlimited possibilities.  For example, boolean values
// only have true or false, so it is restricted, but a number could be anything, so it's not.
//
int sexp_tree::query_restricted_opf_range(int opf)
{
	return _model.query_restricted_opf_range(opf);
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
sexp_list_item *sexp_tree::get_listing_opf(int opf, int parent_node, int arg_index)
{
	return _model.get_listing_opf(opf, parent_node, arg_index);
}

// Goober5000
int sexp_tree::find_argument_number(int parent_node, int child_node) const
{
	return _model.find_argument_number(parent_node, child_node);
}

// Goober5000
// backtrack through parents until we find the operator matching
// parent_op, then find the argument we went through
int sexp_tree::find_ancestral_argument_number(int parent_op, int child_node) const
{
	return _model.find_ancestral_argument_number(parent_op, child_node);
}

/**
* Gets the proper data image for the tree item's place
* in its parent hierarchy.
*/
int sexp_tree::get_data_image(int node)
{
	return static_cast<int>(_model.get_data_image(node));
}

sexp_list_item *sexp_tree::get_container_modifiers(int con_data_node) const
{
	return _model.get_container_modifiers(con_data_node);
}

int sexp_tree::get_sibling_place(int node)
{
	return _model.get_sibling_place(node);
}


sexp_list_item *sexp_tree::get_listing_opf_null()
{
	return _model.get_listing_opf_null();
}

sexp_list_item *sexp_tree::get_listing_opf_flexible_argument()
{
	return _model.get_listing_opf_flexible_argument();
}

sexp_list_item *sexp_tree::get_listing_opf_bool(int parent_node)
{
	return _model.get_listing_opf_bool(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_positive()
{
	return _model.get_listing_opf_positive();
}

sexp_list_item *sexp_tree::get_listing_opf_number()
{
	return _model.get_listing_opf_number();
}

sexp_list_item *sexp_tree::get_listing_opf_ship(int parent_node)
{
	return _model.get_listing_opf_ship(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_prop()
{
	return _model.get_listing_opf_prop();
}

sexp_list_item *sexp_tree::get_listing_opf_wing()
{
	return _model.get_listing_opf_wing();
}

// specific types of subsystems we're looking for
#define OPS_CAP_CARGO		1	
#define OPS_STRENGTH			2
#define OPS_BEAM_TURRET		3
#define OPS_AWACS				4
#define OPS_ROTATE			5
#define OPS_TRANSLATE			6
#define OPS_ARMOR			7
sexp_list_item *sexp_tree::get_listing_opf_subsystem(int parent_node, int arg_index)
{
	return _model.get_listing_opf_subsystem(parent_node, arg_index);
}

sexp_list_item *sexp_tree::get_listing_opf_subsystem_type(int parent_node)
{
	return _model.get_listing_opf_subsystem_type(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_point()
{
	return _model.get_listing_opf_point();
}

sexp_list_item *sexp_tree::get_listing_opf_iff()
{
	return _model.get_listing_opf_iff();
}

sexp_list_item *sexp_tree::get_listing_opf_ai_class()
{
	return _model.get_listing_opf_ai_class();
}

sexp_list_item *sexp_tree::get_listing_opf_support_ship_class()
{
	return _model.get_listing_opf_support_ship_class();
}

sexp_list_item *sexp_tree::get_listing_opf_ssm_class()
{
	return _model.get_listing_opf_ssm_class();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_with_bay()
{
	return _model.get_listing_opf_ship_with_bay();
}

sexp_list_item *sexp_tree::get_listing_opf_soundtrack_name()
{
	return _model.get_listing_opf_soundtrack_name();
}

sexp_list_item *sexp_tree::get_listing_opf_arrival_location()
{
	return _model.get_listing_opf_arrival_location();
}

sexp_list_item *sexp_tree::get_listing_opf_departure_location()
{
	return _model.get_listing_opf_departure_location();
}

sexp_list_item *sexp_tree::get_listing_opf_arrival_anchor_all()
{
	return _model.get_listing_opf_arrival_anchor_all();
}

sexp_list_item *sexp_tree::get_listing_opf_ai_goal(int parent_node)
{
	return _model.get_listing_opf_ai_goal(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_docker_point(int parent_node, int arg_num)
{
	return _model.get_listing_opf_docker_point(parent_node, arg_num);
}

sexp_list_item *sexp_tree::get_listing_opf_dockee_point(int parent_node)
{
	return _model.get_listing_opf_dockee_point(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_message()
{
	return _model.get_listing_opf_message();
}

sexp_list_item *sexp_tree::get_listing_opf_persona()
{
	return _model.get_listing_opf_persona();
}

sexp_list_item *sexp_tree::get_listing_opf_font()
{
	return _model.get_listing_opf_font();
}

sexp_list_item *sexp_tree::get_listing_opf_who_from()
{
	return _model.get_listing_opf_who_from();
}

sexp_list_item *sexp_tree::get_listing_opf_priority()
{
	return _model.get_listing_opf_priority();
}

sexp_list_item *sexp_tree::get_listing_opf_sound_environment()
{
	return _model.get_listing_opf_sound_environment();
}

sexp_list_item *sexp_tree::get_listing_opf_sound_environment_option()
{
	return _model.get_listing_opf_sound_environment_option();
}

sexp_list_item *sexp_tree::get_listing_opf_adjust_audio_volume()
{
	return _model.get_listing_opf_adjust_audio_volume();
}

sexp_list_item *sexp_tree::get_listing_opf_builtin_hud_gauge() 
{
	return _model.get_listing_opf_builtin_hud_gauge();
}

sexp_list_item *sexp_tree::get_listing_opf_custom_hud_gauge()
{
	return _model.get_listing_opf_custom_hud_gauge();
}

sexp_list_item *sexp_tree::get_listing_opf_any_hud_gauge()
{
	return _model.get_listing_opf_any_hud_gauge();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_effect()
{
	return _model.get_listing_opf_ship_effect();
}

sexp_list_item *sexp_tree::get_listing_opf_explosion_option()
{
	return _model.get_listing_opf_explosion_option();
}

sexp_list_item *sexp_tree::get_listing_opf_waypoint_path()
{
	return _model.get_listing_opf_waypoint_path();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_point()
{
	return _model.get_listing_opf_ship_point();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing_wholeteam()
{
	return _model.get_listing_opf_ship_wing_wholeteam();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing_shiponteam_point()
{
	return _model.get_listing_opf_ship_wing_shiponteam_point();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing_point()
{
	return _model.get_listing_opf_ship_wing_point();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing_point_or_none()
{
	return _model.get_listing_opf_ship_wing_point_or_none();
}

sexp_list_item *sexp_tree::get_listing_opf_mission_name()
{
	return _model.get_listing_opf_mission_name();
}

sexp_list_item *sexp_tree::get_listing_opf_goal_name(int parent_node)
{
	return _model.get_listing_opf_goal_name(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_ship_wing()
{
	return _model.get_listing_opf_ship_wing();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_prop()
{
	return _model.get_listing_opf_ship_prop();
}

sexp_list_item *sexp_tree::get_listing_opf_order_recipient()
{
	return _model.get_listing_opf_order_recipient();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_type()
{
	return _model.get_listing_opf_ship_type();
}

sexp_list_item *sexp_tree::get_listing_opf_keypress()
{
	return _model.get_listing_opf_keypress();
}

sexp_list_item *sexp_tree::get_listing_opf_event_name(int parent_node)
{
	return _model.get_listing_opf_event_name(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_ai_order()
{
	return _model.get_listing_opf_ai_order();
}

sexp_list_item *sexp_tree::get_listing_opf_skill_level()
{
	return _model.get_listing_opf_skill_level();
}

sexp_list_item *sexp_tree::get_listing_opf_cargo()
{
	return _model.get_listing_opf_cargo();
}

sexp_list_item *sexp_tree::get_listing_opf_string()
{
	return _model.get_listing_opf_string();
}

sexp_list_item *sexp_tree::get_listing_opf_medal_name()
{
	return _model.get_listing_opf_medal_name();
}

sexp_list_item *sexp_tree::get_listing_opf_weapon_name()
{
	return _model.get_listing_opf_weapon_name();
}

sexp_list_item *sexp_tree::get_listing_opf_intel_name()
{
	return _model.get_listing_opf_intel_name();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_class_name()
{
	return _model.get_listing_opf_ship_class_name();
}

sexp_list_item* sexp_tree::get_listing_opf_prop_class_name()
{
	return _model.get_listing_opf_prop_class_name();
}

sexp_list_item *sexp_tree::get_listing_opf_huge_weapon()
{
	return _model.get_listing_opf_huge_weapon();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_not_player()
{
	return _model.get_listing_opf_ship_not_player();
}

sexp_list_item *sexp_tree::get_listing_opf_ship_or_none()
{
	return _model.get_listing_opf_ship_or_none();
}

sexp_list_item *sexp_tree::get_listing_opf_subsystem_or_none(int parent_node, int arg_index)
{
	return _model.get_listing_opf_subsystem_or_none(parent_node, arg_index);
}

sexp_list_item *sexp_tree::get_listing_opf_subsys_or_generic(int parent_node, int arg_index)
{
	return _model.get_listing_opf_subsys_or_generic(parent_node, arg_index);
}

sexp_list_item *sexp_tree::get_listing_opf_jump_nodes()
{
	return _model.get_listing_opf_jump_nodes();
}

// creates list of Sexp_variables
sexp_list_item *sexp_tree::get_listing_opf_variable_names()
{
	return _model.get_listing_opf_variable_names();
}

// get default skybox model name
sexp_list_item *sexp_tree::get_listing_opf_skybox_model()
{
	return _model.get_listing_opf_skybox_model();
}

sexp_list_item *sexp_tree::get_listing_opf_skybox_flags()
{
	return _model.get_listing_opf_skybox_flags();
}

sexp_list_item *sexp_tree::get_listing_opf_background_bitmap()
{
	return _model.get_listing_opf_background_bitmap();
}

sexp_list_item *sexp_tree::get_listing_opf_sun_bitmap()
{
	return _model.get_listing_opf_sun_bitmap();
}

sexp_list_item *sexp_tree::get_listing_opf_nebula_storm_type()
{
	return _model.get_listing_opf_nebula_storm_type();
}

sexp_list_item *sexp_tree::get_listing_opf_nebula_poof()
{
	return _model.get_listing_opf_nebula_poof();
}

sexp_list_item* sexp_tree::get_listing_opf_turret_target_order()
{
	return _model.get_listing_opf_turret_target_order();
}

sexp_list_item* sexp_tree::get_listing_opf_turret_types()
{
	return _model.get_listing_opf_turret_types();
}

sexp_list_item *sexp_tree::get_listing_opf_post_effect()
{
	return _model.get_listing_opf_post_effect();
}


sexp_list_item *sexp_tree::get_listing_opf_turret_target_priorities()
{
	return _model.get_listing_opf_turret_target_priorities();
}

sexp_list_item *sexp_tree::get_listing_opf_armor_type()
{
	return _model.get_listing_opf_armor_type();
}

sexp_list_item *sexp_tree::get_listing_opf_damage_type()
{
	return _model.get_listing_opf_damage_type();
}

sexp_list_item *sexp_tree::get_listing_opf_animation_type()
{
	return _model.get_listing_opf_animation_type();
}

sexp_list_item *sexp_tree::get_listing_opf_hud_elements()
{
	return _model.get_listing_opf_hud_elements();
}

sexp_list_item *sexp_tree::get_listing_opf_weapon_banks()
{
	return _model.get_listing_opf_weapon_banks();
}

sexp_list_item *sexp_tree::get_listing_opf_mission_moods()
{
	return _model.get_listing_opf_mission_moods();
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
	return _model.get_listing_opf_ship_flags();
}

sexp_list_item *sexp_tree::get_listing_opf_wing_flags()
{
	return _model.get_listing_opf_wing_flags();
}

sexp_list_item *sexp_tree::get_listing_opf_team_colors()
{
	return _model.get_listing_opf_team_colors();
}

sexp_list_item *sexp_tree::get_listing_opf_nebula_patterns()
{
	return _model.get_listing_opf_nebula_patterns();
}

sexp_list_item *sexp_tree::get_listing_opf_asteroid_types()
{
	return _model.get_listing_opf_asteroid_types();
}

sexp_list_item *sexp_tree::get_listing_opf_debris_types()
{
	return _model.get_listing_opf_debris_types();
}

sexp_list_item* sexp_tree::get_listing_opf_motion_debris()
{
	return _model.get_listing_opf_motion_debris();
}

sexp_list_item* sexp_tree::get_listing_opf_bolt_types()
{
	return _model.get_listing_opf_bolt_types();
}

sexp_list_item* sexp_tree::get_listing_opf_traitor_overrides()
{
	return _model.get_listing_opf_traitor_overrides();
}

sexp_list_item* sexp_tree::get_listing_opf_lua_enum(int parent_node, int arg_index)
{
	return _model.get_listing_opf_lua_enum(parent_node, arg_index);
}

sexp_list_item* sexp_tree::get_listing_opf_lua_general_orders()
{
	return _model.get_listing_opf_lua_general_orders();
}

sexp_list_item* sexp_tree::get_listing_opf_message_types()
{
	return _model.get_listing_opf_message_types();
}

sexp_list_item* sexp_tree::get_listing_opf_mission_custom_strings()
{
	return _model.get_listing_opf_mission_custom_strings();
}

extern SCP_vector<game_snd>	Snds;

sexp_list_item *sexp_tree::get_listing_opf_game_snds()
{
	return _model.get_listing_opf_game_snds();
}

sexp_list_item *sexp_tree::get_listing_opf_fireball()
{
	return _model.get_listing_opf_fireball();
}

sexp_list_item *sexp_tree::get_listing_opf_species()	// NOLINT
{
	return _model.get_listing_opf_species();
}

sexp_list_item *sexp_tree::get_listing_opf_language()	// NOLINT
{
	return _model.get_listing_opf_language();
}

sexp_list_item *sexp_tree::get_listing_opf_functional_when_eval_type()	// NOLINT
{
	return _model.get_listing_opf_functional_when_eval_type();
}

sexp_list_item *sexp_tree::get_listing_opf_animation_name(int parent_node)
{
	return _model.get_listing_opf_animation_name(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_sexp_containers(ContainerType con_type)
{
	return _model.get_listing_opf_sexp_containers(con_type);
}

sexp_list_item *sexp_tree::get_listing_opf_wing_formation()	// NOLINT
{
	return _model.get_listing_opf_wing_formation();
}

sexp_list_item *sexp_tree::get_list_container_modifiers() const
{
	return _model.get_list_container_modifiers();
}

// FIXME TODO: if you use this function with remove-from-map SEXP, don't use SEXPT_MODIFIER
sexp_list_item *sexp_tree::get_map_container_modifiers(int con_data_node) const
{
	return _model.get_map_container_modifiers(con_data_node);
}

// get potential options for container multidimensional modifiers
// the value could be either string or number, checked in-mission
sexp_list_item *sexp_tree::get_container_multidim_modifiers(int con_data_node) const
{
	return _model.get_container_multidim_modifiers(con_data_node);
}

sexp_list_item* sexp_tree::check_for_dynamic_sexp_enum(int opf)
{
	return _model.check_for_dynamic_sexp_enum(opf);
}

// given a node's parent, check if node is eligible for being used with the special argument
bool sexp_tree::is_node_eligible_for_special_argument(int parent_node) const
{
	return _model.is_node_eligible_for_special_argument(parent_node);
}

// Deletes sexp_variable from sexp_tree.
// resets tree to not include given variable, and resets text and type
void sexp_tree::delete_sexp_tree_variable(const char *var_name)
{
	char search_str[64];
	char replace_text[TOKEN_LENGTH];
	
	sprintf(search_str, "%s(", var_name);

	// store old item index
	int old_item_index = item_index;

	for (uint idx=0; idx<tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if ( strstr(tree_nodes[idx].text, search_str) != NULL ) {

				// check type is number or string
				Assert( (tree_nodes[idx].type & SEXPT_NUMBER) || (tree_nodes[idx].type & SEXPT_STRING) );

				// reset type as not variable
				int type = tree_nodes[idx].type &= ~SEXPT_VARIABLE;

				// reset text
				if (tree_nodes[idx].type & SEXPT_NUMBER) {
					strcpy_s(replace_text, "number");
				} else {
					strcpy_s(replace_text, "string");
				}

				// set item_index and replace data
				item_index = idx;
				replace_data(replace_text, type);
			}
		}
	}

	// restore item_index
	item_index = old_item_index;
}


// Modify sexp_tree for a change in sexp_variable (name, type, or default value)
void sexp_tree::modify_sexp_tree_variable(const char *old_name, int sexp_var_index)
{
	char search_str[64];
	int type;

	Assert(Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_SET);
	Assert( (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) || (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) );

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

	for (uint idx=0; idx<tree_nodes.size(); idx++) {
		if (tree_nodes[idx].type & SEXPT_VARIABLE) {
			if ( strstr(tree_nodes[idx].text, search_str) != NULL ) {
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
int sexp_tree::get_item_index_to_var_index()
{
	return _model.get_item_index_to_var_index();
}

int sexp_tree::get_tree_name_to_sexp_variable_index(const char *tree_name)
{
	return SexpTreeModel::get_tree_name_to_sexp_variable_index(tree_name);
}

int sexp_tree::get_variable_count(const char *var_name)
{
	return _model.get_variable_count(var_name);
}

// Returns the number of times a variable with this name has been used by player loadout
int sexp_tree::get_loadout_variable_count(int var_index)
{
	return _model.get_loadout_variable_count(var_index);
}

int sexp_tree::get_container_usage_count(const SCP_string &container_name) const
{
	return _model.get_container_usage_count(container_name);
}

bool sexp_tree::rename_container_nodes(const SCP_string &old_name, const SCP_string &new_name)
{
	Assertion(!old_name.empty(),
		"Attempt to rename container nodes looking for empty name. Please report!");
	Assertion(!new_name.empty(),
		"Attempt to rename container nodes with empty name. Please report!");
	Assertion(new_name.length() <= sexp_container::NAME_MAX_LENGTH,
		"Attempt to rename container nodes with name %s that is too long (%d > %d). Please report!",
		new_name.c_str(),
		(int)new_name.length(),
		sexp_container::NAME_MAX_LENGTH);

	bool renamed_anything = false;

	for (int node_idx = 0; node_idx < (int)tree_nodes.size(); node_idx++) {
		if (is_matching_container_node(node_idx, old_name)) {
			strcpy_s(tree_nodes[node_idx].text, new_name.c_str());
			SetItemText(tree_item_handle(tree_nodes[node_idx]), new_name.c_str());
			renamed_anything = true;
		}
	}

	return renamed_anything;
}

bool sexp_tree::is_matching_container_node(int node, const SCP_string &container_name) const
{
	return _model.is_matching_container_node(node, container_name);
}

bool sexp_tree::is_container_name_argument(int node) const
{
	return _model.is_container_name_argument(node);
}

bool sexp_tree::is_container_name_opf_type(const int op_type)
{
	return SexpTreeModel::is_container_name_opf_type(op_type);
}
