/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// CampaignEditorDlg.cpp : implementation file
//

#include <setjmp.h>
#include "stdafx.h"
#include "FRED.h"
#include "CampaignEditorDlg.h"
#include "CampaignTreeView.h"
#include "CampaignTreeWnd.h"
#include "Management.h"
#include "cfile/cfile.h"
#include "FREDDoc.h"
#include "parse/parselo.h"
#include "mission/missiongoals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int Cur_campaign_mission = -1;
int Cur_campaign_link = -1;

/////////////////////////////////////////////////////////////////////////////
// campaign_editor dialog

IMPLEMENT_DYNCREATE(campaign_editor, CFormView)

campaign_editor *Campaign_tree_formp;

campaign_editor::campaign_editor()
	: CFormView(campaign_editor::IDD)
{
	//{{AFX_DATA_INIT(campaign_editor)
	m_name = _T("");
	m_type = -1;
	m_num_players = _T("");
	m_desc = _T("");
	m_branch_desc = _T("");
	m_branch_brief_anim = _T("");
	m_branch_brief_sound = _T("");
	m_custom_tech_db = FALSE;
	//}}AFX_DATA_INIT

	m_tree.m_mode = MODE_CAMPAIGN;
	m_num_links = 0;
	m_tree.link_modified(&Campaign_modified);
	m_last_mission = -1;
}

campaign_editor::~campaign_editor()
{
}

void campaign_editor::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(campaign_editor)
	DDX_Control(pDX, IDC_SEXP_TREE, m_tree);
	DDX_Control(pDX, IDC_FILELIST, m_filelist);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_CBIndex(pDX, IDC_CAMPAIGN_TYPE, m_type);
	DDX_Text(pDX, IDC_NUM_PLAYERS, m_num_players);
	DDX_Text(pDX, IDC_DESC2, m_desc);
	DDX_Text(pDX, IDC_MISSISON_LOOP_DESC, m_branch_desc);
	DDX_Text(pDX, IDC_LOOP_BRIEF_ANIM, m_branch_brief_anim);
	DDX_Text(pDX, IDC_LOOP_BRIEF_SOUND, m_branch_brief_sound);
	DDX_Check(pDX, IDC_CUSTOM_TECH_DB, m_custom_tech_db);
	//}}AFX_DATA_MAP

	DDV_MaxChars(pDX, m_desc, MISSION_DESC_LENGTH - 1);
	DDV_MaxChars(pDX, m_branch_desc, MISSION_DESC_LENGTH - 1);	
}

BEGIN_MESSAGE_MAP(campaign_editor, CFormView)
	//{{AFX_MSG_MAP(campaign_editor)
	ON_BN_CLICKED(ID_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_ALIGN, OnAlign)
	ON_BN_CLICKED(ID_CPGN_CLOSE, OnCpgnClose)
	ON_NOTIFY(NM_RCLICK, IDC_SEXP_TREE, OnRclickTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_SEXP_TREE, OnBeginlabeleditSexpTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_SEXP_TREE, OnEndlabeleditSexpTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SEXP_TREE, OnSelchangedSexpTree)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_COMMAND(ID_END_EDIT, OnEndEdit)
	ON_EN_CHANGE(IDC_BRIEFING_CUTSCENE, OnChangeBriefingCutscene)
	ON_CBN_SELCHANGE(IDC_CAMPAIGN_TYPE, OnSelchangeType)
	ON_BN_CLICKED(IDC_TOGGLE_LOOP, OnToggleLoop)
	ON_BN_CLICKED(IDC_LOOP_BRIEF_BROWSE, OnBrowseLoopAni)
	ON_BN_CLICKED(IDC_LOOP_BRIEF_SOUND_BROWSE, OnBrowseLoopSound)
	ON_EN_CHANGE(IDC_MAIN_HALL, OnChangeMainHall)
	ON_EN_CHANGE(IDC_DEBRIEFING_PERSONA, OnChangeDebriefingPersona)
	ON_BN_CLICKED(IDC_CUSTOM_TECH_DB, OnCustomTechDB)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// campaign_editor diagnostics

