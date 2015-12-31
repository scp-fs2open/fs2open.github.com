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
	
	SCP_vector<sexp_container> edit_sexp_containers;
	enum { IDD = IDD_EDIT_CONTAINER };

	int			m_current_container;
	bool		m_type_list;
	bool		m_type_map;
	bool		m_type_number;
	bool		m_key_type_number;
	bool		m_data_changed;

	SCP_vector<SCP_string> raw_data;
	
	CListBox	m_container_data_lister;
	sexp_tree	*m_p_sexp_tree;
	//}}AFX_DATA

	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditContainerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditContainerDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	afx_msg void OnTypeMap();
	afx_msg void OnTypeList();

	afx_msg void OnTypeNumber();
	afx_msg void OnTypeString();

	void set_container_type();
	void set_argument_type();
	void set_key_type();

	afx_msg void OnEditchangeContainerName();
	afx_msg void OnSelchangeContainerName(); 
	void set_selected_container(int selected_container); 

	afx_msg void OnContainerAdd();
	afx_msg void OnContainerInsert();
	afx_msg void OnContainerRemove();
	afx_msg void OnContainerUpdate();
	
	afx_msg void ListerSelectionGetIter(SCP_vector<SCP_string>::iterator &iter);
	afx_msg void OnListerSelectionChange();
	afx_msg void updata_data_lister();

	void set_lister_data(int index);

	BOOL edit_boxes_have_valid_data();
	BOOL data_edit_box_has_valid_data();
	BOOL key_edit_box_has_valid_data();

	int get_index_edit_sexp_container_name(const char *text);
	BOOL is_container_name_valid(CString &new_name);
	BOOL is_data_valid();
	BOOL is_valid_number(SCP_string test_string);

	BOOL save_current_container();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAddNewContainer();
	afx_msg void OnBnClickedStringKeys();
	afx_msg void OnBnClickedNumberKeys();
	afx_msg void OnBnClickedDeleteContainer();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditContainerDlg_H)