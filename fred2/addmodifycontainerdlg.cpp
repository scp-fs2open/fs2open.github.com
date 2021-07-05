/*
 * Created by Hassan "Karajorma" Kazmi and Josh "jg18" Glatt for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

// AddModifyContainerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "AddModifyContainerDlg.h"
#include "parse/sexp.h"
#include "EditContainerNameDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_RESET_FOCUS	0
#define RESET_FOCUS		1

/////////////////////////////////////////////////////////////////////////////
// AddModifyContainerDlg dialog

namespace {
const SCP_string NEW_CONTAINER_NAME = "<New Container Name>";
} // namespace

CAddModifyContainerDlg::CAddModifyContainerDlg(sexp_tree *p_sexp_tree, CWnd *pParent /*=nullptr*/)
	: CDialog(CAddModifyContainerDlg::IDD, pParent), m_p_sexp_tree(p_sexp_tree)
{
}

void CAddModifyContainerDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONTAINER_DATA_LISTER, m_container_data_lister);
}

BEGIN_MESSAGE_MAP(CAddModifyContainerDlg, CDialog)
	ON_BN_CLICKED(IDC_CONTAINER_ADD, OnContainerAdd)
	ON_BN_CLICKED(IDC_CONTAINER_INSERT, OnContainerInsert)
	ON_BN_CLICKED(IDC_CONTAINER_REMOVE, OnContainerRemove)
	ON_BN_CLICKED(IDC_CONTAINER_UPDATE, OnContainerUpdate)
	
	ON_BN_CLICKED(IDC_CONTAINER_LIST, OnTypeList)
	ON_BN_CLICKED(IDC_CONTAINER_MAP, OnTypeMap)
	ON_BN_CLICKED(IDC_CONTAINER_NUMBER_DATA, OnTypeNumber)
	ON_BN_CLICKED(IDC_CONTAINER_STRING_DATA, OnTypeString)
	
	ON_LBN_SELCHANGE(IDC_CONTAINER_DATA_LISTER, OnListerSelectionChange)

	ON_CBN_SELCHANGE(IDC_CURRENT_CONTAINER_NAME, OnSelchangeContainerName)

	ON_BN_CLICKED(IDC_CONTAINER_NO_PERSIST, OnPersistNone)
	ON_BN_CLICKED(IDC_CONTAINER_CAMPAIGN_PERSIST, OnSaveOnMissionComplete)
	ON_BN_CLICKED(IDC_CONTAINER_PLAYER_PERSIST, OnSaveOnMissionClose)
	ON_BN_CLICKED(IDC_CONTAINER_ETERNAL_PERSIST, OnPersistEternal)

	ON_BN_CLICKED(IDC_ADD_NEW_CONTAINER, &CAddModifyContainerDlg::OnBnClickedAddNewContainer)
	ON_BN_CLICKED(IDC_RENAME_CONTAINER, &CAddModifyContainerDlg::OnBnClickedRenameContainer)

	ON_BN_CLICKED(IDC_CONTAINER_STRING_KEYS, &CAddModifyContainerDlg::OnBnClickedStringKeys)
	ON_BN_CLICKED(IDC_CONTAINER_NUMBER_KEYS, &CAddModifyContainerDlg::OnBnClickedNumberKeys)
	ON_BN_CLICKED(IDC_DELETE_CONTAINER, &CAddModifyContainerDlg::OnBnClickedDeleteContainer)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddModifyContainerDlg message handlers

BOOL CAddModifyContainerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//grab the existing list of containers and duplicate it. We only update it if the user clicks OK. 
	m_containers = get_all_sexp_containers();

	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	cbox->ResetContent();

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
		GetDlgItem(IDC_RENAME_CONTAINER)->EnableWindow(true);
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

