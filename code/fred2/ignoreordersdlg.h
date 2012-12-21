/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "ship/ship.h"

// we won't have more than 9 checkboxes per dialog
#define MAX_CHECKBOXES		10

/////////////////////////////////////////////////////////////////////////////
// ignore_orders_dlg dialog

typedef struct check_box_info {
	CButton *button;
	int		id;
} check_box_info;

class ignore_orders_dlg : public CDialog
{
// Construction
public:
	int m_ship;
	ignore_orders_dlg(CWnd* pParent = NULL);   // standard constructor

	ship *m_shipp;
	check_box_info check_boxes[MAX_CHECKBOXES];
	int m_num_checks_active;

// Dialog Data
	//{{AFX_DATA(ignore_orders_dlg)
	enum { IDD = IDD_IGNORE_ORDERS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ignore_orders_dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ignore_orders_dlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheck1();
	afx_msg void OnCheck2();
	afx_msg void OnCheck3();
	afx_msg void OnCheck4();
	afx_msg void OnCheck5();
	afx_msg void OnCheck6();
	afx_msg void OnCheck7();
	afx_msg void OnCheck8();
	afx_msg void OnCheck9();
	afx_msg void OnCheck10();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
