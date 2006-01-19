/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// InitialShips.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// InitialShips dialog

#define INITIAL_SHIPS	1
#define INITIAL_WEAPONS	2

#define MAX_INITIAL_CHECKBOXES	30

class InitialShips : public CDialog
{
// Construction
public:
	int m_initial_items;
	InitialShips(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(InitialShips)
	enum { IDD = IDD_INITIAL_SHIPS };
	CCheckListBox	m_initial_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(InitialShips)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(InitialShips)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_list_count;
};
