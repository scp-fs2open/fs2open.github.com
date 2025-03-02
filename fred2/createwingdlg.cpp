/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// CreateWingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "CreateWingDlg.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "globalincs/linklist.h"
#include "parse/parselo.h"
#include "iff_defs/iff_defs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// create_wing_dlg dialog

create_wing_dlg::create_wing_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(create_wing_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(create_wing_dlg)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

void create_wing_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(create_wing_dlg)
	DDX_Text(pDX, IDC_NAME, m_name);
	DDV_MaxChars(pDX, m_name, NAME_LENGTH - 4);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(create_wing_dlg, CDialog)
	//{{AFX_MSG_MAP(create_wing_dlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// create_wing_dlg message handlers

void create_wing_dlg::OnOK()
{
	CString msg;
	int i;

	UpdateData(TRUE);
	UpdateData(TRUE);
	m_name = drop_white_space((char *)(LPCSTR) m_name);
	if (m_name.IsEmpty()) {
		MessageBox("You must give a name before you can continue.");
		return;
	}

	for (i=0; i<MAX_WINGS; i++)
		if (!stricmp(Wings[i].name, m_name) && Wings[i].wave_count) {
			msg.Format("The name \"%s\" is already being used by another wing", static_cast<LPCTSTR>(m_name));
			MessageBox(msg);
			return;
		}

	auto len = strlen(m_name);
	for (auto ptr: list_range(&obj_used_list)) {
		if ((ptr->type != OBJ_SHIP) && (ptr->type != OBJ_START))
			continue;
		i = ptr->instance;

		// if this ship is actually going to be in the wing, and if it *can* be in a wing
		// (i.e. it will not be taken out later), then skip the name check
		if (ptr->flags[Object::Object_Flags::Marked]) {
			int ship_type = ship_query_general_type(i);
			if (ship_type >= 0 && Ship_types[ship_type].flags[Ship::Type_Info_Flags::AI_can_form_wing])
				continue;
		}

		// see if this ship name matches what a ship in the wing would be
		if (!strnicmp(m_name, Ships[i].ship_name, len)) {
			auto namep = Ships[i].ship_name + len;
			if (*namep == ' ') {
				namep++;
				while (*namep) {
					if (!isdigit(*namep))
						break;
					namep++;
				}
			}

			if (!*namep) {
				MessageBox("This wing name is already being used by a ship");
				return;
			}
		}
	}

	// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

	for ( i=0; i < (int)Ai_tp_list.size(); i++) {
		if (!stricmp(m_name, Ai_tp_list[i].name)) {
			msg.Format("The name \"%s\" is already being used by a target priority group", static_cast<LPCTSTR>(m_name));
			MessageBox(msg);
			return;
		}
	}

	if (find_matching_waypoint_list((LPCSTR) m_name) != NULL)
	{
		MessageBox("This wing name is already being used by a waypoint path");
		return;
	}
	
	if (!stricmp(m_name.Left(1), "<")) {
		MessageBox("Wing names not allowed to begin with <");
		return;
	}
	
	CDialog::OnOK();
}
