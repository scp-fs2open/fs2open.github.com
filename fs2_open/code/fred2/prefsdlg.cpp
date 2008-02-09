/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/PrefsDlg.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Fred Preferences dialog box handling code
 *
 * $Log: not supported by cvs2svn $
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
 * 3     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "PrefsDlg.h"
#include "FREDDoc.h"
#include "FREDView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrefsDlg dialog



CPrefsDlg::CPrefsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrefsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrefsDlg)
	m_ConfirmDeleting = TRUE;
	m_ShowCapitalShips = TRUE;
	m_ShowElevations = TRUE;
	m_ShowFighters = TRUE;
	m_ShowGrid = TRUE;
	m_ShowMiscObjects = TRUE;
	m_ShowPlanets = TRUE;
	m_ShowWaypoints = TRUE;
	m_ShowStarfield = FALSE;
	//}}AFX_DATA_INIT
}

extern int Show_stars;

void CPrefsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDlg)
	DDX_Check(pDX, ID_CONFIRM_DELETING, m_ConfirmDeleting);
	DDX_Check(pDX, ID_SHOW_CAPITALSHIPS, m_ShowCapitalShips);
	DDX_Check(pDX, ID_SHOW_ELEVATIONS, m_ShowElevations);
	DDX_Check(pDX, ID_SHOW_FIGHTERS, m_ShowFighters);
	DDX_Check(pDX, ID_SHOW_GRID, m_ShowGrid);
	DDX_Check(pDX, ID_SHOW_MISCOBJECTS, m_ShowMiscObjects);
	DDX_Check(pDX, ID_SHOW_PLANETS, m_ShowPlanets);
	DDX_Check(pDX, ID_SHOW_WAYPOINTS, m_ShowWaypoints);
	DDX_Check(pDX, IDC_PREF_STARFIELD, m_ShowStarfield);
	//}}AFX_DATA_MAP

	Show_stars = m_ShowStarfield;
	// CFREDView::SetViewParms(m_ConfirmDeleting);
}


BEGIN_MESSAGE_MAP(CPrefsDlg, CDialog)
	//{{AFX_MSG_MAP(CPrefsDlg)
	ON_BN_CLICKED(IDC_SAVE_DEFAULT_PREFS, OnSaveDefaultPrefs)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDlg message handlers

void CPrefsDlg::OnSaveDefaultPrefs() 
{
	// Put code to save user prefs here.
	
	m_ConfirmDeleting = 1;
}

void CPrefsDlg::OnClose() 
{
	// MessageBeep((WORD) -1);
	
	CDialog::OnClose();
}

BOOL CPrefsDlg::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// MessageBeep((WORD) -1);
	
	return CDialog::Create(IDD, pParentWnd);
}
