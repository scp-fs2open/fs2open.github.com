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
#include "WeaponEditorDlg.h"
#include "globalincs/linklist.h"
#include "Management.h"
#include "weapon/weapon.h"
#include "ship/ship.h"

#define BLANK_FIELD -99

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WeaponEditorDlg dialog

WeaponEditorDlg::WeaponEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(WeaponEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(WeaponEditorDlg)
	m_ai_class = -1;
	m_ammo1 = 0;
	m_ammo2 = 0;
	m_ammo3 = 0;
	m_ammo4 = 0;
	m_gun1 = -1;
	m_gun2 = -1;
	m_gun3 = -1;
	m_missile1 = -1;
	m_missile2 = -1;
	m_missile3 = -1;
	m_missile4 = -1;
	m_cur_item = -1;
	//}}AFX_DATA_INIT
	m_last_item = -1;
	m_multi_edit = 0;
}

int save_number(char *str, int *val)
{
	char buf[40];
	int num;

	num = atoi(str);
	sprintf(buf, "%d", num);
	if (strncmp(str, buf, strlen(buf)))
		return 0;

	*val = num;
	return 1;
}

void WeaponEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CString str;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WeaponEditorDlg)
	DDX_Control(pDX, IDC_SPIN4, m_spin4);
	DDX_Control(pDX, IDC_SPIN3, m_spin3);
	DDX_Control(pDX, IDC_SPIN2, m_spin2);
	DDX_Control(pDX, IDC_SPIN1, m_spin1);
	DDX_CBIndex(pDX, IDC_AI_CLASS, m_ai_class);
	DDX_CBIndex(pDX, IDC_GUN1, m_gun1);
	DDX_CBIndex(pDX, IDC_GUN2, m_gun2);
	DDX_CBIndex(pDX, IDC_GUN3, m_gun3);
	DDX_CBIndex(pDX, IDC_MISSILE1, m_missile1);
	DDX_CBIndex(pDX, IDC_MISSILE2, m_missile2);
	DDX_CBIndex(pDX, IDC_MISSILE3, m_missile3);
	DDX_CBIndex(pDX, IDC_MISSILE4, m_missile4);
	DDX_LBIndex(pDX, IDC_LIST, m_cur_item);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		GetDlgItem(IDC_AMMO1)->GetWindowText(str);
		if (save_number((char *) (LPCSTR) str, &m_ammo1)) {
			m_ammo_max1 = (m_missile1 <= 0) ? 0 : get_max_ammo_count_for_bank(m_ship_class, 0, m_missile1 + First_secondary_index - 1);
			if (m_ammo1 < 0)
				m_ammo1 = 0;
			if (m_ammo1 > m_ammo_max1)
				m_ammo1 = m_ammo_max1;
		}

		GetDlgItem(IDC_AMMO2)->GetWindowText(str);
		if (save_number((char *) (LPCSTR) str, &m_ammo2)) {
			m_ammo_max2 = (m_missile2 <= 0) ? 0 : get_max_ammo_count_for_bank(m_ship_class, 1, m_missile2 + First_secondary_index - 1);
			if (m_ammo2 < 0)
				m_ammo2 = 0;
			if (m_ammo2 > m_ammo_max2)
				m_ammo2 = m_ammo_max2;
		}

		GetDlgItem(IDC_AMMO3)->GetWindowText(str);
		if (save_number((char *) (LPCSTR) str, &m_ammo3)) {
			m_ammo_max3 = (m_missile3 <= 0) ? 0 : get_max_ammo_count_for_bank(m_ship_class, 2, m_missile3 + First_secondary_index - 1);
			if (m_ammo3 < 0)
				m_ammo3 = 0;
			if (m_ammo3 > m_ammo_max3)
				m_ammo3 = m_ammo_max3;
		}

		GetDlgItem(IDC_AMMO4)->GetWindowText(str);
		if (save_number((char *) (LPCSTR) str, &m_ammo4)) {
			m_ammo_max4 = (m_missile4 <= 0) ? 0 : get_max_ammo_count_for_bank(m_ship_class, 3, m_missile4 + First_secondary_index - 1);
			if (m_ammo4 < 0)
				m_ammo4 = 0;
			if (m_ammo4 > m_ammo_max4)
				m_ammo4 = m_ammo_max4;
		}

		m_spin1.SetRange(0, m_ammo_max1);
		m_spin2.SetRange(0, m_ammo_max2);
		m_spin3.SetRange(0, m_ammo_max3);
		m_spin4.SetRange(0, m_ammo_max4);

	} else {
		if (m_ammo1 != BLANK_FIELD)
			DDX_Text(pDX, IDC_AMMO1, m_ammo1);
		else
			GetDlgItem(IDC_AMMO1)->SetWindowText("");

		if (m_ammo2 != BLANK_FIELD)
			DDX_Text(pDX, IDC_AMMO2, m_ammo2);
		else
			GetDlgItem(IDC_AMMO2)->SetWindowText("");

		if (m_ammo3 != BLANK_FIELD)
			DDX_Text(pDX, IDC_AMMO3, m_ammo3);
		else
			GetDlgItem(IDC_AMMO3)->SetWindowText("");

		if (m_ammo4 != BLANK_FIELD)
			DDX_Text(pDX, IDC_AMMO4, m_ammo4);
		else
			GetDlgItem(IDC_AMMO4)->SetWindowText("");
	}
}

