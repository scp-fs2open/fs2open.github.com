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
#include "WaypointPathDlg.h"
#include "Management.h"
#include "MainFrm.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "globalincs/linklist.h"
#include "ship/ship.h"
#include "ai/aigoals.h"
#include "starfield/starfield.h"
#include "jumpnode/jumpnode.h"
#include "iff_defs/iff_defs.h"

#define ID_JUMP_NODE_MENU	8000
#define ID_WAYPOINT_MENU	9000

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// waypoint_path_dlg dialog

waypoint_path_dlg::waypoint_path_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(waypoint_path_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(waypoint_path_dlg)
	m_name = _T("");
	//}}AFX_DATA_INIT
	bypass_errors = 0;
}

void waypoint_path_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(waypoint_path_dlg)
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(waypoint_path_dlg, CDialog)
	//{{AFX_MSG_MAP(waypoint_path_dlg)
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// waypoint_path_dlg message handlers

BOOL waypoint_path_dlg::Create()
{
	BOOL r;
	r = CDialog::Create(IDD, Fred_main_wnd);
	initialize_data(1);
	return r;
}

void waypoint_path_dlg::OnInitMenu(CMenu* pMenu)
{
	int i;
	CMenu *m;

	m = pMenu->GetSubMenu(0);
	clear_menu(m);
	for (i=0; i<Num_waypoint_lists; i++) {
		if (Waypoint_lists[i].count) {
			m->AppendMenu(MF_ENABLED | MF_STRING, ID_WAYPOINT_MENU + i, Waypoint_lists[i].name);
		}
	}

	i = 0; 
	for ( jump_node *jnp = (jump_node *)Jump_nodes.get_first(); !Jump_nodes.is_end(jnp); jnp = (jump_node *)jnp->get_next() ) {
		m->AppendMenu(MF_ENABLED | MF_STRING, ID_JUMP_NODE_MENU + i, jnp->get_name_ptr());
		if (jnp->get_objnum() == cur_object_index) {
			m->CheckMenuItem(ID_JUMP_NODE_MENU + i,  MF_BYCOMMAND | MF_CHECKED);
		}
		i++;

	}

	m->DeleteMenu(ID_PLACEHOLDER, MF_BYCOMMAND);
	if (cur_waypoint_list != -1)
		m->CheckMenuItem(ID_WAYPOINT_MENU + cur_waypoint_list, MF_BYCOMMAND | MF_CHECKED);

	CDialog::OnInitMenu(pMenu);
}

void waypoint_path_dlg::OnOK()
{
}

