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
#include "starfield/starfield.h"
#include "FREDDoc.h"
#include "debris/debris.h"	//	Asteroid stuff.
#include "species_defs/species_defs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_FIELD_MENU 9000

// helps in looping over combo boxes
int Dlg_id[MAX_ACTIVE_DEBRIS_TYPES] = {IDC_SHIP_DEBRIS1, IDC_SHIP_DEBRIS2, IDC_SHIP_DEBRIS3};


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
	DDX_Check(pDX, IDC_ENABLE_ASTEROIDS, m_enable_asteroids);
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
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(asteroid_editor, CDialog)
	//{{AFX_MSG_MAP(asteroid_editor)
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_ENABLE_ASTEROIDS, OnEnableAsteroids)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ENABLE_INNER_BOX, OnEnableInnerBox)
	ON_BN_CLICKED(IDC_PASSIVE_FIELD, OnPassiveField)
	ON_BN_CLICKED(IDC_FIELD_SHIP, OnFieldShip)
	ON_BN_CLICKED(IDC_ACTIVE_FIELD, OnActiveField)
	ON_BN_CLICKED(IDC_FIELD_ASTEROID, OnFieldAsteroid)
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

		// check if passive, ship debris field, at least one speceis selected
		if (a_field[0].field_type == FT_PASSIVE) {
			if (a_field[0].debris_genre == DG_SHIP) {
				if ( (a_field[0].field_debris_type[0] == -1) && (a_field[0].field_debris_type[1] == -1) && (a_field[0].field_debris_type[2] == -1) ) {
					MessageBox("You must choose one or more types of ship debris");
					return 0;
				}
			}
		}

		// check at least one asteroid subtype is selected
		if (a_field[0].debris_genre == DG_ASTEROID) {
			if ( (a_field[0].field_debris_type[0] == -1) && (a_field[0].field_debris_type[1] == -1) && (a_field[0].field_debris_type[2] == -1) ) {
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
	for (i=0; i<1 /*MAX_ASTEROID_FIELDS*/; i++)
		Asteroid_field = a_field[i];

	update_map_window();
	theApp.record_window_data(&Asteroid_wnd_data, this);
	CDialog::OnOK();
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
	int num_asteroids, idx, cur_choice;

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
		if ( (m_field_type == FT_PASSIVE) && (m_debris_genre == DG_SHIP) ) {
			// we should have ship debris
			for (idx=0; idx<MAX_ACTIVE_DEBRIS_TYPES; idx++) {
				// loop over combo boxes, store the item data of the cur selection, -1 in no cur selection
				int cur_sel = ((CComboBox*)GetDlgItem(Dlg_id[idx]))->GetCurSel();
				if (cur_sel != CB_ERR) {
					cur_choice = ((CComboBox*)GetDlgItem(Dlg_id[idx]))->GetItemData(cur_sel);
				} else {
					cur_choice = -1;
				}
				MODIFY(a_field[cur_field].field_debris_type[idx], cur_choice);
			}
		}

		if ( m_debris_genre == DG_ASTEROID ) {
			if ( ((CButton *)GetDlgItem(IDC_SUBTYPE1))->GetCheck() == 1) {
				cur_choice = 1;
			} else {
				cur_choice = -1;
			}
			MODIFY(a_field[cur_field].field_debris_type[0], cur_choice);


			if ( ((CButton *)GetDlgItem(IDC_SUBTYPE2))->GetCheck() == 1) {
				cur_choice = 1;
			} else {
				cur_choice = -1;
			}
			MODIFY(a_field[cur_field].field_debris_type[1], cur_choice);


			if ( ((CButton *)GetDlgItem(IDC_SUBTYPE3))->GetCheck() == 1) {
				cur_choice = 1;
			} else {
				cur_choice = -1;
			}
			MODIFY(a_field[cur_field].field_debris_type[2], cur_choice);
		}

		MODIFY(a_field[last_field].has_inner_bound, m_enable_inner_bounds);

		MODIFY(a_field[last_field].inner_min_bound.xyz.x, (float) atof(m_box_min_x));
		MODIFY(a_field[last_field].inner_min_bound.xyz.y, (float) atof(m_box_min_y));
		MODIFY(a_field[last_field].inner_min_bound.xyz.z, (float) atof(m_box_min_z));
		MODIFY(a_field[last_field].inner_max_bound.xyz.x, (float) atof(m_box_max_x));
		MODIFY(a_field[last_field].inner_max_bound.xyz.y, (float) atof(m_box_max_y));
		MODIFY(a_field[last_field].inner_max_bound.xyz.z, (float) atof(m_box_max_z));
	}

	Assert(cur_field >= 0);
	// get from temp asteroid field into class
	m_enable_asteroids = a_field[cur_field].num_initial_asteroids ? TRUE : FALSE;
	m_enable_inner_bounds = a_field[cur_field].has_inner_bound ? TRUE : FALSE;
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

	// set up combo boxes
	uint i;
	int j, k, index, box_index;

	// add "None" to each box
	for (k = 0; k < MAX_ACTIVE_DEBRIS_TYPES; k++)
	{
		box_index = ((CComboBox*)GetDlgItem(Dlg_id[k]))->AddString("None");
		((CComboBox*)GetDlgItem(Dlg_id[k]))->SetItemData(box_index, static_cast<DWORD_PTR>(-1));
	}

	// now add each kind of debris to each box
	CString name;
	char *debris_size[NUM_DEBRIS_SIZES] = { "Small", "Medium", "Large" };

	// each species
	for (i = 0; i < Species_info.size(); i++)
	{
		// each size
		for (j = 0; j < NUM_DEBRIS_SIZES; j++)
		{
			name = CString(Species_info[i].species_name) + " " + debris_size[j];
			index = NUM_DEBRIS_SIZES + ((i * NUM_DEBRIS_SIZES) + j);

			// each box
			for (k = 0; k < MAX_ACTIVE_DEBRIS_TYPES; k++)
			{
				box_index = ((CComboBox*)GetDlgItem(Dlg_id[k]))->AddString(name);
				((CComboBox*)GetDlgItem(Dlg_id[k]))->SetItemData(box_index, index);
			}
		}
	}

	// set active debris type for each combo box
	int box_count, cur_box_data;
	for (idx=0;idx<MAX_ACTIVE_DEBRIS_TYPES; idx++) {
		// Only set selection if not "None"
		if (a_field[cur_field].field_debris_type[idx] != -1) {
			box_count = ((CComboBox*)GetDlgItem(Dlg_id[idx]))->GetCount();
			for (box_index=0; box_index<box_count; box_index++) {
				cur_box_data = ((CComboBox*)GetDlgItem(Dlg_id[idx]))->GetItemData(box_index);
				if (cur_box_data == a_field[cur_field].field_debris_type[idx]) {
					// set cur sel
					((CComboBox*)GetDlgItem(Dlg_id[idx]))->SetCurSel(box_index);
					break;
				}
			}
		}
	}

	// set up asteroid subtype checkboxes
	((CButton*)GetDlgItem(IDC_SUBTYPE1))->SetCheck(a_field[cur_field].field_debris_type[0] == 1);
	((CButton*)GetDlgItem(IDC_SUBTYPE2))->SetCheck(a_field[cur_field].field_debris_type[1] == 1);
	((CButton*)GetDlgItem(IDC_SUBTYPE3))->SetCheck(a_field[cur_field].field_debris_type[2] == 1);


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

	GetDlgItem(IDC_FIELD_SHIP)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_FIELD_ASTEROID)->EnableWindow(m_enable_asteroids);
	GetDlgItem(IDC_SHIP_DEBRIS1)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_SHIP));
	GetDlgItem(IDC_SHIP_DEBRIS2)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_SHIP));
	GetDlgItem(IDC_SHIP_DEBRIS3)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_SHIP));
	GetDlgItem(IDC_SUBTYPE1)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_ASTEROID));
	GetDlgItem(IDC_SUBTYPE2)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_ASTEROID));
	GetDlgItem(IDC_SUBTYPE3)->EnableWindow(m_enable_asteroids && (Asteroid_field.debris_genre == DG_ASTEROID));

	OnEnableInnerBox();
	OnEnableField();
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
	if ( m_enable_asteroids && (m_field_type == FT_PASSIVE) && (m_debris_genre == DG_SHIP) ) {
		OnFieldShip();
	}
}


