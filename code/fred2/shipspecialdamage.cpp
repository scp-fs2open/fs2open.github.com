/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// ShipSpecialDamage.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "ShipSpecialDamage.h"
#include "globalincs/linklist.h"
#include "parse/sexp.h"
#include "FREDDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ShipSpecialDamage dialog


ShipSpecialDamage::ShipSpecialDamage(CWnd* pParent /*=NULL*/)
	: CDialog(ShipSpecialDamage::IDD, pParent)
{
	//{{AFX_DATA_INIT(ShipSpecialDamage)
	//}}AFX_DATA_INIT
}


void ShipSpecialDamage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ShipSpecialDamage)
	DDX_Check(pDX, IDC_ENABLE_SHOCKWAVE, m_shock_enabled);
	DDX_Check(pDX, IDC_ENABLE_SPECIAL_EXP, m_special_exp_enabled);
	DDX_Text(pDX, IDC_SPECIAL_INNER_RAD, m_inner_rad);
	DDV_MinMaxInt(pDX, m_inner_rad, 1, INT_MAX);
	DDX_Text(pDX, IDC_SPECIAL_OUTER_RAD, m_outer_rad);
	DDV_MinMaxInt(pDX, m_outer_rad, 2, INT_MAX);
	DDX_Text(pDX, IDC_SPECIAL_DAMAGE, m_damage);
	DDV_MinMaxInt(pDX, m_damage, 0, INT_MAX);
	DDX_Text(pDX, IDC_SPECIAL_SHOCK_SPEED, m_shock_speed);
	DDV_MinMaxInt(pDX, m_shock_speed, 0, INT_MAX);
	DDX_Text(pDX, IDC_SPECIAL_BLAST, m_blast);
	DDV_MinMaxInt(pDX, m_blast, 0, INT_MAX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ShipSpecialDamage, CDialog)
	//{{AFX_MSG_MAP(ShipSpecialDamage)
	ON_BN_CLICKED(IDC_ENABLE_SHOCKWAVE, OnEnableShockwave)
	ON_BN_CLICKED(IDC_ENABLE_SPECIAL_EXP, OnEnableSpecialExp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ShipSpecialDamage message handlers

void ShipSpecialDamage::OnEnableShockwave() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	// enable/disable shock speed
	DoGray();
}

void ShipSpecialDamage::OnEnableSpecialExp() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	DoGray();
}

BOOL ShipSpecialDamage::OnInitDialog() 
{
	
	// TODO: Add extra initialization here

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

	Assert (Objects[cur_object_index].flags & OF_MARKED);
	m_ship_num = Objects[cur_object_index].instance;

	if (!(Ships[m_ship_num].use_special_explosion)) {
		// get default_table_values
		ship_info *sip;
		sip = &Ship_info[Ships[m_ship_num].ship_info_index];

		m_inner_rad = (int)sip->shockwave.inner_rad;
		m_outer_rad = (int)sip->shockwave.outer_rad;
		m_damage = (int) sip->shockwave.damage;
		m_blast = (int) sip->shockwave.blast;
		m_shock_enabled = (int) sip->explosion_propagates;
		m_shock_speed = (int) sip->shockwave.speed;
		m_special_exp_enabled = FALSE;

		if (m_inner_rad < 1) {
			m_inner_rad = 1;
		}
		if (m_outer_rad < 2) {
			m_outer_rad = 2;
		}
		if (m_shock_speed < 1) {
			m_shock_speed = 1;
		}
	}
	else {
		m_inner_rad = Ships[m_ship_num].special_exp_inner;
		m_outer_rad = Ships[m_ship_num].special_exp_outer;
		m_damage = Ships[m_ship_num].special_exp_damage;
		m_blast = Ships[m_ship_num].special_exp_blast;
		m_shock_enabled = Ships[m_ship_num].use_shockwave;
		m_shock_speed = Ships[m_ship_num].special_exp_shockwave_speed;
		m_special_exp_enabled = TRUE;
	}


	CDialog::OnInitDialog();

	// maybe gray out lots of stuff
	DoGray();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ShipSpecialDamage::DoGray()
{
	GetDlgItem(IDC_SPECIAL_DAMAGE)->EnableWindow(m_special_exp_enabled);
	GetDlgItem(IDC_SPECIAL_BLAST)->EnableWindow(m_special_exp_enabled);
	GetDlgItem(IDC_SPECIAL_INNER_RAD)->EnableWindow(m_special_exp_enabled);
	GetDlgItem(IDC_SPECIAL_OUTER_RAD)->EnableWindow(m_special_exp_enabled);
	GetDlgItem(IDC_ENABLE_SHOCKWAVE)->EnableWindow(m_special_exp_enabled);
	GetDlgItem(IDC_SPECIAL_SHOCK_SPEED)->EnableWindow(m_special_exp_enabled && m_shock_enabled);
}


void ShipSpecialDamage::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void ShipSpecialDamage::OnOK() 
{
	UpdateData(TRUE);

	int i;

	if (m_inner_rad > m_outer_rad) {
		MessageBox("Inner radius must be less than outer radius");
		return;
	}

	for ( i = 0; i < num_selected_ships; i++ ) {
		update_ship(m_selected_ships[i]);
	}

	CDialog::OnOK();
}

void ShipSpecialDamage::update_ship(int shipnum)
{
	ship *shipp = &Ships[shipnum];
	
	// set to update
	set_modified();

	if (m_special_exp_enabled) {
		// set em
		shipp->use_special_explosion = true;
		shipp->special_exp_inner = m_inner_rad;
		shipp->special_exp_outer = m_outer_rad;
		shipp->special_exp_damage = m_damage;
		shipp->special_exp_blast = m_blast;
		shipp->use_shockwave = (m_shock_enabled ? 1:0) ;
		if (m_shock_speed) {
			if (m_shock_speed < 1) {
				m_shock_speed = 1;
				MessageBox("Shockwave speed must be defined! Setting this to 1 now");
			}
			shipp->special_exp_shockwave_speed = m_shock_speed;
		}
	} else {
		shipp->use_special_explosion = false;
		shipp->special_exp_inner = -1;
		shipp->special_exp_outer = -1;
		shipp->special_exp_damage = -1;
		shipp->special_exp_blast = -1;
		shipp->use_shockwave = false;
		shipp->special_exp_shockwave_speed = -1;
	}
}

