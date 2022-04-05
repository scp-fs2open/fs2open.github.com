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

BOOL saved_filter_ships = TRUE;
BOOL saved_filter_starts = TRUE;
BOOL saved_filter_waypoints = TRUE;

SCP_vector<bool> saved_filter_iff;
bool saved_filter_iff_inited = false;

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

	if (!saved_filter_iff_inited)
	{
		saved_filter_iff.clear();
		for (i = 0; i < (int)Iff_info.size(); i++)
			saved_filter_iff.push_back(true);

		saved_filter_iff_inited = true;
    }

	m_filter_ships = saved_filter_ships;
	m_filter_starts = saved_filter_starts;
	m_filter_waypoints = saved_filter_waypoints;

	activity = 0;

	obj_index.reserve(64);
	wing_and_waypoint_index.reserve(MAX_WINGS);
	wing_and_waypoint_sel_last.reserve(MAX_WINGS);
}

void ship_select::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ship_select)
	DDX_Control(pDX, IDC_IFF_LIST, m_iff_list);
	DDX_Control(pDX, IDC_SHIP_LIST, m_ship_list);
	DDX_Control(pDX, IDC_WING_LIST, m_wing_and_waypoint_list);
	DDX_Check(pDX, IDC_FILTER_SHIPS, m_filter_ships);
	DDX_Check(pDX, IDC_FILTER_STARTS, m_filter_starts);
	DDX_Check(pDX, IDC_FILTER_WAYPOINTS, m_filter_waypoints);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(ship_select, CDialog)
	//{{AFX_MSG_MAP(ship_select)
	ON_CBN_SELCHANGE(IDC_WING_DISPLAY_FILTER, OnSelchangeWingDisplayFilter)
	ON_BN_CLICKED(IDC_FILTER_SHIPS, OnFilterShips)
	ON_BN_CLICKED(IDC_FILTER_STARTS, OnFilterStarts)
	ON_BN_CLICKED(IDC_FILTER_WAYPOINTS, OnFilterWaypoints)
	ON_LBN_SELCHANGE(IDC_IFF_LIST, OnSelchangeIFFList)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_ALL, OnAll)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_LBN_DBLCLK(IDC_SHIP_LIST, OnDblclkShipList)
	ON_LBN_SELCHANGE(IDC_WING_LIST, OnSelchangeWingAndWaypointList)
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
	object *ptr;

	CDialog::OnInitDialog();

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		ptr->flags.set(Object::Object_Flags::Temp_marked, ptr->flags[Object::Object_Flags::Marked]);
		ptr = GET_NEXT(ptr);
	}

	// set up the IFF list first, so that the object list will be able to check it
	m_iff_list.ResetContent();
	for (int i = 0; i < (int)Iff_info.size(); i++)
	{
		m_iff_list.AddString(Iff_info[i].iff_name);
		m_iff_list.SetSel(i, saved_filter_iff[i] ? TRUE : FALSE);
	}
	GetDlgItem(IDC_IFF_LIST)->EnableWindow(m_filter_ships);

	// set up the object list
	create_list();

	num_wings = 0;
	wing_and_waypoint_index.clear();
	wing_and_waypoint_sel_last.clear();

	// Elements 0 - num_wings are wings, and elements num_wings - wing_and_waypoint_index.size are waypoint paths
	m_wing_and_waypoint_list.ResetContent();
	for (int i=0; i<MAX_WINGS; i++)
	{
		if (Wings[i].wave_count)
		{
			m_wing_and_waypoint_list.AddString(Wings[i].name);
			wing_and_waypoint_index.push_back(i);
			wing_and_waypoint_sel_last.push_back(false);
			num_wings++;
		}
	}

	int i = 0;
	for (auto &wp_list: Waypoint_lists)
	{
		m_wing_and_waypoint_list.AddString(wp_list.get_name());
		wing_and_waypoint_index.push_back(i);
		wing_and_waypoint_sel_last.push_back(false);
		i++;
	}

	return TRUE;
}

void ship_select::create_list()
{
	SCP_string text;
	object *ptr;

	update_status(true);
	m_ship_list.ResetContent();
	obj_index.clear();

	if (m_filter_starts)
	{
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list))
		{
			if (ptr->type == OBJ_START)
			{
				m_ship_list.AddString(Ships[ptr->instance].ship_name);
				obj_index.push_back(OBJ_INDEX(ptr));
				if (ptr->flags[Object::Object_Flags::Temp_marked])
					m_ship_list.SetSel((int)obj_index.size());
			}

			ptr = GET_NEXT(ptr);
		}
	}

	if (m_filter_ships)
	{
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list))
		{
			if (ptr->type == OBJ_SHIP)
			{
				if (m_iff_list.GetSel(Ships[ptr->instance].team) > 0)
				{
					m_ship_list.AddString(Ships[ptr->instance].ship_name);
					obj_index.push_back(OBJ_INDEX(ptr));
					if (ptr->flags[Object::Object_Flags::Temp_marked])
						m_ship_list.SetSel((int)obj_index.size());
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
				m_ship_list.AddString(text.c_str());
				obj_index.push_back(OBJ_INDEX(ptr));
				if (ptr->flags[Object::Object_Flags::Temp_marked])
					m_ship_list.SetSel((int)obj_index.size());
			}

			ptr = GET_NEXT(ptr);
		}
	}
}

void ship_select::OnOK()
{
	object *ptr;

	unmark_all();
	update_status();
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		if (ptr->flags[Object::Object_Flags::Temp_marked])
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

	saved_filter_ships = m_filter_ships;
	saved_filter_starts = m_filter_starts;
	saved_filter_waypoints = m_filter_waypoints;

	for (int i = 0; i < (int)Iff_info.size(); i++)
		saved_filter_iff[i] = m_iff_list.GetSel(i) > 0;

	CDialog::OnOK();
}

