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
#include "AdjustGridDlg.h"
#include "mission/missiongrid.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// adjust_grid_dlg dialog

adjust_grid_dlg::adjust_grid_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(adjust_grid_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(adjust_grid_dlg)
	m_x = 0;
	m_y = 0;
	m_z = 0;
	//}}AFX_DATA_INIT
}

void adjust_grid_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(adjust_grid_dlg)
	DDX_Control(pDX, IDC_SPIN_Z, m_spinz);
	DDX_Control(pDX, IDC_SPIN_Y, m_spiny);
	DDX_Control(pDX, IDC_SPIN_X, m_spinx);
	DDX_Text(pDX, IDC_EDIT_X, m_x);
	DDX_Text(pDX, IDC_EDIT_Y, m_y);
	DDX_Text(pDX, IDC_EDIT_Z, m_z);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(adjust_grid_dlg, CDialog)
	//{{AFX_MSG_MAP(adjust_grid_dlg)
	ON_BN_CLICKED(IDC_XY_PLANE, OnXyPlane)
	ON_BN_CLICKED(IDC_XZ_PLANE, OnXzPlane)
	ON_BN_CLICKED(IDC_YZ_PLANE, OnYzPlane)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// adjust_grid_dlg message handlers

BOOL adjust_grid_dlg::OnInitDialog() 
{
	m_x = (int) The_grid->center.xyz.x;
	m_y = (int) The_grid->center.xyz.y;
	m_z = (int) The_grid->center.xyz.z;
	CDialog::OnInitDialog();
	if (The_grid->gmatrix.vec.uvec.xyz.y) {
		((CButton *) GetDlgItem(IDC_XZ_PLANE))->SetCheck(TRUE);
		GetDlgItem(IDC_EDIT_X)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_Z)->EnableWindow(FALSE);

	} else if (The_grid->gmatrix.vec.uvec.xyz.z) {
		((CButton *) GetDlgItem(IDC_XY_PLANE))->SetCheck(TRUE);
		GetDlgItem(IDC_EDIT_X)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_Y)->EnableWindow(FALSE);

	} else {
		((CButton *) GetDlgItem(IDC_YZ_PLANE))->SetCheck(TRUE);
		GetDlgItem(IDC_EDIT_Y)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_Z)->EnableWindow(FALSE);
	}

	m_spinx.SetRange(SHRT_MIN, SHRT_MAX);
	m_spiny.SetRange(SHRT_MIN, SHRT_MAX);
	m_spinz.SetRange(SHRT_MIN, SHRT_MAX);
	return TRUE;
}

void adjust_grid_dlg::OnOK()
{
	UpdateData(TRUE);
	The_grid->center.xyz.x = (float) m_x;
	The_grid->center.xyz.y = (float) m_y;
	The_grid->center.xyz.z = (float) m_z;

	if (((CButton *) GetDlgItem(IDC_XY_PLANE)) -> GetCheck()) {
		The_grid->gmatrix.vec.fvec = vmd_x_vector;
		The_grid->gmatrix.vec.rvec = vmd_y_vector;

	} else if (((CButton *) GetDlgItem(IDC_YZ_PLANE)) -> GetCheck()) {
		The_grid->gmatrix.vec.fvec = vmd_y_vector;
		The_grid->gmatrix.vec.rvec = vmd_z_vector;

	} else {  // XZ plane
		The_grid->gmatrix.vec.fvec = vmd_x_vector;
		The_grid->gmatrix.vec.rvec = vmd_z_vector;
	}

	modify_grid(The_grid);
	CDialog::OnOK();
}

void adjust_grid_dlg::OnXyPlane() 
{
	GetDlgItem(IDC_EDIT_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_Y)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_Y)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_Z)->EnableWindow(TRUE);
	GetDlgItem(IDC_SPIN_Z)->EnableWindow(TRUE);
}

void adjust_grid_dlg::OnXzPlane() 
{
	GetDlgItem(IDC_EDIT_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_Y)->EnableWindow(TRUE);
	GetDlgItem(IDC_SPIN_Y)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_Z)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_Z)->EnableWindow(FALSE);
}

void adjust_grid_dlg::OnYzPlane() 
{
	GetDlgItem(IDC_EDIT_X)->EnableWindow(TRUE);
	GetDlgItem(IDC_SPIN_X)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_Y)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_Y)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_Z)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIN_Z)->EnableWindow(FALSE);
}
