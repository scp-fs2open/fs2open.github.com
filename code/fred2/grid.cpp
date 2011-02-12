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
#include "Grid.h"

#include "render/3d.h"
#include "object/object.h"
#include "editor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int double_fine_gridlines;

/////////////////////////////////////////////////////////////////////////////
// CGrid dialog

//	Modeless constructor, MK
CGrid::CGrid(CView* pView)
{
	m_pGView = pView;
}


CGrid::CGrid(CWnd* pParent /*=NULL*/)
	: CDialog(CGrid::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGrid)
	m_GridSize = 0;
	//}}AFX_DATA_INIT

	m_pGView = NULL;

}


void CGrid::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGrid)
	DDX_Text(pDX, IDC_GRID_SIZE, m_GridSize);
	DDV_MinMaxUInt(pDX, m_GridSize, 2, 20);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGrid, CDialog)
	//{{AFX_MSG_MAP(CGrid)
	ON_BN_CLICKED(IDC_GRID_XY_PLANE, OnGridXyPlane)
	ON_BN_CLICKED(IDC_GRID_XZ_PLANE, OnGridXzPlane)
	ON_BN_CLICKED(IDC_GRID_YZ_PLANE, OnGridYzPlane)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_KILLFOCUS()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGrid message handlers

extern grid *The_grid;

void GridOrient(vec3d *forward, vec3d *right)
{
	vec3d	center;
	int		nrows, ncols;
	float	square_size;

	if (The_grid != NULL){
		center = The_grid->center;
		nrows = The_grid->nrows;
		ncols = The_grid->ncols;
		square_size = The_grid->square_size;
	} else {
		vm_vec_make(&center, 0.0f, 0.0f, 0.0f);
		nrows = 20;
		ncols = 20;
		square_size = 2.0f;
	}

	The_grid = create_grid(The_grid, forward,
		right,
		&center,
		nrows, ncols,
		square_size);

	physics_init(&The_grid->physics);
}

void CGrid::OnGridXyPlane() 
{
	vec3d	forward, right;

	vm_vec_make(&forward, 0.0f, 1.0f, 0.0f);
	vm_vec_make(&right, 1.0f, 0.0f, 0.0f);

	GridOrient(&forward, &right);
}

void CGrid::OnGridXzPlane() 
{
	vec3d	forward, right;

	vm_vec_make(&forward, 0.0f, 0.0f, 1.0f);
	vm_vec_make(&right, 1.0f, 0.0f, 0.0f);

	GridOrient(&forward, &right);
}

void CGrid::OnGridYzPlane() 
{
	vec3d	forward, right;

	vm_vec_make(&forward, 0.0f, 1.0f, 0.0f);
	vm_vec_make(&right, 0.0f, 0.0f, 1.0f);

	GridOrient(&forward, &right);	
}

BOOL CGrid::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CDialog::Create(IDD, pParentWnd);
}

BOOL CGrid::Create()
{
	return CDialog::Create(CGrid::IDD);
}


void CGrid::OnClose() 
{
	DestroyWindow();
}

void CGrid::OnDestroy() 
{
	UpdateData(TRUE);

	CDialog::OnDestroy();
	
}

void CGrid::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);
	
	DestroyWindow();

}

BOOL CGrid::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	UpdateData(TRUE);
	
	return CDialog::DestroyWindow();
}

BOOL CGrid::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_GRID_SIZE);
	pSpin->SetRange(2, 20);
	if ((m_GridSize < 2) || (m_GridSize > 20))
		m_GridSize = The_grid->ncols/5;
	pSpin->SetPos(m_GridSize);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGrid::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CString	strValue;

	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_GRID_SIZE);
	strValue.Format("%i", pSpin->GetPos());
	pSpin->GetBuddy()->SetWindowText(strValue);

	The_grid->nrows = pSpin->GetPos()*5;
	The_grid->ncols = The_grid->nrows;

	modify_grid(The_grid);

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
