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

constexpr auto INPUT_THRESHOLD = 0.01f;		// smallest increment of input box
constexpr auto INPUT_FORMAT = "%.01f";

/////////////////////////////////////////////////////////////////////////////
// orient_editor dialog

orient_editor::orient_editor(CWnd* pParent /*=NULL*/)
	: CDialog(orient_editor::IDD, pParent)
{
	//{{AFX_DATA_INIT(orient_editor)
	m_object_index = 0;
	m_point_to = FALSE;
	m_position_x = _T("");
	m_position_y = _T("");
	m_position_z = _T("");
	m_location_x = _T("0.0");
	m_location_y = _T("0.0");
	m_location_z = _T("0.0");
	m_orientation_p = _T("");
	m_orientation_b = _T("");
	m_orientation_h = _T("");
	//}}AFX_DATA_INIT

	Assert(query_valid_object());
	auto pos = &Objects[cur_object_index].pos;
	m_position_x.Format(INPUT_FORMAT, pos->xyz.x);
	m_position_y.Format(INPUT_FORMAT, pos->xyz.y);
	m_position_z.Format(INPUT_FORMAT, pos->xyz.z);

	angles ang;
	vm_extract_angles_matrix(&ang, &Objects[cur_object_index].orient);
	m_orientation_p.Format(INPUT_FORMAT, to_degrees(ang.p));
	m_orientation_b.Format(INPUT_FORMAT, to_degrees(ang.b));
	m_orientation_h.Format(INPUT_FORMAT, to_degrees(ang.h));
}

void orient_editor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(orient_editor)
	DDX_Control(pDX, IDC_SPIN1, m_spin1);
	DDX_Control(pDX, IDC_SPIN2, m_spin2);
	DDX_Control(pDX, IDC_SPIN3, m_spin3);
	DDX_Control(pDX, IDC_SPIN4, m_spin4);
	DDX_Control(pDX, IDC_SPIN5, m_spin5);
	DDX_Control(pDX, IDC_SPIN6, m_spin6);
	DDX_Control(pDX, IDC_SPIN11, m_spin11);
	DDX_Control(pDX, IDC_SPIN12, m_spin12);
	DDX_Control(pDX, IDC_SPIN13, m_spin13);
	DDX_CBIndex(pDX, IDC_OBJECT_LIST, m_object_index);
	DDX_Check(pDX, IDC_POINT_TO_CHECKBOX, m_point_to);
	DDX_Text(pDX, IDC_POSITION_X, m_position_x);
	DDX_Text(pDX, IDC_POSITION_Y, m_position_y);
	DDX_Text(pDX, IDC_POSITION_Z, m_position_z);
	DDX_Text(pDX, IDC_LOCATION_X, m_location_x);
	DDX_Text(pDX, IDC_LOCATION_Y, m_location_y);
	DDX_Text(pDX, IDC_LOCATION_Z, m_location_z);
	DDX_Text(pDX, IDC_ORIENTATION_P, m_orientation_p);
	DDX_Text(pDX, IDC_ORIENTATION_B, m_orientation_b);
	DDX_Text(pDX, IDC_ORIENTATION_H, m_orientation_h);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(orient_editor, CDialog)
	//{{AFX_MSG_MAP(orient_editor)
	ON_BN_CLICKED(IDC_POINT_TO_CHECKBOX, OnPointTo)
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
	((CButton *) GetDlgItem(IDC_POINT_TO_OBJECT))->SetCheck(TRUE);
	((CButton *) GetDlgItem(IDC_TRANSFORM_INDEPENDENT))->SetCheck(TRUE);
	box = (CComboBox *) GetDlgItem(IDC_OBJECT_LIST);
	box->ResetContent();

	total = 0;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		int objnum = OBJ_INDEX(ptr);

		if (Marked != 1 || objnum != cur_object_index) {
			if ((ptr->type == OBJ_START) || (ptr->type == OBJ_SHIP)) {
				box->AddString(Ships[ptr->instance].ship_name);
				index[total++] = objnum;

			} else if (ptr->type == OBJ_WAYPOINT) {
				int waypoint_num;
				waypoint_list *wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
				Assert(wp_list != NULL);
				sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);

				box->AddString(text);
				index[total++] = objnum;

			} else if (ptr->type == OBJ_JUMP_NODE) {
				box->AddString(jumpnode_get_by_objnum(objnum)->GetName());
				index[total++] = objnum;

			} else if (ptr->type != OBJ_POINT)
				Warning(LOCATION, "Unknown object type %d", ptr->type);
		}

		ptr = GET_NEXT(ptr);
	}

	type = Objects[cur_object_index].type;
	if (Marked == 1 && (type == OBJ_WAYPOINT || type == OBJ_JUMP_NODE)) {
		GetDlgItem(IDC_POINT_TO_CHECKBOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_POINT_TO_OBJECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_POINT_TO_LOCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_OBJECT_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_LOCATION_X)->EnableWindow(FALSE);
		GetDlgItem(IDC_LOCATION_Y)->EnableWindow(FALSE);
		GetDlgItem(IDC_LOCATION_Z)->EnableWindow(FALSE);
		GetDlgItem(IDC_ORIENTATION_P)->EnableWindow(FALSE);
		GetDlgItem(IDC_ORIENTATION_B)->EnableWindow(FALSE);
		GetDlgItem(IDC_ORIENTATION_H)->EnableWindow(FALSE);
		m_object_index = -1;

	} else {
		m_object_index = 0;
	}

	if (Marked == 1 || (type == OBJ_WAYPOINT || type == OBJ_JUMP_NODE)) {
		GetDlgItem(IDC_TRANSFORM_INDEPENDENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSFORM_RELATIVE)->EnableWindow(FALSE);
	}

	m_spin1.SetRange((short)-9999, (short)9999);
	m_spin1.SetPos((int) convert(m_position_x));
	m_spin2.SetRange((short)-9999, (short)9999);
	m_spin2.SetPos((int) convert(m_position_y));
	m_spin3.SetRange((short)-9999, (short)9999);
	m_spin3.SetPos((int) convert(m_position_z));

	m_spin4.SetRange((short)-9999, (short)9999);
	m_spin5.SetRange((short)-9999, (short)9999);
	m_spin6.SetRange((short)-9999, (short)9999);

	m_spin11.SetRange((short)-180, (short)180);
	m_spin11.SetPos((int)convert(m_orientation_p));
	m_spin12.SetRange((short)-180, (short)180);
	m_spin12.SetPos((int)convert(m_orientation_b));
	m_spin13.SetRange((short)-180, (short)180);
	m_spin13.SetPos((int)convert(m_orientation_h));

	UpdateData(FALSE);
	return TRUE;
}

