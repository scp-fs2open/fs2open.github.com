/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// ModifyVariableDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "ModifyVariableDlg.h"
#include "parse/sexp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	NO_RESET_FOCUS	0
#define	RESET_FOCUS		1


/////////////////////////////////////////////////////////////////////////////
// CModifyVariableDlg dialog

CModifyVariableDlg::CModifyVariableDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyVariableDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyVariableDlg)
	m_default_value = _T("");
	m_cur_variable_name = _T("");
	//}}AFX_DATA_INIT
}


void CModifyVariableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyVariableDlg)
	DDX_Text(pDX, IDC_MODIFY_DEFAULT_VALUE, m_default_value);
	DDV_MaxChars(pDX, m_default_value, 31);
	DDX_CBString(pDX, IDC_MODIFY_VARIABLE_NAME, m_cur_variable_name);
	DDV_MaxChars(pDX, m_cur_variable_name, 31);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyVariableDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyVariableDlg)
	ON_BN_CLICKED(ID_DELETE_VARIABLE, OnDeleteVariable)
	ON_BN_CLICKED(IDC_TYPE_STRING, OnTypeString)
	ON_BN_CLICKED(IDC_TYPE_NUMBER, OnTypeNumber)
	ON_BN_CLICKED(IDC_TYPE_PLAYER_PERSISTENT, OnTypePlayerPersistent)
	ON_BN_CLICKED(IDC_TYPE_CAMPAIGN_PERSISTENT, OnTypeCampaignPersistent)
	ON_BN_CLICKED(IDC_TYPE_NETWORK_VARIABLE, OnTypeNetworkVariable)
	ON_CBN_SELCHANGE(IDC_MODIFY_VARIABLE_NAME, OnSelchangeModifyVariableName)
	ON_CBN_EDITCHANGE(IDC_MODIFY_VARIABLE_NAME, OnEditchangeModifyVariableName)
	ON_EN_KILLFOCUS(IDC_MODIFY_DEFAULT_VALUE, OnKillfocusModifyDefaultValue)
	ON_CBN_DROPDOWN(IDC_MODIFY_VARIABLE_NAME, OnDropdownModifyVariableName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyVariableDlg message handlers

// Maybe delete variable
void CModifyVariableDlg::OnDeleteVariable() 
{
	CString temp_name;
	int rval;

	// Check for name change
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_MODIFY_VARIABLE_NAME);
	cbox->GetWindowText(temp_name);

	// Can't delete. Name has been changed
	if ( stricmp(Sexp_variables[get_sexp_var_index()].variable_name, temp_name) ) {
		MessageBox("Can not delete variable.  Name has been changed.");
		return;
	}
	
	char message[128] = "Can not delete variable.";
	if (!IsChangeSafe(message))	{
		return;
	}

	// maybe delete variable
	rval = MessageBox("This will permanantly delete the variable.  Do you want to continue?", NULL, MB_OKCANCEL);

	if (rval == IDOK) {
		// delete variable and exit
		m_deleted = true;
		// next statement does UpdataData(TRUE);
		CDialog::OnOK();
	}
}

// Karajorma - Checks whether it is safe to delete or modify this string
bool CModifyVariableDlg::IsChangeSafe(char *message)
{
	// Can't as there are SEXPs using it. 
	int num_counts = m_p_sexp_tree->get_variable_count(Sexp_variables[get_sexp_var_index()].variable_name);
	if (num_counts > 0) {
		char buffer[256];
		sprintf(buffer, "%s Used in %d SEXP(s).", message, num_counts);
		MessageBox(buffer);
		return false;
	}
	
	// Can't as it is used in the team loadout
	num_counts = m_p_sexp_tree->get_loadout_variable_count(get_sexp_var_index());
	if (num_counts > 0) {
		char buffer[256];
		sprintf(buffer, "%s Used in %d location(s) in loadout.", message, num_counts);
		MessageBox(buffer);
		return false;
	}

	return true;
}

// Set type to string
void CModifyVariableDlg::OnTypeString() 
{
	// check if type actually modified
	if (m_type_number == true) {

		// Don't allow type change if in use.		
		char message[128] = "Can not modify variable type.";
		if (!IsChangeSafe(message))	{
			m_type_number = true;
			m_modified_type = false;
			set_variable_type();
			return;
		}

		// keep track if type is really changed
		if (m_modified_type == true) {
			m_modified_type = false;
		} else {
			m_modified_type = true;
		}
	}
	m_type_number = false;
	set_variable_type();
}

