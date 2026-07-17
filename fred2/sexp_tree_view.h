/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _SEXP_TREE_VIEW_H
#define _SEXP_TREE_VIEW_H

// 4786 is identifier truncated to 255 characters (happens all the time in Microsoft #includes) -- Goober5000
#pragma warning(disable: 4786)

#include "OperatorComboBox.h"
#include "missioneditor/sexp_tree_model.h"
#include "missioneditor/sexp_tree_actions.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"
#include "parse/parselo.h"

// FRED2 BITMAP_* compatibility aliases (map to shared NodeImage enum values)
#define BITMAP_OPERATOR         static_cast<int>(NodeImage::OPERATOR)
#define BITMAP_DATA             static_cast<int>(NodeImage::DATA)
#define BITMAP_VARIABLE         static_cast<int>(NodeImage::VARIABLE)
#define BITMAP_ROOT             static_cast<int>(NodeImage::ROOT)
#define BITMAP_ROOT_DIRECTIVE   static_cast<int>(NodeImage::ROOT_DIRECTIVE)
#define BITMAP_CHAIN            static_cast<int>(NodeImage::CHAIN)
#define BITMAP_CHAIN_DIRECTIVE  static_cast<int>(NodeImage::CHAIN_DIRECTIVE)
#define BITMAP_GREEN_DOT        static_cast<int>(NodeImage::GREEN_DOT)
#define BITMAP_BLACK_DOT        static_cast<int>(NodeImage::BLACK_DOT)
#define BITMAP_BLUE_DOT         BITMAP_ROOT
#define BITMAP_RED_DOT          BITMAP_ROOT_DIRECTIVE
#define BITMAP_NUMBERED_DATA    static_cast<int>(NodeImage::DATA_00)
// There are 20 number bitmaps, 9 to 28, counting by 5s from 0 to 95
#define BITMAP_COMMENT          static_cast<int>(NodeImage::COMMENT)
#define BITMAP_CONTAINER_NAME   static_cast<int>(NodeImage::CONTAINER_NAME)
#define BITMAP_CONTAINER_DATA   static_cast<int>(NodeImage::CONTAINER_DATA)

// Typed handle accessors for FRED2 (MFC HTREEITEM)
inline HTREEITEM tree_item_handle(const sexp_tree_item& item) {
	return static_cast<HTREEITEM>(item.handle);
}

class sexp_tree_view : public CTreeCtrl, public ISexpTreeUI
{
public:
	// Shared model and action layer (must be declared before reference aliases below)
	SexpTreeModel _model;
	SexpTreeActions _actions;

	sexp_tree_view();

	void update_help(HTREEITEM h);
	static const char *help(int code);
	HTREEITEM insert(LPCTSTR lpszItem, int image = BITMAP_ROOT, int sel_image = BITMAP_ROOT, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM handle(int node) const;
	int get_node(HTREEITEM h) const;
	int get_type(HTREEITEM h) const;
	void setup(CEdit *ptr = nullptr);
	void move_root(HTREEITEM source, HTREEITEM dest, bool insert_before);
	void move_branch(int source, int parent);
	HTREEITEM move_branch(HTREEITEM source, HTREEITEM parent = TVI_ROOT, HTREEITEM after = TVI_LAST);
	void copy_branch(HTREEITEM source, HTREEITEM parent = TVI_ROOT, HTREEITEM after = TVI_LAST);
	void setup_selected(HTREEITEM h = nullptr);
	void ensure_visible(int node);
	int node_error(int node, const char *msg, int *bypass);
	void expand_branch(HTREEITEM h);
	int end_label_edit(TVITEMA &item);
	int edit_label(HTREEITEM h, bool *is_operator = nullptr) const;
	virtual void edit_comment(HTREEITEM h);
	virtual void edit_bg_color(HTREEITEM h);
	virtual SCP_string get_node_comment(int node_index) const;
	virtual SCP_string get_item_comment(HTREEITEM h, int node_index) const;
	void right_clicked();
	int ctree_size;
	virtual void build_tree();
	void clear_tree(const char *op = nullptr);
	void reset_handles();
	void load_tree(int index, const char *deflt = "true");
	void add_operator(const char *op, HTREEITEM h = TVI_ROOT);
	void add_sub_tree(int node, HTREEITEM root);
	void hilite_item(int node);

	int& item_index = _model.item_index;
	int& select_sexp_node = _model.select_sexp_node;  // used to select an sexp item on dialog box open.
	BOOL		m_dragging;
	HTREEITEM	m_h_drag;
	HTREEITEM	m_h_drop;
	CImageList	*m_p_image_list;
	CEdit *help_box;
	CEdit *mini_help_box;
	CPoint m_pt;
	OperatorComboBox m_operator_box;

	void start_operator_edit(HTREEITEM h);
	void end_operator_edit(bool confirm);

	// ISexpTreeUI implementation
	void* ui_insert_item(const char* text, NodeImage image, void* parent_handle, void* insert_after) override;
	void ui_delete_item(void* handle) override;
	void ui_set_item_text(void* handle, const char* text) override;
	void ui_set_item_image(void* handle, NodeImage image) override;
	void* ui_get_child_item(void* handle) const override;
	bool ui_has_children(void* handle) const override;
	void ui_expand_item(void* handle) override;
	void ui_select_item(void* handle) override;
	void ui_ensure_visible(void* handle) override;
	void ui_notify_modified() override;
	void ui_update_help(void* handle) override;
	void ui_add_children_visual(int parent_node_index) override;
	void ui_move_branch(int source_node, int parent_node) override;
	void ui_expand_branch(void* handle) override;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(sexp_tree_view)
	public:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(sexp_tree_view)
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	virtual void NodeCut();
	virtual void NodeDelete();
	virtual void NodeCopy();
	virtual void NodeReplacePaste();
	virtual void NodeAddPaste();

	void update_item(HTREEITEM handle);

	int& flag = _model.flag;
	int*& modified = _model.modified;

	// Fallback editor interface installed at construction so _model._interface is never
	// null (shared model/OPF code dereferences it unconditionally).  Dialogs that provide
	// editor context (events, goals, cutscenes, campaign) overwrite it with themselves.
	SexpTreeEditorInterface m_default_interface;

	bool m_operator_popup_active;
	bool m_operator_popup_created;
	int m_font_height;
	int m_font_max_width;

	SCP_vector<sexp_tree_item>& tree_nodes = _model.tree_nodes;
	int& total_nodes = _model.total_nodes;

	HTREEITEM item_handle;
	int& root_item = _model.root_item;

	int m_add_count = 0;
	int m_replace_count = 0;
	int m_modify_variable = 0;

	DECLARE_MESSAGE_MAP()
};

#endif
