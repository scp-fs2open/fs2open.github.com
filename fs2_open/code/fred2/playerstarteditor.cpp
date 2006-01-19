/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/fred2/PlayerStartEditor.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Player starting point editor dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/12/29 08:21:00  wmcoolmon
 * No my widdle FRED, I didn't forget about you ^_^ (codebase commit)
 *
 * Revision 1.3  2005/09/29 05:18:59  Goober5000
 * the FRED stuff
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:00  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 4     2/23/99 7:03p Dave
 * Rewrote a horribly mangled and evil team loadout dialog. Bugs gone.
 * 
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "PlayerStartEditor.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "Management.h"
#include "weapon/weapon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// player_start_editor dialog

player_start_editor::player_start_editor(CWnd* pParent) : CDialog(player_start_editor::IDD, pParent)
{
	//{{AFX_DATA_INIT(player_start_editor)
	m_delay = 0;	
	m_weapon_pool = 0;
	m_ship_pool = 0;
	//}}AFX_DATA_INIT

	selected_team = 0;
	dlg_inited = 0;
}

void player_start_editor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(player_start_editor)
	DDX_Control(pDX, IDC_POOL_SPIN, m_pool_spin);
	DDX_Control(pDX, IDC_DELAY_SPIN, m_delay_spin);
	DDX_Control(pDX, IDC_SPIN1, m_spin1);
	DDX_Control(pDX, IDC_SHIP_LIST, m_ship_list);
	DDX_Control(pDX, IDC_WEAPON_LIST, m_weapon_list);	
	DDX_Text(pDX, IDC_DELAY, m_delay);	
	DDX_Text(pDX, IDC_SHIP_POOL, m_ship_pool);
	DDX_Text(pDX, IDC_WEAPON_POOL, m_weapon_pool);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(player_start_editor, CDialog)
	//{{AFX_MSG_MAP(player_start_editor)
	ON_WM_INITMENU()
	ON_LBN_SELCHANGE(IDC_SHIP_LIST, OnSelchangeShipList)	
	ON_WM_CLOSE()
	ON_LBN_SELCHANGE(IDC_WEAPON_LIST, OnSelchangeWeaponList)	
	ON_EN_UPDATE(IDC_SHIP_POOL, OnUpdateShipPool)
	ON_EN_UPDATE(IDC_WEAPON_POOL, OnUpdateWeaponPool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// player_start_editor message handlers

BOOL player_start_editor::OnInitDialog() 
{
	int i;
	int idx;	

	// initialize ship pool data
	memset(ship_pool, 0, sizeof(int) * MAX_TVT_TEAMS * MAX_SHIP_CLASSES);
	for(i=0; i<MAX_TVT_TEAMS; i++){
		for(idx=0; idx<Team_data[i].number_choices; idx++){
			ship_pool[i][Team_data[i].ship_list[idx]] = Team_data[i].ship_count[idx];
		}
	}

	// initialize weapon pool data
	memset(weapon_pool, 0, sizeof(int) * MAX_TVT_TEAMS * MAX_WEAPON_TYPES);
	for(i=0; i<MAX_TVT_TEAMS; i++){
		for(idx=0; idx<MAX_WEAPON_TYPES; idx++){
			weapon_pool[i][idx] = Team_data[i].weaponry_pool[idx];
		}
	}

	// entry delay time
	m_delay = f2i(Entry_delay_time);

	// misc window crap
	CDialog::OnInitDialog();
	theApp.init_window(&Player_wnd_data, this);
	m_spin1.SetRange(0, 99);
	m_pool_spin.SetRange(0, 9999);
	m_delay_spin.SetRange(0, 30);	

	// regenerate all the controls
	reset_controls();

	dlg_inited = 1;

	return TRUE;
}

// regenerate all controls
void player_start_editor::reset_controls()
{	
	int i;
	int ct;

	// create a checklistbox for each "player" ship type	
	m_ship_list.ResetContent();
	ct = 0;
	for (i=0; i<Num_ship_classes; i++) {
		if (Ship_info[i].flags & SIF_PLAYER_SHIP) {
			m_ship_list.AddString(Ship_info[i].name);
			
			// if the ship currently has pool entries, check it
			if(ship_pool[selected_team][i] > 0){
				m_ship_list.SetCheck(ct, TRUE);
			} else {
				m_ship_list.SetCheck(ct, FALSE);
			}

			// next
			ct++;
		}
	}

	// create a checklistbox for each weapon ship type	
	m_weapon_list.ResetContent();
	ct = 0;
	for (i=0; i<Num_weapon_types; i++) {
		if (Weapon_info[i].wi_flags & WIF_PLAYER_ALLOWED) {
			m_weapon_list.AddString(Weapon_info[i].name);
			
			// if the ship currently has pool entries, check it
			if(weapon_pool[selected_team][i] > 0){
				m_weapon_list.SetCheck(ct, TRUE);
			} else {
				m_weapon_list.SetCheck(ct, FALSE);
			}

			ct++;
		}
	}	

	// be sure that nothing is selected	
	m_ship_list.SetCurSel(-1);
	m_weapon_list.SetCurSel(-1);
	UpdateData(FALSE);	
}

void player_start_editor::OnInitMenu(CMenu* pMenu)
{
	int i;
	CMenu *m;

	// disable any items we should disable
	m = pMenu->GetSubMenu(0);

	// uncheck all menu items
	for (i = 0; i < Num_teams; i++ ){
		m->CheckMenuItem(i, MF_BYPOSITION | MF_UNCHECKED);
	}

	for ( i = Num_teams; i < MAX_TVT_TEAMS; i++ ){
		m->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
	}

	// put a check next to the currently selected item
	m->CheckMenuItem(selected_team, MF_BYPOSITION | MF_CHECKED);

	CDialog::OnInitMenu(pMenu);
}

// switch between active teams
BOOL player_start_editor::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id;

	// select a team
	id = LOWORD(wParam);
	switch(id){
	case ID_TEAM_1:
		selected_team = 0;
		reset_controls();
		break;

	case ID_TEAM_2:
		selected_team = 1;
		reset_controls();
		break;	
	}	
	
	// low level stuff
	return CDialog::OnCommand(wParam, lParam);
}

