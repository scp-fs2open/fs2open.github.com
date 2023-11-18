/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "ShipCheckListBox.h"

BEGIN_MESSAGE_MAP(ShipCheckListBox, CCheckListBox)
	//{{AFX_MSG_MAP(CCheckListBox)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL ShipCheckListBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL b;

	b = CCheckListBox::Create(LBS_OWNERDRAWFIXED | dwStyle, rect, pParentWnd, nID);
	SetCheckStyle(BS_AUTOCHECKBOX);
	return b;
}

// prevent checkboxes from overlapping
// see https://stackoverflow.com/questions/3147958/mfc-cchecklistbox-items-overlap
void ShipCheckListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// set item height once:
	if ((GetStyle() & (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS)) ==
	                  (LBS_OWNERDRAWFIXED | LBS_HASSTRINGS) && m_cyText == 0)
	{
		SetItemHeight(0, CalcMinimumItemHeight() + 2);
	}

	// add some space between box and string: 
	lpDrawItemStruct->rcItem.left += 1;

	CCheckListBox::DrawItem(lpDrawItemStruct);
}

void ShipCheckListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE)
	{
		int i, list_size;

		list_size = GetCount();
		for (i=0; i<list_size; i++)
			if (GetSel(i) > 0)
			{
				if (GetCheck(i))
					SetCheck(i, 0);
				else
					SetCheck(i, 1);
			}

	} else
		CCheckListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}