void CAddModifyContainerDlg::set_selected_container()
{
	if (m_current_container != -1) {
		CComboBox *cbox = (CComboBox*)GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
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
	set_persistence_options();
	update_data_lister();
}

void CAddModifyContainerDlg::OnOK() 
{
	update_sexp_containers(m_containers);

	if (!m_old_to_new_names.empty()) {
		// TODO: rename nodes in Sexp_nodes
		// TODO: rename nodes in sexp_tree::tree_nodes
	}

	CDialog::OnOK();
}

void CAddModifyContainerDlg::OnBnClickedStringKeys()
{
	auto &container = get_current_container();
	Assert(container.is_map());
	Assert(container.empty());
	if (none(container.type & ContainerType::STRING_KEYS)) {
		container.type &= ~ContainerType::NUMBER_KEYS;
		container.type |= ContainerType::STRING_KEYS;
	}
}

void CAddModifyContainerDlg::OnBnClickedNumberKeys()
{
	auto &container = get_current_container();
	Assert(container.is_map());
	Assert(container.empty());
	if (none(container.type & ContainerType::NUMBER_KEYS)) {
		container.type &= ~ContainerType::STRING_KEYS;
		container.type |= ContainerType::NUMBER_KEYS;
	}
}

void CAddModifyContainerDlg::OnTypeList() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	if (!container.is_list()) {
		container.type &= ~ContainerType::MAP;
		// removing the "*_KEYS" type flag is unnecessary, because save_containers() won't save it
		// and keeping it improves UX if the user switches back to map later
		container.type |= ContainerType::LIST;
		update_controls();
	}
}

void CAddModifyContainerDlg::OnTypeMap() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	if (!container.is_map()) {
		container.type &= ~ContainerType::LIST;
		container.type |= ContainerType::MAP;
		if (none(container.type & ContainerType::NUMBER_KEYS)) {
			container.type |= ContainerType::STRING_KEYS;
		}

		update_controls();
		set_key_type();
	}
}

void CAddModifyContainerDlg::set_container_type()
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

void CAddModifyContainerDlg::OnTypeNumber() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	container.type &= ~ContainerType::STRING_DATA;
	container.type |= ContainerType::NUMBER_DATA;
	container.opf_type = OPF_NUMBER;
}

void CAddModifyContainerDlg::OnTypeString() 
{
	auto &container = get_current_container();
	Assert(container.empty());
	container.type &= ~ContainerType::NUMBER_DATA;
	container.type |= ContainerType::STRING_DATA;
	container.opf_type = OPF_ANYTHING;
}

void CAddModifyContainerDlg::set_data_type()
{
	CButton *button_string = (CButton *) GetDlgItem(IDC_CONTAINER_STRING_DATA);
	CButton *button_number = (CButton *) GetDlgItem(IDC_CONTAINER_NUMBER_DATA);

	const auto &container = get_current_container();
	const bool number_selected = any(container.type & ContainerType::NUMBER_DATA);
	const bool string_selected = any(container.type & ContainerType::STRING_DATA);

	button_number->SetCheck(number_selected);
	button_string->SetCheck(string_selected);
}

void CAddModifyContainerDlg::set_key_type()
{
	CButton *button_string = (CButton *) GetDlgItem(IDC_CONTAINER_STRING_KEYS);
	CButton *button_number = (CButton *) GetDlgItem(IDC_CONTAINER_NUMBER_KEYS);

	const auto &container = get_current_container();
	Assert(container.is_map());
	const bool number_keys_selected = any(container.type & ContainerType::NUMBER_KEYS);
 	const bool string_keys_selected = any(container.type & ContainerType::STRING_KEYS);
	Assert((int)number_keys_selected ^ (int)string_keys_selected);
	
	button_number->SetCheck(number_keys_selected);
	button_string->SetCheck(string_keys_selected);
}

void CAddModifyContainerDlg::OnSelchangeContainerName()
{
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);

	const int selected_container = cbox->GetCurSel();
	if (selected_container != m_current_container) {
		m_current_container = selected_container;

		set_selected_container();
	}
}

bool CAddModifyContainerDlg::is_container_name_in_use(const char *text, bool ignore_current) const
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