#ifdef _DEBUG
void campaign_editor::AssertValid() const
{
	CFormView::AssertValid();
}

void campaign_editor::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// campaign_editor message handlers

void campaign_editor::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UpdateData(FALSE);
}

void campaign_editor::OnLoad() 
{
	char buf[512];
	int size, offset;

	if (Cur_campaign_mission < 0){
		return;
	}

	if (Campaign_modified){
		if (Campaign_wnd->save_modified()){
			return;
		}
	}

	cf_find_file_location(Campaign.missions[Cur_campaign_mission].name, CF_TYPE_MISSIONS, 512, buf, &size, &offset, false);

	FREDDoc_ptr->SetPathName(buf);
	Campaign_wnd->DestroyWindow();

//		if (FREDDoc_ptr->OnOpenDocument(Campaign.missions[Cur_campaign_mission].name)) {
//			Bypass_clear_mission = 1;
//			Campaign_wnd->DestroyWindow();
//
//		} else {
//			MessageBox("Failed to load mission!", "Error");
//		}
}

BOOL campaign_editor::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	int i;
	BOOL r;
	CComboBox *box;

	r = CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	if (r) {
		box = (CComboBox *) GetDlgItem(IDC_CAMPAIGN_TYPE);
		box->ResetContent();
		for (i=0; i<MAX_CAMPAIGN_TYPES; i++){
			box->AddString(campaign_types[i]);
		}
	}

	return r;
}

void campaign_editor::load_campaign()
{
	Cur_campaign_mission = Cur_campaign_link = -1;
	load_tree(0);

	if (!strlen(Campaign.filename))
		strcpy_s(Campaign.filename, Default_campaign_file_name);

	if (mission_campaign_load(Campaign.filename, NULL, 0)) {
		MessageBox("Couldn't open Campaign file!", "Error");
		Campaign_wnd->OnCpgnFileNew();
		return;
	}

	Campaign_modified = 0;
	Campaign_tree_viewp->construct_tree();
	initialize();
}

void campaign_editor::OnAlign() 
{
	Campaign_tree_viewp->sort_elements();
	Campaign_tree_viewp->realign_tree();
	Campaign_tree_viewp->Invalidate();
}

void campaign_editor::initialize( int init_files )
{
	Cur_campaign_mission = Cur_campaign_link = -1;
	m_tree.setup((CEdit *) GetDlgItem(IDC_HELP_BOX));
	load_tree(0);
	Campaign_tree_viewp->initialize();

	// only initialize the file dialog box when the parameter says to.  This should
	// only happen when a campaign type changes
	if ( init_files ){
		m_filelist.initialize();
	}

	m_name = Campaign.name;
	m_type = Campaign.type;
	m_num_players.Format("%d", Campaign.num_players);

	if (Campaign.desc) {
		convert_multiline_string(m_desc, Campaign.desc);
	} else {
		m_desc = _T("");
	}

	m_branch_desc = _T("");

	m_branch_brief_anim = _T("");	
	m_branch_brief_sound = _T("");	

	// single player should hide the two dialog items about number of players allowed
	if ( m_type == CAMPAIGN_TYPE_SINGLE ) {
		GetDlgItem(IDC_NUM_PLAYERS)->ShowWindow( SW_HIDE );
		GetDlgItem(IDC_PLAYERS_LABEL)->ShowWindow( SW_HIDE );
	} else {
		GetDlgItem(IDC_NUM_PLAYERS)->ShowWindow( SW_SHOW );
		GetDlgItem(IDC_PLAYERS_LABEL)->ShowWindow( SW_SHOW );
	}

	// set up the tech db checkbox
	m_custom_tech_db = (BOOL)(Campaign.flags & CF_CUSTOM_TECH_DATABASE);

	UpdateData(FALSE);
}

