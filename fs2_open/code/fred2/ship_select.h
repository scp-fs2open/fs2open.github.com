/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/ship_select.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Object selection (marking) dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
 *
 * Revision 1.4  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.3  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 10    9/09/97 10:29a Hoffoss
 * Added support for neutral team, and fixed changes made to how team is
 * used in ship structure.
 * 
 * 9     8/26/97 11:08a Hoffoss
 * Added waypoint paths to object selection dialog.
 * 
 * 8     7/28/97 5:10p Hoffoss
 * Removed all occurances of neutral team from Fred.
 * 
 * 7     7/24/97 2:43p Hoffoss
 * Made changes whiteside requested.  Double clicking acts as clicking ok
 * button.
 * 
 * 6     5/26/97 10:30a Hoffoss
 * Added select wing to object select dialog.
 * 
 * 5     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

/////////////////////////////////////////////////////////////////////////////
// ship_select dialog

#include "ShipCheckListBox.h"
#include "object/object.h"
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
	int wing_index[MAX_WINGS + MAX_WAYPOINT_LISTS];
	int wing_sel_last[MAX_WINGS + MAX_WAYPOINT_LISTS];
};
