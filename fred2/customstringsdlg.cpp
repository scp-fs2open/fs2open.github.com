/*
 * Created by Mike "MjnMixael" Nelson for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

// CustomStringsDlg.cpp : implementation file
//
#include "stdafx.h"
#include "FRED.h"
#include "customstringsdlg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_RESET_FOCUS 0
#define RESET_FOCUS 1

/////////////////////////////////////////////////////////////////////////////
// CustomStringsDlg dialog

CustomStringsDlg::CustomStringsDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CustomStringsDlg::IDD, pParent)
{
}

void CustomStringsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_DATA_LISTER, m_data_lister);
}

// see https://stackoverflow.com/questions/17828258/how-to-prevent-mfc-dialog-closing-on-enter-and-escape-keys
// This is needed because despite the override of the normal OK and Cancel routes, Enter will still be recognized
BOOL CustomStringsDlg::PreTranslateMessage(MSG* pMsg)
{
	if (!m_text_edit_focus && pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
			return TRUE; // Do not process further
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CustomStringsDlg, CDialog)
ON_BN_CLICKED(IDC_CUSTOM_ADD, OnStringAdd)
ON_BN_CLICKED(IDC_CUSTOM_REMOVE, OnStringRemove)
ON_BN_CLICKED(IDC_CUSTOM_UPDATE, OnStringUpdate)

ON_BN_CLICKED(ID_OK, OnButtonOk)
ON_BN_CLICKED(ID_CANCEL, OnButtonCancel)
ON_WM_CLOSE()

ON_LBN_SELCHANGE(IDC_CUSTOM_DATA_LISTER, OnListerSelectionChange)

ON_EN_SETFOCUS(IDC_CUSTOM_STRING, OnEnSetFocusEditString)
ON_EN_KILLFOCUS(IDC_CUSTOM_STRING, OnEnKillFocusEditString)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CustomStringsDlg message handlers

BOOL CustomStringsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_text_edit_focus = false;
	m_modified = false;

	// grab the existing list of custom strings and duplicate it. We only update it if the user clicks OK.
	m_custom_strings = The_mission.custom_strings;

	update_data_lister();

	// Send default name and values into dialog box
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CustomStringsDlg::OnButtonOk()
{
	// now we set the custom data to our copy
	The_mission.custom_strings = m_custom_strings;

	CDialog::OnOK();
}

void CustomStringsDlg::OnButtonCancel()
{
	if (query_modified()) {
		int z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL) {
			m_modified = false;
			return;
		}

		if (z == IDYES) {
			OnButtonOk();
			return;
		}
	}

	CDialog::OnCancel();
}

// this is called when you hit the enter key
void CustomStringsDlg::OnOK()
{
	// override MFC default behavior and do nothing
}

// this is called when you hit the escape key
void CustomStringsDlg::OnCancel()
{
	// override MFC default behavior and do nothing
}

// this is called when you hit the Close button
void CustomStringsDlg::OnClose()
{
	OnButtonCancel();
}

void CustomStringsDlg::OnListerSelectionChange()
{
	const int index = m_data_lister.GetCurSel();

	if (index == LB_ERR) {
		update_text_edit_boxes("", "", "");
		return;
	}

	Assertion(m_data_lister.GetCount() > 0, "Attempt to change custom string selection when there are no custom strings! Please report.");
	Assertion(index >= 0 && index < static_cast<int>(m_custom_strings.size()), "Selected an invalid custom string! Please report.");

	const SCP_string& key = m_lister_keys[index];
	const custom_string *cs = nullptr;

	for (size_t i = 0; i <= m_custom_strings.size(); i++) {
		if (m_custom_strings[i].name == key) {
			cs = &m_custom_strings[i];
			break;
		}
	}

	if (cs != nullptr) {
		update_text_edit_boxes(cs->name, cs->value, cs->text);
	}

}

void CustomStringsDlg::OnStringAdd()
{
	add_pair_entry();
}

void CustomStringsDlg::add_pair_entry()
{
	if (!edit_boxes_have_valid_data()) {
		return;
	}

	custom_string cs;

	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);
	cs.name = key_str;

	CEdit* data_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);
	cs.value = data_str;

	CEdit* text_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_STRING);
	CString text_str;
	text_edit->GetWindowText(text_str);
	cs.text = text_str;

	m_custom_strings.push_back(cs);
	m_modified = true;

	update_data_lister();
	update_text_edit_boxes("", "", "");
}

void CustomStringsDlg::OnStringRemove()
{
	const int index = m_data_lister.GetCurSel();

	if (index == LB_ERR) {
		MessageBox("Nothing selected! You must select something to remove!");
		return;
	}

	const auto& key = m_lister_keys[index];

	for (size_t i = 0; i <= m_custom_strings.size(); i++) {
		if (m_custom_strings[i].name == key) {
			m_custom_strings.erase(m_custom_strings.begin() + i);
			break;
		}
	}
	m_modified = true;

	update_data_lister();
	update_text_edit_boxes("", "", "");
}

void CustomStringsDlg::OnStringUpdate()
{
	if (!edit_boxes_have_valid_data(true)) {
		return;
	}

	const int index = m_data_lister.GetCurSel();

	if (index == LB_ERR) {
		MessageBox("Nothing selected! Use Add to enter new data");
		return;
	}

	const auto& key = m_lister_keys[index];

	size_t i;

	for (i = 0; i <= m_custom_strings.size(); i++) {
		if (m_custom_strings[i].name == key) {
			break;
		}
	}
	
	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);
	m_custom_strings[i].name = key_str;

	CEdit* data_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);
	m_custom_strings[i].value = data_str;

	CEdit* text_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_STRING);
	CString text_str;
	text_edit->GetWindowText(text_str);
	m_custom_strings[i].text = text_str;

	m_modified = true;

	update_data_lister();
	update_text_edit_boxes("", "", "");
}

bool CustomStringsDlg::edit_boxes_have_valid_data(bool update)
{
	if (!data_edit_box_has_valid_data()) {
		return false;
	}

	if (!text_edit_box_has_valid_data()) {
		return false;
	}

	if (!key_edit_box_has_valid_data(update)) {
		return false;
	}

	return true;
}

bool CustomStringsDlg::key_edit_box_has_valid_data(bool update)
{

	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);

	if (key_str.IsEmpty()) {
		MessageBox("You have not entered a key!");
		return false;
	}

	if (key_str.Find(" ") != -1) {
		MessageBox("Names cannot contain spaces!");
		return false;
	}

	for (const auto& cs : m_custom_strings) {
		const CString key = cs.name.c_str();
		if (update) {
			const int index = m_data_lister.GetCurSel();

			if (!SCP_vector_inbounds(m_lister_keys, index)) {
				MessageBox("Must select an item to update!");
				return false;
			}

			const auto& this_key = m_lister_keys[index];
			if (!strcmp(this_key.c_str(), key)) {
				continue;
			}
		}
		if (key == key_str) {
			MessageBox("Names must be unique!");
			return false;
		}
	}

	return true;
}

bool CustomStringsDlg::data_edit_box_has_valid_data()
{
	CEdit* data_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);

	if (data_str.IsEmpty()) {
		MessageBox("You have not entered any data!");
		return false;
	}

	return true;
}

bool CustomStringsDlg::text_edit_box_has_valid_data()
{
	CEdit* text_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_STRING);
	CString text_str;
	text_edit->GetWindowText(text_str);

	if (text_str.IsEmpty()) {
		MessageBox("You have not entered any text!");
		return false;
	}

	return true;
}

void CustomStringsDlg::update_data_lister()
{
	m_data_lister.ResetContent();
	m_lister_keys.clear();

	for (const auto& cs : m_custom_strings) {
		m_lister_keys.emplace_back(cs.name);
		const SCP_string lister_entry = cs.name + " - " + cs.value;
		m_data_lister.AddString(lister_entry.c_str());
	}

	m_data_lister.EnableWindow(m_data_lister.GetCount() > 0);
}

void CustomStringsDlg::update_text_edit_boxes(const SCP_string& key, const SCP_string& value, const SCP_string& text)
{
	
	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	key_edit->SetWindowText(key.c_str());

	CEdit* data_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA);
	data_edit->SetWindowText(value.c_str());

	CEdit* data_text = (CEdit*)GetDlgItem(IDC_CUSTOM_STRING);
	data_text->SetWindowText(text.c_str());
}

void CustomStringsDlg::OnEnSetFocusEditString()
{
	m_text_edit_focus = true;
}

void CustomStringsDlg::OnEnKillFocusEditString()
{
	m_text_edit_focus = false;
}

bool CustomStringsDlg::query_modified() const
{
	return m_modified;
}
