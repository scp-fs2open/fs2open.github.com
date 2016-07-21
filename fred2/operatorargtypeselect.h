/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// OperatorArgTypeSelect dialog

class OperatorArgTypeSelect : public CDialog
{
// Construction
public:
	OperatorArgTypeSelect(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(OperatorArgTypeSelect)
	enum { IDD = IDD_OPERATOR_ARGUMENT_TYPES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OperatorArgTypeSelect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OperatorArgTypeSelect)
	afx_msg void OnBoolean();
	afx_msg void OnNumbers();
	afx_msg void OnShips();
	afx_msg void OnWings();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
