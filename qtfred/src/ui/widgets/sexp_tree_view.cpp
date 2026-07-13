#include "sexp_tree_view.h"
#include "mission/util.h"
#include "mission/Editor.h"
#include "mission/object.h"

#include <ui/util/menu.h>
#include <ui/util/SignalBlockers.h>
#include <ui/dialogs/VariableDialog.h>
#include <ui/Theme.h>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QStyleOptionViewItem>
#include <QKeyEvent>
#include <QShortcut>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QFont>
#include <QHash>
#include <QDebug>

#include <functional>

extern SCP_vector<game_snd> Snds;

//********************sexp_tree_view********************

namespace fso::fred {

namespace {

// True for the numbered-data family: the plain data node plus the legacy DATA_00..DATA_95
// variants. QtFRED ignores the model's fixed every-5th quantization and decides numbering
// itself from the sibling position and the user's "number every N" preference.
bool isDataFamily(NodeImage image) {
	const int v = static_cast<int>(image);
	return image == NodeImage::DATA
		|| (v >= static_cast<int>(NodeImage::DATA_00) && v <= static_cast<int>(NodeImage::DATA_95));
}

// Loads a sexp icon master from the Qt resource system.
QPixmap loadSexpMaster(const char* name) {
	return {QStringLiteral(":/images/sexp_icons/") + QLatin1String(name) + QStringLiteral(".png")};
}

// Shrinks `src` by `factor` and re-centers it on a transparent canvas of the original size,
// so a smaller icon still occupies the same cell as the others.
QPixmap scaleCentered(const QPixmap& src, qreal factor) {
	if (src.isNull() || factor >= 0.999)
		return src;
	const QSize inner(qRound(src.width() * factor), qRound(src.height() * factor));
	const QPixmap scaled = src.scaled(inner, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap out(src.size());
	out.fill(Qt::transparent);
	QPainter p(&out);
	p.drawPixmap((src.width() - scaled.width()) / 2, (src.height() - scaled.height()) / 2, scaled);
	p.end();
	return out;
}

// Draws `number` centered on the data_num badge in red, matching the classic numbered-data look.
void drawBadgeNumber(QPixmap& badge, int number) {
	QPainter p(&badge);
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::TextAntialiasing);
	QFont f = p.font();
	f.setBold(true);
	f.setPixelSize(qRound(badge.height() * 0.40));
	p.setFont(f);
	p.setPen(QColor(200, 40, 40));
	// The white box occupies roughly x[3..43] y[9..39] of the 50x50 master.
	p.drawText(QRectF(3, 9, 40, 30), Qt::AlignCenter, QString::number(number));
	p.end();
}

// Builds the fully-rendered pixmap for a node: colorizes the appropriate shared master
// icon, draws the number badge when applicable, then applies the uniform drop shadow. 
// See the tint helpers in Theme.cpp for the Multiply/Screen rules.
QPixmap renderSexpPixmap(NodeImage image, int number) {
	// The branch/plain-chain color follows the theme text color so it reads on both themes.
	const QColor themeInk = qApp->palette().color(QPalette::WindowText);

	// Cache by image/number/theme. The theme color is part of the key, so a palette change simply produces fresh entries.
	static QHash<QString, QPixmap> cache;
	const QString key = QStringLiteral("%1|%2|%3")
		.arg(static_cast<int>(image)).arg(number).arg(themeInk.rgba());
	const auto cached = cache.constFind(key);
	if (cached != cache.constEnd())
		return cached.value();

	// The plain chain reads as a stark white silhouette in dark mode; soften it to grey there
	// while keeping the near-black look in light mode.
	const bool dark = qApp->palette().color(QPalette::Window).lightness() < 128;
	const QColor blue(60, 110, 255);
	const QColor red(210, 50, 50);
	const QColor green(70, 180, 75);
	const QColor chainInk = dark ? QColor(165, 165, 165) : themeInk;
	const QColor variableRed(255, 120, 120); // Multiply -> reddish page, clearly distinct from plain data

	QPixmap out;
	qreal scale = 1.0;
	if (isDataFamily(image)) {
		if (number > 0) {
			out = loadSexpMaster("data_num");
			drawBadgeNumber(out, number);
		} else {
			// Screen-tinting the grayscale document with black is a no-op, so use it directly.
			out = loadSexpMaster("data");
		}
	} else {
		switch (image) {
		case NodeImage::OPERATOR:        out = loadSexpMaster("operator"); break;        // baked red
		case NodeImage::COMMENT:         out = loadSexpMaster("comment"); break;         // baked
		case NodeImage::CONTAINER_NAME:  out = loadSexpMaster("container_name"); break;  // baked
		case NodeImage::CONTAINER_DATA:  out = loadSexpMaster("container_data"); break;  // baked
		case NodeImage::VARIABLE:        out = tintMultiply(loadSexpMaster("data"), variableRed); break;
		case NodeImage::ROOT:            out = tintMultiply(loadSexpMaster("dot"), blue); scale = 0.65; break;
		case NodeImage::ROOT_DIRECTIVE:  out = tintMultiply(loadSexpMaster("dot"), red); scale = 0.65; break;
		case NodeImage::BLACK_DOT:       out = tintMultiply(loadSexpMaster("dot"), themeInk); scale = 0.65; break;
		case NodeImage::GREEN_DOT:       out = tintMultiply(loadSexpMaster("dot"), green); scale = 0.65; break;
		case NodeImage::CHAIN:           out = tintMultiply(loadSexpMaster("chain"), chainInk); break;
		case NodeImage::CHAIN_DIRECTIVE: out = tintMultiply(loadSexpMaster("chain"), red); break;
		default:                         out = loadSexpMaster("operator"); break;
		}
	}
	out = scaleCentered(out, scale);
	out = applyIconShadow(out);
	cache.insert(key, out);
	return out;
}

// Returns true if the item is a top-level (root) item in the tree widget.
bool isRoot(QTreeWidgetItem* it)
{
	return it && !it->parent();
}
} // namespace

// Builds a QIcon for a NodeImage by colorizing the shared master art (see renderSexpPixmap).
QIcon sexp_tree_view::convertNodeImageToIcon(NodeImage image, int number) {
	return {renderSexpPixmap(image, number)};
}

/**
 * Custom delegate that paints a small comment badge icon after the node text
 * whenever a NoteRole annotation is present on the item. This provides a visual
 * indicator that a node has a user comment without altering the tree structure.
 */
class NoteBadgeDelegate final : public QStyledItemDelegate {
  public:
	explicit NoteBadgeDelegate(sexp_tree_view* tree) : QStyledItemDelegate(tree) {}

	void paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyleOptionViewItem opt(option);
		initStyleOption(&opt, index);

		// draw the standard icon + text first
		const QWidget* w = opt.widget;
		const QStyle* s = w ? w->style() : QApplication::style();
		s->drawControl(QStyle::CE_ItemViewItem, &opt, p, w);

