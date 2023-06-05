/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _JUMPNODEDLG_H
#define _JUMPNODEDLG_H

/////////////////////////////////////////////////////////////////////////////
// jumpnode_dlg dialog

class jumpnode_dlg : public CDialog
{
// Construction
public:
	int bypass_errors;
	int update_data();
	void initialize_data(int full_update);
	void OnOK();
	BOOL Create();
	jumpnode_dlg(CWnd* pParent = NULL); // standard constructor

// Dialog Data
	//{{AFX_DATA(jumpnode_dlg)
	enum { IDD = IDD_JUMPNODE_EDITOR };
	CString	m_name;
	CString m_display;
	CString m_filename;
	int m_color_r;
	int m_color_g;
	int m_color_b;
	int m_color_a;
	int m_hidden;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(jumpnode_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(jumpnode_dlg)
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnClose();
	afx_msg void OnHidden();
	afx_msg void OnKillfocusName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
