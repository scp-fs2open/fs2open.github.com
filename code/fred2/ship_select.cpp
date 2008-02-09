/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/ship_select.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Object selection (marking) dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/01/16 00:34:34  Goober5000
 * final (I hope) IFF fixage involving the FRED dialogs :p
 * --Goober5000
 *
 * Revision 1.4  2006/01/15 00:48:25  Goober5000
 * this should complete the new IFF stuff for FRED
 * --Goober5000
 *
 * Revision 1.3  2006/01/14 23:49:01  Goober5000
 * second pass; all the errors are fixed now; one more thing to take care of
 * --Goober5000
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
 * 21    2/13/98 11:48a Hoffoss
 * Fixed up some bugs with object selection when filters are turned off.
 * 
 * 20    9/16/97 9:41p Hoffoss
 * Changed Fred code around to stop using Parse_player structure for
 * player information, and use actual ships instead.
 * 
 * 19    9/09/97 10:29a Hoffoss
 * Added support for neutral team, and fixed changes made to how team is
 * used in ship structure.
 * 
 * 18    9/06/97 2:13p Mike
 * Replace support for TEAM_NEUTRAL
 * 
 * 17    8/26/97 11:08a Hoffoss
 * Added waypoint paths to object selection dialog.
 * 
 * 16    7/28/97 5:10p Hoffoss
 * Removed all occurances of neutral team from Fred.
 * 
 * 15    7/24/97 2:43p Hoffoss
 * Made changes whiteside requested.  Double clicking acts as clicking ok
 * button.
 * 
 * 14    7/24/97 10:24a Mike
 * Restore support for Unknown team
 * 
 * 13    7/10/97 11:24a Hoffoss
 * Fixed bugs in briefing editor system.  Make icon button not updating
 * when selecting ships via list select, and after making icon, it wasn't
 * being displayed, though it was selected.
 * 
 * 12    5/26/97 10:30a Hoffoss
 * Added select wing to object select dialog.
 * 
 * 11    5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 10    3/07/97 4:37p Mike
 * Make rockeye missile home.
 * Remove UNKNOWN and NEUTRAL teams.
 * 
 * 9     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "ship_select.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "ship/ship.h"
#include "Management.h"
#include "FREDView.h"

#define ACTIVITY_SHIP 1
#define ACTIVITY_WING 2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int filter_ships = TRUE;
int filter_starts = TRUE;
int filter_waypoints = TRUE;

int filter_iff[MAX_IFFS];
int filter_iff_inited = FALSE;

int IDC_FILTER_SHIPS_IFF[MAX_IFFS];

/////////////////////////////////////////////////////////////////////////////
// ship_select dialog

ship_select::ship_select(CWnd* pParent /*=NULL*/)
	: CDialog(ship_select::IDD, pParent)
{
	int i;

	//{{AFX_DATA_INIT(ship_select)
	m_filter_ships = TRUE;
	m_filter_starts = TRUE;
	m_filter_waypoints = TRUE;
	//}}AFX_DATA_INIT

	// this is stupid
	IDC_FILTER_SHIPS_IFF[0] = IDC_FILTER_SHIPS_IFF_0;
	IDC_FILTER_SHIPS_IFF[1] = IDC_FILTER_SHIPS_IFF_1;
	IDC_FILTER_SHIPS_IFF[2] = IDC_FILTER_SHIPS_IFF_2;
	IDC_FILTER_SHIPS_IFF[3] = IDC_FILTER_SHIPS_IFF_3;
	IDC_FILTER_SHIPS_IFF[4] = IDC_FILTER_SHIPS_IFF_4;
	IDC_FILTER_SHIPS_IFF[5] = IDC_FILTER_SHIPS_IFF_5;
	IDC_FILTER_SHIPS_IFF[6] = IDC_FILTER_SHIPS_IFF_6;
	IDC_FILTER_SHIPS_IFF[7] = IDC_FILTER_SHIPS_IFF_7;
	IDC_FILTER_SHIPS_IFF[8] = IDC_FILTER_SHIPS_IFF_8;
	IDC_FILTER_SHIPS_IFF[9] = IDC_FILTER_SHIPS_IFF_9;

	if (filter_iff_inited == FALSE)
	{
		for (i = 0; i < MAX_IFFS; i++)
			filter_iff[i] = TRUE;

		filter_iff_inited = TRUE;
	}

	m_filter_ships = filter_ships;
	m_filter_starts = filter_starts;
	m_filter_waypoints = filter_waypoints;

	for (i = 0; i < MAX_IFFS; i++)
		m_filter_iff[i] = filter_iff[i];

	activity = 0;
}

