/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/MissionGoalsDlg.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Mission goals editor dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.3  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
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
 * 26    5/20/98 1:04p Hoffoss
 * Made credits screen use new artwork and removed rating field usage from
 * Fred (a goal struct member).
 * 
 * 25    3/31/98 12:23a Allender
 * changed macro names of campaign types to be more descriptive.  Added
 * "team" to objectives dialog for team v. team missions.  Added two
 * distinct multiplayer campaign types
 * 
 * 24    12/08/97 2:03p Hoffoss
 * Added Fred support for MGF_NO_MUSIC flag in objectives.
 * 
 * 23    10/09/97 1:03p Hoffoss
 * Renaming events or goals now updates sexp references as well.
 * 
 * 22    8/12/97 3:33p Hoffoss
 * Fixed the "press cancel to go to reference" code to work properly.
 * 
 * 21    7/30/97 5:23p Hoffoss
 * Removed Sexp tree verification code, since it duplicates normal sexp
 * verification, and is just another set of code to keep maintained.
 * 
 * 20    7/25/97 3:05p Allender
 * added score field to goals and events editor
 * 
 * 19    7/17/97 4:10p Hoffoss
 * Added drag and drop to sexp trees for reordering root items.
 * 
 * 18    7/07/97 12:04p Allender
 * mission goal validation.
 * 
 * 17    6/02/97 8:47p Hoffoss
 * Fixed bug with inserting an operator at root position, but under a
 * label.
 * 
 * 16    5/20/97 2:28p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
 * 
 * 15    5/01/97 4:12p Hoffoss
 * Added return handling to dialogs.
 * 
 * 14    4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 13    4/11/97 10:11a Hoffoss
 * Name fields supported by Fred for Events and Mission Goals.
 * 
 * 12    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "Sexp_tree.h"
#include "mission/missiongoals.h"

/////////////////////////////////////////////////////////////////////////////
// CMissionGoalsDlg dialog

#define MAX_GOAL_ELEMENTS 300
#define OPERAND	0x01
#define EDITABLE	0x02

class sexp_goal_tree : public sexp_tree
{
public:
	int load_sub_tree(int index);
	int get_new_node_position();
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
	sexp_goal_tree	m_goals_tree;
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
