/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