void ship_select::DoDataExchange(CDataExchange* pDX)
{
	int i;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ship_select)
	DDX_Control(pDX, IDC_WING_LIST, m_wing_list);
	DDX_Control(pDX, IDC_SHIP_LIST, m_ship_list);
	DDX_Check(pDX, IDC_FILTER_SHIPS, m_filter_ships);
	DDX_Check(pDX, IDC_FILTER_STARTS, m_filter_starts);
	DDX_Check(pDX, IDC_FILTER_WAYPOINTS, m_filter_waypoints);
	for (i = 0; i < MAX_IFFS; i++)
		DDX_Check(pDX, IDC_FILTER_SHIPS_IFF[i], m_filter_iff[i]);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(ship_select, CDialog)
	//{{AFX_MSG_MAP(ship_select)
	ON_CBN_SELCHANGE(IDC_WING_DISPLAY_FILTER, OnSelchangeWingDisplayFilter)
	ON_BN_CLICKED(IDC_FILTER_SHIPS, OnFilterShips)
	ON_BN_CLICKED(IDC_FILTER_STARTS, OnFilterStarts)
	ON_BN_CLICKED(IDC_FILTER_WAYPOINTS, OnFilterWaypoints)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_0, OnFilterShipsIFF0)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_1, OnFilterShipsIFF1)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_2, OnFilterShipsIFF2)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_3, OnFilterShipsIFF3)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_4, OnFilterShipsIFF4)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_5, OnFilterShipsIFF5)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_6, OnFilterShipsIFF6)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_7, OnFilterShipsIFF7)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_8, OnFilterShipsIFF8)
	ON_BN_CLICKED(IDC_FILTER_SHIPS_IFF_9, OnFilterShipsIFF9)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_ALL, OnAll)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_LBN_DBLCLK(IDC_SHIP_LIST, OnDblclkShipList)
	ON_LBN_SELCHANGE(IDC_WING_LIST, OnSelchangeWingList)
	ON_LBN_SELCHANGE(IDC_SHIP_LIST, OnSelchangeShipList)
	ON_LBN_DBLCLK(IDC_WING_LIST, OnDblclkWingList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ship_select message handlers

void ship_select::OnSelchangeWingDisplayFilter() 
{
	UpdateData(TRUE);
	create_list();
}

BOOL ship_select::OnInitDialog() 
{
	int i, flags;
	object *ptr;

	wlist_size = wplist_size = 0;
	CDialog::OnInitDialog();

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		flags = ptr->flags & ~OF_TEMP_MARKED;
		if (flags & OF_MARKED)
			flags |= OF_TEMP_MARKED;
		else
			flags &= ~OF_TEMP_MARKED;

		ptr->flags = flags;
		ptr = GET_NEXT(ptr);
	}

	list_size = 0;
	create_list();

	// init dialog stuff
	for (i = 0; i < MAX_IFFS; i++)
	{
		if (i < Num_iffs)
		{
			GetDlgItem(IDC_FILTER_SHIPS_IFF[i])->SetWindowText(Iff_info[i].iff_name);
		}

		GetDlgItem(IDC_FILTER_SHIPS_IFF[i])->ShowWindow(i < Num_iffs);
	}

	for (i = 0; i < Num_iffs; i++)
		GetDlgItem(IDC_FILTER_SHIPS_IFF[i])->EnableWindow(m_filter_ships);

	// Elements 0 - wlist_size are wings, and elements wlist_size - wplist_size are waypoint paths
	m_wing_list.ResetContent();
	wlist_size = 0;
	for (i=0; i<MAX_WINGS; i++)
		if (Wings[i].wave_count) {
			m_wing_list.AddString(Wings[i].name);
			wing_sel_last[wlist_size] = 0;
			wing_index[wlist_size++] = i;
		}

	wplist_size = wlist_size;
	for (i=0; i<Num_waypoint_lists; i++) {
		m_wing_list.AddString(Waypoint_lists[i].name);
		wing_sel_last[wplist_size] = 0;
		wing_index[wplist_size++] = i;
	}

	return TRUE;
}

