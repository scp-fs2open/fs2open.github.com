/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

// EditContainerAddNewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "EditContainerAddNewDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_RESET_FOCUS	0
#define RESET_FOCUS		1

/////////////////////////////////////////////////////////////////////////////
// EditContainerAddNewDlg dialog


CEditContainerAddNewDlg::CEditContainerAddNewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditContainerAddNewDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CEditContainerAddNewDlg)
	 //m_container_name = _T("");	
	//}}AFX_DATA_INIT
}


void CEditContainerAddNewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditContainerAddNewDlg)
	DDX_Text(pDX, IDC_NEW_CONTAINER_NAME, m_new_container_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditContainerAddNewDlg, CDialog)
	//{{AFX_MSG_MAP(CEditVariableDlg))
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditContainerAddNewDlg message handlers

BOOL CEditContainerAddNewDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	cancelled = true; 
	m_new_container_name = "<New Container Name>";

	// Send default name and values into dialog box
	UpdateData(FALSE);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditContainerAddNewDlg::OnOK() 
{	
	cancelled = false; 
	CDialog::OnOK();
}