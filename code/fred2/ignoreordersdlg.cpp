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
#include "IgnoreOrdersDlg.h"
#include "hud/hudsquadmsg.h"				// need this for the menu stuff
#include "globalincs/linklist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// super cool macro to make IDC_* names

/////////////////////////////////////////////////////////////////////////////
// ignore_orders_dlg dialog

ignore_orders_dlg::ignore_orders_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(ignore_orders_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ignore_orders_dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void ignore_orders_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ignore_orders_dlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ignore_orders_dlg, CDialog)
	//{{AFX_MSG_MAP(ignore_orders_dlg)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnCheck2)
	ON_BN_CLICKED(IDC_CHECK3, OnCheck3)
	ON_BN_CLICKED(IDC_CHECK4, OnCheck4)
	ON_BN_CLICKED(IDC_CHECK5, OnCheck5)
	ON_BN_CLICKED(IDC_CHECK6, OnCheck6)
	ON_BN_CLICKED(IDC_CHECK7, OnCheck7)
	ON_BN_CLICKED(IDC_CHECK8, OnCheck8)
	ON_BN_CLICKED(IDC_CHECK9, OnCheck9)
	ON_BN_CLICKED(IDC_CHECK10, OnCheck10)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ignore_orders_dlg message handlers

BOOL ignore_orders_dlg::OnInitDialog() 
{
	int i, default_orders, last_bottom, orders_accepted;
	RECT	window_size;
	char buf[128];
	object *objp;

	CDialog::OnInitDialog();
	
	check_boxes[0].button = (CButton *)GetDlgItem( IDC_CHECK1 );
	check_boxes[1].button = (CButton *)GetDlgItem( IDC_CHECK2 );
	check_boxes[2].button = (CButton *)GetDlgItem( IDC_CHECK3 );
	check_boxes[3].button = (CButton *)GetDlgItem( IDC_CHECK4 );
	check_boxes[4].button = (CButton *)GetDlgItem( IDC_CHECK5 );
	check_boxes[5].button = (CButton *)GetDlgItem( IDC_CHECK6 );
	check_boxes[6].button = (CButton *)GetDlgItem( IDC_CHECK7 );
	check_boxes[7].button = (CButton *)GetDlgItem( IDC_CHECK8 );
	check_boxes[8].button = (CButton *)GetDlgItem( IDC_CHECK9 );
	check_boxes[9].button = (CButton *)GetDlgItem( IDC_CHECK10 );

	// change the labels on the check boxes to reflect the set of default
	// orders for this ship
	if ( m_ship >= 0 ) {
		default_orders = ship_get_default_orders_accepted( &Ship_info[Ships[m_ship].ship_info_index] );
	} else {
		// we are doing multiple edit on ships.  We just need to get default orders for
		// the first marked ship since they'd better all be the same anyway!!!
		default_orders = 0;
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags & OF_MARKED)) {
				int these_orders;

				these_orders = ship_get_default_orders_accepted( &Ship_info[Ships[objp->instance].ship_info_index] );
				if ( default_orders == 0 )
					default_orders = these_orders;
				else if ( default_orders != these_orders )
					Int3();
			}
		}

	}

	// set the checkboxes for the orders accepted
	m_num_checks_active = 0;
	for (i = 0; i < NUM_COMM_ORDER_ITEMS; i++ )
	{
		if (default_orders & Comm_orders[i].item)
		{
			// Not enough space to display checkboxes for all comm orders!
			// Need to add more checkboxes.
			if (m_num_checks_active >= MAX_CHECKBOXES)
			{
				Int3();
				break;
			}

			check_boxes[m_num_checks_active].button->SetWindowText(Comm_orders[i].name);
			check_boxes[m_num_checks_active].id = Comm_orders[i].item;
			m_num_checks_active++;
		}
	}

	// resize the dialog by shrinking the height by the number of checkboxes that
	// we removed
	GetWindowRect( &window_size );

	// hide the rest of the dialog items
	last_bottom = 0;
	for ( i = MAX_CHECKBOXES - 1; i >= m_num_checks_active; i-- ) {
		RECT check_size;

		// get the size of the checkbox, then hide it.
		check_boxes[i].button->GetWindowRect( &check_size );
		check_boxes[i].button->ShowWindow(SW_HIDE);

		// shrink the size of the parent window by the size of the checkbox
		if ( last_bottom != 0 )
			window_size.bottom -= (last_bottom - check_size.bottom );

		last_bottom = check_size.bottom;

	}

	// call MoveWindow to resize the window
	MoveWindow( &window_size, TRUE );

	// set the check marks in the box based on orders_accepted valud in the ship structure(s)
	if ( m_ship >= 0 ) {
		orders_accepted = Ships[m_ship].orders_accepted;
		for ( i = 0; i < m_num_checks_active; i++ ) {
			if ( check_boxes[i].id & orders_accepted )
				check_boxes[i].button->SetCheck(1);
		}
	} else {
		int first_time;

		first_time = 1;
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) && (objp->flags & OF_MARKED)) {

				// get the orders for this ship.  If a state is not set 
				orders_accepted = Ships[objp->instance].orders_accepted;
				if ( first_time ) {
					for ( i = 0; i < m_num_checks_active; i++ ) {
						if ( check_boxes[i].id & orders_accepted )
							check_boxes[i].button->SetCheck(1);
					}
					first_time = 0;
				} else {
					for ( i = 0; i < m_num_checks_active; i++ ) {
						// see if the order matches the check box order
						if ( check_boxes[i].id & orders_accepted ) {
							// if it matches, if it is not already set, then it is indeterminate.
							if ( !check_boxes[i].button->GetCheck() )
								check_boxes[i].button->SetCheck(2);
						} else {
							// if the order isn't active, and already set, mark as indeterminite.
							if ( check_boxes[i].button->GetCheck() )
								check_boxes[i].button->SetCheck(2);
						}
					}
				}
			}
		}
	}

	// finally, set the title of the window to be something else.  We really aren't checking orders
	// which will be ignore, but rather orders which will be accepted
	sprintf(buf, "Player orders accepted" );
	SetWindowText(buf);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// for the OnOK function, we scan through the list of checkboxes, and reset the ship's orders accepted
