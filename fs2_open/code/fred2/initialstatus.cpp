/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// InitialStatus.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "InitialStatus.h"
#include "Management.h"
#include "globalincs/linklist.h"
#include "object/objectdock.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// non-class function prototypes, bah
void initial_status_mark_dock_leader_helper(object *objp, dock_function_info *infop);
void initial_status_unmark_dock_handled_flag(object *objp, dock_function_info *infop);
void reset_arrival_to_false(int shipnum, bool reset_wing);


/////////////////////////////////////////////////////////////////////////////
// initial_status dialog

initial_status::initial_status(CWnd* pParent /*=NULL*/)
	: CDialog(initial_status::IDD, pParent)
{
	//{{AFX_DATA_INIT(initial_status)
	m_damage = 0;
	m_shields = 0;
	m_velocity = 0;
	m_hull = 0;
	m_has_shields = FALSE;
	m_force_shields = FALSE;
	m_locked = FALSE;
	m_primaries_locked = FALSE;
	m_secondaries_locked = FALSE;
	m_turrets_locked = FALSE;
	m_afterburner_locked = FALSE;
	m_cargo_name = _T("");
	//}}AFX_DATA_INIT
	inited = 0;
	cur_subsys = LB_ERR;

	dockpoint_array = NULL;
	num_dock_points = 0;

	cur_docker_point = -1;
	cur_dockee = -1;
	cur_dockee_point = -1;

	m_multi_edit = 0;
}

initial_status::~initial_status()
{
	if (dockpoint_array != NULL)
		delete [] dockpoint_array;
}

void initial_status::DoDataExchange(CDataExchange* pDX)
{
	CString str;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(initial_status)
	DDX_Control(pDX, IDC_HULL_SPIN, m_hull_spin);
	DDX_Control(pDX, IDC_VELOCITY_SPIN, m_velocity_spin);
	DDX_Control(pDX, IDC_SHIELDS_SPIN, m_shields_spin);
	DDX_Control(pDX, IDC_DAMAGE_SPIN, m_damage_spin);
	DDX_Text(pDX, IDC_DAMAGE, m_damage);
	DDV_MinMaxInt(pDX, m_damage, 0, 100);
	DDX_Check(pDX, IDC_HAS_SHIELDS, m_has_shields);
	DDX_Check(pDX, IDC_FORCE_SHIELDS, m_force_shields);
	DDX_Check(pDX, IDC_LOCKED, m_locked);
	DDX_Check(pDX, IDC_PRIMARIES_LOCKED, m_primaries_locked);
	DDX_Check(pDX, IDC_SECONDARIES_LOCKED, m_secondaries_locked);
	DDX_Check(pDX, IDC_TURRETS_LOCKED, m_turrets_locked);
	DDX_Check(pDX, IDC_AFTERBURNER_LOCKED, m_afterburner_locked);
	DDX_Text(pDX, IDC_CARGO_NAME, m_cargo_name);
	DDV_MaxChars(pDX, m_cargo_name, 20);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		GetDlgItem(IDC_VELOCITY)->GetWindowText(str);
		m_velocity = atoi(str);
		if (m_velocity < 0)
			m_velocity = 0;
		if (m_velocity > 100)
			m_velocity = 100;

		GetDlgItem(IDC_SHIELDS)->GetWindowText(str);
		m_shields = atoi(str);
		if (m_shields < 0)
			m_shields = 0;
		if (m_shields > 100)
			m_shields = 100;

		GetDlgItem(IDC_HULL)->GetWindowText(str);
		m_hull = atoi(str);
		if (m_hull < 0)
			m_hull = 0;
		if (m_hull > 100)
			m_hull = 100;
	}
}

