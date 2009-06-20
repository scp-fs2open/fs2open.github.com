/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// CPrefsDlg dialog

class CPrefsDlg : public CDialog
{
// Construction
public:
	CPrefsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsDlg)
	enum { IDD = IDD_PREFERENCES };
	BOOL	m_ConfirmDeleting;
	BOOL	m_ShowCapitalShips;
	BOOL	m_ShowElevations;
	BOOL	m_ShowFighters;
	BOOL	m_ShowGrid;
	BOOL	m_ShowMiscObjects;
	BOOL	m_ShowPlanets;
	BOOL	m_ShowWaypoints;
	BOOL	m_ShowStarfield;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsDlg)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsDlg)
	afx_msg void OnSaveDefaultPrefs();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
