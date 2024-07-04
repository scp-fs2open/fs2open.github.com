// ListItemChooser.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "ListItemChooser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListItemChooser dialog


ListItemChooser::ListItemChooser(const SCP_vector<SCP_string>& listItems)
	: CDialog(ListItemChooser::IDD)
{
	//{{AFX_DATA_INIT(ListItemChooser)
	//}}AFX_DATA_INIT

	m_listItems = listItems;
	m_chosenItem = -1;
}


void ListItemChooser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ListItemChooser)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ListItemChooser, CDialog)
	//{{AFX_MSG_MAP(ListItemChooser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ListItemChooser message handlers

BOOL ListItemChooser::OnInitDialog()
{
	for (const auto &item: m_listItems) 
	{
		((CComboBox*) GetDlgItem(IDC_LISTITEM))->AddString(item.c_str());
	}
	((CComboBox*) GetDlgItem(IDC_LISTITEM))->SetCurSel(0);

	CDialog::OnInitDialog();
	UpdateData(FALSE);
	return TRUE;
}

void ListItemChooser::OnOK() 
{
	UpdateData(TRUE);

	m_chosenItem = ((CComboBox*) GetDlgItem(IDC_LISTITEM))->GetCurSel();

	CDialog::OnOK();
}

void ListItemChooser::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void ListItemChooser::OnClose()
{
	OnCancel();
}

int ListItemChooser::GetChosenIndex()
{
	return m_chosenItem;
}
