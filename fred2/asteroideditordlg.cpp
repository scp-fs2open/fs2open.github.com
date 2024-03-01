/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// AsteroidEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "AsteroidEditorDlg.h"
#include "listitemchooser.h"
#include "starfield/starfield.h"
#include "FREDDoc.h"
#include "debris/debris.h"	//	Asteroid stuff.
#include "species_defs/species_defs.h"
#include "checkboxlistdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_FIELD_MENU 9000


/////////////////////////////////////////////////////////////////////////////
// asteroid_editor dialog

asteroid_editor::asteroid_editor(CWnd* pParent /*=NULL*/)
	: CDialog(asteroid_editor::IDD, pParent)
{
	int i;

	//{{AFX_DATA_INIT(asteroid_editor)
	m_avg_speed = 0;
	m_density = 0;
	m_enable_asteroids = FALSE;
	m_max_x = _T("");
	m_max_y = _T("");
	m_max_z = _T("");
	m_min_x = _T("");
	m_min_y = _T("");
	m_min_z = _T("");
	m_enable_inner_bounds = FALSE;
	m_field_type = FT_ACTIVE;
	m_box_max_x = _T("");
	m_box_max_y = _T("");
	m_box_max_z = _T("");
	m_box_min_x = _T("");
	m_box_min_y = _T("");
	m_box_min_z = _T("");
	m_field_target_index = -1;
	m_field_enhanced_checks = FALSE;
	m_field_targets.clear();
	//}}AFX_DATA_INIT

	last_field = -1;
	i=0;
//	for (i=0; i<MAX_ASTEROID_FIELDS; i++)
		a_field[i] = Asteroid_field;	//	Only supporting one field per mission.
}

void asteroid_editor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(asteroid_editor)
	DDX_Control(pDX, IDC_DENSITY_SPIN, m_density_spin);
	DDX_Text(pDX, IDC_AVG_SPEED, m_avg_speed);
	DDX_Text(pDX, IDC_DENSITY, m_density);
	DDX_Text(pDX, IDC_MAX_X, m_max_x);
	DDX_Text(pDX, IDC_MAX_Y, m_max_y);
	DDX_Text(pDX, IDC_MAX_Z, m_max_z);
	DDX_Text(pDX, IDC_MIN_X, m_min_x);
	DDX_Text(pDX, IDC_MIN_Y, m_min_y);
	DDX_Text(pDX, IDC_MIN_Z, m_min_z);
	DDX_Check(pDX, IDC_ENABLE_INNER_BOX, m_enable_inner_bounds);
	DDX_Text(pDX, IDC_INNER_MAX_X, m_box_max_x);
	DDX_Text(pDX, IDC_INNER_MAX_Y, m_box_max_y);
	DDX_Text(pDX, IDC_INNER_MAX_Z, m_box_max_z);
	DDX_Text(pDX, IDC_INNER_MIN_X, m_box_min_x);
	DDX_Text(pDX, IDC_INNER_MIN_Y, m_box_min_y);
	DDX_Text(pDX, IDC_INNER_MIN_Z, m_box_min_z);
	DDX_Check(pDX, IDC_RANGE_OVERRIDE, m_field_enhanced_checks);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(asteroid_editor, CDialog)
	//{{AFX_MSG_MAP(asteroid_editor)
	ON_WM_INITMENU()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ENABLE_INNER_BOX, OnEnableInnerBox)
	ON_BN_CLICKED(IDC_PASSIVE_FIELD, OnPassiveField)
	ON_BN_CLICKED(IDC_FIELD_DEBRIS, OnFieldDebris)
	ON_BN_CLICKED(IDC_ACTIVE_FIELD, OnActiveField)
	ON_BN_CLICKED(IDC_FIELD_ASTEROID, OnFieldAsteroid)
	ON_BN_CLICKED(IDC_ADD_FIELD, OnAddField)
	ON_BN_CLICKED(IDC_REMOVE_FIELD, OnRemoveField)
	ON_LBN_SELCHANGE(IDC_FIELD_TARGET_LIST, OnFieldTargetChange)
	ON_BN_CLICKED(IDC_ADD_FIELD_TARGET, OnAddFieldTarget)
	ON_BN_CLICKED(IDC_REMOVE_FIELD_TARGET, OnRemoveFieldTarget)
	ON_BN_CLICKED(IDC_RANGE_OVERRIDE, OnEnableRangeOverride)
	ON_BN_CLICKED(IDC_SELECT_DEBRIS, OnBnClickedSelectDebris)
	ON_BN_CLICKED(IDC_SELECT_ASTEROID, OnBnClickedSelectAsteroid)
	//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// asteroid_editor message handlers

