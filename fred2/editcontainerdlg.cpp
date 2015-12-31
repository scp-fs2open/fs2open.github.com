/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

// EditContainerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "EditContainerDlg.h"
#include "parse/sexp.h"
#include "EditContainerAddNewDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_RESET_FOCUS	0
#define RESET_FOCUS		1

/////////////////////////////////////////////////////////////////////////////
// EditContainerDlg dialog


CEditContainerDlg::CEditContainerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditContainerDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CEditContainerDlg)
	/*
	m_default_value = _T("");
	*/
	// m_container_name = _T("");
	
	//}}AFX_DATA_INIT
}


void CEditContainerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditContainerDlg)
	DDX_Control(pDX, IDC_CONTAINER_DATA_LISTER, m_container_data_lister);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditContainerDlg, CDialog)
	//{{AFX_MSG_MAP(CEditVariableDlg))
	
	ON_BN_CLICKED(IDC_CONTAINER_ADD, OnContainerAdd)
	ON_BN_CLICKED(IDC_CONTAINER_INSERT, OnContainerInsert)
	ON_BN_CLICKED(IDC_CONTAINER_REMOVE, OnContainerRemove)
	ON_BN_CLICKED(IDC_CONTAINER_UPDATE, OnContainerUpdate)
	
	ON_BN_CLICKED(IDC_CONTAINER_LIST, OnTypeList)
	ON_BN_CLICKED(IDC_CONTAINER_MAP, OnTypeMap)
	ON_BN_CLICKED(IDC_CONTAINER_NUMBER, OnTypeNumber)
	ON_BN_CLICKED(IDC_CONTAINER_STRING, OnTypeString)
	
	ON_LBN_SELCHANGE(IDC_CONTAINER_DATA_LISTER, OnListerSelectionChange)

	ON_CBN_SELCHANGE(IDC_CURRENT_CONTAINER_NAME, OnSelchangeContainerName)
	ON_CBN_EDITCHANGE(IDC_CURRENT_CONTAINER_NAME, OnEditchangeContainerName)

	ON_BN_CLICKED(IDC_ADD_NEW_CONTAINER, &CEditContainerDlg::OnBnClickedAddNewContainer)

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_STRING_KEYS, &CEditContainerDlg::OnBnClickedStringKeys)
	ON_BN_CLICKED(IDC_NUMBER_KEYS, &CEditContainerDlg::OnBnClickedNumberKeys)
	ON_BN_CLICKED(IDC_DELETE_CONTAINER, &CEditContainerDlg::OnBnClickedDeleteContainer)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditContainerDlg message handlers

BOOL CEditContainerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_type_list = true;
	m_type_map = false;
	m_type_number = false; 
	m_key_type_number = true; 
	m_data_changed = false; 

	//grab the existing list of containers and duplicate it. We only update it if the user clicks OK. 
	edit_sexp_containers = Sexp_containers;

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	cbox->ResetContent();
		
	raw_data.clear(); 

	// do we already have any containers defined? 
	if (!edit_sexp_containers.empty()) {
		cbox->EnableWindow(true); 
		for (int i = 0; i < (int)edit_sexp_containers.size(); i++) {
			cbox->AddString(edit_sexp_containers[i].container_name.c_str()); 
		}
		m_current_container = 0; 
		cbox->SetCurSel(m_current_container); 
		set_selected_container(m_current_container);
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(true);	
	}
	else {
		set_container_type();
		set_argument_type(); 
		set_key_type();

		m_current_container = -1; 
	}

	// Send default name and values into dialog box
	UpdateData(FALSE);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditContainerDlg::set_selected_container(int selected_container)
{
	Assert (selected_container >= 0 && selected_container <  (int)edit_sexp_containers.size()); 

	if (edit_sexp_containers[selected_container].type & SEXP_CONTAINER_NUMBER_DATA) {
		m_type_number = true; 
	}
	else {
		m_type_number = false;
	}

	if (edit_sexp_containers[selected_container].type & SEXP_CONTAINER_LIST) {
		m_type_list = true;
		m_type_map = false;	

		GetDlgItem(IDC_CONTAINER_KEY)->EnableWindow(false);
		GetDlgItem(IDC_KEY_TYPE)->EnableWindow(false);
		GetDlgItem(IDC_STRING_KEYS)->EnableWindow(false);
		GetDlgItem(IDC_NUMBER_KEYS)->EnableWindow(false);
		GetDlgItem(IDC_CONTAINER_DATA)->EnableWindow(true);	
	}
	else {
		m_type_list = false;
		m_type_map = true;	

		if (edit_sexp_containers[selected_container].type & SEXP_CONTAINER_NUMBER_KEYS) {
			m_key_type_number = true;
		}
		else {
			m_key_type_number = false;
		}

		GetDlgItem(IDC_CONTAINER_KEY)->EnableWindow(true);
		GetDlgItem(IDC_KEY_TYPE)->EnableWindow(true);
		GetDlgItem(IDC_STRING_KEYS)->EnableWindow(true);
		GetDlgItem(IDC_NUMBER_KEYS)->EnableWindow(true);
		GetDlgItem(IDC_CONTAINER_DATA)->EnableWindow(true);	
	}	

	raw_data.clear(); 
		
	set_container_type(); 
	set_argument_type(); 
	set_key_type();

	set_lister_data(selected_container); 

}