void asteroid_editor::OnActiveField()
{
	// set field type
	m_field_type = FT_ACTIVE;
	m_debris_genre = DG_ASTEROID;

	// gray out ship and species
	GetDlgItem(IDC_FIELD_SHIP)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHIP_DEBRIS1)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHIP_DEBRIS2)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHIP_DEBRIS3)->EnableWindow(FALSE);
	GetDlgItem(IDC_SUBTYPE1)->EnableWindow(TRUE);
	GetDlgItem(IDC_SUBTYPE2)->EnableWindow(TRUE);
	GetDlgItem(IDC_SUBTYPE3)->EnableWindow(TRUE);

	// force check of asteroid
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_FIELD_SHIP))->SetCheck(0);

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
	GetDlgItem(IDC_FIELD_SHIP)->EnableWindow(TRUE);

	// maybe activate species
	GetDlgItem(IDC_SHIP_DEBRIS1)->EnableWindow(m_debris_genre == DG_SHIP);
	GetDlgItem(IDC_SHIP_DEBRIS2)->EnableWindow(m_debris_genre == DG_SHIP);
	GetDlgItem(IDC_SHIP_DEBRIS3)->EnableWindow(m_debris_genre == DG_SHIP);

	// maybe activate asteroid subtype
	GetDlgItem(IDC_SUBTYPE1)->EnableWindow(m_debris_genre == DG_ASTEROID);
	GetDlgItem(IDC_SUBTYPE2)->EnableWindow(m_debris_genre == DG_ASTEROID);
	GetDlgItem(IDC_SUBTYPE3)->EnableWindow(m_debris_genre == DG_ASTEROID);


	// force check of current debris type
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(m_debris_genre == DG_ASTEROID);
	((CButton*)GetDlgItem(IDC_FIELD_SHIP))->SetCheck(m_debris_genre == DG_SHIP);

	// force check of passive field
	((CButton*)GetDlgItem(IDC_ACTIVE_FIELD))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_PASSIVE_FIELD))->SetCheck(1);

	// disable inner box
	GetDlgItem(IDC_ENABLE_INNER_BOX)->EnableWindow(FALSE);
}

