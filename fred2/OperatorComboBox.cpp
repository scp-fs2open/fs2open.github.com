// OperatorComboBox.cpp : implementation file
//

#include "OperatorComboBox.h"
#include "parse/parselo.h"
#include "parse/sexp.h"

BEGIN_MESSAGE_MAP(OperatorComboBox, CComboBox)
	//{{AFX_MSG_MAP(OperatorComboBox)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(OperatorComboBoxList, CListBox)
	//{{AFX_MSG_MAP(OperatorComboBoxList)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

OperatorComboBox::OperatorComboBox(const char* (*help_callback)(int))
	: m_listbox(help_callback)
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
		int op_const = (int)GetItemData(item);
		auto helptext = m_help_callback(op_const);
		if (helptext == nullptr)
			return -1;

		pTI->hwnd = m_listbox.GetSafeHwnd();
		pTI->uId = item;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		pTI->rect = rect;
		return pTI->uId;
	}

	return -1;
}

OperatorComboBoxList::OperatorComboBoxList(const char* (*help_callback)(int))
{
	m_help_callback = help_callback;
}

//here we supply the text for the item
BOOL OperatorComboBoxList::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
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

	int item = LBItemFromPt(GetSafeHwnd(), pt, FALSE);

	if (item >= 0 && item < GetCount())
	{
		int op_const = (int)GetItemData(item);

		SCP_string helptext = m_help_callback(op_const);
		replace_all(helptext, "\t", "    ");
		m_tooltiptextA = helptext.c_str();
		m_tooltiptextW = helptext.c_str();

#ifndef _UNICODE
		if (pNMHDR->code == TTN_NEEDTEXTA)
		{
			pTTTA->lpszText = (LPSTR)(LPCSTR)m_tooltiptextA;
			::SendMessage(pTTTA->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 400);
		}
		else
		{
			pTTTW->lpszText = (LPWSTR)(LPCWSTR)m_tooltiptextW;
			::SendMessage(pTTTW->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 400);
		}
#else
		if (pNMHDR->code == TTN_NEEDTEXTA)
		{
			pTTTA->lpszText = (LPSTR)(LPCSTR)m_tooltiptextA;
			::SendMessage(pTTTA->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 400);
		}
		else
		{
			pTTTW->lpszText = (LPWSTR)(LPCWSTR)m_tooltiptextW;
			::SendMessage(pTTTW->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 400);
		}
#endif
	}

	*pResult = 0;

	return TRUE;    // message was handled
}