void CEditContainerDlg::set_lister_data(int index)
{
	int i; 
	Assert (index >= 0 && index <  (int)edit_sexp_containers.size()); 

	raw_data.clear(); 
	
	if (edit_sexp_containers[index].type & SEXP_CONTAINER_LIST) {
		for (i = 0; i < (int)edit_sexp_containers[index].list_data.size(); i++) {
			raw_data.push_back(edit_sexp_containers[index].list_data[i]);
		}
	}
	else if  (edit_sexp_containers[index].type & SEXP_CONTAINER_MAP) {
		for (SCP_unordered_map<SCP_string, SCP_string>::iterator map_iter = edit_sexp_containers[index].map_data.begin(); map_iter != edit_sexp_containers[index].map_data.end(); map_iter++) {
			raw_data.push_back(map_iter->first); 
			raw_data.push_back(map_iter->second); 
		}
	}

	updata_data_lister();
}

void CEditContainerDlg::OnOK() 
{
	if (m_current_container >=0) {
		// if we can't save changed data, don't quit
		if (m_data_changed) {
			if (!save_current_container()) {
				return;
			}
		}
	}

	// swap the contents of the edit_sexp_containers vector into the real Sexp_containers vector
	Sexp_containers.swap(edit_sexp_containers); 
	CDialog::OnOK();
}

void CEditContainerDlg::OnBnClickedStringKeys()
{
	m_key_type_number = false;
	m_data_changed = true; 
}

void CEditContainerDlg::OnBnClickedNumberKeys()
{
	m_key_type_number = true;
	m_data_changed = true; 
}

// Set type to list
void CEditContainerDlg::OnTypeList() 
{
	m_type_list = true;
	m_type_map = false; 
	m_data_changed = true; 

	//set_container_type(); 
	updata_data_lister();

	GetDlgItem(IDC_CONTAINER_KEY)->EnableWindow(false);
	GetDlgItem(IDC_KEY_TYPE)->EnableWindow(false);
	GetDlgItem(IDC_STRING_KEYS)->EnableWindow(false);
	GetDlgItem(IDC_NUMBER_KEYS)->EnableWindow(false);
	GetDlgItem(IDC_CONTAINER_DATA)->EnableWindow(true);
}

// Set type to map
void CEditContainerDlg::OnTypeMap() 
{
	if (!raw_data.empty()) {
		if (raw_data.size() %2 != 0) {
			MessageBox("Can not switch to Map type with an odd number of entries. Entries will be switched to key - value pairs so there must be an even number of entries.");
			set_container_type(); 
			return; 
		}
	}
	m_type_list = false;
	m_type_map = true;
	m_data_changed = true; 

	//set_container_type();   
	updata_data_lister();

	// since we're switching from a list, set the type of key to be the same as the type for the data. That way if there is already data entered, it will be the correct type. 
	m_key_type_number = m_type_number; 

	GetDlgItem(IDC_CONTAINER_KEY)->EnableWindow(true);
	GetDlgItem(IDC_KEY_TYPE)->EnableWindow(true);
	GetDlgItem(IDC_STRING_KEYS)->EnableWindow(true);
	GetDlgItem(IDC_NUMBER_KEYS)->EnableWindow(true);
	GetDlgItem(IDC_CONTAINER_DATA)->EnableWindow(true);
}

