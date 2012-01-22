/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// CampaignEditorDlg.h : header file
//

#include "resource.h"
#include "Sexp_tree.h"
#include "CampaignFilelistBox.h"

/////////////////////////////////////////////////////////////////////////////
// campaign_editor form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class campaign_sexp_tree : public sexp_tree
{
};

class campaign_editor : public CFormView
{
private:
	int m_num_links;
	int m_last_mission;

protected:
	campaign_editor();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(campaign_editor)

// Form Data
public:
	void mission_selected(int num);
	void insert_handler(int old, int node);
	void swap_handler(int node1, int node2);
	void update();
	void load_tree(int save = 1);
	void save_tree(int clear = 1);
	int handler(int code, int node, char *str = NULL);
	void initialize( int init_files = 1 );
	void load_campaign();
	void update_loop_desc_window();
	void campaign_editor::save_loop_desc_window();
	//{{AFX_DATA(campaign_editor)
	enum { IDD = IDD_CAMPAIGN };
	campaign_sexp_tree	m_tree;
	campaign_filelist_box	m_filelist;
	CString	m_name;
	int		m_type;
	CString	m_num_players;
	CString	m_desc;
	CString	m_branch_desc;
	CString	m_branch_brief_anim;
	CString	m_branch_brief_sound;
	BOOL	m_custom_tech_db;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(campaign_editor)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~campaign_editor();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(campaign_editor)
	afx_msg void OnLoad();
	afx_msg void OnAlign();
	afx_msg void OnCpgnClose();
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditSexpTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditSexpTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedSexpTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnEndEdit();
	afx_msg void OnChangeBriefingCutscene();
	afx_msg void OnSelchangeType();
	afx_msg void MainHallChange();
	afx_msg void OnToggleLoop();
	afx_msg void OnBrowseLoopAni();
	afx_msg void OnBrowseLoopSound();
	afx_msg void OnChangeMainHall();
	afx_msg void OnChangeDebriefingPersona();
	afx_msg void OnCustomTechDB();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern campaign_editor *Campaign_tree_formp;
extern int Cur_campaign_mission;
extern int Cur_campaign_link;
