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


CEditContainerDlg::CEditContainerDlg(sexp_tree *p_sexp_tree, CWnd *pParent /*=nullptr*/)
	: CDialog(CEditContainerDlg::IDD, pParent), m_p_sexp_tree(p_sexp_tree)
{
}

void CEditContainerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONTAINER_DATA_LISTER, m_container_data_lister);
}

BEGIN_MESSAGE_MAP(CEditContainerDlg, CDialog)
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
	m_containers = Sexp_containers;

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	cbox->ResetContent();
	cbox->LimitText(sexp_container::NAME_MAX_LENGTH);

	CEdit *key_edit = (CEdit *)GetDlgItem(IDC_CONTAINER_KEY);
	key_edit->SetLimitText(sexp_container::VALUE_MAX_LENGTH);
	CEdit *data_edit = (CEdit *)GetDlgItem(IDC_CONTAINER_DATA);
	data_edit->SetLimitText(sexp_container::VALUE_MAX_LENGTH);

	// do we already have any containers defined? 
	if (has_containers()) {
		cbox->EnableWindow(true); 
		for (const auto &container : m_containers) {
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
	if (m_current_container != -1) {
		CComboBox* cbox = (CComboBox*)GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
		Assert(has_containers());
		Assert(cbox->GetCount() == num_containers());
		Assert(m_current_container < cbox->GetCount());
		cbox->SetCurSel(m_current_container);
	}

	update_controls();
	set_container_type(); 
	set_data_type();
	if (get_current_container().is_map()) {
		set_key_type();
	}
	update_data_lister();
}

void CEditContainerDlg::OnOK() 
{
	update_sexp_containers(m_containers);
	// TODO: reset dialog's vars? Is there a chance the dialog will get re-used?

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
		container.type &= ~SEXP_CONTAINER_MAP;
		container.type |= SEXP_CONTAINER_LIST;
		update_controls();
	}
}

void CEditContainerDlg::OnTypeMap() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	if (!container.is_map()) {
		container.type &= ~SEXP_CONTAINER_LIST;
		container.type |= SEXP_CONTAINER_MAP;
		if (!(container.type & SEXP_CONTAINER_NUMBER_KEYS)) {
			container.type |= SEXP_CONTAINER_STRING_KEYS;
		}

		update_controls();
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
	container.opf_type = OPF_NUMBER;
}

void CEditContainerDlg::OnTypeString() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	container.type &= ~SEXP_CONTAINER_NUMBER_DATA;
	container.type |= SEXP_CONTAINER_STRING_DATA;
	container.opf_type = OPF_ANYTHING;
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
	Assert((int)number_keys_selected ^ (int)string_keys_selected);
	
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
	Assert(cbox->IsWindowEnabled());
	Assert(cbox->GetCount() > 0);
	Assert(has_containers());
	Assert(m_current_container >= 0);
	cbox->GetWindowText(new_name);

	auto &container = get_current_container();
	if (is_container_name_valid(new_name, true)) {
		container.container_name = new_name;
	} else {
		// restore unmodified name
		cbox->SetWindowText(container.container_name.c_str());
	}
}

bool CEditContainerDlg::is_container_name_in_use(const char *text, bool ignore_current) const
{
	for (int i = 0; i < num_containers(); ++i) {
		if (ignore_current && (m_current_container == i)) {
			continue;
		} else if (!stricmp(m_containers[i].container_name.c_str(), text)) {
			return true;
		}
	}

	return false;
}

bool CEditContainerDlg::is_container_name_valid(CString &new_name, bool is_rename)
{
	if (new_name.IsEmpty()) {
		MessageBox("Container name can't be empty.");
		return false;
	}

	if (!stricmp(new_name, "<New Container Name>")) {
		MessageBox("Invalid container name.");
		return false;
	}

	if (new_name[0] == ' ' || new_name[new_name.GetLength()-1] == ' ') {
		MessageBox("Container names cannot begin or end with spaces.");
		return false;
	}

	// handle & chars
	if (strchr(new_name, '&') != nullptr) {
		if (is_rename) {
			MessageBox("Container names cannot contain &.");
			return false;
		} else {
			MessageBox("Container names cannot contain &. Replacing with hyphens.");
			new_name.Replace((TCHAR)'&', (TCHAR)'-');
		}
	}

	if (is_container_name_in_use(LPCTSTR(new_name), is_rename)) {
		MessageBox("Conflicting container name. A container with this name already exists");
		return false;
	}

	return true;
}

