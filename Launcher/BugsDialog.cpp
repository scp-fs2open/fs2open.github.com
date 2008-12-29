// BugsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "BugsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBugsDialog dialog


CBugsDialog::CBugsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBugsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBugsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBugsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBugsDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBugsDialog, CDialog)
	//{{AFX_MSG_MAP(CBugsDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBugsDialog message handlers
