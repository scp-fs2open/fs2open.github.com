/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// CMissionNotesDlg dialog
#pragma once
class CMissionNotesDlg : public CDialog
{
// Construction
public:
	int query_modified();
	void OnCancel();
	void OnOK();
	int update_data();
	int initialize_data();
	CMissionNotesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMissionNotesDlg)
	enum { IDD = IDD_MISSION_NOTES };
	CSpinButtonCtrl	m_respawn_spin;
	CSpinButtonCtrl	m_max_respawn_delay_spin;
	CString	m_created;
	CString	m_modified;
	CString	m_mission_notes;
	CString	m_designer_name;
	CString	m_mission_title;
	CString	m_mission_desc;
	CString	m_squad_filename;
	CString	m_squad_name;
	CString m_loading_640;
	CString m_loading_1024;
	int		m_ai_profile;
	int		m_event_music;
	CString	m_substitute_event_music;
	int		m_command_persona;
	CString	m_command_sender;
	BOOL		m_full_war;
	BOOL		m_red_alert;
	BOOL		m_scramble;
	BOOL		m_daisy_chained_docking;
	UINT		m_num_respawns;
	int			m_max_respawn_delay;
	int			m_disallow_support;
	BOOL		m_no_promotion;
	BOOL		m_no_builtin_msgs;
	BOOL		m_no_builtin_command_msgs;
	BOOL		m_no_traitor;
	BOOL		m_toggle_trails;
	BOOL		m_support_repairs_hull;
	BOOL		m_beam_free_all_by_default;
	BOOL		m_player_start_using_ai;
	BOOL		m_no_briefing;
	BOOL		m_no_debriefing;
	BOOL		m_autpilot_cinematics;
	BOOL		m_no_autpilot;
	BOOL		m_2d_mission;
	float		m_max_hull_repair_val;
	float		m_max_subsys_repair_val;
	BOOL		m_contrail_threshold_flag;
	int			m_contrail_threshold;
	//}}AFX_DATA

	CString	m_mission_notes_orig;
	CString	m_designer_name_orig;
	CString	m_mission_title_orig;
	CString	m_mission_desc_orig;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissionNotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_type;
	void set_types();

	// Generated message map functions
	//{{AFX_MSG(CMissionNotesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnTraining();
	afx_msg void OnMulti();
	afx_msg void OnSingle();
	afx_msg void OnSquadLogo();
	afx_msg void OnLoad640();
	afx_msg void OnLoad1024();
	afx_msg void OnToggleContrailThreshold();
	afx_msg void OnCustomWingNames();
	afx_msg void OnSoundEnvironment();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeLoadingScreen641();
};
