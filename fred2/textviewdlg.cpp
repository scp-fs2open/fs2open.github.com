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
// TextViewDlg dialog

TextViewDlg::TextViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TextViewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(TextViewDlg)
	m_edit = _T("");
	//}}AFX_DATA_INIT

	m_original_text = _T("");
	m_caption = _T("");
	m_editable = false;
}

void TextViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TextViewDlg)
	DDX_Text(pDX, IDC_EDIT1, m_edit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(TextViewDlg, CDialog)
	//{{AFX_MSG_MAP(TextViewDlg)
	ON_WM_CLOSE()
	ON_EN_SETFOCUS(IDC_EDIT1, OnSetfocusEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TextViewDlg message handlers

BOOL TextViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_original_text = m_edit;

	if (!m_caption.IsEmpty())
		SetCaption(m_caption);
	SetEditable(m_editable);

	return TRUE;
}

void TextViewDlg::OnClose()
{
	UpdateData(TRUE);
	CDialog::OnClose();
}

void TextViewDlg::LoadShipsTblText(const ship_info *sip)
{
	char line[256], line2[256], file_text[82];
	int i, j, n, found = 0, comment = 0, num_files = 0;
	SCP_vector<SCP_string> tbl_file_names;
	CFILE *fp;

	SetCaption("Ship Table Data");

	if (!sip)
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

			if (!stricmp(line2 + i, sip->name)) {
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
	num_files = cf_get_file_list(tbl_file_names, CF_TYPE_TABLES, NOX("*-shp.tbm"), CF_SORT_REVERSE);

	for (n = 0; n < num_files; n++){
		tbl_file_names[n] += ".tbm";

		fp = cfopen(tbl_file_names[n].c_str(), "r");
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

				if (!stricmp(line2 + i, sip->name)) {
					memset( file_text, 0, sizeof(file_text) );
					snprintf(file_text, sizeof(file_text)-1, "--  %s  -------------------------------\r\n", tbl_file_names[n].c_str());
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

void TextViewDlg::OnSetfocusEdit1()
{
	// when the dialog is first displayed, prevent it from selecting all the text
	((CEdit *)GetDlgItem(IDC_EDIT1))->SetSel(-1, -1);
}

void TextViewDlg::SetText(const CString &text)
{
	m_edit = text;
	if (IsWindow(m_hWnd))
		UpdateData(FALSE);
}

void TextViewDlg::GetText(CString &text)
{
	if (IsWindow(m_hWnd))
		UpdateData(TRUE);
	text = m_edit;
}

void TextViewDlg::SetCaption(const CString &caption)
{
	m_caption = caption;
	if (IsWindow(m_hWnd))
		SetWindowText(m_caption);
}

void TextViewDlg::SetEditable(bool editable)
{
	m_editable = editable;
	if (IsWindow(m_hWnd))
		((CEdit *)GetDlgItem(IDC_EDIT1))->SetReadOnly(editable ? FALSE : TRUE);
}
