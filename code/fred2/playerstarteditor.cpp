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
#include "FREDDoc.h"
#include "PlayerStartEditor.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "Management.h"
#include "weapon/weapon.h"

#ifdef _DEBUG
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
	autobalance = false; 
	previous_team = -1;
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
	DDX_Control(pDX, IDC_SHIP_VARIABLES_LIST, m_ship_variable_list);
	DDX_Control(pDX, IDC_WEAPON_VARIABLES_LIST, m_weapon_variable_list);
	DDX_Control(pDX, IDC_SHIP_VARIABLES_COMBO, m_ship_quantity_variable);	
	DDX_Control(pDX, IDC_WEAPON_VARIABLES_COMBO, m_weapon_quantity_variable);
	DDX_Control(pDX, IDC_WINGS_SHP_COUNT, m_ships_used_in_wings);
	DDX_Control(pDX, IDC_WINGS_WPN_COUNT, m_weapons_used_in_wings);				
	DDX_Text(pDX, IDC_DELAY, m_delay);	
	DDX_Text(pDX, IDC_SHIP_POOL, m_ship_pool);
	DDX_Text(pDX, IDC_WEAPON_POOL, m_weapon_pool);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(player_start_editor, CDialog)
	//{{AFX_MSG_MAP(player_start_editor)
	ON_WM_INITMENU()
	ON_LBN_SELCHANGE(IDC_SHIP_LIST, OnSelchangeShipList)	
	ON_CLBN_CHKCHANGE(IDC_SHIP_LIST, OnSelchangeShipList)
	ON_LBN_SELCHANGE(IDC_WEAPON_LIST, OnSelchangeWeaponList)	
	ON_CLBN_CHKCHANGE(IDC_SHIP_LIST, OnSelchangeWeaponList)
	ON_EN_UPDATE(IDC_SHIP_POOL, OnUpdateShipPool)
	ON_EN_UPDATE(IDC_WEAPON_POOL, OnUpdateWeaponPool)
	ON_LBN_SELCHANGE(IDC_SHIP_VARIABLES_LIST, OnSelchangeShipVariablesList)
	ON_LBN_SELCHANGE(IDC_WEAPON_VARIABLES_LIST, OnSelchangeWeaponVariablesList)
	ON_CBN_SELCHANGE(IDC_SHIP_VARIABLES_COMBO, OnSelchangeShipVariablesCombo)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_WEAPON_VARIABLES_COMBO, OnSelchangeWeaponVariablesCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// player_start_editor message handlers