void campaign_editor::mission_selected(int num)
{
	CEdit *bc_dialog, *bc_hall, *bc_persona;
	char mainhalltext[10];
	char personatext[10];

	bc_dialog = (CEdit *) GetDlgItem(IDC_BRIEFING_CUTSCENE);
	bc_hall = (CEdit *) GetDlgItem(IDC_MAIN_HALL);
	bc_persona = (CEdit *) GetDlgItem(IDC_DEBRIEFING_PERSONA);

	// clear out the text for the briefing cutscene, and put in new text if specified
	bc_dialog->SetWindowText("");
	if ( strlen(Campaign.missions[num].briefing_cutscene) )
		bc_dialog->SetWindowText(Campaign.missions[num].briefing_cutscene);

	// new main hall stuff
	sprintf(mainhalltext, "%s", const_cast<char*>(Campaign.missions[num].main_hall.c_str()));
	bc_hall->SetWindowText(CString(mainhalltext));

	// new debriefing persona stuff
	sprintf(personatext, "%d", Campaign.missions[num].debrief_persona_index);
	bc_persona->SetWindowText(CString(personatext));
}

void campaign_editor::update()
{
	char buf[MISSION_DESC_LENGTH];

	// get data from dlog box
	UpdateData(TRUE);

	// update campaign name
	string_copy(Campaign.name, m_name, NAME_LENGTH);
	Campaign.type = m_type;

	// update campaign desc
	deconvert_multiline_string(buf, m_desc, MISSION_DESC_LENGTH);
	if (Campaign.desc) {
		free(Campaign.desc);
	}

	Campaign.desc = NULL;
	if (strlen(buf)) {
		Campaign.desc = strdup(buf);
	}

	// update flags
	Campaign.flags = CF_DEFAULT_VALUE;
	if (m_custom_tech_db)
		Campaign.flags |= CF_CUSTOM_TECH_DATABASE;

	// maybe update mission loop text
	save_loop_desc_window();

	// set the number of players in a multiplayer mission equal to the number of players in the first mission
	if ( Campaign.type != CAMPAIGN_TYPE_SINGLE ) {
		if ( Campaign.num_missions == 0 ) {
			Campaign.num_players = 0;
		} else {
			mission a_mission;

			get_mission_info(Campaign.missions[0].name, &a_mission);
			Campaign.num_players = a_mission.num_players;
		}
	}
}

void campaign_editor::OnCpgnClose() 
{
	Campaign_wnd->OnClose();
}

void campaign_editor::load_tree(int save_first)
{
	char text[80];
	int i;
	HTREEITEM h;

	if (save_first)
		save_tree();

	m_tree.clear_tree();
	m_tree.DeleteAllItems();
	m_num_links = 0;
	UpdateData(TRUE);
	update_loop_desc_window();

	m_last_mission = Cur_campaign_mission;
	if (Cur_campaign_mission < 0) {
		GetDlgItem(IDC_SEXP_TREE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BRIEFING_CUTSCENE)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAIN_HALL)->EnableWindow(FALSE);
		return;
	}

	GetDlgItem(IDC_SEXP_TREE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BRIEFING_CUTSCENE)->EnableWindow(TRUE);
	GetDlgItem(IDC_MAIN_HALL)->EnableWindow(TRUE);

	GetDlgItem(IDC_MISSISON_LOOP_DESC)->EnableWindow(FALSE);	
	GetDlgItem(IDC_LOOP_BRIEF_ANIM)->EnableWindow(FALSE);
	GetDlgItem(IDC_LOOP_BRIEF_SOUND)->EnableWindow(FALSE);
	GetDlgItem(IDC_LOOP_BRIEF_BROWSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_LOOP_BRIEF_SOUND_BROWSE)->EnableWindow(FALSE);

	for (i=0; i<Total_links; i++) {
		if (Links[i].from == Cur_campaign_mission) {
			Links[i].node = m_tree.load_sub_tree(Links[i].sexp, true, "do-nothing");
			m_num_links++;

			if (Links[i].from == Links[i].to) {
				strcpy_s(text, "Repeat mission");
			} else if ( (Links[i].to == -1) && (Links[i].from != -1) ) {
				strcpy_s(text, "End of Campaign");
			} else {
				sprintf(text, "Branch to %s", Campaign.missions[Links[i].to].name);
			}

			// insert item into tree
			int image, sel_image;
			if (Links[i].is_mission_loop || Links[i].is_mission_fork) {
				image = BITMAP_BLUE_DOT;
				sel_image = BITMAP_GREEN_DOT;
			} else {
				image = BITMAP_BLACK_DOT;
				sel_image = BITMAP_RED_DOT;
			}

			h = m_tree.insert(text, image, sel_image);

			m_tree.SetItemData(h, Links[i].node);
			m_tree.add_sub_tree(Links[i].node, h);
		}
	}
}

