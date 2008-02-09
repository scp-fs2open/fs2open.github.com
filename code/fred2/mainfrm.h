/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/fred2/MainFrm.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * MainFrm.h : interface of the CMainFrame class
 * The main frame class of a document/view architechure, which we hate but must
 * deal with, due to Microsoft limiting our freedom and forcing us to use whether
 * we want to or not.  The main frame is basically the container window that other
 * view windows are within.  In Fred, our view window is always maximized inside
 * the main frame window, so you can't tell the difference between the two.  A few
 * old MFC events are handled here because the people working on the code before
 * me (Hoffoss) decided to put it here.  I've been putting it all in FredView.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.2  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.1.1.1  2002/07/15 03:10:59  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 3     9/08/99 12:07a Andsager
 * Add browser based help to Fred
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 14    9/16/98 6:54p Dave
 * Upped  max sexpression nodes to 1800 (from 1600). Changed FRED to sort
 * the ship list box. Added code so that tracker stats are not stored with
 * only 1 player.
 * 
 * 13    8/17/97 10:22p Hoffoss
 * Fixed several bugs in Fred with Undo feature.  In the process, recoded
 * a lot of CFile.cpp.
 * 
 * 12    7/21/97 3:57p Hoffoss
 * Removed group combo box from toolbar, since I don't think I'll ever get
 * it working right.
 * 
 * 11    6/09/97 4:57p Hoffoss
 * Added autosave and undo to Fred.
 * 
 * 10    5/05/97 1:35p Hoffoss
 * View window is now refocused when a new ship type selection is made.
 * 
 * 9     3/10/97 12:54p Hoffoss
 * Added drop down combo box to toolbar and fixed compiling errors Mark
 * (maybe Mike?) introduced to code.
 * 
 * 8     2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 7     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
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
