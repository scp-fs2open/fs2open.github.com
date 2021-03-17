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

	//grab the existing list of containers and duplicate it. We only update it if the user clicks OK. 
	edit_sexp_containers = Sexp_containers;

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	cbox->ResetContent();

	// do we already have any containers defined? 
	if (!edit_sexp_containers.empty()) {
		cbox->EnableWindow(true); 
		for (const auto &container : edit_sexp_containers) {
			cbox->AddString(container.container_name.c_str()); 
		}
		m_current_container = 0;
		cbox->SetCurSel(m_current_container); 
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(true);	
	} else {
		m_current_container = -1;
	}
	set_selected_container();

	// Send default name and values into dialog box
	UpdateData(FALSE);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditContainerDlg::set_selected_container()
{	
	update_type_controls();
	update_data_entry_controls();
	set_container_type(); 
	set_data_type();
	if (get_current_container().is_map()) {
		set_key_type();
	}
}

void CEditContainerDlg::OnOK() 
{
	//if (m_current_container >=0) {
	//	// if we can't save changed data, don't quit
	//	//if (m_data_changed) {
	//	// FIXME TEMP: ignore m_data_changed
	//	if (m_current_container >= 0) {
	//		if (!save_current_container()) {
	//			return;
	//		}
	//	}
	//}

	Sexp_containers = std::move(edit_sexp_containers); 
	edit_sexp_containers.clear();
	CDialog::OnOK();
}

void CEditContainerDlg::OnBnClickedStringKeys()
{
	auto &container = get_current_container();
	Assert(container.is_map());
	Assert(container.empty());
	if (!(container.type & SEXP_CONTAINER_STRING_KEYS)) {
		container.type &= ~SEXP_CONTAINER_NUMBER_KEYS;
		container.type |= SEXP_CONTAINER_STRING_KEYS;
	}
}

void CEditContainerDlg::OnBnClickedNumberKeys()
{
	auto &container = get_current_container();
	Assert(container.is_map());
	Assert(container.empty());
	if (!(container.type & SEXP_CONTAINER_NUMBER_KEYS)) {
		container.type &= ~SEXP_CONTAINER_STRING_KEYS;
		container.type |= SEXP_CONTAINER_NUMBER_KEYS;
	}
}

void CEditContainerDlg::OnTypeList() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	if (!container.is_list()) {
		container.set_to_list();
		update_type_controls();
	}
}

void CEditContainerDlg::OnTypeMap() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	if (!container.is_map()) {
		container.set_to_map();
		container.type |= SEXP_CONTAINER_STRING_KEYS;

		update_type_controls();
		set_key_type();
	}
}

void CEditContainerDlg::set_container_type()
{
	CButton *button_list = (CButton *) GetDlgItem(IDC_CONTAINER_LIST);
	CButton *button_map = (CButton *) GetDlgItem(IDC_CONTAINER_MAP);

	const auto &container = get_current_container();
	const bool list_selected = container.is_list();
	const bool map_selected = container.is_map();
	Assert((int)list_selected ^ (int)map_selected);

	button_list->SetCheck(list_selected);
	button_map->SetCheck(map_selected);
}

void CEditContainerDlg::OnTypeNumber() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	container.type &= ~SEXP_CONTAINER_STRING_DATA;
	container.type |= SEXP_CONTAINER_NUMBER_DATA;
	// TODO: update container.opf_type?
}

void CEditContainerDlg::OnTypeString() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	container.type &= ~SEXP_CONTAINER_NUMBER_DATA;
	container.type |= SEXP_CONTAINER_STRING_DATA;
}

void CEditContainerDlg::set_data_type()
{
	CButton *button_string = (CButton *) GetDlgItem(IDC_CONTAINER_STRING);
	CButton *button_number = (CButton *) GetDlgItem(IDC_CONTAINER_NUMBER);

	const auto &container = get_current_container();
	const bool number_selected = container.type & SEXP_CONTAINER_NUMBER_DATA;
	const bool string_selected = container.type & SEXP_CONTAINER_STRING_DATA;

	button_number->SetCheck(number_selected);
	button_string->SetCheck(string_selected);
}