void asteroid_editor::OnInitMenu(CMenu* pMenu)
{
	int i;
	CString str;
	CMenu *m;

	m = pMenu->GetSubMenu(0);
	clear_menu(m);
	i = 0; //for (i=0; i<MAX_ASTEROID_FIELDS; i++) {
		str.Format("Asteroid Field %d", i);
		m->AppendMenu(MF_ENABLED | MF_STRING, ID_FIELD_MENU + i, str);
	//}

	m->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
	if (cur_field != -1)
		m->CheckMenuItem(ID_FIELD_MENU + cur_field, MF_BYCOMMAND | MF_CHECKED);

	CDialog::OnInitMenu(pMenu);
}

BOOL asteroid_editor::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id;

	id = LOWORD(wParam);
	if (id >= ID_FIELD_MENU && id < ID_FIELD_MENU + 1) { //MAX_ASTEROID_FIELDS) {
		cur_field = id - ID_FIELD_MENU;
		update_init();
	}

	return CDialog::OnCommand(wParam, lParam);
}

int asteroid_editor::query_modified()
{
	int i;

	for (i=0; i<1 /*MAX_ASTEROID_FIELDS*/; i++) {
		if (a_field[i].num_initial_asteroids != Asteroid_field.num_initial_asteroids)
			return 1;
		if (vm_vec_dist_quick(&a_field[i].vel, &Asteroid_field.vel) == 0.0f)
			return 1;
		if (a_field[i].min_bound.xyz.x != Asteroid_field.min_bound.xyz.x)
			return 1;
		if (a_field[i].min_bound.xyz.y != Asteroid_field.min_bound.xyz.y)
			return 1;
		if (a_field[i].min_bound.xyz.z != Asteroid_field.min_bound.xyz.z)
			return 1;
		if (a_field[i].max_bound.xyz.x != Asteroid_field.max_bound.xyz.x)
			return 1;
		if (a_field[i].max_bound.xyz.y != Asteroid_field.max_bound.xyz.y)
			return 1;
		if (a_field[i].max_bound.xyz.z != Asteroid_field.max_bound.xyz.z)
			return 1;


		if (a_field[i].has_inner_bound != Asteroid_field.has_inner_bound)
			return 1;

		if (a_field[i].field_type != Asteroid_field.field_type)
			return 1;

		if (a_field[i].has_inner_bound) {
			if (a_field[i].inner_max_bound.xyz.x != Asteroid_field.inner_max_bound.xyz.x)
				return 1;

			if (a_field[i].inner_max_bound.xyz.y != Asteroid_field.inner_max_bound.xyz.y)
				return 1;

			if (a_field[i].inner_max_bound.xyz.z != Asteroid_field.inner_max_bound.xyz.z)
				return 1;

			if (a_field[i].inner_min_bound.xyz.x != Asteroid_field.inner_min_bound.xyz.x)
				return 1;

			if (a_field[i].inner_min_bound.xyz.y != Asteroid_field.inner_min_bound.xyz.y)
				return 1;

			if (a_field[i].inner_min_bound.xyz.z != Asteroid_field.inner_min_bound.xyz.z)
				return 1;
		}

		if (a_field[i].enhanced_visibility_checks != Asteroid_field.enhanced_visibility_checks)
			return 1;

		if (a_field[i].target_names != Asteroid_field.target_names)
			return 1;
	}

	return 0;
}

