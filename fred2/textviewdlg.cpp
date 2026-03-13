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
#include "utils/table_viewer.h"

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
	SetCaption("Ship Table Data");

	if (!sip)
		return;

	m_edit += table_viewer::get_table_entry_text("ships.tbl", "*-shp.tbm", sip->name).c_str();
}

void TextViewDlg::LoadMusicTblText()
{
	SetCaption("Music Table Data");
	m_edit += table_viewer::get_complete_table_text("music.tbl", "*-mus.tbm").c_str();
}

void TextViewDlg::OnSetfocusEdit1()
{
	// when the dialog is first displayed, prevent it from selecting all the text
	((CEdit *)GetDlgItem(IDC_EDIT1))->SetSel(-1, -1);
}

void TextViewDlg::SetText(const CString &text)
{
	m_edit = text;

	// accommodate MFC newlines
	m_edit.Replace("\n", "\r\n");
	m_edit.Replace("\r\r", "\r");

	if (IsWindow(m_hWnd))
		UpdateData(FALSE);
}

void TextViewDlg::GetText(CString &text)
{
	if (IsWindow(m_hWnd))
		UpdateData(TRUE);

	text = m_edit;

	// accommodate MFC newlines
	text.Replace("\r\n", "\n");
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
