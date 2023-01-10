// MusicPlayerDlg.h : header file

#ifndef _MUSICPLAYERDLG_H
#define _MUSICPLAYERDLG_H

class music_player_dlg : public CDialog {
  public:
	music_player_dlg(CWnd* pParent = nullptr);
	BOOL Create();
	afx_msg void UpdateSelection();
	afx_msg void PlayMusic();

	// Dialog Data
	//{{AFX_DATA(calc_relative_coords_dlg)
	enum {
		IDD = IDD_MUSIC_PLAYER
	};
	CListBox m_music_list;
	SCP_string m_music_item;
	int m_music_id;
	int m_cursor_pos;
	SCP_vector<SCP_string> m_player_list;
	int m_autoplay;
	int m_num_music_files;
	//}}AFX_DATA

	protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	//{{AFX_MSG(calc_relative_coords_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void StopMusic();
	afx_msg void OnSelMusicList();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnNextTrack();
	afx_msg void OnPreviousTrack();
	afx_msg void OnMusicTbl();
	afx_msg void OnClose();
	afx_msg void OnAutoplay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

}
;
#endif // _MUSICPLAYERDLG_H