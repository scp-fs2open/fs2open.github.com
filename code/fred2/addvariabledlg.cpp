/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// AddVariableDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "AddVariableDlg.h"
#include "parse/sexp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_RESET_FOCUS	0
#define RESET_FOCUS		1

/////////////////////////////////////////////////////////////////////////////
// CAddVariableDlg dialog


CAddVariableDlg::CAddVariableDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddVariableDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddVariableDlg)
	m_default_value = _T("");
	m_variable_name = _T("");
	//}}AFX_DATA_INIT
}


void CAddVariableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddVariableDlg)
	DDX_Text(pDX, IDC_ADD_VARIABLE_DEFAULT_VALUE, m_default_value);
	DDV_MaxChars(pDX, m_default_value, 31);
	DDX_Text(pDX, IDC_ADD_VARIABLE_NAME, m_variable_name);
	DDV_MaxChars(pDX, m_variable_name, 31);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddVariableDlg, CDialog)
	//{{AFX_MSG_MAP(CAddVariableDlg)
	ON_BN_CLICKED(IDC_TYPE_NUMBER, OnTypeNumber)
	ON_BN_CLICKED(IDC_TYPE_STRING, OnTypeString)
	ON_BN_CLICKED(IDC_TYPE_CAMPAIGN_PERSISTENT, OnTypeCampaignPersistent)
	ON_BN_CLICKED(IDC_TYPE_PLAYER_PERSISTENT, OnTypePlayerPersistent)
	ON_BN_CLICKED(IDC_TYPE_NETWORK_VARIABLE, OnTypeNetworkVariable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddVariableDlg message handlers

void CAddVariableDlg::OnOK() 
{
	// validation name
	validate_variable_name(RESET_FOCUS);

	// validate data
	if ( m_name_validated ) {
		validate_data(RESET_FOCUS);
	}

	// both ok, then store results
	if ( m_name_validated && m_data_validated ) {
//		int sexp_add_variable(char *text, char*, int type);
//		char temp_name[32];
//		char temp_value[32];
//		strcpy_s(temp_name, m_variable_name);
//		strcpy_s(temp_value, m_default_value);
		// SEXP_VARIABLE_NUMBER SEXP_VARIABLE_STRING
//		int type;
//
//		if (m_type_number) {
//			type = SEXP_VARIABLE_NUMBER;
//		} else {
//			type = SEXP_VARIABLE_STRING;
//		}
//
//		// Goober5000
//		if (m_type_player_persistent) {
//			type |= SEXP_VARIABLE_PLAYER_PERSISTENT;
//		} else if (m_type_campaign_persistent) {
//			type |= SEXP_VARIABLE_CAMPAIGN_PERSISTENT;
//		}
//
//		m_sexp_var_index = sexp_add_variable(temp_value, temp_name, type);
//		this get done for free CDialog::OnOk() UpdateData(TRUE);
		m_create = true;

		CDialog::OnOK();
	}
}

BOOL CAddVariableDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_variable_name = "<Variable Name>";
	m_default_value = "<Default Value>";

	// Set variable type to number
	m_type_number = true;
	m_type_campaign_persistent = false;
	m_type_player_persistent = false;
	m_type_network_variable = false;
	set_variable_type();

	m_name_validated = false;
	m_data_validated = false;
	m_create = false;

	// Send default name and values into dialog box
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

bool is_sexp_variable_name(const char* temp_name)
{
	for (int i=0; i<MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_SET) {
			if ( !strcmp(Sexp_variables[i].text, temp_name) ) {
				return false;
			}
		}
	}
	// name not found
	return true;
}


