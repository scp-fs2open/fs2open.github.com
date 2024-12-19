/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#include "stdafx.h"
#include "FRED.h"
#include "warpparamsdlg.h"
#include "globalincs/linklist.h"
#include "ship/shipfx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// warp_params_dlg dialog

warp_params_dlg::warp_params_dlg(CWnd* pParent) : CDialog(warp_params_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(warp_params_dlg)
	m_warp_type = 0;
	m_start_sound = _T("");
	m_end_sound = _T("");
	m_warpout_engage_time = _T("");
	m_speed = _T("");
	m_time = _T("");
	m_accel_exp = _T("");
	m_radius = _T("");
	m_anim = _T("");
	m_supercap_warp_physics = FALSE;
	m_player_warpout_speed = _T("");
	//}}AFX_DATA_INIT
}

void warp_params_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(warp_params_dlg)
	DDX_CBIndex(pDX, IDC_WARP_TYPE, m_warp_type);
	DDX_Text(pDX, IDC_START_SOUND, m_start_sound);
	DDX_Text(pDX, IDC_END_SOUND, m_end_sound);
	DDX_Text(pDX, IDC_WARPOUT_ENGAGE_TIME, m_warpout_engage_time);
	DDX_Text(pDX, IDC_SPEED, m_speed);
	DDX_Text(pDX, IDC_TIME, m_time);
	DDX_Text(pDX, IDC_ACCEL_EXP, m_accel_exp);
	DDX_Text(pDX, IDC_RADIUS, m_radius);
	DDX_Text(pDX, IDC_ANIM, m_anim);
	DDX_Check(pDX, IDC_SUPERCAP_WARP_PHYSICS, m_supercap_warp_physics);
	DDX_Text(pDX, IDC_PLAYER_WARPOUT_SPEED, m_player_warpout_speed);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(warp_params_dlg, CDialog)
	//{{AFX_MSG_MAP(warp_params_dlg)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_WARP_TYPE, OnSelchangeWarpType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// warp_params_dlg message handlers

BOOL warp_params_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CComboBox *ptr = (CComboBox *) GetDlgItem(IDC_WARP_TYPE);
	ptr->ResetContent();
	for (int i = 0; i < Num_warp_types; ++i)
		ptr->AddString(Warp_types[i]);
	for (const auto &fi: Fireball_info)
		ptr->AddString(fi.unique_id);

	// find the params of the first marked ship
	WarpParams *params = nullptr;
	for (object *objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp))
	{
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))
		{
			if (objp->flags[Object::Object_Flags::Marked])
			{
				if (m_warp_in)
					params = &Warp_params[Ships[objp->instance].warpin_params_index];
				else
					params = &Warp_params[Ships[objp->instance].warpout_params_index];

				break;
			}
		}
	}

	// set fields to our params
	if (params != nullptr)
	{
		// our warp type could be a fireball instead of a straight-up type
		if (params->warp_type & WT_DEFAULT_WITH_FIREBALL)
			m_warp_type = (params->warp_type & WT_FLAG_MASK) + Num_warp_types;
		else if (params->warp_type >= 0 && params->warp_type < Num_warp_types)
			m_warp_type = params->warp_type;

		if (params->snd_start.isValid())
			m_start_sound = gamesnd_get_game_sound(params->snd_start)->name.c_str();
		if (params->snd_end.isValid())
			m_end_sound = gamesnd_get_game_sound(params->snd_end)->name.c_str();

		if (params->warpout_engage_time > 0)
			m_warpout_engage_time.Format(_T("%.2f"), i2fl(params->warpout_engage_time) / 1000.0f);

		if (params->speed > 0.0f)
			m_speed.Format(_T("%.2f"), params->speed);

		if (params->time > 0.0f)
			m_time.Format(_T("%.2f"), i2fl(params->time) / 1000.0f);

		if (params->accel_exp > 0.0f)
			m_accel_exp.Format(_T("%.2f"), params->accel_exp);

		if (params->radius > 0.0f)
			m_radius.Format(_T("%.2f"), params->radius);

		if (strlen(params->anim) > 0)
			m_anim = params->anim;

		m_supercap_warp_physics = params->supercap_warp_physics ? TRUE : FALSE;

		if (params->warpout_player_speed > 0.0f)
			m_player_warpout_speed.Format(_T("%.2f"), params->warpout_player_speed);
	}

	// set up gui
	reset_controls();

	return TRUE;
}

