#include "stdafx.h"
#include "FRED.h"
#include "propdlg.h"
#include "Management.h"
#include "MainFrm.h"
#include "object/object.h"
#include "prop/prop.h"

prop_dlg::prop_dlg(CWnd* pParent /*=NULL*/) : CDialog(prop_dlg::IDD, pParent)
{
	m_name = _T("");
	bypass_errors = 0;
}

BOOL prop_dlg::Create()
{
	BOOL r = CDialog::Create(IDD, Fred_main_wnd);
	initialize_data(1);
	return r;
}

void prop_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROP_NAME, m_name);
	DDX_Control(pDX, IDC_PROP_FLAGS, m_flags_list);
}

BEGIN_MESSAGE_MAP(prop_dlg, CDialog)
ON_WM_CLOSE()
ON_WM_SIZE()
ON_BN_CLICKED(IDC_PROP_NEXT, OnPropNext)
ON_BN_CLICKED(IDC_PROP_PREV, OnPropPrev)
END_MESSAGE_MAP()

void prop_dlg::OnClose()
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

BOOL prop_dlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// In the future, handle prop-specific menu commands here.

	return CDialog::OnCommand(wParam, lParam);
}

void prop_dlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (!GetSafeHwnd())
		return;

	// Resize/move the flag list box to fill new space
	CWnd* flagList = GetDlgItem(IDC_PROP_FLAGS);
	if (flagList) {
		CRect r;
		flagList->GetWindowRect(&r);
		ScreenToClient(&r);
		r.right = cx - 10;
		r.bottom = cy - 10;
		flagList->MoveWindow(&r);
	}
}


void prop_dlg::initialize_data(int full_update)
{
	int enable = TRUE;
	
	if (!GetSafeHwnd())
		return;

	// Check if we have a selected prop
	if (query_valid_object() && Objects[cur_object_index].type == OBJ_PROP) {
		auto& prp = Props[Objects[cur_object_index].instance];
		m_name = _T(prp.prop_name);
	} else {
		// No valid prop selected; disable editing fields
		m_name = _T("");
		enable = FALSE;
	}

	m_flags_list.ResetContent();

	for (size_t i = 0; i < Num_parse_prop_flags; ++i) {
		auto& def = Parse_prop_flags[i];
		auto& desc = Parse_prop_flag_descriptions[i];

		CString display_str;
		display_str.Format("%s (%s)", def.name, desc.flag_desc);

		int idx = m_flags_list.AddString(display_str);
		m_flags_list.SetItemData(idx, static_cast<DWORD_PTR>(i));
	}

	// Set selection states if a valid prop is selected
	if (query_valid_object() && Objects[cur_object_index].type == OBJ_PROP) {

		for (int i = 0; i < m_flags_list.GetCount(); ++i) {
			size_t flag_index = static_cast<size_t>(m_flags_list.GetItemData(i));
			if (flag_index >= Num_parse_prop_flags)
				continue;
			auto& def = Parse_prop_flags[flag_index];
			
			// Handle each flag manually since they may not always match 1:1 from mission object flags to parsed object or true object flags
			if (!stricmp(def.name, "no_collide")) {
				if (!Objects[cur_object_index].flags[Object::Object_Flags::Collides]) {
					m_flags_list.SetSel(i);
				}
			}
		}
	}

	if (full_update)
		UpdateData(FALSE);

	GetDlgItem(IDC_PROP_NAME)->EnableWindow(enable);
	m_flags_list.EnableWindow(enable);
}

int prop_dlg::update_data()
{
	if (!GetSafeHwnd())
		return 0;

	if (query_valid_object() && Objects[cur_object_index].type == OBJ_PROP) {
		int this_instance = Objects[cur_object_index].instance;
		auto& prp = Props[this_instance];

		m_name.TrimLeft();
		m_name.TrimRight();

		for (size_t i = 0; i < Props.size(); ++i) {
			if ((int)i == this_instance)
				continue; // skip self

			if (!stricmp(m_name, Props[i].prop_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				int z = MessageBox("This prop name is already being used by another prop\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(prp.prop_name);
				UpdateData(FALSE);
				return 1;
			}
		}
		
		// Passed name validation
		strcpy_s(prp.prop_name, m_name);

		prp.flags.reset(); // Clear all flags

		for (int i = 0; i < m_flags_list.GetCount(); ++i) {
			size_t flag_index = static_cast<size_t>(m_flags_list.GetItemData(i));
			if (flag_index >= Num_parse_prop_flags)
				continue;
			auto& def = Parse_prop_flags[flag_index];

			bool selected = m_flags_list.GetSel(i);

			// Handle each flag manually
			if (!stricmp(def.name, "no_collide")) {
				Objects[cur_object_index].flags.set(Object::Object_Flags::Collides, !selected);
			}
		}
	}

	update_map_window();
	return 0;
}

void prop_dlg::select_prop_from_object_list(object* start, bool forward)
{
	object* ptr = start;

	// Search forward or backward
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_PROP) {
			unmark_all();
			mark_object(OBJ_INDEX(ptr));
			set_cur_object_index(OBJ_INDEX(ptr));
			initialize_data(1);
			return;
		}
		ptr = forward ? GET_NEXT(ptr) : GET_PREV(ptr);
	}

	// Wraparound search
	ptr = forward ? GET_FIRST(&obj_used_list) : GET_LAST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_PROP) {
			unmark_all();
			mark_object(OBJ_INDEX(ptr));
			set_cur_object_index(OBJ_INDEX(ptr));
			initialize_data(1);
			return;
		}
		ptr = forward ? GET_NEXT(ptr) : GET_PREV(ptr);
	}
}

void prop_dlg::OnPropNext()
{
	UpdateData(TRUE);
	if (update_data() < 0)
		return; // Only abort if user cancelled (e.g. name conflict they didn't resolve)

	select_prop_from_object_list(GET_NEXT(&Objects[cur_object_index]), true);
}

void prop_dlg::OnPropPrev()
{
	UpdateData(TRUE);
	if (update_data() < 0)
		return; // Only abort if user cancelled (e.g. name conflict they didn't resolve)

	select_prop_from_object_list(GET_PREV(&Objects[cur_object_index]), false);
}

void prop_dlg::OnOK() {}
