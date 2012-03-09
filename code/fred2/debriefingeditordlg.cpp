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
#include "DebriefingEditorDlg.h"
#include "FREDDoc.h"
#include "mission/missionbriefcommon.h"
#include "mission/missionparse.h"
#include "globalincs/linklist.h"
#include "parse/sexp.h"
#include "gamesnd/eventmusic.h"
#include "cfile/cfile.h"
#include "sound/audiostr.h"
#include "localization/localize.h"
#include "jumpnode/jumpnode.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// debriefing_editor_dlg dialog

debriefing_editor_dlg::debriefing_editor_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(debriefing_editor_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(debriefing_editor_dlg)
	m_text = _T("");
	m_voice = _T("");
	m_stage_title = _T("");
	m_rec_text = _T("");
	m_debriefPass_music = 0;
	m_debriefAvg_music = 0;
	m_debriefFail_music = 0;
	m_current_debriefing = -1;
	//}}AFX_DATA_INIT

	modified = 0;
	m_cur_stage = 0;
	m_last_stage = -1;
	m_voice_id = -1;
	select_sexp_node = -1;
}

void debriefing_editor_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(debriefing_editor_dlg)
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_Text(pDX, IDC_TEXT, m_text);
	DDX_Text(pDX, IDC_VOICE, m_voice);
	DDX_Text(pDX, IDC_STAGE_TITLE, m_stage_title);
	DDX_Text(pDX, IDC_REC_TEXT, m_rec_text);
	DDX_CBIndex(pDX, IDC_SUCCESSFUL_MISSION_TRACK, m_debriefPass_music);
	DDX_CBIndex(pDX, IDC_DEBRIEFING_TRACK, m_debriefAvg_music);
	DDX_CBIndex(pDX, IDC_FAILED_MISSION_TRACK, m_debriefFail_music);
	//}}AFX_DATA_MAP

	DDV_MaxChars(pDX, m_voice, MAX_FILENAME_LEN - 1);
}

BEGIN_MESSAGE_MAP(debriefing_editor_dlg, CDialog)
	//{{AFX_MSG_MAP(debriefing_editor_dlg)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_ADD_STAGE, OnAddStage)
	ON_BN_CLICKED(IDC_DELETE_STAGE, OnDeleteStage)
	ON_BN_CLICKED(IDC_INSERT_STAGE, OnInsertStage)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnRclickTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE, OnBeginlabeleditTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE, OnEndlabeleditTree)
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// debriefing_editor_dlg message handlers

void debriefing_editor_dlg::OnInitMenu(CMenu* pMenu)
{
	int i;
	CMenu *m;

	// disable any items we should disable
	m = pMenu->GetSubMenu(0);

	// uncheck all menu items
	for (i = 0; i < Num_teams; i++ )
		m->CheckMenuItem( i, MF_BYPOSITION | MF_UNCHECKED );

	for ( i = Num_teams; i < MAX_TVT_TEAMS; i++ )
		m->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);


	// put a check next to the currently selected item
	m->CheckMenuItem(m_current_debriefing, MF_BYPOSITION | MF_CHECKED );

	// Karajorma - it might be nice to autobalance the briefings and debriefings but I'm not doing anything till I
	// understand how the how system works better. disabling the option for now. 
	m = pMenu->GetSubMenu(1); 
	m->EnableMenuItem(ID_AUTOBALANCE, MF_GRAYED); 

	CDialog::OnInitMenu(pMenu);
}

BOOL debriefing_editor_dlg::OnInitDialog()
{
	int i, n;

	CDialog::OnInitDialog();
	m_play_bm.LoadBitmap(IDB_PLAY);
	((CButton *) GetDlgItem(IDC_PLAY)) -> SetBitmap(m_play_bm);
	CComboBox *box;
	box = (CComboBox *) GetDlgItem(IDC_ICON_IMAGE);

	m_current_debriefing = 0;
	UpdateData(FALSE);

	Debriefing = &Debriefings[m_current_debriefing];

	box = (CComboBox *) GetDlgItem(IDC_SUCCESSFUL_MISSION_TRACK);
	box->AddString("None");
	for (i=0; i<Num_music_files; i++)
		box->AddString(Spooled_music[i].name);

	box = (CComboBox *) GetDlgItem(IDC_DEBRIEFING_TRACK);
	box->AddString("None");
	for (i=0; i<Num_music_files; i++)
		box->AddString(Spooled_music[i].name);

	box = (CComboBox *) GetDlgItem(IDC_FAILED_MISSION_TRACK);
	box->AddString("None");
	for (i=0; i<Num_music_files; i++)
		box->AddString(Spooled_music[i].name);

	m_debriefPass_music = Mission_music[SCORE_DEBRIEF_SUCCESS] + 1;
	m_debriefAvg_music = Mission_music[SCORE_DEBRIEF_AVERAGE] + 1;
	m_debriefFail_music = Mission_music[SCORE_DEBRIEF_FAIL] + 1;

	m_tree.link_modified(&modified);  // provide way to indicate trees are modified in dialog
	n = m_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;
	if (n != -1) {
		for (i=0; i<Debriefing->num_stages; i++)
			if (query_node_in_sexp(n, Debriefing->stages[i].formula))
				break;

		if (i < Debriefing->num_stages) {
			m_cur_stage = i;
			update_data();
			GetDlgItem(IDC_TREE) -> SetFocus();
			m_tree.hilite_item(m_tree.select_sexp_node);
			set_modified();
			return FALSE;
		}
	}

	CDialog::OnInitDialog();
	update_data();
	set_modified();

	// hard coded stuff to deal with the multiple briefings per mission.

	return TRUE;
}

