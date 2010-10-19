/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "FRED.h"
#include "StarfieldEditor.h"
#include "starfield/starfield.h"
#include "FREDDoc.h"

#ifdef _DEBUG
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
	
	m_slider.SetRange(0, MAX_STARS);
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
