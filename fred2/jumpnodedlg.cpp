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
#include "JumpnodeDlg.h"
#include "Management.h"
#include "MainFrm.h"
#include "object/object.h"
#include "jumpnode/jumpnode.h"

#define ID_JUMP_NODE_MENU	8000
#define ID_WAYPOINT_MENU	9000

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// waypoint_path_dlg dialog

jumpnode_dlg::jumpnode_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(jumpnode_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(jumpnode_dlg)
	m_name = _T("");
	m_display = _T("");
	m_filename = _T("");
	m_color_r = 0;
	m_color_g = 0;
	m_color_b = 0;
	m_color_a = 0;
	m_hidden = FALSE;
	//}}AFX_DATA_INIT
	bypass_errors = 0;
}

void jumpnode_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(jumpnode_dlg)
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_ALT_NAME, m_display);
	DDX_Text(pDX, IDC_MODEL_FILENAME, m_filename);
	DDX_Text(pDX, IDC_NODE_R, m_color_r);
	DDV_MinMaxInt(pDX, m_color_r, 0, 255);
	DDX_Text(pDX, IDC_NODE_G, m_color_g);
	DDV_MinMaxInt(pDX, m_color_g, 0, 255);
	DDX_Text(pDX, IDC_NODE_B, m_color_b);
	DDV_MinMaxInt(pDX, m_color_b, 0, 255);
	DDX_Text(pDX, IDC_NODE_A, m_color_a);
	DDV_MinMaxInt(pDX, m_color_a, 0, 255);
	DDX_Check(pDX, IDC_NODE_HIDDEN, m_hidden);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(jumpnode_dlg, CDialog)
	//{{AFX_MSG_MAP(jumpnode_dlg)
	ON_BN_CLICKED(IDC_NODE_HIDDEN, OnHidden)
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	ON_EN_KILLFOCUS(IDC_NAME, OnKillfocusName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// jumpnode_dlg message handlers

BOOL jumpnode_dlg::Create()
{
	BOOL r;
	r = CDialog::Create(IDD, Fred_main_wnd);
	initialize_data(1);
	return r;
}

void jumpnode_dlg::OnInitMenu(CMenu* pMenu)
{
	int i;
	SCP_list<CJumpNode>::iterator jnp;
	CMenu *m;

	m = pMenu->GetSubMenu(0);
	clear_menu(m);

	i = 0; 
	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
		m->AppendMenu(MF_ENABLED | MF_STRING, ID_JUMP_NODE_MENU + i, jnp->GetName());
		if (jnp->GetSCPObjectNumber() == cur_object_index) {
			m->CheckMenuItem(ID_JUMP_NODE_MENU + i,  MF_BYCOMMAND | MF_CHECKED);
		}
		i++;

	}

	m->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);

	CDialog::OnInitMenu(pMenu);
}

void jumpnode_dlg::OnOK() {
}

