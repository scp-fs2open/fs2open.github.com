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

	for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
	{
		m_gun[bank] = -1;
	}
	for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
	{
		m_missile[bank] = -1;
		m_ammo[bank] = BLANK_FIELD;
		m_ammo_max[bank] = 0;
	}

	m_IDC_GUN[0] = IDC_GUN1;
	m_IDC_GUN[1] = IDC_GUN2;
	m_IDC_GUN[2] = IDC_GUN3;
	m_IDC_MISSILE[0] = IDC_MISSILE1;
	m_IDC_MISSILE[1] = IDC_MISSILE2;
	m_IDC_MISSILE[2] = IDC_MISSILE3;
	m_IDC_MISSILE[3] = IDC_MISSILE4;
	m_IDC_AMMO[0] = IDC_AMMO1;
	m_IDC_AMMO[1] = IDC_AMMO2;
	m_IDC_AMMO[2] = IDC_AMMO3;
	m_IDC_AMMO[3] = IDC_AMMO4;
	m_IDC_SPIN[0] = IDC_SPIN1;
	m_IDC_SPIN[1] = IDC_SPIN2;
	m_IDC_SPIN[2] = IDC_SPIN3;
	m_IDC_SPIN[3] = IDC_SPIN4;

	m_cur_item = -1;
	//}}AFX_DATA_INIT
	m_last_item = -1;
	m_multi_edit = 0;

	cur_weapon = nullptr;
	m_ship = -1;
	m_ship_class = -1;
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

	for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
	{
		DDX_CBIndex(pDX, m_IDC_GUN[bank], m_gun[bank]);
	}
	for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
	{
		DDX_CBIndex(pDX, m_IDC_MISSILE[bank], m_missile[bank]);
		DDX_Control(pDX, m_IDC_SPIN[bank], m_spin[bank]);
	}

	DDX_CBIndex(pDX, IDC_AI_CLASS, m_ai_class);
	DDX_LBIndex(pDX, IDC_LIST, m_cur_item);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
		{
			GetDlgItem(m_IDC_AMMO[bank])->GetWindowText(str);
			if (save_number((char *) (LPCSTR) str, &m_ammo[bank])) {
				if (m_missile[bank] <= 0) {
					m_ammo_max[bank] = 0;
				} else {
					int weapon_class = combo_index_to_weapon_class(m_IDC_MISSILE[bank], m_missile[bank]);
					m_ammo_max[bank] = (m_cur_item == 0) 
						? get_max_ammo_count_for_bank(m_ship_class, bank, weapon_class)
						: get_max_ammo_count_for_turret_bank(cur_weapon, bank, weapon_class);
				}
				CLAMP(m_ammo[bank], 0, m_ammo_max[bank]);
			}

			m_spin[bank].SetRange(0, (short)m_ammo_max[bank]);
		}
	} else {
		for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
		{
			if (m_ammo[bank] != BLANK_FIELD)
				DDX_Text(pDX, m_IDC_AMMO[bank], m_ammo[bank]);
			else
				GetDlgItem(m_IDC_AMMO[bank])->SetWindowText("");
		}
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
	int i, z, big = 1, inst, flag = 0;
	object *ptr;
	model_subsystem *psub;
	ship_subsys *ssl, *pss;
	CComboBox *box;
	CListBox *list;

	CDialog::OnInitDialog();
	m_ship = cur_ship;
	if (m_ship == -1)
		m_ship = Objects[cur_object_index].instance;

	list = (CListBox *) GetDlgItem(IDC_LIST);

	z = list->AddString("Pilot");
	if (m_multi_edit) {
		list->SetItemDataPtr(z, &pilot);
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				inst = ptr->instance;
                if (!(Ship_info[Ships[inst].ship_info_index].is_big_or_huge()))
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
        if (!(Ship_info[Ships[m_ship].ship_info_index].is_big_or_huge()))
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

	// all weapon boxes have None at the top
	for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
	{
		box = (CComboBox*)GetDlgItem(m_IDC_GUN[bank]);
		box->ResetContent();
		box->AddString("None");
		box->SetItemData(0, (DWORD_PTR)-1);
	}
	for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
	{
		box = (CComboBox*)GetDlgItem(m_IDC_MISSILE[bank]);
		box->ResetContent();
		box->AddString("None");
		box->SetItemData(0, (DWORD_PTR)-1);
	}

	// populate all weapon boxes with items mapped to weapon classes
	for (int weapon_class = 0; weapon_class < weapon_info_size(); weapon_class++)
	{
		auto wip = &Weapon_info[weapon_class];

		// can't select child weapons or no_fred weapons
		if (wip->wi_flags[Weapon::Info_Flags::Child] || wip->wi_flags[Weapon::Info_Flags::No_fred])
			continue;

		// can only select big weapons if the ship is big
		if (!big && wip->wi_flags[Weapon::Info_Flags::Big_only])
			continue;

		// primary weapon
		if (weapon_class < First_secondary_index)
		{
			for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
			{
				box = (CComboBox*)GetDlgItem(m_IDC_GUN[bank]);
				i = box->GetCount();
				box->AddString(wip->name);
				box->SetItemData(i, weapon_class);
			}
		}
		// secondary weapon
		else
		{
			for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
			{
				box = (CComboBox*)GetDlgItem(m_IDC_MISSILE[bank]);
				i = box->GetCount();
				box->AddString(wip->name);
				box->SetItemData(i, weapon_class);
			}
		}
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
	std::array<CString, MAX_SHIP_SECONDARY_BANKS> ammo_str;

	for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
		GetDlgItem(m_IDC_AMMO[bank])->GetWindowText(ammo_str[bank]);

	if (m_last_item >= 0) {
		cur_weapon->ai_class = m_ai_class;

		for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
			cur_weapon->primary_bank_weapons[bank] = combo_index_to_weapon_class(m_IDC_GUN[bank], m_gun[bank]);

		for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
		{
			cur_weapon->secondary_bank_weapons[bank] = combo_index_to_weapon_class(m_IDC_MISSILE[bank], m_missile[bank]);
			cur_weapon->secondary_bank_ammo[bank] = m_ammo_max[bank] ? fl2ir(m_ammo[bank] * 100.0f / m_ammo_max[bank]) : 0;
		}

		if (m_multi_edit) {
			for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
				if (ammo_str[bank].IsEmpty())
					cur_weapon->secondary_bank_ammo[bank] = BLANK_FIELD;
		}
	}

	for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
	{
		m_gun[bank] = -1;
	}
	for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
	{
		m_missile[bank] = -1;
		m_ammo[bank] = BLANK_FIELD;
		m_ammo_max[bank] = 0;
	}

	if (m_cur_item < 0) {
		m_last_item = m_cur_item;

		for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
		{
			GetDlgItem(m_IDC_GUN[bank])->EnableWindow(FALSE);
		}
		for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
		{
			GetDlgItem(m_IDC_MISSILE[bank])->EnableWindow(FALSE);
			GetDlgItem(m_IDC_AMMO[bank])->EnableWindow(FALSE);
			GetDlgItem(m_IDC_SPIN[bank])->EnableWindow(FALSE);
		}

		GetDlgItem(IDC_AI_CLASS)->EnableWindow(FALSE);
		UpdateData(FALSE);
		return;
	}

	cur_weapon = (ship_weapon *) ((CListBox *) GetDlgItem(IDC_LIST))->GetItemDataPtr(m_cur_item);
	GetDlgItem(IDC_AI_CLASS)->EnableWindow(TRUE);
	m_ai_class = cur_weapon->ai_class;

	for (int bank = 0; bank < MAX_SHIP_PRIMARY_BANKS; bank++)
	{
		if (cur_weapon->num_primary_banks > bank) {
			m_gun[bank] = weapon_class_to_combo_index(m_IDC_GUN[bank], cur_weapon->primary_bank_weapons[bank]);
			GetDlgItem(m_IDC_GUN[bank])->EnableWindow(TRUE);
		} else
			GetDlgItem(m_IDC_GUN[bank])->EnableWindow(FALSE);
	}

	for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
	{
		if (cur_weapon->num_secondary_banks > bank) {
			m_missile[bank] = weapon_class_to_combo_index(m_IDC_MISSILE[bank], cur_weapon->secondary_bank_weapons[bank]);
			if (m_missile[bank] > 0) {
				if (m_cur_item == 0) {
					m_ammo_max[bank] = get_max_ammo_count_for_bank(m_ship_class, bank, cur_weapon->secondary_bank_weapons[bank]);
				} else {
					m_ammo_max[bank] = get_max_ammo_count_for_turret_bank(cur_weapon, bank, cur_weapon->secondary_bank_weapons[bank]);
				}
				if (cur_weapon->secondary_bank_ammo[bank] != BLANK_FIELD)
					m_ammo[bank] = fl2ir(cur_weapon->secondary_bank_ammo[bank] * m_ammo_max[bank] / 100.0f);
			}

			GetDlgItem(m_IDC_MISSILE[bank])->EnableWindow(TRUE);
			GetDlgItem(m_IDC_AMMO[bank])->EnableWindow(TRUE);
			GetDlgItem(m_IDC_SPIN[bank])->EnableWindow(TRUE);

		} else {
			GetDlgItem(m_IDC_MISSILE[bank])->EnableWindow(FALSE);
			GetDlgItem(m_IDC_AMMO[bank])->EnableWindow(FALSE);
			GetDlgItem(m_IDC_SPIN[bank])->EnableWindow(FALSE);
		}
	}

	m_last_item = m_cur_item;
	UpdateData(FALSE);
	if (m_multi_edit) {
		for (int bank = 0; bank < MAX_SHIP_SECONDARY_BANKS; bank++)
			if (m_ammo[bank] == BLANK_FIELD)
				GetDlgItem(m_IDC_AMMO[bank])->SetWindowText("");
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
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
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
	OnSelchangeMissile(0);
}

void WeaponEditorDlg::OnSelchangeMissile2()
{
	OnSelchangeMissile(1);
}

void WeaponEditorDlg::OnSelchangeMissile3()
{
	OnSelchangeMissile(2);
}

void WeaponEditorDlg::OnSelchangeMissile4()
{
	OnSelchangeMissile(3);
}

void WeaponEditorDlg::OnSelchangeMissile(int secondary_index)
{
	UpdateData(TRUE);
	UpdateData(TRUE);

	if (m_missile[secondary_index] == 0) {
		m_ammo_max[secondary_index] = 0;
	} else {
		int weapon_class = combo_index_to_weapon_class(m_IDC_MISSILE[secondary_index], m_missile[secondary_index]);

		if (m_cur_item == 0) {
			m_ammo_max[secondary_index] = get_max_ammo_count_for_bank(m_ship_class, secondary_index, weapon_class);
		} else {
			m_ammo_max[secondary_index] = get_max_ammo_count_for_turret_bank(cur_weapon, secondary_index, weapon_class);
		}
	}
	m_ammo[secondary_index] = m_ammo_max[secondary_index] ? m_ammo_max[secondary_index] : 0;
	change_selection();
}

int WeaponEditorDlg::combo_index_to_weapon_class(int dialog_id, int combo_index)
{
	auto ptr = (CComboBox*)GetDlgItem(dialog_id);

	if (combo_index < 0 || combo_index >= ptr->GetCount())
		return -1;

	return (int)ptr->GetItemData(combo_index);
}

int WeaponEditorDlg::weapon_class_to_combo_index(int dialog_id, int weapon_class)
{
	if (weapon_class < 0 || weapon_class >= weapon_info_size())
		return 0;	// "None"

	auto ptr = (CComboBox*)GetDlgItem(dialog_id);

	for (int i = 0; i < ptr->GetCount(); i++)
		if ((int)ptr->GetItemData(i) == weapon_class)
			return i;

	return -1;	// will not display anything
}
