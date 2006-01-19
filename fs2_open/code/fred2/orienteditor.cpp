/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/OrientEditor.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * Object orientation editor (or just object editor) dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/04/13 20:11:06  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.3  2002/08/15 04:35:44  penguin
 * Changes to build with fs2_open code.lib
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:00  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:02p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 18    7/13/98 10:04a Hoffoss
 * Fixed bug where jump nodes in mission would screw up indexing.
 * 
 * 17    3/12/98 2:21p Johnson
 * Fixed some Fred bugs related to jump nodes.
 * 
 * 16    9/16/97 9:41p Hoffoss
 * Changed Fred code around to stop using Parse_player structure for
 * player information, and use actual ships instead.
 * 
 * 15    8/16/97 2:02a Hoffoss
 * Made docked objects move together in Fred.
 * 
 * 14    8/15/97 8:24p Hoffoss
 * Fixed bug with orient editor.
 * 
 * 13    8/14/97 3:14p Hoffoss
 * Fixed name displayed for players.
 * 
 * 12    7/29/97 2:20p Hoffoss
 * Fixed some minor bugs.
 * 
 * 11    6/26/97 11:18a Hoffoss
 * Fixed bug in orient editor.  OBJ_POINT type wasn't accounted for.
 * 
 * 10    5/20/97 2:28p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
 * 
 * 9     5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 8     4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 7     3/12/97 4:33p Hoffoss
 * added spin controls to orient editor, light intensity level can be
 * specified in BG editor.
 * 
 * 6     2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 5     2/21/97 5:34p Hoffoss
 * Added extensive modification detection and fixed a bug in initial
 * orders editor.
 * 
 * 4     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "OrientEditor.h"
#include "Management.h"
#include "globalincs/linklist.h"
#include "FREDView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PREC	0.0001f

/////////////////////////////////////////////////////////////////////////////
// orient_editor dialog

orient_editor::orient_editor(CWnd* pParent /*=NULL*/)
	: CDialog(orient_editor::IDD, pParent)
{
	vec3d pos;

	//{{AFX_DATA_INIT(orient_editor)
	m_object_index = 0;
	m_point_to = FALSE;
	m_position_z = _T("");
	m_position_y = _T("");
	m_position_x = _T("");
	m_location_x = _T("0.0");
	m_location_y = _T("0.0");
	m_location_z = _T("0.0");
	//}}AFX_DATA_INIT
	Assert(query_valid_object());
	pos = Objects[cur_object_index].pos;
	m_position_x.Format("%.1f", pos.xyz.x);
	m_position_y.Format("%.1f", pos.xyz.y);
	m_position_z.Format("%.1f", pos.xyz.z);
}

void orient_editor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(orient_editor)
	DDX_Control(pDX, IDC_SPIN6, m_spin6);
	DDX_Control(pDX, IDC_SPIN5, m_spin5);
	DDX_Control(pDX, IDC_SPIN4, m_spin4);
	DDX_Control(pDX, IDC_SPIN3, m_spin3);
	DDX_Control(pDX, IDC_SPIN2, m_spin2);
	DDX_Control(pDX, IDC_SPIN1, m_spin1);
	DDX_CBIndex(pDX, IDC_OBJECT_LIST, m_object_index);
	DDX_Check(pDX, IDC_POINT_TO_CHECKBOX, m_point_to);
	DDX_Text(pDX, IDC_POSITION_Z, m_position_z);
	DDX_Text(pDX, IDC_POSITION_Y, m_position_y);
	DDX_Text(pDX, IDC_POSITION_X, m_position_x);
	DDX_Text(pDX, IDC_LOCATION_X, m_location_x);
	DDX_Text(pDX, IDC_LOCATION_Y, m_location_y);
	DDX_Text(pDX, IDC_LOCATION_Z, m_location_z);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(orient_editor, CDialog)
	//{{AFX_MSG_MAP(orient_editor)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// orient_editor message handlers

