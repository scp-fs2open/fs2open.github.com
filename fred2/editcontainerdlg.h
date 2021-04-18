/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#pragma once

#include "parse/sexp.h"

/////////////////////////////////////////////////////////////////////////////
// Edit (Add/Modify) Container dialog

class CEditContainerDlg : public CDialog
{
public:
	CEditContainerDlg(sexp_tree *p_sexp_tree, CWnd *pParent = nullptr);

	enum { IDD = IDD_EDIT_CONTAINER };

	CListBox	m_container_data_lister;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	void OnOK() override;
	BOOL OnInitDialog() override;

	afx_msg void OnTypeMap();
	afx_msg void OnTypeList();

	afx_msg void OnTypeNumber();
	afx_msg void OnTypeString();

	void set_container_type();
	void set_data_type();
	void set_key_type();

	afx_msg void OnEditchangeContainerName();
	afx_msg void OnSelchangeContainerName(); 
	void set_selected_container(); 

	afx_msg void OnContainerAdd();
	afx_msg void OnContainerInsert();
	afx_msg void OnContainerRemove();
	afx_msg void OnContainerUpdate();
	
	afx_msg void ListerSelectionGetIter(SCP_vector<SCP_string>::iterator &iter);
	afx_msg void OnListerSelectionChange();

	void update_controls();
	void update_type_controls();
	void update_data_entry_controls();
	void update_data_lister();
	void update_text_edit_boxes(const SCP_string &key, const SCP_string &data);

	bool edit_boxes_have_valid_data(bool dup_key_ok);
	bool data_edit_box_has_valid_data();
	bool key_edit_box_has_valid_data(bool dup_ok);

	void add_container_entry(int insert_index);

	bool is_container_name_in_use(const char *text, bool ignore_current) const;
	bool is_container_name_valid(CString &new_name, bool is_rename);
	bool is_valid_number(const char *test_str) const;

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedAddNewContainer();
	afx_msg void OnBnClickedStringKeys();
	afx_msg void OnBnClickedNumberKeys();
	afx_msg void OnBnClickedDeleteContainer();

private:
	bool has_containers() const { return !m_containers.empty(); }
	int num_containers() const { return (int)m_containers.size(); }
	sexp_container &get_current_container();

	const sexp_tree * const m_p_sexp_tree;
	SCP_vector<sexp_container> m_containers;
	// used when m_containers is empty
	sexp_container m_dummy_container; // used as placeholder when there are no containers
	SCP_vector<SCP_string> m_lister_keys; // read-only view of list data or map keys
	int			m_current_container = -1;
};