void ship_select::update_status(bool first_time)
{
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
        ptr->flags.remove(Object::Object_Flags::Temp_marked);
		ptr = GET_NEXT(ptr);
	}

	for (int i=0; i<m_ship_list.GetCount(); i++)
	{
		bool z = m_ship_list.GetSel(i) > 0;
    	Objects[obj_index[i]].flags.set(Object::Object_Flags::Temp_marked, z);
	}
	if(!first_time)
		OnSelchangeShipList();
}

void ship_select::OnFilterShips() 
{
	UpdateData(TRUE);
	create_list();

	GetDlgItem(IDC_IFF_LIST)->EnableWindow(m_filter_ships);
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

void ship_select::OnClear() 
{
	int i;
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list))
	{
		ptr->flags.remove(Object::Object_Flags::Temp_marked);
		ptr = GET_NEXT(ptr);
	}

	for (i=0; i<m_ship_list.GetCount(); i++)
		m_ship_list.SetSel(i, FALSE);

	for (i=0; i<m_wing_and_waypoint_list.GetCount(); i++) {
		wing_and_waypoint_sel_last[i] = false;
		m_wing_and_waypoint_list.SetSel(i, FALSE);
	}
}

void ship_select::OnAll() 
{
	int i;

	for (i=0; i<m_ship_list.GetCount(); i++)
	{
		Objects[obj_index[i]].flags.set(Object::Object_Flags::Temp_marked);
		m_ship_list.SetSel(i);
	}

	for (i=0; i<m_wing_and_waypoint_list.GetCount(); i++)
	{
		wing_and_waypoint_sel_last[i] = true;
		m_wing_and_waypoint_list.SetSel(i, TRUE);
	}
}

void ship_select::OnInvert() 
{
	for (int i=0; i<m_ship_list.GetCount(); i++)
	{
		bool selected = (m_ship_list.GetSel(i) > 0);

		Objects[obj_index[i]].flags.set(Object::Object_Flags::Temp_marked, !selected);
		m_ship_list.SetSel(i, selected ? FALSE : TRUE);
	}

	OnSelchangeShipList();
}

void ship_select::OnDblclkShipList() 
{
	OnOK();
}

int ship_select::find_index_with_objnum(int objnum)
{
	for (int k = 0; k < (int)obj_index.size(); k++)
	{
		if (obj_index[k] == objnum)
			return k;
	}
	return -1;
}

void ship_select::OnSelchangeWingAndWaypointList() 
{
	if (activity)
		return;

	activity = ACTIVITY_WING;

	for (int i = 0; i < m_wing_and_waypoint_list.GetCount(); i++)
	{
		bool z = m_wing_and_waypoint_list.GetSel(i) > 0;
		if (z != wing_and_waypoint_sel_last[i])
		{
			// we are dealing with a wing
			if (i < num_wings)
			{
				for (int j = 0; j < Wings[wing_and_waypoint_index[i]].wave_count; j++)
				{
					int k = find_index_with_objnum(wing_objects[wing_and_waypoint_index[i]][j]);
					if (k >= 0)
						m_ship_list.SetSel(k, z ? TRUE : FALSE);
				}
			}
			// we are dealing with a waypoint
			else
			{
				auto wp_list = find_waypoint_list_at_index(wing_and_waypoint_index[i]);
				Assert(wp_list != nullptr);

				for (auto& wp : wp_list->get_waypoints())
				{
					int k = find_index_with_objnum(wp.get_objnum());
					if (k >= 0)
						m_ship_list.SetSel(k, z ? TRUE : FALSE);
				}
			}

			wing_and_waypoint_sel_last[i] = z;
		}
	}

	activity = 0;
}

void ship_select::OnSelchangeShipList() 
{
	if (activity)
		return;

	activity = ACTIVITY_SHIP;

	for (int i = 0; i < m_wing_and_waypoint_list.GetCount(); i++)
	{
		int count = 0;
		int group_count = -1;

		// we are dealing with a wing
		if (i < num_wings)
		{
			group_count = Wings[wing_and_waypoint_index[i]].wave_count;
			for (int j = 0; j < group_count; j++)
			{
				int k = find_index_with_objnum(wing_objects[wing_and_waypoint_index[i]][j]);
				if (k >= 0 && m_ship_list.GetSel(k) > 0)
					count++;
			}
		}
		// we are dealing with a waypoint
		else
		{
			auto wp_list = find_waypoint_list_at_index(wing_and_waypoint_index[i]);
			Assert(wp_list != nullptr);

			group_count = (int)wp_list->get_waypoints().size();
			for (auto& wp : wp_list->get_waypoints())
			{
				int k = find_index_with_objnum(wp.get_objnum());
				if (k >= 0 && m_ship_list.GetSel(k) > 0)
					count++;
			}
		}

		wing_and_waypoint_sel_last[i] = (count == group_count);
		m_wing_and_waypoint_list.SetSel(i, wing_and_waypoint_sel_last[i] ? TRUE : FALSE);
	}

	activity = 0;
}

void ship_select::OnDblclkWingList() 
{
	OnOK();
}

void ship_select::OnSelchangeIFFList()
{
	UpdateData(true);
	create_list();

	// refresh whatever is selected
	// using auto&& so that I can handle the rvalue reference
	//     returned for the vector<bool> case
	// see https://stackoverflow.com/questions/13130708/what-is-the-advantage-of-using-forwarding-references-in-range-based-for-loops/13130795#13130795
	for (auto&& b : wing_and_waypoint_sel_last)
		b = false;
	OnSelchangeWingAndWaypointList();
}
