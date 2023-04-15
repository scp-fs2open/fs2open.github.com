#include "stdafx.h"
#include "FRED.h"
#include "volumetricsdlg.h"


volumetrics_dlg::volumetrics_dlg(CWnd* pParent /*=nullptr*/) : CDialog(volumetrics_dlg::IDD, pParent) {

}

volumetrics_dlg::~volumetrics_dlg()
{
}

void volumetrics_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void volumetrics_dlg::OnClose()
{
	UpdateData(TRUE);
}

BEGIN_MESSAGE_MAP(volumetrics_dlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

