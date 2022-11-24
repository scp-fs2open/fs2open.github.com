/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "ship/ship.h"

/////////////////////////////////////////////////////////////////////////////
// WeaponEditorDlg dialog

class WeaponEditorDlg : public CDialog
{
// Construction
public:
	void update_pilot();
	void OnCancel();
	void change_selection();
	void OnOK();
	WeaponEditorDlg(CWnd* pParent = NULL);   // standard constructor

	std::array<int, MAX_SHIP_SECONDARY_BANKS> m_ammo_max;
	int m_last_item;
	int m_ship;
	int m_ship_class;
	int m_multi_edit;
	ship_weapon pilot, *cur_weapon;

	std::array<int, MAX_SHIP_PRIMARY_BANKS> m_IDC_GUN;
	std::array<int, MAX_SHIP_SECONDARY_BANKS> m_IDC_MISSILE;
	std::array<int, MAX_SHIP_SECONDARY_BANKS> m_IDC_AMMO;
	std::array<int, MAX_SHIP_SECONDARY_BANKS> m_IDC_SPIN;

// Dialog Data
	//{{AFX_DATA(WeaponEditorDlg)
	enum { IDD = IDD_WEAPON_EDITOR };
	std::array<int, MAX_SHIP_SECONDARY_BANKS> m_ammo;
	std::array<CSpinButtonCtrl, MAX_SHIP_SECONDARY_BANKS> m_spin;
	int		m_ai_class;
	std::array<int, MAX_SHIP_PRIMARY_BANKS> m_gun;
	std::array<int, MAX_SHIP_SECONDARY_BANKS> m_missile;
	int		m_cur_item;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WeaponEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WeaponEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnClose();
	afx_msg void OnSelchangeMissile1();
	afx_msg void OnSelchangeMissile2();
	afx_msg void OnSelchangeMissile3();
	afx_msg void OnSelchangeMissile4();
	afx_msg void OnSelchangeMissile(int secondary_index);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int combo_index_to_weapon_class(int dialog_id, int combo_index);
	int weapon_class_to_combo_index(int dialog_id, int weapon_class);
};
