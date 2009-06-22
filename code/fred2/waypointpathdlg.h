/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
