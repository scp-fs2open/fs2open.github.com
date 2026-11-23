#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "propdlg.h"
#include "Management.h"
#include "MainFrm.h"
#include "object/object.h"
#include "parse/sexp.h"
#include "prop/prop.h"

prop_dlg::prop_dlg(CWnd* pParent /*=NULL*/) : CDialog(prop_dlg::IDD, pParent)
{
	m_name = _T("");
	bypass_errors = 0;
	m_spawn_delay = 0;
	m_despawn_delay = 0;
	m_cues_modified = 0;
	select_sexp_node = -1;
}

BOOL prop_dlg::Create()
{
	BOOL r = CDialog::Create(IDD, Fred_main_wnd);
	m_spawn_tree._model.modified = &m_cues_modified;
	m_spawn_tree.setup((CEdit*)GetDlgItem(IDC_HELP_BOX));
	m_despawn_tree._model.modified = &m_cues_modified;
	m_despawn_tree.setup();
	initialize_data(1);
	return r;
}

void prop_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROP_NAME, m_name);
	DDX_Control(pDX, IDC_PROP_FLAGS, m_flags_list);
	DDX_Control(pDX, IDC_PROP_SPAWN_TREE, m_spawn_tree);
	DDX_Control(pDX, IDC_PROP_DESPAWN_TREE, m_despawn_tree);
	DDX_Text(pDX, IDC_PROP_SPAWN_DELAY, m_spawn_delay);
	DDX_Text(pDX, IDC_PROP_DESPAWN_DELAY, m_despawn_delay);
}

BEGIN_MESSAGE_MAP(prop_dlg, CDialog)
ON_WM_CLOSE()
ON_WM_SIZE()
ON_BN_CLICKED(IDC_PROP_NEXT, OnPropNext)
ON_BN_CLICKED(IDC_PROP_PREV, OnPropPrev)
ON_NOTIFY(NM_RCLICK, IDC_PROP_SPAWN_TREE, OnRclickSpawnTree)
ON_NOTIFY(NM_RCLICK, IDC_PROP_DESPAWN_TREE, OnRclickDespawnTree)
ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_PROP_SPAWN_TREE, OnBeginlabeleditSpawnTree)
ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_PROP_DESPAWN_TREE, OnBeginlabeleditDespawnTree)
ON_NOTIFY(TVN_ENDLABELEDIT, IDC_PROP_SPAWN_TREE, OnEndlabeleditSpawnTree)
ON_NOTIFY(TVN_ENDLABELEDIT, IDC_PROP_DESPAWN_TREE, OnEndlabeleditDespawnTree)
ON_NOTIFY(TVN_SELCHANGED, IDC_PROP_SPAWN_TREE, OnSelchangedSpawnTree)
ON_NOTIFY(TVN_SELCHANGED, IDC_PROP_DESPAWN_TREE, OnSelchangedDespawnTree)
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

	m_spawn_tree.select_sexp_node = m_despawn_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;

	// Check if we have a selected prop
	if (query_valid_object() && Objects[cur_object_index].type == OBJ_PROP) {
		auto prp = prop_id_lookup(Objects[cur_object_index].instance);
		m_name = _T(prp->prop_name);
		m_spawn_delay = prp->spawn_delay;
		m_despawn_delay = prp->despawn_delay;
		m_spawn_tree.load_tree(prp->spawn_cue);
		m_despawn_tree.load_tree(prp->despawn_cue, "false");

		if (m_spawn_tree.select_sexp_node != -1)
			m_spawn_tree.hilite_item(m_spawn_tree.select_sexp_node);
		else if (m_despawn_tree.select_sexp_node != -1)
			m_despawn_tree.hilite_item(m_despawn_tree.select_sexp_node);
	} else {
		// No valid prop selected; disable editing fields
		m_name = _T("");
		m_spawn_delay = 0;
		m_despawn_delay = 0;
		m_spawn_tree.clear_tree();
		m_spawn_tree.DeleteAllItems();
		m_despawn_tree.clear_tree();
		m_despawn_tree.DeleteAllItems();
		enable = FALSE;
	}
	m_cues_modified = 0;

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
	GetDlgItem(IDC_PROP_SPAWN_DELAY)->EnableWindow(enable);
	GetDlgItem(IDC_PROP_DESPAWN_DELAY)->EnableWindow(enable);
	m_spawn_tree.EnableWindow(enable);
	m_despawn_tree.EnableWindow(enable);
}