		// if there's a note, paint the badge directly after the text
		const QString note = index.data(sexp_tree_view::NoteRole).toString();
		if (!note.isEmpty()) {
			// where Qt drew the text
			QRect textRect = s->subElementRect(QStyle::SE_ItemViewItemText, &opt, w);

			// compute how much text actually fit
			QFontMetrics fm(opt.font);
			const QString shown = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
			const int textWidth = fm.horizontalAdvance(shown);

			// pick an icon; use your existing mapping
			const QIcon icon = sexp_tree_view::convertNodeImageToIcon(NodeImage::COMMENT);
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

// Constructor: initializes the QTreeWidget with single selection, hidden header, and custom context menu.
// Creates the SexpTreeActions layer with references to _model and *this (as ISexpTreeUI).
// Connects Qt signals: customContextMenuRequested -> customMenuHandler, itemChanged -> handleItemChange,
// itemSelectionChanged -> handleNewItemSelected, itemDoubleClicked -> openNodeEditor.
// Installs the NoteBadgeDelegate for column 0 to paint comment badges.
sexp_tree_view::sexp_tree_view(QWidget* parent) : QTreeWidget(parent), _actions(_model, *this) {
	setSelectionMode(QTreeWidget::SingleSelection);
	setSelectionBehavior(QTreeWidget::SelectItems);

	setContextMenuPolicy(Qt::CustomContextMenu);

	setHeaderHidden(true);

	// Editing is always initiated explicitly (Space or the context menu's Edit Data),
	// so disable Qt's automatic edit triggers. Otherwise a double-click on an editable
	// item would start an inline edit and fight with the expand-on-double-click below.
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	// We toggle expansion ourselves in the itemDoubleClicked handler, so turn off Qt's
	// built-in double-click expansion to avoid toggling twice.
	setExpandsOnDoubleClick(false);

	select_sexp_node = -1;
	root_item = -1;
	clear_tree();

	connect(this, &QWidget::customContextMenuRequested, this, &sexp_tree_view::customMenuHandler);
	connect(this, &QTreeWidget::itemChanged, this, &sexp_tree_view::handleItemChange);
	connect(this, &QTreeWidget::itemSelectionChanged, this, &sexp_tree_view::handleNewItemSelected);
	// Double-clicking a node toggles its expansion when it has children; a leaf node
	// has nothing to expand, so double-clicking it does nothing.
	connect(this, &QTreeWidget::itemDoubleClicked, this, [](QTreeWidgetItem* item, int /*column*/) {
		if (item && item->childCount() > 0) {
			item->setExpanded(!item->isExpanded());
		}
	});

	setItemDelegateForColumn(0, new NoteBadgeDelegate(this));

	// Persistent keyboard shortcuts for Cut/Copy/Paste/Add-Paste/Delete on tree nodes.
	// Each gate-checks compute_context_menu_state() so we never invoke an action that
	// is invalid for the current selection (which would otherwise hit an Assertion or
	// produce a malformed sexp — see issue 4405).
	installShortcut(QKeySequence::Cut,
		[](const SexpContextMenuState& s) { return s.can_cut; },
		[this]() { cutActionHandler(); });
	installShortcut(QKeySequence::Copy,
		[](const SexpContextMenuState& s) { return s.can_copy; },
		[this]() { copyActionHandler(); });
	// Mapping matches FRED2 (fred.rc): Ctrl+V = add as child, Ctrl+Shift+V = overwrite.
	// The destructive replace is on the modified combo so it can't be triggered accidentally.
	installShortcut(QKeySequence::Paste,
		[](const SexpContextMenuState& s) { return s.can_paste_add; },
		[this]() { addPasteActionHandler(); });
	installShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V),
		[](const SexpContextMenuState& s) { return s.can_paste; },
		[this]() { pasteActionHandler(); });
	installShortcut(QKeySequence::Delete,
		[](const SexpContextMenuState& s) { return s.can_delete; },
		[this]() { deleteActionHandler(); });
}


sexp_tree_view::~sexp_tree_view() = default;

// --- ISexpTreeUI implementation ---
// These callbacks are invoked by SexpTreeActions to manipulate the Qt widget.
// Each casts void* handles to QTreeWidgetItem* and delegates to Qt operations.

// Creates a new QTreeWidgetItem with the given text and icon. Called by _actions when adding nodes.
void* sexp_tree_view::ui_insert_item(const char* text, NodeImage image, void* parent_handle, void* insert_after)
{
	auto* hParent = static_cast<QTreeWidgetItem*>(parent_handle);
	auto* hAfter = static_cast<QTreeWidgetItem*>(insert_after);
	auto* item = insert(QString::fromUtf8(text), image, hParent, hAfter);
	// Inserting a data node can shift the argument positions of its siblings, so renumber them.
	if (isDataFamily(image) && hParent)
		refreshDataNumbers(hParent);
	return static_cast<void*>(item);
}

// Deletes a QTreeWidgetItem from the tree. Called by _actions when removing nodes.
void sexp_tree_view::ui_delete_item(void* handle)
{
	auto* item = static_cast<QTreeWidgetItem*>(handle);
	QTreeWidgetItem* parent = item->parent();
	delete item;
	// Removing a data node shifts the argument positions of its siblings, so renumber them.
	if (parent)
		refreshDataNumbers(parent);
}

// Sets the display text on a QTreeWidgetItem. Called by _actions after data changes.
void sexp_tree_view::ui_set_item_text(void* handle, const char* text)
{
	static_cast<QTreeWidgetItem*>(handle)->setText(0, QString::fromUtf8(text));
}

// Sets the icon on a QTreeWidgetItem. Called by _actions after type/image changes.
void sexp_tree_view::ui_set_item_image(void* handle, NodeImage image)
{
	applyNodeIcon(static_cast<QTreeWidgetItem*>(handle), image);
}

// Returns the first child QTreeWidgetItem, or nullptr. Called by _actions to traverse the tree.
void* sexp_tree_view::ui_get_child_item(void* handle) const
{
	auto* item = static_cast<QTreeWidgetItem*>(handle);
	if (item->childCount() > 0)
		return static_cast<void*>(item->child(0));
	return nullptr;
}

// Returns true if the item has children. Called by _actions to check tree structure.
bool sexp_tree_view::ui_has_children(void* handle) const
{
	return static_cast<QTreeWidgetItem*>(handle)->childCount() > 0;
}

// Expands a single item. Called by _actions after inserting children.
void sexp_tree_view::ui_expand_item(void* handle)
{
	static_cast<QTreeWidgetItem*>(handle)->setExpanded(true);
}

// Makes an item the current selection. Called by _actions after add/replace operations.
void sexp_tree_view::ui_select_item(void* handle)
{
	setCurrentItem(static_cast<QTreeWidgetItem*>(handle));
}

// Scrolls the tree view so the item is visible. Called by _actions.
void sexp_tree_view::ui_ensure_visible(void* handle)
{
	scrollToItem(static_cast<QTreeWidgetItem*>(handle));
}

// Emits the modified() signal. Called by _actions after any tree mutation.
void sexp_tree_view::ui_notify_modified()
{
	modified();
}

// Refreshes the help panel for the given item. Called by _actions after selection changes.
void sexp_tree_view::ui_update_help(void* handle)
{
	update_help(static_cast<QTreeWidgetItem*>(handle));
}

// Creates Qt child items for all model children of a given parent node.
// Walks tree_nodes[parent].child -> next chain, calling add_sub_tree() for each.
// Called by _actions when expanding a previously-collapsed container data node.
void sexp_tree_view::ui_add_children_visual(int parent_node_index)
{
	int i = tree_nodes[parent_node_index].child;
	while (i != -1) {
		add_sub_tree(i, tree_item_handle(tree_nodes[parent_node_index]));
		i = tree_nodes[i].next;
	}
}

void sexp_tree_view::ui_move_branch(int source_node, int parent_node)
{
	move_branch(source_node, parent_node);
}

// Recursively expands a branch. Called by _actions after container data replacement.
void sexp_tree_view::ui_expand_branch(void* handle)
{
	expand_branch(static_cast<QTreeWidgetItem*>(handle));
}

// Clears all model node data via _model.clear_tree_data(). If op is non-null, also clears the
// Qt widget. If op is non-empty, allocates a root operator node via _model.allocate_node()/set_node()
// and rebuilds the visual tree.
void sexp_tree_view::clear_tree(const char* op) {
	_model.clear_tree_data(nullptr);
	if (op) {
		clear();  // QTreeWidget::clear()
		if (strlen(op)) {
			_model.set_node(_model.allocate_node(-1), (SEXPT_OPERATOR | SEXPT_VALID), op);
			build_tree();
		}
	}
}

// Nulls out all tree_nodes[].handle pointers. Called before a visual rebuild to ensure
// stale Qt item pointers are not dereferenced.
void sexp_tree_view::reset_handles() {
	uint i;

	for (i = 0; i < tree_nodes.size(); i++) {
		tree_nodes[i].handle = nullptr;
	}
}

// Loads sexp data from the game's Sexp_nodes[] array into the model via _model.load_tree_data(),
// then clears the Qt widget and rebuilds the visual tree. The index parameter is the starting
// sexp node index; deflt is the default expression text if index is -1.
void sexp_tree_view::load_tree(int index, const char* deflt) {
	_model.load_tree_data(index, deflt);
	clear();  // QTreeWidget::clear() - clear visual tree
	build_tree();
}


// Clears the Qt widget and rebuilds the entire visual tree from tree_nodes[] starting at node 0.
// Calls add_sub_tree() recursively. No model dependency beyond reading tree_nodes[].
void sexp_tree_view::build_tree() {
	if (!flag) {
		select_sexp_node = -1;
	}

	clear();
	add_sub_tree(0, nullptr);
}

// Recursively creates QTreeWidgetItems for a model node and all its children/siblings.
// For each node: calls _model.compute_node_visual_info() to determine icon and flags,
// creates the Qt item via insert(), sets the editable flag, then recurses into children.
// Operator and container-data nodes recurse via add_sub_tree(); leaf data nodes are created directly.
void sexp_tree_view::add_sub_tree(int node, QTreeWidgetItem* root) {
	int node2;

	Assertion(SCP_vector_inbounds(tree_nodes, node), "Invalid node index");
	node2 = tree_nodes[node].child;

	auto info = _model.compute_node_visual_info(node);
	tree_nodes[node].flags = info.flags;
	tree_nodes[node].handle = insert(tree_nodes[node].text, info.image, root);
	root = tree_item_handle(tree_nodes[node]);

	tree_item_handle(tree_nodes[node])->setFlags(
		tree_item_handle(tree_nodes[node])->flags().setFlag(Qt::ItemIsEditable, (tree_nodes[node].flags & EDITABLE)));

	node = node2;
	while (node != -1) {
		Assertion(SCP_vector_inbounds(tree_nodes, node), "Invalid node index");
		Assertion(tree_nodes[node].type & SEXPT_VALID, "Invalid node type");
		if (tree_nodes[node].type & (SEXPT_OPERATOR | SEXPT_CONTAINER_DATA)) {
			add_sub_tree(node, root);
		} else {
			Assertion(tree_nodes[node].child == -1, "Invalid child node");
			auto child_info = _model.compute_node_visual_info(node);
			tree_nodes[node].flags = child_info.flags;
			tree_nodes[node].handle = insert(tree_nodes[node].text, child_info.image, root);

			tree_item_handle(tree_nodes[node])->setFlags(
				tree_item_handle(tree_nodes[node])->flags().setFlag(Qt::ItemIsEditable, (tree_nodes[node].flags & EDITABLE)));
		}

		node = tree_nodes[node].next;
	}
}

// Recursively expands a QTreeWidgetItem and all its children. Pure Qt operation.
void sexp_tree_view::expand_branch(QTreeWidgetItem* h) {
	h->setExpanded(true);
	for (auto i = 0; i < h->childCount(); ++i) {
		expand_branch(h->child(i));
	}
}

// Opens a QInputDialog for editing the comment annotation (NoteRole) on a tree item.
// Stores the result in the item's NoteRole data, refreshes visuals, and emits nodeAnnotationChanged().
// Pure UI operation - no model dependency.
void sexp_tree_view::editNoteForItem(QTreeWidgetItem* it)
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

// Opens a QColorDialog for choosing a background highlight color (BgColorRole) on a tree item.
// Stores the result in the item's BgColorRole data, refreshes visuals, and emits nodeBgColorChanged().
// Pure UI operation - no model dependency.
void sexp_tree_view::editBgColorForItem(QTreeWidgetItem* it)
{
	const auto start = it->data(0, BgColorRole).value<QColor>();
	const QColor c = QColorDialog::getColor(start.isValid() ? start : Qt::yellow, this, tr("Choose Background Color"));
	if (!c.isValid())
		return;
	it->setData(0, BgColorRole, c);
	applyVisuals(it);

	Q_EMIT nodeBgColorChanged(static_cast<void*>(it), c);
}


// Adds an operator node under the current position. Delegates entirely to _actions.add_operator()
// which handles model allocation, node setup, Qt item creation (via ISexpTreeUI callbacks),
// and adding default child arguments. Returns the new item_index.
int sexp_tree_view::add_operator(const char* op, QTreeWidgetItem* h) {
	_actions.add_operator(op, static_cast<void*>(h));
	return item_index;
}

// Displays an error dialog for a given node. Selects and scrolls to the error node,
// shows a QMessageBox::critical asking whether to continue checking for errors.
// Returns -1 if user declines, 0 otherwise. Pure UI operation.
int sexp_tree_view::node_error(int node, const char* msg, int* bypass) {
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

// Selects and scrolls to a node in the tree. Expands ancestors via ensure_visible(),
// clears existing selection, and makes the node the current item. Pure UI operation.
void sexp_tree_view::hilite_item(int node) {

	ensure_visible(node);
	clearSelection();
	setCurrentItem(tree_item_handle(tree_nodes[node]));
	scrollToItem(tree_item_handle(tree_nodes[node]));
}

// Expands all ancestor items of the given node so it becomes visible in the tree.
// Walks up the Qt parent chain calling setExpanded(). Pure UI operation.
void sexp_tree_view::ensure_visible(int node) {
	auto handle = tree_item_handle(tree_nodes[node])->parent();

	while (handle != nullptr) {
		handle->setExpanded(true);
		handle = handle->parent();
	}
}

// Moves a branch in both model data and the Qt tree. First calls _model.move_branch_data()
// to update the parent/child/next linkage, then calls the QTreeWidgetItem overload of
// move_branch() to recreate the visual subtree under the new parent.
void sexp_tree_view::move_branch(int source, int parent) {
	if (source != -1) {
		Assertion(parent > -1, "move_branch called with negative parent index %d (source %d)", parent, source);
		_model.move_branch_data(source, parent);
		move_branch(tree_item_handle(tree_nodes[source]), tree_item_handle(tree_nodes[parent]));
	}
}

// Recursively re-creates a Qt subtree under a new parent/after position. For each item:
// finds its tree_nodes[] slot to update the handle, creates a new QTreeWidgetItem via insertWithIcon(),
// copies all custom data roles (FormulaDataRole, NoteRole, BgColorRole), applies visuals,
// recursively moves all children, preserves expansion state, then deletes the old source item.
QTreeWidgetItem* sexp_tree_view::move_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent, QTreeWidgetItem* after)
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

// Recursively copies a Qt subtree under a new parent without removing the source.
// Creates new items via insertWithIcon(), copies all custom data roles, applies visuals,
// and preserves expansion state. If the source maps to a model node, updates that node's
// handle to point at the newly created copy for parity with legacy FRED2 behavior.
void sexp_tree_view::copy_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent, QTreeWidgetItem* after)
{
	if (!source)
		return;

	const auto icon = source->icon(0);
	QTreeWidgetItem* h = insertWithIcon(source->text(0), icon, parent, after);
	size_t idx = 0;
	for (; idx < tree_nodes.size(); ++idx) {
		if (tree_nodes[idx].handle == source) {
			break;
		}
	}
	if (idx < tree_nodes.size()) {
		tree_nodes[idx].handle = h;
	}

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

// Reorders top-level (root) items by removing source from its position and reinserting
// relative to dest. If insert_before is true, places before dest; otherwise after.
// Emits rootOrderChanged() and modified(). Pure Qt widget operation.
void sexp_tree_view::move_root(QTreeWidgetItem* source, QTreeWidgetItem* dest, bool insert_before)
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

// Thin wrapper: converts a NodeImage enum to a QIcon and delegates to insertWithIcon().
QTreeWidgetItem* sexp_tree_view::insert(const QString& lpszItem, NodeImage image, QTreeWidgetItem* hParent, QTreeWidgetItem* hInsertAfter) {
	// Create the item first, then icon it: the numbered-data badge depends on the item's
	// final sibling position, which only exists once it is inserted into the tree.
	auto* item = insertWithIcon(lpszItem, QIcon(), hParent, hInsertAfter);
	applyNodeIcon(item, image);
	return item;
}

int sexp_tree_view::numberEveryN() const {
	return _viewport ? _viewport->sexp_number_every_n : 5;
}

void sexp_tree_view::applyNodeIcon(QTreeWidgetItem* item, NodeImage image) {
	if (!item)
		return;

	// Remember the base image so refreshDataNumbers() can renumber siblings without a model lookup.
	item->setData(0, NodeImageRole, static_cast<int>(image));

	int number = -1;
	if (isDataFamily(image)) {
		const int n = numberEveryN();
		if (n > 0) {
			QTreeWidgetItem* parent = item->parent();
			const int pos = (parent ? parent->indexOfChild(item) : indexOfTopLevelItem(item)) + 1;
			if (pos > 0 && (pos % n) == 0)
				number = pos;
		}
	}
	item->setIcon(0, convertNodeImageToIcon(image, number));
}

void sexp_tree_view::refreshDataNumbers(QTreeWidgetItem* parent) {
	if (!parent)
		return;
	for (int i = 0; i < parent->childCount(); ++i) {
		QTreeWidgetItem* child = parent->child(i);
		const QVariant stored = child->data(0, NodeImageRole);
		if (!stored.isValid())
			continue;
		const auto image = static_cast<NodeImage>(stored.toInt());
		if (isDataFamily(image))
			applyNodeIcon(child, image);
	}
}

// Handles keyboard input. When the operator quick-search popup is active, routes keys:
// Escape -> cancel popup, Enter -> confirm selection, Up/Down/Page/Home/End -> forwarded to list,
// other keys -> forwarded to the filter text field.
// When no popup: Space opens the node editor (popup or inline edit) for the current item.
// All other keys delegate to QTreeWidget default handling.
void sexp_tree_view::keyPressEvent(QKeyEvent* e)
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

	// Space enters "Edit Data" mode on the selected node. For editable data nodes this
	// starts inline text editing. Operator nodes have no editable data of their own, 
	// so they fall back to the operator quick-search popup.
	if (e->key() == Qt::Key_Space && currentItem()) {
		item_index = get_node(currentItem());
		if (_model.compute_context_menu_state().can_edit_text) {
			editDataActionHandler();
		} else {
			openNodeEditor(currentItem());
		}
		return;
	}

	QTreeWidget::keyPressEvent(e);
}