bool CAddModifyContainerDlg::is_container_name_valid(CString &new_name, bool is_rename)
{
	if (new_name.IsEmpty()) {
		MessageBox("Container name can't be empty.");
		return false;
	}

	if (!stricmp(new_name, NEW_CONTAINER_NAME.c_str())) {
		MessageBox("Invalid container name.");
		return false;
	}

	if (new_name[0] == ' ' || new_name[new_name.GetLength()-1] == ' ') {
		MessageBox("Container names cannot begin or end with spaces.");
		return false;
	}

	// handle & chars
	if (strchr(new_name, sexp_container::DELIM) != nullptr) {
		if (is_rename) {
			MessageBox("Container names cannot contain &.");
			return false;
		} else {
			MessageBox("Container names cannot contain &. Replacing with hyphens.");
			new_name.Replace((TCHAR)sexp_container::DELIM, (TCHAR)'-');
		}
	}

	if (is_container_name_in_use(LPCTSTR(new_name), is_rename)) {
		MessageBox("Conflicting container name. A container with this name already exists");
		return false;
	}

	return true;
}

bool CAddModifyContainerDlg::is_valid_number(const char *test_str) const
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

void CAddModifyContainerDlg::OnListerSelectionChange()
{
	const int index = m_container_data_lister.GetCurSel();

	if (index == LB_ERR) {
		// TODO: clear key and data text  edit boxes?
		return;
	}

	Assert(m_container_data_lister.GetCount() > 0);
	Assert(index >= 0 && index < get_current_container().size());

	const auto &container = get_current_container();
	Assert(!container.empty());

	if (container.is_list()) {
		update_text_edit_boxes("", m_lister_keys[index]);
	} else if (container.is_map()) {
		const SCP_string &key = m_lister_keys[index];
		update_text_edit_boxes(key, container.map_data.at(key));
	} else {
		UNREACHABLE("Unknown container type %d", (int)container.type);
	}
}

void CAddModifyContainerDlg::OnPersistNone()
{
	auto &container = get_current_container();
	container.type &= ~(ContainerType::SAVE_ON_MISSION_PROGRESS | ContainerType::SAVE_ON_MISSION_CLOSE |
						ContainerType::SAVE_TO_PLAYER_FILE);

	CButton *button_eternal = (CButton *)GetDlgItem(IDC_CONTAINER_ETERNAL_PERSIST);
	button_eternal->SetCheck(FALSE);
	button_eternal->EnableWindow(FALSE);
}

void CAddModifyContainerDlg::OnSaveOnMissionComplete()
{
	auto &container = get_current_container();
	container.type &= ~ContainerType::SAVE_ON_MISSION_CLOSE;
	container.type |= ContainerType::SAVE_ON_MISSION_PROGRESS;

	CButton *button_eternal = (CButton *)GetDlgItem(IDC_CONTAINER_ETERNAL_PERSIST);
	if (!button_eternal->IsWindowEnabled()) {
		button_eternal->EnableWindow(TRUE);
	}
}

void CAddModifyContainerDlg::OnSaveOnMissionClose()
{
	auto &container = get_current_container();
	container.type &= ~ContainerType::SAVE_ON_MISSION_PROGRESS;
	container.type |= ContainerType::SAVE_ON_MISSION_CLOSE;

	CButton *button_eternal = (CButton *)GetDlgItem(IDC_CONTAINER_ETERNAL_PERSIST);
	if (!button_eternal->IsWindowEnabled()) {
		button_eternal->EnableWindow(TRUE);
	}
}

void CAddModifyContainerDlg::OnPersistEternal()
{
	auto &container = get_current_container();
	Assert(container.is_persistent());

	CButton *button_eternal = (CButton *)GetDlgItem(IDC_CONTAINER_ETERNAL_PERSIST);
	if (button_eternal->GetCheck()) {
		container.type |= ContainerType::SAVE_TO_PLAYER_FILE;
	} else {
		container.type &= ~ContainerType::SAVE_TO_PLAYER_FILE;
	}
}