void campaign_editor::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_tree.right_clicked(MODE_CAMPAIGN);
	*pResult = 0;
}

void campaign_editor::OnBeginlabeleditSexpTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		Campaign_modified = 1;
	} else {
		*pResult = 1;
	}
}

void campaign_editor::OnEndlabeleditSexpTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_tree.end_label_edit(pTVDispInfo->item);
}

int campaign_editor::handler(int code, int node, char *str)
{
	int i;

	switch (code) {
	case ROOT_DELETED:
		for (i=0; i<Total_links; i++){
			if ((Links[i].from == Cur_campaign_mission) && (Links[i].node == node)){
				break;
			}
		}

		Campaign_tree_viewp->delete_link(i);
		m_num_links--;
		return node;

	default:
		Int3();
	}

	return -1;
}

void campaign_editor::save_tree(int clear)
{
	int i;

	if (m_last_mission < 0){
		return;  // nothing to save
	}

	for (i=0; i<Total_links; i++){
		if (Links[i].from == m_last_mission) {
			sexp_unmark_persistent(Links[i].sexp);
			free_sexp2(Links[i].sexp);
			Links[i].sexp = m_tree.save_tree(Links[i].node);
			sexp_mark_persistent(Links[i].sexp);
		}
	}

	if (clear){
		m_last_mission = -1;
	}
}

void campaign_editor::OnSelchangedSexpTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int i, node;
	HTREEITEM h, h2;

	// get handle of selected item
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	h = pNMTreeView->itemNew.hItem;
	Assert(h);

	// update help on sexp
	m_tree.update_help(h);

	// get handle of parent
	while ((h2 = m_tree.GetParentItem(h))>0){
		h = h2;
	}

	// get identifier of parent
	node = m_tree.GetItemData(h);
	for (i=0; i<Total_links; i++){
		if ((Links[i].from == Cur_campaign_mission) && (Links[i].node == node)){
			break;
		}
	}

	if (i == Total_links) {
		Cur_campaign_link = -1;
		return;
	}

	// update mission loop text
	UpdateData(TRUE);
	save_loop_desc_window();

	Cur_campaign_link = i;
	update_loop_desc_window();
	Campaign_tree_viewp->Invalidate();
	*pResult = 0;
}

void campaign_editor::OnMoveUp() 
{
	int i, last = -1;
	campaign_tree_link temp;
	HTREEITEM h1, h2;

	if (Cur_campaign_link >= 0) {
		save_tree();
		for (i=0; i<Total_links; i++){
			if (Links[i].from == Cur_campaign_mission) {
				if (i == Cur_campaign_link){
					break;
				}

				last = i;
			}
		}

		if ((last != -1) && (i < Total_links)) {
			h1 = m_tree.GetParentItem(m_tree.handle(Links[i].node));
			h2 = m_tree.GetParentItem(m_tree.handle(Links[last].node));
			m_tree.swap_roots(h1, h2);
			m_tree.SelectItem(m_tree.GetParentItem(m_tree.handle(Links[i].node)));

			temp = Links[last];
			Links[last] = Links[i];
			Links[i] = temp;
			Cur_campaign_link = last;
		}
	}

	GetDlgItem(IDC_SEXP_TREE)->SetFocus();
}