void CEditContainerDlg::set_key_type()
{
	CButton *button_string = (CButton *) GetDlgItem(IDC_STRING_KEYS);
	CButton *button_number = (CButton *) GetDlgItem(IDC_NUMBER_KEYS);

	const auto &container = get_current_container();
	Assert(container.is_map());
	const bool number_keys_selected = container.type & SEXP_CONTAINER_NUMBER_KEYS;
 	const bool string_keys_selected = container.type & SEXP_CONTAINER_STRING_KEYS;
	
	button_number->SetCheck(number_keys_selected);
	button_string->SetCheck(string_keys_selected);
}

void CEditContainerDlg::OnSelchangeContainerName()
{
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);

	const int selected_container = cbox->GetCurSel();
	if (selected_container != m_current_container) {
		m_current_container = selected_container;

		set_selected_container();
	}
}

void CEditContainerDlg::OnEditchangeContainerName()
{
	CString new_name;

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	// TODO: how could this value be different from m_current_container?
	const int selected_container = cbox->GetCurSel();
	cbox->GetWindowText(new_name);

	if (is_container_name_valid(new_name)) {
		edit_sexp_containers[selected_container].container_name = new_name; 
	}
}

bool CEditContainerDlg::is_container_name_in_use(const char *text) const
{
	for (const auto &container : edit_sexp_containers) {
		if (!stricmp(container.container_name.c_str(), text)) {
			return true;
		}
	}

	return false;
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
			new_name.Replace((TCHAR) '&', (TCHAR) '-');
		}

		//not already in list and length > 0
		if (!new_name.IsEmpty() && !is_container_name_in_use(LPCTSTR(new_name))) {
			name_validated = true;
		} else {
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
	sprintf(buf, "%d", temp_num); // TODO: use snprintf

	if ( stricmp(buf, test_string.c_str()) ) {
		return false;
	} else {
		return true; 
	}
}


//BOOL CEditContainerDlg::is_data_valid()
//{
//	int i; 
//
//	// type check data
//	// ToDo - type check strings if using strict type checking
//	int YouHaveMoreToDo; 
//
//	// if not strict typing all strings are fine regardless of whether we're dealing with maps or lists.
//	if (!m_type_number && (m_type_list || (m_type_map && !m_key_type_number))) {
//		return true; 
//	}
//
//	else if (m_type_map ) {
//		// start at the first data entry
//		for (i = 1; i < (int)raw_data.size(); i = i+2) {
//			if (!is_valid_number(raw_data[i])) {
//				return false; 
//			}
//		}
//	}
//	else if (m_type_list) {
//		for (i = 0; i < (int)raw_data.size(); i++) {
//			if (!is_valid_number(raw_data[i])) {
//				return false; 
//			}
//		}
//	}
//	// unknown container type
//	else {
//		return false; 
//	}
//
//	// if we made it this far, the data should be fine
//	return true;
//}

void CEditContainerDlg::OnListerSelectionChange()
{
	const int index = m_container_data_lister.GetCurSel();

	if (index == LB_ERR) {
		// TODO: clear key and data text  edit boxes?
		return;
	}

	Assert(m_container_data_lister.GetCount() > 0);
	Assert(index >= 0 && index < get_current_container().size());

	const auto& container = get_current_container();
	Assert(!container.empty());

	if (container.is_list()) {
		update_text_edit_boxes("", m_lister_keys[index]);
	} else if (container.is_map()) {
		const SCP_string &key = m_lister_keys[index];
		update_text_edit_boxes(key, container.map_data.at(key));
	} else {
		Assert(false);
	}
}

void CEditContainerDlg::OnContainerAdd()
{
	Assert(!edit_sexp_containers.empty());
	Assert(m_current_container >= 0 && m_current_container < (int)edit_sexp_containers.size());

	if (!edit_boxes_have_valid_data()) {
		return;
	}

	CEdit *data_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);

	auto &container = get_current_container();
	if (container.is_list()) {
		container.list_data.emplace_back(data_str);
	} else if (container.is_map()) {
		CEdit *key_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_KEY);
		CString key_str;
		key_edit->GetWindowText(key_str);
		container.map_data.emplace(key_str, data_str);
	}

	if (container.size() == 1) {
		update_data_entry_controls();
	}

	update_data_lister();
	update_text_edit_boxes("", "");
}

