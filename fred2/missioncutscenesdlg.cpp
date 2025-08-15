/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "globalincs/linklist.h"
#include "parse/sexp.h"
#include "MissionCutscenesDlg.h"
#include "Management.h"
#include "OperatorArgTypeSelect.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMissionCutscenesDlg* Cutscene_editor_dlg; // global reference needed by sexp_tree class

CString cutscene_descriptions[Num_movie_types] = {
	"Plays just before the fiction viewer game state",
	"Plays just before the command briefing game state",
	"Plays just before the briefing game state",
	"Plays just before the mission starts after Accept has been pressed",
	"Plays just before the debriefing game state",
	"Plays when the debriefing has been accepted but before exiting the mission",
	"Plays when the campaign has been completed"
};

/////////////////////////////////////////////////////////////////////////////
// sexp_cutscene_tree class member functions

/////////////////////////////////////////////////////////////////////////////
// CMissionCutscenesDlg dialog class member functions

CMissionCutscenesDlg::CMissionCutscenesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMissionCutscenesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissionCutscenesDlg)
	m_cutscene_type = -1;
	m_display_cutscene_types = 0;
	m_name = _T("");
	m_desc = _T("");
	//}}AFX_DATA_INIT
	m_cutscenes_tree.m_mode = MODE_CUTSCENES; // We don't need to perform actions here, so use the same method as Goals
	m_cutscenes_tree.link_modified(&modified);
	modified = 0;
	select_sexp_node = -1;
}

BOOL CMissionCutscenesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();  // let the base class do the default work

	int adjust = 0;

	if (!Show_sexp_help)
	{
		CRect rect;
		GetDlgItem(IDC_HELP_BOX)->GetWindowRect(rect);
		adjust = rect.top - rect.bottom - 20;
	}

	theApp.init_window(&Mission_cutscenes_wnd_data, this, adjust);
	m_cutscenes_tree.setup((CEdit*)GetDlgItem(IDC_HELP_BOX));
	load_tree();
	create_tree();

	Cutscene_editor_dlg = this;
	int i = m_cutscenes_tree.select_sexp_node;
	if (i != -1) {
		GetDlgItem(IDC_CUTSCENES_TREE)->SetFocus();
		m_cutscenes_tree.hilite_item(i);
		return FALSE;
	}

	return TRUE;
}

void CMissionCutscenesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissionCutscenesDlg)
	DDX_Control(pDX, IDC_CUTSCENES_TREE, m_cutscenes_tree);
	DDX_CBIndex(pDX, IDC_CUTSCENE_TYPE_DROP, m_cutscene_type);
	DDX_CBIndex(pDX, IDC_DISPLAY_CUTSCENE_TYPES_DROP, m_display_cutscene_types);
	DDX_Text(pDX, IDC_CUTSCENE_NAME, m_name);
	DDV_MaxChars(pDX, m_name, NAME_LENGTH - 1);
	DDX_Text(pDX, IDC_CUTSCENE_HELP_BOX, m_desc);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMissionCutscenesDlg, CDialog)
	//{{AFX_MSG_MAP(CMissionCutscenesDlg)
	ON_CBN_SELCHANGE(IDC_DISPLAY_CUTSCENE_TYPES_DROP, OnSelchangeDisplayCutsceneTypesDrop)
	ON_NOTIFY(TVN_SELCHANGED, IDC_CUTSCENES_TREE, OnSelchangedCutscenesTree)
	ON_NOTIFY(NM_RCLICK, IDC_CUTSCENES_TREE, OnRclickCutscenesTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_CUTSCENES_TREE, OnEndlabeleditCutscenesTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_CUTSCENES_TREE, OnBeginlabeleditCutscenesTree)
	ON_BN_CLICKED(IDC_BUTTON_NEW_CUTSCENE, OnButtonNewCutscene)
	ON_CBN_SELCHANGE(IDC_CUTSCENE_TYPE_DROP, OnSelchangeCutsceneTypeDrop)
	ON_EN_CHANGE(IDC_CUTSCENE_NAME, OnChangeCutsceneName)
	ON_EN_CHANGE(IDC_CUTSCENE_HELP_BOX, OnChangeCutsceneName)
	ON_BN_CLICKED(ID_OK, OnButtonOk)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissionCutscenesDlg message handlers

// Initialization: sets up internal working copy of mission cutscenes and cutscene trees.
void CMissionCutscenesDlg::load_tree()
{
	m_cutscenes_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;

	m_cutscenes_tree.clear_tree();
	m_cutscenes.clear();
	m_sig.clear();
	for (int i=0; i<static_cast<int>(The_mission.cutscenes.size()); i++) {
		m_cutscenes.push_back(The_mission.cutscenes[i]);
		m_sig.push_back(i);

		if (m_cutscenes[i].filename[0] == '\0')
			strcpy_s(m_cutscenes[i].filename, "<Unnamed>");

		m_cutscenes[i].formula = m_cutscenes_tree.load_sub_tree(The_mission.cutscenes[i].formula, true, "true");
	}

	m_cutscenes_tree.post_load();
	cur_cutscene = -1;
	update_cur_cutscene();
}

