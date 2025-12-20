/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#pragma once

#include "missioneditor/sexp_tree_model.h"
#include "missioneditor/sexp_tree_actions.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "parse/parselo.h"

#include "mission/Editor.h"

#include <QTreeView>
#include <QTreeWidgetItem>
#include <QListWidget>

// various tree operations notification codes (to be handled by derived class)
#define ROOT_DELETED    1
#define ROOT_RENAMED    2

namespace fso {
namespace fred {

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

class sexp_tree: public QTreeWidget, public ISexpTreeUI {

 Q_OBJECT
 public:
	// Shared model and action layer (must be declared before reference aliases below)
	SexpTreeModel _model;
	SexpTreeActions _actions;

	 enum {
		 FormulaDataRole = Qt::UserRole + 1,
		 SexpNodeIdRole = Qt::UserRole + 2,
		 NoteRole = Qt::UserRole + 100,
		 BgColorRole = Qt::UserRole + 101
	 };

 	static QIcon convertNodeImageToIcon(NodeImage image);

	explicit sexp_tree(QWidget* parent = nullptr);
	~sexp_tree() override;

	void update_help(QTreeWidgetItem* h);
	const char* help(int code);
	QTreeWidgetItem* insert(const QString& lpszItem,
							NodeImage image = NodeImage::ROOT,
							QTreeWidgetItem* hParent = nullptr,
							QTreeWidgetItem* hInsertAfter = nullptr);
	QTreeWidgetItem* handle(int node);
	int get_type(QTreeWidgetItem* h);
	int get_node(QTreeWidgetItem* h);
	int get_root(int node);
	void move_root(QTreeWidgetItem* source, QTreeWidgetItem* dest, bool insert_before);
	void move_branch(int source, int parent = -1);
	QTreeWidgetItem*
	move_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent = nullptr, QTreeWidgetItem* after = nullptr);
	void copy_branch(QTreeWidgetItem* source, QTreeWidgetItem* parent = nullptr, QTreeWidgetItem* after = nullptr);
	void ensure_visible(int node);
	int node_error(int node, const char* msg, int* bypass);
	void expand_branch(QTreeWidgetItem* h);
	void editNoteForItem(QTreeWidgetItem* h);
	void editBgColorForItem(QTreeWidgetItem* h);
	int ctree_size;
	virtual void build_tree();
	void clear_tree(const char* op = NULL);
	void reset_handles();
	void load_tree(int index, const char* deflt = "true");
	int add_operator(const char* op, QTreeWidgetItem* h = nullptr);
	void add_sub_tree(int node, QTreeWidgetItem* root);
	void hilite_item(int node);

	// OPF listing and container modifier queries are accessed directly via _model

	int getCurrentItemIndex() const;
	void setCurrentItemIndex(int index);
	int& select_sexp_node = _model.select_sexp_node;  // used to select an sexp item on dialog box open.

	void initializeEditor(Editor* edit, SexpTreeEditorInterface* editorInterface = nullptr);

	void deleteCurrentItem();

	static void applyVisuals(QTreeWidgetItem* it);

	// ISexpTreeUI implementation
	void* ui_insert_item(const char* text, NodeImage image, void* parent_handle, void* insert_after) override;
	void ui_delete_item(void* handle) override;
	void ui_set_item_text(void* handle, const char* text) override;
	void ui_set_item_image(void* handle, NodeImage image) override;
	void* ui_get_child_item(void* handle) override;
	bool ui_has_children(void* handle) override;
	void ui_expand_item(void* handle) override;
	void ui_select_item(void* handle) override;
	void ui_ensure_visible(void* handle) override;
	void ui_notify_modified() override;
	void ui_update_help(void* handle) override;
	void ui_add_children_visual(int parent_node_index) override;
	void ui_expand_branch(void* handle) override;

 signals:
	void miniHelpChanged(const QString& text);
	void helpChanged(const QString& text);
	void modified();

	void rootNodeDeleted(int node);
	void rootNodeRenamed(int node);
	void rootNodeFormulaChanged(int old, int node);
	void nodeChanged(int node);
	void rootOrderChanged();

	void selectedRootChanged(int formula);

	void nodeAnnotationChanged(void* handle, const QString& note);
	void nodeBgColorChanged(void* handle, const QColor& color);

	// Generated message map functions
 protected:
	void keyPressEvent(QKeyEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* ev) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;

	QTreeWidgetItem* insertWithIcon(const QString& lpszItem,
									const QIcon& image,
									QTreeWidgetItem* hParent = nullptr,
									QTreeWidgetItem* hInsertAfter = nullptr);

	//bool edit(const QModelIndex& index, QAbstractItemView::EditTrigger trigger, QEvent* event) override
	//{
		//_currently_editing = true; // mark explicit edit
		//return QTreeWidget::edit(index, trigger, event);
	//}

	void customMenuHandler(const QPoint& pos);

	void handleNewItemSelected();

	std::unique_ptr<QMenu> buildContextMenu(QTreeWidgetItem* h);

	int& flag = _model.flag;

	SCP_vector<sexp_tree_item>& tree_nodes = _model.tree_nodes;
	int& total_nodes = _model.total_nodes;

	// This flag is used for keeping track if we are currently editing a tree node
	bool _currently_editing = false;

	int& root_item = _model.root_item;

	int& item_index = _model.item_index;

	Editor* _editor = nullptr;
	SexpTreeEditorInterface*& _interface = _model._interface;

	// If there is no special interface then we supply a default one which needs to be stored somewhere
	std::unique_ptr<SexpTreeEditorInterface> _owned_interface;

	int m_add_count = 0, m_replace_count = 0;
	int m_modify_variable = 0;

	//  Operator quick search popup
	QFrame* _opPopup = nullptr;
	QLineEdit* _opEdit = nullptr;
	QListWidget* _opList = nullptr;
	QStringList _opAll;    // all valid operators for current context
	int _opNodeIndex = -1; // tree_nodes[] index of the node being edited
	bool _opPopupActive = false;

	void openNodeEditor(QTreeWidgetItem* item);
	void startOperatorQuickSearch(QTreeWidgetItem* item, const QString& seed = QString());
	void endOperatorQuickSearch(bool confirm);
	void filterOperatorPopup(const QString& text);
	QStringList validOperatorsForNode(int nodeIndex);

	void handleItemChange(QTreeWidgetItem* item, int column);

	void beginItemEdit(QTreeWidgetItem* item);

	void deleteActionHandler();
	void cutActionHandler();
	void copyActionHandler();
	void pasteActionHandler();
	void addPasteActionHandler();
	void editDataActionHandler();
	void addNumberDataHandler();
	void addStringDataHandler();
	void replaceNumberDataHandler();
	void replaceStringDataHandler();
	void addReplaceTypedDataHandler(int data_idx, bool replace);
	void handleReplaceVariableAction(int idx);
	void handleReplaceContainerNameAction(int idx);
	void handleReplaceContainerDataAction(int idx);

	void insertOperatorAction(int op);
};

}
}