#define MIN_BOX_THICKNESS 400
int asteroid_editor::validate_data()
{
	if (!m_enable_asteroids) {
		return 1;
	} else {
		// check x
		if (a_field[0].max_bound.xyz.x < a_field[0].min_bound.xyz.x) {
			MessageBox("Asteroid x min is greater than max");
			return 0;
		}

		// check y
		if (a_field[0].max_bound.xyz.y < a_field[0].min_bound.xyz.y) {
			MessageBox("Asteroid y min is greater than max");
			return 0;
		}

		// check z
		if (a_field[0].max_bound.xyz.z < a_field[0].min_bound.xyz.z) {
			MessageBox("Asteroid z min is greater than max");
			return 0;
		}

		// check if inner bounds enabled
		if (a_field[0].has_inner_bound) {
			if (a_field[0].inner_max_bound.xyz.x < a_field[0].inner_min_bound.xyz.x) {
				MessageBox("Asteroid x inner min is greater than inner max");
				return 0;
			}

			if (a_field[0].inner_max_bound.xyz.y < a_field[0].inner_min_bound.xyz.y) {
				MessageBox("Asteroid y inner min is greater than inner max");
				return 0;
			}

			if (a_field[0].inner_max_bound.xyz.z < a_field[0].inner_min_bound.xyz.z) {
				MessageBox("Asteroid z inner min is greater than inner max");
				return 0;
			}

			// check x thickness
			if (a_field[0].inner_min_bound.xyz.x - MIN_BOX_THICKNESS < a_field[0].min_bound.xyz.x) {
				MessageBox("Asteroid x thickness from outer box to inner box must be > 400");
				return 0;
			}

			if (a_field[0].inner_max_bound.xyz.x + MIN_BOX_THICKNESS > a_field[0].max_bound.xyz.x) {
				MessageBox("Asteroid x thickness from outer box to inner box must be > 400");
				return 0;
			}

			// check y thickness
			if (a_field[0].inner_min_bound.xyz.y - MIN_BOX_THICKNESS < a_field[0].min_bound.xyz.y) {
				MessageBox("Asteroid y thickness from outer box to inner box must be > 400");
				return 0;
			}

			if (a_field[0].inner_max_bound.xyz.y + MIN_BOX_THICKNESS > a_field[0].max_bound.xyz.y) {
				MessageBox("Asteroid y thickness from outer box to inner box must be > 400");
				return 0;
			}

			// check z thickness
			if (a_field[0].inner_min_bound.xyz.z - MIN_BOX_THICKNESS < a_field[0].min_bound.xyz.z) {
				MessageBox("Asteroid z thickness from outer box to inner box must be > 400");
				return 0;
			}

			if (a_field[0].inner_max_bound.xyz.z + MIN_BOX_THICKNESS > a_field[0].max_bound.xyz.z) {
				MessageBox("Asteroid z thickness from outer box to inner box must be > 400");
				return 0;
			}
		}

		// Compress the debris field vector to remove "-1" values
		if (a_field[cur_field].field_debris_type.size() > 0) {
			a_field[cur_field].field_debris_type.erase(
				std::remove_if(a_field[cur_field].field_debris_type.begin(),
					a_field[cur_field].field_debris_type.end(),
					[](int value) { return value < 0; }
					),
				a_field[cur_field].field_debris_type.end());
		}

		// check if passive, debris field, at least one debris type selected
		if (a_field[0].field_type == FT_PASSIVE) {
			if (a_field[0].debris_genre == DG_DEBRIS) {
				if (a_field[cur_field].field_debris_type.size() == 0) {
					MessageBox("You must choose one or more types of ship debris");
					return 0;
				}
			}
		}

		// check at least one asteroid subtype is selected
		if (a_field[0].debris_genre == DG_ASTEROID) {
			if (a_field[0].field_asteroid_type.size() == 0) {
				MessageBox("You must choose one or more asteroid subtypes");
				return 0;
			}
		}

	}

	return 1;
}

void asteroid_editor::OnOK()
{
	int i;

	update_init();
	if (!validate_data()) {
		return;
	}
	// If it's an asteroid field, then clear the debris types
	// Otherwise clear the asteroid types
	if (a_field->debris_genre == DG_ASTEROID) {
		a_field->field_debris_type.clear();
	} else {
		a_field->field_asteroid_type.clear();
	}
	for (i=0; i<1 /*MAX_ASTEROID_FIELDS*/; i++)
		Asteroid_field = a_field[i];

	update_map_window();
	theApp.record_window_data(&Asteroid_wnd_data, this);
	CDialog::OnOK();

	FREDDoc_ptr->autosave("asteroid field editor");
}

