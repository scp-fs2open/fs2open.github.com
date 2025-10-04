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
#include "ai/ailua.h"
#include "asteroid/asteroid.h"
#include "mission/missionmessage.h"
#include "mission/missioncampaign.h"
#include "mission/missionparse.h"
#include "missioneditor/common.h"
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
#include "prop/prop.h"

#include <ui/util/menu.h>
#include <ui/util/SignalBlockers.h>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QStyleOptionViewItem>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QApplication>
#include <QPainter>
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
		return ":/images/operator.png";
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
	return ":/images/operator.png";
}

QPoint s_dragStartPos;
QTreeWidgetItem* s_dragSourceRoot = nullptr;
bool s_dragging = false;

bool isRoot(QTreeWidgetItem* it)
{
	return it && !it->parent();
}
}

// SexpTreeEditorInterface methods are now in the shared sexp_tree_model.cpp

QIcon sexp_tree::convertNodeImageToIcon(NodeImage image) {
	return QIcon(node_image_to_resource_name(image));
}

class NoteBadgeDelegate final : public QStyledItemDelegate {
  public:
	explicit NoteBadgeDelegate(sexp_tree* tree) : QStyledItemDelegate(tree) {}

	void paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyleOptionViewItem opt(option);
		initStyleOption(&opt, index);

		// draw the standard icon + text first
		const QWidget* w = opt.widget;
		const QStyle* s = w ? w->style() : QApplication::style();
		s->drawControl(QStyle::CE_ItemViewItem, &opt, p, w);

		// if there�s a note, paint the badge directly after the text
		const QString note = index.data(sexp_tree::NoteRole).toString();
		if (!note.isEmpty()) {
			// where Qt drew the text
			QRect textRect = s->subElementRect(QStyle::SE_ItemViewItemText, &opt, w);

			// compute how much text actually fit (respect eliding)
			QFontMetrics fm(opt.font);
			const QString shown = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
			const int textWidth = fm.horizontalAdvance(shown);

			// pick an icon; use your existing mapping
			const QIcon icon = sexp_tree::convertNodeImageToIcon(NodeImage::COMMENT);
			const int dpi = p->device() ? p->device()->logicalDpiX() : 96;
			const int sz = opt.decorationSize.isValid() ? opt.decorationSize.height() : int(16 * dpi / 96);
			const QPixmap pm = icon.pixmap(sz, sz);

			// place badge just after the text, with a small pad
			const int pad = 10;
			int x = textRect.left() + textWidth + pad;
			int y = textRect.center().y() - pm.height() / 2;

			// keep inside cell if the row is very tight
			const int rightBound = option.rect.right() - 2;
			if (x + pm.width() > rightBound)
				x = rightBound - pm.width();

			p->save();
			// ensure good contrast on selected rows
			if (opt.state & QStyle::State_Selected)
				p->setCompositionMode(QPainter::CompositionMode_SourceOver);
			p->drawPixmap(x, y, pm);
			p->restore();
		}
	}
};

// constructor
sexp_tree::sexp_tree(QWidget* parent) : QTreeWidget(parent), _actions(_model, *this) {
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
	connect(this, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item, int /*column*/) {openNodeEditor(item);});

	setItemDelegateForColumn(0, new NoteBadgeDelegate(this));
}

sexp_tree::~sexp_tree() = default;

// --- ISexpTreeUI implementation ---

void* sexp_tree::ui_insert_item(const char* text, NodeImage image, void* parent_handle, void* insert_after)
{
	auto* hParent = static_cast<QTreeWidgetItem*>(parent_handle);
	auto* hAfter = static_cast<QTreeWidgetItem*>(insert_after);
	auto* item = insert(QString::fromUtf8(text), image, hParent, hAfter);
	return static_cast<void*>(item);
}

void sexp_tree::ui_delete_item(void* handle)
{
	delete static_cast<QTreeWidgetItem*>(handle);
}

void sexp_tree::ui_set_item_text(void* handle, const char* text)
{
	static_cast<QTreeWidgetItem*>(handle)->setText(0, QString::fromUtf8(text));
}

void sexp_tree::ui_set_item_image(void* handle, NodeImage image)
{
	static_cast<QTreeWidgetItem*>(handle)->setIcon(0, convertNodeImageToIcon(image));
}

void* sexp_tree::ui_get_child_item(void* handle)
{
	auto* item = static_cast<QTreeWidgetItem*>(handle);
	if (item->childCount() > 0)
		return static_cast<void*>(item->child(0));
	return nullptr;
}

bool sexp_tree::ui_has_children(void* handle)
{
	return static_cast<QTreeWidgetItem*>(handle)->childCount() > 0;
}

void sexp_tree::ui_expand_item(void* handle)
{
	static_cast<QTreeWidgetItem*>(handle)->setExpanded(true);
}

void sexp_tree::ui_select_item(void* handle)
{
	setCurrentItem(static_cast<QTreeWidgetItem*>(handle));
}

void sexp_tree::ui_ensure_visible(void* handle)
{
	scrollToItem(static_cast<QTreeWidgetItem*>(handle));
}

void sexp_tree::ui_notify_modified()
{
	modified();
}

void sexp_tree::ui_update_help(void* handle)
{
	update_help(static_cast<QTreeWidgetItem*>(handle));
}

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
	if (node < 0) node = root_item;
	return _model.query_false(node);
}

// builds an sexp of the tree and returns the index of it.  This allocates sexp nodes.
int sexp_tree::save_tree(int node) {
	if (node < 0) node = root_item;
	return _model.save_tree(node);
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
	return _model.save_branch(cur, at_root);
}

// find the next free tree node and return its index.
int sexp_tree::find_free_node() {
	return _model.find_free_node();
}

// allocate a node.  Remains used until freed.
int sexp_tree::allocate_node() {
	return _model.allocate_node();
}

// allocate a child node under 'parent'.  Appends to end of list.
int sexp_tree::allocate_node(int parent, int after) {
	return _model.allocate_node(parent, after);
}