// variable based on the state of the checkboxes
void ignore_orders_dlg::OnOK() 
{
	int orders_accepted, i;
	object *objp;

	// clear out the orders, then set the bits according to which check boxes are set
	if ( m_ship >= 0 ) {
		orders_accepted = 0;
		for ( i = 0; i < m_num_checks_active; i++ ) {
			if ( check_boxes[i].button->GetCheck() )
				orders_accepted |= check_boxes[i].id;
		}
		Ships[m_ship].orders_accepted = orders_accepted;
	} else {
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags & OF_MARKED)) {
				Ships[objp->instance].orders_accepted = 0;
				for ( i = 0; i < m_num_checks_active; i++ ) {
					int box_value;

					box_value = check_boxes[i].button->GetCheck();
					// get the status of the checkbox -- if in the indeterminite state, then
					// skip it
					if ( box_value == 2 )
						continue;

					// if the button is set, then set the bit, otherwise, clear the bit
					if ( box_value == 1 )
						Ships[objp->instance].orders_accepted |= check_boxes[i].id;
					else
						Ships[objp->instance].orders_accepted &= ~(check_boxes[i].id);
				}
			}
		}
	}
	
	CDialog::OnOK();
}

// stupid routines to deal with the tri-state values of the checkboxes
void ignore_orders_dlg::OnCheck1() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK1);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck2() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK2);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck3() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK3);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck4() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK4);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck5() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK5);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck6() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK6);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck7() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK7);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck8() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK8);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck9() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK9);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}

void ignore_orders_dlg::OnCheck10() 
{
	CButton *button;

	button = (CButton *)GetDlgItem(IDC_CHECK10);
	if (button->GetCheck() == 1)
		button->SetCheck(0);
	else
		button->SetCheck(1);
}
