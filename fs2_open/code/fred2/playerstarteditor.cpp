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
 * $Revision: 1.3 $
 * $Date: 2007-01-07 12:55:57 $
 * $Author: taylor $
 *
 * Player starting point editor dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/06/02 09:52:42  karajorma
 * Complete overhaul of how ship loadout is handled to support the use of variables for setting the class and quantity of ships present in loadout.
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
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
	DDX_Control(pDX, IDC_SHIP_VARIABLES_LIST, m_ship_variable_list);
	DDX_Control(pDX, IDC_WEAPON_VARIABLES_LIST, m_weapon_variable_list);
	DDX_Control(pDX, IDC_SHIP_VARIABLES_COMBO, m_ship_quantity_variable);	
	DDX_Control(pDX, IDC_WEAPON_VARIABLES_COMBO, m_weapon_quantity_variable);		
	DDX_Text(pDX, IDC_DELAY, m_delay);	
	DDX_Text(pDX, IDC_SHIP_POOL, m_ship_pool);
	DDX_Text(pDX, IDC_WEAPON_POOL, m_weapon_pool);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(player_start_editor, CDialog)
	//{{AFX_MSG_MAP(player_start_editor)
	ON_WM_INITMENU()
	ON_LBN_SELCHANGE(IDC_SHIP_LIST, OnSelchangeShipList)	
	ON_LBN_SELCHANGE(IDC_WEAPON_LIST, OnSelchangeWeaponList)	
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
	SetupShipAndWeaponPools(); 

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

void player_start_editor::SetupShipAndWeaponPools()
{
	int i;
	int idx;	

	// initialize ship pool data
	memset(static_ship_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SHIP_CLASSES);
	memset(dynamic_ship_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SEXP_VARIABLES);
	memset(static_ship_variable_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SHIP_CLASSES);
	memset(dynamic_ship_variable_pool, -1, sizeof(int) * MAX_TVT_TEAMS * MAX_SEXP_VARIABLES);
	for(i=0; i<MAX_TVT_TEAMS; i++){
		for(idx=0; idx<Team_data[i].number_choices; idx++)
		{
			if (!strcmp(Team_data[i].ship_list_variables[idx], ""))
			{
				// This pool is set to hold the number of ships available at an index corresponding to the Ship_info array.
				static_ship_pool[i][Team_data[i].ship_list[idx]] = Team_data[i].ship_count[idx];
				// This pool is set to hold whether a ship at a Ship_info index has been set by a variable (and the 
				// variables index in Sexp_variables) if it has).
				static_ship_variable_pool[i][Team_data[i].ship_list[idx]] = get_index_sexp_variable_name(Team_data[i].ship_count_variables[idx]);
			}
			else
			{
				// This pool is set to hold the number of ships available at an index corresponding to the Sexp_variables array
				dynamic_ship_pool[i][get_index_sexp_variable_name(Team_data[i].ship_list_variables[idx])] = Team_data[i].ship_count[idx];
				// This pool is set to hold whether a ship at a Ship_info index has been set by a variable (and the 
				// variables index in Sexp_variables) if it has).
				dynamic_ship_variable_pool[i][get_index_sexp_variable_name(Team_data[i].ship_list_variables[idx])] = get_index_sexp_variable_name(Team_data[i].ship_count_variables[idx]);
			}
		}
	}

	// initialize weapon pool data
	memset(weapon_pool, 0, sizeof(int) * MAX_TVT_TEAMS * MAX_WEAPON_TYPES);
	for(i=0; i<MAX_TVT_TEAMS; i++){
		for(idx=0; idx<MAX_WEAPON_TYPES; idx++){
			weapon_pool[i][idx] = Team_data[i].weaponry_pool[idx];
		}
	}
}