void warp_params_dlg::OnSelchangeWarpType()
{
	// get stuff from GUI
	UpdateData(TRUE);

	reset_controls();
}

void warp_params_dlg::reset_controls()
{
	// edit captions/labels
	SetWindowText(m_warp_in ? "Warp-In Parameters" : "Warp-Out Parameters");
	GetDlgItem(IDC_ACCEL_EXP_LABEL)->SetWindowText(m_warp_in ? "Deceleration Exponent (Hyperspace Only)" : "Acceleration Exponent (Hyperspace Only)");

	// enable/disable certain things for warpin
	GetDlgItem(IDC_WARPOUT_ENGAGE_TIME_LABEL)->EnableWindow(!m_warp_in);
	GetDlgItem(IDC_WARPOUT_ENGAGE_TIME)->EnableWindow(!m_warp_in);
	GetDlgItem(IDC_PLAYER_WARPOUT_SPEED_LABEL)->EnableWindow(!m_warp_in);
	GetDlgItem(IDC_PLAYER_WARPOUT_SPEED)->EnableWindow(!m_warp_in);

	// enable/disable certain things for certain warp types
	GetDlgItem(IDC_ANIM_LABEL)->EnableWindow(m_warp_type == WT_SWEEPER);
	GetDlgItem(IDC_ANIM)->EnableWindow(m_warp_type == WT_SWEEPER);
	GetDlgItem(IDC_ACCEL_EXP_LABEL)->EnableWindow(m_warp_type == WT_HYPERSPACE);
	GetDlgItem(IDC_ACCEL_EXP)->EnableWindow(m_warp_type == WT_HYPERSPACE);

	// store stuff to gui
	UpdateData(FALSE);
}

// cancel
void warp_params_dlg::OnCancel()
{
	CDialog::OnCancel();
}

// ok
void warp_params_dlg::OnOK()
{
	// grab stuff from GUI
	UpdateData(TRUE);

	// save stuff in params object
	WarpParams params;
	params.direction = m_warp_in ? WarpDirection::WARP_IN : WarpDirection::WARP_OUT;

	if (m_warp_type < Num_warp_types)
		params.warp_type = m_warp_type;
	else
		params.warp_type = (m_warp_type - Num_warp_types) | WT_DEFAULT_WITH_FIREBALL;

	if (m_start_sound.GetLength() > 0)
		params.snd_start = gamesnd_get_by_name(m_start_sound);

	if (m_end_sound.GetLength() > 0)
		params.snd_end = gamesnd_get_by_name(m_end_sound);

	if (!m_warp_in && m_warpout_engage_time.GetLength() > 0)
	{
		float t_time = (float)atof(m_warpout_engage_time);
		if (t_time > 0.0f)
			params.warpout_engage_time = fl2i(t_time*1000.0f);
	}

	if (m_speed.GetLength() > 0)
	{
		float speed = (float)atof(m_speed);
		if (speed > 0.0f)
			params.speed = speed;
	}

	if (m_time.GetLength() > 0)
	{
		float t_time = (float)atof(m_time);
		if (t_time > 0.0f)
			params.time = fl2i(t_time*1000.0f);
	}

	if (m_accel_exp.GetLength() > 0)
	{
		float accel_exp = (float)atof(m_accel_exp);
		if (accel_exp >= 0.0f)
			params.accel_exp = accel_exp;
	}

	if (m_radius.GetLength() > 0)
	{
		float rad = (float)atof(m_radius);
		if (rad > 0.0f)
			params.radius = rad;
	}

	if (m_anim.GetLength() > 0)
		strcpy_s(params.anim, m_anim);

	params.supercap_warp_physics = (m_supercap_warp_physics == TRUE);

	if (!m_warp_in && m_player_warpout_speed.GetLength() > 0)
	{
		float speed = (float)atof(m_player_warpout_speed);
		if (speed > 0.0f)
			params.warpout_player_speed = speed;
	}

	// resolve this set of parameters
	int index = find_or_add_warp_params(params);

	// assign to all marked ships
	for (object *objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp))
	{
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))
		{
			if (objp->flags[Object::Object_Flags::Marked])
			{
				if (m_warp_in)
					Ships[objp->instance].warpin_params_index = index;
				else
					Ships[objp->instance].warpout_params_index = index;
			}
		}
	}

	CDialog::OnOK();
}