BOOL player_start_editor::OnInitDialog() 
{
	int i, j;
	int idx;	

	// initialize ship pool data
	memset(static_ship_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SHIP_CLASSES);
	memset(dynamic_ship_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SEXP_VARIABLES);
	memset(static_ship_variable_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SHIP_CLASSES);
	memset(dynamic_ship_variable_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SEXP_VARIABLES);
	for(i=0; i<MAX_TVT_TEAMS; i++){
		for(idx=0; idx<Team_data[i].num_ship_choices; idx++)
		{
			// do we have a variable for this entry? if we don't....
			if (!strlen(Team_data[i].ship_list_variables[idx]))
			{
				// This pool is set to hold the number of ships available at an index corresponding to the Ship_info array.
				static_ship_pool[i][Team_data[i].ship_list[idx]] = Team_data[i].ship_count[idx];
				// This pool is set to hold whether a ship at a Ship_info index has been set by a variable (and the 
				// variables index in Sexp_variables) if it has).
				if (strlen(Team_data[i].ship_count_variables[idx])) {
					static_ship_variable_pool[i][Team_data[i].ship_list[idx]] = get_index_sexp_variable_name(Team_data[i].ship_count_variables[idx]);
				}
				else {
					static_ship_variable_pool[i][Team_data[i].ship_list[idx]] = -1;
				}
			}
			// if we do....
			else
			{
				// This pool is set to hold the number of ships available at an index corresponding to the Sexp_variables array
				dynamic_ship_pool[i][get_index_sexp_variable_name(Team_data[i].ship_list_variables[idx])] = Team_data[i].ship_count[idx];
				// This pool is set to hold whether a ship at a Ship_info index has been set by a variable (and the 
				// variables index in Sexp_variables) if it has).
				if (strlen(Team_data[i].ship_count_variables[idx])) {
					dynamic_ship_variable_pool[i][get_index_sexp_variable_name(Team_data[i].ship_list_variables[idx])] = get_index_sexp_variable_name(Team_data[i].ship_count_variables[idx]);
				}
				else {
					dynamic_ship_variable_pool[i][get_index_sexp_variable_name(Team_data[i].ship_list_variables[idx])] = -1;
				}
			}
		}
	}

	
	// initialize weapon pool data
	memset(static_weapon_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_WEAPON_TYPES);
	memset(dynamic_weapon_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SEXP_VARIABLES);
	memset(static_weapon_variable_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_WEAPON_TYPES);
	memset(dynamic_weapon_variable_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SEXP_VARIABLES);
	for(i=0; i<MAX_TVT_TEAMS; i++){
		for(idx=0; idx<Team_data[i].num_weapon_choices; idx++)
		{
			// do we have a variable for this entry?
			if (!strlen(Team_data[i].weaponry_pool_variable[idx]))
			{
				static_weapon_pool[i][Team_data[i].weaponry_pool[idx]] = Team_data[i].weaponry_count[idx];
				if (strlen(Team_data[i].weaponry_amount_variable[idx])) {
					static_weapon_variable_pool[i][Team_data[i].weaponry_pool[idx]] = get_index_sexp_variable_name(Team_data[i].weaponry_amount_variable[idx]);
				}
				else {
					static_weapon_variable_pool[i][Team_data[i].weaponry_pool[idx]] = -1;
				}
			}
			// if we do....
			else
			{
				dynamic_weapon_pool[i][get_index_sexp_variable_name(Team_data[i].weaponry_pool_variable[idx])] = Team_data[i].weaponry_count[idx];
				if (strlen(Team_data[i].weaponry_amount_variable[idx])) {
					dynamic_weapon_variable_pool[i][get_index_sexp_variable_name(Team_data[i].weaponry_pool_variable[idx])] = get_index_sexp_variable_name(Team_data[i].weaponry_amount_variable[idx]);
				}
				else {
					dynamic_weapon_variable_pool[i][get_index_sexp_variable_name(Team_data[i].weaponry_pool_variable[idx])] =  -1;
				}
			}
		}
	}

	// initialise the ship and weapon usage list
	memset(ship_usage, 0, sizeof(int) * MAX_TVT_TEAMS * MAX_SHIP_CLASSES);
	memset(weapon_usage, 0, sizeof(int) * MAX_TVT_TEAMS * MAX_WEAPON_TYPES);


	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) { 
		for (i=0; i<MAX_TVT_TEAMS; i++) {
			for (j=0; j<MAX_TVT_WINGS_PER_TEAM; j++) {
				generate_ship_usage_list(ship_usage[i], TVT_wings[(i*MAX_TVT_WINGS_PER_TEAM) + j]);
				generate_weaponry_usage_list(weapon_usage[i], TVT_wings[(i*MAX_TVT_WINGS_PER_TEAM) + j]);
			}			
		}
	}
	else {
		for (i=0; i<MAX_STARTING_WINGS; i++) {
			generate_ship_usage_list(ship_usage[0], Starting_wings[i]);
			generate_weaponry_usage_list(weapon_usage[0], Starting_wings[i]);
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
	char buff[TOKEN_LENGTH + TOKEN_LENGTH + 2];  // VariableName[VariableValue]

	if (autobalance && (previous_team != -1) && (previous_team != selected_team)) {
		// copy across all the ship stuff
		for (i=0; i<MAX_SHIP_CLASSES; i++) {
			static_ship_pool[selected_team][i] = static_ship_pool[previous_team][i]; 
			static_ship_variable_pool[selected_team][i] = static_ship_variable_pool[previous_team][i]; 
		}

		// copy across weapon stuff
		for (i=0; i<MAX_WEAPON_TYPES; i++) {
			static_weapon_pool[selected_team][i] = static_weapon_pool[previous_team][i]; 
			static_weapon_variable_pool[selected_team][i] = static_weapon_variable_pool[previous_team][i]; 
		}
			
		// copy across variable stuff
		for (i=0; i<MAX_SEXP_VARIABLES; i++) {
			dynamic_ship_pool[selected_team][i] = dynamic_ship_pool[previous_team][i];
			dynamic_ship_variable_pool[selected_team][i] = dynamic_ship_variable_pool[previous_team][i];
			dynamic_weapon_pool[selected_team][i] = dynamic_weapon_pool[previous_team][i];
			dynamic_weapon_variable_pool[selected_team][i] = dynamic_weapon_variable_pool[previous_team][i];
		}
	}
	
	m_ship_variable_list.ResetContent();
	m_ship_quantity_variable.ResetContent();

	m_weapon_variable_list.ResetContent();
	m_weapon_quantity_variable.ResetContent();

	// Add the default entry to both variable quantity ComboBoxes
	m_ship_quantity_variable.AddString("Don't Use Variables");
	m_weapon_quantity_variable.AddString("Don't Use Variables");

	int current_entry = 0;
	int num_sexp_variables = sexp_variable_count();	
	for (i=0; i < num_sexp_variables; i++)
	{
		if (Sexp_variables[i].type & SEXP_VARIABLE_STRING) 
		{
			sprintf(buff, "%s [%s]", Sexp_variables[i].variable_name, Sexp_variables[i].text);
			m_ship_variable_list.AddString(buff);
			m_weapon_variable_list.AddString(buff);

			// Now we set the checkbox for ships
			if((dynamic_ship_pool[selected_team][i] > 0) ||
			   (dynamic_ship_variable_pool[selected_team][i] != -1))
			{
				m_ship_variable_list.SetCheck(current_entry, TRUE);
			}
			else 
			{
				m_ship_variable_list.SetCheck(current_entry, FALSE);
			}

			// and now for weapons
			if((dynamic_weapon_pool[selected_team][i] > 0) ||
			   (dynamic_weapon_variable_pool[selected_team][i] != -1))
			{
				m_weapon_variable_list.SetCheck(current_entry, TRUE);
			}
			else 
			{
				m_weapon_variable_list.SetCheck(current_entry, FALSE);
			}

			current_entry++;
		}

		// Since we're looping through Sexp_variables amyway we might as well fill the number variable combo boxes
		else if (Sexp_variables[i].type & SEXP_VARIABLE_NUMBER)
		{
			m_ship_quantity_variable.AddString(Sexp_variables[i].variable_name); 
			m_weapon_quantity_variable.AddString(Sexp_variables[i].variable_name); 
		}
	}

	// create a checklistbox for each "player" ship type	
	m_ship_list.ResetContent();
	ct = 0;
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		if (it->flags & SIF_PLAYER_SHIP) {
			i = std::distance(Ship_info.cbegin(), it);
			m_ship_list.AddString(it->name);
			
			// if the ship currently has pool entries or was set by a variable, check it
			if((static_ship_pool[selected_team][i] > 0) || (static_ship_variable_pool[selected_team][i] != -1)) {
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
			
			// if the ship currently has pool entries or was set by a variable, check it
			if((static_weapon_pool[selected_team][i] > 0) || (static_weapon_variable_pool[selected_team][i] != -1)){
				m_weapon_list.SetCheck(ct, TRUE);
			} else {
				m_weapon_list.SetCheck(ct, FALSE);
			}

			ct++;
		} else if (static_weapon_pool[selected_team][i] > 0 || (static_weapon_variable_pool[selected_team][i] != -1)) {
			// not sure if this should be a verbal warning or not, so I'm adding both and making it verbal for now
			Warning(LOCATION, "Weapon '%s' in weapon pool isn't allowed on player loadout!  Resetting count to 0...\n", Weapon_info[i].name);
			static_weapon_pool[selected_team][i] = 0;
			static_weapon_variable_pool[selected_team][i] = -1;
		}
	}	

	// be sure that nothing is selected	
	m_ship_list.SetCurSel(-1);
	m_ship_variable_list.SetCurSel(-1);
	m_ship_quantity_variable.SetCurSel(-1);
	m_weapon_list.SetCurSel(-1);
	m_weapon_variable_list.SetCurSel(-1);
	m_weapon_quantity_variable.SetCurSel(-1);

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


	m = pMenu->GetSubMenu(1); 
	if (Num_teams > 1) {
		m->CheckMenuItem(ID_AUTOBALANCE, autobalance ? MF_CHECKED : MF_UNCHECKED); 
	}
	else {
		m->EnableMenuItem(ID_AUTOBALANCE, MF_GRAYED); 
	}
	
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
		previous_team = selected_team; 
		selected_team = 0;
		reset_controls();
		break;

	case ID_TEAM_2:
		previous_team = selected_team; 
		selected_team = 1;
		reset_controls();
		break;	

	case ID_AUTOBALANCE:
		autobalance = !autobalance; 
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
	char ship_usage_buff[10];

	// If the ship list is selected the variable ship list should be deselected
	m_ship_variable_list.SetCurSel(-1);

	// determine if we've selected something
	selected = m_ship_list.GetCurSel();	
	if (selected != -1) {
		// lookup the ship
		m_ship_list.GetText(selected, ship_name);
		si_index = ship_info_lookup(ship_name);

		// if we have a valid ship type
		if(si_index >= 0){
			// if this item is checked
			if(m_ship_list.GetCheck(selected)) {								
				if (static_ship_variable_pool[selected_team][si_index] == -1) {
					if (static_ship_pool[selected_team][si_index] <= 0){
						static_ship_pool[selected_team][si_index] = 5;
					}
					m_ship_pool = static_ship_pool[selected_team][si_index];
					// Set the ship variable ComboBox to reflect that we are not using variables for this ship
					m_ship_quantity_variable.SetCurSel(0); 	
				}
				// If the number of ships was set by a variable
				else {
					Assert (Sexp_variables[static_ship_variable_pool[selected_team][si_index]].type & SEXP_VARIABLE_NUMBER);

					m_ship_pool = atoi(Sexp_variables[static_ship_variable_pool[selected_team][si_index]].text);
					int selected_variable = sexp_variable_typed_count(static_ship_variable_pool[selected_team][si_index], SEXP_VARIABLE_NUMBER);
					m_ship_quantity_variable.SetCurSel(selected_variable + 1);
				}
			} 
			// otherwise zero the count
			else {
				static_ship_pool[selected_team][si_index] = 0;
				static_ship_variable_pool[selected_team][si_index] = -1;
				m_ship_pool = 0;
				m_ship_quantity_variable.SetCurSel(0);
			}
		
			// set the number used in wings
			sprintf(ship_usage_buff, "%d", ship_usage[selected_team][si_index]); 
			m_ships_used_in_wings.SetWindowText(ship_usage_buff); 
	
		} else {
			Int3();
		}
	}
		
	// update stuff
	UpdateData(FALSE);
}

// ship variable list changed
void player_start_editor::OnSelchangeShipVariablesList() 
{
	int selection; 

	// If the variable list is selected the ship list should be deselected
	m_ship_list.SetCurSel(-1);

	//Have we selected something?
	selection = m_ship_variable_list.GetCurSel(); 
	if (selection != -1) {
		int sexp_index = get_nth_variable_index(selection+1, SEXP_VARIABLE_STRING);

		if (sexp_index > -1) {
			Assert(selection == sexp_variable_typed_count(sexp_index, SEXP_VARIABLE_STRING));

			// Is this item checked? 
			if (m_ship_variable_list.GetCheck(selection)) {
				if (dynamic_ship_variable_pool[selected_team][sexp_index] == -1) {
					if (dynamic_ship_pool[selected_team][sexp_index] <= 0) {
						dynamic_ship_pool[selected_team][sexp_index] = 5;
					}
					m_ship_pool = dynamic_ship_pool[selected_team][sexp_index];
					m_ship_quantity_variable.SetCurSel(0); 						
				}
				else {
					Assert (Sexp_variables[dynamic_ship_variable_pool[selected_team][sexp_index]].type & SEXP_VARIABLE_NUMBER);
					m_ship_pool = atoi(Sexp_variables[dynamic_ship_variable_pool[selected_team][sexp_index]].text);
					int selected_variable = sexp_variable_typed_count(dynamic_ship_variable_pool[selected_team][sexp_index], SEXP_VARIABLE_NUMBER);
					m_ship_quantity_variable.SetCurSel(selected_variable + 1);
				}
			}
			// We've unselected the tickbox, reset everything
			else {
				dynamic_ship_pool[selected_team][sexp_index] = -1;
				dynamic_ship_variable_pool[selected_team][sexp_index] = -1;
				m_ship_pool = 0;
				m_ship_quantity_variable.SetCurSel(0); 	
			}

			// It might be nice to have FRED work out if any ships of the class represented by the variable are in the wings
			// but for now just set it to zero
			m_ships_used_in_wings.SetWindowText("0"); 
		}
		else {
			Int3();
		}

	}

	// update stuff
	UpdateData(FALSE);
}

void player_start_editor::OnSelchangeShipVariablesCombo() 
{
	// Get the new selection
	char variable_name[TOKEN_LENGTH]; 
	bool update_static_pool = false; 
	bool update_dynamic_pool = false; 

	m_ship_quantity_variable.GetLBText(m_ship_quantity_variable.GetCurSel(), variable_name);
	int sexp_index = get_index_sexp_variable_name(variable_name);

	Assert ((sexp_index > -1) || (!strcmp("Don't Use Variables", variable_name))); 

	// See if the ship_list was selected
	int ship_index = GetSelectedShipListIndex(); 
	if (ship_index >= 0) {
		static_ship_variable_pool[selected_team][ship_index] = sexp_index; 
		update_static_pool = true; 
	}
	
	// Maybe it's the ship_variables_list that is actually selected
	int ship_variable_index = GetSelectedShipVariableListIndex();
	if (ship_variable_index >= 0 ) {
		dynamic_ship_variable_pool[selected_team][ship_variable_index] = sexp_index;
		update_dynamic_pool = true; 
	}

	// Somethings gone wrong if they're both marked as true
	Assert (! (update_static_pool && update_dynamic_pool));

	// Update the ship_pool
	if (update_static_pool || update_dynamic_pool) {
		int new_quantity = 5;
		if (sexp_index > -1) {
			Assert (Sexp_variables[sexp_index].type & SEXP_VARIABLE_NUMBER); 
			new_quantity = atoi(Sexp_variables[sexp_index].text); 
		}

		if (update_static_pool)	{
			static_ship_pool[selected_team][ship_index] = new_quantity;
		}
		else {
			dynamic_ship_pool[selected_team][ship_variable_index] = new_quantity;
		}

		m_ship_pool = new_quantity;
	}	
	
	// update stuff
	UpdateData(FALSE);
}

// weapon list changed
void player_start_editor::OnSelchangeWeaponList() 
{
	int selected;
	int wi_index;
	char weapon_name[255] = "";
	char weapon_usage_buff[10];

	// If the weapon list is selected the variable weapon list shouldn't be
	m_weapon_variable_list.SetCurSel(-1);

	// determine if we've selected something
	selected = m_weapon_list.GetCurSel();	
	if (selected != -1) {
		// lookup the weapon
		m_weapon_list.GetText(selected, weapon_name);
		wi_index = weapon_info_lookup(weapon_name);

		// if we have a valid ship type
		if(wi_index >= 0){
			// if this item is checked
			if(m_weapon_list.GetCheck(selected)) {								
				if (static_weapon_variable_pool[selected_team][wi_index] == -1) {
					if (static_weapon_pool[selected_team][wi_index] <= 0){
						static_weapon_pool[selected_team][wi_index] = 100;
					}
					m_weapon_pool = static_weapon_pool[selected_team][wi_index];
					// Set the combo reflect that we are not using variables for this weapon
					m_weapon_quantity_variable.SetCurSel(0); 	
				}
				// If the number of ships was set by a variable
				else {
					Assert (Sexp_variables[static_weapon_variable_pool[selected_team][wi_index]].type & SEXP_VARIABLE_NUMBER);

					m_weapon_pool = atoi(Sexp_variables[static_weapon_variable_pool[selected_team][wi_index]].text);
					int selected_variable = sexp_variable_typed_count(static_weapon_variable_pool[selected_team][wi_index], SEXP_VARIABLE_NUMBER);
					m_weapon_quantity_variable.SetCurSel(selected_variable + 1);
				}
			} 
			// otherwise zero the count
			else {
				static_weapon_pool[selected_team][wi_index] = 0;				
				static_weapon_variable_pool[selected_team][wi_index] = -1;
				m_weapon_pool = 0;
				m_weapon_quantity_variable.SetCurSel(0); 	
			}
		
			// set the number used in wings
			sprintf(weapon_usage_buff, "%d", weapon_usage[selected_team][wi_index]); 
			m_weapons_used_in_wings.SetWindowText(weapon_usage_buff); 
	
		} else {
			Int3();
		}
	}
		
	// update stuff
	UpdateData(FALSE);
}

void player_start_editor::OnSelchangeWeaponVariablesList() 
{
	int selection; 

	// deselect the other list
	m_weapon_list.SetCurSel(-1);

	//Have we selected something?
	selection = m_weapon_variable_list.GetCurSel(); 
	if (selection != -1) {
		int sexp_index = get_nth_variable_index(selection+1, SEXP_VARIABLE_STRING);

		if (sexp_index > -1) {
			Assert(selection == sexp_variable_typed_count(sexp_index, SEXP_VARIABLE_STRING));

			// Is this item checked? 
			if (m_weapon_variable_list.GetCheck(selection)) {
				if (dynamic_weapon_variable_pool[selected_team][sexp_index] == -1) {
					if (dynamic_weapon_pool[selected_team][sexp_index] <= 0) {
						dynamic_weapon_pool[selected_team][sexp_index] = 5;
					}
					m_weapon_pool = dynamic_weapon_pool[selected_team][sexp_index];
					m_weapon_quantity_variable.SetCurSel(0); 						
				}
				else {
					Assert (Sexp_variables[dynamic_weapon_variable_pool[selected_team][sexp_index]].type & SEXP_VARIABLE_NUMBER);
					m_weapon_pool = atoi(Sexp_variables[dynamic_weapon_variable_pool[selected_team][sexp_index]].text);
					int selected_variable = sexp_variable_typed_count(dynamic_weapon_variable_pool[selected_team][sexp_index], SEXP_VARIABLE_NUMBER);
					m_weapon_quantity_variable.SetCurSel(selected_variable + 1);
				}
			}
			// We've unselected the tickbox, reset everything
			else {
				dynamic_weapon_pool[selected_team][sexp_index] = -1;
				dynamic_weapon_variable_pool[selected_team][sexp_index] = -1;
				m_weapon_pool = 0;
				m_weapon_quantity_variable.SetCurSel(0); 	
			}

			// It might be nice to have FRED work out if any weapons of this type are in the wings
			// but for now just set it to zero
			m_weapons_used_in_wings.SetWindowText("0"); 
		}
		else {
			Int3();
		}
	}

	// update stuff
	UpdateData(FALSE);
}

void player_start_editor::OnSelchangeWeaponVariablesCombo() 
{
	// Get the new selection
	char variable_name[TOKEN_LENGTH]; 
	bool update_static_pool = false; 
	bool update_dynamic_pool = false; 

	m_weapon_quantity_variable.GetLBText(m_weapon_quantity_variable.GetCurSel(), variable_name);
	int sexp_index = get_index_sexp_variable_name(variable_name);

	Assert ((sexp_index > -1) || (!strcmp("Don't Use Variables", variable_name))); 

	// See if the weapon_list was selected
	int weapon_index = GetSelectedWeaponListIndex(); 
	if (weapon_index >= 0) {
		static_weapon_variable_pool[selected_team][weapon_index] = sexp_index; 
		update_static_pool = true; 
	}
	
	// Maybe it's the weapon_variables_list that is actually selected
	int weapon_variable_index = GetSelectedWeaponVariableListIndex();
	if (weapon_variable_index >= 0 ) {
		dynamic_weapon_variable_pool[selected_team][weapon_variable_index] = sexp_index;
		update_dynamic_pool = true; 
	}

	// Somethings gone wrong if they're both marked as true
	Assert (! (update_static_pool && update_dynamic_pool));

	// Update the weapon_pool
	if (update_static_pool || update_dynamic_pool) {
		int new_quantity = 100;
		if (sexp_index > -1) {
			Assert (Sexp_variables[sexp_index].type & SEXP_VARIABLE_NUMBER); 
			new_quantity = atoi(Sexp_variables[sexp_index].text); 
		}

		if (update_static_pool)	{
			static_weapon_pool[selected_team][weapon_index] = new_quantity;
		}
		else {
			dynamic_weapon_pool[selected_team][weapon_variable_index] = new_quantity;
		}

		m_weapon_pool = new_quantity;
	}	
	
	// update stuff
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
	int num_choices; 
	
	int num_sexp_variables = sexp_variable_count();	

	// store player entry time delay
	Entry_delay_time = i2f(m_delay);	

	// store ship pools	
	for(i=0; i<MAX_TVT_TEAMS; i++) {
		num_choices = 0; 
		// First look through the variables list and write out anything there
		for (idx=0; idx < num_sexp_variables; idx++) 		{
			// As soon as we come across a sexp_variable we are using
			if (dynamic_ship_pool[i][idx] != -1) 			{
				Assert (Sexp_variables[idx].type & SEXP_VARIABLE_STRING); 
				int ship_class = ship_info_lookup(Sexp_variables[idx].text);
				
				// If the variable doesn't actually contain a valid ship class name. Warn the user and skip to the next one
				if (ship_class < 0)  {
					char buffer[256];
					sprintf(buffer, 
							"Sexp Variable %s holds the value %s. This is not a valid ship class. Skipping this entry",
							Sexp_variables[idx].variable_name,
							Sexp_variables[idx].text
							);
					MessageBox(buffer);
					continue;
				}
				
				/* Can we can prevent the user from having to enter all his variables again just cause he can't spull gud?
				// If the variable doesn't actually contain a valid ship class name. Warn the user
				if (ship_class < 0)
				{
					char buffer[256];
					sprintf(buffer, 
							"Sexp Variable %s holds the value %s. This is not a valid ship class. You should change this!",
							Sexp_variables[idx].variable_name,
							Sexp_variables[idx].text
							);
					MessageBox(buffer);
					ship_class = ship_info_lookup(default_player_ship);
					if (ship_class < 0 ) {
						sprintf(buffer, "No default ship is set either. Skipping variable %s!", Sexp_variables[idx].variable_name); 
					}
				}*/


				// Copy the variable to Team_data
				if (idx != -1) {
					Assert (idx < MAX_SEXP_VARIABLES);
					strcpy_s(Team_data[i].ship_list_variables[num_choices], Sexp_variables[idx].variable_name);
				}
				else {
					strcpy_s(Team_data[i].ship_list_variables[num_choices], ""); 
				}
				Team_data[i].ship_list[num_choices] = -1;

				// Now we need to set the number of this type available
				if (dynamic_ship_variable_pool[i][idx] == -1) {
					Team_data[i].ship_count[num_choices] = dynamic_ship_pool[i][idx];
					strcpy_s(Team_data[i].ship_count_variables[num_choices], ""); 
				}
				else {
					Assert (Sexp_variables[dynamic_ship_variable_pool[i][idx]].type & SEXP_VARIABLE_NUMBER);

					strcpy_s(Team_data[i].ship_count_variables[num_choices], Sexp_variables[dynamic_ship_variable_pool[i][idx]].variable_name);
					Team_data[i].ship_count[num_choices] = atoi(Sexp_variables[dynamic_ship_variable_pool[i][idx]].text);
				}

				num_choices++;
			}
		}

		// Now we deal with the loadout ships that are statically assigned by class

		for (idx = 0; idx < static_cast<int>(Ship_info.size()); idx++) {
			// if we have ships here
			if(static_ship_pool[i][idx] > 0 || static_ship_variable_pool[i][idx] > -1) {
				Team_data[i].ship_list[num_choices] = idx;
				strcpy_s(Team_data[i].ship_list_variables[num_choices], "");

				// Now set the number of this class available
				if (static_ship_variable_pool[i][idx] == -1) {
					Team_data[i].ship_count[num_choices] = static_ship_pool[i][idx];
					strcpy_s(Team_data[i].ship_count_variables[num_choices], "");
				}
				else {
					Assert (Sexp_variables[static_ship_variable_pool[i][idx]].type & SEXP_VARIABLE_NUMBER);
					
					strcpy_s(Team_data[i].ship_count_variables[num_choices], Sexp_variables[static_ship_variable_pool[i][idx]].variable_name);
					Team_data[i].ship_count[num_choices] = atoi(Sexp_variables[static_ship_variable_pool[i][idx]].text);
				}

				num_choices++;
			}
		}
		Team_data[i].num_ship_choices = num_choices; 
	}

	// store weapon pools
	for(i=0; i<MAX_TVT_TEAMS; i++){		
		num_choices = 0; 

		// First look through the variables list and write out anything there
		for (idx=0; idx < num_sexp_variables; idx++) {
			// As soon as we come across a sexp_variable we are using
			if (dynamic_weapon_pool[i][idx] != -1) {
				Assert (Sexp_variables[idx].type & SEXP_VARIABLE_STRING); 
				int weapon_class = weapon_info_lookup(Sexp_variables[idx].text);
				
				// If the variable doesn't actually contain a valid ship class name. Warn the user and skip to the next one
				if (weapon_class < 0)
				{
					char buffer[256];
					sprintf(buffer, 
							"Sexp Variable %s holds the value %s. This is not a valid weapon class. Skipping this entry",
							Sexp_variables[idx].variable_name,
							Sexp_variables[idx].text
							);
					MessageBox(buffer);
					continue;
				}
				
				// Copy the variable to Team_data
				strcpy_s(Team_data[i].weaponry_pool_variable[num_choices], Sexp_variables[idx].variable_name);
				Team_data[i].weaponry_pool[num_choices] = -1;

				// Now we need to set the number of this class available
				if (dynamic_weapon_variable_pool[i][idx] == -1)
				{
					Team_data[i].weaponry_count[num_choices] = dynamic_weapon_pool[i][idx];
					strcpy_s(Team_data[i].weaponry_amount_variable[num_choices], ""); 
				}
				else 
				{
					Assert (Sexp_variables[dynamic_weapon_variable_pool[i][idx]].type & SEXP_VARIABLE_NUMBER);

					strcpy_s(Team_data[i].weaponry_amount_variable[num_choices], Sexp_variables[dynamic_weapon_variable_pool[i][idx]].variable_name);
					Team_data[i].weaponry_count[num_choices] = atoi(Sexp_variables[dynamic_weapon_variable_pool[i][idx]].text);
				}

				num_choices++;
			}
		}

		// Now we deal with the loadout weapons that are statically assigned by class

		for(idx=0; idx<Num_weapon_types; idx++)
		{
			// if we have weapons here
			if(static_weapon_pool[i][idx] > 0 || static_weapon_variable_pool[i][idx] > -1)
			{
				Team_data[i].weaponry_pool[num_choices] = idx;
				strcpy_s(Team_data[i].weaponry_pool_variable[num_choices], "");

				// Now set the number of this class available
				if (static_weapon_variable_pool[i][idx] == -1)
				{
					Team_data[i].weaponry_count[num_choices] = static_weapon_pool[i][idx];
					strcpy_s(Team_data[i].weaponry_amount_variable[num_choices], "");
				}
				else 
				{
					Assert (Sexp_variables[static_weapon_variable_pool[i][idx]].type & SEXP_VARIABLE_NUMBER);
					
					strcpy_s(Team_data[i].weaponry_amount_variable[num_choices], Sexp_variables[static_weapon_variable_pool[i][idx]].variable_name);
					Team_data[i].weaponry_count[num_choices] = atoi(Sexp_variables[static_weapon_variable_pool[i][idx]].text);
				}

				num_choices++;
			}
		}
		Team_data[i].num_weapon_choices = num_choices; 
	}

	theApp.record_window_data(&Player_wnd_data, this);
	CDialog::OnOK();
}

// Returns the ship_info index of the selected and checked ship_list item or -1 if nothing is checked or 
// the ship is invalid
int player_start_editor::GetSelectedShipListIndex()
{
	char name[255] = "";
	int selected = m_ship_list.GetCurSel();	
	if((selected != -1) && m_ship_list.GetCheck(selected))
	{
		// lookup the ship
		m_ship_list.GetText(m_ship_list.GetCurSel(), name);
		int ship_index = ship_info_lookup(name);
		return ship_index;
	}
	return -1; 
}

// Returns the weapon_info index of the selected and checked ship_list item or -1 if nothing is checked or 
// the weapon is invalid
int player_start_editor::GetSelectedWeaponListIndex()
{
	char name[255] = "";
	int selected = m_weapon_list.GetCurSel();	
	if((selected != -1) && m_weapon_list.GetCheck(selected))
	{
		// lookup the weapon
		m_weapon_list.GetText(m_weapon_list.GetCurSel(), name);
		int weapon_index = weapon_info_lookup(name);
		return weapon_index;
	}
	return -1; 
}

// Returns the Sexp_variables index of the selected and checked ship_variables_list item or -1 if nothing is checked 
// or the ship is invalid
int player_start_editor::GetSelectedShipVariableListIndex()
{
	// Try the ship_variables_list	
	int selected = m_ship_variable_list.GetCurSel();
	if((selected != -1) && m_ship_variable_list.GetCheck(selected))
	{	
		//lookup the variable		
		return get_nth_variable_index(selected+1, SEXP_VARIABLE_STRING); 
	}
	return -1; 
}

// Returns the Sexp_variables index of the selected and checked weapon_variables_list item or -1 if nothing is checked 
// or the weapon is invalid
int player_start_editor::GetSelectedWeaponVariableListIndex()
{
	// Try the ship_variables_list	
	int selected = m_weapon_variable_list.GetCurSel();
	if((selected != -1) && m_weapon_variable_list.GetCheck(selected))
	{			
		return get_nth_variable_index(selected+1, SEXP_VARIABLE_STRING); 
	}
	return -1; 
}

// Updates the Sexp_variable in a variable combobox to the value supplied
void player_start_editor::UpdateQuantityVariable(CComboBox *variable_list, int pool_value)
{
	char variable_name[TOKEN_LENGTH]; 
	variable_list->GetLBText(variable_list->GetCurSel(), variable_name); 
	int variable_index = get_index_sexp_variable_name(variable_name);
	if (variable_index > -1) 
	{
		char variable_value[TOKEN_LENGTH];
		sprintf(variable_value, "%d", pool_value); 
		Assert (Sexp_variables[variable_index].type & SEXP_VARIABLE_NUMBER);
		sexp_fred_modify_variable(variable_value, variable_name, variable_index, SEXP_VARIABLE_NUMBER);
	}
}

// ship pool count change
void player_start_editor::OnUpdateShipPool() 
{
	int si_index;
	if (!dlg_inited){
		return;
	}

	UpdateData(TRUE);	
	
	si_index = GetSelectedShipListIndex(); 
	// Update the pool if we have a valid ship type
	if(si_index >= 0)
	{
		static_ship_pool[selected_team][si_index] = m_ship_pool;
		// If this value came from a variable we should update its value
		UpdateQuantityVariable(&m_ship_quantity_variable, m_ship_pool);
		return ; 
	}
	
	// Maybe it's the ship_variables_list that is actually selected
	int sexp_index = GetSelectedShipVariableListIndex();
	// if we have a valid sexp_variable
	if(sexp_index >= 0)
	{
		dynamic_ship_pool[selected_team][sexp_index] = m_ship_pool;	
		UpdateQuantityVariable(&m_ship_quantity_variable, m_ship_pool);
	}	
}

// weapon pool count change
void player_start_editor::OnUpdateWeaponPool() 
{
	int wi_index;

	if (!dlg_inited){
		return;
	}

	UpdateData(TRUE);	
	
	wi_index = GetSelectedWeaponListIndex(); 
	// Update the pool if we have a valid weapon type
	if(wi_index >= 0)
	{
		static_weapon_pool[selected_team][wi_index] = m_weapon_pool;
		// If this value came from a variable we should update its value
		UpdateQuantityVariable(&m_weapon_quantity_variable, m_weapon_pool);
		return ; 
	}
	
	// Maybe it's the ship_variables_list that is actually selected
	int sexp_index = GetSelectedWeaponVariableListIndex();
	// if we have a valid sexp_variable
	if(sexp_index >= 0)
	{
		dynamic_weapon_pool[selected_team][sexp_index] = m_weapon_pool;	
		UpdateQuantityVariable(&m_weapon_quantity_variable, m_weapon_pool);
	}
}