// free a node and all its children.  Also clears pointers to it, if any.
//   node = node chain to free
//   cascade =  0: free just this node and children under it. (default)
//             !0: free this node and all siblings after it.
//
void sexp_tree::free_node(int node, int cascade) {
	_model.free_node(node, cascade);
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
	_model.set_node(node, type, text);
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

	tree_nodes[node].handle = insert(tree_nodes[node].text, bitmap, root);
	root = tree_item_handle(tree_nodes[node]);

	tree_item_handle(tree_nodes[node])->setFlags(
		tree_item_handle(tree_nodes[node])->flags().setFlag(Qt::ItemIsEditable, (tree_nodes[node].flags & EDITABLE)));

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

			tree_item_handle(tree_nodes[node])->setFlags(
				tree_item_handle(tree_nodes[node])->flags().setFlag(Qt::ItemIsEditable, (tree_nodes[node].flags & EDITABLE)));
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
	return _model.count_args(node);
}

// identify what type of argument this is.  You call it with the node of the first argument
// of an operator.  It will search through enough of the arguments to determine what type of
// data they are.
int sexp_tree::identify_arg_type(int node) {
	return _model.identify_arg_type(node);
}

// given a tree node, returns the argument type it should be.
// OPF_NULL means no value (or a "void" value) is returned.  OPF_NONE means there shouldn't be any argument at this position at all.
int sexp_tree::query_node_argument_type(int node) const {
	return _model.query_node_argument_type(node);
}

// Look for the valid operator that is the closest match for 'str' and return the operator
// number of it.  What operators are valid is determined by 'node', and an operator is valid
// if it is allowed to fit at position 'node'
//
const SCP_string &sexp_tree::match_closest_operator(const SCP_string &str, int node) {
	return _model.match_closest_operator(str, node);
}

// adds to or replaces (based on passed in flag) the current operator
void sexp_tree::add_or_replace_operator(int op, int replace_flag) {
	_actions.add_or_replace_operator(op, replace_flag);
}

// sexp_list_item methods are now in the shared sexp_tree_model.cpp

int sexp_tree::add_default_operator(int op_index, int argnum) {
	return _actions.add_default_operator(op_index, argnum);
}

int sexp_tree::get_default_value(sexp_list_item* item, char* text_buf, int op, int i) {
	return _model.get_default_value(item, text_buf, op, i);
}

int sexp_tree::query_default_argument_available(int op) {
	return _model.query_default_argument_available(op);
}

int sexp_tree::query_default_argument_available(int op, int i) {
	return _model.query_default_argument_available(op, i);
}

// expand a combined line (one with an operator and its one argument on the same line) into
// 2 lines.
void sexp_tree::expand_operator(int node) {
	_actions.expand_operator(node);
}

// expand a CTreeCtrl branch and all of its children
void sexp_tree::expand_branch(QTreeWidgetItem* h) {
	h->setExpanded(true);
	for (auto i = 0; i < h->childCount(); ++i) {
		expand_branch(h->child(i));
	}
}

// edit the comment for the operator pointed to by item_index
void sexp_tree::editNoteForItem(QTreeWidgetItem* it)
{
	const QString old = it->data(0, NoteRole).toString();
	bool ok = false;
	const QString text = QInputDialog::getMultiLineText(this, tr("Edit Note"), tr("Node note:"), old, &ok);
	if (!ok)
		return;
	it->setData(0, NoteRole, text);
	applyVisuals(it);

	Q_EMIT nodeAnnotationChanged(static_cast<void*>(it), text);
}

void sexp_tree::editBgColorForItem(QTreeWidgetItem* it)
{
	const auto start = it->data(0, BgColorRole).value<QColor>();
	const QColor c = QColorDialog::getColor(start.isValid() ? start : Qt::yellow, this, tr("Choose Background Color"));
	if (!c.isValid())
		return;
	it->setData(0, BgColorRole, c);
	applyVisuals(it);

	Q_EMIT nodeBgColorChanged(static_cast<void*>(it), c);
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
int sexp_tree::add_data(const char* new_data, int type) {
	return _actions.add_data(new_data, type);
}

// add a (variable) data node under operator pointed to by item_index
int sexp_tree::add_variable_data(const char* new_data, int type) {
	return _actions.add_variable_data(new_data, type);
}

// add a container name node under operator pointed to by item_index
int sexp_tree::add_container_name(const char* container_name)
{
	return _actions.add_container_name(container_name);
}

// add a (container) data node under operator pointed to by item_index
void sexp_tree::add_container_data(const char* container_name)
{
	_actions.add_container_data(container_name);
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
		tree_nodes[node].handle = insert(op, NodeImage::OPERATOR, tree_item_handle(tree_nodes[item_index]));
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
int sexp_tree::node_error(int node, const char* msg, int* bypass) {
	if (bypass) {
		*bypass = 1;
	}

	item_index = node;
	auto item_handle = tree_item_handle(tree_nodes[node]);
	if (tree_nodes[node].flags & COMBINED) {
		item_handle = tree_item_handle(tree_nodes[tree_nodes[node].parent]);
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
	setCurrentItem(tree_item_handle(tree_nodes[node]));
	scrollToItem(tree_item_handle(tree_nodes[node]));
}

// because the MFC function EnsureVisible() doesn't do what it says it does, I wrote this.
void sexp_tree::ensure_visible(int node) {
	auto handle = tree_item_handle(tree_nodes[node])->parent();

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
	return _model.get_modify_variable_type(parent);
}


void sexp_tree::verify_and_fix_arguments(int node) {
	_actions.verify_and_fix_arguments(node);
}

void sexp_tree::replace_data(const char* new_data, int type) {
	_actions.replace_data(new_data, type);
}


// Replaces data with sexp_variable type data
void sexp_tree::replace_variable_data(int var_idx, int type) {
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

void sexp_tree::replace_operator(const char* op) {
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

			move_branch(tree_item_handle(tree_nodes[source]), tree_item_handle(tree_nodes[parent]));

		} else {
			move_branch(tree_item_handle(tree_nodes[source]));
		}
	}
}

QTreeWidgetItem* sexp_tree::move_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent, QTreeWidgetItem* after)
{
	if (!source)
		return nullptr;

	// Find matching tree_nodes slot, if any, to update its handle
	uint idx = 0;
	while (idx < tree_nodes.size() && tree_nodes[idx].handle != source) {
		++idx;
	}

	// Create the destination item
	const auto icon = source->icon(0);
	QTreeWidgetItem* h = insertWithIcon(source->text(0), icon, parent, after);
	if (idx < tree_nodes.size()) {
		tree_nodes[idx].handle = h;
	}

	// Copy all per-item data we rely on for annotations/visuals
	h->setData(0, FormulaDataRole, source->data(0, FormulaDataRole));
	h->setData(0, NoteRole, source->data(0, NoteRole));
	h->setData(0, BgColorRole, source->data(0, BgColorRole));
	applyVisuals(h);

	// Move children safely
	while (source->childCount() > 0) {
		auto* child = source->child(0);
		move_branch(child, h);
	}

	h->setExpanded(source->isExpanded());

	// Remove the old item from the tree
	if (auto* p = source->parent()) {
		p->removeChild(source);
		delete source;
	} else if (auto* tw = source->treeWidget()) {
		const int topIdx = tw->indexOfTopLevelItem(source);
		if (topIdx >= 0)
			tw->takeTopLevelItem(topIdx);
		delete source;
	}

	return h;
}

void sexp_tree::copy_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent, QTreeWidgetItem* after)
{
	if (!source)
		return;

	const auto icon = source->icon(0);
	QTreeWidgetItem* h = insertWithIcon(source->text(0), icon, parent, after);

	// Copy per-item data/annotations
	h->setData(0, FormulaDataRole, source->data(0, FormulaDataRole));
	h->setData(0, NoteRole, source->data(0, NoteRole));
	h->setData(0, BgColorRole, source->data(0, BgColorRole));
	applyVisuals(h);

	// Copy children (recursively COPY, not move)
	for (int i = 0; i < source->childCount(); ++i) {
		copy_branch(source->child(i), h);
	}

	h->setExpanded(source->isExpanded());
}

