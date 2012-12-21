/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



extern int Shield_sys_teams[MAX_IFFS];
extern int Shield_sys_types[MAX_SHIP_CLASSES];

/////////////////////////////////////////////////////////////////////////////
// shield_sys_dlg dialog

class shield_sys_dlg : public CDialog
{
// Construction
public:
	void set_team();
	void set_type();
	shield_sys_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(shield_sys_dlg)
	enum { IDD = IDD_SHIELD_SYS };
	int		m_team;
	int		m_type;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(shield_sys_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(shield_sys_dlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTeam();
	afx_msg void OnSelchangeType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