// Set type check boxes
void CEditContainerDlg::set_container_type()
{
	CButton *button_list = (CButton *) GetDlgItem(IDC_CONTAINER_LIST);
	CButton *button_map = (CButton *) GetDlgItem(IDC_CONTAINER_MAP);

	button_list->SetCheck( m_type_list);
	button_map->SetCheck(m_type_map);
}


// Set type to number
void CEditContainerDlg::OnTypeNumber() 
{
	m_type_number = true;
	m_data_changed = true; 
//	set_argument_type();
}


// Set type to string
void CEditContainerDlg::OnTypeString() 
{
	m_type_number = false;
	m_data_changed = true; 
//	set_argument_type();
}


// Set type check boxes
void CEditContainerDlg::set_argument_type()
{
	CButton *button_string = (CButton *) GetDlgItem(IDC_CONTAINER_STRING);
	CButton *button_number = (CButton *) GetDlgItem(IDC_CONTAINER_NUMBER);

	button_number->SetCheck( m_type_number);
	button_string->SetCheck(!m_type_number);
}


// Set type check boxes
void CEditContainerDlg::set_key_type()
{
	CButton *button_string = (CButton *) GetDlgItem(IDC_STRING_KEYS);
	CButton *button_number = (CButton *) GetDlgItem(IDC_NUMBER_KEYS);

	button_number->SetCheck( m_key_type_number);
	button_string->SetCheck(!m_key_type_number);
}

void CEditContainerDlg::OnSelchangeContainerName()
{
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);

	// save the currently selected container before we do anything with the new one
	if (m_data_changed) {
		if (!save_current_container()) {
			//we couldn't save the old container, so back out of the container change.
			Assert(m_current_container >= 0);
			cbox->SetCurSel(m_current_container); 
			return;
		}
	}


	m_current_container = cbox->GetCurSel(); 

	set_selected_container(m_current_container);
	
	m_data_changed = false; 
}


void CEditContainerDlg::OnEditchangeContainerName()
{
	CString new_name;

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	int selected_container = cbox->GetCurSel();
	cbox->GetWindowText(new_name);

	if (is_container_name_valid(new_name)) {
		edit_sexp_containers[selected_container].container_name = new_name; 
	}
}


// basically the same as the function in sexp.cpp but checks the backed up edit_sexp_containers used by this dialog instead. 
int CEditContainerDlg::get_index_edit_sexp_container_name(const char *text)
{
	for (int i = 0; i < (int)edit_sexp_containers.size() ; i++) {
		if ( !stricmp(edit_sexp_containers[i].container_name.c_str(), text) ) {
			return i;
		}
	}

	// not found
	return -1;
}



// Check variable name (1) changed (2) > 0 length (3) does not already exist - pretty much stolen from addvariabledlg.cpp
BOOL CEditContainerDlg::is_container_name_valid(CString &new_name)
{
	bool name_validated = false;	// assume invalid until proved wrong

	// Check if not default name
	if ( stricmp(new_name, "<New Container Name>") ) {
		bool to_do_check_if_spaces_okay; 
		/*
		// remove any spaces
		if (strchr(new_name, ' ') != NULL) {
			// display msg
			MessageBox("Container names cannot contain spaces.  Replacing with hyphens.");

			// replace chars
			new_name.Replace((TCHAR) ' ', (TCHAR) '-');
		}
		*/

		// remove any ampersands too
		if (strchr(new_name, '&') != NULL) {
			// display msg
			MessageBox("Container names cannot contain &.  Replacing with hyphens.");

			// replace chars
			new_name.Replace((TCHAR) ' ', (TCHAR) '-');
		}

		//not already in list and length > 0
		if ( (strlen(new_name) > 0) && (get_index_edit_sexp_container_name(LPCTSTR(new_name)) == -1) ) { 			
			name_validated = true;
		}
		else {
			// conflicting container name
			name_validated = false;
			MessageBox("Conflicting container name. A container with this name already exists");
		}
	}
	else {
		// name unchanged from default
		name_validated = false;
		MessageBox("Invalid container name. Please write a name in the box above then click Add Container.");
	}

	return name_validated; 
}