// Old version of move_root
/*void sexp_tree::move_root(QTreeWidgetItem* source, QTreeWidgetItem* dest, bool insert_before)
{
	auto after = dest;

	if (insert_before) {
		Warning(LOCATION, "Inserting before a tree item is not yet implemented in qtFRED");
	}

	auto h = move_branch(source, itemFromIndex(rootIndex()), after);
	setCurrentItem(h);
	modified();
}*/

void sexp_tree::move_root(QTreeWidgetItem* source, QTreeWidgetItem* dest, bool insert_before)
{
	if (!source || !dest)
		return;
	if (source->parent() || dest->parent())
		return; // roots only

	// Take the source out of the top-level list
	const int srcIdx = indexOfTopLevelItem(source);
	if (srcIdx < 0)
		return;

	// Remove first so the destination index we compute is correct after removal
	QTreeWidgetItem* moved = takeTopLevelItem(srcIdx);

	// Recompute the current index of dest after the removal
	int dstIdx = indexOfTopLevelItem(dest);
	if (dstIdx < 0) {
		// put it back where it was
		insertTopLevelItem(srcIdx, moved);
		return;
	}

	if (!insert_before) {
		// inserting after the drop target
		++dstIdx;
	}

	// Clamp and insert
	dstIdx = std::max(0, std::min(dstIdx, topLevelItemCount()));
	insertTopLevelItem(dstIdx, moved);
	setCurrentItem(moved);

	// Mark the tree modified
	modified();

	Q_EMIT rootOrderChanged();
}

QTreeWidgetItem*
sexp_tree::insert(const QString& lpszItem, NodeImage image, QTreeWidgetItem* hParent, QTreeWidgetItem* hInsertAfter) {
	return insertWithIcon(lpszItem, convertNodeImageToIcon(image), hParent, hInsertAfter);
}

void sexp_tree::keyPressEvent(QKeyEvent* e)
{
	// Clear stale state if popup was closed externally
	if (_opPopup && _opPopupActive && !_opPopup->isVisible()) {
		_opPopupActive = false;
		_opNodeIndex = -1;
	}

	// Route keys while popup is active
	if (_opPopupActive && _opPopup) {
		switch (e->key()) {
		case Qt::Key_Escape:
			endOperatorQuickSearch(false);
			return;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			endOperatorQuickSearch(true);
			return;
		case Qt::Key_Up:
		case Qt::Key_Down:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
		case Qt::Key_Home:
		case Qt::Key_End:
			QCoreApplication::sendEvent(_opList, e);
			return;
		default:
			QCoreApplication::sendEvent(_opEdit, e);
			return;
		}
	}

	// Space opens the editor for the selected node
	if (e->key() == Qt::Key_Space && currentItem()) {
		openNodeEditor(currentItem());
		return;
	}

	QTreeWidget::keyPressEvent(e);
}

bool sexp_tree::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == _opPopup) {
		switch (ev->type()) {
		case QEvent::Hide:
		case QEvent::Close:
		case QEvent::WindowDeactivate:
			// Treat any external close as cancel; just clear state.
			_opPopupActive = false;
			_opNodeIndex = -1;
			setFocus(Qt::OtherFocusReason);
			break;
		default:
			break;
		}
	}
	return QTreeWidget::eventFilter(obj, ev);
}

void sexp_tree::mousePressEvent(QMouseEvent* e)
{
	s_dragStartPos = e->pos();
	s_dragSourceRoot = itemAt(e->pos());
	if (!isRoot(s_dragSourceRoot))
		s_dragSourceRoot = nullptr; // roots only
	s_dragging = false;
	QTreeWidget::mousePressEvent(e);
}

void sexp_tree::mouseMoveEvent(QMouseEvent* e)
{
	if (!s_dragSourceRoot) {
		QTreeWidget::mouseMoveEvent(e);
		return;
	}
	if (!(e->buttons() & Qt::LeftButton)) {
		QTreeWidget::mouseMoveEvent(e);
		return;
	}
	const int dist = (e->pos() - s_dragStartPos).manhattanLength();
	if (!s_dragging && dist < QApplication::startDragDistance()) {
		QTreeWidget::mouseMoveEvent(e);
		return;
	}

	// �Dragging� � we just highlight potential drop target (a root under the cursor)
	s_dragging = true;
	if (auto* over = itemAt(e->pos())) {
		if (isRoot(over))
			setCurrentItem(over); // simple visual cue like OG�s SelectDropTarget
	}

	// No QDrag payload; we�ll do the move on mouse release to keep logic simple.
	QTreeWidget::mouseMoveEvent(e);
}

