/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "Sexp_tree.h"

/////////////////////////////////////////////////////////////////////////////
// wing_editor dialog

class wing_editor : public CDialog
{
// Construction
public:
	int cue_height;
	int bypass_errors;
	int modified;
	int select_sexp_node;

	void initialize_data_safe(int full_update);
	void update_data_safe();
	void show_hide_sexp_help();
	void calc_cue_height();
	int verify();
	wing_editor(CWnd* pParent = NULL);   // standard constructor
	BOOL Create();
	void OnOK();
	int update_data(int redraw = 1);
	void initialize_data(int full);

// Dialog Data
	//{{AFX_DATA(wing_editor)
	enum { IDD = IDD_WING_EDITOR };
	CSpinButtonCtrl	m_departure_delay_spin;
	CSpinButtonCtrl	m_arrival_delay_spin;
	sexp_tree	m_departure_tree;
	sexp_tree	m_arrival_tree;
	CSpinButtonCtrl	m_threshold_spin;
	CSpinButtonCtrl	m_waves_spin;
	CString	m_wing_name;
	int		m_special_ship;
	int		m_waves;
	int		m_threshold;
	int		m_arrival_location;
	int		m_departure_location;
	int		m_arrival_delay;
	int		m_departure_delay;
	BOOL	m_reinforcement;
	int		m_hotkey;
	BOOL	m_ignore_count;
	int		m_arrival_delay_max;
	int		m_arrival_delay_min;
	int		m_arrival_dist;
	int		m_arrival_target;
	BOOL	m_no_arrival_music;
	int		m_departure_target;
	BOOL	m_no_arrival_message;
	BOOL	m_no_arrival_warp;
	BOOL	m_no_departure_warp;
	BOOL	m_no_dynamic;
	CString	m_wing_squad_filename;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(wing_editor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(wing_editor)
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnDeltaposSpinWaves(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteWing();
	afx_msg void OnDisbandWing();
	afx_msg void OnClose();
	afx_msg void OnGoals2();
	afx_msg void OnReinforcement();
	afx_msg void OnNext();
	afx_msg void OnSelchangedArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHideCues();
	afx_msg void OnPrev();
	afx_msg void OnSelchangeArrivalLocation();
	afx_msg void OnSelchangeDepartureLocation();
	afx_msg void OnSelchangeHotkey();
	afx_msg void OnSquadLogo();
	afx_msg void OnRestrictArrival();
	afx_msg void OnRestrictDeparture();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