// Set type to number
void CModifyVariableDlg::OnTypeNumber() 
{
	// check if type actually modified
	if (m_type_number == false) {
		
		// Don't allow type change if in use.			
		char message[128] = "Can not modify variable type.";
		if (!IsChangeSafe(message))	{
			m_type_number = false;
			m_modified_type = false;
			set_variable_type();
			return;
		}

		// keep track if type is really changed
		if (m_modified_type == true) {
			m_modified_type = false;
		} else {
			m_modified_type = true;
		}
	}
	m_type_number = true;
	set_variable_type();
}

// set player persistence
void CModifyVariableDlg::OnTypePlayerPersistent()
{
	m_type_player_persistent = ((CButton *) GetDlgItem(IDC_TYPE_PLAYER_PERSISTENT))->GetCheck() ? true : false;

	// keep track if type is really changed (ugh - just force it to be always changed)
	m_modified_persistence = true;

	if (m_type_player_persistent)
		m_type_campaign_persistent = false;

	set_variable_type();
}

// set campaign persistence
void CModifyVariableDlg::OnTypeCampaignPersistent()
{
	m_type_campaign_persistent = ((CButton *) GetDlgItem(IDC_TYPE_CAMPAIGN_PERSISTENT))->GetCheck() ? true : false;

	// keep track if type is really changed (ugh - just force it to be always changed)
	m_modified_persistence = true;

	if (m_type_campaign_persistent)
		m_type_player_persistent = false;

	set_variable_type();
}

void CModifyVariableDlg::OnTypeNetworkVariable()
{
	m_type_network_variable = ((CButton *) GetDlgItem(IDC_TYPE_NETWORK_VARIABLE))->GetCheck() ? true : false;

	m_modified_type = true; 

	set_variable_type();
}

void CModifyVariableDlg::OnSelchangeModifyVariableName()
{
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_MODIFY_VARIABLE_NAME);

	// get index of current selection
	int index = cbox->GetCurSel();

	// check an item was actually selected, and not outside the box
	if (index == CB_ERR) {
		return;
	}

	// check if another has been modified
	if (m_modified_type || m_modified_persistence || m_modified_name || m_modified_value) {
		
		// Don't send message if changing to self
		if (index != m_combo_last_modified_index) {
			MessageBox("Can only modify one variable.");
		}

		//reset focus to current
		cbox->SetCurSel(m_combo_last_modified_index);
		return;
	}

	m_combo_last_modified_index = index;

	// Get index into sexp_variables
	int sexp_variable_index = get_sexp_var_index();
	
	// Set new type for selection
	m_type_number = ((Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NUMBER) != 0) ? true : false;
	m_type_campaign_persistent = ((Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT) != 0) ? true : false;
	m_type_player_persistent = ((Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_PLAYER_PERSISTENT) != 0) ? true : false;
	m_type_network_variable = ((Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_NETWORK) != 0) ? true : false;
	set_variable_type();

	// Set new default value for selection
	if (sexp_variable_index > -1) {
		CEdit *edit = (CEdit *) GetDlgItem(IDC_MODIFY_DEFAULT_VALUE);
		edit->SetWindowText(Sexp_variables[sexp_variable_index].text);
	}
}

// Check if variable name has changed from Sexp_variables[].varaible name
void CModifyVariableDlg::OnEditchangeModifyVariableName() 
{
	// Do string compare to check for change
	CString temp_name;

	// Get current variable name
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_MODIFY_VARIABLE_NAME);
	cbox->GetWindowText(temp_name);

	// Check if variable name is modified
	if ( strcmp(Sexp_variables[get_sexp_var_index()].variable_name, temp_name) ) {
		m_modified_name = true;
	} else {
		m_modified_name = false;
	}
}

