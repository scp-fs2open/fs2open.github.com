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
#include "ReinforcementEditorDlg.h"
#include "mission/missionparse.h"
#include "globalincs/linklist.h"
#include "ship/ship.h"
#include "FREDDoc.h"
#include "Management.h"
#include "mission/missionmessage.h"

#define ID_WING_DATA 9000

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// reinforcement_editor_dlg dialog

reinforcement_editor_dlg::reinforcement_editor_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(reinforcement_editor_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(reinforcement_editor_dlg)
	m_uses = 0;
	m_delay = 0;
	//}}AFX_DATA_INIT
	cur = -1;
}

void reinforcement_editor_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(reinforcement_editor_dlg)
	DDX_Control(pDX, IDC_DELAY_SPIN, m_delay_spin);
	DDX_Control(pDX, IDC_USES_SPIN, m_uses_spin);
	DDX_Text(pDX, IDC_USES, m_uses);
	DDX_Text(pDX, IDC_DELAY, m_delay);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(reinforcement_editor_dlg, CDialog)
	//{{AFX_MSG_MAP(reinforcement_editor_dlg)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// reinforcement_editor_dlg message handlers

BOOL reinforcement_editor_dlg::OnInitDialog() 
{
	int i;
	CListBox	*box;

	CDialog::OnInitDialog();
	theApp.init_window(&Reinforcement_wnd_data, this);
	box = (CListBox *) GetDlgItem(IDC_LIST);
	for (i=0; i<Num_reinforcements; i++) {
		m_reinforcements[i] = Reinforcements[i];
		box->AddString(Reinforcements[i].name);
	}

	m_num_reinforcements = Num_reinforcements;
	m_uses_spin.SetRange(1, 99);
	m_delay_spin.SetRange(0, 1000);
	update_data();
	if (Num_reinforcements == MAX_REINFORCEMENTS)
		GetDlgItem(IDC_ADD) -> EnableWindow(FALSE);

	return TRUE;
}

void reinforcement_editor_dlg::update_data()
{
	object *objp;
	int enable = TRUE;

	if (cur < 0) {
		m_uses = 0;
		m_delay = 0;
		enable = FALSE;

	} else {
		m_uses = m_reinforcements[cur].uses;
		m_delay = m_reinforcements[cur].arrival_delay;
	}

	UpdateData(FALSE);

	GetDlgItem(IDC_USES)->EnableWindow(enable);
	GetDlgItem(IDC_USES_SPIN)->EnableWindow(enable);
	GetDlgItem(IDC_DELAY)->EnableWindow(enable);
	GetDlgItem(IDC_DELAY_SPIN)->EnableWindow(enable);

	if (cur < 0)
		return;

	// disable the uses entries if the reinforcement is a ship
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->type == OBJ_SHIP) {
			if ( !stricmp( Ships[objp->instance].ship_name, m_reinforcements[cur].name) ) {
				GetDlgItem(IDC_USES)->EnableWindow(FALSE);
				GetDlgItem(IDC_USES_SPIN)->EnableWindow(FALSE);
				break;
			}
		}
		objp = GET_NEXT(objp);
	}
}

void reinforcement_editor_dlg::OnSelchangeList() 
{
	save_data();
	cur = ((CListBox *) GetDlgItem(IDC_LIST))->GetCurSel();
	GetDlgItem(IDC_DELETE) -> EnableWindow(cur != -1);
	update_data();
}

void reinforcement_editor_dlg::save_data()
{
	UpdateData(TRUE);
	UpdateData(TRUE);
	if (cur >= 0) {
		Assert(cur < m_num_reinforcements);
		m_reinforcements[cur].uses = m_uses;
		m_reinforcements[cur].arrival_delay = m_delay;

		// save the message information to the reinforcement structure.  First clear out the string
		// entires in the Reinforcement structure
		memset( m_reinforcements[cur].no_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );
		memset( m_reinforcements[cur].yes_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );
	}
}

int reinforcement_editor_dlg::query_modified()
{
	int i, j;

	save_data();
	if (Num_reinforcements != m_num_reinforcements)
		return 1;

	for (i=0; i<Num_reinforcements; i++) {
		if (stricmp(m_reinforcements[i].name, Reinforcements[i].name))
			return 1;
		if (m_reinforcements[i].uses != Reinforcements[i].uses)
			return 1;
		if ( m_reinforcements[i].arrival_delay != Reinforcements[i].arrival_delay )
			return 1;

		// determine if the message content has changed
		for ( j = 0; j < MAX_REINFORCEMENT_MESSAGES; j++ ) {
			if ( stricmp(m_reinforcements[i].no_messages[j], Reinforcements[i].no_messages[j]) )
				return 1;
		}
		for ( j = 0; j < MAX_REINFORCEMENT_MESSAGES; j++ ) {
			if ( stricmp(m_reinforcements[i].yes_messages[j], Reinforcements[i].yes_messages[j]) )
				return 1;
		}
	}

	return 0;
}

