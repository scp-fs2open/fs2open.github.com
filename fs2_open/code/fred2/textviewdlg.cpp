/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// TextViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "TextViewDlg.h"
#include "cfile/cfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// text_view_dlg dialog

text_view_dlg::text_view_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(text_view_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(text_view_dlg)
	m_edit = _T("");
	//}}AFX_DATA_INIT
}

void text_view_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(text_view_dlg)
	DDX_Text(pDX, IDC_EDIT1, m_edit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(text_view_dlg, CDialog)
	//{{AFX_MSG_MAP(text_view_dlg)
	ON_EN_SETFOCUS(IDC_EDIT1, OnSetfocusEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// text_view_dlg message handlers

void text_view_dlg::set(int ship_class)
{
	char line[256], line2[256];
	int i, j, found = 0, comment = 0;
	CFILE *fp;

	if (ship_class < 0)
		return;

	fp = cfopen("ships.tbl", "r");
	Assert(fp);

	while (cfgets(line, 255, fp)) {
		while (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

		for (i=j=0; line[i]; i++) {
			if (line[i] == '/' && line[i+1] == '/')
				break;

			if (line[i] == '/' && line[i+1] == '*') {
				comment = 1;
				i++;
				continue;
			}

			if (line[i] == '*' && line[i+1] == '/') {
				comment = 0;
				i++;
				continue;
			}

			if (!comment)
				line2[j++] = line[i];
		}

		line2[j] = 0;
		if (!strnicmp(line2, "$Name:", 6)) {
			found = 0;
			i = 6;
			while (line2[i] == ' ' || line2[i] == '\t')
				i++;

			if (!stricmp(line2 + i, Ship_info[ship_class].name))
				found = 1;
		}

		if (found) {
			m_edit += line;
			m_edit += "\r\n";
		}
	}

	cfclose(fp);
}

void text_view_dlg::OnSetfocusEdit1() 
{
	((CEdit *) GetDlgItem(IDC_EDIT1)) -> SetSel(-1, -1);
}
