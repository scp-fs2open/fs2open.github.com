// MusicPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "musicplayerdlg.h"
#include "sound/audiostr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//music_player_dlg* Music_player = NULL;

SCP_vector<SCP_string> Music_player_list;

music_player_dlg::music_player_dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(music_player_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(calc_relative_coords_dlg)
	m_music_item = "";
	m_music_id = -1;
	//}}AFX_DATA_INIT
}

void music_player_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(calc_relative_coords_dlg)
	DDX_Control(pDX, IDC_MUSIC_LIST, m_music_list);

	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(music_player_dlg, CDialog)
//{{AFX_MSG_MAP(music_player_dlg)
ON_LBN_SELCHANGE(IDC_MUSIC_LIST, OnSelchangeOriginList)
ON_BN_CLICKED(IDC_BUTTON_PLAY_MUSIC, OnPlay)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL music_player_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_music_list.ResetContent();

	m_object_indexes.clear();

	SCP_vector<SCP_string> files;
	cf_get_file_list(files, CF_TYPE_MUSIC, "*", CF_SORT_NAME);

	for (auto& file : files) {
		m_music_list.AddString(file.c_str());
		Music_player_list.push_back(file);
	}

	return TRUE;
}

void music_player_dlg::OnSelchangeOriginList()
{
	m_music_item = Music_player_list[m_music_list.GetCurSel()];
	//m_music_item.SetString(this_item.c_str());
	//m_music_item.Format("%s", this_item.c_str());
}

void music_player_dlg::OnPlay()
{

	if (m_music_id >= 0) {
		audiostream_close_file(m_music_id, 0);
		m_music_id = -1;
		return;
	}
	mprintf(("Trying to play music %s\n", m_music_item.c_str()));

	SCP_string thisMusic = m_music_item + ".wav";

	// we use ASF_EVENTMUSIC here so that it will keep the extension in place
	m_music_id = audiostream_open(thisMusic.c_str(), ASF_EVENTMUSIC);

	if (m_music_id >= 0) {
		audiostream_play(m_music_id, 1.0f, 0);
	}

}