void campaign_editor::OnMoveDown() 
{
	int i, j;
	campaign_tree_link temp;
	HTREEITEM h1, h2;

	if (Cur_campaign_link >= 0) {
		save_tree();
		for (i=0; i<Total_links; i++)
			if (Links[i].from == Cur_campaign_mission)
				if (i == Cur_campaign_link)
					break;

		for (j=i+1; j<Total_links; j++)
			if (Links[j].from == Cur_campaign_mission)
				break;

		if (j < Total_links) {
			h1 = m_tree.GetParentItem(m_tree.handle(Links[i].node));
			h2 = m_tree.GetParentItem(m_tree.handle(Links[j].node));
			m_tree.swap_roots(h1, h2);
			m_tree.SelectItem(m_tree.GetParentItem(m_tree.handle(Links[i].node)));

			temp = Links[j];
			Links[j] = Links[i];
			Links[i] = temp;
			Cur_campaign_link = j;
		}
	}

	GetDlgItem(IDC_SEXP_TREE)->SetFocus();
}

void campaign_editor::swap_handler(int node1, int node2)
{
	int index1, index2;
	campaign_tree_link temp;

	for (index1=0; index1<Total_links; index1++){
		if ((Links[index1].from == Cur_campaign_mission) && (Links[index1].node == node1)){
			break;
		}
	}

	Assert(index1 < Total_links);
	for (index2=0; index2<Total_links; index2++){
		if ((Links[index2].from == Cur_campaign_mission) && (Links[index2].node == node2)){
			break;
		}
	}

	Assert(index2 < Total_links);
	temp = Links[index1];
//	Links[index1] = Links[index2];
	while (index1 < index2) {
		Links[index1] = Links[index1 + 1];
		index1++;
	}

	while (index1 > index2 + 1) {
		Links[index1] = Links[index1 - 1];
		index1--;
	}

	// update Cur_campaign_link
	Cur_campaign_link = index1;

	Links[index1] = temp;
}

void campaign_editor::insert_handler(int old, int node)
{
	int i;

	for (i=0; i<Total_links; i++){
		if ((Links[i].from == Cur_campaign_mission) && (Links[i].node == old)){
			break;
		}
	}

	Assert(i < Total_links);
	Links[i].node = node;
	return;
}

void campaign_editor::OnEndEdit() 
{
	HWND h;
	CWnd *w;

	w = GetFocus();
	if (w) {
		h = w->m_hWnd;
		GetDlgItem(IDC_SEXP_TREE)->SetFocus();
		::SetFocus(h);
	}
}

void campaign_editor::OnChangeBriefingCutscene() 
{
	CEdit *bc_dialog;

	bc_dialog = (CEdit *) GetDlgItem(IDC_BRIEFING_CUTSCENE);

	// maybe save off the current cutscene names.
	if ( Cur_campaign_mission != -1 ) {

		// see if the contents of the edit box have changed.  Luckily, this returns 0 when the edit box is
		// cleared.
		if ( bc_dialog->GetModify() ) {
			bc_dialog->GetWindowText( Campaign.missions[Cur_campaign_mission].briefing_cutscene, NAME_LENGTH );
			Campaign_modified = 1;
		}
	}

}

// what to do when changing campaign type
void campaign_editor::OnSelchangeType() 
{
	// if campaign type is single player, then disable the multiplayer items
	update();
	initialize();
	Campaign_modified = 1;
}


// update the loop mission text
void campaign_editor::save_loop_desc_window()
{
	char buffer[MISSION_DESC_LENGTH];

	// update only if active link and there is a mission has mission loop or fork
	if ( (Cur_campaign_link >= 0) && (Links[Cur_campaign_link].is_mission_loop || Links[Cur_campaign_link].is_mission_fork) ) {
		deconvert_multiline_string(buffer, m_branch_desc, MISSION_DESC_LENGTH);
		if (Links[Cur_campaign_link].mission_branch_txt) {
			free(Links[Cur_campaign_link].mission_branch_txt);
		}
		if (Links[Cur_campaign_link].mission_branch_brief_anim) {
			free(Links[Cur_campaign_link].mission_branch_brief_anim);
		}
		if (Links[Cur_campaign_link].mission_branch_brief_sound) {
			free(Links[Cur_campaign_link].mission_branch_brief_sound);
		}

		if (strlen(buffer)) {
			Links[Cur_campaign_link].mission_branch_txt = strdup(buffer);
		} else {
			Links[Cur_campaign_link].mission_branch_txt = NULL;
		}

		deconvert_multiline_string(buffer, m_branch_brief_anim, MAX_FILENAME_LEN);
		if(strlen(buffer)){
			Links[Cur_campaign_link].mission_branch_brief_anim = strdup(buffer);
		} else {
			Links[Cur_campaign_link].mission_branch_brief_anim = NULL;
		}

		deconvert_multiline_string(buffer, m_branch_brief_sound, MAX_FILENAME_LEN);
		if(strlen(buffer)){
			Links[Cur_campaign_link].mission_branch_brief_sound = strdup(buffer);
		} else {
			Links[Cur_campaign_link].mission_branch_brief_sound = NULL;
		}
	}
}

