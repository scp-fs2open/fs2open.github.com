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
#include "ShieldSysDlg.h"
#include "ship/ship.h"
#include "mission/missionparse.h"
#include "iff_defs/iff_defs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

SCP_vector<int> Shield_sys_teams;
SCP_map<int, int> Shield_sys_types;	// ship class -> shield status (0=has, 1=none, 2=mixed); absent = 0

/////////////////////////////////////////////////////////////////////////////
// shield_sys_dlg dialog

shield_sys_dlg::shield_sys_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(shield_sys_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(shield_sys_dlg)
	m_team = 0;
	m_type = 0;
	//}}AFX_DATA_INIT
}

void shield_sys_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(shield_sys_dlg)
	DDX_CBIndex(pDX, IDC_TEAM, m_team);
	DDX_CBIndex(pDX, IDC_TYPE, m_type);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(shield_sys_dlg, CDialog)
	//{{AFX_MSG_MAP(shield_sys_dlg)
	ON_CBN_SELCHANGE(IDC_TEAM, OnSelchangeTeam)
	ON_CBN_SELCHANGE(IDC_TYPE, OnSelchangeType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// shield_sys_dlg message handlers

BOOL shield_sys_dlg::OnInitDialog() 
{
	int i, z;
	int* teams = new int[Iff_info.size()]();
	SCP_set<int> types_seen;		// which ship classes we've encountered this pass
	CComboBox *box;

	for (i=0; i< (int)Iff_info.size(); i++)
		teams[i] = 0;

	for (i=0; i<MAX_SHIPS; i++)
		if (Ships[i].objnum >= 0) {
			z = (Objects[Ships[i].objnum].flags[Object::Object_Flags::No_shields]) ? 1 : 0;
			if (!teams[Ships[i].team])
				Shield_sys_teams[Ships[i].team] = z;
			else if (Shield_sys_teams[Ships[i].team] != z)
				Shield_sys_teams[Ships[i].team] = 2;

			if (types_seen.insert(Ships[i].ship_info_index).second)
				Shield_sys_types[Ships[i].ship_info_index] = z;
			else if (Shield_sys_types[Ships[i].ship_info_index] != z)
				Shield_sys_types[Ships[i].ship_info_index] = 2;

			teams[Ships[i].team]++;
		}

	delete[] teams;

	box = (CComboBox *) GetDlgItem(IDC_TYPE);
	box->ResetContent();
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it)
		box->AddString(it->name);

	box = (CComboBox *) GetDlgItem(IDC_TEAM);
	box->ResetContent();
	for (i=0; i< (int)Iff_info.size(); i++)
		box->AddString(Iff_info[i].iff_name);

	CDialog::OnInitDialog();
	set_team();
	set_type();
	return TRUE;
}

void shield_sys_dlg::OnOK() 
{
	int i, z;

	OnSelchangeTeam();
	OnSelchangeType();
	for (i=0; i<MAX_SHIPS; i++)
		if (Ships[i].objnum >= 0) {
			z = Shield_sys_teams[Ships[i].team];
			int type_z = Shield_sys_types.value_or(Ships[i].ship_info_index, 0);
			if (!type_z)
				z = 0;
			else if (type_z == 1)
				z = 1;

			if (!z)
                Objects[Ships[i].objnum].flags.remove(Object::Object_Flags::No_shields);
			else if (z == 1)
                Objects[Ships[i].objnum].flags.set(Object::Object_Flags::No_shields);
		}

	CDialog::OnOK();
}

void shield_sys_dlg::OnSelchangeTeam()
{
	Assert(m_team >= 0);
	if (((CButton *) GetDlgItem(IDC_TEAM_YES))->GetCheck())
		Shield_sys_teams[m_team] = 0;
	else if (((CButton *) GetDlgItem(IDC_TEAM_NO))->GetCheck())
		Shield_sys_teams[m_team] = 1;

	UpdateData(TRUE);
	set_team();
}

void shield_sys_dlg::set_team()
{
	if (!Shield_sys_teams[m_team])
		((CButton *) GetDlgItem(IDC_TEAM_YES))->SetCheck(TRUE);
	else
		((CButton *) GetDlgItem(IDC_TEAM_YES))->SetCheck(FALSE);

	if (Shield_sys_teams[m_team] == 1)
		((CButton *) GetDlgItem(IDC_TEAM_NO))->SetCheck(TRUE);
	else
		((CButton *) GetDlgItem(IDC_TEAM_NO))->SetCheck(FALSE);
}

void shield_sys_dlg::OnSelchangeType()
{
	Assert(m_type >= 0);
	if (((CButton *) GetDlgItem(IDC_TYPE_YES))->GetCheck())
		Shield_sys_types[m_type] = 0;
	else if (((CButton *) GetDlgItem(IDC_TYPE_NO))->GetCheck())
		Shield_sys_types[m_type] = 1;

	UpdateData(TRUE);
	set_type();
}

void shield_sys_dlg::set_type()
{
	int type_z = Shield_sys_types.value_or(m_type, 0);

	if (!type_z)
		((CButton *) GetDlgItem(IDC_TYPE_YES))->SetCheck(TRUE);
	else
		((CButton *) GetDlgItem(IDC_TYPE_YES))->SetCheck(FALSE);

	if (type_z == 1)
		((CButton *) GetDlgItem(IDC_TYPE_NO))->SetCheck(TRUE);
	else
		((CButton *) GetDlgItem(IDC_TYPE_NO))->SetCheck(FALSE);
}