void sexp_tree::mouseReleaseEvent(QMouseEvent* e)
{
	if (s_dragging && s_dragSourceRoot) {
		auto* dropTarget = itemAt(e->pos());
		if (dropTarget && isRoot(dropTarget) && dropTarget != s_dragSourceRoot) {
			// OG rule: if moving up, insert_before=true; if moving down, insert_after
			// (so we �end up where we dropped�). :contentReference[oaicite:1]{index=1}
			const int srcIdx = indexOfTopLevelItem(s_dragSourceRoot);
			const int dstIdx = indexOfTopLevelItem(dropTarget);
			const bool insert_before = (srcIdx > dstIdx);

			// Perform the visual move
			move_root(s_dragSourceRoot, dropTarget, insert_before);
		}
	}
	s_dragSourceRoot = nullptr;
	s_dragging = false;
	QTreeWidget::mouseReleaseEvent(e);
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
	return tree_item_handle(tree_nodes[node]);
}

const char* sexp_tree::help(int code) {
	return SexpTreeModel::help(code);
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

int sexp_tree::get_root(int node) {
	while (tree_nodes[node].parent >= 0) {
		node = tree_nodes[node].parent;
	}
	return node;
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
					mprintf(("Allender!  If you add new sexp operators, add help for them too! :) Sexp %s has no help.\n", Operators[i].text.c_str()));
				}
			}
		}
	}

	for (i = 0; i < (int) tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			break;
		}
	}

	// Node comments are not yet implemented in qtFRED, so just adding some base code here
	// that can be used when the feature is completed - Mjn

	//int thisIndex = event_annotation_lookup(h);
	SCP_string nodeComment;

	//if (thisIndex >= 0) {
		//if (!Event_annotations[thisIndex].comment.empty()) {
			//nodeComment = "Node Comments:\r\n   " + Event_annotations[thisIndex].comment;
		//}
	//} else {
		nodeComment = "";
	//}

	if ((i >= (int) tree_nodes.size()) || !tree_nodes[i].type) {
		helpChanged(nodeComment.c_str());
		miniHelpChanged("");
		return;
	}

	// Now that we're done with top level nodes we can add the empty lines because
	// everything else below is supposed to have help text
	if (!nodeComment.empty())
		nodeComment.insert(0, "\r\n\r\n");

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
						auto text = QString("Message Text:\n%1%2").arg(Messages[j].message, nodeComment.c_str());
						helpChanged(text);
						return;
					}
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

				// If we still didn't find anything, say so!
				if (desc.empty())
					desc = "Unknown flag. Let a coder know!";

				auto text = QString("%s").arg(desc.c_str());
				helpChanged(text);
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

				auto text = QString("%s").arg(desc.c_str());
				helpChanged(text);
				return;
			}
		}

		i = z;
	}

	code = get_operator_const(tree_nodes[i].text);
	auto str = help(code);
	QString text;
	if (!str) {
		text = QString("No help available%1").arg(nodeComment.c_str());
	} else {
		text = QString("%1%2").arg(str, nodeComment.c_str());
	}

	helpChanged(text);
}

// find list of sexp_tree nodes with text
// stuff node indices into find[]
int sexp_tree::find_text(const char* text, int* find, int max_depth) {
	return _model.find_text(text, find, max_depth);
}


