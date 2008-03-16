/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/ShipCheckListBox.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:31 $
 * $Author: Goober5000 $
 *
 * A custom check list box class that allows space bar to toggle the state of all
 * the selected checkboxes.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2002/07/15 03:11:02  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 4     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
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