// Event filter installed on the operator popup frame. Clears popup state (active flag, node index)
// and returns focus to the tree when the popup is hidden, closed, or loses focus.
bool sexp_tree_view::eventFilter(QObject* obj, QEvent* ev)
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

// Re-render all node icons on a palette change. The render cache keys on the theme
// color, so convertNodeImageToIcon() naturally produces fresh pixmaps for the new theme.
void sexp_tree_view::changeEvent(QEvent* e)
{
	QTreeWidget::changeEvent(e);
	if (e->type() == QEvent::PaletteChange)
		refreshAllIcons();
}

void sexp_tree_view::refreshAllIcons()
{
	std::function<void(QTreeWidgetItem*)> walk = [&](QTreeWidgetItem* it) {
		const QVariant stored = it->data(0, NodeImageRole);
		if (stored.isValid())
			applyNodeIcon(it, static_cast<NodeImage>(stored.toInt()));
		for (int i = 0; i < it->childCount(); ++i)
			walk(it->child(i));
	};
	for (int i = 0; i < topLevelItemCount(); ++i)
		walk(topLevelItem(i));
}

void sexp_tree_view::refreshAllInstances()
{
	const auto widgets = QApplication::allWidgets();
	for (QWidget* w : widgets) {
		if (auto* tree = qobject_cast<sexp_tree_view*>(w))
			tree->refreshAllIcons();
	}
}