/**
 * Checks whether the position value is close enough to the input string value that we can assume the input has not changed.
 */
bool orient_editor::is_close(float val, const CString &input_str) const
{
	val = perform_input_rounding(val);
	float input_val = convert(input_str);

	float diff = val - input_val;
	return abs(diff) < INPUT_THRESHOLD;
}

/**
 * Checks whether the angle value is close enough to the input string value that we can assume the input has not changed.
 */
bool orient_editor::is_angle_close(float rad, const CString &input_str) const
{
	float deg = perform_input_rounding(to_degrees(rad));
	float input_deg = normalize_degrees(convert(input_str));

	float diff = deg - input_deg;
	return abs(diff) < INPUT_THRESHOLD;
}

bool orient_editor::query_modified()
{
	if (!is_close(Objects[cur_object_index].pos.xyz.x, m_position_x))
		return true;
	if (!is_close(Objects[cur_object_index].pos.xyz.y, m_position_y))
		return true;
	if (!is_close(Objects[cur_object_index].pos.xyz.z, m_position_z))
		return true;

	angles ang;
	vm_extract_angles_matrix(&ang, &Objects[cur_object_index].orient);

	if (!is_angle_close(ang.p, m_orientation_p))
		return true;
	if (!is_angle_close(ang.b, m_orientation_b))
		return true;
	if (!is_angle_close(ang.h, m_orientation_h))
		return true;

	if (((CButton *) GetDlgItem(IDC_POINT_TO_CHECKBOX))->GetCheck() == TRUE)
		return true;

	return false;
}

