/*
 * Created by Mike "MjnMixael" Nelson for The FreeSpace 2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

// CustomDataDlg.cpp : implementation file
//
#include "stdafx.h"
#include "FRED.h"
#include "customdatadlg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_RESET_FOCUS 0
#define RESET_FOCUS 1

/////////////////////////////////////////////////////////////////////////////
// CustomDataDlg dialog

CustomDataDlg::CustomDataDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CustomDataDlg::IDD, pParent)
{
}

void CustomDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_DATA_LISTER, m_data_lister);
}

// see https://stackoverflow.com/questions/17828258/how-to-prevent-mfc-dialog-closing-on-enter-and-escape-keys
// This is needed because despite the override of the normal OK and Cancel routes, Enter will still be recognized
BOOL CustomDataDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
			return TRUE; // Do not process further
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CustomDataDlg, CDialog)
ON_BN_CLICKED(IDC_CUSTOM_ADD, OnPairAdd)
ON_BN_CLICKED(IDC_CUSTOM_REMOVE, OnPairRemove)
ON_BN_CLICKED(IDC_CUSTOM_UPDATE, OnPairUpdate)

ON_BN_CLICKED(ID_OK, OnButtonOk)
ON_BN_CLICKED(ID_CANCEL, OnButtonCancel)
ON_WM_CLOSE()

ON_LBN_SELCHANGE(IDC_CUSTOM_DATA_LISTER, OnListerSelectionChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CustomDataDlg message handlers

BOOL CustomDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// grab the existing list of custom data pairs and duplicate it. We only update it if the user clicks OK.
	m_custom_data = The_mission.custom_data;

	update_data_lister();

	// Send default name and values into dialog box
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
				 // EXCEPTION: OCX Property Pages should return FALSE
}

void CustomDataDlg::OnButtonOk()
{
	// now we set the custom data to our copy
	The_mission.custom_data = m_custom_data;

	CDialog::OnOK();
}

void CustomDataDlg::OnButtonCancel()
{
	if (query_modified()) {
		int z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL) {
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
void CustomDataDlg::OnOK()
{
	// override MFC default behavior and do nothing
}

// this is called when you hit the escape key
void CustomDataDlg::OnCancel()
{
	// override MFC default behavior and do nothing
}

// this is called when you hit the Close button
void CustomDataDlg::OnClose()
{
	OnButtonCancel();
}

void CustomDataDlg::OnListerSelectionChange()
{
	const int index = m_data_lister.GetCurSel();

	if (index == LB_ERR) {
		// TODO: clear key and data text  edit boxes?
		return;
	}

	Assert(m_data_lister.GetCount() > 0);
	Assert(index >= 0 && index < static_cast<int>(m_custom_data.size()));

	const SCP_string& key = m_lister_keys[index];

	update_text_edit_boxes(key, m_custom_data[key]);
	update_help_text("No help text provided");

	for (size_t i = 0; i < Default_custom_data.size(); i++) {
		if (Default_custom_data[i].key == key) {
			update_help_text(Default_custom_data[i].description);
		}
	}

}

void CustomDataDlg::OnPairAdd()
{
	add_pair_entry(static_cast<int>(m_custom_data.size()));
}

void CustomDataDlg::add_pair_entry(const int insert_index)
{
	if (!edit_boxes_have_valid_data(false)) {
		return;
	}

	CEdit* data_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);

	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);
	m_custom_data.emplace(key_str, data_str);

	update_data_lister();
	update_text_edit_boxes("", "");
}

void CustomDataDlg::OnPairRemove()
{
	const int index = m_data_lister.GetCurSel();

	if (index == LB_ERR) {
		MessageBox("Nothing selected! You must select something to remove!");
		return;
	}

	const auto& key = m_lister_keys[index];
	m_custom_data.erase(key);

	update_data_lister();
	update_help_text("");
}

void CustomDataDlg::OnPairUpdate()
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
	m_custom_data.erase(key);

	CEdit* data_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA);
	CString data_str;
	data_edit->GetWindowText(data_str);
	
	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);
	m_custom_data[SCP_string(key_str)] = data_str;

	update_data_lister();
	update_text_edit_boxes("", "");
}

bool CustomDataDlg::edit_boxes_have_valid_data(bool dup_key_ok)
{
	if (!data_edit_box_has_valid_data()) {
		return false;
	}
	if (!key_edit_box_has_valid_data(dup_key_ok)) {
		return false;
	}

	return true;
}

bool CustomDataDlg::key_edit_box_has_valid_data(bool dup_key_ok)
{

	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	CString key_str;
	key_edit->GetWindowText(key_str);

	if (key_str.IsEmpty()) {
		MessageBox("You have not entered a key!");
		return false;
	}

	if (key_str.Find(" ") != -1) {
		MessageBox("Keys cannot contain spaces!");
		return false;
	}

	// Only check for duplicate keys if there are other keys to check!
	if (m_lister_keys.size() > 0) {

		const int index = m_data_lister.GetCurSel();
		SCP_string this_key = "";

		// If we're here then we're updating a pair and the key can be "duplicated"
		// because it's not actually being changed
		if (dup_key_ok && SCP_vector_inbounds(m_lister_keys, index)) {
			this_key = m_lister_keys[index];
		}

		if (strcmp(this_key.c_str(), key_str)) {
			for (const auto& pair : m_custom_data) {
				const CString key = pair.first.c_str();
				if (key == key_str) {
					MessageBox("Keys must be unique!");
					return false;
				}
			}
		}
	}

	return true;
}

bool CustomDataDlg::data_edit_box_has_valid_data()
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

void CustomDataDlg::update_data_lister()
{
	m_data_lister.ResetContent();
	m_lister_keys.clear();

	for (const auto& pair : m_custom_data) {
		m_lister_keys.emplace_back(pair.first);
		const SCP_string lister_entry = pair.first + " - " + pair.second;
		m_data_lister.AddString(lister_entry.c_str());
	}

	m_data_lister.EnableWindow(m_data_lister.GetCount() > 0);
}

void CustomDataDlg::update_text_edit_boxes(const SCP_string& key, const SCP_string& data)
{
	CEdit* key_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_KEY);
	key_edit->SetWindowText(key.c_str());

	CEdit* data_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA);
	data_edit->SetWindowText(data.c_str());
}

void CustomDataDlg::update_help_text(const SCP_string& description)
{
	CEdit* desc_edit = (CEdit*)GetDlgItem(IDC_CUSTOM_DATA_DESC);
	desc_edit->SetWindowText(description.c_str());

}

bool CustomDataDlg::query_modified() const
{
	return The_mission.custom_data != m_custom_data;
}