// Records drag start position and source item for root-level drag-and-drop. Only root items
// (no parent) can be dragged. Resets drag state and delegates to QTreeWidget.
void sexp_tree_view::mousePressEvent(QMouseEvent* e)
{
	_dragStartPos = e->pos();
	_dragSourceRoot = itemAt(e->pos());
	if (!isRoot(_dragSourceRoot))
		_dragSourceRoot = nullptr; // roots only
	_dragging = false;
	QTreeWidget::mousePressEvent(e);
}

// Tracks mouse movement during a drag. Once past the drag threshold, highlights potential
// drop targets (other root items) by selecting them as a visual cue. Only root-to-root
// dragging is supported. No QDrag payload is used; the actual move happens on mouse release.
void sexp_tree_view::mouseMoveEvent(QMouseEvent* e)
{
	if (!_dragSourceRoot) {
		QTreeWidget::mouseMoveEvent(e);
		return;
	}
	if (!(e->buttons() & Qt::LeftButton)) {
		QTreeWidget::mouseMoveEvent(e);
		return;
	}
	const int dist = (e->pos() - _dragStartPos).manhattanLength();
	if (!_dragging && dist < QApplication::startDragDistance()) {
		QTreeWidget::mouseMoveEvent(e);
		return;
	}

	// Dragging - we just highlight potential drop target (a root under the cursor)
	_dragging = true;
	if (auto* over = itemAt(e->pos())) {
		if (isRoot(over))
			setCurrentItem(over); // simple visual cue like OG�s SelectDropTarget
	}

	// No QDrag payload; we�ll do the move on mouse release to keep logic simple.
	QTreeWidget::mouseMoveEvent(e);
}