// regenerate all controls
void player_start_editor::reset_controls()
{	
	int i;
	int ct;
	
	m_ship_variable_list.ResetContent();
	m_weapon_variable_list.ResetContent();
	m_ship_quantity_variable.ResetContent();
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
			// Do ships
			m_ship_variable_list.AddString(Sexp_variables[i].variable_name);

			// Now we set the checkbox. 
			if((dynamic_ship_pool[selected_team][i] > 0) ||
			   (dynamic_ship_variable_pool[selected_team][i] != NOT_SET_BY_SEXP_VARIABLE))
			{
				m_ship_variable_list.SetCheck(current_entry, TRUE);
			}
			else 
			{
				m_ship_variable_list.SetCheck(current_entry, FALSE);
			}

			// Do weapons
			m_weapon_variable_list.AddString(Sexp_variables[i].variable_name);

			/**Rest of the weapons variable list goes here! */

			// next
			current_entry++;
		}

		// Seeing as how we're looping through Sexp_variables we might as well fill the number variable combo boxes
		else if (Sexp_variables[i].type & SEXP_VARIABLE_NUMBER)
		{
			m_ship_quantity_variable.AddString(Sexp_variables[i].variable_name); 
			m_weapon_quantity_variable.AddString(Sexp_variables[i].variable_name); 
		}
	}

	// If at at the end of all this there are no entries, disable the variables windows. 
	if (!current_entry)
	{
		m_ship_variable_list.EnableWindow(false);
		m_weapon_variable_list.EnableWindow(false);
	}


	// create a checklistbox for each "player" ship type	
	m_ship_list.ResetContent();
	ct = 0;
	for (i=0; i<Num_ship_classes; i++) {
		if (Ship_info[i].flags & SIF_PLAYER_SHIP) {
			m_ship_list.AddString(Ship_info[i].name);
			
			// if the ship currently has pool entries or was set by a variable, check it
			if((static_ship_pool[selected_team][i] > 0) ||
			   (static_ship_variable_pool[selected_team][i] != NOT_SET_BY_SEXP_VARIABLE))
			{
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
		} else if (weapon_pool[selected_team][i] > 0) {
			// not sure if this should be a verbal warning or not, so I'm adding both and making it verbal for now
			Warning(LOCATION, "Weapon '%s' in weapon pool isn't allowed on player loadout!  Resetting count to 0...\n", Weapon_info[i].name);
		//	mprintf(("Weapon '%s' in weapon pool isn't allowed on player loadout!  Resetting to 0 count.\n", Weapon_info[i].name));
			weapon_pool[selected_team][i] = 0;
		}
	}	

	// be sure that nothing is selected	
	m_ship_list.SetCurSel(-1);
	m_ship_variable_list.SetCurSel(-1);
	m_weapon_list.SetCurSel(-1);
	m_weapon_variable_list.SetCurSel(-1);
	m_ship_quantity_variable.SetCurSel(-1);
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
	int ship_index;
	char ship_name[255] = "";

	// If the ShipList is selected the variable ship list should be deselected
	m_ship_variable_list.SetCurSel(-1);

	// determine if we've selected something
	selected = m_ship_list.GetCurSel();	
	if (selected != -1) 
	{
		// lookup the ship
		m_ship_list.GetText(m_ship_list.GetCurSel(), ship_name);
		ship_index = ship_info_lookup(ship_name);

		// if we have a valid ship type
		if(ship_index >= 0)
		{
			// if this item is checked
			if(m_ship_list.GetCheck(selected)) 
			{
				
				if (static_ship_variable_pool[selected_team][ship_index] == NOT_SET_BY_SEXP_VARIABLE)
				{
					if (static_ship_pool[selected_team][ship_index] <= 0)
					{
						static_ship_pool[selected_team][ship_index] = 5;
					}
					m_ship_pool = static_ship_pool[selected_team][ship_index];
					// Set the ship variable ComboBox to reflect that we are not using variables for this ship
					m_ship_quantity_variable.SetCurSel(0); 	
				}
				// If the number of ships was set by a variable
				else 
				{
					Assert (Sexp_variables[static_ship_variable_pool[selected_team][ship_index]].type & SEXP_VARIABLE_NUMBER);
					m_ship_pool = atoi(Sexp_variables[static_ship_variable_pool[selected_team][ship_index]].text);
					int selected_variable = GetTypedVariableIndex(static_ship_variable_pool[selected_team][ship_index], false);
					m_ship_quantity_variable.SetCurSel(selected_variable + VARIABLES_COMBO_OFFSET);
				}
			} 
			// otherwise zero the count
			else 
			{
				static_ship_pool[selected_team][ship_index] = 0;
				m_ship_pool = 0;
				m_ship_quantity_variable.SetCurSel(0); 	
			}		
		} else {
			Int3();
		}
	}
		
	// update shtuff
	UpdateData(FALSE);
}

