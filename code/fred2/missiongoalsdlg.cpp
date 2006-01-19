/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/fred2/MissionGoalsDlg.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Mission goals editor dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:00  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 4     2/17/99 2:11p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 3     1/19/99 3:57p Andsager
 * Round 2 of variables
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
 * 47    5/23/98 12:20p Sandeep
 * 
 * 46    5/22/98 1:06a Hoffoss
 * Made Fred not use OLE.
 * 
 * 45    5/20/98 1:04p Hoffoss
 * Made credits screen use new artwork and removed rating field usage from
 * Fred (a goal struct member).
 * 
 * 44    3/31/98 12:23a Allender
 * changed macro names of campaign types to be more descriptive.  Added
 * "team" to objectives dialog for team v. team missions.  Added two
 * distinct multiplayer campaign types
 * 
 * 43    12/08/97 2:03p Hoffoss
 * Added Fred support for MGF_NO_MUSIC flag in objectives.
 * 
 * 42    11/11/97 4:13p Duncan
 * changed assert to abort operation instead.
 * 
 * 41    10/10/97 6:21p Hoffoss
 * Put in Fred support for training object list editing.
 * 
 * 40    10/10/97 2:53p Johnson
 * Fixed bug with new items being selected before they are fully
 * registered as added.
 * 
 * 39    10/09/97 1:03p Hoffoss
 * Renaming events or goals now updates sexp references as well.
 * 
 * 38    9/30/97 12:30p Hoffoss
 * Drag and drop reordering of sexp tree roots now does an insert after
 * rather than a swap operation.
 * 
 * 37    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 36    8/12/97 3:33p Hoffoss
 * Fixed the "press cancel to go to reference" code to work properly.
 * 
 * 35    8/12/97 2:39p Johnson
 * fixed bug with sexp tree goal name problem.
 * 
 * 34    8/01/97 3:10p Hoffoss
 * Made Sexp help hidable.
 * 
 * 33    7/30/97 5:23p Hoffoss
 * Removed Sexp tree verification code, since it duplicates normal sexp
 * verification, and is just another set of code to keep maintained.
 * 
 * 32    7/25/97 3:05p Allender
 * added score field to goals and events editor
 * 
 * 31    7/25/97 2:40p Hoffoss
 * Fixed bug in sexp tree selection updating handling.
 * 
 * 30    7/24/97 12:45p Hoffoss
 * Added sexp help system to sexp trees and some dialog boxes.
 * 
 * 29    7/17/97 4:10p Hoffoss
 * Added drag and drop to sexp trees for reordering root items.
 * 
 * 28    7/16/97 6:30p Hoffoss
 * Added icons to sexp trees, mainly because I think they will be required
 * for drag n drop.
 * 
 * 27    7/07/97 12:04p Allender
 * mission goal validation.
 * 
 * 26    6/02/97 8:47p Hoffoss
 * Fixed bug with inserting an operator at root position, but under a
 * label.
 * 
 * 25    5/20/97 2:28p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
 * 
 * 24    5/01/97 4:12p Hoffoss
 * Added return handling to dialogs.
 * 
 * 23    4/25/97 12:50p Allender
 * change globals to new naming conventions
 * 
 * 22    4/23/97 11:55a Hoffoss
 * Fixed many bugs uncovered while trying to create Mission 6.
 * 
 * 21    4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 20    4/11/97 4:22p Hoffoss
 * Fixed bug in Sexp trees, moved Show starfield option to view menu and
 * removed preferences dialog box.
 * 
 * 19    4/11/97 10:11a Hoffoss
 * Name fields supported by Fred for Events and Mission Goals.
 * 
 * 18    4/10/97 3:20p Mike
 * Change hull damage to be like shields.
 * 
 * 17    4/01/97 5:15p Hoffoss
 * Fixed errors in max length checks, renaming a wing now renames the
 * ships in the wing as well, as it should.
 * 
 * 16    2/21/97 5:34p Hoffoss
 * Added extensive modification detection and fixed a bug in initial
 * orders editor.
 * 
 * 15    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "globalincs/linklist.h"