bool CEditContainerDlg::is_valid_number(const char *test_str) const
{
	const int temp_num = atoi(test_str);
	char buf[TOKEN_LENGTH];
	const int ret = snprintf(buf, sizeof(buf), "%d", temp_num);
	if (ret <= 0) {
		return false;
	}
	buf[ret] = '\0';
	return !strcmp(buf, test_str);
}

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
	add_container_entry((int)get_current_container().size());
}

void CEditContainerDlg::OnContainerInsert()
{
	Assert(get_current_container().is_list());

	// TODO: create/use m_lister_index instead
	const int index = m_container_data_lister.GetCurSel();

	// TODO: enable Insert button only when a lister entry is selected
	if (index == LB_ERR) {
		MessageBox("Nothing selected! Use Add to append new data");
		return;
	}

	add_container_entry(index);
}

void CEditContainerDlg::add_container_entry(const int insert_index)
{
	Assert(has_containers());
	Assert(m_current_container >= 0 && m_current_container < num_containers());

	if (!edit_boxes_have_valid_data(false)) {
		return;
	}

	CEdit *data_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);

	auto &container = get_current_container();
	if (container.is_list()) {
		Assert(insert_index >= 0 && insert_index <= (int)container.size());
		if (insert_index == (int)container.size()) {
			container.list_data.emplace_back(data_str);
		} else {
			auto insert_it = std::next(container.list_data.begin(), insert_index);
			container.list_data.emplace(insert_it, data_str);
		}
	} else if (container.is_map()) {
		CEdit *key_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_KEY);
		CString key_str;
		key_edit->GetWindowText(key_str);
		container.map_data.emplace(key_str, data_str);
	}

	if (container.size() == 1) {
		update_controls();
	}

	update_data_lister();
	update_text_edit_boxes("", "");
}

void CEditContainerDlg::OnContainerRemove()
{
	Assert(has_containers());
	Assert(m_current_container >= 0 && m_current_container < num_containers());
	Assert(!get_current_container().empty());

	auto &container = get_current_container();

	const int index = m_container_data_lister.GetCurSel(); 
	if (index == LB_ERR) {
		MessageBox("Select an item from the list before pressing Remove.");
		return;
	}
	Assert(index >= 0 && index < container.size());

	if (container.is_list()) {
		container.list_data.erase(std::next(container.list_data.begin(), index));
	} else if (container.is_map()) {
		const auto& key = m_lister_keys[index];
		container.map_data.erase(key);
	}

	if (container.empty()) {
		update_controls();
	}

	update_data_lister();
}

// TODO: if the current container is a map, enable the Update button only when the value in the key edit box is one of the map keys

void CEditContainerDlg::OnContainerUpdate()
{
	Assert(has_containers());
	Assert(m_current_container >= 0 && m_current_container < num_containers());
	Assert(!get_current_container().empty());

	if (!edit_boxes_have_valid_data(true)) {
		return;
	}

	// TODO: for lists, enable Update button only when an item in the list is selected
	// TODO: for maps, enable Update (and disable Add) when the value in the key edit box is already a map key.
	// Or maybe all this is more work than it's worth

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

bool CEditContainerDlg::edit_boxes_have_valid_data(bool dup_key_ok)
{
	if (!data_edit_box_has_valid_data()) {
		return false;
	}
	if (get_current_container().is_map() && !key_edit_box_has_valid_data(dup_key_ok)) {
		return false;
	}

	return true; 
}

bool CEditContainerDlg::key_edit_box_has_valid_data(bool dup_ok)
{
	Assert(has_containers());
	Assert(m_current_container >= 0 && m_current_container < num_containers());
	const auto& container = get_current_container();
	Assert(container.is_map());

	CEdit *key_edit = (CEdit *) GetDlgItem(IDC_CONTAINER_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);

	if (key_str.IsEmpty()) {
		MessageBox("You have not entered a key!");
		return false;
	}

	if (!dup_ok) {
		const auto &map_data = container.map_data;
		const auto count = std::count_if(map_data.cbegin(),
			map_data.cend(),
			[key_str](const std::pair<SCP_string, SCP_string>& map_entry) -> bool {
				return !stricmp(map_entry.first.c_str(), key_str);
			});

		if (count > 0) {
			MessageBox("This key already exists! Use Update to change an existing key's data.");
			return false;
		}
	}

	if ((container.type & SEXP_CONTAINER_NUMBER_KEYS) && !is_valid_number(key_str)) {
		Assert(container.is_map());
		MessageBox("This key is not a valid number and this map container uses numeric keys!");
		return false;
	}

	// TODO: validate strictly typed keys

	return true; 
}

bool CEditContainerDlg::data_edit_box_has_valid_data()
{
	CEdit *data_edit = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);

	if (data_str.IsEmpty()) {
		MessageBox("You have not entered any data!");
		return false;
	}

	if ((get_current_container().type & SEXP_CONTAINER_NUMBER_DATA) && !is_valid_number(data_str)) {
		MessageBox("This data is not a valid number and this container uses numeric data!");
		return false;
	}

	// TODO: validate strictly typed data

	return true;
}

