#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "supportrearmdlg.h"
#include "mission/missionparse.h"
#include "weapon/weapon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSupportRearmDlg::CSupportRearmDlg(CWnd* pParent) : CDialog(CSupportRearmDlg::IDD, pParent)
{
	m_disallow_support_ships = FALSE;
	m_limit_rearm_to_pool = FALSE;
	m_support_repairs_hull = FALSE;
	m_disallow_support_rearm = FALSE;
	m_allow_weapon_precedence = FALSE;
	m_rearm_pool_from_loadout = FALSE;
	m_max_hull_repair_val = 0.0f;
	m_max_subsys_repair_val = 100.0f;
	m_weapon_pool_amount = 0;
	memset(m_rearm_weapon_pool, 0, sizeof(m_rearm_weapon_pool));
}

void CSupportRearmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_DISALLOW_SUPPORT_SHIPS, m_disallow_support_ships);
	DDX_Check(pDX, IDC_LIMIT_SUPPORT_REARM_TO_POOL, m_limit_rearm_to_pool);
	DDX_Check(pDX, IDC_SUPPORT_REPAIRS_HULL, m_support_repairs_hull);
	DDX_Check(pDX, IDC_DISALLOW_SUPPORT_REARM, m_disallow_support_rearm);
	DDX_Check(pDX, IDC_ALLOW_SUPPORT_REARM_PRECEDENCE, m_allow_weapon_precedence);
	DDX_Check(pDX, IDC_SUPPORT_REARM_POOL_FROM_LOADOUT, m_rearm_pool_from_loadout);
	DDX_Text(pDX, IDC_MAX_HULL_REPAIR_VAL, m_max_hull_repair_val);
	DDV_MinMaxFloat(pDX, m_max_hull_repair_val, 0.0f, 100.0f);
	DDX_Text(pDX, IDC_MAX_SUBSYS_REPAIR_VAL, m_max_subsys_repair_val);
	DDV_MinMaxFloat(pDX, m_max_subsys_repair_val, 0.0f, 100.0f);
	DDX_Text(pDX, IDC_SUPPORT_REARM_POOL_AMOUNT, m_weapon_pool_amount);
}

BEGIN_MESSAGE_MAP(CSupportRearmDlg, CDialog)
ON_LBN_SELCHANGE(IDC_SUPPORT_REARM_WEAPON_LIST, OnSelchangeWeaponList)
ON_BN_CLICKED(IDC_SUPPORT_REARM_SET_AMOUNT, OnSetPoolAmount)
ON_BN_CLICKED(IDC_SUPPORT_REARM_SET_UNLIMITED, OnSetPoolUnlimited)
ON_BN_CLICKED(IDC_SUPPORT_REARM_SET_ZERO, OnSetPoolZero)
ON_BN_CLICKED(IDC_SUPPORT_REARM_SET_ALL_AMOUNT, OnSetAllPoolAmount)
ON_BN_CLICKED(IDC_SUPPORT_REARM_SET_ALL_UNLIMITED, OnSetAllPoolUnlimited)
ON_BN_CLICKED(IDC_SUPPORT_REARM_SET_ALL_ZERO, OnSetAllPoolZero)
ON_BN_CLICKED(IDC_DISALLOW_SUPPORT_SHIPS, OnOptionChanged)
ON_BN_CLICKED(IDC_SUPPORT_REPAIRS_HULL, OnOptionChanged)
ON_BN_CLICKED(IDC_DISALLOW_SUPPORT_REARM, OnOptionChanged)
ON_BN_CLICKED(IDC_LIMIT_SUPPORT_REARM_TO_POOL, OnOptionChanged)
ON_BN_CLICKED(IDC_SUPPORT_REARM_POOL_FROM_LOADOUT, OnOptionChanged)
ON_BN_CLICKED(IDC_ALLOW_SUPPORT_REARM_PRECEDENCE, OnOptionChanged)
ON_WM_DRAWITEM()
END_MESSAGE_MAP()

