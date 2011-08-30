/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// ship_select dialog

#include "ShipCheckListBox.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "iff_defs/iff_defs.h"

class ship_select : public CDialog
{
// Construction
public:
	void update_status();
	void OnOK();
	void create_list();
	ship_select(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ship_select)
	enum { IDD = IDD_SHIP_SELECT };
	CListBox	m_wing_list;
	CListBox	m_ship_list;
	BOOL	m_filter_ships;
	BOOL	m_filter_starts;
	BOOL	m_filter_waypoints;
	BOOL	m_filter_iff[MAX_IFFS];
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ship_select)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void OnFilterShipsIFF(int iff);

	// Generated message map functions
	//{{AFX_MSG(ship_select)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeWingDisplayFilter();
	afx_msg void OnFilterShips();
	afx_msg void OnFilterStarts();
	afx_msg void OnFilterWaypoints();
	afx_msg void OnFilterShipsIFF0();
	afx_msg void OnFilterShipsIFF1();
	afx_msg void OnFilterShipsIFF2();
	afx_msg void OnFilterShipsIFF3();
	afx_msg void OnFilterShipsIFF4();
	afx_msg void OnFilterShipsIFF5();
	afx_msg void OnFilterShipsIFF6();
	afx_msg void OnFilterShipsIFF7();
	afx_msg void OnFilterShipsIFF8();
	afx_msg void OnFilterShipsIFF9();
	afx_msg void OnClear();
	afx_msg void OnAll();
	afx_msg void OnInvert();
	afx_msg void OnDblclkShipList();
	afx_msg void OnSelchangeWingList();
	afx_msg void OnSelchangeShipList();
	afx_msg void OnDblclkWingList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int activity, list_size, wlist_size, wplist_size;
	object *obj_index[MAX_OBJECTS];
	SCP_vector<int> wing_index;
	SCP_vector<int> wing_sel_last;
};
