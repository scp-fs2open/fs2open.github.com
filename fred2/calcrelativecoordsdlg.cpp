// CalcRelativeCoordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "calcrelativecoordsdlg.h"
#include "orienteditor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

calc_relative_coords_dlg::calc_relative_coords_dlg(CWnd *pParent /*=nullptr*/)
	: CDialog(calc_relative_coords_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(calc_relative_coords_dlg)
	m_distance = _T("");
	m_orientation_p = _T("");
	m_orientation_b = _T("");
	m_orientation_h = _T("");
	//}}AFX_DATA_INIT
}

void calc_relative_coords_dlg::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(calc_relative_coords_dlg)
	DDX_Control(pDX, IDC_ORIGIN_LIST, m_origin_list);
	DDX_Control(pDX, IDC_SATELLITE_LIST, m_satellite_list);
	DDX_Text(pDX, IDC_DISTANCE, m_distance);
	DDX_Text(pDX, IDC_ORIENTATION_P, m_orientation_p);
	DDX_Text(pDX, IDC_ORIENTATION_B, m_orientation_b);
	DDX_Text(pDX, IDC_ORIENTATION_H, m_orientation_h);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(calc_relative_coords_dlg, CDialog)
	//{{AFX_MSG_MAP(calc_relative_coords_dlg)
	ON_LBN_SELCHANGE(IDC_ORIGIN_LIST, OnSelchangeOriginList)
	ON_LBN_SELCHANGE(IDC_SATELLITE_LIST, OnSelchangeSatelliteList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL calc_relative_coords_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_origin_list.ResetContent();
	m_satellite_list.ResetContent();
	m_object_indexes.clear();

	bool marked_satellite = false;

	for (auto ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr))
	{
		bool added = false;

		if (ptr->type == OBJ_START || ptr->type == OBJ_SHIP)
		{
			m_origin_list.AddString(Ships[ptr->instance].ship_name);
			m_satellite_list.AddString(Ships[ptr->instance].ship_name);

			added = true;
		}
		else if (ptr->type == OBJ_WAYPOINT)
		{
			SCP_string text;
			int waypoint_num;

			auto wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
			Assert(wp_list != nullptr);
			sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);
			m_origin_list.AddString(text.c_str());
			m_satellite_list.AddString(text.c_str());

			added = true;
		}

		if (added)
		{
			int objnum = OBJ_INDEX(ptr);

			if (ptr->flags[Object::Object_Flags::Marked])
			{
				int list_size = (int)m_object_indexes.size();

				if (cur_object_index == objnum)
				{
					m_origin_list.SetCurSel(list_size);
				}
				else if (!marked_satellite)
				{
					m_satellite_list.SetCurSel(list_size);
					marked_satellite = true;
				}
			}

			m_object_indexes.push_back(objnum);
		}
	}

	if (marked_satellite)
		update_coords();

	return TRUE;
}

void calc_relative_coords_dlg::OnSelchangeOriginList()
{
	update_coords();
}

void calc_relative_coords_dlg::OnSelchangeSatelliteList()
{
	update_coords();
}

void calc_relative_coords_dlg::update_coords()
{
	int origin_index = m_origin_list.GetCurSel();
	int satellite_index = m_satellite_list.GetCurSel();

	if (origin_index < 0 || satellite_index < 0)
	{
		m_distance = _T("0.0");
		m_orientation_p = _T("");
		m_orientation_b = _T("");
		m_orientation_h = _T("");

		UpdateData(FALSE);
		return;
	}

	int origin_objnum = m_object_indexes[origin_index];
	int satellite_objnum = m_object_indexes[satellite_index];

	if (origin_objnum == satellite_objnum)
	{
		m_distance = _T("0.0");
		m_orientation_p = _T("");
		m_orientation_b = _T("");
		m_orientation_h = _T("");

		UpdateData(FALSE);
		return;
	}

	auto origin_pos = Objects[origin_objnum].pos;
	auto satellite_pos = Objects[satellite_objnum].pos;

	// distance
	m_distance.Format("%.01f", vm_vec_dist(&origin_pos, &satellite_pos));

	// transform the coordinate frame
	vec3d delta_vec, local_vec;
	vm_vec_sub(&delta_vec, &satellite_pos, &origin_pos);
	if (Objects[origin_objnum].type != OBJ_WAYPOINT)
		vm_vec_rotate(&local_vec, &delta_vec, &Objects[origin_objnum].orient);

	// find the orientation
	matrix m;
	vm_vector_2_matrix(&m, &local_vec);

	// find the angles
	angles ang;
	vm_extract_angles_matrix(&ang, &m);
	m_orientation_p.Format("%.01f", orient_editor::to_degrees(ang.p));
	m_orientation_b.Format("%.01f", orient_editor::to_degrees(ang.b));
	m_orientation_h.Format("%.01f", orient_editor::to_degrees(ang.h));

	UpdateData(FALSE);
}
