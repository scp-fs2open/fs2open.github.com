/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/AdjustGridDlg.cpp $
 * $Revision: 1.2 $
 * $Date: 2006-01-21 02:22:04 $
 * $Author: wmcoolmon $
 *
 * Editor to allow one to change Fred's grid orientation and position.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.3  2002/08/15 04:35:44  penguin
 * Changes to build with fs2_open code.lib
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:52  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 2:59p Dave
 * 
 * 3     8/18/97 10:01p Hoffoss
 * Improved dialog by graying out fields that don't have any effect on
 * current plane setting.
 * 
 * 2     8/18/97 9:31p Hoffoss
 * Added grid adjustment dialog and shield system editor dialog.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "AdjustGridDlg.h"
#include "mission/missiongrid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
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

	m_spinx.SetRange((short)99999, (short)-99999);
	m_spiny.SetRange((short)99999, (short)-99999);
	m_spinz.SetRange((short)99999, (short)-99999);
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