BEGIN_MESSAGE_MAP(WeaponEditorDlg, CDialog)
	//{{AFX_MSG_MAP(WeaponEditorDlg)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_MISSILE1, OnSelchangeMissile1)
	ON_CBN_SELCHANGE(IDC_MISSILE2, OnSelchangeMissile2)
	ON_CBN_SELCHANGE(IDC_MISSILE3, OnSelchangeMissile3)
	ON_CBN_SELCHANGE(IDC_MISSILE4, OnSelchangeMissile4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WeaponEditorDlg message handlers

BOOL WeaponEditorDlg::OnInitDialog() 
{
	int i, z, big = 1, end1, end2, inst, flag = 0;
	object *ptr;
	model_subsystem *psub;
	ship_subsys *ssl, *pss;
	CComboBox *box;
	CListBox *list;

	CDialog::OnInitDialog();
	m_ship = cur_ship;
	if (m_ship == -1)
		m_ship = Objects[cur_object_index].instance;

	end1 = First_secondary_index;
	end2 = Num_weapon_types;

	list = (CListBox *) GetDlgItem(IDC_LIST);

	z = list->AddString("Pilot");
	if (m_multi_edit) {
		list->SetItemDataPtr(z, &pilot);
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags & OF_MARKED)) {
				inst = ptr->instance;
				if (!(ship_get_SIF(inst) & (SIF_BIG_SHIP | SIF_HUGE_SHIP)))
					big = 0;

				if (!flag) {
					pilot = Ships[inst].weapons;
					m_ship_class = Ships[inst].ship_info_index;
					flag = 1;

				} else {
					Assert(Ships[inst].ship_info_index == m_ship_class);
					if (pilot.ai_class != Ships[inst].weapons.ai_class)
						pilot.ai_class = BLANK_FIELD;

					for (i=0; i<MAX_SHIP_PRIMARY_BANKS; i++)
						if (pilot.primary_bank_weapons[i] != Ships[inst].weapons.primary_bank_weapons[i])
							pilot.primary_bank_weapons[i] = BLANK_FIELD;

					for (i=0; i<MAX_SHIP_SECONDARY_BANKS; i++) {
						if (pilot.secondary_bank_weapons[i] != Ships[inst].weapons.secondary_bank_weapons[i])
							pilot.secondary_bank_weapons[i] = BLANK_FIELD;
						if (pilot.secondary_bank_ammo[i] != Ships[inst].weapons.secondary_bank_ammo[i])
							pilot.secondary_bank_ammo[i] = BLANK_FIELD;
					}
				}
			}

			ptr = GET_NEXT(ptr);
		}

	} else {
		if (!(ship_get_SIF(m_ship) & (SIF_BIG_SHIP | SIF_HUGE_SHIP)))
			big = 0;

		m_ship_class = Ships[m_ship].ship_info_index;
		list->SetItemDataPtr(z, &Ships[m_ship].weapons);
		ssl = &Ships[m_ship].subsys_list;
		for (pss = GET_FIRST(ssl); pss != END_OF_LIST(ssl); pss = GET_NEXT(pss)) {
			psub = pss->system_info;
			if (psub->type == SUBSYSTEM_TURRET) {
				z = list->AddString(psub->subobj_name);
				list->SetItemDataPtr(z, &pss->weapons);
			}
		}
	}

	box = (CComboBox *) GetDlgItem(IDC_AI_CLASS);
	for (i=0; i<Num_ai_classes; i++){
		box->AddString(Ai_class_names[i]);
	}

	for (i=0; i<end1; i++){
		if ((Weapon_info[i].wi_flags & WIF_CHILD) || (!big && (Weapon_info[i].wi_flags & WIF_BIG_ONLY))){
			end1 = i;
		}
	}

	box = (CComboBox *) GetDlgItem(IDC_GUN1);
	box->AddString("None");
	for (i=0; i<end1; i++){
		box->AddString(Weapon_info[i].name);
	}

	box = (CComboBox *) GetDlgItem(IDC_GUN2);
	box->AddString("None");
	for (i=0; i<end1; i++){
		box->AddString(Weapon_info[i].name);
	}

	box = (CComboBox *) GetDlgItem(IDC_GUN3);
	box->AddString("None");
	for (i=0; i<end1; i++){
		box->AddString(Weapon_info[i].name);
	}

	for (i=First_secondary_index; i<end2; i++){
		if ((Weapon_info[i].wi_flags & WIF_CHILD) || (!big && (Weapon_info[i].wi_flags & WIF_BIG_ONLY))){
			end2 = i;
		}
	}

	box = (CComboBox *) GetDlgItem(IDC_MISSILE1);
	box->AddString("None");
	for (i=First_secondary_index; i<end2; i++){
		box->AddString(Weapon_info[i].name);
	}

	box = (CComboBox *) GetDlgItem(IDC_MISSILE2);
	box->AddString("None");
	for (i=First_secondary_index; i<end2; i++){
		box->AddString(Weapon_info[i].name);
	}

	box = (CComboBox *) GetDlgItem(IDC_MISSILE3);
	box->AddString("None");
	for (i=First_secondary_index; i<end2; i++){
		box->AddString(Weapon_info[i].name);
	}

	box = (CComboBox *) GetDlgItem(IDC_MISSILE4);
	box->AddString("None");
	for (i=First_secondary_index; i<end2; i++){
		box->AddString(Weapon_info[i].name);
	}

	m_cur_item = 0;
	UpdateData(FALSE);
	change_selection();
	UpdateData(TRUE);
	return TRUE;
}

