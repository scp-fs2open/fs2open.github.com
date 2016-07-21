/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// TextViewDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// text_view_dlg dialog

class text_view_dlg : public CDialog
{
// Construction
public:
	void set(int ship);
	text_view_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(text_view_dlg)
	enum { IDD = IDD_TEXT_VIEW };
	CString	m_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(text_view_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(text_view_dlg)
	afx_msg void OnSetfocusEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
