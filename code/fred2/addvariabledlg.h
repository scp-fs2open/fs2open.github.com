/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#if !defined(AFX_ADDVARIABLEDLG_H__0F668CB5_AAEE_11D2_A899_0060088FAE88__INCLUDED_)
#define AFX_ADDVARIABLEDLG_H__0F668CB5_AAEE_11D2_A899_0060088FAE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddVariableDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddVariableDlg dialog

class CAddVariableDlg : public CDialog
{
// Construction
public:
	CAddVariableDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddVariableDlg)
	enum { IDD = IDD_ADD_VARIABLE };
	CString	m_default_value;
	CString	m_variable_name;
	bool		m_name_validated;
	bool		m_data_validated;
	bool		m_type_number;
	bool		m_type_campaign_persistent;
	bool		m_type_player_persistent;
	bool		m_type_network_variable;
	bool		m_create;
	int		m_sexp_var_index;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddVariableDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddVariableDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void validate_variable_name(int set_focus);
	afx_msg void validate_data(int set_focus);
	afx_msg void OnTypeNumber();
	afx_msg void OnTypeString();
	afx_msg void OnTypePlayerPersistent();
	afx_msg void OnTypeCampaignPersistent();
	afx_msg void OnTypeNetworkVariable();
	afx_msg void set_variable_type();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDVARIABLEDLG_H__0F668CB5_AAEE_11D2_A899_0060088FAE88__INCLUDED_)
