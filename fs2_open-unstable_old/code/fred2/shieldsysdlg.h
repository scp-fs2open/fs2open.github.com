/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/ShieldSysDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Shield generator system editor.  This dialog allows one to indicate whether all ships
 * (on a certain team or of a certain type) have a shield system or not.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
 *
 * Revision 1.6  2005/12/29 08:21:00  wmcoolmon
 * No my widdle FRED, I didn't forget about you ^_^ (codebase commit)
 *
 * Revision 1.5  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.4  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.3  2003/09/05 05:32:36  Goober5000
 * moved custom buttons from shield sys dialog and added dialog to set
 * ship flags globally
 * --Goober5000
 *
 * Revision 1.2  2003/08/26 06:39:34  Goober5000
 * added special buttons on shield dialog to edit global ship flags
 * --Goober5000
 *
 * Revision 1.1.1.1  2002/07/15 03:11:02  inquisitor
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
 * 2     8/18/97 9:31p Hoffoss
 * Added grid adjustment dialog and shield system editor dialog.
 *
 * $NoKeywords: $
 */

extern int Shield_sys_teams[MAX_IFFS];
extern int Shield_sys_types[MAX_SHIP_CLASSES];

/////////////////////////////////////////////////////////////////////////////
// shield_sys_dlg dialog

class shield_sys_dlg : public CDialog
{
// Construction
public:
	void set_team();
	void set_type();
	shield_sys_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(shield_sys_dlg)
	enum { IDD = IDD_SHIELD_SYS };
	int		m_team;
	int		m_type;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(shield_sys_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(shield_sys_dlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTeam();
	afx_msg void OnSelchangeType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
