/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _SHIPEDITORDLG_H
#define _SHIPEDITORDLG_H

#include "Sexp_tree.h"
#include "ShipGoalsDlg.h"
#include "Management.h"

/////////////////////////////////////////////////////////////////////////////
// CShipEditorDlg dialog

#define	WM_GOODBYE	(WM_USER+5)
#define	ID_ALWAYS_ON_TOP	0x0f00

class numeric_edit_control
{
	int value;
	int unique;
	int control_id;
	CWnd *dlg;

public:
	void setup(int id, CWnd *wnd);
	void blank() { unique = 0; }
	void init(int n);
	void set(int n);
	void display();
	void save(int *n);
	void fix(int n);
};

class CShipEditorDlg : public CDialog
{
private:
	int make_ship_list(int *arr);
	int update_ship(int ship);
	int initialized;
	int multi_edit;
	int always_on_top;
	int cue_height;
	int mission_type;  // indicates if single player(1) or multiplayer(0)
	CView*	m_pSEView;
	CCriticalSection CS_update;

// Construction
public:
	int player_ship, single_ship;
	int editing;
	int modified;
	int select_sexp_node;
	int bypass_errors;
	int bypass_all;

	int enable;  // used to enable(1)/disable(0) controls based on if any ship selected
	int p_enable;  // used to enable(1)/disable(0) controls based on if a player ship

	int tristate_set(int val, int cur_state);
	void show_hide_sexp_help();
	void calc_cue_height();
	int verify();
	void OnInitMenu(CMenu *m);
	void OnOK();
	int update_data(int redraw = 1);
	void initialize_data(int full);
	CShipEditorDlg(CWnd* pParent = NULL);   // standard constructor
	CShipEditorDlg(CView* pView);

	// alternate ship name stuff
	void ship_alt_name_init(int base_ship);
	void ship_alt_name_close(int base_ship);

	// callsign stuff
	void ship_callsign_init(int base_ship);
	void ship_callsign_close(int base_ship);

	BOOL Create();

// Dialog Data
	//{{AFX_DATA(CShipEditorDlg)
	enum { IDD = IDD_SHIP_EDITOR };
	CButton	m_no_departure_warp;
	CButton	m_no_arrival_warp;
	CButton	m_player_ship;
	CSpinButtonCtrl	m_destroy_spin;
	CSpinButtonCtrl	m_departure_delay_spin;
	CSpinButtonCtrl	m_arrival_delay_spin;
	sexp_tree	m_departure_tree;
	sexp_tree	m_arrival_tree;
	CString	m_ship_name;
	CString	m_cargo1;
	int		m_ship_class;
	int		m_team;
	int		m_arrival_location;
	int		m_departure_location;
	int		m_ai_class;
	numeric_edit_control	m_arrival_delay;
	numeric_edit_control	m_departure_delay;
	int		m_hotkey;
	BOOL	m_update_arrival;
	BOOL	m_update_departure;
	numeric_edit_control	m_destroy_value;
	numeric_edit_control	m_score;
	numeric_edit_control	m_assist_score;
	numeric_edit_control	m_arrival_dist;
	numeric_edit_control m_kdamage;
	int		m_arrival_target;
	int		m_departure_target;
	int		m_persona;	
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShipEditorDlg)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShipEditorDlg)
	afx_msg void OnClose();
	afx_msg void OnRclickArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGoals();
	afx_msg void OnSelchangeShipClass();
	afx_msg void OnInitialStatus();
	afx_msg void OnWeapons();
	afx_msg void OnShipReset();
	afx_msg void OnDeleteShip();
	afx_msg void OnShipTbl();
	afx_msg void OnNext();
	afx_msg void OnSelchangedArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHideCues();
	afx_msg void OnPrev();
	afx_msg void OnSelchangeArrivalLocation();
	afx_msg void OnPlayerShip();
	afx_msg void OnNoArrivalWarp();
	afx_msg void OnNoDepartureWarp();
	afx_msg void OnSelchangeDepartureLocation();
	afx_msg void OnSelchangeHotkey();
	afx_msg void OnFlags();
	afx_msg void OnIgnoreOrders();
	afx_msg void OnSpecialExp();
	afx_msg void OnTextures();
	afx_msg void OnSpecialHitpoints();
	afx_msg void OnAltShipClass();
	afx_msg void OnSetAsPlayerShip();
	afx_msg void OnRestrictArrival();
	afx_msg void OnRestrictDeparture();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