void CEditContainerDlg::update_controls()
{
	update_type_controls();
	update_data_entry_controls();
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
	const bool has_container = has_containers();
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
	const auto &container = get_current_container();

	if (container.is_list()) {
		for (const auto &list_entry : container.list_data) {
			m_lister_keys.emplace_back(list_entry);
			m_container_data_lister.AddString(list_entry.c_str());
		}
	} else if (container.is_map()) {
		for (const auto& map_entry : container.map_data) {
			m_lister_keys.emplace_back(map_entry.first);
		}

		if (container.type & SEXP_CONTAINER_STRING_KEYS) {
			std::sort(m_lister_keys.begin(), m_lister_keys.end());
		} else if (container.type & SEXP_CONTAINER_NUMBER_KEYS) {
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

	CEditContainerAddNewDlg dlg; 
	dlg.DoModal(); 

	// if we didn't enter a name
	if (dlg.cancelled) {
		return;
	}

	CString temp_name = dlg.m_new_container_name; 
	if (!is_container_name_valid(temp_name, false)) {
		return; 
	}

	m_containers.emplace_back(); 
	auto &new_container = m_containers.back();
	new_container.container_name = temp_name;

	const auto &container = get_current_container();
	new_container.type = container.type;
	new_container.opf_type = container.opf_type;

	// add the new container to the list of containers combo box and make it the selected option
	cbox->AddString(temp_name); 
	m_current_container = num_containers() - 1;

	// in case this is the first container
	if (num_containers() == 1) {
		cbox->EnableWindow(true);
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(true);	
	}

	set_selected_container();
}

void CEditContainerDlg::OnBnClickedDeleteContainer()
{
	Assert(has_containers()); 
	Assert(m_current_container >= 0 && m_current_container < num_containers());

	char buffer[256]; 

	const int times_used = m_p_sexp_tree->get_container_count(m_containers[m_current_container].container_name.c_str()); 
	if (times_used > 0) {
		sprintf(buffer, "Container %s is used in %d events. Please edit those uses out first, then delete the container.", m_containers[m_current_container].container_name.c_str(), times_used); 
		MessageBox(buffer);
		return;
	}

	sprintf(buffer, "This will delete the %s container. Are you sure?", m_containers[m_current_container].container_name.c_str()); 

	const int ret = MessageBox(buffer, "Delete?", MB_OKCANCEL | MB_ICONEXCLAMATION);

	if (ret == IDCANCEL) {
		return;
	}

	// prevent sudden change in type selection buttons
	if (num_containers() == 1) {
		Assert(m_current_container == 0);
		const auto &last_container = m_containers.front();
		m_dummy_container.type = last_container.type;
		m_dummy_container.opf_type = last_container.opf_type;
	}

	m_containers.erase(m_containers.begin() + m_current_container);

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	cbox->DeleteString((int)cbox->GetCurSel()); 

	//maybe de-activate the delete container button
	if (m_containers.empty()) {
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
	Assert(m_current_container >= -1 && m_current_container < num_containers());
	return m_current_container < 0 ? m_dummy_container : m_containers[m_current_container];
}
