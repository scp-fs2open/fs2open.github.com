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
	//}}AFX_DATA_MAP

	DDV_MaxChars(pDX, m_name, NAME_LENGTH - 4);
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
	object *ptr;

	UpdateData(TRUE);
	UpdateData(TRUE);
	m_name = drop_white_space((char *)(LPCSTR) m_name);
	if (m_name.IsEmpty()) {
		MessageBox("You must give a name before you can continue.");
		return;
	}

	for (i=0; i<MAX_WINGS; i++)
		if (!stricmp(Wings[i].name, m_name) && Wings[i].wave_count) {
			msg.Format("The name \"%s\" is already being used by another wing", m_name);
			MessageBox(msg);
			return;
		}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)){
			i = ptr->instance;
			if (!strnicmp(m_name, Ships[i].ship_name, strlen(m_name))) {
				char *namep;

				namep = Ships[i].ship_name + strlen(m_name);
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

		ptr = GET_NEXT(ptr);
	}

	for (i=0; i<Num_iffs; i++) {
		if (!stricmp(m_name, Iff_info[i].iff_name)) {
			msg.Format("The name \"%s\" is already being used by a team", m_name);
			MessageBox(msg);
			return;
		}
	}

	for ( i=0; i < (int)Ai_tp_list.size(); i++) {
		if (!stricmp(m_name, Ai_tp_list[i].name)) {
			msg.Format("The name \"%s\" is already being used by a target priority group", m_name);
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
