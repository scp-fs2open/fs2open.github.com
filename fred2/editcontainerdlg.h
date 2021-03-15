/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#if !defined(AFX_EditContainerDlg_H)
#define AFX_EditContainerDlg_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditContainerDlg.h : header file
//

#include "parse/sexp.h"

/////////////////////////////////////////////////////////////////////////////
// Edit Container dialog

class CEditContainerDlg : public CDialog
{
// Construction
public:
	CEditContainerDlg(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CEditContainerDlg)
	enum { IDD = IDD_EDIT_CONTAINER };

	CListBox	m_container_data_lister;
	sexp_tree	*m_p_sexp_tree;
	//}}AFX_DATA

	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditContainerDlg)
  protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditContainerDlg)
	void OnOK() override;
	BOOL OnInitDialog() override;

	// TODO OnContainerType* 
	afx_msg void OnTypeMap();
	afx_msg void OnTypeList();

	afx_msg void OnTypeNumber();
	afx_msg void OnTypeString();

	// TODO: change set_ to update_
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
	void update_data_lister();
	void update_map_specific_controls();
	void update_text_edit_boxes(const SCP_string &key, const SCP_string &data);

	//todo validate_*_boxws
	BOOL edit_boxes_have_valid_data();
	BOOL data_edit_box_has_valid_data();
	BOOL key_edit_box_has_valid_data();

	void add_container_entry(int insert_index);

	bool is_container_name_in_use(const char *text) const;
	BOOL is_container_name_valid(CString &new_name);
	//BOOL is_data_valid();
	BOOL is_valid_number(SCP_string test_string);

	//BOOL save_current_container();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAddNewContainer();
	afx_msg void OnBnClickedStringKeys();
	afx_msg void OnBnClickedNumberKeys();
	afx_msg void OnBnClickedDeleteContainer();

private:
	sexp_container &get_current_container();

	SCP_vector<sexp_container> edit_sexp_containers;
	SCP_vector<SCP_string> m_lister_keys; // read-only view of list data or map keys
	int			m_current_container = -1;
	// FIXME TODO: maybe get rid of these
	//SCP_string m_key;
	//SCP_string m_data;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditContainerDlg_H)