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
	void update_status(bool first_time = false);
	void OnOK();
	void create_list();
	ship_select(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ship_select)
	enum { IDD = IDD_SHIP_SELECT };
	CListBox	m_iff_list;
	CListBox	m_ship_list;
	CListBox	m_wing_and_waypoint_list;
	BOOL	m_filter_ships;
	BOOL	m_filter_starts;
	BOOL	m_filter_waypoints;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ship_select)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ship_select)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeWingDisplayFilter();
	afx_msg void OnFilterShips();
	afx_msg void OnFilterStarts();
	afx_msg void OnFilterWaypoints();
	afx_msg void OnSelchangeIFFList();
	afx_msg void OnClear();
	afx_msg void OnAll();
	afx_msg void OnInvert();
	afx_msg void OnDblclkShipList();
	afx_msg void OnSelchangeWingAndWaypointList();
	afx_msg void OnSelchangeShipList();
	afx_msg void OnDblclkWingList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// helper function
	int find_index_with_objnum(int objnum);

private:
	int activity, num_wings;
	SCP_vector<int> obj_index;
	SCP_vector<int> wing_and_waypoint_index;
	SCP_vector<bool> wing_and_waypoint_sel_last;
};