BOOL asteroid_editor::OnInitDialog() 
{
	cur_field = 0;

	CDialog::OnInitDialog();
	update_init();
	theApp.init_window(&Asteroid_wnd_data, this);

	m_density_spin.SetRange(1, MAX_ASTEROIDS);

	return TRUE;
}

void asteroid_editor::update_init()
{
	int num_asteroids;
	CString str;

	UpdateData(TRUE);
	if (last_field >= 0) {
		// store into temp asteroid field
		num_asteroids = a_field[last_field].num_initial_asteroids;
		a_field[last_field].num_initial_asteroids = m_enable_asteroids ? m_density : 0;
		if (a_field[last_field].num_initial_asteroids < 0)
			a_field[last_field].num_initial_asteroids = 0;

		if (a_field[last_field].num_initial_asteroids > MAX_ASTEROIDS)
			a_field[last_field].num_initial_asteroids = MAX_ASTEROIDS;

		if (num_asteroids != a_field[last_field].num_initial_asteroids)
			set_modified();

		vec3d	vel_vec = {1.0f, 0.0f, 0.0f};
		vm_vec_scale(&vel_vec, (float) m_avg_speed);

		MODIFY(a_field[last_field].vel.xyz.x, vel_vec.xyz.x);
		MODIFY(a_field[last_field].vel.xyz.y, vel_vec.xyz.y);
		MODIFY(a_field[last_field].vel.xyz.z, vel_vec.xyz.z);

		MODIFY(a_field[last_field].min_bound.xyz.x, (float) atof(m_min_x));
		MODIFY(a_field[last_field].min_bound.xyz.y, (float) atof(m_min_y));
		MODIFY(a_field[last_field].min_bound.xyz.z, (float) atof(m_min_z));
		MODIFY(a_field[last_field].max_bound.xyz.x, (float) atof(m_max_x));
		MODIFY(a_field[last_field].max_bound.xyz.y, (float) atof(m_max_y));
		MODIFY(a_field[last_field].max_bound.xyz.z, (float) atof(m_max_z));

		// type of field
		MODIFY(a_field[last_field].field_type, m_field_type);
		MODIFY(a_field[last_field].debris_genre, m_debris_genre);

		MODIFY(a_field[last_field].has_inner_bound, (bool)m_enable_inner_bounds);

		MODIFY(a_field[last_field].inner_min_bound.xyz.x, (float) atof(m_box_min_x));
		MODIFY(a_field[last_field].inner_min_bound.xyz.y, (float) atof(m_box_min_y));
		MODIFY(a_field[last_field].inner_min_bound.xyz.z, (float) atof(m_box_min_z));
		MODIFY(a_field[last_field].inner_max_bound.xyz.x, (float) atof(m_box_max_x));
		MODIFY(a_field[last_field].inner_max_bound.xyz.y, (float) atof(m_box_max_y));
		MODIFY(a_field[last_field].inner_max_bound.xyz.z, (float) atof(m_box_max_z));

		MODIFY(a_field[last_field].enhanced_visibility_checks, (bool)m_field_enhanced_checks);

		MODIFY(a_field[last_field].target_names, m_field_targets);
	}

	Assert(cur_field >= 0);
	// get from temp asteroid field into class
	m_enable_asteroids = a_field[cur_field].num_initial_asteroids ? TRUE : FALSE;
	m_enable_inner_bounds = a_field[cur_field].has_inner_bound ? TRUE : FALSE;
	m_field_enhanced_checks = a_field[cur_field].enhanced_visibility_checks ? TRUE : FALSE;
	m_density = a_field[cur_field].num_initial_asteroids;
	if (!m_enable_asteroids)
		m_density = 10;

	// set field type
	m_field_type = a_field[cur_field].field_type;
	m_debris_genre = a_field[cur_field].debris_genre;
//	m_debris_species = a_field[cur_field].debris_species;

	m_avg_speed = (int) vm_vec_mag(&a_field[cur_field].vel);
	m_min_x.Format("%.1f", a_field[cur_field].min_bound.xyz.x);
	m_min_y.Format("%.1f", a_field[cur_field].min_bound.xyz.y);
	m_min_z.Format("%.1f", a_field[cur_field].min_bound.xyz.z);
	m_max_x.Format("%.1f", a_field[cur_field].max_bound.xyz.x);
	m_max_y.Format("%.1f", a_field[cur_field].max_bound.xyz.y);
	m_max_z.Format("%.1f", a_field[cur_field].max_bound.xyz.z);

	m_box_min_x.Format("%.1f", a_field[cur_field].inner_min_bound.xyz.x);
	m_box_min_y.Format("%.1f", a_field[cur_field].inner_min_bound.xyz.y);
	m_box_min_z.Format("%.1f", a_field[cur_field].inner_min_bound.xyz.z);
	m_box_max_x.Format("%.1f", a_field[cur_field].inner_max_bound.xyz.x);
	m_box_max_y.Format("%.1f", a_field[cur_field].inner_max_bound.xyz.y);
	m_box_max_z.Format("%.1f", a_field[cur_field].inner_max_bound.xyz.z);

	m_field_targets = a_field[cur_field].target_names;

	UpdateData(FALSE);
	OnEnableAsteroids();

	last_field = cur_field;
}