BOOL CSupportRearmDlg::OnInitDialog()
{
	m_disallow_support_ships = (The_mission.support_ships.max_support_ships == 0) ? TRUE : FALSE;
	m_limit_rearm_to_pool = The_mission.flags[Mission::Mission_Flags::Limited_support_rearm_pool] ? TRUE : FALSE;
	m_support_repairs_hull = The_mission.flags[Mission::Mission_Flags::Support_repairs_hull] ? TRUE : FALSE;
	m_disallow_support_rearm = The_mission.support_ships.disallow_rearm ? TRUE : FALSE;
	m_allow_weapon_precedence = The_mission.support_ships.allow_rearm_weapon_precedence ? TRUE : FALSE;
	m_rearm_pool_from_loadout = The_mission.support_ships.rearm_pool_from_loadout ? TRUE : FALSE;
	m_max_hull_repair_val = The_mission.support_ships.max_hull_repair_val;
	m_max_subsys_repair_val = The_mission.support_ships.max_subsys_repair_val;
	memcpy(m_rearm_weapon_pool, The_mission.support_ships.rearm_weapon_pool, sizeof(m_rearm_weapon_pool));

	CDialog::OnInitDialog();

	populate_weapon_list();
	auto* weapon_list = (CListBox*)GetDlgItem(IDC_SUPPORT_REARM_WEAPON_LIST);
	if (weapon_list != nullptr && weapon_list->GetCount() > 0) {
		weapon_list->SetCurSel(0);
	}
	update_weapon_amount_display();
	UpdateData(FALSE);
	update_control_states();

	return TRUE;
}

CString CSupportRearmDlg::format_weapon_pool_entry(int weapon_class) const
{
	CString text;
	const auto& wi = Weapon_info[weapon_class];
	int amount = m_rearm_weapon_pool[weapon_class];
	if (wi.disallow_rearm) {
		text.Format("%s - 0 (disabled by weapon settings)", wi.name);
		return text;
	}
	if (amount < 0) {
		text.Format("%s - Unlimited", wi.name);
	} else {
		text.Format("%s - %d", wi.name, amount);
	}
	return text;
}

void CSupportRearmDlg::populate_weapon_list()
{
	auto* weapon_list = (CListBox*)GetDlgItem(IDC_SUPPORT_REARM_WEAPON_LIST);
	if (weapon_list == nullptr) {
		return;
	}

	weapon_list->ResetContent();
	for (int i = 0; i < weapon_info_size(); ++i) {
		if (!Weapon_info[i].wi_flags[Weapon::Info_Flags::Player_allowed]) {
			continue;
		}

		int list_index = weapon_list->AddString(format_weapon_pool_entry(i));
		weapon_list->SetItemData(list_index, i);
	}
}

int CSupportRearmDlg::get_selected_weapon_class() const
{
	auto* weapon_list = (CListBox*)GetDlgItem(IDC_SUPPORT_REARM_WEAPON_LIST);
	if (weapon_list == nullptr) {
		return -1;
	}

	int sel = weapon_list->GetCurSel();
	if (sel == LB_ERR) {
		return -1;
	}

	return (int)weapon_list->GetItemData(sel);
}

void CSupportRearmDlg::update_weapon_amount_display()
{
	int weapon_class = get_selected_weapon_class();
	if (weapon_class < 0 || weapon_class >= weapon_info_size()) {
		m_weapon_pool_amount = 0;
	} else {
		m_weapon_pool_amount = m_rearm_weapon_pool[weapon_class];
	}

	UpdateData(FALSE);
}

void CSupportRearmDlg::set_selected_weapon_amount(int amount)
{
	auto* weapon_list = (CListBox*)GetDlgItem(IDC_SUPPORT_REARM_WEAPON_LIST);
	if (weapon_list == nullptr) {
		return;
	}

	const int sel = weapon_list->GetCurSel();
	const int weapon_class = get_selected_weapon_class();
	if (sel == LB_ERR || weapon_class < 0 || weapon_class >= weapon_info_size()) {
		return;
	}

	if (Weapon_info[weapon_class].disallow_rearm) {
		amount = 0;
	} else if (amount < 0) {
		amount = -1;
	}

	m_rearm_weapon_pool[weapon_class] = amount;
	weapon_list->DeleteString(sel);
	weapon_list->InsertString(sel, format_weapon_pool_entry(weapon_class));
	weapon_list->SetItemData(sel, weapon_class);
	weapon_list->SetCurSel(sel);
	update_weapon_amount_display();
}

