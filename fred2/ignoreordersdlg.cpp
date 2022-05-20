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
	ON_CLBN_CHKCHANGE(IDC_IGNORE_ORDERS_LIST, OnCheckChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ignore_orders_dlg message handlers

BOOL ignore_orders_dlg::OnInitDialog() 
{
	int i;
	std::set<size_t> default_orders, orders_accepted;
	char buf[128];
	object *objp;

	m_orderList.clear();

	CDialog::OnInitDialog();

	//Because Windows, this needs to be manually created in addition to having the resource thingy.
	m_checklistbox = std::unique_ptr<CCheckListBox>(new CCheckListBox());
	m_checklistbox->SubclassDlgItem(IDC_IGNORE_ORDERS_LIST, this);
	m_checklistbox->SetCheckStyle(BS_AUTO3STATE);

	// change the labels on the check boxes to reflect the set of default
	// orders for this ship
	if ( m_ship >= 0 ) {
		default_orders = ship_get_default_orders_accepted( &Ship_info[Ships[m_ship].ship_info_index] );
	} else {
		// we are doing multiple edit on ships.  We just need to get default orders for
		// the first marked ship since they'd better all be the same anyway!!!
		default_orders.clear();
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags[Object::Object_Flags::Marked])) {
				const std::set<size_t>& these_orders = ship_get_default_orders_accepted( &Ship_info[Ships[objp->instance].ship_info_index] );
				if ( default_orders.empty() )
					default_orders = these_orders;
				else if ( default_orders != these_orders )
					Int3();
			}
		}

	}

	CCheckListBox* list = ((CCheckListBox*)GetDlgItem(IDC_IGNORE_ORDERS_LIST));
	

	for (size_t order_id : default_orders){
		int checkboxNr = (int) m_orderList.size();

		list->AddString(Player_orders[order_id].localized_name.c_str());
		list->SetCheck(checkboxNr, BST_UNCHECKED);

		m_orderList.push_back(order_id);
	}

	// set the check marks in the box based on orders_accepted valud in the ship structure(s)
	if ( m_ship >= 0 ) {
		orders_accepted = Ships[m_ship].orders_accepted;
		for ( i = 0; i < (int) m_orderList.size(); i++) {
			if ( orders_accepted.find(m_orderList[i]) != orders_accepted.end())
				list->SetCheck(i, BST_CHECKED);
		}
	} else {
		int first_time;

		first_time = 1;
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) && (objp->flags[Object::Object_Flags::Marked])) {

				// get the orders for this ship.  If a state is not set 
				orders_accepted = Ships[objp->instance].orders_accepted;
				if ( first_time ) {
					for (i = 0; i < (int) m_orderList.size(); i++) {
						if (orders_accepted.find(m_orderList[i]) != orders_accepted.end())
							list->SetCheck(i, BST_CHECKED);
					}
					first_time = 0;
				} else {
					for (i = 0; i < (int) m_orderList.size(); i++) {
						// see if the order matches the check box order
						if ( orders_accepted.find(m_orderList[i]) != orders_accepted.end() ) {
							// if it matches, if it is not already set, then it is indeterminate.
							if ( list->GetCheck(i) == BST_UNCHECKED )
								list->SetCheck(i, BST_INDETERMINATE);
						} else {
							// if the order isn't active, and already set, mark as indeterminite.
							if ( list->GetCheck(i) != BST_UNCHECKED )
								list->SetCheck(i, BST_INDETERMINATE);
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
	std::set<size_t> orders_accepted;
	int i;
	object *objp;

	CCheckListBox* list = ((CCheckListBox*)GetDlgItem(IDC_IGNORE_ORDERS_LIST));

	// clear out the orders, then set the bits according to which check boxes are set
	if ( m_ship >= 0 ) {
		for ( i = 0; i < (int) m_orderList.size(); i++) {
			if (list->GetCheck(i) == BST_CHECKED)
				orders_accepted.insert(m_orderList[i]);
		}
		Ships[m_ship].orders_accepted = orders_accepted;
	} else {
		for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) && (objp->flags[Object::Object_Flags::Marked])) {
				for (i = 0; i < (int) m_orderList.size(); i++) {
					int box_value;

					box_value = list->GetCheck(i);
					// get the status of the checkbox -- if in the indeterminite state, then
					// skip it
					if ( box_value == BST_INDETERMINATE )
						continue;

					// if the button is set, then set the bit, otherwise, clear the bit
					if ( box_value == BST_CHECKED )
						Ships[objp->instance].orders_accepted.insert(m_orderList[i]);
					else
						Ships[objp->instance].orders_accepted.erase(m_orderList[i]);
				}
			}
		}
	}
	
	CDialog::OnOK();
}

void ignore_orders_dlg::OnCheckChange()
{
	CCheckListBox* list = ((CCheckListBox*)GetDlgItem(IDC_IGNORE_ORDERS_LIST));

	int i = list->GetCurSel();
	if (list->GetCheck(i) == BST_INDETERMINATE)
		list->SetCheck(i, BST_UNCHECKED);
	
}