BOOL CEditContainerDlg::is_valid_number(SCP_string test_string) {
	// verify valid number
	int temp_num = atoi(test_string.c_str());
	char buf[TOKEN_LENGTH];
	sprintf(buf, "%d", temp_num);

	if ( stricmp(buf, test_string.c_str()) ) {
		return false;
	} else {
		return true; 
	}
}


BOOL CEditContainerDlg::is_data_valid()
{
	int i; 

	if (m_type_map) {
		if ( raw_data.size() %2 != 0 ) {
			MessageBox("Data is corrupt, you have more keys than data entries.");
		}
	}

	// type check data
	// ToDo - type check strings if using strict type checking
	int YouHaveMoreToDo; 

	// if not strict typing all strings are fine regardless of whether we're dealing with maps or lists.
	if (!m_type_number && (m_type_list || (m_type_map && !m_key_type_number))) {
		return true; 
	}

	else if (m_type_map ) {
		// start at the first data entry
		for (i = 1; i < (int)raw_data.size(); i = i+2) {
			if (!is_valid_number(raw_data[i])) {
				return false; 
			}
		}
	}
	else if (m_type_list) {
		for (i = 0; i < (int)raw_data.size(); i++) {
			if (!is_valid_number(raw_data[i])) {
				return false; 
			}
		}
	}
	// unknown container type
	else {
		return false; 
	}

	// if we made it this far, the data should be fine
	return true;
}

void CEditContainerDlg::OnListerSelectionChange()
{	
	int index = m_container_data_lister.GetCurSel(); 

	if (m_type_map) {
		index = index * 2; 
		CEdit *edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
		edit->SetWindowText(raw_data[index].c_str());

		CEdit *edit2 = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
		edit2->SetWindowText(raw_data[index+1].c_str());
	}
	else {
		CEdit *edit2 = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
		edit2->SetWindowText(raw_data[index].c_str());
	}
}

void CEditContainerDlg::OnContainerAdd()
{
	if (!edit_boxes_have_valid_data()) {
		return;
	}
	
	CString temp_string; 
	SCP_string temp_scp_string; 

	// for maps, we first need to get the key
	if (m_type_map) {
		CEdit *edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
		edit->GetWindowText(temp_string);
		raw_data.push_back(temp_scp_string.assign(temp_string));
	}
	
	// for both types we need to get the data
	CEdit *edit2 = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	edit2->GetWindowText(temp_string);
	raw_data.push_back(temp_scp_string.assign(temp_string));

	m_data_changed = true; 

	updata_data_lister();

}


void CEditContainerDlg::OnContainerInsert()
{
	if (!edit_boxes_have_valid_data()) {
		return;
	}
	
	CString temp_string; 
	SCP_string temp_scp_string; 

	SCP_vector<SCP_string>::iterator iter ;
	
	// for both types we need to get the data
	CEdit *edit2 = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	edit2->GetWindowText(temp_string);
	ListerSelectionGetIter(iter);
	raw_data.insert(iter, temp_scp_string.assign(temp_string));

	// insert the key before the newly added data entry
	if (m_type_map) {
		CEdit *edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
		edit->GetWindowText(temp_string);
		ListerSelectionGetIter(iter);
		raw_data.insert(iter, temp_scp_string.assign(temp_string));
	}

	m_data_changed = true; 

	updata_data_lister();
}

void CEditContainerDlg::OnContainerRemove()
{
	int index = m_container_data_lister.GetCurSel(); 
	SCP_vector<SCP_string>::iterator iter = raw_data.begin();

	// find the deletion point
	if (m_type_map) {
		index = index * 2; 
	}

	raw_data.erase(raw_data.begin() + index); 
	// for maps we need to delete the key and data
	if (m_type_map) {
		raw_data.erase(raw_data.begin() + index); 
	}
	
	m_data_changed = true; 

	updata_data_lister();
}

