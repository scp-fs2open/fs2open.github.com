
// CheckBoxListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "CheckBoxListDlg.h"
#include "cfile/cfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CheckBoxListDlg dialog

CheckBoxListDlg::CheckBoxListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CheckBoxListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CheckBoxListDlg)
	//}}AFX_DATA_INIT

	m_offline_options.clear();
	m_caption = _T("");
}

void CheckBoxListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CheckBoxListDlg)
	DDX_Control(pDX, IDC_EDIT1, m_checklist);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CheckBoxListDlg, CDialog)
	//{{AFX_MSG_MAP(CheckBoxListDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CheckBoxListDlg message handlers

BOOL CheckBoxListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// fix overlapping checkboxes issue
	// https://stackoverflow.com/questions/57951333/cchecklistbox-items-get-overlapped-on-selection-if-app-build-using-visual-studi
	m_checklist.SetFont(GetFont());

	if (!m_offline_options.empty())
		SetOptions(m_offline_options);

	if (!m_caption.IsEmpty())
		SetCaption(m_caption);

	return TRUE;
}

void CheckBoxListDlg::OnClose()
{
	UpdateData(TRUE);

	for (int i = 0; i < m_checklist.GetCount(); i++)
		m_offline_options[i].second = m_checklist.GetCheck(i) != BST_UNCHECKED;

	CDialog::OnClose();
}

void CheckBoxListDlg::SetOptions(const SCP_vector<CString> &options)
{
	if (IsWindow(m_hWnd))
	{
		m_checklist.ResetContent();
		for (const auto& option : options)
			m_checklist.AddString((LPCTSTR)option);

		UpdateData(FALSE);
	}
	else
	{
		m_offline_options.clear();
		for (const auto& option : options)
			m_offline_options.emplace_back(option, false);
	}
}

void CheckBoxListDlg::SetOptions(const SCP_vector<std::pair<CString, bool>> &options)
{
	if (IsWindow(m_hWnd))
	{
		m_checklist.ResetContent();
		for (const auto& option : options)
		{
			m_checklist.AddString((LPCTSTR)option.first);
			m_checklist.SetCheck(m_checklist.GetCount() - 1, option.second ? BST_CHECKED : BST_UNCHECKED);
		}

		UpdateData(FALSE);
	}
	else
		m_offline_options = options;
}

bool CheckBoxListDlg::IsChecked(int index)
{
	if (IsWindow(m_hWnd))
	{
		if (index < 0 || index >= m_checklist.GetCount())
			return FALSE;

		UpdateData(TRUE);
		return m_checklist.GetCheck(index) != BST_UNCHECKED;
	}
	else
	{
		if (index < 0 || index >= (int)m_offline_options.size())
			return FALSE;

		return m_offline_options[index].second;
	}
}

void CheckBoxListDlg::SetChecked(int index, bool checked)
{
	if (IsWindow(m_hWnd))
	{
		if (index < 0 || index >= m_checklist.GetCount())
			return;

		m_checklist.SetCheck(index, checked ? BST_CHECKED : BST_UNCHECKED);
		UpdateData(FALSE);
	}
	else
	{
		if (index < 0 || index >= (int)m_offline_options.size())
			return;

		m_offline_options[index].second = checked;
	}
}

void CheckBoxListDlg::SetCaption(const CString &caption)
{
	m_caption = caption;
	if (IsWindow(m_hWnd))
		SetWindowText(m_caption);
}