BEGIN_MESSAGE_MAP(initial_status, CDialog)
	//{{AFX_MSG_MAP(initial_status)
	ON_LBN_SELCHANGE(IDC_SUBSYS, OnSelchangeSubsys)
	ON_LBN_SELCHANGE(IDC_DOCKER_POINT, OnSelchangeDockerPoint)
	ON_CBN_SELCHANGE(IDC_DOCKEE, OnSelchangeDockee)
	ON_CBN_SELCHANGE(IDC_DOCKEE_POINT, OnSelchangeDockeePoint)
	ON_BN_CLICKED(IDC_HAS_SHIELDS, OnHasShields)
	ON_BN_CLICKED(IDC_FORCE_SHIELDS, OnForceShields)
	ON_BN_CLICKED(IDC_LOCKED, OnLocked)
	ON_BN_CLICKED(IDC_PRIMARIES_LOCKED, OnPrimariesLocked)
	ON_BN_CLICKED(IDC_SECONDARIES_LOCKED, OnSecondariesLocked)
	ON_BN_CLICKED(IDC_TURRETS_LOCKED, OnTurretsLocked)
	ON_BN_CLICKED(IDC_AFTERBURNER_LOCKED, OnAfterburnersLocked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// initial_status message handlers

BOOL initial_status::OnInitDialog() 
{
	int z, vflag, sflag, hflag;
	ship_subsys *ptr;
	CString str;
	object *objp;

	m_ship = cur_ship;
	if (m_ship == -1) {
		Assert((Objects[cur_object_index].type == OBJ_SHIP) || (Objects[cur_object_index].type == OBJ_START));
		m_ship = get_ship_from_obj(cur_object_index);
		Assert(m_ship >= 0);
	}

	// initialize dockpoint stuff
	if (!m_multi_edit)
	{
		num_dock_points = model_get_num_dock_points(Ship_info[Ships[m_ship].ship_info_index].model_num);
		dockpoint_array = new dockpoint_information[num_dock_points];
		objp = &Objects[Ships[m_ship].objnum];
		for (int i = 0; i < num_dock_points; i++)
		{
			object *docked_objp = dock_find_object_at_dockpoint(objp, i);
			if (docked_objp != NULL)
			{
				dockpoint_array[i].dockee_shipnum = docked_objp->instance;
				dockpoint_array[i].dockee_point = dock_find_dockpoint_used_by_object(docked_objp, objp);
			}
			else
			{
				dockpoint_array[i].dockee_shipnum = -1;
				dockpoint_array[i].dockee_point = -1;
			}
		}
	}

	vflag = sflag = hflag = 0;
	m_velocity = (int) Objects[cur_object_index].phys_info.speed;
	m_shields = (int) Objects[cur_object_index].shield_quadrant[0];
	m_hull = (int) Objects[cur_object_index].hull_strength;
	if (Objects[cur_object_index].flags & OF_NO_SHIELDS)
		m_has_shields = 0;
	else
		m_has_shields = 1;
	
	if (Ships[m_ship].flags2 & SF2_FORCE_SHIELDS_ON)
		m_force_shields = 1;
	else
		m_force_shields = 0;

	if (Ships[m_ship].flags & SF_LOCKED)
		m_locked = 1;
	else
		m_locked = 0;

	// Lock primaries
	if (Ships[m_ship].flags2 & SF2_PRIMARIES_LOCKED) {
		m_primaries_locked = 1;
	}
	else {
		m_primaries_locked = 0;
	}

	//Lock secondaries
	if (Ships[m_ship].flags2 & SF2_SECONDARIES_LOCKED) {
		m_secondaries_locked = 1;
	}
	else {
		m_secondaries_locked = 0;
	}

	//Lock turrets
	if (Ships[m_ship].flags2 & SF2_LOCK_ALL_TURRETS_INITIALLY) {
		m_turrets_locked = 1;
	}
	else {
		m_turrets_locked = 0;
	}

	if (Ships[m_ship].flags2 & SF2_AFTERBURNER_LOCKED) {
		m_afterburner_locked = 1;
	}
	else {
		m_afterburner_locked = 0;
	}



	if (m_multi_edit) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags & OF_MARKED)) {
				if (objp->phys_info.speed != m_velocity)
					vflag = 1;
				if ((int) objp->shield_quadrant[0] != m_shields)
					sflag = 1;
				if ((int) objp->hull_strength != m_hull)
					hflag = 1;
				if (objp->flags & OF_NO_SHIELDS) {
					if (m_has_shields)
						m_has_shields = 2;

				} else {
					if (!m_has_shields)
						m_has_shields = 2;
				}

				Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));
				
				if (Ships[get_ship_from_obj(objp)].flags2 & SF2_FORCE_SHIELDS_ON) {
					if (!m_force_shields)
						m_force_shields = 2;

				} else {
					if (m_force_shields)
						m_force_shields = 2;
				}
				if (Ships[get_ship_from_obj(objp)].flags & SF_LOCKED) {
					if (!m_locked)
						m_locked = 2;

				} else {
					if (m_locked)
						m_locked = 2;
				}

				if (Ships[get_ship_from_obj(objp)].flags2 & SF2_PRIMARIES_LOCKED){
					if (!m_primaries_locked)
						m_primaries_locked = 2;
				}
				else {
					if (m_primaries_locked)
						m_primaries_locked = 2;
				}
				
				if (Ships[get_ship_from_obj(objp)].flags2 & SF2_SECONDARIES_LOCKED){
					if (!m_secondaries_locked)
						m_secondaries_locked = 2;
				}
				else {
					if (m_secondaries_locked)
						m_secondaries_locked = 2;
				}
								
				if (Ships[get_ship_from_obj(objp)].flags2 & SF2_LOCK_ALL_TURRETS_INITIALLY){
					if (!m_turrets_locked)
						m_turrets_locked = 2;
				}
				else {
					if (m_turrets_locked)
						m_turrets_locked = 2;
				}
				
				if (Ships[get_ship_from_obj(objp)].flags2 & SF2_AFTERBURNER_LOCKED){
					if (!m_afterburner_locked)
						m_afterburner_locked = 2;
				}
				else {
					if (m_afterburner_locked)
						m_afterburner_locked = 2;
				}
			}

			objp = GET_NEXT(objp);
		}
	}

	CDialog::OnInitDialog();
	str.Format("%d", m_velocity);
	GetDlgItem(IDC_VELOCITY)->SetWindowText(str);
	str.Format("%d", m_shields);
	GetDlgItem(IDC_SHIELDS)->SetWindowText(str);
	str.Format("%d", m_hull);
	GetDlgItem(IDC_HULL)->SetWindowText(str);

	inited = 1;

	GetDlgItem(IDC_SHIELDS)->EnableWindow(m_has_shields ? TRUE : FALSE);
	GetDlgItem(IDC_SHIELDS_SPIN)->EnableWindow(m_has_shields ? TRUE : FALSE);

	m_velocity_spin.SetRange(0, 100);
	m_hull_spin.SetRange(0, 100);
	m_shields_spin.SetRange(0, 100);
	m_damage_spin.SetRange(0, 100);

	// init boxes
	lstDockerPoints = (CListBox *) GetDlgItem(IDC_DOCKER_POINT);
	cboDockees = (CComboBox *) GetDlgItem(IDC_DOCKEE);
	cboDockeePoints = (CComboBox *) GetDlgItem(IDC_DOCKEE_POINT);
	lstDockerPoints->ResetContent();
	cboDockees->ResetContent();
	cboDockeePoints->ResetContent();

	if (!m_multi_edit)
	{
		for (int dockpoint = 0; dockpoint < num_dock_points; dockpoint++)
		{
			z = lstDockerPoints->AddString(model_get_dock_name(Ship_info[Ships[m_ship].ship_info_index].model_num, dockpoint));
			lstDockerPoints->SetItemData(z, dockpoint);
		}

		for (ptr = GET_FIRST(&Ships[m_ship].subsys_list); ptr != END_OF_LIST(&Ships[m_ship].subsys_list); ptr = GET_NEXT(ptr))
		{
			((CListBox *) GetDlgItem(IDC_SUBSYS))->AddString(ptr->system_info->subobj_name);
		}
	}
	else
	{
		GetDlgItem(IDC_SUBSYS)->EnableWindow(FALSE);
		GetDlgItem(IDC_DAMAGE)->EnableWindow(FALSE);
	}
	change_docker_point(false);
	change_subsys();

	UpdateData(FALSE);

	if (vflag)
		GetDlgItem(IDC_VELOCITY)->SetWindowText("");
	if (sflag)
		GetDlgItem(IDC_SHIELDS)->SetWindowText("");
	if (hflag)
		GetDlgItem(IDC_HULL)->SetWindowText("");

	return TRUE;
}