void asteroid_editor::OnFieldShip()
{
	// set debris type 
	m_debris_genre = DG_SHIP;

	GetDlgItem(IDC_SHIP_DEBRIS1)->EnableWindow(TRUE);
	GetDlgItem(IDC_SHIP_DEBRIS2)->EnableWindow(TRUE);
	GetDlgItem(IDC_SHIP_DEBRIS3)->EnableWindow(TRUE);

	GetDlgItem(IDC_SUBTYPE1)->EnableWindow(FALSE);
	GetDlgItem(IDC_SUBTYPE2)->EnableWindow(FALSE);
	GetDlgItem(IDC_SUBTYPE3)->EnableWindow(FALSE);

	// force check of ship
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(0);
	((CButton*)GetDlgItem(IDC_FIELD_SHIP))->SetCheck(1);

}

void asteroid_editor::OnFieldAsteroid()
{
	// set debris type 
	m_debris_genre = DG_ASTEROID;

	GetDlgItem(IDC_SHIP_DEBRIS1)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHIP_DEBRIS2)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHIP_DEBRIS3)->EnableWindow(FALSE);


	GetDlgItem(IDC_SUBTYPE1)->EnableWindow(TRUE);
	GetDlgItem(IDC_SUBTYPE2)->EnableWindow(TRUE);
	GetDlgItem(IDC_SUBTYPE3)->EnableWindow(TRUE);

	// force check of asteroid
	((CButton*)GetDlgItem(IDC_FIELD_ASTEROID))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_FIELD_SHIP))->SetCheck(0);
}