void CAddModifyContainerDlg::set_persistence_options()
{
	CButton *button_no_persist = (CButton *)GetDlgItem(IDC_CONTAINER_NO_PERSIST);
	CButton *button_campaign_persist = (CButton *)GetDlgItem(IDC_CONTAINER_CAMPAIGN_PERSIST);
	CButton *button_player_persist = (CButton *)GetDlgItem(IDC_CONTAINER_PLAYER_PERSIST);
	CButton *button_eternal = (CButton *)GetDlgItem(IDC_CONTAINER_ETERNAL_PERSIST);
	const auto &container = get_current_container();

	if (container.is_persistent()) {
		button_no_persist->SetCheck(FALSE);

		if (any(container.type & ContainerType::SAVE_ON_MISSION_PROGRESS)) {
			Assert(none(container.type & ContainerType::SAVE_ON_MISSION_CLOSE));
			button_campaign_persist->SetCheck(TRUE);
			button_player_persist->SetCheck(FALSE);
		} else {
			Assert(any(container.type & ContainerType::SAVE_ON_MISSION_CLOSE));
			button_campaign_persist->SetCheck(FALSE);
			button_player_persist->SetCheck(TRUE);
		}

		button_eternal->SetCheck(container.is_eternal());
	} else {
		Assert(!container.is_eternal());
		button_no_persist->SetCheck(TRUE);
		button_player_persist->SetCheck(FALSE);
		button_campaign_persist->SetCheck(FALSE);
		button_eternal->SetCheck(FALSE);
	}
}


void CAddModifyContainerDlg::OnContainerAdd()
{
	add_container_entry((int)get_current_container().size());
}

void CAddModifyContainerDlg::OnContainerInsert()
{
	Assert(get_current_container().is_list());

	const int index = m_container_data_lister.GetCurSel();

	// Potential TODO: enable Insert button only when a lister entry is selected
	if (index == LB_ERR) {
		MessageBox("Nothing selected! Use Add to append new data");
		return;
	}

	add_container_entry(index);
}

void CAddModifyContainerDlg::add_container_entry(const int insert_index)
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

void CAddModifyContainerDlg::OnContainerRemove()
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
		const auto &key = m_lister_keys[index];
		container.map_data.erase(key);
	}

	if (container.empty()) {
		update_controls();
	}

	update_data_lister();
}

void CAddModifyContainerDlg::OnContainerUpdate()
{
	Assert(has_containers());
	Assert(m_current_container >= 0 && m_current_container < num_containers());
	Assert(!get_current_container().empty());

	if (!edit_boxes_have_valid_data(true)) {
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
		UNREACHABLE("Unknown container type %d", (int)container.type);
	}

	update_data_lister();
}

bool CAddModifyContainerDlg::edit_boxes_have_valid_data(bool dup_key_ok)
{
	if (!data_edit_box_has_valid_data()) {
		return false;
	}
	if (get_current_container().is_map() && !key_edit_box_has_valid_data(dup_key_ok)) {
		return false;
	}

	return true; 
}

bool CAddModifyContainerDlg::key_edit_box_has_valid_data(bool dup_ok)
{
	Assert(has_containers());
	Assert(m_current_container >= 0 && m_current_container < num_containers());
	const auto &container = get_current_container();
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
			[key_str](const std::pair<SCP_string, SCP_string> &map_entry) -> bool {
				return !stricmp(map_entry.first.c_str(), key_str);
			});

		if (count > 0) {
			MessageBox("This key already exists! Use Update to change an existing key's data.");
			return false;
		}
	}

	if (any(container.type & ContainerType::NUMBER_KEYS) && !is_valid_number(key_str)) {
		Assert(container.is_map());
		MessageBox("This key is not a valid number and this map container uses numeric keys!");
		return false;
	}

	// TODO: validate strictly typed keys

	return true; 
}

bool CAddModifyContainerDlg::data_edit_box_has_valid_data()
{
	CEdit *data_edit = (CEdit *) GetDlgItem(IDC_CONTAINER_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);

	if (data_str.IsEmpty()) {
		MessageBox("You have not entered any data!");
		return false;
	}

	if (any(get_current_container().type & ContainerType::NUMBER_DATA) && !is_valid_number(data_str)) {
		MessageBox("This data is not a valid number and this container uses numeric data!");
		return false;
	}

	// TODO: validate strictly typed data

	return true;
}

