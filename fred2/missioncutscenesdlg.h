/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "Sexp_tree.h"

/////////////////////////////////////////////////////////////////////////////
// CMissionCutscenesDlg dialog

#define OPERAND	0x01
#define EDITABLE	0x02

class cutscene_sexp_tree : public sexp_tree
{
};

class CMissionCutscenesDlg : public CDialog
{
// Construction
public:
	void move_handler(int node1, int node2, bool insert_before);
	int query_modified();
	void OnCancel();
	void OnOK();
	void load_tree();
	void update_cur_cutscene();
	void create_tree();
	CMissionCutscenesDlg(CWnd* pParent = NULL); // standard constructor
	BOOL OnInitDialog();
	int handler(int code, int goal);
	void insert_handler(int old, int node);
	int select_sexp_node;

// Dialog Data
	//{{AFX_DATA(CMissionCutscenesDlg)
	enum { IDD = IDD_MISSION_CUTSCENES };
	cutscene_sexp_tree	m_cutscenes_tree;
	int		m_cutscene_type;
	int		m_display_cutscene_types;
	CString	m_name;
	CString m_desc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissionCutscenesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMissionCutscenesDlg)
	afx_msg void OnSelchangeDisplayCutsceneTypesDrop();
	afx_msg void OnSelchangedCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonNewCutscene();
	afx_msg void OnSelchangeCutsceneTypeDrop();
	afx_msg void OnChangeCutsceneName();
	afx_msg void OnOk();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int cur_cutscene;
	SCP_vector<int> m_sig;
	SCP_vector<mission_cutscene> m_cutscenes;
	int modified;
};

extern CMissionCutscenesDlg* Cutscene_editor_dlg; // global reference needed by sexp_tree class