void CEditContainerDlg::OnContainerInsert()
{
	// TODO: use add_container_entry() funtion below instead of this
	// pass m_lister_index, once added

	if (!edit_boxes_have_valid_data()) {
		return;
	}
	
	CString temp_string; 
	SCP_string temp_scp_string; 

	SCP_vector<SCP_string>::iterator iter ;
	
	//// for both types we need to get the data
	//CEdit *edit2 = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	//edit2->GetWindowText(temp_string);
	//ListerSelectionGetIter(iter);
	//raw_data.insert(iter, temp_scp_string.assign(temp_string));

	//// insert the key before the newly added data entry
	//if (m_type_map) {
	//	CEdit *edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
	//	edit->GetWindowText(temp_string);
	//	ListerSelectionGetIter(iter);
	//	raw_data.insert(iter, temp_scp_string.assign(temp_string));
	//}

	update_data_lister();
}

void CEditContainerDlg::add_container_entry(const int insert_index)
{
	// TODO move "container add" stuff here
}

void CEditContainerDlg::OnContainerRemove()
{
	Assert(!edit_sexp_containers.empty());
	Assert(m_current_container >= 0 && m_current_container < (int)edit_sexp_containers.size());
	Assert(!get_current_container().empty());

	auto &container = get_current_container();
	const int index = m_container_data_lister.GetCurSel(); 
	// TODO: check for LB_ERR, see OnContainerUpdate()
	Assert(index >= 0 && index < container.size());

	if (container.is_list()) {
		container.list_data.erase(std::next(container.list_data.begin(), index));
	} else if (container.is_map()) {
		const auto& key = m_lister_keys[index];
		container.map_data.erase(key);
	}

	if (container.empty()) {
		update_data_entry_controls();
	}

	update_data_lister();
}

void CEditContainerDlg::OnContainerUpdate()
{
	Assert(!edit_sexp_containers.empty());
	Assert(m_current_container >= 0 && m_current_container < (int)edit_sexp_containers.size());
	Assert(!get_current_container().empty());

	if (!data_edit_box_has_valid_data()) {
		return;
	}

	const int index = m_container_data_lister.GetCurSel(); 

	if (index == LB_ERR) {
		MessageBox("Nothing selected! Use Add to enter new data");
		return; 
	}
	
	auto &container = get_current_container();
	
	CEdit *data_edit = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);
	if (container.is_list()) {
		*std::next(container.list_data.begin(), index) = SCP_string(data_str);
	} else if (container.is_map()) {
		CEdit *key_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_KEY);
		CString key_str;
		key_edit->GetWindowText(key_str);
		container.map_data[SCP_string(key_str)] = data_str;
	} else {
		Assert(false);
	}

	update_data_lister();
}

BOOL CEditContainerDlg::edit_boxes_have_valid_data()
{
	// if we are in list mode, check that the container data box has data in it. 
	if (data_edit_box_has_valid_data()) {
		// if we are in map mode we must check that key box has data and that the key doesn't match an existing key.
		if (get_current_container().is_map()) {
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
	CEdit *key_edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);

	const auto &container = get_current_container();

	if (!key_str.IsEmpty()) {
		const auto &map_data = container.map_data;
		const auto count = std::count_if(map_data.cbegin(),
			map_data.cend(),
			[key_str](const std::pair<SCP_string, SCP_string>& map_entry) -> bool {
				return !stricmp(map_entry.first.c_str(), key_str);
			});
		if (count > 0) {
			MessageBox("This key already exists. You may not reuse keys in a SEXP map!");
			return false;
		}

		if ((container.type & SEXP_CONTAINER_NUMBER_KEYS) && !is_valid_number(SCP_string(key_str))) {
			Assert(container.is_map());
			MessageBox("This key is not a valid number and this map container uses numeric keys!");
			return false;
		}

		return true;
	} else {
		MessageBox("You have not entered a key!");
	}

	return false; 
}

BOOL CEditContainerDlg::data_edit_box_has_valid_data()
{		
	CEdit *data_edit = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);
	if (!data_str.IsEmpty()) {
		if ((get_current_container().type & SEXP_CONTAINER_NUMBER_DATA) && !is_valid_number(SCP_string(data_str))) {
			MessageBox("This data is not a valid number and this container uses numeric data!");
			return false;
		}

		return true;
	} else {
		MessageBox("You have not entered any data!");
		return false;
	}
}