void CAddModifyContainerDlg::update_controls()
{
	update_type_controls();
	update_data_entry_controls();
}

void CAddModifyContainerDlg::update_type_controls()
{
	const auto &container = get_current_container();
	const bool container_empty = container.empty();
	const bool map_selected = container.is_map();

	// container type
	GetDlgItem(IDC_CONTAINER_LIST)->EnableWindow(container_empty);
	GetDlgItem(IDC_CONTAINER_MAP)->EnableWindow(container_empty);

	// data type
	GetDlgItem(IDC_CONTAINER_STRING_DATA)->EnableWindow(container_empty);
	GetDlgItem(IDC_CONTAINER_NUMBER_DATA)->EnableWindow(container_empty);

	// key type
	GetDlgItem(IDC_CONTAINER_STRING_KEYS)->EnableWindow(container_empty && map_selected);
	GetDlgItem(IDC_CONTAINER_NUMBER_KEYS)->EnableWindow(container_empty && map_selected);

	// persistence
	GetDlgItem(IDC_CONTAINER_ETERNAL_PERSIST)->EnableWindow(container.is_persistent());

}

void CAddModifyContainerDlg::update_data_entry_controls()
{
	const bool has_container = has_containers();
	const auto &container = get_current_container();
	const bool container_empty = container.empty();
	const bool map_selected = container.is_map();

	// data manipulation buttons
	GetDlgItem(IDC_CONTAINER_ADD)->EnableWindow(has_container);
	GetDlgItem(IDC_CONTAINER_INSERT)->EnableWindow(has_container && !map_selected);
	GetDlgItem(IDC_CONTAINER_REMOVE)->EnableWindow(has_container && !container_empty);
	GetDlgItem(IDC_CONTAINER_UPDATE)->EnableWindow(has_container && !container_empty);

	// text edit boxes
	GetDlgItem(IDC_CONTAINER_KEY)->EnableWindow(has_container && map_selected);
	GetDlgItem(IDC_CONTAINER_DATA)->EnableWindow(has_container);
}

void CAddModifyContainerDlg::update_data_lister()
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
		for (const auto &map_entry : container.map_data) {
			m_lister_keys.emplace_back(map_entry.first);
		}

		if (any(container.type & ContainerType::STRING_KEYS)) {
			std::sort(m_lister_keys.begin(), m_lister_keys.end());
		} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
			std::sort(m_lister_keys.begin(),
				m_lister_keys.end(),
				[](const SCP_string &str1, const SCP_string &str2) -> bool {
					return std::atoi(str1.c_str()) < std::atoi(str2.c_str());
				});
		} else {
			UNREACHABLE("Unknown container type %d", (int)container.type);
		}

		for (const auto &key : m_lister_keys) {
			const SCP_string  lister_entry = key + " - " + container.map_data.at(key);
			m_container_data_lister.AddString(lister_entry.c_str());
		}
	} else {
		UNREACHABLE("Unknown container type %d", (int)container.type);
	}

	m_container_data_lister.EnableWindow(m_container_data_lister.GetCount() > 0);
}

void CAddModifyContainerDlg::OnBnClickedAddNewContainer()
{
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_CURRENT_CONTAINER_NAME);

	CEditContainerNameDlg dlg("Add New Container", NEW_CONTAINER_NAME);
	dlg.DoModal();

	// if we didn't enter a name
	if (dlg.cancelled()) {
		return;
	}

	CString temp_name = dlg.new_container_name();
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
		GetDlgItem(IDC_RENAME_CONTAINER)->EnableWindow(true);
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(true);
	}

	set_selected_container();
}

