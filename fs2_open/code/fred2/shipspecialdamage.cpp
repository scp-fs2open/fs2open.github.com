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
#define new DEBUG_NEW
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
	DDV_MinMaxInt(pDX, m_inner_rad, 10, 10000);
	DDX_Text(pDX, IDC_SPECIAL_OUTER_RAD, m_outer_rad);
	DDV_MinMaxInt(pDX, m_outer_rad, 11, 10000);
	DDX_Text(pDX, IDC_SPECIAL_DAMAGE, m_damage);
	DDV_MinMaxInt(pDX, m_damage, 0, 100000);
	DDX_Text(pDX, IDC_SPECIAL_SHOCK_SPEED, m_shock_speed);
	DDV_MinMaxInt(pDX, m_shock_speed, 10, 10000);
	DDX_Text(pDX, IDC_SPECIAL_BLAST, m_blast);
	DDV_MinMaxInt(pDX, m_blast, 0, 100000);
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
	DoGray();}

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

	if (Ships[m_ship_num].special_exp_index == -1) {
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

		if (m_inner_rad < 10) m_inner_rad = 10;
		if (m_outer_rad < 11) m_outer_rad = 11;
		if (m_shock_speed < 10) m_shock_speed = 10;
	} else {
		int index = Ships[m_ship_num].special_exp_index;
		Assert( (index > 0) && (index < MAX_SEXP_VARIABLES-(BLOCK_EXP_SIZE-1)) );

		m_inner_rad = atoi(Sexp_variables[index++].text);
		m_outer_rad = atoi(Sexp_variables[index++].text);
		m_damage = atoi(Sexp_variables[index++].text);
		m_blast = atoi(Sexp_variables[index++].text);
		m_shock_enabled = atoi(Sexp_variables[index++].text);
		m_shock_speed = atoi(Sexp_variables[index++].text);
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

	object *objp;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags & OF_MARKED)
				update_ship(objp->instance);
		}

		objp = GET_NEXT(objp);
	}

	CDialog::OnOK();
}

void ShipSpecialDamage::update_ship(int shipnum)
{
	ship *shipp = &Ships[shipnum];

	// TODO: Add extra validation here
	if (m_special_exp_enabled) {

		int start;

		if (m_inner_rad > m_outer_rad) {
			MessageBox("Inner radius must be less than outer radius");
			return;
		}

		if (shipp->special_exp_index == -1) {
			// get free sexp_variables
			start = sexp_variable_allocate_block(shipp->ship_name, SEXP_VARIABLE_BLOCK | SEXP_VARIABLE_BLOCK_EXP);
			if (start == -1) {
				MessageBox("Unable to allocate storage, try deleting Sexp variables");
				return;
			} else {
				shipp->special_exp_index = start;
			}
		} else {
			start = shipp->special_exp_index;
		}
		// set to update
		set_modified();

		// set em
		sprintf(Sexp_variables[start+INNER_RAD].text, "%d", m_inner_rad);
		sprintf(Sexp_variables[start+OUTER_RAD].text, "%d", m_outer_rad);
		sprintf(Sexp_variables[start+DAMAGE].text, "%d", m_damage);
		sprintf(Sexp_variables[start+BLAST].text, "%d", m_blast);
		sprintf(Sexp_variables[start+PROPAGATE].text, "%d", m_shock_enabled);
		sprintf(Sexp_variables[start+SHOCK_SPEED].text, "%d", m_shock_speed);

	} else {
		if (shipp->special_exp_index != -1) {
			// set to update
			set_modified();

			// free block
			sexp_variable_block_free(shipp->ship_name, Ships[m_ship_num].special_exp_index, SEXP_VARIABLE_BLOCK |SEXP_VARIABLE_BLOCK_EXP);

			// set index to no exp block
			shipp->special_exp_index = -1;
		}
	}
}

