// OperatorComboBox.cpp : implementation file
//

#include "OperatorComboBox.h"
#include "parse/sexp.h"

BEGIN_MESSAGE_MAP(OperatorComboBox, CComboBox)
	//{{AFX_MSG_MAP(OperatorComboBox)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

OperatorComboBox::OperatorComboBox(const char* (*help_callback)(int))
{
	m_help_callback = help_callback;

	for (auto& op : Operators)
		m_sorted_operators.emplace_back(op.text, op.value);

	// sort all operators case-insensitively
	std::sort(m_sorted_operators.begin(), m_sorted_operators.end(), [](const std::pair<SCP_string, int>& a, const std::pair<SCP_string, int>& b)
		{
			return SCP_string_lcase_less_than()(a.first, b.first);
		});
}

OperatorComboBox::~OperatorComboBox()
{
	m_sorted_operators.clear();
}

// See https://jeffpar.github.io/kbarchive/kb/174/Q174667/
HBRUSH OperatorComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_EDIT)
	{
		//[ASCII 160][ASCII 160][ASCII 160]Edit control
		if (m_edit.GetSafeHwnd() == NULL)
			m_edit.SubclassWindow(pWnd->GetSafeHwnd());
	}
	else if (nCtlColor == CTLCOLOR_LISTBOX)
	{
		//ListBox control
		if (m_listbox.GetSafeHwnd() == NULL)
			m_listbox.SubclassWindow(pWnd->GetSafeHwnd());
	}
	HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

// See https://jeffpar.github.io/kbarchive/kb/174/Q174667/
void OperatorComboBox::OnDestroy()
{
	if (m_edit.GetSafeHwnd() != NULL)
		m_edit.UnsubclassWindow();
	if (m_listbox.GetSafeHwnd() != NULL)
		m_listbox.UnsubclassWindow();
	CComboBox::OnDestroy();
}

void OperatorComboBox::refresh_popup_operators()
{
	// add all operators and their constants
	ResetContent();
	for (int i = 0; i < (int)m_sorted_operators.size(); ++i)
	{
		AddString(_T(m_sorted_operators[i].first.c_str()));
		SetItemData(i, m_sorted_operators[i].second);
	}
}

// tooltip stuff is based on example at
// https://www.codeproject.com/articles/1761/ctreectrl-clistctrl-clistbox-with-tooltip-based-on

void OperatorComboBox::PreSubclassWindow()
{
	CComboBox::PreSubclassWindow();
	EnableToolTips(TRUE);
}

INT_PTR OperatorComboBox::OnToolHitTest(CPoint point, TOOLINFO *pTI) const
{
	// not linked yet
	if (m_listbox.GetSafeHwnd() == NULL)
		return -1;

	CPoint screenpoint = point;
	ClientToScreen(&screenpoint);

	int item = LBItemFromPt(m_listbox.GetSafeHwnd(), screenpoint, FALSE);

	if (item >= 0 && item < GetCount())
	{
		CRect rect;
		m_listbox.GetItemRect(item, &rect);

		pTI->hwnd = m_hWnd;
		pTI->uId = item;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		pTI->rect = rect;
		return pTI->uId;
	}

	return -1;
}

//here we supply the text for the item
BOOL OperatorComboBox::OnToolTipText(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	CString strTipText;
	UINT_PTR nID = pNMHDR->idFrom;

	// Do not process the message from built in tooltip
	if (nID == (UINT_PTR)m_hWnd &&
		((pNMHDR->code == TTN_NEEDTEXTA && pTTTA->uFlags & TTF_IDISHWND) ||
		(pNMHDR->code == TTN_NEEDTEXTW && pTTTW->uFlags & TTF_IDISHWND)))
		return FALSE;

	// Get the mouse position
	auto pMessage = GetCurrentMessage(); // get mouse pos
	ASSERT(pMessage);
	auto pt = pMessage->pt;

	int item = LBItemFromPt(m_listbox.GetSafeHwnd(), pt, FALSE);

	if (item >= 0 && item < GetCount())
	{
		int op_const = (int)GetItemData(item);
		strTipText = m_help_callback(op_const);

#ifndef _UNICODE
		if (pNMHDR->code == TTN_NEEDTEXTA)
			lstrcpyn(pTTTA->szText, strTipText, 80);
		else
			_mbstowcsz(pTTTW->szText, strTipText, 80);
#else
		if (pNMHDR->code == TTN_NEEDTEXTA)
			_wcstombsz(pTTTA->szText, strTipText, 80);
		else
			lstrcpyn(pTTTW->szText, strTipText, 80);
#endif
	}

	*pResult = 0;

	return TRUE;    // message was handled
}