void jumpnode_dlg::OnClose()
{	
	UpdateData(TRUE);
	
	if (update_data()) {
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		bypass_errors = 0;
		return;
	}

	SetWindowPos(Fred_main_wnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
	Fred_main_wnd->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void jumpnode_dlg::initialize_data(int full_update)
{
	int enable = TRUE;

	if (!GetSafeHwnd())
		return;

	if (Objects[cur_object_index].type == OBJ_JUMP_NODE) {
		auto jnp = jumpnode_get_by_objnum(cur_object_index);
		m_name = _T(jnp->GetName());
		m_display = _T(jnp->GetDisplayName());

		int model = jnp->GetModelNumber();
		polymodel* pm = model_get(model);
		m_filename = _T(pm->filename);

		const auto &jn_color = jnp->GetColor();
		m_color_r = jn_color.red;
		m_color_g = jn_color.green;
		m_color_b = jn_color.blue;
		m_color_a = jn_color.alpha;

		m_hidden = (int)jnp->IsHidden();

	} else {
		m_name = _T("");
		m_display = _T("");
		m_filename = _T("");
		m_color_r = 0;
		m_color_g = 0;
		m_color_b = 0;
		m_color_a = 0;
		m_hidden = FALSE;
		enable = FALSE;
	}

	if (full_update)
		UpdateData(FALSE);

	GetDlgItem(IDC_NAME)->EnableWindow(enable);
}

int jumpnode_dlg::update_data()
{
	const char *str;
	char old_name[255];
	int i, z;
	object *ptr;

	if (!GetSafeHwnd())
		return 0;

	if (query_valid_object() && Objects[cur_object_index].type == OBJ_JUMP_NODE) {
		auto jnp = jumpnode_get_by_objnum(cur_object_index);

		for (i=0; i<MAX_WINGS; i++)
		{
			if (!stricmp(Wings[i].name, m_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This jump node name is already being used by a wing\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(jnp->GetName());
				UpdateData(FALSE);
			}
		}

		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
				if (!stricmp(m_name, Ships[ptr->instance].ship_name)) {
					if (bypass_errors)
						return 1;

					bypass_errors = 1;
					z = MessageBox("This jump node name is already being used by a ship\n"
						"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

					if (z == IDCANCEL)
						return -1;

					m_name = _T(jnp->GetName());
					UpdateData(FALSE);
				}
			}

			ptr = GET_NEXT(ptr);
		}

		// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

		for ( i=0; i < (int)Ai_tp_list.size(); i++) {
			if (!stricmp(m_name, Ai_tp_list[i].name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This jump node name is already being used by a target priority group.\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(jnp->GetName());
				UpdateData(FALSE);
			}
		}

		if (find_matching_waypoint_list((LPCSTR) m_name) != NULL)
		{
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("This jump node name is already being used by a waypoint path\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_name = _T(jnp->GetName());
			UpdateData(FALSE);
		}

		if (!stricmp(m_name.Left(1), "<")) {
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("Jump node names not allowed to begin with <\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_name = _T(jnp->GetName());
			UpdateData(FALSE);
		}

		CJumpNode* found = jumpnode_get_by_name(m_name);
		if(found != NULL && &(*jnp) != found)
		{
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("This jump node name is already being used by another jump node\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_name = _T(jnp->GetName());
			UpdateData(FALSE);
		}
		
		strcpy_s(old_name, jnp->GetName());
		jnp->SetName((LPCSTR) m_name);
		jnp->SetDisplayName((LPCSTR) m_display);

		int model = jnp->GetModelNumber();
		polymodel* pm = model_get(model);
		CString old_filename = _T(pm->filename);

		// Does that pof file exist?
		if (cf_exists_full((LPCSTR)m_filename, CF_TYPE_MODELS)) {
			if (stricmp((LPCSTR)m_filename, JN_DEFAULT_MODEL)) {
				jnp->SetModel((LPCSTR)m_filename);
			}
		} else {
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("This jump node pof file does not exist\n"
						   "Press OK to restore old file", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_filename = old_filename;
			UpdateData(FALSE);
		}

		jnp->SetAlphaColor(m_color_r, m_color_g, m_color_b, m_color_a);

		jnp->SetVisibility(!(bool)m_hidden);
		
		str = (LPCTSTR) m_name;
		if (strcmp(old_name, str)) {
			update_sexp_references(old_name, str);
		}
		
	}

	update_map_window();

	return 0;
}

BOOL jumpnode_dlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int id, point;
	object *ptr;

	id = LOWORD(wParam);
	if ((id >= ID_JUMP_NODE_MENU) && (id < ID_JUMP_NODE_MENU + (int) Jump_nodes.size())) {
		if (!update_data()) {
			point = id - ID_JUMP_NODE_MENU;
			unmark_all();
			ptr = GET_FIRST(&obj_used_list);
			while ((ptr != END_OF_LIST(&obj_used_list)) && (point > -1)) {
				if (ptr->type == OBJ_JUMP_NODE) {
					if (point == 0) {
						mark_object(OBJ_INDEX(ptr));
					}
					point--; 
				}

				ptr = GET_NEXT(ptr);
			}

			return 1;
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void jumpnode_dlg::OnHidden()
{
	if (m_hidden == 1)
		m_hidden = 0;
	else
		m_hidden = 1;

	((CButton*)GetDlgItem(IDC_NODE_HIDDEN))->SetCheck(m_hidden);
}

void jumpnode_dlg::OnKillfocusName()
{
	char buffer[NAME_LENGTH];

	// Note, in this dialog, UpdateData(TRUE) is not used to move data from controls to variables until the dialog is closed, so we edit the control text directly

	// grab the name
	GetDlgItemText(IDC_NAME, buffer, NAME_LENGTH);

	// if this name has a hash, truncate it for the display name
	if (get_pointer_to_first_hash_symbol(buffer))
		end_string_at_first_hash_symbol(buffer);

	// set the display name derived from this name
	SetDlgItemText(IDC_ALT_NAME, buffer);
}
