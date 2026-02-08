/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "sexp_tree_ui.h"
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

// constructor
sexp_tree::sexp_tree()
	: m_operator_box(help), _actions(_model, *this)
{
	select_sexp_node = -1;
	root_item = -1;
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

void sexp_tree::ui_add_children_visual(int parent_node_index)
{
	int i = tree_nodes[parent_node_index].child;
	while (i != -1) {
		add_sub_tree(i, tree_item_handle(tree_nodes[parent_node_index]));
		i = tree_nodes[i].next;
	}
}

void sexp_tree::ui_expand_branch(void* handle)
{
	expand_branch((HTREEITEM)handle);
}

// clears out the tree, so all the nodes are unused.
void sexp_tree::clear_tree(const char *op)
{
	_model.clear_tree_data(nullptr);  // clear data only (don't set default op yet)

	if (op) {
		DeleteAllItems();
		if (strlen(op)) {
			_model.set_node(_model.allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
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
	_model.load_tree_data(index, deflt);
	DeleteAllItems();
	build_tree();
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

	auto info = _model.compute_node_visual_info(node);
	tree_nodes[node].flags = info.flags;
	int bitmap = static_cast<int>(info.image);
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
			auto child_info = _model.compute_node_visual_info(node);
			tree_nodes[node].flags = child_info.flags;
			int bmap = static_cast<int>(child_info.image);
			tree_nodes[node].handle = insert(tree_nodes[node].text, bmap, bmap, root);
		}

		node = tree_nodes[node].next;
	}
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
void sexp_tree::right_clicked()
{
	int i, j, subcategory_id;
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

		auto state = _model.compute_context_menu_state();

		// annotations only work in the event editor
		if (state.can_edit_comment) {
			menu.EnableMenuItem(ID_EDIT_COMMENT, MF_ENABLED);
		} else {
			menu.EnableMenuItem(ID_EDIT_COMMENT, MF_GRAYED);
		}
		if (state.can_edit_bg_color) {
			menu.EnableMenuItem(ID_EDIT_BG_COLOR, MF_ENABLED);
		} else {
			menu.EnableMenuItem(ID_EDIT_BG_COLOR, MF_GRAYED);
		}

		menu.EnableMenuItem(ID_SEXP_TREE_ADD_VARIABLE, MF_ENABLED);
		if (state.can_modify_variable) {
			menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_ENABLED);
		} else {
			menu.EnableMenuItem(ID_SEXP_TREE_MODIFY_VARIABLE, MF_GRAYED);
		}

		// Build variable menu from state
		for (const auto& var : state.replace_variables) {
			UINT flags = MF_STRING;
			if (!var.enabled) flags |= MF_GRAYED;
			if (!((var.var_index + 3) % 30)) flags |= MF_MENUBARBREAK;
			char buf[128];
			sprintf(buf, "%s (%s)", Sexp_variables[var.var_index].variable_name, Sexp_variables[var.var_index].text);
			replace_variable_menu->AppendMenu(flags, (ID_VARIABLE_MENU + var.var_index), buf);
		}

		// Build container name menu from state
		if (state.show_container_names) {
			const auto& containers = get_all_sexp_containers();
			for (int idx = 0; idx < (int)state.replace_container_names.size(); idx++) {
				UINT flags = MF_STRING;
				if (!state.replace_container_names[idx].enabled) flags |= MF_GRAYED;
				replace_container_name_menu->AppendMenu(flags, (ID_CONTAINER_NAME_MENU + idx), containers[idx].container_name.c_str());
			}
		}

		// Build container data menu from state
		if (state.show_container_data) {
			const auto& containers = get_all_sexp_containers();
			for (int idx = 0; idx < (int)state.replace_container_data.size(); idx++) {
				UINT flags = MF_STRING;
				if (!state.replace_container_data[idx].enabled) flags |= MF_GRAYED;
				replace_container_data_menu->AppendMenu(flags, (ID_CONTAINER_DATA_MENU + idx), containers[idx].container_name.c_str());
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
			if (SexpTreeModel::is_operator_hidden(Operators[i].value))
				continue;

			UINT add_flags = MF_STRING | (state.op_add_enabled[i] ? 0 : MF_GRAYED);
			UINT replace_flags = MF_STRING | (state.op_replace_enabled[i] ? 0 : MF_GRAYED);
			UINT insert_flags = MF_STRING | (state.op_insert_enabled[i] ? 0 : MF_GRAYED);

			// add only if it is not in a subcategory
			subcategory_id = get_subcategory(Operators[i].value);
			if (subcategory_id == OP_SUBCATEGORY_NONE)
			{
				for (j=0; j<(int)op_menu.size(); j++)
				{
					if (op_menu[j].id == get_category(Operators[i].value))
					{
						add_op_submenu[j].AppendMenu(add_flags, Operators[i].value, Operators[i].text.c_str());
						replace_op_submenu[j].AppendMenu(replace_flags, Operators[i].value | OP_REPLACE_FLAG, Operators[i].text.c_str());
						insert_op_submenu[j].AppendMenu(insert_flags, Operators[i].value | OP_INSERT_FLAG, Operators[i].text.c_str());
						break;
					}
				}
			}
			else
			{
				for (j=0; j<(int)op_submenu.size(); j++)
				{
					if (op_submenu[j].id == subcategory_id)
					{
						add_op_subcategory_menu[j].AppendMenu(add_flags, Operators[i].value, Operators[i].text.c_str());
						replace_op_subcategory_menu[j].AppendMenu(replace_flags, Operators[i].value | OP_REPLACE_FLAG, Operators[i].text.c_str());
						insert_op_subcategory_menu[j].AppendMenu(insert_flags, Operators[i].value | OP_INSERT_FLAG, Operators[i].text.c_str());
						break;
					}
				}
			}
		}

		// find local index (i) of current item (from its handle)
		SelectItem(h);
		update_item(h);

		// special case: item is a ROOT node, and a label that can be edited (not an item in the sexp tree)
		if (state.is_labeled_root) {
			if (state.is_root_editable) {
				menu.EnableMenuItem(ID_EDIT_TEXT, MF_ENABLED);
			} else {
				menu.EnableMenuItem(ID_EDIT_TEXT, MF_GRAYED);
			}

			menu.EnableMenuItem(ID_EDIT_COPY, MF_GRAYED);

			gray_menu_tree(popup_menu);
			state.cleanup();
			popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, mouse.x, mouse.y, this);
			return;
		}

		Assert(item_index != -1);  // handle not found, which should be impossible.
		if (!state.can_edit_text) {
			menu.EnableMenuItem(ID_EDIT_TEXT, MF_GRAYED);
		}

		if (!state.can_delete) {
			menu.EnableMenuItem(ID_DELETE, MF_GRAYED);
		}

		// Set add/replace state from pre-computed values
		m_add_count = state.add_count;
		m_replace_count = state.replace_count;
		m_modify_variable = state.modify_variable;

		if (state.can_add_number) menu.EnableMenuItem(ID_ADD_NUMBER, MF_ENABLED);
		if (state.can_add_string) menu.EnableMenuItem(ID_ADD_STRING, MF_ENABLED);
		if (state.can_replace_number) menu.EnableMenuItem(ID_REPLACE_NUMBER, MF_ENABLED);
		if (state.can_replace_string) menu.EnableMenuItem(ID_REPLACE_STRING, MF_ENABLED);

		// Build add data menu items from the data list
		if (state.add_data_list) {
			sexp_list_item* ptr = state.add_data_list;
			int data_idx = 0;
			while (ptr) {
				if (ptr->op < 0) {
					UINT flags = MF_STRING | MF_ENABLED;
					if (!((data_idx + 3) % 30)) flags |= MF_MENUBARBREAK;
					if (state.add_data_opf_type == OPF_VARIABLE_NAME) {
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

		// Build replace data menu items
		if (state.replace_data_list) {
			sexp_list_item* ptr = state.replace_data_list;
			int data_idx = 0;
			while (ptr) {
				if (ptr->op < 0) {
					if ((data_idx + 3) % 30)
						replace_data_menu->AppendMenu(MF_STRING | MF_ENABLED, ID_REPLACE_MENU + data_idx, ptr->text.c_str());
					else
						replace_data_menu->AppendMenu(MF_MENUBARBREAK | MF_STRING | MF_ENABLED, ID_REPLACE_MENU + data_idx, ptr->text.c_str());
				}
				data_idx++;
				ptr = ptr->next;
			}
		}

		// Clipboard and copy operations
		if (state.can_paste) menu.EnableMenuItem(ID_EDIT_PASTE, MF_ENABLED);
		if (state.can_paste_add) menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_ENABLED);
		if (state.can_cut) menu.EnableMenuItem(ID_EDIT_CUT, MF_ENABLED);
		if (!state.can_copy) menu.EnableMenuItem(ID_EDIT_COPY, MF_GRAYED);
		if (!state.can_paste) menu.EnableMenuItem(ID_EDIT_PASTE, MF_GRAYED);
		if (!state.can_paste_add) menu.EnableMenuItem(ID_EDIT_PASTE_SPECIAL, MF_GRAYED);

		gray_menu_tree(popup_menu);
		state.cleanup();
		popup_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, mouse.x, mouse.y, this);
	}
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
		if (_model._interface && _model._interface->getFlags()[TreeFlags::RootEditable]) {
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

int sexp_tree::end_label_edit(TVITEMA &item)
{
	if (!item.pszText)
		return 0;

	HTREEITEM h = item.hItem; 
	SCP_string str(item.pszText);
	int r = 1;
	uint node;

	for (node=0; node<tree_nodes.size(); node++)
		if (tree_nodes[node].handle == h)
			break;

	if (node == tree_nodes.size()) {
		if (_model._interface && _model._interface->getFlags()[TreeFlags::RootEditable]) {
			item_index = (int)GetItemData(h);
			_model._interface->onRootRenamed(item_index, str.c_str());
			return 1;
		} else
			Int3();  // root labels shouldn't have been editable!
	}

	Assert(node < tree_nodes.size());
	auto result = _model.validate_label_edit(node, str);

	if (result.is_operator) {
		if (!result.update_node && result.resolved_text == str) {
			return 0;	// Goober5000 - avoids crashing (no match found)
		}

		// use the text of the operator we found
		SetItemText(h, result.resolved_text.c_str());
		str = result.resolved_text;

		item_index = node;
		if (result.operator_index >= 0) {
			_actions.add_or_replace_operator(result.operator_index, 1);
		}
		r = 0;
	} else if (result.negative_number_error) {
		MessageBox("Can not enter a negative value", "Invalid Number", MB_ICONEXCLAMATION);
	}

	if (result.update_node) {
		*modified = 1;
		_model.apply_label_edit(node, result.resolved_text);
	}
	else {
		item.pszText = tree_nodes[node].text;
		return 1;
	}

	return r;
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
	auto opf_type = (sexp_opf_t)_model.query_node_argument_type(item_index);

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
		dlg.m_start_index = _model.get_item_index_to_var_index();

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
			_actions.delete_sexp_tree_variable(dlg.m_cur_variable_name);

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
			_actions.modify_sexp_tree_variable(old_name, sexp_var_index);

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
			if (_actions.rename_container_nodes(old_name, new_name)) {
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
		if (m_modify_variable || _model.query_node_argument_type(item_index) == OPF_CONTAINER_VALUE) {
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
		_actions.replace_variable_data(var_idx, (type | SEXPT_VARIABLE));

		return 1;
	}


	if ((id >= ID_ADD_MENU) && (id < ID_ADD_MENU + 511)) {
		auto saved_id = id;
		Assert(item_index >= 0);

		int type = 0;

		if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
			list = _model._opf.get_container_multidim_modifiers(item_index);
		} else {
			op = get_operator_index(tree_nodes[item_index].text);
			Assert(op >= 0);

			type = query_operator_argument_type(op, m_add_count);
			list = _model._opf.get_listing_opf(type, item_index, m_add_count);
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
		_actions.expand_operator(item_index);
		node = _actions.add_data(ptr->text.c_str(), ptr->type);
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
			_actions.replace_variable_data(var_idx, (type | SEXPT_VARIABLE));
			item_index = saved_item_index;
		}

		return 1;
	}

	if ((id >= ID_REPLACE_MENU) && (id < ID_REPLACE_MENU + 511)) {
		Assert(item_index >= 0);
		Assert(tree_nodes[item_index].parent >= 0);

		if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
			list = _model._opf.get_container_modifiers(tree_nodes[item_index].parent);
		} else {
			op = get_operator_index(tree_nodes[tree_nodes[item_index].parent].text);
			Assert(op >= 0);

			auto type = query_operator_argument_type(op, m_replace_count); // check argument type at this position
			list = _model._opf.get_listing_opf(type, tree_nodes[item_index].parent, m_replace_count);
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
		_actions.expand_operator(item_index);
		_actions.replace_data(ptr->text.c_str(), ptr->type);
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

		_actions.replace_container_name(containers[container_index]);
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
		_actions.replace_container_data(containers[container_index], (type | SEXPT_CONTAINER_DATA), true, true, true);

		HTREEITEM handle = tree_item_handle(tree_nodes[item_index]);
		expand_branch(handle);
	}

	for (op=0; op<(int)Operators.size(); op++) {
		if (id == Operators[op].value) {
			_actions.add_or_replace_operator(op);
			return 1;
		}

		if (id == (Operators[op].value | OP_REPLACE_FLAG)) {
			_actions.add_or_replace_operator(op, 1);
			expand_branch(item_handle);
			return 1;
		}

		if (id == (Operators[op].value | OP_INSERT_FLAG)) {
			int flags;

			z = tree_nodes[item_index].parent;
			flags = tree_nodes[item_index].flags;
			node = _model.allocate_node(z, item_index);
			_model.set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
			tree_nodes[node].flags = flags;
			if (z >= 0)
				h = tree_item_handle(tree_nodes[z]);

			else {
				h = GetParentItem(tree_item_handle(tree_nodes[item_index]));
				if (_model._interface && _model._interface->getFlags()[TreeFlags::LabeledRoot]) {
					_model._interface->onRootInserted(item_index, node);
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
				_actions.add_default_operator(op, i);

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
			_actions.expand_operator(item_index);
			if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
				_actions.replace_data("number", (SEXPT_NUMBER | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				_actions.replace_data("number", (SEXPT_NUMBER | SEXPT_VALID));
			}
			EditLabel(tree_item_handle(tree_nodes[item_index]));
			return 1;

		case ID_REPLACE_STRING:
			_actions.expand_operator(item_index);
			if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
				_actions.replace_data("string", (SEXPT_STRING | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				_actions.replace_data("string", (SEXPT_STRING | SEXPT_VALID));
			}
			EditLabel(tree_item_handle(tree_nodes[item_index]));
			return 1;

		case ID_ADD_STRING:	{
			int theNode;

			if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
				theNode = _actions.add_data("string", (SEXPT_STRING | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				theNode = _actions.add_data("string", (SEXPT_STRING | SEXPT_VALID));
			}
			EditLabel(tree_item_handle(tree_nodes[theNode]));
			return 1;
		}

		case ID_ADD_NUMBER:	{
			int theNode;

			if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
				theNode = _actions.add_data("number", (SEXPT_NUMBER | SEXPT_MODIFIER | SEXPT_VALID));
			} else {
				theNode = _actions.add_data("number", (SEXPT_NUMBER | SEXPT_VALID));
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
							_actions.add_or_replace_operator(op, 1);
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
						_actions.add_or_replace_operator(op, 1);
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

	if (_model._interface && _model._interface->getFlags()[TreeFlags::RootDeletable] && (item_index == -1)) {
		item_index = (int)GetItemData(item_handle);
		theNode = _model._interface->onRootDeleted(item_index);

		Assert(theNode >= 0);
		_model.free_node2(theNode);
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
	_model.free_node(item_index);
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
	_actions.clipboard_copy();
}

void sexp_tree::NodeReplacePaste()
{
	_actions.clipboard_paste_replace();
}

void sexp_tree::NodeAddPaste()
{
	_actions.clipboard_paste_add();
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
	if (source != -1) {
		_model.move_branch_data(source, parent);
		if (parent) {
			move_branch(tree_item_handle(tree_nodes[source]), tree_item_handle(tree_nodes[parent]));
		} else {
			move_branch(tree_item_handle(tree_nodes[source]));
		}
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

	if (!_model._interface || !_model._interface->getFlags()[TreeFlags::LabeledRoot] || GetParentItem(m_h_drag))
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

			if (_model._interface) {
				_model._interface->onRootMoved(node1, node2, insert_before);
			} else {
				UNREACHABLE("No interface set for labeled-root tree!");
			}

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

int sexp_tree::get_node(HTREEITEM h)
{
	for (int i = 0; i < static_cast<int>(tree_nodes.size()); i++) {
		if (tree_nodes[i].handle == h)
			return i;
	}
	return -1;
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
	// Validate operator help strings
	for (int i = 0; i < (int)Operators.size(); i++) {
		for (int j = 0; j < (int)op_menu.size(); j++) {
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

	// Find node index from handle
	int node_index = -1;
	for (int i = 0; i < (int)tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			node_index = i;
			break;
		}
	}

	// Build annotation comment
	SCP_string nodeComment;
	int thisIndex = event_annotation_lookup(node_index);
	if (thisIndex >= 0) {
		if (!Event_annotations[thisIndex].comment.empty()) {
			nodeComment = "Node Comments:\r\n   " + Event_annotations[thisIndex].comment;
		}
	}

	// Delegate to model for help text computation
	auto result = _model.compute_help_text(node_index, nodeComment);

	help_box->SetWindowText(result.help_text.c_str());
	if (mini_help_box)
		mini_help_box->SetWindowText(result.mini_help_text.c_str());
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