void asteroid_editor::OnEnableAsteroids() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_DENSITY)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_DENSITY_SPIN)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_AVG_SPEED)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_MIN_X)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_MIN_Y)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_MIN_Z)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_MAX_X)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_MAX_Y)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_MAX_Z)->EnableWindow(m_enable_asteroids);

	GetDlgItem(IDC_ENABLE_INNER_BOX)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_PASSIVE_FIELD)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_ACTIVE_FIELD)->EnableWindow(m_enable_asteroids);

	GetDlgItem(IDC_FIELD_DEBRIS)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_FIELD_ASTEROID)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_SELECT_DEBRIS)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_DEBRIS));
	GetDlgItem(IDC_SELECT_ASTEROID)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_ASTEROID));

	GetDlgItem(IDC_FIELD_TARGET_LIST)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_ADD_FIELD_TARGET)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_REMOVE_FIELD_TARGET)->EnableWindow(m_enable_asteroids);

	((CListBox*)GetDlgItem(IDC_FIELD_TARGET_LIST))->ResetContent();
	if (m_enable_asteroids) {
		for (const auto& target : m_field_targets) {
			((CListBox*)GetDlgItem(IDC_FIELD_TARGET_LIST))->AddString(target.c_str());
		}
	}

	// sorta temporary until multiple fields are really enabled
	((CComboBox*)GetDlgItem(IDC_FIELD_NUM))->Clear();
	GetDlgItem(IDC_FIELD_NUM)->EnableWindow(m_enable_asteroids);
	if (m_enable_asteroids) {
		GetDlgItem(IDC_ADD_FIELD)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_FIELD)->EnableWindow(TRUE);
		((CComboBox*)GetDlgItem(IDC_FIELD_NUM))->AddString("Field 1");
		((CComboBox*)GetDlgItem(IDC_FIELD_NUM))->SetCurSel(0);
	}
	else {
		GetDlgItem(IDC_ADD_FIELD)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_FIELD)->EnableWindow(FALSE);
		((CComboBox*)GetDlgItem(IDC_FIELD_NUM))->SetCurSel(-1);
	}
	UpdateData(FALSE);

	OnEnableInnerBox();
	OnEnableField();
	OnFieldTargetChange();
}

void asteroid_editor::OnCancel()
{
	theApp.record_window_data(&Asteroid_wnd_data, this);
	CDialog::OnCancel();
}