BOOL CModifyVariableDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int i, box_index;
	// Init combo box and translation table from combo box to sexp_varaibles
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_MODIFY_VARIABLE_NAME);
	cbox->ResetContent();

	for (i=0; i<MAX_SEXP_VARIABLES; i++) {
		m_translate_combo_to_sexp[i] = -1;
	}

	// initialize list -- Box is set to *not* sort
	for (i=0; i<MAX_SEXP_VARIABLES; i++) {
		if ((Sexp_variables[i].type & SEXP_VARIABLE_SET) && !(Sexp_variables[i].type & SEXP_VARIABLE_BLOCK)){
			box_index = cbox->AddString(Sexp_variables[i].variable_name);
			
			// check no error
			if ( !((box_index == CB_ERR) || (box_index == CB_ERRSPACE)) ) {
				m_translate_combo_to_sexp[box_index] = i;
			}
		}
	}
	
	// Exit gracefully if nothing added to combo box
	if (cbox->GetCount() == 0) {
		Int3();	// this should not happen
		OnCancel();
	}

	int last_modified = 0;
	// Set current variable
	if (m_start_index > -1) {
		for (i=0; i<MAX_SEXP_VARIABLES; i++) {
			if (m_translate_combo_to_sexp[i] == m_start_index) {
				last_modified = i;
				break;
			}
		}
	}

	m_combo_last_modified_index = last_modified;
	cbox->SetCurSel(last_modified);

	// Set the default value
	if (m_translate_combo_to_sexp[last_modified] > -1) {
		CEdit *edit = (CEdit *) GetDlgItem(IDC_MODIFY_DEFAULT_VALUE);
		edit->SetWindowText(Sexp_variables[m_translate_combo_to_sexp[last_modified]].text);
	}

	// Set old variable name
	m_old_var_name = Sexp_variables[m_translate_combo_to_sexp[last_modified]].variable_name;
	
	// Set type
	m_type_number = ((Sexp_variables[last_modified].type & SEXP_VARIABLE_NUMBER) != 0) ? true : false;
	m_type_campaign_persistent = ((Sexp_variables[last_modified].type & SEXP_VARIABLE_CAMPAIGN_PERSISTENT) != 0) ? true : false;
	m_type_player_persistent = ((Sexp_variables[last_modified].type & SEXP_VARIABLE_PLAYER_PERSISTENT) != 0) ? true : false;
	m_type_network_variable = ((Sexp_variables[last_modified].type & SEXP_VARIABLE_NETWORK) != 0) ? true : false;
	set_variable_type();

	// keep track of changes
	m_modified_name  = false;
	m_modified_value = false;
	m_modified_type  = false;
	m_modified_persistence = false;
	m_deleted		  = false;
	m_do_modify		  = false;

	m_data_validated		= false;
	m_var_name_validated = false;


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyVariableDlg::set_variable_type()
{
	// get buttons
	CButton *button_string = (CButton *) GetDlgItem(IDC_TYPE_STRING);
	CButton *button_number = (CButton *) GetDlgItem(IDC_TYPE_NUMBER);

	// assign state
	button_number->SetCheck( m_type_number);
	button_string->SetCheck(!m_type_number);

	((CButton *) GetDlgItem(IDC_TYPE_CAMPAIGN_PERSISTENT))->SetCheck(m_type_campaign_persistent);
	((CButton *) GetDlgItem(IDC_TYPE_PLAYER_PERSISTENT))->SetCheck(m_type_player_persistent);
	((CButton *) GetDlgItem(IDC_TYPE_NETWORK_VARIABLE))->SetCheck(m_type_network_variable);
}

void CModifyVariableDlg::OnOK() 
{
	CString temp_data;

	// Validate data
	CEdit *edit = (CEdit *) GetDlgItem(IDC_MODIFY_DEFAULT_VALUE);
	edit->GetWindowText(temp_data);

	// validate data
	validate_data(temp_data, RESET_FOCUS);
	if (m_data_validated) {
		// Don't get OnKillfocusModifyDefaultValue when ok
		if (!m_modified_value) {
			if ( strcmp(Sexp_variables[get_sexp_var_index()].text, temp_data) ) {
				m_modified_value = true;
			}
		}

		// validate variable name
		validate_var_name(RESET_FOCUS);
		if (m_var_name_validated) {

			// maybe set m_do_modify -- this is needed. compare with OnCancel()
			if (m_modified_name || m_modified_persistence || m_modified_value || m_modified_type) {
				m_do_modify = true;
			}
			CDialog::OnOK();
		}
	}
}

