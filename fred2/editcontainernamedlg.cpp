/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "stdafx.h"
#include "FRED.h"
#include "EditContainerNameDlg.h"
#include "parse/sexp.h"
#include "parse/sexp_container.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_RESET_FOCUS	0
#define RESET_FOCUS		1

CEditContainerNameDlg::CEditContainerNameDlg(const SCP_string &window_title, const SCP_string &old_name, CWnd *pParent)
	: CDialog(CEditContainerNameDlg::IDD, pParent), m_window_title(window_title.c_str()), m_new_container_name(old_name.c_str()), m_cancelled(true)
{
}

void CEditContainerNameDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NEW_CONTAINER_NAME, m_new_container_name);
	DDV_MaxChars(pDX, m_new_container_name, sexp_container::NAME_MAX_LENGTH);
}

BEGIN_MESSAGE_MAP(CEditContainerNameDlg, CDialog)
// nothing needed
END_MESSAGE_MAP()

BOOL CEditContainerNameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(m_window_title);

	CEdit *new_name_edit = (CEdit *)GetDlgItem(IDC_NEW_CONTAINER_NAME);
	new_name_edit->SetLimitText(sexp_container::NAME_MAX_LENGTH);

	// Send default name and values into dialog box
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditContainerNameDlg::OnOK()
{
	m_cancelled = false;
	CDialog::OnOK();
}
