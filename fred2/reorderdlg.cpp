// reorderdlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "freddoc.h"
#include "management.h"
#include "reorderdlg.h"

#include "missioneditor/common.h"
#include "ship/ship.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

reorder_dlg::reorder_dlg(CWnd *pParent /*=nullptr*/)
	: CDialog(reorder_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(reorder_dlg)
	//}}AFX_DATA_INIT
}

void reorder_dlg::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(reorder_dlg)
	DDX_Control(pDX, IDC_REORDER_TYPE, m_type_combo);
	DDX_Control(pDX, IDC_REORDER_LIST, m_list);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(reorder_dlg, CDialog)
	//{{AFX_MSG_MAP(reorder_dlg)
	ON_CBN_SELCHANGE(IDC_REORDER_TYPE, OnSelchangeReorderType)
	ON_LBN_SELCHANGE(IDC_REORDER_LIST, OnSelchangeReorderList)
	ON_BN_CLICKED(IDC_REORDER_MOVE_TO_TOP, OnMoveToTop)
	ON_BN_CLICKED(IDC_REORDER_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_REORDER_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_REORDER_MOVE_TO_BOTTOM, OnMoveToBottom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL reorder_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_type_combo.AddString("Ships");
	m_type_combo.AddString("Wings");
	m_type_combo.SetCurSel(0);

	populate_list();
	update_buttons();

	return TRUE;
}

bool reorder_dlg::ships_selected() const
{
	return m_type_combo.GetCurSel() == 0;
}

void reorder_dlg::populate_list()
{
	m_list.ResetContent();
	m_slots.clear();

	if (ships_selected())
	{
		for (int i = 0; i < MAX_SHIPS; ++i)
		{
			if (Ships[i].objnum >= 0)
			{
				m_list.AddString(Ships[i].ship_name);
				m_slots.push_back(i);
			}
		}
	}
	else
	{
		for (int i = 0; i < MAX_WINGS; ++i)
		{
			if (Wings[i].wave_count > 0)
			{
				m_list.AddString(Wings[i].name);
				m_slots.push_back(i);
			}
		}
	}
}

void reorder_dlg::update_buttons()
{
	int pos = m_list.GetCurSel();
	int count = (int)m_slots.size();
	bool can_move_up = (pos > 0);
	bool can_move_down = (pos >= 0 && pos < count - 1);

	GetDlgItem(IDC_REORDER_MOVE_TO_TOP)->EnableWindow(can_move_up);
	GetDlgItem(IDC_REORDER_MOVE_UP)->EnableWindow(can_move_up);
	GetDlgItem(IDC_REORDER_MOVE_DOWN)->EnableWindow(can_move_down);
	GetDlgItem(IDC_REORDER_MOVE_TO_BOTTOM)->EnableWindow(can_move_down);
}

void reorder_dlg::move_selected(bool up, bool all_the_way)
{
	int pos = m_list.GetCurSel();
	int count = (int)m_slots.size();
	if (pos < 0 || pos >= count)
		return;

	int target;
	if (up)
		target = all_the_way ? 0 : pos - 1;
	else
		target = all_the_way ? count - 1 : pos + 1;

	if (target < 0 || target >= count || target == pos)
		return;

	// Rotate the item to the target position, which preserves the relative
	// order of everything else in the list.
	if (ships_selected())
	{
		FredShipSlotConfig cfg;
		cfg.fred_alt_names = Fred_alt_names;
		cfg.fred_callsigns = Fred_callsigns;
		cfg.cur_ship = &cur_ship;
		rotate_ship_slots(m_slots, pos, target, cfg);
	}
	else
	{
		FredWingSlotConfig cfg;
		cfg.wing_objects = wing_objects;
		cfg.cur_wing = &cur_wing;
		rotate_wing_slots(m_slots, pos, target, cfg);
	}

	set_modified();

	populate_list();
	m_list.SetCurSel(target);
	update_buttons();
}

void reorder_dlg::OnSelchangeReorderType()
{
	populate_list();
	update_buttons();
}

void reorder_dlg::OnSelchangeReorderList()
{
	update_buttons();
}

void reorder_dlg::OnMoveToTop()
{
	move_selected(true, true);
}

void reorder_dlg::OnMoveUp()
{
	move_selected(true, false);
}

void reorder_dlg::OnMoveDown()
{
	move_selected(false, false);
}

void reorder_dlg::OnMoveToBottom()
{
	move_selected(false, true);
}