// Check variable name (1) changed (2) > 0 length (3) does not already exist
void CAddVariableDlg::validate_variable_name(int set_focus)
{
	CString temp_name;

	CEdit *edit = (CEdit *) GetDlgItem(IDC_ADD_VARIABLE_NAME);
	edit->GetWindowText(temp_name);

	// Check if any change and not already in list
	if ( stricmp(temp_name, "<Variable Name>") ) {
		if ( (strlen(temp_name) > 0) && (get_index_sexp_variable_name(LPCTSTR(temp_name)) == -1) ) { //not already in list and length > 0 {
			// Goober5000 - replace spaces with hyphens
			if (strchr(temp_name, ' ') != NULL)
			{
				// display msg
				MessageBox("Variable names cannot contain spaces.  Replacing with hyphens.");

				// replace chars
				//Modified to make the code build. - Thunderbird
				temp_name.Replace((TCHAR) ' ', (TCHAR) '-');
				//temp_name.Replace((TCHAR) ' ', (TCHAR) '-'));

				// fix what's displayed
				edit->SetFocus();
				edit->SetWindowText(temp_name);
				edit->SetSel(0, -1);
			}

			m_name_validated = true;
		} else {
			// conflicting variable name
			if (strlen(temp_name) == 0) {
				edit->SetWindowText("<Variable Name>");
			}
			m_name_validated = false;
			if (set_focus == RESET_FOCUS) {
				MessageBox("Conflicting variable name");
				edit->SetFocus();
				edit->SetSel(0, -1);
			}
		}
	} else {
		// name unchanged from default
		m_name_validated = false;
		if (set_focus == RESET_FOCUS) {
			MessageBox("Invalid variable name");
			edit->SetFocus();
			edit->SetSel(0, -1);
		}
	}

}

void CAddVariableDlg::validate_data(int set_focus)
{
	CString temp_data;

	CEdit *edit = (CEdit *) GetDlgItem(IDC_ADD_VARIABLE_DEFAULT_VALUE);
	edit->GetWindowText(temp_data);

	// check for 0 string length
	if (strlen(temp_data) == 0) {
		m_data_validated = false;
	} else {
		if (m_type_number) {
			// verify valid number
			int temp_num = atoi(temp_data);
			char buf[TOKEN_LENGTH];
			sprintf(buf, "%d", temp_num);

			if ( stricmp(buf, temp_data) ) {
				m_data_validated = false;
			} else {
				m_data_validated = true;
			}
		} else {
			m_data_validated = true;
		}
	}

	// Display message and reset focus
	if ( (!m_data_validated) && (set_focus == RESET_FOCUS) ) {
		MessageBox("Invalid Default Value.");
		edit->SetFocus();
		edit->SetSel(0, -1);
	}
}

// Set type to number
void CAddVariableDlg::OnTypeNumber() 
{
	m_type_number = true;
	set_variable_type();
}

// Set type to string
void CAddVariableDlg::OnTypeString() 
{
	m_type_number = false;
	set_variable_type();
}

void CAddVariableDlg::OnTypePlayerPersistent()
{
	m_type_player_persistent = ((CButton *) GetDlgItem(IDC_TYPE_PLAYER_PERSISTENT))->GetCheck() ? true : false;

	if (m_type_player_persistent)
		m_type_campaign_persistent = false;

	set_variable_type();
}

void CAddVariableDlg::OnTypeCampaignPersistent()
{
	m_type_campaign_persistent = ((CButton *) GetDlgItem(IDC_TYPE_CAMPAIGN_PERSISTENT))->GetCheck() ? true : false;

	if (m_type_campaign_persistent)
		m_type_player_persistent = false;

	set_variable_type();
}

void CAddVariableDlg::OnTypeNetworkVariable()
{
	m_type_network_variable = ((CButton *) GetDlgItem(IDC_TYPE_NETWORK_VARIABLE))->GetCheck() ? true : false;
	set_variable_type();
}

// Set type check boxes
void CAddVariableDlg::set_variable_type()
{

	CButton *button_string = (CButton *) GetDlgItem(IDC_TYPE_STRING);
	CButton *button_number = (CButton *) GetDlgItem(IDC_TYPE_NUMBER);

	button_number->SetCheck( m_type_number);
	button_string->SetCheck(!m_type_number);

	((CButton *) GetDlgItem(IDC_TYPE_CAMPAIGN_PERSISTENT))->SetCheck(m_type_campaign_persistent);
	((CButton *) GetDlgItem(IDC_TYPE_PLAYER_PERSISTENT))->SetCheck(m_type_player_persistent);
	((CButton *) GetDlgItem(IDC_TYPE_NETWORK_VARIABLE))->SetCheck(m_type_network_variable);
}
