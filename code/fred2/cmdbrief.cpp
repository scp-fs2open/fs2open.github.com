/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/CmdBrief.cpp $
 * $Revision: 1.1.2.3 $
 * $Date: 2007-04-08 08:28:20 $
 * $Author: karajorma $
 *
 * Command Briefing Editor
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.2.2  2007/03/07 22:36:51  karajorma
 * Make .ogg selectable in FRED.
 *
 * Revision 1.1.2.1  2006/10/24 13:41:54  taylor
 * fix the cmdbrief editor deleting the wrong stage (Mantis bug #1115)
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.3  2005/10/22 05:54:48  wmcoolmon
 * Fixed cf_find_file_location
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:53  inquisitor
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
 * 8     4/30/98 8:23p John
 * Fixed some bugs with Fred caused by my new cfile code.
 * 
 * 7     4/22/98 9:56a Sandeep
 * 
 * 6     4/20/98 4:40p Hoffoss
 * Added a button to 4 editors to play the chosen wave file.
 * 
 * 5     4/03/98 12:39p Hoffoss
 * Changed starting directory for browse buttons in several editors.
 * 
 * 4     3/19/98 4:24p Hoffoss
 * Added remaining support for command brief screen (ANI and WAVE file
 * playing).
 * 
 * 3     3/06/98 2:36p Hoffoss
 * Placed correct text size limits on edit boxes.
 * 
 * 2     3/05/98 3:59p Hoffoss
 * Added a bunch of new command brief stuff, and asteroid initialization
 * to Fred.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "CmdBrief.h"
#include "cfile/cfile.h"
#include "sound/audiostr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
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

	DDV_MaxChars(pDX, m_text, CMD_BRIEF_TEXT_MAX - 1);
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
		char buf[CMD_BRIEF_TEXT_MAX];

		if (last_cmd_brief->stage[m_last_stage].text)
			free(last_cmd_brief->stage[m_last_stage].text);

		deconvert_multiline_string(buf, m_text, CMD_BRIEF_TEXT_MAX);
		last_cmd_brief->stage[m_last_stage].text = strdup(buf);
		string_copy(last_cmd_brief->stage[m_last_stage].ani_filename, m_ani_filename, MAX_FILENAME_LEN);
		string_copy(last_cmd_brief->stage[m_last_stage].wave_filename, m_wave_filename, MAX_FILENAME_LEN);
	}

	// load data of new stage into dialog
	if (Cur_cmd_brief && Cur_cmd_brief->num_stages > 0) {
		if (m_cur_stage < 0 || m_cur_stage >= Cur_cmd_brief->num_stages)
			m_cur_stage = 0;

		m_stage_title.Format("Stage %d of %d", m_cur_stage + 1, Cur_cmd_brief->num_stages);
		m_text = convert_multiline_string(Cur_cmd_brief->stage[m_cur_stage].text);
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
	if (Cur_cmd_brief->stage[z].text)
		free(Cur_cmd_brief->stage[z].text);

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
		Cur_cmd_brief->stage[to].text = strdup("<Text here>");
		strcpy(Cur_cmd_brief->stage[to].ani_filename, "<default>");
		strcpy(Cur_cmd_brief->stage[to].wave_filename, "none");
		return;
	}

	Cur_cmd_brief->stage[to] = Cur_cmd_brief->stage[from];
	Cur_cmd_brief->stage[to].text = strdup(Cur_cmd_brief->stage[from].text);
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
