// MusicPlayerDlg.h : header file

#ifndef _MUSICPLAYERDLG_H
#define _MUSICPLAYERDLG_H

class music_player_dlg : public CDialog {
  public:
	music_player_dlg(CWnd* pParent = nullptr);
	BOOL Create();

	// Dialog Data
	//{{AFX_DATA(calc_relative_coords_dlg)
	enum {
		IDD = IDD_MUSIC_PLAYER
	};
	CListBox m_music_list;
	SCP_string m_music_item;
	int m_music_id;
	//}}AFX_DATA

	SCP_vector<int> m_object_indexes;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	//{{AFX_MSG(calc_relative_coords_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelMusicList();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnMusicTbl();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

}
;
#endif // _MUSICPLAYERDLG_H