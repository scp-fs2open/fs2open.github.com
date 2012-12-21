/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// adjust_grid_dlg dialog

class adjust_grid_dlg : public CDialog
{
// Construction
public:
	void OnOK();
	adjust_grid_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(adjust_grid_dlg)
	enum { IDD = IDD_ADJUST_GRID };
	CSpinButtonCtrl	m_spinz;
	CSpinButtonCtrl	m_spiny;
	CSpinButtonCtrl	m_spinx;
	int		m_x;
	int		m_y;
	int		m_z;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(adjust_grid_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(adjust_grid_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnXyPlane();
	afx_msg void OnXzPlane();
	afx_msg void OnYzPlane();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
