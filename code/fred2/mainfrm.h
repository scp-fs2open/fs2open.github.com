/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



//class CShipEditorDlg;

#define	WM_MENU_POPUP_TEST	(WM_USER+9)

class color_combo_box : public CComboBox
{
	void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	int	CalcMinimumItemHeight();
	void	MeasureItem(LPMEASUREITEMSTRUCT);

public :	
	int SetCurSelNEW(int model_index);
	int GetCurSelNEW();
};

class CMainFrame : public CFrameWnd
{
private:
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	void init_tools();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
	CToolBar    m_wndToolBar;
	CStatusBar  m_wndStatusBar;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditorsAiClasses();
	afx_msg void OnEditorsGoals();
	afx_msg void OnEditorsArt();
	afx_msg void OnEditorsMusic();
	afx_msg void OnEditorsShipClasses();
	afx_msg void OnEditorsSound();
	afx_msg void OnEditorsTerrain();
	afx_msg void OnFileMissionnotes();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnViewStatusBar();
	afx_msg void OnUpdateViewStatusBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLeft( CCmdUI* pCmdUI);
	afx_msg void OnUpdateRight( CCmdUI* pCmdUI);
	afx_msg void OnMikeGridcontrol();
	afx_msg void OnMenuPopupToggle1();
	afx_msg void OnUpdateMenuPopupToggle1(CCmdUI* pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHelpInputInterface();
	afx_msg void OnClose();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnFredHelp();
	//}}AFX_MSG

	afx_msg void OnNewShipTypeChange();
	LONG OnMenuPopupTest(UINT wParam, LONG lParam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern CMainFrame *Fred_main_wnd;
extern color_combo_box	m_new_ship_type_combo_box;