void CModifyVariableDlg::OnKillfocusModifyDefaultValue() 
{
	// Do string compare to check for change
	CString temp_data;

	CEdit *edit = (CEdit *) GetDlgItem(IDC_MODIFY_DEFAULT_VALUE);
	edit->GetWindowText(temp_data);

	if ( strcmp(Sexp_variables[get_sexp_var_index()].text, temp_data) ) {
		m_modified_value = true;
	} else {
		m_modified_value = false;
	}
}

// validate data
// check (1)  zero length (2) invalid chars (3) value if numberf
void CModifyVariableDlg::validate_data(CString &temp_data, int set_focus)
{
	// display invalid data message
	bool message = false;
	char message_text[256];

	// check length > 0
	int length  = strlen(temp_data);
	if (length == 0) {
		strcpy_s(message_text, "Invalid  Default Value");
		message = true;

	} else if (m_type_number) {
		// check if string and str(atoi(stri)) are same
		int temp_num = atoi(temp_data);
		char buf[TOKEN_LENGTH];
		sprintf(buf, "%d", temp_num);

		if ( stricmp(buf, temp_data) ) {
			message = true;
			strcpy_s(message_text, "Invalid  Default Value");
		} else {
			message = false;
		}
	}

	// check for invalid characters
	int rval = strcspn(temp_data, "@()");
	if (rval != length) {
		message = true;
		sprintf(message_text, "Invalid char '%c' in Default Value", temp_data[rval]);
	}

	// display message
	if ( message ) {
		m_data_validated = false;
		MessageBox(message_text);

		// reset focus
		if (set_focus == RESET_FOCUS) {
			CEdit *edit = (CEdit *) GetDlgItem(IDC_MODIFY_DEFAULT_VALUE);
			edit->SetFocus();
			edit->SetSel(0, -1);
		}
	}

	// string always ok, numbers if no message
	m_data_validated = !message;
}

// validate variable name
// check (1) zero length (2) invalid chars (3) already in use
void CModifyVariableDlg::validate_var_name(int set_focus)
{
	CString temp_name;
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_MODIFY_VARIABLE_NAME);
	cbox->GetWindowText(temp_name);

	int cur_sel = cbox->GetCurSel();
	m_old_var_name = Sexp_variables[m_translate_combo_to_sexp[cur_sel]].variable_name;

	// display invalid data message
	bool message = false;
	char message_text[256];

	// check length > 0
	int length  = strlen(temp_name);
	if (length == 0) {
		strcpy_s(message_text, "Invalid Variable Name");
		message = true;
	} else {

		// check for invalid characters
		int rval = strcspn(temp_name, "@()");
		if (rval != length) {
			message = true;
			sprintf(message_text, "Invalid char '%c' in Variable Name", temp_name[rval]);
		} else {
			int index = get_index_sexp_variable_name(temp_name);

			// if not a new name and not start name
			if ( (index != -1) && (index != m_translate_combo_to_sexp[m_combo_last_modified_index]) ) {
				message = true;
				strcpy_s(message_text, "Variable Name already in use");
			}
		}
	}

	// display message
	if ( message ) {
		MessageBox(message_text);

		// reset focus
		if (set_focus == RESET_FOCUS) {
			cbox->SetFocus();
		}
	}

	// set var_name_validated
	m_var_name_validated = !message;
}


int CModifyVariableDlg::get_sexp_var_index()
{
	int index = m_translate_combo_to_sexp[m_combo_last_modified_index];
	Assert( (index >= 0) && (index < MAX_SEXP_VARIABLES) );

	return index;
}

// Reset text in drop down list
void CModifyVariableDlg::OnDropdownModifyVariableName() 
{
	CString temp_name;

	// Get current variable name
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_MODIFY_VARIABLE_NAME);
	cbox->GetWindowText(temp_name);

	// Reset combo box text
	int rval;
	rval = cbox->InsertString(m_combo_last_modified_index, temp_name);
	if ( (rval == CB_ERR) || (rval == CB_ERRSPACE) ) {
		AfxMessageBox("An internal error has occured.");
		OnCancel();
	}
	cbox->DeleteString(m_combo_last_modified_index+1);

	cbox->SetCurSel(m_combo_last_modified_index);
}