void WeaponEditorDlg::OnSelchangeList() 
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	change_selection();
}

void WeaponEditorDlg::change_selection()
{
	CString a1, a2, a3, a4;

	GetDlgItem(IDC_AMMO1)->GetWindowText(a1);
	GetDlgItem(IDC_AMMO2)->GetWindowText(a2);
	GetDlgItem(IDC_AMMO3)->GetWindowText(a3);
	GetDlgItem(IDC_AMMO4)->GetWindowText(a4);

	if (m_last_item >= 0) {
		cur_weapon->ai_class = m_ai_class;
		cur_weapon->primary_bank_weapons[0] = m_gun1 - 1;
		cur_weapon->primary_bank_weapons[1] = m_gun2 - 1;
		cur_weapon->primary_bank_weapons[2] = m_gun3 - 1;
		if (m_missile1 > 0)
			m_missile1 += First_secondary_index;

		cur_weapon->secondary_bank_weapons[0] = m_missile1 - 1;
		if (m_missile2 > 0)
			m_missile2 += First_secondary_index;

		cur_weapon->secondary_bank_weapons[1] = m_missile2 - 1;
		if (m_missile3 > 0)
			m_missile3 += First_secondary_index;

		cur_weapon->secondary_bank_weapons[2] = m_missile3 - 1;
		if (m_missile4 > 0)
			m_missile4 += First_secondary_index;

		cur_weapon->secondary_bank_weapons[3] = m_missile4 - 1;
		cur_weapon->secondary_bank_ammo[0] = m_ammo_max1 ? (m_ammo1 * 100 / m_ammo_max1) : 0;
		cur_weapon->secondary_bank_ammo[1] = m_ammo_max2 ? (m_ammo2 * 100 / m_ammo_max2) : 0;
		cur_weapon->secondary_bank_ammo[2] = m_ammo_max3 ? (m_ammo3 * 100 / m_ammo_max3) : 0;
		cur_weapon->secondary_bank_ammo[3] = m_ammo_max4 ? (m_ammo4 * 100 / m_ammo_max4) : 0;
		if (m_multi_edit) {
			if (!strlen(a1))
				cur_weapon->secondary_bank_ammo[0] = BLANK_FIELD;
			if (!strlen(a2))
				cur_weapon->secondary_bank_ammo[1] = BLANK_FIELD;
			if (!strlen(a3))
				cur_weapon->secondary_bank_ammo[2] = BLANK_FIELD;
			if (!strlen(a4))
				cur_weapon->secondary_bank_ammo[3] = BLANK_FIELD;
		}
	}

	m_gun1 = m_gun2 = m_gun3 = m_missile1 = m_missile2 = m_missile3 = m_missile4 = -1;
	m_ammo1 = m_ammo2 = m_ammo3 = m_ammo4 = BLANK_FIELD;
	m_ammo_max1 = m_ammo_max2 = m_ammo_max3 = m_ammo_max4 = 0;
	if (m_cur_item < 0) {
		m_last_item = m_cur_item;
		GetDlgItem(IDC_GUN1)->EnableWindow(FALSE);
		GetDlgItem(IDC_GUN2)->EnableWindow(FALSE);
		GetDlgItem(IDC_GUN3)->EnableWindow(FALSE);
		GetDlgItem(IDC_MISSILE1)->EnableWindow(FALSE);
		GetDlgItem(IDC_MISSILE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_MISSILE3)->EnableWindow(FALSE);
		GetDlgItem(IDC_MISSILE4)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO3)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO4)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN1)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN2)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN3)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN4)->EnableWindow(FALSE);
		GetDlgItem(IDC_AI_CLASS)->EnableWindow(FALSE);
		UpdateData(FALSE);
		return;
	}

	cur_weapon = (ship_weapon *) ((CListBox *) GetDlgItem(IDC_LIST))->GetItemDataPtr(m_cur_item);
	GetDlgItem(IDC_AI_CLASS)->EnableWindow(TRUE);
	m_ai_class = cur_weapon->ai_class;

	if (cur_weapon->num_primary_banks > 0) {
		m_gun1 = cur_weapon->primary_bank_weapons[0] + 1;
		GetDlgItem(IDC_GUN1)->EnableWindow(TRUE);
	} else
		GetDlgItem(IDC_GUN1)->EnableWindow(FALSE);

	if (cur_weapon->num_primary_banks > 1) {
		m_gun2 = cur_weapon->primary_bank_weapons[1] + 1;
		GetDlgItem(IDC_GUN2)->EnableWindow(TRUE);
	} else
		GetDlgItem(IDC_GUN2)->EnableWindow(FALSE);

	if (cur_weapon->num_primary_banks > 2) {
		m_gun3 = cur_weapon->primary_bank_weapons[2] + 1;
		GetDlgItem(IDC_GUN3)->EnableWindow(TRUE);
	} else
		GetDlgItem(IDC_GUN3)->EnableWindow(FALSE);

	if (cur_weapon->num_secondary_banks > 0) {
		m_missile1 = cur_weapon->secondary_bank_weapons[0] + 1;
		if (m_missile1 > 0) {
			m_ammo_max1 = get_max_ammo_count_for_bank(m_ship_class, 0, m_missile1 - 1);
			if (cur_weapon->secondary_bank_ammo[0] != BLANK_FIELD)
				m_ammo1 = cur_weapon->secondary_bank_ammo[0] * m_ammo_max1 / 100;
			m_missile1 -= First_secondary_index;
		}

		GetDlgItem(IDC_MISSILE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_AMMO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_SPIN1)->EnableWindow(TRUE);

	} else {
		GetDlgItem(IDC_MISSILE1)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN1)->EnableWindow(FALSE);
	}

	if (cur_weapon->num_secondary_banks > 1) {
		m_missile2 = cur_weapon->secondary_bank_weapons[1] + 1;
		if (m_missile2 > 0) {
			m_ammo_max2 = get_max_ammo_count_for_bank(m_ship_class, 1, m_missile2 - 1);
			if (cur_weapon->secondary_bank_ammo[1] != BLANK_FIELD)
				m_ammo2 = cur_weapon->secondary_bank_ammo[1] * m_ammo_max2 / 100;
			m_missile2 -= First_secondary_index;
		}

		GetDlgItem(IDC_MISSILE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_AMMO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_SPIN2)->EnableWindow(TRUE);

	} else {
		GetDlgItem(IDC_MISSILE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN2)->EnableWindow(FALSE);
	}

	if (cur_weapon->num_secondary_banks > 2) {
		m_missile3 = cur_weapon->secondary_bank_weapons[2] + 1;
		if (m_missile3 > 0) {
			m_ammo_max3 = get_max_ammo_count_for_bank(m_ship_class, 2, m_missile3 - 1);
			if (cur_weapon->secondary_bank_ammo[2] != BLANK_FIELD)
				m_ammo3 = cur_weapon->secondary_bank_ammo[2] * m_ammo_max3 / 100;
			m_missile3 -= First_secondary_index;
		}

		GetDlgItem(IDC_MISSILE3)->EnableWindow(TRUE);
		GetDlgItem(IDC_AMMO3)->EnableWindow(TRUE);
		GetDlgItem(IDC_SPIN3)->EnableWindow(TRUE);

	} else {
		GetDlgItem(IDC_MISSILE3)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO3)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN3)->EnableWindow(FALSE);
	}

	if (cur_weapon->num_secondary_banks > 3) {
		m_missile4 = cur_weapon->secondary_bank_weapons[3] + 1;
		if (m_missile4 > 0) {
			m_ammo_max4 = get_max_ammo_count_for_bank(m_ship_class, 3, m_missile4 - 1);
			if (cur_weapon->secondary_bank_ammo[3] != BLANK_FIELD)
				m_ammo4 = cur_weapon->secondary_bank_ammo[3] * m_ammo_max4 / 100;
			m_missile4 -= First_secondary_index;
		}

		GetDlgItem(IDC_MISSILE4)->EnableWindow(TRUE);
		GetDlgItem(IDC_AMMO4)->EnableWindow(TRUE);
		GetDlgItem(IDC_SPIN4)->EnableWindow(TRUE);

	} else {
		GetDlgItem(IDC_MISSILE4)->EnableWindow(FALSE);
		GetDlgItem(IDC_AMMO4)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN4)->EnableWindow(FALSE);
	}

	m_last_item = m_cur_item;
	UpdateData(FALSE);
	if (m_multi_edit) {
		if (m_ammo1 == BLANK_FIELD)
			GetDlgItem(IDC_AMMO1)->SetWindowText("");
		if (m_ammo2 == BLANK_FIELD)
			GetDlgItem(IDC_AMMO2)->SetWindowText("");
		if (m_ammo3 == BLANK_FIELD)
			GetDlgItem(IDC_AMMO3)->SetWindowText("");
		if (m_ammo4 == BLANK_FIELD)
			GetDlgItem(IDC_AMMO4)->SetWindowText("");
	}
}