void player_start_editor::OnSelchangeShipVariablesList() 
{
	// If the ShipList is selected the variable ship list should be deselected
	m_ship_list.SetCurSel(-1);

	//Have we selected something?
	int selection = m_ship_variable_list.GetCurSel(); 
	if (selection != -1) 
	{
		// Is this a valid SEXP variable
		char sexp_variable_name[TOKEN_LENGTH]; 
		m_ship_variable_list.GetText(selection, sexp_variable_name);
		int sexp_index = get_index_sexp_variable_name(sexp_variable_name);

		if (sexp_index > -1) 
		{
			// The selection should the selection'th string variable in Sexp_variables[]
			Assert(selection == GetTypedVariableIndex(sexp_index, true));

			// Is this item checked? 
			if (m_ship_variable_list.GetCheck(selection))
			{
				if (dynamic_ship_variable_pool[selected_team][sexp_index] == NOT_SET_BY_SEXP_VARIABLE)
				{
					if (dynamic_ship_pool[selected_team][sexp_index] <= 0)
					{
						dynamic_ship_pool[selected_team][sexp_index] = 5;
					}
					m_ship_pool = dynamic_ship_pool[selected_team][sexp_index];
					m_ship_quantity_variable.SetCurSel(0); 						
				}
				else 
				{
					Assert (Sexp_variables[dynamic_ship_variable_pool[selected_team][sexp_index]].type & SEXP_VARIABLE_NUMBER);
					m_ship_pool = atoi(Sexp_variables[dynamic_ship_variable_pool[selected_team][sexp_index]].text);
					int selected_variable = GetTypedVariableIndex(dynamic_ship_variable_pool[selected_team][sexp_index], false);
					m_ship_quantity_variable.SetCurSel(selected_variable + VARIABLES_COMBO_OFFSET);
				}
			}
			// Set everything to zero
			else 
			{
				dynamic_ship_pool[selected_team][sexp_index] = NOT_SET_BY_SEXP_VARIABLE;
				m_ship_pool = 0;
				m_ship_quantity_variable.SetCurSel(0); 	
			}
		}
		else
		{
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
	char name[255] = "";
	bool update_static_pool = false; 
	bool update_dynamic_pool = false; 

	m_ship_quantity_variable.GetLBText(m_ship_quantity_variable.GetCurSel(), variable_name);
	int sexp_index = get_index_sexp_variable_name(variable_name);

	Assert ((sexp_index > -1) || (!strcmp("Don't Use Variables", variable_name))); 

	// See if the ship_list was selected
	int ship_index = GetSelectedShipListIndex(); 
	if (ship_index >= 0) 
	{
		static_ship_variable_pool[selected_team][ship_index] = sexp_index; 
		update_static_pool = true; 
	}
	
	// Maybe it's the ship_variables_list that is actually selected
	int ship_variable_index = GetSelectedShipVariableListIndex();
	if (ship_variable_index >= 0 )
	{
		dynamic_ship_variable_pool[selected_team][ship_variable_index] = sexp_index;
		update_dynamic_pool = true; 
	}


	// Update the ship_pool
	if (update_static_pool || update_dynamic_pool)
	{
		int new_quantity = 5;
		if (sexp_index > -1)
		{
			Assert (Sexp_variables[sexp_index].type & SEXP_VARIABLE_NUMBER); 
			new_quantity = atoi(Sexp_variables[sexp_index].text); 
		}
		if (update_static_pool)
		{
			static_ship_pool[selected_team][ship_index] = new_quantity;
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

	// Deselect the variables list when this one is selected	
	m_weapon_quantity_variable.SetCurSel(-1);

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
	for(i=0; i<MAX_TVT_TEAMS; i++)
	{
		int num_choices = 0; 

		// First look through the variables list and write out anything there
		int num_sexp_variables = sexp_variable_count();	
		for (idx=0; idx < num_sexp_variables; idx++)
		{
			// As soon as we come across a sexp_variable we are using
			if (dynamic_ship_pool[i][idx] != NOT_SET_BY_SEXP_VARIABLE)
			{
				Assert (Sexp_variables[idx].type & SEXP_VARIABLE_STRING); 
				int ship_class = ship_info_lookup(Sexp_variables[idx].text);
				
				// If the variable doesn't actually contain a valid ship class name. Warn the player and skip to the next one
				if (ship_class < 0)
				{
					char buffer[256];
					sprintf(buffer, 
							"Sexp Variable %s holds the value %s. This is not a valid ship class. Skipping this entry",
							Sexp_variables[idx].variable_name,
							Sexp_variables[idx].text
							);
					MessageBox(buffer);
					continue;
				}

				// Copy the variable to Team_data
				strcpy(Team_data[i].ship_list_variables[num_choices], Sexp_variables[idx].variable_name);
				// Copy the class to Team_data
				Team_data[i].ship_list[num_choices] = ship_class; 

				// Now we need to set the number of this class available
				if (dynamic_ship_variable_pool[i][idx] == NOT_SET_BY_SEXP_VARIABLE)
				{
					Team_data[i].ship_count[num_choices] = dynamic_ship_pool[i][idx];
					strcpy(Team_data[i].ship_count_variables[num_choices], ""); 
				}
				else 
				{
					strcpy(Team_data[i].ship_count_variables[num_choices], Sexp_variables[dynamic_ship_variable_pool[i][idx]].variable_name);
					Assert (Sexp_variables[dynamic_ship_variable_pool[i][idx]].type & SEXP_VARIABLE_NUMBER);
					Team_data[i].ship_count[num_choices] = atoi(Sexp_variables[dynamic_ship_variable_pool[i][idx]].text);
				}

				num_choices++;
			}
		}

		// Now we deal with the loadout ships that are statically assigned by class

		for(idx=0; idx<Num_ship_classes; idx++)
		{
			// if we have ships here
			if(static_ship_pool[i][idx] > -1)
			{
				Team_data[i].ship_list[num_choices] = idx;
				strcpy(Team_data[i].ship_list_variables[num_choices], "");

				// Now set the number of this class available
				if (static_ship_variable_pool[i][idx] == NOT_SET_BY_SEXP_VARIABLE)
				{
					Team_data[i].ship_count[num_choices] = static_ship_pool[i][idx];
					strcpy(Team_data[i].ship_count_variables[num_choices], "");
				}
				else 
				{
					strcpy(Team_data[i].ship_count_variables[num_choices], Sexp_variables[static_ship_variable_pool[i][idx]].variable_name);
					Assert (Sexp_variables[static_ship_variable_pool[i][idx]].type & SEXP_VARIABLE_NUMBER);
					Team_data[i].ship_count[num_choices] = atoi(Sexp_variables[static_ship_variable_pool[i][idx]].text);
				}

				num_choices++;
			}
		}
		Team_data[i].number_choices = num_choices; 
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

// Returns the Sexp_variables index of the selected and checked ship_variables_list item or -1 if nothing is checked 
// or the ship is invalid
int player_start_editor::GetSelectedShipVariableListIndex()
{
	char name[255] = "";

		// Try the ship_variables_list	
	int selected = m_ship_variable_list.GetCurSel();
	if((selected != -1) && m_ship_variable_list.GetCheck(selected))
	{	
		//lookup the variable
		m_ship_variable_list.GetText(m_ship_variable_list.GetCurSel(), name);
		int sexp_index = get_index_sexp_variable_name(name);
		return sexp_index; 
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
	if (!dlg_inited){
		return;
	}

	UpdateData(TRUE);	
	
	int si_index = GetSelectedShipListIndex(); 
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
	}
}

// Given a Sexp_Variable index, returns the index this would have in Sexp_variables if the array only had variables of
// a certain type or -1 if the supplied index wasn't of the supplied type.
int player_start_editor::GetTypedVariableIndex(int sexp_variables_index, bool string_variable)
{
	// Loop through Sexp_variables until we have found the one corresponding to the argument
	int count = 0;
	for (int i=0; i < MAX_SEXP_VARIABLES; i++)
	{
		if (string_variable && (Sexp_variables[i].type & SEXP_VARIABLE_NUMBER))
		{
			continue; 
		}
		else if (!string_variable && (Sexp_variables[i].type & SEXP_VARIABLE_STRING))
		{
			continue;
		}

		// The type is correct at least. Now lets see if the index is correct
		if (i == sexp_variables_index)
		{
			return count ; 
		}
		count++;
	}
	return -1;
}


void player_start_editor::OnSelchangeWeaponVariablesList() 
{
	// TODO: Add your control notification handler code here
	
}

void player_start_editor::OnSelchangeWeaponVariablesCombo() 
{
	// TODO: Add your control notification handler code here
	
}

