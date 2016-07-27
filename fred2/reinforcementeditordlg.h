/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "ship/ship.h"

/////////////////////////////////////////////////////////////////////////////
// reinforcement_editor_dlg dialog

class reinforcement_editor_dlg : public CDialog
{
// Construction
public:
	int query_modified();
	void OnOK();
	void OnCancel();
	void save_data();
	void update_data();
	reinforcement_editor_dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(reinforcement_editor_dlg)
	enum { IDD = IDD_REINFORCEMENT_EDITOR };
	CSpinButtonCtrl	m_delay_spin;
	CSpinButtonCtrl	m_uses_spin;
	int		m_uses;
	int		m_delay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(reinforcement_editor_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void remove_selected( CListBox *box );
	void move_messages( CListBox *box );

	// Generated message map functions
	//{{AFX_MSG(reinforcement_editor_dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_num_reinforcements;
	reinforcements m_reinforcements[MAX_REINFORCEMENTS];
	int cur;
};

/////////////////////////////////////////////////////////////////////////////
// reinforcement_select dialog

class reinforcement_select : public CDialog
{
// Construction
public:
	int cur;
	char name[NAME_LENGTH];
	reinforcement_select(CWnd* pParent = NULL);   // standard constructor
	void OnOK();
	void OnCancel();

// Dialog Data
	//{{AFX_DATA(reinforcement_select)
	enum { IDD = IDD_REINFORCEMENT_SELECT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(reinforcement_select)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(reinforcement_select)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
