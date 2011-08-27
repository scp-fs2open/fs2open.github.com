// ShipSpecialHitpoints.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "ShipSpecialHitpoints.h"
#include "globalincs/linklist.h"
#include "parse/sexp.h"
#include "FREDDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ShipSpecialHitpoints dialog


ShipSpecialHitpoints::ShipSpecialHitpoints(CWnd* pParent /*=NULL*/)
	: CDialog(ShipSpecialHitpoints::IDD, pParent)
{
	//{{AFX_DATA_INIT(ShipSpecialHitpoints)
	//}}AFX_DATA_INIT
}


void ShipSpecialHitpoints::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ShipSpecialHitpoints)
	DDX_Check(pDX, IDC_ENABLE_SPECIAL_HITPOINTS, m_special_hitpoints_enabled);
	DDX_Check(pDX, IDC_ENABLE_SPECIAL_SHIELD, m_special_shield_enabled);
	DDX_Text(pDX, IDC_SPECIAL_SHIELDS, m_shields);
	DDV_MinMaxInt(pDX, m_shields, -1, INT_MAX);
	DDX_Text(pDX, IDC_SPECIAL_HULL, m_hull);
	DDV_MinMaxInt(pDX, m_hull, 1, INT_MAX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ShipSpecialHitpoints, CDialog)
	//{{AFX_MSG_MAP(ShipSpecialHitpoints)
	ON_BN_CLICKED(IDC_ENABLE_SPECIAL_HITPOINTS, OnEnableSpecialHitpoints)
	ON_BN_CLICKED(IDC_ENABLE_SPECIAL_SHIELD, OnEnableSpecialShieldpoints)
	ON_BN_CLICKED(ID_OK, OnOk)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ShipSpecialHitpoints message handlers

void ShipSpecialHitpoints::OnEnableSpecialHitpoints() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	DoGray();
}

void ShipSpecialHitpoints::OnEnableSpecialShieldpoints() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	DoGray();
}

BOOL ShipSpecialHitpoints::OnInitDialog() 
{
	// get ship num
	object *objp;

	num_selected_ships = 0;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags & OF_MARKED) {
				m_selected_ships[num_selected_ships] = objp->instance;
				num_selected_ships++;
			}
		}
		objp = GET_NEXT(objp);
	}

	Assert(num_selected_ships);
	Assert (Objects[cur_object_index].flags & OF_MARKED);
	m_ship_num = Objects[cur_object_index].instance; 

	// get the details from the first ship
	//hull
	if (Ships[m_ship_num].special_hitpoints) {
		m_hull = Ships[m_ship_num].special_hitpoints;
		m_special_hitpoints_enabled = TRUE;
	}
	else {
		// get default_table_values
		ship_info *sip;
		sip = &Ship_info[Ships[m_ship_num].ship_info_index];

		m_hull = (int)sip->max_hull_strength;
		m_special_hitpoints_enabled = FALSE;

		if (m_hull < 1) m_hull = 10;
	}

	//shields
	if (Ships[m_ship_num].special_shield > 0){
		m_shields = Ships[m_ship_num].special_shield;
		m_special_shield_enabled = TRUE;
	}
	else {
		// get default_table_values
		ship_info *sip;
		sip = &Ship_info[Ships[m_ship_num].ship_info_index];

		m_shields = (int)sip->max_shield_strength;
		m_special_shield_enabled = FALSE;

		if (m_shields < 0) {
			m_shields = 0;
		}
	} 

	CDialog::OnInitDialog();

	// maybe gray out lots of stuff
	DoGray();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ShipSpecialHitpoints::DoGray()
{
	GetDlgItem(IDC_SPECIAL_SHIELDS)->EnableWindow(m_special_shield_enabled);
	GetDlgItem(IDC_SPECIAL_HULL)->EnableWindow(m_special_hitpoints_enabled);
}

void ShipSpecialHitpoints::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void ShipSpecialHitpoints::OnOk() 
{
	float temp_max_hull_strength;
	int new_shield_strength, new_hull_strength;
	int i;

	UpdateData(TRUE);

	// TODO: Add extra validation here
	if (m_special_hitpoints_enabled) {

		// Don't update anything if the hull strength is invalid
		if (m_hull < 1) {
			return;
		}

		// set to update
		set_modified();

		new_hull_strength = m_hull;
		//Ships[m_ship_num].special_hitpoints = m_hull;

	} 
	else {
		// set to update
		set_modified();

		new_hull_strength = 0;
	}

	if (m_special_shield_enabled) {

		// Don't update anything if the hull strength is invalid
		if (m_shields < 0) 	{
			return;
		}

		// set to update
		set_modified();

		new_shield_strength = m_shields;
		//Ships[m_ship_num].special_shield = m_shields;

	} 
	else {
		// set to update
		set_modified();

		new_shield_strength = -1;
	}

	for ( i=0; i<num_selected_ships; i++) {
		// set the special hitpoints/shield
		Ships[m_selected_ships[i]].special_hitpoints = new_hull_strength;
		Ships[m_selected_ships[i]].special_shield = new_shield_strength;

		// calc kamikaze stuff
		if (Ships[m_selected_ships[i]].special_hitpoints)
		{
			temp_max_hull_strength = (float)Ships[m_selected_ships[i]].special_hitpoints;
		}
		else
		{
			temp_max_hull_strength = Ship_info[Ships[m_selected_ships[i]].ship_info_index].max_hull_strength;
		}

		Ai_info[Ships[m_selected_ships[i]].ai_index].kamikaze_damage = min(1000, 200 + (int)(temp_max_hull_strength / 4.0f));
	}

	
	CDialog::OnOK();
}
