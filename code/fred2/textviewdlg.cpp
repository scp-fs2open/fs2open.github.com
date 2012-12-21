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
	char line[256], line2[256], file_text[82];
	int i, j, n, found = 0, comment = 0, num_files = 0;
	char tbl_file_arr[MAX_TBL_PARTS][MAX_FILENAME_LEN];
	char *tbl_file_names[MAX_TBL_PARTS];
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
			drop_trailing_white_space(line2);
			found = 0;
			i = 6;

			while (line2[i] == ' ' || line2[i] == '\t' || line2[i] == '@')
				i++;

			if (!stricmp(line2 + i, Ship_info[ship_class].name)) {
				m_edit += "-- ships.tbl  -------------------------------\r\n";
				found = 1;
			}
		}

		if (found) {
			m_edit += line;
			m_edit += "\r\n";
		}
	}

	cfclose(fp);


	// done with ships.tbl, so now check all modular ship tables...
	num_files = cf_get_file_list_preallocated(MAX_TBL_PARTS, tbl_file_arr, tbl_file_names, CF_TYPE_TABLES, NOX("*-shp.tbm"), CF_SORT_REVERSE);

	for (n = 0; n < num_files; n++){
		strcat(tbl_file_names[n], ".tbm");

		fp = cfopen(tbl_file_names[n], "r");
		Assert(fp);

		memset( line, 0, sizeof(line) );
		memset( line2, 0, sizeof(line2) );
		found = 0;
		comment = 0;

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
				drop_trailing_white_space(line2);
				found = 0;
				i = 6;

				while (line2[i] == ' ' || line2[i] == '\t' || line2[i] == '@')
					i++;

				if (!stricmp(line2 + i, Ship_info[ship_class].name)) {
					memset( file_text, 0, sizeof(file_text) );
					snprintf(file_text, sizeof(file_text)-1, "--  %s  -------------------------------\r\n", tbl_file_names[n]);
					m_edit += file_text;
					found = 1;
				}
			}

			if (found) {
				m_edit += line;
				m_edit += "\r\n";
			}
		}

		cfclose(fp);
	}
}

void text_view_dlg::OnSetfocusEdit1() 
{
	((CEdit *) GetDlgItem(IDC_EDIT1)) -> SetSel(-1, -1);
}
