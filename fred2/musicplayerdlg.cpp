// MusicPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "MainFrm.h"
#include "musicplayerdlg.h"
#include "TextViewDlg.h"
#include "mod_table/mod_table.h"
#include "sound/audiostr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

music_player_dlg::music_player_dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(music_player_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(calc_relative_coords_dlg)
	m_music_item = "";
	m_music_id = -1;
	m_cursor_pos = -1;
	m_player_list = {};
	m_autoplay = FALSE;
	m_num_music_files = 0;
	//}}AFX_DATA_INIT
}

void music_player_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(calc_relative_coords_dlg)
	DDX_Control(pDX, IDC_MUSIC_LIST, m_music_list);
	DDX_Check(pDX, IDC_MUSIC_AUTOPLAY, m_autoplay);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(music_player_dlg, CDialog)
//{{AFX_MSG_MAP(music_player_dlg)
ON_LBN_SELCHANGE(IDC_MUSIC_LIST, OnSelMusicList)
ON_BN_CLICKED(IDC_BUTTON_PLAY_MUSIC, OnPlay)
ON_BN_CLICKED(IDC_BUTTON_STOP_MUSIC, OnStop)
ON_BN_CLICKED(IDC_BUTTON_NEXT_MUSIC, OnNextTrack)
ON_BN_CLICKED(IDC_BUTTON_PREV_MUSIC, OnPreviousTrack)
ON_BN_CLICKED(IDC_BUTTON_MUSIC_TBL, OnMusicTbl)
ON_BN_CLICKED(IDC_MUSIC_AUTOPLAY, OnAutoplay)
ON_WM_CLOSE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL music_player_dlg::Create()
{
	BOOL r;
	r = CDialog::Create(IDD, Fred_main_wnd);

	m_play_bm.LoadBitmap(IDB_PLAY);
	((CButton*)GetDlgItem(IDC_BUTTON_PLAY_MUSIC))->SetBitmap(m_play_bm);

	m_stop_bm.LoadBitmap(IDB_STOP);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP_MUSIC))->SetBitmap(m_stop_bm);

	m_next_bm.LoadBitmap(IDB_NEXT);
	((CButton*)GetDlgItem(IDC_BUTTON_NEXT_MUSIC))->SetBitmap(m_next_bm);

	m_prev_bm.LoadBitmap(IDB_PREV);
	((CButton*)GetDlgItem(IDC_BUTTON_PREV_MUSIC))->SetBitmap(m_prev_bm);

	return r;
}

BOOL music_player_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_music_list.ResetContent();

	SCP_vector<SCP_string> files;
	cf_get_file_list(files, CF_TYPE_MUSIC, "*", CF_SORT_NAME);

	for (auto& file : files) {
		bool addItem = true;

		// check if the file was asked to be ignored in game_settings
		for (auto& thisFile : Ignored_music_player_files) {
			if (!stricmp(thisFile.c_str(), file.c_str())) {
				addItem = false;
				break;
			}
		}

		//add item if not ignored
		if (addItem) {
			m_music_list.AddString(file.c_str());
			m_player_list.push_back(file);
			m_num_music_files++;
		}
	}

	return TRUE;
}

void music_player_dlg::PlayMusic()
{
	StopMusic();
	
	// cfile strips the extension so first let's try .wav
	SCP_string thisMusic = m_music_item + ".wav";
	m_music_id = audiostream_open(thisMusic.c_str(), ASF_EVENTMUSIC);

	// if no file was loaded then it must be .ogg
	if (m_music_id < 0) {
		thisMusic = m_music_item + ".ogg";
		m_music_id = audiostream_open(thisMusic.c_str(), ASF_EVENTMUSIC);
	}

	if (m_music_id >= 0) {
		audiostream_play(m_music_id, 1.0f, 0);
	} else {
		return;
	}
}

void music_player_dlg::StopMusic()
{
	if (m_music_id >= 0) {
		audiostream_close_file(m_music_id, 0);
		m_music_id = -1;
	}
}

bool music_player_dlg::SelectNextTrack()
{
	if ((m_cursor_pos >= 0) && (m_cursor_pos < (m_num_music_files - 1))) {
		m_cursor_pos++;
		m_music_list.SetCurSel(m_cursor_pos);
		UpdateSelection();
		return true;
	}
	
	return false;
}

bool music_player_dlg::SelectPrevTrack()
{
	if ((m_cursor_pos > 0) && (m_cursor_pos <= (m_num_music_files - 1))) {
		m_cursor_pos--;
		m_music_list.SetCurSel(m_cursor_pos);
		UpdateSelection();
		return true;
	}

	return false;
}

void music_player_dlg::UpdateSelection()
{
	m_cursor_pos = m_music_list.GetCurSel();
	m_music_item = m_player_list[m_cursor_pos];
}

void music_player_dlg::OnSelMusicList()
{
	UpdateSelection();
}

void music_player_dlg::OnPlay()
{
	PlayMusic();
}

void music_player_dlg::OnStop()
{
	StopMusic();
}

void music_player_dlg::OnNextTrack()
{
	if (SelectNextTrack() && audiostream_is_playing(m_music_id)) {
		PlayMusic();
	}
}

void music_player_dlg::OnPreviousTrack()
{
	if (SelectPrevTrack() && audiostream_is_playing(m_music_id)) {
		PlayMusic();
	}
}

void music_player_dlg::OnMusicTbl()
{
	TextViewDlg dlg;

	dlg.LoadMusicTblText();
	dlg.DoModal();
}

void music_player_dlg::OnClose()
{
	StopMusic();

	CDialog::OnClose();
}

void music_player_dlg::OnAutoplay()
{
	if (m_autoplay == 1)
		m_autoplay = 0;
	else
		m_autoplay = 1;

	((CButton*)GetDlgItem(IDC_MUSIC_AUTOPLAY))->SetCheck(m_autoplay);
}