void reinforcement_editor_dlg::OnOK()
{
	int i, j;

	save_data();
	// clear arrival cues for any new reinforcements to the list
	for (i=0; i<m_num_reinforcements; i++) {
		for (j=0; j<Num_reinforcements; j++)
			if (!stricmp(m_reinforcements[i].name, Reinforcements[j].name))
				break;

		if (j == Num_reinforcements) {
			for (j=0; j<MAX_SHIPS; j++)
				if ((Ships[j].objnum != -1) && !stricmp(m_reinforcements[i].name, Ships[j].ship_name)) {
					//free_sexp2(Ships[j].arrival_cue);
					//Ships[j].arrival_cue = Locked_sexp_false;
					break;
				}

			if (j == MAX_SHIPS) {
				for (j=0; j<MAX_WINGS; j++)
					if (Wings[j].wave_count && !stricmp(m_reinforcements[i].name, Wings[j].name)) {
						//free_sexp2(Wings[j].arrival_cue);
						//Wings[j].arrival_cue = Locked_sexp_false;
						break;
					}

				Assert(j < MAX_WINGS);
			}
		}
	}

	if (Num_reinforcements != m_num_reinforcements)
		set_modified();

	Num_reinforcements = m_num_reinforcements;
	for (i=0; i<m_num_reinforcements; i++) {
		if (memcmp((void *) &Reinforcements[i], (void *) &m_reinforcements[i], sizeof(reinforcements)))
			set_modified();

		Reinforcements[i] = m_reinforcements[i];
		set_reinforcement( Reinforcements[i].name, 1 );		// this call should
	}

	Update_ship = Update_wing = 1;
	theApp.record_window_data(&Reinforcement_wnd_data, this);
	CDialog::OnOK();
}

void reinforcement_editor_dlg::OnCancel()
{
	theApp.record_window_data(&Reinforcement_wnd_data, this);
	CDialog::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
// reinforcement_select dialog

reinforcement_select::reinforcement_select(CWnd* pParent /*=NULL*/)
	: CDialog(reinforcement_select::IDD, pParent)
{
	//{{AFX_DATA_INIT(reinforcement_select)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	cur = -1;
}

void reinforcement_select::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(reinforcement_select)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(reinforcement_select, CDialog)
	//{{AFX_MSG_MAP(reinforcement_select)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// reinforcement_select message handlers

BOOL reinforcement_select::OnInitDialog()
{
	int i, z;
	CListBox *box;
	object *ptr;

	CDialog::OnInitDialog();
	box = (CListBox *) GetDlgItem(IDC_LIST);
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_SHIP && Ships[ptr->instance].wingnum < 0) {
			z = box->AddString(Ships[ptr->instance].ship_name);
			box->SetItemData(z, OBJ_INDEX(ptr));
		}

		ptr = GET_NEXT(ptr);
	}

	for (i=0; i<Num_wings; i++) {
		z = box->AddString(Wings[i].name);
		box->SetItemData(z, ID_WING_DATA + i);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void reinforcement_select::OnSelchangeList() 
{
	cur = ((CListBox *) GetDlgItem(IDC_LIST))->GetCurSel();
	GetDlgItem(IDOK)->EnableWindow(cur != -1);
}

void reinforcement_select::OnOK()
{
	cur = ((CListBox *) GetDlgItem(IDC_LIST))->GetCurSel();
	Assert(cur != -1);
	((CListBox *) GetDlgItem(IDC_LIST)) -> GetText(cur, name);
	CDialog::OnOK();
}

void reinforcement_select::OnCancel()
{
	cur = -1;
	CDialog::OnCancel();
}

void reinforcement_editor_dlg::OnAdd()
{
	int i, wing_index;
	reinforcement_select dlg;
	CString name_check;

	dlg.DoModal();
	if (dlg.cur != -1) {
		// if we've run out of reinforcement slots
		if (m_num_reinforcements == MAX_REINFORCEMENTS) {
			MessageBox("Reached limit on reinforcements.  Can't add more!");
			return;
		}

		// if this is a wing, make sure its a valid wing (no mixed ship teams)
		wing_index = wing_lookup((char*)dlg.name);
		if(wing_index >= 0){
			if(wing_has_conflicting_teams(wing_index)){
				MessageBox("Cannot have a reinforcement wing with mixed teams, sucka!");
				return;
			}
		}

		i = m_num_reinforcements++;
		strcpy_s(m_reinforcements[i].name, dlg.name);
		((CListBox *) GetDlgItem(IDC_LIST)) -> AddString(dlg.name);
		m_reinforcements[i].type = 0;
		m_reinforcements[i].uses = 1;
		m_reinforcements[i].arrival_delay = 0;
		memset( m_reinforcements[i].no_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );
		memset( m_reinforcements[i].yes_messages, 0, MAX_REINFORCEMENT_MESSAGES * NAME_LENGTH );
		if (m_num_reinforcements == MAX_REINFORCEMENTS){
			GetDlgItem(IDC_ADD) -> EnableWindow(FALSE);
		}
	}
}

void reinforcement_editor_dlg::OnDelete()
{
	int i;

	if (cur != -1) {
		((CListBox *) GetDlgItem(IDC_LIST)) -> DeleteString(cur);
		for (i=cur; i<m_num_reinforcements-1; i++)
			m_reinforcements[i] = m_reinforcements[i + 1];
	}

	m_num_reinforcements--;
	cur = -1;
	if (!m_reinforcements)
		GetDlgItem(IDC_DELETE) -> EnableWindow(FALSE);
}

void reinforcement_editor_dlg::OnClose() 
{
	int z;

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL)
			return;

		if (z == IDYES) {
			OnOK();
			return;
		}
	}
	
	CDialog::OnClose();
}
