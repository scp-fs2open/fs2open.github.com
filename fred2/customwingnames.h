#if !defined(AFX_USTOMWINGNAME_H__D30D7519_D2C9_4907_8D87_EDB690B30608__INCLUDED_)
#define AFX_USTOMWINGNAME_H__D30D7519_D2C9_4907_8D87_EDB690B30608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomWingNames.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CustomWingNames dialog

class CustomWingNames : public CDialog
{
// Construction
public:
	CustomWingNames(CWnd* pParent = NULL);   // standard constructor
	int query_modified();
	void OnOK();
	void OnCancel();

// Dialog Data
	//{{AFX_DATA(CustomWingNames)
	enum { IDD = IDD_CUSTOM_WING_NAMES };
	CString	m_squadron_1;
	CString	m_squadron_2;
	CString	m_squadron_3;
	CString	m_squadron_4;
	CString	m_squadron_5;
	CString	m_starting_1;
	CString	m_starting_2;
	CString	m_starting_3;
	CString	m_tvt_1;
	CString	m_tvt_2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CustomWingNames)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CustomWingNames)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USTOMWINGNAME_H__D30D7519_D2C9_4907_8D87_EDB690B30608__INCLUDED_)
