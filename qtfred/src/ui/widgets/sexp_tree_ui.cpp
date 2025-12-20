/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "sexp_tree_ui.h"
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
	expand_branch(static_cast<QTreeWidgetItem*>(handle));
}

// clears out the tree, so all the nodes are unused.
void sexp_tree::clear_tree(const char* op) {
	_model.clear_tree_data(nullptr);
	if (op) {
		clear();  // QTreeWidget::clear()
		if (strlen(op)) {
			_model.set_node(_model.allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
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
	_model.load_tree_data(index, deflt);
	clear();  // QTreeWidget::clear() - clear visual tree
	build_tree();
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

	auto info = _model.compute_node_visual_info(node);
	tree_nodes[node].flags = info.flags;
	tree_nodes[node].handle = insert(tree_nodes[node].text, info.image, root);
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
			auto child_info = _model.compute_node_visual_info(node);
			tree_nodes[node].flags = child_info.flags;
			tree_nodes[node].handle = insert(tree_nodes[node].text, child_info.image, root);

			tree_item_handle(tree_nodes[node])->setFlags(
				tree_item_handle(tree_nodes[node])->flags().setFlag(Qt::ItemIsEditable, (tree_nodes[node].flags & EDITABLE)));
		}

		node = tree_nodes[node].next;
	}
}


// sexp_list_item methods are now in the shared sexp_tree_model.cpp

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


// add an operator under operator pointed to by item_index.  Updates item_index to point
// to this new operator.
int sexp_tree::add_operator(const char* op, QTreeWidgetItem* h) {
	_actions.add_operator(op, static_cast<void*>(h));
	return item_index;
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
	if (source != -1) {
		_model.move_branch_data(source, parent);
		if (parent) {
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
		return;
	}

	// Validate operator help strings
	for (int i = 0; i < (int) Operators.size(); i++) {
		for (int j = 0; j < (int) op_menu.size(); j++) {
			if (get_category(Operators[i].value) == op_menu[j].id) {
				if (!help(Operators[i].value)) {
					mprintf(("Allender!  If you add new sexp operators, add help for them too! :) Sexp %s has no help.\n", Operators[i].text.c_str()));
				}
			}
		}
	}

	// Find node index from handle
	int node_index = -1;
	for (int i = 0; i < (int) tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			node_index = i;
			break;
		}
	}

	// Node comments are not yet implemented in qtFRED, so just adding some base code here
	// that can be used when the feature is completed - Mjn
	SCP_string nodeComment;
	//int thisIndex = event_annotation_lookup(h);
	//if (thisIndex >= 0) {
	//	if (!Event_annotations[thisIndex].comment.empty()) {
	//		nodeComment = "Node Comments:\r\n   " + Event_annotations[thisIndex].comment;
	//	}
	//}

	// Delegate to model for help text computation
	auto result = _model.compute_help_text(node_index, nodeComment);

	helpChanged(QString::fromStdString(result.help_text));
	miniHelpChanged(QString::fromStdString(result.mini_help_text));
}



// Individual OPF listing forwarders, container modifier forwarders, and
// is_node_eligible_for_special_argument have been removed.
// All callers now use _model.get_listing_opf() etc. directly.



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
	int i, j, subcategory_id;

	Assert((int) op_menu.size() < MAX_OP_MENUS);
	Assert((int) op_submenu.size() < MAX_SUBMENUS);

	update_help(h);

	// get item_index
	item_index = -1;
	for (i = 0; i < (int) tree_nodes.size(); i++) {
		if (tree_nodes[i].handle == h) {
			setCurrentItemIndex(i);
			break;
		}
	}

	auto state = _model.compute_context_menu_state(_model.m_mode);

	std::unique_ptr<QMenu> popup_menu(new QMenu(tr("Edit SEXP tree")));

	auto delete_act =
		popup_menu->addAction(tr("&Delete Item"), this, [this]() { deleteActionHandler(); }, QKeySequence::Delete);
	auto edit_data_act = popup_menu->addAction(tr("&Edit Data"), this, [this]() { editDataActionHandler(); });
	popup_menu->addAction(tr("Expand All"), this, [this]() { expand_branch(currentItem()); });

	popup_menu->addSection(tr("Annotations"));
	auto edit_comment_act = popup_menu->addAction(tr("Edit Comment"), this, [this, h]() { editNoteForItem(h); });
	auto edit_color_act = popup_menu->addAction(tr("Edit Color"), this, [this, h]() { editBgColorForItem(h); });
	edit_comment_act->setEnabled(state.can_edit_comment);
	edit_color_act->setEnabled(state.can_edit_bg_color);

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

	// Build variable menu from state
	for (const auto& var : state.replace_variables) {
		char buf[128];
		sprintf(buf, "%s (%s)", Sexp_variables[var.var_index].variable_name, Sexp_variables[var.var_index].text);
		auto action = replace_variable_menu->addAction(QString::fromUtf8(buf),
			this, [this, idx = var.var_index]() { handleReplaceVariableAction(idx); });
		action->setEnabled(var.enabled);
	}

	// Build container name menu from state
	if (state.show_container_names) {
		const auto& containers = get_all_sexp_containers();
		for (int idx = 0; idx < (int)state.replace_container_names.size(); idx++) {
			auto action = replace_container_name_menu->addAction(
				QString::fromStdString(containers[idx].container_name),
				this, [this, idx]() { handleReplaceContainerNameAction(idx); });
			action->setEnabled(state.replace_container_names[idx].enabled);
		}
	}

	// Build container data menu from state
	if (state.show_container_data) {
		const auto& containers = get_all_sexp_containers();
		for (int idx = 0; idx < (int)state.replace_container_data.size(); idx++) {
			auto action = replace_container_data_menu->addAction(
				QString::fromStdString(containers[idx].container_name),
				this, [this, idx]() { handleReplaceContainerDataAction(idx); });
			action->setEnabled(state.replace_container_data[idx].enabled);
		}
	}

	// can't modify if no variables
	modify_variable_act->setEnabled(state.can_modify_variable);

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

	// add operator menu items to the various CATEGORY submenus they belong in,
	// with enabled state from the pre-computed vectors
	for (i = 0; i < (int) Operators.size(); i++) {
		if (SexpTreeModel::is_operator_hidden(Operators[i].value))
			continue;

		bool add_en = state.op_add_enabled[i];
		bool replace_en = state.op_replace_enabled[i];
		bool insert_en = state.op_insert_enabled[i];

		subcategory_id = get_subcategory(Operators[i].value);
		if (subcategory_id == OP_SUBCATEGORY_NONE) {
			for (j = 0; j < (int) op_menu.size(); j++) {
				if (op_menu[j].id == get_category(Operators[i].value)) {
					auto add_act = add_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 0); });
					add_act->setEnabled(add_en);

					auto replace_act = replace_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 1); });
					replace_act->setEnabled(replace_en);

					auto insert_act = insert_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { insertOperatorAction(i); });
					insert_act->setEnabled(insert_en);
					break;
				}
			}
		} else {
			for (j = 0; j < (int) op_submenu.size(); j++) {
				if (op_submenu[j].id == subcategory_id) {
					auto add_act = add_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 0); });
					add_act->setEnabled(add_en);

					auto replace_act = replace_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 1); });
					replace_act->setEnabled(replace_en);

					auto insert_act = insert_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { insertOperatorAction(i); });
					insert_act->setEnabled(insert_en);
					break;
				}
			}
		}
	}

	// special case: item is a ROOT node, and a label that can be edited (not an item in the sexp tree)
	if (state.is_labeled_root) {
		edit_data_act->setEnabled(state.is_root_editable);
		copy_act->setEnabled(false);
		insert_op_menu->setEnabled(false);

		util::propagate_disabled_status(popup_menu.get());
		state.cleanup();
		return popup_menu;
	}

	Assert(item_index != -1);  // handle not found, which should be impossible.
	edit_data_act->setEnabled(state.can_edit_text);
	delete_act->setEnabled(state.can_delete);

	// Set add/replace state from pre-computed values
	m_add_count = state.add_count;
	m_replace_count = state.replace_count;
	m_modify_variable = state.modify_variable;

	add_number_act->setEnabled(state.can_add_number);
	add_string_act->setEnabled(state.can_add_string);
	replace_number_act->setEnabled(state.can_replace_number);
	replace_string_act->setEnabled(state.can_replace_string);

	// Build add data menu items
	if (state.add_data_list) {
		sexp_list_item* ptr = state.add_data_list;
		int data_idx = 0;
		while (ptr) {
			if (ptr->op < 0) {
				add_data_menu->addAction(QString::fromStdString(ptr->text),
					this, [this, data_idx]() { addReplaceTypedDataHandler(data_idx, false); });
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
				replace_data_menu->addAction(QString::fromStdString(ptr->text),
					this, [this, data_idx]() { addReplaceTypedDataHandler(data_idx, true); });
			}
			data_idx++;
			ptr = ptr->next;
		}
	}

	// Clipboard and copy operations
	paste_act->setEnabled(state.can_paste);
	add_paste_act->setEnabled(state.can_paste_add);
	cut_act->setEnabled(state.can_cut);
	copy_act->setEnabled(state.can_copy);

	state.cleanup();
	util::propagate_disabled_status(popup_menu.get());
	return popup_menu;
}
void sexp_tree::cutActionHandler() {
	_actions.clipboard_copy();

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
			_model.free_node2(formulaNode);
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
			_model.free_node2(formulaNode);
		delete item;
		setCurrentItemIndex(-1);
		modified();
		return;
	}

	Assertion(tree_nodes[parent].handle == h_parent, "Tree node handle mismatch!");
	_model.free_node(item_index);
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
	const int argIndex = (parent >= 0) ? _model.find_argument_number(parent, nodeIndex) : 0;

	// Original behavior: compute the OPF type expected at this node
	const int opf = _model.query_node_argument_type(nodeIndex); // handles top-level = OPF_NULL, etc.
	if (opf < 0)
		return out;

	// Build the canonical list for this OPF (this mirrors classic FRED)
	sexp_list_item* list = _model.get_listing_opf(opf, parent, argIndex); // may be nullptr
	for (auto* p = list; p; p = p->next) {
		if (p->op >= 0) {
			const int opIndex = p->op;

			// Optional: keep parity with the menu, which disables ops lacking default args
			if (!_model.query_default_argument_available(opIndex))
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
			const int expected = _model.query_node_argument_type(node); // OPF_*
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

				_actions.replace_data(typed.toUtf8().constData(), type);
				setFocus(Qt::OtherFocusReason);
				return; // done
			}
		}

		// fall back to closest operator match from typed text
		if (chosenOp.isEmpty() && !typed.isEmpty()) {
			auto best = _model.match_closest_operator(typed.toStdString(), node);
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
			_actions.add_or_replace_operator(op_num, /*replace_flag*/ 1);
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
	SCP_string text = str.toUtf8().constData();
	auto result = _model.validate_label_edit(node, text);

	if (result.is_operator) {
		if (!result.update_node && result.resolved_text == text) {
			return;	// Goober5000 - avoids crashing (no match found)
		}

		// use the text of the operator we found
		str = QString::fromStdString(result.resolved_text);
		item->setText(0, str);

		setCurrentItemIndex(node);
		if (result.operator_index >= 0) {
			_actions.add_or_replace_operator(result.operator_index, 1);
		}
	} else if (result.negative_number_error) {
		QMessageBox::critical(this, "Invalid Number", "Can not enter a negative value");
	}

	if (result.update_node) {
		modified();
		nodeChanged(node);
		_model.apply_label_edit(node, result.resolved_text);
	} else {
		auto len = strlen(tree_nodes[node].text);
		item->setText(0, QString::fromUtf8(tree_nodes[node].text, static_cast<int>(len)));
	}
}
void sexp_tree::copyActionHandler() {
	_actions.clipboard_copy();
}
void sexp_tree::pasteActionHandler() {
	_actions.clipboard_paste_replace();
}
void sexp_tree::insertOperatorAction(int op) {
	int flags;

	auto z = tree_nodes[item_index].parent;
	flags = tree_nodes[item_index].flags;
	auto node = _model.allocate_node(z, item_index);
	_model.set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
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

	auto item_handle = static_cast<QTreeWidgetItem*>(tree_nodes[node].handle =
						   insert(Operators[op].text.c_str(), NodeImage::OPERATOR, h, tree_item_handle(tree_nodes[item_index])));
	move_branch(item_index, node);

	setCurrentItemIndex(node);
	for (auto i = 1; i < Operators[op].min; i++) {
		_actions.add_default_operator(op, i);
	}

	item_handle->setExpanded(true);
	modified();
}
void sexp_tree::addNumberDataHandler() {
	int theType = SEXPT_NUMBER | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		theType |= SEXPT_MODIFIER;
	}

	int theNode = _actions.add_data("number", theType);
	beginItemEdit(tree_item_handle(tree_nodes[theNode]));
}
void sexp_tree::addStringDataHandler() {
	int theType = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		theType |= SEXPT_MODIFIER;
	}

	int theNode = _actions.add_data("string", theType);
	beginItemEdit(tree_item_handle(tree_nodes[theNode]));
}
void sexp_tree::replaceNumberDataHandler() {
	_actions.expand_operator(item_index);
	int type = SEXPT_NUMBER | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	_actions.replace_data("number", type);
	beginItemEdit(tree_item_handle(tree_nodes[item_index]));
}
void sexp_tree::replaceStringDataHandler() {
	_actions.expand_operator(item_index);
	int type = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	_actions.replace_data("string", type);
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
		if (replace && m_replace_count == 0) {
			list = _model.get_container_modifiers(op_node);
		} else {
			list = _model.get_container_multidim_modifiers(op_node);
		}
	} else {
		int op = get_operator_index(tree_nodes[op_node].text);
		Assert(op >= 0);
		auto argcount = replace ? m_replace_count : m_add_count;
		auto type = query_operator_argument_type(op, argcount);
		list = _model.get_listing_opf(type, item_index, argcount);
	}
	Assert(list);

	auto ptr = list;
	while (data_idx) {
		data_idx--;
		ptr = ptr->next;
		Assert(ptr);
	}

	Assert((SEXPT_TYPE(ptr->type) != SEXPT_OPERATOR) && (ptr->op < 0));
	_actions.expand_operator(item_index);
	if (replace) {
		_actions.replace_data(ptr->text.c_str(), ptr->type);
	} else {
		_actions.add_data(ptr->text.c_str(), ptr->type);
	}
	list->destroy();
}
void sexp_tree::addPasteActionHandler() {
	_actions.clipboard_paste_add();
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
	if (m_modify_variable || _model.query_node_argument_type(item_index) == OPF_CONTAINER_VALUE) {
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
	_actions.replace_variable_data(id, (type | SEXPT_VARIABLE));

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

	_actions.replace_container_name(containers[idx]);
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
	_actions.replace_container_data(containers[idx], (type | SEXPT_CONTAINER_DATA), true, true, true);

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