#include "parse/sexp.h"
#include "MissionGoalsDlg.h"
#include "Management.h"
#include "OperatorArgTypeSelect.h"

#define ID_ADD_SHIPS			9000
#define ID_REPLACE_SHIPS	11000
#define ID_ADD_WINGS			13000
#define ID_REPLACE_WINGS	15000

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMissionGoalsDlg *Goal_editor_dlg; // global reference needed by sexp_tree class

/////////////////////////////////////////////////////////////////////////////
// sexp_goal_tree class member functions

// determine the node number that would be allocated without actually allocating it yet.
int sexp_goal_tree::get_new_node_position()
{
	int i;

	for (i=0; i<MAX_SEXP_TREE_SIZE; i++)
		if (nodes[i].type == SEXPT_UNUSED)
			return i;

	return -1;
}

// construct tree nodes for an sexp, adding them to the list and returning first node
int sexp_goal_tree::load_sub_tree(int index)
{
	int cur;

	if (index < 0) {
		cur = allocate_node(-1);
		set_node(cur, (SEXPT_OPERATOR | SEXPT_VALID), "true");  // setup a default tree if none
		return cur;
	}

	// assumption: first token is an operator.  I require this because it would cause problems
	// with child/parent relations otherwise, and it should be this way anyway, since the
	// return type of the whole sexp is boolean, and only operators can satisfy this.
	Assert(Sexp_nodes[index].subtype == SEXP_ATOM_OPERATOR);
	cur = get_new_node_position();
	load_branch(index, -1);
	return cur;
}

/////////////////////////////////////////////////////////////////////////////
// CMissionGoalsDlg dialog class member functions

CMissionGoalsDlg::CMissionGoalsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMissionGoalsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissionGoalsDlg)
	m_goal_desc = _T("");
	m_goal_type = -1;
	m_display_goal_types = 0;
	m_name = _T("");
	m_goal_invalid = FALSE;
	m_goal_score = 0;
	m_no_music = FALSE;
	m_team = -1;
	//}}AFX_DATA_INIT
	m_goals_tree.m_mode = MODE_GOALS;
	m_num_goals = 0;
	m_goals_tree.link_modified(&modified);
	modified = 0;
	select_sexp_node = -1;
}

BOOL CMissionGoalsDlg::OnInitDialog()
{
	int i, adjust = 0;

	CDialog::OnInitDialog();  // let the base class do the default work
	if (!Show_sexp_help)
		adjust = -SEXP_HELP_BOX_SIZE;

	theApp.init_window(&Mission_goals_wnd_data, this, adjust);
	m_goals_tree.setup((CEdit *) GetDlgItem(IDC_HELP_BOX));
	load_tree();
	create_tree();
	if (m_num_goals >= MAX_GOALS)
		GetDlgItem(IDC_BUTTON_NEW_GOAL)->EnableWindow(FALSE);

	Goal_editor_dlg = this;
	i = m_goals_tree.select_sexp_node;
	if (i != -1) {
		GetDlgItem(IDC_GOALS_TREE) -> SetFocus();
		m_goals_tree.hilite_item(i);
		return FALSE;
	}

	return TRUE;
}

void CMissionGoalsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissionGoalsDlg)
	DDX_Control(pDX, IDC_GOALS_TREE, m_goals_tree);
	DDX_Text(pDX, IDC_GOAL_DESC, m_goal_desc);
	DDX_CBIndex(pDX, IDC_GOAL_TYPE_DROP, m_goal_type);
	DDX_CBIndex(pDX, IDC_DISPLAY_GOAL_TYPES_DROP, m_display_goal_types);
	DDX_Text(pDX, IDC_GOAL_NAME, m_name);
	DDX_Check(pDX, IDC_GOAL_INVALID, m_goal_invalid);
	DDX_Text(pDX, IDC_GOAL_SCORE, m_goal_score);
	DDX_Check(pDX, IDC_NO_MUSIC, m_no_music);
	DDX_CBIndex(pDX, IDC_OBJ_TEAM, m_team);
	//}}AFX_DATA_MAP
	DDV_MaxChars(pDX, m_goal_desc, MAX_GOAL_TEXT - 1);
	DDV_MaxChars(pDX, m_name, NAME_LENGTH - 1);
}

