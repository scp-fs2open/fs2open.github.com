/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __BRIEFINGEDITORDLG_H__
#define __BRIEFINGEDITORDLG_H__

#include "parse/sexp.h"
#include "mission/missionbriefcommon.h"

/////////////////////////////////////////////////////////////////////////////
// briefing_editor_dlg dialog

class briefing_editor_dlg : public CDialog
{
// Construction
public:
	void focus_sexp(int select_sexp_node);
	int calc_num_lines_for_icons(int num);
	void batch_render();
	void save_editor_state();
	void restore_editor_state();
	void reset_icon_loop(int stage);
	int get_next_icon(int id);
	void OnOK();
	void OnCancel();
	int find_icon(int id, int stage);
	void propagate_icon(int num);
	void reset_editor();
	int check_mouse_hit(int x, int y);
	void delete_icon(int num);
	void update_positions();
	void icon_select(int num);
	void draw_icon(object *objp);
	void create();
	void update_data(int update = 1);
	briefing_editor_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(briefing_editor_dlg)
	enum { IDD = IDD_BRIEFING_EDITOR };
	sexp_tree	m_tree;
	CButton	m_lines;
	BOOL	m_hilight;
	int		m_icon_image;
	CString	m_icon_label;
	CString	m_stage_title;
	CString	m_text;
	CString	m_time;
	CString	m_voice;
	CString	m_icon_text;
	int		m_icon_team;
	int		m_ship_type;
	BOOL	m_change_local;
	int		m_id;
	int		m_briefing_music;
	CString	m_substitute_briefing_music;
	BOOL	m_cut_next;
	BOOL	m_cut_prev;
	int		m_current_briefing;
	BOOL	m_flipicon;
	//}}AFX_DATA

	CBitmap m_play_bm;

	// copy view variables
	int		m_copy_view_set;
	vec3d	m_copy_view_pos;
	matrix	m_copy_view_orient;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(briefing_editor_dlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	brief_icon *iconp;
	briefing *save_briefing;
	int icon_loop, stage_loop;
	int m_cur_stage;
	int m_last_stage;
	int m_cur_icon;
	int m_last_icon;
	int m_voice_id;
	int stage_saved;
	int icon_saved;
	int modified;
//	int point_obj;
	int icon_obj[MAX_STAGE_ICONS];
	int icon_marked[MAX_STAGE_ICONS];
	int line_marked[MAX_BRIEF_STAGE_LINES];

	void copy_stage(int from, int to);

	// Generated message map functions
	//{{AFX_MSG(briefing_editor_dlg)
	afx_msg void OnClose();
	afx_msg void OnNext();
	afx_msg void OnPrev();
	afx_msg void OnBrowse();
	afx_msg void OnAddStage();
	afx_msg void OnDeleteStage();
	afx_msg void OnInsertStage();
	afx_msg void OnMakeIcon();
	afx_msg void OnDeleteIcon();
	afx_msg void OnGotoView();
	afx_msg void OnSaveView();
	afx_msg void OnSelchangeIconImage();
	afx_msg void OnSelchangeTeam();
	afx_msg void OnPropagateIcons();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnLines();
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPlay();
	afx_msg void OnCopyView();
	afx_msg void OnPasteView();
	afx_msg void OnFlipIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

#endif
