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