void campaign_editor::update_loop_desc_window()
{
	bool enable_branch_desc_window;

	enable_branch_desc_window = false;
	if ((Cur_campaign_link >= 0) && (Links[Cur_campaign_link].is_mission_loop || Links[Cur_campaign_link].is_mission_fork)) {
		enable_branch_desc_window = true;
	}

	// maybe enable description window
	GetDlgItem(IDC_MISSISON_LOOP_DESC)->EnableWindow(enable_branch_desc_window);
	GetDlgItem(IDC_LOOP_BRIEF_ANIM)->EnableWindow(enable_branch_desc_window);
	GetDlgItem(IDC_LOOP_BRIEF_SOUND)->EnableWindow(enable_branch_desc_window);
	GetDlgItem(IDC_LOOP_BRIEF_BROWSE)->EnableWindow(enable_branch_desc_window);
	GetDlgItem(IDC_LOOP_BRIEF_SOUND_BROWSE)->EnableWindow(enable_branch_desc_window);

	// set new text
	if ((Cur_campaign_link >= 0) && Links[Cur_campaign_link].mission_branch_txt && enable_branch_desc_window) {
		convert_multiline_string(m_branch_desc, Links[Cur_campaign_link].mission_branch_txt);		
	} else {
		m_branch_desc = _T("");
	}

	// set new text
	if ((Cur_campaign_link >= 0) && Links[Cur_campaign_link].mission_branch_brief_anim && enable_branch_desc_window) {
		convert_multiline_string(m_branch_brief_anim, Links[Cur_campaign_link].mission_branch_brief_anim);		
	} else {
		m_branch_brief_anim = _T("");
	}

	// set new text
	if ((Cur_campaign_link >= 0) && Links[Cur_campaign_link].mission_branch_brief_sound && enable_branch_desc_window) {
		convert_multiline_string(m_branch_brief_sound, Links[Cur_campaign_link].mission_branch_brief_sound);
	} else {
		m_branch_brief_sound = _T("");
	}

	// reset text
	UpdateData(FALSE);
}