void initial_status::OnOK()
{
	char buf[256];
	int vflag = 0, sflag = 0, hflag = 0;
	object *objp;

	if (GetDlgItem(IDC_VELOCITY)->GetWindowText(buf, 255))
		vflag = 1;
	if (GetDlgItem(IDC_SHIELDS)->GetWindowText(buf, 255))
		sflag = 1;
	if (GetDlgItem(IDC_HULL)->GetWindowText(buf, 255))
		hflag = 1;

	UpdateData(TRUE);

	change_subsys();

	if (m_multi_edit) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags & OF_MARKED)) {
				if (vflag)
					MODIFY(objp->phys_info.speed, (float) m_velocity);

				if (sflag)
					MODIFY(objp->shield_quadrant[0], (float) m_shields);

				if (hflag)
					MODIFY(objp->hull_strength, (float) m_hull);

				if (m_has_shields == 1)
					objp->flags &= ~OF_NO_SHIELDS;
				else if (!m_has_shields)
					objp->flags |= OF_NO_SHIELDS;
				
				if (m_force_shields == 1) {
					Ships[get_ship_from_obj(objp)].flags2 |= SF2_FORCE_SHIELDS_ON;
				}
				else if (!m_force_shields) {
					Ships[get_ship_from_obj(objp)].flags2 &= ~SF2_FORCE_SHIELDS_ON;
				}
				
				if (m_locked == 1)
					Ships[get_ship_from_obj(objp)].flags |= SF_LOCKED;
				else if (!m_locked)
					Ships[get_ship_from_obj(objp)].flags &= ~SF_LOCKED;

				if (m_primaries_locked == 1) {
					Ships[get_ship_from_obj(objp)].flags2 |= SF2_PRIMARIES_LOCKED;
				}
				else if (!m_primaries_locked) {
					Ships[get_ship_from_obj(objp)].flags2 &= ~SF2_PRIMARIES_LOCKED;
				}

				if (m_secondaries_locked == 1) {
					Ships[get_ship_from_obj(objp)].flags2 |= SF2_SECONDARIES_LOCKED;
				}
				else if (!m_secondaries_locked)	{
					Ships[get_ship_from_obj(objp)].flags2 &= ~SF2_SECONDARIES_LOCKED;
				}

				if (m_turrets_locked == 1) {
					Ships[get_ship_from_obj(objp)].flags2 |= SF2_LOCK_ALL_TURRETS_INITIALLY;
				}
				else if (!m_turrets_locked) {
					Ships[get_ship_from_obj(objp)].flags2 &= ~SF2_LOCK_ALL_TURRETS_INITIALLY;
				}
				
				if (m_afterburner_locked == 1) {
					Ships[get_ship_from_obj(objp)].flags2 |= SF2_AFTERBURNER_LOCKED;
				}
				else if (!m_afterburner_locked)	{
					Ships[get_ship_from_obj(objp)].flags2 &= ~SF2_AFTERBURNER_LOCKED;
				}
			}

			objp = GET_NEXT(objp);
		}

	} else {
		MODIFY(Objects[cur_object_index].phys_info.speed, (float) m_velocity);
		MODIFY(Objects[cur_object_index].shield_quadrant[0], (float) m_shields);
		MODIFY(Objects[cur_object_index].hull_strength, (float) m_hull);
		if (m_has_shields)
			Objects[cur_object_index].flags &= ~OF_NO_SHIELDS;
		else
			Objects[cur_object_index].flags |= OF_NO_SHIELDS;

		if (m_force_shields == 1)
			Ships[m_ship].flags2 |= SF2_FORCE_SHIELDS_ON;
		else if (!m_force_shields)
			Ships[m_ship].flags2 &= ~SF2_FORCE_SHIELDS_ON;		

		if (m_locked == 1)
			Ships[m_ship].flags |= SF_LOCKED;
		else if (!m_locked)
			Ships[m_ship].flags &= ~SF_LOCKED;		

		if (m_primaries_locked == 1)
			Ships[m_ship].flags2 |= SF2_PRIMARIES_LOCKED;
		else if (!m_primaries_locked)
			Ships[m_ship].flags2 &= ~SF2_PRIMARIES_LOCKED;		

		if (m_secondaries_locked == 1)
			Ships[m_ship].flags2 |= SF2_SECONDARIES_LOCKED;
		else if (!m_secondaries_locked)
			Ships[m_ship].flags2 &= ~SF2_SECONDARIES_LOCKED;		

		if (m_turrets_locked == 1)
			Ships[m_ship].flags2 |= SF2_LOCK_ALL_TURRETS_INITIALLY;
		else if (!m_turrets_locked)
			Ships[m_ship].flags2 &= ~SF2_LOCK_ALL_TURRETS_INITIALLY;		

		if (m_afterburner_locked == 1)
			Ships[m_ship].flags2 |= SF2_AFTERBURNER_LOCKED;
		else if (!m_afterburner_locked)
			Ships[m_ship].flags2 &= ~SF2_AFTERBURNER_LOCKED;
	}


	update_docking_info();

	CDialog::OnOK();
}