void asteroid_editor::OnClose() 
{
	int z;

	update_init();
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

// enable inner box (asteroid free zone)
// only allowed for active debris field
void asteroid_editor::OnEnableInnerBox() 
{
	UpdateData(TRUE);

	GetDlgItem(IDC_INNER_MIN_X)->EnableWindow(m_enable_inner_bounds && m_enable_asteroids);
	GetDlgItem(IDC_INNER_MAX_X)->EnableWindow(m_enable_inner_bounds && m_enable_asteroids);
	GetDlgItem(IDC_INNER_MIN_Y)->EnableWindow(m_enable_inner_bounds && m_enable_asteroids);
	GetDlgItem(IDC_INNER_MAX_Y)->EnableWindow(m_enable_inner_bounds && m_enable_asteroids);
	GetDlgItem(IDC_INNER_MIN_Z)->EnableWindow(m_enable_inner_bounds && m_enable_asteroids);
	GetDlgItem(IDC_INNER_MAX_Z)->EnableWindow(m_enable_inner_bounds && m_enable_asteroids);
}

void asteroid_editor::OnEnableRangeOverride()
{
	UpdateData(TRUE);
}

// 
void asteroid_editor::OnEnableField() 
{
	// set check in active
	if (m_enable_asteroids) {
		if (m_field_type == FT_ACTIVE) {
			OnActiveField();
		} else {
			OnPassiveField();
		}
	}

	// maybe enable species
	if ( m_enable_asteroids && (m_field_type == FT_PASSIVE) && (m_debris_genre == DG_DEBRIS) ) {
		OnFieldDebris();
	}
}


void asteroid_editor::OnActiveField()
{
	// set field type
	m_field_type = FT_ACTIVE;
	m_debris_genre = DG_ASTEROID;

	// gray out debris
	GetDlgItem(IDC_FIELD_DEBRIS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SELECT_DEBRIS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SELECT_ASTEROID)->EnableWindow(TRUE);

	// force check of asteroid
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_FIELD_DEBRIS))->SetCheck(0);

	// force check of active field
	((CButton*)GetDlgItem(IDC_ACTIVE_FIELD))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_PASSIVE_FIELD))->SetCheck(0);

	// enable inner box
	GetDlgItem(IDC_ENABLE_INNER_BOX)->EnableWindow(TRUE);
}

void asteroid_editor::OnPassiveField()
{
	// set field type
	m_field_type = FT_PASSIVE;

	// acivate ship
	GetDlgItem(IDC_FIELD_DEBRIS)->EnableWindow(TRUE);

	// maybe activate debris
	GetDlgItem(IDC_SELECT_DEBRIS)->EnableWindow(m_debris_genre == DG_DEBRIS);

	// maybe activate asteroid subtype
	GetDlgItem(IDC_SELECT_ASTEROID)->EnableWindow(m_debris_genre == DG_ASTEROID);


	// force check of current debris type
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(m_debris_genre == DG_ASTEROID);
	((CButton*)GetDlgItem(IDC_FIELD_DEBRIS))->SetCheck(m_debris_genre == DG_DEBRIS);

	// force check of passive field
	((CButton*)GetDlgItem(IDC_ACTIVE_FIELD))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_PASSIVE_FIELD))->SetCheck(1);

	// disable inner box
	GetDlgItem(IDC_ENABLE_INNER_BOX)->EnableWindow(FALSE);
}

void asteroid_editor::OnFieldDebris()
{
	// set debris type 
	m_debris_genre = DG_DEBRIS;

	GetDlgItem(IDC_SELECT_DEBRIS)->EnableWindow(TRUE);

	GetDlgItem(IDC_SELECT_ASTEROID)->EnableWindow(FALSE);

	// force check of debris
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_FIELD_DEBRIS))->SetCheck(1);

	// force check of passive field
	((CButton*)GetDlgItem(IDC_ACTIVE_FIELD))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_PASSIVE_FIELD))->SetCheck(1);

}

void asteroid_editor::OnFieldAsteroid()
{
	// set asteroid type 
	m_debris_genre = DG_ASTEROID;

	GetDlgItem(IDC_SELECT_DEBRIS)->EnableWindow(FALSE);


	GetDlgItem(IDC_SELECT_ASTEROID)->EnableWindow(TRUE);

	// force check of asteroid
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_FIELD_DEBRIS))->SetCheck(0);

	// force check of existing field type
	if (m_field_type == FT_ACTIVE) {
		((CButton*)GetDlgItem(IDC_ACTIVE_FIELD))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_PASSIVE_FIELD))->SetCheck(0);
	} else {
		((CButton*)GetDlgItem(IDC_ACTIVE_FIELD))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_PASSIVE_FIELD))->SetCheck(1);
	}
	
}

