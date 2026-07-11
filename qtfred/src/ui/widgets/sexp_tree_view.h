#pragma once

#include "missioneditor/sexp_tree_model.h"
#include "missioneditor/sexp_tree_actions.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "parse/parselo.h"

#include "mission/Editor.h"
#include "mission/EditorViewport.h"

#include <QTreeView>
#include <QTreeWidgetItem>
#include <QListWidget>
#include <QShortcut>
#include <QKeySequence>

namespace fso::fred {

// Bring shared types into the fso::fred namespace so existing code compiles unchanged
using ::sexp_tree_item;
using ::sexp_list_item;
using ::NodeImage;
using ::TreeFlags;
using ::SexpTreeEditorInterface;
using ::ISexpTreeUI;
using ::SexpTreeModel;
using ::SexpTreeActions;

// Typed handle accessor for QtFRED (Qt QTreeWidgetItem*)
inline QTreeWidgetItem* tree_item_handle(const sexp_tree_item& item) {
	return static_cast<QTreeWidgetItem*>(item.handle);
}

/**
 * Qt widget for displaying and editing SEXP trees.
 *
 * This is the main UI class for the SEXP tree editor in QtFRED. It extends QTreeWidget
 * to display sexp nodes as a visual tree, and implements ISexpTreeUI so that the shared
 * SexpTreeActions layer can drive Qt widget updates (insert/delete/select items, etc.)
 * without depending on Qt directly.
 *
 * Owns a SexpTreeModel (all data/logic) and SexpTreeActions (all mutation operations).
 * All sexp-level decisions (validity, argument types, operator eligibility) are delegated
 * to _model. All coordinated model+UI mutations go through _actions. This class handles
 * only Qt-specific concerns: widget creation, event handling, context menus, dialogs,
 * drag-and-drop, the operator quick-search popup, and signal emission.
 */
class sexp_tree_view: public QTreeWidget, public ISexpTreeUI {

 Q_OBJECT
 public:
	SexpTreeModel _model;
	SexpTreeActions _actions;

	 /**
	  * Custom Qt item data roles stored on each QTreeWidgetItem.
	  * FormulaDataRole: the formula/root node index for top-level items
	  * SexpNodeIdRole: the tree_nodes[] index for a given item
	  * NoteRole: user-editable comment string (displayed as tooltip + badge icon)
	  * BgColorRole: user-chosen background highlight color
	  */
	 enum {
		 FormulaDataRole = Qt::UserRole + 1,
		 SexpNodeIdRole = Qt::UserRole + 2,
		 NoteRole = Qt::UserRole + 100,
		 BgColorRole = Qt::UserRole + 101,
		 NodeImageRole = Qt::UserRole + 102  //!< Base NodeImage stored on each item so numbered-data badges can renumber without a model lookup
	 };

	//! Builds the QIcon for a NodeImage by colorizing the shared master art and applying a
	//! uniform drop shadow. For a numbered data node, pass the 1-based argument position in
	//! `number` to draw it on the badge; -1 (the default) means "no number".
 	static QIcon convertNodeImageToIcon(NodeImage image, int number = -1);

	//! Refreshes the icons of every sexp_tree_view currently alive. Used to apply a changed
	//! "number every N" preference to already-open trees without waiting for a rebuild.
	static void refreshAllInstances();

	explicit sexp_tree_view(QWidget* parent = nullptr);
	~sexp_tree_view() override;

	//! Finds the node index for handle h, delegates to _model.compute_help_text(),
	//! then emits helpChanged() and miniHelpChanged() signals.
	void update_help(QTreeWidgetItem* h);

	//! Returns the help string for a given operator code. Delegates to SexpTreeModel::help().
	static const char* help(int code);

	//! Creates a new QTreeWidgetItem with text and icon under hParent, after hInsertAfter.
	//! Thin wrapper that converts NodeImage to QIcon and calls insertWithIcon().
	QTreeWidgetItem* insert(const QString& lpszItem, NodeImage image = NodeImage::ROOT, QTreeWidgetItem* hParent = nullptr, QTreeWidgetItem* hInsertAfter = nullptr);

	//! Returns the QTreeWidgetItem* handle for a given tree_nodes[] index.
	QTreeWidgetItem* handle(int node) const;

	//! Looks up the sexp type (SEXPT_OPERATOR, SEXPT_STRING, etc.) for the node matching handle h.
	//! Performs a linear scan of tree_nodes[].
	int get_type(QTreeWidgetItem* h) const;