void CEditContainerDlg::update_type_controls()
{
	const auto& container = get_current_container();
	const bool container_empty = container.empty();
	const bool map_selected = container.is_map();

	// container type
	GetDlgItem(IDC_CONTAINER_LIST)->EnableWindow(container_empty);
	GetDlgItem(IDC_CONTAINER_MAP)->EnableWindow(container_empty);

	// data type
	GetDlgItem(IDC_CONTAINER_STRING)->EnableWindow(container_empty);
	GetDlgItem(IDC_CONTAINER_NUMBER)->EnableWindow(container_empty);

	// key type
	GetDlgItem(IDC_STRING_KEYS)->EnableWindow(container_empty && map_selected);
	GetDlgItem(IDC_NUMBER_KEYS)->EnableWindow(container_empty && map_selected);
}

void CEditContainerDlg::update_data_entry_controls()
{
	const bool has_container = !edit_sexp_containers.empty();
	const auto &container = get_current_container();
	const bool container_empty = container.empty();
	const bool map_selected = container.is_map();

	// data manipulation buttons
	GetDlgItem(IDC_CONTAINER_ADD)->EnableWindow(has_container);
	// TODO: also check that m_lister_index is valid
	GetDlgItem(IDC_CONTAINER_INSERT)->EnableWindow(has_container && !map_selected);
	GetDlgItem(IDC_CONTAINER_REMOVE)->EnableWindow(has_container && !container_empty);
	// TODO: also check that m_lister_index is valid
	GetDlgItem(IDC_CONTAINER_UPDATE)->EnableWindow(has_container && !container_empty);

	// text edit boxes
	GetDlgItem(IDC_CONTAINER_KEY)->EnableWindow(has_container && map_selected);
	GetDlgItem(IDC_CONTAINER_DATA)->EnableWindow(has_container);
}

void CEditContainerDlg::update_data_lister()
{
	m_container_data_lister.ResetContent(); 
	m_lister_keys.clear();

	// Data is displayed in one of two ways depending on container type
	Assert(m_current_container >= 0 && m_current_container < (int)edit_sexp_containers.size());

	const auto &container = get_current_container();

	if (container.is_list()) {
		for (const auto &list_entry : container.list_data) {
			m_lister_keys.emplace_back(list_entry);
			m_container_data_lister.AddString(list_entry.c_str());
		}
	} else if (container.is_map()) {
		for (const auto& map_entry : container.map_data) {
			// TODO: type check key, maybe also value
			m_lister_keys.emplace_back(map_entry.first);
		}

		if (container.type & SEXP_CONTAINER_STRING_KEYS) {
			std::sort(m_lister_keys.begin(), m_lister_keys.end());
		} else if (container.type & SEXP_CONTAINER_STRING_KEYS) {
			std::sort(m_lister_keys.begin(),
				m_lister_keys.end(),
				[](const SCP_string& str1, const SCP_string& str2) -> bool {
					return std::atoi(str1.c_str()) < std::atoi(str2.c_str());
				});
		} else {
			Assert(false);
		}

		for (const auto &key : m_lister_keys) {
			const SCP_string  lister_entry = key + " - " + container.map_data.at(key);
			m_container_data_lister.AddString(lister_entry.c_str());
		}
	} else {
		Assert(false);
	}

	m_container_data_lister.EnableWindow(m_container_data_lister.GetCount() > 0);
}

void CEditContainerDlg::OnBnClickedAddNewContainer()
{
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);

	//// try to save the old container first
	////if (m_data_changed) {
	//// FIXME TEMP: ignore m_data_changed
	//if (m_current_container >= 0) {
	//	if (!save_current_container()) {
	//		// we couldn't save the old container, don't do anything with the new one yet. 
	//		return; 
	//	}
	//}
	
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

	edit_sexp_containers.emplace_back(); 
	auto &new_container = edit_sexp_containers.back();
	
	const auto &container = get_current_container();
	new_container.type = container.type;
	new_container.opf_type = container.opf_type;

	// add the new container to the list of containers combo box and make it the selected option
	cbox->AddString(temp_name); 
	m_current_container = (int)edit_sexp_containers.size() - 1;
	cbox->SetCurSel(m_current_container);

	update_data_lister();  

	// in case this is the first container
	if (edit_sexp_containers.size() == 1) {
		cbox->EnableWindow(true);
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(true);	
	}
}