// Determine if a given opf code has a restricted argument range (i.e. has a specific, limited
// set of argument values, or has virtually unlimited possibilities.  For example, boolean values
// only have true or false, so it is restricted, but a number could be anything, so it's not.
//
int sexp_tree::query_restricted_opf_range(int opf) {
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
sexp_list_item* sexp_tree::get_listing_opf(int opf, int parent_node, int arg_index) {
	return _model.get_listing_opf(opf, parent_node, arg_index);
}

// Goober5000
int sexp_tree::find_argument_number(int parent_node, int child_node) const {
	return _model.find_argument_number(parent_node, child_node);
}

// Goober5000
// backtrack through parents until we find the operator matching
// parent_op, then find the argument we went through
int sexp_tree::find_ancestral_argument_number(int parent_op, int child_node) const {
	return _model.find_ancestral_argument_number(parent_op, child_node);
}

/**
* Gets the proper data image for the tree item's place
* in its parent hierarchy.
*/
NodeImage sexp_tree::get_data_image(int node) {
	return _model.get_data_image(node);
}

int sexp_tree::get_sibling_place(int node) {
	return _model.get_sibling_place(node);
}


sexp_list_item* sexp_tree::get_listing_opf_null() {
	return _model.get_listing_opf_null();
}

sexp_list_item* sexp_tree::get_listing_opf_flexible_argument() {
	return _model.get_listing_opf_flexible_argument();
}

sexp_list_item* sexp_tree::get_listing_opf_bool(int parent_node) {
	return _model.get_listing_opf_bool(parent_node);
}

sexp_list_item* sexp_tree::get_listing_opf_positive() {
	return _model.get_listing_opf_positive();
}

sexp_list_item* sexp_tree::get_listing_opf_number() {
	return _model.get_listing_opf_number();
}

sexp_list_item* sexp_tree::get_listing_opf_ship(int parent_node) {
	return _model.get_listing_opf_ship(parent_node);
}

sexp_list_item *sexp_tree::get_listing_opf_prop()
{
	return _model.get_listing_opf_prop();
}

sexp_list_item* sexp_tree::get_listing_opf_wing() {
	return _model.get_listing_opf_wing();
}

// specific types of subsystems we're looking for
#define OPS_CAP_CARGO		1	
#define OPS_STRENGTH		2
#define OPS_BEAM_TURRET		3
#define OPS_AWACS			4
#define OPS_ROTATE			5
#define OPS_TRANSLATE		6
#define OPS_ARMOR			7
sexp_list_item* sexp_tree::get_listing_opf_subsystem(int parent_node, int arg_index) {
	return _model.get_listing_opf_subsystem(parent_node, arg_index);
}

sexp_list_item* sexp_tree::get_listing_opf_subsystem_type(int parent_node) {
	return _model.get_listing_opf_subsystem_type(parent_node);
}

sexp_list_item* sexp_tree::get_listing_opf_point() {
	return _model.get_listing_opf_point();
}

sexp_list_item* sexp_tree::get_listing_opf_iff() {
	return _model.get_listing_opf_iff();
}

sexp_list_item* sexp_tree::get_listing_opf_ai_class() {
	return _model.get_listing_opf_ai_class();
}

sexp_list_item* sexp_tree::get_listing_opf_support_ship_class() {
	return _model.get_listing_opf_support_ship_class();
}

sexp_list_item* sexp_tree::get_listing_opf_ssm_class() {
	return _model.get_listing_opf_ssm_class();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_with_bay() {
	return _model.get_listing_opf_ship_with_bay();
}

sexp_list_item* sexp_tree::get_listing_opf_soundtrack_name() {
	return _model.get_listing_opf_soundtrack_name();
}

sexp_list_item* sexp_tree::get_listing_opf_arrival_location() {
	return _model.get_listing_opf_arrival_location();
}

sexp_list_item* sexp_tree::get_listing_opf_departure_location() {
	return _model.get_listing_opf_departure_location();
}

sexp_list_item* sexp_tree::get_listing_opf_arrival_anchor_all() {
	return _model.get_listing_opf_arrival_anchor_all();
}

sexp_list_item* sexp_tree::get_listing_opf_ai_goal(int parent_node) {
	return _model.get_listing_opf_ai_goal(parent_node);
}

sexp_list_item* sexp_tree::get_listing_opf_docker_point(int parent_node, int arg_num) {
	return _model.get_listing_opf_docker_point(parent_node, arg_num);
}

sexp_list_item* sexp_tree::get_listing_opf_dockee_point(int parent_node) {
	return _model.get_listing_opf_dockee_point(parent_node);
}

sexp_list_item* sexp_tree::get_listing_opf_message() {
	return _model.get_listing_opf_message();
}

sexp_list_item* sexp_tree::get_listing_opf_persona() {
	return _model.get_listing_opf_persona();
}

sexp_list_item* sexp_tree::get_listing_opf_font() {
	return _model.get_listing_opf_font();
}

sexp_list_item* sexp_tree::get_listing_opf_who_from() {
	return _model.get_listing_opf_who_from();
}

sexp_list_item* sexp_tree::get_listing_opf_priority() {
	return _model.get_listing_opf_priority();
}

sexp_list_item* sexp_tree::get_listing_opf_sound_environment() {
	return _model.get_listing_opf_sound_environment();
}

sexp_list_item* sexp_tree::get_listing_opf_sound_environment_option() {
	return _model.get_listing_opf_sound_environment_option();
}

sexp_list_item* sexp_tree::get_listing_opf_adjust_audio_volume() {
	return _model.get_listing_opf_adjust_audio_volume();
}

sexp_list_item* sexp_tree::get_listing_opf_builtin_hud_gauge() {
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

sexp_list_item* sexp_tree::get_listing_opf_ship_effect() {
	return _model.get_listing_opf_ship_effect();
}

sexp_list_item* sexp_tree::get_listing_opf_explosion_option() {
	return _model.get_listing_opf_explosion_option();
}

sexp_list_item* sexp_tree::get_listing_opf_waypoint_path() {
	return _model.get_listing_opf_waypoint_path();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_point() {
	return _model.get_listing_opf_ship_point();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_wholeteam() {
	return _model.get_listing_opf_ship_wing_wholeteam();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_shiponteam_point() {
	return _model.get_listing_opf_ship_wing_shiponteam_point();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_point() {
	return _model.get_listing_opf_ship_wing_point();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing_point_or_none() {
	return _model.get_listing_opf_ship_wing_point_or_none();
}

sexp_list_item* sexp_tree::get_listing_opf_mission_name() {
	return _model.get_listing_opf_mission_name();
}

sexp_list_item* sexp_tree::get_listing_opf_goal_name(int parent_node) {
	return _model.get_listing_opf_goal_name(parent_node);
}

sexp_list_item* sexp_tree::get_listing_opf_ship_wing() {
	return _model.get_listing_opf_ship_wing();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_prop()
{
	return _model.get_listing_opf_ship_prop();
}

sexp_list_item* sexp_tree::get_listing_opf_order_recipient() {
	return _model.get_listing_opf_order_recipient();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_type() {
	return _model.get_listing_opf_ship_type();
}

sexp_list_item* sexp_tree::get_listing_opf_keypress() {
	return _model.get_listing_opf_keypress();
}

sexp_list_item* sexp_tree::get_listing_opf_event_name(int parent_node) {
	return _model.get_listing_opf_event_name(parent_node);
}

sexp_list_item* sexp_tree::get_listing_opf_ai_order() {
	return _model.get_listing_opf_ai_order();
}

sexp_list_item* sexp_tree::get_listing_opf_skill_level() {
	return _model.get_listing_opf_skill_level();
}

sexp_list_item* sexp_tree::get_listing_opf_cargo() {
	return _model.get_listing_opf_cargo();
}

sexp_list_item* sexp_tree::get_listing_opf_string() {
	return _model.get_listing_opf_string();
}

sexp_list_item* sexp_tree::get_listing_opf_medal_name() {
	return _model.get_listing_opf_medal_name();
}

sexp_list_item* sexp_tree::get_listing_opf_weapon_name() {
	return _model.get_listing_opf_weapon_name();
}

sexp_list_item* sexp_tree::get_listing_opf_intel_name() {
	return _model.get_listing_opf_intel_name();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_class_name() {
	return _model.get_listing_opf_ship_class_name();
}

sexp_list_item* sexp_tree::get_listing_opf_prop_class_name()
{
	return _model.get_listing_opf_prop_class_name();
}

sexp_list_item* sexp_tree::get_listing_opf_huge_weapon() {
	return _model.get_listing_opf_huge_weapon();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_not_player() {
	return _model.get_listing_opf_ship_not_player();
}

sexp_list_item* sexp_tree::get_listing_opf_ship_or_none() {
	return _model.get_listing_opf_ship_or_none();
}

sexp_list_item* sexp_tree::get_listing_opf_subsystem_or_none(int parent_node, int arg_index) {
	return _model.get_listing_opf_subsystem_or_none(parent_node, arg_index);
}

sexp_list_item* sexp_tree::get_listing_opf_subsys_or_generic(int parent_node, int arg_index)
{
	return _model.get_listing_opf_subsys_or_generic(parent_node, arg_index);
}

sexp_list_item* sexp_tree::get_listing_opf_jump_nodes() {
	return _model.get_listing_opf_jump_nodes();
}

// creates list of Sexp_variables
sexp_list_item* sexp_tree::get_listing_opf_variable_names() {
	return _model.get_listing_opf_variable_names();
}

// get default skybox model name
sexp_list_item* sexp_tree::get_listing_opf_skybox_model() {
	return _model.get_listing_opf_skybox_model();
}

sexp_list_item* sexp_tree::get_listing_opf_skybox_flags() {
	return _model.get_listing_opf_skybox_flags();
}

sexp_list_item* sexp_tree::get_listing_opf_background_bitmap() {
	return _model.get_listing_opf_background_bitmap();
}

sexp_list_item* sexp_tree::get_listing_opf_sun_bitmap() {
	return _model.get_listing_opf_sun_bitmap();
}

sexp_list_item* sexp_tree::get_listing_opf_nebula_storm_type() {
	return _model.get_listing_opf_nebula_storm_type();
}

sexp_list_item* sexp_tree::get_listing_opf_nebula_poof() {
	return _model.get_listing_opf_nebula_poof();
}

sexp_list_item* sexp_tree::get_listing_opf_turret_target_order() {
	return _model.get_listing_opf_turret_target_order();
}

sexp_list_item* sexp_tree::get_listing_opf_turret_types()
{
	return _model.get_listing_opf_turret_types();
}

sexp_list_item* sexp_tree::get_listing_opf_post_effect() {
	return _model.get_listing_opf_post_effect();
}


sexp_list_item* sexp_tree::get_listing_opf_turret_target_priorities() {
	return _model.get_listing_opf_turret_target_priorities();
}

sexp_list_item* sexp_tree::get_listing_opf_armor_type() {
	return _model.get_listing_opf_armor_type();
}

sexp_list_item* sexp_tree::get_listing_opf_damage_type() {
	return _model.get_listing_opf_damage_type();
}

sexp_list_item* sexp_tree::get_listing_opf_animation_type() {
	return _model.get_listing_opf_animation_type();
}

sexp_list_item* sexp_tree::get_listing_opf_hud_elements() {
	return _model.get_listing_opf_hud_elements();
}

sexp_list_item* sexp_tree::get_listing_opf_weapon_banks() {
	return _model.get_listing_opf_weapon_banks();
}

sexp_list_item* sexp_tree::get_listing_opf_mission_moods() {
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

sexp_list_item* sexp_tree::get_listing_opf_wing_flags() {
	return _model.get_listing_opf_wing_flags();
}

sexp_list_item* sexp_tree::get_listing_opf_team_colors() {
	return _model.get_listing_opf_team_colors();
}

sexp_list_item* sexp_tree::get_listing_opf_nebula_patterns() {
	return _model.get_listing_opf_nebula_patterns();
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

sexp_list_item* sexp_tree::get_listing_opf_lua_general_orders()
{
	return _model.get_listing_opf_lua_general_orders();
}

sexp_list_item* sexp_tree::get_listing_opf_message_types()
{
	return _model.get_listing_opf_message_types();
}

sexp_list_item *sexp_tree::get_listing_opf_asteroid_types()
{
	return _model.get_listing_opf_asteroid_types();
}

sexp_list_item *sexp_tree::get_listing_opf_debris_types()
{
	return _model.get_listing_opf_debris_types();
}

sexp_list_item* sexp_tree::get_listing_opf_lua_enum(int parent_node, int arg_index)
{
	return _model.get_listing_opf_lua_enum(parent_node, arg_index);
}

sexp_list_item* sexp_tree::get_listing_opf_mission_custom_strings()
{
	return _model.get_listing_opf_mission_custom_strings();
}

sexp_list_item* sexp_tree::get_listing_opf_game_snds() {
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

sexp_list_item *sexp_tree::get_container_modifiers(int con_data_node) const
{
	return _model.get_container_modifiers(con_data_node);
}

sexp_list_item *sexp_tree::get_list_container_modifiers()
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
	return _model.get_item_index_to_var_index();
}

int sexp_tree::get_tree_name_to_sexp_variable_index(const char* tree_name) {
	return SexpTreeModel::get_tree_name_to_sexp_variable_index(tree_name);
}

int sexp_tree::get_variable_count(const char* var_name) {
	return _model.get_variable_count(var_name);
}

// Returns the number of times a variable with this name has been used by player loadout
int sexp_tree::get_loadout_variable_count(int var_index) {
	return _model.get_loadout_variable_count(var_index);
}

bool sexp_tree::is_container_name_opf_type(const int op_type)
{
	return SexpTreeModel::is_container_name_opf_type(op_type);
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

	popup_menu->addSection(tr("Annotations"));
	auto edit_comment_act = popup_menu->addAction(tr("Edit Comment"), this, [this, h]() { editNoteForItem(h); });
	auto edit_color_act = popup_menu->addAction(tr("Edit Color"), this, [this, h]() { editBgColorForItem(h); });
	edit_comment_act->setEnabled(_interface->getFlags()[TreeFlags::AnnotationsAllowed]);
	edit_color_act->setEnabled(_interface->getFlags()[TreeFlags::AnnotationsAllowed]);

	popup_menu->addSection(tr("Copy operations"));
	auto cut_act = popup_menu->addAction(tr("Cut"), this, [this]() { cutActionHandler(); }, QKeySequence::Cut);
	cut_act->setEnabled(false);
	auto copy_act = popup_menu->addAction(tr("Copy"), this, [this]() { copyActionHandler(); }, QKeySequence::Copy);
	auto paste_act = popup_menu->addAction(tr("Paste"), this, [this]() { pasteActionHandler(); }, QKeySequence::Paste); //TODO match paste/add paste
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
  
  // TODO: Context menu extras will be handled through a Qt-specific interface extension
  // when needed. Currently no dialog provides extras (all return empty list).

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
				// op < 0 means we're on a container modifier, and nested Replace Container Data is allowed
				if (op_type != OPF_VARIABLE_NAME && (op < 0 || !is_argument_provider_op(Operators[op].value))) {
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
void sexp_tree::deleteActionHandler()
{
	if (currentItem() == nullptr || !_interface) {
		return;
	}

	auto* item = currentItem();
	const bool isRootItem = (item->parent() == nullptr);

	// Root delete: allowed if flag is set and the selected item is a top-level row
	if (_interface->getFlags()[TreeFlags::RootDeletable] && isRootItem) {
		const int formulaNode = item->data(0, FormulaDataRole).toInt();

		// Tell the dialog/model first so it can drop the event row
		rootNodeDeleted(formulaNode);

		// Free the underlying SEXP subtree safely
		if (formulaNode >= 0) {
			free_node2(formulaNode);
		}

		// Remove the UI item and reset selection/index
		delete item;
		setCurrentItemIndex(-1);
		modified();
		return;
	}

	// Non-root delete
	Assertion(item_index >= 0, "Attempt to delete node at invalid index!");
	auto* h_parent = item->parent();
	const int parent = tree_nodes[item_index].parent;

	// If we somehow reached here on a root, bail safely
	if (parent == -1) {
		// Treat it as a root delete fallback
		const int formulaNode = item->data(0, FormulaDataRole).toInt();
		rootNodeDeleted(formulaNode);
		if (formulaNode >= 0)
			free_node2(formulaNode);
		delete item;
		setCurrentItemIndex(-1);
		modified();
		return;
	}

	Assertion(tree_nodes[parent].handle == h_parent, "Tree node handle mismatch!");
	free_node(item_index);
	delete item;

	modified();
}
void sexp_tree::editDataActionHandler() {
	beginItemEdit(currentItem());
}

// Compute the valid operators for replacing/adding at the given node, based on parent arg type.
// This mirrors the original menu enable/disable logic. See original for how "type" is computed.
QStringList sexp_tree::validOperatorsForNode(int nodeIndex)
{
	QStringList out;
	if (nodeIndex < 0 || nodeIndex >= static_cast<int>(tree_nodes.size()))
		return out;

	const int parent = tree_nodes[nodeIndex].parent;
	const int argIndex = (parent >= 0) ? find_argument_number(parent, nodeIndex) : 0;

	// Original behavior: compute the OPF type expected at this node
	const int opf = query_node_argument_type(nodeIndex); // handles top-level = OPF_NULL, etc.
	if (opf < 0)
		return out;

	// Build the canonical list for this OPF (this mirrors classic FRED)
	sexp_list_item* list = get_listing_opf(opf, parent, argIndex); // may be nullptr
	for (auto* p = list; p; p = p->next) {
		if (p->op >= 0) {
			const int opIndex = p->op;

			// Optional: keep parity with the menu, which disables ops lacking default args
			if (!query_default_argument_available(opIndex))
				continue;

			out.push_back(QString::fromStdString(Operators[opIndex].text));
		}
		// (items with p->op < 0 are data items like strings/ships/etc.; we ignore for operator search)
	}

	if (list)
		list->destroy();

	out.removeDuplicates();
	std::sort(out.begin(), out.end(), [](const QString& a, const QString& b) {
		return a.compare(b, Qt::CaseInsensitive) < 0;
	});
	return out;
}

void sexp_tree::openNodeEditor(QTreeWidgetItem* item)
{
	if (!item || !_interface)
		return;

	// if this is the root and it's not editable, bail.
	if (!_interface->getFlags()[TreeFlags::RootEditable] && !item->parent())
		return;

	if (item && !item->parent()) { // root only
		beginItemEdit(item);       // sets _currently_editing + calls editItem
		return;
	}

	// If an operator popup is already up, ignore
	if (_opPopupActive && _opPopup && _opPopup->isVisible())
		return;

	// Map item -> internal node index
	int nodeIdx = -1;
	for (uint i = 0; i < tree_nodes.size(); ++i) {
		if (tree_nodes[i].handle == item) {
			nodeIdx = static_cast<int>(i);
			break;
		}
	}
	if (nodeIdx < 0)
		return;

	// operator chooser vs inline data edit
	const QStringList ops = validOperatorsForNode(nodeIdx); // uses get_listing_opf(...)
	if (!ops.isEmpty()) {
		startOperatorQuickSearch(item, QString());
		return;
	}

	// Fallback to inline edit
	beginItemEdit(item);
}

void sexp_tree::startOperatorQuickSearch(QTreeWidgetItem* item, const QString& seed)
{
	if (!item)
		return;

	// Map item -> node index
	int nodeIdx = -1;
	for (uint i = 0; i < tree_nodes.size(); ++i) {
		if (tree_nodes[i].handle == item) {
			nodeIdx = static_cast<int>(i);
			break;
		}
	}
	if (nodeIdx < 0)
		return;

	// Only allow on editable positions (operator or data) that live beneath a parent
	// (We�ll compute OPF from parent or root as necessary)
	_opAll = validOperatorsForNode(nodeIdx);
	if (_opAll.isEmpty())
		return;

	_opNodeIndex = nodeIdx;

	if (!_opPopup) {
		_opPopup = new QFrame(viewport(), Qt::Popup);
		_opPopup->setFrameShape(QFrame::Box);
		_opPopup->setFrameShadow(QFrame::Plain);
		_opPopup->installEventFilter(this); // <-- important
		auto* layout = new QVBoxLayout(_opPopup);
		layout->setContentsMargins(4, 4, 4, 4);
		_opEdit = new QLineEdit(_opPopup);
		_opList = new QListWidget(_opPopup);
		_opList->setSelectionMode(QAbstractItemView::SingleSelection);
		_opList->setUniformItemSizes(true);
		layout->addWidget(_opEdit);
		layout->addWidget(_opList);
		connect(_opEdit, &QLineEdit::textChanged, this, &sexp_tree::filterOperatorPopup);
		connect(_opEdit, &QLineEdit::returnPressed, [this]() { endOperatorQuickSearch(true); });
		connect(_opList, &QListWidget::itemActivated, [this](QListWidgetItem*) { endOperatorQuickSearch(true); });
		connect(_opList, &QListWidget::itemClicked, [this](QListWidgetItem*) { endOperatorQuickSearch(true); });
	}

	_opList->clear();
	_opList->addItems(_opAll);
	if (!seed.isEmpty()) {
		_opEdit->setText(seed);
		_opEdit->selectAll();
		filterOperatorPopup(seed);
	} else {
		_opEdit->clear();
		if (_opList->count() > 0)
			_opList->setCurrentRow(0);
	}

	// Size the popup: width = widest operator text + scrollbar + padding; height ~10 rows
	QFontMetrics fm(_opList->font());
	int w = 0;
	for (const auto& s : _opAll)
		w = std::max(w, fm.horizontalAdvance(s));
	w += _opList->verticalScrollBar()->sizeHint().width() + 24; // padding
	int rowH = fm.height() + 6;
	int h = (std::min(10, std::max(4, _opList->count())) * rowH) + _opEdit->sizeHint().height() + 12;

	// Place below the item
	QRect itemRect = visualItemRect(item);
	QPoint topLeft = viewport()->mapToGlobal(itemRect.topLeft());
	_opPopup->setGeometry(QRect(topLeft.x(), topLeft.y(), std::max(w, 260), h));
	_opPopup->show();
	_opEdit->setFocus();
	_opPopupActive = true;
}

void sexp_tree::filterOperatorPopup(const QString& text)
{
	_opList->clear();
	if (text.isEmpty()) {
		_opList->addItems(_opAll);
	} else {
		for (const auto& s : _opAll) {
			if (s.contains(text, Qt::CaseInsensitive))
				_opList->addItem(s);
		}
	}
	if (_opList->count() > 0)
		_opList->setCurrentRow(0);
}

void sexp_tree::endOperatorQuickSearch(bool confirm)
{
	if (!_opPopupActive)
		return;

	// Cache before hiding since hide triggers eventFilter which clears state
	const int node = _opNodeIndex;

	QString chosenOp;
	QString typed = (_opEdit ? _opEdit->text().trimmed() : QString());

	if (confirm) {
		// If user selected an operator in the list, prefer that
		if (_opList && _opList->currentItem())
			chosenOp = _opList->currentItem()->text();

		// If nothing selected, see if typed text is a valid *number* for this slot
		if (chosenOp.isEmpty() && !typed.isEmpty()) {
			const int expected = query_node_argument_type(node); // OPF_*
			const bool expectsNumber = (expected == OPF_NUMBER) || (expected == OPF_POSITIVE) ||
									   (expected == OPF_AMBIGUOUS); // allow numerics here too???

			// Accept +/- integers
			static const QRegularExpression kIntRx(QStringLiteral(R"(^[+-]?\d+$)"));
			const bool isInt = kIntRx.match(typed).hasMatch();

			// Enforce positivity if required
			bool okForPositive = true;
			if (expected == OPF_POSITIVE && isInt) {
				okForPositive = typed.toLongLong() > 0;
			}

			if (expectsNumber && isInt && okForPositive) {
				// Commit as NUMBER data
				if (_opPopup)
					_opPopup->hide();
				_opPopupActive = false;
				_opNodeIndex = -1;

				setCurrentItemIndex(node); // sets item_index for replace_data()
				int type = SEXPT_NUMBER | SEXPT_VALID;
				if (tree_nodes[item_index].type & SEXPT_MODIFIER)
					type |= SEXPT_MODIFIER;

				replace_data(typed.toUtf8().constData(), type);
				setFocus(Qt::OtherFocusReason);
				return; // done
			}
		}

		// fall back to closest operator match from typed text
		if (chosenOp.isEmpty() && !typed.isEmpty()) {
			auto best = match_closest_operator(typed.toStdString(), node);
			if (!best.empty())
				chosenOp = QString::fromStdString(best);
		}
	}

	// Close popup and reset state
	if (_opPopup)
		_opPopup->hide();
	_opPopupActive = false;
	_opNodeIndex = -1;

	// Commit operator if we resolved one
	if (confirm && !chosenOp.isEmpty() && node >= 0 && node < static_cast<int>(tree_nodes.size())) {
		setCurrentItemIndex(node);
		const int op_num = get_operator_index(chosenOp.toUtf8().constData());
		if (op_num >= 0) {
			add_or_replace_operator(op_num, /*replace_flag*/ 1);
			if (tree_nodes[node].handle)
				tree_item_handle(tree_nodes[node])->setExpanded(true);
		}
	}

	setFocus(Qt::OtherFocusReason);
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
		SCP_string text = str.toUtf8().constData();
		auto op = match_closest_operator(text, node);
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

		nodeChanged(node);

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
		int new_type = tree_nodes[item_index].type & (~(SEXPT_VARIABLE | SEXPT_CONTAINER_NAME) | SEXPT_CONTAINER_DATA);
		replace_container_data(container, new_type, false, true, !has_modifiers);
		if (has_modifiers) {
			load_branch(Sexp_nodes[Sexp_clipboard].first, item_index);
			int i = tree_nodes[item_index].child;
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
		h = tree_item_handle(tree_nodes[z]);
	} else {
		h = tree_item_handle(tree_nodes[item_index])->parent();
		if (!_interface->getFlags()[TreeFlags::LabeledRoot]) {
			h = nullptr;
			root_item = node;
		} else {
			rootNodeFormulaChanged(item_index, node);
			h->setData(0, FormulaDataRole, node);
		}
	}

	auto item_handle = tree_nodes[node].handle =
						   insert(Operators[op].text.c_str(), NodeImage::OPERATOR, h, tree_item_handle(tree_nodes[item_index]));
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
	beginItemEdit(tree_item_handle(tree_nodes[theNode]));
}
void sexp_tree::addStringDataHandler() {
	int theType = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		theType |= SEXPT_MODIFIER;
	}

	int theNode = add_data("string", theType);
	beginItemEdit(tree_item_handle(tree_nodes[theNode]));
}
void sexp_tree::replaceNumberDataHandler() {
	expand_operator(item_index);
	int type = SEXPT_NUMBER | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	replace_data("number", type);
	beginItemEdit(tree_item_handle(tree_nodes[item_index]));
}
void sexp_tree::replaceStringDataHandler() {
	expand_operator(item_index);
	int type = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	replace_data("string", type);
	beginItemEdit(tree_item_handle(tree_nodes[item_index]));
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
			int i = tree_nodes[item_index].child;
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

	expand_branch(currentItem());
}
void sexp_tree::setCurrentItemIndex(int node) {
	item_index = node;
	if (node < 0) {
		setCurrentItem(nullptr);
	} else {
		setCurrentItem(tree_item_handle(tree_nodes[node]));
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

	auto *handle = tree_item_handle(tree_nodes[item_index]);
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
void sexp_tree::applyVisuals(QTreeWidgetItem* it)
{
	const auto note = it->data(0, NoteRole).toString();
	const auto color = it->data(0, BgColorRole).value<QColor>();
	it->setToolTip(0, note);

	// Background color for the entire row
	if (color.isValid()) {
		it->setBackground(0, QBrush(color));
	}
}
int sexp_tree::getCurrentItemIndex() const {
	return item_index;
}

}
}