// create the CTreeCtrl tree from the cutscene tree, filtering based on m_display_cutscene_types
void CMissionCutscenesDlg::create_tree()
{
	m_desc = _T(cutscene_descriptions[m_display_cutscene_types]);

	m_cutscenes_tree.DeleteAllItems();
	m_cutscenes_tree.reset_handles();
	for (size_t i = 0; i < m_cutscenes.size(); i++) {
		if (m_cutscenes[i].type != m_display_cutscene_types)
			continue;

		HTREEITEM h = m_cutscenes_tree.insert(m_cutscenes[i].filename);
		m_cutscenes_tree.SetItemData(h, m_cutscenes[i].formula);
		m_cutscenes_tree.add_sub_tree(m_cutscenes[i].formula, h);
	}

	cur_cutscene = -1;
	update_cur_cutscene();
}

// Display cutscene types selection changed, so update the display
void CMissionCutscenesDlg::OnSelchangeDisplayCutsceneTypesDrop()
{
	UpdateData(TRUE);
	create_tree();
}

// New tree item selected.  Because cutscene info is displayed for the selected tree item,
// we need to update the display when this occurs.
void CMissionCutscenesDlg::OnSelchangedCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = reinterpret_cast<NM_TREEVIEW*>(pNMHDR);

	HTREEITEM h = pNMTreeView->itemNew.hItem;
	if (!h)
		return;

	HTREEITEM h2;
	m_cutscenes_tree.update_help(h);
	while ((h2 = m_cutscenes_tree.GetParentItem(h)) != 0) {
		h = h2;
	}

	int z = static_cast<int>(m_cutscenes_tree.GetItemData(h));
	int i;
	for (i = 0; i < static_cast<int>(m_cutscenes.size()); i++) {
		if (m_cutscenes[i].formula == z) {
			break;
		}
	}

	Assertion(i < static_cast<int>(m_cutscenes.size()), "Attempt to select non-existing cutscene. Please report!");
	cur_cutscene = i;
	update_cur_cutscene();
	*pResult = 0;
}

// update display info to reflect the currently selected cutscene.
void CMissionCutscenesDlg::update_cur_cutscene()
{
	if (cur_cutscene < 0) {
		m_name = _T("");
		m_cutscene_type = -1;
		UpdateData(FALSE);
		GetDlgItem(IDC_CUTSCENE_TYPE_DROP) -> EnableWindow(FALSE);
		GetDlgItem(IDC_CUTSCENE_NAME) -> EnableWindow(FALSE);

		UpdateData(FALSE);
		return;
	}

	m_name = _T(m_cutscenes[cur_cutscene].filename);
	m_cutscene_type = m_cutscenes[cur_cutscene].type;

	UpdateData(FALSE);
	GetDlgItem(IDC_CUTSCENE_TYPE_DROP) -> EnableWindow(TRUE);
	GetDlgItem(IDC_CUTSCENE_NAME) -> EnableWindow(TRUE);

	UpdateData(FALSE);
}

// handler for context menu (i.e. a right mouse button click).
void CMissionCutscenesDlg::OnRclickCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_cutscenes_tree.right_clicked(MODE_CUTSCENES); // We don't need to perform actions here, so use the same method as Goals
	*pResult = 0;
}

// cutscene tree item label editing is requested.  Determine if it should be allowed.
void CMissionCutscenesDlg::OnBeginlabeleditCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_cutscenes_tree.edit_label(pTVDispInfo->item.hItem) == 1) {
		*pResult = 0;
		modified = 1;
	} else {
		*pResult = 1;
	}
}

// Once we finish editing, we need to clean up, which we do here.
void CMissionCutscenesDlg::OnEndlabeleditCutscenesTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_cutscenes_tree.end_label_edit(pTVDispInfo->item);
}

void CMissionCutscenesDlg::OnOK()
{
	CWnd *w;

	w = GetFocus();
	if (w) {
		HWND h = w->m_hWnd;
		GetDlgItem(IDC_CUTSCENES_TREE)->SetFocus();
		::SetFocus(h);
	}
}

int CMissionCutscenesDlg::query_modified()
{
	if (modified)
		return 1;

	if (The_mission.cutscenes.size() != m_cutscenes.size())
		return 1;

	for (int i = 0; i < (int)The_mission.cutscenes.size(); i++) {
		if (!stricmp(The_mission.cutscenes[i].filename, m_cutscenes[i].filename))
			return 1;
		if (The_mission.cutscenes[i].type != m_cutscenes[i].type)
			return 1;
	}

	return 0;
}

void CMissionCutscenesDlg::OnButtonOk()
{
	SCP_vector<std::pair<SCP_string, SCP_string>> names;

	UpdateData(TRUE);
	if (query_modified())
		set_modified();

	for (auto& cutscene : The_mission.cutscenes) {
		free_sexp2(cutscene.formula);
	}

	// copy all dialog cutscenes to the mission
	The_mission.cutscenes.clear();
	for (const auto& dialog_cutscene : m_cutscenes) {
		The_mission.cutscenes.push_back(dialog_cutscene);
		The_mission.cutscenes.back().formula = m_cutscenes_tree.save_tree(dialog_cutscene.formula);
	}

	theApp.record_window_data(&Mission_cutscenes_wnd_data, this);
	CDialog::OnOK();
}