//BOOL CEditContainerDlg::save_current_container()
//{
//	if (!is_data_valid()) {
//		MessageBox("Since data is not valid, it can not be saved. Please correct the data before continuing.");
//		return false;
//	}
//
//	Assert(m_current_container >= 0);
//
//	edit_sexp_containers[m_current_container].type = 0;
//
//	if (m_type_map) {
//		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_MAP;
//
//		if (m_key_type_number) {
//			edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_NUMBER_KEYS;
//		} else {
//			edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_STRING_KEYS;
//		}
//
//		// write the raw data into edit_sexp_containers
//		Assertion(raw_data.size() % 2 == 0, "Invalid size for map type");
//		edit_sexp_containers[m_current_container].map_data.clear();
//
//		for (int i = 0; i < (int)raw_data.size(); i = i + 2) {
//			edit_sexp_containers[m_current_container].map_data.insert(
//				std::pair<SCP_string, SCP_string>(raw_data[i], raw_data[i + 1]));
//		}
//	} else {
//		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_LIST;
//		edit_sexp_containers[m_current_container].list_data.clear();
//
//		// write the raw data into edit_sexp_containers
//		for (int i = 0; i < (int)raw_data.size(); i++) {
//			edit_sexp_containers[m_current_container].list_data.push_back(raw_data[i]);
//		}
//	}
//
//	if (m_type_number) {
//		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_NUMBER_DATA;
//	} else {
//		edit_sexp_containers[m_current_container].type |= SEXP_CONTAINER_STRING_DATA;
//	}
//
//	return true;
//}
void CEditContainerDlg::OnBnClickedDeleteContainer()
{
	Assert(!edit_sexp_containers.empty()); 
	Assert(m_current_container >= 0 && m_current_container < (int)edit_sexp_containers.size());

	char buffer[256]; 

	const int times_used = m_p_sexp_tree->get_container_count(edit_sexp_containers[m_current_container].container_name.c_str()); 
	if (times_used > 0) {
		sprintf(buffer, "Container %s is used in %d events. Please edit those uses out first, then delete the container.", edit_sexp_containers[m_current_container].container_name.c_str(), times_used); 
		MessageBox(buffer);
		return;
	}

	sprintf(buffer, "This will delete the %s container. Are you sure?", edit_sexp_containers[m_current_container].container_name.c_str()); 

	const int ret = MessageBox(buffer, "Delete?", MB_OKCANCEL | MB_ICONEXCLAMATION);

	if (ret == IDCANCEL) {
		return;
	}

	// prevent sudden change in type selection buttons
	if (edit_sexp_containers.size() == 1) {
		Assert(m_current_container == 0);
		const auto &last_container = edit_sexp_containers.front();
		m_dummy_container.type = last_container.type;
		m_dummy_container.opf_type = last_container.opf_type;
	}

	edit_sexp_containers.erase(edit_sexp_containers.begin() + m_current_container);

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	cbox->DeleteString((int)cbox->GetCurSel()); 

	//maybe de-activate the delete container button
	if (edit_sexp_containers.empty()) {
		m_current_container = -1; 
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(false);	
	} else {
		m_current_container = 0; 
		cbox->SetCurSel(m_current_container);
	}
	set_selected_container();
}

void CEditContainerDlg::update_text_edit_boxes(const SCP_string &key, const SCP_string &data)
{
	CEdit *key_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_KEY);
	key_edit->SetWindowText(key.c_str());

	CEdit *data_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_DATA);
	data_edit->SetWindowText(data.c_str());
}

sexp_container &CEditContainerDlg::get_current_container()
{
	Assert(m_current_container >= -1 && m_current_container < (int)edit_sexp_containers.size());

	return m_current_container < 0 ? m_dummy_container : edit_sexp_containers[m_current_container];
}