int prop_dlg::update_data(int redraw)
{
	if (!GetSafeHwnd())
		return 0;

	UpdateData(TRUE);

	if (query_valid_object() && Objects[cur_object_index].type == OBJ_PROP) {
		int this_instance = Objects[cur_object_index].instance;
		auto prp = prop_id_lookup(this_instance);

		m_name.TrimLeft();
		m_name.TrimRight();

		for (size_t i = 0; i < Props.size(); ++i) {
			if (static_cast<int>(i) == this_instance)
				continue; // skip self

			if (!Props[i].has_value()) {
				continue;
			}

			if (!stricmp(m_name, Props[i].value().prop_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				int z = MessageBox("This prop name is already being used by another prop\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_name = _T(prp->prop_name);
				UpdateData(FALSE);
				break;
			}
		}
		
		// Passed name validation
		strcpy_s(prp->prop_name, m_name);

		prp->flags.reset(); // Clear all flags

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

		int spawn_delay = (m_spawn_delay > 0) ? m_spawn_delay : 0;
		int despawn_delay = (m_despawn_delay > 0) ? m_despawn_delay : 0;
		if (spawn_delay != prp->spawn_delay || despawn_delay != prp->despawn_delay) {
			prp->spawn_delay = spawn_delay;
			prp->despawn_delay = despawn_delay;
			set_modified();
		}

		// a tree that was never edited may not hold this prop's cue, so only an edit is written back
		if (m_cues_modified) {
			if (prp->spawn_cue >= 0)
				free_sexp2(prp->spawn_cue);
			prp->spawn_cue = m_spawn_tree._model.save_tree();

			if (prp->despawn_cue >= 0)
				free_sexp2(prp->despawn_cue);
			prp->despawn_cue = m_despawn_tree._model.save_tree();

			m_cues_modified = 0;
			set_modified();
		}
	}

	bypass_errors = 0;

	if (redraw)
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

void prop_dlg::OnOK()
{
	// Enter ends a label edit in a cue tree; moving the focus away and back commits it
	CWnd* w = GetFocus();
	if (w) {
		HWND h = w->m_hWnd;
		GetDlgItem(IDC_PROP_NAME)->SetFocus();
		::SetFocus(h);
	}
}

void prop_dlg::OnRclickSpawnTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	m_spawn_tree.right_clicked();
	*pResult = 0;
}

void prop_dlg::OnRclickDespawnTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	m_despawn_tree.right_clicked();
	*pResult = 0;
}

void prop_dlg::OnBeginlabeleditSpawnTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_spawn_tree.edit_label(pTVDispInfo->item.hItem) == 1) {
		*pResult = 0;
		m_cues_modified = 1;
	} else {
		*pResult = 1;
	}
}

void prop_dlg::OnBeginlabeleditDespawnTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_despawn_tree.edit_label(pTVDispInfo->item.hItem) == 1) {
		*pResult = 0;
		m_cues_modified = 1;
	} else {
		*pResult = 1;
	}
}

void prop_dlg::OnEndlabeleditSpawnTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_spawn_tree.end_label_edit(pTVDispInfo->item);
}

void prop_dlg::OnEndlabeleditDespawnTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_despawn_tree.end_label_edit(pTVDispInfo->item);
}

void prop_dlg::OnSelchangedSpawnTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM h = pNMTreeView->itemNew.hItem;
	if (h)
		m_spawn_tree.update_help(h);

	*pResult = 0;
}

void prop_dlg::OnSelchangedDespawnTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM h = pNMTreeView->itemNew.hItem;
	if (h)
		m_despawn_tree.update_help(h);

	*pResult = 0;
}
