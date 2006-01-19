// ShipSpecialHitpoints.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "ShipSpecialHitpoints.h"
#include "globalincs/linklist.h"
#include "parse/sexp.h"
#include "FREDDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
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
	DDX_Text(pDX, IDC_SPECIAL_SHIELDS, m_shields);
	DDV_MinMaxInt(pDX, m_shields, 0, 10000);
	DDX_Text(pDX, IDC_SPECIAL_HULL, m_hull);
	DDV_MinMaxInt(pDX, m_hull, 10, 1000000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ShipSpecialHitpoints, CDialog)
	//{{AFX_MSG_MAP(ShipSpecialHitpoints)
	ON_BN_CLICKED(IDC_ENABLE_SPECIAL_HITPOINTS, OnEnableSpecialHitpoints)
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

BOOL ShipSpecialHitpoints::OnInitDialog() 
{
	// get ship num
	object *objp;

	m_ship_num = -1;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags & OF_MARKED) {

				m_ship_num = objp->instance;
				break;
			}
		}
		objp = GET_NEXT(objp);
	}

	if (Ships[m_ship_num].special_hitpoint_index == -1)
	{
		// get default_table_values
		ship_info *sip;
		sip = &Ship_info[Ships[m_ship_num].ship_info_index];

		m_shields = (int)sip->max_shield_strength;
		m_hull = (int)sip->max_hull_strength;
		m_special_hitpoints_enabled = FALSE;

		if (m_shields < 0) m_shields = 0;
		if (m_hull < 10) m_hull = 10;
	} else {
		int index = Ships[m_ship_num].special_hitpoint_index;
//		Assert( (index > 0) && (index < MAX_SEXP_VARIABLES-(BLOCK_HITPOINT_SIZE-1)) );

		m_shields = atoi(Sexp_variables[index++].text);
		m_hull = atoi(Sexp_variables[index++].text);

		m_special_hitpoints_enabled = TRUE;
	}

	CDialog::OnInitDialog();

	// maybe gray out lots of stuff
	DoGray();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ShipSpecialHitpoints::DoGray()
{
	GetDlgItem(IDC_SPECIAL_SHIELDS)->EnableWindow(m_special_hitpoints_enabled);
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

	UpdateData(TRUE);

	// TODO: Add extra validation here
	if (m_special_hitpoints_enabled) {

		int start;

		if (Ships[m_ship_num].special_hitpoint_index == -1) {
			// get free sexp_variables
			start = sexp_variable_allocate_block(Ships[m_ship_num].ship_name, SEXP_VARIABLE_BLOCK | SEXP_VARIABLE_BLOCK_HIT);
			if (start == -1) {
				MessageBox("Unable to allocate storage, try deleting Sexp variables");
				return;
			} else {
				Ships[m_ship_num].special_hitpoint_index = start;
			}
		} else {
			start = Ships[m_ship_num].special_hitpoint_index;
		}
		// set to update
		set_modified();

		// set em
		sprintf(Sexp_variables[start+SHIELD_STRENGTH].text, "%d", m_shields);
		sprintf(Sexp_variables[start+HULL_STRENGTH].text, "%d", m_hull);

	} else {
		if (Ships[m_ship_num].special_hitpoint_index != -1) {
			// set to update
			set_modified();

			// free block
			sexp_variable_block_free(Ships[m_ship_num].ship_name, Ships[m_ship_num].special_hitpoint_index, SEXP_VARIABLE_BLOCK | SEXP_VARIABLE_BLOCK_HIT);

			// set index to no hit block
			Ships[m_ship_num].special_hitpoint_index = -1;
		}
	}

	// calc kamikaze stuff
	if (Ships[m_ship_num].special_hitpoint_index != -1)
	{
		temp_max_hull_strength = (float) atoi(Sexp_variables[Ships[m_ship_num].special_hitpoint_index+HULL_STRENGTH].text);
	}
	else
	{
		temp_max_hull_strength = Ship_info[Ships[m_ship_num].ship_info_index].max_hull_strength;
	}

	Ai_info[Ships[m_ship_num].ai_index].kamikaze_damage = min(1000.0f, 200.0f + (temp_max_hull_strength / 4.0f));


	CDialog::OnOK();
}