BOOL orient_editor::OnInitDialog() 
{
	char text[80];
	int type;
	CComboBox *box;
	object *ptr;

	CDialog::OnInitDialog();
	theApp.init_window(&Object_wnd_data, this);
	((CButton *) GetDlgItem(IDC_POINT_TO_OBJECT))->SetCheck(1);
	box = (CComboBox *) GetDlgItem(IDC_OBJECT_LIST);
	box->ResetContent();

	total = 0;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (Marked != 1 || OBJ_INDEX(ptr) != cur_object_index) {
			if ((ptr->type == OBJ_START) || (ptr->type == OBJ_SHIP)) {
				box->AddString(Ships[ptr->instance].ship_name);
				index[total++] = OBJ_INDEX(ptr);

			} else if (ptr->type == OBJ_WAYPOINT) {
				sprintf(text, "%s:%d", Waypoint_lists[ptr->instance / 65536].name,
					(ptr->instance & 0xffff) + 1);

				box->AddString(text);
				index[total++] = OBJ_INDEX(ptr);

			} else if ((ptr->type == OBJ_POINT) || (ptr->type == OBJ_JUMP_NODE)) {
			} else
				Assert(0);  // unknown object type.
		}

		ptr = GET_NEXT(ptr);
	}

	type = Objects[cur_object_index].type;
	if (Marked == 1 && type == OBJ_WAYPOINT) {
		GetDlgItem(IDC_POINT_TO_CHECKBOX)->EnableWindow(0);
		GetDlgItem(IDC_POINT_TO_OBJECT)->EnableWindow(0);
		GetDlgItem(IDC_POINT_TO_LOCATION)->EnableWindow(0);
		GetDlgItem(IDC_OBJECT_LIST)->EnableWindow(0);
		GetDlgItem(IDC_LOCATION_X)->EnableWindow(0);
		GetDlgItem(IDC_LOCATION_Y)->EnableWindow(0);
		GetDlgItem(IDC_LOCATION_Z)->EnableWindow(0);
		m_object_index = -1;

	} else {
		m_object_index = 0;
	}

	m_spin1.SetRange(99999, -99999);
	m_spin1.SetPos((int) convert(m_position_x));
	m_spin2.SetRange(99999, -99999);
	m_spin2.SetPos((int) convert(m_position_y));
	m_spin3.SetRange(99999, -99999);
	m_spin3.SetPos((int) convert(m_position_z));
	m_spin4.SetRange(99999, -99999);
	m_spin5.SetRange(99999, -99999);
	m_spin6.SetRange(99999, -99999);
	UpdateData(FALSE);
	return TRUE;
}

int orient_editor::query_modified()
{
	float dif;

	dif = Objects[cur_object_index].pos.xyz.x - convert(m_position_x);
	if ((dif > PREC) || (dif < -PREC))
		return 1;
	dif = Objects[cur_object_index].pos.xyz.y - convert(m_position_y);
	if ((dif > PREC) || (dif < -PREC))
		return 1;
	dif = Objects[cur_object_index].pos.xyz.z - convert(m_position_z);
	if ((dif > PREC) || (dif < -PREC))
		return 1;

	if (((CButton *) GetDlgItem(IDC_POINT_TO_CHECKBOX))->GetCheck() == 1)
		return 1;

	return 0;
}

void orient_editor::OnOK()
{
	vec3d delta, pos;
	object *ptr;

	UpdateData(TRUE);
	pos.xyz.x = convert(m_position_x);
	pos.xyz.y = convert(m_position_y);
	pos.xyz.z = convert(m_position_z);

	if ((((CButton *) GetDlgItem(IDC_POINT_TO_OBJECT))->GetCheck() == 1) ||
		(((CButton *) GetDlgItem(IDC_POINT_TO_LOCATION))->GetCheck() == 1))
			set_modified();

	vm_vec_sub(&delta, &pos, &Objects[cur_object_index].pos);
	if (delta.xyz.x || delta.xyz.y || delta.xyz.z)
		set_modified();

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags & OF_MARKED) {
			vm_vec_add2(&ptr->pos, &delta);
			update_object(ptr);
		}

		ptr = GET_NEXT(ptr);
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags & OF_MARKED)
			object_moved(ptr);

		ptr = GET_NEXT(ptr);
	}

	theApp.record_window_data(&Object_wnd_data, this);
	CDialog::OnOK();
}

void orient_editor::update_object(object *ptr)
{
	if (ptr->type != OBJ_WAYPOINT && m_point_to) {
		vec3d v, loc;
		matrix m;

		memset(&v, 0, sizeof(vec3d));
		loc.xyz.x = convert(m_location_x);
		loc.xyz.y = convert(m_location_y);
		loc.xyz.z = convert(m_location_z);
		if (((CButton *) GetDlgItem(IDC_POINT_TO_OBJECT))->GetCheck() == 1) {
			v = Objects[index[m_object_index]].pos;
			vm_vec_sub2(&v, &ptr->pos);

		} else if (((CButton *) GetDlgItem(IDC_POINT_TO_LOCATION))->GetCheck() == 1) {
			vm_vec_sub(&v, &loc, &ptr->pos);

		} else {
			Assert(0);  // neither radio button is checked.
		}

		if (!v.xyz.x && !v.xyz.y && !v.xyz.z){
			return;  // can't point to itself.
		}

		vm_vector_2_matrix(&m, &v, NULL, NULL);
		ptr->orient = m;
	}
}

float orient_editor::convert(CString &str)
{
	char buf[256];
	int i, j, len;

	string_copy(buf, str, 255);
	len = strlen(buf);
	for (i=j=0; i<len; i++)
		if (buf[i] != ',')
			buf[j++] = buf[i];

	buf[j] = 0;
	return (float) atof(buf);
}

void orient_editor::OnCancel()
{
	theApp.record_window_data(&Object_wnd_data, this);
	CDialog::OnCancel();
}

void orient_editor::OnClose() 
{
	int z;

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL)
			return;

		if (z == IDYES) {
			OnOK();
			return;
		}
	}

	CDialog::OnClose();
}
