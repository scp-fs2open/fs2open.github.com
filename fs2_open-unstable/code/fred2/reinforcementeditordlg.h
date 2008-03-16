/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/ReinforcementEditorDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Reinforcements editor dialog handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.3  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:01  inquisitor
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
 * 9     5/23/98 3:33p Hoffoss
 * Removed unused code in reinforcements editor and make ships.tbl button
 * in ship editor disappear in release build.
 * 
 * 8     7/16/97 11:02p Allender
 * added messaging for reinforcements.  One (or one of several) can now
 * play if reinforcement are not yet available, or when they are arriving
 * 
 * 7     5/20/97 2:29p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
 * 
 * 6     4/29/97 3:02p Hoffoss
 * Reinforcement type is now automatically handled by Fred.
 * 
 * 5     4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 4     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 3     2/04/97 3:10p Hoffoss
 * Reinforcements editor fully implemented.
 * 
 * 2     2/03/97 1:32p Hoffoss
 * Reinforcement editor functional, but still missing a few options.
 * Checking in good code now prior to experimenting, so I can revert if
 * needed.
 *
 * $NoKeywords: $
 */

#include "ship/ship.h"

/////////////////////////////////////////////////////////////////////////////
// reinforcement_editor_dlg dialog

class reinforcement_editor_dlg : public CDialog
{
// Construction
public:
	int query_modified();
	void OnOK();
	void OnCancel();
	void save_data();
	void update_data();
	reinforcement_editor_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(reinforcement_editor_dlg)
	enum { IDD = IDD_REINFORCEMENT_EDITOR };
	CSpinButtonCtrl	m_delay_spin;
	CSpinButtonCtrl	m_uses_spin;
	int		m_uses;
	int		m_delay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(reinforcement_editor_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void remove_selected( CListBox *box );
	void move_messages( CListBox *box );

	// Generated message map functions
	//{{AFX_MSG(reinforcement_editor_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_num_reinforcements;
	reinforcements m_reinforcements[MAX_REINFORCEMENTS];
	int cur;
};

/////////////////////////////////////////////////////////////////////////////
// reinforcement_select dialog

class reinforcement_select : public CDialog
{
// Construction
public:
	int cur;
	char name[NAME_LENGTH];
	reinforcement_select(CWnd* pParent = NULL);   // standard constructor
	void OnOK();
	void OnCancel();

// Dialog Data
	//{{AFX_DATA(reinforcement_select)
	enum { IDD = IDD_REINFORCEMENT_SELECT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(reinforcement_select)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(reinforcement_select)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