	//! Returns the tree_nodes[] index for the node matching handle h.
	//! Performs a linear scan of tree_nodes[].
	int get_node(QTreeWidgetItem* h) const;

	//! Walks parent links from node up to find the root node index.
	//! Uses _model.tree_nodes[].parent.
	int get_root(int node) const;

	//! Reorders top-level (root) items by removing source and reinserting relative to dest.
	//! Emits rootOrderChanged() and modified(). Pure Qt widget operation.
	void move_root(QTreeWidgetItem* source, QTreeWidgetItem* dest, bool insert_before);

	//! Moves a branch in both the model data and the Qt tree.
	//! Calls _model.move_branch_data() then the QTreeWidgetItem overload of move_branch().
	void move_branch(int source, int parent);

	//! Recursively re-creates a Qt subtree under a new parent. Copies all custom data roles
	//! (FormulaDataRole, NoteRole, BgColorRole), updates tree_nodes[].handle, preserves
	//! expansion state, and deletes the old source item.
	QTreeWidgetItem* move_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent = nullptr, QTreeWidgetItem* after = nullptr);

	//! Recursively copies a Qt subtree under a new parent without removing the source.
	//! Copies all custom data roles and preserves expansion state.
	void copy_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent = nullptr, QTreeWidgetItem* after = nullptr);

	//! Expands all ancestor items of the given node so it becomes visible in the tree.
	void ensure_visible(int node);

	//! Displays an error dialog for a node, selects and scrolls to it.
	//! Returns -1 if user declines to continue, 0 otherwise. Pure UI.
	int node_error(int node, const char* msg, int* bypass);

	//! Recursively expands a QTreeWidgetItem and all its children.
	void expand_branch(QTreeWidgetItem* h);

	//! Opens a QInputDialog for editing the comment annotation (NoteRole) on a tree item.
	//! Emits nodeAnnotationChanged() on success.
	void editNoteForItem(QTreeWidgetItem* h);

	//! Opens a QColorDialog for choosing a background highlight color (BgColorRole).
	//! Emits nodeBgColorChanged() on success.
	void editBgColorForItem(QTreeWidgetItem* h);

	int ctree_size;

	//! Clears the Qt widget and rebuilds the visual tree from tree_nodes[] starting at node 0.
	//! Calls add_sub_tree() recursively.
	virtual void build_tree();

	//! Resets all model node data via _model.clear_tree_data(). If op is provided and non-empty,
	//! clears the Qt tree, allocates a root operator node via _model, and rebuilds.
	void clear_tree(const char* op = nullptr);

	//! Nulls out all tree_nodes[].handle pointers. Used before a visual rebuild.
	void reset_handles();

	//! Loads sexp data from the game's Sexp_nodes[] into the model via _model.load_tree_data(),
	//! then clears and rebuilds the visual tree.
	void load_tree(int index, const char* deflt = "true");

	//! Adds an operator node under the current position. Delegates to _actions.add_operator().
	//! Returns the new item_index.
	int add_operator(const char* op, QTreeWidgetItem* h = nullptr);

	//! Recursively creates QTreeWidgetItems for a model node and all its children/siblings.
	//! Uses _model.compute_node_visual_info() for icon and flag computation.
	void add_sub_tree(int node, QTreeWidgetItem* root);

	//! Selects and scrolls to a node in the tree. Uses ensure_visible() and Qt selection.
	void hilite_item(int node);

	//! Returns the current item_index (tree_nodes[] index of the selected node).
	int getCurrentItemIndex() const;

	//! Sets item_index and syncs the Qt selection to match the corresponding tree item.
	void setCurrentItemIndex(int index);

	int& select_sexp_node = _model.select_sexp_node;  //!< Used to pre-select a node on dialog open.

	//! Stores the Editor and SexpTreeEditorInterface pointers. If no interface is provided,
	//! creates a default SexpTreeEditorInterface.
	void initializeEditor(Editor* edit, SexpTreeEditorInterface* editorInterface = nullptr, EditorViewport* viewport = nullptr);

	//! Public entry point for deleting the currently selected item. Calls deleteActionHandler().
	void deleteCurrentItem();

	//! Reads NoteRole and BgColorRole from a QTreeWidgetItem and applies tooltip + background color.
	static void applyVisuals(QTreeWidgetItem* it);

	// --- ISexpTreeUI implementation ---
	// These are callback methods invoked by SexpTreeActions to manipulate the Qt tree
	// without the shared action layer depending on Qt. Each casts void* to QTreeWidgetItem*.

	void* ui_insert_item(const char* text, NodeImage image, void* parent_handle, void* insert_after) override; //!< Creates a QTreeWidgetItem via insert()
	void ui_delete_item(void* handle) override;                  //!< Deletes a QTreeWidgetItem
	void ui_set_item_text(void* handle, const char* text) override;   //!< Sets display text via setText()
	void ui_set_item_image(void* handle, NodeImage image) override;   //!< Sets icon via setIcon()
	void* ui_get_child_item(void* handle) const override;        //!< Returns first child item, or nullptr
	bool ui_has_children(void* handle) const override;           //!< Returns true if childCount() > 0
	void ui_expand_item(void* handle) override;                  //!< Expands a single item via setExpanded()
	void ui_select_item(void* handle) override;                  //!< Selects item via setCurrentItem()
	void ui_ensure_visible(void* handle) override;               //!< Scrolls to item via scrollToItem()
	void ui_notify_modified() override;                          //!< Emits the modified() signal
	void ui_update_help(void* handle) override;                  //!< Calls update_help() to refresh help panel
	void ui_add_children_visual(int parent_node_index) override; //!< Adds Qt child items for all model children of a node
	void ui_move_branch(int source_node, int parent_node) override; //!< Moves existing subtree under a new parent node
	void ui_expand_branch(void* handle) override;                //!< Recursively expands via expand_branch()

 signals:
	void miniHelpChanged(const QString& text);  //!< Emitted when the short help description changes (from update_help)
	void helpChanged(const QString& text);       //!< Emitted when the full help text changes (from update_help)
	void modified();                             //!< Emitted whenever the tree is mutated (any add/delete/replace/move)

	void rootNodeDeleted(int node);              //!< Emitted when a root-level item is deleted (formula node index)
	void rootNodeRenamed(int node);              //!< Emitted when a root-level item's label is edited inline
	void rootNodeFormulaChanged(int old, int node); //!< Emitted when insertOperatorAction wraps a root, changing its formula index
	void nodeChanged(int node);                  //!< Emitted when a data node's text is changed via inline edit
	void rootOrderChanged();                     //!< Emitted when root items are reordered via drag-and-drop

	void selectedRootChanged(int formula);       //!< Emitted when selection moves to a different root's subtree

	void nodeAnnotationChanged(void* handle, const QString& note);  //!< Emitted when a node's comment is edited
	void nodeBgColorChanged(void* handle, const QColor& color);     //!< Emitted when a node's background color is changed

 protected:
	//! Routes keys to operator popup when active (Escape=cancel, Enter=confirm, arrows forwarded to list).
	//! Space opens the node editor on the current item. Otherwise delegates to QTreeWidget.
	void keyPressEvent(QKeyEvent* e) override;

	//! Watches the operator popup for Hide/Close/Deactivate events to clear popup state.
	bool eventFilter(QObject* obj, QEvent* ev) override;

	//! Re-renders every node icon when the palette changes (light/dark toggle) so the
	//! theme-adaptive icons (dots, chain, etc.) update while the tree is visible.
	void changeEvent(QEvent* e) override;

	//! Recomputes and re-applies the icon for every item from its stored NodeImageRole.
	void refreshAllIcons();

	//! Records drag start position and source root item for root-level drag-and-drop.
	void mousePressEvent(QMouseEvent* e) override;

	//! Highlights potential drop-target root items during a drag operation.
	void mouseMoveEvent(QMouseEvent* e) override;

	//! Completes root-level drag by calling move_root() to reorder top-level items.
	void mouseReleaseEvent(QMouseEvent* e) override;

	//! Core Qt item creation. Creates a QTreeWidgetItem with text and pre-built QIcon under
	//! the specified parent/after position. Blocks signals during creation to prevent spurious
	//! itemChanged events.
	QTreeWidgetItem* insertWithIcon(const QString& lpszItem, const QIcon& image, QTreeWidgetItem* hParent = nullptr, QTreeWidgetItem* hInsertAfter = nullptr);

	//! Computes and sets the icon for a tree item. For numbered-data nodes this derives the
	//! 1-based argument position from the item's sibling index and the "number every N"
	//! preference, so the badge shows the argument count at every Nth position.
	void applyNodeIcon(QTreeWidgetItem* item, NodeImage image);

	//! Re-applies icons to all children of `parent`. Called after a data node is inserted or
	//! removed so the numbered-data badges renumber to match the shifted sibling positions.
	void refreshDataNumbers(QTreeWidgetItem* parent);

	//! Returns the "number every N arguments" preference (0 = never number). Reads the value
	//! from the EditorViewport, falling back to a default when no viewport is set.
	int numberEveryN() const;

	//! Slot for customContextMenuRequested. Gets the item at pos, builds and executes the context menu.
	void customMenuHandler(const QPoint& pos);

	//! Creates a persistent QShortcut on this widget bound to `key`. When the user presses
	//! the shortcut while this widget has focus, recomputes the context menu state and only
	//! invokes `action` if `gate(state)` returns true. Used to wire Cut/Copy/Paste/Delete
	//! hotkeys without the FRED2 hazard of triggering actions that are invalid for the
	//! current selection.
	template <typename GateFn, typename ActionFn>
	void installShortcut(const QKeySequence& key, GateFn gate, ActionFn action)
	{
		auto* sc = new QShortcut(key, this);
		sc->setContext(Qt::WidgetShortcut);
		connect(sc, &QShortcut::activated, this,
			[this, gate = std::move(gate), action = std::move(action)]() {
				if (_currently_editing || _opPopupActive)
					return;
				if (currentItem() == nullptr)
					return;
				// Re-sync item_index to the currently selected item before computing
				// menu state. Some mutations (like a paste that adds a child) move the
				// model's item_index onto the newly created node without changing the
				// visual selection, which would otherwise make a repeated hotkey act on
				// the wrong node and fail its gate. The right-click menu re-derives
				// item_index the same way, which is why the menu path can be repeated.
				item_index = get_node(currentItem());
				// item_index may be -1 in labeled-root mode (the user selected the
				// label itself). compute_context_menu_state() handles that path and
				// returns a state whose can_* flags already reflect what's legal
				// (e.g. can_delete only when TreeFlags::RootDeletable is set).
				auto state = _model.compute_context_menu_state();
				if (!gate(state))
					return;
				action();
			});
	}

	//! Slot for itemSelectionChanged. Updates help text, sets item_index, and emits selectedRootChanged().
	void handleNewItemSelected();

	//! Builds the full right-click context menu for a given item. Calls _model.compute_context_menu_state()
	//! to determine all enabled/disabled states, then constructs menus for Delete, Edit, Cut/Copy/Paste,
	//! Add/Replace/Insert Operator (categorized submenus), Add/Replace Data, Variables, and Containers.
	//! Operator actions connect to _actions.add_or_replace_operator(). Data actions connect to local handlers.
	std::unique_ptr<QMenu> buildContextMenu(QTreeWidgetItem* h);

	int& flag = _model.flag;                                    //!< Alias for _model.flag
	SCP_vector<sexp_tree_item>& tree_nodes = _model.tree_nodes; //!< Alias for _model.tree_nodes
	int& total_nodes = _model.total_nodes;                      //!< Alias for _model.total_nodes

	bool _currently_editing = false; //!< True while the user is inline-editing a tree node's text

	int& root_item = _model.root_item;   //!< Alias for _model.root_item
	int& item_index = _model.item_index; //!< Alias for _model.item_index (currently selected node)

	Editor* _editor = nullptr;                                   //!< The FRED Editor instance
	EditorViewport* _viewport = nullptr;                         //!< The EditorViewport (set by initializeEditor)
	SexpTreeEditorInterface*& _interface = _model._interface;    //!< Alias for _model._interface (flags for tree behavior)
	std::unique_ptr<SexpTreeEditorInterface> _owned_interface;   //!< Default interface if none is supplied externally

	int m_add_count = 0, m_replace_count = 0; //!< Argument position counters from context menu state (for typed data handlers)
	int m_modify_variable = 0;                //!< Whether modify-variable is active (affects variable type checking)

	// --- Operator quick search popup widgets and state ---
	QFrame* _opPopup = nullptr;     //!< Popup frame containing the search field and list
	QLineEdit* _opEdit = nullptr;   //!< Text filter field in the popup
	QListWidget* _opList = nullptr; //!< Filtered list of valid operators in the popup
	QStringList _opAll;             //!< All valid operators for the current node context
	int _opNodeIndex = -1;          //!< tree_nodes[] index of the node being edited via popup
	bool _opPopupActive = false;    //!< True while the popup is shown and accepting input
	QPoint _dragStartPos;           //!< Mouse position where the current root drag started
	QTreeWidgetItem* _dragSourceRoot = nullptr; //!< Root item being dragged (root-level reordering only)
	bool _dragging = false;                     //!< True once drag distance threshold has been exceeded

	//! Decides whether to open the operator quick-search popup or inline text edit for an item.
	//! Operators and nodes with valid operator choices get the popup; root labels and pure data get inline edit.
	//! Relies on validOperatorsForNode() -> _model._opf.get_listing_opf().
	void openNodeEditor(QTreeWidgetItem* item);

	//! Creates/shows the operator quick-search popup below the given item. Populates the list
	//! from validOperatorsForNode(). Relies on _model._opf.get_listing_opf() and _model.query_node_argument_type().
	void startOperatorQuickSearch(QTreeWidgetItem* item, const QString& seed = QString());

	//! Closes the operator popup. On confirm: commits the chosen operator via _actions.add_or_replace_operator(),
	//! or commits typed numbers via _actions.replace_data(). Falls back to _model.match_closest_operator().
	void endOperatorQuickSearch(bool confirm);

	//! Filters the operator list widget as the user types, matching case-insensitively.
	void filterOperatorPopup(const QString& text);

	//! Computes the list of valid operator names for a given node position. Queries
	//! _model.find_argument_number(), _model.query_node_argument_type(), _model._opf.get_listing_opf(),
	//! and _model._opf.query_default_argument_available().
	QStringList validOperatorsForNode(int nodeIndex) const;

	//! Slot for itemChanged. Handles inline edit completion. For root labels: emits rootNodeRenamed().
	//! For operators: validates via _model.validate_label_edit() and commits via _actions.add_or_replace_operator().
	//! For data: commits via _model.apply_label_edit() and emits nodeChanged().
	void handleItemChange(QTreeWidgetItem* item, int column);

	//! Sets _currently_editing flag and calls Qt editItem() to start inline text editing.
	void beginItemEdit(QTreeWidgetItem* item);

	// --- Context menu action handlers ---

	//! Deletes the selected item. Root items: emits rootNodeDeleted(), calls _model.free_node2().
	//! Non-root: calls _model.free_node(). Removes the Qt item and emits modified().
	void deleteActionHandler();

	//! Copies selected subtree to clipboard via _actions.clipboard_copy(), then deletes it.
	void cutActionHandler();

	//! Copies selected subtree to clipboard via _actions.clipboard_copy().
	void copyActionHandler();

	//! Replaces current node with clipboard contents via _actions.clipboard_paste_replace().
	void pasteActionHandler();

	//! Adds clipboard contents as a new child via _actions.clipboard_paste_add().
	void addPasteActionHandler();

	//! Starts inline edit on the current item via beginItemEdit().
	void editDataActionHandler();

	//! Adds a "number" data node via _actions.add_data() and starts inline edit on it.
	void addNumberDataHandler();

	//! Adds a "string" data node via _actions.add_data() and starts inline edit on it.
	void addStringDataHandler();

	//! Expands operator via _actions.expand_operator(), replaces with "number" via _actions.replace_data(),
	//! and starts inline edit.
	void replaceNumberDataHandler();

	//! Expands operator via _actions.expand_operator(), replaces with "string" via _actions.replace_data(),
	//! and starts inline edit.
	void replaceStringDataHandler();

	//! Handles add/replace of a specific typed data item from the context menu list.
	//! Resolves the item from _model._opf.get_listing_opf() or _model._opf.get_container_modifiers()/
	//! _model._opf.get_container_multidim_modifiers(), then commits via _actions.add_data() or _actions.replace_data().
	void addReplaceTypedDataHandler(int data_idx, bool replace);

	//! Replaces current data node with a variable reference via _actions.replace_variable_data().
	//! Uses _model.query_node_argument_type() for type checking.
	void handleReplaceVariableAction(int idx);

	//! Replaces current node with a container name via _actions.replace_container_name().
	void handleReplaceContainerNameAction(int idx);

	//! Replaces current node with container data access via _actions.replace_container_data().
	//! Expands the resulting branch.
	void handleReplaceContainerDataAction(int idx);

	//! Inserts an operator above the current node (wrapping it). Allocates a new model node via
	//! _model.allocate_node()/set_node(), creates the Qt item, moves the old branch under it via
	//! move_branch(), and adds minimum required children via _actions.add_default_operator().
	void insertOperatorAction(int op);
};

} // namespace fso::fred