void asteroid_editor::OnAddField()
{
	m_enable_asteroids = TRUE;
	OnEnableAsteroids();
}

void asteroid_editor::OnRemoveField()
{
	m_enable_asteroids = FALSE;
	OnEnableAsteroids();
}

void asteroid_editor::OnFieldTargetChange()
{
	m_field_target_index = ((CListBox*)GetDlgItem(IDC_FIELD_TARGET_LIST))->GetCurSel();
	GetDlgItem(IDC_REMOVE_FIELD_TARGET)->EnableWindow(m_field_target_index >= 0);
}

void asteroid_editor::OnAddFieldTarget()
{
	SCP_vector<SCP_string> targets;
	CString str;

	// add any ship in the mission
	for (int i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum >= 0)
			targets.push_back(Ships[i].ship_name);
	}

	ListItemChooser dlg(targets);
	if (dlg.DoModal() == IDCANCEL)
		return;

	auto which = dlg.GetChosenIndex();
	if (which < 0)
		return;
	auto new_target = targets[which].c_str();

	// make sure we aren't adding a target that is already in the list
	if (std::find(m_field_targets.begin(), m_field_targets.end(), new_target) != m_field_targets.end())
		return;

	auto targetList = ((CListBox*)GetDlgItem(IDC_FIELD_TARGET_LIST));
	m_field_targets.push_back(new_target);
	targetList->AddString(new_target);

	OnFieldTargetChange();
}

void asteroid_editor::OnRemoveFieldTarget()
{
	// if we don't have an active item
	if (m_field_target_index < 0)
		return;

	// remove the item from the list
	m_field_targets.erase(m_field_targets.begin() + m_field_target_index);
	((CListBox*)GetDlgItem(IDC_FIELD_TARGET_LIST))->DeleteString(m_field_target_index);

	OnFieldTargetChange();
}

void asteroid_editor::OnBnClickedSelectDebris()
{
	// create a list of debris types
	SCP_vector<std::pair<CString, bool>> debris;
	for (size_t i = 0; i < Asteroid_info.size(); i++) {
		if (Asteroid_info[i].type == -1) {
			bool active = false;
			for (size_t j = 0; j < a_field[cur_field].field_debris_type.size(); j++) {
				if (static_cast<int>(i) == a_field[cur_field].field_debris_type[j]) {
					active = true;
					break;
				}
			}
			debris.emplace_back(Asteroid_info[i].name, active);
		}
	}
	
	// display the checklist
	CheckBoxListDlg dlg;
	dlg.SetCaption("Select Debris");
	dlg.SetOptions(debris);
	dlg.DoModal();

	// activate selected debris
	a_field[cur_field].field_debris_type.clear();
	for (int i = 0; i < static_cast<int>(debris.size()); i++) {
		if (dlg.IsChecked(i)) {
			a_field[cur_field].field_debris_type.push_back(get_asteroid_index(debris[i].first.GetString()));
		}
	}
}

void asteroid_editor::OnBnClickedSelectAsteroid()
{
	// create a list of asteroid types
	SCP_vector<std::pair<CString, bool>> asteroids;
	auto list = get_list_valid_asteroid_subtypes();
	for (size_t i = 0; i < list.size(); i++) {
		bool active = false;
		for (size_t j = 0; j < a_field[cur_field].field_asteroid_type.size(); j++) {
			if (list[i] == a_field[cur_field].field_asteroid_type[j]) {
				active = true;
				break;
			}
		}
		asteroids.emplace_back(list[i].c_str(), active);
	}

	// display the checklist
	CheckBoxListDlg dlg;
	dlg.SetCaption("Select Asteroid Types");
	dlg.SetOptions(asteroids);
	dlg.DoModal();

	// activate selected asteroids
	a_field[cur_field].field_asteroid_type.clear();
	for (int i = 0; i < static_cast<int>(asteroids.size()); i++) {
		if (dlg.IsChecked(i)) {
			a_field[cur_field].field_asteroid_type.push_back(asteroids[i].first.GetString());
		}
	}
}