void initial_status::OnHasShields() 
{
	if (m_has_shields == 1) {
		m_has_shields = 0;

		// can't force shields and also have them off
		if (m_force_shields) {
			m_force_shields = 0; 
			((CButton *) GetDlgItem(IDC_FORCE_SHIELDS))->SetCheck(m_force_shields);

			// warn on multiple ships of different state
			if (m_multi_edit) {
				MessageBox("At least one selected ship was set to Force Shields On. This is now turned off for all selected ships", "Resetting Flag", MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}
	else
		m_has_shields = 1;

	((CButton *) GetDlgItem(IDC_HAS_SHIELDS))->SetCheck(m_has_shields);
	GetDlgItem(IDC_SHIELDS)->EnableWindow(m_has_shields);
	GetDlgItem(IDC_SHIELDS_SPIN)->EnableWindow(m_has_shields);
}

void initial_status::OnForceShields() 
{
	if (m_force_shields == 1)
		m_force_shields = 0;
	else {
		m_force_shields = 1;

		// can't force shields and also turn have them off
		if (m_has_shields != 1) {
			m_has_shields = 1; 
			((CButton *) GetDlgItem(IDC_HAS_SHIELDS))->SetCheck(m_has_shields);
			GetDlgItem(IDC_SHIELDS)->EnableWindow(m_has_shields);
			GetDlgItem(IDC_SHIELDS_SPIN)->EnableWindow(m_has_shields);

			// warn on multiple ships of different state
			if (m_multi_edit) {
				MessageBox("At least one selected ship was set to have no shields. Shields are now enabled for all selected ships", "Resetting Flag", MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}
	
	((CButton *) GetDlgItem(IDC_FORCE_SHIELDS))->SetCheck(m_force_shields);
}

void initial_status::OnLocked() 
{
	if (m_locked == 1)
		m_locked = 0;
	else
		m_locked = 1;

	((CButton *) GetDlgItem(IDC_LOCKED))->SetCheck(m_locked);
}

void initial_status::OnSelchangeSubsys() 
{
	UpdateData(TRUE);
	change_subsys();
	UpdateData(FALSE);
}

void initial_status::change_subsys()
{
	int z, cargo_index, enable = FALSE, enable_cargo_name = FALSE;
	int ship_has_scannable_subsystems;
	ship_subsys *ptr;

	// Goober5000
	ship_has_scannable_subsystems = (Ship_info[Ships[m_ship].ship_info_index].flags & SIF_HUGE_SHIP);
	if (Ships[m_ship].flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING)
		ship_has_scannable_subsystems = !ship_has_scannable_subsystems;

	if (cur_subsys != LB_ERR) {
		ptr = GET_FIRST(&Ships[m_ship].subsys_list);
		while (cur_subsys--) {
			Assert(ptr != END_OF_LIST(&Ships[m_ship].subsys_list));
			ptr = GET_NEXT(ptr);
		}

		MODIFY(ptr -> current_hits, 100.0f - (float) m_damage);

		// update cargo name
		if (strlen(m_cargo_name) > 0) {
			cargo_index = string_lookup(m_cargo_name, Cargo_names, Num_cargo);
			if (cargo_index == -1) {
				if (Num_cargo < MAX_CARGO);
				cargo_index = Num_cargo++;
				strcpy(Cargo_names[cargo_index], m_cargo_name);
				ptr->subsys_cargo_name = cargo_index;
			} else {
				ptr->subsys_cargo_name = cargo_index;
			}
		} else {
			ptr->subsys_cargo_name = 0;
		}
		set_modified();
	}

	cur_subsys = z = ((CListBox *) GetDlgItem(IDC_SUBSYS)) -> GetCurSel();
	if (z == LB_ERR) {
		m_damage = 100;

	} else {
		ptr = GET_FIRST(&Ships[m_ship].subsys_list);
		while (z--) {
			Assert(ptr != END_OF_LIST(&Ships[m_ship].subsys_list));
			ptr = GET_NEXT(ptr);
		}

		m_damage = 100 - (int) ptr -> current_hits;
		if ( ship_has_scannable_subsystems && valid_cap_subsys_cargo_list(ptr->system_info->subobj_name) ) {
			enable_cargo_name = TRUE;
			if (ptr->subsys_cargo_name > 0) {
				m_cargo_name = Cargo_names[ptr->subsys_cargo_name];
			} else {
				m_cargo_name = _T("");
			}
		} else {
			m_cargo_name = _T("");
		}
		enable = TRUE;
	}

	GetDlgItem(IDC_DAMAGE) -> EnableWindow(enable);
	GetDlgItem(IDC_DAMAGE_SPIN) -> EnableWindow(enable);
	GetDlgItem(IDC_CARGO_NAME)->EnableWindow(enable && enable_cargo_name);
}

void initial_status::OnSelchangeDockerPoint() 
{
	change_docker_point(true);
}

void initial_status::change_docker_point(bool store_selection)
{
	int sel;

	// grab from controls
	UpdateData(TRUE);

	// get new selection
	sel = lstDockerPoints->GetCurSel();
	if (sel != LB_ERR)
		cur_docker_point = lstDockerPoints->GetItemData(sel);
	else
		cur_docker_point = -1;

	// populate the next dropdowns
	if (cur_docker_point < 0)
	{
		// clear the dropdowns
		list_dockees(-1);
		list_dockee_points(-1);
	}
	else
	{
		// populate with all possible dockees
		list_dockees(model_get_dock_index_type(Ship_info[Ships[m_ship].ship_info_index].model_num, cur_docker_point));

		// see if there's a dockee here
		if (dockpoint_array[cur_docker_point].dockee_shipnum >= 0)
		{
			// select the dockee
			cboDockees->SelectString(-1, Ships[dockpoint_array[cur_docker_point].dockee_shipnum].ship_name);
			change_dockee(false);
		}
		else
		{
			// select "Nothing"
			cboDockees->SetCurSel(0);
			change_dockee(true);
		}
	}

	// store to controls
	UpdateData(FALSE);
}

void initial_status::OnSelchangeDockee() 
{
	change_dockee(true);
}

void initial_status::change_dockee(bool store_selection)
{
	int sel;

	// grab from controls
	UpdateData(TRUE);

	// get new selection
	sel = cboDockees->GetCurSel();
	if (sel != CB_ERR)
		cur_dockee = cboDockees->GetItemData(sel);
	else
		cur_dockee = -1;

	// are we storing it?
	if (store_selection)
	{
		dockpoint_array[cur_docker_point].dockee_shipnum = cur_dockee;
		dockpoint_array[cur_docker_point].dockee_point = -1;
	}

	// populate the next dropdown
	if (cur_dockee < 0)
	{
		// clear the dropdown
		list_dockee_points(-1);
	}
	else
	{
		// populate with dockee points
		list_dockee_points(cur_dockee);

		// see if there's a dockpoint here
		if (dockpoint_array[cur_docker_point].dockee_point >= 0)
		{
			// select the dockpoint
			cboDockeePoints->SelectString(-1, model_get_dock_name(Ship_info[Ships[cur_dockee].ship_info_index].model_num, dockpoint_array[cur_docker_point].dockee_point));
			change_dockee_point(false);
		}
		// there might not be any dockpoints available
		else if (cboDockeePoints->GetCount() == 0)
		{
			cboDockeePoints->EnableWindow(FALSE);
			change_dockee_point(false);
		}
		else
		{
			// just select the first dockpoint
			cboDockeePoints->SetCurSel(0);
			change_dockee_point(true);
		}
	}

	// store to controls
	UpdateData(FALSE);
}

void initial_status::OnSelchangeDockeePoint()
{
	change_dockee_point(true);
}

void initial_status::change_dockee_point(bool store_selection)
{
	int sel;

	// grab from controls
	UpdateData(TRUE);

	// get new value
	sel = cboDockeePoints->GetCurSel();
	if (sel != CB_ERR)
		cur_dockee_point = cboDockeePoints->GetItemData(sel);
	else
		cur_dockee_point = -1;

	// are we storing it?
	if (store_selection)
	{
		dockpoint_array[cur_docker_point].dockee_point = cur_dockee_point;
	}

	// store to controls
	UpdateData(FALSE);
}

void initial_status::list_dockees(int dock_types)
{
	int z;

	// enable/disable dropdown
	cboDockees->EnableWindow((dock_types >= 0) ? TRUE : FALSE);

	// clear the existing dockees
	cboDockees->ResetContent();

	// that might be all we need to do
	if (dock_types < 0)
		return;

	// populate with potential dockees

	// add "nothing"
	z = cboDockees->AddString("Nothing");
	cboDockees->SetItemData(z, -1);

	// add ships
	for (object *objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp))
	{
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))
		{
			int ship = get_ship_from_obj(objp);

			// mustn't be the same ship
			if (ship == m_ship)
				continue;

			// mustn't also be docked elsewhere
			bool docked_elsewhere = false;
			for (int i = 0; i < num_dock_points; i++)
			{
				// don't erroneously check the same point
				if (i == cur_docker_point)
					continue;

				// see if this ship is also on a different dockpoint
				if (dockpoint_array[i].dockee_shipnum == ship)
				{
					docked_elsewhere = true;
					break;
				}
			}

			if (docked_elsewhere)
				continue;

			// docking must be valid
			if (!ship_docking_valid(m_ship, ship) && !ship_docking_valid(ship, m_ship))
				continue;

			// dock types must match
			if (!(model_get_dock_types(Ship_info[Ships[ship].ship_info_index].model_num) & dock_types))
				continue;

			// add to list
			z = cboDockees->AddString(Ships[ship].ship_name);
			cboDockees->SetItemData(z, ship);
		}
	}
}

void initial_status::list_dockee_points(int shipnum)
{
	int z;
	ship *shipp = &Ships[m_ship];
	ship *other_shipp = &Ships[shipnum];

	// enable/disable dropdown
	cboDockeePoints->EnableWindow((shipnum >= 0) ? TRUE : FALSE);

	// clear the existing dockee points
	cboDockeePoints->ResetContent();

	// that might be all we need to do
	if (shipnum < 0)
		return;

	// get the required dock type(s)
	int dock_type = model_get_dock_index_type(Ship_info[shipp->ship_info_index].model_num, cur_docker_point);

	// populate with the right kind of dockee points
	for (int i = 0; i < model_get_num_dock_points(Ship_info[other_shipp->ship_info_index].model_num); i++)
	{
		// make sure this dockpoint is not occupied by someone else
		object *docked_objp = dock_find_object_at_dockpoint(&Objects[other_shipp->objnum], i);
		if ((docked_objp != NULL) && (docked_objp != &Objects[shipp->objnum]))
			continue;
		
		// make sure its type matches
		if (!(model_get_dock_index_type(Ship_info[other_shipp->ship_info_index].model_num, i) & dock_type))
			continue;

		// add to list
		z = cboDockeePoints->AddString(model_get_dock_name(Ship_info[other_shipp->ship_info_index].model_num, i));
		cboDockeePoints->SetItemData(z, i);
	}
}

void initial_status::update_docking_info()
{
	int i;
	object *objp = &Objects[Ships[m_ship].objnum];

	// remove old info
	for (i = 0; i < num_dock_points; i++)
	{
		// see if the object at this point is no longer there
		object *dockee_objp = dock_find_object_at_dockpoint(objp, i);
		if (dockee_objp != NULL)
		{
			// check if the dockee ship thinks that this ship is docked to this dock point
			if (objp != dock_find_object_at_dockpoint(dockee_objp, dockpoint_array[i].dockee_point) ) {
				// undock it
				undock(objp, dockee_objp);
			}
		}
	}

	// add new info
	for (i = 0; i < num_dock_points; i++)
	{
		// see if there is an object at this point that wasn't there before
		if (dockpoint_array[i].dockee_shipnum >= 0)
		{
			if (dock_find_object_at_dockpoint(objp, i) == NULL)
			{
				object *dockee_objp = &Objects[Ships[dockpoint_array[i].dockee_shipnum].objnum];
				int dockee_point = dockpoint_array[i].dockee_point;

				// dock it
				dock(objp, i, dockee_objp, dockee_point);
			}
		}
	}

	update_map_window();
}

void initial_status::dock(object *objp, int dockpoint, object *other_objp, int other_dockpoint)
{
	if (objp == NULL || other_objp == NULL)
		return;

	if (dockpoint < 0 || other_dockpoint < 0)
		return;

	dock_function_info dfi;

	// do the docking (do it in reverse so that the current object stays put)
	ai_dock_with_object(other_objp, other_dockpoint, objp, dockpoint, AIDO_DOCK_NOW);

	// unmark the handled flag in preparation for the next step
	dock_evaluate_all_docked_objects(objp, &dfi, initial_status_unmark_dock_handled_flag);

	// move all other objects to catch up with it
	dock_move_docked_objects(objp);

	// set the dock leader
	dock_evaluate_all_docked_objects(objp, &dfi, initial_status_mark_dock_leader_helper);

	// if no leader, mark me
	if (dfi.maintained_variables.int_value == 0)
		Ships[objp->instance].flags |= SF_DOCK_LEADER;
}

void initial_status::undock(object *objp1, object *objp2)
{
	vec3d v;
	int ship_num, other_ship_num;

	if (objp1 == NULL || objp2 == NULL)
		return;

	vm_vec_sub(&v, &objp2->pos, &objp1->pos);
	vm_vec_normalize(&v);
	ship_num = get_ship_from_obj(OBJ_INDEX(objp1));
	other_ship_num = get_ship_from_obj(OBJ_INDEX(objp2));

	if (ship_class_compare(Ships[ship_num].ship_info_index, Ships[other_ship_num].ship_info_index) > 0)
		vm_vec_scale_add2(&objp2->pos, &v, objp2->radius * 2.0f);
	else
		vm_vec_scale_add2(&objp1->pos, &v, objp1->radius * -2.0f);

	ai_do_objects_undocked_stuff(objp1, objp2);

	// check to see if one of these ships has an arrival cue of false.  If so, then
	// reset it back to default value of true.  be sure to correctly update before
	// and after setting data.
	// Goober5000 - but don't reset it if it's part of a wing!
	Ship_editor_dialog.update_data(1);

	if ( Ships[ship_num].arrival_cue == Locked_sexp_false && Ships[ship_num].wingnum < 0 ) {
		Ships[ship_num].arrival_cue = Locked_sexp_true;
	} else if ( Ships[other_ship_num].arrival_cue == Locked_sexp_false && Ships[other_ship_num].wingnum < 0 ) {
		Ships[other_ship_num].arrival_cue = Locked_sexp_true;
	}

	// if this ship is no longer docked, ensure its dock leader flag is clear
	if (!object_is_docked(&Objects[Ships[ship_num].objnum]))
		Ships[ship_num].flags &= ~SF_DOCK_LEADER;

	// same for the other ship
	if (!object_is_docked(&Objects[Ships[other_ship_num].objnum]))
		Ships[other_ship_num].flags &= ~SF_DOCK_LEADER;

	Ship_editor_dialog.initialize_data(1);
}

// ----- the following are not contained in the class because of apparent scope issues -----

// NOTE - in both retail and SCP, the dock "leader" is defined as the only guy in his
// group with a non-false arrival cue
void initial_status_mark_dock_leader_helper(object *objp, dock_function_info *infop)
{
	ship *shipp = &Ships[objp->instance];

	// all ships except the leader should have a locked false arrival cue
	if (shipp->arrival_cue != Locked_sexp_false)
	{
		object *existing_leader;

		// increment number of leaders found
		infop->maintained_variables.int_value++;

		// see if we already found a leader
		existing_leader = infop->maintained_variables.objp_value;
		if (existing_leader != NULL)
		{
			ship *leader_shipp = &Ships[existing_leader->instance];

			// keep existing leader if he has a higher priority than us
			if (ship_class_compare(shipp->ship_info_index, leader_shipp->ship_info_index) < 0)
			{
				// set my arrival cue to false
				reset_arrival_to_false(SHIP_INDEX(shipp), true);
				return;
			}

			// otherwise, unmark the existing leader and set his arrival cue to false
			leader_shipp->flags &= ~SF_DOCK_LEADER;
			reset_arrival_to_false(SHIP_INDEX(leader_shipp), true);
		}

		// mark and save me as the leader
		shipp->flags |= SF_DOCK_LEADER;
		infop->maintained_variables.objp_value = objp;
	}
}

// self-explanatory, really
void initial_status_unmark_dock_handled_flag(object *objp, dock_function_info *infop)
{
	objp->flags &= ~OF_DOCKED_ALREADY_HANDLED;
}

bool set_cue_to_false(int *cue)
{
	// if the cue is not false, make it false.  Be sure to set all ship editor dialog functions
	// to update data before and after we modify the cue.
	if (*cue != Locked_sexp_false)
	{
		Ship_editor_dialog.update_data(1);

		free_sexp2(*cue);
		*cue = Locked_sexp_false;

		Ship_editor_dialog.initialize_data(1);

		return true;
	}
	else
		return false;
}

// function to set the arrival cue of a ship to false
void reset_arrival_to_false(int shipnum, bool reset_wing)
{
	char buf[256];
	ship *shipp = &Ships[shipnum];

	// falsify the ship cue
	if (set_cue_to_false(&shipp->arrival_cue))
	{
		sprintf(buf, "Setting arrival cue of ship %s\nto false for initial docking purposes.", shipp->ship_name);
		MessageBox(NULL, buf, "", MB_OK | MB_ICONEXCLAMATION);
	}

	// falsify the wing cue and all ships in that wing
	if (reset_wing && shipp->wingnum >= 0)
	{
		int i;
		wing *wingp = &Wings[shipp->wingnum];

		if (set_cue_to_false(&wingp->arrival_cue))
		{
			sprintf(buf, "Setting arrival cue of wing %s\nto false for initial docking purposes.", wingp->name);
			MessageBox(NULL, buf, "", MB_OK | MB_ICONEXCLAMATION);
		}

		for (i = 0; i < wingp->wave_count; i++)
			reset_arrival_to_false(wingp->ship_index[i], false);
	}
}

void initial_status::OnPrimariesLocked() 
{
	if (m_primaries_locked == 1)
		m_primaries_locked = 0;
	else
		m_primaries_locked = 1;

	((CButton *) GetDlgItem(IDC_PRIMARIES_LOCKED))->SetCheck(m_primaries_locked);	
}

void initial_status::OnSecondariesLocked() 
{
	if (m_secondaries_locked == 1)
		m_secondaries_locked = 0;
	else
		m_secondaries_locked = 1;

	((CButton *) GetDlgItem(IDC_SECONDARIES_LOCKED))->SetCheck(m_secondaries_locked);	
}

void initial_status::OnTurretsLocked() 
{
	if (m_turrets_locked == 1)
		m_turrets_locked = 0;
	else
		m_turrets_locked = 1;

	((CButton *) GetDlgItem(IDC_TURRETS_LOCKED))->SetCheck(m_turrets_locked);	
}

void initial_status::OnAfterburnersLocked() 
{
	if (m_afterburner_locked == 1)
		m_afterburner_locked = 0;
	else
		m_afterburner_locked = 1;

	((CButton *) GetDlgItem(IDC_AFTERBURNER_LOCKED))->SetCheck(m_afterburner_locked);	
}