// ship list changed
void player_start_editor::OnSelchangeShipList() 
{			
	int selected;
	int si_index;
	char ship_name[255] = "";

	// determine if we've selected something
	selected = m_ship_list.GetCurSel();	
	if (selected != -1) {
		// lookup the ship
		m_ship_list.GetText(m_ship_list.GetCurSel(), ship_name);
		si_index = ship_info_lookup(ship_name);

		// if we have a valid ship type
		if(si_index >= 0){
			// if this item is checked
			if(m_ship_list.GetCheck(selected)) {
				if(ship_pool[selected_team][si_index] <= 0){
					ship_pool[selected_team][si_index] = 5;
					m_ship_pool = 5;
				} else {
					m_ship_pool = ship_pool[selected_team][si_index];
				}
			} 
			// otherwise zero the count
			else {
				ship_pool[selected_team][si_index] = 0;
				m_ship_pool = 0;
			}		
		} else {
			Int3();
		}
	}
		
	// update shtuff
	UpdateData(FALSE);
}

// weapon list changed
void player_start_editor::OnSelchangeWeaponList() 
{
	int selected;
	int wi_index;
	char weapon_name[255] = "";

	// determine if we've selected something
	selected = m_weapon_list.GetCurSel();	
	if (selected != -1) {
		// lookup the weapon
		m_weapon_list.GetText(m_weapon_list.GetCurSel(), weapon_name);
		wi_index = weapon_name_lookup(weapon_name);

		// if we have a valid weapon type
		if(wi_index >= 0){
			// if this item is checked
			if(m_weapon_list.GetCheck(selected)) {
				if(weapon_pool[selected_team][wi_index] <= 0){
					weapon_pool[selected_team][wi_index] = 100;
					m_weapon_pool = 100;
				} else {
					m_weapon_pool = weapon_pool[selected_team][wi_index];
				}
			} 
			// otherwise zero the count
			else {
				weapon_pool[selected_team][wi_index] = 0;
				m_weapon_pool = 0;
			}		
		} else {
			Int3();
		}
	}
		
	// update shtuff
	UpdateData(FALSE);
}

// cancel
void player_start_editor::OnCancel()
{
	theApp.record_window_data(&Player_wnd_data, this);
	CDialog::OnCancel();
}

// ok
void player_start_editor::OnOK()
{
	int i, idx;

	// store player entry time delay
	Entry_delay_time = i2f(m_delay);	

	// store ship pools	
	for(i=0; i<MAX_TVT_TEAMS; i++){
		Team_data[i].number_choices = 0;
		for(idx=0; idx<Num_ship_classes; idx++){
			// if we have ships here
			if(ship_pool[i][idx] > 0){
				Team_data[i].ship_count[Team_data[i].number_choices] = ship_pool[i][idx];
				Team_data[i].ship_list[Team_data[i].number_choices++] = idx;
			}
		}
	}

	// store weapon pools
	for(i=0; i<MAX_TVT_TEAMS; i++){		
		for(idx=0; idx<Num_weapon_types; idx++){
			// if we have weapons here
			Team_data[i].weaponry_pool[idx] = weapon_pool[i][idx];
		}
	}

	theApp.record_window_data(&Player_wnd_data, this);
	CDialog::OnOK();
}

// ship pool count change
void player_start_editor::OnUpdateShipPool() 
{
	int selected, si_index;
	char ship_name[255] = "";

	if (!dlg_inited){
		return;
	}

	UpdateData(TRUE);	
	
	// if we have a ship selected and checked, update the pool	
	selected = m_ship_list.GetCurSel();	
	if((selected != -1) && m_ship_list.GetCheck(selected)){
		// lookup the ship
		m_ship_list.GetText(m_ship_list.GetCurSel(), ship_name);
		si_index = ship_info_lookup(ship_name);

		// if we have a valid ship type
		if(si_index >= 0){
			ship_pool[selected_team][si_index] = m_ship_pool;
		}
	};
}

// weapon pool count change
void player_start_editor::OnUpdateWeaponPool() 
{
	int selected, wi_index;
	char weapon_name[255] = "";

	if (!dlg_inited){
		return;
	}

	UpdateData(TRUE);	
	
	// if we have a ship selected and checked, update the pool	
	selected = m_weapon_list.GetCurSel();	
	if((selected != -1) && m_weapon_list.GetCheck(selected)){
		// lookup the ship
		m_weapon_list.GetText(m_weapon_list.GetCurSel(), weapon_name);
		wi_index = weapon_info_lookup(weapon_name);

		// if we have a valid ship type
		if(wi_index >= 0){
			weapon_pool[selected_team][wi_index] = m_weapon_pool;
		}
	};
}