void CEditContainerDlg::OnContainerUpdate()
{
	if (!data_edit_box_has_valid_data()) {
		return;
	}

	int index = m_container_data_lister.GetCurSel(); 

	if (index == LB_ERR) {
		MessageBox("Nothing selected! Use Add to enter new data");
		return; 
	}

	SCP_vector<SCP_string>::iterator iter = raw_data.begin();

	// find the entry to update
	if (m_type_map) {
		index = index * 2;
	}

	CString temp_string; 
	
	// for maps, we first need to get the key
	if (m_type_map) {
		CEdit *edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
		edit->GetWindowText(temp_string);
		raw_data[index].assign(temp_string);
	}
	
	// for both types we need to get the data
	CEdit *edit2 = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	edit2->GetWindowText(temp_string);
	if (m_type_map) {
		raw_data[index+1].assign(temp_string);
	}
	else {
		raw_data[index].assign(temp_string);
	}

	m_data_changed = true; 	

	updata_data_lister();
}

void CEditContainerDlg::ListerSelectionGetIter(SCP_vector<SCP_string>::iterator &iter)
{
	
	int index = m_container_data_lister.GetCurSel(); 

	iter = raw_data.begin();

	// find the entry to update
	if (m_type_map) {
		iter = iter + (index * 2);
	}
	else {
		iter = iter + index; 
	}
}

BOOL CEditContainerDlg::edit_boxes_have_valid_data()
{
	// if we are in list mode, check that the container data box has data in it. 
	if (data_edit_box_has_valid_data()) {
		// if we are in map mode we must check that key box has data and that the key doesn't match an existing key.
		if (m_type_map) {
			if (!key_edit_box_has_valid_data()) {
				return false;
			}
		}
	}
	else {
		return false; 
	}

	return true; 
}

BOOL CEditContainerDlg::key_edit_box_has_valid_data()
{
	CEdit *edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
	CString temp_name;
	edit->GetWindowText(temp_name);

	if (strlen(temp_name) > 0) {		
		if ( raw_data.size() %2 != 0 ) {
			MessageBox("Data is corrupt, you have more keys than data entries.");
			return false;
		}

		for (int i = 0; i < (int)raw_data.size(); i = i+2) {
			if (stricmp(raw_data[i].c_str(), temp_name) == 0) {
				MessageBox("This key already exists. You may not reuse keys in a SEXP map!");
				return false;
			}
		}

		// if we have number keys, we need to check if this key is a number. 
		if (m_key_type_number && !is_valid_number(SCP_string(temp_name))) {
			MessageBox("This key is not a valid number and you have selected number keys! Choose strings if you don't want to use a number");
			return false;
		}

		return true;
	}
	else {
		MessageBox("You have not entered a key!");
	}

	return false; 
}

BOOL CEditContainerDlg::data_edit_box_has_valid_data()
{		
	CEdit *edit = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	CString temp_name;
	edit->GetWindowText(temp_name);

	if (strlen(temp_name) > 0) {
		return true;
	}
	else {
		MessageBox("You have not entered any data!");
		return false;
	}
}


void CEditContainerDlg::updata_data_lister()
{
	m_container_data_lister.ResetContent(); 

	// Data is displayed in one of two ways depending on whether we are populating a list or a map
	if (m_type_list) {
		for (int i = 0; i < (int)raw_data.size(); i++) {
			m_container_data_lister.AddString(raw_data[i].c_str()); 
		}
	}

	if (m_type_map) {
		SCP_string lister_entry; 

		if ( raw_data.size() %2 != 0 ) {
			MessageBox("Data is corrupt, you have more keys than data entries.");
		}
		for (int i = 0; i < (int)raw_data.size(); i = i+2) {
			lister_entry.assign(raw_data[i]); 
			lister_entry.append(" - ");
			lister_entry.append(raw_data[i+1]); 

			m_container_data_lister.AddString(lister_entry.c_str()); 
		}
	}
}