// Completes a root-level drag-and-drop. If the drag source and drop target are different root items,
// determines insert_before based on relative position and calls move_root() to reorder.
// Clears drag state and delegates to QTreeWidget.
void sexp_tree_view::mouseReleaseEvent(QMouseEvent* e)
{
	if (_dragging && _dragSourceRoot) {
		auto* dropTarget = itemAt(e->pos());
		if (dropTarget && isRoot(dropTarget) && dropTarget != _dragSourceRoot) {
			// OG rule: if moving up, insert_before=true; if moving down, insert_after
			// (so we end up where we dropped). :contentReference[oaicite:1]{index=1}
			const int srcIdx = indexOfTopLevelItem(_dragSourceRoot);
			const int dstIdx = indexOfTopLevelItem(dropTarget);
			const bool insert_before = (srcIdx > dstIdx);

			// Perform the visual move
			move_root(_dragSourceRoot, dropTarget, insert_before);
		}
	}
	_dragSourceRoot = nullptr;
	_dragging = false;
	QTreeWidget::mouseReleaseEvent(e);
}

// Core Qt item creation function. Creates a QTreeWidgetItem with text, icon, and editable flag
// under the specified parent (or as top-level) after the specified sibling. Blocks all Qt signals
// during creation to prevent spurious itemChanged callbacks.
QTreeWidgetItem* sexp_tree_view::insertWithIcon(const QString& lpszItem, const QIcon& image, QTreeWidgetItem* hParent, QTreeWidgetItem* hInsertAfter) {
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

// Returns the QTreeWidgetItem* handle for a given tree_nodes[] index.
QTreeWidgetItem* sexp_tree_view::handle(int node) const {
	return tree_item_handle(tree_nodes[node]);
}

// Returns the help string for a given operator code. Delegates to SexpTreeModel::help() (static).
const char* sexp_tree_view::help(int code) {
	return SexpTreeModel::help(code);
}

// Returns the sexp type flags (SEXPT_OPERATOR, SEXPT_STRING, etc.) for the node matching
// handle h. Performs a linear scan of tree_nodes[] to find the matching handle.
int sexp_tree_view::get_type(QTreeWidgetItem* h) const {
	uint i;

	// get index into sexp_tree_view
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

// Returns the tree_nodes[] index for the node matching handle h.
// Performs a linear scan of tree_nodes[]. Returns -1 if not found.
int sexp_tree_view::get_node(QTreeWidgetItem* h) const {
	uint i;

	// get index into sexp_tree_view
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

// Walks parent links from the given node up to find and return the root node index.
int sexp_tree_view::get_root(int node) const {
	while (tree_nodes[node].parent >= 0) {
		node = tree_nodes[node].parent;
	}
	return node;
}

// Updates the help text panel for the selected item. Finds the node index from the handle,
// then delegates to _model.compute_help_text() which returns both the full help text and
// the mini (one-line) help text. Emits helpChanged() and miniHelpChanged() signals.
void sexp_tree_view::update_help(QTreeWidgetItem* h) {
	if (h == nullptr) {
		helpChanged("");
		miniHelpChanged("");
		return;
	}

	// Validate operator help strings (once per session)
	static bool help_validated = false;
	if (!help_validated) {
		help_validated = true;
		for (const auto& oper : Operators) {
			for (const auto& menu : op_menu) {
				if (get_category(oper.value) == menu.id) {
					if (!help(oper.value)) {
						mprintf(("If you add new sexp operators, add help for them too, pilot! :) Sexp %s has no help. (Now go read a book!)\n", oper.text.c_str()));
					}
				}
			}
		}
	}

	// Find node index from handle
	int node_index = -1;
	for (int i = 0; i < static_cast<int>(tree_nodes.size()); i++) {
		if (tree_nodes[i].handle == h) {
			node_index = i;
			break;
		}
	}

	// Node comments are not yet implemented in qtFRED, so just adding some base code here
	// that can be used when the feature is completed - Mjn
	SCP_string nodeComment;

	// Delegate to model for help text computation
	auto result = _model.compute_help_text(node_index, nodeComment);

	helpChanged(QString::fromStdString(result.help_text));
	miniHelpChanged(QString::fromStdString(result.mini_help_text));
}

// Stores the Editor and SexpTreeEditorInterface pointers. If no custom interface is provided,
// creates a default SexpTreeEditorInterface. The interface controls tree behavior flags
// (e.g. RootDeletable, RootEditable, LabeledRoot).
void sexp_tree_view::initializeEditor(::fso::fred::Editor* edit, SexpTreeEditorInterface* editorInterface, EditorViewport* viewport) {
	if (editorInterface == nullptr) {
		// If there is no special interface then we supply the default implementation
		_owned_interface.reset(new SexpTreeEditorInterface());
		editorInterface = _owned_interface.get();
	}

	_editor = edit;
	_interface = editorInterface;
	_viewport = viewport;
}

// Slot connected to customContextMenuRequested. Gets the QTreeWidgetItem at the click position,
// builds the full context menu via buildContextMenu(), and executes it at the global position.
void sexp_tree_view::customMenuHandler(const QPoint& pos) {
	QTreeWidgetItem* h = this->itemAt(pos);

	if (h == nullptr) {
		return;
	}



	auto menu = buildContextMenu(h);

	menu->exec(mapToGlobal(pos));
}

// Builds the complete right-click context menu for the given tree item.
//
// First calls _model.compute_context_menu_state() to get all enabled/disabled states, then
// constructs the full menu hierarchy:
//   - Delete, Edit Data, Expand All
//   - Annotations: Edit Comment, Edit Color
//   - Copy operations: Cut, Copy, Paste
//   - Add Operator (categorized submenus by op_menu[]/op_submenu[])
//   - Add Data: Number, String, plus typed items from state.add_data_list
//   - Add Paste
//   - Insert Operator (categorized submenus)
//   - Replace Operator (categorized submenus)
//   - Replace Data: Number, String, plus typed items from state.replace_data_list
//   - Variables: Add Variable, Modify Variable, Replace Variable (from state.replace_variables)
//   - Containers: Add/Modify Container, Replace Container Name/Data
//
// Operator add/replace/insert actions connect to _actions.add_or_replace_operator() or insertOperatorAction().
// Data actions connect to local handlers (addNumberDataHandler, addReplaceTypedDataHandler, etc.).
// Per-operator enabled state comes from state.op_add_enabled[], state.op_replace_enabled[],
// state.op_insert_enabled[] arrays computed by the model.
std::unique_ptr<QMenu> sexp_tree_view::buildContextMenu(QTreeWidgetItem* h) {
	int i, j, subcategory_id;

	Assertion(static_cast<int>(op_menu.size()) < SEXP_TREE_MAX_OP_MENUS, "Operator menu too large!");
	Assertion(static_cast<int>(op_submenu.size()) < SEXP_TREE_MAX_SUBMENUS, "Operator submenu too large!");

	update_help(h);

	// get item_index — direct assignment like FRED2's update_item(), no signal cascade.
	// setCurrentItem(h) is deferred until after menu state is computed (see below).
	item_index = -1;
	for (i = 0; i < static_cast<int>(tree_nodes.size()); i++) {
		if (tree_nodes[i].handle == h) {
			item_index = i;
			break;
		}
	}

	auto state = _model.compute_context_menu_state();

	std::unique_ptr<QMenu> popup_menu(new QMenu(tr("Edit SEXP tree")));

	auto delete_act =
		popup_menu->addAction(tr("&Delete Item"), QKeySequence::Delete, this, [this]() { deleteActionHandler(); });
	auto edit_data_act = popup_menu->addAction(tr("&Edit Data"), this, [this]() { editDataActionHandler(); });
	popup_menu->addAction(tr("Expand All"), this, [this]() { expand_branch(currentItem()); });

	popup_menu->addSection(tr("Annotations"));
	auto edit_comment_act = popup_menu->addAction(tr("Edit Comment"), this, [this, h]() { editNoteForItem(h); });
	auto edit_color_act = popup_menu->addAction(tr("Edit Color"), this, [this, h]() { editBgColorForItem(h); });
	edit_comment_act->setEnabled(state.can_edit_comment);
	edit_color_act->setEnabled(state.can_edit_bg_color);

	popup_menu->addSection(tr("Copy operations"));
	auto cut_act = popup_menu->addAction(tr("Cut"), QKeySequence::Cut, this, [this]() { cutActionHandler(); });
	cut_act->setEnabled(false);
	auto copy_act = popup_menu->addAction(tr("Copy"), QKeySequence::Copy, this, [this]() { copyActionHandler(); });
	auto paste_act = popup_menu->addAction(tr("Paste (Overwrite)"),
		QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), this,
		[this]() { pasteActionHandler(); });
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
	auto add_paste_act =
		popup_menu->addAction(tr("Paste (Add Child)"), QKeySequence::Paste, this, [this]() { addPasteActionHandler(); });
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

	auto modify_variable_act = popup_menu->addAction(tr("Add/Modify Variable"), this, [this]() {
		if (_viewport) {
			auto* dlg = new dialogs::VariableDialog(this, _viewport, dialogs::VariableDialog::VariablesTab);
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->show();
		}
	});

	auto replace_variable_menu = popup_menu->addMenu(tr("Replace Variable"));
	popup_menu->addSeparator();

	popup_menu->addSection("Containers");

	auto add_modify_container_act = popup_menu->addAction(tr("Add/Modify Container"), this, [this]() {
		if (_viewport) {
			auto* dlg = new dialogs::VariableDialog(this, _viewport, dialogs::VariableDialog::ContainersTab);
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->show();
		}
	});
	add_modify_container_act->setEnabled(_viewport != nullptr);
	auto replace_container_name_menu = popup_menu->addMenu(tr("Replace Container Name"));
	auto replace_container_data_menu = popup_menu->addMenu(tr("Replace Container Data"));

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
		for (int idx = 0; idx < static_cast<int>(state.replace_container_names.size()); idx++) {
			auto action = replace_container_name_menu->addAction(
				QString::fromStdString(containers[idx].container_name),
				this, [this, idx]() { handleReplaceContainerNameAction(idx); });
			action->setEnabled(state.replace_container_names[idx].enabled);
		}
	}

	// Build container data menu from state
	if (state.show_container_data) {
		const auto& containers = get_all_sexp_containers();
		for (int idx = 0; idx < static_cast<int>(state.replace_container_data.size()); idx++) {
			auto action = replace_container_data_menu->addAction(
				QString::fromStdString(containers[idx].container_name),
				this, [this, idx]() { handleReplaceContainerDataAction(idx); });
			action->setEnabled(state.replace_container_data[idx].enabled);
		}
	}

	modify_variable_act->setEnabled(_viewport != nullptr);

	// add popup menus for all the operator categories
	QMenu* add_op_submenu[SEXP_TREE_MAX_OP_MENUS];
	QMenu* replace_op_submenu[SEXP_TREE_MAX_OP_MENUS];
	QMenu* insert_op_submenu[SEXP_TREE_MAX_OP_MENUS];
	for (i = 0; i < static_cast<int>(op_menu.size()); i++) {
		add_op_submenu[i] = add_op_menu->addMenu(QString::fromStdString(op_menu[i].name));
		replace_op_submenu[i] = replace_op_menu->addMenu(QString::fromStdString(op_menu[i].name));
		insert_op_submenu[i] = insert_op_menu->addMenu(QString::fromStdString(op_menu[i].name));
	}

	// add all the submenu items first
	QMenu* add_op_subcategory_menu[SEXP_TREE_MAX_SUBMENUS];
	QMenu* replace_op_subcategory_menu[SEXP_TREE_MAX_SUBMENUS];
	QMenu* insert_op_subcategory_menu[SEXP_TREE_MAX_SUBMENUS];
	for (i = 0; i < static_cast<int>(op_submenu.size()); i++) {
		for (j = 0; j < static_cast<int>(op_menu.size()); j++) {
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
	for (i = 0; i < static_cast<int>(Operators.size()); i++) {
		if (SexpTreeModel::is_operator_hidden(Operators[i].value))
			continue;

		bool add_en = state.op_add_enabled[i];
		bool replace_en = state.op_replace_enabled[i];
		bool insert_en = state.op_insert_enabled[i];

		subcategory_id = get_subcategory(Operators[i].value);
		if (subcategory_id == OP_SUBCATEGORY_NONE) {
			for (j = 0; j < static_cast<int>(op_menu.size()); j++) {
				if (op_menu[j].id == get_category(Operators[i].value)) {
					auto add_act = add_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 0); Q_EMIT modified(); });
					add_act->setEnabled(add_en);

					auto replace_act = replace_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 1); Q_EMIT modified(); });
					replace_act->setEnabled(replace_en);

					auto insert_act = insert_op_submenu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { insertOperatorAction(i); });
					insert_act->setEnabled(insert_en);
					break;
				}
			}
		} else {
			for (j = 0; j < static_cast<int>(op_submenu.size()); j++) {
				if (op_submenu[j].id == subcategory_id) {
					auto add_act = add_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 0); Q_EMIT modified(); });
					add_act->setEnabled(add_en);

					auto replace_act = replace_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { _actions.add_or_replace_operator(i, 1); Q_EMIT modified(); });
					replace_act->setEnabled(replace_en);

					auto insert_act = insert_op_subcategory_menu[j]->addAction(QString::fromStdString(Operators[i].text),
						this, [this, i]() { insertOperatorAction(i); });
					insert_act->setEnabled(insert_en);
					break;
				}
			}
		}
	}

	// Now visually select the item and fire selection-change signals, matching FRED2's pattern
	// of calling SelectItem(h) + update_item(h) AFTER building menu state (not before).
	setCurrentItem(h);
	// Re-sync item_index in case the selection signal changed it (mirrors FRED2's update_item after SelectItem)
	item_index = -1;
	for (i = 0; i < static_cast<int>(tree_nodes.size()); i++) {
		if (tree_nodes[i].handle == h) {
			item_index = i;
			break;
		}
	}

	// special case: item is a ROOT node, and a label that can be edited (not an item in the sexp tree)
	if (state.is_labeled_root) {
		edit_data_act->setEnabled(state.is_root_editable);
		delete_act->setEnabled(state.can_delete);
		copy_act->setEnabled(false);
		insert_op_menu->setEnabled(false);

		util::propagate_disabled_status(popup_menu.get());
		state.cleanup();
		return popup_menu;
	}

	Assertion(item_index != -1, "Menu handle not found!"); // handle not found, which should be impossible.
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

