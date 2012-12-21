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
#include "CmdBrief.h"
#include "cfile/cfile.h"
#include "sound/audiostr.h"
#include "localization/localize.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cmd_brief_dlg dialog

cmd_brief_dlg::cmd_brief_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(cmd_brief_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(cmd_brief_dlg)
	m_ani_filename = _T("");
	m_text = _T("");
	m_stage_title = _T("");
	m_wave_filename = _T("");
	//}}AFX_DATA_INIT

	m_wave_id = -1;
}

void cmd_brief_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cmd_brief_dlg)
	DDX_Text(pDX, IDC_ANI_FILENAME, m_ani_filename);
	DDX_Text(pDX, IDC_TEXT, m_text);
	DDX_Text(pDX, IDC_STAGE_TITLE, m_stage_title);
	DDX_Text(pDX, IDC_WAVE_FILENAME, m_wave_filename);
	//}}AFX_DATA_MAP

	DDV_MaxChars(pDX, m_ani_filename, MAX_FILENAME_LEN - 1);
	DDV_MaxChars(pDX, m_wave_filename, MAX_FILENAME_LEN - 1);
}

BEGIN_MESSAGE_MAP(cmd_brief_dlg, CDialog)
	//{{AFX_MSG_MAP(cmd_brief_dlg)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_BN_CLICKED(IDC_ADD_STAGE, OnAddStage)
	ON_BN_CLICKED(IDC_INSERT_STAGE, OnInsertStage)
	ON_BN_CLICKED(IDC_DELETE_STAGE, OnDeleteStage)
	ON_BN_CLICKED(IDC_BROWSE_ANI, OnBrowseAni)
	ON_BN_CLICKED(IDC_BROWSE_WAVE, OnBrowseWave)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cmd_brief_dlg message handlers

BOOL cmd_brief_dlg::OnInitDialog() 
{
	Cur_cmd_brief = Cmd_briefs;  // default to first cmd briefing
	m_cur_stage = 0;
	last_cmd_brief = NULL;

	CDialog::OnInitDialog();
	m_play_bm.LoadBitmap(IDB_PLAY);
	((CButton *) GetDlgItem(IDC_PLAY)) -> SetBitmap(m_play_bm);

	update_data();
	return TRUE;
}

void cmd_brief_dlg::update_data(int update)
{
	int enable;

	if (update)
		UpdateData(TRUE);

	// save previously editing data before we load over it.
	if (last_cmd_brief && m_last_stage >= 0 && m_last_stage < last_cmd_brief->num_stages) {
		cmd_brief_stage *last_stage = &last_cmd_brief->stage[m_last_stage];

		deconvert_multiline_string(last_stage->text, m_text);
		lcl_fred_replace_stuff(last_stage->text);

		string_copy(last_stage->ani_filename, m_ani_filename, MAX_FILENAME_LEN);
		string_copy(last_stage->wave_filename, m_wave_filename, MAX_FILENAME_LEN);
	}

	// load data of new stage into dialog
	if (Cur_cmd_brief && Cur_cmd_brief->num_stages > 0) {
		if (m_cur_stage < 0 || m_cur_stage >= Cur_cmd_brief->num_stages)
			m_cur_stage = 0;

		m_stage_title.Format("Stage %d of %d", m_cur_stage + 1, Cur_cmd_brief->num_stages);
		convert_multiline_string(m_text, Cur_cmd_brief->stage[m_cur_stage].text);
		m_ani_filename = Cur_cmd_brief->stage[m_cur_stage].ani_filename;
		m_wave_filename = Cur_cmd_brief->stage[m_cur_stage].wave_filename;
		enable = TRUE;

	} else {
		m_stage_title = _T("No stages");
		m_text = _T("");
		m_ani_filename = _T("");
		m_wave_filename = _T("");
		enable = FALSE;
		m_cur_stage = -1;
	}

	if (m_cur_stage < Cur_cmd_brief->num_stages - 1)
		GetDlgItem(IDC_NEXT) -> EnableWindow(enable);
	else
		GetDlgItem(IDC_NEXT) -> EnableWindow(FALSE);

	if (m_cur_stage)
		GetDlgItem(IDC_PREV) -> EnableWindow(enable);
	else
		GetDlgItem(IDC_PREV) -> EnableWindow(FALSE);

	if (Cur_cmd_brief->num_stages >= CMD_BRIEF_STAGES_MAX)
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(FALSE);
	else
		GetDlgItem(IDC_ADD_STAGE) -> EnableWindow(TRUE);

	if (Cur_cmd_brief->num_stages) {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(enable);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(enable);

	} else {
		GetDlgItem(IDC_DELETE_STAGE) -> EnableWindow(FALSE);
		GetDlgItem(IDC_INSERT_STAGE) -> EnableWindow(FALSE);
	}

	GetDlgItem(IDC_WAVE_FILENAME) -> EnableWindow(enable);
	GetDlgItem(IDC_ANI_FILENAME) -> EnableWindow(enable);
	GetDlgItem(IDC_BROWSE_ANI) -> EnableWindow(enable);
	GetDlgItem(IDC_BROWSE_WAVE) -> EnableWindow(enable);
	GetDlgItem(IDC_TEXT) -> EnableWindow(enable);

	UpdateData(FALSE);

	last_cmd_brief = Cur_cmd_brief;
	m_last_stage = m_cur_stage;
}