void CEditContainerDlg::OnBnClickedAddNewContainer()
{
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);

	// try to save the old container first
	if (m_data_changed) { 
		if (!save_current_container()) {
			// we couldn't save the old container, don't do anything with the new one yet. 
			return; 
		}
	}
	
	CEditContainerAddNewDlg dlg; 

	dlg.DoModal(); 

	// if we didn't enter a name
	if (dlg.cancelled) {
		return;
	}

	CString temp_name = dlg.m_new_container_name; 

	if (!is_container_name_valid(temp_name)) {	
		return; 
	}

	sexp_container temp_container; 
	temp_container.type = 0; 

	temp_container.container_name = temp_name; 
	if (m_type_map) {
		temp_container.type |= SEXP_CONTAINER_MAP; 

		if (m_key_type_number) {
			temp_container.type |= SEXP_CONTAINER_NUMBER_KEYS; 
		}
		else {
			temp_container.type |= SEXP_CONTAINER_STRING_KEYS;
		}
	}
	else {
		temp_container.type |= SEXP_CONTAINER_LIST; 
	}

	if (m_type_number) {
		temp_container.type |= SEXP_CONTAINER_NUMBER_DATA;
	}
	else {
		temp_container.type |= SEXP_CONTAINER_STRING_DATA;
	}

	edit_sexp_containers.push_back(temp_container); 
	
	// add the new container to the list of containers combo box and make it the selected option
	cbox->AddString(temp_name); 
	m_current_container = (int) edit_sexp_containers.size() - 1; 
	cbox->SetCurSel(m_current_container);

	// get rid of any old data that might still be displayed
	raw_data.clear(); 
	
	m_data_changed = false; 

	updata_data_lister();  

	//maybe activate the delete container button
	if (edit_sexp_containers.size() == 1) {
		cbox->EnableWindow(true);
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(true);	
	}
}

BOOL CEditContainerDlg::save_current_container()
{
	if (!is_data_valid()) {
		MessageBox("Since data is not valid, it can not be saved. Please correct the data before continuing.");
		return false; 
	}

	Assert(m_current_container >= 0);
	
	edit_sexp_containers[m_current_container].type = 0; 

	if (m_type_map) {
		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_MAP; 

		if (m_key_type_number) {
			edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_NUMBER_KEYS; 
		}
		else {
			edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_STRING_KEYS;
		}

		// write the raw data into edit_sexp_containers
		Assertion(raw_data.size() % 2 == 0, "Invalid size for map type"); 
		edit_sexp_containers[m_current_container].map_data.clear();

		for (int i = 0; i < (int)raw_data.size(); i = i+2) {
			edit_sexp_containers[m_current_container].map_data.insert(std::pair<SCP_string, SCP_string>(raw_data[i], raw_data[i+1]));
		}
	}
	else {
		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_LIST; 
		edit_sexp_containers[m_current_container].list_data.clear();

		// write the raw data into edit_sexp_containers
		for (int i = 0; i < (int)raw_data.size(); i++) {
			edit_sexp_containers[m_current_container].list_data.push_back(raw_data[i]);
		}
	} 

	if (m_type_number) {
		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_NUMBER_DATA;
	}
	else {
		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_STRING_DATA;
	}

	return true; 
}
void CEditContainerDlg::OnBnClickedDeleteContainer()
{
	// The button shouldn't even be enabled if there are no containers!
	Assert(!edit_sexp_containers.empty()); 

	Assert(m_current_container >= 0);

	char buffer[256]; 

	int times_used = m_p_sexp_tree->get_container_count(edit_sexp_containers[m_current_container].container_name.c_str()); 
	if (times_used) {
		sprintf(buffer, "Container %s is used in %d events. Please edit those uses out first, then delete the container.", edit_sexp_containers[m_current_container].container_name.c_str(), times_used); 
		MessageBox(buffer);
		return;
	}

	sprintf(buffer, "This will delete the %s container. Are you sure?", edit_sexp_containers[m_current_container].container_name.c_str()); 

	int r = MessageBox(buffer, "Delete?", MB_OKCANCEL | MB_ICONEXCLAMATION);

	if (r == IDCANCEL) {
		return;
	}

	edit_sexp_containers.erase(edit_sexp_containers.begin() + m_current_container);
	m_data_changed = true;

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	cbox->DeleteString((int)cbox->GetCurSel()); 

	m_data_changed = false; // because we don't want to risk trying to save whatever garbage is left back into the containers vector.  

	//maybe de-activate the delete container button
	if (edit_sexp_containers.empty()) {
		m_current_container = -1; 
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(false);	
	}
	else {
		// select the first container
		m_current_container = 0; 
		cbox->SetCurSel(m_current_container);
		set_selected_container(m_current_container);
	}

	
}
