// MusicPlayerDlg.h : header file

#ifndef _MUSICPLAYERDLG_H
#define _MUSICPLAYERDLG_H

class music_player_dlg : public CDialog {
  public:
	music_player_dlg(CWnd* pParent = nullptr);
	BOOL Create();
	void UpdateSelection();
	void PlayMusic();
	void StopMusic();
	bool SelectNextTrack();
	bool SelectPrevTrack();
	bool IsPlayerActive();
	void DoFrame();

	enum {
		IDD = IDD_MUSIC_PLAYER
	};

  protected:
	CListBox m_music_list;
	CString m_music_item;
	int m_music_id;
	int m_cursor_pos;
	int m_autoplay;

	HICON m_play_icon;
	HICON m_stop_icon;
	HICON m_next_icon;
	HICON m_prev_icon;

	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	virtual BOOL DestroyWindow();

	virtual BOOL OnInitDialog();
	afx_msg void OnSelMusicList();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnNextTrack();
	afx_msg void OnPreviousTrack();
	afx_msg void OnMusicTbl();
	afx_msg void OnClose();
	afx_msg void OnAutoplay();

	DECLARE_MESSAGE_MAP()
};

#endif // _MUSICPLAYERDLG_H