void debriefing_editor_dlg::update_data(int update)
{
	int enable, save_debriefing;
	debrief_stage *ptr;

	save_debriefing = m_current_debriefing;

	if (update)
		UpdateData(TRUE);

	// based on the game type, enable the multiple briefings combo box (or disable it)

	// set up the pointer to the briefing that we are editing
	if ( save_debriefing != m_current_debriefing )
		Debriefing = &Debriefings[save_debriefing];
	else
		Debriefing = &Debriefings[m_current_debriefing];

	if (m_last_stage >= 0) {
		ptr = &Debriefing->stages[m_last_stage];
		if (ptr->formula >= 0)
			free_sexp2(ptr->formula);

		ptr->formula = m_tree.save_tree();
		deconvert_multiline_string(ptr->text, m_text);
		lcl_fred_replace_stuff(ptr->text);
		deconvert_multiline_string(ptr->recommendation_text, m_rec_text);
		lcl_fred_replace_stuff(ptr->recommendation_text);
		string_copy(ptr->voice, m_voice, MAX_FILENAME_LEN);
	}

	// now get new stage data
	if ((m_cur_stage >= 0) && (m_cur_stage < Debriefing->num_stages)) {
		ptr = &Debriefing->stages[m_cur_stage];
		m_stage_title.Format("Stage %d of %d", m_cur_stage + 1, Debriefing->num_stages);
		m_tree.load_tree(ptr->formula);
		m_text = convert_multiline_string(ptr->text.c_str());
		m_rec_text = convert_multiline_string(ptr->recommendation_text.c_str());
		m_voice = ptr->voice;
		enable = TRUE;

	} else {
		m_stage_title = _T("No stages");
		m_tree.clear_tree();
		m_text = _T("");
		m_rec_text = _T("");
		m_voice = _T("");
		enable = FALSE;
		m_cur_stage = -1;
	}

	if (m_cur_stage == Debriefing->num_stages - 1)
		GetDlgItem(IDC_NEXT) -> EnableWindow(FALSE);
	else
		GetDlgItem(IDC_NEXT) -> EnableWindow(enable);

	if (m_cur_stage)
		GetDlgItem(IDC_PREV) -> EnableWindow(enable);
	else
		GetDlgItem(IDC_PREV) -> EnableWindow(FALSE);

	if (Debriefing->num_stages >= MAX_DEBRIEF_STAGES)
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(FALSE);
	else
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(TRUE);

	if (Debriefing->num_stages) {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(enable);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(enable);

	} else {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(FALSE);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(FALSE);
	}

	GetDlgItem(IDC_VOICE) -> EnableWindow(enable);
	GetDlgItem(IDC_BROWSE) -> EnableWindow(enable);
	GetDlgItem(IDC_TEXT) -> EnableWindow(enable);
	GetDlgItem(IDC_REC_TEXT) -> EnableWindow(enable);
	GetDlgItem(IDC_TREE) -> EnableWindow(enable);

	m_last_stage = m_cur_stage;
	UpdateData(FALSE);

	#ifndef NDEBUG
	count_free_sexp_nodes();
	#endif
}

void debriefing_editor_dlg::OnNext() 
{
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	m_cur_stage++;
	update_data();
}

void debriefing_editor_dlg::OnPrev() 
{
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	m_cur_stage--;
	update_data();
}

void debriefing_editor_dlg::OnBrowse() 
{
	int z;
	CString name;

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	UpdateData(TRUE);

	if (The_mission.game_type & MISSION_TYPE_TRAINING)
		z = cfile_push_chdir(CF_TYPE_VOICE_TRAINING);
	else
		z = cfile_push_chdir(CF_TYPE_VOICE_DEBRIEFINGS);

	CFileDialog dlg(TRUE, "wav", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK) {
		m_voice = dlg.GetFileName();
		UpdateData(FALSE);
	}

	if (!z)
		cfile_pop_dir();
}