// Cut handler: copies the selected subtree to clipboard via _actions.clipboard_copy(),
// then falls through to deleteActionHandler() to remove it.
void sexp_tree_view::cutActionHandler() {
	_actions.clipboard_copy();

	// fall through to ID_DELETE case.
	deleteActionHandler();
}

// Delete handler: removes the currently selected item from both model and Qt tree.
// Root items: checks _interface RootDeletable flag, emits rootNodeDeleted(), frees the model
// subtree via _model.free_node2(), and deletes the QTreeWidgetItem.
// Non-root items: calls _model.free_node() to unlink from the model, deletes the Qt item.
// Emits modified() in all cases.
void sexp_tree_view::deleteActionHandler()
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
// Edit Data handler: starts inline text editing on the currently selected item via beginItemEdit().
void sexp_tree_view::editDataActionHandler() {
	beginItemEdit(currentItem());
}

// Computes the list of valid operator names for the given node position, based on the
// expected argument type (OPF_*) at that position. Queries:
//   - _model.find_argument_number() to determine which argument position this is
//   - _model.query_node_argument_type() to get the expected OPF type
//   - _model._opf.get_listing_opf() to build the canonical list of valid items for that OPF
//   - _model._opf.query_default_argument_available() to filter operators that lack default args
// Returns a sorted, deduplicated QStringList of operator names. Used by the operator
// quick-search popup and openNodeEditor() to decide whether to show the popup.
QStringList sexp_tree_view::validOperatorsForNode(int nodeIndex) const
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
	sexp_list_item* list = _model._opf.get_listing_opf(opf, parent, argIndex); // may be nullptr
	for (auto* p = list; p; p = p->next) {
		if (p->op >= 0) {
			const int opIndex = p->op;

			// Optional: keep parity with the menu, which disables ops lacking default args
			if (!_model._opf.query_default_argument_available(opIndex))
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

// Decides how to edit a node: roots get inline text edit, operator/data positions with valid
// operator choices get the operator quick-search popup, and pure data nodes get inline edit.
// Relies on validOperatorsForNode() -> _model._opf.get_listing_opf() to determine if operators are available.
void sexp_tree_view::openNodeEditor(QTreeWidgetItem* item)
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

// Creates and shows the operator quick-search popup below the given tree item.
// Populates _opAll from validOperatorsForNode(), creates the popup widgets (QFrame with
// QLineEdit filter + QListWidget) if not already created, and installs event filters.
// Connects: text changes -> filterOperatorPopup(), return/click -> endOperatorQuickSearch(true).
// The popup is sized based on the widest operator name and positioned below the item's visual rect.
void sexp_tree_view::startOperatorQuickSearch(QTreeWidgetItem* item, const QString& seed)
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
		connect(_opEdit, &QLineEdit::textChanged, this, &sexp_tree_view::filterOperatorPopup);
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

// Filters the operator list widget as the user types. Clears and repopulates _opList with
// operators from _opAll that contain the text (case-insensitive). Selects the first match.
void sexp_tree_view::filterOperatorPopup(const QString& text)
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

// Closes the operator quick-search popup and commits the result if confirm is true.
// Commit logic:
//   1. If user selected an item in the list, use that operator name
//   2. If typed text is a valid number and the node expects a number (OPF_NUMBER/OPF_POSITIVE),
//      commit as numeric data via _actions.replace_data()
//   3. Otherwise, fall back to _model.match_closest_operator() for fuzzy matching
// On successful operator resolution, calls _actions.add_or_replace_operator() to commit.
// Hides the popup, clears state, and returns focus to the tree.
void sexp_tree_view::endOperatorQuickSearch(bool confirm)
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
			Q_EMIT modified();
			if (tree_nodes[node].handle)
				tree_item_handle(tree_nodes[node])->setExpanded(true);
		}
	}

	setFocus(Qt::OtherFocusReason);
}