void CMissionCutscenesDlg::OnButtonNewCutscene()
{
	m_cutscenes.emplace_back();
	m_sig.push_back(-1);

	m_cutscenes.back().type = m_display_cutscene_types;
	strcpy_s(m_cutscenes.back().filename, "cutscene filename");
	HTREEITEM h = m_cutscenes_tree.insert(m_cutscenes.back().filename);

	m_cutscenes_tree.item_index = -1;
	m_cutscenes_tree.add_operator("true", h);
	int index = m_cutscenes.back().formula = m_cutscenes_tree.item_index;
	m_cutscenes_tree.SetItemData(h, index);

	m_cutscenes_tree.SelectItem(h);
}

int CMissionCutscenesDlg::handler(int code, int node)
{
	switch (code) {
		case ROOT_DELETED:
			size_t i;
			for (i = 0; i < m_cutscenes.size(); i++){
				if (m_cutscenes[i].formula == node) {
					break;
				}
			}

			Assertion(i < m_cutscenes.size(), "Attempt to delete non-existing cutscene. Please report!");
			m_cutscenes.erase(m_cutscenes.begin() + i);
			m_sig.erase(m_sig.begin() + i);

			return node;

		default:
			UNREACHABLE("Unknown cutscene context menu case. Please report!");
	}

	return -1;
}

void CMissionCutscenesDlg::OnSelchangeCutsceneTypeDrop()
{
	if (cur_cutscene < 0) {
		return;
	}

	UpdateData(TRUE);
	UpdateData(TRUE);  // doesn't seem to update unless we do it twice..

	m_cutscenes[cur_cutscene].type = m_cutscene_type;

	HTREEITEM h = m_cutscenes_tree.GetSelectedItem();
	if (!h) {
		return;
	}

	HTREEITEM h2;
	while ((h2 = m_cutscenes_tree.GetParentItem(h)) != 0) {
		h = h2;
	}

	m_cutscenes_tree.DeleteItem(h);
	cur_cutscene = -1;
	update_cur_cutscene();
}

void CMissionCutscenesDlg::OnChangeCutsceneName()
{
	if (cur_cutscene < 0) {
		return;
	}

	UpdateData(TRUE);
	HTREEITEM h = m_cutscenes_tree.GetSelectedItem();
	if (!h){
		return;
	}

	HTREEITEM h2;
	while ((h2 = m_cutscenes_tree.GetParentItem(h)) != 0) {
		h = h2;
	}

	m_cutscenes_tree.SetItemText(h, m_name);
	strcpy_s(m_cutscenes[cur_cutscene].filename, m_name);
}

void CMissionCutscenesDlg::OnCancel()
{
	theApp.record_window_data(&Messages_wnd_data, this);
	CDialog::OnCancel();
}

void CMissionCutscenesDlg::OnClose()
{
	if (query_modified()) {
		int z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL)
			return;

		if (z == IDYES) {
			OnButtonOk();
			return;
		}
	}
	
	CDialog::OnClose();
}

void CMissionCutscenesDlg::insert_handler(int old, int node)
{
	size_t i;

	for (i = 0; i < m_cutscenes.size(); i++) {
		if (m_cutscenes[i].formula == old) {
			break;
		}
	}

	Assertion(i < m_cutscenes.size(), "Attempt to set formula for non-existing cutscene. Please report!");
	m_cutscenes[i].formula = node;
	return;
}

void CMissionCutscenesDlg::move_handler(int node1, int node2, bool insert_before)
{
	size_t index1;
	for (index1 = 0; index1 < m_cutscenes.size(); index1++) {
		if (m_cutscenes[index1].formula == node1) {
			break;
		}
	}
	Assertion(index1 < m_cutscenes.size(), "Attept to modify non-existing cutscene. Please report!");

	size_t index2;
	for (index2 = 0; index2 < m_cutscenes.size(); index2++) {
		if (m_cutscenes[index2].formula == node2) {
			break;
		}
	}
	Assertion(index2 < m_cutscenes.size(), "Attempt to insert non-existing cutscene. Please report!");

	mission_cutscene foo = m_cutscenes[index1];
	int s = m_sig[index1];

	int offset = insert_before ? -1 : 0;

	while (index1 < index2 + offset) {
		m_cutscenes[index1] = m_cutscenes[index1 + 1];
		m_sig[index1] = m_sig[index1 + 1];
		index1++;
	}
	while (index1 > index2 + offset + 1) {
		m_cutscenes[index1] = m_cutscenes[index1 - 1];
		m_sig[index1] = m_sig[index1 - 1];
		index1--;
	}

	m_cutscenes[index1] = foo;
	m_sig[index1] = s;
}
