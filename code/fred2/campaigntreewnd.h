/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// campaign_tree_wnd window

class campaign_tree_wnd : public CFrameWnd
{
	DECLARE_DYNCREATE(campaign_tree_wnd)

// Construction
public:
	campaign_tree_wnd();

// Attributes
public:

// Operations
public:
	int error_checker();
	int fred_check_sexp(int sexp, int type, char *msg, ...);
	int error(char *msg, ...);
	int internal_error(char *msg, ...);
	int save_modified();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(campaign_tree_wnd)
	//}}AFX_VIRTUAL

private:
	int g_err;

// Implementation
public:
	virtual ~campaign_tree_wnd();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	// Generated message map functions
	CSplitterWnd m_splitter;

	//{{AFX_MSG(campaign_tree_wnd)
	afx_msg void OnUpdateCpgnFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnCpgnFileOpen();
	afx_msg void OnDestroy();
	afx_msg void OnCpgnFileSave();
	afx_msg void OnCpgnFileSaveAs();
	afx_msg void OnCpgnFileNew();
	afx_msg void OnClose2();
	afx_msg void OnErrorChecker();
	afx_msg void OnClose();
	afx_msg void OnInitialShips();
	afx_msg void OnInitialWeapons();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern int Mission_filename_cb_format;
extern int Campaign_modified;
extern int Bypass_clear_mission;
extern campaign_tree_wnd *Campaign_wnd;