// Slot connected to QTreeWidget::itemChanged. Handles inline edit completion.
// Only processes changes when _currently_editing is true (set by beginItemEdit).
//
// If the item is not found in tree_nodes[] (i.e. it's a root label), emits rootNodeRenamed().
// Otherwise validates the new text via _model.validate_label_edit():
//   - If it resolves to an operator: calls _actions.add_or_replace_operator() to replace the node
//   - If it's a negative number in a positive-only context: shows an error dialog
//   - If the text is valid data: calls _model.apply_label_edit() and emits nodeChanged()
//   - If invalid: reverts the item text to the original tree_nodes[].text
void sexp_tree_view::handleItemChange(QTreeWidgetItem* item, int  /*column*/) {
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

	Assertion(node < tree_nodes.size(), "Invalid node index");
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
			Q_EMIT modified();
		}
	} else if (result.negative_number_error) {
		QMessageBox::critical(this, "Invalid Number", "Can not enter a negative value");
	}

	if (result.update_node) {
		_model.apply_label_edit(node, result.resolved_text);
		modified();
		nodeChanged(node);
	} else {
		auto len = strlen(tree_nodes[node].text);
		item->setText(0, QString::fromUtf8(tree_nodes[node].text, static_cast<int>(len)));
	}
}
// Copy handler: copies the selected subtree to the internal clipboard via _actions.clipboard_copy().
void sexp_tree_view::copyActionHandler() {
	_actions.clipboard_copy();
}

// Paste handler: replaces the current node with clipboard contents via _actions.clipboard_paste_replace().
void sexp_tree_view::pasteActionHandler() {
	_actions.clipboard_paste_replace();
	Q_EMIT modified();
}

// Inserts an operator ABOVE the current node, wrapping it as a child.
// Allocates a new model node via _model.allocate_node() linked to the current node's parent,
// sets it as an operator via _model.set_node(), creates the Qt item via insert(), then calls
// move_branch() to move the original node under the new operator. Adds minimum required
// default children via _actions.add_default_operator(). Emits modified().
// If the current node is a root in a labeled-root tree, emits rootNodeFormulaChanged().
void sexp_tree_view::insertOperatorAction(int op) {
	auto* old_item = tree_item_handle(tree_nodes[item_index]);
	auto* root_parent = old_item ? old_item->parent() : nullptr;
	const int old_item_index = item_index;
	const int node = _actions.insert_operator(op, root_parent);

	if (_interface->getFlags()[TreeFlags::LabeledRoot] && root_parent != nullptr) {
		rootNodeFormulaChanged(old_item_index, node);
		root_parent->setData(0, FormulaDataRole, node);
	}

	setCurrentItemIndex(node);
	Q_EMIT modified();
}

// Add Number handler: adds a SEXPT_NUMBER data node via _actions.add_data("number", ...),
// then starts inline editing on the new item so the user can type the actual value.
// If the parent is a container data node, adds the SEXPT_MODIFIER flag.
void sexp_tree_view::addNumberDataHandler() {
	int theType = SEXPT_NUMBER | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		theType |= SEXPT_MODIFIER;
	}

	int theNode = _actions.add_data("number", theType);
	beginItemEdit(tree_item_handle(tree_nodes[theNode]));
}