BEGIN_MESSAGE_MAP(CMissionGoalsDlg, CDialog)
	//{{AFX_MSG_MAP(CMissionGoalsDlg)
	ON_CBN_SELCHANGE(IDC_DISPLAY_GOAL_TYPES_DROP, OnSelchangeDisplayGoalTypesDrop)
	ON_NOTIFY(TVN_SELCHANGED, IDC_GOALS_TREE, OnSelchangedGoalsTree)
	ON_NOTIFY(NM_RCLICK, IDC_GOALS_TREE, OnRclickGoalsTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_GOALS_TREE, OnEndlabeleditGoalsTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_GOALS_TREE, OnBeginlabeleditGoalsTree)
	ON_BN_CLICKED(IDC_BUTTON_NEW_GOAL, OnButtonNewGoal)
	ON_EN_CHANGE(IDC_GOAL_DESC, OnChangeGoalDesc)
	ON_EN_CHANGE(IDC_GOAL_RATING, OnChangeGoalRating)
	ON_CBN_SELCHANGE(IDC_GOAL_TYPE_DROP, OnSelchangeGoalTypeDrop)
	ON_EN_CHANGE(IDC_GOAL_NAME, OnChangeGoalName)
	ON_BN_CLICKED(ID_OK, OnOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_GOAL_INVALID, OnGoalInvalid)
	ON_EN_CHANGE(IDC_GOAL_SCORE, OnChangeGoalScore)
	ON_BN_CLICKED(IDC_NO_MUSIC, OnNoMusic)
	ON_CBN_SELCHANGE(IDC_OBJ_TEAM, OnSelchangeTeam)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissionGoalsDlg message handlers

// Initialization: sets up internal working copy of mission goals and goal trees.
void CMissionGoalsDlg::load_tree()
{
	int i;

	m_goals_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;

	m_goals_tree.clear_tree();
	m_num_goals = Num_goals;
	for (i=0; i<Num_goals; i++) {
		m_goals[i] = Mission_goals[i];
		m_sig[i] = i;
		if (!(*m_goals[i].name))
			strcpy(m_goals[i].name, "<unnamed>");

		m_goals[i].formula = m_goals_tree.load_sub_tree(Mission_goals[i].formula);
	}

	m_goals_tree.post_load();
	cur_goal = -1;
	update_cur_goal();
}

// create the CTreeCtrl tree from the goal tree, filtering based on m_display_goal_types
void CMissionGoalsDlg::create_tree()
{
	int i;
	HTREEITEM h;

	m_goals_tree.DeleteAllItems();
	m_goals_tree.reset_handles();
	for (i=0; i<m_num_goals; i++) {
		if ( (m_goals[i].type & GOAL_TYPE_MASK) != m_display_goal_types)
			continue;

		h = m_goals_tree.insert(m_goals[i].name);
		m_goals_tree.SetItemData(h, m_goals[i].formula);
		m_goals_tree.add_sub_tree(m_goals[i].formula, h);
	}

	cur_goal = -1;
	update_cur_goal();
}

// Display goal types selection changed, so update the display
void CMissionGoalsDlg::OnSelchangeDisplayGoalTypesDrop() 
{
	UpdateData(TRUE);
	create_tree();
}

// New tree item selected.  Because goal info is displayed for the selected tree item,
// we need to update the display when this occurs.
void CMissionGoalsDlg::OnSelchangedGoalsTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int i, z;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM h, h2;

	h = pNMTreeView->itemNew.hItem;
	if (!h)
		return;

	m_goals_tree.update_help(h);
	while ((h2 = m_goals_tree.GetParentItem(h)) != 0)
		h = h2;

	z = m_goals_tree.GetItemData(h);
	for (i=0; i<m_num_goals; i++)
		if (m_goals[i].formula == z)
			break;

	Assert(i < m_num_goals);
	cur_goal = i;
	update_cur_goal();
	*pResult = 0;
}

