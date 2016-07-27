/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// InitialShips.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "InitialShips.h"
#include "CampaignTreeView.h"
#include "CampaignEditorDlg.h"
#include "weapon/weapon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDCAT(x,y)		x ## y

/////////////////////////////////////////////////////////////////////////////
// InitialShips dialog


InitialShips::InitialShips(CWnd* pParent /*=NULL*/)
	: CDialog(InitialShips::IDD, pParent)
{
	//{{AFX_DATA_INIT(InitialShips)
	//}}AFX_DATA_INIT
}


void InitialShips::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(InitialShips)
	DDX_Control(pDX, IDC_INITIAL_LIST, m_initial_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(InitialShips, CDialog)
	//{{AFX_MSG_MAP(InitialShips)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// InitialShips message handlers

BOOL InitialShips::OnInitDialog() 
{
	int i;

	CDialog::OnInitDialog();

	m_list_count = 0;
	// change the window text, get the index into the array, and check the box for either the ships
	// or weapons
	if ( m_initial_items == INITIAL_SHIPS ) {
		for ( auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it ) {
			if ( it->flags & SIF_PLAYER_SHIP ) {
				i = std::distance(Ship_info.cbegin(), it);
				m_initial_list.AddString( it->name );
				if ( Campaign.ships_allowed[i] ) {
					m_initial_list.SetCheck(m_list_count, 1);
				} else if ( (strlen(Campaign.filename) == 0) && strstr(it->name, "Myrmidon") ) { //-V805
					m_initial_list.SetCheck(m_list_count, 1);
				} else {
					m_initial_list.SetCheck(m_list_count, 0);
				}
				m_initial_list.SetItemData(m_list_count, i);
				m_list_count++;
			}
		}

		// set the titale of the window
		SetWindowText("Initial Ships Allowed");
	} else if ( m_initial_items == INITIAL_WEAPONS ) {
		// get the list of initial weapons available by looking at all possible player ships, getting
		// the weapon information for those ships, then putting those weapons onto the list
		int allowed_weapons[MAX_WEAPON_TYPES];

		memset( allowed_weapons, 0, sizeof(allowed_weapons) );
		for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
			if ( it->flags & SIF_PLAYER_SHIP ) {
				for ( i = 0; i < MAX_WEAPON_TYPES; i++ ) {
					if ( it->allowed_weapons[i] )
						allowed_weapons[i] = 1;
				}
			}
		}

		// now add the weapons to the list
		for (i = 0; i < MAX_WEAPON_TYPES; i++ ) {
			if ( allowed_weapons[i] ) {
				m_initial_list.AddString( Weapon_info[i].name );
				int add_weapon = 0;
				if ( Campaign.weapons_allowed[i] ) {
					add_weapon = 1;
				} else if ( strlen(Campaign.filename) == 0 ) { //-V805
					if ( strstr(Weapon_info[i].name, "Subach")) {
						add_weapon = 1;
					} else if ( strstr(Weapon_info[i].name, "Akheton")) {
						add_weapon = 1;
					} else if ( strstr(Weapon_info[i].name, "Rockeye")) {
						add_weapon = 1;
					} else if ( strstr(Weapon_info[i].name, "Tempest")) {
						add_weapon = 1;
					}
				}

				if (add_weapon) {
					m_initial_list.SetCheck( m_list_count, 1 );
				} else {
					m_initial_list.SetCheck( m_list_count, 0 );
				}

				m_initial_list.SetItemData(m_list_count, i );
				m_list_count++;
			}
		}
		SetWindowText("Initial Weapons Allowed");
	} else
		Int3();


	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void InitialShips::OnOK() 
{
	int i, index;

	// zero out whichever array we are setting
	if ( m_initial_items == INITIAL_SHIPS ) {
		for ( i = 0; i < MAX_SHIP_CLASSES; i++ ){
			Campaign.ships_allowed[i] = 0;
		}
	} else if ( m_initial_items == INITIAL_WEAPONS ) {
		for (i = 0; i < MAX_WEAPON_TYPES; i++ )
			Campaign.weapons_allowed[i] = 0;
	}

	for ( i = 0; i < m_list_count; i++ ) {
		if ( m_initial_list.GetCheck(i) ) {
			// this item is checked.  Get the index into either the ship info array or the weapons
			// array
			index = m_initial_list.GetItemData(i);
			if ( m_initial_items == INITIAL_SHIPS ) {
				Campaign.ships_allowed[index] = 1;
			} else if ( m_initial_items == INITIAL_WEAPONS ) {
				Campaign.weapons_allowed[index] = 1;
			} else
				Int3();
			
		}
	}

	CDialog::OnOK();
}

