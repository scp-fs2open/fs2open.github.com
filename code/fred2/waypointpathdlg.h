/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/WaypointPathDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Waypoint editor
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.2  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:02p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 4     5/23/97 1:53p Hoffoss
 * Fixed problems with modeless dialog updating.  It won't get caught in
 * an infinate loop anymore, but still gives an error warning 3 times when
 * using cancel and trying to switch window focus to main window.  Don't
 * know if I can fix that, but it's not too critical right now.
 * 
 * 3     3/13/97 12:09p Hoffoss
 * Waypoint path editor finished (apparently I didn't get around to
 * completing it before).
 * 
 * 2     2/12/97 12:26p Hoffoss
 * Expanded on global error checker, added initial orders conflict
 * checking and warning, added waypoint editor dialog and code.
 *
 * $NoKeywords: $
 */

#ifndef _WAYPOINTPATHDLG_H
#define _WAYPOINTPATHDLG_H

/////////////////////////////////////////////////////////////////////////////
// waypoint_path_dlg dialog

class waypoint_path_dlg : public CDialog
{
// Construction
public:
	int bypass_errors;
	int update_data(int redraw = 1);
	void initialize_data(int full_update);
	void OnOK();
	BOOL Create();
	waypoint_path_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(waypoint_path_dlg)
	enum { IDD = IDD_WAYPOINT_PATH_EDITOR };
	CString	m_name;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(waypoint_path_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(waypoint_path_dlg)
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif