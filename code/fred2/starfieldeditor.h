/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// starfield_editor dialog

class starfield_editor : public CDialog
{
// Construction
public:
	void OnOK();
	void OnCancel();
	starfield_editor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(starfield_editor)
	enum { IDD = IDD_STARFIELD };
	CSliderCtrl			m_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(starfield_editor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	int initialized;

	// Generated message map functions
	//{{AFX_MSG(starfield_editor)
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableAsteroids();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