// Add String handler: adds a SEXPT_STRING data node via _actions.add_data("string", ...),
// then starts inline editing on the new item. Adds SEXPT_MODIFIER if parent is container data.
void sexp_tree_view::addStringDataHandler() {
	int theType = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_CONTAINER_DATA) {
		theType |= SEXPT_MODIFIER;
	}

	int theNode = _actions.add_data("string", theType);
	beginItemEdit(tree_item_handle(tree_nodes[theNode]));
}

// Replace Number handler: expands the current operator via _actions.expand_operator(),
// replaces the current data with a SEXPT_NUMBER placeholder via _actions.replace_data("number", ...),
// then starts inline editing. Preserves SEXPT_MODIFIER flag if present.
void sexp_tree_view::replaceNumberDataHandler() {
	_actions.expand_operator(item_index);
	int type = SEXPT_NUMBER | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	_actions.replace_data("number", type);
	beginItemEdit(tree_item_handle(tree_nodes[item_index]));
}

// Replace String handler: expands the current operator via _actions.expand_operator(),
// replaces with a SEXPT_STRING placeholder via _actions.replace_data("string", ...),
// then starts inline editing. Preserves SEXPT_MODIFIER flag if present.
void sexp_tree_view::replaceStringDataHandler() {
	_actions.expand_operator(item_index);
	int type = SEXPT_STRING | SEXPT_VALID;
	if (tree_nodes[item_index].type & SEXPT_MODIFIER) {
		type |= SEXPT_MODIFIER;
	}

	_actions.replace_data("string", type);
	beginItemEdit(tree_item_handle(tree_nodes[item_index]));
}

// Sets the _currently_editing flag and calls Qt's editItem() to start inline text editing.
// The flag ensures that handleItemChange() only processes intentional edits, not programmatic changes.
void sexp_tree_view::beginItemEdit(QTreeWidgetItem* item) {
	_currently_editing = true;
	
	editItem(item);
}
// Handles add/replace of a specific typed data item selected from the context menu.
// Resolves the correct sexp_list_item by walking the linked list to data_idx:
//   - For container data nodes: uses _model._opf.get_container_modifiers() or _model._opf.get_container_multidim_modifiers()
//   - For operator arguments: uses _model._opf.get_listing_opf() with the operator's argument type
// Then expands the operator via _actions.expand_operator() and commits via
// _actions.replace_data() (if replace) or _actions.add_data() (if add). Frees the list after use.
void sexp_tree_view::addReplaceTypedDataHandler(int data_idx, bool replace) {
	_actions.add_or_replace_typed_data(data_idx, replace, m_add_count, m_replace_count);
	Q_EMIT modified();
}

// Add Paste handler: adds clipboard contents as a new child via _actions.clipboard_paste_add().
void sexp_tree_view::addPasteActionHandler() {
	_actions.clipboard_paste_add();
	Q_EMIT modified();
}

// Sets item_index and syncs the Qt selection. If node < 0, clears the selection.
// Otherwise selects the QTreeWidgetItem corresponding to tree_nodes[node].
void sexp_tree_view::setCurrentItemIndex(int node) {
	item_index = node;
	if (node < 0) {
		setCurrentItem(nullptr);
	} else {
		setCurrentItem(tree_item_handle(tree_nodes[node]));
	}
}

// Replace Variable handler: replaces the current data node with a variable reference.
// Validates type compatibility between the node's expected type and the variable's type.
// For modify-variable or OPF_CONTAINER_VALUE contexts, allows type coercion.
// Commits via _actions.replace_variable_data(). Uses _model.query_node_argument_type() for type checking.
void sexp_tree_view::handleReplaceVariableAction(int id) {
	const int node_type = get_type(currentItem());
	const bool allow_type_coercion =
		(m_modify_variable != 0) || (_model.query_node_argument_type(item_index) == OPF_CONTAINER_VALUE);
	_actions.replace_variable_with_type_validation(id, node_type, allow_type_coercion);
	Q_EMIT modified();
}

// Replace Container Name handler: replaces the current string node with a container name reference.
// Validates that the current node is a string type. Commits via _actions.replace_container_name().
void sexp_tree_view::handleReplaceContainerNameAction(int idx) {
	Assertion(item_index >= 0, "Attempt to Replace Container Name with no node selected. Please report!");

	const auto &containers = get_all_sexp_containers();
	Assertion(SCP_vector_inbounds(containers, idx), "Unknown Container Index %d. Please report!", idx);

	const int type = get_type(currentItem());
	Assertion(type & SEXPT_STRING,
		"Attempt to replace container name on non-string node %s with type %d. Please report!",
		tree_nodes[item_index].text,
		type);

	_actions.replace_container_name(containers[idx]);
	Q_EMIT modified();
}

// Replace Container Data handler: replaces the current data node with container data access.
// Strips SEXPT_VARIABLE and SEXPT_CONTAINER_NAME flags, adds SEXPT_CONTAINER_DATA.
// Commits via _actions.replace_container_data() with full modifier support, then expands the branch.
void sexp_tree_view::handleReplaceContainerDataAction(int idx) {
	Assertion(item_index >= 0, "Attempt to Replace Container Data with no node selected. Please report!");

	const auto &containers = get_all_sexp_containers();
	Assertion(SCP_vector_inbounds(containers, idx), "Unknown Container index %d. Please report!", idx);

	int type = get_type(currentItem());
	Assertion((type & SEXPT_NUMBER) || (type & SEXPT_STRING), "Attempt to use Replace Container Data on a non-data node. Please report!");

	// variable/container name don't mix with container data
	// DISCUSSME: what about variable name as SEXP arg type?
	type &= ~(SEXPT_VARIABLE | SEXPT_CONTAINER_NAME);
	_actions.replace_container_data(containers[idx], (type | SEXPT_CONTAINER_DATA), true, true, true);
	Q_EMIT modified();

	auto *handle = tree_item_handle(tree_nodes[item_index]);
	expand_branch(handle);
}

// Slot connected to itemSelectionChanged. Updates the help text panel via update_help(),
// sets item_index to the selected node's tree_nodes[] index, walks up to the root item,
// and emits selectedRootChanged() with the root's FormulaDataRole value.
void sexp_tree_view::handleNewItemSelected() {
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

// Public entry point for deleting the currently selected item. Simply delegates to deleteActionHandler().
void sexp_tree_view::deleteCurrentItem() {
	deleteActionHandler();
}

// Reads NoteRole and BgColorRole from a QTreeWidgetItem and applies visual styling:
// NoteRole text -> tooltip, BgColorRole -> background brush. Called after annotation edits
// and during branch move/copy to preserve visual state. Pure UI operation.
void sexp_tree_view::applyVisuals(QTreeWidgetItem* it)
{
	const auto note = it->data(0, NoteRole).toString();
	const auto color = it->data(0, BgColorRole).value<QColor>();
	it->setToolTip(0, note);

	// Background color for the entire row
	if (color.isValid()) {
		it->setBackground(0, QBrush(color));
	}
}

// Returns the current item_index (tree_nodes[] index of the selected node).
int sexp_tree_view::getCurrentItemIndex() const {
	return item_index;
}

} // namespace fso::fred