void ship_select::create_list()
{
	char text[512];
	object *ptr;

	update_status();
	m_ship_list.ResetContent();
	list_size = 0;

	if (m_filter_starts)
	{
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list))
		{
			if (ptr->type == OBJ_START)
			{
				m_ship_list.AddString(Ships[ptr->instance].ship_name);
				obj_index[list_size++] = ptr;
				if (ptr->flags & OF_TEMP_MARKED)
					m_ship_list.SetSel(list_size - 1);
			}

			ptr = GET_NEXT(ptr);
		}
	}

	if (m_filter_ships) {
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list))
		{
			if (ptr->type == OBJ_SHIP)
			{
				if (m_filter_iff[Ships[ptr->instance].team])
				{
					m_ship_list.AddString(Ships[ptr->instance].ship_name);
					obj_index[list_size++] = ptr;
					if (ptr->flags & OF_TEMP_MARKED)
						m_ship_list.SetSel(list_size - 1);
				}
			}

			ptr = GET_NEXT(ptr);
		}
	}

	if (m_filter_waypoints)
	{
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list))
		{
			if (ptr->type == OBJ_WAYPOINT)
			{
				sprintf(text, "%s:%d", Waypoint_lists[ptr->instance / 65536].name,
					(ptr->instance & 0xffff) + 1);
				m_ship_list.AddString(text);
				obj_index[list_size++] = ptr;
				if (ptr->flags & OF_TEMP_MARKED)
					m_ship_list.SetSel(list_size - 1);
			}

			ptr = GET_NEXT(ptr);
		}
	}
}

void ship_select::OnOK()
{
	int i;
	object *ptr;

	unmark_all();
	update_status();
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		if (ptr->flags & OF_TEMP_MARKED)
			mark_object(OBJ_INDEX(ptr));

		ptr = GET_NEXT(ptr);
	}

	if (query_valid_object() && (Marked == 1) && (Objects[cur_object_index].type == OBJ_POINT)) {
		Assert(Briefing_dialog);
		Briefing_dialog->icon_select(Objects[cur_object_index].instance);

	} else {
		if (Briefing_dialog)
			Briefing_dialog->icon_select(-1);
	}

	filter_ships = m_filter_ships;
	filter_starts = m_filter_starts;
	filter_waypoints = m_filter_waypoints;

	for (i = 0; i < MAX_IFFS; i++)
		filter_iff[i] = m_filter_iff[i];

	CDialog::OnOK();
}

void ship_select::update_status()
{
	int i, z;
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ptr->flags &= ~OF_TEMP_MARKED;
		ptr = GET_NEXT(ptr);
	}

	for (i=0; i<list_size; i++)
	{
		z = m_ship_list.GetSel(i);
		if (z < 1)
			obj_index[i]->flags &= ~OF_TEMP_MARKED;
		else
			obj_index[i]->flags |= OF_TEMP_MARKED;
	}

	OnSelchangeShipList();
}

void ship_select::OnFilterShips() 
{
	int i;

	UpdateData(TRUE);
	create_list();

	for (i = 0; i < Num_iffs; i++)
		GetDlgItem(IDC_FILTER_SHIPS_IFF[i])->EnableWindow(m_filter_ships);
}

void ship_select::OnFilterStarts() 
{
	UpdateData(TRUE);
	create_list();
}

void ship_select::OnFilterWaypoints() 
{
	UpdateData(TRUE);
	create_list();
}

void ship_select::OnFilterShipsIFF(int iff)
{
	UpdateData(true);
	create_list();
}

void ship_select::OnClear() 
{
	int i;
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		ptr->flags &= ~OF_TEMP_MARKED;
		ptr = GET_NEXT(ptr);
	}

	for (i=0; i<list_size; i++)
		m_ship_list.SetSel(i, FALSE);

	for (i=0; i<wplist_size; i++) {
		wing_sel_last[i] = 0;
		m_wing_list.SetSel(i, FALSE);
	}
}

void ship_select::OnAll() 
{
	int i;

	for (i=0; i<list_size; i++)
	{
		obj_index[i]->flags |= OF_TEMP_MARKED;
		m_ship_list.SetSel(i);
	}

	for (i=0; i<wplist_size; i++) {
		wing_sel_last[i] = 1;
		m_wing_list.SetSel(i, TRUE);
	}
}

void ship_select::OnInvert() 
{
	int i, z;

	for (i=0; i<list_size; i++)
	{
		z = m_ship_list.GetSel(i);
		if (z > 0)
		{
			obj_index[i]->flags &= ~OF_TEMP_MARKED;
			m_ship_list.SetSel(i, FALSE);

		} else {
			obj_index[i]->flags |= OF_TEMP_MARKED;
			m_ship_list.SetSel(i);
		}
	}

	OnSelchangeShipList();
}

