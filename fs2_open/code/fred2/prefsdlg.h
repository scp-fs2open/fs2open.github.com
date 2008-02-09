/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/PrefsDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Fred Preferences dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.2  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.1.1.1  2002/07/15 03:11:00  inquisitor
 * Initial FRED2 Checking
 *
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
 * 2     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
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