void debriefing_editor_dlg::OnAddStage() 
{
	int i;

	if (Debriefing->num_stages >= MAX_DEBRIEF_STAGES)
		return;

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	m_cur_stage = i = Debriefing->num_stages++;
	copy_stage(i - 1, i, 1);
	update_data(1);
}

void debriefing_editor_dlg::OnDeleteStage() 
{
	int i, z;

	if (m_cur_stage < 0)
		return;

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	Assert(Debriefing->num_stages);
	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	for (i=z+1; i<Debriefing->num_stages; i++) {
		copy_stage(i, i - 1);
	}

	Debriefing->num_stages--;
	m_cur_stage = z;
	if (m_cur_stage >= Debriefing->num_stages)
		m_cur_stage = Debriefing->num_stages - 1;

	update_data(0);
}

void debriefing_editor_dlg::OnInsertStage() 
{
	int i, z;

	if (Debriefing->num_stages >= MAX_DEBRIEF_STAGES)
		return;

	if (!Debriefing->num_stages) {
		OnAddStage();
		return;
	}

	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;

	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	for (i=Debriefing->num_stages; i>z; i--) {
		copy_stage(i - 1, i);
	}

	Debriefing->num_stages++;
	copy_stage(z, z + 1);
	Debriefing->stages[z].formula = -1;
	m_cur_stage = z;
	update_data(0);
}

void debriefing_editor_dlg::copy_stage(int from, int to, int clear_formula)
{
	if ((from < 0) || (from >= Debriefing->num_stages)) {
		Debriefing->stages[to].text = "<Text here>";
		strcpy_s(Debriefing->stages[to].voice, "none.wav");
		Debriefing->stages[to].formula = -1;
		return;
	}
	
	if (clear_formula)
		Debriefing->stages[to].formula = -1;
	else
		Debriefing->stages[to].formula = Debriefing->stages[from].formula;

	Debriefing->stages[to].text = Debriefing->stages[from].text;
	strcpy_s( Debriefing->stages[to].voice, Debriefing->stages[from].voice );
	Debriefing->stages[to].recommendation_text = Debriefing->stages[from].recommendation_text;
}

void debriefing_editor_dlg::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_tree.right_clicked();	
	*pResult = 0;
}

void debriefing_editor_dlg::OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = 1;

	} else
		*pResult = 1;
}

void debriefing_editor_dlg::OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_tree.end_label_edit(pTVDispInfo->item);
}

void debriefing_editor_dlg::OnClose() 
{
	audiostream_close_file(m_voice_id, 0);
	m_voice_id = -1;
	m_cur_stage = -1;
	update_data(1);

	Mission_music[SCORE_DEBRIEF_SUCCESS] = m_debriefPass_music - 1;
	Mission_music[SCORE_DEBRIEF_AVERAGE] = m_debriefAvg_music - 1;
	Mission_music[SCORE_DEBRIEF_FAIL] = m_debriefFail_music - 1;

	CDialog::OnClose();
}

void debriefing_editor_dlg::OnOK()
{
}

BOOL debriefing_editor_dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id;

	// deal with figuring out menu stuff
	id = LOWORD(wParam);
	if ( (id >= ID_TEAM_1) && (id < ID_TEAM_3) ) {
		update_data(1);

		// set the current debriefing
		m_current_debriefing = id - ID_TEAM_1;

		// put user back at first stage for this team (or no current stage is there are none).
		Debriefing = &Debriefings[m_current_debriefing];
		if ( Debriefing->num_stages > 0 )
			m_cur_stage = 0;
		else
			m_cur_stage = -1;

		m_last_stage = -1;
		update_data(0);
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

BOOL debriefing_editor_dlg::DestroyWindow() 
{
	audiostream_close_file(m_voice_id, 0);
	m_play_bm.DeleteObject();
	return CDialog::DestroyWindow();
}

void debriefing_editor_dlg::OnPlay() 
{
	GetDlgItem(IDC_VOICE)->GetWindowText(m_voice);

	if (m_voice_id >= 0) {
		audiostream_close_file(m_voice_id, 0);
		m_voice_id = -1;
		return;
	}

	// we use ASF_EVENTMUSIC here so that it will keep the extension in place
	m_voice_id = audiostream_open((char *)(LPCSTR) m_voice, ASF_EVENTMUSIC);

	if (m_voice_id >= 0) {
		audiostream_play(m_voice_id, 1.0f, 0);
	}
}