// update display info to reflect the currently selected goal.
void CMissionGoalsDlg::update_cur_goal()
{
	if (cur_goal < 0) {
		m_name = _T("");
		m_goal_desc = _T("");
		m_goal_type = -1;
		m_team = 0;
		UpdateData(FALSE);
		GetDlgItem(IDC_GOAL_TYPE_DROP) -> EnableWindow(FALSE);
		GetDlgItem(IDC_GOAL_NAME) -> EnableWindow(FALSE);
		GetDlgItem(IDC_GOAL_DESC) -> EnableWindow(FALSE);
		GetDlgItem(IDC_GOAL_INVALID)->EnableWindow(FALSE);
		GetDlgItem(IDC_GOAL_SCORE)->EnableWindow(FALSE);
		GetDlgItem(IDC_NO_MUSIC)->EnableWindow(FALSE);
		GetDlgItem(IDC_OBJ_TEAM)->EnableWindow(FALSE);
		return;
	}

	m_name = _T(m_goals[cur_goal].name);
	m_goal_desc = _T(m_goals[cur_goal].message);
	m_goal_type = m_goals[cur_goal].type & GOAL_TYPE_MASK;
	if ( m_goals[cur_goal].type & INVALID_GOAL ){
		m_goal_invalid = 1;
	} else {
		m_goal_invalid = 0;
	}

	if ( m_goals[cur_goal].flags & MGF_NO_MUSIC ){
		m_no_music = 1;
	} else {
		m_no_music = 0;
	}

	m_goal_score = m_goals[cur_goal].score;

	m_team = m_goals[cur_goal].team;

	UpdateData(FALSE);
	GetDlgItem(IDC_GOAL_TYPE_DROP) -> EnableWindow(TRUE);
	GetDlgItem(IDC_GOAL_NAME) -> EnableWindow(TRUE);
	GetDlgItem(IDC_GOAL_DESC) -> EnableWindow(TRUE);
//	GetDlgItem(IDC_GOAL_RATING) -> EnableWindow(TRUE);
	GetDlgItem(IDC_GOAL_INVALID)->EnableWindow(TRUE);
	GetDlgItem(IDC_GOAL_SCORE)->EnableWindow(TRUE);
	GetDlgItem(IDC_NO_MUSIC)->EnableWindow(TRUE);
	GetDlgItem(IDC_OBJ_TEAM)->EnableWindow(FALSE);
	if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ){
		GetDlgItem(IDC_OBJ_TEAM)->EnableWindow(TRUE);
	}
}

// handler for context menu (i.e. a right mouse button click).
void CMissionGoalsDlg::OnRclickGoalsTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_goals_tree.right_clicked(MODE_GOALS);
	*pResult = 0;
}

// goal tree item label editing is requested.  Determine if it should be allowed.
void CMissionGoalsDlg::OnBeginlabeleditGoalsTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_goals_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;
	} else {
		*pResult = 1;
	}
}

// Once we finish editing, we need to clean up, which we do here.
void CMissionGoalsDlg::OnEndlabeleditGoalsTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_goals_tree.end_label_edit(pTVDispInfo->item.hItem, pTVDispInfo->item.pszText);
}

void CMissionGoalsDlg::OnOK()
{
	HWND h;
	CWnd *w;

	w = GetFocus();
	if (w) {
		h = w->m_hWnd;
		GetDlgItem(IDC_GOALS_TREE)->SetFocus();
		::SetFocus(h);
	}
}

int CMissionGoalsDlg::query_modified()
{
	int i;

	if (modified)
		return 1;

	if (Num_goals != m_num_goals)
		return 1;

	for (i=0; i<Num_goals; i++) {
		if (stricmp(Mission_goals[i].name, m_goals[i].name))
			return 1;
		if (stricmp(Mission_goals[i].message, m_goals[i].message))
			return 1;
		if (Mission_goals[i].type != m_goals[i].type)
			return 1;
		if ( Mission_goals[i].score != m_goals[i].score )
			return 1;
		if ( Mission_goals[i].team != m_goals[i].team )
			return 1;
	}

	return 0;
}

