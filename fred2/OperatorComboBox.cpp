// OperatorComboBox.cpp : implementation file
//

#include "OperatorComboBox.h"
#include "resource.h"
#include "globalincs/utility.h"
#include "parse/parselo.h"
#include "parse/sexp.h"

BEGIN_MESSAGE_MAP(OperatorComboBox, CComboBox)
	//{{AFX_MSG_MAP(OperatorComboBox)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_CONTROL_REFLECT_EX(CBN_EDITCHANGE, OnEditChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(OperatorComboBoxList, CListBox)
	//{{AFX_MSG_MAP(OperatorComboBoxList)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

OperatorComboBox::OperatorComboBox(const char* (*help_callback)(int))
	: m_listbox(help_callback, OPF_NONE), m_help_callback(help_callback), m_max_operator_length(0), m_pressed_enter(false)
{
	// add all operators and calculate max length
	for (int i = 0; i < (int)Operators.size(); ++i)
	{
		const auto &op = Operators[i];
		m_sorted_operators.emplace_back(op.text, i);
		if (op.text.length() > m_max_operator_length)
			m_max_operator_length = op.text.length();
	}

	// sort all operators case-insensitively
	std::sort(m_sorted_operators.begin(), m_sorted_operators.end(), [](const std::pair<SCP_string, int> &a, const std::pair<SCP_string, int> &b)
		{
			return lcase_lessthan(a.first, b.first);
		});
}

OperatorComboBox::~OperatorComboBox()
{
	m_sorted_operators.clear();
}

// ----------------------------------------
// properly subclassing the edit and list components
// see https://jeffpar.github.io/kbarchive/kb/174/Q174667/

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

void OperatorComboBox::OnDestroy()
{
	if (m_edit.GetSafeHwnd() != NULL)
		m_edit.UnsubclassWindow();
	if (m_listbox.GetSafeHwnd() != NULL)
		m_listbox.UnsubclassWindow();
	CComboBox::OnDestroy();
}

// ----------------------------------------
// handling edit changes
// see https://www.codeproject.com/Articles/187753/Extended-CComboBox

BOOL OperatorComboBox::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// keep track of whether Enter was the most recent key pressed
		m_pressed_enter = (pMsg->wParam == VK_RETURN);

		// when we press Escape, kill the focus so that the CBN_KILLFOCUS handler in sexp_tree.cpp will close the popup
		if (pMsg->wParam == VK_ESCAPE)
			GetOwner()->SetFocus();

		// we don't need to check for DELETE and BACKSPACE here because the edit box isn't cleared because we don't use ResetContent
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

BOOL OperatorComboBox::OnEditChange()
{
	CString typed_text;
	GetWindowText(typed_text);

	filter_popup_operators((LPCSTR)typed_text);

	return FALSE;
}

void OperatorComboBox::refresh_popup_operators(int opf_type, const SCP_string &filter_string)
{
	// operator type might have changed
	m_listbox.SetOpfType(opf_type);

	// reset filter
	filter_popup_operators(filter_string);

	// set the text and select all of it
	SetWindowText(filter_string.c_str());
	if (!filter_string.empty())
		SetEditSel(0, (int)filter_string.length());
}

void OperatorComboBox::filter_popup_operators(const SCP_string &filter_string)
{
	int nIndex = 0;

	// quick check to see if everything is already there
	if (filter_string.empty() && GetCount() == (int)m_sorted_operators.size())
		return;

	// Remove all items in the combo box.  Don't use ResetContent() which also clears the edit control.
	for (int i = GetCount() - 1; i >= 0; i--)
		DeleteString(i);

	// if we're not filtering, just add everything
	if (filter_string.empty())
	{
		for (const auto &op_pair : m_sorted_operators)
		{
			AddString(_T(op_pair.first.c_str()));
			SetItemData(nIndex++, op_pair.second);
		}
		return;
	}

	// if the filter string is just one character, add operators that start with that character
	if (filter_string.length() == 1)
	{
		auto first_ch = SCP_tolower(filter_string[0]);

		for (const auto &op_pair : m_sorted_operators)
		{
			if (first_ch == SCP_tolower(op_pair.first[0]))
			{
				AddString(_T(op_pair.first.c_str()));
				SetItemData(nIndex++, op_pair.second);
			}
		}
		return;
	}

	// find all the operators below a threshold stringcost
	// "an input that has n unmatched chars will have at least MAX_LENGTH * MAX_LENGTH * n, so this sets it as max 2 unaccounted chars"
	size_t threshold = m_max_operator_length * m_max_operator_length * 3;
	SCP_vector<std::tuple<SCP_string, int, size_t>> filtered_operators;
	for (const auto &op_pair : m_sorted_operators)
	{
		size_t cost = stringcost(op_pair.first, filter_string, m_max_operator_length);
		if (cost < threshold)
			filtered_operators.emplace_back(op_pair.first, op_pair.second, cost);
	}

	// sort operators by cost
	std::sort(filtered_operators.begin(), filtered_operators.end(), [](const std::tuple<SCP_string, int, size_t>& a, const std::tuple<SCP_string, int, size_t>& b)
		{
			// compare the size_t parts of both tuples
			return std::get<2>(a) < std::get<2>(b);
		});

	// put them in the combo box
	for (const auto &op_triple : filtered_operators)
	{
		AddString(_T(std::get<0>(op_triple).c_str()));
		SetItemData(nIndex++, std::get<1>(op_triple));
	}
}

void OperatorComboBox::cleanup(bool confirm)
{
	SCP_UNUSED(confirm);

	// for now, all we do is clear what was in the edit box
	SetWindowText("");
}

// ----------------------------------------

bool OperatorComboBox::PressedEnter() const
{
	return m_pressed_enter;
}

int OperatorComboBox::GetOpIndex(int index) const
{
	if (index < 0)
	{
		// choose the operator that is the best match
		CString typed_text;
		GetWindowText(typed_text);
		return sexp_match_closest_operator((LPCSTR)typed_text, m_listbox.GetOpfType());
	}

	return (int)m_listbox.GetItemData(index);
}

bool OperatorComboBox::IsItemEnabled(int index) const
{
	if (index < 0)
		return false;

	return m_listbox.IsItemEnabled(index);
}

// ----------------------------------------

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
		int op_index = (int)GetItemData(item);
		int op_const = Operators[op_index].value;
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

int OperatorComboBoxList::GetOpfType() const
{
	return m_opf_type;
}

void OperatorComboBoxList::SetOpfType(int opf_type)
{
	m_opf_type = opf_type;
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
		int op_index = (int)GetItemData(item);
		int op_const = Operators[op_index].value;
		auto helptext = m_help_callback(op_const);

		if ((helptext != nullptr && *helptext != '\0') || !IsItemEnabled(item))
		{
			SCP_string buffer;
			if (!IsItemEnabled(item))
			{
				int opr_type;
				map_opf_to_opr(m_opf_type, opr_type);

				buffer = "The operator \"";
				buffer += op_index >= 0 ? Operators[op_index].text : "<invalid operator>";
				buffer += "\" cannot be selected because it has an incompatible return type.\r\n\tReturns: ";
				buffer += opr_type_name(query_operator_return_type(op_const));
				buffer += "\r\n\tExpected: ";
				buffer += opr_type_name(opr_type);
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
	int op_index = (int)GetItemData(nIndex);
	int op_const = Operators[op_index].value;
	int opr_type = query_operator_return_type(op_const);

	return sexp_query_type_match(m_opf_type, opr_type);
}