void campaign_editor::OnToggleLoop() 
{
	// check if branch selected
	if (Cur_campaign_link == -1) {
		return;
	}

	// store mission loop text
	UpdateData(TRUE);

	if ( (Cur_campaign_link >= 0) && (Links[Cur_campaign_link].is_mission_loop || Links[Cur_campaign_link].is_mission_fork) ) {
		if (Links[Cur_campaign_link].mission_branch_txt) {
			free(Links[Cur_campaign_link].mission_branch_txt);
		}

		if (Links[Cur_campaign_link].mission_branch_brief_anim) {
			free(Links[Cur_campaign_link].mission_branch_brief_anim);
		}

		if (Links[Cur_campaign_link].mission_branch_brief_sound) {
			free(Links[Cur_campaign_link].mission_branch_brief_sound);
		}

		char buffer[MISSION_DESC_LENGTH];
		
		deconvert_multiline_string(buffer, m_branch_desc, MISSION_DESC_LENGTH);
		if (m_branch_desc && strlen(buffer)) {
			Links[Cur_campaign_link].mission_branch_txt = strdup(buffer);
		} else {
			Links[Cur_campaign_link].mission_branch_txt = NULL;
		}

		deconvert_multiline_string(buffer, m_branch_brief_anim, MISSION_DESC_LENGTH);
		if (m_branch_brief_anim && strlen(buffer)) {
			Links[Cur_campaign_link].mission_branch_brief_anim = strdup(buffer);
		} else {
			Links[Cur_campaign_link].mission_branch_brief_anim = NULL;
		}

		deconvert_multiline_string(buffer, m_branch_brief_sound, MISSION_DESC_LENGTH);
		if (m_branch_brief_sound && strlen(buffer)) {
			Links[Cur_campaign_link].mission_branch_brief_sound = strdup(buffer);
		} else {
			Links[Cur_campaign_link].mission_branch_brief_sound = NULL;
		}
	}

	// toggle to mission_loop setting
	Links[Cur_campaign_link].is_mission_loop = !Links[Cur_campaign_link].is_mission_loop;
	Links[Cur_campaign_link].is_mission_fork = false;

	// reset loop desc window (gray if inactive)
	update_loop_desc_window();

	// set root icon
	int bitmap1, bitmap2;
	if (Links[Cur_campaign_link].is_mission_loop || Links[Cur_campaign_link].is_mission_fork) {
		bitmap2 = BITMAP_GREEN_DOT;
		bitmap1 = BITMAP_BLUE_DOT;
	} else {
		bitmap1 = BITMAP_BLACK_DOT;
		bitmap2 = BITMAP_RED_DOT;
	}

	// Search for item and update bitmap
	HTREEITEM h = m_tree.GetRootItem();
	while (h) {
		if ((int) m_tree.GetItemData(h) == Links[Cur_campaign_link].node) {
			m_tree.SetItemImage(h, bitmap1, bitmap2);
			break;
		}

		h = m_tree.GetNextSiblingItem(h);
	}

	// set to redraw
	Campaign_tree_viewp->Invalidate();
}

void campaign_editor::OnBrowseLoopAni()
{
	int pushed_dir;
	
	UpdateData(TRUE);

	// switch directories
	pushed_dir = cfile_push_chdir(CF_TYPE_INTERFACE);
	CFileDialog dlg(TRUE, "ani", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, "Ani Files (*.ani)|*.ani");

	if (dlg.DoModal() == IDOK) {
		m_branch_brief_anim = dlg.GetFileName();
		UpdateData(FALSE);
	}

	// move back to the proper directory
	if (!pushed_dir){
		cfile_pop_dir();	
	}
}

void campaign_editor::OnBrowseLoopSound()
{
	int pushed_dir;
	
	UpdateData(TRUE);

	// switch directories
	pushed_dir = cfile_push_chdir(CF_TYPE_VOICE_CMD_BRIEF);
	CFileDialog dlg(TRUE, "wav", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, 
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK) {
		m_branch_brief_sound = dlg.GetFileName();
		UpdateData(FALSE);
	}

	// move back to the proper directory
	if (!pushed_dir){
		cfile_pop_dir();	
	}
}

void campaign_editor::OnChangeMainHall() 
{
	CString str;
	UpdateData(TRUE);

	if (Cur_campaign_mission >= 0)
	{
		GetDlgItem(IDC_MAIN_HALL)->GetWindowText(str);
		Campaign.missions[Cur_campaign_mission].main_hall = SCP_string((LPCTSTR)(str));
	}
}

void campaign_editor::OnChangeDebriefingPersona()
{
	CString str;
	int persona;

	UpdateData(TRUE);

	if (Cur_campaign_mission >= 0)
	{
		GetDlgItem(IDC_DEBRIEFING_PERSONA)->GetWindowText(str);
		persona = atoi(str);

		if (persona < 0 || persona > 0xff)
			persona = 0;

		Campaign.missions[Cur_campaign_mission].debrief_persona_index = (ubyte) persona;
	}
}

void campaign_editor::OnCustomTechDB() 
{
	UpdateData(TRUE);

	if (m_custom_tech_db)
		Campaign.flags |= CF_CUSTOM_TECH_DATABASE;
	else
		Campaign.flags &= ~CF_CUSTOM_TECH_DATABASE;
}
