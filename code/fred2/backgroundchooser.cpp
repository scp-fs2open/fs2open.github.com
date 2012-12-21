// BackgroundChooser.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "BackgroundChooser.h"
#include "ship/ship.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BackgroundChooser dialog


BackgroundChooser::BackgroundChooser(int numBackgrounds)
	: CDialog(BackgroundChooser::IDD)
{
	//{{AFX_DATA_INIT(BackgroundChooser)
	//}}AFX_DATA_INIT

	m_numBackgrounds = numBackgrounds;
	m_chosenBackground = -1;
}


void BackgroundChooser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BackgroundChooser)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BackgroundChooser, CDialog)
	//{{AFX_MSG_MAP(BackgroundChooser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BackgroundChooser message handlers

BOOL BackgroundChooser::OnInitDialog()
{
	int i;

	for (i = 0; i < m_numBackgrounds; i++) 
	{
		char temp[NAME_LENGTH];
		sprintf(temp, "Background %d", i + 1);

		((CComboBox*) GetDlgItem(IDC_BACKGROUND))->AddString(temp);
	}
	((CComboBox*) GetDlgItem(IDC_BACKGROUND))->SetCurSel(0);

	CDialog::OnInitDialog();
	UpdateData(FALSE);
	return TRUE;
}

void BackgroundChooser::OnOK() 
{
	UpdateData(TRUE);

	m_chosenBackground = ((CComboBox*) GetDlgItem(IDC_BACKGROUND))->GetCurSel();

	CDialog::OnOK();
}

void BackgroundChooser::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void BackgroundChooser::OnClose()
{
	OnCancel();
}

int BackgroundChooser::GetChosenBackground()
{
	return m_chosenBackground;
}