void waypoint_path_dlg::OnClose() 
{
	if (update_data()) {
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		bypass_errors = 0;
		return;
	}

	SetWindowPos(Fred_main_wnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
	Fred_main_wnd->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void waypoint_path_dlg::initialize_data(int full_update)
{
	int enable = TRUE;

	if (!GetSafeHwnd())
		return;

	if (query_valid_object() && Objects[cur_object_index].type == OBJ_WAYPOINT)
		Assert(cur_waypoint_list == (Objects[cur_object_index].instance / 65536));

	if (cur_waypoint_list >= 0) {
		m_name = _T(Waypoint_lists[cur_waypoint_list].name);

	} else if (Objects[cur_object_index].type == OBJ_JUMP_NODE) {
		m_name = _T(Objects[cur_object_index].jnp->get_name_ptr());

	} else {
		m_name = _T("");
		enable = FALSE;
	}

	if (full_update)
		UpdateData(FALSE);

	GetDlgItem(IDC_NAME)->EnableWindow(enable);
}

int waypoint_path_dlg::update_data(int redraw)
{
	char *str, old_name[255];
	int i, z;
	object *ptr;

	if (!GetSafeHwnd())
		return 0;

	UpdateData(TRUE);
	UpdateData(TRUE);

	if (query_valid_object() && Objects[cur_object_index].type == OBJ_WAYPOINT)
		Assert(cur_waypoint_list == (Objects[cur_object_index].instance / 65536));

	if (cur_waypoint_list >= 0) {
		for (i=0; i<MAX_WINGS; i++)
		{
			if (!stricmp(Wings[i].name, m_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This waypoint path name is already being used by a wing\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(Waypoint_lists[cur_waypoint_list].name);
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
					z = MessageBox("This waypoint path name is already being used by a ship\n"
						"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

					if (z == IDCANCEL)
						return -1;

					m_name = _T(Waypoint_lists[cur_waypoint_list].name);
					UpdateData(FALSE);
				}
			}

			ptr = GET_NEXT(ptr);
		}

		for (i=0; i<Num_iffs; i++) {
			if (!stricmp(m_name, Iff_info[i].iff_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This waypoint path name is already being used by a team.\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(Waypoint_lists[cur_waypoint_list].name);
				UpdateData(FALSE);
			}
		}

		for ( i=0; i < (int)Ai_tp_list.size(); i++) {
			if (!stricmp(m_name, Ai_tp_list[i].name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This waypoint path name is already being used by a target priority group.\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(Waypoint_lists[cur_waypoint_list].name);
				UpdateData(FALSE);
			}
		}

		for (i=0; i<MAX_WAYPOINT_LISTS; i++) {
			if (Waypoint_lists[i].count && !stricmp(Waypoint_lists[i].name, m_name) && (i != cur_waypoint_list)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This waypoint path name is already being used by another waypoint path\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(Waypoint_lists[cur_waypoint_list].name);
				UpdateData(FALSE);
			}
		}

		if(jumpnode_get_by_name(m_name) != NULL)
		{
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("This waypoint path name is already being used by a jump node\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_name = _T(Waypoint_lists[cur_waypoint_list].name);
			UpdateData(FALSE);
		}

		if (!stricmp(m_name.Left(1), "<")) {
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("Waypoint names not allowed to begin with <\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_name = _T(Waypoint_lists[cur_waypoint_list].name);
			UpdateData(FALSE);
		}


		strcpy_s(old_name, Waypoint_lists[cur_waypoint_list].name);
		string_copy(Waypoint_lists[cur_waypoint_list].name, m_name, NAME_LENGTH, 1);

		str = (char *) (LPCTSTR) m_name;
		if (strcmp(old_name, str)) {
			update_sexp_references(old_name, str);
			ai_update_goal_references(REF_TYPE_WAYPOINT, old_name, str);
			update_texture_replacements(old_name, str);
		}

	} else if (Objects[cur_object_index].type == OBJ_JUMP_NODE) {
		jump_node *jnp = Objects[cur_object_index].jnp;

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

				m_name = _T(jnp->get_name_ptr());
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

					m_name = _T(jnp->get_name_ptr());
					UpdateData(FALSE);
				}
			}

			ptr = GET_NEXT(ptr);
		}

		for (i=0; i<Num_iffs; i++) {
			if (!stricmp(m_name, Iff_info[i].iff_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This jump node name is already being used by a team.\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(jnp->get_name_ptr());
				UpdateData(FALSE);
			}
		}

		for ( i=0; i < (int)Ai_tp_list.size(); i++) {
			if (!stricmp(m_name, Ai_tp_list[i].name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This jump node name is already being used by a target priority group.\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(jnp->get_name_ptr());
				UpdateData(FALSE);
			}
		}

		for (i=0; i<MAX_WAYPOINT_LISTS; i++)
		{
			if (Waypoint_lists[i].count && !stricmp(Waypoint_lists[i].name, m_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This jump node name is already being used by a waypoint path\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(jnp->get_name_ptr());
				UpdateData(FALSE);
			}
		}

		if (!stricmp(m_name.Left(1), "<")) {
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("Jump node names not allowed to begin with <\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_name = _T(jnp->get_name_ptr());
			UpdateData(FALSE);
		}

		strcpy_s(old_name, jnp->get_name_ptr());
		string_copy(jnp->get_name_ptr(), m_name, NAME_LENGTH, 1);

		if(jumpnode_check_for_duplicates())
		{
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("This jump node name is already being used by another jump node\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			strcpy(jnp->get_name_ptr(), old_name);

			if (z == IDCANCEL)
				return -1;

			m_name = _T(old_name);
			UpdateData(FALSE);
		}
		
		str = (char *) (LPCTSTR) m_name;
		if (strcmp(old_name, str)) {
			update_sexp_references(old_name, str);
		}
		
	}

	if (redraw)
		update_map_window();

	return 0;
}

BOOL waypoint_path_dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id, point;
	object *ptr;

	id = LOWORD(wParam);
	if ((id >= ID_WAYPOINT_MENU) && (id < ID_WAYPOINT_MENU + MAX_WAYPOINT_LISTS)) {
		if (!update_data()) {
			point = id - ID_WAYPOINT_MENU;
			unmark_all();
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_WAYPOINT)
					if ((ptr->instance / 65536) == point)
						mark_object(OBJ_INDEX(ptr));

				ptr = GET_NEXT(ptr);
			}

			return 1;
		}
	}

	if ((id >= ID_JUMP_NODE_MENU) && (id < ID_JUMP_NODE_MENU + Jump_nodes.get_num_elements())) {
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