void ship_select::OnDblclkShipList() 
{
	OnOK();

/*	int i, j, z, wing;

	z = m_ship_list.GetCaretIndex();
	switch (obj_index[z]->type) {
		case OBJ_SHIP:
			wing = Ships[obj_index[z]->instance].wingnum;
			if (wing >= 0) {
				for (i=0; i<Wings[wing].wave_count; i++)
					for (j=0; j<list_size; j++)
						if (OBJ_INDEX(obj_index[j]) == wing_objects[wing][i]) {
							m_ship_list.SetSel(j);
							break;
						}

				for (i=0; i<wlist_size; i++)
					if (wing_index[i] == wing) {
						m_wing_list.SetSel(i);
						wing_sel_last[i] = 1;
						break;
					}
			}

			break;

		case OBJ_WAYPOINT:
			break;
	}*/
}

void ship_select::OnSelchangeWingList() 
{
	int i, j, k, z;

	if (activity)
		return;

	activity = ACTIVITY_WING;
	for (i=0; i<wlist_size; i++) {
		z = (m_wing_list.GetSel(i) > 0) ? 1 : 0;
		if (z != wing_sel_last[i]) {
			for (j=0; j<Wings[wing_index[i]].wave_count; j++)
				for (k=0; k<list_size; k++)
					if (OBJ_INDEX(obj_index[k]) == wing_objects[wing_index[i]][j]) {
						m_ship_list.SetSel(k, z ? TRUE : FALSE);
						break;
					}

			wing_sel_last[i] = z;
		}
	}

	for (i=wlist_size; i<wplist_size; i++) {
		z = (m_wing_list.GetSel(i) > 0) ? 1 : 0;
		if (z != wing_sel_last[i]) {
			for (j=0; j<Waypoint_lists[wing_index[i]].count; j++)
				for (k=0; k<list_size; k++)
					if ((obj_index[k]->type == OBJ_WAYPOINT) && (obj_index[k]->instance == wing_index[i] * 65536 + j)) {
						m_ship_list.SetSel(k, z ? TRUE : FALSE);
						break;
					}

			wing_sel_last[i] = z;
		}
	}

	activity = 0;
}

void ship_select::OnSelchangeShipList() 
{
	int i, j, k, count;

	if (activity)
		return;

	activity = ACTIVITY_SHIP;
	for (i=0; i<wlist_size; i++) {
		count = 0;
		for (j=0; j<Wings[wing_index[i]].wave_count; j++)
			for (k=0; k<list_size; k++)
				if (OBJ_INDEX(obj_index[k]) == wing_objects[wing_index[i]][j]) {
					if (m_ship_list.GetSel(k))
						count++;

					break;
				}

		if (count == Wings[wing_index[i]].wave_count)
			wing_sel_last[i] = 1;
		else
			wing_sel_last[i] = 0;

		m_wing_list.SetSel(i, wing_sel_last[i] ? TRUE : FALSE);
	}

	for (i=wlist_size; i<wplist_size; i++) {
		count = 0;
		for (j=0; j<Waypoint_lists[wing_index[i]].count; j++)
			for (k=0; k<list_size; k++)
				if ((obj_index[k]->type == OBJ_WAYPOINT) && (obj_index[k]->instance == wing_index[i] * 65536 + j)) {
					if (m_ship_list.GetSel(k))
						count++;

					break;
				}

		if (count == Waypoint_lists[wing_index[i]].count)
			wing_sel_last[i] = 1;
		else
			wing_sel_last[i] = 0;

		m_wing_list.SetSel(i, wing_sel_last[i] ? TRUE : FALSE);
	}

	activity = 0;
}

void ship_select::OnDblclkWingList() 
{
	OnOK();
}

void ship_select::OnFilterShipsIFF0() 
{
	OnFilterShipsIFF(0);
}

void ship_select::OnFilterShipsIFF1() 
{
	OnFilterShipsIFF(1);
}

void ship_select::OnFilterShipsIFF2() 
{
	OnFilterShipsIFF(2);
}

void ship_select::OnFilterShipsIFF3() 
{
	OnFilterShipsIFF(3);
}

void ship_select::OnFilterShipsIFF4() 
{
	OnFilterShipsIFF(4);
}

void ship_select::OnFilterShipsIFF5() 
{
	OnFilterShipsIFF(5);
}

void ship_select::OnFilterShipsIFF6() 
{
	OnFilterShipsIFF(6);
}

void ship_select::OnFilterShipsIFF7() 
{
	OnFilterShipsIFF(7);
}

void ship_select::OnFilterShipsIFF8() 
{
	OnFilterShipsIFF(8);
}

void ship_select::OnFilterShipsIFF9() 
{
	OnFilterShipsIFF(9);
}