void CMissionGoalsDlg::OnOk()
{
	char buf[256], names[2][MAX_GOALS][NAME_LENGTH];
	int i, count;

	for (i=0; i<Num_goals; i++)
		free_sexp2(Mission_goals[i].formula);

	UpdateData(TRUE);
	if (query_modified())
		set_modified();

	count = 0;
	for (i=0; i<Num_goals; i++)
		Mission_goals[i].satisfied = 0;  // use this as a processed flag
	
	// rename all sexp references to old events
	for (i=0; i<m_num_goals; i++)
		if (m_sig[i] >= 0) {
			strcpy(names[0][count], Mission_goals[m_sig[i]].name);
			strcpy(names[1][count], m_goals[i].name);
			count++;
			Mission_goals[m_sig[i]].satisfied = 1;
		}

	// invalidate all sexp references to deleted events.
	for (i=0; i<Num_goals; i++)
		if (!Mission_goals[i].satisfied) {
			sprintf(buf, "<%s>", Mission_goals[i].name);
			strcpy(buf + NAME_LENGTH - 2, ">");  // force it to be not too long
			strcpy(names[0][count], Mission_goals[i].name);
			strcpy(names[1][count], buf);
			count++;
		}

	Num_goals = m_num_goals;
	for (i=0; i<Num_goals; i++) {
		Mission_goals[i] = m_goals[i];
		Mission_goals[i].formula = m_goals_tree.save_tree(Mission_goals[i].formula);
		if ( The_mission.game_type & MISSION_TYPE_MULTI_TEAMS ) {
			Assert( Mission_goals[i].team != -1 );
		}
	}

	// now update all sexp references
	while (count--)
		update_sexp_references(names[0][count], names[1][count], OPF_GOAL_NAME);

	theApp.record_window_data(&Mission_goals_wnd_data, this);
	CDialog::OnOK();
}

void CMissionGoalsDlg::OnButtonNewGoal() 
{
	int index;
	HTREEITEM h;

	Assert(m_num_goals < MAX_GOALS);
	m_goals[m_num_goals].type = m_display_goal_types;			// this also marks the goal as valid since bit not set
	m_sig[m_num_goals] = -1;
	strcpy(m_goals[m_num_goals].name, "Goal name");
	strcpy(m_goals[m_num_goals].message, "Mission goal text");
	h = m_goals_tree.insert(m_goals[m_num_goals].name);

	m_goals_tree.item_index = -1;
	m_goals_tree.add_operator("true", h);
	m_goals[m_num_goals].score = 0;
	index = m_goals[m_num_goals].formula = m_goals_tree.item_index;
	m_goals_tree.SetItemData(h, index);

	// team defaults to the first team.
	m_goals[m_num_goals].team = 0;

	m_num_goals++;

	if (m_num_goals >= MAX_GOALS){
		GetDlgItem(IDC_BUTTON_NEW_GOAL)->EnableWindow(FALSE);
	}

	m_goals_tree.SelectItem(h);
}

int CMissionGoalsDlg::handler(int code, int node)
{
	int goal;

	switch (code) {
	case ROOT_DELETED:
		for (goal=0; goal<m_num_goals; goal++){
			if (m_goals[goal].formula == node){
				break;
			}
		}

		Assert(goal < m_num_goals);
		while (goal < m_num_goals - 1) {
			m_goals[goal] = m_goals[goal + 1];
			m_sig[goal] = m_sig[goal + 1];
			goal++;
		}
		m_num_goals--;
		GetDlgItem(IDC_BUTTON_NEW_GOAL)->EnableWindow(TRUE);
		return node;

	default:
		Int3();
	}

	return -1;
}

void CMissionGoalsDlg::OnChangeGoalDesc() 
{
	if (cur_goal < 0){
		return;
	}

	UpdateData(TRUE);
	string_copy(m_goals[cur_goal].message, m_goal_desc, MAX_GOAL_TEXT);
}