void CAddModifyContainerDlg::OnBnClickedRenameContainer()
{
	Assert(has_containers());
	Assert(m_current_container >= 0);
	Assert(m_current_container < num_containers());

	CComboBox *cbox = (CComboBox *)GetDlgItem(IDC_CURRENT_CONTAINER_NAME);
	Assert(cbox->IsWindowEnabled());
	Assert(cbox->GetCount() > 0);

	auto &container = get_current_container();

	CEditContainerNameDlg dlg("Rename Container", container.container_name);
	dlg.DoModal();

	if (dlg.cancelled()) {
		return;
	}

	CString new_name = dlg.new_container_name();
	if (is_container_name_valid(new_name, true)) {
		// update renaming maps
		const SCP_string new_name_str = new_name;
		const SCP_string curr_name = get_current_container().container_name;
		const auto prev_name_it = m_new_to_old_names.find(curr_name);
		if (prev_name_it != m_new_to_old_names.end()) {
			SCP_string orig_name = prev_name_it->second;
			m_new_to_old_names.erase(curr_name);
			m_old_to_new_names[orig_name] = new_name_str;
			m_new_to_old_names[new_name_str] = orig_name;
		} else {
			m_old_to_new_names[curr_name] = new_name_str;
			m_new_to_old_names[new_name_str] = curr_name;
		}

		container.container_name = new_name;
		cbox->DeleteString((UINT)m_current_container);
		cbox->InsertString(m_current_container, container.container_name.c_str());
		cbox->SetCurSel(m_current_container);
	}
}

void CAddModifyContainerDlg::OnBnClickedDeleteContainer()
{
	Assert(has_containers()); 
	Assert(m_current_container >= 0 && m_current_container < num_containers());

	char buffer[256];

	// if the container has been renamed, search using the original name
	const SCP_string curr_name = get_current_container().container_name;
	SCP_string orig_name;
	SCP_string name_to_check;
	const auto prev_name_it = m_new_to_old_names.find(curr_name);
	if (prev_name_it != m_new_to_old_names.end()) {
		orig_name = prev_name_it->second;
		name_to_check = orig_name;
	} else {
		name_to_check = curr_name;
	}

	const int times_used = m_p_sexp_tree->get_container_usage_count(name_to_check.c_str());
	Assert(times_used >= 0);

	if (times_used > 0) {
		SCP_string container_name_str = "'" + curr_name + "'";
		if (!orig_name.empty()) {
			container_name_str += " (previously '" + orig_name + "')";
		}
		sprintf(buffer,
			"Container %s is currently used %d times. Remove those uses before deleting the container.",
			container_name_str.c_str(),
			times_used);
		MessageBox(buffer);
		return;
	}

	sprintf(buffer, "Delete SEXP container '%s'?", curr_name.c_str());

	const int ret = MessageBox(buffer, "Delete?", MB_OKCANCEL | MB_ICONEXCLAMATION);
	if (ret == IDCANCEL) {
		return;
	}

	// update renaming maps if needed
	if (!orig_name.empty()) {
		m_old_to_new_names.erase(orig_name);
		m_new_to_old_names.erase(curr_name);
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

	if (m_containers.empty()) {
		m_current_container = -1;
		Assert(cbox->GetCount() == 0);
		cbox->EnableWindow(false);
		GetDlgItem(IDC_RENAME_CONTAINER)->EnableWindow(false);
		GetDlgItem(IDC_DELETE_CONTAINER)->EnableWindow(false);
	} else {
		m_current_container = 0; 
		cbox->SetCurSel(m_current_container);
	}
	set_selected_container();
}

void CAddModifyContainerDlg::update_text_edit_boxes(const SCP_string &key, const SCP_string &data)
{
	CEdit *key_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_KEY);
	key_edit->SetWindowText(key.c_str());

	CEdit *data_edit = (CEdit*)GetDlgItem(IDC_CONTAINER_DATA);
	data_edit->SetWindowText(data.c_str());
}

sexp_container &CAddModifyContainerDlg::get_current_container()
{
	Assert(m_current_container >= -1 && m_current_container < num_containers());
	return m_current_container < 0 ? m_dummy_container : m_containers[m_current_container];
}
