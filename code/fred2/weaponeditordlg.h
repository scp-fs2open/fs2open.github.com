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

	int m_ammo_max1;
	int m_ammo_max2;
	int m_ammo_max3;
	int m_ammo_max4;
	int m_last_item;
	int m_ship;
	int m_ship_class;
	int m_multi_edit;
	ship_weapon pilot, *cur_weapon;

// Dialog Data
	//{{AFX_DATA(WeaponEditorDlg)
	enum { IDD = IDD_WEAPON_EDITOR };
	CSpinButtonCtrl	m_spin4;
	CSpinButtonCtrl	m_spin3;
	CSpinButtonCtrl	m_spin2;
	CSpinButtonCtrl	m_spin1;
	int		m_ai_class;
	int		m_ammo1;
	int		m_ammo2;
	int		m_ammo3;
	int		m_ammo4;
	int		m_gun1;
	int		m_gun2;
	int		m_gun3;
	int		m_missile1;
	int		m_missile2;
	int		m_missile3;
	int		m_missile4;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