void WeaponEditorDlg::OnOK()
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	change_selection();
	update_pilot();
	CDialog::OnOK();
}

void WeaponEditorDlg::OnCancel()
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	change_selection();
	update_pilot();
	CDialog::OnCancel();
}

void WeaponEditorDlg::OnClose() 
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	change_selection();
	update_pilot();
	CDialog::OnClose();
}

void WeaponEditorDlg::update_pilot()
{
	int i;
	object *ptr;
	ship_weapon *weapon;

	if (m_multi_edit) {
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags & OF_MARKED)) {
				weapon = &Ships[ptr->instance].weapons;

				if (pilot.ai_class >= 0)
					weapon->ai_class = pilot.ai_class;
				
				for (i=0; i<MAX_SHIP_PRIMARY_BANKS; i++)
					if (pilot.primary_bank_weapons[i] != -2)
						weapon->primary_bank_weapons[i] = pilot.primary_bank_weapons[i];

				for (i=0; i<MAX_SHIP_SECONDARY_BANKS; i++) {
					if (pilot.secondary_bank_weapons[i] != -2)
						weapon->secondary_bank_weapons[i] = pilot.secondary_bank_weapons[i];
					if (pilot.secondary_bank_ammo[i] >= 0)
						weapon->secondary_bank_ammo[i] = pilot.secondary_bank_ammo[i];
				}
			}

			ptr = GET_NEXT(ptr);
		}
	}
}

void WeaponEditorDlg::OnSelchangeMissile1() 
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	m_ammo_max1 = get_max_ammo_count_for_bank(m_ship_class, 0, m_missile1 + First_secondary_index - 1);
	m_ammo1 = m_ammo_max1 ? (m_ammo_max1) : 0;
	change_selection();
}

void WeaponEditorDlg::OnSelchangeMissile2() 
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	m_ammo_max2 = get_max_ammo_count_for_bank(m_ship_class, 0, m_missile2 + First_secondary_index - 1);
	m_ammo2 = m_ammo_max2 ? (m_ammo_max2) : 0;
	change_selection();
}

void WeaponEditorDlg::OnSelchangeMissile3() 
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	m_ammo_max3 = get_max_ammo_count_for_bank(m_ship_class, 0, m_missile3 + First_secondary_index - 1);
	m_ammo3 = m_ammo_max3 ? (m_ammo_max3) : 0;
	change_selection();
}

void WeaponEditorDlg::OnSelchangeMissile4() 
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	m_ammo_max4 = get_max_ammo_count_for_bank(m_ship_class, 0, m_missile4 + First_secondary_index - 1);
	m_ammo4 = m_ammo_max4 ? (m_ammo_max4) : 0;
	change_selection();
}