void CMissionGoalsDlg::OnChangeGoalRating() 
{
	if (cur_goal < 0){
		return;
	}

	UpdateData(TRUE);
}

void CMissionGoalsDlg::OnSelchangeGoalTypeDrop() 
{
	HTREEITEM h, h2;
	int otype;

	if (cur_goal < 0){
		return;
	}

	UpdateData(TRUE);
	UpdateData(TRUE);  // doesn't seem to update unless we do it twice..

	// change the type being sure to keep the invalid bit if set
	otype = m_goals[cur_goal].type;
	m_goals[cur_goal].type = m_goal_type;
	if ( otype & INVALID_GOAL ){
		m_goals[cur_goal].type |= INVALID_GOAL;
	}

	h = m_goals_tree.GetSelectedItem();
	Assert(h);
	while ((h2 = m_goals_tree.GetParentItem(h)) != 0){
		h = h2;
	}

	m_goals_tree.DeleteItem(h);
	cur_goal = -1;
	update_cur_goal();
}

void CMissionGoalsDlg::OnChangeGoalName() 
{
	HTREEITEM h, h2;

	if (cur_goal < 0){
		return;
	}

	UpdateData(TRUE);
	h = m_goals_tree.GetSelectedItem();
	if (!h){
		return;
	}

	while ((h2 = m_goals_tree.GetParentItem(h)) != 0){
		h = h2;
	}

	m_goals_tree.SetItemText(h, m_name);
	string_copy(m_goals[cur_goal].name, m_name, NAME_LENGTH);
}

void CMissionGoalsDlg::OnCancel()
{
	theApp.record_window_data(&Messages_wnd_data, this);
	CDialog::OnCancel();
}

void CMissionGoalsDlg::OnClose() 
{
	int z;

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL)
			return;

		if (z == IDYES) {
			OnOk();
			return;
		}
	}
	
	CDialog::OnClose();
}

void CMissionGoalsDlg::insert_handler(int old, int node)
{
	int i;

	for (i=0; i<m_num_goals; i++){
		if (m_goals[i].formula == old){
			break;
		}
	}

	Assert(i < m_num_goals);
	m_goals[i].formula = node;
	return;
}

void CMissionGoalsDlg::OnGoalInvalid() 
{
	if ( cur_goal < 0 ){
		return;
	}

	m_goal_invalid = !m_goal_invalid;
	m_goals[cur_goal].type ^= INVALID_GOAL;
	UpdateData(TRUE);
}

void CMissionGoalsDlg::OnNoMusic() 
{
	if (cur_goal < 0){
		return;
	}

	m_no_music = !m_no_music;
	m_goals[cur_goal].flags ^= MGF_NO_MUSIC;
	UpdateData(TRUE);
}

void CMissionGoalsDlg::swap_handler(int node1, int node2)
{
	int index1, index2;
	mission_goal m;

	for (index1=0; index1<m_num_goals; index1++){
		if (m_goals[index1].formula == node1){
			break;
		}
	}

	Assert(index1 < m_num_goals);
	for (index2=0; index2<m_num_goals; index2++){
		if (m_goals[index2].formula == node2){
			break;
		}
	}

	Assert(index2 < m_num_goals);
	m = m_goals[index1];
//	m_goals[index1] = m_goals[index2];
	while (index1 < index2) {
		m_goals[index1] = m_goals[index1 + 1];
		m_sig[index1] = m_sig[index1 + 1];
		index1++;
	}

	while (index1 > index2 + 1) {
		m_goals[index1] = m_goals[index1 - 1];
		m_sig[index1] = m_sig[index1 - 1];
		index1--;
	}

	m_goals[index1] = m;
}

void CMissionGoalsDlg::OnChangeGoalScore() 
{
	if (cur_goal < 0){
		return;
	}

	UpdateData(TRUE);
	m_goals[cur_goal].score = m_goal_score;
}


// code when the "team" selection in the combo box changes
void CMissionGoalsDlg::OnSelchangeTeam() 
{
	if ( cur_goal < 0 ){
		return;
	}

	UpdateData(TRUE);
	m_goals[cur_goal].team = m_team;
}