void orient_editor::OnOK()
{
	vec3d delta, *cur_pos;
	matrix m = vmd_identity_matrix;
	angles ang;
	bool change_pos = false, change_orient = false;

	UpdateData(TRUE);

	// there's enough difference in our position that we're changing it
	cur_pos = &Objects[cur_object_index].pos;
	if (!is_close(cur_pos->xyz.x, m_position_x) || !is_close(cur_pos->xyz.y, m_position_y) || !is_close(cur_pos->xyz.z, m_position_z))
	{
		vec3d pos;
		pos.xyz.x = convert(m_position_x);
		pos.xyz.y = convert(m_position_y);
		pos.xyz.z = convert(m_position_z);
		vm_vec_sub(&delta, &pos, cur_pos);
		change_pos = true;
		set_modified();
	}

	// there's enough difference in our orientation that we're changing it
	vm_extract_angles_matrix(&ang, &Objects[cur_object_index].orient);
	if (!is_angle_close(ang.p, m_orientation_p) || !is_angle_close(ang.b, m_orientation_b) || !is_angle_close(ang.h, m_orientation_h))
	{
		ang.p = fl_radians(convert(m_orientation_p));
		ang.b = fl_radians(convert(m_orientation_b));
		ang.h = fl_radians(convert(m_orientation_h));
		vm_angles_2_matrix(&m, &ang);
		change_orient = true;
		set_modified();
	}

	// we're pointing to something
	if (m_point_to)
		set_modified();

	object *origin_objp = nullptr;
	vec3d origin_prev_pos = vmd_zero_vector;
	matrix origin_rotation = vmd_identity_matrix;

	if (Marked > 1 && ((CButton*)GetDlgItem(IDC_TRANSFORM_RELATIVE))->GetCheck() == TRUE) {
		origin_objp = &Objects[cur_object_index];
		origin_prev_pos = origin_objp->pos;
		auto saved_orient = origin_objp->orient;

		// move the origin first
		if (change_pos) {
			vm_vec_add2(&origin_objp->pos, &delta);
		}
		if (m_point_to) {
			actually_point_object(origin_objp);
		}
		else if (change_orient) {
			origin_objp->orient = m;
		}

		// calculate the change in orient
		if (origin_objp->type != OBJ_WAYPOINT) {
			vm_transpose(&saved_orient);
			origin_rotation = saved_orient * origin_objp->orient;
		}
	}

	// move and/or point all the objects
	for (auto ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr != origin_objp && ptr->flags[Object::Object_Flags::Marked]) {
			// relative to the new position of the origin
			if (origin_objp) {
				vec3d relative_pos, transformed_pos;
				vm_vec_sub(&relative_pos, &ptr->pos, &origin_prev_pos);
				vm_vec_unrotate(&transformed_pos, &relative_pos, &origin_rotation);
				vm_vec_add(&ptr->pos, &transformed_pos, &origin_objp->pos);
				ptr->orient = ptr->orient * origin_rotation;
			} 
			// all objects moving independently
			else {
				if (change_pos) {
					vm_vec_add2(&ptr->pos, &delta);
				}
				if (m_point_to) {
					actually_point_object(ptr);
				} else if (change_orient) {
					ptr->orient = m;
				}
			}
		}
	}

	// do post-move cleanup
	if (change_pos) {
		for (auto ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->flags[Object::Object_Flags::Marked]) {
				object_moved(ptr);
			}
		}
	}

	theApp.record_window_data(&Object_wnd_data, this);
	CDialog::OnOK();
}

void orient_editor::actually_point_object(object *ptr)
{
	if (ptr->type != OBJ_WAYPOINT && m_point_to) {
		vec3d v, loc;
		matrix m;

		vm_vec_zero(&v);
		loc.xyz.x = convert(m_location_x);
		loc.xyz.y = convert(m_location_y);
		loc.xyz.z = convert(m_location_z);

		if (((CButton *) GetDlgItem(IDC_POINT_TO_OBJECT))->GetCheck() == TRUE) {
			if (m_object_index >= 0) {
				v = Objects[index[m_object_index]].pos;
				vm_vec_sub2(&v, &ptr->pos);
			}
		} else if (((CButton *) GetDlgItem(IDC_POINT_TO_LOCATION))->GetCheck() == TRUE) {
			vm_vec_sub(&v, &loc, &ptr->pos);

		} else {
			Assertion(0, "neither radio button is checked!");
			return;
		}

		if (IS_VEC_NULL(&v)) {
			return;  // can't point to itself.
		}

		auto uvec = Point_using_uvec ? &ptr->orient.vec.uvec : nullptr;

		vm_vector_2_matrix(&m, &v, uvec, nullptr);
		ptr->orient = m;
	}
}

float orient_editor::to_degrees(float rad)
{
	float deg = fl_degrees(rad);
	return normalize_degrees(deg);
}

float orient_editor::normalize_degrees(float deg)
{
	while (deg < -180.0f)
		deg += 180.0f;
	while (deg > 180.0f)
		deg -= 180.0f;
	// check for negative zero...
	if (deg == -0.0f)
		return 0.0f;
	return deg;
}

/**
 * Extract a float from the CString, being mindful of any comma separators.
 */
float orient_editor::convert(const CString &str) const
{
	char buf[256];
	size_t i, j, len;

	string_copy(buf, str, 256 - 1);
	len = strlen(buf);
	for (i=j=0; i<len; i++)
		if (buf[i] != ',')
			buf[j++] = buf[i];

	buf[j] = 0;
	return (float) atof(buf);
}

/**
 * Perform rounding of the float value by loading it into the input box format string and parsing it again.
 * This accounts for not only decimal rounding to the precision of the input box, but also floating point rounding due to inexact fractions such as 1/10.
 * See also GitHub PR #4475.
 */
float orient_editor::perform_input_rounding(float val) const
{
	CString str;
	str.Format(INPUT_FORMAT, val);
	return convert(str);
}

void orient_editor::OnPointTo()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_ORIENTATION_P)->EnableWindow(!m_point_to);
	GetDlgItem(IDC_ORIENTATION_B)->EnableWindow(!m_point_to);
	GetDlgItem(IDC_ORIENTATION_H)->EnableWindow(!m_point_to);
	GetDlgItem(IDC_SPIN11)->EnableWindow(!m_point_to);
	GetDlgItem(IDC_SPIN12)->EnableWindow(!m_point_to);
	GetDlgItem(IDC_SPIN13)->EnableWindow(!m_point_to);
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
