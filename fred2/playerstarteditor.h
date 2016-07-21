/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "ShipCheckListBox.h"

/////////////////////////////////////////////////////////////////////////////
// player_start_editor dialog

class player_start_editor : public CDialog
{
// Construction
public:	
	player_start_editor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(player_start_editor)
	enum { IDD = IDD_LOADOUT_EDITOR };
	CSpinButtonCtrl	m_pool_spin;
	CSpinButtonCtrl	m_delay_spin;
	CSpinButtonCtrl	m_spin1;
	CCheckListBox		m_ship_list;
	CCheckListBox		m_weapon_list;	
	CCheckListBox		m_ship_variable_list;
	CCheckListBox		m_weapon_variable_list;	
	int					m_delay;	
	int					m_weapon_pool;
	int					m_ship_pool;
	CComboBox			m_ship_quantity_variable;
	CComboBox			m_weapon_quantity_variable;
	CStatic				m_ships_used_in_wings;
	CStatic				m_weapons_used_in_wings;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(player_start_editor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(player_start_editor)
	virtual BOOL OnInitDialog();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnSelchangeShipList();		
	afx_msg void OnSelchangeWeaponList();	
	afx_msg void OnUpdateShipPool();
	afx_msg void OnUpdateWeaponPool();
	void OnCancel();	
	void OnOK();
	afx_msg void OnSelchangeShipVariablesList();
	afx_msg void OnSelchangeWeaponVariablesList();
	afx_msg void OnSelchangeShipVariablesCombo();
	afx_msg void OnSelchangeWeaponVariablesCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// if we've finished initializing the dialog
	int dlg_inited;

	// team we're currently working with
	int selected_team;
	int previous_team;		 // The last team we worked with

	bool autobalance; 

	// ship pool info
	int static_ship_pool[MAX_TVT_TEAMS][MAX_SHIP_CLASSES];	// Holds the number of ships of a class that was set by the team loadout
	int dynamic_ship_pool[MAX_TVT_TEAMS][MAX_SEXP_VARIABLES];	 
	int static_ship_variable_pool[MAX_TVT_TEAMS][MAX_SHIP_CLASSES];
	int dynamic_ship_variable_pool[MAX_TVT_TEAMS][MAX_SEXP_VARIABLES];

	// weapon pool info
	int static_weapon_pool[MAX_TVT_TEAMS][MAX_WEAPON_TYPES];	
	int dynamic_weapon_pool[MAX_TVT_TEAMS][MAX_SEXP_VARIABLES];	 
	int static_weapon_variable_pool[MAX_TVT_TEAMS][MAX_WEAPON_TYPES];
	int dynamic_weapon_variable_pool[MAX_TVT_TEAMS][MAX_SEXP_VARIABLES];

	// ship and weapon usage pools
	int ship_usage[MAX_TVT_TEAMS][MAX_SHIP_CLASSES];
	int weapon_usage[MAX_TVT_TEAMS][MAX_WEAPON_TYPES];

	// regenerate all controls
	void reset_controls();
	int GetSelectedShipListIndex();
	int GetSelectedShipVariableListIndex();
	int GetSelectedWeaponListIndex();
	int GetSelectedWeaponVariableListIndex();

	void UpdateQuantityVariable(CComboBox *variable_list, int pool_value);

};
