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
#include "OrientEditor.h"
#include "Management.h"
#include "globalincs/linklist.h"
#include "FREDView.h"

#ifdef _DEBUG
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
				int waypoint_num;
				waypoint_list *wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
				Assert(wp_list != NULL);
				sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);

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

	m_spin1.SetRange((short)99999, (short)-99999);
	m_spin1.SetPos((int) convert(m_position_x));
	m_spin2.SetRange((short)99999, (short)-99999);
	m_spin2.SetPos((int) convert(m_position_y));
	m_spin3.SetRange((short)99999, (short)-99999);
	m_spin3.SetPos((int) convert(m_position_z));
	m_spin4.SetRange((short)99999, (short)-99999);
	m_spin5.SetRange((short)99999, (short)-99999);
	m_spin6.SetRange((short)99999, (short)-99999);
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
