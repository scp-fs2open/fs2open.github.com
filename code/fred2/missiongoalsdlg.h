/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "Sexp_tree.h"
#include "mission/missiongoals.h"

/////////////////////////////////////////////////////////////////////////////
// CMissionGoalsDlg dialog

#define MAX_GOAL_ELEMENTS 300
#define OPERAND	0x01
#define EDITABLE	0x02

class goal_sexp_tree : public sexp_tree
{
};

class CMissionGoalsDlg : public CDialog
{
// Construction
public:
	void swap_handler(int node1, int node2);
	int query_modified();
	void OnCancel();
	void OnOK();
	void load_tree();
	void update_cur_goal();
	void add_sub_tree(int node, HTREEITEM root);
	void create_tree();
	CMissionGoalsDlg(CWnd* pParent = NULL);   // standard constructor
	BOOL OnInitDialog();
	int handler(int code, int goal);
	void insert_handler(int old, int node);
	int select_sexp_node;

// Dialog Data
	//{{AFX_DATA(CMissionGoalsDlg)
	enum { IDD = IDD_MISSION_GOALS };
	goal_sexp_tree	m_goals_tree;
	CString	m_goal_desc;
	int		m_goal_type;
	int		m_display_goal_types;
	CString	m_name;
	BOOL	m_goal_invalid;
	int		m_goal_score;
	BOOL	m_no_music;
	int		m_team;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissionGoalsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMissionGoalsDlg)
	afx_msg void OnSelchangeDisplayGoalTypesDrop();
	afx_msg void OnSelchangedGoalsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickGoalsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditGoalsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditGoalsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonNewGoal();
	afx_msg void OnChangeGoalDesc();
	afx_msg void OnChangeGoalRating();
	afx_msg void OnSelchangeGoalTypeDrop();
	afx_msg void OnChangeGoalName();
	afx_msg void OnOk();
	afx_msg void OnClose();
	afx_msg void OnGoalInvalid();
	afx_msg void OnChangeGoalScore();
	afx_msg void OnNoMusic();
	afx_msg void OnSelchangeTeam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int cur_goal;
	int m_num_goals;
	int m_sig[MAX_GOALS];
	mission_goal m_goals[MAX_GOALS];
	int modified;
};

extern CMissionGoalsDlg *Goal_editor_dlg; // global reference needed by sexp_tree class