void CSupportRearmDlg::set_all_weapon_amount(int amount)
{
	const int normalized_amount = (amount < 0) ? -1 : amount;
	auto* weapon_list = (CListBox*)GetDlgItem(IDC_SUPPORT_REARM_WEAPON_LIST);
	if (weapon_list != nullptr) {
		for (int i = 0; i < weapon_list->GetCount(); ++i) {
			const int weapon_class = static_cast<int>(weapon_list->GetItemData(i));
			if (weapon_class < 0 || weapon_class >= weapon_info_size()) {
				continue;
			}

			if (Weapon_info[weapon_class].disallow_rearm) {
				m_rearm_weapon_pool[weapon_class] = 0;
			} else {
				m_rearm_weapon_pool[weapon_class] = normalized_amount;
			}
		}

		const int previous_sel = weapon_list->GetCurSel();
		populate_weapon_list();
		if (previous_sel != LB_ERR && previous_sel < weapon_list->GetCount()) {
			weapon_list->SetCurSel(previous_sel);
		} else if (weapon_list->GetCount() > 0) {
			weapon_list->SetCurSel(0);
		}
	}

	update_weapon_amount_display();
}

void CSupportRearmDlg::update_control_states()
{
	const bool support_enabled = (m_disallow_support_ships == FALSE);
	const bool rearm_allowed = support_enabled && (m_disallow_support_rearm == FALSE);
	const bool pool_controls_enabled =
		rearm_allowed && (m_limit_rearm_to_pool != FALSE) && (m_rearm_pool_from_loadout == FALSE);
	const int selected_weapon_class = get_selected_weapon_class();
	const bool selected_weapon_disallow_rearm =
		(selected_weapon_class >= 0 && selected_weapon_class < weapon_info_size() &&
			Weapon_info[selected_weapon_class].disallow_rearm);
	const bool right_controls_enabled = pool_controls_enabled && !selected_weapon_disallow_rearm;

	auto enable = [this](int control_id, bool state) {
		if (auto* ctrl = GetDlgItem(control_id)) {
			ctrl->EnableWindow(state ? TRUE : FALSE);
		}
	};

	enable(IDC_SUPPORT_REPAIRS_HULL, support_enabled);
	enable(IDC_MAX_HULL_REPAIR_VAL, support_enabled);
	enable(IDC_MAX_SUBSYS_REPAIR_VAL, support_enabled);
	enable(IDC_DISALLOW_SUPPORT_REARM, support_enabled);

	const bool limited_pool_enabled = rearm_allowed && (m_limit_rearm_to_pool != FALSE);

	enable(IDC_LIMIT_SUPPORT_REARM_TO_POOL, rearm_allowed);
	enable(IDC_SUPPORT_REARM_POOL_FROM_LOADOUT, limited_pool_enabled);
	enable(IDC_ALLOW_SUPPORT_REARM_PRECEDENCE, limited_pool_enabled);

	enable(IDC_SUPPORT_REARM_WEAPON_LIST, pool_controls_enabled);
	enable(IDC_SUPPORT_REARM_POOL_AMOUNT, right_controls_enabled);
	enable(IDC_SUPPORT_REARM_SET_AMOUNT, right_controls_enabled);
	enable(IDC_SUPPORT_REARM_SET_UNLIMITED, right_controls_enabled);
	enable(IDC_SUPPORT_REARM_SET_ZERO, right_controls_enabled);
	enable(IDC_SUPPORT_REARM_SET_ALL_AMOUNT, right_controls_enabled);
	enable(IDC_SUPPORT_REARM_SET_ALL_UNLIMITED, right_controls_enabled);
	enable(IDC_SUPPORT_REARM_SET_ALL_ZERO, right_controls_enabled);
}

void CSupportRearmDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl != IDC_SUPPORT_REARM_WEAPON_LIST || lpDrawItemStruct == nullptr) {
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return;
	}

	if (lpDrawItemStruct->itemID == static_cast<UINT>(-1)) {
		return;
	}

	auto* weapon_list = (CListBox*)GetDlgItem(IDC_SUPPORT_REARM_WEAPON_LIST);
	if (weapon_list == nullptr) {
		return;
	}

	CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect(lpDrawItemStruct->rcItem);

	const bool selected = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
	const bool focus = (lpDrawItemStruct->itemState & ODS_FOCUS) != 0;
	const int weapon_class = static_cast<int>(weapon_list->GetItemData(lpDrawItemStruct->itemID));
	const bool disallow_rearm =
		(weapon_class >= 0 && weapon_class < weapon_info_size()) ? Weapon_info[weapon_class].disallow_rearm : false;

	COLORREF bk = selected ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW);
	COLORREF text = selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT);
	if (disallow_rearm) {
		text = GetSysColor(COLOR_GRAYTEXT);
	}

	dc->FillSolidRect(&rect, bk);
	dc->SetBkMode(TRANSPARENT);
	dc->SetTextColor(text);

	CString item_text;
	weapon_list->GetText(lpDrawItemStruct->itemID, item_text);
	CRect text_rect = rect;
	text_rect.DeflateRect(2, 0);
	dc->DrawText(item_text, &text_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	if (focus) {
		dc->DrawFocusRect(&rect);
	}
}

void CSupportRearmDlg::OnOptionChanged()
{
	if (!UpdateData(TRUE)) {
		return;
	}

	update_control_states();
}

void CSupportRearmDlg::OnSelchangeWeaponList()
{
	update_weapon_amount_display();
	update_control_states();
}

void CSupportRearmDlg::OnSetPoolAmount()
{
	if (!UpdateData(TRUE)) {
		return;
	}

	set_selected_weapon_amount(m_weapon_pool_amount);
}

void CSupportRearmDlg::OnSetPoolUnlimited()
{
	set_selected_weapon_amount(-1);
}

void CSupportRearmDlg::OnSetPoolZero()
{
	set_selected_weapon_amount(0);
}

void CSupportRearmDlg::OnSetAllPoolAmount()
{
	if (!UpdateData(TRUE)) {
		return;
	}

	set_all_weapon_amount(m_weapon_pool_amount);
}

void CSupportRearmDlg::OnSetAllPoolUnlimited()
{
	set_all_weapon_amount(-1);
}

void CSupportRearmDlg::OnSetAllPoolZero()
{
	set_all_weapon_amount(0);
}

void CSupportRearmDlg::OnOK()
{
	if (!UpdateData(TRUE)) {
		return;
	}

	The_mission.support_ships.max_support_ships = (m_disallow_support_ships != FALSE) ? 0 : -1;
	The_mission.flags.set(Mission::Mission_Flags::Limited_support_rearm_pool, m_limit_rearm_to_pool != FALSE);
	The_mission.flags.set(Mission::Mission_Flags::Support_repairs_hull, m_support_repairs_hull != FALSE);
	The_mission.support_ships.disallow_rearm = (m_disallow_support_rearm != FALSE);
	The_mission.support_ships.allow_rearm_weapon_precedence = (m_allow_weapon_precedence != FALSE);
	The_mission.support_ships.rearm_pool_from_loadout = (m_rearm_pool_from_loadout != FALSE);
	The_mission.support_ships.max_hull_repair_val = m_max_hull_repair_val;
	The_mission.support_ships.max_subsys_repair_val = m_max_subsys_repair_val;
	memcpy(The_mission.support_ships.rearm_weapon_pool, m_rearm_weapon_pool, sizeof(m_rearm_weapon_pool));

	for (int i = 0; i < weapon_info_size(); ++i) {
		if (Weapon_info[i].disallow_rearm || !Weapon_info[i].wi_flags[Weapon::Info_Flags::Player_allowed]) {
			The_mission.support_ships.rearm_weapon_pool[i] = 0;
		}
	}

	CDialog::OnOK();
}