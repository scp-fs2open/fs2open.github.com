/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/StarfieldEditor.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Starfield editor dialog handling code
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
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 7     4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 6     4/16/97 5:18p Hoffoss
 * Moved Asteroid field editor stuff to a seperate dialog box.
 * 
 * 5     3/17/97 3:00p Hoffoss
 * slider updates on the fly now.
 * 
 * 4     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 3     1/31/97 3:16p Hoffoss
 * Asteroid field management implemented.
 *
 * $NoKeywords: $
 */

/////////////////////////////////////////////////////////////////////////////
// starfield_editor dialog

class starfield_editor : public CDialog
{
// Construction
public:
	void OnOK();
	void OnCancel();
	starfield_editor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(starfield_editor)
	enum { IDD = IDD_STARFIELD };
	CSliderCtrl			m_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(starfield_editor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	int initialized;

	// Generated message map functions
	//{{AFX_MSG(starfield_editor)
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableAsteroids();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
