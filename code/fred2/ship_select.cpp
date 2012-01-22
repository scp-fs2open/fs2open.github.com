/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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

	wing_index.reserve(MAX_WINGS);
	wing_sel_last.reserve(MAX_WINGS);
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

	wlist_size = wplist_size = 0;
	wing_index.clear();
	wing_sel_last.clear();

	// Elements 0 - wlist_size are wings, and elements wlist_size - wplist_size are waypoint paths
	m_wing_list.ResetContent();
	wlist_size = 0;
	for (i=0; i<MAX_WINGS; i++) {
		if (Wings[i].wave_count) {
			m_wing_list.AddString(Wings[i].name);
			wing_sel_last.push_back(0);
			wing_index.push_back(i);
			wlist_size++;
		}
	}

	wplist_size = wlist_size;

	SCP_list<waypoint_list>::iterator ii;
	for (i = 0, ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++i, ++ii) {
		m_wing_list.AddString(ii->get_name());
		wing_sel_last.push_back(0);
		wing_index.push_back(i);
		wplist_size++;
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
				int waypoint_num;
				waypoint_list *wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
				Assert(wp_list != NULL);
				sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);
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
			waypoint_list *wp_list = find_waypoint_list_at_index(wing_index[i]);
			Assert(wp_list != NULL);
			SCP_list<waypoint>::iterator jj;
			for (j = 0, jj = wp_list->get_waypoints().begin(); jj != wp_list->get_waypoints().end(); ++j, ++jj) {
				for (k=0; k<list_size; k++) {
					if ((obj_index[k]->type == OBJ_WAYPOINT) && (obj_index[k]->instance == calc_waypoint_instance(wing_index[i], j))) {
						m_ship_list.SetSel(k, z ? TRUE : FALSE);
						break;
					}
				}
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
		waypoint_list *wp_list = find_waypoint_list_at_index(wing_index[i]);
		Assert(wp_list != NULL);
		SCP_list<waypoint>::iterator jj;

		count = 0;
		for (j = 0, jj = wp_list->get_waypoints().begin(); jj != wp_list->get_waypoints().end(); ++j, ++jj) {
			for (k=0; k<list_size; k++) {
				if ((obj_index[k]->type == OBJ_WAYPOINT) && (obj_index[k]->instance == calc_waypoint_instance(wing_index[i], j))) {
					if (m_ship_list.GetSel(k))
						count++;

					break;
				}
			}
		}

		if ((uint) count == wp_list->get_waypoints().size())
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

