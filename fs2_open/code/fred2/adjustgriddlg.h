/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/AdjustGridDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Editor to allow one to change Fred's grid orientation and position.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.2  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.1.1.1  2002/07/15 03:10:52  inquisitor
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
 * 1     10/07/98 2:59p Dave
 * 
 * 3     8/18/97 10:01p Hoffoss
 * Improved dialog by graying out fields that don't have any effect on
 * current plane setting.
 * 
 * 2     8/18/97 9:31p Hoffoss
 * Added grid adjustment dialog and shield system editor dialog.
 *
 * $NoKeywords: $
 */

/////////////////////////////////////////////////////////////////////////////
// adjust_grid_dlg dialog

class adjust_grid_dlg : public CDialog
{
// Construction
public:
	void OnOK();
	adjust_grid_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(adjust_grid_dlg)
	enum { IDD = IDD_ADJUST_GRID };
	CSpinButtonCtrl	m_spinz;
	CSpinButtonCtrl	m_spiny;
	CSpinButtonCtrl	m_spinx;
	int		m_x;
	int		m_y;
	int		m_z;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(adjust_grid_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(adjust_grid_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnXyPlane();
	afx_msg void OnXzPlane();
	afx_msg void OnYzPlane();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
