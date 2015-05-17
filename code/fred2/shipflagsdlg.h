/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "ShipEditorDlg.h"

// ShipFlagsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ship_flags_dlg dialog

class ship_flags_dlg : public CDialog
{
// Construction
public:
	ship_flags_dlg(CWnd* pParent = NULL);   // standard constructor
	void OnOK();
	void update_ship(int ship);
	void setup(int n);
	int tristate_set(int val, int cur_state);
	void set_modified();

// Dialog Data
	//{{AFX_DATA(ship_flags_dlg)
	enum { IDD = IDD_SHIP_FLAGS };
	CButton	m_red_alert_carry;
	CButton	m_scannable;
	CButton	m_reinforcement;
	CButton	m_protect_ship;
	CButton	m_beam_protect_ship;
	CButton	m_flak_protect_ship;
	CButton	m_laser_protect_ship;
	CButton	m_missile_protect_ship;
	CButton	m_no_dynamic;
	CButton	m_no_arrival_music;
	CButton	m_kamikaze;
	CButton	m_invulnerable;
	CButton	m_targetable_as_bomb;
	CButton m_immobile;
	CButton	m_ignore_count;
	CButton	m_hidden;
	CButton	m_primitive_sensors;
	CButton	m_no_subspace_drive;
	CButton	m_affected_by_gravity;
	CButton	m_toggle_subsystem_scanning;
	CButton	m_escort;
	CButton	m_destroy;
	CButton	m_cargo_known;
	CButton	m_special_warpin;
	CButton	m_disable_messages;
	CButton m_no_death_scream;
	CButton m_always_death_scream;
	CButton m_guardian;
	CButton m_vaporize;
	CButton m_stealth;
	CButton m_friendly_stealth_invisible;
	CButton m_nav_carry;
	CButton m_nav_needslink;
	CButton m_hide_ship_name;
	CButton m_disable_ets;
	CButton m_cloaked;
	CButton m_no_collide;
	CButton	m_set_class_dynamically;
	CButton	m_scramble_messages;

	CSpinButtonCtrl	m_destroy_spin;
	numeric_edit_control m_kdamage;
	numeric_edit_control m_destroy_value;
	numeric_edit_control m_escort_value;
	numeric_edit_control m_respawn_priority;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ship_flags_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int p_enable;  // used to enable(1)/disable(0) controls based on if a player ship

	// Generated message map functions
	//{{AFX_MSG(ship_flags_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCargoKnown();
	afx_msg void OnDestroyCheck();
	afx_msg void OnEscort();
	afx_msg void OnHiddenFromSensors();
	afx_msg void OnPrimitiveSensors();
	afx_msg void OnNoSubspaceDrive();
	afx_msg void OnAffectedByGravity();
	afx_msg void OnIgnoreCount();
	afx_msg void OnInvulnerable();
	afx_msg void OnTargetableAsBomb();
	afx_msg void OnImmobile();
	afx_msg void OnKamikaze();
	afx_msg void OnNoArrivalMusic();
	afx_msg void OnNoDynamic();
	afx_msg void OnProtectShip();
	afx_msg void OnBeamProtectShip();
	afx_msg void OnFlakProtectShip();
	afx_msg void OnLaserProtectShip();
	afx_msg void OnMissileProtectShip();
	afx_msg void OnReinforcement();
	afx_msg void OnScannable();
	afx_msg void OnRedalertcarry();
	afx_msg void OnToggleSubsystemScanning();
	afx_msg void OnDisableBuiltinShip();
	afx_msg void OnNoDeathScream();
	afx_msg void OnAlwaysDeathScream();
	afx_msg void OnGuardian();
	afx_msg void OnVaporize();
	afx_msg void OnStealth();
	afx_msg void OnFriendlyStealthInvisible();
	afx_msg void OnNavCarry();
	afx_msg void OnNavNeedslink();
	afx_msg void OnHideShipName();
	afx_msg void OnSetClassDynamically();
	afx_msg void OnDisableETS();
	afx_msg void OnCloaked();
	afx_msg void OnScrambleMessages();
	afx_msg void OnNoCollide();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
