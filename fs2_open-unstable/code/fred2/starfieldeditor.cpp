/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/StarfieldEditor.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Starfield editor dialog handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
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
 * 11    12/08/97 4:48p Hoffoss
 * Moved starfield editor controls to background editor.
 * 
 * 10    4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 9     4/17/97 9:33a Hoffoss
 * Squished a warning.
 * 
 * 8     4/16/97 5:18p Hoffoss
 * Moved Asteroid field editor stuff to a seperate dialog box.
 * 
 * 7     3/17/97 3:00p Hoffoss
 * slider updates on the fly now.
 * 
 * 6     2/21/97 5:34p Hoffoss
 * Added extensive modification detection and fixed a bug in initial
 * orders editor.
 * 
 * 5     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 * 
 * 4     1/31/97 3:16p Hoffoss
 * Asteroid field management implemented.
 *
 * $NoKeywords: $
 */

#include "stdafx.h"
#include "FRED.h"
#include "StarfieldEditor.h"
#include "starfield/starfield.h"
#include "FREDDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// starfield_editor dialog

starfield_editor::starfield_editor(CWnd* pParent /*=NULL*/)
	: CDialog(starfield_editor::IDD, pParent)
{
	//{{AFX_DATA_INIT(starfield_editor)
	//}}AFX_DATA_INIT
}

void starfield_editor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(starfield_editor)
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(starfield_editor, CDialog)
	//{{AFX_MSG_MAP(starfield_editor)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// starfield_editor message handlers

void starfield_editor::OnOK()
{
	char buf[40];

	UpdateData(TRUE);
	theApp.record_window_data(&Starfield_wnd_data, this);
	MODIFY(Num_stars, m_slider.GetPos());
	sprintf(buf, "%d", Num_stars);
	GetDlgItem(IDC_TOTAL)->SetWindowText(buf);
	update_map_window();
}

void starfield_editor::OnCancel()
{
	theApp.record_window_data(&Starfield_wnd_data, this);
	CDialog::OnCancel();
}

BOOL starfield_editor::OnInitDialog() 
{
	char buf[40];
	CDialog::OnInitDialog();
	theApp.init_window(&Starfield_wnd_data, this);
	
	m_slider.SetRange(100, MAX_STARS);
	m_slider.SetPos(Num_stars);
	sprintf(buf, "%d", Num_stars);
	GetDlgItem(IDC_TOTAL)->SetWindowText(buf);
	return TRUE;
}

void starfield_editor::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	char buf[40];

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

	MODIFY(Num_stars, m_slider.GetPos());
	sprintf(buf, "%d", Num_stars);
	GetDlgItem(IDC_TOTAL)->SetWindowText(buf);
}
