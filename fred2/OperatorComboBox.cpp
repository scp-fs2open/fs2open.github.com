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
	: m_listbox(help_callback, OPF_NONE)
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

void OperatorComboBox::refresh_popup_operators(int opf_type)
{
	// operator type might have changed
	m_listbox.SetOpfType(opf_type);

	// add all operators and their constants
	ResetContent();
	for (int i = 0; i < (int)m_sorted_operators.size(); ++i)
	{
		AddString(_T(m_sorted_operators[i].first.c_str()));
		SetItemData(i, m_sorted_operators[i].second);
	}
}

int OperatorComboBox::GetOpConst(int index) const
{
	return (int)m_listbox.GetItemData(index);
}

bool OperatorComboBox::IsItemEnabled(int index) const
{
	return m_listbox.IsItemEnabled(index);
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
		int op_const = (int)GetItemData(item);
		auto helptext = m_help_callback(op_const);
		if (helptext == nullptr && m_listbox.IsItemEnabled(item))	// a disabled item will show a tooltip regardless
			return -1;

		CRect rect;
		m_listbox.GetItemRect(item, &rect);

		pTI->hwnd = m_listbox.GetSafeHwnd();
		pTI->uId = item;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		pTI->rect = rect;
		return pTI->uId;
	}

	return -1;
}

OperatorComboBoxList::OperatorComboBoxList(const char* (*help_callback)(int), int opf_type)
{
	m_help_callback = help_callback;
	SetOpfType(opf_type);
}

void OperatorComboBoxList::SetOpfType(int opf_type)
{
	map_opf_to_opr(opf_type, m_expected_opr_type);
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
		auto helptext = m_help_callback(op_const);

		if ((helptext != nullptr && *helptext != '\0') || !IsItemEnabled(item))
		{
			SCP_string buffer;
			if (!IsItemEnabled(item))
			{
				int op_index = find_operator_index(op_const);
				buffer = "The operator \"";
				buffer += op_index >= 0 ? Operators[op_index].text : "<invalid operator>";
				buffer += "\" cannot be selected because it has an incompatible return type.\r\n\tReturns: ";
				buffer += opr_type_name(query_operator_return_type(op_const));
				buffer += "\r\n\tExpected: ";
				buffer += opr_type_name(m_expected_opr_type);
			}
			else
			{
				buffer = helptext;	// never mind the MSVC warning; helptext will be non-null and non-empty here
			}
			replace_all(buffer, "\t", "    ");
			m_tooltiptextA = buffer.c_str();
			m_tooltiptextW = buffer.c_str();

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
	}

	*pResult = 0;

	return TRUE;    // message was handled
}

// visual part of disabled items is based on example at:
// https://www.codeproject.com/Articles/451/CListBox-with-disabled-items

void OperatorComboBoxList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) == (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS));

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if (((LONG)(lpDrawItemStruct->itemID) >= 0) &&
		(lpDrawItemStruct->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)))
	{
		BOOL fDisabled = !IsWindowEnabled() || !IsItemEnabled(lpDrawItemStruct->itemID);

		COLORREF newTextColor = fDisabled ?
			RGB(0x80, 0x80, 0x80) : GetSysColor(COLOR_WINDOWTEXT);  // light gray

		COLORREF oldTextColor = pDC->SetTextColor(newTextColor);

		COLORREF newBkColor = GetSysColor(COLOR_WINDOW);
		COLORREF oldBkColor = pDC->SetBkColor(newBkColor);

		if (newTextColor == newBkColor)
			newTextColor = RGB(0xC0, 0xC0, 0xC0);   // dark gray

		if (!fDisabled && ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0))
		{
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
		}

		CString strText;
		GetText(lpDrawItemStruct->itemID, strText);

		const RECT& rc = lpDrawItemStruct->rcItem;

		pDC->ExtTextOut(rc.left + 2,
			rc.top + 2,// + max(0, (cyItem - m_cyText) / 2),
			ETO_OPAQUE, &rc,
			strText, strText.GetLength(), NULL);

		pDC->SetTextColor(oldTextColor);
		pDC->SetBkColor(oldBkColor);
	}

	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) != 0)
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem);
}

// we need to override this method when we create an owner-drawn combo box
void OperatorComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// do nothing; the supplied width and height are fine
}

BOOL OperatorComboBoxList::IsItemEnabled(UINT nIndex) const
{
	int op_const = (int)GetItemData(nIndex);
	int opr_type = query_operator_return_type(op_const);

	// check for special cases
	if (opr_type == OPR_AMBIGUOUS)										// OPR_AMBIGUOUS matches everything
		return true;
	if (opr_type == OPR_POSITIVE && m_expected_opr_type == OPR_NUMBER)	// positive data type can map to number data type just fine
		return true;
	if (opr_type == OPR_NUMBER && m_expected_opr_type == OPR_POSITIVE)	// this isn't kosher, but we hack it to make it work
		return true;

	// check that types match
	return opr_type == m_expected_opr_type;
}
