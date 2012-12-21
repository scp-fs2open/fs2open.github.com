/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "mission/missionbriefcommon.h"

/////////////////////////////////////////////////////////////////////////////
// debriefing_editor_dlg dialog

class debriefing_editor_dlg : public CDialog
{
// Construction
public:
	void OnOK();
	void update_data(int update = 1);
	debriefing_editor_dlg(CWnd* pParent = NULL);   // standard constructor
	int select_sexp_node;

// Dialog Data
	//{{AFX_DATA(debriefing_editor_dlg)
	enum { IDD = IDD_DEBRIEFING_EDITOR };
	sexp_tree	m_tree;
	CString	m_text;
	CString	m_voice;
	CString	m_stage_title;
	CString	m_rec_text;
	int		m_debriefPass_music;
	int		m_debriefAvg_music;
	int		m_debriefFail_music;
	int		m_current_debriefing;
	//}}AFX_DATA

	CBitmap m_play_bm;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(debriefing_editor_dlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_cur_stage;
	int m_last_stage;
	int m_voice_id;
	int modified;

	void copy_stage(int from, int to, int clear_formula = 0);

	// Generated message map functions
	//{{AFX_MSG(debriefing_editor_dlg)
	afx_msg void OnNext();
	afx_msg void OnPrev();
	afx_msg void OnBrowse();
	afx_msg void OnAddStage();
	afx_msg void OnDeleteStage();
	afx_msg void OnInsertStage();
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnPlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};
