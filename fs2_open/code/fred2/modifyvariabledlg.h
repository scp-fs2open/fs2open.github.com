/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#if !defined(AFX_MODIFYVARIABLEDLG_H__710D45F1_ABBF_11D2_A89A_0060088FAE88__INCLUDED_)
#define AFX_MODIFYVARIABLEDLG_H__710D45F1_ABBF_11D2_A89A_0060088FAE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyVariableDlg.h : header file
//

#include "parse/sexp.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyVariableDlg dialog

class CModifyVariableDlg : public CDialog
{
// Construction
public:
	CModifyVariableDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyVariableDlg)
	enum { IDD = IDD_MODIFY_VARIABLE };
	CString		m_cur_variable_name;
	CString		m_default_value;
	CString		m_old_var_name;
	bool			m_type_number;
	bool			m_type_player_persistent;
	bool			m_type_campaign_persistent;
	bool			m_type_network_variable;
	bool			m_modified_name;
	bool			m_modified_value;
	bool			m_modified_type;
	bool			m_modified_persistence;
	bool			m_deleted;
	bool			m_data_validated;
	bool			m_var_name_validated;
	bool			m_do_modify;
	int			m_combo_last_modified_index;
	int			m_translate_combo_to_sexp[MAX_SEXP_VARIABLES];
	int			m_start_index;		// index of sexp_variables which is right clicked to get this menu
	sexp_tree	*m_p_sexp_tree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyVariableDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

private: 
	bool IsChangeSafe(char *message);

protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyVariableDlg)
	afx_msg void OnDeleteVariable();
	afx_msg void OnTypeString();
	afx_msg void OnTypeNumber();
	afx_msg void OnTypePlayerPersistent();
	afx_msg void OnTypeCampaignPersistent();
	afx_msg void OnTypeNetworkVariable();
	afx_msg void OnSelchangeModifyVariableName();
	afx_msg void OnEditchangeModifyVariableName();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnKillfocusModifyDefaultValue();
	afx_msg void set_variable_type();
	afx_msg void validate_data(CString &temp_data, int set_focus);
	afx_msg void validate_var_name(int set_focus);
	afx_msg	int get_sexp_var_index();
	afx_msg void OnDropdownModifyVariableName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYVARIABLEDLG_H__710D45F1_ABBF_11D2_A89A_0060088FAE88__INCLUDED_)