void cmd_brief_dlg::OnOK()
{
	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	update_data();
	CDialog::OnOK();
}

void cmd_brief_dlg::OnNext() 
{
	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	m_cur_stage++;
	update_data();
}

void cmd_brief_dlg::OnPrev() 
{
	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	m_cur_stage--;
	update_data();
}

void cmd_brief_dlg::OnAddStage() 
{
	int i;

	if (Cur_cmd_brief->num_stages >= CMD_BRIEF_STAGES_MAX)
		return;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	m_cur_stage = i = Cur_cmd_brief->num_stages++;
	copy_stage(i - 1, i);
	update_data(1);
}

void cmd_brief_dlg::OnInsertStage() 
{
	int i, z;

	if (Cur_cmd_brief->num_stages >= CMD_BRIEF_STAGES_MAX)
		return;

	if (!Cur_cmd_brief->num_stages) {
		OnAddStage();
		return;
	}

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	for (i=Cur_cmd_brief->num_stages; i>z; i--)
		Cur_cmd_brief->stage[i] = Cur_cmd_brief->stage[i - 1];

	Cur_cmd_brief->num_stages++;
	copy_stage(z, z + 1);
	m_cur_stage = z;
	m_last_stage = -1;
	update_data(0);
}

void cmd_brief_dlg::OnDeleteStage() 
{
	int i, z;

	if (m_cur_stage < 0)
		return;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	Assert(Cur_cmd_brief->num_stages);
	z = m_cur_stage;
	m_cur_stage = -1;
	update_data(1);
	Cur_cmd_brief->stage[z].text = "";

	for (i=z+1; i<Cur_cmd_brief->num_stages; i++)
		Cur_cmd_brief->stage[i-1] = Cur_cmd_brief->stage[i];

	Cur_cmd_brief->num_stages--;
	m_cur_stage = z;
	m_last_stage = -1;

	if (m_cur_stage >= Cur_cmd_brief->num_stages)
		m_cur_stage = Cur_cmd_brief->num_stages - 1;

	update_data(0);
}

void cmd_brief_dlg::copy_stage(int from, int to)
{
	if ((from < 0) || (from >= Cur_cmd_brief->num_stages)) {
		Cur_cmd_brief->stage[to].text = "<Text here>";
		strcpy_s(Cur_cmd_brief->stage[to].ani_filename, "<default>");
		strcpy_s(Cur_cmd_brief->stage[to].wave_filename, "none");
		return;
	}

	Cur_cmd_brief->stage[to] = Cur_cmd_brief->stage[from];
	Cur_cmd_brief->stage[to].text = Cur_cmd_brief->stage[from].text;
}

void cmd_brief_dlg::OnBrowseAni() 
{
	int z;
	CString name;	

	UpdateData(TRUE);
	z = cfile_push_chdir(CF_TYPE_INTERFACE);
	CFileDialog dlg(TRUE, "ani", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Ani Files (*.ani)|*.ani|Avi Files (*.avi)|*.avi|Both (*.ani, *.avi)|*.ani;*.avi||");

	if (dlg.DoModal() == IDOK) {
		m_ani_filename = dlg.GetFileName();
		UpdateData(FALSE);
	}

	if (!z)
		cfile_pop_dir();
}

void cmd_brief_dlg::OnBrowseWave() 
{
	int z;
	CString name;

	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	UpdateData(TRUE);
	z = cfile_push_chdir(CF_TYPE_VOICE_CMD_BRIEF);
	CFileDialog dlg(TRUE, "wav", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
		"Voice Files (*.ogg, *.wav)|*.ogg;*.wav|Ogg Vorbis Files (*.ogg)|*.ogg|Wave Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK) {
		m_wave_filename = dlg.GetFileName();
		UpdateData(FALSE);
	}

	if (!z)
		cfile_pop_dir();
}

BOOL cmd_brief_dlg::DestroyWindow() 
{
	audiostream_close_file(m_wave_id, 0);
	m_wave_id = -1;

	m_play_bm.DeleteObject();
	return CDialog::DestroyWindow();
}

void cmd_brief_dlg::OnPlay() 
{
	GetDlgItem(IDC_WAVE_FILENAME)->GetWindowText(m_wave_filename);

	if (m_wave_id >= 0) {
		audiostream_close_file(m_wave_id, 0);
		m_wave_id = -1;
		return;
	}

	// we use ASF_EVENTMUSIC here so that it will keep the extension in place
	m_wave_id = audiostream_open((char *)(LPCSTR) m_wave_filename, ASF_EVENTMUSIC);

	if (m_wave_id >= 0) {
		audiostream_play(m_wave_id, 1.0f, 0);
	}
}
