/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/ShipGoalsDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Initial orders editor dialog box handling code.  This dialog is used for both
 * ship and wing initial orders, and can support more if need be without modification.
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
 * 13    8/26/97 4:18p Hoffoss
 * Added error checking to initial orders dialog when ok is clicked.
 * 
 * 12    7/30/97 12:31p Hoffoss
 * Made improvements to ship goals editor (initial orders) to disallow
 * illegal orders.
 * 
 * 11    5/30/97 4:50p Hoffoss
 * Added code to allow marked ship editing of data in child dialogs of
 * ship editor dialog.
 * 
 * 10    3/10/97 5:37p Hoffoss
 * fixed bug in dock goal selection.
 * 
 * 9     3/03/97 4:32p Hoffoss
 * Initial orders supports new docking stuff Allender added.
 * 
 * 8     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "Management.h"

#ifndef _SHIPGOALSDLG_H
#define _SHIPGOALSDLG_H

#define ED_MAX_GOALS 10
#define MAX_EDITOR_GOAL_PRIORITY	200

/////////////////////////////////////////////////////////////////////////////
// ShipGoalsDlg dialog

class ShipGoalsDlg : public CDialog
{
// Construction
public:
	int verify_orders(int ship = -1);
	void initialize_multi();
	void OnOK();
	void update();
	void initialize(ai_goal *goals, int ship = cur_ship);
	ShipGoalsDlg(CWnd* pParent = NULL);   // standard constructor

	int self_ship, self_wing;
	int m_behavior[ED_MAX_GOALS];
	int m_object[ED_MAX_GOALS];
	int m_priority[ED_MAX_GOALS];
	int m_subsys[ED_MAX_GOALS];
	int m_dock2[ED_MAX_GOALS];
	int m_data[ED_MAX_GOALS];

	CComboBox *m_behavior_box[ED_MAX_GOALS];
	CComboBox *m_object_box[ED_MAX_GOALS];
	CComboBox *m_subsys_box[ED_MAX_GOALS];
	CComboBox *m_dock2_box[ED_MAX_GOALS];
	CComboBox *m_priority_box[ED_MAX_GOALS];

// Dialog Data
	//{{AFX_DATA(ShipGoalsDlg)
	enum { IDD = IDD_SHIP_GOALS_EDITOR };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ShipGoalsDlg)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ShipGoalsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBehavior1();
	afx_msg void OnSelchangeBehavior2();
	afx_msg void OnSelchangeBehavior3();
	afx_msg void OnSelchangeBehavior4();
	afx_msg void OnSelchangeBehavior5();
	afx_msg void OnSelchangeBehavior6();
	afx_msg void OnSelchangeBehavior7();
	afx_msg void OnSelchangeBehavior8();
	afx_msg void OnSelchangeBehavior9();
	afx_msg void OnSelchangeBehavior10();
	afx_msg void OnSelchangeObject1();
	afx_msg void OnSelchangeObject2();
	afx_msg void OnSelchangeObject3();
	afx_msg void OnSelchangeObject4();
	afx_msg void OnSelchangeObject5();
	afx_msg void OnSelchangeObject6();
	afx_msg void OnSelchangeObject7();
	afx_msg void OnSelchangeObject8();
	afx_msg void OnSelchangeObject9();
	afx_msg void OnSelchangeObject10();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void set_item(int item, int init = 0);
	void update_item(int item, int multi = 0);
	void set_object(int item);

	ai_goal *goalp;
};

extern char *goal_behaviors[];

#endif
